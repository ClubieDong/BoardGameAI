#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

class Game;
class State;
class Action;

class Player {
public:
    virtual ~Player() = default;
    virtual void StartThinking() {}
    virtual void StopThinking() {}
    virtual std::unique_ptr<Action> GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) = 0;
    virtual void Update(const Action &) {}

    static std::unique_ptr<Player> Create(const std::string &type, const Game &game, const State &state,
                                          const nlohmann::json &data);
};
