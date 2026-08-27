// Copyright (c) 2009-2022 The Bitcoin Core and Devcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DEVCOIN_DEVCOIN_H
#define DEVCOIN_DEVCOIN_H

#include <amount.h>
#include <inttypes.h>

// Keep the historical consensus ceiling while creating templates accepted by legacy nodes.
static const int64_t initialSubsidy = 50000 * COIN;
static const int64_t minerSubsidy = 2500 * COIN;

#endif // DEVCOIN_DEVCOIN_H
