#!./perl

# These tests are not complete. Patches welcome.

use Test::More;

BEGIN {use_ok( 'Tie::Hash' )};

# these are "abstract virtual" parent methods
for my $method (qw( TIEHASH EXISTS )) {
	eval { Tie::Hash->$method() };
	like( $@, qr/doesn't define an? $method/, "croaks on inherited $method()" );
}

done_testing();
