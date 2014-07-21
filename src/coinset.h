// From the many, one
// From one, the source.
//
#ifndef COINSET_H
#define COINSET_H

#include <set>

#include "uint256.h"

class Coin;

class CoinSet: public std::set<Coin *>
{
public:
    void addCoin(Coin *coin);
    void removeCoin(Coin *coin);
    bool hasCoin(Coin *coin) const;
    uint64 getValue() const;
};

#endif