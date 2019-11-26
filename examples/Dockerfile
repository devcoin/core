#
# Dockerfile for Devcoind
#
# version: 0.2.0
#

FROM debian:stretch-slim as builder
MAINTAINER Fernando Paredes Garcia <fernando@develcuy.com>

# Update packages and install package dependencies
RUN apt-get update && apt-get install --no-install-recommends -y build-essential ca-certificates libboost-all-dev libssl-dev libz-dev git wget \
    && rm -rf /var/lib/apt/lists/*;
ENV SRC /usr/local/src
ENV BDBVER db-4.8.30.NC
WORKDIR $SRC
# Download and Compile Berkeley DB 4.8
RUN set -ex; \
        mkdir -p $SRC/db4_dest; \
        wget --quiet https://download.oracle.com/berkeley-db/$BDBVER.tar.gz; \
        echo "12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef  $BDBVER.tar.gz" | sha256sum -c - || exit 1; \
        tar -xzf $BDBVER.tar.gz; \
        cd $BDBVER/build_unix/; \
        sed s/__atomic_compare_exchange/__atomic_compare_exchange_db/g -i ../dbinc/atomic.h; \
        ../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$SRC/db4_dest; \
        make install; \
        cd $SRC; \
        rm -rf $BDBVER $BDBVER.tar.gz db4_dest/docs

# Download Devcoind source
RUN git clone --depth=1 https://github.com/devcoin/core.git $SRC/devcoin
# Compile & Install Devcoind
RUN set -ex; \
        cd $SRC/devcoin/src; \
        make -j4 -f makefile.unix USE_UPNP=- BDB_LIB_PATH=$SRC/db4_dest/lib/ BDB_INCLUDE_PATH=$SRC/db4_dest/include/; \
        strip devcoind; \
        mv devcoind /usr/local/bin/; \
        make -f makefile.unix clean

# Use osixia/light-baseimage
# https://github.com/osixia/docker-light-baseimage
FROM osixia/light-baseimage:1.1.2

COPY --from=builder /usr/local/bin/devcoind /usr/local/bin/devcoind
# Update packages and install package dependencies
RUN apt-get -y update \
    && /container/tool/add-multiple-process-stack \
    && LC_ALL=C DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
       libboost-system1.62.0 libboost-filesystem1.62.0 libboost-program-options1.62.0 libboost-thread1.62.0 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Create devcoin user
RUN useradd devcoin; chsh -s /bin/bash devcoin; mkdir -p /home/devcoin/.devcoin

# Configure devcoind settings
RUN echo '#Noob config file for devcoind, should be enough!\n\
server=1\n\
daemon=0\n\
rpcuser=devcoind\n\
rpcpassword=devcoin\n\
# OpenSSL settings used when rpcssl=1\n\
rpcsslciphers=TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH\n\
rpcsslcertificatechainfile=server.cert\n\
rpcsslprivatekeyfile=server.pem\n\
rpcport=52332\n\
rpcallowip=*\n\
# Set gen=1 to attempt to generate coins\n\
gen=0\n\
# Allow direct connections for the 'pay via IP address' feature.\n\
allowreceivebyip=1\n\
# Enable transaction index\n\
txindex=1\n\
'\
> /home/devcoin/.devcoin/devcoin.conf
RUN chown devcoin: -R /home/devcoin

# Setup devcoind service
ENV DEVCOIND /container/run/process/devcoind
RUN mkdir /var/log/devcoind; mkdir -p $DEVCOIND/log; echo '#!/bin/sh\n\
\n\
exec 2>&1\n\
\n\
cd /home/devcoin/; exec /sbin/setuser devcoin devcoind\n\
'\
> $DEVCOIND/run
RUN echo '#!/bin/sh\n\
exec /usr/bin/svlogd -tt /var/log/devcoind\n\
'\
> $DEVCOIND/log/run
RUN chmod +x $DEVCOIND/run; chmod +x $DEVCOIND/log/run; ln -s $DEVCOIND /etc/service/

EXPOSE 52332
