// From the many, one
// From one, the source.
//
#include <boost/foreach.hpp>

#include "coin.h"
#include "coinset.h"

void CoinSet::addCoin(Coin *coin)
{
    this->insert(coin);
}
    
void CoinSet::removeCoin(Coin *coin)
{
    this->erase(coin);
}

bool CoinSet::hasCoin(Coin *coin) const
{
    return this->count(coin) > 0;
}

uint64 CoinSet::getValue() const
{
    uint64 total = 0;
    BOOST_FOREACH(Coin *coin, *this) {
        total += coin->getValue();
    }
    return total;
}
