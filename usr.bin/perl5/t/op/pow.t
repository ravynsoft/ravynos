#!./perl -w
# Now they'll be wanting biff! and zap! tests too.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

# This calculation ought to be within 0.001 of the right answer.
my $bits_in_uv = int (0.001 + log (~0+1) / log 2);

# 3**30 < 2**48, don't trust things outside that range on a Cray
# Likewise other 3 should not overflow 48 bits if I did my sums right.
my @pow = ([  3, 30, 1e-14],
           [  4, 32,     0],
           [  5, 20, 1e-14],
           [2.5, 10, 1e-14],
           [ -2, 69,     0],
           [ -3, 30, 1e-14],
);
my $tests;
$tests += $_->[1] foreach @pow;

plan tests => 13 + $bits_in_uv + $tests;

# (-3)**3 gave 27 instead of -27 before change #20167.
# Let's test the other similar edge cases, too.
is((-3)**0, 1,   "negative ** 0 = 1");
is((-3)**1, -3,  "negative ** 1 = self");
is((-3)**2, 9,   "negative ** 2 = positive");
is((-3)**3, -27, "(negative int) ** (odd power) is negative");

# Positives shouldn't be a problem
is(3**0, 1,      "positive ** 0 = 1");
is(3**1, 3,      "positive ** 1 = self");
is(3**2, 9,      "positive ** 2 = positive");
is(3**3, 27,     "(positive int) ** (odd power) is positive");

# And test order of operations while we are at it
is(-3**0, -1,      "positive ** 0, then negated, = -1");
is(-3**1, -3,      "positive ** 1, then negated, = negative of self");
is(-3**2, -9,      "positive ** 2, then negated, = negative of square");
is(-3**3, -27,     "(positive int) ** (odd power), then negated, is negative");


# Ought to be 32, 64, 36 or something like that.

my $remainder = $bits_in_uv & 3;

cmp_ok ($remainder, '==', 0, 'Sanity check bits in UV calculation')
    or printf "# ~0 is %d (0x%d) which gives $bits_in_uv bits\n", ~0, ~0;

# These are a lot of brute force tests to see how accurate $m ** $n is.
# Unfortunately rather a lot of perl programs expect 2 ** $n to be integer
# perfect, forgetting that it's a call to floating point pow() which never
# claims to deliver perfection.
foreach my $n (0..$bits_in_uv - 1) {
    my $pow = 2 ** $n;
    my $int = 1 << $n;
    cmp_ok ($pow, '==', $int, "2 ** $n vs 1 << $n");
}

foreach my $pow (@pow) {
    my ($base, $max, $range) = @$pow;
    my $expect = 1;
    foreach my $n (0..$max-1) {
        my $got = $base ** $n;
        within ($got, $expect, $range, "$base ** $n got[$got] expect[$expect]");
        $expect *= $base;
    }
}
