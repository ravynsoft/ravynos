#!/usr/bin/perl
#
# Check flock() feature
#
# This isn't a real test; it just checks to make sure we can call the method.
# It doesn't even check to make sure that the default behavior
# (LOCK_EX) is occurring.  This is because I don't know how to write a good
# portable test for flocking.  I checked the Perl core distribution,
# and found that Perl doesn't test flock either!

use strict;
use warnings;

BEGIN {
  eval { flock STDOUT, 0 };
  if ($@ && $@ =~ /unimplemented/) {
    print "1..0\n";
    exit;
  }
}

use Fcntl ':flock';             # This works at least back to 5.004_04

my $file = "tf14-$$.txt";
my ($o, $n);
my @a;

print "1..4\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

# 2-4  Who the heck knows?
open F, '>', $file or die $!;
close F;
$o = tie @a, 'Tie::File', $file, recsep => 'blah';
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

print $o->flock() ? "ok $N\n" : "not ok $N\n";
$N++;

print $o->flock(LOCK_UN) ? "ok $N\n" : "not ok $N\n";
$N++;


END {
  undef $o;
  untie @a;
  1 while unlink $file;
}

