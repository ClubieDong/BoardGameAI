#pragma once

#include <functional>
#include <memory>
#include <utility>

namespace std {
template <typename T1, typename T2>
struct hash<std::pair<T1, T2>> {
    size_t operator()(const std::pair<T1, T2> &obj) const {
        return std::hash<T1>()(obj.first) ^ std::hash<T2>()(obj.second);
    }
};
} // namespace std

class NonCopyableNonMoveable {
public:
    NonCopyableNonMoveable() = default;
    NonCopyableNonMoveable(const NonCopyableNonMoveable &) = delete;
    NonCopyableNonMoveable(NonCopyableNonMoveable &&) = delete;
    NonCopyableNonMoveable &operator=(const NonCopyableNonMoveable &) = delete;
    NonCopyableNonMoveable &operator=(NonCopyableNonMoveable &&) = delete;
};

// Example:
//   class Base : public ClonableEqualable<Base> {};
//   class Derived : public Base::CRTP<Derived> {
//   public:
//       friend bool operator==(const Derived &, const Derived &);
//   };
template <typename TBase>
class ClonableEqualable {
public:
    virtual ~ClonableEqualable() = default;
    virtual std::unique_ptr<TBase> Clone() const = 0;
    virtual bool Equal(const TBase &obj) const = 0;

    template <typename TDerived>
    class CRTP : public TBase {
    public:
        virtual std::unique_ptr<TBase> Clone() const override {
            return std::make_unique<TDerived>(static_cast<const TDerived &>(*this));
        }
        virtual bool Equal(const TBase &obj) const override {
            return static_cast<const TDerived &>(*this) == static_cast<const TDerived &>(obj);
        }
    };
};
