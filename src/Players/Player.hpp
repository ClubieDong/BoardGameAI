#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

class Game;
class State;

class Player {
public:
    virtual ~Player() = default;

    static std::unique_ptr<Player> Create(const std::string &type, const Game &game, const State &state,
                                          const nlohmann::json &data);
};
