#include "TicTacToe.hpp"
#include <string>
#include <cctype>
#include <cassert>

std::istream &operator>>(std::istream &is, TicTacToe::Action &action)
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

void TicTacToe::operator()(Action action)
{
    assert(IsValid(action));
    _Board[action.Row][action.Col] = _NextPlayer;
    ++_MoveCount;
    // Row
    bool win = _Board[action.Row][0] == _NextPlayer &&
               _Board[action.Row][1] == _NextPlayer &&
               _Board[action.Row][2] == _NextPlayer;
    // Col
    win = win || (_Board[0][action.Col] == _NextPlayer &&
                  _Board[1][action.Col] == _NextPlayer &&
                  _Board[2][action.Col] == _NextPlayer);
    // Main diagonal
    win = win || (action.Row == action.Col &&
                  _Board[0][0] == _NextPlayer &&
                  _Board[1][1] == _NextPlayer &&
                  _Board[2][2] == _NextPlayer);
    // Counter diagonal
    win = win || (action.Row + action.Col == 2 &&
                  _Board[0][2] == _NextPlayer &&
                  _Board[1][1] == _NextPlayer &&
                  _Board[2][0] == _NextPlayer);
    if (win)
    {
        _Result.emplace();
        (*_Result)[_NextPlayer] = 1;
        (*_Result)[!_NextPlayer] = 0;
    }
    // Draw
    else if (_MoveCount == 9)
    {
        _Result.emplace();
        (*_Result)[0] = (*_Result)[1] = 0.5;
    }
    _NextPlayer = !_NextPlayer;
}

std::ostream &operator<<(std::ostream &os, const TicTacToe &game)
{
    os << "  ";
    for (char i = 'A'; i < 'A' + 3; ++i)
        os << i << ' ';
    os << '\n';
    for (unsigned char i = 0; i < 3; ++i)
    {
        os << i + 1 << ' ';
        for (unsigned char j = 0; j < 3; ++j)
            if (game._Board[i][j] == 0)
                // os << "○ ";
                os << "* ";
            else if (game._Board[i][j] == 1)
                // os << "● ";
                os << "@ ";
            else
                os << "  ";
        os << i + 1 << '\n';
    }
    os << "  ";
    for (char i = 'A'; i < 'A' + 3; ++i)
        os << i << ' ';
    os << '\n';
    return os;
}
