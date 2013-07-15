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

use strict;
use warnings;
use LWP::Simple;
use Date::Simple;
use XML::LibXML::Simple;
use Cache::FileCache;
# use Data::Dumper;             # For testing only

#
# sub forward declarations
#
sub info();
sub usage();
sub printData(@);
sub calcXQuote($);
sub fillCache(@);
sub readFile($);
sub fetchFull();
sub fetchMax90Days();
sub fetchToday();

#
# Our European Central Bank data sources
#
my $dailyFile  = "http://www.ecb.int/stats/eurofxref/eurofxref-daily.xml";
my $ninetyFile = "http://www.ecb.europa.eu/stats/eurofxref/eurofxref-hist-90d.xml";
my $currFile   = "http://www.ecb.int/stats/exchange/eurofxref/html/[curr].xml";

#
# main
#
if($#ARGV < 0) { usage(); }
if($ARGV[0] eq "--info") { info(); }
if($#ARGV < 2) { usage(); }

my $force    = 0;
if($#ARGV > 2) { $force = $ARGV[3] eq "--force" ? 1 : 0; }

my $fromDate = Date::Simple->new($ARGV[0]);
my $toDate   = Date::Simple->new($ARGV[1]);
my $symbol   = $ARGV[2];
my @curr     = split ("/", $ARGV[2]);
my $curr1    = $curr[0];
my $curr2    = (not defined $curr[1]) ? "EUR" : $curr[1];
my $cache    = new Cache::FileCache( { 'namespace' => 'ecb'} );

# From Wikipedia
#   http://en.wikipedia.org/wiki/Currency_pair
#
#   The currency that is used as the reference is called the counter currency or
#   quote currency and the currency that is quoted in relation is called the
#   base currency or transaction currency.
#
#   A widely traded currency pair is the relation of the euro against the
#   US dollar, designated as EUR/USD. The quotation EUR/USD 1.2500 means that
#   one euro is exchanged for 1.2500 US dollars. Here, EUR is the base currency
#   and USD is the counter currency.
#
# Let us make some definitions:
#
# EURquote
#   The quotes delivered by the ECB, e.g EUR/USD
#
# Xquote
#   Our desired cross quotation e.g. GBP/USD. When the EURquote for GBP is
#   EURquote1 and the EURquote for USD is EURquote2 than is Xquote
#   calculated by:
#       Xquote = EURquote2 / EURquote1
#

my $EURquote1 = 0;
my $EURquote2 = 0;

print "Date;Open;High;Low;Close;Volume;OpenInterest;Quality\n";

my $today = Date::Simple::today();
if($today->day_of_week() == 0) { $today -= 2; } # Set Sunday to last Friday
if($today->day_of_week() == 6) { $today -= 1; } # Set Saturday to last Friday

if($fromDate eq $toDate and $toDate eq $today)
{
  fetchToday();
}
elsif($fromDate >= $today - 90)
{
  fetchMax90Days();
}
else
{
  fetchFull();
}

exit 0;

#
# subs
#
sub fetchToday()
{
  fillCache("daily", $dailyFile);

  my $data = $cache->get("daily");
  my $ref  = XMLin($data, KeyAttr => ['time', 'currency'] , ForceArray => 0 );
  my $date = $ref->{'Cube'}->{'Cube'}->{time};

  if(Date::Simple::date($date) ne $toDate)
  {
    exit 0; #exit good, but data file not enough up to date
  }

  $EURquote1 = $ref->{'Cube'}->{'Cube'}->{'Cube'}->{$curr1}->{rate};
  $EURquote2 = $ref->{'Cube'}->{'Cube'}->{'Cube'}->{$curr2}->{rate};

  calcXQuote($date);
}

sub fetchMax90Days()
{
  fillCache("ninety", $ninetyFile);
  readFile("ninety");
}

sub fetchFull()
{
  my $ref1 = undef;
  my $ref2 = undef;

  if($curr1 ne "EUR")
  {
    my $file = $currFile;
    $file =~ s/\[curr\]/$curr1/g;
    $file = lc $file;
#     print STDERR "$curr1 :: $file\n";
    fillCache($curr1, $file);

    my $data = $cache->get($curr1);
    $ref1 = XMLin($data, KeyAttr => ['TIME_PERIOD', 'OBS_VALUE'] , ForceArray => 0 );
#   print Dumper($ref);
  }

  if($curr2 ne "EUR")
  {
    my $file = $currFile;
    $file =~ s/\[curr\]/$curr2/g;
    $file = lc $file;
#     print STDERR "$curr2 :: $file\n";
    fillCache($curr2, $file);

    my $data = $cache->get($curr2);
    $ref2 = XMLin($data, KeyAttr => ['TIME_PERIOD', 'OBS_VALUE'] , ForceArray => 0 );
  }

  for(; $fromDate <= $toDate; $fromDate += 1)
  {
    next if(   $fromDate->day_of_week() == 0    # skip Saturday/Sunday
            or $fromDate->day_of_week() == 6);

    if($ref1)
    {
      $EURquote1 = $ref1->{'DataSet'}->{'Series'}->{'Obs'}->{"$fromDate"}->{'OBS_VALUE'};
    }
    if($ref2)
    {
      $EURquote2 = $ref2->{'DataSet'}->{'Series'}->{'Obs'}->{"$fromDate"}->{'OBS_VALUE'};
    }

    calcXQuote("$fromDate");
  }
}

sub readFile($)
{
  my $fileType = shift;

  my $data = $cache->get($fileType);
  my $ref = XMLin($data, KeyAttr => ['time', 'currency'] , ForceArray => 0 );
#   print Dumper($ref);

  for(; $fromDate <= $toDate; $fromDate += 1)
  {
    next if(   $fromDate->day_of_week() == 0    # skip Saturday/Sunday
            or $fromDate->day_of_week() == 6);

    $EURquote1 = $ref->{'Cube'}->{'Cube'}->{"$fromDate"}->{'Cube'}->{$curr1}->{rate};
    $EURquote2 = $ref->{'Cube'}->{'Cube'}->{"$fromDate"}->{'Cube'}->{$curr2}->{rate};

    calcXQuote("$fromDate");
  }
#   print STDERR "$fileType \n";
}

sub fillCache(@)
{
  my $fileType = shift;
  my $file     = shift;

  # Avoid to bother the ECB server when we work on weekend or holidays
  my $nextPoll = $cache->get($fileType . 'NextPoll', 0);
  $nextPoll = 0 unless defined $nextPoll;
  if(time < $nextPoll and not $force)
  {
    printf STDERR "No need to fetch \'$fileType\', %dmin remains\n", ($nextPoll - time) / 60;
    return;
  }

  my $timestamp = $cache->get($fileType . 'Timestanp');
  $timestamp = 0 unless defined $timestamp;

  my @header = head($file);
  if($header[2] > $timestamp or $force)
  {
    $timestamp = $header[2];
    $cache->set($fileType . 'Timestanp', $timestamp);
    $cache->set($fileType, get($file));
    print STDERR "Cache " . $fileType . " new filled.\n";

    # 86400 = one day, 1800 = half an hour
    # Half an hour less, who knows how clocklike the ECB is
    $nextPoll = $timestamp + 86400 - 1800;
    # On Friday set next poll to Monday
    my @ts = localtime($timestamp);
    my $dts = Date::Simple::ymd($ts[5] + 1900, $ts[4] + 1, $ts[3]);
    printf STDERR "File date: $dts \n";

    if($dts->day_of_week() == 5) { $nextPoll += 86400 + 86400; }
    $cache->set($fileType . 'NextPoll', $nextPoll);
  }
}

sub calcXQuote($)
{
  my $date = shift;

  if($curr1 eq "EUR") { $EURquote1 = 1; }
  if($curr2 eq "EUR") { $EURquote2 = 1; }

  $EURquote1 = 0 unless defined $EURquote1;
  $EURquote2 = 0 unless defined $EURquote2;

  if($EURquote1 == 0 or $EURquote2 == 0)
  {
    # Any symbol unknown, no problem
    return;
  }

  my $Xquote = $EURquote2 / $EURquote1;

  printData($date, $Xquote);
}

sub printData(@)
{
  my $date  = shift;
  my $quote = shift;

  # "Date;Open;High;Low;Close;Volume;OpenInterest;Quality\n";
  print "$date;$quote;$quote;$quote;$quote;0;0;Gold\n"
}

sub usage()
{
  print STDERR "Call me like this: \n" .
                "  ./me fromDate toDate symbol [--force]\n" .
                "  ./me --info\n" .
                "  ./me 2013-07-01 2007-07-08 USD/JPY\n\n" .
                "Use \'--force\' to refill cache by download fresh data. (Testing only)";
  die "\n";
}

sub info()
{
  print "Name:    fetchBar_EuroCB\n";
  print "Date:    2013-07-15\n";
  print "Author:  loh.tar\n";
  print "Purpose: To fetch currency quotes from the European Central Bank\n";
  print "Input:   FromDate ToDate Symbol\n";
  print "Output:  Date Open High Low Close Volume OpenInterest Quality\n";
  print "Comment: The ECB delivers only Euro foreign exchange reference rates, " .
                 "but there is a build in converter so e.g. USD/JPY can fetched too. " .
                 "The rates are usually updated by 3pm CET.\n";

  exit 0; #exit good
}

# Usual in perl last line is...
1;
