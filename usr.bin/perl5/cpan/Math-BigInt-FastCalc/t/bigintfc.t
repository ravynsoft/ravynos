# -*- mode: perl; -*-

# Test Math::BigInt::FastCalc

use strict;
use warnings;

use Test::More tests => 524;

use Math::BigInt::FastCalc;

my ($BASE_LEN, $BASE, $AND_BITS, $XOR_BITS, $OR_BITS,
    $BASE_LEN_SMALL, $MAX_VAL,
    $MAX_BITS, $MAX_EXP_F, $MAX_EXP_I, $USE_INT)
  = Math::BigInt::Calc -> _base_len();

note(<<"EOF");

BASE_LEN  = $BASE_LEN
BASE      = $BASE
MAX_VAL   = $MAX_VAL
AND_BITS  = $AND_BITS
XOR_BITS  = $XOR_BITS
OR_BITS   = $OR_BITS
MAX_EXP_F = $MAX_EXP_F
MAX_EXP_I = $MAX_EXP_I
USE_INT   = $USE_INT
EOF

my $LIB = 'Math::BigInt::FastCalc';
my $REF = 'ARRAY';

# _new and _str

my $x = $LIB->_new("123");
my $y = $LIB->_new("321");
is(ref($x), $REF, q|ref($x) is a $REF|);
is($LIB->_str($x), 123,     qq|$LIB->_str(\$x) = 123|);
is($LIB->_str($y), 321,     qq|$LIB->_str(\$y) = 321|);

###############################################################################
# _add, _sub, _mul, _div

is($LIB->_str($LIB->_add($x, $y)), 444,
   qq|$LIB->_str($LIB->_add(\$x, \$y)) = 444|);
is($LIB->_str($LIB->_sub($x, $y)), 123,
   qq|$LIB->_str($LIB->_sub(\$x, \$y)) = 123|);
is($LIB->_str($LIB->_mul($x, $y)), 39483,
   qq|$LIB->_str($LIB->_mul(\$x, \$y)) = 39483|);
is($LIB->_str($LIB->_div($x, $y)), 123,
   qq|$LIB->_str($LIB->_div(\$x, \$y)) = 123|);

###############################################################################
# check that mul/div doesn't change $y
# and returns the same reference, not something new

is($LIB->_str($LIB->_mul($x, $y)), 39483,
   qq|$LIB->_str($LIB->_mul(\$x, \$y)) = 39483|);
is($LIB->_str($x), 39483,
   qq|$LIB->_str(\$x) = 39483|);
is($LIB->_str($y), 321,
   qq|$LIB->_str(\$y) = 321|);

is($LIB->_str($LIB->_div($x, $y)), 123,
   qq|$LIB->_str($LIB->_div(\$x, \$y)) = 123|);
is($LIB->_str($x), 123,
   qq|$LIB->_str(\$x) = 123|);
is($LIB->_str($y), 321,
   qq|$LIB->_str(\$y) = 321|);

$x = $LIB->_new("39483");
my ($x1, $r1) = $LIB->_div($x, $y);
is("$x1", "$x", q|"$x1" = "$x"|);
$LIB->_inc($x1);
is("$x1", "$x", q|"$x1" = "$x"|);
is($LIB->_str($r1), "0", qq|$LIB->_str(\$r1) = "0"|);

$x = $LIB->_new("39483");       # reset

###############################################################################

my $z = $LIB->_new("2");
is($LIB->_str($LIB->_add($x, $z)), 39485,
   qq|$LIB->_str($LIB->_add(\$x, \$z)) = 39485|);
my ($re, $rr) = $LIB->_div($x, $y);

is($LIB->_str($re), 123, qq|$LIB->_str(\$re) = 123|);
is($LIB->_str($rr), 2,   qq|$LIB->_str(\$rr) = 2|);

# is_zero, _is_one, _one, _zero

ok(! $LIB->_is_zero($x), qq|$LIB->_is_zero(\$x)|);
ok(! $LIB->_is_one($x),  qq|$LIB->_is_one(\$x)|);

is($LIB->_str($LIB->_zero()), "0", qq|$LIB->_str($LIB->_zero()) = "0"|);
is($LIB->_str($LIB->_one()),  "1", qq|$LIB->_str($LIB->_one())  = "1"|);

# _two() and _ten()

is($LIB->_str($LIB->_two()), "2",  qq|$LIB->_str($LIB->_two()) = "2"|);
is($LIB->_str($LIB->_ten()), "10", qq|$LIB->_str($LIB->_ten()) = "10"|);

ok(! $LIB->_is_ten($LIB->_two()), qq|$LIB->_is_ten($LIB->_two()) is false|);
ok(  $LIB->_is_two($LIB->_two()), qq|$LIB->_is_two($LIB->_two()) is true|);
ok(  $LIB->_is_ten($LIB->_ten()), qq|$LIB->_is_ten($LIB->_ten()) is true|);
ok(! $LIB->_is_two($LIB->_ten()), qq|$LIB->_is_two($LIB->_ten()) is false|);

ok(  $LIB->_is_one($LIB->_one()), qq|$LIB->_is_one($LIB->_one()) is true|);
ok(! $LIB->_is_one($LIB->_two()), qq|$LIB->_is_one($LIB->_two()) is false|);
ok(! $LIB->_is_one($LIB->_ten()), qq|$LIB->_is_one($LIB->_ten()) is false|);

ok(! $LIB->_is_one($LIB->_zero()),  qq/$LIB->_is_one($LIB->_zero()) is false/);
ok(  $LIB->_is_zero($LIB->_zero()), qq|$LIB->_is_zero($LIB->_zero()) is true|);
ok(! $LIB->_is_zero($LIB->_one()),  qq/$LIB->_is_zero($LIB->_one()) is false/);

# is_odd, is_even

ok(  $LIB->_is_odd($LIB->_one()),   qq/$LIB->_is_odd($LIB->_one()) is true/);
ok(! $LIB->_is_odd($LIB->_zero()),  qq/$LIB->_is_odd($LIB->_zero()) is false/);
ok(! $LIB->_is_even($LIB->_one()),  qq/$LIB->_is_even($LIB->_one()) is false/);
ok(  $LIB->_is_even($LIB->_zero()), qq/$LIB->_is_even($LIB->_zero()) is true/);

# _alen and _len

for my $method (qw/_alen _len/) {
    $x = $LIB->_new("1");
    is($LIB->$method($x), 1, qq|$LIB->$method(\$x) = 1|);
    $x = $LIB->_new("12");
    is($LIB->$method($x), 2, qq|$LIB->$method(\$x) = 2|);
    $x = $LIB->_new("123");
    is($LIB->$method($x), 3, qq|$LIB->$method(\$x) = 3|);
    $x = $LIB->_new("1234");
    is($LIB->$method($x), 4, qq|$LIB->$method(\$x) = 4|);
    $x = $LIB->_new("12345");
    is($LIB->$method($x), 5, qq|$LIB->$method(\$x) = 5|);
    $x = $LIB->_new("123456");
    is($LIB->$method($x), 6, qq|$LIB->$method(\$x) = 6|);
    $x = $LIB->_new("1234567");
    is($LIB->$method($x), 7, qq|$LIB->$method(\$x) = 7|);
    $x = $LIB->_new("12345678");
    is($LIB->$method($x), 8, qq|$LIB->$method(\$x) = 8|);
    $x = $LIB->_new("123456789");
    is($LIB->$method($x), 9, qq|$LIB->$method(\$x) = 9|);

    $x = $LIB->_new("8");
    is($LIB->$method($x), 1, qq|$LIB->$method(\$x) = 1|);
    $x = $LIB->_new("21");
    is($LIB->$method($x), 2, qq|$LIB->$method(\$x) = 2|);
    $x = $LIB->_new("321");
    is($LIB->$method($x), 3, qq|$LIB->$method(\$x) = 3|);
    $x = $LIB->_new("4321");
    is($LIB->$method($x), 4, qq|$LIB->$method(\$x) = 4|);
    $x = $LIB->_new("54321");
    is($LIB->$method($x), 5, qq|$LIB->$method(\$x) = 5|);
    $x = $LIB->_new("654321");
    is($LIB->$method($x), 6, qq|$LIB->$method(\$x) = 6|);
    $x = $LIB->_new("7654321");
    is($LIB->$method($x), 7, qq|$LIB->$method(\$x) = 7|);
    $x = $LIB->_new("87654321");
    is($LIB->$method($x), 8, qq|$LIB->$method(\$x) = 8|);
    $x = $LIB->_new("987654321");
    is($LIB->$method($x), 9, qq|$LIB->$method(\$x) = 9|);

    $x = $LIB->_new("0");
    is($LIB->$method($x), 1, qq|$LIB->$method(\$x) = 1|);
    $x = $LIB->_new("20");
    is($LIB->$method($x), 2, qq|$LIB->$method(\$x) = 2|);
    $x = $LIB->_new("320");
    is($LIB->$method($x), 3, qq|$LIB->$method(\$x) = 3|);
    $x = $LIB->_new("4320");
    is($LIB->$method($x), 4, qq|$LIB->$method(\$x) = 4|);
    $x = $LIB->_new("54320");
    is($LIB->$method($x), 5, qq|$LIB->$method(\$x) = 5|);
    $x = $LIB->_new("654320");
    is($LIB->$method($x), 6, qq|$LIB->$method(\$x) = 6|);
    $x = $LIB->_new("7654320");
    is($LIB->$method($x), 7, qq|$LIB->$method(\$x) = 7|);
    $x = $LIB->_new("87654320");
    is($LIB->$method($x), 8, qq|$LIB->$method(\$x) = 8|);
    $x = $LIB->_new("987654320");
    is($LIB->$method($x), 9, qq|$LIB->$method(\$x) = 9|);

    for (my $i = 1; $i < 9; $i++) {
        my $a = "$i" . '0' x ($i - 1);
        $x = $LIB->_new($a);
        is($LIB->_len($x), $i, qq|$LIB->_len(\$x) = $i|);
    }
}

# _digit

$x = $LIB->_new("123456789");
is($LIB->_digit($x, 0),   9, qq|$LIB->_digit(\$x, 0) = 9|);
is($LIB->_digit($x, 1),   8, qq|$LIB->_digit(\$x, 1) = 8|);
is($LIB->_digit($x, 2),   7, qq|$LIB->_digit(\$x, 2) = 7|);
is($LIB->_digit($x, 8),   1, qq|$LIB->_digit(\$x, 8) = 1|);
is($LIB->_digit($x, 9),   0, qq|$LIB->_digit(\$x, 9) = 0|);
is($LIB->_digit($x, -1),  1, qq|$LIB->_digit(\$x, -1) = 1|);
is($LIB->_digit($x, -2),  2, qq|$LIB->_digit(\$x, -2) = 2|);
is($LIB->_digit($x, -3),  3, qq|$LIB->_digit(\$x, -3) = 3|);
is($LIB->_digit($x, -9),  9, qq|$LIB->_digit(\$x, -9) = 9|);
is($LIB->_digit($x, -10), 0, qq|$LIB->_digit(\$x, -10) = 0|);

# _copy

foreach (qw/ 1 12 123 1234 12345 123456 1234567 12345678 123456789/) {
    $x = $LIB->_new("$_");
    is($LIB->_str($LIB->_copy($x)), "$_",
       qq|$LIB->_str($LIB->_copy(\$x)) = "$_"|);
    is($LIB->_str($x), "$_",           # did _copy destroy original x?
       qq|$LIB->_str(\$x) = "$_"|);
}

# _zeros

$x = $LIB->_new("1256000000");
is($LIB->_zeros($x), 6, qq|$LIB->_zeros(\$x) = 6|);

$x = $LIB->_new("152");
is($LIB->_zeros($x), 0, qq|$LIB->_zeros(\$x) = 0|);

$x = $LIB->_new("123000");
is($LIB->_zeros($x), 3, qq|$LIB->_zeros(\$x) = 3|);

$x = $LIB->_new("0");
is($LIB->_zeros($x), 0, qq|$LIB->_zeros(\$x) = 0|);

# _lsft, _rsft

$x = $LIB->_new("10");
$y = $LIB->_new("3");
is($LIB->_str($LIB->_lsft($x, $y, 10)), 10000,
   qq|$LIB->_str($LIB->_lsft(\$x, \$y, 10)) = 10000|);

$x = $LIB->_new("20");
$y = $LIB->_new("3");
is($LIB->_str($LIB->_lsft($x, $y, 10)), 20000,
   qq|$LIB->_str($LIB->_lsft(\$x, \$y, 10)) = 20000|);

$x = $LIB->_new("128");
$y = $LIB->_new("4");
is($LIB->_str($LIB->_lsft($x, $y, 2)), 128 << 4,
   qq|$LIB->_str($LIB->_lsft(\$x, \$y, 2)) = 128 << 4|);

$x = $LIB->_new("1000");
$y = $LIB->_new("3");
is($LIB->_str($LIB->_rsft($x, $y, 10)), 1,
   qq|$LIB->_str($LIB->_rsft(\$x, \$y, 10)) = 1|);

$x = $LIB->_new("20000");
$y = $LIB->_new("3");
is($LIB->_str($LIB->_rsft($x, $y, 10)), 20,
   qq|$LIB->_str($LIB->_rsft(\$x, \$y, 10)) = 20|);

$x = $LIB->_new("256");
$y = $LIB->_new("4");
is($LIB->_str($LIB->_rsft($x, $y, 2)), 256 >> 4,
   qq|$LIB->_str($LIB->_rsft(\$x, \$y, 2)) = 256 >> 4|);

$x = $LIB->_new("6411906467305339182857313397200584952398");
$y = $LIB->_new("45");
is($LIB->_str($LIB->_rsft($x, $y, 10)), 0,
   qq|$LIB->_str($LIB->_rsft(\$x, \$y, 10)) = 0|);

# _lsft() with large bases

for my $xstr ("1", "2", "3") {
    for my $nstr ("1", "2", "3") {
        for my $bpow (25, 50, 75) {
            my $bstr = "1" . ("0" x $bpow);
            my $expected = $xstr . ("0" x ($bpow * $nstr));
            my $xobj = $LIB->_new($xstr);
            my $nobj = $LIB->_new($nstr);
            my $bobj = $LIB->_new($bstr);

            is($LIB->_str($LIB->_lsft($xobj, $nobj, $bobj)), $expected,
               qq|$LIB->_str($LIB->_lsft($LIB->_new("$xstr"), |
                                    . qq|$LIB->_new("$nstr"), |
                                    . qq|$LIB->_new("$bstr")))|);
            is($LIB->_str($nobj), $nstr, q|$n is unmodified|);
            is($LIB->_str($bobj), $bstr, q|$b is unmodified|);
        }
    }
}

# _acmp

$x = $LIB->_new("123456789");
$y = $LIB->_new("987654321");
is($LIB->_acmp($x, $y), -1, qq|$LIB->_acmp(\$x, \$y) = -1|);
is($LIB->_acmp($y, $x), 1,  qq|$LIB->_acmp(\$y, \$x) = 1|);
is($LIB->_acmp($x, $x), 0,  qq|$LIB->_acmp(\$x, \$x) = 0|);
is($LIB->_acmp($y, $y), 0,  qq|$LIB->_acmp(\$y, \$y) = 0|);
$x = $LIB->_new("12");
$y = $LIB->_new("12");
is($LIB->_acmp($x, $y), 0,  qq|$LIB->_acmp(\$x, \$y) = 0|);
$x = $LIB->_new("21");
is($LIB->_acmp($x, $y), 1,  qq|$LIB->_acmp(\$x, \$y) = 1|);
is($LIB->_acmp($y, $x), -1, qq|$LIB->_acmp(\$y, \$x) = -1|);
$x = $LIB->_new("123456789");
$y = $LIB->_new("1987654321");
is($LIB->_acmp($x, $y), -1, qq|$LIB->_acmp(\$x, \$y) = -1|);
is($LIB->_acmp($y, $x), +1, qq|$LIB->_acmp(\$y, \$x) = +1|);

$x = $LIB->_new("1234567890123456789");
$y = $LIB->_new("987654321012345678");
is($LIB->_acmp($x, $y), 1,  qq|$LIB->_acmp(\$x, \$y) = 1|);
is($LIB->_acmp($y, $x), -1, qq|$LIB->_acmp(\$y, \$x) = -1|);
is($LIB->_acmp($x, $x), 0,  qq|$LIB->_acmp(\$x, \$x) = 0|);
is($LIB->_acmp($y, $y), 0,  qq|$LIB->_acmp(\$y, \$y) = 0|);

$x = $LIB->_new("1234");
$y = $LIB->_new("987654321012345678");
is($LIB->_acmp($x, $y), -1, qq|$LIB->_acmp(\$x, \$y) = -1|);
is($LIB->_acmp($y, $x), 1,  qq|$LIB->_acmp(\$y, \$x) = 1|);
is($LIB->_acmp($x, $x), 0,  qq|$LIB->_acmp(\$x, \$x) = 0|);
is($LIB->_acmp($y, $y), 0,  qq|$LIB->_acmp(\$y, \$y) = 0|);

# _modinv

$x = $LIB->_new("8");
$y = $LIB->_new("5033");
my ($xmod, $sign) = $LIB->_modinv($x, $y);
is($LIB->_str($xmod), "629",            # -629 % 5033 == 4404
   qq|$LIB->_str(\$xmod) = "629"|);
is($sign, "-", q|$sign = "-"|);

# _div

$x = $LIB->_new("3333");
$y = $LIB->_new("1111");
is($LIB->_str(scalar($LIB->_div($x, $y))), 3,
   qq|$LIB->_str(scalar($LIB->_div(\$x, \$y))) = 3|);

$x = $LIB->_new("33333");
$y = $LIB->_new("1111");
($x, $y) = $LIB->_div($x, $y);
is($LIB->_str($x), 30, qq|$LIB->_str(\$x) = 30|);
is($LIB->_str($y),  3, qq|$LIB->_str(\$y) = 3|);

$x = $LIB->_new("123");
$y = $LIB->_new("1111");
($x, $y) = $LIB->_div($x, $y);
is($LIB->_str($x), 0,   qq|$LIB->_str(\$x) = 0|);
is($LIB->_str($y), 123, qq|$LIB->_str(\$y) = 123|);

# _num

foreach (qw/1 12 123 1234 12345 1234567 12345678 123456789 1234567890/) {

    $x = $LIB->_new("$_");
    is(ref($x), $REF, q|ref($x) = "$REF"|);
    is($LIB->_str($x), "$_", qq|$LIB->_str(\$x) = "$_"|);

    $x = $LIB->_num($x);
    is(ref($x), "", q|ref($x) = ""|);
    is($x,      $_, qq|\$x = $_|);
}

# _sqrt

$x = $LIB->_new("144");
is($LIB->_str($LIB->_sqrt($x)), "12",
   qq|$LIB->_str($LIB->_sqrt(\$x)) = "12"|);
$x = $LIB->_new("144000000000000");
is($LIB->_str($LIB->_sqrt($x)), "12000000",
   qq|$LIB->_str($LIB->_sqrt(\$x)) = "12000000"|);

# _root

$x = $LIB->_new("81");
my $n = $LIB->_new("3");        # 4*4*4 = 64, 5*5*5 = 125
is($LIB->_str($LIB->_root($x, $n)), "4",
   qq|$LIB->_str($LIB->_root(\$x, \$n)) = "4"|); # 4.xx => 4.0

$x = $LIB->_new("81");
$n = $LIB->_new("4");          # 3*3*3*3 == 81
is($LIB->_str($LIB->_root($x, $n)), "3",
   qq|$LIB->_str($LIB->_root(\$x, \$n)) = "3"|);

# _pow (and _root)

$x = $LIB->_new("0");
$n = $LIB->_new("3");          # 0 ** y => 0
is($LIB->_str($LIB->_pow($x, $n)), 0,
   qq|$LIB->_str($LIB->_pow(\$x, \$n)) = 0|);

$x = $LIB->_new("3");
$n = $LIB->_new("0");          # x ** 0 => 1
is($LIB->_str($LIB->_pow($x, $n)), 1,
   qq|$LIB->_str($LIB->_pow(\$x, \$n)) = 1|);

$x = $LIB->_new("1");
$n = $LIB->_new("3");          # 1 ** y => 1
is($LIB->_str($LIB->_pow($x, $n)), 1,
   qq|$LIB->_str($LIB->_pow(\$x, \$n)) = 1|);

$x = $LIB->_new("5");
$n = $LIB->_new("1");          # x ** 1 => x
is($LIB->_str($LIB->_pow($x, $n)), 5,
   qq|$LIB->_str($LIB->_pow(\$x, \$n)) = 5|);

$x = $LIB->_new("81");
$n = $LIB->_new("3");          # 81 ** 3 == 531441
is($LIB->_str($LIB->_pow($x, $n)), 81 ** 3,
   qq|$LIB->_str($LIB->_pow(\$x, \$n)) = 81 ** 3|);

is($LIB->_str($LIB->_root($x, $n)), 81,
   qq|$LIB->_str($LIB->_root(\$x, \$n)) = 81|);

$x = $LIB->_new("81");
is($LIB->_str($LIB->_pow($x, $n)), 81 ** 3,
   qq|$LIB->_str($LIB->_pow(\$x, \$n)) = 81 ** 3|);
is($LIB->_str($LIB->_pow($x, $n)), "150094635296999121",      # 531441 ** 3
   qq|$LIB->_str($LIB->_pow(\$x, \$n)) = "150094635296999121"|);

is($LIB->_str($LIB->_root($x, $n)), "531441",
   qq|$LIB->_str($LIB->_root(\$x, \$n)) = "531441"|);
is($LIB->_str($LIB->_root($x, $n)), "81",
   qq|$LIB->_str($LIB->_root(\$x, \$n)) = "81"|);

$x = $LIB->_new("81");
$n = $LIB->_new("14");
is($LIB->_str($LIB->_pow($x, $n)), "523347633027360537213511521",
   qq|$LIB->_str($LIB->_pow(\$x, \$n)) = "523347633027360537213511521"|);
is($LIB->_str($LIB->_root($x, $n)), "81",
   qq|$LIB->_str($LIB->_root(\$x, \$n)) = "81"|);

$x = $LIB->_new("523347633027360537213511520");
is($LIB->_str($LIB->_root($x, $n)), "80",
   qq|$LIB->_str($LIB->_root(\$x, \$n)) = "80"|);

$x = $LIB->_new("523347633027360537213511522");
is($LIB->_str($LIB->_root($x, $n)), "81",
   qq|$LIB->_str($LIB->_root(\$x, \$n)) = "81"|);

my $res = [ qw/9 31 99 316 999 3162 9999 31622 99999/ ];

# 99 ** 2 = 9801, 999 ** 2 = 998001 etc

for my $i (2 .. 9) {
    $x = '9' x $i;
    $x = $LIB->_new($x);
    $n = $LIB->_new("2");
    my $rc = '9' x ($i-1). '8' . '0' x ($i - 1) . '1';
    print "# _pow( ", '9' x $i, ", 2) \n" unless
      is($LIB->_str($LIB->_pow($x, $n)), $rc,
         qq|$LIB->_str($LIB->_pow(\$x, \$n)) = $rc|);

  SKIP: {
        # If $i > $BASE_LEN, the test takes a really long time.
        skip "$i > $BASE_LEN", 2 unless $i <= $BASE_LEN;

        $x = '9' x $i;
        $x = $LIB->_new($x);
        $n = '9' x $i;
        $n = $LIB->_new($n);
        print "# _root( ", '9' x $i, ", ", 9 x $i, ") \n";
        print "# _root( ", '9' x $i, ", ", 9 x $i, ") \n"
          unless is($LIB->_str($LIB->_root($x, $n)), '1',
                    qq|$LIB->_str($LIB->_root(\$x, \$n)) = '1'|);

        $x = '9' x $i;
        $x = $LIB->_new($x);
        $n = $LIB->_new("2");
        print "# BASE_LEN $BASE_LEN _root( ", '9' x $i, ", ", 9 x $i, ") \n"
          unless is($LIB->_str($LIB->_root($x, $n)), $res->[$i-2],
                    qq|$LIB->_str($LIB->_root(\$x, \$n)) = $res->[$i-2]|);
    }
}

##############################################################################
# _fac

$x = $LIB->_new("0");
is($LIB->_str($LIB->_fac($x)), "1",
   qq|$LIB->_str($LIB->_fac(\$x)) = "1"|);

$x = $LIB->_new("1");
is($LIB->_str($LIB->_fac($x)), "1",
   qq|$LIB->_str($LIB->_fac(\$x)) = "1"|);

$x = $LIB->_new("2");
is($LIB->_str($LIB->_fac($x)), "2",
   qq|$LIB->_str($LIB->_fac(\$x)) = "2"|);

$x = $LIB->_new("3");
is($LIB->_str($LIB->_fac($x)), "6",
   qq|$LIB->_str($LIB->_fac(\$x)) = "6"|);

$x = $LIB->_new("4");
is($LIB->_str($LIB->_fac($x)), "24",
   qq|$LIB->_str($LIB->_fac(\$x)) = "24"|);

$x = $LIB->_new("5");
is($LIB->_str($LIB->_fac($x)), "120",
   qq|$LIB->_str($LIB->_fac(\$x)) = "120"|);

$x = $LIB->_new("10");
is($LIB->_str($LIB->_fac($x)), "3628800",
   qq|$LIB->_str($LIB->_fac(\$x)) = "3628800"|);

$x = $LIB->_new("11");
is($LIB->_str($LIB->_fac($x)), "39916800",
   qq|$LIB->_str($LIB->_fac(\$x)) = "39916800"|);

$x = $LIB->_new("12");
is($LIB->_str($LIB->_fac($x)), "479001600",
   qq|$LIB->_str($LIB->_fac(\$x)) = "479001600"|);

$x = $LIB->_new("13");
is($LIB->_str($LIB->_fac($x)), "6227020800",
   qq|$LIB->_str($LIB->_fac(\$x)) = "6227020800"|);

# test that _fac modifies $x in place for small arguments

$x = $LIB->_new("3");
$LIB->_fac($x);
is($LIB->_str($x), "6",
   qq|$LIB->_str(\$x) = "6"|);

$x = $LIB->_new("13");
$LIB->_fac($x);
is($LIB->_str($x), "6227020800",
   qq|$LIB->_str(\$x) = "6227020800"|);

# _inc and _dec

for (qw/1 11 121 1231 12341 1234561 12345671 123456781 1234567891/) {
    $x = $LIB->_new("$_");
    $LIB->_inc($x);
    my $expected = substr($_, 0, length($_) - 1) . '2';
    is($LIB->_str($x), $expected, qq|$LIB->_str(\$x) = $expected|);
    $LIB->_dec($x);
    is($LIB->_str($x), $_, qq|$LIB->_str(\$x) = $_|);
}

for (qw/19 119 1219 12319 1234519 12345619 123456719 1234567819/) {
    $x = $LIB->_new("$_");
    $LIB->_inc($x);
    my $expected = substr($_, 0, length($_)-2) . '20';
    is($LIB->_str($x), $expected, qq|$LIB->_str(\$x) = $expected|);
    $LIB->_dec($x);
    is($LIB->_str($x), $_, qq|$LIB->_str(\$x) = $_|);
}

for (1 .. 20) {
    my $p = "9" x $_;                       # = $q - 1
    my $q = "1" . ("0" x $_);               # = $p + 1

    $x = $LIB->_new("$p");
    $LIB->_inc($x);
    is($LIB->_str($x), $q, qq|\$x = $LIB->_new("$p"); $LIB->_inc()|);

    $x = $LIB->_new("$q");
    $LIB->_dec($x);
    is($LIB->_str($x), $p, qq|\$x = $LIB->_new("$q"); $LIB->_dec()|);
}

for (1 .. 20) {
    my $p = "1" . ("0" x $_);               # = $q - 1
    my $q = "1" . ("0" x ($_ - 1)) . "1";   # = $p + 1

    $x = $LIB->_new("$p");
    $LIB->_inc($x);
    is($LIB->_str($x), $q, qq|\$x = $LIB->_new("$p"); $LIB->_inc()|);

    $x = $LIB->_new("$q");
    $LIB->_dec($x);
    is($LIB->_str($x), $p, qq|\$x = $LIB->_new("$q"); $LIB->_dec()|);
}

$x = $LIB->_new("1000");
$LIB->_inc($x);
is($LIB->_str($x), "1001", qq|$LIB->_str(\$x) = "1001"|);
$LIB->_dec($x);
is($LIB->_str($x), "1000", qq|$LIB->_str(\$x) = "1000"|);

my $BL = $LIB -> _base_len();

$x = '1' . '0' x $BL;
$z = '1' . '0' x ($BL - 1);
$z .= '1';
$x = $LIB->_new($x);
$LIB->_inc($x);
is($LIB->_str($x), $z, qq|$LIB->_str(\$x) = $z|);

$x = '1' . '0' x $BL;
$z = '9' x $BL;
$x = $LIB->_new($x);
$LIB->_dec($x);
is($LIB->_str($x), $z, qq|$LIB->_str(\$x) = $z|);

# should not happen:
# $x = $LIB->_new("-2");
# $y = $LIB->_new("4");
# is($LIB->_acmp($x, $y), -1, qq|$LIB->_acmp($x, $y) = -1|);

###############################################################################
# _mod

$x = $LIB->_new("1000");
$y = $LIB->_new("3");
is($LIB->_str(scalar($LIB->_mod($x, $y))), 1,
   qq|$LIB->_str(scalar($LIB->_mod(\$x, \$y))) = 1|);

$x = $LIB->_new("1000");
$y = $LIB->_new("2");
is($LIB->_str(scalar($LIB->_mod($x, $y))), 0,
   qq|$LIB->_str(scalar($LIB->_mod(\$x, \$y))) = 0|);

# _and, _or, _xor

$x = $LIB->_new("5");
$y = $LIB->_new("2");
is($LIB->_str(scalar($LIB->_xor($x, $y))), 7,
   qq|$LIB->_str(scalar($LIB->_xor(\$x, \$y))) = 7|);

$x = $LIB->_new("5");
$y = $LIB->_new("2");
is($LIB->_str(scalar($LIB->_or($x, $y))), 7,
   qq|$LIB->_str(scalar($LIB->_or(\$x, \$y))) = 7|);

$x = $LIB->_new("5");
$y = $LIB->_new("3");
is($LIB->_str(scalar($LIB->_and($x, $y))), 1,
   qq|$LIB->_str(scalar($LIB->_and(\$x, \$y))) = 1|);

# _from_hex, _from_bin, _from_oct

is($LIB->_str($LIB->_from_hex("0xFf")), 255,
   qq|$LIB->_str($LIB->_from_hex("0xFf")) = 255|);
is($LIB->_str($LIB->_from_bin("0b10101011")), 160+11,
   qq|$LIB->_str($LIB->_from_bin("0b10101011")) = 160+11|);
is($LIB->_str($LIB->_from_oct("0100")), 8*8,
   qq|$LIB->_str($LIB->_from_oct("0100")) = 8*8|);
is($LIB->_str($LIB->_from_oct("01000")), 8*8*8,
   qq|$LIB->_str($LIB->_from_oct("01000")) = 8*8*8|);
is($LIB->_str($LIB->_from_oct("010001")), 8*8*8*8+1,
   qq|$LIB->_str($LIB->_from_oct("010001")) = 8*8*8*8+1|);
is($LIB->_str($LIB->_from_oct("010007")), 8*8*8*8+7,
   qq|$LIB->_str($LIB->_from_oct("010007")) = 8*8*8*8+7|);

# _as_hex, _as_bin, as_oct

is($LIB->_str($LIB->_from_hex($LIB->_as_hex($LIB->_new("128")))), 128,
   qq|$LIB->_str($LIB->_from_hex($LIB->_as_hex(|
   . qq|$LIB->_new("128")))) = 128|);
is($LIB->_str($LIB->_from_bin($LIB->_as_bin($LIB->_new("128")))), 128,
   qq|$LIB->_str($LIB->_from_bin($LIB->_as_bin(|
   . qq|$LIB->_new("128")))) = 128|);
is($LIB->_str($LIB->_from_oct($LIB->_as_oct($LIB->_new("128")))), 128,
   qq|$LIB->_str($LIB->_from_oct($LIB->_as_oct(|
   . qq|$LIB->_new("128")))) = 128|);

is($LIB->_str($LIB->_from_oct($LIB->_as_oct($LIB->_new("123456")))),
   123456,
   qq|$LIB->_str($LIB->_from_oct($LIB->_as_oct|
   . qq|($LIB->_new("123456")))) = 123456|);
is($LIB->_str($LIB->_from_oct($LIB->_as_oct($LIB->_new("123456789")))),
   "123456789",
   qq|$LIB->_str($LIB->_from_oct($LIB->_as_oct(|
   . qq|$LIB->_new("123456789")))) = "123456789"|);
is($LIB->_str($LIB->_from_oct($LIB->_as_oct($LIB->_new("1234567890123")))),
   "1234567890123",
   qq|$LIB->_str($LIB->_from_oct($LIB->_as_oct(|
   . qq|$LIB->_new("1234567890123")))) = "1234567890123"|);

my $long = "123456789012345678901234567890";
is($LIB->_str($LIB->_from_hex($LIB->_as_hex($LIB->_new($long)))), $long,
   qq|$LIB->_str($LIB->_from_hex($LIB->_as_hex(|
   . qq|$LIB->_new("$long")))) = "$long"|);
is($LIB->_str($LIB->_from_bin($LIB->_as_bin($LIB->_new($long)))), $long,
   qq|$LIB->_str($LIB->_from_bin($LIB->_as_bin(|
   . qq|$LIB->_new("$long")))) = "$long"|);
is($LIB->_str($LIB->_from_oct($LIB->_as_oct($LIB->_new($long)))), $long,
   qq|$LIB->_str($LIB->_from_oct($LIB->_as_oct(|
   . qq|$LIB->_new("$long")))) = "$long"|);

is($LIB->_str($LIB->_from_hex($LIB->_as_hex($LIB->_new("0")))), 0,
   qq|$LIB->_str($LIB->_from_hex($LIB->_as_hex(|
   . qq|$LIB->_new("0")))) = 0|);
is($LIB->_str($LIB->_from_bin($LIB->_as_bin($LIB->_new("0")))), 0,
   qq|$LIB->_str($LIB->_from_bin($LIB->_as_bin(|
   . qq|$LIB->_new("0")))) = 0|);
is($LIB->_str($LIB->_from_oct($LIB->_as_oct($LIB->_new("0")))), 0,
   qq|$LIB->_str($LIB->_from_oct($LIB->_as_oct(|
   . qq|$LIB->_new("0")))) = 0|);

is($LIB->_as_hex($LIB->_new("0")), "0x0",
   qq|$LIB->_as_hex($LIB->_new("0")) = "0x0"|);
is($LIB->_as_bin($LIB->_new("0")), "0b0",
   qq|$LIB->_as_bin($LIB->_new("0")) = "0b0"|);
is($LIB->_as_oct($LIB->_new("0")), "00",
   qq|$LIB->_as_oct($LIB->_new("0")) = "00"|);

is($LIB->_as_hex($LIB->_new("12")), "0xc",
   qq|$LIB->_as_hex($LIB->_new("12")) = "0xc"|);
is($LIB->_as_bin($LIB->_new("12")), "0b1100",
   qq|$LIB->_as_bin($LIB->_new("12")) = "0b1100"|);
is($LIB->_as_oct($LIB->_new("64")), "0100",
   qq|$LIB->_as_oct($LIB->_new("64")) = "0100"|);

# _1ex

is($LIB->_str($LIB->_1ex(0)), "1",
   qq|$LIB->_str($LIB->_1ex(0)) = "1"|);
is($LIB->_str($LIB->_1ex(1)), "10",
   qq|$LIB->_str($LIB->_1ex(1)) = "10"|);
is($LIB->_str($LIB->_1ex(2)), "100",
   qq|$LIB->_str($LIB->_1ex(2)) = "100"|);
is($LIB->_str($LIB->_1ex(12)), "1000000000000",
   qq|$LIB->_str($LIB->_1ex(12)) = "1000000000000"|);
is($LIB->_str($LIB->_1ex(16)), "10000000000000000",
   qq|$LIB->_str($LIB->_1ex(16)) = "10000000000000000"|);

# _check

$x = $LIB->_new("123456789");
is($LIB->_check($x), 0,
   qq|$LIB->_check(\$x) = 0|);
is($LIB->_check(123), "123 is not a reference",
   qq|$LIB->_check(123) = "123 is not a reference"|);

###############################################################################
# __strip_zeros

{
    no strict 'refs';

    # correct empty arrays
    $x = &{$LIB."::__strip_zeros"}([]);
    is(@$x, 1, q|@$x = 1|);
    is($x->[0], 0, q|$x->[0] = 0|);

    # don't strip single elements
    $x = &{$LIB."::__strip_zeros"}([0]);
    is(@$x, 1, q|@$x = 1|);
    is($x->[0], 0, q|$x->[0] = 0|);
    $x = &{$LIB."::__strip_zeros"}([1]);
    is(@$x, 1, q|@$x = 1|);
    is($x->[0], 1, q|$x->[0] = 1|);

    # don't strip non-zero elements
    $x = &{$LIB."::__strip_zeros"}([0, 1]);
    is(@$x, 2, q|@$x = 2|);
    is($x->[0], 0, q|$x->[0] = 0|);
    is($x->[1], 1, q|$x->[1] = 1|);
    $x = &{$LIB."::__strip_zeros"}([0, 1, 2]);
    is(@$x, 3, q|@$x = 3|);
    is($x->[0], 0, q|$x->[0] = 0|);
    is($x->[1], 1, q|$x->[1] = 1|);
    is($x->[2], 2, q|$x->[2] = 2|);

    # but strip leading zeros
    $x = &{$LIB."::__strip_zeros"}([0, 1, 2, 0]);
    is(@$x, 3, q|@$x = 3|);
    is($x->[0], 0, q|$x->[0] = 0|);
    is($x->[1], 1, q|$x->[1] = 1|);
    is($x->[2], 2, q|$x->[2] = 2|);

    $x = &{$LIB."::__strip_zeros"}([0, 1, 2, 0, 0]);
    is(@$x, 3, q|@$x = 3|);
    is($x->[0], 0, q|$x->[0] = 0|);
    is($x->[1], 1, q|$x->[1] = 1|);
    is($x->[2], 2, q|$x->[2] = 2|);

    $x = &{$LIB."::__strip_zeros"}([0, 1, 2, 0, 0, 0]);
    is(@$x, 3, q|@$x = 3|);
    is($x->[0], 0, q|$x->[0] = 0|);
    is($x->[1], 1, q|$x->[1] = 1|);
    is($x->[2], 2, q|$x->[2] = 2|);

    # collapse multiple zeros
    $x = &{$LIB."::__strip_zeros"}([0, 0, 0, 0]);
    is(@$x, 1, q|@$x = 1|);
    is($x->[0], 0, q|$x->[0] = 0|);
}
