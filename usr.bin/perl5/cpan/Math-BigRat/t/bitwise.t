# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More tests => 2602;

my @classes = ('Math::BigRat');

# We should test all the following operators:
#
#     & | ^ << >> &= |= ^= <<= >>=
#
# as well as the corresponding methods
#
#     band bior bxor blsft brsft

for my $class (@classes) {
    use_ok($class);

    for my $op (qw( & | ^ )) {
        for (my $xscalar = 0 ; $xscalar <= 8 ; $xscalar += 0.5) {
            for (my $yscalar = 0 ; $yscalar <= 8 ; $yscalar += 0.5) {

                my $xint = int $xscalar;
                my $yint = int $yscalar;

                my $x = $class -> new("$xscalar");
                my $y = $class -> new("$yscalar");

                my $test     = "$x $op $y";
                my $expected = eval "$xscalar $op $yscalar";
                my $got      = eval "\$x $op \$y";

                is($@, '', 'is $@ empty');
                isa_ok($got, $class, $test);
                is($got, $expected,
                   "$x $op $y = $xint $op $yint = $expected");
            }
        }
    }
}
