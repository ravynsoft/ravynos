# -*- mode: perl; -*-

package Math::BigInt::Trace;

use strict;
use warnings;

use Exporter;
use Math::BigInt;

our @ISA = qw(Exporter Math::BigInt);

our $VERSION = '0.66';

use overload;                   # inherit overload from Math::BigInt

# Globals
our $accuracy   = undef;
our $precision  = undef;
our $round_mode = 'even';
our $div_scale  = 40;

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;

    my $value = shift;

    my $a = $accuracy;
    $a = $_[0] if defined $_[0];

    my $p = $precision;
    $p = $_[1] if defined $_[1];

    my $self = $class -> SUPER::new($value, $a, $p, $round_mode);

    printf "Math::BigInt new '%s' => '%s' (%s)\n",
      $value, $self, ref($self);

    return $self;
}

sub import {
    my $class = shift;

    printf "%s -> import(%s)\n", $class, join(", ", @_);

    # we catch the constants, the rest goes to parent

    my $constant = grep { $_ eq ':constant' } @_;
    my @a = grep { $_ ne ':constant' } @_;

    if ($constant) {
        overload::constant

            integer => sub {
                $class -> new(shift);
            },

            float   => sub {
                $class -> new(shift);
            },

            binary  => sub {
                # E.g., a literal 0377 shall result in an object whose value
                # is decimal 255, but new("0377") returns decimal 377.
                return $class -> from_oct($_[0]) if $_[0] =~ /^0_*[0-7]/;
                $class -> new(shift);
            };
    }

    $class -> SUPER::import(@a);                # need it for subclasses
    #$self -> export_to_level(1, $class, @_);    # need this ?
}

1;
