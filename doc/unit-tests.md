Compiling/running devcoind unit tests
-------------------------------------

devcoind unit tests are in the `src/test/` directory; they
use the Boost::Test unit-testing framework.

To compile and run the tests:

	`cd src`
	`make -f [your system's makefile] test_devcoin`
	`./test_devcoin # Runs the unit tests`

If all tests succeed the last line of output will be:
    `*** No errors detected`

To run a specific suite of tests use the `--run_test` option:
    `./test_devcoin --log_level=test_suite --run_test=[suite name usually the file name]`
For example:
    `./test_devcoin --log_level=test_suite --run_test=coin_tests`

To add more tests, add `BOOST_AUTO_TEST_CASE` functions to the existing
.cpp files in the test/ directory or add new .cpp files that
implement new BOOST_AUTO_TEST_SUITE sections (the makefiles are
set up to add test/*.cpp to test_devcoin automatically).


@belovachap (July 10, 2014) We need to update the next section as well
----------------------------------------------------------------------
Compiling/running Bitcoin-Qt unit tests
---------------------------------------

Bitcoin-Qt unit tests are in the src/qt/test/ directory; they
use the Qt unit-testing framework.

To compile and run the tests:

	qmake bitcoin-qt.pro BITCOIN_QT_TEST=1
	make
	./bitcoin-qt_test

To add more tests, add them to the `src/qt/test/` directory,
the `src/qt/test/test_main.cpp` file, and bitcoin-qt.pro.
