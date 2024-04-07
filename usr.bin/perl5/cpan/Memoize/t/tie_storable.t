use strict; use warnings;
use Test::More;

use lib 't/lib';
use DBMTest 'Memoize::Storable', extra_tests => 1;

test_dbm $file;
cleanup;

SKIP: {
	skip "skip Storable $Storable::VERSION too old for last_op_in_netorder", 1
		unless eval { Storable->VERSION('0.609') };
	{ tie my %cache, 'Memoize::Storable', $file, 'nstore' or die $! }
	ok Storable::last_op_in_netorder(), 'nstore option works';
	cleanup;
}
