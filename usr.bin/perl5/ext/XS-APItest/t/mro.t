#!perl

use XS::APItest;
use Test::More;

plan tests => 1;

use mro;
mro::set_mro(AA => 'justisa');

@AA::ISA = qw "BB CC";

sub BB::fromp { "bb" }
sub CC::fromp { "cc" }

is fromp AA, 'bb', 'first elem of linearisation is not ignored';
