#pragma once

#include "../../AbstractGames/MNKGame/ActionGenerators/Neighbor.hpp"

namespace gomoku::action_generator {
class Neighbor : public m_n_k_game::action_generator::Neighbor<15, 15, 5> {
public:
    explicit Neighbor(const Game &game, const nlohmann::json &data)
        : m_n_k_game::action_generator::Neighbor<15, 15, 5>(game, data["range"]) {}

    virtual std::string_view GetType() const override { return "gomoku/default"; }
};
} // namespace gomoku::action_generator
