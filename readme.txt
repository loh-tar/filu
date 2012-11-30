This file is part of Filu (C) 2007, 2010, 2011, 2012 loh.tar@googlemail.com


Index
=======
1- Installation
2- Customizing
3- Further Readings
4- Uninstall


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
Minimum required Qt version is 4.6.


Arch
------
To compile the programs you need:
  base-devel
  cmake
  qt

The perl scripts needs the packages:
  perl-libwww
  perl-timedate

One more package available in AUR:
  perl-date-simple

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
