// From the many, one.
// From one, the source.
//
#include <boost/test/unit_test.hpp>

#include "core.h"


BOOST_AUTO_TEST_SUITE(core_tests)

BOOST_AUTO_TEST_CASE(money_range)
{
    // MoneyRange returns true for a valid amount.
    //
    int64_t amount = 1000;
    BOOST_CHECK(MoneyRange(amount));
}

BOOST_AUTO_TEST_CASE(money_range_negative)
{
    // MoneyRange returns false for negative amounts.
    //
    int64_t amount = -1000;
    BOOST_CHECK(!MoneyRange(amount));
}

BOOST_AUTO_TEST_CASE(money_range_zero)
{
    // MoneyRange returns true for zero.
    //
    int64_t amount = 0;
    BOOST_CHECK(MoneyRange(amount));
}

BOOST_AUTO_TEST_CASE(money_range_overflow)
{
    // MoneyRange returns false for amounts over MAX_MONEY
    //
    int64_t amount = MAX_MONEY + 1;
    BOOST_CHECK(!MoneyRange(amount));
}

BOOST_AUTO_TEST_SUITE_END()
