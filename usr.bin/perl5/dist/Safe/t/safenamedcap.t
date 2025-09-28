BEGIN {
    if ($] < 5.010) {
	print "1..0\n";
	exit 0;
    }
    require Config;
    import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/) {
	print "1..0\n";
	exit 0;
    }
}

use strict;
use Test::More;
use Safe;
plan(tests => 1);

BEGIN { Safe->new }
"foo" =~ /(?<foo>fo*)/;
is( $+{foo}, "foo", "Named capture works" );
