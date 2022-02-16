#pragma once

#include "../Utilities/Helpers.hpp"
#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

class Game;
struct State;
struct Action;

class Player : public NonCopyableNonMoveable {
public:
    static std::unique_ptr<Player> Create(const std::string &type, const Game &game, const State &state,
                                          const nlohmann::json &data);

    virtual ~Player() = default;
    virtual std::string_view GetType() const = 0;

    virtual void StartThinking() {}
    virtual void StopThinking() {}
    virtual std::unique_ptr<Action> GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) = 0;
    virtual void Update(const Action &) {}
};
