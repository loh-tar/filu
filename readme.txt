This file is part of Filu (C) 2007, 2010-2013 loh.tar@googlemail.com


Index
=======
1- Installation
2- Further Readings
3- Customizing
4- Uninstall
5- Troubleshooting


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

The following infos apply to Debian based Linux and Arch Linux. If you use a
different OS you may need to do something deviating. The described way will
install the Filu programs and the PostgreSQL server on the same local machine.
For details about the PostgreSQL configuration see 3- Customizing.

*Please* report in any case if you come in trouble while the installation or
later by using Filu. Without feedback I can't fix it!


1-1- Needed Dependencies
==========================

Qt              At least version 4.8, Qt 5 is not tested
muParser        At least version 2.0.0, recommended is 2.2.3
TA-Lib          At least version 0.4.0
PostgreSQL      At least version 8.4


Debian
--------
The last (short) test was with Debian 7.1 Wheezy and Ubuntu 13.4 Raring Ringtail

NOTE: There is an install script available which will perform all the following
      steps for you, just call it this way:
        cd /path-to/FiluSource-YYYY-MM-DD
        ./install-aptget

To compile the programs you need the .deb packages:
  build-essential
  cmake
  libqt4-dev
  libqt4-sql
  libqt4-sql-psql
  libpq-dev
  libmuparser-dev

The perl scripts needs the .dep packages:
  libcache-cache-perl
  libdate-simple-perl
  libio-html-perl
  libtimedate-perl
  libwww-perl
  libxml-libxml-simple-perl
  libhtml-tableextract-perl

Install TA-Lib from http://ta-lib.org. Use the packages by Marco van Zwetselaar:
  Download from http://www.zwets.com/debs/unstable the files
    ta-lib0-dev_0.4.0-2_i386.deb and libta-lib0_0.4.0-2_i386.deb
  sudo dpkg -i \
    /path-to/libta-lib0_0.4.0-2_i386.deb \
    /path-to/ta-lib0-dev_0.4.0-2_i386.deb

Or compile from source which will take some time:
  Download and unzip the file ta-lib-0.4.0-src.tar.gz
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
  perl-html-tableextract

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

You should now find at your application menu below 'Office' the programs:
  performerf
  managerf
  inspectorf

Furthermore you have:
  agentf
  filu-cfg-postgresql


1-3- Setup PostgreSQL
=======================
The only thing what missing now, is the database and a database user.
The default for both is 'filu'. To create them simple run:
  sudo filu-cfg-postgresql

If there is reported that the PostgreSQL server not is running, start them. On
some distributions, like Arch or Fedora, you have to create a cluster after
install of PostgreSQL and have to start the server manually. To do so run on
Fedora:
  sudo postgresql-setup initdb
  sudo systemctl start postgresql

To ensure the server is available in the future run:
  sudo systemctl enable postgresql.service


1-4- As A Last Point
======================
Now its time to check the working with:
  agentf

There should not print any error message but some other stuff.


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


5- Troubleshooting
====================
Here some note which hopefuly may help if something went wrong.

- If CMake reports not to find PostgreSQL look at
    http://stackoverflow.com/a/13934972
