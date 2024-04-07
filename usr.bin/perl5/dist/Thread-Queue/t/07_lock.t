use strict;
use warnings;

BEGIN {
    use Config;
    if (! $Config{'useithreads'}) {
        print("1..0 # SKIP Perl not compiled with 'useithreads'\n");
        exit(0);
    }
}

use threads;
use Thread::Queue;
use Thread::Semaphore;

BEGIN { # perl RT 133382
if ($] == 5.008) {
    require 't/test.pl';   # Test::More work-alike for Perl 5.8.0
} else {
    require Test::More;
}
Test::More->import();
} # end BEGIN
plan('tests' => 3);

# The following tests locking a queue

my $q = Thread::Queue->new(1..10);
ok($q, 'New queue');

my $sm = Thread::Semaphore->new(0);
my $st = Thread::Semaphore->new(0);

my $thr = threads->create(sub {
    {
        lock($q);
        $sm->up();
        $st->down();
        threads::yield();
        select(undef, undef, undef, 0.1);
        my @x = $q->extract(5,2);
        is_deeply(\@x, [6,7], 'Thread dequeues under lock');
    }
});

$sm->down();
$st->up();
my @x = $q->dequeue_nb(100);
is_deeply(\@x, [1..5,8..10], 'Main dequeues');

$thr->join();

exit(0);

# EOF
