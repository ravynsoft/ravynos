#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use strict;

my (@ary, %ary, %hash);

plan 88;

ok !defined($a);

$a = 1+1;
ok defined($a);

undef $a;
ok !defined($a);

$a = "hi";
ok defined($a);

$a = $b;
ok !defined($a);

@ary = ("1arg");
$a = pop(@ary);
ok defined($a);
$a = pop(@ary);
ok !defined($a);

@ary = ("1arg");
$a = shift(@ary);
ok defined($a);
$a = shift(@ary);
ok !defined($a);

$ary{'foo'} = 'hi';
ok defined($ary{'foo'});
ok !defined($ary{'bar'});
undef $ary{'foo'};
ok !defined($ary{'foo'});

sub foo { pass; 1 }

&foo || fail;

ok defined &foo;
undef &foo;
ok !defined(&foo);

eval { undef $1 };
like $@, qr/^Modification of a read/;

eval { $1 = undef };
like $@, qr/^Modification of a read/;

{
    # [perl #17753] segfault when undef'ing unquoted string constant
    eval 'undef tcp';
    like $@, qr/^Can't modify constant item/;
}

# bugid 3096
# undefing a hash may free objects with destructors that then try to
# modify the hash. Ensure that the hash remains consistent

{
    my (%hash, %mirror);

    my $iters = 5;

    for (1..$iters) {
	$hash{"k$_"} = bless ["k$_"], 'X';
	$mirror{"k$_"} = "k$_";
    }


    my $c = $iters;
    my $events;

    sub X::DESTROY {
	my $key = $_[0][0];
	$events .= 'D';
	note("----- DELETE($key) ------");
	delete $mirror{$key};

	is join('-', sort keys %hash), join('-', sort keys %mirror),
	    "$key: keys";
	is join('-', sort map $_->[0], values %hash),
	    join('-', sort values %mirror), "$key: values";

	# don't know exactly what we'll get from the iterator, but
	# it must be a sensible value
	my ($k, $v) = each %hash;
	ok defined $k ? exists($mirror{$k}) : (keys(%mirror) == 0),
	    "$key: each 1";

	is delete $hash{$key}, undef, "$key: delete";
	($k, $v) = each %hash;
	ok defined $k ? exists($mirror{$k}) : (keys(%mirror) <= 1),
	    "$key: each 2";

	$c++;
	if ($c <= $iters * 2) {
	    $hash{"k$c"} = bless ["k$c"], 'X';
	    $mirror{"k$c"} = "k$c";
	}
	$events .= 'E';
    }

    each %hash; # set eiter
    undef %hash;

    is scalar keys %hash, 0, "hash empty at end";
    is $events, ('DE' x ($iters*2)), "events";
    my ($k, $v) = each %hash;
    is $k, undef, 'each undef at end';
}

# part of #105906: inlined undef constant getting copied
BEGIN { $::{z} = \undef }
for (z,z) {
    push @_, \$_;
}
is $_[0], $_[1], 'undef constants preserve identity';

# [perl #122556]
my $messages;
package Thingie;
DESTROY { $messages .= 'destroyed ' }
package main;
sub body {
    sub {
        my $t = bless [], 'Thingie';
        undef $t;
    }->(), $messages .= 'after ';

    return;
}
body();
is $messages, 'destroyed after ', 'undef $scalar frees refs immediately';


# this will segfault if it fails

sub PVBM () { 'foo' }
{ my $dummy = index 'foo', PVBM }

my $pvbm = PVBM;
undef $pvbm;
ok !defined $pvbm;

# Prior to GH#20077 (Add OPpTARGET_MY optimization to OP_UNDEF), any PV
# allocation was kept with "$x = undef" but freed with "undef $x". That
# behaviour was carried over and is expected to still be present.
# (I totally copied most of this block from other t/op/* files.)

SKIP: {
    skip_without_dynamic_extension("Devel::Peek", 2);

    my $out = runperl(stderr => 1,
                  progs => [ split /\n/, <<'EOS' ]);
    require Devel::Peek;
    my $f = q(x) x 40; $f = undef;
    Devel::Peek::Dump($f);
    undef $f;
    Devel::Peek::Dump($f);
EOS

    my ($space, $first, $second) = split /SV =/, $out;
    like($first, qr/\bPV = 0x[0-9a-f]+\b/, '$x = undef preserves PV allocation');
    like($second, qr/\bPV = 0\b$/, 'undef $x frees PV allocation');
}

# Tests suggested for GH#20077 (Add OPpTARGET_MY optimization to OP_UNDEF)
# (No failures were observed during development, these are just checking
# that no failures are introduced down the line.)

{
    my $y= 1; my @x= ($y= undef);
    is( defined($x[0]), "", 'lval undef assignment in list context');
    is( defined($y)  , "", 'scalar undef assignment in list context');

    $y= 1; my $z; sub f{$z = shift} f($y=undef);
    is( defined($y)  , "", 'undef assignment in sub args');
    is( defined($z)  , "", 'undef assignment reaches @_');

    ($y,$z)=(1,2); sub f{} f(($y=undef),$z);
    is( defined($y)  , "", 'undef assignment reaches @_');
    is( $z, 2, 'undef adjacent argument is unchanged');
}

{
    my $h= { baz => 1 }; my @k= keys %{($h=undef)||{}};
    is( defined($h)  , "", 'scalar undef assignment in keys');
    is( scalar @k, 0, 'undef assignment dor anonhash');

    my $y= 1; my @x= \($y= undef);
    is( defined($y)  , "", 'scalar undef assignment before reference');
    is( scalar @x, 1, 'assignment of one element to array');
    is( defined($x[0]->$*), "", 'assignment of undef element to array');
}

# GH#20336 - "my $x = undef" pushed &PL_sv_undef onto the stack, but
#            should be pushing $x (i.e. a mutable copy of &PL_sv_undef)
is( ++(my $x = undef), 1, '"my $x = undef" pushes $x onto the stack' );
