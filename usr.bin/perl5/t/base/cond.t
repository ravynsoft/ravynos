#!./perl

# make sure conditional operators work

print "1..4\n";

$x = '0';

$x eq $x && (print "ok 1 - operator eq\n");
$x ne $x && (print "not ok 1 - operator ne\n");
$x eq $x || (print "not ok 2 - operator eq\n");
$x ne $x || (print "ok 2 - operator ne\n");

$x == $x && (print "ok 3 - operator ==\n");
$x != $x && (print "not ok 3 - operator !=\n");
$x == $x || (print "not ok 4 - operator ==\n");
$x != $x || (print "ok 4 - operator !=\n");
