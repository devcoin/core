// From the many, one
// From one, the source.
//
#include <boost/test/unit_test.hpp>

#include "coin.h"
#include "coinset.h"
#include "wallet.h"

// makeTestCoin returns a Coin* with the specified value in CENTs.
//
Coin *makeTestCoin(uint64 value) {
    CWalletTx *transaction = new CWalletTx(); 
    
    CTxOut tx_out;
    tx_out.nValue = value*CENT;
    transaction->vout.push_back(tx_out);

    uint output_index = 0;
    uint block_depth = 10;

    return new Coin(transaction, output_index, block_depth);
}

struct TestCoins {
    Coin *coin_one;
    Coin *coin_two;

    TestCoins() {
        coin_one = makeTestCoin(1);
        coin_two = makeTestCoin(2);
    }
};

BOOST_FIXTURE_TEST_SUITE(coinset_tests, TestCoins)

BOOST_AUTO_TEST_CASE(coinset_add_a_coin)
{
    CoinSet coins;
    coins.addCoin(coin_one);
    BOOST_CHECK_EQUAL(coins.size(), 1);
    BOOST_CHECK(coins.hasCoin(coin_one));
}

BOOST_AUTO_TEST_CASE(coinset_remove_a_coin)
{
    CoinSet coins;
    coins.addCoin(coin_one);
    coins.removeCoin(coin_one);
    BOOST_CHECK(coins.empty());
}

BOOST_AUTO_TEST_CASE(coinset_add_two_coins_then_remove_one)
{
    CoinSet coins;
    coins.addCoin(coin_one);
    coins.addCoin(coin_two);
    coins.removeCoin(coin_one);
    BOOST_CHECK_EQUAL(coins.size(), 1);
    BOOST_CHECK(!coins.hasCoin(coin_one));
    BOOST_CHECK(coins.hasCoin(coin_two));
}


BOOST_AUTO_TEST_CASE(coinset_get_value)
{
    CoinSet coins;
    BOOST_CHECK_EQUAL(coins.getValue(), 0*CENT);
    
    coins.addCoin(coin_one);
    BOOST_CHECK_EQUAL(coins.getValue(), 1*CENT);

    coins.addCoin(coin_two);
    BOOST_CHECK_EQUAL(coins.getValue(), 3*CENT);

    coins.removeCoin(coin_one);
    BOOST_CHECK_EQUAL(coins.getValue(), 2*CENT);    

    coins.removeCoin(coin_two);
    BOOST_CHECK_EQUAL(coins.getValue(), 0*CENT);    
}

BOOST_AUTO_TEST_SUITE_END()
