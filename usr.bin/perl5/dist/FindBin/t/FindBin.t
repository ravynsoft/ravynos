#!./perl

use strict;
use warnings;

print "1..2\n";

use FindBin qw($Bin);

print "# $Bin\n";
print "not " unless $Bin =~ m,[/.]t\]?$,;
print "ok 1\n";

$0 = "-";
FindBin::again();

print "not " if $FindBin::Script ne "-";
print "ok 2\n";
