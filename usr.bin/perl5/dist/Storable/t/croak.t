#!./perl -w

# Please keep this test this simple. (ie just one test.)
# There's some sort of not-croaking properly problem in Storable when built
# with 5.005_03. This test shows it up, whereas malice.t does not.
# In particular, don't use Test; as this covers up the problem.

sub BEGIN {
    if ($ENV{PERL_CORE}) {
	require Config; import Config;
	%Config=%Config if 0; # cease -w
	if ($Config{'extensions'} !~ /\bStorable\b/) {
	    print "1..0 # Skip: Storable was not built\n";
	    exit 0;
	}
    }
}

use strict;

BEGIN {
  die "Oi! No! Don't change this test so that Carp is used before Storable"
    if defined &Carp::carp;
}
use Storable qw(freeze thaw);

print "1..2\n";

for my $test (1,2) {
  eval {thaw "\xFF\xFF"};
  if ($@ =~ /Storable binary image v127.255 more recent than I am \(v2\.\d+\)/)
    {
      print "ok $test\n";
    } else {
      chomp $@;
      print "not ok $test # Expected a meaningful croak. Got '$@'\n";
    }
}
