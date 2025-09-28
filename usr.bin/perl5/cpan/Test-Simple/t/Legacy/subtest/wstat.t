#!/usr/bin/perl -w

# Test that setting $? doesn't affect subtest success

use strict;
use Test::More;

subtest foo => sub {
    plan tests => 1;
    $? = 1;
    pass('bar');
};

is $?, 1, "exit code keeps on from a subtest";

subtest foo2 => sub {
    plan tests => 1;
    pass('bar2');
    $? = 1;
};

is $?, 1, "exit code keeps on from a subtest";

done_testing(4);
