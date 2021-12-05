#include "Game.hpp"
#include <unordered_map>
#include <string>
#include <string_view>
#include <typeindex>
#include "TicTacToe/Game.hpp"
#include "../Utilities/Utilities.hpp"

template <typename T>
static std::unique_ptr<State> CreateDefaultState(const Game &game)
{
    return std::make_unique<T>(game);
}

template <typename T>
static std::unique_ptr<State> CreateState(const nlohmann::json &data)
{
    return std::make_unique<T>(data);
}

template <typename T>
static std::unique_ptr<Action> CreateAction(const nlohmann::json &data)
{
    return std::make_unique<T>(data);
}

template <typename T>
static std::unique_ptr<Game> CreateGame(const nlohmann::json &data)
{
    return std::make_unique<T>(data);
}

using DefaultStateCreatorFunc = std::unique_ptr<State> (*)(const Game &);
using StateCreatorFunc = std::unique_ptr<State> (*)(const nlohmann::json &);
static const std::unordered_map<std::type_index, std::pair<DefaultStateCreatorFunc, StateCreatorFunc>> StateCreatorMap = {
    {typeid(tic_tac_toe::Game), {CreateDefaultState<tic_tac_toe::State>, CreateState<tic_tac_toe::State>}},
};

using ActionCreatorFunc = std::unique_ptr<Action> (*)(const nlohmann::json &);
static const std::unordered_map<std::type_index, ActionCreatorFunc> ActionCreatorMap = {
    {typeid(tic_tac_toe::Game), CreateAction<tic_tac_toe::Action>},
};

using GameCreatorFunc = std::unique_ptr<Game> (*)(const nlohmann::json &);
static const std::unordered_map<std::string, GameCreatorFunc> GameCreatorMap = {
    {"tic_tac_toe", CreateGame<tic_tac_toe::Game>},
};

std::unique_ptr<State> State::Create(const Game &game)
{
    const std::type_index type = typeid(game);
    const auto creator = StateCreatorMap.at(type).first;
    return creator(game);
}

std::unique_ptr<State> State::Create(const Game &game, const nlohmann::json &data)
{
    const std::type_index type = typeid(game);
    const auto creator = StateCreatorMap.at(type).second;
    return creator(data);
}

std::unique_ptr<Action> Action::Create(const Game &game, const nlohmann::json &data)
{
    const std::type_index type = typeid(game);
    const auto creator = ActionCreatorMap.at(type);
    return creator(data);
}

std::unique_ptr<Game> Game::Create(const nlohmann::json &data)
{
    // `data` has been validated in caller
    const std::string &type = data["type"];
    const nlohmann::json &gameData = data["data"];
    const auto creator = GameCreatorMap.at(type);
    return creator(gameData);
}
