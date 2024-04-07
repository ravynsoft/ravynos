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

    local $SIG{'HUP'} = sub {};
    my $thr = threads->create(sub {});
    eval { $thr->kill('HUP') };
    $thr->join();
    if ($@ && $@ =~ /safe signals/) {
        print("1..0 # SKIP Not using safe signals\n");
        exit(0);
    }

    require Thread::Queue;
    require Thread::Semaphore;

    $| = 1;
    print("1..18\n");   ### Number of tests that will be run ###
};


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

### Thread cancel ###

# Set up to capture warning when thread terminates
my @errs :shared;
$SIG{__WARN__} = sub { push(@errs, @_); };

sub thr_func {
    my $q = shift;

    # Thread 'cancellation' signal handler
    $SIG{'KILL'} = sub {
        $q->enqueue(1, 'Thread received signal');
        die("Thread killed\n");
    };

    # Thread sleeps until signalled
    $q->enqueue(1, 'Thread sleeping');
    sleep(1) for (1..10);
    # Should not go past here
    $q->enqueue(0, 'Thread terminated normally');
    return ('ERROR');
}

# Create thread
my $thr = threads->create('thr_func', $q);
ok($thr && $thr->tid() == 2, 'Created thread');
threads->yield();
sleep(1);

# Signal thread
ok($thr->kill('KILL') == $thr, 'Signalled thread');
threads->yield();

# Cleanup
my $rc = $thr->join();
ok(! $rc, 'No thread return value');

# Check for thread termination message
ok(@errs && $errs[0] =~ /Thread killed/, 'Thread termination warning');


### Thread suspend/resume ###

sub thr_func2
{
    my $q = shift;

    my $sema = shift;
    $q->enqueue($sema, 'Thread received semaphore');

    # Set up the signal handler for suspension/resumption
    $SIG{'STOP'} = sub {
        $q->enqueue(1, 'Thread suspending');
        $sema->down();
        $q->enqueue(1, 'Thread resuming');
        $sema->up();
    };

    # Set up the signal handler for graceful termination
    my $term = 0;
    $SIG{'TERM'} = sub {
        $q->enqueue(1, 'Thread caught termination signal');
        $term = 1;
    };

    # Do work until signalled to terminate
    while (! $term) {
        sleep(1);
    }

    $q->enqueue(1, 'Thread done');
    return ('OKAY');
}


# Create a semaphore for use in suspending the thread
my $sema = Thread::Semaphore->new();
ok($sema, 'Semaphore created');

# Create a thread and send it the semaphore
$thr = threads->create('thr_func2', $q, $sema);
ok($thr && $thr->tid() == 3, 'Created thread');
threads->yield();
sleep(1);

# Suspend the thread
$sema->down();
ok($thr->kill('STOP') == $thr, 'Suspended thread');

threads->yield();
sleep(1);

# Allow the thread to continue
$sema->up();

threads->yield();
sleep(1);

# Terminate the thread
ok($thr->kill('TERM') == $thr, 'Signalled thread to terminate');

$rc = $thr->join();
ok($rc eq 'OKAY', 'Thread return value');

ok($thr->kill('TERM') == $thr, 'Ignore signal to terminated thread');

exit(0);

# EOF
