#include "Player.hpp"
#include "../../Games/Game.hpp"
#include "../../Utilities/Utilities.hpp"
#include <random>
#include <vector>

namespace random_move {
Player::Player(const Game &game, const State &state, const nlohmann::json &data) : m_Game(&game), m_State(&state) {
    const auto &actionGeneratorJson = data["actionGenerator"];
    m_ActionGenerator = ActionGenerator::Create(actionGeneratorJson["type"], game, actionGeneratorJson["data"]);
    m_ActionGeneratorData = m_ActionGenerator->CreateData(state);
}

std::unique_ptr<Action> Player::GetBestAction(std::optional<std::chrono::duration<double>>) {
    unsigned int count = 0;
    std::unique_ptr<Action> chosenAction;
    m_ActionGenerator->ForEachAction(*m_ActionGeneratorData, *m_State, [&](const Action &action) {
        std::uniform_int_distribution<unsigned int> random(0, count++);
        if (random(Util::GetRandomEngine()) == 0)
            chosenAction = m_Game->CloneAction(action);
    });
    assert(chosenAction);
    return chosenAction;
}
} // namespace random_move
