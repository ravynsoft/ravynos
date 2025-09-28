#!./perl

use strict;
use warnings;

use File::Find qw( find finddepth );
use File::Temp qw();
use Test::More;

my $warn_msg;

BEGIN {
    $SIG{'__WARN__'} = sub {
        $warn_msg = $_[0];
        warn "# $_[0]";
        return;
    }
}

sub test_find_correct_paths_with_follow {
    $warn_msg = '';
    my $dir = File::Temp->newdir('file-find-XXXXXX', TMPDIR => 1, CLEANUP => 1);

    find(
        {
            follow => 1,
            wanted => sub { return },
        },
        $dir,
    );

    unlike(
        $warn_msg,
        qr/Couldn't chdir/,
        'find: Derive absolute path correctly with follow => 1',
    );
}

sub test_finddepth_correct_paths_with_follow {
    $warn_msg = '';
    my $dir = File::Temp->newdir('file-find-XXXXXX', TMPDIR => 1, CLEANUP => 1);

    finddepth(
        {
            follow => 1,
            wanted => sub { return },
        },
        $dir,
    );

    unlike(
        $warn_msg,
        qr/Couldn't chdir/,
        'finddepth: Derive absolute path correctly with follow => 1',
    );
}
sub run {
    test_find_correct_paths_with_follow;
    test_finddepth_correct_paths_with_follow;
    done_testing;
}

run();
