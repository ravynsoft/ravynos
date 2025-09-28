#!/usr/bin/perl

use strict;
use warnings;

#
# Check interactions of deferred writing
# with miscellaneous methods like DELETE, EXISTS,
# FETCHSIZE, STORESIZE, CLEAR, EXTEND
#

use POSIX 'SEEK_SET';
my $file = "tf32-$$.txt";
$: = Tie::File::_default_recsep();
my $data = "rec0$:rec1$:rec2$:";
my ($o, $n);

print "1..53\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

open F, '>', $file or die $!;
binmode F;
print F $data;
close F;

my @a;
$o = tie @a, 'Tie::File', $file;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

# (3-6) EXISTS
if ($] >= 5.006) {
  eval << 'TESTS';
$o->defer;
expect(not exists $a[4]);
$a[4] = "rec4";
expect(exists $a[4]);
check_contents($data);          # nothing written yet
$o->discard;
TESTS
} else {
    for (3..6) {
      print "ok $_ \# skipped (no exists for arrays)\n";
          $N++;
    }
}

# (7-10) FETCHSIZE
$o->defer;
expect($#a, 2);
$a[4] = "rec4";
expect($#a, 4);
check_contents($data);          # nothing written yet
$o->discard;

# (11-21) STORESIZE
$o->defer;
$#a = 4;
check_contents($data);          # nothing written yet
expect($#a, 4);
$o->flush;
expect($#a, 4);
check_contents("$data$:$:");    # two extra empty records

$o->defer;
$a[4] = "rec4";
$#a = 2;
expect($a[4], undef);
check_contents($data);          # written data was unwritten
$o->flush;
check_contents($data);          # nothing left to write

# (22-28) CLEAR
$o->defer;
$a[9] = "rec9";
check_contents($data);          # nothing written yet
@a = ();
check_contents("");             # this happens right away
expect($a[9], undef);
$o->flush;
check_contents("");             # nothing left to write

# (29-34) EXTEND
# Actually it's not real clear what these tests are for
# since EXTEND has no defined semantics
$o->defer;
@a = (0..3);
check_contents("");             # nothing happened yet
expect($a[3], "3");
expect($a[4], undef);
$o->flush;
check_contents("0$:1$:2$:3$:"); # file now 4 records long

# (35-53) DELETE
if ($] >= 5.006) {
  eval << 'TESTS';
my $del;
$o->defer;
$del = delete $a[2];
check_contents("0$:1$:2$:3$:"); # nothing happened yet
expect($a[2], "");
expect($del, "2");
$del = delete $a[3];            # shortens file!
check_contents("0$:1$:2$:");    # deferred writes NOT flushed
expect($a[3], undef);
expect($a[2], "");
expect($del, "3");
$a[2] = "cookies";
$del = delete $a[2];            # shortens file!
expect($a[2], undef);
expect($del, 'cookies');
check_contents("0$:1$:");
$a[0] = "crackers";
$del = delete $a[0];            # file unchanged
expect($a[0], "");
expect($del, 'crackers');
check_contents("0$:1$:");       # no change yet
$o->flush;
check_contents("$:1$:");        # record 0 is NOT 'cookies';
TESTS
} else {
    for (35..53) {
      print "ok $_ \# skipped (no delete for arrays)\n";
          $N++;
    }
}

################################################################


sub check_caches {
  my ($xcache, $xdefer) = @_;

#  my $integrity = $o->_check_integrity($file, $ENV{INTEGRITY});
#  print $integrity ? "ok $N\n" : "not ok $N\n";
#  $N++;

  my $good = 1;
  $good &&= hash_equal($o->{cache}, $xcache, "true cache", "expected cache");
  $good &&= hash_equal($o->{deferred}, $xdefer, "true defer", "expected defer");
  print $good ? "ok $N\n" : "not ok $N\n";
  $N++;
}

sub hash_equal {
  my ($a, $b, $ha, $hb) = @_;
  $ha = 'first hash'  unless defined $ha;
  $hb = 'second hash' unless defined $hb;

  my $good = 1;
  my %b_seen;

  for my $k (keys %$a) {
    if (! exists $b->{$k}) {
      print ctrlfix("# Key $k is in $ha but not $hb"), "\n";
      $good = 0;
    } elsif ($b->{$k} ne $a->{$k}) {
      print ctrlfix("# Key $k is <$a->{$k}> in $ha but <$b->{$k}> in $hb"), "\n";
      $b_seen{$k} = 1;
      $good = 0;
    } else {
      $b_seen{$k} = 1;
    }
  }

  for my $k (keys %$b) {
    unless ($b_seen{$k}) {
      print ctrlfix("# Key $k is in $hb but not $ha"), "\n";
      $good = 0;
    }
  }

  $good;
}


sub check_contents {
  my $x = shift;

  my $integrity = $o->_check_integrity($file, $ENV{INTEGRITY});
  print $integrity ? "ok $N\n" : "not ok $N\n";
  $N++;

  local *FH = $o->{fh};
  seek FH, 0, SEEK_SET;

  my $a;
  { local $/; $a = <FH> }
  $a = "" unless defined $a;
  if ($a eq $x) {
    print "ok $N\n";
  } else {
    my $msg = ctrlfix("# expected <$x>, got <$a>");
    print "not ok $N\n$msg\n";
  }
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
  local $_ = shift;
  s/\n/\\n/g;
  s/\r/\\r/g;
  $_;
}

END {
  undef $o;
  untie @a;
  1 while unlink $file;
}

