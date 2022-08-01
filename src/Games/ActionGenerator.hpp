#pragma once

#include "../Utilities/Utilities.hpp"
#include <cassert>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

class Game;
struct State;
struct Action;

class ActionGenerator : public Util::NonCopyableNonMoveable {
public:
    struct Data {
        virtual ~Data() = default;
    };

    template <typename DerivedData>
    class CRTP;

    static std::unique_ptr<ActionGenerator> Create(const std::string &type, const Game &game,
                                                   const nlohmann::json &data);

    virtual ~ActionGenerator() = default;
    virtual std::string_view GetType() const = 0;

    virtual std::unique_ptr<Data> CreateData(const State &state) const = 0;
    virtual std::unique_ptr<Data> CloneData(const Data &data) const = 0;
    virtual bool EqualData(const Data &left, const Data &right) const = 0;

    // This function should not return a null pointer,
    // because every valid state must have at least one action.
    virtual std::unique_ptr<Action> FirstAction(const Data &data, const State &state) const = 0;
    virtual bool NextAction(const Data &data, const State &state, Action &action) const = 0;
    virtual void Update(Data &, const Action &) const {}

    template <typename Func>
    void ForEachAction(const Data &data, const State &state, Func func) const {
        auto action = FirstAction(data, state);
        assert(action);
        do
            func(*action);
        while (NextAction(data, state, *action));
    }

    std::unique_ptr<Action> GetNthAction(const Data &data, const State &state, unsigned int idx) const;
};

template <typename DerivedData>
class ActionGenerator::CRTP : public ActionGenerator {
public:
    virtual std::unique_ptr<Data> CreateData(const State &state) const override {
        return std::make_unique<DerivedData>(state);
    }
    virtual std::unique_ptr<Data> CloneData(const Data &data) const override {
        return std::make_unique<DerivedData>(static_cast<const DerivedData &>(data));
    }
    virtual bool EqualData(const Data &left, const Data &right) const override {
        return static_cast<const DerivedData &>(left) == static_cast<const DerivedData &>(right);
    }
};
