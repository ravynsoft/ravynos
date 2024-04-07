# -*- mode: perl; -*-

# check that simple requiring BigRat works

use strict;
use warnings;

use Test::More tests => 1;

my ($x);

require Math::BigRat;
$x = Math::BigRat->new(1);
++$x;

is($x, 2, '$x got successfully modified');

# all tests done
