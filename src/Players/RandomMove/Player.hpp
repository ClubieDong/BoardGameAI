#pragma once

#include "../Player.hpp"

namespace random_move {
class Player : public ::Player {
public:
    explicit Player(const Game &game, const Game::State &state, const nlohmann::json &data)
        : ::Player(game, state, data) {}
    virtual std::string_view GetType() const override { return "random_move"; }

    virtual std::unique_ptr<Game::Action> GetBestAction(std::optional<std::chrono::duration<double>>) override {
        return m_ActionGenerator->GetRandomAction(*m_ActionGeneratorData, *m_State);
    }
};
} // namespace random_move
