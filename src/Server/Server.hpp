#pragma once

#include "../Games/ActionGenerator.hpp"
#include "../Games/Game.hpp"
#include "../Players/Player.hpp"
#include <atomic>
#include <istream>
#include <memory>
#include <mutex>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <ostream>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

class Server {
private:
    struct StateRecord;
    struct PlayerRecord;
    struct ActionGeneratorRecord;

    struct GameRecord {
        // Used to lock this record object
        mutable std::mutex MtxRecord; // TODO: shared_mutex?
        // Used to lock the `Game` object
        mutable std::mutex MtxGame; // TODO: shared_mutex? TODO: DO I Need this?
        const std::unique_ptr<Game> GamePtr;
        std::vector<StateRecord *> SubStates;

        explicit GameRecord(std::unique_ptr<Game> &&gamePtr) : GamePtr(std::move(gamePtr)) {}
    };

    struct StateRecord {
        // Used to lock this record object
        mutable std::mutex MtxRecord; // TODO: shared_mutex?
        // Used to lock the `State` object
        mutable std::mutex MtxState; // TODO: shared_mutex? TODO: Do I Need this?
        const std::unique_ptr<State> StatePtr;
        const GameRecord *ParentGame;
        std::vector<PlayerRecord *> SubPlayers;
        std::vector<ActionGeneratorRecord *> SubActionGenerators;

        explicit StateRecord(std::unique_ptr<State> &&statePtr, GameRecord *parentGame)
            : StatePtr(std::move(statePtr)), ParentGame(parentGame) {}
    };

    struct PlayerRecord {
        // Used to lock the `Player` object;
        mutable std::mutex MtxPlayer; // TODO: shared_mutex? TODO: Do I Need this?
        const std::unique_ptr<Player> PlayerPtr;
        const StateRecord *ParentState;

        explicit PlayerRecord(std::unique_ptr<Player> &&playerPtr, StateRecord *parentState)
            : PlayerPtr(std::move(playerPtr)), ParentState(parentState) {}
    };

    struct ActionGeneratorRecord {
        // Used to lock the `ActionGenerator` object
        mutable std::mutex MtxActionGenerator; // TODO: shared_mutex? TODO: Do I Need this?
        const std::unique_ptr<ActionGenerator> ActionGeneratorPtr;
        const std::unique_ptr<ActionGenerator::Data> ActionGeneratorDataPtr;
        const StateRecord *ParentState;

        explicit ActionGeneratorRecord(std::unique_ptr<ActionGenerator> &&actionGeneratorPtr,
                                       std::unique_ptr<ActionGenerator::Data> &&actionGeneratorDataPtr,
                                       StateRecord *parentState)
            : ActionGeneratorPtr(std::move(actionGeneratorPtr)),
              ActionGeneratorDataPtr(std::move(actionGeneratorDataPtr)), ParentState(parentState) {}
    };

    std::istream &_InputStream;
    std::ostream &_OutputStream;
    mutable std::mutex _MtxOutputStream;
    mutable std::shared_mutex _MtxGameList, _MtxStateList, _MtxPlayerList, _MtxActionGeneratorList;
    // Need to be guarded by the corresponding mutex
    unsigned int _GameCount = 0, _StateCount = 0, _PlayerCount = 0, _ActionGeneratorCount = 0;
    std::unordered_map<unsigned int, GameRecord> _GameList;
    std::unordered_map<unsigned int, StateRecord> _StateList;
    std::unordered_map<unsigned int, PlayerRecord> _PlayerList;
    std::unordered_map<unsigned int, ActionGeneratorRecord> _ActionGeneratorList;

    GameRecord &GetGame(unsigned int id) {
        const std::shared_lock lock(_MtxGameList);
        return _GameList.at(id);
    }
    StateRecord &GetState(unsigned int id) {
        const std::shared_lock lock(_MtxStateList);
        return _StateList.at(id);
    }
    PlayerRecord &GetPlayer(unsigned int id) {
        const std::shared_lock lock(_MtxPlayerList);
        return _PlayerList.at(id);
    }
    ActionGeneratorRecord &GetActionGenerator(unsigned int id) {
        const std::shared_lock lock(_MtxActionGeneratorList);
        return _ActionGeneratorList.at(id);
    }

public:
    explicit Server(std::istream &is, std::ostream &os) : _InputStream(is), _OutputStream(os) {}
    void Run();

private:
    static const std::unordered_map<std::string, nlohmann::json (Server::*)(const nlohmann::json &)> _ServiceMap;
    static void Serve(Server *self, std::string &&reqStr);

    nlohmann::json Echo(const nlohmann::json &data);
    nlohmann::json AddGame(const nlohmann::json &data);
    nlohmann::json AddState(const nlohmann::json &data);
    nlohmann::json AddPlayer(const nlohmann::json &data);
    nlohmann::json AddActionGenerator(const nlohmann::json &data);
    nlohmann::json GenerateActions(const nlohmann::json &data);
    nlohmann::json TakeAction(const nlohmann::json &data);
    nlohmann::json StartThinking(const nlohmann::json &data);
    nlohmann::json StopThinking(const nlohmann::json &data);
    nlohmann::json GetBestAction(const nlohmann::json &data);
    nlohmann::json QueryDetails(const nlohmann::json &data);
};
