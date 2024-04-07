# -*- mode: perl; -*-

# test that overloaded compare works when NaN are involved

use strict;
use warnings;

use Test::More tests => 26;

use Math::BigInt;
use Math::BigFloat;

compare('Math::BigInt');
compare('Math::BigFloat');

sub compare {
    my $class = shift;

    my $nan = $class->bnan();
    my $one = $class->bone();

    is($one, $one, "$class->bone() == $class->bone()");

    is($one != $nan, 1, "$class->bone() != $class->bnan()");
    is($nan != $one, 1, "$class->bnan() != $class->bone()");
    is($nan != $nan, 1, "$class->bnan() != $class->bnan()");

    is($nan == $one, '', "$class->bnan() == $class->bone()");
    is($one == $nan, '', "$class->bone() == $class->bnan()");
    is($nan == $nan, '', "$class->bnan() == $class->bnan()");

    is($nan <= $one, '', "$class->bnan() <= $class->bone()");
    is($one <= $nan, '', "$class->bone() <= $class->bnan()");
    is($nan <= $nan, '', "$class->bnan() <= $class->bnan()");

    is($nan >= $one, '', "$class->bnan() >= $class->bone()");
    is($one >= $nan, '', "$class->bone() >= $class->bnan()");
    is($nan >= $nan, '', "$class->bnan() >= $class->bnan()");
}
