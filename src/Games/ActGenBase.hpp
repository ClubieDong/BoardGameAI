#pragma once

#include <memory>
#include <cassert>
#include "GameBase.hpp"

enum class ActGenType
{
    TicTacToe_DefaultActGen,
};

class ActGenDataBase
{
public:
    virtual ~ActGenDataBase() = default;

    virtual ActGenType GetActGenType() const = 0;
};

class ActGenBase
{
protected:
    const GameBase *_Game;
    const StateBase *_State;

public:
    virtual ~ActGenBase() = default;

    virtual GameType GetGameType() const = 0;
    virtual ActGenType GetActGenType() const = 0;

    virtual std::unique_ptr<ActGenDataBase> Start(const GameBase &game, const StateBase &state)
    {
        assert(GetGameType() == game.GetGameType());
        assert(GetGameType() == state.GetGameType());
        _Game = &game;
        _State = &state;
        return nullptr;
    }
    virtual void Update([[maybe_unused]] ActGenDataBase *data,
                        [[maybe_unused]] const ActionBase &act) const
    {
        assert(!data || GetActGenType() == data->GetActGenType());
        assert(GetGameType() == act.GetGameType());
    }

    virtual std::unique_ptr<ActionBase> NewAction() const = 0;
    virtual bool NextAction(const ActGenDataBase *data, ActionBase &act) const = 0;
};
