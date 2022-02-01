#include "../Utilities/Hash.hpp"
#include "ActionGenerator.hpp"
#include "TicTacToe/ActionGenerators/Default.hpp"
#include "TicTacToe/Game.hpp"
#include <typeindex>
#include <unordered_map>

template <typename T>
static std::unique_ptr<ActionGenerator::Data> CreateActionGeneratorData() {
    return std::make_unique<T>();
}

template <typename T>
static std::unique_ptr<ActionGenerator> CreateActionGenerator(const Game &game, const State &state,
                                                              const nlohmann::json &data) {
    return std::make_unique<T>(game, state, data);
}

using ActionGeneratorDataCreatorFunc = std::unique_ptr<ActionGenerator::Data> (*)();
static const std::unordered_map<std::type_index, ActionGeneratorDataCreatorFunc> ActionGeneratorDataCreatorMap = {
    {typeid(tic_tac_toe::action_generator::Default),
     CreateActionGeneratorData<tic_tac_toe::action_generator::Default::Data>},
};

using ActionGeneartorCreatorFunc = std::unique_ptr<ActionGenerator> (*)(const Game &, const State &,
                                                                        const nlohmann::json &);
static const std::unordered_map<std::pair<std::type_index, std::string>, ActionGeneartorCreatorFunc>
    ActionGeneratorCreatorMap = {
        {{typeid(tic_tac_toe::Game), "default"}, CreateActionGenerator<tic_tac_toe::action_generator::Default>},
};

std::unique_ptr<ActionGenerator::Data> ActionGenerator::Data::Create(const ActionGenerator &actionGenerator) {
    const std::type_index &type = typeid(actionGenerator);
    const auto creator = ActionGeneratorDataCreatorMap.at(type);
    return creator();
}

std::unique_ptr<ActionGenerator> ActionGenerator::Create(const std::string &type, const Game &game, const State &state,
                                                         const nlohmann::json &data) {
    const auto creator = ActionGeneratorCreatorMap.at({typeid(game), type});
    return creator(game, state, data);
}
