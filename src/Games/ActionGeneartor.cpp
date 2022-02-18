#include "ActionGenerator.hpp"
#include "Gobang/ActionGenerators/Default.hpp"
#include "TicTacToe/ActionGenerators/Default.hpp"
#include <unordered_map>

template <typename T>
static std::unique_ptr<ActionGenerator> CreateActionGenerator(const Game &game, const State &state,
                                                              const nlohmann::json &data) {
    return std::make_unique<T>(game, state, data);
}

using ActionGeneartorCreatorFunc = std::unique_ptr<ActionGenerator> (*)(const Game &, const State &,
                                                                        const nlohmann::json &);
static const std::unordered_map<std::string, ActionGeneartorCreatorFunc> ActionGeneratorCreatorMap = {
    {"tic_tac_toe/default", CreateActionGenerator<tic_tac_toe::action_generator::Default>},
    {"gobang/default", CreateActionGenerator<gobang::action_generator::Default>},
};

std::unique_ptr<ActionGenerator> ActionGenerator::Create(const std::string &type, const Game &game, const State &state,
                                                         const nlohmann::json &data) {
    const auto &actionGeneratorType = std::string(game.GetType()) + '/' + type;
    Util::GetJsonValidator("action_generators/" + actionGeneratorType + ".schema.json").validate(data);
    const auto creator = ActionGeneratorCreatorMap.at(actionGeneratorType);
    return creator(game, state, data);
}
