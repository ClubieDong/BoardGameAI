#include "Server.hpp"
#include "../Utilities/Utilities.hpp"
#include <chrono>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <thread>
#include <vector>

const std::unordered_map<std::string, nlohmann::json (Server::*)(const nlohmann::json &)> Server::_ServiceMap = {
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
        const auto service = _ServiceMap.at(type);
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
    const std::string &type = data["type"];
    auto game = Game::Create(type, data["data"]);
    unsigned int id;
    {
        const std::scoped_lock lock(_MtxGameList);
        id = ++_GameCount;
        _GameList.try_emplace(id, std::move(game));
    }
    return {{"gameID", id}};
}

nlohmann::json Server::AddState(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/add_state.schema.json").validate(data);
    auto &game = GetGame(data["gameID"]);
    std::unique_ptr<State> state;
    if (data.contains("data"))
        state = State::Create(*game.GamePtr, data["data"]);
    else
        state = State::Create(*game.GamePtr);
    unsigned int id;
    StateRecord *recordPtr;
    {
        const std::scoped_lock lock(_MtxStateList);
        id = ++_StateCount;
        recordPtr = &_StateList.try_emplace(id, std::move(state), &game).first->second;
    }
    {
        const std::scoped_lock lock(game.MtxRecord);
        game.SubStates.push_back(recordPtr);
    }
    return {{"stateID", id}};
}

nlohmann::json Server::AddPlayer(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/add_player.schema.json").validate(data);
    const std::string &type = data["type"];
    auto &state = GetState(data["stateID"]);
    const auto &game = *state.ParentGame;
    auto player = Player::Create(type, *game.GamePtr, *state.StatePtr, data["data"]);
    unsigned int id;
    PlayerRecord *recordPtr;
    {
        const std::scoped_lock lock(_MtxPlayerList);
        id = ++_PlayerCount;
        recordPtr = &_PlayerList.try_emplace(id, std::move(player), &state).first->second;
    }
    // Data race may occur here (`state` being removed), but it is the client's responsibility to avoid that.
    {
        const std::scoped_lock lock(state.MtxRecord);
        state.SubPlayers.push_back(recordPtr);
    }
    return {{"playerID", id}};
}

nlohmann::json Server::AddActionGenerator(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/add_action_generator.schema.json").validate(data);
    const std::string &type = data["type"];
    auto &state = GetState(data["stateID"]);
    const auto &game = *state.ParentGame;
    auto actionGenerator = ActionGenerator::Create(type, *game.GamePtr, *state.StatePtr, data["data"]);
    auto actionGeneratorData = ActionGenerator::Data::Create(*actionGenerator);
    unsigned int id;
    ActionGeneratorRecord *recordPtr;
    {
        const std::scoped_lock lock(_MtxActionGeneratorList);
        id = ++_ActionGeneratorCount;
        recordPtr =
            &_ActionGeneratorList.try_emplace(id, std::move(actionGenerator), std::move(actionGeneratorData), &state)
                 .first->second;
    }
    // Data race may occur here (`state` being removed), but it is the client's responsibility to avoid that.
    {
        const std::scoped_lock lock(state.MtxRecord);
        state.SubActionGenerators.push_back(recordPtr);
    }
    return {{"actionGeneratorID", id}};
}

nlohmann::json Server::GenerateActions(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/generate_actions.schema.json").validate(data);
    const auto &record = GetActionGenerator(data["actionGeneratorID"]);
    const auto &generator = *record.ActionGeneratorPtr;
    const auto &generatorData = *record.ActionGeneratorDataPtr;
    nlohmann::json actions;
    generator.ForEach(generatorData, [&](const Action &action) { actions.push_back(action.GetJson()); });
    return {{"actions", actions}};
}

nlohmann::json Server::TakeAction(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/take_action.schema.json").validate(data);
    const auto &record = GetState(data["stateID"]);
    auto &mutex = record.MtxState;
    const auto &game = *record.ParentGame->GamePtr;
    auto &state = *record.StatePtr;
    auto action = Action::Create(game, data["action"]);
    if (!game.IsValidAction(state, *action))
        throw std::invalid_argument("The action is invalid");
    std::optional<std::vector<double>> result;
    {
        const std::scoped_lock lock(mutex);
        result = game.TakeAction(state, *action);
    }
    nlohmann::json response = {
        {"finished", static_cast<bool>(result)},
        {"state", state.GetJson()},
    };
    if (result)
        response["result"] = *result;
    return response;
}

nlohmann::json Server::StartThinking(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/start_thinking.schema.json").validate(data);
    const auto &record = GetPlayer(data["playerID"]);
    auto &mutex = record.MtxPlayer;
    auto &player = *record.PlayerPtr;
    {
        const std::scoped_lock lock(mutex);
        player.StartThinking();
    }
    return nlohmann::json::object();
}

nlohmann::json Server::StopThinking(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/stop_thinking.schema.json").validate(data);
    const auto &record = GetPlayer(data["playerID"]);
    auto &mutex = record.MtxPlayer;
    auto &player = *record.PlayerPtr;
    {
        const std::scoped_lock lock(mutex);
        player.StopThinking();
    }
    return nlohmann::json::object();
}

nlohmann::json Server::GetBestAction(const nlohmann::json &data) {
    Util::GetJsonValidator("requests/get_best_action.schema.json").validate(data);
    const auto &record = GetPlayer(data["playerID"]);
    auto &mutex = record.MtxPlayer;
    auto &player = *record.PlayerPtr;
    std::optional<std::chrono::duration<double>> time;
    if (data.contains("maxThinkTime"))
        time = std::chrono::duration<double>(data["maxThinkTime"]);
    std::unique_ptr<Action> bestAction;
    {
        const std::scoped_lock lock(mutex);
        bestAction = player.GetBestAction(time);
    }
    return {{"action", bestAction->GetJson()}};
}

nlohmann::json Server::QueryDetails(const nlohmann::json &) {
    // TODO
    return {};
}
