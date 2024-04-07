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

my $goto = 0;
my $Exit_Code;
BEGIN {
    *CORE::GLOBAL::exit = sub { $Exit_Code = shift; goto XXX if $goto; CORE::exit($Exit_Code)};
}

use Test::Builder;
use Test::More;

my $skip = ref(Test::Builder->new->{Stack}->top->format) ne 'Test::Builder::Formatter';
plan skip_all => "This test cannot be run with the current formatter"
    if $skip;

$goto = 1;
my $output;
my $TB = Test::More->builder;
$TB->output(\$output);

my $Test = Test::Builder->create;
$Test->level(0);

$Test->plan(tests => 2);

plan tests => 4;

ok 'foo';
subtest 'bar' => sub {
    plan tests => 3;
    ok 'sub_foo';
    subtest 'sub_bar' => sub {
        plan tests => 3;
        ok 'sub_sub_foo';
        ok 'sub_sub_bar';
        BAIL_OUT("ROCKS FALL! EVERYONE DIES!");
        ok 'sub_sub_baz';
    };
    ok 'sub_baz';
};

XXX:

$Test->is_eq( $output, <<'OUT' );
1..4
ok 1
# Subtest: bar
    1..3
    ok 1
    # Subtest: sub_bar
        1..3
        ok 1
        ok 2
Bail out!  ROCKS FALL! EVERYONE DIES!
OUT

$Test->is_eq( $Exit_Code, 255 );

Test2::API::test2_stack()->top->set_no_ending(1);
