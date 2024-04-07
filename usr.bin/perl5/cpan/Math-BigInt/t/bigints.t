# -*- mode: perl; -*-

use strict;
use warnings;
use lib 't';

use Test::More tests => 50;

# testing of Math::BigInt:Scalar (used by the testsuite),
# primarily for interface/api and not for the math functionality

use Math::BigInt::Scalar;

my $class = 'Math::BigInt::Scalar'; # pass classname to sub's

# _new and _str

my $x = $class->_new("123");
my $y = $class->_new("321");
is(ref($x), 'SCALAR', 'ref($x)');
is($class->_str($x), 123, "$class->_str(\$x)");
is($class->_str($y), 321, "$class->_str(\$y)");

# _add, _sub, _mul, _div

is($class->_str($class->_add($x, $y)), 444,
   "$class->_str($class->_add(\$x, \$y)");
is($class->_str($class->_sub($x, $y)), 123,
   "$class->_str($class->_sub(\$x, \$y)");
is($class->_str($class->_mul($x, $y)), 39483,
   "$class->_str($class->_mul(\$x, \$y))");
is($class->_str($class->_div($x, $y)), 123,
   "$class->_str($class->_div(\$x, \$y)");

$class->_mul($x, $y);
is($class->_str($x), 39483, "$class->_str(\$x)");
is($class->_str($y),   321, "$class->_str(\$y)");

my $z = $class->_new("2");
is($class->_str($class->_add($x, $z)), 39485,
   "$class->_str($class->_add(\$x, \$z)");

my ($re, $rr) = $class->_div($x, $y);
is($class->_str($re), 123, "$class->_str(\$re)");
is($class->_str($rr),   2, "$class->_str(\$rr)");

# is_zero, _is_one, _one, _zero

is($class->_is_zero($x), 0, "$class->_is_zero($x)");
is($class->_is_one($x),  0, "$class->_is_one($x)");

is($class->_is_one($class->_one()), 1,
   "$class->_is_one($class->_one())");
is($class->_is_one($class->_zero()), 0,
   "$class->_is_one($class->_zero())");
is($class->_is_zero($class->_zero()), 1,
   "$class->_is_zero($class->_zero())");
is($class->_is_zero($class->_one()), 0,
   "$class->_is_zero($class->_one())");

# is_odd, is_even

is($class->_is_odd($class->_one()), 1,
   "$class->_is_odd($class->_one())");
is($class->_is_odd($class->_zero()), 0,
   "$class->_is_odd($class->_zero())");
is($class->_is_even($class->_one()), 0,
   "$class->_is_even($class->_one())");
is($class->_is_even($class->_zero()), 1,
   "$class->_is_even($class->_zero())");

# _digit

$x = $class->_new("123456789");
is($class->_digit($x,  0), 9, "$class->_digit(\$x, 0)");
is($class->_digit($x,  1), 8, "$class->_digit(\$x, 1)");
is($class->_digit($x,  2), 7, "$class->_digit(\$x, 2)");
is($class->_digit($x, -1), 1, "$class->_digit(\$x, -1)");
is($class->_digit($x, -2), 2, "$class->_digit(\$x, -2)");
is($class->_digit($x, -3), 3, "$class->_digit(\$x, -3)");

# _copy

$x = $class->_new("12356");
is($class->_str($class->_copy($x)), 12356,
   "$class->_str($class->_copy(\$x))");

# _acmp

$x = $class->_new("123456789");
$y = $class->_new("987654321");
is($class->_acmp($x, $y), -1, "$class->_acmp(\$x, \$y)");
is($class->_acmp($y, $x),  1, "$class->_acmp(\$y, \$x)");
is($class->_acmp($x, $x),  0, "$class->_acmp(\$x, \$x)");
is($class->_acmp($y, $y),  0, "$class->_acmp(\$y, \$y)");

# _div

$x = $class->_new("3333");
$y = $class->_new("1111");
is($class->_str(scalar $class->_div($x, $y)), 3,
   "$class->_str(scalar $class->_div(\$x, \$y))");

$x = $class->_new("33333");
$y = $class->_new("1111");
($x, $y) = $class->_div($x, $y);
is($class->_str($x), 30, "$class->_str(\$x)");
is($class->_str($y),  3, "$class->_str(\$y)");

$x = $class->_new("123");
$y = $class->_new("1111");
($x, $y) = $class->_div($x, $y);
is($class->_str($x),   0, "$class->_str(\$x)");
is($class->_str($y), 123, "$class->_str(\$y)");

# _num

$x = $class->_new("12345");
$x = $class->_num($x);
is(ref($x) || '', '', 'ref($x) || ""');
is($x, 12345, '$x');

# _len

$x = $class->_new("12345");
$x = $class->_len($x);
is(ref($x) || '', '', 'ref($x) || ""');
is($x, 5, '$x');

# _and, _or, _xor

$x = $class->_new("3");
$y = $class->_new("4");
is($class->_str($class->_or($x, $y)), 7,
   "$class->_str($class->_or($x, $y))");

$x = $class->_new("1");
$y = $class->_new("4");
is($class->_str($class->_xor($x, $y)), 5,
   "$class->_str($class->_xor($x, $y))");

$x = $class->_new("7");
$y = $class->_new("3");
is($class->_str($class->_and($x, $y)), 3,
   "$class->_str($class->_and($x, $y))");

# _pow

$x = $class->_new("2");
$y = $class->_new("4");
is($class->_str($class->_pow($x, $y)), 16,
   "$class->_str($class->_pow($x, $y))");

$x = $class->_new("2");
$y = $class->_new("5");
is($class->_str($class->_pow($x, $y)), 32,
   "$class->_str($class->_pow($x, $y))");

$x = $class->_new("3");
$y = $class->_new("3");
is($class->_str($class->_pow($x, $y)), 27,
   "$class->_str($class->_pow($x, $y))");

# _check

$x = $class->_new("123456789");
is($class->_check($x), 0,
   "$class->_check(\$x)");
is($class->_check(123), '123 is not a reference',
   "$class->_check(123)");
