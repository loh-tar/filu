#!/usr/bin/perl
#
#    This file is part of Filu.
#
#    Copyright (C) 2007, 2010, 2011, 2012, 2013 loh.tar@googlemail.com
#
#    Filu is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    Filu is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Filu. If not, see <http://www.gnu.org/licenses/>.
#

#
# http://cpansearch.perl.org/src/MSISK/HTML-TableExtract-2.10/lib/HTML/TableExtract.pm
#

use strict;
use warnings;
use LWP::Simple;
use HTML::TableExtract;
use Date::Parse;

require IO::HTML;    # If not installed LWP::Simple::get() returns undef

#
# sub forward declarations
#
sub info();
sub usage();
sub parse($);

#
# Our European Central Bank data source
#
# http://www.ecb.int/euro.html"; # Nice source too, but need diffrend parsing
my $url="http://www.ecb.int/stats/exchange/eurofxref/html/index.en.html";

#
# main
#
my $search4 = "--all";    # List all currencies if no search pattern is given
$search4 = $ARGV[0] if defined $ARGV[0];

if($search4 eq "--help")
{
  usage();
  exit 0;
}

if($search4 eq "--info")
{
  info();
  exit 0;
}

# Print early the header so the user see an action
print "[Header]Name;ECB;Type;Market;Quality\n";

parse($url);
#exit good
exit 0;

#
# subs
#
sub parse($)
{
  my $content = get($_[0]);
  die "No raw data from server" unless defined $content;

  # Sadly has the table we are interested in no useable headers,
  # but fortunately is it the one and only table there.
  # So we can use simple HTML::TableExtract->new() or
  my $te = HTML::TableExtract->new( depth => 0, count => 0);

  $te->parse($content);
#   $te->parse_file('index.en.html'); # Testing only

  # Shorthand...top level rows() method assumes the first table found in
  # the document if no arguments are supplied.
  foreach my $row ($te->rows)
  {
#       print join(',', grep defined, @$row), "\n";

    # @$row[0] hold the symbol and @$row[1] the currency name
    next unless @$row[0] =~ m/^[\w]{3}$/;     # Filter some garbage before and after the currencies
    next if @$row[0] eq "ISK";                # The last rate was published on 3 Dec 2008

    @$row[1] =~ s/\s+$//;                     # There is an ugl trailing whitespace to remove

    my $fi = "@$row[1];@$row[0]";
    print "$fi;Currency;Forex;Gold\n" if $fi =~ m/$search4/i or $search4 eq "--all";
  }
}

sub usage()
{
  print "Call me like this: \n" .
        "  ./me <Pattern>\n" .
        "  ./me dol\n";
}

sub info()
{
  print "Name:    fetchFi_ECB\n";
  print "Date:    2013-07-15\n";
  print "Author:  loh.tar\n";
  print "Purpose: To fetch currencies by the ECB, the European Central Bank\n";
  print "Input:   Pattern\n";
  print "Output:  Name ECB Type Market Quality\n";
  print "Comment: The European Central Bank delivers only most important currencies\n";
}

# Usual in perl last line is...
1;
