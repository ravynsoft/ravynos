#!perl

use strict;
use warnings;

use Test::More tests => 7;

use XS::APItest qw(DEFSV);

is $_, undef;
is DEFSV, undef;
is \DEFSV, \$_;

DEFSV = "foo";
is DEFSV, "foo";
is $_, "foo";

$_ = "bar";
is DEFSV, "bar";
is $_, "bar";
