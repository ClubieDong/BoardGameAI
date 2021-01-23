#pragma once

#include <vector>
#include <array>
#include <set>
#include <cassert>
#include <map>
#include <queue>
#include <algorithm>

template <unsigned int TokenSize>
class NFA;

template <unsigned int TokenSize>
class DFA
{
    friend class NFA<TokenSize>;
    friend class Simulator;

private:
    class Row
    {
    private:
        std::array<unsigned int, TokenSize> _Goto;
        int _Accept = -1;

    public:
        inline void SetTransfer(unsigned int token, unsigned int state)
        {
            assert(token < TokenSize);
            _Goto[token] = state;
        }
        inline int GetAccept() const { return _Accept; }
        inline void SetAccept(int accept) { _Accept = accept; }
        inline const unsigned int &operator[](unsigned int token) const { return _Goto[token]; }
    };

    std::vector<Row> _Table;

public:
    class Simulator
    {
    private:
        const DFA *_DFA;
        unsigned int _State = 0;

    public:
        inline explicit Simulator(const DFA &dfa) : _DFA(&dfa) {}
        inline void Restart() { _State = 0; }
        inline int Act(unsigned int token)
        {
            _State = _DFA->_Table[_State][token];
            return _DFA->_Table[_State].GetAccept();
        }
    };
};

template <unsigned int TokenSize>
class NFA
{
private:
    class Row
    {
    private:
        std::array<std::set<unsigned int>, TokenSize> _Goto;
        int _Accept = -1;

    public:
        inline void AddTransfer(unsigned int token, unsigned int state)
        {
            assert(token < TokenSize);
            _Goto[token].insert(state);
        }
        inline int GetAccept() const { return _Accept; }
        inline void SetAccept(int accept) { _Accept = accept; }
        inline const std::set<unsigned int> &operator[](unsigned int token) const { return _Goto[token]; }
    };

    std::vector<Row> _Table;

public:
    NFA(const std::vector<std::vector<unsigned int>> &templates)
    {
        // Add self loop of init state, for all tokens
        _Table.emplace_back();
        for (unsigned int i = 0; i < TokenSize; ++i)
            _Table.front().AddTransfer(i, 0);
        for (unsigned int i = 0; i < templates.size(); ++i)
        {
            const auto &t = templates[i];
            assert(t.size() > 0);
            _Table.front().AddTransfer(t[0], _Table.size());
            _Table.emplace_back();
            for (unsigned int j = 1; j < t.size(); ++j)
            {
                _Table.back().AddTransfer(t[j], _Table.size());
                _Table.emplace_back();
            }
            _Table.back().SetAccept(i);
        }
    }

    DFA<TokenSize> Determinate() const
    {
        DFA<TokenSize> dfa;
        std::map<std::set<unsigned int>, unsigned int> map;
        std::queue<decltype(map)::const_iterator> queue;
        map.emplace(std::set<unsigned int>{0}, 0);
        queue.push(map.cbegin());
        while (!queue.empty())
        {
            const auto &p = queue.front()->first;
            queue.pop();
            dfa._Table.emplace_back();
            for (unsigned int token = 0; token < TokenSize; ++token)
            {
                // Calculate the union
                std::set<unsigned int> uni;
                for (auto i : p)
                {
                    const auto &s = _Table[i][token];
                    uni.insert(s.cbegin(), s.cend());
                }
                // Check if the union already exists
                auto iter = map.find(uni);
                if (iter == map.end())
                {
                    // New state
                    auto newState = map.size();
                    auto [insertedIter, success] = map.emplace(std::move(uni), newState);
                    assert(success);
                    queue.push(insertedIter);
                    dfa._Table.back().SetTransfer(token, newState);
                }
                else
                    // Already exists
                    dfa._Table.back().SetTransfer(token, iter->second);
            }
            // Check if there is one and only one accept state in p
            int accept = -1;
            for (auto i : p)
            {
                auto t = _Table[i].GetAccept();
                if (t != -1 && (accept == -1 || t < accept))
                    accept = t;
            }
            dfa._Table.back().SetAccept(accept);
        }
        return dfa;
    }
};
