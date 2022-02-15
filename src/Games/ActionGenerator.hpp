#pragma once

#include "../Utilities/Helpers.hpp"
#include <cassert>
#include <memory>
#include <nlohmann/json.hpp>
#include <string_view>

class Game;
class State;
class Action;

class ActionGenerator : public NonCopyableNonMoveable {
public:
    class Data : public ClonableEqualable<Data> {
    public:
        static std::unique_ptr<Data> Create(const ActionGenerator &actionGenerator);
    };

    virtual ~ActionGenerator() = default;
    virtual std::string_view GetType() const = 0;
    // This function should not return a null pointer,
    // because every valid state must have at least one action.
    virtual std::unique_ptr<Action> FirstAction(const Data &data) const = 0;
    virtual bool NextAction(const Data &data, Action &action) const = 0;
    virtual void Update(Data &, const Action &) const {}

    template <typename Func>
    void ForEach(const Data &data, Func func) const {
        auto action = FirstAction(data);
        assert(action);
        do
            func(*action);
        while (NextAction(data, *action));
    }

    static std::unique_ptr<ActionGenerator> Create(const std::string &type, const Game &game, const State &state,
                                                   const nlohmann::json &data);
};
