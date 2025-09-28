#!/usr/bin/perl

use strict;
use warnings;

my $file = "tf22-$$.txt";
$: = Tie::File::_default_recsep();

print "1..71\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

my @a;
my $o = tie @a, 'Tie::File', $file, autochomp => 1, autodefer => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

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

# (63-68) Test the ->autochomp() method
@a = qw(Gold Frankincense Myrrh);
my $ac;
$ac = $o->autochomp();
expect($ac);
# See if that accidentally changed it
$ac = $o->autochomp();
expect($ac);
# Now clear it
$ac = $o->autochomp(0);
expect($ac);
expect(join("-", @a), "Gold$:-Frankincense$:-Myrrh$:");
# Now set it again
$ac = $o->autochomp(1);
expect(!$ac);
expect(join("-", @a), "Gold-Frankincense-Myrrh");

# (69) Does 'splice' work correctly with autochomp?
my @sr;
@sr = splice @a, 0, 2;
expect(join("-", @sr), "Gold-Frankincense");

# (70-71) Didn't you forget that fetch may return an unchomped cached record?
my $a1 = $a[0];                    # populate cache
my $a2 = $a[0];
expect($a1, "Myrrh");
expect($a2, "Myrrh");
# Actually no, you didn't---_fetch might return such a record, but 
# the chomping is done by FETCH.

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
    unless ($aa eq $c[$_]) {
      $msg = "expected <$c[$_]>, got <$aa>";
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

sub expect {
  if (@_ == 1) {
    print $_[0] ? "ok $N\n" : "not ok $N\n";
  } elsif (@_ == 2) {
    my ($a, $x) = @_;
    if    (! defined($a) && ! defined($x)) { print "ok $N\n" }
    elsif (  defined($a) && ! defined($x)) { 
      ctrlfix(my $msg = "expected UNDEF, got <$a>");
      print "not ok $N \# $msg\n";
    }
    elsif (! defined($a) &&   defined($x)) { 
      ctrlfix(my $msg = "expected <$x>, got UNDEF");
      print "not ok $N \# $msg\n";
    } elsif ($a eq $x) { print "ok $N\n" }
    else {
      ctrlfix(my $msg = "expected <$x>, got <$a>");
      print "not ok $N \# $msg\n";
    }
  } else {
    die "expect() got ", scalar(@_), " args, should have been 1 or 2";
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

