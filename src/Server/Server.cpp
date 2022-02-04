#include "Server.hpp"
#include "../Utilities/Utilities.hpp"
#include <chrono>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <thread>
#include <vector>

static const std::unordered_map<std::string, nlohmann::json (Server::*)(const nlohmann::json &)> ServiceMap = {
    {"echo", &Server::Echo},
    {"add_game", &Server::AddGame},
    {"add_state", &Server::AddState},
    {"add_player", &Server::AddPlayer},
    {"add_action_generator", &Server::AddActionGenerator},
    {"generate_actions", &Server::GenerateActions},
    {"take_action", &Server::TakeAction},
    {"start_thinking", &Server::StartThinking},
    {"stop_thinking", &Server::StopThinking},
    {"get_best_action", &Server::GetBestAction},
    {"query_details", &Server::QueryDetails},
};

void Server::Run() {
    std::string reqStr;
    while (true) {
        std::getline(_InputStream, reqStr);
        std::thread(Serve, this, std::move(reqStr)).detach();
    }
}

void Server::Serve(Server *self, std::string &&reqStr) {
    nlohmann::json response;
    try {
        const auto request = nlohmann::json::parse(reqStr);
        if (request.contains("id"))
            response["id"] = request["id"];
        Util::GetJsonValidator("request.schema.json").validate(request);
        const std::string &type = request["type"];
        const nlohmann::json &data = request["data"];
        const auto service = ServiceMap.at(type);
        response["data"] = (self->*service)(data);
        response["success"] = true;
    } catch (const std::exception &e) {
        response["errMsg"] = e.what();
        response["success"] = false;
    }
    const std::scoped_lock lock(self->_MtxOutputStream);
    self->_OutputStream << response << std::endl;
}

nlohmann::json Server::Echo(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/echo.schema.json").validate(data);
    const std::chrono::duration<double> time(data["sleepTime"]);
    std::this_thread::sleep_for(time);
    if (!data.contains("data"))
        return nlohmann::json::object();
    return {{"data", data["data"]}};
}

nlohmann::json Server::AddGame(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/add_game.schema.json").validate(data);
    const auto id = ++_GameCount;
    auto game = Game::Create(data["type"], data["data"]);
    _GameList.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(std::move(game)));
    return {{"gameID", id}};
}

nlohmann::json Server::AddState(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/add_state.schema.json").validate(data);
    const auto id = ++_StateCount;
    auto &gameRecord = _GameList.at(data["gameID"]);
    auto state =
        data.contains("data") ? State::Create(*gameRecord.GamePtr, data["data"]) : State::Create(*gameRecord.GamePtr);
    auto recordPtr = &_StateList
                          .emplace(std::piecewise_construct, std::forward_as_tuple(id),
                                   std::forward_as_tuple(std::move(state), &gameRecord))
                          .first->second;
    // Data race may occur here (`gameRecord` being removed), but it is the client's responsibility to avoid it
    gameRecord.SubStates.push_back(recordPtr);
    return {{"stateID", id}};
}

nlohmann::json Server::AddPlayer(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/add_player.schema.json").validate(data);
    const auto id = ++_PlayerCount;
    auto &stateRecord = _StateList.at(data["stateID"]);
    const auto &gameRecord = *stateRecord.ParentGame;
    auto player = Player::Create(data["type"], *gameRecord.GamePtr, *stateRecord.StatePtr, data["data"]);
    auto recordPtr = &_PlayerList
                          .emplace(std::piecewise_construct, std::forward_as_tuple(id),
                                   std::forward_as_tuple(std::move(player), &stateRecord))
                          .first->second;
    // Data race may occur here (`stateRecord` being removed), but it is the client's responsibility to avoid it
    stateRecord.SubPlayers.push_back(recordPtr);
    return {{"playerID", id}};
}

nlohmann::json Server::AddActionGenerator(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/add_action_generator.schema.json").validate(data);
    const auto id = ++_ActionGeneratorCount;
    auto &stateRecord = _StateList.at(data["stateID"]);
    const auto &gameRecord = *stateRecord.ParentGame;
    auto actionGenerator =
        ActionGenerator::Create(data["type"], *gameRecord.GamePtr, *stateRecord.StatePtr, data["data"]);
    auto actionGeneratorData = ActionGenerator::Data::Create(*actionGenerator);
    auto recordPtr =
        &_ActionGeneratorList
             .emplace(std::piecewise_construct, std::forward_as_tuple(id),
                      std::forward_as_tuple(std::move(actionGenerator), std::move(actionGeneratorData), &stateRecord))
             .first->second;
    // Data race may occur here (`stateRecord` being removed), but it is the client's responsibility to avoid it
    stateRecord.SubActionGenerators.push_back(recordPtr);
    return {{"actionGeneratorID", id}};
}

nlohmann::json Server::GenerateActions(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/generate_actions.schema.json").validate(data);
    const auto &actionGeneratorRecord = _ActionGeneratorList.at(data["actionGeneratorID"]);
    const auto &actionGenerator = *actionGeneratorRecord.ActionGeneratorPtr;
    const auto &actionGeneratorData = *actionGeneratorRecord.ActionGeneratorDataPtr;
    nlohmann::json actions;
    actionGenerator.ForEach(actionGeneratorData, [&](const Action &action) { actions.push_back(action.GetJson()); });
    return {{"actions", actions}};
}

nlohmann::json Server::TakeAction(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/take_action.schema.json").validate(data);
    const auto &stateRecord = _StateList.at(data["stateID"]);
    const auto &game = *stateRecord.ParentGame->GamePtr;
    auto &state = *stateRecord.StatePtr;
    const auto action = Action::Create(game, data["action"]);
    if (!game.IsValidAction(state, *action))
        throw std::invalid_argument("The action is invalid");
    std::optional<std::vector<double>> result;
    {
        if (!stateRecord.MtxState.try_lock())
            throw std::invalid_argument("The specified state is busy");
        const std::scoped_lock lock(std::adopt_lock, stateRecord.MtxState);
        result = game.TakeAction(state, *action);
    }
    // Data race may occur here (updating a state being computed),
    // but using locks will cause the request to be blocked by the computations of players and action generators,
    // so locks are not used here, it is the client's responsibility to avoid data races
    for (const auto &player : stateRecord.SubPlayers)
        player->PlayerPtr->Update(*action);
    for (const auto &actionGenerator : stateRecord.SubActionGenerators)
        actionGenerator->ActionGeneratorPtr->Update(*actionGenerator->ActionGeneratorDataPtr, *action);
    nlohmann::json response = {
        {"finished", result.has_value()},
        {"state", state.GetJson()},
    };
    if (result)
        response["result"] = *result;
    return response;
}

nlohmann::json Server::StartThinking(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/start_thinking.schema.json").validate(data);
    const auto &playerRecord = _PlayerList.at(data["playerID"]);
    {
        if (!playerRecord.MtxPlayer.try_lock())
            throw std::invalid_argument("The specified player is busy");
        const std::scoped_lock lock(std::adopt_lock, playerRecord.MtxPlayer);
        playerRecord.PlayerPtr->StartThinking();
    }
    return nlohmann::json::object();
}

nlohmann::json Server::StopThinking(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/stop_thinking.schema.json").validate(data);
    const auto &playerRecord = _PlayerList.at(data["playerID"]);
    {
        if (!playerRecord.MtxPlayer.try_lock())
            throw std::invalid_argument("The specified player is busy");
        const std::scoped_lock lock(std::adopt_lock, playerRecord.MtxPlayer);
        playerRecord.PlayerPtr->StopThinking();
    }
    return nlohmann::json::object();
}

nlohmann::json Server::GetBestAction(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/get_best_action.schema.json").validate(data);
    const auto &playerRecord = _PlayerList.at(data["playerID"]);
    std::optional<std::chrono::duration<double>> time;
    if (data.contains("maxThinkTime"))
        time = std::chrono::duration<double>(data["maxThinkTime"]);
    std::unique_ptr<Action> bestAction;
    {
        if (!playerRecord.MtxPlayer.try_lock())
            throw std::invalid_argument("The specified player is busy");
        const std::scoped_lock lock(std::adopt_lock, playerRecord.MtxPlayer);
        bestAction = playerRecord.PlayerPtr->GetBestAction(time);
    }
    return {{"action", bestAction->GetJson()}};
}

nlohmann::json Server::QueryDetails(const nlohmann::json &) {
    // TODO
    return {};
}
