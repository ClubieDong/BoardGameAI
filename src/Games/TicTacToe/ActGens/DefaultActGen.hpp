#pragma once

#include <memory>
#include "../../ActGenBase.hpp"
#include "../../GameBase.hpp"
#include "../TicTacToe.hpp"

namespace tictactoe
{
    class DefaultActGen : public ActGenBase
    {
    public:
        virtual GameType GetGameType() const final override { return GameType::TicTacToe; }
        virtual ActGenType GetActGenType() const final override { return ActGenType::TicTacToe_DefaultActGen; }
        
        virtual std::unique_ptr<ActionBase> NewAction() const final override
        {
            return std::make_unique<Action>();
        }
        virtual bool NextAction(const ActGenDataBase *data, ActionBase &act) const final override;
    };
}
