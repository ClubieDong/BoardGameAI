#include "Game.hpp"
#include "../Utilities/Utilities.hpp"
#include "TicTacToe/Game.hpp"
#include <string>
#include <unordered_map>

template <typename T>
static std::unique_ptr<State> CreateDefaultState(const Game &game) {
    return std::make_unique<T>(game);
}

template <typename T>
static std::unique_ptr<State> CreateState(const nlohmann::json &data) {
    return std::make_unique<T>(data);
}

template <typename T>
static std::unique_ptr<Action> CreateAction(const nlohmann::json &data) {
    return std::make_unique<T>(data);
}

template <typename T>
static std::unique_ptr<Game> CreateGame(const nlohmann::json &data) {
    return std::make_unique<T>(data);
}

using DefaultStateCreatorFunc = std::unique_ptr<State> (*)(const Game &);
using StateCreatorFunc = std::unique_ptr<State> (*)(const nlohmann::json &);
static const std::unordered_map<std::string_view, std::pair<DefaultStateCreatorFunc, StateCreatorFunc>>
    StateCreatorMap = {
        {"tic_tac_toe", {CreateDefaultState<tic_tac_toe::State>, CreateState<tic_tac_toe::State>}},
};

using ActionCreatorFunc = std::unique_ptr<Action> (*)(const nlohmann::json &);
static const std::unordered_map<std::string_view, ActionCreatorFunc> ActionCreatorMap = {
    {"tic_tac_toe", CreateAction<tic_tac_toe::Action>},
};

using GameCreatorFunc = std::unique_ptr<Game> (*)(const nlohmann::json &);
static const std::unordered_map<std::string, GameCreatorFunc> GameCreatorMap = {
    {"tic_tac_toe", CreateGame<tic_tac_toe::Game>},
};

std::unique_ptr<State> State::Create(const Game &game) {
    const auto &type = game.GetType();
    const auto creator = StateCreatorMap.at(type).first;
    return creator(game);
}

std::unique_ptr<State> State::Create(const Game &game, const nlohmann::json &data) {
    const auto &type = game.GetType();
    Util::GetJsonValidator("states/" + std::string(type) + ".schema.json").validate(data);
    const auto creator = StateCreatorMap.at(type).second;
    return creator(data);
}

std::unique_ptr<Action> Action::Create(const Game &game, const nlohmann::json &data) {
    const auto &type = game.GetType();
    Util::GetJsonValidator("actions/" + std::string(type) + ".schema.json").validate(data);
    const auto creator = ActionCreatorMap.at(type);
    return creator(data);
}

std::unique_ptr<Game> Game::Create(const std::string &type, const nlohmann::json &data) {
    Util::GetJsonValidator("games/" + type + ".schema.json").validate(data);
    const auto creator = GameCreatorMap.at(type);
    return creator(data);
}
