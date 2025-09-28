#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

# use strict;

plan tests => 39;

# simple use cases
{
    my %h = map { $_ => uc $_ } 'a'..'z';

    is( join(':', %h{'c','d','e'}), 'c:C:d:D:e:E', "correct result and order");
    is( join(':', %h{'e','d','c'}), 'e:E:d:D:c:C', "correct result and order");
    is( join(':', %h{'e','c','d'}), 'e:E:c:C:d:D', "correct result and order");

    ok( eq_hash( { %h{'q','w'} }, { q => 'Q', w => 'W' } ), "correct hash" );

    is( join(':', %h{()}), '', "correct result for empty slice");
}

# not existing elements
{
    my %h = map { $_ => uc $_ } 'a'..'d';
    ok( eq_hash( { %h{qw(e d)} }, { e => undef, d => 'D' } ),
        "not existing returned with undef value" );

    ok( !exists $h{e}, "no autovivification" );
}

# repeated keys
{
    my %h = map { $_ => uc $_ } 'a'..'d';
    my @a = %h{ ('c') x 3 };
    ok eq_array( \@a, [ ('c', 'C') x 3 ]), "repetead keys end with repeated results";
}

# scalar context
{
    my @warn;
    local $SIG{__WARN__} = sub {push @warn, "@_"};

    my %h = map { $_ => uc $_ } 'a'..'z';
    is scalar eval"%h{'c','d','e'}", 'E', 'last element in scalar context';

    like ($warn[0],
     qr/^\%h\{\.\.\.\} in scalar context better written as \$h\{\.\.\.\}/);

    eval 'is( scalar %h{i}, "I", "correct value");';

    is (scalar @warn, 2);
    like ($warn[1],
          qr/^\%h\{"i"\} in scalar context better written as \$h\{"i"\}/);
}

# autovivification
{
    my %h = map { $_ => uc $_ } 'a'..'b';

    my @a = %h{'c','d'};
    is( join(':', map {$_//'undef'} @a), 'c:undef:d:undef', "correct result");
    ok( eq_hash( \%h, { a => 'A', b => 'B' } ), "correct hash" );
}

# hash refs
{
    my $h = { map { $_ => uc $_ } 'a'..'z' };

    is( join(':', %$h{'c','d','e'}), 'c:C:d:D:e:E', "correct result and order");
    is( join(':', %{$h}{'c','d','e'}), 'c:C:d:D:e:E', "correct result and order");
}

# no interpolation
{
    my %h = map { $_ => uc $_ } 'a'..'b';
    is( "%h{'a','b'}", q{%h{'a','b'}}, 'no interpolation within strings' );
}

# ref of a slice produces list
{
    my %h = map { $_ => uc $_ } 'a'..'z';
    my @a = \%h{ qw'c d e' };

    my $ok = 1;
    $ok = 0 if grep !ref, @a;
    ok $ok, "all elements are refs";

    is join( ':', map{ $$_ } @a ), 'c:C:d:D:e:E'
}

# lvalue usage in foreach
{
    my %h = qw(a 1 b 2 c 3);
    $_++ foreach %h{'b', 'c'};
    ok( eq_hash( \%h, { a => 1, b => 3, c => 4 } ), "correct hash" );
}

# lvalue subs in foreach
{
    my %h = qw(a 1 b 2 c 3);
    sub foo:lvalue{ %h{qw(a b)} };
    $_++ foreach foo();
    ok( eq_hash( \%h, { a => 2, b => 3, c => 3 } ), "correct hash" );
}

# errors
{
    my %h = map { $_ => uc $_ } 'a'..'b';
    # no local
    {
        local $@;
        eval 'local %h{qw(a b)}';
        like $@, qr{^Can't modify key/value hash slice in local at},
            'local dies';
    }
    # no assign
    {
        local $@;
        eval '%h{qw(a b)} = qw(B A)';
        like $@, qr{^Can't modify key/value hash slice in list assignment},
            'assign dies';
    }
    # lvalue subs in assignment
    {
        local $@;
        eval 'sub bar:lvalue{ %h{qw(a b)} }; (bar) = "1"';
        like $@, qr{^Can't modify key/value hash slice in list assignment},
            'not allowed as result of lvalue sub';
        eval 'sub bbar:lvalue{ %h{qw(a b)} }; bbar() = "1"';
        like $@,
             qr{^Can't modify key/value hash slice in scalar assignment},
            'not allowed as result of lvalue sub';
    }
}

# warnings
{
    my @warn;
    local $SIG{__WARN__} = sub {push @warn, "@_"};

    my %h = map { $_ => uc $_ } 'a'..'c';
    {
        @warn = ();
        my $v = eval '%h{a}';
        is (scalar @warn, 1, 'warning in scalar context');
        like $warn[0],
             qr{^%h\{"a"\} in scalar context better written as \$h\{"a"\}},
            "correct warning text";
    }
    {
        @warn = ();
        my ($k,$v) = eval '%h{a}';
        is ($k, 'a');
        is ($v, 'A');
        is (scalar @warn, 0, 'no warning in list context');
    }

    {
        my $h = \%h;
        eval '%$h->{a}';
        like($@, qr/Can't use a hash as a reference/, 'hash reference is error' );

        eval '%$h->{"b","c"}';
        like($@, qr/Can't use a hash as a reference/, 'hash slice reference is error' );
    }
}

# simple case with tied
{
    require Tie::Hash;
    tie my %h, 'Tie::StdHash';
    %h = map { $_ => uc $_ } 'a'..'c';

    ok( eq_array( [%h{'b','a', 'e'}], [qw(b B a A e), undef] ),
        "works on tied" );

    ok( !exists $h{e}, "no autovivification" );
}

# keys/value/each refuse to compile kvhslice
{
    my %h = 'a'..'b';
    my %i = (foo => \%h);
    eval '() = keys %i{foo=>}';
    like($@, qr/Experimental keys on scalar is now forbidden/,
         'keys %hash{key} forbidden');
    eval '() = values %i{foo=>}';
    like($@, qr/Experimental values on scalar is now forbidden/,
         'values %hash{key} forbidden');
    eval '() = each %i{foo=>}';
    like($@, qr/Experimental each on scalar is now forbidden/,
         'each %hash{key} forbidden');
}

# \% prototype expects hash deref
sub nowt_but_hash(\%) {}
eval 'nowt_but_hash %INC{bar}';
like $@, qr`^Type of arg 1 to main::nowt_but_hash must be hash \(not(?x:
           ) key/value hash slice\) at `,
    '\% prototype';
