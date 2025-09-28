#!./perl -w
use strict;

use Test::More;
use Config;
use File::Temp 'tempdir';
use File::Spec;
use Fcntl qw( :mode );

BEGIN {
    plan(skip_all => "GDBM_File was not built")
	unless $Config{extensions} =~ /\bGDBM_File\b/;

    plan(tests => 7);
    use_ok('GDBM_File');
}

SKIP: {
    skip "GDBM_File crash tolerance not available", 6,
        unless GDBM_File->crash_tolerance_status;

    my $wd = tempdir(CLEANUP => 1);
    chdir $wd;

    sub createdb {
        my ($name, $type, $code) = @_;
        my %h;
        $type //= 0;
        my $dbh = tie(%h, 'GDBM_File', $name, GDBM_NEWDB|$type, 0640);
        if ($code) {
            &{$code}($dbh, \%h);
        }
        untie %h
    }
    my $even = 'a.db';
    my $odd = 'b.db';
    my $time = time;

    #
    # Valid cases
    #

    # access modes
    createdb($even);
    createdb($odd);
    chmod S_IRUSR, $even;
    chmod S_IWUSR, $odd;
    is_deeply([GDBM_File->latest_snapshot($even, $odd)],
  	      [ 'a.db', GDBM_SNAPSHOT_OK ], "different acess modes");

    # mtimes
    chmod S_IRUSR, $odd;
    utime($time, $time, $even);
    utime($time, $time-5, $odd);
    is_deeply([GDBM_File->latest_snapshot($even, $odd)],
 	      [ 'a.db', GDBM_SNAPSHOT_OK ], "different mtimes");
    unlink $even, $odd;

    # numsync
    createdb($even, GDBM_NUMSYNC);
    createdb($odd, GDBM_NUMSYNC, sub { shift->sync });
    chmod S_IRUSR, $even, $odd;
    utime($time, $time, $even, $odd);
    is_deeply([GDBM_File->latest_snapshot($even, $odd)],
	      [ 'b.db', GDBM_SNAPSHOT_OK ], "different numsync value");

    #
    # Erroneous cases
    #
    
    unlink $even, $odd;

    # Same snapshots
    createdb($even);
    createdb($odd);
    chmod S_IRUSR, $even, $odd;
    utime($time, $time, $even, $odd);
    is_deeply([GDBM_File->latest_snapshot($even, $odd)],
	      [ undef, GDBM_SNAPSHOT_SAME ], "GDBM_SNAPSHOT_SAME");

    # Both writable
    chmod S_IWUSR, $even, $odd;
    is_deeply([GDBM_File->latest_snapshot($even, $odd)],
	      [ undef, GDBM_SNAPSHOT_BAD ], "GDBM_SNAPSHOT_BAD");

    # numsync difference > 1
    unlink $even, $odd;

    createdb($even, GDBM_NUMSYNC);
    createdb($odd, GDBM_NUMSYNC,
	 sub {
	     my $dbh = shift;
             $dbh->sync;
	     $dbh->sync;
	 });
    chmod S_IRUSR, $even, $odd;
    utime($time, $time, $even, $odd);
    is_deeply([GDBM_File->latest_snapshot($even, $odd)],
	      [ undef, GDBM_SNAPSHOT_SUSPICIOUS ], "GDBM_SNAPSHOT_SUSPICIOUS");
}
