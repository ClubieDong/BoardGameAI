#pragma once

#include "../../Games/ActionGenerator.hpp"
#include "../Player.hpp"

class Game;
struct State;
struct Action;

namespace random_move {
class Player : public ::Player {
private:
    const Game *m_Game;
    const State *m_State;
    std::unique_ptr<ActionGenerator> m_ActionGenerator;
    std::unique_ptr<ActionGenerator::Data> m_ActionGeneratorData;

public:
    explicit Player(const Game &game, const State &state, const nlohmann::json &data);

    virtual std::string_view GetType() const override { return "random_move"; }
    virtual std::unique_ptr<Action> GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) override;
};
} // namespace random_move
