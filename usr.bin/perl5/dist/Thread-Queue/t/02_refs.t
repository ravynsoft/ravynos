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
use Thread::Queue;

BEGIN { # perl RT 133382
if ($] == 5.008) {
    require 't/test.pl';   # Test::More work-alike for Perl 5.8.0
} else {
    require Test::More;
}
Test::More->import();
} # end BEGIN
plan('tests' => 46);

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
my $obj2 = &share({});
$$obj2{'bar'} = 86;
$$obj2{'key'} = 'foo';
bless($obj2, 'Bar');

# Scalar ref
my $sref1 = \do{ my $scalar = 'foo'; };

# Shared scalar ref object
my $sref2 = \do{ my $scalar = 69; };
share($sref2);
bless($sref2, 'Baz');

# Ref of ref
my $foo = [ 5, 'bork', { 'now' => 123 } ];
my $bar = \$foo;
my $baz = \$bar;
my $qux = \$baz;
is_deeply($$$$qux, $foo, 'Ref of ref');

# Circular refs
my $cir1;
$cir1 = \$cir1;

my $cir1s : shared;
$cir1s = \$cir1s;

my $cir2;
$cir2 = [ \$cir2, { 'ref' => \$cir2 } ];

my $cir3 :shared = &share({});
$cir3->{'self'} = \$cir3;
bless($cir3, 'Circular');

# Queue up items
my $q = Thread::Queue->new(\@ary1, \@ary2);
ok($q, 'New queue');
is($q->pending(), 2, 'Queue count');
$q->enqueue($obj1, $obj2);
is($q->pending(), 4, 'Queue count');
$q->enqueue($sref1, $sref2, $foo, $qux);
is($q->pending(), 8, 'Queue count');
$q->enqueue($cir1, $cir1s, $cir2, $cir3);
is($q->pending(), 12, 'Queue count');

# Process items in thread
threads->create(sub {
    is($q->pending(), 12, 'Queue count in thread');

    my $tary1 = $q->dequeue();
    ok($tary1, 'Thread got item');
    is(ref($tary1), 'ARRAY', 'Item is array ref');
    is_deeply($tary1, \@ary1, 'Complex array');
    $$tary1[1] = 123;

    my $tary2 = $q->dequeue();
    ok($tary2, 'Thread got item');
    is(ref($tary2), 'ARRAY', 'Item is array ref');
    for (my $ii=0; $ii < @ary2; $ii++) {
        is($$tary2[$ii], $ary2[$ii], 'Shared array element check');
    }
    $$tary2[1] = 444;

    my $tobj1 = $q->dequeue();
    ok($tobj1, 'Thread got item');
    is(ref($tobj1), 'Foo', 'Item is object');
    is_deeply($tobj1, $obj1, 'Object comparison');
    $$tobj1{'foo'} = '.|.';
    $$tobj1{'smiley'} = ':)';

    my $tobj2 = $q->dequeue();
    ok($tobj2, 'Thread got item');
    is(ref($tobj2), 'Bar', 'Item is object');
    is($$tobj2{'bar'}, 86, 'Shared object element check');
    is($$tobj2{'key'}, 'foo', 'Shared object element check');
    $$tobj2{'tick'} = 'tock';
    $$tobj2{'frowny'} = ':(';

    my $tsref1 = $q->dequeue();
    ok($tsref1, 'Thread got item');
    is(ref($tsref1), 'SCALAR', 'Item is scalar ref');
    is($$tsref1, 'foo', 'Scalar ref contents');
    $$tsref1 = 0;

    my $tsref2 = $q->dequeue();
    ok($tsref2, 'Thread got item');
    is(ref($tsref2), 'Baz', 'Item is object');
    is($$tsref2, 69, 'Shared scalar ref contents');
    $$tsref2 = 'zzz';

    my $myfoo = $q->dequeue();
    is_deeply($myfoo, $foo, 'Array ref');

    my $qux = $q->dequeue();
    is_deeply($$$$qux, $foo, 'Ref of ref');

    my ($c1, $c1s, $c2, $c3) = $q->dequeue(4);
    SKIP: {
        skip("Needs threads::shared >= 1.19", 5)
            if ($threads::shared::VERSION < 1.19);

        is(threads::shared::_id($$c1),
           threads::shared::_id($c1),
                'Circular ref - scalar');

        is(threads::shared::_id($$c1s),
           threads::shared::_id($c1s),
                'Circular ref - shared scalar');

        is(threads::shared::_id(${$c2->[0]}),
           threads::shared::_id($c2),
                'Circular ref - array');

        is(threads::shared::_id(${$c2->[1]->{'ref'}}),
           threads::shared::_id($c2),
                'Circular ref - mixed');

        is(threads::shared::_id(${$c3->{'self'}}),
           threads::shared::_id($c3),
                'Circular ref - hash');
    }

    is($q->pending(), 0, 'Empty queue');
    my $nothing = $q->dequeue_nb();
    ok(! defined($nothing), 'Nothing on queue');
})->join();

# Check results of thread's activities
is($q->pending(), 0, 'Empty queue');

is($ary1[1], 'bar', 'Array unchanged');
is($ary2[1], 444, 'Shared array changed');

is($$obj1{'foo'}, 'bar', 'Object unchanged');
ok(! exists($$obj1{'smiley'}), 'Object unchanged');

is($$obj2{'tick'}, 'tock', 'Shared object changed');
is($$obj2{'frowny'}, ':(', 'Shared object changed');

is($$sref1, 'foo', 'Scalar ref unchanged');
is($$sref2, 'zzz', 'Shared scalar ref changed');

exit(0);

# EOF
