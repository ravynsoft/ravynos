# -*- mode: perl; -*-
#
# Verify that
#   - Math::BigInt::objectify() calls as_int() (or as_number(), as a fallback)
#     if the target object class is Math::BigInt.
#   - Math::BigInt::objectify() calls as_float() if the target object class is
#     Math::BigFloat.
#
# See RT #16221 and RT #52124.

use strict;
use warnings;

package main;

use Test::More tests => 2;
use Math::BigInt;
use Math::BigFloat;

############################################################################

my $int = Math::BigInt->new(10);
my $int_percent = My::Percent::Float->new(100);

is($int * $int_percent, 10, '$int * $int_percent = 10');

############################################################################

my $float = Math::BigFloat->new(10);
my $float_percent = My::Percent::Float->new(100);

is($float * $float_percent, 10, '$float * $float_percent = 10');

############################################################################

package My::Percent::Int;

sub new {
    my $class = shift;
    my $num = shift;
    return bless \$num, $class;
}

sub as_number {
    my $self = shift;
    return Math::BigInt->new($$self / 100);
}

sub as_string {
    my $self = shift;
    return $$self;
}

############################################################################

package My::Percent::Float;

sub new {
    my $class = shift;
    my $num = shift;
    return bless \$num, $class;
}

sub as_int {
    my $self = shift;
    return Math::BigInt->new($$self / 100);
}

sub as_float {
    my $self = shift;
    return Math::BigFloat->new($$self / 100);
}

sub as_string {
    my $self = shift;
    return $$self;
}
