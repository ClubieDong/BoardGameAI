#include "DefaultActGen.hpp"
#include <cassert>

namespace tictactoe
{
    bool DefaultActGen::NextAction(const ActGenDataBase *, ActionBase &_act) const
    {
        // Assert and convert polymorphic types
        assert(_act.GetGameType() == GameType::TicTacToe);
        auto &act = dynamic_cast<Action &>(_act);
        auto &state = *dynamic_cast<const State *>(_State);

        do
        {
            ++act._Col;
            if (act._Col >= 3)
            {
                ++act._Row;
                act._Col = 0;
            }
            if (act._Row >= 3)
                return false;
        } while (state._Board[act._Row][act._Col] != 2);
        return true;
    }
}
