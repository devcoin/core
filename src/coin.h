// From the many, one
// From one, the source.
//
#ifndef DEVCOIN_COIN_H
#define DEVCOIN_COIN_H

#include "uint256.h"

class CWalletTx;

// Coin holds the information needed to access a particular output in a
// transaction. It is meant to replace COutput.
//
class Coin
{
private:
    // The transaction that contains this coin. e.g.
    // `transaction->vout[outputIndex]` will get you the coin's information.
    //
    const CWalletTx *transaction;
    
    // The index of transaction output that represents this coin.
    //
    uint outputIndex;

    // The number of blocks that have been added to the blockchain since
    // transaction was included.
    //
    uint blockDepth;

public:

    Coin(const CWalletTx *transaction, uint outputIndex, uint blockDepth);
    
    const CWalletTx *getTransaction() const;
    uint getOutputIndex() const;
    uint getBlockDepth() const;

    uint64 getValue() const;

    std::string ToString() const;
    void print() const;
};

#endif