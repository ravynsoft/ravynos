# -*- mode: perl; -*-

# check that simple requiring Math::BigFloat and then binf() works

use strict;
use warnings;

use Test::More tests => 1;

require Math::BigFloat;

my $x = Math::BigFloat->binf();
is($x, 'inf', '$x is inf');

# all tests done
