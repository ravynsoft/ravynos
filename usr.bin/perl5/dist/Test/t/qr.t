#!./perl -w

use strict;
BEGIN {
    if ($] < 5.005) {
	print "1..0\n";
	print "ok 1 # skipped; this test requires at least perl 5.005\n";
	exit;
    }
}
use Test; plan tests => 1;

ok 'abc', qr/b/;
