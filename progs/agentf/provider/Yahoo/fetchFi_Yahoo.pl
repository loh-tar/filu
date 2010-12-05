#!/usr/bin/perl

# don't buffer output, IMPORTANT!
$|=1 ;

# perl -MCPAN -e'shell'
# install Finance::Quote::Yahoo::Europe

use Finance::Quote;
$q = Finance::Quote->new;

my %markets;
$markets{PA} = "Paris";
$markets{BC} = "Barcelona";
$markets{BE} = "Berlin";
$markets{BI} = "Bilbao";
$markets{BR} = "Breme";
$markets{CO} = "Copenhagen";
$markets{D}  = "Dusseldorf";
$markets{F}  = "Frankfurt";
$markets{H}  = "Hamburg";
$markets{HA} = "Hanover";
$markets{L}  = "London";
$markets{MA} = "Madrid";
$markets{MC} = "Madrid (M.C.)";
$markets{MI} = "Milan";
$markets{MU} = "Munich";
$markets{O}  = "Oslo";
$markets{ST} = "Stockholm";
$markets{SG} = "Stuttgart";
$markets{VA} = "Valence";
$markets{VI} = "Vienna";
$markets{DE} = "Xetra";

# my @markets = qw{ PA BC BE BI BR CO D F H HA L MA MC MI MU O ST SG VA VI DE };

print "[Header]RefSymbol;Name;Type;Yahoo;Market\n";

foreach $market( keys(%markets ) )
{
    my %info = $q->fetch("europe","$ARGV[0].$market"); # Failover to other methods ok.

    my $curr;
    my $name;
    my $symbol;
    my $reuters;

    foreach $key( keys(%info) )
    {
        if ( $key =~ m/.*currency$/ ) { $curr =  $info { $key }  };
        if ( $key =~ m/.*name$/ ) { $name =  $info { $key }  };
        if ( $key =~ m/.*symbol$/ ) { $symbol =  $info { $key }  };
    }

    # Daimler Benz, Stock, DCX.DE, XETRA, EUR, yahoo,'t',  ,
    #if ( $name ne "") {
    #    print "$name, Stock, $symbol, " . $markets{ $market} . ", $curr, yahoo, 't'\n";
    #    $symbol =~ s/\..*//;
    #    print "$name, Stock, $symbol, " . $markets{ $market} . ", $curr, Reuters, 'f'\n";
    #};

    if ( $name ne "")
    {
      $reuters = $symbol;
      $reuters =~ s/\..*//;
      print "$reuters;$name;Stock;$symbol;" . $markets{ $market} . "\n";
    };
}
