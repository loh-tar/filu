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

The following infos apply to Debian based Linux, Fedora and Arch Linux. If you
use a different OS you may need to do something deviating. The described way
will install the Filu programs and the PostgreSQL server on the same local
machine. For details about the PostgreSQL configuration see 3- Customizing.

There are install scripts available (tested only on 32 bit systems) for apt-get
and yum based Linux. Running on a slow Netbook take around 10min to install
on Ubuntu. Run just simple:
  ./install-by-aptget

Take a look at the fitting readme-foo.txt for details of the used packages and
repositories.

*Please* report in any case if you come in trouble while the installation or
later by using Filu. Without feedback I can't fix it!


1-1- Needed Dependencies
==========================
The main programs need to complile:
  Compiler        Only tested with gcc
  Qt              At least version 4.8, Qt 5 is not tested
  muParser        At least version 2.0.0, recommended is 2.2.3
  TA-Lib          At least version 0.4.0

The European Central Bank scripts needs Perl modules:
  Cache::FileCache
  Date::Parse
  Date::Simple
  HTML::TableExtract
  LWP::Simple
  XML::LibXML::Simple

The Yahoo scripts needs Perl modules:
  Date::Parse
  Date::Simple
  HTML::TableExtract
  LWP::Simple

The database:
  PostgreSQL      At least version 8.4


1-2- Compilation
====================
After install of all dependencies you have to do:
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

If there is reported that the PostgreSQL server couldn't be started, start them.
On some distributions, like Arch or Fedora, you have to create a cluster after
install of PostgreSQL and have to start the server manually. The script try to
do it for you but may fail.

Ensure the server is available in the future. On systemd systems run:
  sudo systemctl enable postgresql


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

Removing of all installed dependencies is up to you, sorry.


5- Troubleshooting
====================
Here some notes which hopefully may help if something went wrong or is missing.

Install TA-Lib From Source
----------------------------
Download and unzip the source from http://ta-lib.org
  cd into the extracted source directory
  ./configure
  make
  sudo make install

Install muParser From Source
------------------------------
Download and unzip the source from http://sourceforge.net/projects/muparser/
  cd into the extracted source directory
  ./configure --enable-samples=no
  make
  sudo make install

Install Missing Perl Packages
-------------------------------
If so, like xml-libxml-simple on Fedora, you can install them direct from cpan
easily with App-cpanminus. In our example is one more dependency needed:
  yum -y install perl-App-cpanminus libxml2-devel
  cpanm XML::LibXML::Simple

CMake Reports Not To Find PostgreSQL
--------------------------------------
Look at http://stackoverflow.com/a/13934972, Good Luck!

The Compilation Abort With Not To Find Something
--------------------------------------------------
Read attentively what is missing and search the web, or your package manager,
where the missing file is included and (re)install the package.

