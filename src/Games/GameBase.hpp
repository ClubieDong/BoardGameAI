#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <cassert>

enum class GameType
{
    TicTacToe,
};

class StateBase
{
protected:
    unsigned char _NextPlayer;

public:
    explicit StateBase(unsigned char nextPlayer) : _NextPlayer(nextPlayer) {}

    virtual ~StateBase() = default;

    virtual GameType GetGameType() const = 0;
    unsigned char GetNextPlayer() const { return _NextPlayer; }
};

class ActionBase
{
public:
    virtual ~ActionBase() = default;

    virtual GameType GetGameType() const = 0;
    virtual std::unique_ptr<ActionBase> Clone() const = 0;
};

class GameBase
{
public:
    const unsigned char PlayerCount;

    explicit GameBase(unsigned char playerCount) : PlayerCount(playerCount) {}

    virtual ~GameBase() = default;

    virtual GameType GetGameType() const = 0;
    virtual bool IsValidAction(const StateBase &state, const ActionBase &act) const = 0;
    virtual std::unique_ptr<StateBase> NewState() const = 0;
    virtual std::optional<std::vector<double>> Move(StateBase &state, const ActionBase &act) const = 0;
};
