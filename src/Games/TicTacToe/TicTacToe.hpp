#pragma once

#include <array>
#include <memory>
#include <optional>
#include <vector>
#include "../GameBase.hpp"

class TicTacToe;

namespace tictactoe
{
    class State : public StateBase
    {
        friend class ::TicTacToe;
        friend class DefaultActGen;

    protected:
        std::array<std::array<unsigned char, 3>, 3> _Board;
        unsigned char _MoveCount = 0;

    public:
        State() : StateBase(0)
        {
            for (auto &row : _Board)
                row.fill(2);
        }

        virtual GameType GetGameType() const final override { return GameType::TicTacToe; }
    };

    class Action : public ActionBase
    {
        friend class ::TicTacToe;
        friend class DefaultActGen;

    protected:
        unsigned char _Row = 0, _Col = -1;

        virtual GameType GetGameType() const final override { return GameType::TicTacToe; }

        virtual std::unique_ptr<ActionBase> Clone() const final override
        {
            return std::make_unique<Action>(*this);
        }
    };
}

class TicTacToe : public GameBase
{
public:
    TicTacToe() : GameBase(2) {}

    virtual GameType GetGameType() const final override { return GameType::TicTacToe; }

    virtual bool IsValidAction(const StateBase &state, const ActionBase &act) const final override;

    virtual std::unique_ptr<StateBase> NewState() const final override
    {
        return std::make_unique<tictactoe::State>();
    }
    virtual std::optional<std::vector<double>> Move(StateBase &state, const ActionBase &act) const final override;
};
