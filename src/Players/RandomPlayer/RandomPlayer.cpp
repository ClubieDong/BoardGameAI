#include "RandomPlayer.hpp"
#include <vector>
#include <cassert>
#include <chrono>
#include <random>

std::unique_ptr<ActionBase> RandomPlayer::Move()
{
    // TODO: Optimize
    std::vector<std::unique_ptr<ActionBase>> acts;
    for (auto act = ActGen->NewAction();
         assert(act), ActGen->NextAction(_ActGenData.get(), *act);)
        acts.push_back(act->Clone());
    assert(!acts.empty());
    static auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::default_random_engine e(seed);
    std::uniform_int_distribution<unsigned int> random(0, acts.size() - 1);
    return std::move(acts[random(e)]);
}
