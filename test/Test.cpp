#include "../src/Server/Server.hpp"
#include <gtest/gtest.h>
#include <iostream>

TEST(Test, Case1) {
    Server server(std::cin, std::cout);
    server.AddGame(R"({"type":"gomoku","data":{}})"_json);
    server.AddState(R"({"gameID":1})"_json);
    server.AddPlayer(
        R"({"gameID":1,"stateID":1,"type":"mcts","data":{"explorationFactor":1,"goalMatrix":[[1,0],[0,1]],"actionGenerator":{"type":"neighbor","data":{"range":1}},"rolloutPlayer":{"type":"random_move","data":{"actionGenerator":{"type":"neighbor","data":{"range":1}}}},"parallel":true,"workers":2}})"_json);
    server.TakeAction(R"({"gameID":1,"stateID":1,"action":{"row":7,"col":7}})"_json);
    server.StartThinking(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
    server.GetBestAction(R"({"gameID":1,"stateID":1,"playerID":1,"maxThinkTime":2})"_json);
    server.StopThinking(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
    const auto details = server.QueryDetails(R"({"gameID":1,"stateID":1,"playerID":1,"data":{}})"_json);
    std::cout << details << '\n';
}

TEST(Test, Case2) {
    Server server(std::cin, std::cout);
    server.RunGames(
        R"({"rounds":1,"parallel":false,"game":{"type":"gomoku","data":{}},"players":[{"type":"mcts","data":{"explorationFactor":1,"goalMatrix":[[1,0],[0,1]],"actionGenerator":{"type":"neighbor","data":{"range":1}},"rolloutPlayer":{"type":"random_move","data":{"actionGenerator":{"type":"neighbor","data":{"range":1}}}},"parallel":false,"iterations":1000},"allowBackgroundThinking":false},{"type":"mcts","data":{"explorationFactor":1,"goalMatrix":[[1,0],[0,1]],"actionGenerator":{"type":"neighbor","data":{"range":1}},"rolloutPlayer":{"type":"random_move","data":{"actionGenerator":{"type":"neighbor","data":{"range":1}}}},"parallel":false,"iterations":1000},"allowBackgroundThinking":false}]})"_json);
}
