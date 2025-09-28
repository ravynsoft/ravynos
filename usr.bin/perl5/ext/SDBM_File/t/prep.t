#!./perl
use strict;
use Test::More tests => 4;

use SDBM_File;
use File::Temp 'tempfile';
use Fcntl;

my ($dirfh, $dirname) = tempfile(UNLINK => 1);
my ($pagfh, $pagname) = tempfile(UNLINK => 1);

# close so Win32 allows them to be re-opened
close $dirfh;
close $pagfh;

{
    my %h;

    ok(eval { tie %h, "SDBM_File", $dirname, O_CREAT | O_RDWR | O_TRUNC, 0640, $pagname; 1 },
       "create SDBM with explicit filenames")
      or diag $@;
    is(keys %h, 0, "should be empty");

    # basic sanity checks, the real storage checks are done by sdbm.t
    $h{abc} = 1;
    $h{def} = 1;
}

{
    my %h;
    ok(eval { tie %h, "SDBM_File", $dirname, O_RDWR, 0640, $pagname; 1 },
       "open SDBM with explicit filenames");
    is_deeply([ sort keys  %h] , [ qw(abc def) ], "should have two keys");
}
