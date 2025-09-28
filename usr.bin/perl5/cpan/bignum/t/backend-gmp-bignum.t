# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More;

BEGIN {
    eval { require Math::BigInt::GMP; };
    if ($@) {
        plan skip_all => "Math::BigInt::GMP not installed";
    } else {
        plan tests => "1";
    }
}

use bignum only => "GMP";

my $x = 1;
is($x -> config("lib"), "Math::BigInt::GMP",
   "backend is Math::BigInt::GMP");
