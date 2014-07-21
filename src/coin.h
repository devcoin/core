// From the many, one
// From one, the source.
//
#ifndef COIN_H
#define COIN_H

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
    unsigned int outputIndex;

    // The number of blocks that have been added to the blockchain since
    // transaction was included.
    //
    unsigned int blockDepth;

public:

    Coin(
        const CWalletTx *transaction,
        unsigned int outputIndex,
        unsigned int blockDepth
    );
    
    const CWalletTx *getTransaction() const;
    unsigned int getOutputIndex() const;
    unsigned int getBlockDepth() const;

    uint64 getValue() const;

    std::string ToString() const;
    void print() const;
};

#endif