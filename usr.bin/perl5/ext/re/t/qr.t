#!./perl

BEGIN {
	require Config;
	if (($Config::Config{'extensions'} !~ /\bre\b/) ){
        	print "1..0 # Skip -- Perl configured without re module\n";
		exit 0;
	}
}

use Test::More tests => 1;
isa_ok( qr//, "Regexp" );
