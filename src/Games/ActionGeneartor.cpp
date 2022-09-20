#include "ActionGenerator.hpp"
#include "Gomoku/ActionGenerators/Default.hpp"
#include "Gomoku/ActionGenerators/Neighbor.hpp"
#include "TicTacToe/ActionGenerators/Default.hpp"
#include <unordered_map>

template <typename T>
static std::unique_ptr<ActionGenerator> CreateActionGenerator(const Game &game, const nlohmann::json &data) {
    return std::make_unique<T>(game, data);
}

using ActionGeneartorCreatorFunc = std::unique_ptr<ActionGenerator> (*)(const Game &, const nlohmann::json &);
static const std::unordered_map<std::string, ActionGeneartorCreatorFunc> ActionGeneratorCreatorMap = {
    {"tic_tac_toe/default", CreateActionGenerator<tic_tac_toe::action_generator::Default>},
    {"gomoku/default", CreateActionGenerator<gomoku::action_generator::Default>},
    {"gomoku/neighbor", CreateActionGenerator<gomoku::action_generator::Neighbor>},
};

std::unique_ptr<ActionGenerator> ActionGenerator::Create(const std::string &type, const Game &game,
                                                         const nlohmann::json &data) {
    const auto actionGeneratorType = std::string(game.GetType()) + '/' + type;
    Util::GetJsonValidator("action_generators/" + actionGeneratorType + ".schema.json").validate(data);
    const auto creator = ActionGeneratorCreatorMap.at(actionGeneratorType);
    return creator(game, data);
}

ActionGenerator::IteratorWrapper ActionGenerator::begin(const Data &data, const Game::State &state) const {
    return IteratorWrapper(*this, data, state, FirstIterator(data, state));
}

ActionGenerator::IteratorWrapper ActionGenerator::end(const Data &data, const Game::State &state) const {
    return IteratorWrapper(*this, data, state, nullptr);
}

std::vector<std::unique_ptr<Game::Action>> ActionGenerator::GetActionList(const Data &data,
                                                                          const Game::State &state) const {
    std::vector<std::unique_ptr<Game::Action>> actionList;
    std::for_each(begin(data, state), end(data, state),
                  [&](const Game::Action &action) { actionList.push_back(action.Clone()); });
    return actionList;
}

std::unique_ptr<Game::Action> ActionGenerator::GetNthAction(const Data &data, const Game::State &state,
                                                            unsigned int idx) const {
    auto iter = begin(data, state);
    while (idx--)
        ++iter;
    return iter->Clone();
}

std::unique_ptr<Game::Action> ActionGenerator::GetRandomAction(const Data &data, const Game::State &state) const {
    // TODO: Comment
    unsigned int count = 0;
    std::unique_ptr<Game::Action> chosenAction;
    auto &engine = Util::GetRandomEngine();
    std::for_each(begin(data, state), end(data, state), [&](const Game::Action &action) {
        std::uniform_int_distribution<unsigned int> random(0, count++);
        if (random(engine) == 0)
            chosenAction = action.Clone();
    });
    return chosenAction;
}

bool operator==(const ActionGenerator::IteratorWrapper &left, const ActionGenerator::IteratorWrapper &right) {
    assert(left.m_ActionGenerator == right.m_ActionGenerator);
    assert(left.m_ActionGeneratorData == right.m_ActionGeneratorData);
    assert(left.m_State == right.m_State);
    if (left.m_Iterator == right.m_Iterator)
        return true;
    if (!left.m_Iterator || !right.m_Iterator)
        return false;
    return left.m_Iterator->Equal(*right.m_Iterator);
}

ActionGenerator::IteratorWrapper &ActionGenerator::IteratorWrapper::operator++() {
    assert(m_Iterator);
    if (!m_ActionGenerator->NextIterator(*m_ActionGeneratorData, *m_State, *m_Iterator))
        m_Iterator = nullptr;
    return *this;
}

ActionGenerator::IteratorWrapper ActionGenerator::IteratorWrapper::operator++(int) {
    assert(m_Iterator);
    auto clonedIter = m_Iterator->Clone();
    ++*this;
    return IteratorWrapper(*m_ActionGenerator, *m_ActionGeneratorData, *m_State, std::move(clonedIter));
}

const Game::Action &ActionGenerator::IteratorWrapper::operator*() const {
    assert(m_Iterator);
    return m_ActionGenerator->GetActionFromIterator(*m_ActionGeneratorData, *m_State, *m_Iterator);
}
