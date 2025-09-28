#!perl

use strict;
use warnings;
use Test::More tests => 6;

use XS::APItest;

sub foo () { "abc" }

sub bar { }

is(XS::APItest::gv_const_sv(*foo), "abc", "on const glob");
is(XS::APItest::gv_const_sv("foo"), "abc", "on const by name");
is(XS::APItest::gv_const_sv($::{"foo"}), "abc", "on const by lookup");
is(XS::APItest::gv_const_sv(*bar), undef, "on non-const glob");
is(XS::APItest::gv_const_sv("bar"), undef, "on non-const by name");
is(XS::APItest::gv_const_sv($::{"bar"}), undef, "on non-const by lookup");
