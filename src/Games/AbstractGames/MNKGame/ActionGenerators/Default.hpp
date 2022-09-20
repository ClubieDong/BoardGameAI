#pragma once

#include "../../../ActionGenerator.hpp"
#include "../Game.hpp"

namespace m_n_k_game::action_generator {
template <unsigned char RowCount, unsigned char ColCount, unsigned char Renju>
class Default : public ActionGenerator {
public:
    struct Iterator : public ActionGenerator::Iterator {
        typename Game<RowCount, ColCount, Renju>::Action Action;

        explicit Iterator(typename Game<RowCount, ColCount, Renju>::PosType position) : Action(position) {}

        friend bool operator==(const Iterator &left, const Iterator &right) { return left.Action == right.Action; }

        virtual std::unique_ptr<ActionGenerator::Iterator> Clone() const override {
            return std::make_unique<Iterator>(*this);
        }
        virtual bool Equal(const ActionGenerator::Iterator &iterator) const override {
            return *this == static_cast<const Iterator &>(iterator);
        }
    };

    explicit Default(const ::Game &game) : ActionGenerator(game) {}

    virtual std::unique_ptr<ActionGenerator::Iterator> FirstIterator(const ActionGenerator::Data &data,
                                                                     const ::Game::State &state) const override {
        auto iterator = std::make_unique<Iterator>(-1);
        [[maybe_unused]] const auto isValid = NextIterator(data, state, *iterator);
        assert(isValid);
        return iterator;
    }

    virtual bool NextIterator(const ActionGenerator::Data &, const ::Game::State &state_,
                              ActionGenerator::Iterator &iterator_) const override {
        const auto &state = static_cast<const typename Game<RowCount, ColCount, Renju>::State &>(state_);
        auto &iterator = static_cast<Iterator &>(iterator_);
        for (++iterator.Action.Position; iterator.Action.Position < RowCount * ColCount; ++iterator.Action.Position)
            if (state.GetGrid(iterator.Action.Position) == 0)
                return true;
        return false;
    }

    virtual const ::Game::Action &GetActionFromIterator(const ActionGenerator::Data &, const ::Game::State &,
                                                        const ActionGenerator::Iterator &iterator) const override {
        return static_cast<const Iterator &>(iterator).Action;
    }
};
} // namespace m_n_k_game::action_generator
