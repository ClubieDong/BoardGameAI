# BoardGameAI
Multiple AIs (MCTS, etc.) for multiple games (Gobang, etc.)

## Introduction
This project is consisted of two decoupled part: 1) Game, and 2) Player.

## Gamess
For each game, one or more action genreators is written, which generates valid actions on a certain state of the game.

### Gobang

Also known as renju, five in a row, etc.

#### Default action generator
Generates moves on empty points.

#### Neighbor action generator
Generates moves on empty points adjacent to existing moves.

#### Smart action generator
Generates moves that lead to immediate win, and that do not lead to immediate lose. It is based on DFA.

### TicTacToe

#### Default action generator
Generates moves on empty points.

## Players

### HumanPlayer
A command line interface for humans to play with AI.

### RandomPlayer
Randomly chooses an action from the given action generator.

### MCTS & Parallel MCTS
Monte Carlo Tree Search algorithm.

## License
The project is licensed under MIT license.

## Contact
Email: 810443053@qq.com
