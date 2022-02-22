#include "Player.hpp"
#include "../../Games/ActionGenerator.hpp"
#include "../../Games/Game.hpp"
#include <cmath>
#include <limits>
#include <random>

namespace mcts {
bool Player::Node::IsLeaveNode() const {
    return !std::holds_alternative<NormalNode>(*this) || std::get<NormalNode>(*this).Children.size() == 0;
}

std::uint32_t Player::Node::GetCount() const {
    if (std::holds_alternative<TerminalNode>(*this))
        return std::get<TerminalNode>(*this).Count;
    if (std::holds_alternative<NormalNode>(*this))
        return std::get<NormalNode>(*this).Count;
    // NewNode
    return 0;
}

double Player::Node::GetScore() const {
    assert(!std::holds_alternative<NewNode>(*this));
    if (std::holds_alternative<TerminalNode>(*this))
        return std::get<TerminalNode>(*this).Score;
    if (std::holds_alternative<NormalNode>(*this))
        return std::get<NormalNode>(*this).Score;
    // Unreachable
    return 0.0;
}

void Player::Node::UpdateScore(double score) {
    assert(!std::holds_alternative<NewNode>(*this));
    if (std::holds_alternative<TerminalNode>(*this)) {
        auto &node = std::get<TerminalNode>(*this);
        node.Score = (node.Score * node.Count + score) / (node.Count + 1);
        ++node.Count;
    } else if (std::holds_alternative<NormalNode>(*this)) {
        auto &node = std::get<NormalNode>(*this);
        node.Score = (node.Score * node.Count + score) / (node.Count + 1);
        ++node.Count;
    }
    // Unreachable
}

Player::Node *Player::Select(Node *root, std::stack<Node *> &path) const {
    auto node = root;
    // While node is not a leaf node
    while (!node->IsLeaveNode()) {
        path.push(node);
        auto maxUCB = std::numeric_limits<double>::lowest();
        Node *maxNode = nullptr;
        for (auto &childNode : std::get<NormalNode>(*node).Children) {
            // Avoid 0 as divider, when count == 0, UCB is infinite
            // TODO: Randomize
            if (childNode.GetCount() == 0) {
                maxNode = &childNode;
                break;
            }
            const auto ucb = childNode.GetScore() +
                             m_ExplorationFactor * std::sqrt(2 * std::log(node->GetCount()) / childNode.GetCount());
            if (ucb > maxUCB) {
                maxUCB = ucb;
                maxNode = &childNode;
            }
        }
        assert(maxNode);
        node = maxNode;
    }
    return node;
}

Player::Node *Player::Expand(Node *node, std::stack<Node *> &path) const {
    if (std::holds_alternative<TerminalNode>(*node))
        return node;
    if (std::holds_alternative<NormalNode>(*node)) {
        // Create a child node of type NewNode for each action
        path.push(node);
        auto &normalNode = std::get<NormalNode>(*node);
        assert(normalNode.Children.size() == 0);
        m_ActionGenerator->ForEachAction(*normalNode.ActionGeneratorData, *normalNode.State, [&](const Action &action) {
            normalNode.Children.emplace_back(std::in_place_type<NewNode>, m_Game->CloneAction(action));
        });
        assert(normalNode.Children.size() > 0);
        // TODO: normalNode.Children.shrink_to_fit();
        normalNode.NewChildCount = normalNode.Children.size();
        // TODO: Randomize
        node = &normalNode.Children[0];
    }
    assert(std::holds_alternative<NewNode>(*node));
    assert(path.size() > 0);
    // Take action from the previous state to turn this NewNode into a NormalNode
    auto &prevNode = std::get<NormalNode>(*path.top());
    assert(prevNode.State && prevNode.ActionGeneratorData && prevNode.NewChildCount > 0);
    auto state = m_Game->CloneState(*prevNode.State);
    auto actionGeneratorData = m_ActionGenerator->CloneData(*prevNode.ActionGeneratorData);
    const auto action = std::move(std::get<NewNode>(*node).Action);
    m_ActionGenerator->Update(*actionGeneratorData, *action);
    auto result = m_Game->TakeAction(*state, *action);
    if (result)
        node->emplace<TerminalNode>(std::move(*result));
    else {
        const auto nextPlayer = m_Game->GetNextPlayer(*state);
        node->emplace<NormalNode>(std::move(state), std::move(actionGeneratorData), nextPlayer);
    }
    // Check if the previous node is fully expanded
    --prevNode.NewChildCount;
    if (prevNode.NewChildCount == 0) {
        prevNode.State.reset();
        prevNode.ActionGeneratorData.reset();
    }
    return node;
}

std::vector<double> Player::Rollout(const Node *node) const {
    if (std::holds_alternative<TerminalNode>(*node))
        return std::get<TerminalNode>(*node).Result;
    const auto &normalNode = std::get<NormalNode>(*node);
    assert(normalNode.State);
    const auto state = m_Game->CloneState(*normalNode.State);
    const auto player = Player::Create(m_RolloutPolicyType, *m_Game, *state, m_RolloutPolicyData);
    std::optional<std::vector<double>> result;
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

void Player::BackPropagate(Node *node, std::stack<Node *> &path, const std::vector<double> &result) const {
    // Calculate the incremental score for each player
    std::vector<double> score;
    score.reserve(m_GoalMatrix.size());
    for (const auto &coef : m_GoalMatrix)
        score.push_back(std::inner_product(result.cbegin(), result.cend(), coef.cbegin(), 0));
    // Update each node on the path
    while (!path.empty()) {
        const auto &lastNode = std::get<NormalNode>(*path.top());
        node->UpdateScore(score[lastNode.NextPlayer]);
        node = path.top();
        path.pop();
    }
    // Don't forget the root node
    ++std::get<NormalNode>(*node).Count;
}

std::unique_ptr<Action> Player::ChooseBestAction(const Node *root) const {
    const auto &normalNode = std::get<NormalNode>(*root);
    assert(normalNode.Children.size() > 0);
    unsigned int maxCount = 0, maxIdx = 0;
    for (unsigned int idx = 0; idx < normalNode.Children.size(); ++idx) {
        const auto count = normalNode.Children[idx].GetCount();
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
    m_ActionGeneratorData = m_ActionGenerator->CreateData();
    m_Iterations = data["iterations"];
    m_ExplorationFactor = data["explorationFactor"];
    m_GoalMatrix = data["goalMatrix"];
    m_RolloutPolicyType = rolloutPlayerJson["type"];
    m_RolloutPolicyData = rolloutPlayerJson["data"];
}

std::unique_ptr<Action> Player::GetBestAction(std::optional<std::chrono::duration<double>>) {
    Node root(std::in_place_type<NormalNode>, m_Game->CloneState(*m_State),
              m_ActionGenerator->CloneData(*m_ActionGeneratorData), m_Game->GetNextPlayer(*m_State));
    std::stack<Node *> path;
    for (unsigned int iter = 0; iter < m_Iterations; ++iter) {
        auto node = Select(&root, path);
        node = Expand(node, path);
        const auto result = Rollout(node);
        BackPropagate(node, path, result);
    }
    return ChooseBestAction(&root);
}
} // namespace mcts
