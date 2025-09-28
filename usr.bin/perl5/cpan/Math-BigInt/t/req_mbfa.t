# -*- mode: perl; -*-

# check that simple requiring Math::BigFloat and then bnan() works

use strict;
use warnings;

use Test::More tests => 1;

require Math::BigFloat;

my $x = Math::BigFloat->bnan(1);
is($x, 'NaN', '$x is NaN');

# all tests done
