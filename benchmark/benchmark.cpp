#include "../src/Server/Server.hpp"
#include <benchmark/benchmark.h>
#include <iostream>

static void BM_Gobang_MCTS(benchmark::State &state) {
    for (auto _ : state) {
        Server server(std::cin, std::cout);
        server.AddGame(R"({"type":"gobang","data":{}})"_json);
        server.AddState(R"({"gameID":1})"_json);
        server.AddPlayer(
            R"({"gameID":1,"stateID":1,"type":"mcts","data":{"iterations":10000,"explorationFactor":1,"goalMatrix":[[-1,1],[1,-1]],"actionGenerator":{"type":"neighbor","data":{"range":1}},"rolloutPlayer":{"type":"random_move","data":{"actionGenerator":{"type":"neighbor","data":{"range":1}}}}}})"_json);
        server.StartThinking(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
        server.GetBestAction(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
        server.StopThinking(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
    }
}
BENCHMARK(BM_Gobang_MCTS);
