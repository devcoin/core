// From the many, one
// From one, the source.
//
#ifndef WALLET_RESERVEKEY_H
#define WALLET_RESERVEKEY_H

#include "uint256.h"

class CPubKey;
class CWallet;

/** A key allocated from the key pool. */
class CReserveKey
{
protected:
    CWallet* pwallet;
    int64 nIndex;
    CPubKey vchPubKey;

public:
    CReserveKey(CWallet* pwalletIn);
    ~CReserveKey();
    void ReturnKey();
    bool GetReservedKey(CPubKey &pubkey);
    void KeepKey();
};

#endif