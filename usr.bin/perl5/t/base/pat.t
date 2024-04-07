#!./perl

print "1..2\n";

# first test to see if we can run the tests.

$_ = 'test';
if (/^test/) { print "ok 1 - match regex\n"; } else { print "not ok 1 - match regex\n";}
if (/^foo/) { print "not ok 2 - match regex\n"; } else { print "ok 2 - match regex\n";}
