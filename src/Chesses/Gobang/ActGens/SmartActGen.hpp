#pragma once

#include <array>
#include <bitset>
#include <variant>
#include "NeighborActGen.hpp"
#include "../../../Utilities/DFA.hpp"

namespace gobang
{
    template <typename Gobang, unsigned char Distance = 2>
    class SmartActGen
    {
    public:
        inline static constexpr unsigned char Size = Gobang::Size;
        using Action = typename Gobang::Action;
        static_assert(Gobang::Renju == 5);

    private:
        const Gobang *_Game;
        NeighborActGen<Gobang, Distance> _NeighborActGen;

        // For each bit in unsigned char:
        // the highest bit
        // 7: four for player1 (i.e. xxxx! xxx!x xx!xx x!xxx !xxxx)
        // 6: four for player2 (i.e. oooo! ooo!o oo!oo o!ooo !oooo)
        // 5: three for player1, offend (i.e. -xxx!- -!xxx- -xx!x- -x!xx-)
        // 4: three for player2, offend (i.e. -ooo!- -!ooo- -oo!o- -o!oo-)
        // 3: three for player1, defend (i.e. !xxx!? ?!xxx! !xx!x! !x!xx!)
        // 2: three for player2, defend (i.e. !ooo!? ?!ooo! !oo!o! !o!oo!)
        // 1: dead four for player1 (i.e. xxx!! xx!x! xx!!x x!xx! x!x!x x!!xx !xxx! !xx!x !x!xx !!xxx)
        // 0: dead four for player2 (i.e. ooo!! oo!o! oo!!o o!oo! o!o!o o!!oo !ooo! !oo!o !o!oo !!ooo)
        // the lowest bit
        // 'x' means player1's move, 'o' means player2's move
        // '-', '!', and '?' mean empty, '!' means selected action
        // '?' means conditionally selected action
        std::array<std::array<unsigned char, Size>, Size> _Fields = {};
        // The number of ones in each of the corresponding fields
        std::array<unsigned char, 8> _Counts = {};

        inline static const DFA<3> _DFA = NFA<3>({
                                                     // Four for player1
                                                     {0, 0, 0, 0, 2}, // 0
                                                     {0, 0, 0, 2, 0}, // 1
                                                     {0, 0, 2, 0, 0}, // 2
                                                     {0, 2, 0, 0, 0}, // 3
                                                     {2, 0, 0, 0, 0}, // 4
                                                     // Four for player2
                                                     {1, 1, 1, 1, 2}, // 5
                                                     {1, 1, 1, 2, 1}, // 6
                                                     {1, 1, 2, 1, 1}, // 7
                                                     {1, 2, 1, 1, 1}, // 8
                                                     {2, 1, 1, 1, 1}, // 9
                                                     // Three for player1
                                                     {2, 0, 0, 0, 2, 2}, // 10
                                                     {2, 2, 0, 0, 0, 2}, // 11
                                                     {2, 0, 0, 2, 0, 2}, // 12
                                                     {2, 0, 2, 0, 0, 2}, // 13
                                                     // Three for player2
                                                     {2, 1, 1, 1, 2, 2}, // 14
                                                     {2, 2, 1, 1, 1, 2}, // 15
                                                     {2, 1, 1, 2, 1, 2}, // 16
                                                     {2, 1, 2, 1, 1, 2}, // 17
                                                     // Dead four for player1
                                                     {0, 0, 0, 2, 2}, // 18
                                                     {0, 0, 2, 0, 2}, // 19
                                                     {0, 0, 2, 2, 0}, // 20
                                                     {0, 2, 0, 0, 2}, // 21
                                                     {0, 2, 0, 2, 0}, // 22
                                                     {0, 2, 2, 0, 0}, // 23
                                                     {2, 0, 0, 0, 2}, // 24
                                                     {2, 0, 0, 2, 0}, // 25
                                                     {2, 0, 2, 0, 0}, // 26
                                                     {2, 2, 0, 0, 0}, // 27
                                                     // Dead four for player2
                                                     {1, 1, 1, 2, 2}, // 28
                                                     {1, 1, 2, 1, 2}, // 29
                                                     {1, 1, 2, 2, 1}, // 30
                                                     {1, 2, 1, 1, 2}, // 31
                                                     {1, 2, 1, 2, 1}, // 32
                                                     {1, 2, 2, 1, 1}, // 33
                                                     {2, 1, 1, 1, 2}, // 34
                                                     {2, 1, 1, 2, 1}, // 35
                                                     {2, 1, 2, 1, 1}, // 36
                                                     {2, 2, 1, 1, 1}, // 37
                                                 })
                                              .Determinate();
        DFA<3>::Simulator _DFASim = DFA<3>::Simulator(_DFA);

        inline void SetBit(unsigned int row, unsigned int col, unsigned int bit)
        {
            unsigned char mask = 1 << bit;
            _Counts[bit] += !(_Fields[row][col] & mask);
            _Fields[row][col] |= mask;
        }

    public:
        class ActionIterator
        {
            friend class SmartActGen;

        private:
            class SmartActIter
            {
            private:
                const SmartActGen *_ActGen;
                unsigned char _Bitmask;
                Action _Action;

            public:
                inline explicit SmartActIter() = default;
                inline explicit SmartActIter(const SmartActGen *actGen, unsigned char bitmask)
                    : _ActGen(actGen), _Bitmask(bitmask) { ++*this; }
                inline bool NotEnd() const { return _Action.Row < Size; }
                inline Action operator*() const { return _Action; }
                inline void operator++()
                {
                    while (true)
                    {
                        ++_Action.Col;
                        if (_Action.Col == Size)
                        {
                            _Action.Col = 0;
                            ++_Action.Row;
                        }
                        if (_Action.Row >= Size)
                            break;
                        if (_ActGen->_Fields[_Action.Row][_Action.Col] & _Bitmask)
                            break;
                    }
                }
            };

            class NeighborActIter
            {
            private:
                typename NeighborActGen<Gobang, Distance>::ActionIterator _Iter, _End;

            public:
                inline explicit NeighborActIter(const NeighborActGen<Gobang, Distance> &actGen)
                    : _Iter(actGen.begin()), _End(actGen.end()) {}
                inline bool NotEnd() const { return _Iter != _End; }
                inline Action operator*() const { return *_Iter; }
                inline void operator++() { ++_Iter; }
            };

            std::variant<SmartActIter, NeighborActIter> _ActIter;

            inline explicit ActionIterator() = default;
            inline explicit ActionIterator(const SmartActGen *actGen, unsigned char bitmask)
                : _ActIter(SmartActIter(actGen, bitmask)) {}
            inline explicit ActionIterator(const NeighborActGen<Gobang, Distance> &actGen)
                : _ActIter(NeighborActIter(actGen)) {}

        public:
            inline bool operator!=(ActionIterator) const
            {
                return _ActIter.index() == 0 ? std::get<0>(_ActIter).NotEnd() : std::get<1>(_ActIter).NotEnd();
            }
            inline Action operator*() const
            {
                return _ActIter.index() == 0 ? *std::get<0>(_ActIter) : *std::get<1>(_ActIter);
            }
            void operator++()
            {
                return _ActIter.index() == 0 ? ++std::get<0>(_ActIter) : ++std::get<1>(_ActIter);
            }
        };

        inline explicit SmartActGen(const Gobang &game) : _Game(&game), _NeighborActGen(game)
        {
            Action act;
            for (act.Row = 0; act.Row < Size; ++act.Row)
                for (act.Col = 0; act.Col < Size; ++act.Col)
                    if (_Game->GetBoard()[act.Row][act.Col] != 2)
                        Notify(act);
        }

        inline ActionIterator begin() const
        {
            if (_Game->GetNextPlayer() == 0)
            {
                // Player1
                if (_Counts[7])
                    return ActionIterator(this, 1 << 7);
                if (_Counts[6])
                    return ActionIterator(this, 1 << 6);
                if (_Counts[5])
                    return ActionIterator(this, 1 << 5);
                if (_Counts[2])
                    return ActionIterator(this, 1 << 2 | 1 << 1);
            }
            else
            {
                // Player2
                if (_Counts[6])
                    return ActionIterator(this, 1 << 6);
                if (_Counts[7])
                    return ActionIterator(this, 1 << 7);
                if (_Counts[4])
                    return ActionIterator(this, 1 << 4);
                if (_Counts[3])
                    return ActionIterator(this, 1 << 3 | 1 << 0);
            }
            return ActionIterator(_NeighborActGen);
        }
        inline ActionIterator end() const { return ActionIterator(); }

        inline void SetGame(const Gobang &game) 
        { 
            _NeighborActGen.SetGame(game);
            _Game = &game;
        }

        void Notify(Action action)
        {
            _NeighborActGen.Notify(action);
            constexpr std::array<int, 4> DX = {0, 1, 1, -1};
            constexpr std::array<int, 4> DY = {1, 0, 1, 1};
            constexpr std::array<int, 5> FOUR = {0, -1, -2, -3, -4};
            constexpr std::array<int, 4> THREE = {-1, -4, -2, -3};
            constexpr std::array<int, 10> DEAD_FOUR_1 = {-1, -2, -2, -3, -3, -3, -4, -4, -4, -4};
            constexpr std::array<int, 10> DEAD_FOUR_2 = {0, 0, -1, 0, -1, -2, 0, -1, -2, -3};
            // Min and max indicies for each direction
            std::array<int, 4> minIdx = {std::max<int>(-5, -action.Col),
                                         std::max<int>(-5, -action.Row),
                                         std::max<int>({-5, -action.Row, -action.Col}),
                                         std::max<int>({-5, action.Row - Size + 1, -action.Col})};
            std::array<int, 4> maxIdx = {std::min<int>(6, Size - action.Col),
                                         std::min<int>(6, Size - action.Row),
                                         std::min<int>({6, Size - action.Row, Size - action.Col}),
                                         std::min<int>({6, action.Row, Size - action.Col})};
            // TODO: Unroll the loop
            for (unsigned int dire = 0; dire < 4; ++dire)
            {
                _DFASim.Restart();
                for (int idx = minIdx[dire], x = action.Row + idx * DX[dire], y = action.Col + idx * DY[dire];
                     idx < maxIdx[dire]; ++idx, x += DX[dire], y += DY[dire])
                {
                    // Clear fields
                    for (unsigned char i = 0, k = 1; i < 8; ++i, k <<= 1)
                        _Counts[i] -= static_cast<bool>(_Fields[x][y] & k);
                    _Fields[x][y] = 0;
                    auto accept = _DFASim.Act(_Game->GetBoard()[x][y]);
                    // Not matched
                    if (accept == -1)
                        continue;
                    // Four for player1
                    if (accept < 5)
                        SetBit(x + FOUR[accept] * DX[dire], y + FOUR[accept] * DY[dire], 7);
                    // Four for player2
                    else if (accept < 10)
                    {
                        accept -= 5;
                        SetBit(x + FOUR[accept] * DX[dire], y + FOUR[accept] * DY[dire], 6);
                    }
                    // Three for player1
                    else if (accept < 14)
                    {
                        accept -= 10;
                        // Offend, only when there is no four for player1
                        if (_Counts[7] == 0)
                            SetBit(x + THREE[accept] * DX[dire], y + THREE[accept] * DY[dire], 5);
                        // Defend, only when there is no four or three for player2
                        if (_Counts[6] == 0 && _Counts[4] == 0)
                        {
                            SetBit(x + THREE[accept] * DX[dire], y + THREE[accept] * DY[dire], 3);
                            if (accept != 0 || idx - 6 < minIdx[dire] || _Game->GetBoard()[x - 6 * DX[dire]][y - 6 * DY[dire]] == 1)
                                SetBit(x, y, 3);
                            if (accept != 1 || idx + 1 >= maxIdx[dire] || _Game->GetBoard()[x + DX[dire]][y + DY[dire]] == 1)
                                SetBit(x - 5 * DX[dire], y - 5 * DY[dire], 3);
                        }
                    }
                    // Three for player2
                    else if (accept < 18)
                    {
                        accept -= 14;
                        // Offend, only when there is no four for player2
                        if (_Counts[6] == 0)
                            SetBit(x + THREE[accept] * DX[dire], y + THREE[accept] * DY[dire], 4);
                        // Defend, only when there is no four or three for player1
                        if (_Counts[7] == 0 && _Counts[5] == 0)
                        {
                            SetBit(x + THREE[accept] * DX[dire], y + THREE[accept] * DY[dire], 2);
                            if (accept != 0 || idx - 6 < minIdx[dire] || _Game->GetBoard()[x - 6 * DX[dire]][y - 6 * DY[dire]] == 0)
                                SetBit(x, y, 2);
                            if (accept != 1 || idx + 1 >= maxIdx[dire] || _Game->GetBoard()[x + DX[dire]][y + DY[dire]] == 0)
                                SetBit(x - 5 * DX[dire], y - 5 * DY[dire], 2);
                        }
                    }
                    // Dead four for player1
                    else if (accept < 28)
                    {
                        // Only when there is no four or three for player1
                        if (_Counts[7] == 0 && _Counts[5] == 0)
                        {
                            accept -= 18;
                            SetBit(x + DEAD_FOUR_1[accept] * DX[dire], y + DEAD_FOUR_1[accept] * DY[dire], 1);
                            SetBit(x + DEAD_FOUR_2[accept] * DX[dire], y + DEAD_FOUR_2[accept] * DY[dire], 1);
                        }
                    }
                    // Dead four for player2
                    else // if (accept < 38)
                    {
                        // Only when there is no four or three for player2
                        if (_Counts[6] == 0 && _Counts[4] == 0)
                        {
                            accept -= 28;
                            SetBit(x + DEAD_FOUR_1[accept] * DX[dire], y + DEAD_FOUR_1[accept] * DY[dire], 0);
                            SetBit(x + DEAD_FOUR_2[accept] * DX[dire], y + DEAD_FOUR_2[accept] * DY[dire], 0);
                        }
                    }
                }
            }
        }
    };
} // namespace gobang
