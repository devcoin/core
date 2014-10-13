// From the many, one.
// From one, the source.
//
#include <boost/test/unit_test.hpp>

#include "core.h"
#include "main.h"


BOOST_AUTO_TEST_SUITE(main_tests)

BOOST_AUTO_TEST_CASE(get_block_value)
{
    // GetBlockValue returns a constant amount regardless of block height.
    //
    uint64_t random_height = 1234;
    int64_t fees = 0;
    BOOST_CHECK_EQUAL(GetBlockValue(random_height, fees), initialSubsidy);
}

BOOST_AUTO_TEST_CASE(get_block_value_with_fees)
{
    // GetBlockValue adds any fees passed to the block value.
    //
    uint64_t random_height = 567890;
    int64_t fees = 1000;
    BOOST_CHECK_EQUAL(
        GetBlockValue(random_height, fees),
        initialSubsidy + fees
    );
}

BOOST_AUTO_TEST_SUITE_END()
