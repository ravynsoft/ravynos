# -*- mode: perl; -*-

# check that simple requiring Math::BigFloat and then new() works

use strict;
use warnings;

use Test::More tests => 1;

require Math::BigFloat;

my $x = Math::BigFloat->new(1);
++$x;
is($x, 2, '$x is 2');

# all tests done
