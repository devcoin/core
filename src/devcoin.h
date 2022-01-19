// Copyright (c) 2009-2022 The Bitcoin Core and Devcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

inline int64_t roundint64(double d)
{
    return (int64_t)(d > 0 ? d + 0.5 : d - 0.5);
}

//DEVCOIN globals
static const int64_t initialSubsidy = 50000*COIN ;
static const int64_t share = roundint64(initialSubsidy * 0.9);
static const int64_t fallbackReduction = roundint64((initialSubsidy + share) / 2);
static const int step = 4000;
const std::string receiverCSV = std::string("receiver.csv");
