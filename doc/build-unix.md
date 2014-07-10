UNIX BUILD NOTES [Debian 7 (Wheezy), Linux Mint 17 (Qiana) and Ubuntu 14.04 (Trusty)]
=====================================================================================
Some notes on how to build Devcoin in Unix. 

Dependencies
---------------------

 Library     | Purpose          | Description
 ------------|------------------|----------------------
 libssl      | SSL Support      | Secure communications
 libdb4.8    | Berkeley DB      | Wallet storage
 libboost    | Boost            | C++ Library
 miniupnpc   | UPnP Support     | Optional firewall-jumping support
 qt          | GUI              | GUI toolkit
 libqrencode | QR codes in GUI  | Optional for generating QR codes

[miniupnpc](http://miniupnp.free.fr/) may be used for UPnP port mapping.  It can be downloaded from [here](
http://miniupnp.tuxfamily.org/files/).  UPnP support is compiled in and
turned off by default.  See the configure options for upnp behavior desired:

	--without-miniupnpc      No UPnP support miniupnp not required
	--disable-upnp-default   (the default) UPnP support turned off by default at runtime
	--enable-upnp-default    UPnP support turned on by default at runtime

Licenses of statically linked libraries:
 Berkeley DB   New BSD license with additional requirement that linked
               software must be free open source
 Boost         MIT-like license
 miniupnpc     New (3-clause) BSD license

- For the versions used in the release, see doc/release-process.md under *Fetch and build inputs*.

System requirements
--------------------

C++ compilers are memory-hungry. It is recommended to have at least 1 GB of
memory available when compiling Devcoin Core. With 512MB of memory or less
compilation will take much longer due to swap thrashing.

internal compiler error: Killed (program cc1plus)
[ 1377.575785] Out of memory: Kill process 12305 (cc1plus) score 905 or sacrifice child
[ 1377.575800] Killed process 12305 (cc1plus) total-vm:579928kB, anon-rss:546144kB, file-rss:0kB

If you see output like this, your machine does not have enough memory to compile. You can fix this by adding more swap. To add a 1gb swap file, in /swapfile:

sudo dd if=/dev/zero of=/swapfile bs=64M count=16
sudo mkswap /swapfile
sudo swapon /swapfile

After compiling, remove swapfile:

sudo swapoff /swapfile
sudo rm /swapfile



Dependency Build Instructions: Ubuntu & Debian
----------------------------------------------
Update your openssl (heart bleed bug fix):

	sudo apt-get install openssl
	
Build requirements:

	sudo apt-get install build-essential libtool pkg-config libssl-dev
	
Get libcurl dependencies:

	sudo apt-get build-dep curl	
	
for Ubuntu 12.04 and later:

	sudo apt-get install libboost-all-dev
	(If using Boost 1.37, append -mt to the boost libraries in the makefile, see below how to build)

 db4.8 packages are available [here](https://launchpad.net/~bitcoin/+archive/bitcoin).
 You can add the repository using the following command:

        sudo add-apt-repository ppa:bitcoin/bitcoin
        sudo apt-get update

 Ubuntu 12.04 and later have packages for libdb5.1-dev and libdb5.1++-dev,
 but using these will break binary wallet compatibility, and is not recommended.

for Ubuntu 13.10:
	libboost1.54 will not work,
	remove libboost1.54-all-dev and install libboost1.53-all-dev instead.

for Debian 7 (Wheezy) and later:

 Method 1:
 
 The oldstable repository contains db4.8 packages.
 Add the following line to /etc/apt/sources.list,
 replacing [mirror] with any official debian mirror.

	deb http://[mirror]/debian/ oldstable main

 Method 2:
 
 we can reach back into the Debian 6 (Squeeze) repository. Create a file to point to the Squeeze repo:
 
	sudo vi /etc/apt/sources.list.d/debian-squeeze.list
	
 And enter the following line into the text file and save it:	
	
	deb http://ftp.us.debian.org/debian/ squeeze main
		
 To enable the change run

	sudo apt-get update
	
 And get any package upgrades:

	sudo apt-get upgrade	

for other Debian & Ubuntu (with ppa):

	sudo apt-get install libdb4.8-dev libdb4.8++-dev

Optional:

	sudo apt-get install libminiupnpc-dev (see --with-miniupnpc and --enable-upnp-default)

Build Devcoind:

	git clone --depth=1 https://github.com/coinzen/devcoin.git
	cd devcoin
	cd src
	make -f makefile.unix clean
	make -f makefile.unix USE_UPNP=-

Dependencies for the GUI: Ubuntu & Debian
-----------------------------------------

If you want to build Devcoin-Qt, make sure that the required packages for Qt development
are installed. Either Qt 4 or Qt 5 are necessary to build the GUI.
If both Qt 4 and Qt 5 are installed, Qt 4 will be used. Please make sure Devcoind has already 
compiling before you try to build the GUI.

To build with Qt 4 you need the following:

    sudo apt-get install libqt4-dev qt4-qmake

For Qt 5 you need the following:

    sudo apt-get install libqt5gui5 libqt5core5 libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev

libqrencode (optional) can be installed with:

    sudo apt-get install libpng-dev libqrencode-dev

Once these are installed, you can build Devcoin-qt (GUI).

Build Devcoin GUI:

	cd ~/devcoin
	qmake USE_UPNP=0 USE_DBUS=1 USE_QRCODE=1
	make

Notes
-----
The release is built with GCC and then "strip Devcoind" to strip the debug
symbols, which reduces the executable size by about 90%.


miniupnpc
---------
	tar -xzvf miniupnpc-1.6.tar.gz
	cd miniupnpc-1.6
	make
	sudo su
	make install


Berkeley DB
-----------
It is recommended to use Berkeley DB 4.8. If you have to build it yourself:

```bash
Devcoin_ROOT=$(pwd)

# Pick some path to install BDB to, here we create a directory within the Devcoin directory
BDB_PREFIX="${Devcoin_ROOT}/db4"
mkdir -p $BDB_PREFIX

# Fetch the source and verify that it is not tampered with
wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
echo '12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef  db-4.8.30.NC.tar.gz' | sha256sum -c
# -> db-4.8.30.NC.tar.gz: OK
tar -xzvf db-4.8.30.NC.tar.gz

# Build the library and install to our prefix
cd db-4.8.30.NC/build_unix/
#  Note: Do a static build so that it can be embedded into the exectuable, instead of having to find a .so at runtime
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX
make install

# Configure Devcoin Core to use our own-built instance of BDB
cd $Devcoin_ROOT
./configure (other args...) LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/"
```

**Note**: You only need Berkeley DB if the wallet is enabled (see the section *Disable-Wallet mode* below).

Boost
-----
If you need to build Boost yourself:

	sudo su
	./bootstrap.sh
	./bjam install


Security
--------
To help make your Devcoin installation more secure by making certain attacks impossible to
exploit even if a vulnerability is found, binaries are hardened by default.
This can be disabled with:

Hardening Flags:

	./configure --enable-hardening
	./configure --disable-hardening


Hardening enables the following features:

* Position Independent Executable
    Build position independent code to take advantage of Address Space Layout Randomization
    offered by some kernels. An attacker who is able to cause execution of code at an arbitrary
    memory location is thwarted if he doesn't know where anything useful is located.
    The stack and heap are randomly located by default but this allows the code section to be
    randomly located as well.

    On an Amd64 processor where a library was not compiled with -fPIC, this will cause an error
    such as: "relocation R_X86_64_32 against `......' can not be used when making a shared object;"

    To test that you have built PIE executable, install scanelf, part of paxutils, and use:

    	scanelf -e ./Devcoin

    The output should contain:
     TYPE
    ET_DYN

* Non-executable Stack
    If the stack is executable then trivial stack based buffer overflow exploits are possible if
    vulnerable buffers are found. By default, Devcoin should be built with a non-executable stack
    but if one of the libraries it uses asks for an executable stack or someone makes a mistake
    and uses a compiler extension which requires an executable stack, it will silently build an
    executable without the non-executable stack protection.

    To verify that the stack is non-executable after compiling use:
    `scanelf -e ./Devcoin`

    the output should contain:
	STK/REL/PTL
	RW- R-- RW-

    The STK RW- means that the stack is readable and writeable but not executable.