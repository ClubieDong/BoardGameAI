#pragma once

#include "../../Games/Game.hpp"
#include "../Player.hpp"

namespace random_move {
class Player : public ::Player {
private:
    const State *_StatePtr;

public:
    explicit Player(const Game &game, const State &state, const nlohmann::json &data);
};
} // namespace random_move
