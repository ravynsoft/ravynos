# -*- mode: perl; -*-

# Test bpi() and bexp()

use strict;
use warnings;

use Test::More tests => 8;

use Math::BigFloat;

#############################################################################

my $pi = Math::BigFloat::bpi();

is($pi->{_a}, undef, 'A is not defined');
is($pi->{_p}, undef, 'P is not defined');

$pi = Math::BigFloat->bpi();

is($pi->{_a}, undef, 'A is not defined');
is($pi->{_p}, undef, 'P is not defined');

$pi = Math::BigFloat->bpi(10);

is($pi->{_a}, 10,    'A is defined');
is($pi->{_p}, undef, 'P is not defined');

#############################################################################

my $e = Math::BigFloat->new(1)->bexp();

is($e->{_a}, undef, 'A is not defined');
is($e->{_p}, undef, 'P is not defined');
