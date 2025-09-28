# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 4278            # tests in require'd file
                         + 20;          # tests in this file

use Math::BigInt only => 'Calc';

our ($CLASS, $LIB);
$CLASS = "Math::BigInt";
$LIB   = Math::BigInt -> config('lib');         # backend library

my $x;

#############################################################################
# bgcd() as function, class method and instance method.

my $gcd0 = Math::BigInt::bgcd(-12, 18, 27);
isa_ok($gcd0, "Math::BigInt", "bgcd() as function");
is($gcd0, 3, "bgcd() as function");

my $gcd1 = Math::BigInt->bgcd(-12, 18, 27);
isa_ok($gcd1, "Math::BigInt", "bgcd() as class method");
is($gcd1, 3, "bgcd() as class method");

$x = Math::BigInt -> new(-12);
my $gcd2 = $x -> bgcd(18, 27);
isa_ok($gcd2, "Math::BigInt", "bgcd() as instance method");
is($gcd2, 3, "bgcd() as instance method");
is($x, -12, "bgcd() does not modify invocand");

#############################################################################
# blcm() as function, class method and instance method.

my $lcm0 = Math::BigInt::blcm(-12, 18, 27);
isa_ok($lcm0, "Math::BigInt", "blcm() as function");
is($lcm0, 108, "blcm() as function");

my $lcm1 = Math::BigInt->blcm(-12, 18, 27);
isa_ok($lcm1, "Math::BigInt", "blcm() as class method");
is($lcm1, 108, "blcm() as class method");

$x = Math::BigInt -> new(-12);
my $lcm2 = $x -> blcm(18, 27);
isa_ok($lcm2, "Math::BigInt", "blcm() as instance method");
is($lcm2, 108, "blcm() as instance method");
is($x, -12, "blcm() does not modify invocand");

#############################################################################
# from_hex(), from_bin() and from_oct() tests

$x = Math::BigInt->from_hex('0xcafe');
is($x, "51966",
   qq|Math::BigInt->from_hex("0xcafe")|);

$x = Math::BigInt->from_hex('0xcafebabedead');
is($x, "223195403574957",
   qq|Math::BigInt->from_hex("0xcafebabedead")|);

$x = Math::BigInt->from_bin('0b1001');
is($x, "9",
   qq|Math::BigInt->from_bin("0b1001")|);

$x = Math::BigInt->from_bin('0b1001100110011001100110011001');
is($x, "161061273",
   qq|Math::BigInt->from_bin("0b1001100110011001100110011001");|);

$x = Math::BigInt->from_oct('0775');
is($x, "509",
   qq|Math::BigInt->from_oct("0775");|);

$x = Math::BigInt->from_oct('07777777777777711111111222222222');
is($x, "9903520314281112085086151826",
   qq|Math::BigInt->from_oct("07777777777777711111111222222222");|);

#############################################################################
# all the other tests

require './t/bigintpm.inc';       # all tests here for sharing
