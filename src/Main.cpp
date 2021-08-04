#include <memory>
#include "Games/TicTacToe/TicTacToe.hpp"
#include "Games/TicTacToe/ActGens/DefaultActGen.hpp"
#include "Players/RandomPlayer/RandomPlayer.hpp"
#include "Controller.hpp"

// DEBUG
#include <iostream>

int main()
{
    auto game = std::make_unique<TicTacToe>();
    std::vector<std::unique_ptr<PlayerBase>> players(2);
    players[0] = std::make_unique<RandomPlayer>(
        std::make_unique<tictactoe::DefaultActGen>());
    players[1] = std::make_unique<RandomPlayer>(
        std::make_unique<tictactoe::DefaultActGen>());
    Controller controller(std::move(game), std::move(players));
    auto res = controller.Start();
    std::cout << res[0] << ':' << res[1] << '\n';
    return 0;
}
