#pragma once

#include <memory>
#include "../PlayerBase.hpp"

class RandomPlayer : public PlayerBase
{
public:
    // TODO: Check this
    using PlayerBase::PlayerBase;

    virtual std::unique_ptr<ActionBase> Move() final override;
};
