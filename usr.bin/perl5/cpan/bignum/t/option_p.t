# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 6;

{
    my $class = "Math::BigInt";

    use bigint p => "12";
    cmp_ok($class -> precision(), "==", 12, "$class precision = 12");

    bigint -> import(precision => "23");
    cmp_ok($class -> precision(), "==", 23, "$class precision = 23");
}

{
    my $class = "Math::BigFloat";

    use bigfloat p => "13";
    cmp_ok($class -> precision(), "==", 13, "$class precision = 12");

    bigfloat -> import(precision => "24");
    cmp_ok($class -> precision(), "==", 24, "$class precision = 23");
}

{
    my $class = "Math::BigRat";

    use bigrat p => "14";
    cmp_ok($class -> precision(), "==", 14, "$class precision = 12");

    bigrat -> import(precision => "25");
    cmp_ok($class -> precision(), "==", 25, "$class precision = 23");
}
