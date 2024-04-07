# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More;

BEGIN {
    eval { require Math::BigInt::Pari; };
    if ($@) {
        plan skip_all => "Math::BigInt::Pari not installed";
    } else {
        plan tests => "1";
    }
}

use bignum only => "Pari";

my $x = 1;
is($x -> config("lib"), "Math::BigInt::Pari",
   "backend is Math::BigInt::Pari");
