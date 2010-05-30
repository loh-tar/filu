This file is part of Filu (c) 2007, 2010 loh.tar@googlemail.com


First in advance because it's an ugly bug in Qt 4.5.x and later we need an older
version. I solved this by install Qt from source on my Ubuntu 10.04 but maybe
you can use *deb packages from Ubuntu 8.10. I didn't try it.

You need as the replacement for the below listed packages libqt4-sql,
libqt4-sql-psql and libqt4-dev:

  build-essential
  libpq-dev
  libglib2.0-dev
  libx11-dev
  libxext-dev
  libxrandr-dev
  libxrender-dev
  libxcursor-dev
  libxfixes-dev
  libxinerama-dev
  libfontconfig1-dev
  libfreetype6-dev
  libxi-dev

As one-liner:
  sudo apt-get install build-essential libpq-dev libglib2.0-dev libx11-dev libxext-dev libxrandr-dev libxrender-dev libxcursor-dev libxfixes-dev libxinerama-dev libfontconfig1-dev libfreetype6-dev libxi-dev

Download and unzip qt-x11-opensource-src-4.3.5.tar.gz from
http://qt.nokia.com/ftp://ftp.qt.nokia.com/qt/source/

./configure -opensource -qt-sql-psql -no-qt3support -nomake tools -nomake examples -nomake demos -no-exceptions
make
sudo make install

Sadly need this a lot of time, some hours, if you don't have a high performance
computer :-(

In ~/.bashrc add the lines:

 PATH=/usr/local/Trolltech/Qt-4.3.5/bin:$PATH
 export PATH

I hope I have nothing forgot, good luck!


Index
=======
1- Installation
2- Customizing
3- Further Readings
4- Uninstall


1- Installation
=================
To become Filu working follow these steps recursive from top to down.


1-1- Install The Database
===========================
Follow the readme in database/


1-2- Install The Filu Program Collection
==========================================
The following infos apply to Ubuntu, a Debian like Linux. If you use a different
OS you may need to do something deviating.


1-2-1- Needed Dependencies
============================
To compile the programs you need the .deb packages:
  build-essential
  libqt4-sql
  libqt4-sql-psql
  libqt4-dev

The perl scripts needs the .dep packages:
  libdate-simple-perl
  libfinance-quote-perl

Install TALib from: http://ta-lib.org/
Last tested version is 0.4.0.
  download ta-lib-0.4.0-src.tar.gz
  unzip
  cd into the extracted source dir
  ./configure
  make
  sudo make install

Install muParser from: http://sourceforge.net/projects/muparser/
Last tested version is v132
  download v132.tar.gz
  unzip
  cd into the extracted source dir
  ./configure --enable-samples=no
  make
  sudo make install


1-2-2- Compilation
====================
After all steps above you can do as usual:
  cd into FiluSource dir
  ./configure
  make
  sudo make install
  sudo ldconfig

After successful install you can check the working with:
  agentf

There should not print any error message but a lot of other stuff. Furthermore
you have the programs:
  performerf
  managerf
  diggerf


2- Customizing
================
If you need different settings as the defaults, you can modify some by create
a system wide config file /etc/xdg/Filu.conf
or an user config file ~/.config/Filu.conf
with contents like these.
NOTE: DON'T include some remarks as showing below, it's only to docu here!

  [General]
  SqlPath=/usr/local/lib/Filu/sqls/
  HostName=localhost
  HostPort=5432
  DatabaseName=filu
  FiluSchema=filu
  UserName=filu
  Password=filu           // yes, of course pw in clear text is not as save
  CommitBlockSize=500
  DaysToFetchIfNoData=365 // fetch one year from the inet
  SqlDebugLevel=1         // 0:no extra debug info
                          // 1:extra after an error
                          // 2:print each processed sql


3- Further Readings
=====================
More information you will find at doc directory. It may a good idea to start
with first-steps.txt.


4- Uninstall
==============
cd in each of the above visited directories and do:
  sudo make uninstall
