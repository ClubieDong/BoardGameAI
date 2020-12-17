#pragma once

#include "../TicTacToe.hpp"

namespace tictactoe
{
    class DefaultActGen
    {
    public:
        using Action = TicTacToe::Action;

    private:
        const TicTacToe *_Game;

    public:
        class ActionIterator
        {
            friend class DefaultActGen;

        private:
            const DefaultActGen *_ActGen = nullptr;
            Action _Action;
            inline explicit ActionIterator() = default;
            inline explicit ActionIterator(const DefaultActGen *actGen)
                : _ActGen(actGen) { ++*this; }

        public:
            inline bool operator!=(ActionIterator) const { return _Action.Row < 3; }
            inline Action operator*() const { return _Action; }
            void operator++();
        };

        inline explicit DefaultActGen(const TicTacToe &game) : _Game(&game) {}

        inline ActionIterator begin() const { return ActionIterator(this); }
        inline ActionIterator end() const { return ActionIterator(); }

        inline void SetGame(const TicTacToe &game) { _Game = &game; }
        inline void Notify(Action) const {}
    };
} // namespace tictactoe
