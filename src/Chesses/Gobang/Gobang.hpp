#pragma once

#include <array>
#include <optional>
#include <iostream>
#include <string>
#include <iomanip>
#include <cassert>
#include <cctype>
#include <bitset>

template <unsigned char Size = 15, unsigned char Renju = 5>
class Gobang
{
    static_assert(Renju >= 3);
    static_assert(Renju <= Size && Size <= 26);

public:
    inline static constexpr unsigned int PlayerCount = 2;
    using Result = std::array<double, 2>;

private:
    // 0: Player1; 1: Player2; 2: Empty
    std::array<std::array<unsigned char, Size>, Size> _Board;
    unsigned int _NextPlayer = 0, _MoveCount = 0;
    std::optional<Result> _Result;
    std::array<std::bitset<Size>, Size> _Available;

public:
    class Action
    {
        friend class Gobang;
        friend class ActionIterator;

    private:
        unsigned char _Row = 0, _Col = -1;
        inline explicit Action(unsigned char row, unsigned char col) : _Row(row), _Col(col) {}

    public:
        inline explicit Action() = default;
        friend std::istream &operator>>(std::istream &is, Action &action)
        {
            std::string value;
            std::getline(is, value);
            action._Row = action._Col = -1;
            for (size_t i = 0; i < value.size(); ++i)
                if (std::isupper(value[i]))
                {
                    action._Col = value[i] - 'A';
                    break;
                }
                else if (std::islower(value[i]))
                {
                    action._Col = value[i] - 'a';
                    break;
                }
            if (action._Col == static_cast<unsigned char>(-1))
                return is;
            for (size_t i = 0; i < value.size(); ++i)
                if (std::isdigit(value[i]))
                {
                    action._Row = std::stoi(value.substr(i));
                    --action._Row;
                    break;
                }
            return is;
        }
        inline friend std::ostream &operator<<(std::ostream &os, Action action)
        {
            os << static_cast<char>(action._Col + 'A') << action._Row + 1;
            return os;
        }
    };

    class ActionIterator
    {
        friend class Gobang;

    private:
        const Gobang *_Game = nullptr;
        Action _Action;
        inline explicit ActionIterator() = default;
        inline explicit ActionIterator(const Gobang *game) : _Game(game) { ++*this; }

    public:
        inline bool operator!=(ActionIterator) const { return _Action._Row < Size; }
        inline Action operator*() const { return _Action; }
        void operator++()
        {
            while (true)
            {
                ++_Action._Col;
                if (_Action._Col == Size)
                {
                    _Action._Col = 0;
                    ++_Action._Row;
                }
                if (_Action._Row >= Size)
                    break;
                if (_Game->_MoveCount == 0 || _Game->_Available[_Action._Row][_Action._Col])
                    break;
            }
        }
    };

    inline ActionIterator begin() const { return ActionIterator(this); }
    inline ActionIterator end() const { return ActionIterator(); }

    inline explicit Gobang()
    {
        for (auto &i : _Board)
            i.fill(2);
    }

    inline unsigned int GetNextPlayer() const { return _NextPlayer; }
    inline bool IsValid(Action action) const
    {
        return action._Row < Size && action._Col < Size && _Board[action._Row][action._Col] == 2;
    }
    inline std::optional<Result> GetResult() const { return _Result; }

    void operator()(Action action)
    {
        assert(IsValid(action));
        _Board[action._Row][action._Col] = _NextPlayer;
        unsigned char xbegin = std::max(0, action._Row - 2);
        unsigned char xend = std::min(Size - 1, action._Row + 2);
        unsigned char ybegin = std::max(0, action._Col - 2);
        unsigned char yend = std::min(Size - 1, action._Col + 2);
        for (auto x = xbegin; x <= xend; ++x)
            for (auto y = ybegin; y <= yend; ++y)
                _Available[x][y] = _Board[x][y] == 2;
        ++_MoveCount;
        constexpr std::array<signed char, 4> DX = {0, 1, 1, 1};
        constexpr std::array<signed char, 4> DY = {1, 0, 1, -1};
        bool win = false;
        for (unsigned char dire = 0; dire < 4; ++dire)
        {
            unsigned char count = 0;
            auto x = action._Row, y = action._Col;
            while (true)
            {
                x += DX[dire], y += DY[dire];
                if (x >= Size || y >= Size)
                    break;
                if (_Board[x][y] != _NextPlayer)
                    break;
                ++count;
                if (count >= Renju - 1)
                {
                    win = true;
                    break;
                }
            }
            if (win)
                break;
            x = action._Row, y = action._Col;
            while (true)
            {
                x -= DX[dire], y -= DY[dire];
                if (x >= Size || y >= Size)
                    break;
                if (_Board[x][y] != _NextPlayer)
                    break;
                ++count;
                if (count >= Renju - 1)
                {
                    win = true;
                    break;
                }
            }
            if (win)
                break;
        }
        if (win)
        {
            _Result.emplace();
            (*_Result)[_NextPlayer] = 1;
            (*_Result)[!_NextPlayer] = 0;
        }
        // Draw
        else if (_MoveCount == Size * Size)
        {
            _Result.emplace();
            (*_Result)[0] = (*_Result)[1] = 0.5;
        }
        _NextPlayer = !_NextPlayer;
    }
    friend std::ostream &operator<<(std::ostream &os, const Gobang &game)
    {
        if constexpr (Size == 15)
            os << "           v       v       v\n  ";
        os << "   ";
        for (char i = 'A'; i < 'A' + Size; ++i)
            os << i << ' ';
        os << '\n';
        for (unsigned char i = 0; i < Size; ++i)
        {
            if constexpr (Size == 15)
            {
                if (i == 3 || i == 7 || i == 11)
                    os << "> ";
                else
                    os << "  ";
            }
            os << std::setw(2) << i + 1 << ' ';
            for (unsigned char j = 0; j < Size; ++j)
                if (game._Board[i][j] == 0)
                    os << "○ ";
                else if (game._Board[i][j] == 1)
                    os << "● ";
                else
                    os << "  ";
            os << std::setw(2) << std::left << i + 1 << std::right;
            if constexpr (Size == 15)
            {
                if (i == 3 || i == 7 || i == 11)
                    os << " <";
                else
                    os << "  ";
            }
            os << '\n';
        }
        if constexpr (Size == 15)
            os << "  ";
        os << "   ";
        for (char i = 'A'; i < 'A' + Size; ++i)
            os << i << ' ';
        os << '\n';
        if constexpr (Size == 15)
            os << "           ^       ^       ^\n";
        return os;
    }
};
