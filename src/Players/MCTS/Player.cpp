#include "Player.hpp"
#include "../../Games/ActionGenerator.hpp"
#include "../../Games/Game.hpp"
#include <algorithm>
#include <cmath>
#include <future>
#include <limits>
#include <numeric>
#include <thread>
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
//     `NewNode` stores `Action`. This is used when the parent node is `FullyExpandedNode`, which does not store state.
//   PartiallyExpandedNode:
//     When expanding a node, to save memory, instead of expanding all child nodes at once, we create one child node per
//     visit. `PartiallyExpandedNode` uses `NextAction` to save which state to expand on the next visit. All existing
//     child nodes of `PartiallyExpandedNode` are `NewNode` or `TerminalNode`.
//   FullyExpandedNode:
//     When all child nodes of `PartiallyExpandedNode` are created, `PartiallyExpandedNode` is turned into a
//     `FullyExpandedNode`. To save memory, `FullyExpandedNode` does not store the state because state is not used
//     anymore, but stores `NextPlayer`, which is used during backpropagation. When `PartiallyExpandedNode` is turned
//     into `FullyExpandedNode`, all `NewNode`s in its child nodes should be turned into `UnexpandedNode` due to the
//     release of the state in their parent state, so there's no `NewNode` in the child nodes of `FullyExpandedNode`.

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
//   [When `PartiallyExpandedNode` is visited]
//              |                 |
//              V                 V
//          [NewNode]       [TerminalNode]
//              |
//              | parent turned into `FullyExpandedNode`
//              V
//       [UnexpandedNode]
//              | visited the second time
//              V
//   [PartiallyExpandedNode]
//              | all child node created
//              V
//     [FullyExpandedNode]

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

    explicit TerminalNode(float score, uint32_t rolloutCount, std::vector<float> &&result)
        : Node(score, rolloutCount), Result(std::move(result)) {}
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

    explicit PartiallyExpandedNode(std::unique_ptr<struct State> &&state,
                                   std::unique_ptr<ActionGenerator::Data> &&actionGeneratorData,
                                   std::unique_ptr<Action> &&nextAction)
        : ExpandedNode(0.0f, 0, {}), State(std::move(state)), ActionGeneratorData(std::move(actionGeneratorData)),
          NextAction(std::move(nextAction)) {}
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

enum class Player::Signal { StartThinking, StopThinking, GetBestAction, QueryDetails, Prune, Exit };

struct Player::ThreadData {
    std::thread Thread;
    // From main thread to worker thread
    std::promise<Signal> PromiseSignal;
    std::future<Signal> FutureSignal;
    // From worker thread to main thread
    std::promise<void> PromiseDone;
    std::future<void> FutureDone;
    // Information reported by the worker thread
    std::vector<unsigned int> ActionRolloutCount;
    std::vector<float> ActionScore;
    unsigned int TotalRolloutCount;

    ThreadData() : FutureSignal(PromiseSignal.get_future()), FutureDone(PromiseDone.get_future()) {}
};

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
        assert(maxNode && *maxNode);
        node = maxNode;
    }
    assert(*node);
    return *node;
}

Player::Node &Player::Expand(std::unique_ptr<Player::Node> &node, std::stack<ExpandedNode *> &path) const {
    assert(node && typeid(*node) != typeid(FullyExpandedNode) && typeid(*node) != typeid(NewNode));
    if (typeid(*node) == typeid(TerminalNode))
        return *node;
    if (typeid(*node) == typeid(UnexpandedNode)) {
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
        // Because the parent state is about to be freed (there is no `State` in `FullyExpandedNode`), all `NewNode`s of
        // the children should be turned into `UnexpandedNode`, that is, the children should store `State` instead of
        // `Action`
        for (auto &childNode : partExpNode.Children) {
            assert(typeid(*childNode) == typeid(NewNode) || typeid(*childNode) == typeid(TerminalNode));
            if (typeid(*childNode) != typeid(NewNode))
                continue;
            const auto &childNewNode = static_cast<const NewNode &>(*childNode);
            // Clone the state and action generator data from the parent, and take action on the cloned ones
            auto state = m_Game->CloneState(*partExpNode.State);
            auto result = m_Game->TakeAction(*state, *childNewNode.Action);
            if (result) {
                childNode =
                    std::make_unique<TerminalNode>(childNode->Score, childNode->RolloutCount, std::move(*result));
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
    assert(dynamic_cast<ExpandedNode *>(node.get()));
    auto &expNode = static_cast<ExpandedNode &>(*node);
    path.push(&expNode);
    // Select the newly created node, it may be `NewNode`, `TerminalNode`, or `UnexpandedNode`
    assert(!expNode.Children.empty());
    return *expNode.Children.back();
}

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
            return std::move(*result);
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
    return std::move(*result);
}

void Player::BackPropagate(Node *node, std::stack<ExpandedNode *> &path, const std::vector<float> &result) const {
    // Calculate the incremental score for each player
    std::vector<float> score;
    score.reserve(m_GoalMatrix.size());
    for (const auto &coef : m_GoalMatrix)
        score.push_back(std::inner_product(result.cbegin(), result.cend(), coef.cbegin(), 0.0f));
    // Update each node on the path
    while (!path.empty()) {
        // Get the next player
        unsigned char nextPlayer;
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
    // The `RolloutCount` of the root node is used when traversing the tree, but the `Score` of the root node is not
    // used, so we don't update score
    ++node->RolloutCount;
}

std::unique_ptr<Player::Node> Player::CreateRootNode() const {
    auto state = m_Game->CloneState(*m_State);
    auto actionGeneratorData = m_ActionGenerator->CloneData(*m_ActionGeneratorData);
    auto nextAction = m_ActionGenerator->FirstAction(*actionGeneratorData, *state);
    return std::make_unique<PartiallyExpandedNode>(std::move(state), std::move(actionGeneratorData),
                                                   std::move(nextAction));
}

void Player::RunSingleIteration(std::unique_ptr<Node> &root, std::stack<ExpandedNode *> &path) const {
    auto &selectedNode = Select(root, path);
    auto &expandedNode = Expand(selectedNode, path);
    const auto result = Rollout(expandedNode, path);
    BackPropagate(&expandedNode, path, result);
}

std::unique_ptr<Action> Player::ChooseBestActionSequential(const Node &root) const {
    if (typeid(root) != typeid(FullyExpandedNode))
        // If the root node is not `FullyExpandedNode`, not all actions are evaluated, so we return the first action as
        // a fallback strategy, the same for `ReportData` and `Prune` below
        // TODO: Need a warning message
        return m_ActionGenerator->FirstAction(*m_ActionGeneratorData, *m_State);
    const auto &fullExpNode = static_cast<const FullyExpandedNode &>(root);
    assert(fullExpNode.Children.size() > 0);
    // Get the index of the action with the most rollouts
    unsigned int maxIdx = 0, maxCount = 0;
    for (unsigned int idx = 0; idx < fullExpNode.Children.size(); ++idx) {
        const auto count = fullExpNode.Children[idx]->RolloutCount;
        if (count > maxCount) {
            maxCount = count;
            maxIdx = idx;
        }
    }
    return m_ActionGenerator->GetNthAction(*m_ActionGeneratorData, *m_State, maxIdx);
}

void Player::ReportData(ThreadData &data, const Node &root, bool includeScore) const {
    if (typeid(root) != typeid(FullyExpandedNode)) {
        data.ActionRolloutCount.clear();
        if (includeScore) {
            data.ActionScore.clear();
            data.TotalRolloutCount = 0;
        }
        return;
    }
    // Action rollout count
    const auto &fullExpNode = static_cast<const FullyExpandedNode &>(root);
    data.ActionRolloutCount.resize(fullExpNode.Children.size());
    for (unsigned int idx = 0; idx < fullExpNode.Children.size(); ++idx)
        data.ActionRolloutCount[idx] = fullExpNode.Children[idx]->RolloutCount;
    // Action score and total rollout count
    if (includeScore) {
        data.ActionScore.resize(fullExpNode.Children.size());
        for (unsigned int idx = 0; idx < fullExpNode.Children.size(); ++idx)
            data.ActionScore[idx] = fullExpNode.Children[idx]->Score;
        data.TotalRolloutCount = root.RolloutCount;
    }
}

void Player::Prune(std::unique_ptr<Node> &root) const {
    if (typeid(*root) != typeid(FullyExpandedNode)) {
        root = CreateRootNode();
        return;
    }
    auto &fullExpNode = static_cast<FullyExpandedNode &>(*root);
    // If `m_PruneActionIndex` is equal to `m_ActionList.size()`, the action taken is not found in `m_ActionList`, it
    // means that the opponent took an action that we did not consider, the entire existing game tree is to be freed
    assert(m_PruneActionIndex <= fullExpNode.Children.size());
    if (m_PruneActionIndex == fullExpNode.Children.size())
        root = CreateRootNode();
    else
        root = std::move(fullExpNode.Children[m_PruneActionIndex]);
}

std::unique_ptr<Action> Player::ChooseBestActionParallel() {
    SendSignal(Signal::GetBestAction);
    // Calculate the action with the most visit count
    unsigned int maxIdx = 0, maxCount = 0;
    for (unsigned int idx = 0; idx < m_ActionList.size(); ++idx) {
        // Count the visit count of all threads for the action
        unsigned int count = 0;
        for (const auto &data : m_ThreadList)
            if (data->ActionRolloutCount.size() > 0) {
                assert(data->ActionRolloutCount.size() == m_ActionList.size());
                count += data->ActionRolloutCount[idx];
            }
        if (count > maxCount) {
            maxIdx = idx;
            maxCount = count;
        }
    }
    return m_ActionGenerator->GetNthAction(*m_ActionGeneratorData, *m_State, maxIdx);
}

void Player::ThreadMain(ThreadData *data) {
    auto root = CreateRootNode();
    std::stack<ExpandedNode *> path;
    bool working = false;
    while (true) {
        // Since `std::future` lacks the `is_ready` function, we have to use `wait_until(...) != ready` to check if
        // there is a signal sent, see https://stackoverflow.com/questions/10890242/get-the-status-of-a-stdfuture for
        // details
        if (working &&
            data->FutureSignal.wait_until(std::chrono::steady_clock::time_point::min()) != std::future_status::ready) {
            RunSingleIteration(root, path);
            continue;
        }
        const auto signal = data->FutureSignal.get();
        // `std::promise` can only be used once, create another one after use
        data->PromiseSignal = {};
        data->FutureSignal = data->PromiseSignal.get_future();
        if (signal == Signal::StartThinking)
            working = true;
        else if (signal == Signal::StopThinking)
            working = false;
        else if (signal == Signal::GetBestAction)
            ReportData(*data, *root, false);
        else if (signal == Signal::QueryDetails)
            ReportData(*data, *root, true);
        else if (signal == Signal::Prune)
            Prune(root);
        data->PromiseDone.set_value();
        if (signal == Signal::Exit)
            return;
    }
}

void Player::SendSignal(Signal signal) {
    if (m_ThreadList.empty())
        for (unsigned int idx = 0; idx < m_Workers; ++idx) {
            auto data = std::make_unique<ThreadData>();
            data->Thread = std::thread(&Player::ThreadMain, this, data.get());
            m_ThreadList.push_back(std::move(data));
        }
    for (const auto &data : m_ThreadList)
        data->PromiseSignal.set_value(signal);
    for (const auto &data : m_ThreadList) {
        data->FutureDone.wait();
        // `std::promise` can only be used once, create another one after use
        data->PromiseDone = {};
        data->FutureDone = data->PromiseDone.get_future();
    }
}

Player::Player(const Game &game, const State &state, const nlohmann::json &data) : ::Player(game, state, data) {
    const auto &rolloutPlayerJson = data["rolloutPlayer"];
    m_ExplorationFactor = data["explorationFactor"];
    m_GoalMatrix = data["goalMatrix"].get<std::vector<std::vector<double>>>();
    m_RolloutPolicyType = rolloutPlayerJson["type"];
    m_RolloutPolicyData = rolloutPlayerJson["data"];
    m_Parallel = data["parallel"];
    if (m_Parallel) {
        m_Workers = data["workers"];
        if (m_Workers == 0)
            m_Workers = std::thread::hardware_concurrency();
        if (m_Workers == 0)
            // `hardware_concurrency` may return zero
            // TODO: Need a warning message
            m_Workers = 1;
        // To avoid leaking `this` during construction, worker threads are created the first time `SendSignal` is called
        m_ActionList = m_ActionGenerator->GetActionList(*m_ActionGeneratorData, *m_State, *m_Game);
    } else
        m_Iterations = data["iterations"];
}

Player::~Player() {
    if (m_Parallel) {
        SendSignal(Signal::Exit);
        for (const auto &data : m_ThreadList)
            data->Thread.join();
    }
}

void Player::StartThinking() {
    if (m_Parallel)
        SendSignal(Signal::StartThinking);
}

void Player::StopThinking() {
    if (m_Parallel)
        SendSignal(Signal::StopThinking);
}

std::unique_ptr<Action> Player::GetBestAction(std::optional<std::chrono::duration<double>> maxThinkTime) {
    if (m_Parallel) {
        if (maxThinkTime)
            std::this_thread::sleep_for(*maxThinkTime);
        return ChooseBestActionParallel();
    }
    auto root = CreateRootNode();
    std::stack<ExpandedNode *> path;
    for (unsigned int iter = 0; iter < m_Iterations; ++iter)
        RunSingleIteration(root, path);
    return ChooseBestActionSequential(*root);
}

void Player::Update(const Action &action) {
    // It's OK to update action generator data while worker threads are running
    m_ActionGenerator->Update(*m_ActionGeneratorData, action);
    if (m_Parallel) {
        // If the action taken is not found in `m_ActionList`, `m_PruneActionIndex` is equal to `m_ActionList.size()`
        m_PruneActionIndex =
            std::find_if(m_ActionList.cbegin(), m_ActionList.cend(),
                         [&](const std::unique_ptr<Action> &actPtr) { return m_Game->EqualAction(*actPtr, action); }) -
            m_ActionList.cbegin();
        SendSignal(Signal::Prune);
        m_ActionList = m_ActionGenerator->GetActionList(*m_ActionGeneratorData, *m_State, *m_Game);
    }
}

nlohmann::json Player::QueryDetails(const nlohmann::json &) {
    if (!m_Parallel)
        return nlohmann::json::object();
    SendSignal(Signal::QueryDetails);
    // Accumulate the data reported by each worker threads
    std::vector<unsigned int> actionRolloutCount(m_ActionList.size(), 0);
    std::vector<float> actionScore(m_ActionList.size(), 0.0f);
    unsigned int totalRolloutCount = 0;
    for (const auto &data : m_ThreadList) {
        if (data->ActionRolloutCount.size() == 0)
            continue;
        assert(data->ActionRolloutCount.size() == m_ActionList.size());
        assert(data->ActionScore.size() == m_ActionList.size());
        for (unsigned int idx = 0; idx < m_ActionList.size(); ++idx) {
            actionRolloutCount[idx] += data->ActionRolloutCount[idx];
            actionScore[idx] += data->ActionScore[idx] * data->TotalRolloutCount;
        }
        totalRolloutCount += data->TotalRolloutCount;
    }
    for (unsigned int idx = 0; idx < m_ActionList.size(); ++idx)
        if (totalRolloutCount != 0)
            actionScore[idx] /= totalRolloutCount;
        else
            actionScore[idx] = 0.0f;
    // Build the response
    auto actionListJson = nlohmann::json::array();
    for (unsigned int idx = 0; idx < m_ActionList.size(); ++idx)
        actionListJson.push_back({
            {"action", m_Game->GetJsonOfAction(*m_ActionList[idx])},
            {"rollouts", actionRolloutCount[idx]},
            {"score", actionScore[idx]},
        });
    std::sort(
        actionListJson.begin(), actionListJson.end(),
        [](const nlohmann::json &left, const nlohmann::json &right) { return left["rollouts"] > right["rollouts"]; });
    return {
        {"totalRollouts", totalRolloutCount},
        {"actions", std::move(actionListJson)},
    };
}
} // namespace mcts
