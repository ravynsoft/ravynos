#!/usr/bin/perl

use strict;
use warnings;

use POSIX 'SEEK_SET';

my $file = "tf18-$$.txt";
$/ = "blah";

print "1..5\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

my @a;
my $o = tie @a, 'Tie::File', $file, autodefer => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

$a[0] = 'rec0';
check_contents("rec0blah");
$a[1] = "rec1blah";
check_contents("rec0blahrec1blah");
$a[2] = "rec2blahblah";             # should we detect this?
check_contents("rec0blahrec1blahrec2blahblah");

sub check_contents {
  my $x = shift;
  local *FH = $o->{fh};
  seek FH, 0, SEEK_SET;
  my $a;
  { local $/; $a = <FH> }
  $a = "" unless defined $a;
  if ($a eq $x) {
    print "ok $N\n";
  } else {
    my $msg = "not ok $N # expected <$x>, got <$a>";
    ctrlfix($msg);
    print "$msg\n";
  }
  $N++;
}

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

