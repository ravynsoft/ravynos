#!/usr/bin/perl

use Test::Builder::Tester tests => 10;
use Test::More;

ok(1,"This is a basic test");

test_out("ok 1 - tested");
ok(1,"tested");
test_test("captured okay on basic");

test_out("ok 1 - tested");
ok(1,"tested");
test_test("captured okay again without changing number");

ok(1,"test unrelated to Test::Builder::Tester");

test_out("ok 1 - one");
test_out("ok 2 - two");
ok(1,"one");
ok(2,"two");
test_test("multiple tests");

test_out(qr/ok 1 - tested\n/);
ok(1,"tested");
test_test("regexp matching");

test_out("not ok 1 - should fail");
test_err("#     Failed test ($0 at line 32)");
test_err("#          got: 'foo'");
test_err("#     expected: 'bar'");
is("foo","bar","should fail");
test_test("testing failing");


test_out("not ok 1");
test_out("not ok 2");
test_fail(+2);
test_fail(+1);
fail();  fail();
test_test("testing failing on the same line with no name");


test_out("not ok 1 - name");
test_out("not ok 2 - name");
test_fail(+2);
test_fail(+1);
fail("name");  fail("name");
test_test("testing failing on the same line with the same name");


test_out("not ok 1 - name # TODO Something");
test_out("#     Failed (TODO) test ($0 at line 56)");
TODO: { 
    local $TODO = "Something";
    fail("name");
}
test_test("testing failing with todo");

