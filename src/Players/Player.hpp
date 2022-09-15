#pragma once

#include "../Games/ActionGenerator.hpp"
#include "../Utilities/Utilities.hpp"
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

class Game;
struct State;
struct Action;

class Player : public Util::NonCopyableNonMoveable {
protected:
    const Game *m_Game;
    const State *m_State;
    std::unique_ptr<ActionGenerator> m_ActionGenerator;
    std::unique_ptr<ActionGenerator::Data> m_ActionGeneratorData;

public:
    static std::unique_ptr<Player> Create(const std::string &type, const Game &game, const State &state,
                                          const nlohmann::json &data);

    explicit Player(const Game &game, const State &state, const nlohmann::json &data);
    virtual ~Player() = default;
    virtual std::string_view GetType() const = 0;

    virtual void StartThinking() {}
    virtual void StopThinking() {}
    virtual std::unique_ptr<Action> GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) = 0;
    virtual void Update(const Action &action) { m_ActionGenerator->Update(*m_ActionGeneratorData, action); }
    virtual nlohmann::json QueryDetails(const nlohmann::json &) { return nlohmann::json::object(); }
};
