#pragma once

#include <array>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>
#include "../Game.hpp"
#include "../../Utilities/Utilities.hpp"

namespace tic_tac_toe
{
    class State : public ::State
    {
    public:
        unsigned char MoveCount;
        std::array<std::array<unsigned char, 3>, 3> Board;

        State(const ::Game &) : MoveCount(0), Board() {}
        explicit State(const nlohmann::json &data);
        virtual nlohmann::json GetJson() const override { return {{"board", Board}}; }
    };

    class Action : public ::Action
    {
    public:
        unsigned char Row, Col;

        explicit Action(unsigned char row, unsigned char col) : Row(row), Col(col) {}
        explicit Action(const nlohmann::json &data);
        virtual nlohmann::json GetJson() const override { return {{"row", Row}, {"col", Col}}; }
    };

    class Game : public ::Game
    {
    public:
        explicit Game(const nlohmann::json &data);
        virtual bool IsValidAction(const ::State &_state, const ::Action &_action) const override
        {
            const auto &state = static_cast<const State &>(_state);
            const auto &action = static_cast<const Action &>(_action);
            return state.Board[action.Row][action.Col] == 0;
        }
        virtual std::optional<std::vector<double>> TakeAction(::State &_state, const ::Action &_action) const override;
    };
}
