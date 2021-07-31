#pragma once

#include <array>
#include <optional>
#include <iostream>

class TicTacToe
{
public:
    inline static constexpr unsigned int PlayerCount = 2;
    using Result = std::array<double, 2>;
    using Board = std::array<std::array<unsigned char, 3>, 3>;

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
        friend std::istream &operator>>(std::istream &is, Action &action);
        inline friend std::ostream &operator<<(std::ostream &os, Action action)
        {
            os << static_cast<char>(action.Col + 'A') << action.Row + 1;
            return os;
        }
    };

    inline explicit TicTacToe()
    {
        for (auto &i : _Board)
            i.fill(2);
    }

    inline unsigned int GetNextPlayer() const { return _NextPlayer; }
    inline bool IsValid(Action action) const
    {
        return action.Row < 3 && action.Col < 3 && _Board[action.Row][action.Col] == 2;
    }
    inline std::optional<Result> GetResult() const { return _Result; }
    inline const Board &GetBoard() const { return _Board; }

    void operator()(Action action);
    friend std::ostream &operator<<(std::ostream &os, const TicTacToe &game);
};
