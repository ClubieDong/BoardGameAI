#include "DefaultActGen.hpp"

namespace tictactoe
{
    void DefaultActGen::ActionIterator::operator++()
    {
        while (true)
        {
            ++_Action.Col;
            if (_Action.Col == 3)
            {
                _Action.Col = 0;
                ++_Action.Row;
            }
            if (_Action.Row >= 3)
                break;
            if (_ActGen->_Game->GetBoard()[_Action.Row][_Action.Col] == 2)
                break;
        }
    }
} // namespace tictactoe