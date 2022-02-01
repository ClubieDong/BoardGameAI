#include "../../../src/Games/TicTacToe/Game.hpp"
#include <gtest/gtest.h>

TEST(TicTacToe_State, Constructor) {
    { // Create default state
        const tic_tac_toe::Game game("{}"_json);
        const tic_tac_toe::State state(game);
        EXPECT_EQ(state.MoveCount, 0);
        EXPECT_EQ(state.Board, (decltype(state.Board){0, 0, 0, 0, 0, 0, 0, 0, 0}));
    }
    { // Create state from JSON
        const tic_tac_toe::State state(R"({
            "board": [[1, 2, 1], [2, 1, 2], [0, 0, 0]]
        })"_json);
        EXPECT_EQ(state.MoveCount, 6);
        EXPECT_EQ(state.Board, (decltype(state.Board){1, 2, 1, 2, 1, 2, 0, 0, 0}));
    }
}

TEST(TicTacToe_State, GetJson) {
    const auto expect = R"({
        "board": [[1, 2, 1], [2, 1, 2], [0, 0, 0]]
    })"_json;
    const tic_tac_toe::State state(expect);
    const auto actual = state.GetJson();
    EXPECT_EQ(actual, expect);
}

TEST(TicTacToe_Action, Constructor) {
    { // Create action from row and col
        const tic_tac_toe::Action action(1, 2);
        EXPECT_EQ(action.Row, 1);
        EXPECT_EQ(action.Col, 2);
    }
    { // Create action from JSON
        const tic_tac_toe::Action action(R"({"row": 1, "col": 2})"_json);
        EXPECT_EQ(action.Row, 1);
        EXPECT_EQ(action.Col, 2);
    }
}

TEST(TicTacToe_Action, GetJson) {
    const tic_tac_toe::Action action(1, 2);
    const auto json = action.GetJson();
    EXPECT_EQ(json, R"({"row": 1, "col": 2})"_json);
}

TEST(TicTacToe_Game, Constructor) {
    const tic_tac_toe::Game game("{}"_json);
}

TEST(TicTacToe_Game, IsValidAction) {
    const tic_tac_toe::Game game("{}"_json);
    const tic_tac_toe::State state(R"({
        "board": [[1, 2, 1], [2, 1, 2], [0, 0, 0]]
    })"_json);
    { // Valid action
        const tic_tac_toe::Action action(2, 1);
        EXPECT_TRUE(game.IsValidAction(state, action));
    }
    { // Row is out of range
        const tic_tac_toe::Action action(3, 1);
        EXPECT_FALSE(game.IsValidAction(state, action));
    }
    { // Col is out of range
        const tic_tac_toe::Action action(1, 3);
        EXPECT_FALSE(game.IsValidAction(state, action));
    }
    { // The grid is not empty
        const tic_tac_toe::Action action(0, 0);
        EXPECT_FALSE(game.IsValidAction(state, action));
    }
}

TEST(TicTacToe_Game, TakeAction) {
    const tic_tac_toe::Game game("{}"_json);
    { // Non-terminal action
        tic_tac_toe::State state(game);
        const tic_tac_toe::Action action(1, 1);
        const auto result = game.TakeAction(state, action);
        EXPECT_EQ(result, std::nullopt);
    }
    { // First player win action
        tic_tac_toe::State state(R"({
            "board": [[1, 1, 0], [2, 2, 0], [0, 0, 0]]
        })"_json);
        const tic_tac_toe::Action action(0, 2);
        const auto result = game.TakeAction(state, action);
        EXPECT_TRUE(result.has_value());
        EXPECT_EQ(*result, (std::vector{1.0, 0.0}));
    }
    { // Second player win action
        tic_tac_toe::State state(R"({
            "board": [[1, 1, 0], [2, 2, 0], [1, 0, 0]]
        })"_json);
        const tic_tac_toe::Action action(1, 2);
        const auto result = game.TakeAction(state, action);
        EXPECT_TRUE(result.has_value());
        EXPECT_EQ(*result, (std::vector{0.0, 1.0}));
    }
    { // Draw action
        tic_tac_toe::State state(R"({
            "board": [[2, 1, 2], [2, 1, 1], [1, 2, 0]]
        })"_json);
        const tic_tac_toe::Action action(2, 2);
        const auto result = game.TakeAction(state, action);
        EXPECT_TRUE(result.has_value());
        EXPECT_EQ(*result, (std::vector{0.5, 0.5}));
    }
}
