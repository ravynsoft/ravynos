#!./perl
#
# All the tests in this file are ones that run exceptionally slowly
# (each test taking seconds or even minutes) in the absence of particular
# optimisations. Thus it is a sort of canary for optimisations being
# broken.
#
# Although it includes a watchdog timeout, this is set to a generous limit
# to allow for running on slow systems; therefore a broken optimisation
# might be indicated merely by this test file taking unusually long to
# run, rather than actually timing out.
#

use strict;
use warnings;
use 5.010;

sub run_tests;

$| = 1;


BEGIN {
    chdir 't' if -d 't';
    @INC = ('../lib');
    require Config; import Config;
    require './test.pl';
}

plan tests => 1;

use warnings;
use strict;

watchdog(60);

SKIP: {
    # RT #121975 COW speedup lost after e8c6a474

    # without COW, this test takes minutes; with COW, its less than a
    # second
    #
    skip  "PERL_NO_COW", 1 if $Config{ccflags} =~ /PERL_NO_COW/;

    my ($x, $y);
    $x = "x" x 1_000_000;
    $y = $x for 1..1_000_000;
    pass("COW 1Mb strings");
}

1;
