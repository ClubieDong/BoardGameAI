#pragma once

#include <ratio>
#include <vector>
#include <memory>
#include <array>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <set>
#include <chrono>
#include <algorithm>
#include "../RandomPlayer/RandomPlayer.hpp"
#include "../../Utilities/ThreadSafeQueue.hpp"

template <typename Game, unsigned int TimeLimit, unsigned long long MemoryLimit, unsigned int WorkerLimit,
          typename Ratio = std::ratio<14, 10>, typename Rollout = RandomPlayer<Game>>
class ParallelMCTS
{
    static_assert(TimeLimit > 0);
    static_assert(WorkerLimit > 0);
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
        friend class ParallelMCTS;

    private:
        inline static unsigned long long _NodeCount = 0;
        inline static unsigned long long _GameCount = 0;

        Node *_Parent = nullptr;
        std::vector<std::unique_ptr<Node>> _Children;
        std::unique_ptr<Game> _Game;
        unsigned int _NextPlayer;
        Action _Action;
        std::array<double, PlayerCount> _Value{};
        unsigned int _Count = 0, _Working = 0;

        inline void RemoveGame()
        {
            if (!_Game)
                return;
            _Game = nullptr;
            --_GameCount;
        }

    public:
        inline explicit Node(const Game &game)
            : _Game(std::make_unique<Game>(game)), _NextPlayer(game.GetNextPlayer())
        {
            ++_NodeCount;
            ++_GameCount;
        }
        inline explicit Node(const Game &game, Action action, Node &parent)
            : _Parent(&parent), _Game(std::make_unique<Game>(game)),
              _NextPlayer(game.GetNextPlayer()), _Action(action)
        {
            ++_NodeCount;
            ++_GameCount;
        }
        inline ~Node()
        {
            if (_Game)
                --_GameCount;
            --_NodeCount;
        }
    };

    const Game *_Game;
    std::unique_ptr<Node> _Root;
    mutable std::mutex _RootMtx;
    std::condition_variable _RootCV;
    std::atomic<unsigned int> _Pending = 0;
    ThreadSafeQueue<Action> _ActionQueue;
    ThreadSafeQueue<std::pair<Node *, Game>> _RolloutQueue;
    ThreadSafeQueue<std::pair<Node *, Result>> _ResultQueue;
    std::thread _TPrune, _TSelExp, _TBp;
    std::array<std::thread, WorkerLimit> _TSims;

    void Prune()
    {
        std::unique_lock<std::mutex> rootLock(_RootMtx);
        while (true)
        {
            while (_ActionQueue.IsEmpty() || _Pending > 0)
                _RootCV.wait(rootLock);
            while (!_ActionQueue.IsEmpty())
            {
                auto action = _ActionQueue.Pop();
                auto pred = [action](const std::unique_ptr<Node> &x) { return x->_Action == action; };
                auto iter = std::find_if(_Root->_Children.begin(), _Root->_Children.end(), pred);
                if (iter == _Root->_Children.end())
                    _Root = std::make_unique<Node>(*_Game);
                else
                    _Root = std::move(*iter);
            }
            _RootCV.notify_all();
        }
    }

    void SelectAndExpand()
    {
        std::unique_lock<std::mutex> rootLock(_RootMtx);
        while (true)
        {
            while (!_ActionQueue.IsEmpty() || _RolloutQueue.GetSize() >= WorkerLimit ||
                   (MemoryLimit != 0 && Node::_NodeCount * sizeof(Node) + Node::_GameCount * sizeof(Game) > MemoryLimit))
                _RootCV.wait(rootLock);
            while (_RolloutQueue.GetSize() < WorkerLimit)
            {
                ++_Pending;
                // Selection
                Node *p = _Root.get();
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
                        if (node._Count + p->_Working == 0)
                        {
                            index = i;
                            break;
                        }
                        double ucb;
                        if (node._Count == 0)
                            ucb = 1.0 / PlayerCount;
                        else
                            ucb = node._Value[player] / node._Count;
                        ucb += C * std::sqrt(std::log(p->_Count + p->_Working) / (node._Count + node._Working));
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
                    for (auto act : *p->_Game)
                    {
                        auto game = *p->_Game;
                        game(act);
                        auto node = std::make_unique<Node>(game, act, *p);
                        p->_Children.push_back(std::move(node));
                    }
                    assert(p->_Children.size() > 0);
                    p->_Children.shrink_to_fit();
                    p->RemoveGame();
                    p = p->_Children[0].get();
                }
                _RolloutQueue.Push({p, *p->_Game});
                // Incomplete back propagation
                while (true)
                {
                    ++p->_Working;
                    if (p == _Root.get())
                        break;
                    p = p->_Parent;
                }
            }
        }
    }

    void Simulate()
    {
        while (true)
        {
            auto [p, game] = _RolloutQueue.Pop();
            _RootCV.notify_all();
            // Rollout
            Rollout roll(game);
            auto value = game.GetResult();
            while (!value)
            {
                auto act = roll();
                game(act);
                value = game.GetResult();
            }
            _ResultQueue.Push({p, *value});
        }
    }

    void BackPropagate()
    {
        while (true)
        {
            auto [p, value] = _ResultQueue.Pop();
            std::lock_guard<std::mutex> rootLock(_RootMtx);
            // Complete back propagation
            while (true)
            {
                ++p->_Count;
                --p->_Working;
                for (unsigned int i = 0; i < PlayerCount; ++i)
                    p->_Value[i] += value[i];
                if (p == _Root.get())
                    break;
                p = p->_Parent;
            }
            --_Pending;
            _RootCV.notify_all();
        }
    }

public:
    inline explicit ParallelMCTS(const Game &game) : _Game(&game), _Root(std::make_unique<Node>(game))
    {
        _TPrune = std::thread([this] { Prune(); });
        _TSelExp = std::thread([this] { SelectAndExpand(); });
        for (unsigned int i = 0; i < WorkerLimit; ++i)
            _TSims[i] = std::thread([this] { Simulate(); });
        _TBp = std::thread([this] { BackPropagate(); });
    }
    inline explicit ParallelMCTS(const Game &game, const std::vector<Action> &) : ParallelMCTS(game) {}

    ParallelMCTS(const ParallelMCTS &) = delete;
    ParallelMCTS &operator=(const ParallelMCTS &) = delete;
    ParallelMCTS(ParallelMCTS &&) = delete;
    ParallelMCTS &operator=(ParallelMCTS &&) = delete;

    inline void Notify(Action action)
    {
        _ActionQueue.Push(action);
        _RootCV.notify_all();
    }

    inline Action operator()()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(TimeLimit));
        std::lock_guard<std::mutex> rootLock(_RootMtx);
        // Choose best action
        std::set<unsigned int> bestSet;
        double bestWinRate = 0;
        for (unsigned int i = 0; i < _Root->_Children.size(); ++i)
        {
            double rate = _Root->_Children[i]->_Value[_Game->GetNextPlayer()] /
                          _Root->_Children[i]->_Count;
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
        Action act = _Root->_Children[bests[random(e)]]->_Action;
        return act;
    }
};
