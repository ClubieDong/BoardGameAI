#pragma once

#include "../Utilities/Utilities.hpp"
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// TODO: Result: small vector optimization
// TODO: Comments
// TODO: explicit

class Game : public Util::NonCopyableNonMoveable {
public:
    struct State {
        virtual ~State() = default;
        virtual std::unique_ptr<State> Clone() const = 0;
        virtual bool Equal(const State &state) const = 0;
        virtual nlohmann::json GetJson() const = 0;
    };

    struct Action {
        virtual ~Action() = default;
        virtual std::unique_ptr<Action> Clone() const = 0;
        virtual bool Equal(const Action &action) const = 0;
        virtual nlohmann::json GetJson() const = 0;
    };

    static std::unique_ptr<Game> Create(const std::string &type, const nlohmann::json &data);

    virtual ~Game() = default;
    virtual std::string_view GetType() const = 0;

    virtual std::unique_ptr<State> CreateDefaultState() const = 0;
    virtual std::unique_ptr<State> CreateState(const nlohmann::json &data) const = 0;
    virtual std::unique_ptr<Action> CreateAction(const nlohmann::json &data) const = 0;

    virtual unsigned char GetNextPlayer(const State &state) const = 0;
    virtual bool IsValidAction(const State &state, const Action &action) const = 0;
    virtual std::optional<std::vector<float>> TakeAction(State &state, const Action &action) const = 0;
};
