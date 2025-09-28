# -*- mode: perl; -*-

# check that simple requiring Math::BigInt works

use strict;
use warnings;

use Test::More tests => 1;

require Math::BigInt;

my $x = Math::BigInt->new(1);
++$x;

is($x, 2, '$x is 2');

# all tests done
