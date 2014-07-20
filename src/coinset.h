// From the many, one
// From one, the source.
//
#ifndef DEVCOIN_COIN_H
#define DEVCOIN_COIN_H

#include <set>

#include "uint256.h"

class Coin;

class CoinSet
{
private:
    std::set<Coin> coins;

public:
    CoinSet();
    void addCoin(Coin coin);
    void removeCoin(Coin coin);
    bool hasCoin(Coin coin);
    uint64 getValue();
};

#endif