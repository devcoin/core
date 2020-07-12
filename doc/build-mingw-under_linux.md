# How to build Devcoin for Windows on Linux (MinGW)

This guide was made on Ubuntu Bionic 18.04

## Enviroment setup

Let's have a folder to build dependencies and the wallet.
```
$ export BUILD_DIR="${HOME}/Devcoin-build"
$ mkdir -p "${BUILD_DIR}"
```
We also have to choose the target architecture.

For 32-bit:

```
$ export TARGET_ARCH=i686
```

For 64-bit:

```
$ export TARGET_ARCH=x86_64
```


## Install dependencies

```
$ sudo apt install g++-mingw-w64-${TARGET_ARCH} mingw-w64-${TARGET_ARCH}-dev curl
$ sudo update-alternatives --config ${TARGET_ARCH}-w64-mingw32-gcc # choose gcc-posix
$ sudo update-alternatives --config ${TARGET_ARCH}-w64-mingw32-g++ # choose g++-posix
```


## Boost

For Windows 32bit:

```
$ cd "${BUILD_DIR}"
$ wget http://dl.bintray.com/boostorg/release/1.65.0/source/boost_1_65_0.tar.bz2
$ tar xjfp boost_1_65_0.tar.bz2
$ cd boost_1_65_0
$ ./bootstrap.sh --without-icu
$ echo "using gcc : 7.3 : ${TARGET_ARCH}-w64-mingw32-g++ : <rc>${TARGET_ARCH}-64-mingw32-windres <archiver>${TARGET_ARCH}-w64-mingw32-ar ;" > user-config.jam
$ ./bjam toolset=gcc target-os=windows variant=release threading=multi threadapi=win32 --user-config=user-config.jam -j 2 --without-mpi --without-python -sNO_BZIP2=1 -sNO_ZLIB=1 --layout=tagged stage
```

For Windows 64bit:

```
$ cd "${BUILD_DIR}"
$ wget http://dl.bintray.com/boostorg/release/1.65.0/source/boost_1_65_0.tar.bz2
$ tar xjfp boost_1_65_0.tar.bz2
$ cd boost_1_65_0
$ ./bootstrap.sh --without-icu
$ echo "using gcc : 7.3 : ${TARGET_ARCH}-w64-mingw32-g++ : <rc>${TARGET_ARCH}-w64-mingw32-windres <archiver>${TARGET_ARCH}-w64-mingw32-ar ;" > user-config.jam
$ ./bjam toolset=gcc target-os=windows address-model=64 architecture=x86 variant=release threading=multi threadapi=win32 --user-config=user-config.jam -j 2 --without-mpi --without-python -sNO_BZIP2=1 -sNO_ZLIB=1 --layout=tagged stage
```


## SSL 

There are working binaries provided by the official CURL repository.

For Windows 32bit:

```
$ cd "${BUILD_DIR}"
$ wget https://curl.haxx.se/windows/dl-7.68.0/openssl-1.1.1d-win32-mingw.tar.xz
$ tar xf openssl-1.1.1d-win32-mingw.tar.xz
```

For Windows 64bit:

```
$ cd "${BUILD_DIR}"
$ wget https://curl.haxx.se/windows/dl-7.68.0/openssl-1.1.1d-win64-mingw.tar.xz
$ tar xf openssl-1.1.1d-win64-mingw.tar.xz
```


## BerkeleyDB

```
$ cd "${BUILD_DIR}"
$ wget http://anduin.linuxfromscratch.org/BLFS/bdb/db-5.3.28.tar.gz
$ tar xzfp db-5.3.28.tar.gz
$ cd db-5.3.28/
$ sed -i.old 's/__atomic_compare_exchange/__atomic_compare_exchange_db/' src/dbinc/atomic.h
$ sed -i.old 's/WinIoCtl\.h/winioctl\.h/g' src/dbinc/win_db.h
$ cd build_unix/
$ sh ../dist/configure --host=${TARGET_ARCH}-w64-mingw32 --enable-cxx --enable-mingw
$ make -j2
```


## Curl

There are working binaries provided by the official CURL repository.

For Windows 32bit:

```
$ cd "${BUILD_DIR}"
$ https://curl.haxx.se/windows/dl-7.68.0/curl-7.68.0-win32-mingw.tar.xz
$ tar xf curl-7.68.0-win32-mingw.tar.xz
```

For Windows 64bit:

```
$ cd "${BUILD_DIR}"
$ https://curl.haxx.se/windows/dl-7.68.0/curl-7.68.0-win64-mingw.tar.xz
$ tar xf curl-7.68.0-win64-mingw.tar.xz
```


## Make devcoind.exe

Download devcoin source code:

```
$ git clone --depth=1 https://github.com/devcoin/core/
```

First should build LevelDB (included in Devcoin source code):

```
$ cd "${BUILD_DIR}/core/src/leveldb"
$ chmod +x build_detect_platform
$ TARGET_OS=NATIVE_WINDOWS make libleveldb.a libmemenv.a CC=${TARGET_ARCH}-w64-mingw32-gcc CXX=${TARGET_ARCH}-w64-mingw32-g++
$ ${TARGET_ARCH}-w64-mingw32-ranlib libleveldb.a && ${TARGET_ARCH}-w64-mingw32-ranlib libmemenv.a

```

Now build Devcoind.exe.

For Windows 32bit:

```
$ cd "${BUILD_DIR}/core/src"
$ make -f makefile.linux-mingw -j8 CROSS_COMPILE=${TARGET_ARCH}-w64-mingw32- CXXFLAGS="-static-libgcc -static-libstdc++" LMODE=dynamic
```

For Windows 64bit:

```
$ cd "${BUILD_DIR}/core/src"
$ make -f makefile.linux-mingw64 -j8 CROSS_COMPILE=${TARGET_ARCH}-w64-mingw32- CXXFLAGS="-static-libgcc -static-libstdc++" LMODE=dynamic
```

Let's package Devcoind.exe along its runtime dependencies into a zip file.

For Windows 32bit:

```
$ cd "${BUILD_DIR}"
$ mkdir Devcoin-win32
$ cp "${BUILD_DIR}"/core/src/Devcoind.exe Devcoin-win32
$ cp "${BUILD_DIR}"/curl-7.68.0-win64-mingw/bin/libcurl.dll Devcoin-win32
$ cp "${BUILD_DIR}"/openssl-1.1.1d-win64-mingw/*.dll Devcoin-win32
$ cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll Devcoin-win32
$ zip "${BUILD_DIR}"/Devcoin-win32.zip Devcoin-win32/*
```

For Windows 64bit:

```
$ cd "${BUILD_DIR}"
$ mkdir Devcoin-win64
$ cp "${BUILD_DIR}"/core/src/Devcoind.exe Devcoin-win64
$ cp "${BUILD_DIR}"/curl-7.68.0-win64-mingw/bin/libcurl-x64.dll Devcoin-win64
$ cp "${BUILD_DIR}"/openssl-1.1.1d-win64-mingw/*.dll Devcoin-win64
$ cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll Devcoin-win64
$ zip "${BUILD_DIR}"/Devcoin-win64.zip Devcoin-win64/*
```


## Build devcoin-qt.exe
```
TODO
```

