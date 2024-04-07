#!/usr/bin/perl -Tw

use strict;
use Test::More tests => 5;

BEGIN {
    use_ok( 'Locale::Maketext' );
}

# declare some classes...
{
  package Woozle;
  our @ISA = ('Locale::Maketext');
  sub dubbil   { return $_[1] * 2 }
  sub numerate { return $_[2] . 'en' }
}
{
  package Woozle::elx;
  our @ISA = ('Woozle');
  our %Lexicon = (
   'd2' => 'hum [dubbil,_1]',
   'd3' => 'hoo [quant,_1,zaz]',
   'd4' => 'hoo [*,_1,zaz]',
  );
  keys %Lexicon; # dodges the 'used only once' warning
}

my $lh = Woozle->get_handle('elx');
isa_ok( $lh, 'Woozle::elx' );

is( $lh->maketext('d2', 7), 'hum 14' );
is( $lh->maketext('d3', 7), 'hoo 7 zazen' );
is( $lh->maketext('d4', 7), 'hoo 7 zazen' );
