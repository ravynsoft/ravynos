# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 10;

use bignum downgrade => undef;

is(bignum -> downgrade(), undef,
   "bignum's downgrade class is undefined");
is(bignum -> upgrade(), "Math::BigFloat",
  "bignum's upgrade class is Math::BigFloat");

is(Math::BigFloat -> downgrade(), undef,
  "Math::BigFloat's downgrade class is undefined");
is(Math::BigInt -> upgrade(), "Math::BigFloat",
  "Math::BigInt's upgrade class is Math::BigFloat");

my $i1 = 7;
my $i2 = 2;
my $f1 = 3.75;
my $f2 = 1.25;

is(ref($i1), "Math::BigInt", "literal $i1 is a Math::BigInt");
is(ref($f1), "Math::BigFloat", "literal $f1 is a Math::BigFloat");

# Verify that the result is upgraded to a Math::BigFloat.

cmp_ok($i1/$i2, "==", "3.5", "$i1/$i2 is 3.5");
is(ref($i1/$i2), "Math::BigFloat",
   "$i1/$i2 is 3.5 as a Math::BigFloat due to upgrading");

# Verify that the result is not downgraded to a Math::BigInt.

cmp_ok($f1/$f2, "==", "3", "$f1/$f2 is 3");
is(ref($f1/$f2), "Math::BigFloat",
   "$f1/$f2 is 3 as a Math::BigFloat due to no downgrading");
