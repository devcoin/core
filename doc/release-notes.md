22.0 Release Notes
==================

Devcoin Core version 22.0 is now available from:

  <https://devcoin.org/bin/devcoin-core-22.0/>

This release includes new features, various bug fixes and performance
improvements, as well as updated translations.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/devcoin/core/issues>

To receive security and update notifications, please subscribe to:

  <https://devcoin.org/en/list/announcements/join/>

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes in some cases), then run the
installer (on Windows) or just copy over `/Applications/Devcoin-Qt` (on Mac)
or `devcoind`/`devcoin-qt` (on Linux).

Upgrading directly from a version of Devcoin Core that has reached its EOL is
possible, but it might take some time if the data directory needs to be migrated. Old
wallet versions of Devcoin Core are generally supported.

Compatibility
==============

Devcoin Core is supported and extensively tested on operating systems
using the Linux kernel, macOS 10.14+, and Windows 7 and newer.  Devcoin
Core should also work on most other Unix-like systems but is not as
frequently tested on them.  It is not recommended to use Devcoin Core on
unsupported systems.

From Devcoin Core 22.0 onwards, macOS versions earlier than 10.14 are no longer supported.

Notable changes
===============

P2P and network changes
-----------------------

New and Updated RPCs
--------------------

Build System
------------

Files
-----

New settings
------------

Updated settings
----------------

Tools and Utilities
-------------------

Wallet
------

GUI changes
-----------

Low-level changes
=================

RPC
---

Tests
-----

22.0 change log
===============

A detailed list of changes in this version follows. To keep the list to a manageable length, small refactors and typo fixes are not included, and similar changes are sometimes condensed into one line.

### Consensus

### Policy

### Mining

### Block and transaction handling

### P2P protocol and network code

### Wallet

### RPC and other APIs

### GUI

### Build system

### Tests and QA

### Miscellaneous

### Documentation

Credits
=======

Thanks to everyone who directly contributed to this release:

As well as to everyone that helped with translations on
[Transifex](https://www.transifex.com/devcoin/devcoin/).
