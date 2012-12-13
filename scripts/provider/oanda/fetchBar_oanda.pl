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

use LWP::Simple;
use Date::Simple(':all');

if ($#ARGV < 0) { &usage }
if ($ARGV[0] eq "--info") { &info }
if ($#ARGV < 3) { &usage }

#build url:
my $fromDateOri  = Date::Simple->new($ARGV[0]);
my $toDateOri    = Date::Simple->new($ARGV[1]);

# build one url for each 199 days
my @oandaQuotes;
my $fromDate;
my $toDate;

# print $toDateOri - $fromDateOri . "\n";
while ( $toDate - $fromDate >= 199 || ( !defined($toDate) && !defined($fromDate) ) )
{
    # build for 199 Days:


    if ( !defined($fromDate) )
    {
        $fromDate     = $fromDateOri;
    }
    else
    {
        $fromDate     = $toDate +1;
    }

    if ( $toDateOri - $fromDate < 199 )
    {
        $toDate       = Date::Simple->new($toDateOri);
    }
    else
    {
        $toDate       = Date::Simple->new($fromDate + 199);
    }

    # build date parts
    my $fromDay      = $fromDate->day;
    my $fromMonth    = $fromDate->month;
    my $fromYear     = $fromDate->year;

    my $toDay        = $toDate->day;
    my $toMonth      = $toDate->month;
    my $toYear       = $toDate->year;

    my $oanda_symbol = $ARGV[2];

    # build one url for each 199 days
    my $oanda        = 'http://www.oanda.com/convert/fxhistory?lang=en&' .
                       'date1=' . $fromMonth . '%2F' . $fromDay . '%2F' . $fromYear . '&' .
                       'date=' . $toMonth . '%2F' . $toDay . '%2F' . $toYear . '&date_fmt=us&' .
                       'exch=' . $oanda_symbol .
                       '&exch2=&expr=USD&expr2=&margin_fixed=0&&SUBMIT=Get+Table&format=CSV&redirected=1';

    # debug
    # print "> $fromDate - $toDate < > $fromDay $fromMonth $fromYear - $toDay $toMonth $toYear < ende\n";
    # print "$oanda \n";

    my $htmlSource   = get($oanda);
    my @lines        = getLines($htmlSource);

    foreach my $line(@lines)
    {
        my $result = &parse($line);
        if( defined $result )
        {
          print $result . "\n";
        }
    } #end for each line

} # end while

#exit good
1;


#
# -------- subs --------
#

sub usage()
{
    print STDERR "Call me like this: \n" .
                 "  ./me fromdate todate symbol\n" .
                 "  ./me 2007-01-01 2007-04-21 EUR\n";
    die "\n";
}

sub info()
{
  print "Name:    fetchBar_oanda\n";
  print "Date:    2006\n";
  print "Author:  Christian Kindler\n";
  print "Purpose: To fetch currency bar data in a given time frame\n";
  print "Input:   FromDate ToDate Symbol";
  print "Output:  Date Open High Low Close Volume OpenInterest Quality\n";
  print "Comment: Currently broken, go and FIXME\n";

  exit 0; #exit good
}

sub parse($)
{
    my $line = $_[0];
    my $newline;

    # input array @columns is containig:
    # Date,Open,High,Low,Close,Volume,Adj. Close*
    # 07/20/2006,0.79830
    # parse to YYYY-MM-DD,open,high,low,close,vol,oi

    # my @columns = split(/,/, $line);
   if ( $line =~ m/(\d\d)\/(\d\d)\/(\d\d\d\d),(\d*\.\d*)/ )
   {
       # $newline = "$3-$1-$2,,,,$4,,";
       $newline = "$3-$1-$2,$4,$4,$4,$4,0,0";
   }


    # return the result
    $newline;
}

sub getLines($)
{
    my @lines;

    if ($_[0] =~ m/\\r\\n/)
    {
        #windows
        @lines = split ("\r\n", $_[0]);
    }
    else
    {
        #unix
        @lines = split ("\n", $_[0]);
    }

    return @lines;
}
