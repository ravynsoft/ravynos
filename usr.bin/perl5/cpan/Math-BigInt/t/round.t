# -*- mode: perl; -*-

# test rounding with non-integer A and P parameters

use strict;
use warnings;

use Test::More tests => 95;

use Math::BigFloat;

my $mbf = 'Math::BigFloat';
#my $mbi = 'Math::BigInt';

my $x = $mbf->new('123456.123456');

# unary ops with A
_do_a($x, 'round', 3, '123000');
_do_a($x, 'bfround', 3, '123500');
_do_a($x, 'bfround', 2, '123460');
_do_a($x, 'bfround', -2, '123456.12');
_do_a($x, 'bfround', -3, '123456.123');

_do_a($x, 'bround', 4, '123500');
_do_a($x, 'bround', 3, '123000');
_do_a($x, 'bround', 2, '120000');

_do_a($x, 'bsqrt', 4, '351.4');
_do_a($x, 'bsqrt', 3, '351');
_do_a($x, 'bsqrt', 2, '350');

# setting P
_do_p($x, 'bsqrt', 2, '350');
_do_p($x, 'bsqrt', -2, '351.36');

# binary ops
_do_2_a($x, 'bdiv', 2, 6, '61728.1');
_do_2_a($x, 'bdiv', 2, 4, '61730');
_do_2_a($x, 'bdiv', 2, 3, '61700');

_do_2_p($x, 'bdiv', 2, -6, '61728.061728');
_do_2_p($x, 'bdiv', 2, -4, '61728.0617');
_do_2_p($x, 'bdiv', 2, -3, '61728.062');

# all tests done

#############################################################################

sub _do_a {
    my ($x, $method, $A, $result) = @_;

    is($x->copy->$method($A), $result, "$method($A)");
    is($x->copy->$method($A.'.1'), $result, "$method(${A}.1)");
    is($x->copy->$method($A.'.5'), $result, "$method(${A}.5)");
    is($x->copy->$method($A.'.6'), $result, "$method(${A}.6)");
    is($x->copy->$method($A.'.9'), $result, "$method(${A}.9)");
}

sub _do_p {
    my ($x, $method, $P, $result) = @_;

    is($x->copy->$method(undef, $P), $result, "$method(undef, $P)");
    is($x->copy->$method(undef, $P.'.1'), $result, "$method(undef, ${P}.1)");
    is($x->copy->$method(undef, $P.'.5'), $result, "$method(undef.${P}.5)");
    is($x->copy->$method(undef, $P.'.6'), $result, "$method(undef, ${P}.6)");
    is($x->copy->$method(undef, $P.'.9'), $result, "$method(undef, ${P}.9)");
}

sub _do_2_a {
    my ($x, $method, $y, $A, $result) = @_;

    my $cy = $mbf->new($y);

    is($x->copy->$method($cy, $A), $result, "$method($cy, $A)");
    is($x->copy->$method($cy, $A.'.1'), $result, "$method($cy, ${A}.1)");
    is($x->copy->$method($cy, $A.'.5'), $result, "$method($cy, ${A}.5)");
    is($x->copy->$method($cy, $A.'.6'), $result, "$method($cy, ${A}.6)");
    is($x->copy->$method($cy, $A.'.9'), $result, "$method($cy, ${A}.9)");
}

sub _do_2_p {
    my ($x, $method, $y, $P, $result) = @_;

    my $cy = $mbf->new($y);

    is($x->copy->$method($cy, undef, $P), $result,
       "$method(undef, $P)");
    is($x->copy->$method($cy, undef, $P.'.1'), $result,
       "$method($cy, undef, ${P}.1)");
    is($x->copy->$method($cy, undef, $P.'.5'), $result,
       "$method($cy, undef, ${P}.5)");
    is($x->copy->$method($cy, undef, $P.'.6'), $result,
       "$method($cy, undef, ${P}.6)");
    is($x->copy->$method($cy, undef, $P.'.9'), $result,
       "$method($cy, undef, ${P}.9)");
}
