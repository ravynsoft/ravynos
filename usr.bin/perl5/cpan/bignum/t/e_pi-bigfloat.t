# -*- mode: perl; -*-

###############################################################################
# test for e() and PI() exports

use strict;
use warnings;

use Test::More tests => 4;

use bigfloat qw/e PI bexp bpi/;

is(e,  "2.718281828459045235360287471352662497757", 'e');
is(PI, "3.141592653589793238462643383279502884197", 'PI');

is(bexp(1, 10), "2.718281828", 'bexp(1, 10)');
is(bpi(10),     "3.141592654", 'bpi(10)');
