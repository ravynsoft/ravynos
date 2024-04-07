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
plan('tests' => 16);

my $q = Thread::Queue->new(1..10);
ok($q, 'New queue');

threads->create(sub {
    $q->insert(5);
    $q->insert(-5);
    $q->insert(100);
    $q->insert(-100);
})->join();

my @x = $q->dequeue_nb(100);
is_deeply(\@x, [1..10], 'No-op inserts');


$q = Thread::Queue->new(1..10);
ok($q, 'New queue');

threads->create(sub {
    $q->insert(10, qw/tail/);
    $q->insert(0, qw/head/);
})->join();

@x = $q->dequeue_nb(100);
is_deeply(\@x, ['head',1..10,'tail'], 'Edge inserts');


$q = Thread::Queue->new(1..10);
ok($q, 'New queue');

threads->create(sub {
    $q->insert(5, qw/foo bar/);
    $q->insert(-2, qw/qux/);
})->join();

@x = $q->dequeue_nb(100);
is_deeply(\@x, [1..5,'foo','bar',6..8,'qux',9,10], 'Middle inserts');


$q = Thread::Queue->new(1..10);
ok($q, 'New queue');

threads->create(sub {
    $q->insert(20, qw/tail/);
    $q->insert(-20, qw/head/);
})->join();

@x = $q->dequeue_nb(100);
is_deeply(\@x, ['head',1..10,'tail'], 'Extreme inserts');


$q = Thread::Queue->new();
ok($q, 'New queue');
threads->create(sub { $q->insert(0, 1..3); })->join();
@x = $q->dequeue_nb(100);
is_deeply(\@x, [1..3], 'Empty queue insert');

$q = Thread::Queue->new();
ok($q, 'New queue');
threads->create(sub { $q->insert(20, 1..3); })->join();
@x = $q->dequeue_nb(100);
is_deeply(\@x, [1..3], 'Empty queue insert');

$q = Thread::Queue->new();
ok($q, 'New queue');
threads->create(sub { $q->insert(-1, 1..3); })->join();
@x = $q->dequeue_nb(100);
is_deeply(\@x, [1..3], 'Empty queue insert');

$q = Thread::Queue->new();
ok($q, 'New queue');
threads->create(sub {
    $q->insert(2, 1..3);
    $q->insert(1, 'foo');
})->join();
@x = $q->dequeue_nb(100);
is_deeply(\@x, [1,'foo',2,3], 'Empty queue insert');

exit(0);

# EOF
