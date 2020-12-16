#pragma once

#include <vector>
#include <chrono>
#include <random>
#include <cassert>

template <typename Game>
class RandomPlayer
{
public:
    using GameType = Game;
    using Action = typename Game::Action;

private:
    const Game *_Game;

public:
    inline explicit RandomPlayer(const Game &game) : _Game(&game) {}
    inline explicit RandomPlayer(const Game &game, const std::vector<Action> &) : _Game(&game) {}

    RandomPlayer(const RandomPlayer &) = delete;
    RandomPlayer &operator=(const RandomPlayer &) = delete;
    inline RandomPlayer(RandomPlayer &&) = default;
    inline RandomPlayer &operator=(RandomPlayer &&) = default;

    inline void Notify(Action) const {}
    Action operator()() const
    {
        std::vector<Action> acts;
        for (Action act : *_Game)
            acts.emplace_back(act);
        assert(acts.size() > 0);
        static auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        static std::default_random_engine e(seed);
        std::uniform_int_distribution<unsigned int> random(0, acts.size() - 1);
        return acts[random(e)];
    }
};
