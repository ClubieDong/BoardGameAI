#pragma once

#include "../Utilities/Utilities.hpp"
#include "Game.hpp"
#include <cassert>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

class ActionGenerator : public Util::NonCopyableNonMoveable {
protected:
    const Game *m_Game;

public:
    struct Data {
        virtual ~Data() = default;
        virtual std::unique_ptr<Data> Clone() const { return std::make_unique<Data>(); }
        virtual bool Equal(const Data &) const { return true; }
    };

    struct Iterator {
        virtual ~Iterator() = default;
        virtual std::unique_ptr<Iterator> Clone() const = 0;
        virtual bool Equal(const Iterator &iterator) const = 0;
    };
    class IteratorWrapper;

    static std::unique_ptr<ActionGenerator> Create(const std::string &type, const Game &game,
                                                   const nlohmann::json &data);

    explicit ActionGenerator(const Game &game) : m_Game(&game) {}
    virtual ~ActionGenerator() = default;
    virtual std::string_view GetType() const = 0;

    virtual std::unique_ptr<Data> CreateData(const Game::State &) const { return std::make_unique<Data>(); }
    virtual void UpdateData(Data &, const Game::State &, const Game::Action &) const {}

    virtual std::unique_ptr<Iterator> FirstIterator(const Data &data, const Game::State &state) const = 0;
    virtual bool NextIterator(const Data &data, const Game::State &state, Iterator &iterator) const = 0;
    virtual const Game::Action &GetActionFromIterator(const Data &data, const Game::State &state,
                                                      const Iterator &iterator) const = 0;
    IteratorWrapper begin(const Data &data, const Game::State &state) const;
    IteratorWrapper end(const Data &data, const Game::State &state) const;

    // The base class provides default implementations of the following methods by using `FirstIterator`,
    // `NextIterator`, `GetActionFromIterator`, better implementations can be overridden by subclasses
    virtual std::vector<std::unique_ptr<Game::Action>> GetActionList(const Data &data, const Game::State &state) const;
    virtual std::unique_ptr<Game::Action> GetNthAction(const Data &data, const Game::State &state,
                                                       unsigned int idx) const;
    virtual std::unique_ptr<Game::Action> GetRandomAction(const Data &data, const Game::State &state) const;
};

class ActionGenerator::IteratorWrapper {
private:
    const ActionGenerator *m_ActionGenerator;
    const Data *m_ActionGeneratorData;
    const Game::State *m_State;
    // When `m_Iterator` is null, this is the past-the-end iterator
    std::unique_ptr<Iterator> m_Iterator;

public:
    explicit IteratorWrapper(const ActionGenerator &actionGenerator, const Data &actionGeneratorData,
                             const Game::State &state, std::unique_ptr<Iterator> &&iterator)
        : m_ActionGenerator(&actionGenerator), m_ActionGeneratorData(&actionGeneratorData), m_State(&state),
          m_Iterator(std::move(iterator)) {}

    friend bool operator==(const IteratorWrapper &left, const IteratorWrapper &right);
    friend bool operator!=(const IteratorWrapper &left, const IteratorWrapper &right) { return !(left == right); }
    IteratorWrapper &operator++();
    IteratorWrapper operator++(int);
    const Game::Action &operator*() const;
    const Game::Action *operator->() const { return &operator*(); }
};
