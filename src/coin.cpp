// From the many, one
// From one, the source.
//
#include "coin.h"

Coin::Coin(const CWalletTx *transaction, uint outputIndex, uint blockDepth)
{
    this.transaction = transaction;
    this.outputIndex = outputIndex;
    this.blockDepth = blockDepth;
}

uint64 Coin::getValue() {
    return 0;
}

const CWalletTx *Coin::getTransaction()
{
    return this.transaction;
}

uint Coin::getOutputIndex()
{
    return this.outputIndex;
}

uint Coin::getBlockDepth()
{
    return this.blockDepth;
}

std::string Coin::ToString() const
{
    return strprintf("COutput(%s, %d, %d) [%s]", tx->GetHash().ToString().c_str(), i, nDepth, FormatMoney(tx->vout[i].nValue).c_str());
}

void Coin::print() const
{
    printf("%s\n", ToString().c_str());
}
