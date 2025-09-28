#!perl
BEGIN {
    chdir 't' if -d 't';
    @INC = "../lib";
}

use strict;
require './test.pl';
use Config qw(%Config);

$ENV{PERL_TEST_MEMORY} >= 1
    or skip_all("Need ~1Gb for this test");
$Config{ptrsize} >= 8
    or skip_all("Need 64-bit pointers for this test");

plan(7);

# RT #111730: Negative offset to vec in lvalue context

my $v = "";
ok(scalar eval { vec($v, 0x80000000, 1) = 1 }, "set a bit at a large offset");
ok(vec($v, 0x80000000, 1), "check a bit at a large offset");

ok(scalar eval { vec($v, 0x100000000, 1) = 1 },
   "set a bit at a larger offset");
ok(vec($v, 0x100000000, 1), "check a bit at a larger offset");

# real out of range values
ok(!eval { vec($v, -0x80000000, 1) = 1 },
   "shouldn't be able to set at a large negative offset");
ok(!eval { vec($v, -0x100000000, 1) = 1 },
   "shouldn't be able to set at a larger negative offset");

ok(!vec($v, 0, 1), "make sure we didn't wrap");
