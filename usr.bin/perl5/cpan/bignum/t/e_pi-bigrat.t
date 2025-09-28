# -*- mode: perl; -*-

###############################################################################
# test for e() and PI() exports

use strict;
use warnings;

use Test::More tests => 4;

use bigrat qw/e PI bexp bpi/;

is(e,  "2718281828459045235360287471352662497757/"
   . "1000000000000000000000000000000000000000", 'e');
is(PI, "3141592653589793238462643383279502884197/"
   . "1000000000000000000000000000000000000000", 'PI');

# These tests should actually produce big rationals, but this is not yet
# implemented. Fixme!

is(bexp(1, 10), "679570457/250000000", 'bexp(1, 10)');
is(bpi(10),     "1570796327/500000000", 'bpi(10)');
