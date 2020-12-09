#include "Controller.hpp"
#include "Chesses/TicTacToe/TicTacToe.hpp"
#include "Players/HumanPlayer/HumanPlayer.hpp"

int main()
{
    Controller<TicTacToe, HumanPlayer, HumanPlayer> controller;
    auto result = controller.Start();
    std::cout << "Game over! " << result[0] << ':' << result[1] << '\n';
    return 0;
}
