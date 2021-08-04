#include "Controller.hpp"

std::vector<double> Controller::Start()
{
    auto state = _Game->NewState();
    assert(state);
    for (auto &player : _Players)
        player->Start(*_Game, *state);
    while (true)
    {
        auto playerIdx = state->GetNextPlayer();
        assert(playerIdx < _Game->PlayerCount);
        auto action = _Players[playerIdx]->Move();
        assert(action);
        auto result = _Game->Move(*state, *action);
        for (auto &player : _Players)
            player->Update(*action);
        if (result)
            return *result;
    }
}
