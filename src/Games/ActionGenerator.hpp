#pragma once

#include <memory>
#include <cassert>
#include <nlohmann/json.hpp>

class Game;
class State;
class Action;

class ActionGenerator
{
public:
    class Data
    {
    public:
        virtual ~Data() = default;

        static std::unique_ptr<Data> Create(const ActionGenerator &actionGenerator);
    };

    virtual ~ActionGenerator() = default;
    // This function should not return a null pointer,
    // because every valid state must have at least one action.
    virtual std::unique_ptr<Action> FirstAction(const Data &data) const = 0;
    virtual bool NextAction(const Data &data, Action &action) const = 0;

    template <typename Func>
    void ForEach(const Data &data, Func func) const
    {
        auto action = FirstAction(data);
        assert(action);
        do
            func(*action);
        while (NextAction(data, *action));
    }

    static std::unique_ptr<ActionGenerator> Create(const std::string &type, const Game &game, const State &state, const nlohmann::json &data);
};
