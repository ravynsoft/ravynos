# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 173;

my $class;

BEGIN {
    $class = 'Math::BigRat';
    use_ok($class);
}

while (<DATA>) {
    s/#.*$//;                   # remove comments
    s/\s+$//;                   # remove trailing whitespace
    next unless length;         # skip empty lines

    my ($xval, $yval, $zval) = split /:/;
    my ($x, $y, $got, @got);

    for my $context_is_scalar (0, 1) {
        for my $y_is_scalar (0, 1) {

            my $test = qq|\$x = $class -> new("$xval");|;

            $test .= $y_is_scalar
                     ? qq| \$y = "$yval";|
                     : qq| \$y = $class -> new("$yval");|;

            $test .= $context_is_scalar
                     ? qq| \$got = \$x -> badd(\$y);|
                     : qq| \@got = \$x -> badd(\$y);|;

            my $desc = "badd() in ";
            $desc .= $context_is_scalar ? "scalar context" : "list context";
            $desc .= $y_is_scalar ? " with y as scalar" : " with y as object";

            subtest $desc,
              sub {
                  plan tests => $context_is_scalar ? 7 : 8;

                  eval $test;
                  is($@, "", "'$test' gives emtpy \$\@");

                  if ($context_is_scalar) {

                      # Check output.

                      is(ref($got), $class,
                         "'$test' output arg is a $class");

                      is($got -> bstr(), $zval,
                         "'$test' output arg has the right value");

                  } else {

                      # Check number of output arguments.

                      cmp_ok(scalar @got, '==', 1,
                             "'$test' gives one output arg");

                      # Check output.

                      is(ref($got[0]), $class,
                         "'$test' output arg is a $class");

                      is($got[0] -> bstr(), $zval,
                         "'$test' output arg has the right value");
                  }

                  # Check the invocand.

                  is(ref($x), $class,
                     "'$test' invocand is still a $class");

                  is($x -> bstr(), $zval,
                     "'$test' invocand has the right value");

                  # Check the input argument.

                  if ($y_is_scalar) {

                      is(ref($y), '',
                         "'$test' second input arg is still a scalar");

                      is($y, $yval,
                         "'$test' second input arg is unmodified");

                  } else {

                      is(ref($y), $class,
                         "'$test' second input arg is still a $class");

                      is($y -> bstr(), $yval,
                         "'$test' second input arg is unmodified");
                  }
              };
        }
    }
}

__DATA__

# x and/or y is NaN

NaN:NaN:NaN

NaN:-inf:NaN
NaN:-3:NaN
NaN:0:NaN
NaN:3:NaN
NaN:inf:NaN

-inf:NaN:NaN
-3:NaN:NaN
0:NaN:NaN
3:NaN:NaN
inf:NaN:NaN

# x = inf

inf:-inf:NaN
inf:-3:inf
inf:-2:inf
inf:-1:inf
inf:0:inf
inf:1:inf
inf:2:inf
inf:3:inf
inf:inf:inf

# x = -inf

-inf:-inf:-inf
-inf:-3:-inf
-inf:-2:-inf
-inf:-1:-inf
-inf:0:-inf
-inf:1:-inf
-inf:2:-inf
-inf:3:-inf
-inf:inf:NaN

# y = inf

-3:inf:inf
-2:inf:inf
-1:inf:inf
0:inf:inf
1:inf:inf
2:inf:inf
3:inf:inf

# y = -inf

-3:-inf:-inf
-2:-inf:-inf
-1:-inf:-inf
0:-inf:-inf
1:-inf:-inf
2:-inf:-inf
3:-inf:-inf
