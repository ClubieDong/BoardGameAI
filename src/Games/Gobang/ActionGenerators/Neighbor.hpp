#pragma once

#include "../../ActionGenerator.hpp"
#include "../Game.hpp"
#include <bitset>
#include <cassert>

namespace gobang::action_generator {
namespace data {
struct Neighbor : ::ActionGenerator::Data {
    std::bitset<15 * 15> InRange;
    explicit Neighbor(const ::State &) {}
    friend bool operator==(const Neighbor &left, const Neighbor &right) { return left.InRange == right.InRange; }
};
} // namespace data

class Neighbor : public ::ActionGenerator::CRTP<data::Neighbor> {
private:
    unsigned char m_Range;

public:
    explicit Neighbor(const ::Game &, const nlohmann::json &data) : m_Range(data["range"]) {}

    virtual std::string_view GetType() const override { return "gobang/neighbor"; }
    virtual std::unique_ptr<::ActionGenerator::Data> CreateData(const ::State &state) const override;
    virtual std::unique_ptr<::Action> FirstAction(const ::ActionGenerator::Data &data,
                                                  const ::State &state) const override;
    virtual bool NextAction(const ::ActionGenerator::Data &data, const ::State &state, ::Action &action) const override;
    virtual void Update(::ActionGenerator::Data &data, const ::Action &action) const override;
};
} // namespace gobang::action_generator
