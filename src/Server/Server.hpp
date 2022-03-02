#pragma once

#include "../Games/ActionGenerator.hpp"
#include "../Games/Game.hpp"
#include "../Players/Player.hpp"
#include "../Utilities/ConcurrentIDMap.hpp"
#include <atomic>
#include <istream>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <ostream>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Server {
private:
    struct StateRecord;
    struct PlayerRecord;
    struct ActionGeneratorRecord;

    struct GameRecord {
        const std::unique_ptr<const Game> GamePtr;
        ConcurrentIDMap<StateRecord> SubStates;

        explicit GameRecord(std::unique_ptr<const Game> &&gamePtr) : GamePtr(std::move(gamePtr)) {}
    };

    struct StateRecord {
        // Used to lock the `State` object
        mutable std::shared_mutex MtxState;
        const std::unique_ptr<State> StatePtr;
        ConcurrentIDMap<PlayerRecord> SubPlayers;
        ConcurrentIDMap<ActionGeneratorRecord> SubActionGenerators;

        explicit StateRecord(std::unique_ptr<State> &&statePtr) : StatePtr(std::move(statePtr)) {}
    };

    struct PlayerRecord {
        // Used to lock the `Player` object
        mutable std::shared_mutex MtxPlayer;
        const std::unique_ptr<Player> PlayerPtr;

        explicit PlayerRecord(std::unique_ptr<Player> &&playerPtr) : PlayerPtr(std::move(playerPtr)) {}
    };

    struct ActionGeneratorRecord {
        // Used to lock the `ActionGenerator::Data` object
        mutable std::shared_mutex MtxActionGeneratorData;
        const std::unique_ptr<const ActionGenerator> ActionGeneratorPtr;
        const std::unique_ptr<ActionGenerator::Data> ActionGeneratorDataPtr;

        explicit ActionGeneratorRecord(std::unique_ptr<const ActionGenerator> &&actionGeneratorPtr,
                                       std::unique_ptr<ActionGenerator::Data> &&actionGeneratorDataPtr)
            : ActionGeneratorPtr(std::move(actionGeneratorPtr)),
              ActionGeneratorDataPtr(std::move(actionGeneratorDataPtr)) {}
    };

    // Used to lock the output stream
    mutable std::mutex m_MtxOutputStream;
    std::istream &m_InputStream;
    std::ostream &m_OutputStream;

    ConcurrentIDMap<GameRecord> m_GameMap;

    static void Serve(Server *self, std::string &&reqStr);

    template <typename Func>
    void AccessGame(const nlohmann::json &data, Func func) {
        m_GameMap.Access(data["gameID"], func);
    }

    template <typename Func>
    void AccessState(const nlohmann::json &data, Func func) {
        m_GameMap.Access(data["gameID"], [&](GameRecord &gameRecord) {
            gameRecord.SubStates.Access(data["stateID"],
                                        [&](StateRecord &stateRecord) { func(gameRecord, stateRecord); });
        });
    }

    template <typename Func>
    void AccessPlayer(const nlohmann::json &data, Func func) {
        m_GameMap.Access(data["gameID"], [&](GameRecord &gameRecord) {
            gameRecord.SubStates.Access(data["stateID"], [&](StateRecord &stateRecord) {
                stateRecord.SubPlayers.Access(
                    data["playerID"], [&](PlayerRecord &playerRecord) { func(gameRecord, stateRecord, playerRecord); });
            });
        });
    }

    template <typename Func>
    void AccessActionGenerator(const nlohmann::json &data, Func func) {
        m_GameMap.Access(data["gameID"], [&](GameRecord &gameRecord) {
            gameRecord.SubStates.Access(data["stateID"], [&](StateRecord &stateRecord) {
                stateRecord.SubActionGenerators.Access(data["actionGeneratorID"],
                                                       [&](ActionGeneratorRecord &actionGeneratorRecord) {
                                                           func(gameRecord, stateRecord, actionGeneratorRecord);
                                                       });
            });
        });
    }

public:
    explicit Server(std::istream &is, std::ostream &os) : m_InputStream(is), m_OutputStream(os) {}
    void Run();

    nlohmann::json Echo(const nlohmann::json &data);
    nlohmann::json AddGame(const nlohmann::json &data);
    nlohmann::json AddState(const nlohmann::json &data);
    nlohmann::json AddPlayer(const nlohmann::json &data);
    nlohmann::json AddActionGenerator(const nlohmann::json &data);
    nlohmann::json RemoveGame(const nlohmann::json &data);
    nlohmann::json RemoveState(const nlohmann::json &data);
    nlohmann::json RemovePlayer(const nlohmann::json &data);
    nlohmann::json RemoveActionGenerator(const nlohmann::json &data);
    nlohmann::json GenerateActions(const nlohmann::json &data);
    nlohmann::json TakeAction(const nlohmann::json &data);
    nlohmann::json StartThinking(const nlohmann::json &data);
    nlohmann::json StopThinking(const nlohmann::json &data);
    nlohmann::json GetBestAction(const nlohmann::json &data);
    nlohmann::json QueryDetails(const nlohmann::json &data);
    nlohmann::json RunGames(const nlohmann::json &data);
};
