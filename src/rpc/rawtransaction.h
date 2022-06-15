// Copyright (c) 2017-2022 The Bitcoin Core and Devcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEVCOIN_RPC_RAWTRANSACTION_H
#define DEVCOIN_RPC_RAWTRANSACTION_H

class CTransaction;
class UniValue;
class uint256;

void TxToJSON(const CTransaction& tx, const uint256 hashBlock, UniValue& entry, CChainState& active_chainstate);

#endif // DEVCOIN_RPC_RAWTRANSACTION_H
