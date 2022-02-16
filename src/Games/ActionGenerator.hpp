#pragma once

#include "../Utilities/Helpers.hpp"
#include <cassert>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

class Game;
struct State;
struct Action;

class ActionGenerator : public NonCopyableNonMoveable {
public:
    struct Data {};

    template <typename DerivedData>
    class CRTP;

    static std::unique_ptr<ActionGenerator> Create(const std::string &type, const Game &game, const State &state,
                                                   const nlohmann::json &data);

    virtual ~ActionGenerator() = default;
    virtual std::string_view GetType() const = 0;

    virtual std::unique_ptr<Data> CreateData() const = 0;
    virtual std::unique_ptr<Data> CloneData(const Data &data) const = 0;
    virtual bool EqualData(const Data &left, const Data &right) const = 0;

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
};

template <typename DerivedData>
class ActionGenerator::CRTP : public ActionGenerator {
public:
    virtual std::unique_ptr<Data> CreateData() const override { return std::make_unique<DerivedData>(); }
    virtual std::unique_ptr<Data> CloneData(const Data &data) const override {
        return std::make_unique<DerivedData>(static_cast<const DerivedData &>(data));
    }
    virtual bool EqualData(const Data &left, const Data &right) const override {
        return static_cast<const DerivedData &>(left) == static_cast<const DerivedData &>(right);
    }
};
