#include "Player.hpp"
#include "../Utilities/Utilities.hpp"
#include "MCTS/Player.hpp"
#include "RandomMove/Player.hpp"
#include <unordered_map>

template <typename T>
static std::unique_ptr<Player> CreatePlayer(const Game &game, const State &state, const nlohmann::json &data) {
    return std::make_unique<T>(game, state, data);
}

using PlayerCreatorFunc = std::unique_ptr<Player> (*)(const Game &, const State &, const nlohmann::json &);
static const std::unordered_map<std::string, PlayerCreatorFunc> PlayerCreatorMap = {
    {"random_move", CreatePlayer<random_move::Player>},
    {"mcts", CreatePlayer<mcts::Player>},
};

std::unique_ptr<Player> Player::Create(const std::string &type, const Game &game, const State &state,
                                       const nlohmann::json &data) {
    Util::GetJsonValidator("players/" + type + ".schema.json").validate(data);
    const auto creator = PlayerCreatorMap.at(type);
    return creator(game, state, data);
}
