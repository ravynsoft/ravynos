# -*- mode: perl; -*-

package Math::BigInt::Lib::TestUtil;

use strict;
use warnings;

use Exporter;

our @ISA       = qw< Exporter >;
our @EXPORT_OK = qw< randstr >;

# randstr NUM, BASE
#
# Generate a string representing a NUM digit number in base BASE.

sub randstr {
    die "randstr: wrong number of input arguments\n"
      unless @_ == 2;

    my $n = shift;
    my $b = shift;

    die "randstr: first input argument must be >= 0"
      unless $n >= 0;
    die "randstr: second input argument must be in the range 2 .. 36\n"
      unless 2 <= $b && $b <= 36;

    return '' if $n == 0;

    my @dig = (0 .. 9, 'a' .. 'z');

    my $str = $dig[ 1 + int rand ($b - 1) ];
    $str .= $dig[ int rand $b ] for 2 .. $n;

    return $str;
}

1;
