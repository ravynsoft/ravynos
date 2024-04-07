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

BEGIN { # perl RT 133382
if ($] == 5.008) {
    require 't/test.pl';   # Test::More work-alike for Perl 5.8.0
} else {
    require Test::More;
}
Test::More->import();
} # end BEGIN
plan('tests' => 19);

### ->dequeue_timed(TIMEOUT, COUNT) test ###

my $q = Thread::Queue->new();
ok($q, 'New queue');

my @items = qw/foo bar baz qux exit/;
$q->enqueue(@items);
is($q->pending(), scalar(@items), 'Queue count');

threads->create(sub {
    is($q->pending(), scalar(@items), 'Queue count in thread');
    while (my @el = $q->dequeue_timed(2.5, 2)) {
        is($el[0], shift(@items), "Thread got $el[0]");
        if ($el[0] eq 'exit') {
            is(scalar(@el), 1, 'Thread to exit');
        } else {
            is($el[1], shift(@items), "Thread got $el[1]");
        }
    }
    is($q->pending(), 0, 'Empty queue');
    $q->enqueue('done');
})->join();

is($q->pending(), 1, 'Queue count after thread');
is($q->dequeue(), 'done', 'Thread reported done');
is($q->pending(), 0, 'Empty queue');

### ->dequeue_timed(TIMEOUT) test on empty queue ###

threads->create(sub {
    is($q->pending(), 0, 'Empty queue in thread');
    my @el = $q->dequeue_timed(1.5);
    is($el[0], undef, "Thread got no items");
    is($q->pending(), 0, 'Empty queue in thread');
    $q->enqueue('done');
})->join();

is($q->pending(), 1, 'Queue count after thread');
is($q->dequeue(), 'done', 'Thread reported done');
is($q->pending(), 0, 'Empty queue');

exit(0);

# EOF
