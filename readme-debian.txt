This file is part of Filu (C) 2007, 2010-2013 loh.tar@googlemail.com


Debian Specific Notes
=======================
The last test was with Debian 7.1 Wheezy and Ubuntu 13.4 Raring Ringtail

Qick Install: Run ./install-by-aptget

To compile the programs you need the packages:
  build-essential
  cmake
  libqt4-dev
  libqt4-sql
  libqt4-sql-psql
  libpq-dev
  libmuparser-dev

The provider scripts needs the packages:
  libcache-cache-perl
  libdate-simple-perl
  libio-html-perl
  libtimedate-perl
  libwww-perl
  libxml-libxml-simple-perl
  libhtml-tableextract-perl

Install TA-Lib using the packages by Marco van Zwetselaar. Install from
http://www.zwets.com/debs/unstable
  ta-lib0-dev_0.4.0-2_i386.deb
  libta-lib0_0.4.0-2_i386.deb

The database package is:
  postgresql

