#include "Controller.hpp"
#include "Chesses/TicTacToe/TicTacToe.hpp"
#include "Players/HumanPlayer/HumanPlayer.hpp"
#include "Players/MCTS/MCTS.hpp"

int main()
{
    using Game = TicTacToe;
    using Player1 = HumanPlayer<Game>;
    using Player2 = MCTS<Game, 1000>;
    Controller<Game, Player1, Player2> controller;
    auto result = controller.Start();
    std::cout << "Game over! " << result[0] << ':' << result[1] << '\n';
    return 0;
}
