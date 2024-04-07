# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 203;

# basic testing of Math::BigRat

use Math::BigRat;
use Math::BigInt;
use Math::BigFloat;

# shortcuts
my $mbr = 'Math::BigRat';
my $mbi = 'Math::BigInt';
my $mbf = 'Math::BigFloat';

my ($x, $y, $z);

$x = Math::BigRat->new(1234);
is($x, 1234, 'value of $x');
isa_ok($x, 'Math::BigRat');
ok(!$x->isa('Math::BigInt'),
   "An object of class '" . ref($x) . "' isn't a 'Math::BigInt'");
ok(!$x->isa('Math::BigFloat'),
   "An object of class '" . ref($x) . "' isn't a 'Math::BigFloat'");

##############################################################################
# new and bnorm()

foreach my $method (qw/ new bnorm /) {
    $x = $mbr->$method(1234);
    is($x, 1234, qq|\$x = $mbr->$method(1234)|);

    $x = $mbr->$method("1234/1");
    is($x, 1234, qq|\$x = $mbr->$method("1234/1")|);

    $x = $mbr->$method("1234/2");
    is($x, 617, qq|\$x = $mbr->$method("1234/2")|);

    $x = $mbr->$method("100/1.0");
    is($x, 100, qq|\$x = $mbr->$method("100/1.0")|);

    $x = $mbr->$method("10.0/1.0");
    is($x, 10, qq|\$x = $mbr->$method("10.0/1.0")|);

    $x = $mbr->$method("0.1/10");
    is($x, "1/100", qq|\$x = $mbr->$method("0.1/10")|);

    $x = $mbr->$method("0.1/0.1");
    is($x, "1", qq|\$x = $mbr->$method("0.1/0.1")|);

    $x = $mbr->$method("1e2/10");
    is($x, 10, qq|\$x = $mbr->$method("1e2/10")|);

    $x = $mbr->$method("5/1e2");
    is($x, "1/20", qq|\$x = $mbr->$method("5/1e2")|);

    $x = $mbr->$method("1e2/1e1");
    is($x, 10, qq|\$x = $mbr->$method("1e2/1e1")|);

    $x = $mbr->$method("1 / 3");
    is($x, "1/3", qq|\$x = $mbr->$method("1 / 3")|);

    $x = $mbr->$method("-1 / 3");
    is($x, "-1/3", qq|\$x = $mbr->$method("-1 / 3")|);

    $x = $mbr->$method("NaN");
    is($x, "NaN", qq|\$x = $mbr->$method("NaN")|);

    $x = $mbr->$method("inf");
    is($x, "inf", qq|\$x = $mbr->$method("inf")|);

    $x = $mbr->$method("-inf");
    is($x, "-inf", qq|\$x = $mbr->$method("-inf")|);

    $x = $mbr->$method("1/");
    is($x, "NaN", qq|\$x = $mbr->$method("1/")|);

    $x = $mbr->$method("0x7e");
    is($x, 126, qq|\$x = $mbr->$method("0x7e")|);

    # input ala "1+1/3" isn"t parsed ok yet
    $x = $mbr->$method("1+1/3");
    is($x, "NaN", qq|\$x = $mbr->$method("1+1/3")|);

    $x = $mbr->$method("1/1.2");
    is($x, "5/6", qq|\$x = $mbr->$method("1/1.2")|);

    $x = $mbr->$method("1.3/1.2");
    is($x, "13/12", qq|\$x = $mbr->$method("1.3/1.2")|);

    $x = $mbr->$method("1.2/1");
    is($x, "6/5", qq|\$x = $mbr->$method("1.2/1")|);

    ############################################################################
    # other classes as input

    $x = $mbr->$method($mbi->new(1231));
    is($x, "1231", qq|\$x = $mbr->$method($mbi->new(1231))|);

    $x = $mbr->$method($mbf->new(1232));
    is($x, "1232", qq|\$x = $mbr->$method($mbf->new(1232))|);

    $x = $mbr->$method($mbf->new(1232.3));
    is($x, "12323/10", qq|\$x = $mbr->$method($mbf->new(1232.3))|);
}

my $n = 'numerator';
my $d = 'denominator';

$x = $mbr->new('-0');
is($x, '0');
is($x->$n(), '0');
is($x->$d(), '1');

$x = $mbr->new('NaN');
is($x, 'NaN');  is($x->$n(), 'NaN');
is($x->$d(), 'NaN');

$x = $mbr->new('-NaN');
is($x, 'NaN');  is($x->$n(), 'NaN');
is($x->$d(), 'NaN');

$x = $mbr->new('-1r4');
is($x, 'NaN');  is($x->$n(), 'NaN');
is($x->$d(), 'NaN');

$x = $mbr->new('+inf');
is($x, 'inf');  is($x->$n(), 'inf');
is($x->$d(), '1');

$x = $mbr->new('-inf');
is($x, '-inf');
is($x->$n(), '-inf');
is($x->$d(), '1');

$x = $mbr->new('123a4');
is($x, 'NaN');
is($x->$n(), 'NaN');
is($x->$d(), 'NaN');

# wrong inputs
$x = $mbr->new('1e2e2');
is($x, 'NaN');
is($x->$n(), 'NaN');
is($x->$d(), 'NaN');

$x = $mbr->new('1+2+2');
is($x, 'NaN');
is($x->$n(), 'NaN');
is($x->$d(), 'NaN');

# failed due to BigFloat bug
$x = $mbr->new('1.2.2');
is($x, 'NaN');
is($x->$n(), 'NaN');
is($x->$d(), 'NaN');

is($mbr->new('123a4'), 'NaN');
is($mbr->new('123e4'), '1230000');
is($mbr->new('-NaN'), 'NaN');
is($mbr->new('NaN'), 'NaN');
is($mbr->new('+inf'), 'inf');
is($mbr->new('-inf'), '-inf');

##############################################################################
# two Bigints

is($mbr->new($mbi->new(3), $mbi->new(7))->badd(1), '10/7');
is($mbr->new($mbi->new(-13), $mbi->new(7)), '-13/7');
is($mbr->new($mbi->new(13), $mbi->new(-7)), '-13/7');
is($mbr->new($mbi->new(-13), $mbi->new(-7)), '13/7');

##############################################################################
# mixed arguments

is($mbr->new('3/7')->badd(1), '10/7');
is($mbr->new('3/10')->badd(1.1), '7/5');
is($mbr->new('3/7')->badd($mbi->new(1)), '10/7');
is($mbr->new('3/10')->badd($mbf->new('1.1')), '7/5');

is($mbr->new('3/7')->bsub(1), '-4/7');
is($mbr->new('3/10')->bsub(1.1), '-4/5');
is($mbr->new('3/7')->bsub($mbi->new(1)), '-4/7');
is($mbr->new('3/10')->bsub($mbf->new('1.1')), '-4/5');

is($mbr->new('3/7')->bmul(1), '3/7');
is($mbr->new('3/10')->bmul(1.1), '33/100');
is($mbr->new('3/7')->bmul($mbi->new(1)), '3/7');
is($mbr->new('3/10')->bmul($mbf->new('1.1')), '33/100');

is($mbr->new('3/7')->bdiv(1), '3/7');
is($mbr->new('3/10')->bdiv(1.1), '3/11');
is($mbr->new('3/7')->bdiv($mbi->new(1)), '3/7');
is($mbr->new('3/10')->bdiv($mbf->new('1.1')), '3/11');

##############################################################################
$x = $mbr->new('1/4');
$y = $mbr->new('1/3');

is($x + $y, '7/12');
is($x * $y, '1/12');
is($x / $y, '3/4');

$x = $mbr->new('7/5');
$x *= '3/2';
is($x, '21/10');
$x -= '0.1';
is($x, '2');                    # not 21/10

$x = $mbr->new('2/3');
$y = $mbr->new('3/2');
is($x > $y, '');
is($x < $y, 1);
is($x == $y, '');

$x = $mbr->new('-2/3');
$y = $mbr->new('3/2');
is($x > $y, '');
is($x < $y, '1');
is($x == $y, '');

$x = $mbr->new('-2/3');
$y = $mbr->new('-2/3');
is($x > $y, '');
is($x < $y, '');
is($x == $y, '1');

$x = $mbr->new('-2/3');
$y = $mbr->new('-1/3');
is($x > $y, '');
is($x < $y, '1');
is($x == $y, '');

$x = $mbr->new('-124');
$y = $mbr->new('-122');
is($x->bacmp($y), 1);

$x = $mbr->new('-124');
$y = $mbr->new('-122');
is($x->bcmp($y), -1);

$x = $mbr->new('3/7');
$y = $mbr->new('5/7');
is($x+$y, '8/7');

$x = $mbr->new('3/7');
$y = $mbr->new('5/7');
is($x*$y, '15/49');

$x = $mbr->new('3/5');
$y = $mbr->new('5/7');
is($x*$y, '3/7');

$x = $mbr->new('3/5');
$y = $mbr->new('5/7');
is($x/$y, '21/25');

$x = $mbr->new('7/4');
$y = $mbr->new('1');
is($x % $y, '3/4');

$x = $mbr->new('7/4');
$y = $mbr->new('5/13');
is($x % $y, '11/52');

$x = $mbr->new('7/4');
$y = $mbr->new('5/9');
is($x % $y, '1/12');

$x = $mbr->new('-144/9')->bsqrt();
is($x, 'NaN');

$x = $mbr->new('144/9')->bsqrt();
is($x, '4');

$x = $mbr->new('3/4')->bsqrt();
is($x,
   '4330127018922193233818615853764680917357/' .
   '5000000000000000000000000000000000000000');

##############################################################################
# bpow

$x = $mbr->new('2/1');
$z = $x->bpow('3/1');
is($x, '8');

$x = $mbr->new('1/2');
$z = $x->bpow('3/1');
is($x, '1/8');

$x = $mbr->new('1/3');
$z = $x->bpow('4/1');
is($x, '1/81');

$x = $mbr->new('2/3');
$z = $x->bpow('4/1');
is($x, '16/81');

$x = $mbr->new('2/3');
$z = $x->bpow('5/3');
is($x, '31797617848703662994667839220546583581/62500000000000000000000000000000000000');

##############################################################################
# bfac

$x = $mbr->new('1');
$x->bfac();
is($x, '1');

for (my $i = 0; $i < 8; $i++) {
    $x = $mbr->new("$i/1")->bfac();
    is($x, $mbi->new($i)->bfac());
}

# test for $self->bnan() vs. $x->bnan();
$x = $mbr->new('-1');
$x->bfac();
is($x, 'NaN');

##############################################################################
# binc/bdec

note("binc()");
$x = $mbr->new('3/2');
is($x->binc(), '5/2');

note("bdec()");

$x = $mbr->new('15/6');
is($x->bdec(), '3/2');

##############################################################################
# bfloor

note("bfloor()");
$x = $mbr->new('-7/7');
is($x->$n(), '-1');
is($x->$d(), '1');
$x = $mbr->new('-7/7')->bfloor();
is($x->$n(), '-1');
is($x->$d(), '1');

##############################################################################
# bsstr

$x = $mbr->new('7/5')->bsstr();
is($x, '7/5');
$x = $mbr->new('-7/5')->bsstr();
is($x, '-7/5');

##############################################################################

note("numify()");

my @array = qw/1 2 3 4 5 6 7 8 9/;
$x = $mbr->new('8/8');
is($array[$x], 2);

$x = $mbr->new('16/8');
is($array[$x], 3);

$x = $mbr->new('17/8');
is($array[$x], 3);

$x = $mbr->new('33/8');
is($array[$x], 5);

$x = $mbr->new('-33/8');
is($array[$x], 6);

$x = $mbr->new('-8/1');
is($array[$x], 2);      # -8 => 2

require Math::Complex;

my $inf = $Math::Complex::Inf;
my $nan = $inf - $inf;

sub isnumeric {
    my $value = shift;
    ($value ^ $value) eq "0";
}

subtest qq|$mbr -> new("33/8") -> numify()| => sub {
    plan tests => 3;

    $x = $mbr -> new("33/8") -> numify();
    is(ref($x), "", '$x is a scalar');
    ok(isnumeric($x), '$x is numeric');
    cmp_ok($x, "==", 4.125, '$x has the right value');
};

subtest qq|$mbr -> new("-33/8") -> numify()| => sub {
    plan tests => 3;

    $x = $mbr -> new("-33/8") -> numify();
    is(ref($x), "", '$x is a scalar');
    ok(isnumeric($x), '$x is numeric');
    cmp_ok($x, "==", -4.125, '$x has the right value');
};

subtest qq|$mbr -> new("inf") -> numify()| => sub {
    plan tests => 3;

    $x = $mbr -> new("inf") -> numify();
    is(ref($x), "", '$x is a scalar');
    ok(isnumeric($x), '$x is numeric');
    cmp_ok($x, "==", $inf, '$x has the right value');
};

subtest qq|$mbr -> new("-inf") -> numify()| => sub {
    plan tests => 3;

    $x = $mbr -> new("-inf") -> numify();
    is(ref($x), "", '$x is a scalar');
    ok(isnumeric($x), '$x is numeric');
    cmp_ok($x, "==", -$inf, '$x has the right value');
};

subtest qq|$mbr -> new("NaN") -> numify()| => sub {
    plan tests => 3;

    $x = $mbr -> new("NaN") -> numify();
    is(ref($x), "", '$x is a scalar');
    ok(isnumeric($x), '$x is numeric');
    cmp_ok($x, "!=", $nan, '$x has the right value');   # Note: NaN != NaN
};

##############################################################################
# as_hex(), as_bin(), as_oct()

note("as_hex(), as_bin(), as_oct()");

$x = $mbr->new('8/8');
is($x->as_hex(), '0x1');
is($x->as_bin(), '0b1');
is($x->as_oct(), '01');

$x = $mbr->new('80/8');
is($x->as_hex(), '0xa');
is($x->as_bin(), '0b1010');
is($x->as_oct(), '012');

##############################################################################
# broot(), blog(), bmodpow() and bmodinv()

note("broot(), blog(), bmodpow(), bmodinv()");

$x = $mbr->new(2) ** 32;
$y = $mbr->new(4);
$z = $mbr->new(3);

is($x->copy()->broot($y), 2 ** 8);
is(ref($x->copy()->broot($y)), $mbr, "\$x is a $mbr");

is($x->copy()->bmodpow($y, $z), 1);
is(ref($x->copy()->bmodpow($y, $z)), $mbr, "\$x is a $mbr");

$x = $mbr->new(8);
$y = $mbr->new(5033);
$z = $mbr->new(4404);

is($x->copy()->bmodinv($y), $z);
is(ref($x->copy()->bmodinv($y)), $mbr, "\$x is a $mbr");

# square root with exact result
$x = $mbr->new('1.44');
is($x->copy()->broot(2), '6/5');
is(ref($x->copy()->broot(2)), $mbr, "\$x is a $mbr");

# log with exact result
$x = $mbr->new('256.1');
is($x->copy()->blog(2),
   '8000563442710106079310294693803606983661/1000000000000000000000000000000000000000',
   "\$x = $mbr->new('256.1')->blog(2)");
is(ref($x->copy()->blog(2)), $mbr, "\$x is a $mbr");

$x = $mbr->new(144);
is($x->copy()->broot('2'), 12, 'v/144 = 12');

$x = $mbr->new(12*12*12);
is($x->copy()->broot('3'), 12, '(12*12*12) ** 1/3 = 12');

##############################################################################
# from_hex(), from_bin(), from_oct()

note("from_hex(), from_bin(), from_oct()");

$x = Math::BigRat->from_hex('0x100');
is($x, '256', 'from_hex');

$x = $mbr->from_hex('0x100');
is($x, '256', 'from_hex');

$x = Math::BigRat->from_bin('0b100');
is($x, '4', 'from_bin');

$x = $mbr->from_bin('0b100');
is($x, '4', 'from_bin');

$x = Math::BigRat->from_oct('0100');
is($x, '64', 'from_oct');

$x = $mbr->from_oct('0100');
is($x, '64', 'from_oct');

##############################################################################
# as_float()

$x = Math::BigRat->new('1/2');
my $f = $x->as_float();

is($x, '1/2', '$x unmodified');
is($f, '0.5', 'as_float(0.5)');

$x = Math::BigRat->new('2/3');
$f = $x->as_float(5);

is($x, '2/3', '$x unmodified');
is($f, '0.66667', 'as_float(2/3, 5)');

# Integers should be converted exactly.
$x = Math::BigRat->new("3141592653589793238462643383279502884197169399375106");
$f = $x->as_float();

is($x, "3141592653589793238462643383279502884197169399375106", '$x unmodified');
is($f, "3141592653589793238462643383279502884197169399375106",
   'as_float(3141592653589793238462643383279502884197169399375106, 5)');

##############################################################################
# int()

$x = Math::BigRat->new('5/2');
is(int($x), '2', '5/2 converted to integer');

$x = Math::BigRat->new('-1/2');
is(int($x), '0', '-1/2 converted to integer');

##############################################################################
# done

1;
