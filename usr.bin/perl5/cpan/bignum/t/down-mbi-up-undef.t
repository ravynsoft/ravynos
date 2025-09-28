# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More;

BEGIN {
    eval "use Math::BigRat";
    plan skip_all => 'Math::BigRat required for these tests' if $@;
}

plan tests => 10;

use bignum upgrade => undef;

is(bignum -> downgrade(), "Math::BigInt",
  "bignum's upgrade class is Math::BigInt");
is(bignum -> upgrade(), undef,
  "bignum's downgrade class is undefined");

is(Math::BigInt -> upgrade(), undef,
   "Math::BigInt's upgrade class is undefined");
is(Math::BigFloat -> downgrade(), "Math::BigInt",
  "Math::BigFloat's downgrade class is Math::BigInt");

my $i1 = 7;
my $i2 = 2;
my $f1 = 3.75;
my $f2 = 1.25;

is(ref($i1), "Math::BigInt", "literal $i1 is a Math::BigInt");
is(ref($f1), "Math::BigFloat", "literal $f1 is a Math::BigFloat");

# Verify that the result is not upgraded to a Math::BigFloat.

cmp_ok($i1/$i2, "==", "3", "$i1/$i2 is 3");
is(ref($i1/$i2), "Math::BigInt",
   "$i1/$i2 is 3 as a Math::BigInt due to no upgrading");

# Verify that the result is downgraded to a Math::BigInt.

cmp_ok($f1/$f2, "==", "3", "$f1/$f2 is 3");
is(ref($f1/$f2), "Math::BigInt",
   "$f1/$f2 is 3 as a Math::BigInt due to downgrading");
