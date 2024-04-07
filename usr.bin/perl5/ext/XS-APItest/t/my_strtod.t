#!perl -w
use strict;

use Test::More;
use Config;
use XS::APItest;

my ($nv, $rest);

# GH 19492 - d_strtod=undef build segfaults here
($nv, $rest) = my_strtod('17.265625x');
# Note that .265625 is 17/64 so it can be represented exactly
is($nv, 17.265625, 'my_strtod("17.265625x", &e) returns 17.265625');
is($rest, 'x', 'my_strtod("17.265625x", &e) sets e to "x"');

done_testing();
