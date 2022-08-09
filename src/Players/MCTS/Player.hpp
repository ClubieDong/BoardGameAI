#pragma once

#include "../../Games/ActionGenerator.hpp"
#include "../../Games/Game.hpp"
#include "../Player.hpp"
#include <stack>
#include <vector>

namespace mcts {
class Player : public ::Player {
private:
    // The explanation of the following `Node`s is detailed in the corresponding cpp file
    struct Node;
    struct TerminalNode;
    struct NewNode;
    struct UnexpandedNode;
    struct ExpandedNode;
    struct PartiallyExpandedNode;
    struct FullyExpandedNode;

    // Facilities for synchronizing threads
    enum class Signal;
    struct ThreadData;

    // Objects related to the game
    const Game *m_Game;
    const State *m_State;
    std::unique_ptr<ActionGenerator> m_ActionGenerator;
    std::unique_ptr<ActionGenerator::Data> m_ActionGeneratorData;

    // Configurations of the MCTS algorithm, see `schema/players/mcts.schema.json` for details
    double m_ExplorationFactor;
    std::vector<std::vector<double>> m_GoalMatrix;
    std::string m_RolloutPolicyType;
    nlohmann::json m_RolloutPolicyData;
    bool m_Parallel;
    unsigned int m_Iterations = 0;
    unsigned int m_Workers = 0;

    // The following fields are only used for the parallel MCTS algorithm
    std::vector<std::unique_ptr<ThreadData>> m_ThreadList;
    // Actions available in the current state. When `Update` is called, `m_State` has changed, and no action information
    // is stored in the root node of the game tree, so this is needed to calculate the action index during `Prune`
    std::vector<std::unique_ptr<Action>> m_ActionList;
    // Used to tell the worker threads which action was taken during `Prune`. If `m_PruneActionIndex` is out of bounds,
    // it means that the opponent took an action that we did not consider.
    unsigned int m_PruneActionIndex;

    // Traverse the tree and select a leaf node, or a partially expanded node
    std::unique_ptr<Node> &Select(std::unique_ptr<Node> &root, std::stack<ExpandedNode *> &path) const;
    // Expand the node if needed, return a node that is never visited
    Node &Expand(std::unique_ptr<Player::Node> &node, std::stack<ExpandedNode *> &path) const;
    // Rollout at the given node to estimate the value of the node
    std::vector<float> Rollout(const Node &node, const std::stack<ExpandedNode *> &path) const;
    // Update `Score` and `RolloutCount` along the path
    void BackPropagate(Node *node, std::stack<ExpandedNode *> &path, const std::vector<float> &result) const;
    // Create a new root node with the current state and action generator data
    std::unique_ptr<Node> CreateRootNode() const;
    // Call `Select`, `Expand`, `Rollout`, and `BackPropagate`
    void RunSingleIteration(std::unique_ptr<Node> &root, std::stack<ExpandedNode *> &path) const;
    // Choose the most visited action, given the root node. Used for the sequential MCTS algorithm
    std::unique_ptr<Action> ChooseBestActionSequential(const Node &root) const;

    // The following methods are only used for the parallel MCTS algorithm
    // Copy the visited count and score of each child of the given root node into the `ThreadData`
    void ReportData(ThreadData &data, const Node &root, bool includeScore) const;
    // Prune the game tree based on the action taken
    void Prune(std::unique_ptr<Node> &root) const;
    // Choose the most visited action. Used for the parallel MCTS algorithm
    std::unique_ptr<Action> ChooseBestActionParallel() const;
    // Main function for worker threads
    void ThreadMain(ThreadData *data);
    // Send a signal to worker threads and wait for all of them to reply
    void SendSignal(Signal signal) const;

public:
    explicit Player(const Game &game, const State &state, const nlohmann::json &data);
    ~Player();

    virtual std::string_view GetType() const override { return "mcts"; }
    virtual void StartThinking();
    virtual void StopThinking();
    virtual std::unique_ptr<Action> GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) override;
    virtual void Update(const Action &action) override;
    virtual nlohmann::json QueryDetails(const nlohmann::json &data) override;
};
} // namespace mcts
