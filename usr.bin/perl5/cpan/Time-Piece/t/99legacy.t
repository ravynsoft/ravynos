use strict;
use warnings;
no warnings 'deprecated';

use Test::More tests => 5;

BEGIN { use_ok('Time::Piece'); }

# The parse() legacy method is deprecated and will not be maintained.
# The tests in this script illustrate both its functionality and some of
# its bugs. This script should be removed from the test suite once
# parse() has been deleted from Time::Piece.

SKIP: {
    skip "Linux only", 4 if $^O !~ /linux/i;

    my $timestring = '2000-01-01T06:00:00';
    my $t1         = Time::Piece->parse($timestring);
    isnt( $t1->datetime, $timestring, 'LEGACY: parse string months fail' );
    my $t2 = $t1->parse( 0, 0, 6, 1, 0, 100 );
    is( $t2->datetime, $timestring, 'LEGACY: parse array' );
    eval { $t2 = Time::Piece->parse(); };
    is( $t2->datetime, $timestring, 'LEGACY: parse with no args dies' );
    eval { $t2 = Time::Piece::parse( 0, 0, 12, 1, 0, 100 ); };
    is( $t2->datetime, $timestring, 'LEGACY: parse as non-method dies' );
}
