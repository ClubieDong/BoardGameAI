#pragma once

#include <tuple>
#include <cassert>
#include <vector>
#include <type_traits>
#include <optional>
#include <memory>

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
    using PlayersType = std::tuple<std::unique_ptr<Players>...>;

private:
    Result _Score = {};

    template <unsigned int N = 0>
    inline Action NthPlayerMove(PlayersType &players, unsigned int idx)
    {
        // Why I use linear seach instead of binary search?
        // Because for most games, the number of players is no more than 4,
        // in which case linear search performs better than binary search.
        if (idx == N)
            return (*std::get<N>(players))();
        if constexpr (N + 1 < PlayerCount)
            return NthPlayerMove<N + 1>(players, idx);
        assert(false);
    }

    template <unsigned int N = 0>
    inline void NotifyAllPlayers(PlayersType &players, Action action)
    {
        std::get<N>(players)->Notify(action);
        if constexpr (N + 1 < PlayerCount)
            NotifyAllPlayers<N + 1>(players, action);
    }

public:
    inline explicit Controller() = default;
    Controller(const Controller &) = delete;
    Controller &operator=(const Controller &) = delete;
    inline Controller(Controller &&) = default;
    inline Controller &operator=(Controller &&) = default;

    inline const Result &GetScore() const { return _Score; }

    Result Start()
    {
        Game game;
        std::vector<Action> historyActs;
        PlayersType players = {std::make_unique<Players>(const_cast<const Game &>(game),
                                                         const_cast<const std::vector<Action> &>(historyActs))...};
        std::optional<Result> result;
        while (!result)
        {
            auto player = game.GetNextPlayer();
            auto action = NthPlayerMove(players, player);
            game(action);
            NotifyAllPlayers(players, action);
            historyActs.push_back(std::move(action));
            result = game.GetResult();
        }
        for (unsigned int i = 0; i < PlayerCount; ++i)
            _Score[i] += (*result)[i];
        return *result;
    }
};
