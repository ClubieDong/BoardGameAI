#pragma once

#include <tuple>
#include <cassert>
#include <vector>
#include <type_traits>

template <typename Game, typename... Players>
class Controller
{
    // All players plays the right game
    static_assert(std::conjunction_v<std::is_same<Game, typename Players::GameType>...>);
    static_assert(sizeof...(Players) == Game::PlayerCount);

public:
    inline static constexpr unsigned int PlayerCount = Game::PlayerCount;
    using Action = typename Game::Action;
    using Result = typename Game::Result;

private:
    Game _Game;
    std::vector<Action> _HistoryActions;
    std::tuple<Players...> _Players =
        {Players(const_cast<const Game &>(_Game),
                 const_cast<const std::vector<Action> &>(_HistoryActions))...};
    Result _Score{};

    template <unsigned int N = 0>
    inline Action NthPlayerMove(unsigned int idx)
    {
        // Why I use linear seach instead of binary search?
        // Because for most games, the number of players is no more than 4,
        // in which case linear search performs better than binary search.
        if (idx == N)
            return std::get<N>(_Players)();
        if constexpr (N + 1 < PlayerCount)
            return NthPlayerMove<N + 1>(idx);
        assert(false);
    }

public:
    inline explicit Controller() = default;
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;
    Controller(Controller&&) = default;
    Controller& operator=(Controller&&) = default;

    inline const Result &GetScore() const { return _Score; }

    Result Start()
    {
        while (true)
        {
            auto action = NthPlayerMove(_Game.GetNextPlayer());
            _Game(action);
            _HistoryActions.push_back(std::move(action));
            auto result = _Game.GetResult();
            if (result)
            {
                for (unsigned int i = 0; i < PlayerCount; ++i)
                    _Score[i] += (*result)[i];
                return *result;
            }
        }
    }
};
