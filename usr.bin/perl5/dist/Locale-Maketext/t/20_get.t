#!/usr/bin/perl -Tw

use strict;
use Test::More tests => 10;

BEGIN {
    use_ok( 'Locale::Maketext' );
}

print "# --- Making sure that get_handle works ---\n";

# declare some classes...
{
    package Woozle;
    our @ISA = ('Locale::Maketext');
    sub dubbil   { return $_[1] * 2 }
    sub numerate { return $_[2] . 'en' }
}
{
    package Woozle::eu_mt;
    our @ISA = ('Woozle');
    our %Lexicon = (
        'd2' => 'hum [dubbil,_1]',
        'd3' => 'hoo [quant,_1,zaz]',
        'd4' => 'hoo [*,_1,zaz]',
    );
    keys %Lexicon; # dodges the 'used only once' warning
}

my $lh = Woozle->get_handle('eu-mt');
isa_ok( $lh, 'Woozle::eu_mt' );
is( $lh->maketext( 'd2', 7 ), 'hum 14' );

print "# Make sure we can assign to ENV entries\n",
"# (Otherwise we can't run the subsequent tests)...\n";
$ENV{'MYORP'}   = 'Zing';
is( $ENV{'MYORP'}, 'Zing' );
$ENV{'SWUZ'}   = 'KLORTHO HOOBOY';
is( $ENV{'SWUZ'}, 'KLORTHO HOOBOY' );

delete $ENV{'MYORP'};
delete $ENV{'SWUZ'};


print "# Test LANG...\n";
$ENV{'LC_ALL'} = '';
$ENV{'LC_MESSAGES'} = '';
$ENV{'REQUEST_METHOD'} = '';
$ENV{'LANG'}     = 'Eu_MT';
$ENV{'LANGUAGE'} = '';
$lh = Woozle->get_handle();
isa_ok( $lh, 'Woozle::eu_mt' );

print "# Test LANGUAGE...\n";
$ENV{'LANG'}     = '';
$ENV{'LANGUAGE'} = 'Eu-MT';
$lh = Woozle->get_handle();
isa_ok( $lh, 'Woozle::eu_mt' );

print "# Test HTTP_ACCEPT_LANGUAGE...\n";
$ENV{'REQUEST_METHOD'}       = 'GET';
$ENV{'HTTP_ACCEPT_LANGUAGE'} = 'eu-MT';
$lh = Woozle->get_handle();
isa_ok( $lh, 'Woozle::eu_mt' );

$ENV{'HTTP_ACCEPT_LANGUAGE'} = 'x-plorp, zaz, eu-MT, i-klung';
$lh = Woozle->get_handle();
isa_ok( $lh, 'Woozle::eu_mt' );

$ENV{'HTTP_ACCEPT_LANGUAGE'} = 'x-plorp, zaz, eU-Mt, i-klung';
$lh = Woozle->get_handle();
isa_ok( $lh, 'Woozle::eu_mt' );
