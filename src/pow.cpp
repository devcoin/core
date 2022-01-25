// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core and Devcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>

unsigned int GetNextWorkRequired_Original(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
    assert(nHeightFirst >= 0);
    const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    int64_t nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks

    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < nPowTargetTimespan/4)
        nActualTimespan = nPowTargetTimespan/4;
    if (nActualTimespan > nPowTargetTimespan*4)
        nActualTimespan = nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired_Old(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    unsigned int nProofOfWorkLimit = bnPowLimit.GetCompact();

    const int nSmoothBlock = 10700;

    int64_t nPowTargetTimespan = 24 * 60 * 60; // one day

    if (pindexLast->nHeight < nSmoothBlock)
        nPowTargetTimespan *= 14; // two weeks

    const int64_t nInterval = nPowTargetTimespan / params.nPowTargetSpacing;

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    const int nMedianBlock = 10800;
    int64_t nIntervalMinusOne = nInterval-1;

    if (pindexLast->nHeight < 10)
        return pindexLast->nBits;

    // Change at each block after nSmoothBlock
    if (pindexLast->nHeight < nSmoothBlock)
    {
        if ((pindexLast->nHeight+1) % nInterval != 0)
        {
            if (params.fPowAllowMinDifficultyBlocks)
            {
                // Special difficulty rule for testnet:
                // If the new block's timestamp is more than 2* 10 minutes
                // then allow mining of a min-difficulty block.
                if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                    return nProofOfWorkLimit;
                else
                {
                    // Return the last non-special-min-difficulty-rules-block
                    const CBlockIndex* pindex = pindexLast;
                    while (pindex->pprev && pindex->nHeight % nInterval != 0 && pindex->nBits == nProofOfWorkLimit)
                        pindex = pindex->pprev;
                    return pindex->nBits;
                }
            }
            return pindexLast->nBits;
        }
    }

    // Go back by what we want to be one day worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    std::vector<int64_t> blockTimes;
    arith_uint256 averageBits;
    averageBits.SetCompact(0);

    for (int i = 0; pindexFirst && i < nIntervalMinusOne; i++)
    {
        averageBits += arith_uint256().SetCompact(pindexFirst->nBits);
        blockTimes.push_back(pindexFirst->GetBlockTime());
        pindexFirst = pindexFirst->pprev;
    }

    assert(pindexFirst);
    int blockTimeEndIndex = blockTimes.size() - 6;
    sort(blockTimes.begin(), blockTimes.end());
    averageBits /= nIntervalMinusOne;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    int64_t nMedianTimespan = blockTimes[blockTimeEndIndex] - blockTimes[6];
    nMedianTimespan *= nIntervalMinusOne / (int64_t)(blockTimeEndIndex - 6);

    // Change nActualTimespan after nMedianBlock
    if (pindexLast->nHeight > nMedianBlock)
    {
        nActualTimespan = nMedianTimespan;
    }

    //~ printf("nActualTimespan = %d before bounds\n", nActualTimespan);

    if (nActualTimespan < nPowTargetTimespan/4)
        nActualTimespan = nPowTargetTimespan/4;

    if (nActualTimespan > nPowTargetTimespan*4)
        nActualTimespan = nPowTargetTimespan*4;

    // Retarget
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);

    // Change bnNew after nMedianBlock
    if (pindexLast->nHeight > nMedianBlock)
        bnNew = averageBits;

    bnNew *= nActualTimespan;
    bnNew /= nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    /// debug print
    //~ printf("GetNextWorkRequired RETARGET nHeight=%d\n", pindexLast->nHeight);
    //~ printf("bnPowLimit=%s nPowTargetTimespan=%d nActualTimespan=%d\n", bnPowLimit.ToString().c_str(), nPowTargetTimespan, nActualTimespan);
    //~ printf("Target: %08x nInterval=%d \n", pblock->nBits, nInterval);
    //~ printf("Before: %08x %s\n", pindexLast->nBits, arith_uint256().SetCompact(pindexLast->nBits).ToString().c_str());
    //~ printf("After : %08x %s\n", bnNew.GetCompact(), bnNew.ToString().c_str());

    return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    return GetNextWorkRequired_Old(pindexLast, pblock, params);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
