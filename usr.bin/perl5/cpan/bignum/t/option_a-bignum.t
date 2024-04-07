# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 4;

{
    use bignum a => "12";
    for my $class ("Math::BigInt", "Math::BigFloat") {
        cmp_ok($class -> accuracy(), "==", 12, "$class accuracy = 12");
    }

    bignum -> import(accuracy => "23");
    for my $class ("Math::BigInt", "Math::BigFloat") {
        cmp_ok($class -> accuracy(), "==", 23, "$class accuracy = 23");
    }
}
