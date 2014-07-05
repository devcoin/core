Devcoin integration/staging tree
================================

http://www.devcoin.org

Copyright (c) 2011-2014 Devcoin Developers

What is Devcoin?
----------------

Devcoin is a experimental digital currency that enables instant payments to
anyone, anywhere in the world. Devcoin uses peer-to-peer technology to operate
with no central authority: managing transactions and issuing money are carried
out collectively by the network. Devcoin is also the name of the open source
software which enables the use of this currency.

It is an ethically inspired cryptocurrency created in 2011 to support open source projects by programmers, hardware developers, writers, musicians, painters, graphic artists and filmmakers worldwide.

Devcoin is merge mined with Bitcoin making it one of the longest running blockchain based digital currencies.

For more information, as well as an immediately useable, binary version of
the Devcoin client sofware, see http://www.devcoin.org.

License
-------

Devcoin is released under the terms of the MIT license. See `COPYING` for more
information or see http://opensource.org/licenses/MIT.

Development process
-------------------

Developers work in their own trees, then submit pull requests when they think
their feature or bug fix is ready.

If it is a simple/trivial/non-controversial change, then one of the Devcoin
development team members simply pulls it.

If it is a *more complicated or potentially controversial* change, then the patch
submitter will be asked to start a discussion (if they haven't already) on the Devcoin forum current at https://bitcointalk.org/index.php?topic=233997.0

The fork will be accepted if there is broad consensus that it is a good thing.
Developers should expect to rework and resubmit patches if the code doesn't
match the project's coding conventions (see `doc/coding.md`) or are
controversial.

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test. Please be patient and help out, and
remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write unit tests for new code, and to
submit new unit tests for old code.

Unit tests for the core code are in `src/test/`. To compile and run them:

    cd src; make -f makefile.unix test

Unit tests for the GUI code are in `src/qt/test/`. To compile and run them:

    qmake BITCOIN_QT_TEST=1 -o Makefile.test bitcoin-qt.pro
    make -f Makefile.test
    ./bitcoin-qt_test
