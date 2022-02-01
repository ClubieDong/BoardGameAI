#pragma once

#include <memory>
#include <nlohmann/json.hpp>

class Game;

class State {
public:
    virtual ~State() = default;
    virtual nlohmann::json GetJson() const = 0;

    static std::unique_ptr<State> Create(const Game &game);
    static std::unique_ptr<State> Create(const Game &game, const nlohmann::json &data);
};

class Action {
public:
    virtual ~Action() = default;
    virtual nlohmann::json GetJson() const = 0;

    static std::unique_ptr<Action> Create(const Game &game, const nlohmann::json &data);
};

class Game {
public:
    virtual ~Game() = default;
    virtual bool IsValidAction(const State &state, const Action &action) const = 0;
    // TODO: Small vector optimization
    virtual std::optional<std::vector<double>> TakeAction(State &state, const Action &action) const = 0;

    static std::unique_ptr<Game> Create(const std::string &type, const nlohmann::json &data);
};
