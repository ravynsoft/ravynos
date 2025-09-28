use warnings;
use strict;
use Test::More tests => 1;

use XS::APItest;

XS::APItest::test_rv2cv_op_cv();
ok 1;

1;
