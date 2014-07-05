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

Follow this quide to build on Windows and also to get idea about how to build dependencies. Also there need to be patches done to certain files (that are already done) but you can confirm, see this link:
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

If you are trying to build for other platforms please use the makefile.mingw as a base as this is the one that is tested, all compiler links and preprocessor define's are in this file and need to be ported over to other platform makefile's if they should be.	
