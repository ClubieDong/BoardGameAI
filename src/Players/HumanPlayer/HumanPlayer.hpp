#pragma once

#include <iostream>
#include <vector>

template <typename Game>
class HumanPlayer
{
public:
    using GameType = Game;
    using Action = typename Game::Action;

private:
    const Game *_Game;
    const std::vector<Action> *_HistoryActions;

public:
    inline explicit HumanPlayer(const Game &game, const std::vector<Action> &historyActions)
        : _Game(&game), _HistoryActions(&historyActions) {}
        
    HumanPlayer(const HumanPlayer&) = delete;
    HumanPlayer& operator=(const HumanPlayer&) = delete;
    HumanPlayer(HumanPlayer&&) = default;
    HumanPlayer& operator=(HumanPlayer&&) = default;
    
    Action operator()() const
    {
        bool first = true;
        Action action;
        do
        {
            std::ignore = system("clear");
            std::cout << *_Game;
            if (!first)
                std::cout << ">>> Invalid move <<<";
            else if (!_HistoryActions->empty())
                std::cout << "Opponent's move: " << _HistoryActions->back();
            std::cout << "\nYour move: ";
            std::cin >> action;
            first = false;
        } while (!_Game->IsValid(action));
        return action;
    }
};
