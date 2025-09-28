use warnings;
use strict;
use Test::More tests => 1;

use XS::APItest;

BEGIN { XS::APItest::test_copyhints(); }
ok 1;

1;
