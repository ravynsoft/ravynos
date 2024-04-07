#!/usr/bin/perl

use strict;
use warnings;

#
# Unit tests for heap implementation
#
# Test the following methods:
# new
# is_empty
# empty
# insert
# remove
# popheap
# promote
# lookup
# set_val
# rekey
# expire_order


# Finish these later.

# They're nonurgent because the important heap stuff is extensively
# tested by tests 19, 20, 24, 30, 32, 33, and 40, as well as by pretty
# much everything else.
print "1..1\n";


my ($N, @R, $Q, $ar) = (1);

use Tie::File;
print "ok $N\n";
$N++;
exit;

__END__

my @HEAP_MOVE;
sub Fake::Cache::_heap_move { push @HEAP_MOVE, @_ }

my $h = Tie::File::Heap->new(bless [] => 'Fake::Cache');
print "ok $N\n";
$N++;

# (3) Are all the methods there?
{
  my $good = 1;
  for my $meth (qw(new is_empty empty lookup insert remove popheap
                   promote set_val rekey expire_order)) {
    unless ($h->can($meth)) {
      print STDERR "# Method '$meth' is missing.\n";
      $good = 0;
    }
  }
  print $good ? "ok $N\n" : "not ok $N\n";
  $N++;
}

# (4) Straight insert and removal FIFO test
$ar = 'a0';
for (1..10) {
  $h->insert($_, $ar++);
}
for (1..10) {
  push @R, $h->popheap;
}
$iota = iota('a',9);
print "@R" eq $iota
  ? "ok $N\n" : "not ok $N \# expected ($iota), got (@R)\n";
$N++;

# (5) Remove from empty heap
$n = $h->popheap;
print ! defined $n ? "ok $N\n" : "not ok $N \# expected UNDEF, got $n";
$N++;

# (6) Interleaved insert and removal
$Q = 0;
@R = ();
for my $i (1..4) {
  for my $j (1..$i) {
    $h->insert($Q, "b$Q");
    $Q++;
  }
  for my $j (1..$i) {
    push @R, $h->popheap;
  }
}
$iota = iota('b', 9);
print "@R" eq $iota ? "ok $N\n" : "not ok $N \# expected ($iota), got (@R)\n";
$N++;

# (7) It should be empty now
print $h->is_empty ? "ok $N\n" : "not ok $N\n";
$N++;

# (8) Insert and delete
$Q = 1;
for (1..10) {
  $h->insert($_, "c$Q");
  $Q++;
}
for (2, 4, 6, 8, 10) {
  $h->remove($_);
}
@R = ();
push @R, $n while defined ($n = $h->popheap);
print "@R" eq "c1 c3 c5 c7 c9" ? 
  "ok $N\n" : "not ok $N \# expected (c1 c3 c5 c7 c9), got (@R)\n";
$N++;

# (9) Interleaved insert and delete
$Q = 1; my $QQ = 1;
@R = ();
for my $i (1..4) {
  for my $j (1..$i) {
    $h->insert($Q, "d$Q");
    $Q++;
  }
  for my $j (1..$i) {
    $h->remove($QQ) if $QQ % 2 == 0;
    $QQ++;
  }
}
push @R, $n while defined ($n = $h->popheap);
print "@R" eq "d1 d3 d5 d7 d9" ? 
  "ok $N\n" : "not ok $N \# expected (d1 d3 d5 d7 d9), got (@R)\n";
$N++;

# (10) Promote
$Q = 1;
for (1..10) {
  $h->insert($_, "e$Q");
  $Q++;
}
for (2, 4, 6, 8, 10) {
  $h->promote($_);
}
@R = ();
push @R, $n while defined ($n = $h->popheap);
print "@R" eq "e1 e3 e5 e7 e9 e2 e4 e6 e8 e10" ? 
  "ok $N\n" : 
  "not ok $N \# expected (e1 e3 e5 e7 e9 e2 e4 e6 e8 e10), got (@R)\n";
$N++;

# (11-15) Lookup
$Q = 1;
for (1..10) {
  $h->insert($_, "f$Q");
  $Q++;
}
for (2, 4, 6, 4, 8) {
  my $r = $h->lookup($_);
  print $r eq "f$_" ? "ok $N\n" : "not ok $N \# expected f$_, got $r\n";
  $N++;
}

# (16) It shouldn't be empty
print ! $h->is_empty ? "ok $N\n" : "not ok $N\n";
$N++;

# (17) Lookup should have promoted the looked-up records
@R = ();
push @R, $n while defined ($n = $h->popheap);
print "@R" eq "f1 f3 f5 f7 f9 f10 f2 f6 f4 f8" ?
  "ok $N\n" : 
  "not ok $N \# expected (f1 f3 f5 f7 f9 f10 f2 f6 f4 f8), got (@R)\n";
$N++;

# (18-19) Typical 'rekey' operation
$Q = 1;
for (1..10) {
  $h->insert($_, "g$Q");
  $Q++;
}

$h->rekey([6,7,8,9,10], [8,9,10,11,12]);
my %x = qw(1 g1 2 g2  3 g3  4 g4  5 g5
           8 g6 9 g7 10 g8 11 g9 12 g10);
{
  my $good = 1;
  for my $k (keys %x) {
    my $v = $h->lookup($k);
    $v = "UNDEF" unless defined $v;
    unless ($v eq $x{$k}) {
      print "# looked up $k, got $v, expected $x{$k}\n";
      $good = 0;
    }
  }
  print $good ? "ok $N\n" : "not ok $N\n";
  $N++;
}
{
  my $good = 1;
  for my $k (6, 7) {
    my $v = $h->lookup($k);
    if (defined $v) {
      print "# looked up $k, got $v, should have been undef\n";
      $good = 0;
    }
  }
  print $good ? "ok $N\n" : "not ok $N\n";
  $N++;
}

# (20) keys
@R = sort { $a <=> $b } $h->keys;
print "@R" eq "1 2 3 4 5 8 9 10 11 12" ?
  "ok $N\n" : 
  "not ok $N \# expected (1 2 3 4 5 8 9 10 11 12) got (@R)\n";
$N++;

# (21) update
for (1..5, 8..12) {
  $h->update($_, "h$_");
}
@R = ();
for (sort { $a <=> $b } $h->keys) {
  push @R, $h->lookup($_);
}
print "@R" eq "h1 h2 h3 h4 h5 h8 h9 h10 h11 h12" ?
  "ok $N\n" : 
  "not ok $N \# expected (h1 h2 h3 h4 h5 h8 h9 h10 h11 h12) got (@R)\n";
$N++;

# (22-23) bytes
my $B;
$B = $h->bytes;
print $B == 23 ? "ok $N\n" : "not ok $N \# expected 23, got $B\n";
$N++;
$h->update('12', "yobgorgle");
$B = $h->bytes;
print $B == 29 ? "ok $N\n" : "not ok $N \# expected 29, got $B\n";
$N++;

# (24-25) empty
$h->empty;
print $h->is_empty ? "ok $N\n" : "not ok $N\n";
$N++;
$n = $h->popheap;
print ! defined $n ? "ok $N\n" : "not ok $N \# expected UNDEF, got $n";
$N++;

# (26) very weak testing of DESTROY
undef $h;
# are we still alive?
print "ok $N\n";
$N++;


sub iota {
  my ($p, $n) = @_;
  my $r;
  my $i = 0;
  while ($i <= $n) {
    $r .= "$p$i ";
    $i++;
  }
  chop $r;
  $r;
}
