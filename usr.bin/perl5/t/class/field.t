#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require Config;
}

use v5.36;
use feature 'class';
no warnings 'experimental::class';

# We can't test fields in isolation without having at least one method to
# use them from. We'll try to keep most of the heavy testing of method
# abilities to t/class/method.t

# field in method
{
    class Test1 {
        field $f;
        method incr { return ++$f; }
    }

    my $obj = Test1->new;
    $obj->incr;
    is($obj->incr, 2, 'Field $f incremented twice');

    my $obj2 = Test1->new;
    is($obj2->incr, 1, 'Fields are distinct between instances');
}

# fields are distinct
{
    class Test2 {
        field $x;
        field $y;

        method setpos { $x = $_[0]; $y = $_[1] }
        method x      { return $x; }
        method y      { return $y; }
    }

    my $obj = Test2->new;
    $obj->setpos(10, 20);
    is($obj->x, 10, '$pos->x');
    is($obj->y, 20, '$pos->y');
}

# fields of all variable types
{
    class Test3 {
        field $s;
        field @a;
        field %h;

        method setup {
            $s = "scalar";
            @a = ( "array" );
            %h = ( key => "hash" );
            return $self; # test chaining
        }
        method test {
            ::is($s,      "scalar", 'scalar storage');
            ::is($a[0],   "array",  'array storage');
            ::is($h{key}, "hash",   'hash storage');
        }
    }

    Test3->new->setup->test;
}

# fields can be captured by anon subs
{
    class Test4 {
        field $count;

        method make_incrsub {
            return sub { $count++ };
        }

        method count { return $count }
    }

    my $obj = Test4->new;
    my $incr = $obj->make_incrsub;

    $incr->();
    $incr->();
    $incr->();

    is($obj->count, 3, '$obj->count after invoking closure x 3');
}

# fields can be captured by anon methods
{
    class Test5 {
        field $count;

        method make_incrmeth {
            return method { $count++ };
        }

        method count { return $count }
    }

    my $obj = Test5->new;
    my $incr = $obj->make_incrmeth;

    $obj->$incr;
    $obj->$incr;
    $obj->$incr;

    is($obj->count, 3, '$obj->count after invoking method-closure x 3');
}

# fields of multiple unit classes are distinct
{
    class Test6::A;
    field $x = "A";
    method m { return "unit-$x" }

    class Test6::B;
    field $x = "B";
    method m { return "unit-$x" }

    package main;
    ok(eq_array([Test6::A->new->m, Test6::B->new->m], ["unit-A", "unit-B"]),
        'Fields of multiple unit classes remain distinct');
}

# fields can be initialised with constant expressions
{
    class Test7 {
        field $scalar = 123;
        method scalar { return $scalar; }

        field @array = (4, 5, 6);
        method array { return @array; }

        field %hash  = (7 => 89);
        method hash { return %hash; }
    }

    my $obj = Test7->new;

    is($obj->scalar, 123, 'Scalar field can be constant initialised');

    ok(eq_array([$obj->array], [4, 5, 6]), 'Array field can be constant initialised');

    ok(eq_hash({$obj->hash}, {7 => 89}), 'Hash field can be constant initialised');
}

# field initialiser expressions are evaluated within the constructor of each
# instance
{
    my $next_x = 1;
    my @items;
    my %mappings;

    class Test8 {
        field $x = $next_x++;
        method x { return $x; }

        field @y = ("more", @items);
        method y { return @y; }

        field %z = (first => "value", %mappings);
        method z { return %z; }
    }

    is($next_x, 1, '$next_x before any objects');

    @items = ("values");
    $mappings{second} = "here";

    my $obj1 = Test8->new;
    my $obj2 = Test8->new;

    is($obj1->x, 1, 'Object 1 has x == 1');
    is($obj2->x, 2, 'Object 2 has x == 2');

    is($next_x, 3, '$next_x after constructing two');

    ok(eq_array([$obj1->y], ["more", "values"]),
        'Object 1 has correct array field');
    ok(eq_hash({$obj1->z}, {first => "value", second => "here"}),
        'Object 1 has correct hash field');
}

# fields are visible during initialiser expressions of later fields
{
    class Test9 {
        field $one   = 1;
        field $two   = $one + 1;
        field $three = $two + 1;

        field @four = $one;
        field @five = (@four, $two, $three);
        field @six  = grep { $_ > 1 } @five;

        method three { return $three; }

        method six { return @six; }
    }

    my $obj = Test9->new;
    is($obj->three, 3, 'Scalar fields initialised from earlier fields');
    ok(eq_array([$obj->six], [2, 3]), 'Array fields initialised from earlier fields');
}

# fields can take :param attributes to consume constructor parameters
{
    my $next_gamma = 4;

    class Test10 {
        field $alpha :param        = undef;
        field $beta  :param        = 123;
        field $gamma :param(delta) = $next_gamma++;

        method values { return ($alpha, $beta, $gamma); }
    }

    my $obj = Test10->new(
        alpha => "A",
        beta  => "B",
        delta => "G",
    );
    ok(eq_array([$obj->values], [qw(A B G)]),
        'Field initialised by :params');
    is($next_gamma, 4, 'Defaulting expression not evaluated for passed value');

    $obj = Test10->new();
    ok(eq_array([$obj->values], [undef, 123, 4]),
        'Field initialised by defaulting expressions');
    is($next_gamma, 5, 'Defaulting expression evaluated for missing value');
}

# fields can be made non-optional
{
    class Test11 {
        field $x :param;
        field $y :param;
    }

    Test11->new(x => 1, y => 1);

    ok(!eval { Test11->new(x => 2) },
        'Constructor fails without y');
    like($@, qr/^Required parameter 'y' is missing for "Test11" constructor at /,
        'Failure from missing y argument');
}

# field assignment expressions on :param can use //= and ||=
{
    class Test12 {
        field $if_exists  :param(e)   = "DEF";
        field $if_defined :param(d) //= "DEF";
        field $if_true    :param(t) ||= "DEF";

        method values { return ($if_exists, $if_defined, $if_true); }
    }

    ok(eq_array(
        [Test12->new(e => "yes", d => "yes", t => "yes")->values],
        ["yes", "yes", "yes"]),
        'Values for "yes"');

    ok(eq_array(
        [Test12->new(e => 0, d => 0, t => 0)->values],
        [0, 0, "DEF"]),
        'Values for 0');

    ok(eq_array(
        [Test12->new(e => undef, d => undef, t => undef)->values],
        [undef, "DEF", "DEF"]),
        'Values for undef');

    ok(eq_array(
        [Test12->new()->values],
        ["DEF", "DEF", "DEF"]),
        'Values for missing');
}

# field initialiser expressions permit `goto` in do {} blocks
{
    class Test13 {
        field $forwards = do { goto HERE; HERE: 1 };
        field $backwards = do { my $x; HERE: ; goto HERE if !$x++; 2 };

        method values { return ($forwards, $backwards) }
    }

    ok(eq_array(
        [Test13->new->values],
        [1, 2],
        'Values for goto inside do {} blocks in field initialisers'));
}

done_testing;
