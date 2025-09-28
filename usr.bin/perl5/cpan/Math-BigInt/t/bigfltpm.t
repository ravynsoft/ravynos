# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 3070            # tests in require'd file
                         + 19;          # tests in this file

use Math::BigInt only => 'Calc';
use Math::BigFloat;

our ($CLASS, $LIB);
$CLASS = "Math::BigFloat";
$LIB   = Math::BigInt -> config('lib');         # backend library

is($CLASS->config("class"), $CLASS, qq|$CLASS->config("class")|);
is($CLASS->config("with"),  $LIB,   qq|$CLASS->config("with")|);

# bug #17447: Can't call method Math::BigFloat->bsub, not a valid method
my $c = Math::BigFloat->new('123.3');
is($c->bsub(123), '0.3',
   qq|\$c = Math::BigFloat -> new("123.3"); \$y = \$c -> bsub("123")|);

# Bug until Math::BigInt v1.86, the scale wasn't treated as a scalar:
$c = Math::BigFloat->new('0.008');
my $d = Math::BigFloat->new(3);
my $e = $c->bdiv(Math::BigFloat->new(3), $d);

is($e, '0.00267', '0.008 / 3 = 0.0027');

my $x;

#############################################################################
# bgcd() as function, class method and instance method.

my $gcd0 = Math::BigFloat::bgcd(-12, 18, 27);
isa_ok($gcd0, "Math::BigFloat", "bgcd() as function");
is($gcd0, 3, "bgcd() as function");

my $gcd1 = Math::BigFloat->bgcd(-12, 18, 27);
isa_ok($gcd1, "Math::BigFloat", "bgcd() as class method");
is($gcd1, 3, "bgcd() as class method");

$x = Math::BigFloat -> new(-12);
my $gcd2 = $x -> bgcd(18, 27);
isa_ok($gcd2, "Math::BigFloat", "bgcd() as instance method");
is($gcd2, 3, "bgcd() as instance method");
is($x, -12, "bgcd() does not modify invocand");

#############################################################################
# blcm() as function, class method and instance method.

my $lcm0 = Math::BigFloat::blcm(-12, 18, 27);
isa_ok($lcm0, "Math::BigFloat", "blcm() as function");
is($lcm0, 108, "blcm() as function");

my $lcm1 = Math::BigFloat->blcm(-12, 18, 27);
isa_ok($lcm1, "Math::BigFloat", "blcm() as class method");
is($lcm1, 108, "blcm() as class method");

$x = Math::BigFloat -> new(-12);
my $lcm2 = $x -> blcm(18, 27);
isa_ok($lcm2, "Math::BigFloat", "blcm() as instance method");
is($lcm2, 108, "blcm() as instance method");
is($x, -12, "blcm() does not modify invocand");

#############################################################################

SKIP: {
    skip("skipping test which is not for this backend", 1)
      unless $LIB eq 'Math::BigInt::Calc';
    is(ref($e->{_e}->[0]), '', '$e->{_e}->[0] is a scalar');
}

require './t/bigfltpm.inc';     # all tests here for sharing
