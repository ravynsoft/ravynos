# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 21;

use Scalar::Util qw< refaddr >;

my $class;

BEGIN {
    $class = 'Math::BigRat';
    use_ok($class);
}

while (<DATA>) {
    s/#.*$//;                   # remove comments
    s/\s+$//;                   # remove trailing whitespace
    next unless length;         # skip empty lines

    my ($xval, $yval) = split /:/;
    my ($x, $got, @got);

    for my $context_is_scalar (0, 1) {

        my $test = qq|\$x = $class -> new("$xval");|;

        $test .= $context_is_scalar
                 ? qq| \$got = \$x -> binv();|
                 : qq| \@got = \$x -> binv();|;

        my $desc = "binv() in ";
        $desc .= $context_is_scalar ? "scalar context" : "list context";

        subtest $desc,
          sub {
              plan tests => $context_is_scalar ? 4 : 5;

              eval $test;
              is($@, "", "'$test' gives emtpy \$\@");

              if ($context_is_scalar) {

                  # Check output.

                  is(ref($got), $class,
                     "'$test' output arg is a $class");

                  is($x -> bstr(), $yval,
                     "'$test' output arg has the right value");

                  is(refaddr($got), refaddr($x),
                     "'$test' output arg is the invocand");

              } else {

                  # Check number of output arguments.

                  cmp_ok(scalar(@got), '==', 1,
                         "'$test' gives one output arg");

                  # Check output.

                  is(ref($got[0]), $class,
                     "'$test' output arg is a $class");

                  is($got[0] -> bstr(), $yval,
                     "'$test' output arg has the right value");

                  is(refaddr($got[0]), refaddr($x),
                     "'$test' output arg is the invocand");
              }
          };
    }
}

__DATA__

NaN:NaN
inf:0
5:1/5
2:1/2
1:1
0:inf
-1:-1
-2:-1/2
-5:-1/5
-inf:0
