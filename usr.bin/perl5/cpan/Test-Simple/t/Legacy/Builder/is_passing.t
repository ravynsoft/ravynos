#!/usr/bin/perl -w

use strict;
use lib 't/lib';

# We're going to need to override exit() later
BEGIN {
    require Test2::Hub;
    no warnings 'redefine';
    *Test2::Hub::terminate = sub {
        my $status = @_ ? 0 : shift;
        CORE::exit $status;
    };
}

use Test::More;
use Test::Builder;
use Test::Builder::NoOutput;

{
    my $tb = Test::Builder::NoOutput->create;
    ok $tb->is_passing, "a fresh TB object is passing";

    $tb->ok(1);
    ok $tb->is_passing, "  still passing after a test";

    $tb->ok(0);
    ok !$tb->is_passing, "  not passing after a failing test";

    $tb->ok(1);
    ok !$tb->is_passing, "  a passing test doesn't resurrect it";

    $tb->done_testing(3);
    ok !$tb->is_passing, "  a successful plan doesn't help either";
}


# See if is_passing() notices a plan overrun
{
    my $tb = Test::Builder::NoOutput->create;
    $tb->plan( tests => 1 );
    $tb->ok(1);
    ok $tb->is_passing, "Passing with a plan";

    $tb->ok(1);
    ok !$tb->is_passing, "  passing test, but it overran the plan";
}


# is_passing() vs no_plan
{
    my $tb = Test::Builder::NoOutput->create;
    $tb->plan( "no_plan" );
    ok $tb->is_passing, "Passing with no_plan";

    $tb->ok(1);
    ok $tb->is_passing, "  still passing after a test";

    $tb->ok(1);
    ok $tb->is_passing, "  and another test";

    $tb->_ending;
    ok $tb->is_passing, "  and after the ending";
}

# is_passing() vs skip_all
{
    my $tb = Test::Builder::NoOutput->create;

    {
        no warnings 'redefine';
        local *Test2::Hub::terminate = sub { 1 };

        $tb->plan( "skip_all" );
    }
    ok $tb->is_passing, "Passing with skip_all";
}

# is_passing() vs done_testing(#)
{
    my $tb = Test::Builder::NoOutput->create;
    $tb->ok(1);
    $tb->done_testing(2);
    ok !$tb->is_passing, "All tests passed but done_testing() does not match";
}


# is_passing() with no tests run vs done_testing()
{
    my $tb = Test::Builder::NoOutput->create;
    $tb->done_testing();
    ok !$tb->is_passing, "No tests run with done_testing()";
}


# is_passing() with no tests run vs done_testing()
{
    my $tb = Test::Builder::NoOutput->create;
    $tb->ok(1);
    $tb->done_testing();
    ok $tb->is_passing, "All tests passed with done_testing()";
}


done_testing();
