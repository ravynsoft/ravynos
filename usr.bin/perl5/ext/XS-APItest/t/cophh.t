use warnings;
use strict;
use Test::More tests => 2;

use XS::APItest;

XS::APItest::test_cophh();
ok 1;

is_deeply XS::APItest::example_cophh_2hv(), {
    "foo_1" => 111,
    "foo_\x{aa}" => 123,
    "foo_\x{bb}" => 456,
    "foo_\x{cc}" => 789,
    "foo_\x{666}" => 666,
};

1;
