// From the many, one
// From one, the source.
//
#include <stdlib.h>

#include "coin.h"
#include "util.h"
#include "wallet.h"

Coin::Coin(const CWalletTx *transaction, uint outputIndex, uint blockDepth)
{
    this->transaction = transaction;
    this->outputIndex = outputIndex;
    this->blockDepth = blockDepth;
}

uint64 Coin::getValue() const
{
    return this->transaction->vout[this->outputIndex].nValue;
}

const CWalletTx *Coin::getTransaction() const
{
    return this->transaction;
}

uint Coin::getOutputIndex() const
{
    return this->outputIndex;
}

uint Coin::getBlockDepth() const
{
    return this->blockDepth;
}

std::string Coin::ToString() const
{
    return strprintf(
        "Coin(%s, %d, %d) [%s]",
        this->transaction->GetHash().ToString().c_str(),
        this->outputIndex,
        this->blockDepth,
        FormatMoney(this->getValue()).c_str()
    );
}

void Coin::print() const
{
    printf("%s\n", this->ToString().c_str());
}
