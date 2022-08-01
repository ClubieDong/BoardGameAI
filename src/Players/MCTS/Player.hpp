#pragma once

#include "../../Games/ActionGenerator.hpp"
#include "../../Games/Game.hpp"
#include "../Player.hpp"
#include <stack>
#include <vector>

namespace mcts {
class Player : public ::Player {
private:
    struct Node;
    struct TerminalNode;
    struct NewNode;
    struct UnexpandedNode;
    struct ExpandedNode;
    struct PartiallyExpandedNode;
    struct FullyExpandedNode;

    std::unique_ptr<Node> &Select(std::unique_ptr<Node> &root, std::stack<ExpandedNode *> &path) const;
    Node &Expand(std::unique_ptr<Player::Node> &node, std::stack<ExpandedNode *> &path) const;
    std::vector<float> Rollout(const Node &node, const std::stack<ExpandedNode *> &path) const;
    void BackPropagate(Node *node, std::stack<ExpandedNode *> &path, const std::vector<float> &result) const;
    std::unique_ptr<Action> ChooseBestAction(const Node &root) const;

    const Game *m_Game;
    const State *m_State;
    std::unique_ptr<ActionGenerator> m_ActionGenerator;
    std::unique_ptr<ActionGenerator::Data> m_ActionGeneratorData;
    unsigned int m_Iterations;
    double m_ExplorationFactor;
    std::vector<std::vector<double>> m_GoalMatrix;
    std::string m_RolloutPolicyType;
    nlohmann::json m_RolloutPolicyData;

public:
    explicit Player(const Game &game, const State &state, const nlohmann::json &data);

    virtual std::string_view GetType() const override { return "mcts"; }
    virtual std::unique_ptr<Action> GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) override;
    virtual void Update(const Action &action) override { m_ActionGenerator->Update(*m_ActionGeneratorData, action); }
};
} // namespace mcts
