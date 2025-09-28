# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 4;

use Math::BigInt::Calc base_len => 1, use_int => 0;

my ($BASE_LEN, $BASE, $AND_BITS, $XOR_BITS, $OR_BITS,
    $BASE_LEN_SMALL, $MAX_VAL,
    $MAX_BITS, $MAX_EXP_F, $MAX_EXP_I, $USE_INT)
  = Math::BigInt::Calc->_base_len();

note(<<"EOF");

BASE_LEN  = $BASE_LEN
BASE      = $BASE
MAX_VAL   = $MAX_VAL
AND_BITS  = $AND_BITS
XOR_BITS  = $XOR_BITS
OR_BITS   = $OR_BITS
MAX_EXP_F = $MAX_EXP_F
MAX_EXP_I = $MAX_EXP_I
USE_INT   = $USE_INT
EOF

cmp_ok($BASE_LEN, "==", 1, '$BASE_LEN is 1');
cmp_ok($USE_INT,  "==", 0, '$USE_INT is 0');

my $LIB = 'Math::BigInt::Calc';

my $x = $LIB -> _new("31415926535897932384626433832");
my $str = $LIB -> _str($x);
is($str, "31415926535897932384626433832",
   "string representation of $LIB object");

is("[ @$x ]", "[ 2 3 8 3 3 4 6 2 6 4 8 3 2 3 9 7 9 8 5 3 5 6 2 9 5 1 4 1 3 ]",
   "internal representation of $LIB object");
