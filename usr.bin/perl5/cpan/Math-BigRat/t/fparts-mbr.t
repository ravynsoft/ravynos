# -*- mode: perl; -*-

# test fparts(), numerator(), denominator()

use strict;
use warnings;

use Test::More tests => 43;

my $class;

BEGIN {
    $class = 'Math::BigRat';
    use_ok($class);
}

while (<DATA>) {
    s/#.*$//;                   # remove comments
    s/\s+$//;                   # remove trailing whitespace
    next unless length;         # skip empty lines

    my ($x_str, $n_str, $d_str) = split /:/;
    my $test;

    # test fparts()

    $test = qq|\$x = $class -> new("$x_str");|
          . qq| (\$n, \$d) = \$x -> fparts();|;

    subtest $test => sub {
        plan tests => 5;

        my $x = $class -> new($x_str);
        my ($n, $d) = $x -> fparts();

        is(ref($n), $class, "class of numerator");
        is(ref($d), $class, "class of denominator");

        is($n, $n_str, "value of numerator");
        is($d, $d_str, "value of denominator");
        is($x, $x_str, "input is unmodified");
    };

    # test numerator()

    $test = qq|\$x = $class -> new("$x_str");|
          . qq| \$n = \$x -> numerator();|;

    subtest $test => sub {
        plan tests => 3;

        my $x = $class -> new($x_str);
        my $n = $x -> numerator();

        is(ref($n), "Math::BigInt", "class of numerator");

        is($n, $n_str, "value of numerator");
        is($x, $x_str, "input is unmodified");
    };

    # test denominator()

    $test = qq|\$x = $class -> new("$x_str");|
          . qq| \$d = \$x -> denominator();|;

    subtest $test => sub {
        plan tests => 3;

        my $x = $class -> new($x_str);
        my $d = $x -> denominator();

        is(ref($d), "Math::BigInt", "class of denominator");

        is($d, $d_str, "value of denominator");
        is($x, $x_str, "input is unmodified");
    };
}

__DATA__

NaN:NaN:NaN

inf:inf:1
-inf:-inf:1

-30:-30:1
-3:-3:1
-1:-1:1
0:0:1
1:1:1
3:3:1
30:30:1

-31400:-31400:1
-157/50:-157:50
157/50:157:50
31400:31400:1
