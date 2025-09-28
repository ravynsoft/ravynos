#!perl
BEGIN {
    chdir 't' if -d 't';
    @INC = "../lib";
    require './test.pl';
}

use Config qw(%Config);

$ENV{PERL_TEST_MEMORY} >= 2
    or skip_all("Need ~2Gb for this test");
$Config{ptrsize} >= 8
    or skip_all("Need 64-bit pointers for this test");

plan(3);

# [perl #116907]
# ${\2} to defeat constant folding, which in this case actually slows
# things down
my $x=" "x(${\2}**31+20);
pos $x = 2**31-5;
is pos $x, 2147483643, 'setting pos on large string';
pos $x += 10;
is pos $x, 2147483653, 'reading lvalue pos after setting it > 2**31';
is scalar(pos $x), 2147483653, 'reading it with pos() in rvalue context';
