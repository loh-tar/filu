#!/usr/bin/perl
#
#    This file is part of Filu.
#
#    Copyright (C) 2007, 2010  loh.tar@googlemail.com
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

if ( $toDate eq Date::Simple->today() ) {
    #print "today\n";
    $quoteData = $quoteData . get($yahoo_today);
    while( $quoteData =~ s/\"// ) {}
    #print $quoteData;
    #die();
    $quality = 2;
}

$quoteData = $quoteData . get($yahoo);
#print $quoteData;

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
        # convert date to SQL format
        $columns[0] =~ s/Jan/01/;
        $columns[0] =~ s/Feb/02/;
        $columns[0] =~ s/Mar/03/;
        $columns[0] =~ s/Apr/04/;
        $columns[0] =~ s/May/05/;
        $columns[0] =~ s/Jun/06/;
        $columns[0] =~ s/Jul/07/;
        $columns[0] =~ s/Aug/08/;
        $columns[0] =~ s/Sep/09/;
        $columns[0] =~ s/Oct/10/;
        $columns[0] =~ s/Nov/11/;
        $columns[0] =~ s/Dec/12/;

        if ($columns[0] !~ m/\d\d\d\d-\d\d-\d\d/ ) {
            #if english date convert to american

            if ($columns[0] =~ m/\d*(.)\d\d(.)\d*/ ) {}
            my @date = split(/$1/, $columns[0]);

            if ($#date != 2)
            {
                print STDERR "could not parse date " .
                             $columns[0] . "\n";
                return;
            }
            else
            {
                #fix Y2K problem

                if($date[2] < 90 and $date[2] >= 00)
                {
                    $date[2] = "20" . $date[2]
                }
                else
                {
                    $date[2] = "19" . $date[2]
                };

                $columns[0] = ymd( $date[2], $date[0], $date[1] );
            }
        }
        else
        {
            #date is in the right order allready
            $columns[0] = Date::Simple->new($columns[0]);
        }

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
  print STDERR "call me like this: \n" .
                "./me fromdate todate symbol market\n" .
                "./me 2007-01-01 2007-04-21 AAPL NYSE\n";
  die "\n";
}

# Usual in perl last line is...
1;
