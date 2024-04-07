#!/usr/bin/perl -w

# A subtest without a plan implicitly calls "done_testing"

use strict;
use Test::More;

pass "Before";

subtest 'basic' => sub {
    pass "Inside sub test";
};

subtest 'with done' => sub {
    pass 'This has done_testing';
    done_testing;
};

subtest 'with plan' => sub {
    plan tests => 1;
    pass 'I have a plan, Batman!';
};

subtest 'skipping' => sub {
    plan skip_all => 'Skipping';
    fail 'Shouldnt see me!';
};

pass "After";

done_testing;
