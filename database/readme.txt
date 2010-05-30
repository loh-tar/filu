This file is part of Filu (c) 2007, 2010 loh.tar@googlemail.com


The following infos apply to Ubuntu, a Debian like Linux. If you use a different
OS you may need to do something deviating.

Needed .deb Packages
======================
  postgresql-8.4   At least 8.4
  pgadmin3         Not required but useful if you like to edit some data where
                   is not yet a tool at Filu to do the job. Use it with care.


Default Installation
======================
View your /etc/postgresql/.../pg_hba.conf file. Search for:
  # "local" is for Unix domain socket connections only
  local   all         all                               trust

Make sure there is "trust" and not "ident same" user or something else.
Otherwise edit your file to be right and reload postgres:
  sudo /etc/init.d/postgresql-8.4 reload

To create the Filu database use:
  ./configure
  sudo make install

To drop the database use:
  sudo make uninstall

To remove created sql files:
  make clean


Custom Installation
=====================
To build a database with own parameters takes ./configure the following
arguments:
  -d [dbname] // Database name                          default filu
  -s [schema] // Database schema                        default filu
  -u [user]   // Postgres User/Role                     default filu
  -g [Level]  // Debug level. The DB will prints        default 0
              // warning 1, notice 2, info 3

If you create a extra DB for any purpose, it could be that some things already
exists. To prevent an error while install the new DB add some of these switches:
  -D  // don't create Database
  -S  // don't create Schema
  -U  // don't create User
  -T  // don't create Tables, but replace functions, views, indicators and
      // default values

For example:
  ./configure -d myDatabaseName -s mySchema -u myUser

Expecting the user exists already do:
  ./configure -d myDatabaseName -s mySchema -u myUser -U

NOTE: Use only lower case letters for dbame, schema (and user?)


Default Values
================
After creating the database there are no longer any default values, except
these less you could find in the files at /FiluSource/database/defaults. But
don't worry. After successful installation of the Filu programs read
FiluSource/doc/first-steps.txt and be happy.


Backup
========
To backup your database with postgres tools consult:
  man pg_dump
  man pg_restore

An other way is to export all data with AgentF. For details read
doc/export-data.txt.


Drop Your Database
====================
The last created database you could remove with:
  sudo make uninstall

To remove a special database, e.g. testdb, that was created some else, you have
to type:
  sudo su postgres -c'psql -d template1 -c"DROP DATABASE testdb;" '
