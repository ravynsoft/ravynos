#!/usr/bin/env perl
# HARNESS-NO-STREAM

use strict;
use warnings;

use Test::Builder::Tester tests => 1;
use Test::More;

subtest 'foo' => sub {
    plan tests => 1;

    test_out("not ok 1 - foo");
    test_fail(+1);
    fail("foo");
    test_test("fail works");
};
