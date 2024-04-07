#!/usr/bin/perl

use Test::Builder::Tester tests => 1;
use Test::More;

eval {
    test_test("foo");
};
like($@,
     "/Not testing\.  You must declare output with a test function first\./",
     "dies correctly on error");

