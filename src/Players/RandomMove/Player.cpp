#include "Player.hpp"
#include "../../Utilities/Utilities.hpp"
#include <random>
#include <vector>

namespace random_move {
Player::Player(const Game &game, const State &state, const nlohmann::json &data) : _State(&state) {
    Util::GetJsonValidator("players/random_move.schema.json").validate(data);
    const auto &actionGeneratorData = data["actionGenerator"];
    const std::string &type = actionGeneratorData["type"];
    _ActionGenerator = ActionGenerator::Create(type, game, state, actionGeneratorData["data"]);
    _ActionGeneratorData = ActionGenerator::Data::Create(*_ActionGenerator);
}

std::unique_ptr<Action> Player::GetBestAction(std::optional<std::chrono::duration<double>>) {
    // TODO: Optimize
    std::vector<std::unique_ptr<Action>> actions;
    _ActionGenerator->ForEach(*_ActionGeneratorData, [&](const Action &action) { actions.push_back(action.Clone()); });
    assert(!actions.empty());
    std::uniform_int_distribution<std::size_t> random(0, actions.size() - 1);
    const auto idx = random(Util::GetRandomEngine());
    return std::move(actions[idx]);
}
} // namespace random_move
