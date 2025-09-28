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
use threads::shared;
use Thread::Semaphore;

if ($] == 5.008) {
    require './t/test.pl';   # Test::More work-alike for Perl 5.8.0
} else {
    require Test::More;
}
Test::More->import();
plan('tests' => 12);

### Basic usage with multiple threads ###

my $sm = Thread::Semaphore->new(0);
my $st = Thread::Semaphore->new(0);
ok($sm, 'New Semaphore');
ok($st, 'New Semaphore');

my $token :shared = 0;

my $thread = threads->create(sub {
    ok(! $st->down_nb(), 'Semaphore unavailable to thread');
    $sm->up();

    $st->down(2);
    ok(! $st->down_nb(5), 'Semaphore unavailable to thread');
    ok($st->down_nb(2), 'Thread 1 got semaphore');
    ok(! $st->down_nb(2), 'Semaphore unavailable to thread');
    ok($st->down_nb(1), 'Thread 1 got semaphore');
    ok(! $st->down_nb(), 'Semaphore unavailable to thread');
    is($token++, 1, 'Thread done');
    $sm->up();
});

$sm->down(1);
is($token++, 0, 'Main has semaphore');
$st->up();

ok(! $sm->down_nb(), 'Semaphore unavailable to main');
$st->up(4);

$sm->down();
is($token++, 2, 'Main got semaphore');

$thread->join;
exit(0);

# EOF
