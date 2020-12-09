#include <tuple>
#include <cassert>
#include <vector>

template <typename Game, template <typename> typename... Players>
class Controller
{
    static_assert(sizeof...(Players) == Game::PlayerCount);

private:
    using Action = typename Game::Action;
    using GameResult = std::array<double, Game::PlayerCount>;

    Game _Game;
    std::vector<Action> _HistoryActions;
    std::tuple<Players<Game>...> _Players =
        {Players<Game>(const_cast<const Game &>(_Game),
                       const_cast<const std::vector<Action> &>(_HistoryActions))...};
    GameResult _Score{};

    template <unsigned int N = 0>
    inline Action NthPlayerMove(unsigned int idx)
    {
        // Why I use linear seach instead of binary search?
        // Because for most games, the number of players is no more than 4,
        // in which case linear search performs better than binary search.
        if (idx == N)
            return std::get<N>(_Players)();
        if constexpr (N + 1 < Game::PlayerCount)
            return NthPlayerMove<N + 1>(idx);
        assert(false);
    }

public:
    inline const std::vector<Action> &GetHistoryActions() const { return _HistoryActions; }
    inline const GameResult &GetScore() const { return _Score; }

    GameResult Start()
    {
        while (true)
        {
            auto action = NthPlayerMove(_Game.GetNextPlayer());
            auto result = _Game(action);
            _HistoryActions.push_back(std::move(action));
            if (result)
            {
                for (unsigned int i = 0; i < Game::PlayerCount; ++i)
                    _Score[i] += (*result)[i];
                return *result;
            }
        }
    }
};
