use warnings;
use strict;
use Test::More tests => 2;

use XS::APItest;

XS::APItest::test_op_list();
ok 1;

XS::APItest::test_op_linklist();
ok 1;

1;
