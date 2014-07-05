@echo off

devcoind -printtoconsole -conf=C:\dvctest\src\node3\testnet\devcoin.conf -datadir=C:\dvctest\src\node3 -port=18003 -testnet -connect=127.0.0.1:18002

pause