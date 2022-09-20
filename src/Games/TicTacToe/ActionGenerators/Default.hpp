#pragma once

#include "../../AbstractGames/MNKGame/ActionGenerators/Default.hpp"

namespace tic_tac_toe::action_generator {
class Default : public m_n_k_game::action_generator::Default<3, 3, 3> {
public:
    explicit Default(const Game &game, const nlohmann::json &) : m_n_k_game::action_generator::Default<3, 3, 3>(game) {}

    virtual std::string_view GetType() const override { return "tic_tac_toe/default"; }
};
} // namespace tic_tac_toe::action_generator
