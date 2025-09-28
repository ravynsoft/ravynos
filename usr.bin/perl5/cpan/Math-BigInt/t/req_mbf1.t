# -*- mode: perl; -*-

# check that simple requiring Math::BigFloat and then bone() works

use strict;
use warnings;

use Test::More tests => 1;

require Math::BigFloat;

my $x = Math::BigFloat->bone();
is($x, 1, '$x is 1');

# all tests done
