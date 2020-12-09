#include <array>
#include <optional>
#include <iostream>

class TicTacToe
{
private:
    // 0: Player1; 1: Player2; 2: Empty
    std::array<std::array<unsigned char, 3>, 3> _Board;
    unsigned int _NextPlayer = 0, _MoveCount = 0;

public:
    inline static constexpr unsigned int PlayerCount = 2;
    using MoveResult = std::optional<std::array<double, 2>>;

    class Action
    {
        friend class TicTacToe;
        friend class ActionIterator;

    private:
        unsigned char _Row = -1, _Col = -1;
        inline explicit Action(unsigned char row, unsigned char col) : _Row(row), _Col(col) {}

    public:
        inline explicit Action() = default;
        friend std::istream &operator>>(std::istream &is, Action &action);
        inline friend std::ostream &operator<<(std::ostream &os, Action action)
        {
            os << static_cast<char>(action._Col + 'A') << action._Row + 1;
            return os;
        }
    };

    class ActionIterator
    {
        friend class TicTacToe;

    private:
        const TicTacToe *_Game;
        Action _Action;
        inline explicit ActionIterator(const TicTacToe *game) : _Game(game), _Action(0, -1) { ++*this; }

    public:
        inline bool operator!=(ActionIterator) const { return _Action._Row < 3; }
        inline Action operator*() const { return _Action; }
        void operator++();
    };

    inline ActionIterator begin() const { return ActionIterator(this); }
    inline ActionIterator end() const { return ActionIterator(nullptr); }

    inline explicit TicTacToe()
    {
        for (auto &i : _Board)
            i.fill(2);
    }

    inline unsigned int GetNextPlayer() const { return _NextPlayer; }
    inline bool IsValid(Action action) const
    {
        return action._Row < 3 && action._Col < 3 && _Board[action._Row][action._Col] == 2;
    }

    MoveResult operator()(Action action);
    friend std::ostream &operator<<(std::ostream &os, const TicTacToe &game);
};
