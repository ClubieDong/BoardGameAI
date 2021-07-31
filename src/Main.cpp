#include "Controller.hpp"
#include "Games/Gobang/Gobang.hpp"
#include "Games/Gobang/ActGens/SmartActGen.hpp"
#include "Players/HumanPlayer/HumanPlayer.hpp"
#include "Players/MCTS/ParallelMCTS.hpp"
#include "Utilities/Literals.hpp"

int main()
{
    using Game = Gobang<15, 5>;
    using Player1 = HumanPlayer<Game>;
    using Player2 = ParallelMCTS<Game, gobang::SmartActGen<Game, 1>, 15_sec, 15_GB, 12>;
    Controller<Game, Player1, Player2> controller;
    auto result = controller.Start();
    std::cout << "Game over! " << result[0] << ':' << result[1] << '\n';
    return 0;
}
