# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 6;

{
    my $class = "Math::BigInt";

    use bigint a => "12";
    cmp_ok($class -> accuracy(), "==", 12, "$class accuracy = 12");

    bigint -> import(accuracy => "23");
    cmp_ok($class -> accuracy(), "==", 23, "$class accuracy = 23");
}

{
    my $class = "Math::BigFloat";

    use bigfloat a => "13";
    cmp_ok($class -> accuracy(), "==", 13, "$class accuracy = 12");

    bigfloat -> import(accuracy => "24");
    cmp_ok($class -> accuracy(), "==", 24, "$class accuracy = 23");
}

{
    my $class = "Math::BigRat";

    use bigrat a => "14";
    cmp_ok($class -> accuracy(), "==", 14, "$class accuracy = 12");

    bigrat -> import(accuracy => "25");
    cmp_ok($class -> accuracy(), "==", 25, "$class accuracy = 23");
}
