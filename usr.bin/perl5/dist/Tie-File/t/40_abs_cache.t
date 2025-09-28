#!/usr/bin/perl

use strict;
use warnings;

#
# Unit tests for abstract cache implementation
#
# Test the following methods:
# * new()
# * is_empty()
# * empty()
# * lookup(key)
# * remove(key)
# * insert(key,val)
# * update(key,val)
# * rekey(okeys,nkeys)
# * expire()
# * keys()
# * bytes()
# DESTROY()
#
# 20020327 You somehow managed to miss:
# * reduce_size_to(bytes)
#

# print "1..0\n"; exit;
print "1..42\n";

my ($N, @R, $Q, $ar) = (1);

use Tie::File;
print "ok $N\n";
$N++;

my $h = Tie::File::Cache->new(10000) or die;
print "ok $N\n";
$N++;

# (3) Are all the methods there?
{
  my $good = 1;
  for my $meth (qw(new is_empty empty lookup remove
                 insert update rekey expire ckeys bytes
                   set_limit adj_limit flush  reduce_size_to
                   _produce _produce_lru )) {
    unless ($h->can($meth)) {
      print STDERR "# Method '$meth' is missing.\n";
      $good = 0;
    }
  }
  print $good ? "ok $N\n" : "not ok $N\n";
  $N++;
}

# (4-5) Straight insert and removal FIFO test
$ar = 'a0';
for (1..10) {
  $h->insert($_, $ar++);
}
1;
for (1..10) {
  push @R, $h->expire;
}
my $iota = iota('a',9);
print "@R" eq $iota
  ? "ok $N\n" : "not ok $N \# expected ($iota), got (@R)\n";
$N++;
check($h);

# (6-7) Remove from empty heap
my $n = $h->expire;
print ! defined $n ? "ok $N\n" : "not ok $N \# expected UNDEF, got $n";
$N++;
check($h);

# (8-9) Interleaved insert and removal
$Q = 0;
@R = ();
for my $i (1..4) {
  for my $j (1..$i) {
    $h->insert($Q, "b$Q");
    $Q++;
  }
  for my $j (1..$i) {
    push @R, $h->expire;
  }
}
$iota = iota('b', 9);
print "@R" eq $iota ? "ok $N\n" : "not ok $N \# expected ($iota), got (@R)\n";
$N++;
check($h);

# (10) It should be empty now
print $h->is_empty ? "ok $N\n" : "not ok $N\n";
$N++;

# (11-12) Insert and delete
$Q = 1;
for (1..10) {
  $h->insert($_, "c$Q");
  $Q++;
}
for (2, 4, 6, 8, 10) {
  $h->remove($_);
}
@R = ();
push @R, $n while defined ($n = $h->expire);
print "@R" eq "c1 c3 c5 c7 c9" ? 
  "ok $N\n" : "not ok $N \# expected (c1 c3 c5 c7 c9), got (@R)\n";
$N++;
check($h);

# (13-14) Interleaved insert and delete
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
push @R, $n while defined ($n = $h->expire);
print "@R" eq "d1 d3 d5 d7 d9" ? 
  "ok $N\n" : "not ok $N \# expected (d1 d3 d5 d7 d9), got (@R)\n";
$N++;
check($h);

# (15-16) Promote
$h->empty;
$Q = 1;
for (1..10) {
  $h->insert($_, "e$Q");
  unless ($h->_check_integrity) {
    die "Integrity failed after inserting ($_, e$Q)\n";
  }
  $Q++;
}
1;
for (2, 4, 6, 8, 10) {
  $h->_promote($_);
}
@R = ();
push @R, $n while defined ($n = $h->expire);
print "@R" eq "e1 e3 e5 e7 e9 e2 e4 e6 e8 e10" ? 
    "ok $N\n" : 
    "not ok $N \# expected (e1 e3 e5 e7 e9 e2 e4 e6 e8 e10), got (@R)\n";
$N++;
check($h);

# (17-22) Lookup
$Q = 1;
for (1..10) {
  $h->insert($_, "f$Q");
  $Q++;
}
1;
for (2, 4, 6, 4, 8) {
  my $r = $h->lookup($_);
  print $r eq "f$_" ? "ok $N\n" : "not ok $N \# expected f$_, got $r\n";
  $N++;
}
check($h);

# (23) It shouldn't be empty
print ! $h->is_empty ? "ok $N\n" : "not ok $N\n";
$N++;

# (24-25) Lookup should have promoted the looked-up records
@R = ();
push @R, $n while defined ($n = $h->expire);
print "@R" eq "f1 f3 f5 f7 f9 f10 f2 f6 f4 f8" ?
  "ok $N\n" : 
  "not ok $N \# expected (f1 f3 f5 f7 f9 f10 f2 f6 f4 f8), got (@R)\n";
$N++;
check($h);

# (26-29) Typical 'rekey' operation
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
check($h);
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
check($h);

# (30-31) ckeys
@R = sort { $a <=> $b } $h->ckeys;
print "@R" eq "1 2 3 4 5 8 9 10 11 12" ?
  "ok $N\n" : 
  "not ok $N \# expected (1 2 3 4 5 8 9 10 11 12) got (@R)\n";
$N++;
check($h);
1;
# (32-33) update
for (1..5, 8..12) {
  $h->update($_, "h$_");
}
@R = ();
for (sort { $a <=> $b } $h->ckeys) {
  push @R, $h->lookup($_);
}
print "@R" eq "h1 h2 h3 h4 h5 h8 h9 h10 h11 h12" ?
  "ok $N\n" : 
  "not ok $N \# expected (h1 h2 h3 h4 h5 h8 h9 h10 h11 h12) got (@R)\n";
$N++;
check($h);

# (34-37) bytes
my $B;
$B = $h->bytes;
print $B == 23 ? "ok $N\n" : "not ok $N \# expected 23, got $B\n";
$N++;
check($h);
$h->update('12', "yobgorgle");
$B = $h->bytes;
print $B == 29 ? "ok $N\n" : "not ok $N \# expected 29, got $B\n";
$N++;
check($h);

# (38-41) empty
$h->empty;
print $h->is_empty ? "ok $N\n" : "not ok $N\n";
$N++;
check($h);
$n = $h->expire;
print ! defined $n ? "ok $N\n" : "not ok $N \# expected UNDEF, got $n";
$N++;
check($h);

# (42) very weak testing of DESTROY
undef $h;
# are we still alive?
print "ok $N\n";
$N++;

sub check {
  my $h = shift;
  print $h->_check_integrity ? "ok $N\n" : "not ok $N\n";
  $N++;
}

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
