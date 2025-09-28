#!perl
BEGIN {
    chdir 't' if -d 't';
    @INC = "../lib";
}

use strict;
require './test.pl';
use Config qw(%Config);

# memory usage checked with top
$ENV{PERL_TEST_MEMORY} >= 2
    or skip_all("Need ~2GB for this test");
$Config{ptrsize} >= 8
    or skip_all("Need 64-bit pointers for this test");

plan(tests => 4);

my $space = " "; # avoid constant folding from doubling memory usage
# concatenation here increases memory usage significantly
my $work = $space x 0x80000002;
substr($work, 0x80000000) = "\n\n";

# this would SEGV
is(index($work, "\n"), 0x80000000, "test index() over 2G mark");

# this would simply fail
is(rindex($work, "\n"), 0x80000001, "test rindex() over 2G mark");

utf8::upgrade($work);

# this would SEGV
is(index($work, "\n"), 0x80000000, "test index() over 2G mark (utf8-ish)");

# this would simply fail
is(rindex($work, "\n"), 0x80000001, "test rindex() over 2G mark (utf8-ish)");

