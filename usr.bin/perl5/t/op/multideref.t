#!./perl
#
# test OP_MULTIDEREF.
#
# This optimising op is used when one or more array or hash aggregate
# lookups / derefs are performed, and where each key/index is a simple
# constant or scalar var; e.g.
#
#       $r->{foo}[0]{$k}[$i]


BEGIN {
    chdir 't';
    require './test.pl';
    set_up_inc("../lib");
}

use warnings;
use strict;

plan 65;


# check that strict refs hint is handled

{
    package strict_refs;

    our %foo;
    my @a = ('foo');
    eval {
        $a[0]{k} = 7;
    };
    ::like($@, qr/Can't use string/, "strict refs");
    ::ok(!exists $foo{k}, "strict refs, not exist");

    no strict 'refs';

    $a[0]{k} = 13;
    ::is($foo{k}, 13, "no strict refs, exist");
}

# check the basics of multilevel lookups

{
    package basic;

    # build up the multi-level structure piecemeal to try and avoid
    # relying on what we're testing

    my @a;
    my $r = \@a;
    my $rh = {};
    my $ra = [];
    my %h = qw(a 1 b 2 c 3 d 4 e 5 f 6);
    push @a, 66, 77, 'abc', $rh;
    %$rh = (foo => $ra, bar => 'BAR');
    push @$ra, 'def', \%h;

    our ($i1, $i2,  $k1,  $k2)  = (3, 1, 'foo', 'c');
    my ($li1, $li2, $lk1, $lk2) = (3, 1, 'foo', 'c');
    my $z = 0;

    # fetch

    ::is($a[3]{foo}[1]{c}, 3,             'fetch: const indices');
    ::is($a[$i1]{$k1}[$i2]{$k2}, 3,       'fetch: pkg indices');
    ::is($r->[$i1]{$k1}[$i2]{$k2}, 3,     'fetch: deref pkg indices');
    ::is($a[$li1]{$lk1}[$li2]{$lk2}, 3,   'fetch: lexical indices');
    ::is($r->[$li1]{$lk1}[$li2]{$lk2}, 3, 'fetch: deref lexical indices');
    ::is(($r//0)->[$li1]{$lk1}[$li2+$z]{$lk2}, 3,
                            'fetch: general expression and index');


    # store

    ::is($a[3]{foo}[1]{c} = 5, 5,             'store: const indices');
    ::is($a[3]{foo}[1]{c}, 5,                 'store: const indices 2');
    ::is($a[$i1]{$k1}[$i2]{$k2} = 7, 7,       'store: pkg indices');
    ::is($a[$i1]{$k1}[$i2]{$k2}, 7,           'store: pkg indices 2');
    ::is($r->[$i1]{$k1}[$i2]{$k2} = 9, 9,     'store: deref pkg indices');
    ::is($r->[$i1]{$k1}[$i2]{$k2}, 9,         'store: deref pkg indices 2');
    ::is($a[$li1]{$lk1}[$li2]{$lk2} = 11, 11, 'store: lexical indices');
    ::is($a[$li1]{$lk1}[$li2]{$lk2}, 11,      'store: lexical indices 2');
    ::is($r->[$li1]{$lk1}[$li2]{$lk2} = 13, 13, 'store: deref lexical indices');
    ::is($r->[$li1]{$lk1}[$li2]{$lk2}, 13,    'store: deref lexical indices 2');
    ::is(($r//0)->[$li1]{$lk1}[$li2+$z]{$lk2} = 15, 15,
                            'store: general expression and index');
    ::is(($r//0)->[$li1]{$lk1}[$li2+$z]{$lk2}, 15,
                            'store: general expression and index 2');


    # local

    {
        ::is(local $a[3]{foo}[1]{c} = 19, 19,     'local const indices');
        ::is($a[3]{foo}[1]{c}, 19,                'local const indices 2');
    }
    ::is($a[3]{foo}[1]{c}, 15,          'local const indices 3');
    {
        ::is(local $a[$i1]{$k1}[$i2]{$k2} = 21, 21,     'local pkg indices');
        ::is($a[$i1]{$k1}[$i2]{$k2}, 21,          'local pkg indices 2');
    }
    ::is($a[$i1]{$k1}[$i2]{$k2}, 15,     'local pkg indices 3');
    {
        ::is(local $a[$li1]{$lk1}[$li2]{$lk2} = 23, 23, 'local lexical indices');
        ::is($a[$li1]{$lk1}[$li2]{$lk2}, 23,      'local lexical indices 2');
    }
    ::is($a[$li1]{$lk1}[$li2]{$lk2}, 15, 'local lexical indices 3');
    {
        ::is(local+($r//0)->[$li1]{$lk1}[$li2+$z]{$lk2} = 25, 25,
                                                            'local general');
        ::is(($r//0)->[$li1]{$lk1}[$li2+$z]{$lk2}, 25,      'local general 2');
    }
    ::is(($r//0)->[$li1]{$lk1}[$li2+$z]{$lk2}, 15, 'local general 3');


    # exists

    ::ok(exists $a[3]{foo}[1]{c},           'exists: const indices');
    ::ok(exists $a[$i1]{$k1}[$i2]{$k2},     'exists: pkg indices');
    ::ok(exists $r->[$i1]{$k1}[$i2]{$k2},   'exists: deref pkg indices');
    ::ok(exists $a[$li1]{$lk1}[$li2]{$lk2}, 'exists: lexical indices');
    ::ok(exists $r->[$li1]{$lk1}[$li2]{$lk2}, 'exists: deref lexical indices');
    ::ok(exists +($r//0)->[$li1]{$lk1}[$li2+$z]{$lk2}, 'exists: general');

    # delete

    our $k3 = 'a';
    my $lk4 = 'b';
    ::is(delete $a[3]{foo}[1]{c}, 15,          'delete: const indices');
    ::is(delete $a[$i1]{$k1}[$i2]{$k3}, 1,     'delete: pkg indices');
    ::is(delete $r->[$i1]{$k1}[$i2]{d}, 4,     'delete: deref pkg indices');
    ::is(delete $a[$li1]{$lk1}[$li2]{$lk4}, 2, 'delete: lexical indices');
    ::is(delete $r->[$li1]{$lk1}[$li2]{e}, 5,  'delete: deref lexical indices');
    ::is(delete +($r//0)->[$li1]{$lk1}[$li2+$z]{f}, 6,  'delete: general');

    # !exists

    ::ok(!exists $a[3]{foo}[1]{c},            '!exists: const indices');
    ::ok(!exists $a[$i1]{$k1}[$i2]{$k3},      '!exists: pkg indices');
    ::ok(!exists $r->[$i1]{$k1}[$i2]{$k3},    '!exists: deref pkg indices');
    ::ok(!exists $a[$li1]{$lk1}[$li2]{$lk4},  '!exists: lexical indices');
    ::ok(!exists $r->[$li1]{$lk1}[$li2]{$lk4},'!exists: deref lexical indices');
    ::ok(!exists +($r//0)->[$li1]{$lk1}[$li2+$z]{$lk4},'!exists: general');
}


# weird "constant" keys

{
    use constant my_undef => undef;
    use constant my_ref   => [];
    no warnings 'uninitialized';
    my %h1;
    $h1{+my_undef} = 1;
    is(join(':', keys %h1), '', "+my_undef");
    my %h2;
    $h2{+my_ref} = 1;
    like(join(':', keys %h2), qr/x/, "+my_ref");
}



{
    # test that multideref is marked OA_DANGEROUS, i.e. its one of the ops
    # that should set the OPpASSIGN_COMMON flag in list assignments

    my $x = {};
    $x->{a} = [ 1 ];
    $x->{b} = [ 2 ];
    ($x->{a}, $x->{b}) = ($x->{b}, $x->{a});
    is($x->{a}[0], 2, "OA_DANGEROUS a");
    is($x->{b}[0], 1, "OA_DANGEROUS b");
}

# defer


sub defer {}

{
    my %h;
    $h{foo} = {};
    defer($h{foo}{bar});
    ok(!exists $h{foo}{bar}, "defer");
}

# RT #123609
# don't evaluate a const array index unless it's really a const array
# index

{
    my $warn = '';
    local $SIG{__WARN__} = sub { $warn .= $_[0] };
    ok(
        eval q{
            my @a = (1);
            my $arg = 0;
            my $x = $a[ 'foo' eq $arg ? 1 : 0 ];
            1;
        },
        "#123609: eval"
    )
        or diag("eval gave: $@");
    is($warn, "", "#123609: warn");
}

# RT #130727
# a [ah]elem op can be both OPpLVAL_INTRO and OPpDEREF. It may not make
# much sense, but it shouldn't fail an assert.

{
    my @x;
    eval { @{local $x[0][0]} = 1; };
    like $@, qr/Can't use an undefined value as an ARRAY reference/,
                    "RT #130727 error";
    ok !defined $x[0][0],"RT #130727 array not autovivified";

    eval { @{1, local $x[0][0]} = 1; };
    like $@, qr/Can't use an undefined value as an ARRAY reference/,
                    "RT #130727 part 2: error";
    ok !defined $x[0][0],"RT #130727 part 2: array not autovivified";

}

# RT #131627: assertion failure on OPf_PAREN on OP_GV
{
    my @x = (10..12);
    our $rt131627 = 1;

    no strict qw(refs vars);
    is $x[qw(rt131627)->$*], 11, 'RT #131627: $a[qw(var)->$*]';
}

# this used to leak - run the code for ASan to spot any problems
{
    package Foo;
    our %FIELDS = ();
    my Foo $f;
    eval q{ my $x = $f->{c}; };
    ::pass("S_maybe_multideref() shouldn't leak on croak");
}

fresh_perl_is('0for%{scalar local$0[0]}', '', {},
              "RT #134045 assertion on the OP_SCALAR");
