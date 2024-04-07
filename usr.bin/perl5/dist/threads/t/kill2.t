use strict;
use warnings;

BEGIN {
    require($ENV{PERL_CORE} ? '../../t/test.pl' : './t/test.pl');

    use Config;
    if (! $Config{'useithreads'}) {
        skip_all(q/Perl not compiled with 'useithreads'/);
    }
}

use ExtUtils::testlib;

use threads;

BEGIN {
    if (! eval 'use threads::shared; 1') {
        skip_all('threads::shared not available');
    }

    local $SIG{'HUP'} = sub {};
    my $thr = threads->create(sub {});
    eval { $thr->kill('HUP') };
    $thr->join();
    if ($@ && $@ =~ /safe signals/) {
        skip_all('Not using safe signals');
    }

    plan(4);
};

fresh_perl_is(<<'EOI', 'ok', { }, 'No signal handler in thread');
    use threads;
    use Thread::Semaphore;
    my $sema = Thread::Semaphore->new(0);
    my $test = sub {
        my $sema = shift;
        $sema->up();
        while(1) { sleep(1); }
    };
    my $thr = threads->create($test, $sema);
    $sema->down();
    $thr->detach();
    eval {
        $thr->kill('STOP');
    };
    print(($@ =~ /no signal handler set/) ? 'ok' : 'not ok');
EOI

fresh_perl_is(<<'EOI', 'ok', { }, 'Handler to signal mismatch');
    use threads;
    use Thread::Semaphore;
    my $sema = Thread::Semaphore->new(0);
    my $test = sub {
        my $sema = shift;
        $SIG{'TERM'} = sub { threads->exit() };
        $sema->up();
        while(1) { sleep(1); }
    };
    my $thr = threads->create($test, $sema);
    $sema->down();
    $thr->detach();
    eval {
        $thr->kill('STOP');
    };
    print(($@ =~ /no signal handler set/) ? 'ok' : 'not ok');
EOI

fresh_perl_is(<<'EOI', 'ok', { }, 'Handler and signal match');
    use threads;
    use Thread::Semaphore;
    my $sema = Thread::Semaphore->new(0);
    my $test = sub {
        my $sema = shift;
        $SIG{'STOP'} = sub { threads->exit() };
        $sema->up();
        while(1) { sleep(1); }
    };
    my $thr = threads->create($test, $sema);
    $sema->down();
    $thr->detach();
    eval {
        $thr->kill('STOP');
    };
    print((! $@) ? 'ok' : 'not ok');
EOI

fresh_perl_is(<<'EOI', 'ok', { }, 'Ignore signal after thread finishes');
    use threads;

    my $thr = threads->create(sub {
        $SIG{KILL} = sub {
            threads->exit();
        };
        return 0;
    });

    until ($thr->is_joinable()) {
        threads->yield();
    }

    $thr->kill('SIGKILL');
    $thr->join();
    print((! $@) ? 'ok' : 'not ok');
EOI

exit(0);

# EOF
