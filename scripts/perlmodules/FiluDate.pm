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

package FiluDate;

use strict;
use warnings;
our $VERSION = '1.00';
use base 'Exporter';

our @EXPORT = qw(isoDate);

sub isoDate($)
{
  # Convert a date string into ISO format YYYY-MM-DD.
  # Returns "0000-01-01" if "N/A" Not Avaiable
  # Returns "1000-01-01" if date is bad or not detect
  #
  # Supported formats are:
  # ISO of course 2011-07-21
  # US style 7/21/2011
  # German 21.7.2011
  # Forms with short name of month    21-Jul-11
  #                                Jul 21, 2011
  #                                21. Jul 2011

  my $isod = $_[0];

  # Test data
  #$isod = "11 01 2002";
  #$isod = "2011-07-21";
  #$isod = "7/21/2011";
  #$isod = "21.7.2011";
  #$isod = "21-Jul-11";
  #$isod = "21. Jul 2011";
  #$isod = "Jul 21, 2011";

  my $day;
  my $month;
  my $year;

  # Test for 2011-07-21
  if($isod =~ m/\d\d\d\d-\d\d-\d\d/ )
  {
    #print STDERR "Detect ISO style\n";
    my @date = split(/\-/, $isod);
    $day   = $date[2];
    $month = $date[1];
    $year  = $date[0];
  }
  # Test for 7/21/2011
  elsif($isod =~ m/\d{1,2}\/\d{1,2}\/\d{4}/ )
  {
    #print STDERR "Detect US style\n";
    my @date = split(/\//, $isod);
    $day   = $date[1];
    $month = $date[0];
    $year  = $date[2];
  }
  # Test for 21.7.2011
  elsif($isod =~ m/\d{1,2}\.\d{1,2}\.\d{4}/ )
  {
    #print STDERR "Detect German style\n";
    my @date = split(/\./, $isod);
    $day   = $date[0];
    $month = $date[1];
    $year  = $date[2];
  }
  # Test for 21-Jul-11
  elsif($isod =~ m/\d+\-\w+\-\d\d/ )
  {
    #print STDERR "Detect 21-Jul-11 style\n";
    my @date = split(/\-/, $isod);
    $day   = $date[0];
    $month = monthNumber($date[1]);
    $year  = $date[2];
  }
  # Test for 21. Jul 2011
  elsif($isod =~ m/\d+\.\s\w+\s\d\d/ )
  {
    #print STDERR "Detect 21. Jul 2011 style\n";
    my @date = split(/\W+/, $isod);

    $day   = $date[0];
    $month = monthNumber($date[1]);
    $year  = $date[2];
  }
  # Test for Jul 21, 2011
  elsif($isod =~ m/\w+\s\d+\,\s\d\d/ )
  {
    #print STDERR "Detect Jul 21, 2011 style\n";
    my @date = split(/\W+/, $isod);

    $day   = $date[1];
    $month = monthNumber($date[0]);
    $year  = $date[2];
  }
  elsif(lc($isod) eq "n/a") # Not available, e.g. Yahoo give some times
  {
    return "0000-01-01";
  }
  else
  {
    #print STDERR "could not parse date " . $isod . "\n";
    return "1000-01-01";
  }

  #fix Y2K problem
  if(length($year) == 2) # Do nothing if year is e.g. 2011
  {
    if($year < 90 and $year >= 00) { $year = "20" . $year } else { $year = "19" . $year }
  }

  # Last validate checks
  return "1000-01-01" if(!isDateValid($year,$month,$day));

  # Build the beef
  if(length($month) == 1) { $month = "0$month" }
  if(length($day) == 1) { $day = "0$day" }

  return "$year-$month-$day";
}

sub monthNumber($)
{
  my $mn = $_[0];

  $mn =~ s/Jan/1/;
  $mn =~ s/Feb/2/;
  $mn =~ s/Mar/3/;
  $mn =~ s/Apr/4/;
  $mn =~ s/May/5/;
  $mn =~ s/Jun/6/;
  $mn =~ s/Jul/7/;
  $mn =~ s/Aug/8/;
  $mn =~ s/Sep/9/;
  $mn =~ s/Oct/10/;
  $mn =~ s/Nov/11/;
  $mn =~ s/Dec/12/;

  return $mn;
}

#
# Date validation was found at http://www.perlmonks.org/index.pl?node_id=614932
# Thanks to JohnGG
sub isLeap($)
{
  my $year = $_[0];
  return 0 if $year % 4;
  return 1 if $year % 100;
  return 1 unless $year % 400;
  return 0;
}

sub isDateValid($)
{
  my($year, $month, $day) = @_;
  return 0 unless
      $year =~ /^\d+$/
      and $month =~ /^\d+$/
      and $day =~ /^\d+$/;
  my $daysinm = [
      [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31],
      [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]];
  return 0 if $year < 1 or $year > 9999;
  return 0 if $month < 1 or $month > 12;
  return 0 if $day < 1 or $day > $daysinm->[isLeap($year)]->[$month - 1];
  return 1;
}

# Usual in perl last line is...
1;
