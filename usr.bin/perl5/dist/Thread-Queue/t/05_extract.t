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
plan('tests' => 20);

my $q = Thread::Queue->new(1..10);
ok($q, 'New queue');

threads->create(sub {
    # Default count = 1
    is($q->extract(),   1, 'No args');          # 2..10 left
    is($q->extract(0),  2, 'Head');             # 3..10 left
    is($q->extract(5),  8, 'Pos index');        # 3..7,9,10 left
    is($q->extract(-3), 7, 'Neg index');        # 3..6,9,10 left
    my $x = $q->extract(20);                    # unchanged
    ok(! defined($x), 'Big index');
    $x = $q->extract(-20);                      # unchanged
    ok(! defined($x), 'Big neg index');
})->join();

$q = Thread::Queue->new(1..10);
ok($q, 'New queue');

threads->create(sub {
    my @x = $q->extract(0, 2);                  # 3..10 left
    is_deeply(\@x, [1,2], '2 from head');
    @x = $q->extract(6, 2);                     # 3..8 left
    is_deeply(\@x, [9,10], '2 from tail');
    @x = $q->extract(2, 2);                     # 3,4,7,8 left
    is_deeply(\@x, [5,6], '2 from middle');
    @x = $q->extract(2, 4);                     # 3,4 left
    is_deeply(\@x, [7,8], 'Lots from tail');
    @x = $q->extract(3, 4);                     # unchanged
    is_deeply(\@x, [], 'Too far');
})->join();

$q = Thread::Queue->new(1..10);
ok($q, 'New queue');

threads->create(sub {
    my @x = $q->extract(-4, 2);                 # 1..6,9,10 left
    is_deeply(\@x, [7,8], 'Neg index');
    @x = $q->extract(-2, 4);                    # 1..6 left
    is_deeply(\@x, [9,10], 'Lots from tail');
    @x = $q->extract(-6, 2);                    # 3..6 left
    is_deeply(\@x, [1,2], 'Max neg index');
    @x = $q->extract(-10, 3);                   # unchanged
    is_deeply(\@x, [], 'Too far');
    @x = $q->extract(-6, 3);                    # 4..6 left
    is_deeply(\@x, [3], 'Neg overlap');
    @x = $q->extract(-5, 10);                   # empty
    is_deeply(\@x, [4..6], 'Neg big overlap');
})->join();

exit(0);

# EOF
