# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 6;

###############################################################################
package Math::BigFloat::Test;

use Math::BigFloat;
require Exporter;
our @ISA = qw/Math::BigFloat Exporter/;

use overload;

sub isa {
    my ($self, $class) = @_;
    return if $class =~ /^Math::Big(Int|Float$)/;   # we aren't one of these
    UNIVERSAL::isa($self, $class);
}

sub bmul {
    return __PACKAGE__->new(123);
}

sub badd {
    return __PACKAGE__->new(321);
}

###############################################################################
package main;

# use Math::BigInt upgrade => 'Math::BigFloat';
use Math::BigFloat upgrade => 'Math::BigFloat::Test';

my ($x, $y, $z);

our ($CLASS, $EXPECTED_CLASS, $LIB);
$CLASS          = "Math::BigFloat";
$EXPECTED_CLASS = "Math::BigFloat::Test";
$LIB            = "Math::BigInt::Calc";         # backend

is(Math::BigFloat->upgrade(), $EXPECTED_CLASS,
   qq|Math::BigFloat->upgrade()|);
is(Math::BigFloat->downgrade(), undef,
   qq|Math::BigFloat->downgrade()|);

$x = $CLASS->new(123);
$y = $EXPECTED_CLASS->new(123);
$z = $x->bmul($y);
is(ref($z), $EXPECTED_CLASS,
   qq|\$x = $CLASS->new(123); \$y = $EXPECTED_CLASS->new(123);|
   . q| $z = $x->bmul($y); ref($z)|);
is($z, 123,
   qq|\$x = $CLASS->new(123); \$y = $EXPECTED_CLASS->new(123);|
   . q| $z = $x->bmul($y); $z|);

$x = $CLASS->new(123);
$y = $EXPECTED_CLASS->new(123);
$z = $x->badd($y);
is(ref($z), $EXPECTED_CLASS,
   qq|$x = $CLASS->new(123); $y = $EXPECTED_CLASS->new(123);|
   . q| $z = $x->badd($y); ref($z)|);
is($z, 321,
   qq|$x = $CLASS->new(123); $y = $EXPECTED_CLASS->new(123);|
   . q| $z = $x->badd($y); $z|);

# not yet:
#require './t/upgrade.inc';     # all tests here for sharing
