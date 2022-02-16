#include "Player.hpp"
#include "../../Games/Game.hpp"
#include "../../Utilities/Utilities.hpp"
#include <random>
#include <vector>

namespace random_move {
Player::Player(const Game &game, const State &state, const nlohmann::json &data) : m_Game(&game), m_State(&state) {
    const auto &actionGeneratorData = data["actionGenerator"];
    const std::string &type = actionGeneratorData["type"];
    m_ActionGenerator = ActionGenerator::Create(type, game, state, actionGeneratorData["data"]);
    m_ActionGeneratorData = m_ActionGenerator->CreateData();
}

std::unique_ptr<Action> Player::GetBestAction(std::optional<std::chrono::duration<double>>) {
    // TODO: Optimize
    std::vector<std::unique_ptr<Action>> actions;
    m_ActionGenerator->ForEach(*m_ActionGeneratorData,
                               [&](const Action &action) { actions.push_back(m_Game->CloneAction(action)); });
    assert(!actions.empty());
    std::uniform_int_distribution<std::size_t> random(0, actions.size() - 1);
    const auto idx = random(Util::GetRandomEngine());
    return std::move(actions[idx]);
}
} // namespace random_move
