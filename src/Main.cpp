#include "Controller.hpp"
#include "Chesses/Gobang/Gobang.hpp"
#include "Players/HumanPlayer/HumanPlayer.hpp"
#include "Players/MCTS/ParallelMCTS.hpp"
#include "Utilities/Literals.hpp"

int main()
{
    using Game = Gobang<15, 5>;
    using Player1 = HumanPlayer<Game>;
    using Player2 = ParallelMCTS<Game, 2_sec, 500_MB, 1>;
    Controller<Game, Player1, Player2> controller;
    auto result = controller.Start();
    std::cout << "Game over! " << result[0] << ':' << result[1] << '\n';
    return 0;
}
