#include "ActionGenerator.hpp"
#include "Gobang/ActionGenerators/Default.hpp"
#include "Gobang/ActionGenerators/Neighbor.hpp"
#include "TicTacToe/ActionGenerators/Default.hpp"
#include <unordered_map>

template <typename T>
static std::unique_ptr<ActionGenerator> CreateActionGenerator(const Game &game, const nlohmann::json &data) {
    return std::make_unique<T>(game, data);
}

using ActionGeneartorCreatorFunc = std::unique_ptr<ActionGenerator> (*)(const Game &, const nlohmann::json &);
static const std::unordered_map<std::string, ActionGeneartorCreatorFunc> ActionGeneratorCreatorMap = {
    {"tic_tac_toe/default", CreateActionGenerator<tic_tac_toe::action_generator::Default>},
    {"gobang/default", CreateActionGenerator<gobang::action_generator::Default>},
    {"gobang/neighbor", CreateActionGenerator<gobang::action_generator::Neighbor>},
};

std::unique_ptr<ActionGenerator> ActionGenerator::Create(const std::string &type, const Game &game,
                                                         const nlohmann::json &data) {
    const auto actionGeneratorType = std::string(game.GetType()) + '/' + type;
    Util::GetJsonValidator("action_generators/" + actionGeneratorType + ".schema.json").validate(data);
    const auto creator = ActionGeneratorCreatorMap.at(actionGeneratorType);
    return creator(game, data);
}

std::vector<std::unique_ptr<Action>> ActionGenerator::GetActionList(const Data &data, const State &state,
                                                                    const Game &game) const {
    std::vector<std::unique_ptr<Action>> actionList;
    ForEachAction(data, state, [&](const Action &action) { actionList.push_back(game.CloneAction(action)); });
    assert(actionList.size() > 0);
    return actionList;
}

std::unique_ptr<Action> ActionGenerator::GetNthAction(const Data &data, const State &state, unsigned int idx) const {
    auto action = FirstAction(data, state);
    assert(action);
    while (idx--) {
        [[maybe_unused]] const auto isValid = NextAction(data, state, *action);
        assert(isValid);
    }
    return action;
}
