#pragma once

#include <array>
#include <optional>
#include <iostream>
#include <string>
#include <iomanip>
#include <cassert>
#include <cctype>
#include <bitset>

template <unsigned char _Size = 15, unsigned char Renju = 5>
class Gobang
{
public:
    inline static constexpr unsigned char Size = _Size;
    inline static constexpr unsigned int PlayerCount = 2;
    using Result = std::array<double, 2>;
    using Board = std::array<std::array<unsigned char, Size>, Size>;

    static_assert(Renju >= 3);
    static_assert(Renju <= Size && Size <= 26);

private:
    // 0: Player1; 1: Player2; 2: Empty
    Board _Board;
    unsigned int _NextPlayer = 0, _MoveCount = 0;
    std::optional<Result> _Result;

public:
    class Action
    {
    public:
        unsigned char Row = 0, Col = -1;

        inline explicit Action() = default;
        inline explicit Action(unsigned char row, unsigned char col) : Row(row), Col(col) {}

        inline friend bool operator==(Action lhs, Action rhs)
        {
            return lhs.Row == rhs.Row && lhs.Col == rhs.Col;
        }
        friend std::istream &operator>>(std::istream &is, Action &action)
        {
            std::string value;
            std::getline(is, value);
            action.Row = action.Col = -1;
            for (size_t i = 0; i < value.size(); ++i)
                if (std::isupper(value[i]))
                {
                    action.Col = value[i] - 'A';
                    break;
                }
                else if (std::islower(value[i]))
                {
                    action.Col = value[i] - 'a';
                    break;
                }
            if (action.Col == static_cast<unsigned char>(-1))
                return is;
            for (size_t i = 0; i < value.size(); ++i)
                if (std::isdigit(value[i]))
                {
                    action.Row = std::stoi(value.substr(i));
                    --action.Row;
                    break;
                }
            return is;
        }
        inline friend std::ostream &operator<<(std::ostream &os, Action action)
        {
            os << static_cast<char>(action.Col + 'A') << action.Row + 1;
            return os;
        }
    };

    inline explicit Gobang()
    {
        for (auto &i : _Board)
            i.fill(2);
    }

    inline unsigned int GetNextPlayer() const { return _NextPlayer; }
    inline bool IsValid(Action action) const
    {
        return action.Row < Size && action.Col < Size && _Board[action.Row][action.Col] == 2;
    }
    inline std::optional<Result> GetResult() const { return _Result; }
    inline const Board &GetBoard() const { return _Board; }
    inline unsigned int GetMoveCount() const { return _MoveCount; }

    void operator()(Action action)
    {
        assert(IsValid(action));
        _Board[action.Row][action.Col] = _NextPlayer;
        ++_MoveCount;
        constexpr std::array<signed char, 4> DX = {0, 1, 1, 1};
        constexpr std::array<signed char, 4> DY = {1, 0, 1, -1};
        bool win = false;
        for (unsigned char dire = 0; dire < 4; ++dire)
        {
            unsigned char count = 0;
            auto x = action.Row, y = action.Col;
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
            x = action.Row, y = action.Col;
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
                    // os << "○ ";
                    os << "* ";
                else if (game._Board[i][j] == 1)
                    // os << "● ";
                    os << "@ ";
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
