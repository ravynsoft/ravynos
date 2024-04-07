#!/usr/bin/perl -w

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't';
        @INC = ('../lib', 'lib');
    }
    else {
        unshift @INC, 't/lib';
    }
}

use Test::More tests => 7;
use Test::Builder;
use Test::Builder::NoOutput;

my $more_tb = Test::More->builder;
isa_ok $more_tb, 'Test::Builder';

is $more_tb, Test::More->builder, 'create does not interfere with ->builder';
is $more_tb, Test::Builder->new,  '       does not interfere with ->new';

{
    my $new_tb = Test::Builder::NoOutput->create;

    isa_ok $new_tb,  'Test::Builder';
    isnt $more_tb, $new_tb, 'Test::Builder->create makes a new object';

    $new_tb->plan(tests => 1);
    $new_tb->ok(1, "a test");

    is $new_tb->read, <<'OUT';
1..1
ok 1 - a test
OUT
}

pass("Changing output() of new TB doesn't interfere with singleton");
