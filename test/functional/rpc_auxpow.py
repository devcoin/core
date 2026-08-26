#!/usr/bin/env python3
# Copyright (c) 2026 The Devcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test createauxblock/submitauxblock state and strict proof parsing."""

from decimal import Decimal
from pathlib import Path
import re
import struct
import time

from test_framework.messages import (
    COIN,
    CBlockHeader,
    COutPoint,
    CTransaction,
    CTxIn,
    CTxOut,
    ser_uint256,
    ser_uint256_vector,
    uint256_from_compact,
)
from test_framework.script import CScript, OP_TRUE
from test_framework.test_framework import DevcoinTestFramework
from test_framework.util import (
    assert_equal,
    assert_is_hash_string,
    assert_raises_rpc_error,
)


MERGED_MINING_HEADER = bytes.fromhex("fabe6d6d")
PARENT_VERSION = 0x20000000


def build_auxpow(candidate, hash_block=0):
    """Build one canonical single-chain CAuxPow without a tx-length prefix."""
    commitment = (
        MERGED_MINING_HEADER
        + bytes.fromhex(candidate["hash"])
        + struct.pack("<II", 1, 0)
    )

    coinbase = CTransaction()
    coinbase.vin = [
        CTxIn(
            COutPoint(0, 0xffffffff),
            scriptSig=commitment,
            nSequence=0xffffffff,
        )
    ]
    coinbase.vout = [CTxOut(0, CScript([OP_TRUE]))]
    coinbase.calc_sha256()

    parent = CBlockHeader()
    parent.nVersion = PARENT_VERSION
    parent.hashMerkleRoot = coinbase.sha256
    parent.nTime = int(time.time())
    parent.nBits = int(candidate["bits"], 16)
    target = uint256_from_compact(parent.nBits)
    parent.rehash()
    while parent.sha256 > target:
        parent.nNonce += 1
        parent.rehash()

    proof = (
        coinbase.serialize()
        + ser_uint256(hash_block)
        + ser_uint256_vector([])
        + struct.pack("<i", 0)
        + ser_uint256_vector([])
        + struct.pack("<i", 0)
        + parent.serialize()
    )
    return proof.hex()


class AuxpowRPCTest(DevcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True

    def assert_candidate(self, candidate):
        node = self.nodes[0]
        assert_equal(set(candidate), {
            "hash",
            "chainid",
            "previousblockhash",
            "coinbasevalue",
            "bits",
            "height",
            "_target",
        })
        assert_is_hash_string(candidate["hash"])
        assert_is_hash_string(candidate["previousblockhash"])
        assert_is_hash_string(candidate["_target"])
        assert re.fullmatch(r"[0-9a-f]{8}", candidate["bits"])
        assert_equal(candidate["chainid"], 8)
        assert_equal(candidate["previousblockhash"], node.getbestblockhash())
        assert_equal(candidate["height"], node.getblockcount() + 1)
        assert isinstance(candidate["coinbasevalue"], int)
        assert candidate["coinbasevalue"] > 0

    def add_mempool_transaction(self, blocks, address, private_key):
        node = self.nodes[0]
        block_hash = blocks[0]
        coinbase_txid = node.getblock(block_hash)["tx"][0]
        coinbase = node.getrawtransaction(coinbase_txid, True, block_hash)
        value = coinbase["vout"][0]["value"]
        raw = node.createrawtransaction(
            [{"txid": coinbase_txid, "vout": 0}],
            {address: value - Decimal("1")},
        )
        signed = node.signrawtransactionwithkey(raw, [private_key])
        assert signed["complete"]
        return node.sendrawtransaction(signed["hex"], 0)

    def run_test(self):
        node = self.nodes[0]
        deterministic_key = node.get_deterministic_priv_key()
        payout = deterministic_key.address

        receiver_text = f"Coin,{payout}\n"
        # The receiver loader's data-dir conversion is not Unicode-safe, so
        # also seed its working-directory fallback used by the test runner.
        receiver_files = (
            Path(node.datadir) / "receiver" / "receiver_0.csv",
            Path(node.cwd) / "receiver_0.csv",
        )
        for receiver_file in receiver_files:
            receiver_file.parent.mkdir(parents=True, exist_ok=True)
            receiver_file.write_text(receiver_text, encoding="ascii")

        blocks = node.generatetoaddress(101, payout)

        self.log.info("Check candidate shape and strict RPC parsing")
        first = node.createauxblock(payout)
        self.assert_candidate(first)
        first_proof = build_auxpow(first)

        assert_raises_rpc_error(
            -8, "hash must be of length 64", node.submitauxblock, "00", first_proof
        )
        assert_raises_rpc_error(
            -8,
            "hash must be hexadecimal string",
            node.submitauxblock,
            "z" * 64,
            first_proof,
        )
        assert_raises_rpc_error(
            -8, "block hash unknown", node.submitauxblock, "0" * 64, first_proof
        )

        for malformed in ("", "0", "zz"):
            assert_raises_rpc_error(
                -8,
                "auxpow must be hexadecimal string",
                node.submitauxblock,
                first["hash"],
                malformed,
            )
        assert_raises_rpc_error(
            -22,
            "AuxPoW decode failed",
            node.submitauxblock,
            first["hash"],
            first_proof[:-2],
        )
        assert_raises_rpc_error(
            -22,
            "AuxPoW decode failed",
            node.submitauxblock,
            first["hash"],
            first_proof + "00",
        )

        self.log.info("Refresh the current same-tip candidate and retain late work")
        txid = self.add_mempool_transaction(
            blocks, payout, deterministic_key.key
        )
        assert txid in node.getrawmempool()
        node.setmocktime(int(time.time()) + 120)
        refreshed = node.createauxblock(payout)
        repeated = node.createauxblock(payout)
        assert refreshed["hash"] != first["hash"]
        assert_equal(refreshed["coinbasevalue"], first["coinbasevalue"] + COIN)
        assert_equal(repeated, refreshed)

        assert node.submitauxblock(first["hash"].upper(), first_proof)
        assert_equal(node.getbestblockhash(), first["hash"])
        assert_equal(node.getblockcount(), first["height"])
        node.setmocktime(0)

        self.log.info("Restart invalidates candidates, then fresh submission succeeds")
        stale = node.createauxblock(payout)
        stale_proof = build_auxpow(stale)
        self.restart_node(0)
        node = self.nodes[0]
        assert_raises_rpc_error(
            -8,
            "block hash unknown",
            node.submitauxblock,
            stale["hash"],
            stale_proof,
        )

        fresh = node.createauxblock(payout)
        self.assert_candidate(fresh)
        # hashBlock is a legacy field that consensus intentionally ignores.
        fresh_proof = build_auxpow(fresh, hash_block=1)
        assert node.submitauxblock(fresh["hash"], fresh_proof)
        assert_equal(node.getbestblockhash(), fresh["hash"])
        assert_equal(node.getblockcount(), fresh["height"])


if __name__ == "__main__":
    AuxpowRPCTest().main()
