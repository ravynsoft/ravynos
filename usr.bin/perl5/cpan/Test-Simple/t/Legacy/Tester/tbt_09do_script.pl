#!/usr/bin/perl

use strict;
use warnings;

isnt($0, __FILE__, 'code is not executing directly');

test_out("not ok 1 - one");
test_fail(+1);
ok(0,"one");
test_test('test_fail caught fail message inside a do');

1;
