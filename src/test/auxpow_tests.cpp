// Copyright (c) 2014-2026 The Devcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arith_uint256.h>
#include <auxpow.h>
#include <chainparams.h>
#include <consensus/merkle.h>
#include <devcoin.h>
#include <hash.h>
#include <pow.h>
#include <primitives/block.h>
#include <rpc/auxpow_miner.h>
#include <script/script.h>
#include <streams.h>
#include <test/data/mainnet_auxpow_block_496000.json.h>
#include <test/util/setup_common.h>
#include <util/strencodings.h>
#include <util/time.h>
#include <validation.h>
#include <version.h>

#include <boost/test/unit_test.hpp>

#include <univalue.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

UniValue read_json(const std::string& jsondata);

BOOST_AUTO_TEST_SUITE(auxpow_tests)

class CAuxPowForTest : public CAuxPow
{
public:
    CAuxPowForTest() = default;

    explicit CAuxPowForTest(CTransactionRef tx) : CAuxPow(std::move(tx)) {}

    using CAuxPow::CheckMerkleBranch;
    using CAuxPow::coinbaseTx;
    using CAuxPow::nChainIndex;
    using CAuxPow::parentBlock;
    using CAuxPow::vChainMerkleBranch;
    using CAuxPow::vMerkleBranch;

    static size_t ParentBranchSize(const CAuxPow& pow) { return pow.vMerkleBranch.size(); }
    static size_t ChainBranchSize(const CAuxPow& pow) { return pow.vChainMerkleBranch.size(); }
    static int ChainIndex(const CAuxPow& pow) { return pow.nChainIndex; }
};

namespace {

void AppendLE32(valtype& data, uint32_t value)
{
    for (unsigned i = 0; i < 4; ++i) {
        data.push_back(value & 0xff);
        value >>= 8;
    }
}

std::vector<uint256> ComputeMerkleBranch(std::vector<uint256> hashes, size_t index)
{
    std::vector<uint256> branch;
    while (hashes.size() > 1) {
        const size_t sibling = std::min(index ^ 1, hashes.size() - 1);
        branch.push_back(hashes[sibling]);
        if (hashes.size() & 1) hashes.push_back(hashes.back());

        std::vector<uint256> next;
        next.reserve(hashes.size() / 2);
        for (size_t i = 0; i < hashes.size(); i += 2) {
            next.push_back(Hash(hashes[i], hashes[i + 1]));
        }
        hashes = std::move(next);
        index >>= 1;
    }
    return branch;
}

valtype BuildCommitment(const valtype& root, unsigned height, uint32_t nonce,
                        bool include_marker = true)
{
    valtype data;
    if (include_marker) {
        data.insert(data.end(), std::begin(pchMergedMiningHeader),
                    std::end(pchMergedMiningHeader));
    }
    data.insert(data.end(), root.begin(), root.end());
    AppendLE32(data, 1u << height);
    AppendLE32(data, nonce);
    return data;
}

struct BuiltAuxpow {
    std::unique_ptr<CAuxPowForTest> pow;
    valtype root;
    valtype commitment;
};

BuiltAuxpow BuildAuxpow(const uint256& child_hash, int chain_id,
                        unsigned chain_height = 0, int chain_index = -1,
                        uint32_t nonce = 7, int32_t parent_version = 0x20000000,
                        bool witness = false, bool parent_branch = false)
{
    std::vector<uint256> chain_branch;
    for (unsigned i = 0; i < chain_height; ++i) {
        chain_branch.push_back(ArithToUint256(arith_uint256(i + 1)));
    }
    if (chain_index < 0) {
        chain_index = chain_height <= 30
            ? CAuxPow::getExpectedIndex(nonce, chain_id, chain_height)
            : 0;
    }

    const uint256 root_hash = CAuxPowForTest::CheckMerkleBranch(
        child_hash, chain_branch, chain_index);
    valtype root(root_hash.begin(), root_hash.end());
    std::reverse(root.begin(), root.end());
    valtype commitment = BuildCommitment(root, chain_height, nonce);

    CMutableTransaction coinbase;
    coinbase.vin.resize(1);
    coinbase.vin[0].prevout.SetNull();
    coinbase.vin[0].scriptSig = CScript(commitment.begin(), commitment.end());
    if (witness) {
        coinbase.vin[0].scriptWitness.stack.push_back({0x01, 0x02});
    }
    coinbase.vout.emplace_back(0, CScript() << OP_TRUE);
    CTransactionRef coinbase_ref = MakeTransactionRef(std::move(coinbase));

    CBlock parent;
    parent.nVersion = parent_version;
    parent.nTime = 1;
    parent.nBits = 0x207fffff;
    parent.vtx.push_back(coinbase_ref);
    if (parent_branch) {
        CMutableTransaction other;
        other.nLockTime = 1;
        other.vin.resize(1);
        other.vin[0].prevout.hash = ArithToUint256(arith_uint256(42));
        other.vout.emplace_back(0, CScript() << OP_TRUE);
        parent.vtx.push_back(MakeTransactionRef(std::move(other)));
    }
    parent.hashMerkleRoot = BlockMerkleRoot(parent);

    std::vector<uint256> parent_hashes;
    for (const auto& tx : parent.vtx) parent_hashes.push_back(tx->GetHash());

    auto result = std::make_unique<CAuxPowForTest>(coinbase_ref);
    result->vMerkleBranch = ComputeMerkleBranch(std::move(parent_hashes), 0);
    result->vChainMerkleBranch = std::move(chain_branch);
    result->nChainIndex = chain_index;
    result->parentBlock = parent;

    return {std::move(result), std::move(root), std::move(commitment)};
}

void SetCoinbaseScript(BuiltAuxpow& built, const valtype& data)
{
    CMutableTransaction coinbase(*built.pow->coinbaseTx);
    coinbase.vin[0].scriptSig = CScript(data.begin(), data.end());
    built.pow->coinbaseTx = MakeTransactionRef(std::move(coinbase));
    built.pow->parentBlock.hashMerkleRoot = CAuxPowForTest::CheckMerkleBranch(
        built.pow->coinbaseTx->GetHash(), built.pow->vMerkleBranch, 0);
}

void SetCoinbaseOutput(BuiltAuxpow& built, const valtype& data,
                       CAmount value = 0, bool clear_input = true,
                       bool final_output = true)
{
    CMutableTransaction coinbase(*built.pow->coinbaseTx);
    if (clear_input) coinbase.vin[0].scriptSig = CScript() << OP_TRUE << OP_TRUE;

    const CTxOut output(value, CScript() << OP_RETURN << data);
    if (final_output) {
        coinbase.vout.push_back(output);
    } else {
        coinbase.vout.insert(coinbase.vout.begin(), output);
    }
    built.pow->coinbaseTx = MakeTransactionRef(std::move(coinbase));
    built.pow->parentBlock.hashMerkleRoot = CAuxPowForTest::CheckMerkleBranch(
        built.pow->coinbaseTx->GetHash(), built.pow->vMerkleBranch, 0);
}

void Tamper(uint256& value)
{
    arith_uint256 modified = UintToArith256(value);
    modified += 1;
    value = ArithToUint256(modified);
}

std::vector<unsigned char> SerializeAuxpow(const CAuxPow& pow)
{
    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    stream << pow;
    return {stream.begin(), stream.end()};
}

void AdvanceTip(NodeContext& node, const CScript& script_pub_key)
{
    CTxMemPool empty_pool;
    std::unique_ptr<CBlockTemplate> block_template = BlockAssembler(
        node.chainman->ActiveChainstate(), empty_pool, Params()).CreateNewBlock(script_pub_key);
    BOOST_REQUIRE(block_template);

    CBlock& block = block_template->block;
    block.hashMerkleRoot = BlockMerkleRoot(block);
    const auto& params = Params().GetConsensus();
    while (!CheckProofOfWork(block.GetHash(), block.nBits, params)) ++block.nNonce;

    const auto shared_block = std::make_shared<const CBlock>(block);
    BOOST_REQUIRE(node.chainman->ProcessNewBlock(Params(), shared_block, true, nullptr));
}

} // namespace

BOOST_FIXTURE_TEST_CASE(valid_auxpow_formats, BasicTestingSetup)
{
    const auto& params = Params().GetConsensus();
    const int chain_id = params.nAuxpowChainId;
    const uint256 child_hash = uint256S(
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");

    auto single = BuildAuxpow(child_hash, chain_id);
    BOOST_CHECK(single.pow->check(child_hash, chain_id, params));
    BOOST_CHECK(single.pow->vChainMerkleBranch.empty());
    BOOST_CHECK_EQUAL(single.pow->nChainIndex, 0);

    const auto displayed_hash = ParseHex(child_hash.GetHex());
    const auto hash_pos = std::search(single.commitment.begin(), single.commitment.end(),
                                      displayed_hash.begin(), displayed_hash.end());
    BOOST_REQUIRE(hash_pos != single.commitment.end());
    BOOST_CHECK_EQUAL(std::distance(single.commitment.begin(), hash_pos),
                      sizeof(pchMergedMiningHeader));

    valtype legacy = BuildCommitment(single.root, 0, 7, false);
    SetCoinbaseScript(single, legacy);
    BOOST_CHECK(single.pow->check(child_hash, chain_id, params));

    constexpr unsigned height = 3;
    constexpr uint32_t nonce = 11;
    const int expected_index = CAuxPow::getExpectedIndex(nonce, chain_id, height);
    auto extended = BuildAuxpow(child_hash, chain_id, height, expected_index,
                                nonce, 0x20000000, true, true);
    BOOST_CHECK(extended.pow->check(child_hash, chain_id, params));
    BOOST_CHECK(extended.pow->coinbaseTx->HasWitness());
    BOOST_CHECK_EQUAL(extended.pow->vMerkleBranch.size(), 1U);
    BOOST_CHECK_EQUAL(extended.pow->vChainMerkleBranch.size(), height);
    BOOST_CHECK_EQUAL(extended.pow->nChainIndex, expected_index);
    BOOST_CHECK_EQUAL(extended.pow->parentBlock.nVersion, 0x20000000);
}

BOOST_FIXTURE_TEST_CASE(output_commitment_formats, BasicTestingSetup)
{
    const auto& params = Params().GetConsensus();
    const int chain_id = params.nAuxpowChainId;
    const uint256 child_hash = uint256S(
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");

    auto canonical = BuildAuxpow(child_hash, chain_id);
    SetCoinbaseOutput(canonical, canonical.commitment);
    BOOST_REQUIRE_EQUAL(canonical.pow->coinbaseTx->vout.back().scriptPubKey.size(), 46U);
    BOOST_CHECK_EQUAL(HexStr(canonical.pow->coinbaseTx->vout.back().scriptPubKey),
                      "6a2c" + HexStr(canonical.commitment));
    CDataStream output_stream(SER_NETWORK, PROTOCOL_VERSION);
    output_stream << canonical.pow->coinbaseTx->vout.back();
    BOOST_CHECK_EQUAL(output_stream.size(), 55U);
    BOOST_CHECK(canonical.pow->check(child_hash, chain_id, params));
    BOOST_CHECK(!canonical.pow->check(child_hash, chain_id, params, false));

    auto legacy = BuildAuxpow(child_hash, chain_id);
    BOOST_CHECK(legacy.pow->check(child_hash, chain_id, params, false));

    auto duplicate = BuildAuxpow(child_hash, chain_id);
    SetCoinbaseOutput(duplicate, duplicate.commitment, 0, false);
    BOOST_CHECK(!duplicate.pow->check(child_hash, chain_id, params));

    auto nonzero = BuildAuxpow(child_hash, chain_id);
    SetCoinbaseOutput(nonzero, nonzero.commitment, 1);
    BOOST_CHECK(!nonzero.pow->check(child_hash, chain_id, params));

    auto nonfinal = BuildAuxpow(child_hash, chain_id);
    SetCoinbaseOutput(nonfinal, nonfinal.commitment, 0, true, false);
    BOOST_CHECK(!nonfinal.pow->check(child_hash, chain_id, params));

    auto malformed_push = BuildAuxpow(child_hash, chain_id);
    SetCoinbaseOutput(malformed_push, malformed_push.commitment);
    CMutableTransaction malformed_tx(*malformed_push.pow->coinbaseTx);
    malformed_tx.vout.back().scriptPubKey[1] = OP_PUSHDATA1;
    malformed_push.pow->coinbaseTx = MakeTransactionRef(std::move(malformed_tx));
    malformed_push.pow->parentBlock.hashMerkleRoot =
        CAuxPowForTest::CheckMerkleBranch(malformed_push.pow->coinbaseTx->GetHash(),
                                         malformed_push.pow->vMerkleBranch, 0);
    BOOST_CHECK(!malformed_push.pow->check(child_hash, chain_id, params));

    auto wrong_root = BuildAuxpow(child_hash, chain_id);
    valtype wrong_root_data = wrong_root.commitment;
    wrong_root_data[sizeof(pchMergedMiningHeader)] ^= 1;
    SetCoinbaseOutput(wrong_root, wrong_root_data);
    BOOST_CHECK(!wrong_root.pow->check(child_hash, chain_id, params));

    auto wrong_size = BuildAuxpow(child_hash, chain_id);
    valtype wrong_size_data = wrong_size.commitment;
    wrong_size_data[sizeof(pchMergedMiningHeader) + uint256::size()] ^= 1;
    SetCoinbaseOutput(wrong_size, wrong_size_data);
    BOOST_CHECK(!wrong_size.pow->check(child_hash, chain_id, params));

    constexpr unsigned height = 3;
    constexpr uint32_t nonce = 7;
    auto wrong_nonce = BuildAuxpow(child_hash, chain_id, height, -1, nonce);
    valtype wrong_nonce_data = wrong_nonce.commitment;
    uint32_t replacement = nonce + 1;
    while (CAuxPow::getExpectedIndex(replacement, chain_id, height)
           == wrong_nonce.pow->nChainIndex) {
        ++replacement;
    }
    const size_t nonce_pos = sizeof(pchMergedMiningHeader) + uint256::size() + 4;
    for (unsigned i = 0; i < 4; ++i) {
        wrong_nonce_data[nonce_pos + i] = (replacement >> (8 * i)) & 0xff;
    }
    SetCoinbaseOutput(wrong_nonce, wrong_nonce_data);
    BOOST_CHECK(!wrong_nonce.pow->check(child_hash, chain_id, params));
}

BOOST_FIXTURE_TEST_CASE(output_commitment_activation_parameters, BasicTestingSetup)
{
    const auto main = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    const auto testnet = CreateChainParams(*m_node.args, CBaseChainParams::TESTNET);
    const auto signet = CreateChainParams(*m_node.args, CBaseChainParams::SIGNET);
    const auto regtest = CreateChainParams(*m_node.args, CBaseChainParams::REGTEST);

    BOOST_CHECK_EQUAL(main->GetConsensus().nAuxpowOutputCommitmentHeight,
                      std::numeric_limits<int>::max());
    BOOST_CHECK_EQUAL(testnet->GetConsensus().nAuxpowOutputCommitmentHeight, 0);
    BOOST_CHECK_EQUAL(signet->GetConsensus().nAuxpowOutputCommitmentHeight, 0);
    BOOST_CHECK_EQUAL(regtest->GetConsensus().nAuxpowOutputCommitmentHeight, 0);

    Consensus::Params synthetic = main->GetConsensus();
    synthetic.nAuxpowOutputCommitmentHeight = 100;

    CBlockHeader output_block;
    output_block.SetBaseVersion(1, synthetic.nAuxpowChainId);
    output_block.SetAuxpowVersion(true);
    output_block.nBits = 0x207fffff;
    auto output = BuildAuxpow(output_block.GetHash(), synthetic.nAuxpowChainId);
    SetCoinbaseOutput(output, output.commitment);
    output_block.SetAuxpow(std::move(output.pow));

    BOOST_CHECK(!CheckAuxPowCommitment(output_block, 99, synthetic));
    BOOST_CHECK(CheckAuxPowCommitment(output_block, 100, synthetic));
}

BOOST_FIXTURE_TEST_CASE(historical_mainnet_auxpow, BasicTestingSetup)
{
    const UniValue fixtures = read_json(std::string(
        json_tests::mainnet_auxpow_block_496000,
        json_tests::mainnet_auxpow_block_496000
            + sizeof(json_tests::mainnet_auxpow_block_496000)));
    BOOST_REQUIRE_EQUAL(fixtures.size(), 1U);
    const UniValue& fixture = fixtures[0];
    const std::vector<unsigned char> raw = ParseHex(fixture["raw"].get_str());
    BOOST_REQUIRE_EQUAL(raw.size(), 1431U);

    CDataStream input(raw, SER_NETWORK, PROTOCOL_VERSION);
    CBlock block;
    input >> block;
    BOOST_REQUIRE(input.empty());
    BOOST_REQUIRE(block.auxpow);

    const auto main = CreateChainParams(*m_node.args, CBaseChainParams::MAIN);
    const auto& consensus = main->GetConsensus();
    const int height = fixture["height"].get_int();

    BOOST_CHECK_EQUAL(height, 496000);
    BOOST_CHECK_EQUAL(block.GetHash().GetHex(), fixture["hash"].get_str());
    BOOST_CHECK_EQUAL(block.GetChainId(), consensus.nAuxpowChainId);
    BOOST_CHECK_EQUAL(consensus.nAuxpowOutputCommitmentHeight,
                      std::numeric_limits<int>::max());
    BOOST_CHECK_LT(height, consensus.nAuxpowOutputCommitmentHeight);
    BOOST_CHECK_EQUAL(CAuxPowForTest::ParentBranchSize(*block.auxpow), 9U);
    BOOST_CHECK_EQUAL(CAuxPowForTest::ChainBranchSize(*block.auxpow), 5U);
    BOOST_CHECK_EQUAL(CAuxPowForTest::ChainIndex(*block.auxpow), 18);
    BOOST_CHECK(CheckAuxPowCommitment(block, height, consensus));
    BOOST_CHECK(block.auxpow->check(block.GetHash(), block.GetChainId(),
                                    consensus, false));
    BOOST_CHECK(CheckProofOfWork(block, consensus));

    const CAmount historical_reward = block.vtx[0]->GetValueOut();
    BOOST_CHECK_GT(historical_reward, minerSubsidy);
    BOOST_CHECK_LE(historical_reward, GetBlockSubsidy(height, consensus));

    BlockValidationState state;
    BOOST_CHECK(CheckBlock(block, state, consensus, true, true));

    CDataStream output(SER_NETWORK, PROTOCOL_VERSION);
    output << block;
    BOOST_CHECK_EQUAL_COLLECTIONS(raw.begin(), raw.end(), output.begin(), output.end());
}

BOOST_FIXTURE_TEST_CASE(auxpow_serialization, BasicTestingSetup)
{
    const auto& params = Params().GetConsensus();
    const int chain_id = params.nAuxpowChainId;
    const uint256 child_hash = ArithToUint256(arith_uint256(12345));
    auto built = BuildAuxpow(child_hash, chain_id, 2, -1, 19,
                             0x20000000, true, true);

    const std::vector<unsigned char> encoded = SerializeAuxpow(*built.pow);
    CDataStream input(encoded, SER_NETWORK, PROTOCOL_VERSION);
    CAuxPowForTest decoded;
    input >> decoded;
    BOOST_CHECK(input.empty());
    BOOST_CHECK(decoded.coinbaseTx->HasWitness());
    BOOST_CHECK(decoded.check(child_hash, chain_id, params));
    const std::vector<unsigned char> reencoded = SerializeAuxpow(decoded);
    BOOST_CHECK_EQUAL_COLLECTIONS(encoded.begin(), encoded.end(),
                                  reencoded.begin(), reencoded.end());

    CDataStream tx_stream(SER_NETWORK, PROTOCOL_VERSION);
    tx_stream << built.pow->coinbaseTx;
    std::vector<unsigned char> nonzero_hash_block = encoded;
    const size_t hash_block_pos = tx_stream.size();
    BOOST_REQUIRE(nonzero_hash_block.size() >= hash_block_pos + uint256::size());
    std::fill_n(nonzero_hash_block.begin() + hash_block_pos,
                uint256::size(), 0x5a);

    CDataStream nonzero_input(nonzero_hash_block, SER_NETWORK, PROTOCOL_VERSION);
    CAuxPowForTest nonzero_decoded;
    nonzero_input >> nonzero_decoded;
    BOOST_CHECK(nonzero_input.empty());
    BOOST_CHECK(nonzero_decoded.check(child_hash, chain_id, params));
}

BOOST_FIXTURE_TEST_CASE(invalid_auxpow_matrix, BasicTestingSetup)
{
    const auto& params = Params().GetConsensus();
    const int chain_id = params.nAuxpowChainId;
    const uint256 child_hash = ArithToUint256(arith_uint256(54321));
    constexpr unsigned height = 3;
    constexpr uint32_t nonce = 23;

    enum class InvalidCase {
        MISSING_ROOT,
        MISSING_MARKER,
        DUPLICATE_MARKER,
        DETACHED_MARKER,
        MISSING_SIZE,
        MISSING_NONCE,
        WRONG_TREE_SIZE,
        WRONG_INDEX,
        PARENT_MERKLE_MISMATCH,
        BRANCH_TOO_LONG,
        PARENT_CHAIN_ID,
    };
    const std::array<std::pair<const char*, InvalidCase>, 11> cases{{
        {"missing child root", InvalidCase::MISSING_ROOT},
        {"missing marker with late root", InvalidCase::MISSING_MARKER},
        {"duplicate marker", InvalidCase::DUPLICATE_MARKER},
        {"detached marker", InvalidCase::DETACHED_MARKER},
        {"missing tree size", InvalidCase::MISSING_SIZE},
        {"missing tree nonce", InvalidCase::MISSING_NONCE},
        {"wrong tree size", InvalidCase::WRONG_TREE_SIZE},
        {"wrong deterministic index", InvalidCase::WRONG_INDEX},
        {"parent merkle mismatch", InvalidCase::PARENT_MERKLE_MISMATCH},
        {"chain branch over 30", InvalidCase::BRANCH_TOO_LONG},
        {"conflicting parent chain ID", InvalidCase::PARENT_CHAIN_ID},
    }};

    for (const auto& test : cases) {
        auto built = BuildAuxpow(child_hash, chain_id, height, -1, nonce);
        switch (test.second) {
        case InvalidCase::MISSING_ROOT: {
            valtype data = built.commitment;
            data.erase(data.begin() + sizeof(pchMergedMiningHeader),
                       data.begin() + sizeof(pchMergedMiningHeader) + uint256::size());
            SetCoinbaseScript(built, data);
            break;
        }
        case InvalidCase::MISSING_MARKER: {
            valtype data(21, OP_TRUE);
            const valtype suffix = BuildCommitment(built.root, height, nonce, false);
            data.insert(data.end(), suffix.begin(), suffix.end());
            SetCoinbaseScript(built, data);
            break;
        }
        case InvalidCase::DUPLICATE_MARKER: {
            valtype data = built.commitment;
            data.insert(data.end(), std::begin(pchMergedMiningHeader),
                        std::end(pchMergedMiningHeader));
            SetCoinbaseScript(built, data);
            break;
        }
        case InvalidCase::DETACHED_MARKER: {
            valtype data = built.commitment;
            data.insert(data.begin() + sizeof(pchMergedMiningHeader), 0x00);
            SetCoinbaseScript(built, data);
            break;
        }
        case InvalidCase::MISSING_SIZE: {
            valtype data = built.commitment;
            data.resize(sizeof(pchMergedMiningHeader) + uint256::size());
            SetCoinbaseScript(built, data);
            break;
        }
        case InvalidCase::MISSING_NONCE: {
            valtype data = built.commitment;
            data.resize(sizeof(pchMergedMiningHeader) + uint256::size() + 4);
            SetCoinbaseScript(built, data);
            break;
        }
        case InvalidCase::WRONG_TREE_SIZE: {
            valtype data = built.commitment;
            const size_t size_pos = sizeof(pchMergedMiningHeader) + uint256::size();
            data[size_pos] ^= 1;
            SetCoinbaseScript(built, data);
            break;
        }
        case InvalidCase::WRONG_INDEX: {
            const int expected = CAuxPow::getExpectedIndex(nonce, chain_id, height);
            built = BuildAuxpow(child_hash, chain_id, height, expected ^ 1, nonce);
            break;
        }
        case InvalidCase::PARENT_MERKLE_MISMATCH:
            Tamper(built.pow->parentBlock.hashMerkleRoot);
            break;
        case InvalidCase::BRANCH_TOO_LONG:
            built = BuildAuxpow(child_hash, chain_id, 31, 0, nonce);
            break;
        case InvalidCase::PARENT_CHAIN_ID:
            built.pow->parentBlock.nVersion = (chain_id << 16) | (1 << 8) | 1;
            break;
        }

        BOOST_CHECK_MESSAGE(!built.pow->check(child_hash, chain_id, params),
                            test.first);
    }
}

BOOST_FIXTURE_TEST_CASE(parent_hash_target, RegTestingSetup)
{
    const auto& params = Params().GetConsensus();
    const int chain_id = params.nAuxpowChainId;
    const arith_uint256 target = UintToArith256(params.powLimit);

    CBlockHeader child;
    child.SetBaseVersion(1, chain_id);
    child.SetAuxpowVersion(true);
    child.nBits = target.GetCompact();
    child.nTime = 1;

    auto high = BuildAuxpow(child.GetHash(), chain_id);
    while (UintToArith256(high.pow->parentBlock.GetHash()) <= target) {
        ++high.pow->parentBlock.nNonce;
    }
    std::unique_ptr<CAuxPow> high_pow = std::move(high.pow);
    child.SetAuxpow(std::move(high_pow));
    BOOST_CHECK(!CheckProofOfWork(child, params));

    auto solved = BuildAuxpow(child.GetHash(), chain_id);
    while (UintToArith256(solved.pow->parentBlock.GetHash()) > target) {
        ++solved.pow->parentBlock.nNonce;
    }
    std::unique_ptr<CAuxPow> solved_pow = std::move(solved.pow);
    child.SetAuxpow(std::move(solved_pow));
    BOOST_CHECK(CheckProofOfWork(child, params));
}

class AuxpowMinerForTest : public AuxpowMiner
{
private:
    const NodeContext& node;

public:
    explicit AuxpowMinerForTest(const NodeContext& node_in) : node(node_in) {}

    using AuxpowMiner::cs;
    using AuxpowMiner::lookupSavedBlock;
    using AuxpowMiner::startTime;
    using AuxpowMiner::txUpdatedLast;

    const CBlock* getCurrentBlock(const CScript& script, uint256& target)
    {
        return AuxpowMiner::getCurrentBlock(*node.chainman, *node.mempool,
                                            script, target);
    }
};

BOOST_FIXTURE_TEST_CASE(candidate_refresh_and_lifetime, RegTestingSetup)
{
    AuxpowMinerForTest miner(m_node);
    LOCK(miner.cs);

    BOOST_CHECK_EQUAL(miner.txUpdatedLast, 0U);
    BOOST_CHECK_EQUAL(miner.startTime, 0U);

    const int64_t base_time =
        m_node.chainman->ActiveChain().Tip()->GetMedianTimePast() + 1;
    SetMockTime(base_time);

    CScript script_pub_key;
    uint256 target;
    const CBlock* first = miner.getCurrentBlock(script_pub_key, target);
    const uint256 first_hash = first->GetHash();

    arith_uint256 expected_target;
    expected_target.SetCompact(first->nBits);
    BOOST_CHECK(target == ArithToUint256(expected_target));

    SetMockTime(base_time + 100);
    BOOST_CHECK(miner.getCurrentBlock(script_pub_key, target) == first);

    AdvanceTip(m_node, script_pub_key);
    const CBlock* second = miner.getCurrentBlock(script_pub_key, target);
    const uint256 second_hash = second->GetHash();
    BOOST_CHECK(second_hash != first_hash);
    BOOST_CHECK_THROW(miner.lookupSavedBlock(first_hash), UniValue);

    m_node.mempool->AddTransactionsUpdated(1);

    SetMockTime(base_time + 160);
    BOOST_CHECK(miner.getCurrentBlock(script_pub_key, target) == second);

    SetMockTime(base_time + 161);
    const CBlock* refreshed = miner.getCurrentBlock(script_pub_key, target);
    const uint256 refreshed_hash = refreshed->GetHash();
    BOOST_CHECK(refreshed != second);
    BOOST_CHECK(refreshed_hash != second_hash);
    BOOST_CHECK(miner.getCurrentBlock(script_pub_key, target) == refreshed);
    BOOST_CHECK(miner.lookupSavedBlock(second_hash) == second);
    BOOST_CHECK(miner.lookupSavedBlock(refreshed_hash) == refreshed);

    AdvanceTip(m_node, script_pub_key);
    const CBlock* next_tip = miner.getCurrentBlock(script_pub_key, target);
    BOOST_CHECK(next_tip->GetHash() != refreshed_hash);
    BOOST_CHECK_THROW(miner.lookupSavedBlock(second_hash), UniValue);
    BOOST_CHECK_THROW(miner.lookupSavedBlock(refreshed_hash), UniValue);

    SetMockTime(0);
}

BOOST_AUTO_TEST_SUITE_END()
