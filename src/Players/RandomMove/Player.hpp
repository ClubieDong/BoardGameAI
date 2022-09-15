#pragma once

#include "../Player.hpp"

namespace random_move {
class Player : public ::Player {
public:
    explicit Player(const Game &game, const State &state, const nlohmann::json &data) : ::Player(game, state, data) {}

    virtual std::string_view GetType() const override { return "random_move"; }
    virtual std::unique_ptr<Action> GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) override;
};
} // namespace random_move
