#!/usr/bin/perl -Tw

use strict;
use Test::More tests => 3;

BEGIN {
    use_ok( 'Locale::Maketext', 1.01 );
}

use utf8;

# declare some classes...
{
    package Woozle;
    our @ISA = ('Locale::Maketext');
    sub dubbil   { return $_[1] * 2 . chr(2000) }
    sub numerate { return $_[2] . 'en' }
}
{
    package Woozle::eu_mt;
    our @ISA = ('Woozle');
    our %Lexicon = (
        'd2' => chr(1000) . 'hum [dubbil,_1]',
        'd3' => chr(1000) . 'hoo [quant,_1,zaz]',
        'd4' => chr(1000) . 'hoo [*,_1,zaz]',
    );
    keys %Lexicon; # dodges the 'used only once' warning
}

my $lh = Woozle->get_handle('eu-mt');
isa_ok( $lh, 'Woozle::eu_mt' );
is( $lh->maketext('d2', 7), chr(1000).'hum 14'.chr(2000) );

