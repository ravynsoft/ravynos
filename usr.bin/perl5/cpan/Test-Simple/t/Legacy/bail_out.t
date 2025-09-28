#!/usr/bin/perl -w
# HARNESS-NO-STREAM
# HARNESS-NO-PRELOAD

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

my $Exit_Code;
BEGIN {
    *CORE::GLOBAL::exit = sub { $Exit_Code = shift; };
}

# This test uses multiple builders, the real one is using the top hub, we need
# to fix the ending.
Test2::API::test2_stack()->top->set_no_ending(1);

use Test::Builder;
use Test::More;

my $output;
my $TB = Test::More->builder;
$TB->output(\$output);

my $Test = Test::Builder->create;
$Test->level(0);

$Test->plan(tests => 3);

plan tests => 4;

BAIL_OUT("ROCKS FALL! EVERYONE DIES!");


$Test->is_eq( $output, <<'OUT' );
1..4
Bail out!  ROCKS FALL! EVERYONE DIES!
OUT

$Test->is_eq( $Exit_Code, 255 );

$Test->ok( $Test->can("BAILOUT"), "Backwards compat" );
