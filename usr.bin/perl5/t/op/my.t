#!./perl
BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

sub foo {
    my($a, $b) = @_;
    my $c;
    my $d;
    $c = "ok 3\n";
    $d = "ok 4\n";
    { my($a, undef, $c) = ("ok 9\n", "not ok 10\n", "ok 10\n");
      ($x, $y) = ($a, $c); }
    is($a, "ok 1\n", 'value of sub argument maintained outside of block');
    is($b, "ok 2\n", 'sub argument maintained');
    is($c, "ok 3\n", 'variable value maintained outside of block');
    is($d, "ok 4\n", 'variable value maintained');
}

$a = "ok 5\n";
$b = "ok 6\n";
$c = "ok 7\n";
$d = "ok 8\n";

&foo("ok 1\n","ok 2\n");

is($a, "ok 5\n", 'global was not affected by duplicate names inside subroutine');
is($b, "ok 6\n", '...');
is($c, "ok 7\n", '...');
is($d, "ok 8\n", '...');
is($x, "ok 9\n", 'globals modified inside of block keeps its value outside of block');
is($y, "ok 10\n", '...');

# same thing, only with arrays and associative arrays

sub foo2 {
    my($a, @b) = @_;
    my(@c, %d);
    @c = "ok 13\n";
    $d{''} = "ok 14\n";
    { my($a,@c) = ("ok 19\n", "ok 20\n", "ok 21\n"); ($x, $y) = ($a, @c); }
    is($a, "ok 11\n", 'value of sub argument maintained outside of block');
    is(scalar @b, 1, 'did not add any elements to @b');
    is($b[0], "ok 12\n", 'did not alter @b');
    is(scalar @c, 1, 'did not add arguments to @c');
    is($c[0], "ok 13\n", 'did not alter @c');
    is($d{''}, "ok 14\n", 'did not touch %d');
}

$a = "ok 15\n";
@b = "ok 16\n";
@c = "ok 17\n";
$d{''} = "ok 18\n";

&foo2("ok 11\n", "ok 12\n");

is($a, "ok 15\n", 'Global was not modifed out of scope');
is(scalar @b, 1, 'correct number of elements in array');
is($b[0], "ok 16\n", 'array value was not modified out of scope');
is(scalar @c, 1, 'correct number of elements in array');
is($c[0], "ok 17\n", 'array value was not modified out of scope');
is($d{''}, "ok 18\n", 'hash key/value pair is correct');
is($x, "ok 19\n", 'global was modified');
is($y, "ok 20\n", 'this one too');

my $i = "outer";

if (my $i = "inner") {
    is( $i, 'inner', 'my variable inside conditional propagates inside block');
}

if ((my $i = 1) == 0) {
    fail("nested parens do not propagate variable outside");
}
else {
    is($i, 1, 'lexical variable lives available inside else block');
}

my $j = 5;
while (my $i = --$j) {
    last unless is( $i, $j, 'lexical inside while block');
}
continue {
    last unless is( $i, $j, 'lexical inside continue block');
}
is( $j, 0, 'went through the previous while/continue loop all 4 times' );

$j = 5;
for (my $i = 0; (my $k = $i) < $j; ++$i) {
    fail(""), last unless $i >= 0 && $i < $j && $i == $k;
}
ok( ! defined $k, '$k is only defined in the scope of the previous for loop' );

curr_test(37);
$jj = 0;
foreach my $i (30, 31) {
    is( $i, $jj+30, 'assignment inside the foreach loop variable definition');
    $jj++;
}
is( $jj, 2, 'foreach loop executed twice');

is( $i, 'outer', '$i not modified by while/for/foreach using same variable name');

# Ensure that C<my @y> (without parens) doesn't force scalar context.
my @x;
{ @x = my @y }
is(scalar @x, 0, 'my @y without parens does not force scalar context');
{ @x = my %y }
is(scalar @x, 0, 'my %y without parens does not force scalar context');

# Found in HTML::FormatPS
my %fonts = qw(nok 35);
for my $full (keys %fonts) {
    $full =~ s/^n//;
    is( $fonts{nok}, 35, 'Supposed to be copy-on-write via force_normal after a THINKFIRST check.' );
}

#  [perl #29340] optimising away the = () left the padav returning the
# array rather than the contents, leading to 'Bizarre copy of array' error

sub opta { my @a=() }
sub opth { my %h=() }
eval { my $x = opta };
is($@, '', ' perl #29340, No bizarre copy of array error');
eval { my $x = opth };
is($@, '', ' perl #29340, No bizarre copy of array error via hash');

sub foo3 {
    ++my $x->{foo};
    ok(! defined $x->{bar}, '$x->{bar} is not defined');
    ++$x->{bar};
}
eval { foo3(); foo3(); };
is( $@, '', 'no errors while checking autovivification and persistence of hash refs inside subs' );

# my $foo = undef should always assign [perl #37776]
{
    my $count = 35;
    loop:
    my $test = undef;
    is($test, undef, 'var is undef, repeated test');
    $test = 42;
    goto loop if ++$count < 37;
}

# [perl #113554]
eval "my ()";
is( $@, '', "eval of my() passes");

# RT #126844
# This triggered a compile-time assert failure in rpeep()
eval 'my($a,$b),$x,my($c,$d)';
pass("RT #126844");

# RT # 133543
my @false_conditionals = (
    'my $x1 if 0;',
    'my @x2 if 0;',
    'my %x3 if 0;',
    'my ($x4) if 0;',
    'my ($x5,@x6, %x7) if 0;',
    '0 && my $z1;',
    '0 && my (%z2);',
);
for (my $i=0; $i<=$#false_conditionals; $i++) {
    eval $false_conditionals[$i];
    like( $@, qr/^This use of my\(\) in false conditional is no longer allowed/,
        "RT #133543: my() in false conditional: $false_conditionals[$i]");
}

#Variable number of tests due to the way the while/for loops are tested now
done_testing();
