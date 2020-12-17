#pragma once

#include <vector>
#include <memory>
#include <cmath>
#include <cassert>
#include <set>
#include <chrono>
#include <random>
#include <ratio>
#include <type_traits>
#include "../RandomPlayer/RandomPlayer.hpp"

template <typename Game, typename ActGen, unsigned int Iterations, typename Ratio = std::ratio<14, 10>,
          typename Rollout = RandomPlayer<Game, ActGen>>
class MCTS
{
    static_assert(Iterations > 0);
    static_assert(std::is_same_v<Game, typename Rollout::GameType>);

public:
    inline static constexpr unsigned int PlayerCount = Game::PlayerCount;
    using GameType = Game;
    using Action = typename Game::Action;
    using Result = typename Game::Result;

private:
    inline static constexpr double C = static_cast<double>(Ratio::num) / Ratio::den;
    static_assert(C >= 0);

    class Node
    {
        friend class MCTS;

    private:
        Node *_Parent = nullptr;
        Action _Action;
        std::vector<std::unique_ptr<Node>> _Children;
        std::unique_ptr<Game> _Game;
        std::unique_ptr<ActGen> _ActGen;
        unsigned int _NextPlayer;
        std::array<double, PlayerCount> _Value{};
        unsigned int _Count = 0;

    public:
        inline explicit Node(const Game &game, const ActGen &actGen)
            : _Game(std::make_unique<Game>(game)),
              _ActGen(std::make_unique<ActGen>(actGen)),
              _NextPlayer(_Game->GetNextPlayer()) {}
        inline explicit Node(std::unique_ptr<Game> game, std::unique_ptr<ActGen> actGen, Action action, Node &parent)
            : _Parent(&parent), _Action(action), _Game(std::move(game)),
              _ActGen(std::move(actGen)), _NextPlayer(_Game->GetNextPlayer()) {}
    };

    const Game *_Game;
    ActGen _ActGen;

public:
    inline explicit MCTS(const Game &game) : _Game(&game), _ActGen(game) {}
    inline explicit MCTS(const Game &game, const std::vector<Action> &) : MCTS(game) {}

    MCTS(const MCTS &) = delete;
    MCTS &operator=(const MCTS &) = delete;
    inline MCTS(MCTS &&) = default;
    inline MCTS &operator=(MCTS &&) = default;

    inline void Notify(Action act) { _ActGen.Notify(act); }
    Action operator()()
    {
        Node root(*_Game, _ActGen);
        for (unsigned int iter = 0; iter < Iterations; ++iter)
        {
            // Selection
            Node *p = &root;
            // While p is not a leaf node
            while (p->_Children.size() != 0)
            {
                auto player = p->_NextPlayer;
                double maxUCB = 0;
                unsigned int index = -1;
                for (unsigned int i = 0; i < p->_Children.size(); ++i)
                {
                    auto &node = *p->_Children[i];
                    // Avoid 0 as divider
                    // When n_i == 0, UCB is infinite
                    if (node._Count == 0)
                    {
                        index = i;
                        break;
                    }
                    double ucb = node._Value[player] / node._Count;
                    ucb += C * std::sqrt(std::log(p->_Count) / node._Count);
                    if (ucb > maxUCB)
                    {
                        maxUCB = ucb;
                        index = i;
                    }
                }
                assert(index < p->_Children.size());
                p = p->_Children[index].get();
            }
            // Expansion
            if (p->_Count != 0 && !p->_Game->GetResult())
            {
                for (auto act : *p->_ActGen)
                {
                    auto game = std::make_unique<Game>(*p->_Game);
                    auto actGen = std::make_unique<ActGen>(*p->_ActGen);
                    actGen->SetGame(*game);
                    (*game)(act);
                    actGen->Notify(act);
                    auto node = std::make_unique<Node>(std::move(game), std::move(actGen), act, *p);
                    p->_Children.push_back(std::move(node));
                }
                assert(p->_Children.size() > 0);
                p->_Game = nullptr;
                p->_ActGen = nullptr;
                p = p->_Children[0].get();
            }
            // Rollout
            Game game = *p->_Game;
            Rollout roll(game);
            auto value = game.GetResult();
            while (!value)
            {
                auto act = roll();
                game(act);
                roll.Notify(act);
                value = game.GetResult();
            }
            // Back propagation
            while (true)
            {
                ++p->_Count;
                for (unsigned int i = 0; i < PlayerCount; ++i)
                    p->_Value[i] += (*value)[i];
                if (p == &root)
                    break;
                p = p->_Parent;
            }
        }
        // Choose best move
        std::set<unsigned int> bestSet;
        double bestWinRate = 0;
        for (unsigned int i = 0; i < root._Children.size(); ++i)
        {
            double rate = root._Children[i]->_Value[_Game->GetNextPlayer()] /
                          root._Children[i]->_Count;
            if (rate == bestWinRate)
                bestSet.insert(i);
            else if (rate > bestWinRate)
            {
                bestSet.clear();
                bestSet.insert(i);
                bestWinRate = rate;
            }
        }
        std::vector<unsigned int> bests(bestSet.cbegin(), bestSet.cend());
        static auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        static std::default_random_engine e(seed);
        std::uniform_int_distribution<unsigned int> random(0, bests.size() - 1);
        return root._Children[bests[random(e)]]->_Action;
    }
};
