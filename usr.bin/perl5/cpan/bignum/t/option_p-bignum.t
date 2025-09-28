# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 4;

{
    use bignum p => "12";
    for my $class ("Math::BigInt", "Math::BigFloat") {
        cmp_ok($class -> precision(), "==", 12, "$class precision = 12");
    }

    bignum -> import(precision => "23");
    for my $class ("Math::BigInt", "Math::BigFloat") {
        cmp_ok($class -> precision(), "==", 23, "$class precision = 23");
    }
}
