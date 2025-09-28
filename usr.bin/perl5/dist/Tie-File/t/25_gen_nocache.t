#!/usr/bin/perl

use strict;
use warnings;

#
# Regular read-write tests with caching disabled
# (Same as 01_gen.t)
#
my $file = "tf25-$$.txt";

print "1..68\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

my @a;
my $o = tie @a, 'Tie::File', $file, autochomp => 0, autodefer => 0, memory => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

$: = $o->{recsep};

# 3-5 create
$a[0] = 'rec0';
check_contents("rec0");

# 6-11 append
$a[1] = 'rec1';
check_contents("rec0", "rec1");
$a[2] = 'rec2';
check_contents("rec0", "rec1", "rec2");

# 12-20 same-length alterations
$a[0] = 'new0';
check_contents("new0", "rec1", "rec2");
$a[1] = 'new1';
check_contents("new0", "new1", "rec2");
$a[2] = 'new2';
check_contents("new0", "new1", "new2");

# 21-35 lengthening alterations
$a[0] = 'long0';
check_contents("long0", "new1", "new2");
$a[1] = 'long1';
check_contents("long0", "long1", "new2");
$a[2] = 'long2';
check_contents("long0", "long1", "long2");
$a[1] = 'longer1';
check_contents("long0", "longer1", "long2");
$a[0] = 'longer0';
check_contents("longer0", "longer1", "long2");

# 36-50 shortening alterations, including truncation
$a[0] = 'short0';
check_contents("short0", "longer1", "long2");
$a[1] = 'short1';
check_contents("short0", "short1", "long2");
$a[2] = 'short2';
check_contents("short0", "short1", "short2");
$a[1] = 'sh1';
check_contents("short0", "sh1", "short2");
$a[0] = 'sh0';
check_contents("sh0", "sh1", "short2");

# (51-56) file with holes
$a[4] = 'rec4';
check_contents("sh0", "sh1", "short2", "", "rec4");
$a[3] = 'rec3';
check_contents("sh0", "sh1", "short2", "rec3", "rec4");

# (57-59) zero out file
@a = ();
check_contents();

# (60-62) insert into the middle of an empty file
$a[3] = "rec3";
check_contents("", "", "", "rec3");

# (63-68) 20020326 You thought there would be a bug in STORE where if
# a cached record was false, STORE wouldn't see it at all.  But you
# forgot that records always come back from the cache with the record
# separator attached, so they are unlikely to be false.  The only
# really weird case is when the cached record is empty and the record
# separator is "0".  Test that in 09_gen_rs.t.
$a[1] = "0";
check_contents("", "0", "", "rec3");
$a[1] = "whoops";
check_contents("", "whoops", "", "rec3");


use POSIX 'SEEK_SET';
sub check_contents {
  my @c = @_;
  my $x = join $:, @c, '';
  local *FH = $o->{fh};
  seek FH, 0, SEEK_SET;
#  my $open = open FH, '<', $file;
  my $a;
  { local $/; $a = <FH> }
  $a = "" unless defined $a;
  if ($a eq $x) {
    print "ok $N\n";
  } else {
    ctrlfix($a, $x);
    print "not ok $N\n# expected <$x>, got <$a>\n";
  }
  $N++;

  # now check FETCH:
  my $good = 1;
  my $msg;
  for (0.. $#c) {
    my $aa = $a[$_];
    unless ($aa eq "$c[$_]$:") {
      $msg = "expected <$c[$_]$:>, got <$aa>";
      ctrlfix($msg);
      $good = 0;
    }
  }
  print $good ? "ok $N\n" : "not ok $N # $msg\n";
  $N++;

  print $o->_check_integrity($file, $ENV{INTEGRITY}) 
      ? "ok $N\n" : "not ok $N\n";
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

