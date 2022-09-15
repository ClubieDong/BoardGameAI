#include "Player.hpp"
#include "../../Games/Game.hpp"
#include "../../Utilities/Utilities.hpp"
#include <random>
#include <vector>

namespace random_move {
std::unique_ptr<Action> Player::GetBestAction(std::optional<std::chrono::duration<double>>) {
    unsigned int count = 0;
    std::unique_ptr<Action> chosenAction;
    auto &engine = Util::GetRandomEngine();
    m_ActionGenerator->ForEachAction(*m_ActionGeneratorData, *m_State, [&](const Action &action) {
        std::uniform_int_distribution<unsigned int> random(0, count++);
        if (random(engine) == 0)
            chosenAction = m_Game->CloneAction(action);
    });
    assert(chosenAction);
    return chosenAction;
}
} // namespace random_move
