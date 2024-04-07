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

use threads;

BEGIN {
    if (! eval 'use threads::shared; 1') {
        print("1..0 # SKIP threads::shared not available\n");
        exit(0);
    }

    $| = 1;
    print("1..59\n");   ### Number of tests that will be run ###
};

my $TEST;
BEGIN {
    share($TEST);
    $TEST = 1;
}

ok(1, 'Loaded');

sub ok {
    my ($ok, $name) = @_;

    lock($TEST);
    my $id = $TEST++;

    # You have to do it this way or VMS will get confused.
    if ($ok) {
        print("ok $id - $name\n");
    } else {
        print("not ok $id - $name\n");
        printf("# Failed test at line %d\n", (caller)[2]);
    }

    return ($ok);
}


### Start of Testing ###

my ($READY, $GO, $DONE) :shared = (0, 0, 0);

sub do_thread
{
    {
        lock($DONE);
        $DONE = 0;
        lock($READY);
        $READY = 1;
        cond_signal($READY);
    }

    lock($GO);
    while (! $GO) {
        cond_wait($GO);
    }
    $GO = 0;

    lock($READY);
    $READY = 0;
    lock($DONE);
    $DONE = 1;
    cond_signal($DONE);
}

sub wait_until_ready
{
    lock($READY);
    while (! $READY) {
        cond_wait($READY);
    }
}

sub thread_go
{
    {
        lock($GO);
        $GO = 1;
        cond_signal($GO);
    }

    {
        lock($DONE);
        while (! $DONE) {
            cond_wait($DONE);
        }
    }
    threads->yield();
    sleep(1);
}


my $thr = threads->create('do_thread');
wait_until_ready();
ok($thr->is_running(),    'thread running');
ok(threads->list(threads::running) == 1,  'thread running list');
ok(! $thr->is_detached(), 'thread not detached');
ok(! $thr->is_joinable(), 'thread not joinable');
ok(threads->list(threads::joinable) == 0, 'thread joinable list');
ok(threads->list(threads::all) == 1, 'thread list');

thread_go();
ok(! $thr->is_running(),  'thread not running');
ok(threads->list(threads::running) == 0,  'thread running list');
ok(! $thr->is_detached(), 'thread not detached');
ok($thr->is_joinable(),   'thread joinable');
ok(threads->list(threads::joinable) == 1, 'thread joinable list');
ok(threads->list(threads::all) == 1, 'thread list');

$thr->join();
ok(! $thr->is_running(),  'thread not running');
ok(threads->list(threads::running) == 0,  'thread running list');
ok(! $thr->is_detached(), 'thread not detached');
ok(! $thr->is_joinable(), 'thread not joinable');
ok(threads->list(threads::joinable) == 0, 'thread joinable list');
ok(threads->list(threads::all) == 0, 'thread list');

$thr = threads->create('do_thread');
$thr->detach();
ok($thr->is_running(),    'thread running');
ok(threads->list(threads::running) == 0,  'thread running list');
ok($thr->is_detached(),   'thread detached');
ok(! $thr->is_joinable(), 'thread not joinable');
ok(threads->list(threads::joinable) == 0, 'thread joinable list');
ok(threads->list(threads::all) == 0, 'thread list');

thread_go();
ok(! $thr->is_running(),  'thread not running');
ok(threads->list(threads::running) == 0,  'thread running list');
ok($thr->is_detached(),   'thread detached');
ok(! $thr->is_joinable(), 'thread not joinable');
ok(threads->list(threads::joinable) == 0, 'thread joinable list');

$thr = threads->create(sub {
    ok(! threads->is_detached(), 'thread not detached');
    ok(threads->list(threads::running) == 1, 'thread running list');
    ok(threads->list(threads::joinable) == 0, 'thread joinable list');
    ok(threads->list(threads::all) == 1, 'thread list');
    threads->detach();
    do_thread();
    ok(threads->is_detached(),   'thread detached');
    ok(threads->list(threads::running) == 0, 'thread running list');
    ok(threads->list(threads::joinable) == 0, 'thread joinable list');
    ok(threads->list(threads::all) == 0, 'thread list');
});

wait_until_ready();
ok($thr->is_running(),    'thread running');
ok(threads->list(threads::running) == 0,  'thread running list');
ok($thr->is_detached(),   'thread detached');
ok(! $thr->is_joinable(), 'thread not joinable');
ok(threads->list(threads::joinable) == 0, 'thread joinable list');
ok(threads->list(threads::all) == 0, 'thread list');

thread_go();
ok(! $thr->is_running(),  'thread not running');
ok(threads->list(threads::running) == 0,  'thread running list');
ok($thr->is_detached(),   'thread detached');
ok(! $thr->is_joinable(), 'thread not joinable');
ok(threads->list(threads::joinable) == 0, 'thread joinable list');

{
    my $go : shared = 0;
    my $t = threads->create( sub {
        ok(! threads->is_detached(), 'thread not detached');
        ok(threads->list(threads::running) == 1, 'thread running list');
        ok(threads->list(threads::joinable) == 0, 'thread joinable list');
        ok(threads->list(threads::all) == 1, 'thread list');
        lock($go); $go = 1; cond_signal($go);
    });

    { lock ($go); cond_wait($go) until $go; }
    $t->join;
}

{
    my $rdy :shared = 0;
    sub thr_ready
    {
        lock($rdy);
        $rdy++;
        cond_signal($rdy);
    }

    my $go :shared = 0;
    sub thr_wait
    {
        lock($go);
        cond_wait($go) until $go;
    }

    my $done :shared = 0;
    sub thr_done
    {
        lock($done);
        $done++;
        cond_signal($done);
    }

    my $thr_routine = sub { thr_ready(); thr_wait(); thr_done(); };

    # Create 8 threads:
    #  3 running, blocking on $go
    #  2 running, blocking on $go, join pending
    #  2 running, blocking on join of above
    #  1 finished, unjoined

    for (1..3) { threads->create($thr_routine); }

    foreach my $t (map {threads->create($thr_routine)} 1..2) {
        threads->create(sub { thr_ready(); $_[0]->join; thr_done(); }, $t);
    }
    threads->create(sub { thr_ready(); thr_done(); });
    {
        lock($done);
        cond_wait($done) until ($done == 1);
    }
    {
        lock($rdy);
        cond_wait($rdy) until ($rdy == 8);
    }
    threads->yield();
    sleep(1);

    ok(threads->list(threads::running) == 5, 'thread running list');
    ok(threads->list(threads::joinable) == 1, 'thread joinable list');
    ok(threads->list(threads::all) == 6, 'thread all list');

    { lock($go); $go = 1; cond_broadcast($go); }
    {
        lock($done);
        cond_wait($done) until ($done == 8);
    }
    threads->yield();
    sleep(1);

    ok(threads->list(threads::running) == 0, 'thread running list');
    # Two awaiting join() have completed
    ok(threads->list(threads::joinable) == 6, 'thread joinable list');
    ok(threads->list(threads::all) == 6, 'thread all list');

    for (threads->list) { $_->join; }
}

exit(0);

# EOF
