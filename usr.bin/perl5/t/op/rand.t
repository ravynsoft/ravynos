#!./perl

# From Tom Phoenix <rootbeer@teleport.com> 22 Feb 1997
# Based upon a test script by kgb@ast.cam.ac.uk (Karl Glazebrook)

# Looking for the hints? You're in the right place. 
# The hints are near each test, so search for "TEST #", where
# the pound sign is replaced by the number of the test.

# I'd like to include some more robust tests, but anything
# too subtle to be detected here would require a time-consuming
# test. Also, of course, we're here to detect only flaws in Perl;
# if there are flaws in the underlying system rand, that's not
# our responsibility. But if you want better tests, see
# The Art of Computer Programming, Donald E. Knuth, volume 2,
# chapter 3. ISBN 0-201-03822-6 (v. 2)

BEGIN {
    chdir "t" if -d "t";
    require "./test.pl";
    set_up_inc( qw(. ../lib) );
}

use strict;
use Config;

my $reps = 100_000;	# How many times to try rand each time.
			# May be changed, but should be over 500.
			# The more the better! (But slower.)

my $bits = 8;  # how many significant bits we check on each random number
my $nslots = (1<< $bits); # how many different numbers

plan(tests => 7 + $nslots);

# First, let's see whether randbits is set right and that rand() returns
# an even distribution of values
{
    my $sum;
    my @slots = (0) x $nslots;
    my $prob = 1/$nslots;     # probability of a particular slot being
                              # on a particular iteration

    # We are going to generate $reps random numbers, each in the range
    # 0..$nslots-1. They should be evenly distributed. We use @slots to
    # count the number of occurrences of each number. For each count, we
    # check that it is in the range we expect. For example for reps =
    # 100_000 and using 8 bits, we expect each count to be around
    # 100_000/256 = 390. How much around it we tolerate depends on the
    # standard deviation, and how many deviations we allow. If we allow
    # 6-sigmas, then that means that in only 1 run in 506e6 will be get a
    # failure by chance, assuming a fair random number generator. Given
    # that we test each slot, the overall chance of a false negative in
    # this test script is about 1 in 2e6, assuming 256 slots.
    #
    # the actual count in a slot should follow a binomial distribution
    # (e.g. rolling 18 dice, we 'expect' to see 3 sixes, but there's
    # actually a 24% chance of 3, a 20% change of 2 or 4, a 12%
    # chance of 1 or 5, and a 4% chance of 0 or 6 of them).
    #
    # This makes it easy to calculate the expected mean a standard
    # deviation; see
    #     https://en.wikipedia.org/wiki/Binomial_distribution#Variance

    my $mean   = $reps * $prob;
    my $stddev = sqrt($reps * $prob * (1 - $prob));
    my $sigma6 = $stddev * 6.0; # very unlikely to be outside that range
    my $min = $mean - $sigma6;
    my $max = $mean + $sigma6;

    note("reps=$reps; slots=$nslots; min=$min mean=$mean max=$max");

    for (1..$reps) {
	my $n = rand(1);
	if ($n < 0.0 or $n >= 1.0) {
	    diag(<<EOM);
WHOA THERE!  \$Config{drand01} is set to '$Config{drand01}',
but that apparently produces values ($n) < 0.0 or >= 1.0.
Make sure \$Config{drand01} is a valid expression in the
C-language, and produces values in the range [0.0,1.0).

I give up.
EOM
	    exit;
	}
        $slots[int($n * $nslots)]++;
    }

    for my $i (0..$nslots - 1) {
        # this test should randomly fail very rarely. If it fails
        # for you, try re-running this test script a few more times;
        # if it goes away, it was likely a random (ha ha!) glitch.
        # If you keep seeing failures, it means your random number
        # generator is producing a very uneven spread of values.
        ok($slots[$i] >= $min && $slots[$i] <= $max, "checking slot $i")
            or diag("slot $i; count $slots[$i] outside expected range $min..$max");
    }
}


# Now, let's see whether rand accepts its argument
{
    my($max, $min);
    $max = $min = rand(100);
    for (1..$reps) {
	my $n = rand(100);
	$max = $n if $n > $max;
	$min = $n if $n < $min;
    }

    # This test checks to see that rand(100) really falls 
    # within the range 0 - 100, and that the numbers produced
    # have a reasonably-large range among them.
    #
    cmp_ok($min,        '>=',  0, "rand(100) >= 0");
    cmp_ok($max,        '<', 100, "rand(100) < 100");
    cmp_ok($max - $min, '>=', 65, "rand(100) in 65 range");


    # This test checks that rand without an argument
    # is equivalent to rand(1).
    #
    $_ = 12345;		# Just for fun.
    srand 12345;
    my $r = rand;
    srand 12345;
    is(rand(1),  $r,  'rand() without args is rand(1)');


    # This checks that rand without an argument is not
    # rand($_). (In case somebody got overzealous.)
    # 
    cmp_ok($r, '<', 1,   'rand() without args is under 1');
}

{ # [perl #115928] use a standard rand() implementation
    srand(1);
    is(int rand(1000), 41, "our own implementation behaves consistently");
    is(int rand(1000), 454, "and still consistently");
}
