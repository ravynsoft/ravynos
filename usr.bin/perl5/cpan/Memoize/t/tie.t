use strict; use warnings;
use Fcntl;

use lib 't/lib';
use DBMTest 'Memoize::AnyDBM_File', is_scalar_only => 1;

test_dbm $file, O_RDWR | O_CREAT, 0666;
cleanup;
