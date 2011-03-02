This file is part of Filu (C) 2007, 2010, 2011 loh.tar@googlemail.com


Index
=======
1- Installation
2- Customizing
3- Further Readings
4- Uninstall
5- Fix Qt Bug in PSql Driver


1- Installation
=================
To become Filu working follow these steps recursive from top to down.

If you want to update your existing Filu installation to a new snapshot take a
look at doc/changelog.txt for things which must be considered. In general it is
a good idea to uninstall your old installation which means *only* to uninstall
the Filu progs. cd into your old FiluSource-YYYY-MM-DD/build directory and do:
  sudo xargs rm < install_manifest.txt

After that you have only to compile the new progs as described in chapter 1-2-2
without to follow chapter 5, that's all. And *don't* touch your database except
the changelog says something else.


1-1- Install The Database
===========================
Follow the readme in database/


1-2- Install The Filu Program Collection
==========================================
The following infos apply to Ubuntu, a Debian like Linux and Arch Linux. If you
use a different OS you may need to do something deviating.


1-2-1- Needed Dependencies
============================
Arch
------
To compile the programs you need:
  base-devel
  cmake
  qt

The perl scripts needs the packages:
  perl-libwww

Some more packages available with "bauerbill -S --cpan foo":
  perl-date-simple
  perl-timedate

To install TALib and muParser you could follow the Ubuntu instructions or use
packages from AUR:
  muparser
  ta-lib


Ubuntu
--------
To compile the programs you need the .deb packages:
  build-essential
  cmake
  libqt4-sql
  libqt4-sql-psql
  libqt4-dev

The perl scripts needs the .dep packages:
  libwww-perl
  libdate-simple-perl
  libtimedate-perl

Install TALib from: http://ta-lib.org/
Last tested version is 0.4.0.
  download ta-lib-0.4.0-src.tar.gz
  unzip the file
  cd into the extracted source directory
  ./configure
  make
  sudo make install

Install muParser from: http://sourceforge.net/projects/muparser/
Last tested version is v132
  download v132.tar.gz
  unzip the file
  cd into the extracted source directory
  ./configure --enable-samples=no
  make
  sudo make install


1-2-2- Compilation
====================
After all steps above you have to do:
  cd into the FiluSource directory
  mkdir build
  cd build
  cmake ..         Yes, there are two dots!
  make
  sudo make install
  sudo ldconfig

NOTICE: Before you continue you have to do the instructions at chapter 5 below.

After successful install you can check the working with:
  agentf

There should not print any error message but a lot of other stuff. Furthermore
you have the programs:
  performerf
  managerf
  inspectorf


2- Customizing
================
See doc/config-file.txt.


3- Further Readings
=====================
More information you will find at doc directory. It may a good idea to start
with first-steps.txt.


4- Uninstall
==============
To remove the Filu program collection cd into FiluSource/build and do:
  sudo xargs rm < install_manifest.txt

cd into each other of the above visited directories and do:
  sudo make uninstall


5- Fix Qt Bug in PSql Driver
==============================
In Qt later 4.3.5 (released May 2008) is an ugly bug in the PSql driver which is
sadly still not fixed. Moreover it would seem that there is no plan fix it :-(
  http://bugreports.qt.nokia.com/browse/QTBUG-3267
  http://bugreports.qt.nokia.com/browse/QTBUG-7218
  http://qt.gitorious.org/+qt-developers/qt/staging/commit/6c44ab0f6edebce1e7190b94ac5b74c81812f482

As dirty workaround we have to do these steps:

Download and unzip qt-everywhere-opensource-src-4.7.0.tar.gz from
  ftp://ftp.qt.nokia.com/qt/source/

NOTE: If your distribution uses a newer or older version of Qt, it might be wise
      to download the suitable qt-source. They could named different, e.g.
qt-x11-opensource-src-4.5.3.tar.gz. The source tree may also looks different, so
you have to look for the appropriate files. In case of a newer version it is of
cause possible that the bug is fixed and here is nothing to do.

NOTE: When your distribution install an update of Qt may the here compiled
      driver fix overwritten. In that case you have to redo the last step and
replace the original with the self compiled driver.

Edit the file
  qt-everywhere-opensource-src-4.7.0/src/sql/drivers/psql/qsql_psql.cpp

Search for this code block and comment out line 721 with "case PreparedQueries"
that it looks as below:
  bool QPSQLDriver::hasFeature(DriverFeature f) const
  {
      switch (f) {
      case Transactions:
      case QuerySize:
      case LastInsertId:
      case LowPrecisionNumbers:
      case EventNotifications:
          return true;
  //    case PreparedQueries:
      case PositionalPlaceholders:
          return d->pro >= QPSQLDriver::Version82;
      case BatchOperations:

That will cause a warning when compiling in the next step which is OK. Now open
a terminal and cd into the extracted qt source tree.

  cd qt-everywhere-opensource-src-4.7.0/src/plugins/sqldrivers/psql

Arch
------
  qmake -o Makefile "INCLUDEPATH+=/usr/include/libpq/" "LIBS+=-L/usr/lib -lpq" psql.pro
  make

If you like make a backup of the original driver:
  sudo cp /usr/lib/qt/plugins/sqldrivers/libqsqlpsql.so /your-choice-dir/orig-libqsqlpsql.so

Now replace the original with the new compiled one:
  sudo cp libqsqlpsql.so /usr/lib/qt/plugins/sqldrivers/


Ubuntu
--------
Thanks to Thomas for reporting, tested with Kubuntu 10.04.
You need an additional .deb package:
  libpq-dev

  qmake -o Makefile "INCLUDEPATH+=/usr/include/postgresql" "LIBS+=-L/usr/lib -lpq" psql.pro
  make

If you like make a backup of the original driver:
  sudo cp /usr/lib/qt4/plugins/sqldrivers/libqsqlpsql.so /your-choice-dir/orig-libqsqlpsql.so

Now replace the original with the new compiled one:
  sudo cp libqsqlpsql.so /usr/lib/qt4/plugins/sqldrivers/
