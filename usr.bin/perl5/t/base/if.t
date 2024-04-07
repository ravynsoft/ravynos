#!./perl

print "1..2\n";

# first test to see if we can run the tests.

$x = 'test';
if ($x eq $x) { print "ok 1 - if eq\n"; } else { print "not ok 1 - if eq\n";}
if ($x ne $x) { print "not ok 2 - if ne\n"; } else { print "ok 2 - if ne\n";}
