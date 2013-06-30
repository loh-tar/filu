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
#    FiMi
#
#    Copyright (C) 2001-2006 Christian Kindler
#
#    This file is part of FiMi.
#
#    FiMi is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    Foobar is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Foobar; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

use strict;
use LWP::Simple;
use Date::Simple(':all');

use lib '../../perlmodules';
use FiluDate;

if ($#ARGV < 0) { &usage }
if ($ARGV[0] eq "--info") { &info }
if ($#ARGV < 3) { &usage }

#build url:
my $fromDate     = Date::Simple->new($ARGV[0]);
my $fromDay      = $fromDate->day;
my $fromMonth    = $fromDate->month -1;
my $fromYear     = $fromDate->year;

my $toDate       = Date::Simple->new($ARGV[1]);
my $toDay        = $toDate->day;
my $toMonth      = $toDate->month -1;
my $toYear       = $toDate->year;

my $yahoo_symbol = $ARGV[2];
my $yahoo_today  = "http://finance.yahoo.com/d/quotes.csv?s=$yahoo_symbol&f=d1ohgl1vl1";
my $yahoo        = "http://ichart.finance.yahoo.com/table.csv?" .
                   "s=$yahoo_symbol" .
                   "&a=$fromMonth&b=$fromDay&c=$fromYear" .
                   "&d=$toMonth&e=$toDay&f=$toYear&g=d&ignore=.csv";

# download data now
my $quoteData = "";
my $quality = 1;

if($toDate eq Date::Simple->today() ) {

  # Because Yahoo make trouble in July of 2011 I try to
  # fix it with repeated download of todays data.
  # NOTE: They ship garbage data with date in the 1970th
  for(my $i = 0; $i < 5; $i++)
  {
    $quoteData = get($yahoo_today);
    while( $quoteData =~ s/\"// ) {} # Remove double quotes "

    my @columns = split(/,/, $quoteData);
    my $isod = isoDate($columns[0]);

    if($isod == $toDate)
    {
      #print STDERR "good $yahoo_symbol $quoteData";
      $quality = 2;
      $i = 10; # Break
    }
    elsif(($isod >= $fromDate) and ($isod < $toDate))
    {
      # Hm, seams to early to got todays data, we accept them
      #print STDERR "alright $yahoo_symbol $quoteData";
      $quality = 2;
      $i = 10; # Break
    }
    elsif($isod eq "0000-01-01") # Not available
    {
      #print STDERR "nope $yahoo_symbol $quoteData";
      $quoteData = "";
      $i = 10; # Break
    }
    else
    {
      #print STDERR "damn$i $yahoo_symbol $quoteData";
      $quoteData = "";
    }
  }
}

# Fetch historical data
if($fromDate < Date::Simple->today() )
{
  $quoteData = $quoteData . get($yahoo);
}

# parse and print data
my @lines;

if ($quoteData =~ m/\\r\\n/)
{
    #windows
    @lines = split ("\r\n", $quoteData);
}
else
{
    #unix
    @lines = split ("\n", $quoteData);
}

print "Date;Open;High;Low;Close;Volume;OpenInterest;Quality\n";

foreach my $line(@lines)
{
    my $result = &parse($line);
    if( defined $result )
    {
      print "$result;$quality\n";
      # only the first line is less quality
      if ( $quality == 2 ) { $quality = 1 };
    }
}

#exit good
exit 0;

#
# -------- subs --------
#

sub parse($)
{
    my $line = $_[0];
    my $newline;

    my @columns = split(/,/, $line);

    # input array @columns is containig:
    # Date,Open,High,Low,Close,Volume,Adj. Close*
    # 12-Jan-07,30.10,31.17,30.05,30.79,9466200,30.79
    # parse to YYYY-MM-DD,open,high,low,close,vol,oi

    # parse only valid data, no headers no footers
    if ($columns[0] != "Date" and defined $columns[0])
    {
        $columns[0] = isoDate($columns[0]);

        # now build the output
        # there is no open interrest end string with ,
        $newline = "$columns[0];$columns[1];$columns[2];$columns[3];" .
                   "$columns[4];$columns[5];";

        # eliminate unwanted characters
        while ( $newline =~ s/N.A//i) {};

    } # end if ($columns[0] != "Date" and defined $columns[0])

    return $newline;
}

sub usage()
{
  print STDERR "Call me like this: \n" .
                "  ./me fromdate todate symbol market\n" .
                "  ./me 2007-01-01 2007-04-21 AAPL NewYork\n";
  die "\n";
}

sub info()
{
  print "Name:    fetchBar_Yahoo\n";
  print "Date:    2012\n";
  print "Author:  Christian Kindler, loh.tar\n";
  print "Purpose: To fetch EOD Bar data to a FI in a given time frame\n";
  print "Input:   FromDate ToDate Symbol Market";
  print "Output:  Date Open High Low Close Volume OpenInterest Quality\n";
  print "Comment: Sadly delivers Yahoo sometimes buggy historical data\n";

  exit 0; #exit good
}

# Usual in perl last line is...
1;
