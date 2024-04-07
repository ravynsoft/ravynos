#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
}

use strict qw(subs refs);
use warnings;

# A lot of tests to check that reversed for works.

@array = ('A', 'B', 'C');
for (@array) {
    $r .= $_;
}
is ($r, 'ABC', 'Forwards for array');
$r = '';
for (1,2,3) {
    $r .= $_;
}
is ($r, '123', 'Forwards for list');
$r = '';
for (map {$_} @array) {
    $r .= $_;
}
is ($r, 'ABC', 'Forwards for array via map');
$r = '';
for (map {$_} 1,2,3) {
    $r .= $_;
}
is ($r, '123', 'Forwards for list via map');
$r = '';
for (1 .. 3) {
    $r .= $_;
}
is ($r, '123', 'Forwards for list via ..');
$r = '';
for ('A' .. 'C') {
    $r .= $_;
}
is ($r, 'ABC', 'Forwards for list via ..');

$r = '';
for (reverse @array) {
    $r .= $_;
}
is ($r, 'CBA', 'Reverse for array');
$r = '';
for (reverse 1,2,3) {
    $r .= $_;
}
is ($r, '321', 'Reverse for list');
$r = '';
for (reverse map {$_} @array) {
    $r .= $_;
}
is ($r, 'CBA', 'Reverse for array via map');
$r = '';
for (reverse map {$_} 1,2,3) {
    $r .= $_;
}
is ($r, '321', 'Reverse for list via map');
$r = '';
for (reverse 1 .. 3) {
    $r .= $_;
}
is ($r, '321', 'Reverse for list via ..');
$r = '';
for (reverse 'A' .. 'C') {
    $r .= $_;
}
is ($r, 'CBA', 'Reverse for list via ..');

$r = '';
for my $i (@array) {
    $r .= $i;
}
is ($r, 'ABC', 'Forwards for array with var');
$r = '';
for my $i (1,2,3) {
    $r .= $i;
}
is ($r, '123', 'Forwards for list with var');
$r = '';
for my $i (map {$_} @array) {
    $r .= $i;
}
is ($r, 'ABC', 'Forwards for array via map with var');
$r = '';
for my $i (map {$_} 1,2,3) {
    $r .= $i;
}
is ($r, '123', 'Forwards for list via map with var');
$r = '';
for my $i (1 .. 3) {
    $r .= $i;
}
is ($r, '123', 'Forwards for list via .. with var');
$r = '';
for my $i ('A' .. 'C') {
    $r .= $i;
}
is ($r, 'ABC', 'Forwards for list via .. with var');

$r = '';
for my $i (reverse @array) {
    $r .= $i;
}
is ($r, 'CBA', 'Reverse for array with var');
$r = '';
for my $i (reverse 1,2,3) {
    $r .= $i;
}
is ($r, '321', 'Reverse for list with var');
$r = '';
for my $i (reverse map {$_} @array) {
    $r .= $i;
}
is ($r, 'CBA', 'Reverse for array via map with var');
$r = '';
for my $i (reverse map {$_} 1,2,3) {
    $r .= $i;
}
is ($r, '321', 'Reverse for list via map with var');
$r = '';
for my $i (reverse 1 .. 3) {
    $r .= $i;
}
is ($r, '321', 'Reverse for list via .. with var');
$r = '';
for my $i (reverse 'A' .. 'C') {
    $r .= $i;
}
is ($r, 'CBA', 'Reverse for list via .. with var');

# For some reason the generate optree is different when $_ is implicit.
$r = '';
for $_ (@array) {
    $r .= $_;
}
is ($r, 'ABC', 'Forwards for array with explicit $_');
$r = '';
for $_ (1,2,3) {
    $r .= $_;
}
is ($r, '123', 'Forwards for list with explicit $_');
$r = '';
for $_ (map {$_} @array) {
    $r .= $_;
}
is ($r, 'ABC', 'Forwards for array via map with explicit $_');
$r = '';
for $_ (map {$_} 1,2,3) {
    $r .= $_;
}
is ($r, '123', 'Forwards for list via map with explicit $_');
$r = '';
for $_ (1 .. 3) {
    $r .= $_;
}
is ($r, '123', 'Forwards for list via .. with var with explicit $_');
$r = '';
for $_ ('A' .. 'C') {
    $r .= $_;
}
is ($r, 'ABC', 'Forwards for list via .. with var with explicit $_');

$r = '';
for $_ (reverse @array) {
    $r .= $_;
}
is ($r, 'CBA', 'Reverse for array with explicit $_');
$r = '';
for $_ (reverse 1,2,3) {
    $r .= $_;
}
is ($r, '321', 'Reverse for list with explicit $_');
$r = '';
for $_ (reverse map {$_} @array) {
    $r .= $_;
}
is ($r, 'CBA', 'Reverse for array via map with explicit $_');
$r = '';
for $_ (reverse map {$_} 1,2,3) {
    $r .= $_;
}
is ($r, '321', 'Reverse for list via map with explicit $_');
$r = '';
for $_ (reverse 1 .. 3) {
    $r .= $_;
}
is ($r, '321', 'Reverse for list via .. with var with explicit $_');
$r = '';
for $_ (reverse 'A' .. 'C') {
    $r .= $_;
}
is ($r, 'CBA', 'Reverse for list via .. with var with explicit $_');

# I don't think that my is that different from our in the optree. But test a
# few:
$r = '';
for our $i (reverse @array) {
    $r .= $i;
}
is ($r, 'CBA', 'Reverse for array with our var');
$r = '';
for our $i (reverse 1,2,3) {
    $r .= $i;
}
is ($r, '321', 'Reverse for list with our var');
$r = '';
for our $i (reverse map {$_} @array) {
    $r .= $i;
}
is ($r, 'CBA', 'Reverse for array via map with our var');
$r = '';
for our $i (reverse map {$_} 1,2,3) {
    $r .= $i;
}
is ($r, '321', 'Reverse for list via map with our var');
$r = '';
for our $i (reverse 1 .. 3) {
    $r .= $i;
}
is ($r, '321', 'Reverse for list via .. with our var');
$r = '';
for our $i (reverse 'A' .. 'C') {
    $r .= $i;
}
is ($r, 'CBA', 'Reverse for list via .. with our var');


$r = '';
for (1, reverse @array) {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for array with leading value');
$r = '';
for ('A', reverse 1,2,3) {
    $r .= $_;
}
is ($r, 'A321', 'Reverse for list with leading value');
$r = '';
for (1, reverse map {$_} @array) {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for array via map with leading value');
$r = '';
for ('A', reverse map {$_} 1,2,3) {
    $r .= $_;
}
is ($r, 'A321', 'Reverse for list via map with leading value');
$r = '';
for ('A', reverse 1 .. 3) {
    $r .= $_;
}
is ($r, 'A321', 'Reverse for list via .. with leading value');
$r = '';
for (1, reverse 'A' .. 'C') {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for list via .. with leading value');

$r = '';
for (reverse (@array), 1) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for array with trailing value');
$r = '';
for (reverse (1,2,3), 'A') {
    $r .= $_;
}
is ($r, '321A', 'Reverse for list with trailing value');
$r = '';
for (reverse (map {$_} @array), 1) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for array via map with trailing value');
$r = '';
for (reverse (map {$_} 1,2,3), 'A') {
    $r .= $_;
}
is ($r, '321A', 'Reverse for list via map with trailing value');
$r = '';
for (reverse (1 .. 3), 'A') {
    $r .= $_;
}
is ($r, '321A', 'Reverse for list via .. with trailing value');
$r = '';
for (reverse ('A' .. 'C'), 1) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for list via .. with trailing value');


$r = '';
for $_ (1, reverse @array) {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for array with leading value with explicit $_');
$r = '';
for $_ ('A', reverse 1,2,3) {
    $r .= $_;
}
is ($r, 'A321', 'Reverse for list with leading value with explicit $_');
$r = '';
for $_ (1, reverse map {$_} @array) {
    $r .= $_;
}
is ($r, '1CBA',
    'Reverse for array via map with leading value with explicit $_');
$r = '';
for $_ ('A', reverse map {$_} 1,2,3) {
    $r .= $_;
}
is ($r, 'A321', 'Reverse for list via map with leading value with explicit $_');
$r = '';
for $_ ('A', reverse 1 .. 3) {
    $r .= $_;
}
is ($r, 'A321', 'Reverse for list via .. with leading value with explicit $_');
$r = '';
for $_ (1, reverse 'A' .. 'C') {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for list via .. with leading value with explicit $_');

$r = '';
for $_ (reverse (@array), 1) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for array with trailing value with explicit $_');
$r = '';
for $_ (reverse (1,2,3), 'A') {
    $r .= $_;
}
is ($r, '321A', 'Reverse for list with trailing value with explicit $_');
$r = '';
for $_ (reverse (map {$_} @array), 1) {
    $r .= $_;
}
is ($r, 'CBA1',
    'Reverse for array via map with trailing value with explicit $_');
$r = '';
for $_ (reverse (map {$_} 1,2,3), 'A') {
    $r .= $_;
}
is ($r, '321A',
    'Reverse for list via map with trailing value with explicit $_');
$r = '';
for $_ (reverse (1 .. 3), 'A') {
    $r .= $_;
}
is ($r, '321A', 'Reverse for list via .. with trailing value with explicit $_');
$r = '';
for $_ (reverse ('A' .. 'C'), 1) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for list via .. with trailing value with explicit $_');

$r = '';
for my $i (1, reverse @array) {
    $r .= $i;
}
is ($r, '1CBA', 'Reverse for array with leading value and var');
$r = '';
for my $i ('A', reverse 1,2,3) {
    $r .= $i;
}
is ($r, 'A321', 'Reverse for list with leading value and var');
$r = '';
for my $i (1, reverse map {$_} @array) {
    $r .= $i;
}
is ($r, '1CBA', 'Reverse for array via map with leading value and var');
$r = '';
for my $i ('A', reverse map {$_} 1,2,3) {
    $r .= $i;
}
is ($r, 'A321', 'Reverse for list via map with leading value and var');
$r = '';
for my $i ('A', reverse 1 .. 3) {
    $r .= $i;
}
is ($r, 'A321', 'Reverse for list via .. with leading value and var');
$r = '';
for my $i (1, reverse 'A' .. 'C') {
    $r .= $i;
}
is ($r, '1CBA', 'Reverse for list via .. with leading value and var');

$r = '';
for my $i (reverse (@array), 1) {
    $r .= $i;
}
is ($r, 'CBA1', 'Reverse for array with trailing value and var');
$r = '';
for my $i (reverse (1,2,3), 'A') {
    $r .= $i;
}
is ($r, '321A', 'Reverse for list with trailing value and var');
$r = '';
for my $i (reverse (map {$_} @array), 1) {
    $r .= $i;
}
is ($r, 'CBA1', 'Reverse for array via map with trailing value and var');
$r = '';
for my $i (reverse (map {$_} 1,2,3), 'A') {
    $r .= $i;
}
is ($r, '321A', 'Reverse for list via map with trailing value and var');
$r = '';
for my $i (reverse (1 .. 3), 'A') {
    $r .= $i;
}
is ($r, '321A', 'Reverse for list via .. with trailing value and var');
$r = '';
for my $i (reverse ('A' .. 'C'), 1) {
    $r .= $i;
}
is ($r, 'CBA1', 'Reverse for list via .. with trailing value and var');


$r = '';
for (reverse 1, @array) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for value and array');
$r = '';
for (reverse map {$_} 1, @array) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for value and array via map');
$r = '';
for (reverse 1 .. 3, @array) {
    $r .= $_;
}
is ($r, 'CBA321', 'Reverse for .. and array');
$r = '';
for (reverse 'X' .. 'Z', @array) {
    $r .= $_;
}
is ($r, 'CBAZYX', 'Reverse for .. and array');
$r = '';
for (reverse map {$_} 1 .. 3, @array) {
    $r .= $_;
}
is ($r, 'CBA321', 'Reverse for .. and array via map');
$r = '';
for (reverse map {$_} 'X' .. 'Z', @array) {
    $r .= $_;
}
is ($r, 'CBAZYX', 'Reverse for .. and array via map');

$r = '';
for (reverse (@array, 1)) {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for array and value');
$r = '';
for (reverse (map {$_} @array, 1)) {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for array and value via map');

$r = '';
for $_ (reverse 1, @array) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for value and array with explicit $_');
$r = '';
for $_ (reverse map {$_} 1, @array) {
    $r .= $_;
}
is ($r, 'CBA1', 'Reverse for value and array via map with explicit $_');
$r = '';
for $_ (reverse 1 .. 3, @array) {
    $r .= $_;
}
is ($r, 'CBA321', 'Reverse for .. and array with explicit $_');
$r = '';
for $_ (reverse 'X' .. 'Z', @array) {
    $r .= $_;
}
is ($r, 'CBAZYX', 'Reverse for .. and array with explicit $_');
$r = '';
for $_ (reverse map {$_} 1 .. 3, @array) {
    $r .= $_;
}
is ($r, 'CBA321', 'Reverse for .. and array via map with explicit $_');
$r = '';
for $_ (reverse map {$_} 'X' .. 'Z', @array) {
    $r .= $_;
}
is ($r, 'CBAZYX', 'Reverse for .. and array via map with explicit $_');

$r = '';
for $_ (reverse (@array, 1)) {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for array and value with explicit $_');
$r = '';
for $_ (reverse (map {$_} @array, 1)) {
    $r .= $_;
}
is ($r, '1CBA', 'Reverse for array and value via map with explicit $_');


$r = '';
for my $i (reverse 1, @array) {
    $r .= $i;
}
is ($r, 'CBA1', 'Reverse for value and array with var');
$r = '';
for my $i (reverse map {$_} 1, @array) {
    $r .= $i;
}
is ($r, 'CBA1', 'Reverse for value and array via map with var');
$r = '';
for my $i (reverse 1 .. 3, @array) {
    $r .= $i;
}
is ($r, 'CBA321', 'Reverse for .. and array with var');
$r = '';
for my $i (reverse 'X' .. 'Z', @array) {
    $r .= $i;
}
is ($r, 'CBAZYX', 'Reverse for .. and array with var');
$r = '';
for my $i (reverse map {$_} 1 .. 3, @array) {
    $r .= $i;
}
is ($r, 'CBA321', 'Reverse for .. and array via map with var');
$r = '';
for my $i (reverse map {$_} 'X' .. 'Z', @array) {
    $r .= $i;
}
is ($r, 'CBAZYX', 'Reverse for .. and array via map with var');

$r = '';
for my $i (reverse (@array, 1)) {
    $r .= $i;
}
is ($r, '1CBA', 'Reverse for array and value with var');
$r = '';
for my $i (reverse (map {$_} @array, 1)) {
    $r .= $i;
}
is ($r, '1CBA', 'Reverse for array and value via map with var');

{
    no warnings 'void';
    is (do {17; foreach (1, 2) { 1; } }, "",
        "RT #1085: what should be output of perl -we 'print do { foreach (1, 2) { 1; } }'");
}

TODO: {
    local $TODO = "RT #2166: foreach spuriously autovivifies";
    my %h;
    foreach (@h{'a', 'b'}) {}
    is keys(%h), 0, 'RT #2166: foreach spuriously autovivifies';
}

sub {
    foreach (@_) {
        is eval { \$_ }, \undef, 'foreach (@array_containing_undef)'
    }
}->(undef);

SKIP: {
    skip "No XS::APItest under miniperl", 1, if is_miniperl;
    skip "no XS::APItest", 1 if !eval { require XS::APItest };
    my @a;
    sub {
        XS::APItest::alias_av(\@a, 0, undef);
        eval { \$_[0] }
    }->($a[0]);
    is $@, "", 'vivify_defelem does not croak on &PL_sv_undef elements';
}

for $x ($y) {
    $x = 3;
    ($x, my $z) = (1, $y);
    is $z, 3, 'list assignment after aliasing via foreach';
}

for my $x (my $y) {
    $x = 3;
    ($x, my $z) = (1, $y);
    is $z, 3, 'list assignment after aliasing lexical var via foreach';
}

@_ = ();
@_ = (1,2,3,scalar do{for(@_){}} + 1, 4, 5, 6);
is "@_", "1 2 3 1 4 5 6",
   '[perl #124004] scalar for(@empty_array) stack bug';

# DAPM: while messing with the scope code, I broke some cpan/ code,
# but surprisingly didn't break any dedicated tests. So test it:

sub fscope {
    for my $y (1,2) {
	my $a = $y;
	return $a;
    }
}

is(fscope(), 1, 'return via loop in sub');

# make sure a NULL GvSV is restored at the end of the loop

{
    local $foo = "boo";
    {
        local *foo;
        for $foo (1,2) {}
        ok(!defined $foo, "NULL GvSV");
    }
}

# make sure storing an int in a NULL GvSV is ok

{
    local $foo = "boo";
    {
        local *foo;
        for $foo (1..2) {}
        ok(!defined $foo, "NULL GvSV int iterator");
    }
}

# RT #123994 - handle a null GVSV within a loop

{
    local *foo;
    local $foo = "outside";

    my $i = 0;
    for $foo (0..1) {
        is($foo, $i, "RT #123994 int range $i");
        *foo = "";
        $i++;
    }
    is($foo, "outside", "RT #123994 int range outside");

    $i = 0;
    for $foo ('0'..'1') {
        is($foo, $i, "RT #123994 str range $i");
        *foo = "";
        $i++;
    }
    is($foo, "outside", "RT #123994 str range outside");

    $i = 0;
    for $foo (0, 1) {
        is($foo, $i, "RT #123994 list $i");
        *foo = "";
        $i++;
    }
    is($foo, "outside", "RT #123994 list outside");

    my @a = (0,1);
    $i = 0;
    for $foo (@a) {
        is($foo, $i, "RT #123994 array $i");
        *foo = "";
        $i++;
    }
    is($foo, "outside", "RT #123994 array outside");
}

# RT #133558 'reverse' under AIX was causing loop to terminate
# immediately, probably due to compiler bug

{
    my @a = qw(foo);
    my @b;
    push @b, $_ for (reverse @a);
    is "@b", "foo", " RT #133558 reverse array";

    @b = ();
    push @b, $_ for (reverse 'bar');
    is "@b", "bar", " RT #133558 reverse list";
}

{
    my @numbers = 0..2;
    for my $i (@numbers) {
        ++$i;
    }
    is("@numbers", '1 2 3', 'for iterators are aliases');

    my @letters = qw(a b c);

    for my $i (@numbers, @letters) {
        ++$i;
    }
    is("@numbers", '2 3 4', 'iterate on two arrays together one');
    is("@letters", 'b c d', 'iterate on two arrays together two');

    my $got = eval {
        for my $i (@letters, undef, @numbers) {
            ++$i;
        }
        1;
    };
    is($got, undef, 'aliased rvalue');
    like($@, qr/^Modification of a read-only value attempted/,
         'aliased rvalue threw the correct exception');

    is("@letters", 'c d e', 'letters were incremented');
    is("@numbers", '2 3 4', 'numbers were not');

    for my $i (@numbers[0, 1, 0]) {
        ++$i;
    }
    is("@numbers", '4 4 4', 'array slices are lvalues');
}

# It turns out that these are legal. Whether they should be is another matter.

{
    my @opinion = qw(Emergent Behaviour);
    my @observation;
    for CORE::my $var (@opinion) {
        push @observation, $var;
    }
    is("@observation", "@opinion", 'for CORE::my $oh_my_oh_my ...');

    @observation = ();
    for CORE::our $var (@opinion) {
        push @observation, $var;
    }
    is("@observation", "@opinion", 'for CORE::our $var ...');

}

# Likewise, we have this inconsistency currently:
{
    use feature "state";
    ++$Dog::VERSION;

    for my $token (qw(my our)) {
        my $code = "for $token Dog \$spot ('Woof') { } 42";
        is(eval $code, 42, "$code is valid");
        is($@, "", "$code had no errors");
    }

    # But these are all invalid:
    for my $token (qw(CORE::my CORE::our CORE::state state)) {
        my $code = "for $token Dog \$spot ('Woof') { } 42 ";
        is(eval $code, undef, "$code is not valid");
        like($@, qr/^Missing \$ on loop variable/, "$code had emergent error");
    }
}

# the GV of the loop variable didn't have its refcount bumped while being
# used by the loop, so it was possible to free it mid-loop.  This used to
# assert/SEGV

{
    my $f = "a_low_refcnt_package_var";
    my $i = 0;
    no strict 'refs';
    for ${*$f} (5,11,33) {
        delete $main::{$f};
        $i++;
    }
    is($i, 3, "deleting glob is safe");
}


done_testing();
