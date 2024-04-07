# -*- mode: perl; -*-

# check that simple requiring Math::BigFloat and then bzero() works

use strict;
use warnings;

use Test::More tests => 1;

require Math::BigFloat;

my $x = Math::BigFloat->bzero(); $x++;
is($x, 1, '$x is 1');

# all tests done
