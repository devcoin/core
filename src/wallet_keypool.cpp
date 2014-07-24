// From the many, one
// From one, the source.
//
#include "key.h"
#include "wallet_keypool.h"
#include "util.h" // Gives us GetTime()

CKeyPool::CKeyPool()
{
    nTime = GetTime();
}

CKeyPool::CKeyPool(const CPubKey& vchPubKeyIn)
{
    nTime = GetTime();
    vchPubKey = vchPubKeyIn;
}
