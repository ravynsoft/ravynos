# -*- mode: perl; -*-

package Math::BigInt::Subclass;

require 5.005_02;

use strict;
use warnings;

use Exporter;
use Math::BigInt;

our @ISA = qw(Math::BigInt Exporter);
our @EXPORT_OK = qw(bgcd objectify);

our $VERSION = "0.07";

use overload;                   # inherit overload from BigInt

# Globals
our $accuracy   = undef;
our $precision  = undef;
our $round_mode = Math::BigInt::Subclass -> round_mode();
our $div_scale  = Math::BigInt::Subclass -> div_scale();
our $lib = '';

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;

    my $value = shift;
    my $a = $accuracy;  $a = $_[0] if defined $_[0];
    my $p = $precision; $p = $_[1] if defined $_[1];
    my $self = Math::BigInt->new($value, $a, $p, $round_mode);
    bless $self, $class;
    $self->{'_custom'} = 1;     # make sure this never goes away
    return $self;
}

sub bgcd {
    Math::BigInt::bgcd(@_);
}

sub blcm {
    Math::BigInt::blcm(@_);
}

sub as_int {
    Math::BigInt->new($_[0]);
}

BEGIN {
    *objectify = \&Math::BigInt::objectify;

    # these are called by AUTOLOAD from BigFloat, so we need at least these.
    # We cheat, of course..
    *bneg = \&Math::BigInt::bneg;
    *babs = \&Math::BigInt::babs;
    *bnan = \&Math::BigInt::bnan;
    *binf = \&Math::BigInt::binf;
    *bzero = \&Math::BigInt::bzero;
    *bone = \&Math::BigInt::bone;
}

sub import {
    my $self = shift;

    my @a;
    my $t = 0;
    foreach (@_) {
        # remove the "lib => foo" parameters and store it
        if ($t == 1) {
            $lib = $_;
            $t = 0;
            next;
        }
        if ($_ eq 'lib') {
            $t = 1;
            next;
        }
        push @a, $_;
    }
    $self->SUPER::import(@a);             # need it for subclasses
    $self->export_to_level(1, $self, @a); # need this ?
}

1;
