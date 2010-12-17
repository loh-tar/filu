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

package Yahoo;

use strict;
use warnings;
our $VERSION = '1.00';
use base 'Exporter';

our @EXPORT = qw(reutersFromSymbol marketFromSymbol convertMarket);

sub reutersFromSymbol($)
{
  # Chop the Yahoo symbol suffix and return the raw symbol
  # as our fake Reuters symbol.

  if($#_ < 0)
  {
    print STDERR "*** ERROR *** Yahoo::reuters needs 1 argument\n";
    return "ERROR";
  }

  my $reuters;

  $reuters = shift;
  $reuters =~ s/\..*//;

  return $reuters;
}

sub marketFromSymbol($)
{
  # Read the Yahoo symbol suffix and return the Filu market name.

  if($#_ < 0)
  {
    print STDERR "*** ERROR *** Yahoo::marketFromSymbol needs 1 argument\n";
    return "ERROR";
  }

  my $fullSymbol = shift; # Take the first function parameter
  if($fullSymbol =~ m/^\^/){ return "Index"; }

  my @symbol_market = split('\.', $fullSymbol);
  if($#symbol_market == 0)
  {
    # No suffix, we can't distinguish between NYSE/NASDAQ ect
    return "NYSE";
  }
  #print "marketFromSymbol: $symbol_market[0] $symbol_market[1]\n";

  my $marketToken = $symbol_market[1];

  # Edit or add some if needed. When you change here something,
  # change them also in doc/hacking-provider-scripts.txt
  #
  # Take a look at...
  #   http://help.yahoo.com/l/uk/yahoo/finance/basics/fitadelay2.html
  my %markets;
  $markets{BC} = "Barcelona";
  $markets{BE} = "Berlin";
  $markets{BI} = "Bilbao";
  $markets{BR} = "Brussels";
  $markets{CO} = "Copenhagen";
  $markets{DU} = "Dusseldorf";
  $markets{EX} = "Eurex";
  $markets{F}  = "Frankfurt";
  $markets{HK} = "HKSE";
  $markets{HM} = "Hamburg";
  $markets{HA} = "Hanover";
  $markets{L}  = "London";
  $markets{MA} = "Madrid";
  $markets{MC} = "Madrid";
  $markets{MF} = "Madrid";
  $markets{MI} = "Milan";
  $markets{MU} = "Munich";
  $markets{NX} = "Euronext";
  $markets{OL} = "Oslo";
  $markets{PA} = "Euronext";
  $markets{ST} = "Stockholm";
  $markets{SG} = "Stuttgart";
  $markets{TO} = "Toronto";
  $markets{V}  = "CDNX";
  $markets{VA} = "Valence";
  $markets{VI} = "Vienna";
  $markets{DE} = "Xetra";

  return $markets{ uc($marketToken) };
}

sub convertMarket($)
{
  # Translate the 3-char Yahoo market token to our Filu names.

  if($#_ < 0)
  {
    print STDERR "*** ERROR *** Yahoo::convertMarket needs 1 argument\n";
    return "ERROR";
  }

  my $market = shift; # Take the first function parameter

  # Edit, uncomment or add some if needed. Take a look at...
  #   http://help.yahoo.com/l/uk/yahoo/finance/basics/fitadelay2.html
  # ...but they fit not exactly to the token need here.
  #
  # Because Yahoo symbols don't distinguish between NYSE/NASDAQ etc
  # we have to set here always to NYSE.
  # Take care that the names fit with these in 'marketFromSymbol($)' above
  #
  my %markets;
  $markets{BAR} = "Barcelona";
  $markets{BER} = "Berlin";
  # $markets{BI} = "Bilbao";
  $markets{BRU} = "Brussels";
  # $markets{CO} = "Copenhagen";
  $markets{DUS} = "Dusseldorf";
  $markets{ENX} = "Euronext";
  $markets{EUX} = "Eurex";
  $markets{FRA} = "Frankfurt";
  $markets{FSI} = "London";
  $markets{HAM} = "Hamburg";
  $markets{HKG} = "HKSE";
  # $markets{HA} = "Hanover";
  $markets{LSE} = "London";
  $markets{MCE} = "Madrid";
  $markets{MIL} = "Milan";
  $markets{MUN} = "Munich";
  $markets{MEX} = "Mexico";
  $markets{NAS} = "NYSE";
  $markets{NGM} = "NYSE"; # NasdaqGM
  $markets{NMS} = "NYSE"; # NasdaqGM
  $markets{NYQ} = "NYSE";
  $markets{OSL} = "Oslo";
  $markets{PNK} = "NYSE"; # Pink OTC Markets
  $markets{STO} = "Stockholm";
  $markets{STU} = "Stuttgart";
  $markets{TOR} = "Toronto";
  $markets{VAL} = "Valence";
  $markets{VAN} = "CDNX";
  $markets{VIE} = "Vienna";
  $markets{GER} = "Xetra";

  my $filuMarket = $markets{ uc($market) };

  if(!defined $filuMarket)
  {
    $filuMarket = uc($market);
  }

  return $filuMarket;
}

# Usual in perl last line is...
1;
