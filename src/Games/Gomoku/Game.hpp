#pragma once

#include "../AbstractGames/MNKGame/Game.hpp"

namespace gomoku {
class Game : public m_n_k_game::Game<15, 15, 5> {
public:
    explicit Game(const nlohmann::json &) {}
    virtual std::string_view GetType() const override { return "gomoku"; }
};
} // namespace gomoku
