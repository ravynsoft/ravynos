# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 14;

use bignum;

is(bignum -> downgrade(), "Math::BigInt",
   "bignum's downgrade class is Math::BigInt");
is(bignum -> upgrade(), "Math::BigFloat",
  "bignum's upgrade class is Math::BigFloat");

is(Math::BigFloat -> downgrade(), "Math::BigInt",
  "Math::BigFloat's downgrade class is Math::BigInt");
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
is(ref($i1/$i2), "Math::BigFloat", "$i1/$i2 is 3.5 as a Math::BigFloat");

# Verify that the result is downgraded to a Math::BigInt.

cmp_ok($f1/$f2, "==", "3", "$f1/$f2 is 3");
is(ref($f1/$f2), "Math::BigInt", "$f1/$f2 is 3 as a Math::BigInt");

# Change the upgrade class during runtime.

SKIP: {
    eval "use Math::BigRat";
    skip "Math::BigRat not installed", 4 if $@;

    bignum -> upgrade("Math::BigRat");

    my $r1 = 3.75;
    my $r2 = 1.25;

    # Verify that the result is upgraded to a Math::BigRat.

    cmp_ok($i1/$i2, "==", "3.5", "$i1/$i2 is 3.5");
    is(ref($i1/$i2), "Math::BigRat", "$i1/$i2 is 3.5 as a Math::BigRat");

    # Verify that the result is downgraded to a Math::BigInt.

    cmp_ok($r1/$r2, "==", "3", "($r1)/($r2) is 3");
    is(ref($r1/$r2), "Math::BigInt", "($r1)/($r2) is 3 as a Math::BigInt");
};
