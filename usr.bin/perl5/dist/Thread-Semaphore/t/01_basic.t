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
plan('tests' => 10);

### Basic usage with multiple threads ###

my $sm = Thread::Semaphore->new();
my $st = Thread::Semaphore->new(0);
ok($sm, 'New Semaphore');
ok($st, 'New Semaphore');

my $token :shared = 0;

my @threads;

push @threads, threads->create(sub {
    $st->down();
    is($token++, 1, 'Thread 1 got semaphore');
    $sm->up();

    $st->down(4);
    is($token, 5, 'Thread 1 done');
    $sm->up();
});

push @threads, threads->create(sub {
    $st->down(2);
    is($token++, 3, 'Thread 2 got semaphore');
    $sm->up();

    $st->down(4);
    is($token, 5, 'Thread 2 done');
    $sm->up();
});

$sm->down();
is($token++, 0, 'Main has semaphore');
$st->up();

$sm->down();
is($token++, 2, 'Main got semaphore');
$st->up(2);

$sm->down();
is($token++, 4, 'Main re-got semaphore');
$st->up(9);

$sm->down(2);
$st->down();

$_->join for @threads;

ok(1, 'Main done');

exit(0);

# EOF
