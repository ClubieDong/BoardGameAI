#include "Player.hpp"
#include "../../Games/ActionGenerator.hpp"
#include "../../Games/Game.hpp"
#include <cmath>
#include <limits>
#include <numeric>
#include <typeinfo>

namespace mcts {
// There are 5 different node types:
//   TerminalNode:
//     The corresponding state is a terminal state.
//   NewNode:
//     This is the most numerous node type. The `NewNode`s have only been visited once (or are about to be visited for
//     the first time). To save memory, it only stores `Action` that transitions the parent state to the current state,
//     instead of storing the state itself.
//   UnexpandedNode:
//     This is basically the same as `NewNode`, the only difference is that `UnexpandedNode` stores the state while
//     `NewNode` stores `Action`. This is useful when the parent node does not store state.
//   PartiallyExpandedNode:
//     When expanding a node, to save memory, instead of expanding all child nodes at once, we create one child node per
//     visit. `PartiallyExpandedNode` uses `NextAction` to save which state to expand on the next visit.
//   FullyExpandedNode:
//     When all child nodes of `PartiallyExpandedNode` are created, `PartiallyExpandedNode` is turned into a
//     `FullyExpandedNode`. To save memory, `FullyExpandedNode` does not store the state because state is not used
//     anymore, but stores `NextPlayer`, which is used during backpropagation. When `PartiallyExpandedNode` is turned
//     into `FullyExpandedNode`, all `NewNode`s in its child nodes should be turned into `UnexpandedNode` due to the
//     release of the state in their parent state.

// The inheritance diagram of different node types:
//                         [Node(abstract)]
//                                |
//         ------------------------------------------------
//         |            |            |                    |
//   [TerminalNode] [NewNode] [UnexpandedNode] [ExpandedNode(abstract)]
//                                                        |
//                                             ------------------------
//                                             |                      |
//                                  [PartiallyExpandedNode]  [FullyExpandedNode]

// This is how it transitions between different node types
//                     | (created when `PartiallyExpandedNode` is visited)
//             -----------------
//             |               |
//             V               V
//       [TerminalNode]    [NewNode]-------------
//                             |                | (parent turned into `FullyExpandedNode`)
//   (visited the second time) |                V
//                             |         [UnexpandedNode]
//                             V                | (visited the second time)
//                  [PartiallyExpandedNode] <----
//                             | (all child node created)
//                             V
//                    [FullyExpandedNode]

// To offer an insight, here are the numbers of each node type, and the time spent on each step of the algorithm after
// performing 100,000 iterations on Gobang with neighbor action generator (range=1):
//   Node count:
//     TerminalNode:               0
//     NewNode:               65,877
//     UnexpandedNode:        15,409
//     PartiallyExpandedNode: 16,625
//     FullyExpandedNode:      2,089
//   Time spent:
//     Select:           142ms
//     Expand:            81ms
//     Rollout:       10,346ms
//     BackPropagate:     52ms

struct Player::Node {
    float Score;
    uint32_t RolloutCount;

    explicit Node(float score, uint32_t rolloutCount) : Score(score), RolloutCount(rolloutCount) {}
    virtual ~Node() = default;
};

struct Player::TerminalNode : public Node {
    std::vector<float> Result;

    explicit TerminalNode(std::vector<float> &&result) : Node(0.0f, 0), Result(std::move(result)) {}
};

struct Player::NewNode : public Node {
    std::unique_ptr<struct Action> Action;

    explicit NewNode(std::unique_ptr<struct Action> &&action) : Node(0.0f, 0), Action(std::move(action)) {}
};

struct Player::UnexpandedNode : public Node {
    std::unique_ptr<struct State> State;
    std::unique_ptr<ActionGenerator::Data> ActionGeneratorData;

    explicit UnexpandedNode(float score, uint32_t rolloutCount, std::unique_ptr<struct State> &&state,
                            std::unique_ptr<ActionGenerator::Data> &&actionGeneratorData)
        : Node(score, rolloutCount), State(std::move(state)), ActionGeneratorData(std::move(actionGeneratorData)) {}
};

struct Player::ExpandedNode : public Node {
    std::vector<std::unique_ptr<Node>> Children;

    explicit ExpandedNode(float score, uint32_t rolloutCount, std::vector<std::unique_ptr<Node>> &&children)
        : Node(score, rolloutCount), Children(std::move(children)) {}
};

struct Player::PartiallyExpandedNode : public ExpandedNode {
    std::unique_ptr<struct State> State;
    std::unique_ptr<ActionGenerator::Data> ActionGeneratorData;
    std::unique_ptr<Action> NextAction;

    explicit PartiallyExpandedNode(float score, uint32_t rolloutCount, std::unique_ptr<struct State> &&state,
                                   std::unique_ptr<ActionGenerator::Data> &&actionGeneratorData,
                                   std::unique_ptr<Action> &&nextAction)
        : ExpandedNode(score, rolloutCount, {}), State(std::move(state)),
          ActionGeneratorData(std::move(actionGeneratorData)), NextAction(std::move(nextAction)) {}
};

struct Player::FullyExpandedNode : public ExpandedNode {
    uint8_t NextPlayer;

    explicit FullyExpandedNode(float score, uint32_t rolloutCount, std::vector<std::unique_ptr<Node>> &&children,
                               uint8_t nextPlayer)
        : ExpandedNode(score, rolloutCount, std::move(children)), NextPlayer(nextPlayer) {}
};

// Traverse the tree and select a leaf node, or a partially expanded node
std::unique_ptr<Player::Node> &Player::Select(std::unique_ptr<Node> &root, std::stack<ExpandedNode *> &path) const {
    assert(root);
    assert(path.empty());
    auto node = &root;
    while (typeid(**node) == typeid(FullyExpandedNode)) {
        auto &fullExpNode = static_cast<FullyExpandedNode &>(**node);
        path.push(&fullExpNode);
        // Select the child node with the largest UCB
        auto maxUCB = std::numeric_limits<double>::lowest();
        std::unique_ptr<Node> *maxNode = nullptr;
        for (auto &childNode : fullExpNode.Children) {
            // Every child of the fully expanded nodes has been visited at least once
            assert(childNode->RolloutCount > 0);
            const auto ucb = childNode->Score + m_ExplorationFactor * std::sqrt(2 * std::log(fullExpNode.RolloutCount) /
                                                                                childNode->RolloutCount);
            if (ucb > maxUCB) {
                maxUCB = ucb;
                maxNode = &childNode;
            }
        }
        assert(maxNode);
        node = maxNode;
    }
    assert(*node);
    return *node;
}

// Expand the node if needed, return a node that is never visited
Player::Node &Player::Expand(std::unique_ptr<Player::Node> &node, std::stack<ExpandedNode *> &path) const {
    assert(node && typeid(*node) != typeid(FullyExpandedNode));
    if (typeid(*node) == typeid(TerminalNode) || node->RolloutCount == 0)
        return *node;
    if (typeid(*node) == typeid(NewNode)) {
        // Since `RolloutCount` != 0, this is the second time visit to the node
        // This node must have a parent, and the parent must be `PartiallyExpandedNode`
        assert(!path.empty());
        assert(typeid(*path.top()) == typeid(PartiallyExpandedNode));
        const auto &lastPartExpNode = static_cast<const PartiallyExpandedNode &>(*path.top());
        const auto &newNode = static_cast<const NewNode &>(*node);
        // Clone the state and action generator data from the parent, and take action on the cloned ones
        auto state = m_Game->CloneState(*lastPartExpNode.State);
        auto result = m_Game->TakeAction(*state, *newNode.Action);
        if (result) {
            node = std::make_unique<TerminalNode>(std::move(*result));
            return *node;
        }
        auto actionGeneratorData = m_ActionGenerator->CloneData(*lastPartExpNode.ActionGeneratorData);
        m_ActionGenerator->Update(*actionGeneratorData, *newNode.Action);
        auto nextAction = m_ActionGenerator->FirstAction(*actionGeneratorData, *state);
        node = std::make_unique<PartiallyExpandedNode>(node->Score, node->RolloutCount, std::move(state),
                                                       std::move(actionGeneratorData), std::move(nextAction));
    } else if (typeid(*node) == typeid(UnexpandedNode)) {
        // Move state and action generator data from `UnexpandedNode` to the new `PartiallyExpandedNode`
        auto &unExpNode = static_cast<UnexpandedNode &>(*node);
        auto nextAction = m_ActionGenerator->FirstAction(*unExpNode.ActionGeneratorData, *unExpNode.State);
        node = std::make_unique<PartiallyExpandedNode>(node->Score, node->RolloutCount, std::move(unExpNode.State),
                                                       std::move(unExpNode.ActionGeneratorData), std::move(nextAction));
    }
    assert(typeid(*node) == typeid(PartiallyExpandedNode));
    // Expand the current node. Instead of expanding all child nodes at once, we create one child node per visit
    auto &partExpNode = static_cast<PartiallyExpandedNode &>(*node);
    std::unique_ptr<Node> newNode = std::make_unique<NewNode>(m_Game->CloneAction(*partExpNode.NextAction));
    partExpNode.Children.push_back(std::move(newNode));
    // If all children are expanded, turn this node into a `FullyExpandedNode`
    if (!m_ActionGenerator->NextAction(*partExpNode.ActionGeneratorData, *partExpNode.State, *partExpNode.NextAction)) {
        // Because the parent state is about to be freed (there is no `State` in `FullyExpandedNode`),
        // all `NewNode` of the children should be turned into `UnexpandedNode`,
        // i.e. The children should store `State` instead of `Action`
        for (auto &childNode : partExpNode.Children) {
            if (typeid(*childNode) != typeid(NewNode))
                continue;
            const auto &childNewNode = static_cast<const NewNode &>(*childNode);
            // Clone the state and action generator data from the parent, and take action on the cloned ones
            auto state = m_Game->CloneState(*partExpNode.State);
            auto result = m_Game->TakeAction(*state, *childNewNode.Action);
            if (result) {
                childNode = std::make_unique<TerminalNode>(std::move(*result));
                continue;
            }
            auto actionGeneratorData = m_ActionGenerator->CloneData(*partExpNode.ActionGeneratorData);
            m_ActionGenerator->Update(*actionGeneratorData, *childNewNode.Action);
            childNode = std::make_unique<UnexpandedNode>(childNewNode.Score, childNewNode.RolloutCount,
                                                         std::move(state), std::move(actionGeneratorData));
        }
        // Turn the current node into `FullyExpandedNode`
        const auto nextPlayer = m_Game->GetNextPlayer(*partExpNode.State);
        node = std::make_unique<FullyExpandedNode>(partExpNode.Score, partExpNode.RolloutCount,
                                                   std::move(partExpNode.Children), nextPlayer);
    }
    assert(dynamic_cast<const ExpandedNode *>(node.get()));
    auto &expNode = static_cast<ExpandedNode &>(*node);
    path.push(&expNode);
    // Select the newly created node, it may be `NewNode`, `TerminalNode`, or `UnexpandedNode`
    assert(!expNode.Children.empty());
    return *expNode.Children.back();
}

// Rollout at the given node to estimate the value of the node
std::vector<float> Player::Rollout(const Node &node, const std::stack<ExpandedNode *> &path) const {
    if (typeid(node) == typeid(TerminalNode))
        return static_cast<const TerminalNode &>(node).Result;
    assert(node.RolloutCount == 0);
    // Get the state to perform rollout, if it's `NewNode`, the state is calculated from the action
    std::unique_ptr<State> state;
    if (typeid(node) == typeid(NewNode)) {
        assert(!path.empty());
        assert(typeid(*path.top()) == typeid(PartiallyExpandedNode));
        const auto &lastPartExpNode = static_cast<const PartiallyExpandedNode &>(*path.top());
        const auto &newNode = static_cast<const NewNode &>(node);
        state = m_Game->CloneState(*lastPartExpNode.State);
        auto result = m_Game->TakeAction(*state, *newNode.Action);
        if (result)
            return *result;
    } else { // if (typeid(node) == typeid(UnexpandedNode))
        assert(typeid(node) == typeid(UnexpandedNode));
        const auto &unExpNode = static_cast<const UnexpandedNode &>(node);
        state = m_Game->CloneState(*unExpNode.State);
    }
    // Rollout
    const auto player = Player::Create(m_RolloutPolicyType, *m_Game, *state, m_RolloutPolicyData);
    std::optional<std::vector<float>> result;
    player->StartThinking();
    while (true) {
        const auto action = player->GetBestAction(std::nullopt);
        result = m_Game->TakeAction(*state, *action);
        if (result)
            break;
        player->Update(*action);
    }
    player->StopThinking();
    return *result;
}

// Update `Score` and `RolloutCount` along the path
void Player::BackPropagate(Node *node, std::stack<ExpandedNode *> &path, const std::vector<float> &result) const {
    // Calculate the incremental score for each player
    std::vector<float> score;
    score.reserve(m_GoalMatrix.size());
    for (const auto &coef : m_GoalMatrix)
        score.push_back(std::inner_product(result.cbegin(), result.cend(), coef.cbegin(), 0));
    // Update each node on the path
    while (!path.empty()) {
        // Get the next player
        uint8_t nextPlayer;
        if (typeid(*path.top()) == typeid(FullyExpandedNode)) {
            const auto &fullExpNode = static_cast<const FullyExpandedNode &>(*path.top());
            nextPlayer = fullExpNode.NextPlayer;
        } else { // if (typeid(*path.top()) == typeid(PartiallyExpandedNode))
            assert(typeid(*path.top()) == typeid(PartiallyExpandedNode));
            const auto &partExpNode = static_cast<const PartiallyExpandedNode &>(*path.top());
            nextPlayer = m_Game->GetNextPlayer(*partExpNode.State);
        }
        // Update the score
        node->Score = (node->Score * node->RolloutCount + score[nextPlayer]) / (node->RolloutCount + 1);
        ++node->RolloutCount;
        node = path.top();
        path.pop();
    }
    // The `RolloutCount` of the root node is used when traversing the tree,
    // but the `Score` of the root node is not used, so we don't update score
    ++node->RolloutCount;
}

std::unique_ptr<Action> Player::ChooseBestAction(const Node &root) const {
    if (typeid(root) == typeid(UnexpandedNode))
        // If the root node is `UnexpandedNode`, no iterations have completed,
        // return the first action as a fallback strategy
        // TODO: Need a warning message
        return m_ActionGenerator->FirstAction(*m_ActionGeneratorData, *m_State);
    assert(dynamic_cast<const ExpandedNode *>(&root));
    const auto &expNode = static_cast<const ExpandedNode &>(root);
    assert(expNode.Children.size() > 0);
    uint32_t maxCount = 0;
    unsigned int maxIdx = 0;
    for (unsigned int idx = 0; idx < expNode.Children.size(); ++idx) {
        const auto count = expNode.Children[idx]->RolloutCount;
        if (count > maxCount) {
            maxCount = count;
            maxIdx = idx;
        }
    }
    return m_ActionGenerator->GetNthAction(*m_ActionGeneratorData, *m_State, maxIdx);
}

Player::Player(const Game &game, const State &state, const nlohmann::json &data) : m_Game(&game), m_State(&state) {
    const auto &actionGeneratorJson = data["actionGenerator"];
    const auto &rolloutPlayerJson = data["rolloutPlayer"];
    m_ActionGenerator = ActionGenerator::Create(actionGeneratorJson["type"], game, actionGeneratorJson["data"]);
    m_ActionGeneratorData = m_ActionGenerator->CreateData(state);
    m_Iterations = data["iterations"];
    m_ExplorationFactor = data["explorationFactor"];
    m_GoalMatrix = data["goalMatrix"].get<std::vector<std::vector<double>>>();
    m_RolloutPolicyType = rolloutPlayerJson["type"];
    m_RolloutPolicyData = rolloutPlayerJson["data"];
}

std::unique_ptr<Action> Player::GetBestAction(std::optional<std::chrono::duration<double>>) {
    std::unique_ptr<Node> root = std::make_unique<UnexpandedNode>(0.0f, 0, m_Game->CloneState(*m_State),
                                                                  m_ActionGenerator->CloneData(*m_ActionGeneratorData));
    std::stack<ExpandedNode *> path;
    for (unsigned int iter = 0; iter < m_Iterations; ++iter) {
        auto &selectedNode = Select(root, path);
        auto &expandedNode = Expand(selectedNode, path);
        const auto result = Rollout(expandedNode, path);
        BackPropagate(&expandedNode, path, result);
    }
    return ChooseBestAction(*root);
}
} // namespace mcts
