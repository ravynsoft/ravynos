# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More;

BEGIN {
    eval "use Math::BigRat";
    plan skip_all => 'Math::BigRat required for these tests' if $@;
}

plan tests => 10;

use bignum upgrade => "Math::BigRat";

is(bignum -> downgrade(), "Math::BigInt",
  "bignum's upgrade class is Math::BigInt");
is(bignum -> upgrade(), "Math::BigRat",
  "bignum's downgrade class is Math::BigInt");

is(Math::BigInt -> upgrade(), "Math::BigRat",
  "Math::BigInt's upgrade class is Math::BigRat");
is(Math::BigRat -> downgrade(), "Math::BigInt",
  "Math::BigRat's downgrade class is Math::BigInt");

my $i1 = 7;
my $i2 = 2;
my $r1 = 3.75;
my $r2 = 1.25;

is(ref($i1), "Math::BigInt", "literal $i1 is a Math::BigInt");
is(ref($r1), "Math::BigRat", "literal $r1 is a Math::BigRat");

# Verify that the result is upgraded to a Math::BigRat.

cmp_ok($i1/$i2, "==", "3.5", "$i1/$i2 is 3.5");
is(ref($i1/$i2), "Math::BigRat", "$i1/$i2 is 3.5 as a Math::BigRat");

# Verify that the result is downgraded to a Math::BigInt.

cmp_ok($r1/$r2, "==", "3", "($r1)/($r2) is 3");
is(ref($r1/$r2), "Math::BigInt", "($r1)/($r2) is 3 as a Math::BigInt");
