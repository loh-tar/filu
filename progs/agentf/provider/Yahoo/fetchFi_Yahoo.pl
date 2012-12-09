#!/usr/bin/perl
#
#    This file is part of Filu.
#
#    Copyright (C) 2007, 2010, 2011  loh.tar@googlemail.com
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

#
# http://cpansearch.perl.org/src/MSISK/HTML-TableExtract-2.10/lib/HTML/TableExtract.pm
#
# http://uk.finance.yahoo.com/lookup?s=appl&t=A&m=ALL&r=&b=20
# s = search for this
# t = type, which page is shown, no searched = (A-All)(S-Stock)(M-Mutal Funds)
# m = market All,GB,US,DE
# b = how many results rows to skip
# r = sort order, odd=asc, even=dsc, 1,2=Symbol, 2,3=Name, 4,5=Exchange,
#
# http://uk.finance.yahoo.com/lookup?s=foo
# http://uk.finance.yahoo.com/lookup/stocks;?s=AAPL&t=S&m=ALL&r=
# http://uk.finance.yahoo.com/lookup/all
# http://uk.finance.yahoo.com/lookup/funds
# http://uk.finance.yahoo.com/lookup/etfs
# http://uk.finance.yahoo.com/lookup/indices
# http://uk.finance.yahoo.com/lookup/futures
# http://uk.finance.yahoo.com/lookup/currency
#
# http://finance.yahoo.com/lookup?s=US0378331005
#

# Don't buffer output, IMPORTANT!
$|=1;

use strict;
use LWP::Simple;
use HTML::TableExtract;
use Yahoo; # Our own module, shipped with Filu

if ($#ARGV < 0) { &usage }
if ($ARGV[0] eq "--info") { &info }

my $search4 = $ARGV[0];

# Print early the header so the user see an action
print "[Header]Yahoo;Name;ISIN;Type;Market\n";

my $url="http://uk.finance.yahoo.com/lookup/all?s=$search4&t=A&m=ALL&r=&b=";
#print "$url\n";

my $content = get($url);

# Which data has to be extract.
# The order given here is the order we will got the data,
# they is indepened of the original table, cool! But the names must fit.
my $te = HTML::TableExtract->new( headers => [qw(Symbol Name Isin Type Exchange)] );

$te->parse($content);

# Examine all matching tables
foreach my $ts ($te->tables)
{
  #print "Table (", join(',', $ts->coords), "):\n";
  foreach my $row ($ts->rows)
  {
    # Replace Yahoo market token with our own
    @$row[4] = Yahoo::convertMarket(@$row[4]);

    print join(';', @$row), "\n";
  }
}

#exit good
exit 0;

#
# -------- subs --------
#

sub usage()
{
  print STDERR "Call me like this: \n" .
                "  ./me foo\n" .
                "  ./me aapl\n";
  die "\n";
}

sub info()
{
  print "Name:    fetchFi_Yahoo\n";
  print "Purpose: Fetch the master data to a FI\n";
  print "Input:   SearchString";
  print "Output:  Name Type <SymbolType> [RefSymbol]\n";
  print "Comment: The SearchString can be a part of a symbol or FI name\n";

  exit 0; #exit good
}

# Usual in perl last line is...
1;
