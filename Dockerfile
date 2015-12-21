#
# Dockerfile for Devcoind
#
# version: 0.1.0
#

FROM debian:7
MAINTAINER Fernando Paredes Garcia <fernando@develcuy.com>

# Add squeeze packages
RUN echo "deb http://ftp.us.debian.org/debian/ squeeze main" > /etc/apt/sources.list.d/squeeze.list

# Update packages
RUN apt-get update
RUN apt-get dist-upgrade -y

# Install package dependencies
RUN apt-get install -y libdb4.8-dev libdb4.8++-dev build-essential libboost-all-dev git supervisor
RUN apt-get install -y procps

# Download Devcoind source
RUN git clone --depth=1 https://github.com/coinzen/devcoin.git /usr/local/src/devcoin

# Configure Devcoind
RUN useradd devcoin
RUN mkdir -p /home/devcoin/.devcoin
RUN echo '#Noob config file for devcoind, should be enough!\n\
server=1\n\
daemon=1\n\
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

# Configure supervisor
RUN echo '[supervisord]\n\
nodaemon=true\n\
\n\
[program:devcoind]\n\
command=/usr/bin/pidproxy /var/run/devcoind.pid /bin/su devcoin -c "/usr/local/bin/devcoind"\n\
redirect_stderr=true\n'\
>> /etc/supervisor/conf.d/supervisord.conf

# Compile & Install Devcoind
RUN echo 'cd /usr/local/src/devcoin/src/\n\
make -j4 -f makefile.unix USE_UPNP=-\n\
'\
> /root/compile-devcoin.sh
RUN /bin/bash /root/compile-devcoin.sh
RUN mv /usr/local/src/devcoin/src/devcoind /usr/local/bin/

EXPOSE 52332

# Start supervisor
CMD ["/usr/bin/supervisord"]
