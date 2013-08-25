This file is part of Filu (C) 2007, 2010-2013 loh.tar@googlemail.com


Fedora Specific Notes
=======================
The last test was with Fedora 19 Schroedingers Cat

Qick Install:
  ./install-by-yum

Ensure the server is available in the future:
  sudo systemctl enable postgresql

To compile the programs you need the packages:
  gcc-c++
  cmake
  qt-devel
  qt-postgresql
  postgresql-devel
  muParser-devel

Install TA-Lib using packages by Huaren Zhong. Install from
http://download.opensuse.org/repositories/home:/zhonghuaren/Fedora_19
  ta-lib-0.4.0-2.1
  ta-lib-devel-0.4.0-2.1

The provider scripts needs the packages:
  perl-Cache-Cache
  perl-Date-Simple
  perl-IO-HTML
  perl-TimeDate
  perl-libwww-perl
  perl-HTML-TableExtract
  perl-XML-LibXML-Simple      Not available on Fedora,
                              see readme.txt 5- Troubleshooting

The database package is:
  postgresql
  postgresql-server

