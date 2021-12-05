#include "Server.hpp"
#include <iostream>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>
#include <optional>
#include "../Utilities/Utilities.hpp"

const std::unordered_map<std::string, nlohmann::json (Server::*)(const nlohmann::json &)> Server::_ServiceMap = {
    {"echo", &Server::Echo},
    {"add_game", &Server::AddGame},
    {"add_state", &Server::AddState},
    {"add_action_generator", &Server::AddActionGenerator},
    {"generate_actions", &Server::GenerateActions},
    {"take_action", &Server::TakeAction},
};

void Server::Run()
{
    std::string reqStr;
    while (true)
    {
        std::getline(std::cin, reqStr);
        std::thread(Serve, this, std::move(reqStr)).detach();
    }
}

void Server::Serve(Server *self, std::string &&reqStr)
{
    nlohmann::json response;
    try
    {
        const auto request = nlohmann::json::parse(reqStr);
        if (request.contains("id"))
            response["id"] = request["id"];
        Util::GetJsonValidator("request.schema.json").validate(request);
        const std::string &type = request["type"];
        const nlohmann::json &data = request["data"];
        const auto service = _ServiceMap.at(type);
        response["data"] = (self->*service)(data);
        response["success"] = true;
    }
    catch (const std::exception &e)
    {
        response["errMsg"] = e.what();
        response["success"] = false;
    }
    const std::scoped_lock lock(self->_MtxCout);
    std::cout << response << std::endl;
}

nlohmann::json Server::Echo(const nlohmann::json &data)
{
    Util::GetJsonValidator("requests/echo.schema.json").validate(data);
    const auto time = std::chrono::seconds(data["time"]);
    std::this_thread::sleep_for(time);
    if (!data.contains("data"))
        return nlohmann::json::object();
    return {{"data", data["data"]}};
}

nlohmann::json Server::AddGame(const nlohmann::json &data)
{
    Util::GetJsonValidator("requests/add_game.schema.json").validate(data);
    auto game = Game::Create(data);
    unsigned int id;
    {
        const std::scoped_lock lock(_MtxGameList);
        id = ++_GameCount;
        _GameList.try_emplace(id, std::move(game));
    }
    return {{"gameID", id}};
}

nlohmann::json Server::AddState(const nlohmann::json &data)
{
    Util::GetJsonValidator("requests/add_state.schema.json").validate(data);
    auto &game = GetGame(data["gameID"]);
    std::unique_ptr<State> state;
    if (data["data"] == nullptr)
        state = State::Create(*game.GamePtr);
    else
        state = State::Create(*game.GamePtr, data["data"]);
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

nlohmann::json Server::AddActionGenerator(const nlohmann::json &data)
{
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
        recordPtr = &_ActionGeneratorList.try_emplace(id, std::move(actionGenerator),
                                                      std::move(actionGeneratorData), &state)
                         .first->second;
    }
    {
        const std::scoped_lock lock(state.MtxRecord);
        state.SubActionGenerators.push_back(recordPtr);
    }
    return {{"actionGeneratorID", id}};
}

nlohmann::json Server::GenerateActions(const nlohmann::json &data)
{
    Util::GetJsonValidator("requests/generate_actions.schema.json").validate(data);
    const auto &record = GetActionGenerator(data["actionGeneratorID"]);
    const auto &generator = *record.ActionGeneratorPtr;
    const auto &generatorData = *record.ActionGeneratorDataPtr;
    nlohmann::json actions;
    generator.ForEach(generatorData, [&](const Action &action)
                      { actions.push_back(action.GetJson()); });
    return {{"actions", actions}};
}

nlohmann::json Server::TakeAction(const nlohmann::json &data)
{
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
