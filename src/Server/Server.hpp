#pragma once

#include "../Games/ActionGenerator.hpp"
#include "../Games/Game.hpp"
#include "../Players/Player.hpp"
#include <atomic>
#include <istream>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_vector.h>
#include <unordered_map>

class Server {
private:
    struct StateRecord;
    struct PlayerRecord;
    struct ActionGeneratorRecord;

    struct GameRecord {
        const std::unique_ptr<Game> GamePtr;
        tbb::concurrent_vector<StateRecord *> SubStates;

        explicit GameRecord(std::unique_ptr<Game> &&gamePtr) : GamePtr(std::move(gamePtr)) {}
    };

    struct StateRecord {
        // Used to lock the `State` object
        mutable std::mutex MtxState;
        const std::unique_ptr<State> StatePtr;
        const GameRecord *ParentGame;
        tbb::concurrent_vector<PlayerRecord *> SubPlayers;
        tbb::concurrent_vector<ActionGeneratorRecord *> SubActionGenerators;

        explicit StateRecord(std::unique_ptr<State> &&statePtr, const GameRecord *parentGame)
            : StatePtr(std::move(statePtr)), ParentGame(parentGame) {}
    };

    struct PlayerRecord {
        // Used to lock the `Player` object
        mutable std::mutex MtxPlayer;
        const std::unique_ptr<Player> PlayerPtr;
        const StateRecord *ParentState;

        explicit PlayerRecord(std::unique_ptr<Player> &&playerPtr, const StateRecord *parentState)
            : PlayerPtr(std::move(playerPtr)), ParentState(parentState) {}
    };

    struct ActionGeneratorRecord {
        // Used to lock the `ActionGenerator` object
        mutable std::mutex MtxActionGenerator;
        const std::unique_ptr<ActionGenerator> ActionGeneratorPtr;
        const std::unique_ptr<ActionGenerator::Data> ActionGeneratorDataPtr;
        const StateRecord *ParentState;

        explicit ActionGeneratorRecord(std::unique_ptr<ActionGenerator> &&actionGeneratorPtr,
                                       std::unique_ptr<ActionGenerator::Data> &&actionGeneratorDataPtr,
                                       const StateRecord *parentState)
            : ActionGeneratorPtr(std::move(actionGeneratorPtr)),
              ActionGeneratorDataPtr(std::move(actionGeneratorDataPtr)), ParentState(parentState) {}
    };

    // Used to lock the output stream
    mutable std::mutex m_MtxOutputStream;
    std::istream &m_InputStream;
    std::ostream &m_OutputStream;

    std::atomic<unsigned int> m_GameCount = 0, m_StateCount = 0, m_PlayerCount = 0, m_ActionGeneratorCount = 0;
    tbb::concurrent_unordered_map<unsigned int, GameRecord> m_GameList;
    tbb::concurrent_unordered_map<unsigned int, StateRecord> m_StateList;
    tbb::concurrent_unordered_map<unsigned int, PlayerRecord> m_PlayerList;
    tbb::concurrent_unordered_map<unsigned int, ActionGeneratorRecord> m_ActionGeneratorList;

    static void Serve(Server *self, std::string &&reqStr);

public:
    explicit Server(std::istream &is, std::ostream &os) : m_InputStream(is), m_OutputStream(os) {}
    void Run();

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
