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
# http://cpansearch.perl.org/src/MSISK/HTML-TableExtract-2.10/lib/HTML/TableExtract.pm
#
# http://uk.finance.yahoo.com/q/cp?s=$^GDAXI&c=0
# s = search for this
# c = page number, Yahoo shows only 50 FIs per page
# alpha = letter, show only FIs matching this first letter, see no need for these
#

# Don't buffer output, IMPORTANT!
$|=1 ;

use strict;
use LWP::Simple;
use HTML::TableExtract;
use Date::Parse;

use Yahoo; # Our own module, shipped with Filu

if ($#ARGV < 0) { &usage }
if ($ARGV[0] eq "--info") { &info }

my $search4 = $ARGV[0];

# Print early the header so the user see an action
print "[Header]Reuters;Name;Type;Yahoo;Market;Weight;Quality;Notice\n";

my $oneday = 86400; # In seconds
my $now = str2time(scalar(localtime)); # Hold date in (unix time) seconds
$now -= ($oneday * 3); # Now its 3 days before today

# Fetch not only one page
my $page = -1;
my $exit =  0;
do
{
  $page += 1;
  my $url="http://uk.finance.yahoo.com/q/cp?s=$search4&c=$page";
  #print "$url\n";

  $exit = parse($url);
  #print "exit? $exit ?\n";

} until($exit);

#exit good
exit 0;

#
# -------- subs --------
#

sub parse($)
{
  my $content = get($_[0]);
  #print $content;

  # Which data has to be extract.
  # The order given here is the order we will got the data,
  # they is indepened of the original table, cool! But the names must fit.
  # We grap 'Volume' only to be shure that we got the right answer.
  # If search4 is not found Yahoo redirect to FI search, where no 'Volume' is,
  # and therefore we got no data -> thats what we want.
  my $te = HTML::TableExtract->new( headers => ['Symbol', 'Name', 'Last Trade', 'Volume'] );

  $te->parse($content);

  my $exit = 1;
  my $trade;
  my $reuters;
  my $market;

  # Examine all matching tables
  foreach my $ts ($te->tables)
  {
    #print "Table (", join(',', $ts->coords), "):\n";
    foreach my $row ($ts->rows)
    {
      $exit = 0; # Don't exit, we have data, call me again

      $reuters = Yahoo::reutersFromSymbol(@$row[0]);
      $market  = Yahoo::marketFromSymbol(@$row[0]);

      print join(';', $reuters, @$row[1], "Stock", @$row[0], $market, "1"), ";";

      # Check if 'Last Trade' makes sense. Yahoo data are not the best.
      # @$row[2] looks like "46.85 1 Jan 10:00" if bad
      #                     "46.85 10:00" if good, no date!
      $trade = @$row[2];
      $trade =~ s/^[\,\d]+\.\d+//;    # Remove price

      my $date = str2time($trade);

      if(($date - $now) < 0)
      {
        print join(';', "Tin", "Last trade was: $trade"), "\n";
      }
      else
      {
        # Looks good, no need to fill Quality or Notice
        print ";\n";
      }
    }
  }

  return $exit;
}

sub usage()
{
  print STDERR "Call me like this: \n" .
                "  ./me <Symbol>\n" .
                "  ./me ^GDAXI\n";
  die "\n";
}

sub info()
{
  print "Name:    fetchCompList_Yahoo\n";
  print "Purpose: To fetch all associated underlying to an Index with RefSymbol and Weight\n";
  print "Input:   Symbol\n";
  print "Output:  Reuters Name Type Yahoo Market Weight Quality Notice\n";
  print "Comment: Sometimes Yahoo deliver more or less components as needed\n";

  exit 0; #exit good
}

# Usual in perl last line is...
1;
