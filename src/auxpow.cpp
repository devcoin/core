// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2011 Vince Durham
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2019 Daniel Kraft
// Distributed under the MIT/X11 software license, see the accompanying
// file license.txt or http://www.opensource.org/licenses/mit-license.php.

#include <auxpow.h>

#include <consensus/consensus.h>
#include <consensus/merkle.h>
#include <hash.h>
#include <primitives/block.h>
#include <script/script.h>
#include <util/strencodings.h>
#include <util/system.h>

#include <algorithm>
#include <string>

namespace
{

/**
 * Decodes a 32-bit little endian integer from raw bytes.
 */
uint32_t
DecodeLE32 (const unsigned char* bytes)
{
  uint32_t res = 0;
  for (int i = 0; i < 4; ++i)
    {
      res <<= 8;
      res |= bytes[3 - i];
    }
  return res;
}

struct CommitmentResult
{
  bool present;
  bool valid;
  std::string error;
};

template <typename Iterator>
CommitmentResult
CheckCommitmentTail (Iterator pc, Iterator end,
                     const unsigned merkleHeight, const int chainIndex,
                     const int chainId)
{
  if (end - pc < 8)
    return {true, false,
            "Aux POW missing chain merkle tree size and nonce in parent coinbase"};

  const uint32_t nSize = DecodeLE32 (&pc[0]);
  if (nSize != (1u << merkleHeight))
    return {true, false,
            "Aux POW merkle branch size does not match parent coinbase"};

  const uint32_t nNonce = DecodeLE32 (&pc[4]);
  if (chainIndex != CAuxPow::getExpectedIndex (nNonce, chainId, merkleHeight))
    return {true, false, "Aux POW wrong index"};

  return {true, true, {}};
}

CommitmentResult
CheckInputCommitment (const CScript& script, const valtype& root,
                      const unsigned merkleHeight, const int chainIndex,
                      const int chainId)
{
  const auto mmHeaderBegin = std::begin (pchMergedMiningHeader);
  const auto mmHeaderEnd = std::end (pchMergedMiningHeader);
  auto pcHead = std::search (script.begin (), script.end (),
                             mmHeaderBegin, mmHeaderEnd);
  auto pc = std::search (script.begin (), script.end (), root.begin (), root.end ());

  if (pc == script.end ())
    return {pcHead != script.end (), false,
            "Aux POW missing chain merkle root in parent coinbase"};

  if (pcHead != script.end ())
    {
      if (script.end () != std::search (pcHead + 1, script.end (),
                                        mmHeaderBegin, mmHeaderEnd))
        return {true, false, "Multiple merged mining headers in coinbase"};
      if (pcHead + sizeof (pchMergedMiningHeader) != pc)
        return {true, false,
                "Merged mining header is not just before chain merkle root"};
    }
  else if (pc - script.begin () > 20)
    return {true, false,
            "Aux POW chain merkle root must start in the first 20 bytes of the parent coinbase"};

  pc += root.size ();
  return CheckCommitmentTail (pc, script.end (), merkleHeight,
                              chainIndex, chainId);
}

CommitmentResult
CheckOutputCommitments (const CTransaction& coinbase, const valtype& root,
                        const unsigned merkleHeight, const int chainIndex,
                        const int chainId)
{
  CommitmentResult result{false, false, {}};
  for (size_t i = 0; i < coinbase.vout.size (); ++i)
    {
      const CTxOut& output = coinbase.vout[i];
      const CScript& script = output.scriptPubKey;
      if (script.size () != 46 || script[0] != OP_RETURN || script[1] != 44
          || !std::equal (std::begin (pchMergedMiningHeader),
                          std::end (pchMergedMiningHeader), script.begin () + 2))
        continue;

      if (result.present)
        return {true, false, "Multiple merged mining outputs in coinbase"};
      result.present = true;

      if (i + 1 != coinbase.vout.size ())
        result.error = "Merged mining output is not the final coinbase output";
      else if (output.nValue != 0)
        result.error = "Merged mining output value is not zero";
      else if (!std::equal (root.begin (), root.end (), script.begin () + 6))
        result.error = "Aux POW output has the wrong chain merkle root";
      else
        {
          valtype payload (script.begin () + 2, script.end ());
          result = CheckCommitmentTail (payload.begin () + 36, payload.end (),
                                        merkleHeight, chainIndex, chainId);
        }
    }
  return result;
}

} // anonymous namespace

bool
CAuxPow::check (const uint256& hashAuxBlock, int nChainId,
                 const Consensus::Params& params,
                 bool allowOutputCommitment) const
{
    if (params.fStrictChainId) {
        const int32_t &nChainIDParent = parentBlock.GetChainId();
        if(nChainIDParent > 0) {
            if(nChainIDParent == params.nAuxpowChainId)
                return error("Aux POW parent has our chain ID");
        } else {
            const int32_t &nOldChainIDParent = parentBlock.GetOldChainId();
            if(nOldChainIDParent == params.nAuxpowOldChainId)
                return error("Aux POW parent has our old chain ID");
        }
    }

    if (vChainMerkleBranch.size() > 30)
        return error("Aux POW chain merkle branch too long");

    // Check that the chain merkle root is in the coinbase
    const uint256 &nRootHash
      = CheckMerkleBranch (hashAuxBlock, vChainMerkleBranch, nChainIndex);
    valtype vchRootHash(nRootHash.begin (), nRootHash.end ());
    std::reverse (vchRootHash.begin (), vchRootHash.end ()); // correct endian

    // Check that we are in the parent block merkle tree
    if (CheckMerkleBranch(coinbaseTx->GetHash(), vMerkleBranch, 0)
          != parentBlock.hashMerkleRoot)
        return error("Aux POW merkle root incorrect");

    // Check that there is at least one input.
    if (coinbaseTx->vin.empty())
        return error("Aux POW coinbase has no inputs");

    const unsigned &merkleHeight = vChainMerkleBranch.size ();
    const CommitmentResult input = CheckInputCommitment (
        coinbaseTx->vin[0].scriptSig, vchRootHash, merkleHeight,
        nChainIndex, nChainId);
    const CommitmentResult output = CheckOutputCommitments (
        *coinbaseTx, vchRootHash, merkleHeight, nChainIndex, nChainId);

    if (input.present && output.present)
        return error("Multiple merged mining commitment locations in coinbase");
    if (input.valid)
        return true;
    if (output.valid)
      {
        if (!allowOutputCommitment)
            return error("Aux POW coinbase output commitment is not active");
        return true;
      }
    if (input.present)
        return error("%s", input.error.c_str ());
    if (output.present)
        return error("%s", output.error.c_str ());

    return error("Aux POW missing chain merkle root in parent coinbase");
}

int
CAuxPow::getExpectedIndex (const uint32_t &nNonce, const int &nChainId,
                           const unsigned &h)
{
  // Choose a pseudo-random slot in the chain merkle tree
  // but have it be fixed for a size/nonce/chain combination.
  //
  // This prevents the same work from being used twice for the
  // same chain while reducing the chance that two chains clash
  // for the same slot.

  /* Note that h is always <= 30 (enforced by the maximum allowed chain
     merkle branch length), so that 32 bits are enough for the result.  */

  const uint32_t mod = (1u << h);

  uint64_t rand = nNonce;
  rand = rand * 1103515245 + 12345;
  rand %= mod;
  rand += nChainId;
  rand = rand * 1103515245 + 12345;
  rand %= mod;

  return rand;
}

uint256
CAuxPow::CheckMerkleBranch (uint256 hash,
                            const std::vector<uint256>& vMerkleBranch,
                            int nIndex)
{
  if (nIndex == -1)
    return uint256 ();
  for (std::vector<uint256>::const_iterator it(vMerkleBranch.begin ());
       it != vMerkleBranch.end (); ++it)
  {
    if (nIndex & 1)
      hash = Hash (*it, hash);
    else
      hash = Hash (hash, *it);
    nIndex >>= 1;
  }
  return hash;
}

std::unique_ptr<CAuxPow>
CAuxPow::createAuxPow (const CPureBlockHeader& header)
{
  assert (header.IsAuxpow ());

  /* Build a minimal coinbase script input for merge-mining.  */
  const uint256 blockHash = header.GetHash ();
  valtype inputData(blockHash.begin (), blockHash.end ());
  std::reverse (inputData.begin (), inputData.end ());
  inputData.push_back (1);
  inputData.insert (inputData.end (), 7, 0);

  /* Fake a parent-block coinbase with just the required input
     script and no outputs.  */
  CMutableTransaction coinbase;
  coinbase.vin.resize (1);
  coinbase.vin[0].prevout.SetNull ();
  coinbase.vin[0].scriptSig = (CScript () << inputData);
  assert (coinbase.vout.empty ());
  CTransactionRef coinbaseRef = MakeTransactionRef (coinbase);

  /* Build a fake parent block with the coinbase.  */
  CBlock parent;
  parent.nVersion = 1;
  parent.vtx.resize (1);
  parent.vtx[0] = coinbaseRef;
  parent.hashMerkleRoot = BlockMerkleRoot (parent);

  /* Construct the auxpow object.  */
  std::unique_ptr<CAuxPow> auxpow(new CAuxPow (std::move (coinbaseRef)));
  assert (auxpow->vMerkleBranch.empty ());
  assert (auxpow->vChainMerkleBranch.empty ());
  auxpow->nChainIndex = 0;
  auxpow->parentBlock = parent;

  return auxpow;
}

CPureBlockHeader&
CAuxPow::initAuxPow (CBlockHeader& header)
{
  /* Set auxpow flag right now, since we take the block hash below when creating
     the minimal auxpow for header.  */
  header.SetAuxpowVersion(true);

  std::unique_ptr<CAuxPow> apow = createAuxPow (header);
  CPureBlockHeader& result = apow->parentBlock;
  header.SetAuxpow (std::move (apow));

  return result;
}
