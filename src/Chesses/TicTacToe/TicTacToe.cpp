#include "TicTacToe.hpp"
#include <string>
#include <cassert>
#include <cctype>

std::istream &operator>>(std::istream &is, TicTacToe::Action &action)
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

void TicTacToe::ActionIterator::operator++()
{
    while (true)
    {
        ++_Action._Col;
        if (_Action._Col == 3)
        {
            _Action._Col = 0;
            ++_Action._Row;
        }
        if (_Action._Row >= 3)
            break;
        if (_Game->_Board[_Action._Row][_Action._Col] == 2)
            break;
    }
}

void TicTacToe::operator()(Action action)
{
    assert(IsValid(action));
    _Board[action._Row][action._Col] = _NextPlayer;
    ++_MoveCount;
    // Row
    bool win = _Board[action._Row][0] == _NextPlayer &&
               _Board[action._Row][1] == _NextPlayer &&
               _Board[action._Row][2] == _NextPlayer;
    // Col
    win = win || (_Board[0][action._Col] == _NextPlayer &&
                  _Board[1][action._Col] == _NextPlayer &&
                  _Board[2][action._Col] == _NextPlayer);
    // Main diagonal
    win = win || (action._Row == action._Col &&
                  _Board[0][0] == _NextPlayer &&
                  _Board[1][1] == _NextPlayer &&
                  _Board[2][2] == _NextPlayer);
    // Counter diagonal
    win = win || (action._Row + action._Col == 2 &&
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
                os << "○ ";
            else if (game._Board[i][j] == 1)
                os << "● ";
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
