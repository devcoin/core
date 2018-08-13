Devcoin core (devcoind)
=======================

https://www.devcoin.org/

Devcoin Developers © since 2011

What is Devcoin?
----------------



Devcoin ("DVC") is one of the longest running blockchain based digital currencies.

Devcoin is an ethically inspired cryptocurrency created in 2011 to support open 
source projects by programmers, hardware developers, writers, musicians, 
painters, graphic artists and filmmakers worldwide. This is the first 
cryptocurrency that is expressly made to compensate people for their open source 
and Creative Commons work.

Devcoin is merge mined with Bitcoin. As the auxiliary chain in merged mining,
Devcoin receives increased hashing power from the Bitcoin parent chain in
addition to the hashing power of the dedicated Devcoin mining network.

Devcoin is an innovative cryptocurrency which delivers funding to developers from
every mined block.

Receiver files are used to load the addresses for payment into the mining operation. 
This is the first use of receiver files in cryptocurrency, and is a primary 
Devcoin innovation. Earnings are paid in rounds, which are in groups of blocks. 

Every mined Devcoin block generates 50,000 coins. 
5000 coins per mined block go to the miner.
The other 45,000 coins in each mined block are distributed as shares to developers.

A round is 4000 blocks. Developers share 180,000,000 coins during each round. 
180,000,000 is divided into the number of shares to come up with the value per share. 
The value per share fluctuates.  The value depends upon how much work is generated 
by contributors.

For more information, as well as an immediately usable, binary version of
the Devcoin client software, see https://www.devcoin.org.

License
-------

Devcoin is released under the terms of the MIT license. See `COPYING` for more
information or see http://opensource.org/licenses/MIT.

Development process
-------------------

Developers work in their own repositories and branches, then submit pull requests when
they think their feature or bug fix is ready.

If it is a simple/trivial/non-controversial change, then one of the Devcoin
development team members simply pulls it.

If it is a *more complicated or potentially controversial* change, then the
patch submitter will be asked to start a discussion (if they haven't already)
on the Devcoin Core Issues page at https://github.com/devcoin/core/issues

The branch will be accepted if there is broad consensus that it is a good thing.
Developers should expect to rework and resubmit patches if the code doesn't
match the project's coding conventions (see `doc/coding.md`) or are
controversial.

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable.

Building
-------
QT Build (Statically linked)

Depedencies (you can use later version(s) if you know they are backwards compatible):

-MingW 4.8 
-Boost 1.54.0 
-DB 4.8.30 
-OpenSSL 1.0.0d 
-QT 5.2.0 
-Python 2.7 (installed binary) 
-libcurl 7.33.0 (included in src) 
-ActivePerl 5.16.3 Build 1603 (64 bit)  (installed binary)
-Miniupnpc 1.8.20131209 -
-Qrencode 3.4.3
-leveldb (included in src)

Follow this guide to build on Windows and also to get idea about how to build dependencies. Also there need to be patches done to certain files (that are already done) but you can confirm, see this link:
https://bitcointalk.org/index.php?topic=149479.0

Building Boost:

    Same as tutorial

Building DB:
    ensure you add --enable-static field with configure

Building UPNPC:
    Same as tutorial

Building Qrencode:
    Same as tutorial
    
Add -static option to LDFLAGS in makefile.mingw to compile a statically linked executable.
Code:

LDFLAGS=-Wl,--dynamicbase -Wl,--nxcompat -Wl,--large-address-aware -static

Building QT:
    Same as tutorial 



The daemon code is in `src/`. To compile and run for win32:

    cd src; make -f makefile.mingw devcoind.exe
    strip devcoind.exe

To rebuild:
	make -f makefile.mingw clean
	make -f makefile.mingw devcoind.exe
	strip devcoind.exe

The QT code is in 'src/qt'. To compile and run the GUI for win32 (with features UPNP and QRCODE enabled):

    qmake USE_QRCODE=1 bitcoin-qt.pro
    make -f Makefile.release
    
    The executable should be in the .\release directory.

If you are trying to build for other platforms please use the makefile.mingw as a base, as this is the one that is tested. All compiler links and preprocessor defines are in this file and need to be ported over to other platform makefile's if they should be.
