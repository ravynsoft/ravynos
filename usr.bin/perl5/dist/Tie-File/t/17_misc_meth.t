#!/usr/bin/perl
#
# Check miscellaneous tied-array interface methods
# EXTEND, CLEAR, DELETE, EXISTS
#

use strict;
use warnings;


my $file = "tf17-$$.txt";
$: = Tie::File::_default_recsep();
1 while unlink $file;

print "1..35\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

my @a;
my $o = tie @a, 'Tie::File', $file, autodefer => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

# (3-8) EXTEND
$o->EXTEND(3);
check_contents("$:$:$:");
$o->EXTEND(4);
check_contents("$:$:$:$:");
$o->EXTEND(3);
check_contents("$:$:$:$:");

# (9-10) CLEAR
@a = ();
check_contents("");

# (11-20) EXISTS
if ($] >= 5.006) {
  eval << 'TESTS';
print !exists $a[0] ? "ok $N\n" : "not ok $N\n";
$N++;
$a[0] = "I like pie.";
print exists $a[0] ? "ok $N\n" : "not ok $N\n";
$N++;
print !exists $a[1] ? "ok $N\n" : "not ok $N\n";
$N++;
$a[2] = "GIVE ME PIE";
print exists $a[0] ? "ok $N\n" : "not ok $N\n";
$N++;
# exists $a[1] is not defined by this module under these circumstances
print exists $a[1] ? "ok $N\n" : "ok $N\n";
$N++;
print exists $a[2] ? "ok $N\n" : "not ok $N\n";
$N++;
print exists $a[-1] ? "ok $N\n" : "not ok $N\n";
$N++;
print exists $a[-2] ? "ok $N\n" : "not ok $N\n";
$N++;
print exists $a[-3] ? "ok $N\n" : "not ok $N\n";
$N++;
print !exists $a[-4] ? "ok $N\n" : "not ok $N\n";
$N++;
TESTS
  } else {                      # perl 5.005 doesn't have exists $array[1]
    for (11..20) {
      print "ok $_ \# skipped (no exists for arrays)\n";
          $N++;
    }
  }

my $del;

# (21-35) DELETE
if ($] >= 5.006) {
  eval << 'TESTS';
$del = delete $a[0];
check_contents("$:$:GIVE ME PIE$:");
# 20020317 Through 0.20, the 'delete' function returned the wrong values.
expect($del, "I like pie.");
$del = delete $a[2];
check_contents("$:$:");
expect($del, "GIVE ME PIE");
$del = delete $a[0];
check_contents("$:$:");
expect($del, "");
$del = delete $a[1];
check_contents("$:");
expect($del, "");

# 20020317 Through 0.20, we had a bug where deleting an element past the 
# end of the array would actually extend the array to that length.
$del = delete $a[4];
check_contents("$:");
expect($del, undef);



TESTS
  } else {                      # perl 5.005 doesn't have delete $array[1]
    for (21..35) {
      print "ok $_ \# skipped (no delete for arrays)\n";
          $N++;
    }
  }

use POSIX 'SEEK_SET';
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
    ctrlfix(my $msg = "# expected <$x>, got <$a>");
    print "not ok $N # $msg\n";
  }
  $N++;
  print $o->_check_integrity($file, $ENV{INTEGRITY}) ? "ok $N\n" : "not ok $N\n";
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


