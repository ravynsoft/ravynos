use strict;
use warnings;

use Test::More 'tests' => 7;

use Thread::Semaphore;

my $s = Thread::Semaphore->new();
is($$s, 1, 'Non-threaded semaphore');
$s->down();
is($$s, 0, 'Non-threaded semaphore');
$s->up(2);
is($$s, 2, 'Non-threaded semaphore');
$s->down();
is($$s, 1, 'Non-threaded semaphore');
ok(! $s->down_nb(2), 'Non-threaded semaphore');
ok($s->down_nb(), 'Non-threaded semaphore');
ok(! $s->down_timed(1), 'Non-threaded semaphore');

exit(0);

# EOF
