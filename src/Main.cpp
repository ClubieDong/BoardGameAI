#include "Controller.hpp"
#include "Chesses/TicTacToe/TicTacToe.hpp"
#include "Players/HumanPlayer/HumanPlayer.hpp"

int main()
{
    using Game = TicTacToe;
    using Player1 = HumanPlayer<Game>;
    using Player2 = HumanPlayer<Game>;
    Controller<Game, Player1, Player2> controller;
    auto result = controller.Start();
    std::cout << "Game over! " << result[0] << ':' << result[1] << '\n';
    return 0;
}
