#
# Dockerfile for Devcoind
#
# version: 0.1.1
#

FROM phusion/baseimage:0.11
MAINTAINER Fernando Paredes Garcia <fernando@develcuy.com>

# Update packages
RUN apt-get update && apt-get upgrade -y -o Dpkg::Options::="--force-confold"

# Install package dependencies
#RUN apt-get install -y --force-yes libdb4.8-dev libdb4.8++-dev build-essential libboost-all-dev git supervisor
#RUN apt-get install -y procps

RUN apt-get install -y --force-yes git libdb++-dev libdb-dev libboost-all-dev build-essential libtool pkg-config libssl-dev

# Download Devcoind source
RUN git clone --depth=1 https://github.com/devcoin/core.git /usr/local/src/devcoin

# Create devcoin user
RUN useradd devcoin; chsh -s /bin/bash devcoin
RUN mkdir -p /home/devcoin/.devcoin

# Setup devcoind
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
RUN useradd logger; chsh -s /bin/bash logger; mkdir /var/log/devcoind;
RUN mkdir -p /etc/sv/devcoind/log
RUN echo '#!/bin/sh\n\
\n\
exec 2>&1\n\
\n\
cd /home/devcoin/; exec /sbin/setuser devcoin devcoind\n\
'\
> /etc/sv/devcoind/run
RUN echo '#!/bin/sh\n\
exec /usr/bin/svlogd -tt /var/log/devcoind\n\
'\
> /etc/sv/devcoind/log/run
RUN chmod +x /etc/sv/devcoind/run; chmod +x /etc/sv/devcoind/log/run; ln -s /etc/sv/devcoind /etc/service/

# Compile & Install Devcoind
RUN echo 'cd /usr/local/src/devcoin/src/\n\
make -f makefile.unix clean\n\
make -f makefile.unix USE_UPNP=- devcoind\n\
'\
> /root/compile-devcoin.sh
RUN /bin/bash /root/compile-devcoin.sh
RUN mv /usr/local/src/devcoin/src/devcoind /usr/local/bin/

EXPOSE 52332

# Use baseimage-docker's init process.
CMD ["/sbin/my_init"]

# Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

WORKDIR /home/devcoin
