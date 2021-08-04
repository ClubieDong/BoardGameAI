#include "TicTacToe.hpp"

bool TicTacToe::IsValidAction(const StateBase &_state, const ActionBase &_act) const
{
    // Assert and convert polymorphic types
    assert(_state.GetGameType() == GameType::TicTacToe);
    assert(_act.GetGameType() == GameType::TicTacToe);
    auto &state = reinterpret_cast<const tictactoe::State &>(_state);
    auto &act = reinterpret_cast<const tictactoe::Action &>(_act);

    return act._Row < 3 && act._Col < 3 && state._Board[act._Row][act._Col] == 2;
}

std::optional<std::vector<double>> TicTacToe::Move(StateBase &_state, const ActionBase &_act) const
{
    // Assert and convert polymorphic types
    assert(_state.GetGameType() == GameType::TicTacToe);
    assert(_act.GetGameType() == GameType::TicTacToe);
    auto &state = reinterpret_cast<tictactoe::State &>(_state);
    auto &act = reinterpret_cast<const tictactoe::Action &>(_act);

    // Perform the move
    assert(IsValidAction(state, act));
    state._Board[act._Row][act._Col] = state._NextPlayer;
    ++state._MoveCount;

    // Row
    bool win = state._Board[act._Row][0] == state._NextPlayer &&
               state._Board[act._Row][1] == state._NextPlayer &&
               state._Board[act._Row][2] == state._NextPlayer;
    // Col
    win = win || (state._Board[0][act._Col] == state._NextPlayer &&
                  state._Board[1][act._Col] == state._NextPlayer &&
                  state._Board[2][act._Col] == state._NextPlayer);
    // Main diagonal
    win = win || (act._Row == act._Col &&
                  state._Board[0][0] == state._NextPlayer &&
                  state._Board[1][1] == state._NextPlayer &&
                  state._Board[2][2] == state._NextPlayer);
    // Counter diagonal
    win = win || (act._Row + act._Col == 2 &&
                  state._Board[0][2] == state._NextPlayer &&
                  state._Board[1][1] == state._NextPlayer &&
                  state._Board[2][0] == state._NextPlayer);
    if (win)
    {
        std::optional<std::vector<double>> res(std::in_place, 2, 0.0);
        (*res)[state._NextPlayer] = 1.0;
        return res;
    }
    // Draw
    else if (state._MoveCount == 9)
        return std::optional<std::vector<double>>(std::in_place, 2, 0.5);
        
    state._NextPlayer = !state._NextPlayer;
    return std::nullopt;
}
