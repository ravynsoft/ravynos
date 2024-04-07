#!/usr/bin/perl

use Test::Builder::Tester tests => 5;
use Test::More;

# test_fail

test_out("not ok 1 - one");
test_fail(+1);
ok(0,"one");

test_out("not ok 2 - two");
test_fail(+2);

ok(0,"two");

test_test("test fail");

test_fail(+2);
test_out("not ok 1 - one");
ok(0,"one");
test_test("test_fail first");

# test_diag

use Test::Builder;
my $test = new Test::Builder;

test_diag("this is a test string","so is this");
$test->diag("this is a test string\n", "so is this\n");
test_test("test diag");

test_diag("this is a test string","so is this");
$test->diag("this is a test string\n");
$test->diag("so is this\n");
test_test("test diag multi line");

test_diag("this is a test string");
test_diag("so is this");
$test->diag("this is a test string\n");
$test->diag("so is this\n");
test_test("test diag multiple");


