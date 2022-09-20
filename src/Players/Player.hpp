#pragma once

#include "../Games/ActionGenerator.hpp"
#include "../Games/Game.hpp"
#include "../Utilities/Utilities.hpp"
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

class Player : public Util::NonCopyableNonMoveable {
protected:
    const Game *m_Game;
    const Game::State *m_State;
    std::unique_ptr<ActionGenerator> m_ActionGenerator;
    std::unique_ptr<ActionGenerator::Data> m_ActionGeneratorData;

public:
    static std::unique_ptr<Player> Create(const std::string &type, const Game &game, const Game::State &state,
                                          const nlohmann::json &data);

    explicit Player(const Game &game, const Game::State &state, const nlohmann::json &data);
    virtual ~Player() = default;
    virtual std::string_view GetType() const = 0;

    virtual void StartThinking() {}
    virtual void StopThinking() {}
    virtual std::unique_ptr<Game::Action> GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) = 0;
    virtual void Update(const Game::Action &action) {
        m_ActionGenerator->UpdateData(*m_ActionGeneratorData, *m_State, action);
    }
    virtual nlohmann::json QueryDetails(const nlohmann::json &) { return nlohmann::json::object(); }
};
