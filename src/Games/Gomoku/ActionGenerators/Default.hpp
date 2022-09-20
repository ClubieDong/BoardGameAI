#pragma once

#include "../../AbstractGames/MNKGame/ActionGenerators/Default.hpp"

namespace gomoku::action_generator {
class Default : public m_n_k_game::action_generator::Default<15, 15, 5> {
public:
    explicit Default(const Game &game, const nlohmann::json &)
        : m_n_k_game::action_generator::Default<15, 15, 5>(game) {}

    virtual std::string_view GetType() const override { return "gomoku/default"; }
};
} // namespace gomoku::action_generator
