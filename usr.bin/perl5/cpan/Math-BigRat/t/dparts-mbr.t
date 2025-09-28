# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 17;

my $class;

BEGIN {
    $class = 'Math::BigRat';
    use_ok($class);
}

while (<DATA>) {
    s/#.*$//;                   # remove comments
    s/\s+$//;                   # remove trailing whitespace
    next unless length;         # skip empty lines

    my ($x_str, $int_str, $frc_str) = split /:/;
    my $test;

    $test = qq|\$x = $class -> new("$x_str");|
          . qq| (\$i, \$f) = \$x -> dparts();|;

    subtest $test => sub {
        plan tests => 5;

        my $x = $class -> new($x_str);
        my ($int_got, $frc_got) = $x -> dparts();

        is(ref($int_got), $class, "class of integer part");
        is(ref($frc_got), $class, "class of fraction part");

        is($int_got, $int_str, "value of integer part");
        is($frc_got, $frc_str, "value of fraction part");
        is($x,       $x_str,   "input is unmodified");
    };

    $test = qq|\$x = $class -> new("$x_str");|
          . qq| \$i = \$x -> dparts();|;

    subtest $test => sub {
        plan tests => 3,

        my $x = $class -> new($x_str);
        my $int_got = $x -> dparts();

        isa_ok($int_got, $class);

        is($int_got, $int_str, "value of integer part");
        is($x,       $x_str,   "input is unmodified");
    };
}

__DATA__

NaN:NaN:NaN

inf:inf:0
-inf:-inf:0

-9/4:-2:-1/4
-1:-1:0
0:0:0
1:1:0
9/4:2:1/4
