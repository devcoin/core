// From the many, one
// From one, the source.
//
#include <boost/test/unit_test.hpp>
#include <vector>

#include "coin.h"
#include "main.h"
#include "uint256.h"
#include "util.h"
#include "wallet.h"

using namespace std;

BOOST_AUTO_TEST_SUITE(coin_tests)

BOOST_AUTO_TEST_CASE(create_a_coin)
{
    // Create a mock CWalletTx to test out Coin
    //
    CWalletTx transaction = CWalletTx(); 
    
    CTxOut tx_out = CTxOut();
    tx_out.nValue = 1*CENT;
    transaction.vout.push_back(tx_out);

    unsigned int output_index = 0;
    unsigned int block_depth = 10;

    Coin coin(
        &transaction,
        output_index,
        block_depth
    );

    BOOST_CHECK_EQUAL(coin.getTransaction(), &transaction);
    BOOST_CHECK_EQUAL(coin.getOutputIndex(), output_index);
    BOOST_CHECK_EQUAL(coin.getBlockDepth(), block_depth);
    BOOST_CHECK_EQUAL(coin.getValue(), 1*CENT);
}

BOOST_AUTO_TEST_SUITE_END()
