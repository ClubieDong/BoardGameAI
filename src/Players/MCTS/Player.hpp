#pragma once

#include "../../Games/ActionGenerator.hpp"
#include "../../Games/Game.hpp"
#include "../Player.hpp"
#include <stack>
#include <variant>
#include <vector>

namespace mcts {
class Player : public ::Player {
private:
    //                         [NewNode]
    //                             | take action
    //                             v
    //             No -----  <is terminal?> ----- Yes
    //                |                         |
    //                v                         v
    //     [NormalNode(unexpanded)]       [TerminalNode]
    //       Children.size() == 0
    //                | expand
    //                V
    // [NormalNode(partially expanded)]
    //        NewChildCount > 0
    //                | all children take action
    //                V
    //   [NormalNode(fully expanded)]
    //        NewChildCount == 0
    struct Node;
    struct NewNode {
        std::unique_ptr<struct Action> Action;
        explicit NewNode(std::unique_ptr<struct Action> &&action) : Action(std::move(action)) {}
    };
    struct TerminalNode {
        double Score = 0;
        std::uint32_t Count = 0;
        std::vector<double> Result;
        explicit TerminalNode(std::vector<double> &&result) : Result(std::move(result)) {}
    };
    struct NormalNode {
        double Score = 0;
        std::uint32_t Count = 0, NewChildCount = 0;
        std::vector<Node> Children;
        std::unique_ptr<struct State> State;
        std::unique_ptr<ActionGenerator::Data> ActionGeneratorData;
        std::uint8_t NextPlayer;
        explicit NormalNode(std::unique_ptr<struct State> &&state,
                            std::unique_ptr<ActionGenerator::Data> &&actionGeneratorData, unsigned char nextPlayer)
            : State(std::move(state)), ActionGeneratorData(std::move(actionGeneratorData)), NextPlayer(nextPlayer) {}
    };
    struct Node : public std::variant<NewNode, TerminalNode, NormalNode> {
        using std::variant<NewNode, TerminalNode, NormalNode>::variant;
        bool IsLeaveNode() const;
        std::uint32_t GetCount() const;
        double GetScore() const;
        void UpdateScore(double score);
    };

    Node *Select(Node *root, std::stack<Node *> &path) const;
    Node *Expand(Node *node, std::stack<Node *> &path) const;
    std::vector<double> Rollout(const Node *node) const;
    void BackPropagate(Node *node, std::stack<Node *> &path, const std::vector<double> &result) const;
    std::unique_ptr<Action> ChooseBestAction(const Node *root) const;

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
