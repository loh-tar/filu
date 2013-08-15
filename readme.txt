This file is part of Filu (C) 2007, 2010-2013 loh.tar@googlemail.com


Index
=======
1- Installation
2- Further Readings
3- Customizing
4- Uninstall


1- Installation
=================
To become Filu working follow these steps from top to down.

If you want to update your existing Filu installation to a new snapshot take a
look at doc/changelog.txt for things which must be considered. In general it is
a good idea to uninstall your old installation which means *only* to uninstall
the Filu progs. cd into your old FiluSource-YYYY-MM-DD/build directory and do:
  make uninstall

After that you have only to compile the new progs as described in chapter 1-2,
that's all. And *don't* touch your database except the changelog says
something else.

The following infos apply to Ubuntu and Arch Linux. If you use a different OS
you may need to do something deviating. The described way will install the Filu
programs and the PostgreSQL server on the same local machine. For details about
the PostgreSQL configuration see 3- Customizing.

*Please* report in any case if you come in trouble while the installation or
later by using Filu. Without feedback I can't fix it!


1-1- Needed Dependencies
==========================

Qt              At least version 4.8, Qt 5 is not tested
muParser        Recommended version is 2.2.3, older versions like 1.34 works
                but needs special attention
TA-Lib          Last tested version is 0.4.0
PostgreSQL      At least version 8.4


Ubuntu
--------
The last (very poor) test was with Raring Ringtail.

To compile the programs you need the .deb packages:
  build-essential
  cmake
  libqt4-dev
  libqt4-sql
  libqt4-sql-psql
  libmuparser-dev

The perl scripts needs the .dep packages:
  libcache-cache-perl
  libdate-simple-perl
  libio-html-perl
  libtimedate-perl
  libwww-perl
  libxml-libxml-simple-perl

Install TA-Lib from: http://ta-lib.org
Sadly take compile from source some time, so you could also try to use the .deb
package by Marco van Zwetselaar, but I haven't it tested.
  download ta-lib-0.4.0-src.tar.gz
  unzip the file
  cd into the extracted source directory
  ./configure
  make
  sudo make install

The database package is:
  postgresql
  pgadmin3        Not required but useful if you like to edit some data where is
                  not yet a tool at Filu to do the job. Use it with care.

NOTE: If you are a champ you write a patch to build a .deb package.


Arch
------
To compile the programs you need:
  base-devel
  cmake
  qt4
  muparser
  ta-lib          Available in AUR

The perl scripts needs the packages:
  perl-io-html
  perl-libwww
  perl-timedate

Some more packages available in AUR:
  perl-cache-cache
  perl-date-simple
  perl-xml-libxml-simple

The database:
  postgresql

NOTE: If you are a champ you write patch for a PKGBUILD.


1-2- Compilation
====================
After install of all dependencies above you have to do:
  cd into the FiluSource directory
  mkdir build
  cd build
  cmake ..        Yes, there are two dots!
  make
  sudo make install
  sudo ldconfig

The only thing what missing now, is the database and a database user.
The default for both is 'filu'. To create them simple run:
  sudo filu-cfg-postgresql

After successful install you can check the working with:
  agentf

There should not print any error message but some other stuff. Furthermore
you have the programs:
  performerf
  managerf
  inspectorf

Which you should find at your application menu below 'Office'.


2- Further Readings
=====================
More information you will find at doc directory. It may a good idea to start
with first-steps.txt, e.g. just by running 'agentf doc first'


3- Customizing
================
If you prefer different names for the database and/or database user you can run
filu-cfg-postgresql with some options. Try -h and you will enlightened.

For more tunings see:
  doc/config-file.txt
  database/sqls/filu/misc/table_entries.sql


4- Uninstall
==============
To remove the Filu program collection cd into FiluSource/build and do:
  make uninstall

cd into each other of the above visited directories and do:
  sudo make uninstall

