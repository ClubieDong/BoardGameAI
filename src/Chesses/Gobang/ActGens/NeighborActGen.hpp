#pragma once

#include <array>
#include <bitset>
#include <algorithm>

namespace gobang
{
    template <typename Gobang, unsigned char Distance = 1>
    class NeighborActGen
    {
    public:
        inline static constexpr unsigned char Size = Gobang::Size;
        using Action = typename Gobang::Action;

    private:
        const Gobang *_Game;
        std::array<std::bitset<Size>, Size> _Available;

    public:
        class ActionIterator
        {
            friend class NeighborActGen;

        private:
            const NeighborActGen *_ActGen = nullptr;
            Action _Action;
            inline explicit ActionIterator() = default;
            inline explicit ActionIterator(const NeighborActGen *actGen)
                : _ActGen(actGen) { ++*this; }

        public:
            inline bool operator!=(ActionIterator) const { return _Action.Row < Size; }
            inline Action operator*() const { return _Action; }
            void operator++()
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
                    if (_ActGen->_Game->GetMoveCount() == 0 || _ActGen->_Available[_Action.Row][_Action.Col])
                        break;
                }
            }
        };

        inline explicit NeighborActGen(const Gobang &game) : _Game(&game)
        {
            Action act;
            for (act.Row = 0; act.Row < Size; ++act.Row)
                for (act.Col = 0; act.Col < Size; ++act.Col)
                    if (_Game->GetBoard()[act.Row][act.Col] != 2)
                        Notify(act);
        }

        inline ActionIterator begin() const { return ActionIterator(this); }
        inline ActionIterator end() const { return ActionIterator(); }

        inline void SetGame(const Gobang &game) { _Game = &game; }

        inline void Notify(Action action)
        {
            unsigned char xbegin = std::max(0, action.Row - Distance);
            unsigned char xend = std::min(Size - 1, action.Row + Distance);
            unsigned char ybegin = std::max(0, action.Col - Distance);
            unsigned char yend = std::min(Size - 1, action.Col + Distance);
            for (auto x = xbegin; x <= xend; ++x)
                for (auto y = ybegin; y <= yend; ++y)
                    _Available[x][y] = _Game->GetBoard()[x][y] == 2;
        }
    };
} // namespace gobang
