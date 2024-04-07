#!perl
BEGIN {
    chdir 't' if -d 't';
    @INC = "../lib";
}

use strict;
require './test.pl';
use Config qw(%Config);

$ENV{PERL_TEST_MEMORY} >= 3
    or skip_all("Need ~3Gb for this test");
$Config{ptrsize} >= 8
    or skip_all("Need 64-bit pointers for this test");

plan(1);

# RT #100514
my $x = "";
read(DATA, $x, 4, 0x80000000);
is(length $x, 0x80000004, "check we read to the correct offset");
__DATA__
Food

