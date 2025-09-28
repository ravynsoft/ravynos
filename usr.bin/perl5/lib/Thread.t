use strict;
use warnings;

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';

    use Config;
    if (! $Config{usethreads}) {
        print("1..0 # Skip: No threads\n");
        exit(0);
    }
}

use Thread qw(:DEFAULT async yield);

use Test::More;

my $lock;
{
    no warnings 'once';
    if ($threads::shared::threads_shared) {
        &threads::shared::share(\$lock);
    }
}


BASIC:
{
    sub thr_sub
    {
        lock($lock);
        my $self = Thread->self;
        return $self->tid;
    }

    my $thr;
    {
        lock($lock);

        $thr = Thread->new(\&thr_sub);

        isa_ok($thr, 'Thread');

        ok(! $thr->done(), 'Thread running');
        is($thr->tid, 1, "thread's tid");

        my ($thr2) = Thread->list;
        ok($thr2->equal($thr), '->list returned thread');
    }
    yield();
    sleep(1);

    ok($thr->done(), 'Thread done');
    is($thr->join(), 1, "->join returned thread's tid");
}

ASYNC:
{
    my $thr = async { Thread->self->tid; };
    isa_ok($thr, 'Thread');
    is($thr->tid, 2, "async thread's tid");
    is($thr->join, 2, "->join on async returned tid");
}

COND_:
{
    sub thr_wait
    {
        lock($lock);
        cond_wait($lock);
        return Thread->self->tid;
    }

    my $thr;
    {
        lock($lock);
        $thr = Thread->new(\&thr_wait);
        isa_ok($thr, 'Thread');
        ok(! $thr->done(), 'Thread running');
    }
    yield();
    sleep(1);

    {
        lock($lock);
        cond_signal($lock);
    }
    yield();
    sleep(1);

    ok($thr->done(), 'Thread done');
    is($thr->join(), 3, "->join returned thread's tid");
}

done_testing();
