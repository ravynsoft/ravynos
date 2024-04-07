use strict; use warnings;
use Fcntl;

use lib 't/lib';
use DBMTest 'GDBM_File', is_scalar_only => 1;

test_dbm $file, &GDBM_File::GDBM_WRCREAT, 0666;
cleanup;
