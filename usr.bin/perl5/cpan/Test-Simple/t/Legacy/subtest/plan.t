#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ( '../lib', 'lib' );
    }
    else {
        unshift @INC, 't/lib';
    }
}

use strict;
use warnings;

use Test::Builder::NoOutput;

use Test::More tests => 6;

# Formatting may change if we're running under Test::Harness.
$ENV{HARNESS_ACTIVE} = 0;

{
    ok defined &subtest, 'subtest() should be exported to our namespace';
    is prototype('subtest'), undef, '... has no prototype';

    subtest 'subtest with plan', sub {
        plan tests => 2;
        ok 1, 'planned subtests should work';
        ok 1, '... and support more than one test';
    };
    subtest 'subtest without plan', sub {
        plan 'no_plan';
        ok 1, 'no_plan subtests should work';
        ok 1, '... and support more than one test';
        ok 1, '... no matter how many tests are run';
    };
    subtest 'subtest with implicit done_testing()', sub {
        ok 1, 'subtests with an implicit done testing should work';
        ok 1, '... and support more than one test';
        ok 1, '... no matter how many tests are run';
    };
    subtest 'subtest with explicit done_testing()', sub {
        ok 1, 'subtests with an explicit done testing should work';
        ok 1, '... and support more than one test';
        ok 1, '... no matter how many tests are run';
        done_testing();
    };
}
