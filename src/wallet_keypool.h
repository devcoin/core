// From the many, one
// From one, the source.
//
#ifndef WALLET_KEYPOOL_H
#define WALLET_KEYPOOL_H

#include "serialize.h"

class CPubKey;

class CKeyPool
{
public:
    int64 nTime;
    CPubKey vchPubKey;

    CKeyPool();
    CKeyPool(const CPubKey& vchPubKeyIn);

    IMPLEMENT_SERIALIZE
    (
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(nTime);
        READWRITE(vchPubKey);
    )
};

#endif