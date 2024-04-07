# -*- mode: perl; -*-

# for testing subclassing Math::BigFloat

package Math::BigFloat::Subclass;

require 5.006;

use strict;
use warnings;

use Exporter;
use Math::BigFloat;

our @ISA = qw(Math::BigFloat Exporter);

our $VERSION = "0.08";

use overload;                   # inherit overload from BigInt

# Globals
our $accuracy   = undef;
our $precision  = undef;
our $round_mode = Math::BigFloat::Subclass -> round_mode();
our $div_scale  = Math::BigFloat::Subclass -> div_scale();
our $lib = '';

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;

    my $value = shift;
    my $a = $accuracy;  $a = $_[0] if defined $_[0];
    my $p = $precision; $p = $_[1] if defined $_[1];
    # Store the floating point value
    my $self = Math::BigFloat->new($value, $a, $p, $round_mode);
    bless $self, $class;
    $self->{'_custom'} = 1;     # make sure this never goes away
    return $self;
}

BEGIN {
    *objectify = \&Math::BigInt::objectify;
    # to allow Math::BigFloat::Subclass::bgcd( ... ) style calls
    *bgcd = \&Math::BigFloat::bgcd;
    *blcm = \&Math::BigFloat::blcm;
}

1;
