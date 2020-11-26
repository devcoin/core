# How to build Devcoin for Windows on Linux (MinGW)

This guide depends on Docker: https://docs.docker.com/engine/install/


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

## Launch the Docker container

The following command creates a container named `builder`:
```
$ docker run -tid \
             --name builder \
             --cpuset-cpus 0 \
             -e "DISPLAY=unix${DISPLAY}" \
             -v /dev/shm:/dev/shm \
             --cpuset-cpus 2 \
             -v $BUILD_DIR:/workdir \
             -v /tmp/.X11-unix:/tmp/.X11-unix \
             -e BUILD_DIR="/workdir" -e TARGET_ARCH="$TARGET_ARCH" \
             burningdaylight/docker-mingw-qt5 /bin/bash
```

Now you can enter the container:
```
$ docker exec -ti builder bash
```


## Boost

For Windows 32bit:

```
$ cd "${BUILD_DIR}"
$ wget http://dl.bintray.com/boostorg/release/1.65.0/source/boost_1_65_0.tar.bz2
$ tar xjfp boost_1_65_0.tar.bz2
$ cd boost_1_65_0
$ wget https://gist.githubusercontent.com/develCuy/311475bf4d34013f1b4ba4970a272d47/raw/0b308c2e7311c9ab887268b8b6aada3dba8dc0d2/devcoin_boost_1.65.0.patch
$ patch -p1 < devcoin_boost_1.65.0.patch
$ ./bootstrap.sh --without-icu
$ echo "using gcc : 7.3 : ${TARGET_ARCH}-w64-mingw32-g++ : <rc>${TARGET_ARCH}-w64-mingw32-windres <archiver>${TARGET_ARCH}-w64-mingw32-ar ;" > user-config.jam
$ ./bjam toolset=gcc target-os=windows variant=release threading=multi threadapi=win32 --user-config=user-config.jam -j 2 --without-mpi --without-python -sNO_BZIP2=1 -sNO_ZLIB=1 --layout=tagged stage
```

For Windows 64bit:

```
$ cd "${BUILD_DIR}"
$ wget http://dl.bintray.com/boostorg/release/1.65.0/source/boost_1_65_0.tar.bz2
$ tar xjfp boost_1_65_0.tar.bz2
$ cd boost_1_65_0
$ wget https://gist.githubusercontent.com/develCuy/311475bf4d34013f1b4ba4970a272d47/raw/0b308c2e7311c9ab887268b8b6aada3dba8dc0d2/devcoin_boost_1.65.0.patch
$ patch -p1 < devcoin_boost_1.65.0.patch
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
$ wget https://curl.haxx.se/windows/dl-7.68.0/curl-7.68.0-win32-mingw.tar.xz
$ tar xf curl-7.68.0-win32-mingw.tar.xz
```

For Windows 64bit:

```
$ cd "${BUILD_DIR}"
$ wget https://curl.haxx.se/windows/dl-7.68.0/curl-7.68.0-win64-mingw.tar.xz
$ tar xf curl-7.68.0-win64-mingw.tar.xz
```


## libqrencode

```
$ cd "${BUILD_DIR}"
$ wget https://fukuchi.org/works/qrencode/qrencode-3.4.4.tar.gz
$ tar xzfp qrencode-3.4.4.tar.gz
$ cd qrencode-3.4.4
$ ./configure --host=${TARGET_ARCH}-w64-mingw32
$ make -j2

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
$ TARGET_OS=NATIVE_WINDOWS make -j2 libleveldb.a libmemenv.a CC=${TARGET_ARCH}-w64-mingw32-gcc CXX=${TARGET_ARCH}-w64-mingw32-g++
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


## Build devcoin-qt.exe


```
$ cd "${BUILD_DIR}/core"
$ x86_64-w64-mingw32-qmake-qt5 USE_UPNP=- USE_DBUS=0 USE_QRCODE=1 BUILD_DIR="$BUILD_DIR" devcoin-qt_win.pro
$ make -j4
```


## Build the package

Let's package Devcoind.exe along its runtime dependencies into a zip file.


First we need a folder for the package:
```
$ cd "${BUILD_DIR}"
$ export BUNDLE_DIR="Devcoin-${TARGET_ARCH}"
```

For Windows 32bit:

```
$ cp "${BUILD_DIR}"/curl-7.68.0-win32-mingw/bin/libcurl.dll "${BUNDLE_DIR}"
$ cp "${BUILD_DIR}"/openssl-1.1.1d-win32-mingw/*.dll "${BUNDLE_DIR}"
```

For Windows 64bit:

```
$ cp "${BUILD_DIR}"/curl-7.68.0-win64-mingw/bin/libcurl-x64.dll "${BUNDLE_DIR}"
$ cp "${BUILD_DIR}"/openssl-1.1.1d-win64-mingw/*.dll "${BUNDLE_DIR}"
```

Remaining dependencies:

```
$ cp "${BUILD_DIR}"/core/src/Devcoind.exe "${BUNDLE_DIR}"
$ cp "${BUILD_DIR}"/core/release/devcoin-qt.exe "${BUNDLE_DIR}"
$ cp -r /usr/${TARGET_ARCH}-w64-mingw32/lib/qt/plugins/platforms/ "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/libwinpthread-1.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/libgcc_s_seh-1.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/libssp-0.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/libstdc++-6.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/libharfbuzz-0.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/libiconv-2.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/libpcre2-16-0.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/libpng16-16.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/Qt5Core.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/Qt5Gui.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/Qt5Network.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/Qt5Widgets.dll "${BUNDLE_DIR}"
$ cp /usr/${TARGET_ARCH}-w64-mingw32/bin/zlib1.dll "${BUNDLE_DIR}"
```

Wrap everything into a single package:

```
$ zip "${BUILD_DIR}/${BUNDLE_DIR}".zip "${BUNDLE_DIR}"/*
```

