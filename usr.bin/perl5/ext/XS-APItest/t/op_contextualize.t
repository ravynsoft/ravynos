use warnings;
use strict;
use Test::More tests => 1;

use XS::APItest;

XS::APItest::test_op_contextualize();
ok 1;

1;
