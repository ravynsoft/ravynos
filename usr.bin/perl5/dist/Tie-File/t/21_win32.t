#!/usr/bin/perl

use strict;
use warnings;

#
# Formerly, on a Win32 system, Tie::File would create files with
# \n-terminated records instead of \r\n-terminated.  The tests never
# picked this up because they were using $/ everywhere, and $/ is \n
# on windows systems.
#
# These tests (Win32 only) make sure that the file had \r\n as it should.

my $file = "tf21-$$.txt";

unless ($^O =~ /^(MSWin32|dos)$/) {
  print "1..0\n";
  exit;
}


print "1..3\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

my @a;
my $o = tie @a, 'Tie::File', $file, autodefer => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

my $n;

# (3) Make sure that on Win32 systems, the file is written with \r\n by default
@a = qw(fish dog carrot);
undef $o;
untie @a;
open F, '<', $file or die "Couldn't open file $file: $!";
binmode F;
my $a = do {local $/ ; <F> };
my $x = "fish\r\ndog\r\ncarrot\r\n" ;
if ($a eq $x) {
  print "ok $N\n";
} else {
  ctrlfix(my $msg = "expected <$x>, got <$a>");
  print "not ok $N # $msg\n";
}

close F;

sub ctrlfix {
  for (@_) {
    s/\n/\\n/g;
    s/\r/\\r/g;
  }
}



END {
  undef $o;
  untie @a;
  1 while unlink $file;
}

