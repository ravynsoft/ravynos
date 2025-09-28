use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

use ExtUtils::testlib;

sub ok {
    my ($id, $ok, $name) = @_;

    # You have to do it this way or VMS will get confused.
    if ($ok) {
        print("ok $id - $name\n");
    } else {
        print("not ok $id - $name\n");
        printf("# Failed test at line %d\n", (caller)[2]);
    }

    return ($ok);
}

BEGIN {
    $| = 1;
    print("1..15\n");   ### Number of tests that will be run ###
};

use threads;
ok(1, 1, 'Loaded');

### Start of Testing ###

ok(2, scalar @{[threads->list()]} == 0, 'No threads yet');

threads->create(sub {})->join();
ok(3, scalar @{[threads->list()]} == 0, 'Empty thread list after join');

my $thread = threads->create(sub {});
ok(4, scalar(threads->list()) == 1, 'Non-empty thread list');
ok(5, threads->list() == 1,             'Non-empty thread list');
$thread->join();
ok(6, scalar @{[threads->list()]} == 0, 'Thread list empty again');
ok(7, threads->list() == 0,             'Thread list empty again');

$thread = threads->create(sub {
    ok(8, threads->list() == 1, 'Non-empty thread list in thread');
    ok(9, threads->self == (threads->list())[0], 'Self in thread list')
});

threads->yield; # help out non-preemptive thread implementations
sleep 1;

ok(10, scalar(threads->list()) == 1, 'Thread count 1');
ok(11, threads->list() == 1,             'Thread count 1');
my $cnt = threads->list();
ok(12, $cnt == 1,                        'Thread count 1');
my ($thr_x) = threads->list();
ok(13, $thread == $thr_x,                'Thread in list');
$thread->join();
ok(14, scalar @{[threads->list()]} == 0, 'Thread list empty');
ok(15, threads->list() == 0,             'Thread list empty');

exit(0);

# EOF
