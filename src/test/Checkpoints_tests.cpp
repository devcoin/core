//
// Unit tests for block-chain checkpoints
//
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

#include "../checkpoints.h"
#include "../util.h"

using namespace std;

struct CheckPointHashes {
    uint256 block_8900_hash; 
    uint256 block_67720_hash;
    int last_checkpoint_block;

    CheckPointHashes() {
        block_8900_hash = uint256("0x00000000001bb8090630fcabb82ad0ab75df3eb5b008956b3ae2a352a4324f19");
        block_67720_hash = uint256("0x0a111b265d89f77b4c86fa6f44e3e2ad876547b1eccf19319cde922b42c1161e");
        last_checkpoint_block = 145262;
    }
};

BOOST_FIXTURE_TEST_SUITE(checkpoints_tests, CheckPointHashes)

BOOST_AUTO_TEST_CASE(hash_of_checkpoint_block_at_checkpoint_passes)
{
    BOOST_CHECK(Checkpoints::CheckBlock(8900, block_8900_hash));
    BOOST_CHECK(Checkpoints::CheckBlock(67720, block_67720_hash));
}

BOOST_AUTO_TEST_CASE(wrong_hash_at_checkpoint_fails)
{   
    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckBlock(8900, block_67720_hash));
    BOOST_CHECK(!Checkpoints::CheckBlock(67720, block_8900_hash));
}

BOOST_AUTO_TEST_CASE(any_hash_at_not_checkpoint_passes)
{
    // ... but any hash not at a checkpoint should pass:
    BOOST_CHECK(Checkpoints::CheckBlock(8900+1, block_67720_hash));
    BOOST_CHECK(Checkpoints::CheckBlock(67720+1, block_8900_hash));
}

BOOST_AUTO_TEST_CASE(total_blocks_estimate_at_least_last_checkpoint)
{
    // We should never produce an estimate less than the last bheckpoint block
    // height.
    BOOST_CHECK(Checkpoints::GetTotalBlocksEstimate() >= last_checkpoint_block);
}    

BOOST_AUTO_TEST_SUITE_END()
