#pragma once

#include "../../../ActionGenerator.hpp"
#include "../Game.hpp"

namespace m_n_k_game::action_generator {
template <unsigned char RowCount, unsigned char ColCount, unsigned char Renju>
class Neighbor : public ActionGenerator {
private:
    unsigned char m_Range;

public:
    struct Data : public ActionGenerator::Data {
        std::bitset<RowCount * ColCount> InRange;
        std::array<unsigned char, RowCount> CountInRow = {};
        typename Game<RowCount, ColCount, Renju>::PosType TotalCount = 0;

        friend bool operator==(const Data &left, const Data &right) { return left.InRange == right.InRange; }

        virtual std::unique_ptr<ActionGenerator::Data> Clone() const override { return std::make_unique<Data>(*this); }
        virtual bool Equal(const ActionGenerator::Data &data) const override {
            return *this == static_cast<const Data &>(data);
        }
    };

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

    explicit Neighbor(const ::Game &game, unsigned char range) : ActionGenerator(game), m_Range(range) {}

    virtual std::unique_ptr<ActionGenerator::Data> CreateData(const ::Game::State &state_) const override {
        const auto &state = static_cast<const typename Game<RowCount, ColCount, Renju>::State &>(state_);
        auto data = std::make_unique<Data>();
        data->InRange[RowCount / 2 * ColCount + ColCount / 2] = true;
        data->CountInRow[RowCount / 2] = 1;
        for (typename Game<RowCount, ColCount, Renju>::Action action(0); action.Position < RowCount * ColCount;
             ++action.Position)
            if (state.GetGrid(action.Position) != 0)
                UpdateData(*data, state, action);
        return data;
    }

    virtual void UpdateData(ActionGenerator::Data &data_, const ::Game::State &state_,
                            const ::Game::Action &action_) const override {
        auto &data = static_cast<Data &>(data_);
        const auto &state = static_cast<const typename Game<RowCount, ColCount, Renju>::State &>(state_);
        const auto &action = static_cast<const typename Game<RowCount, ColCount, Renju>::Action &>(action_);
        const auto row = action.GetRow(), col = action.GetCol();
        const unsigned char rowBegin = std::max(0, row - m_Range), rowEnd = std::min(RowCount - 1, row + m_Range);
        const unsigned char colBegin = std::max(0, col - m_Range), colEnd = std::min(ColCount - 1, col + m_Range);
        for (auto rowIdx = rowBegin; rowIdx <= rowEnd; ++rowIdx) {
            ++data.CountInRow[rowIdx];
            for (auto colIdx = colBegin; colIdx <= colEnd; ++colIdx)
                if (state.GetGrid(rowIdx * ColCount + colIdx) == 0)
                    data.InRange[rowIdx * ColCount + colIdx] = true;
        }
        data.InRange[action.Position] = false;
        --data.CountInRow[row];
    }

    virtual std::unique_ptr<ActionGenerator::Iterator> FirstIterator(const ActionGenerator::Data &data,
                                                                     const ::Game::State &state) const override {
        auto iterator = std::make_unique<Iterator>(-1);
        [[maybe_unused]] const auto isValid = NextIterator(data, state, *iterator);
        assert(isValid);
        return iterator;
    }

    virtual bool NextIterator(const ActionGenerator::Data &data_, const ::Game::State &,
                              ActionGenerator::Iterator &iterator_) const override {
        const auto &data = static_cast<const Data &>(data_);
        auto &iterator = static_cast<Iterator &>(iterator_);
        ++iterator.Action.Position;
        for (auto col = iterator.Action.GetCol(); col < ColCount; ++col, ++iterator.Action.Position)
            if (data.InRange[iterator.Action.Position])
                return true;
        assert(iterator.Action.Position % ColCount == 0);
        for (auto row = iterator.Action.GetRow(); row < RowCount; ++row)
            if (data.CountInRow[row] == 0)
                iterator.Action.Position += ColCount;
            else
                for (unsigned char col = 0; col < ColCount; ++col, ++iterator.Action.Position)
                    if (data.InRange[iterator.Action.Position])
                        return true;
        return false;
    }

    virtual const ::Game::Action &GetActionFromIterator(const ActionGenerator::Data &, const ::Game::State &,
                                                        const ActionGenerator::Iterator &iterator) const override {
        return static_cast<const Iterator &>(iterator).Action;
    }
};
} // namespace m_n_k_game::action_generator
