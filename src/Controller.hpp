#pragma once

#include <memory>
#include <vector>
#include <cassert>
#include "Games/GameBase.hpp"
#include "Players/PlayerBase.hpp"

class Controller
{
private:
    std::unique_ptr<GameBase> _Game;
    std::vector<std::unique_ptr<PlayerBase>> _Players;

public:
    explicit Controller(std::unique_ptr<GameBase> &&game,
                        std::vector<std::unique_ptr<PlayerBase>> &&players)
        : _Game(std::move(game)), _Players(std::move(players))
    {
        assert(_Game);
        for ([[maybe_unused]] const auto &player : _Players)
            assert(player);
    }

    std::vector<double> Start();
};
