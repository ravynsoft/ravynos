#!./perl
use strict;
use Test::More tests => 4;

use SDBM_File;

# has always been .pag
is(SDBM_File::PAGFEXT, ".pag", "PAGFEXT");

# depends on the platform
like(SDBM_File::DIRFEXT, qr/^\.(?:sdbm_)?dir$/, "DIRFEXT");

is(SDBM_File::PAIRMAX, 1008, "PAIRMAX");

ok(eval { SDBM_File->import(qw(PAIRMAX PAGFEXT DIRFEXT)); 1 }, "exportable");

