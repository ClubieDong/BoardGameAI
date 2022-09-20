#pragma once

#include "../AbstractGames/MNKGame/Game.hpp"

namespace tic_tac_toe {
class Game : public m_n_k_game::Game<3, 3, 3> {
public:
    explicit Game(const nlohmann::json &) {}
    virtual std::string_view GetType() const override { return "tic_tac_toe"; }
};
} // namespace tic_tac_toe
