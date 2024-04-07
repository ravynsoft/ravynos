# -*- mode: perl; -*-

# test calling conventions, and :constant overloading

use strict;
use warnings;

use Test::More tests => 156;

##############################################################################

package Math::BigInt::Test;

use Math::BigInt;
our @ISA = qw/Math::BigInt/;            # subclass of MBI
use overload;

##############################################################################

package Math::BigFloat::Test;

use Math::BigFloat;
our @ISA = qw/Math::BigFloat/;          # subclass of MBI
use overload;

##############################################################################

package main;

use Math::BigInt try => 'Calc';
use Math::BigFloat;

my ($x, $y, $z, $u);

###############################################################################
# check whether op's accept normal strings, even when inherited by subclasses

# do one positive and one negative test to avoid false positives by "accident"

my ($method, $expected);
while (<DATA>) {
    s/#.*$//;                   # remove comments
    s/\s+$//;                   # remove trailing whitespace
    next unless length;         # skip empty lines

    if (s/^&//) {
        $method = $_;
        next;
    }

    my @args = split /:/, $_, 99;
    $expected = pop @args;
    foreach my $class (qw/
                             Math::BigInt       Math::BigFloat
                             Math::BigInt::Test Math::BigFloat::Test
                         /)
    {
        my $arg = $args[0] =~ /"/ || $args[0] eq "" ? $args[0]
                                                    : qq|"$args[0]"|;
        my $try = "$class -> new($arg) -> $method();";
        my $got = eval $try;
        is($got, $expected, $try);
    }
}

__END__
&is_zero
1:0
0:1
&is_one
1:1
0:0
&is_positive
1:1
-1:0
&is_negative
1:0
-1:1
&is_non_positive
1:0
-1:1
&is_non_negative
1:1
-1:0
&is_nan
abc:1
1:0
&is_inf
inf:1
0:0
&bstr
5:5
10:10
-10:-10
abc:NaN
"+inf":inf
"-inf":-inf
&bsstr
1:1e+0
0:0e+0
2:2e+0
200:2e+2
-5:-5e+0
-100:-1e+2
abc:NaN
"+inf":inf
&babs
-1:1
1:1
&bnot
-2:1
1:-2
&bzero
:0
&bnan
:NaN
abc:NaN
&bone
:1
#"+":1
#"-":-1
&binf
:inf
#"+":inf
#"-":-inf
