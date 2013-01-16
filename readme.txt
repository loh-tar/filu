This file is part of Filu (C) 2007, 2010-2013 loh.tar@googlemail.com


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
  make uninstall

After that you have only to compile the new progs as described in chapter 1-2,
that's all. And *don't* touch your database except the changelog says
something else.

The following infos apply to Ubuntu and Arch Linux. If you use a different OS
you may need to do something deviating. But please report in that case what do
you had to do to got it running.

Take a look at the Ubuntu directions for possibly additional hints.


1-1- Needed Dependencies
==========================
Ubuntu
--------
Note: These infos are a bit getting outdated. It was tested with Lucid Lynx.
      So please report if you run in trouble with Precise Pangolin or newer.

To compile the programs you need the .deb packages:
  build-essential
  cmake
  libqt4-sql
  libqt4-sql-psql
  libqt4-dev
Minimum required Qt version is 4.6.

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

The database package is:
  postgresql-8.4   At least 8.4. Version 9.x is NOT tested.
  pgadmin3         Not required but useful if you like to edit some data where
                   is not yet a tool at Filu to do the job. Use it with care.

After the install view your /etc/postgresql/.../pg_hba.conf file
and search for:
  # "local" is for Unix domain socket connections only
  local   all         all                               trust

Make sure there is "trust" and not "ident sameuser" or something else. Otherwise
edit the file to be right and reload postgres:
  sudo /etc/init.d/postgresql-8.4 reload


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

The database:
  postgresql

After install of the server ensure that the server is running, now
and in the future.

Note: If you are a champ, you write, and send me, a PKGBUILD.


1-2- Compilation
====================
After install of all dependencies above you have to do:
  cd into the FiluSource directory
  mkdir build
  cd build
  cmake ..         Yes, there are two dots!
  make
  sudo make install
  sudo ldconfig

The only thing what now is missing is the database and a database user.
The default for both is 'filu'. To create them simple run:
  make init-filu

After successful install you can check the working with:
  agentf

There should not print any error message but some other stuff. Furthermore
you have the programs:
  performerf
  managerf
  inspectorf

Which you should find at your application menu below 'Office'.


2- Customizing
================
If you prefer different names for the database and/or database user you can at
the cmake config step above the switch -D use and run:
  cmake -D DBUSER=anyUserName -D DBNAME=anyDBName .. // Still two dots!
  make init-filu

There are only the two commands 'createuser' and 'createdb' invoked with some
options. So you can these also run direct.

  createuser -e -D -R -S -U postgres anyUserName
  createdb -e -T template1 -U postgres -O anyUserName anyDBName "Info Text"

For more tunings see doc/config-file.txt.


3- Further Readings
=====================
More information you will find at doc directory. It may a good idea to start
with first-steps.txt.


4- Uninstall
==============
To remove the Filu program collection cd into FiluSource/build and do:
  make uninstall

cd into each other of the above visited directories and do:
  sudo make uninstall
