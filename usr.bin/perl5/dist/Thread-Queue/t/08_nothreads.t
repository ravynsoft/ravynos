use strict;
use warnings;

use Test::More 'tests' => 32;

use Thread::Queue;

# Regular array
my @ary1 = qw/foo bar baz/;
push(@ary1, [ 1..3 ], { 'qux' => 99 });

# Shared array
my @ary2 :shared = (99, 21, 86);

# Regular hash-based object
my $obj1 = {
    'foo' => 'bar',
    'qux' => 99,
    'biff' => [ qw/fee fi fo/ ],
    'boff' => { 'bork' => 'true' },
};
bless($obj1, 'Foo');

# Shared hash-based object
my $obj2 = &threads::shared::share({});
$$obj2{'bar'} = 86;
$$obj2{'key'} = 'foo';
bless($obj2, 'Bar');

# Scalar ref
my $sref1 = \do{ my $scalar = 'foo'; };

# Shared scalar ref object
my $sref2 = \do{ my $scalar = 69; };
threads::shared::share($sref2);
bless($sref2, 'Baz');

# Ref of ref
my $foo = [ 5, 'bork', { 'now' => 123 } ];
my $bar = \$foo;
my $baz = \$bar;
my $qux = \$baz;
is_deeply($$$$qux, $foo, 'Ref of ref');

# Queue up items
my $q = Thread::Queue->new(\@ary1, \@ary2);
ok($q, 'New queue');
is($q->pending(), 2, 'Queue count');
$q->enqueue($obj1, $obj2);
is($q->pending(), 4, 'Queue count');
$q->enqueue($sref1, $sref2, $qux);
is($q->pending(), 7, 'Queue count');

# Process items in queue
{
    is($q->pending(), 7, 'Queue count in thread');

    my $ref = $q->peek(3);
    is(ref($ref), 'Bar', 'Item is object');

    my $tary1 = $q->dequeue();
    ok($tary1, 'Thread got item');
    is(ref($tary1), 'ARRAY', 'Item is array ref');
    is_deeply($tary1, \@ary1, 'Complex array');

    my $tary2 = $q->dequeue();
    ok($tary2, 'Thread got item');
    is(ref($tary2), 'ARRAY', 'Item is array ref');
    for (my $ii=0; $ii < @ary2; $ii++) {
        is($$tary2[$ii], $ary2[$ii], 'Shared array element check');
    }

    my $tobj1 = $q->dequeue();
    ok($tobj1, 'Thread got item');
    is(ref($tobj1), 'Foo', 'Item is object');
    is_deeply($tobj1, $obj1, 'Object comparison');

    my $tobj2 = $q->dequeue();
    ok($tobj2, 'Thread got item');
    is(ref($tobj2), 'Bar', 'Item is object');
    is($$tobj2{'bar'}, 86, 'Shared object element check');
    is($$tobj2{'key'}, 'foo', 'Shared object element check');

    my $tsref1 = $q->dequeue();
    ok($tsref1, 'Thread got item');
    is(ref($tsref1), 'SCALAR', 'Item is scalar ref');
    is($$tsref1, 'foo', 'Scalar ref contents');

    my $tsref2 = $q->dequeue();
    ok($tsref2, 'Thread got item');
    is(ref($tsref2), 'Baz', 'Item is object');
    is($$tsref2, 69, 'Shared scalar ref contents');

    my $qux = $q->dequeue();
    is_deeply($$$$qux, $foo, 'Ref of ref');

    is($q->pending(), 0, 'Empty queue');
    my $nothing = $q->dequeue_nb();
    ok(! defined($nothing), 'Nothing on queue');
}

# Check results of thread's activities
is($q->pending(), 0, 'Empty queue');

exit(0);

# EOF
