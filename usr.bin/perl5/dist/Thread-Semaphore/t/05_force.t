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
plan('tests' => 8);

### Basic usage with multiple threads ###

my $sm = Thread::Semaphore->new(0);
my $st = Thread::Semaphore->new(0);
ok($sm, 'New Semaphore');
ok($st, 'New Semaphore');

my $token :shared = 0;

my $thread = threads->create(sub {
    $st->down_force(2);
    is($token++, 0, 'Thread got semaphore');
    $sm->up();

    $st->down();
    is($token++, 3, 'Thread done');
    $sm->up();
});

$sm->down();
is($token++, 1, 'Main has semaphore');
$st->up(2);
threads::yield();

is($token++, 2, 'Main still has semaphore');
$st->up();

$sm->down();
is($token, 4, 'Main re-got semaphore');

$thread->join;

ok(1, 'Main done');

exit(0);

# EOF
