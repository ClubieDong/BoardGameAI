#pragma once

#include <memory>
#include <cassert>
#include "../Games/ActGenBase.hpp"
#include "../Games/GameBase.hpp"

class PlayerBase
{
public:
    std::unique_ptr<ActGenBase> ActGen;

protected:
    const GameBase *_Game;
    const StateBase *_State;
    std::unique_ptr<ActGenDataBase> _ActGenData;

public:
    explicit PlayerBase(std::unique_ptr<ActGenBase> &&actGen) : ActGen(std::move(actGen))
    {
        assert(ActGen);
    }

    virtual ~PlayerBase() = default;

    virtual void Start(const GameBase &game, const StateBase &state)
    {
        assert(game.GetGameType() == state.GetGameType());
        _Game = &game;
        _State = &state;
        _ActGenData = ActGen->Start(game, state);
    }

    virtual std::unique_ptr<ActionBase> Move() = 0;

    virtual void Update(const ActionBase &act)
    {
        ActGen->Update(_ActGenData.get(), act);
    }
};
