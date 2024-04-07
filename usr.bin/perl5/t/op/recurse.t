#!./perl

#
# test recursive functions.
#

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc(qw(. ../lib));
}

plan(tests => 28);

use strict;

sub gcd {
    return gcd($_[0] - $_[1], $_[1]) if ($_[0] > $_[1]);
    return gcd($_[0], $_[1] - $_[0]) if ($_[0] < $_[1]);
    $_[0];
}

sub factorial {
    $_[0] < 2 ? 1 : $_[0] * factorial($_[0] - 1);
}

sub fibonacci {
    $_[0] < 2 ? 1 : fibonacci($_[0] - 2) + fibonacci($_[0] - 1);
}

# Highly recursive, highly aggressive.
# Kids, don't try this at home.
#
# For example ackermann(4,1) will take quite a long time.
# It will simply eat away your memory. Trust me.

sub ackermann {
    return $_[1] + 1               if ($_[0] == 0);
    return ackermann($_[0] - 1, 1) if ($_[1] == 0);
    ackermann($_[0] - 1, ackermann($_[0], $_[1] - 1));
}

# Highly recursive, highly boring.

sub takeuchi {
    $_[1] < $_[0] ?
	takeuchi(takeuchi($_[0] - 1, $_[1], $_[2]),
		 takeuchi($_[1] - 1, $_[2], $_[0]),
		 takeuchi($_[2] - 1, $_[0], $_[1]))
	    : $_[2];
}

is(gcd(1147, 1271), 31, "gcd(1147, 1271) == 31");

is(gcd(1908, 2016), 36, "gcd(1908, 2016) == 36");

is(factorial(10), 3628800, "factorial(10) == 3628800");

is(factorial(factorial(3)), 720, "factorial(factorial(3)) == 720");

is(fibonacci(10), 89, "fibonacci(10) == 89");

is(fibonacci(fibonacci(7)), 17711, "fibonacci(fibonacci(7)) == 17711");

my @ack = qw(1 2 3 4 2 3 4 5 3 5 7 9 5 13 29 61);

for my $x (0..3) { 
    for my $y (0..3) {
	my $a = ackermann($x, $y);
	is($a, shift(@ack), "ackermann($x, $y) == $a");
    }
}

my ($x, $y, $z) = (18, 12, 6);

is(takeuchi($x, $y, $z), $z + 1, "takeuchi($x, $y, $z) == $z + 1");

{
    sub get_first1 {
	get_list1(@_)->[0];
    }

    sub get_list1 {
	return [curr_test] unless $_[0];
	my $u = get_first1(0);
	[$u];
    }
    my $x = get_first1(1);
    ok($x, "premature FREETMPS (change 5699)");
}

{
    sub get_first2 {
	return get_list2(@_)->[0];
    }

    sub get_list2 {
	return [curr_test] unless $_[0];
	my $u = get_first2(0);
	return [$u];
    }
    my $x = get_first2(1);
    ok($x, "premature FREETMPS (change 5699)");
}

{
    local $^W = 0; # We do not need recursion depth warning.

    sub sillysum {
	return $_[0] + ($_[0] > 0 ? sillysum($_[0] - 1) : 0);
    }

    is(sillysum(1000), 1000*1001/2, "recursive sum of 1..1000");
}

# check ok for recursion depth > 65536
{
    my $r;
    eval { 
	$r = runperl(
		     nolib => 1,
		     stderr => 1,
		     prog => q{$d=0; $e=1; sub c { ++$d; if ($d > 66000) { $e=0 } else { c(); c() unless $d % 32768 } --$d } c(); exit $e});
    };
  SKIP: {
      skip("Out of memory -- increase your data/heap?", 2)
	  if $r =~ /Out of memory/i;
      is($r, '', "64K deep recursion - no output expected");
      is($?, 0, "64K deep recursion - no coredump expected");
  }
}

