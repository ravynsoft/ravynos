use strict;
use warnings;

BEGIN {
    # Import test.pl into its own package
    {
        package Test;
        require($ENV{PERL_CORE} ? '../../t/test.pl' : './t/test.pl');
    }

    use Config;
    if (! $Config{'useithreads'}) {
        Test::skip_all(q/Perl not compiled with 'useithreads'/);
    }
}

use ExtUtils::testlib;

use threads;

BEGIN {
    if (! eval 'use threads::shared; 1') {
        Test::skip_all(q/threads::shared not available/);
    }

    require Thread::Queue;

    $| = 1;
    print("1..29\n");   ### Number of tests that will be run ###
}

Test::watchdog(120);   # In case we get stuck

my $q = Thread::Queue->new();
my $TEST = 1;

sub ok
{
    $q->enqueue(@_);

    while ($q->pending()) {
        my $ok   = $q->dequeue();
        my $name = $q->dequeue();
        my $id   = $TEST++;

        if ($ok) {
            print("ok $id - $name\n");
        } else {
            print("not ok $id - $name\n");
            printf("# Failed test at line %d\n", (caller)[2]);
        }
    }
}


### Start of Testing ###
ok(1, 'Loaded');

# Tests freeing the Perl interpreter for each thread
# See http://www.nntp.perl.org/group/perl.perl5.porters/110772 for details

my ($COUNT, $STARTED) :shared;

sub threading_1 {
    my $q = shift;

    my $tid = threads->tid();
    $q->enqueue($tid, "Thread $tid started");

    my $id;
    {
        lock($STARTED);
        $STARTED++;
        $id = $STARTED;
    }
    if ($STARTED < 5) {
        sleep(1);
        threads->create('threading_1', $q)->detach();
    }

    if ($id == 1) {
        sleep(2);
    } elsif ($id == 2) {
        sleep(6);
    } elsif ($id == 3) {
        sleep(3);
    } elsif ($id == 4) {
        sleep(1);
    } else {
        sleep(2);
    }

    lock($COUNT);
    $COUNT++;
    cond_signal($COUNT);
    $q->enqueue($tid, "Thread $tid done");
}

{
    $STARTED = 0;
    $COUNT = 0;
    threads->create('threading_1', $q)->detach();
    {
        my $cnt = 0;
        while ($cnt < 5) {
            {
                lock($COUNT);
                cond_wait($COUNT) if ($COUNT < 5);
                $cnt = $COUNT;
            }
            threads->create(sub {
                threads->create(sub { })->join();
            })->join();
        }
    }
    sleep(1);
}
ok($COUNT == 5, "Done - $COUNT threads");


sub threading_2 {
    my $q = shift;

    my $tid = threads->tid();
    $q->enqueue($tid, "Thread $tid started");

    {
        lock($STARTED);
        $STARTED++;
    }
    if ($STARTED < 5) {
        threads->create('threading_2', $q)->detach();
    }
    threads->yield();

    lock($COUNT);
    $COUNT++;
    cond_signal($COUNT);

    $q->enqueue($tid, "Thread $tid done");
}

{
    $STARTED = 0;
    $COUNT = 0;
    threads->create('threading_2', $q)->detach();
    threads->create(sub {
        threads->create(sub { })->join();
    })->join();
    {
        lock($COUNT);
        while ($COUNT < 5) {
            cond_wait($COUNT);
        }
    }
    sleep(1);
}
ok($COUNT == 5, "Done - $COUNT threads");


{
    threads->create(sub { })->join();
}
ok(1, 'Join');


sub threading_3 {
    my $q = shift;

    my $tid = threads->tid();
    $q->enqueue($tid, "Thread $tid started");

    {
        threads->create(sub {
            my $q = shift;

            my $tid = threads->tid();
            $q->enqueue($tid, "Thread $tid started");

            sleep(1);

            lock($COUNT);
            $COUNT++;
            cond_signal($COUNT);

            $q->enqueue($tid, "Thread $tid done");
        }, $q)->detach();
    }

    lock($COUNT);
    $COUNT++;
    cond_signal($COUNT);

    $q->enqueue($tid, "Thread $tid done");
}

{
    $COUNT = 0;
    threads->create(sub {
        threads->create('threading_3', $q)->detach();
        {
            lock($COUNT);
            while ($COUNT < 2) {
                cond_wait($COUNT);
            }
        }
    })->join();
    sleep(1);
}
ok($COUNT == 2, "Done - $COUNT threads");

exit(0);

# EOF
