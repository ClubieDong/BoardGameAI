#include "Server.hpp"
#include "../Utilities/Parallel.hpp"
#include "../Utilities/Utilities.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <tbb/parallel_for_each.h>
#include <thread>
#include <vector>

static const std::unordered_map<std::string, nlohmann::json (Server::*)(const nlohmann::json &)> ServiceMap = {
    {"echo", &Server::Echo},
    {"add_game", &Server::AddGame},
    {"add_state", &Server::AddState},
    {"add_player", &Server::AddPlayer},
    {"add_action_generator", &Server::AddActionGenerator},
    {"remove_game", &Server::RemoveGame},
    {"remove_state", &Server::RemoveState},
    {"remove_player", &Server::RemovePlayer},
    {"remove_action_generator", &Server::RemoveActionGenerator},
    {"generate_actions", &Server::GenerateActions},
    {"take_action", &Server::TakeAction},
    {"start_thinking", &Server::StartThinking},
    {"stop_thinking", &Server::StopThinking},
    {"get_best_action", &Server::GetBestAction},
    {"query_details", &Server::QueryDetails},
    {"run_games", &Server::RunGames},
};

void Server::Run() {
    std::string reqStr;
    while (true) {
        std::getline(m_InputStream, reqStr);
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
        const nlohmann::json &reqData = request["data"];
        Util::GetJsonValidator("requests/" + type + ".schema.json").validate(reqData);
        const auto service = ServiceMap.at(type);
        auto respData = (self->*service)(reqData);
        Util::GetJsonValidator("responses/" + type + ".schema.json").validate(respData);
        response["data"] = std::move(respData);
        response["success"] = true;
    } catch (const std::exception &e) {
        response["errMsg"] = e.what();
        response["success"] = false;
    }
    Util::GetJsonValidator("response.schema.json").validate(response);
    const std::scoped_lock lock(self->m_MtxOutputStream);
    self->m_OutputStream << response << std::endl;
}

nlohmann::json Server::Echo(const nlohmann::json &data) {
    const std::chrono::duration<double> time(data["sleepTime"]);
    std::this_thread::sleep_for(time);
    if (!data.contains("data"))
        return nlohmann::json::object();
    return {{"data", data["data"]}};
}

nlohmann::json Server::AddGame(const nlohmann::json &data) {
    auto game = Game::Create(data["type"], data["data"]);
    const auto id = m_GameMap.Emplace(std::move(game));
    return {{"gameID", id}};
}

nlohmann::json Server::AddState(const nlohmann::json &data) {
    nlohmann::json response;
    AccessGame(data, [&](GameRecord &gameRecord) {
        const auto &game = *gameRecord.GamePtr;
        auto state = data.contains("data") ? game.CreateState(data["data"]) : game.CreateDefaultState();
        response["state"] = game.GetJsonOfState(*state);
        response["nextPlayer"] = game.GetNextPlayer(*state);
        response["stateID"] = gameRecord.SubStates.Emplace(std::move(state));
    });
    return response;
}

nlohmann::json Server::AddPlayer(const nlohmann::json &data) {
    unsigned int id;
    AccessState(data, [&](const GameRecord &gameRecord, StateRecord &stateRecord) {
        const std::shared_lock lock(stateRecord.MtxState);
        auto player = Player::Create(data["type"], *gameRecord.GamePtr, *stateRecord.StatePtr, data["data"]);
        id = stateRecord.SubPlayers.Emplace(std::move(player));
    });
    return {{"playerID", id}};
}

nlohmann::json Server::AddActionGenerator(const nlohmann::json &data) {
    unsigned int id;
    AccessState(data, [&](const GameRecord &gameRecord, StateRecord &stateRecord) {
        auto actionGenerator = ActionGenerator::Create(data["type"], *gameRecord.GamePtr, data["data"]);
        const std::shared_lock lock(stateRecord.MtxState);
        auto actionGeneratorData = actionGenerator->CreateData(*stateRecord.StatePtr);
        id = stateRecord.SubActionGenerators.Emplace(std::move(actionGenerator), std::move(actionGeneratorData));
    });
    return {{"actionGeneratorID", id}};
}

nlohmann::json Server::RemoveGame(const nlohmann::json &data) {
    m_GameMap.Erase(data["gameID"]);
    return nlohmann::json::object();
}

nlohmann::json Server::RemoveState(const nlohmann::json &data) {
    AccessGame(data, [&](GameRecord &gameRecord) { gameRecord.SubStates.Erase(data["stateID"]); });
    return nlohmann::json::object();
}

nlohmann::json Server::RemovePlayer(const nlohmann::json &data) {
    AccessState(data,
                [&](const GameRecord &, StateRecord &stateRecord) { stateRecord.SubPlayers.Erase(data["playerID"]); });
    return nlohmann::json::object();
}

nlohmann::json Server::RemoveActionGenerator(const nlohmann::json &data) {
    AccessState(data, [&](const GameRecord &, StateRecord &stateRecord) {
        stateRecord.SubActionGenerators.Erase(data["actionGeneratorID"]);
    });
    return nlohmann::json::object();
}

nlohmann::json Server::GenerateActions(const nlohmann::json &data) {
    nlohmann::json actions;
    AccessActionGenerator(data, [&](const GameRecord &gameRecord, const StateRecord &stateRecord,
                                    const ActionGeneratorRecord &actionGeneratorRecord) {
        const auto &game = *gameRecord.GamePtr;
        const auto &actionGenerator = *actionGeneratorRecord.ActionGeneratorPtr;
        const auto &actionGeneratorData = *actionGeneratorRecord.ActionGeneratorDataPtr;
        // Always lock state before locking player or action generator
        const std::shared_lock lockState(stateRecord.MtxState);
        const std::shared_lock lockActionGenerator(actionGeneratorRecord.MtxActionGeneratorData);
        actionGenerator.ForEachAction(actionGeneratorData, *stateRecord.StatePtr,
                                      [&](const Action &action) { actions.push_back(game.GetJsonOfAction(action)); });
    });
    return {{"actions", actions}};
}

nlohmann::json Server::TakeAction(const nlohmann::json &data) {
    nlohmann::json response;
    AccessState(data, [&](const GameRecord &gameRecord, const StateRecord &stateRecord) {
        const auto &game = *gameRecord.GamePtr;
        auto &state = *stateRecord.StatePtr;
        const auto action = game.CreateAction(data["action"]);
        if (!game.IsValidAction(state, *action))
            throw std::invalid_argument("The action is invalid");
        const std::scoped_lock lock(stateRecord.MtxState);
        auto result = game.TakeAction(state, *action);
        // Build response
        response["finished"] = result.has_value();
        response["state"] = game.GetJsonOfState(state);
        if (result)
            response["result"] = std::move(*result);
        else
            response["nextPlayer"] = game.GetNextPlayer(state);
        // Concurrently update players and action generators
        auto futureUpdatePlayers = Parallel::Async([&]() {
            stateRecord.SubPlayers.ForEachParallel([&](const PlayerRecord &playerRecord) {
                const std::scoped_lock lock(playerRecord.MtxPlayer);
                playerRecord.PlayerPtr->Update(*action);
            });
        });
        auto futureUpdateActionGenerators = Parallel::Async([&]() {
            stateRecord.SubActionGenerators.ForEachParallel([&](const ActionGeneratorRecord &actionGeneratorRecord) {
                const std::scoped_lock lock(actionGeneratorRecord.MtxActionGeneratorData);
                actionGeneratorRecord.ActionGeneratorPtr->Update(*actionGeneratorRecord.ActionGeneratorDataPtr,
                                                                 *action);
            });
        });
        futureUpdatePlayers.get();
        futureUpdateActionGenerators.get();
    });
    return response;
}

nlohmann::json Server::StartThinking(const nlohmann::json &data) {
    AccessPlayer(data, [&](const GameRecord &, const StateRecord &stateRecord, const PlayerRecord &playerRecord) {
        // Always lock state before locking player or action generator
        const std::shared_lock lockState(stateRecord.MtxState);
        const std::scoped_lock lockPlayer(playerRecord.MtxPlayer);
        playerRecord.PlayerPtr->StartThinking();
    });
    return nlohmann::json::object();
}

nlohmann::json Server::StopThinking(const nlohmann::json &data) {
    AccessPlayer(data, [&](const GameRecord &, const StateRecord &stateRecord, const PlayerRecord &playerRecord) {
        // Always lock state before locking player or action generator
        const std::shared_lock lockState(stateRecord.MtxState);
        const std::scoped_lock lockPlayer(playerRecord.MtxPlayer);
        playerRecord.PlayerPtr->StopThinking();
    });
    return nlohmann::json::object();
}

nlohmann::json Server::GetBestAction(const nlohmann::json &data) {
    std::optional<std::chrono::duration<double>> time;
    if (data.contains("maxThinkTime"))
        time = std::chrono::duration<double>(data["maxThinkTime"]);
    nlohmann::json bestActionJson;
    AccessPlayer(data,
                 [&](const GameRecord &gameRecord, const StateRecord &stateRecord, const PlayerRecord &playerRecord) {
                     std::unique_ptr<Action> bestAction;
                     {
                         // Always lock state before locking player or action generator
                         const std::shared_lock lockState(stateRecord.MtxState);
                         const std::scoped_lock lock(playerRecord.MtxPlayer);
                         bestAction = playerRecord.PlayerPtr->GetBestAction(time);
                     }
                     const auto &game = *gameRecord.GamePtr;
                     bestActionJson = game.GetJsonOfAction(*bestAction);
                 });
    return {{"action", std::move(bestActionJson)}};
}

nlohmann::json Server::QueryDetails(const nlohmann::json &) {
    // TODO
    return {};
}

nlohmann::json Server::RunGames(const nlohmann::json &data) {
    const unsigned int rounds = data["rounds"];
    const auto playerCount = data["players"].size();
    const auto game = Game::Create(data["game"]["type"], data["game"]["data"]);
    const auto runGame = [&](std::vector<float> &result) {
        const auto state = game->CreateDefaultState();
        // player, maxThinkTime, allowBackgroundThinking
        std::vector<std::tuple<std::unique_ptr<Player>, std::optional<std::chrono::duration<double>>, bool>> players;
        // Create players
        for (const auto &playerData : data["players"]) {
            auto player = Player::Create(playerData["type"], *game, *state, playerData["data"]);
            std::optional<std::chrono::duration<double>> maxThinkTime;
            if (data.contains("maxThinkTime"))
                maxThinkTime = std::chrono::duration<double>(playerData["maxThinkTime"]);
            const bool allowBackgroundThinking = playerData["allowBackgroundThinking"];
            players.emplace_back(std::move(player), maxThinkTime, allowBackgroundThinking);
        }
        // Start thinking
        for (const auto &[player, maxThinkTime, allowBackgroundThinking] : players)
            if (allowBackgroundThinking)
                player->StartThinking();
        // Play the game
        while (true) {
            const auto &[player, maxThinkTime, allowBackgroundThinking] = players[game->GetNextPlayer(*state)];
            if (!allowBackgroundThinking)
                player->StartThinking();
            const auto action = player->GetBestAction(maxThinkTime);
            if (!allowBackgroundThinking)
                player->StopThinking();
            auto actionResult = game->TakeAction(*state, *action);
            if (actionResult) {
                result = std::move(*actionResult);
                break;
            }
            for (const auto &[player, maxThinkTime, allowBackgroundThinking] : players)
                player->Update(*action);
        }
        // Stop thinking
        for (const auto &[player, maxThinkTime, allowBackgroundThinking] : players)
            if (allowBackgroundThinking)
                player->StopThinking();
    };
    std::vector<std::vector<float>> results(rounds);
    if (data["parallel"])
        tbb::parallel_for_each(results, runGame);
    else
        std::for_each(results.begin(), results.end(), runGame);
    std::vector<float> finalResult(playerCount, 0.0f);
    for (const auto &result : results) {
        assert(result.size() == playerCount);
        for (unsigned int idx = 0; idx < playerCount; ++idx)
            finalResult[idx] += result[idx];
    }
    return {{"results", results}, {"finalResult", finalResult}};
}
