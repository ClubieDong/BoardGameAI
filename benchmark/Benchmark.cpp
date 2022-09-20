#include "../src/Server/Server.hpp"
#include <benchmark/benchmark.h>
#include <iostream>

// On average (single thread):
//   9643.2 iter/sec
//    136.6 byte/iter
//   1285.5  KiB/sec
static void BM_Gomoku_MCTS_Sequential(benchmark::State &state) {
    for (auto _ : state) {
        Server server(std::cin, std::cout);
        server.AddGame(R"({"type":"gomoku","data":{}})"_json);
        server.AddState(R"({"gameID":1})"_json);
        server.AddPlayer(
            R"({"gameID":1,"stateID":1,"type":"mcts","data":{"explorationFactor":1,"goalMatrix":[[1,0],[0,1]],"actionGenerator":{"type":"neighbor","data":{"range":1}},"rolloutPlayer":{"type":"random_move","data":{"actionGenerator":{"type":"neighbor","data":{"range":1}}}},"parallel":false,"iterations":10000}})"_json);
        server.StartThinking(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
        server.GetBestAction(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
        server.StopThinking(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
    }
}
BENCHMARK(BM_Gomoku_MCTS_Sequential);

static void BM_Gomoku_MCTS_Parallel(benchmark::State &state) {
    for (auto _ : state) {
        Server server(std::cin, std::cout);
        server.AddGame(R"({"type":"gomoku","data":{}})"_json);
        server.AddState(R"({"gameID":1})"_json);
        server.AddPlayer(
            R"({"gameID":1,"stateID":1,"type":"mcts","data":{"explorationFactor":1,"goalMatrix":[[1,0],[0,1]],"actionGenerator":{"type":"neighbor","data":{"range":1}},"rolloutPlayer":{"type":"random_move","data":{"actionGenerator":{"type":"neighbor","data":{"range":1}}}},"parallel":true,"workers":0}})"_json);
        server.StartThinking(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
        server.GetBestAction(R"({"gameID":1,"stateID":1,"playerID":1,"maxThinkTime":1})"_json);
        server.StopThinking(R"({"gameID":1,"stateID":1,"playerID":1})"_json);
    }
}
BENCHMARK(BM_Gomoku_MCTS_Parallel);
