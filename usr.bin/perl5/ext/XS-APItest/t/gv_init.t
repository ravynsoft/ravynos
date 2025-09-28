#!perl

use strict;
use warnings;
use Test::More tests => 12;

use XS::APItest;

is XS::APItest::gv_init_type("sanity_check", 0, 0, 0), "*main::sanity_check";
ok $::{sanity_check};

for my $type (0..3) {
    is XS::APItest::gv_init_type("test$type", 0, 0, $type), "*main::test$type";
    ok $::{"test$type"};
}

my $latin_1 = "è";
my $utf8    = "\x{30cb}";

is XS::APItest::gv_init_type($latin_1, 0, 0, 1), "*main::$latin_1";
is XS::APItest::gv_init_type($utf8, 0, 0, 1), "*main::$utf8";
