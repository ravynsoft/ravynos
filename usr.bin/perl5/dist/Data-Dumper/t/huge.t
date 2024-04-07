#!./perl -w
#
# automated tests for Data::Dumper that need large amounts of memory; they
# are skipped unless PERL_TEST_MEMORY is set, and at least 10
#

use strict;
use warnings;

use Test::More;

use Config;
use Data::Dumper;

BEGIN {
    plan skip_all => 'Need 64-bit pointers for this test'
        if $Config{ptrsize} < 8;
    plan skip_all => 'Need ~10 GiB of core for this test'
        if !$ENV{PERL_TEST_MEMORY} || $ENV{PERL_TEST_MEMORY} < 10;
}

plan tests => 1;

{
    my $input = q/'/ x 2**31;
    my $len = length Dumper($input);
    # Each single-quote will get backslashed, so the output must have
    # stricly more than twice as many characters as the input.
    cmp_ok($len, '>', 2**32, 'correct output for huge all-quotable value');
    undef $input;
}
