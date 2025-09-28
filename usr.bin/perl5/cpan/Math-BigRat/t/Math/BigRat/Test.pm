package Math::BigRat::Test;

require 5.006;
use strict;
use warnings;

use Exporter;
use Math::BigRat;
use Math::BigFloat;

our @ISA = qw(Math::BigRat Exporter);
our $VERSION = '0.04';

use overload;           # inherit overload from BigRat

# Globals
our $accuracy   = undef;
our $precision  = undef;
our $round_mode = 'even';
our $div_scale  = 40;

my $class = 'Math::BigRat::Test';

#sub new {
#    my $proto  = shift;
#    my $class  = ref($proto) || $proto;
#
#    my $value       = shift;
#    my $a = $accuracy; $a = $_[0] if defined $_[0];
#    my $p = $precision; $p = $_[1] if defined $_[1];
#    # Store the floating point value
#    my $self = Math::BigFloat->new($value, $a, $p, $round_mode);
#    bless $self, $class;
#    $self->{'_custom'} = 1;     # make sure this never goes away
#    return $self;
#}

BEGIN {
    *fstr  = \&bstr;
    *fsstr = \&bsstr;
    *objectify = \&Math::BigInt::objectify;
    *AUTOLOAD  = \&Math::BigRat::AUTOLOAD;
    no strict 'refs';
    foreach my $method (qw/div acmp floor ceil root sqrt log fac modpow modinv/) {
        *{'b' . $method} = \&{'Math::BigRat::b' . $method};
    }
}

sub fround {
    my ($x, $a) = @_;

    #print "$a $accuracy $precision $round_mode\n";
    Math::BigFloat->round_mode($round_mode);
    Math::BigFloat->accuracy($a || $accuracy);
    Math::BigFloat->precision(undef);
    my $y = Math::BigFloat->new($x->bsstr(), undef, undef);
    $class->new($y->fround($a));
}

sub ffround {
    my ($x, $p) = @_;

    Math::BigFloat->round_mode($round_mode);
    Math::BigFloat->accuracy(undef);
    Math::BigFloat->precision($p || $precision);
    my $y = Math::BigFloat->new($x->bsstr(), undef, undef);
    $class->new($y->ffround($p));
}

sub bstr {
    # calculate a BigFloat compatible string output
    my ($x) = @_;

    $x = $class->new($x) unless ref $x;

    if ($x->{sign} !~ /^[+-]$/) { # inf, NaN etc
        my $s = $x->{sign};
        $s =~ s/^\+//;          # +inf => inf
        return $s;
    }

    my $s = '';
    $s = $x->{sign} if $x->{sign} ne '+'; # +3 vs 3

    #  print " bstr \$x ", $accuracy || $x->{_a} || 'notset', " ", $precision || $x->{_p} || 'notset', "\n";
    return $s.$x->{_n} if $x->{_d}->is_one();
    my $output = Math::BigFloat->new($x->{_n})->bdiv($x->{_d});
    local $Math::BigFloat::accuracy  = $accuracy  || $x->{_a};
    local $Math::BigFloat::precision = $precision || $x->{_p};
    $s.$output->bstr();
}

sub numify {
    $_[0]->bsstr();
}

sub bsstr {
    # calculate a BigFloat compatible string output
    my ($x) = @_;

    $x = $class->new($x) unless ref $x;

    if ($x->{sign} !~ /^[+-]$/) {       # inf, NaN etc
        my $s = $x->{sign};
        $s =~ s/^\+//;                  # +inf => inf
        return $s;
    }

    my $s = '';
    $s = $x->{sign} if $x->{sign} ne '+'; # +3 vs 3

    my $output = Math::BigFloat->new($x->{_n})->bdiv($x->{_d});
    return $s.$output->bsstr();
}

1;
