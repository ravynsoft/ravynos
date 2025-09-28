use Test2::Tools::Tiny;
use strict;
use warnings;

use Test2::IPC;
use Test2::Util qw/CAN_THREAD CAN_REALLY_FORK/;

skip_all 'No IPC' unless CAN_REALLY_FORK || CAN_THREAD;

if (CAN_REALLY_FORK) {
    my $pid = fork;
    die "Failed to fork: $!" unless defined $pid;
    if ($pid) {
        waitpid($pid, 0)
    }
    else {
        ok(1, "Pass fork");
        exit 0;
    }
}

if (CAN_THREAD) {
    require threads;
    my $thread = threads->create(
        sub {
            ok(1, "Pass thread");
        }
    );

    $thread->join;
}

done_testing;
