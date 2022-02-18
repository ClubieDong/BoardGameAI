#pragma once

#include "../Utilities/Helpers.hpp"
#include "../Utilities/Utilities.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct State {};
struct Action {};

class Game : public NonCopyableNonMoveable {
public:
    template <typename DerivedState, typename DerivedAction>
    class CRTP;

    static std::unique_ptr<Game> Create(const std::string &type, const nlohmann::json &data);

    virtual ~Game() = default;
    virtual std::string_view GetType() const = 0;

    virtual std::unique_ptr<State> CreateDefaultState() const = 0;
    virtual std::unique_ptr<State> CreateState(const nlohmann::json &data) const = 0;
    virtual std::unique_ptr<Action> CreateAction(const nlohmann::json &data) const = 0;
    virtual std::unique_ptr<State> CloneState(const State &state) const = 0;
    virtual std::unique_ptr<Action> CloneAction(const Action &action) const = 0;
    virtual bool EqualState(const State &left, const State &right) const = 0;
    virtual bool EqualAction(const Action &left, const Action &right) const = 0;
    virtual nlohmann::json GetJsonOfState(const State &state) const = 0;
    virtual nlohmann::json GetJsonOfAction(const Action &action) const = 0;

    virtual unsigned char GetNextPlayer(const State &state) const = 0;
    virtual bool IsValidAction(const State &state, const Action &action) const = 0;
    // TODO: Small vector optimization
    virtual std::optional<std::vector<double>> TakeAction(State &state, const Action &action) const = 0;
};

template <typename DerivedState, typename DerivedAction>
class Game::CRTP : public Game {
public:
    virtual std::unique_ptr<State> CreateDefaultState() const override { return std::make_unique<DerivedState>(); }
    virtual std::unique_ptr<State> CreateState(const nlohmann::json &data) const override {
        Util::GetJsonValidator("states/" + std::string(GetType()) + ".schema.json").validate(data);
        return std::make_unique<DerivedState>(data);
    }
    virtual std::unique_ptr<Action> CreateAction(const nlohmann::json &data) const override {
        Util::GetJsonValidator("actions/" + std::string(GetType()) + ".schema.json").validate(data);
        return std::make_unique<DerivedAction>(data);
    }
    virtual std::unique_ptr<State> CloneState(const State &state) const override {
        return std::make_unique<DerivedState>(static_cast<const DerivedState &>(state));
    }
    virtual std::unique_ptr<Action> CloneAction(const Action &action) const override {
        return std::make_unique<DerivedAction>(static_cast<const DerivedAction &>(action));
    }
    virtual bool EqualState(const State &left, const State &right) const override {
        return static_cast<const DerivedState &>(left) == static_cast<const DerivedState &>(right);
    }
    virtual bool EqualAction(const Action &left, const Action &right) const override {
        return static_cast<const DerivedAction &>(left) == static_cast<const DerivedAction &>(right);
    }
};
