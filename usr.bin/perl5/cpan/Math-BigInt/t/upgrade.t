# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 2134            # tests in require'd file
                         + 6;           # tests in this file

use Math::BigInt;
use Math::BigFloat;

my $x = Math::BigInt -> new(9);
my $y = Math::BigInt -> new(4);

# Without upgrading.

my $zi = $x / $y;
cmp_ok($zi, "==", 2, "9/4 = 2 without upgrading");
is(ref($zi), "Math::BigInt", "9/4 gives a Math::BigInt without upgrading");

# With upgrading.

Math::BigInt -> upgrade("Math::BigFloat");
my $zf = $x / $y;
cmp_ok($zf, "==", 2.25, "9/4 = 2.25 with upgrading");
is(ref($zf), "Math::BigFloat", "9/4 gives a Math::BigFloat with upgrading");

# Other tests.

our ($CLASS, $EXPECTED_CLASS, $LIB);
$CLASS          = "Math::BigInt";
$EXPECTED_CLASS = "Math::BigFloat";
$LIB            = "Math::BigInt::Calc";         # backend

is(Math::BigInt->upgrade(), "Math::BigFloat",
   "Math::BigInt->upgrade()");
is(Math::BigInt->downgrade(), undef,
   "Math::BigInt->downgrade()");

require './t/upgrade.inc';      # all tests here for sharing
