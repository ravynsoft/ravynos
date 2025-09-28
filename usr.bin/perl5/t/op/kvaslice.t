#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

# use strict;

plan tests => 38;

# simple use cases
{
    my @a = 'a'..'z';

    is( join(':', %a[0,1,2]), '0:a:1:b:2:c', "correct result and order");
    is( join(':', %a[2,1,0]), '2:c:1:b:0:a', "correct result and order");
    is( join(':', %a[1,0,2]), '1:b:0:a:2:c', "correct result and order");

    ok( eq_hash( { %a[5,6] }, { 5 => 'f', 6 => 'g' } ), "correct hash" );

    is( join(':', %a[()]), '', "correct result for empty slice");
}

# not existing elements
{
    my @a = 'a'..'d';
    ok( eq_hash( { %a[3..4] }, { 3 => 'd', 4 => undef } ),
        "not existing returned with undef value" );

    ok( !exists $a[5], "no autovivification" );
}

# repeated keys
{
    my @a = 'a'..'d';
    @a = %a[ (1) x 3 ];
    ok eq_array( \@a, [ (1 => 'b') x 3 ]), "repetead keys end with repeated results";
}

# scalar context
{
    my @warn;
    local $SIG{__WARN__} = sub {push @warn, "@_"};

    my @a = 'a'..'z';
    is eval'scalar %a[4,5,6]', 'g', 'last element in scalar context';

    like ($warn[0],
     qr/^\%a\[\.\.\.\] in scalar context better written as \$a\[\.\.\.\]/);

    eval 'is( scalar %a[5], "f", "correct value");';

    is (scalar @warn, 2);
    like ($warn[1], qr/^\%a\[5\] in scalar context better written as \$a\[5\]/);
}

# autovivification
{
    my @a = 'a'..'b';

    my @t = %a[1,2];
    is( join(':', map {$_//'undef'} @t), '1:b:2:undef', "correct result");
    ok( eq_array( \@a, ['a', 'b'] ), "correct array" );
}

# refs
{
    my $a = [ 'a'..'z' ];

    is( join(':', %$a[2,3,4]), '2:c:3:d:4:e', "correct result and order");
    is( join(':', %{$a}[2,3,4]), '2:c:3:d:4:e', "correct result and order");
}

# no interpolation
{
    my @a = 'a'..'b';
    is( "%a[1,2]", q{%a[1,2]}, 'no interpolation within strings' );
}

# ref of a slice produces list
{
    my @a = 'a'..'z';
    my @tmp = \%a[2,3,4];

    my $ok = 1;
    $ok = 0 if grep !ref, @tmp;
    ok $ok, "all elements are refs";

    is join( ':', map{ $$_ } @tmp ), '2:c:3:d:4:e';
}

# lvalue usage in foreach
{
    my @a = qw(0 1 2 3);
    my @i = (1,3);
    $_++ foreach %a[@i];
    ok( eq_array( \@a, [0,2,2,4] ), "correct array" );
    ok( eq_array( \@i, [1,3] ), "indexes not touched" );
}

# lvalue subs in foreach
{
    my @a = qw(0 1 2 3);
    my @i = (1,3);
    sub foo:lvalue{ %a[@i] };
    $_++ foreach foo();
    ok( eq_array( \@a, [0,2,2,4] ), "correct array" );
    ok( eq_array( \@i, [1,3] ), "indexes not touched" );
}

# errors
{
    my @a = 'a'..'b';
    # no local
    {
        local $@;
        eval 'local %a[1,2]';
        like $@, qr{^Can't modify index/value array slice in local at},
            'local dies';
    }
    # no assign
    {
        local $@;
        eval '%a[1,2] = qw(B A)';
        like $@, qr{^Can't modify index/value array slice in list assignment},
            'assign dies';
    }
    # lvalue subs in assignment
    {
        local $@;
        eval 'sub bar:lvalue{ %a[1,2] }; bar() = "1"';
        like $@, qr{^Can't modify index/value array slice in list assignment},
            'not allowed as result of lvalue sub';
    }
}

# warnings
{
    my @warn;
    local $SIG{__WARN__} = sub {push @warn, "@_"};

    my @a = 'a'..'c';
    {
        @warn = ();
        my $v = eval '%a[0]';
        is (scalar @warn, 1, 'warning in scalar context');
        like $warn[0],
             qr{^%a\[0\] in scalar context better written as \$a\[0\]},
            "correct warning text";
    }
    {
        @warn = ();
        my ($k,$v) = eval '%a[0]';
        is ($k, 0);
        is ($v, 'a');
        is (scalar @warn, 0, 'no warning in list context');
    }
}

# simple case with tied
{
    require Tie::Array;
    tie my @a, 'Tie::StdArray';
    @a = 'a'..'c';

    ok( eq_array( [%a[1,2, 3]], [qw(1 b 2 c 3), undef] ),
        "works on tied" );

    ok( !exists $a[3], "no autovivification" );
}

# keys/value/each refuse to compile kvaslice
{
    my %h = 'a'..'b';
    my @i = \%h;
    eval '() = keys %i[(0)]';
    like($@, qr/Experimental keys on scalar is now forbidden/,
         'keys %array[ix] forbidden');
    eval '() = values %i[(0)]';
    like($@, qr/Experimental values on scalar is now forbidden/,
         'values %array[ix] forbidden');
    eval '() = each %i[(0)]';
    like($@, qr/Experimental each on scalar is now forbidden/,
         'each %array[ix] forbidden');
}

# \% prototype expects hash deref
sub nowt_but_hash(\%) {}
eval 'nowt_but_hash %_[0]';
like $@, qr`^Type of arg 1 to main::nowt_but_hash must be hash \(not(?x:
           ) index/value array slice\) at `,
    '\% prototype';
