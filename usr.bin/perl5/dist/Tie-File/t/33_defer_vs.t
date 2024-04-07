#!/usr/bin/perl

use strict;
use warnings;

#
# Deferred caching of varying size records
#
# 30_defer.t always uses records that are 8 bytes long
# (9 on \r\n machines.)  We might miss some sort of
# length-calculation bug as a result.  This file will run some of the same
# tests, but with with varying-length records.
#

use POSIX 'SEEK_SET';
my $file = "tf33-$$.txt";
# print "1..0\n"; exit;
$: = Tie::File::_default_recsep();
my $data = "$:1$:22$:";
my ($o, $n);

print "1..30\n";

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

# (3-6) Deferred storage
$o->defer;
$a[3] = "333";
check_contents($data);          # nothing written yet
$a[4] = "4444";
check_contents($data);          # nothing written yet

# (7-8) Flush
$o->flush;
check_contents($data . "333$:4444$:");          # now it's written

# (9-12) Deferred writing disabled?
$a[3] = "999999999";
check_contents("${data}999999999$:4444$:");
$a[4] = "88888888";
check_contents("${data}999999999$:88888888$:");

# (13-18) Now let's try two batches of records
$#a = 2;
$o->defer;
$a[0] = "55555";
check_contents($data);          # nothing written yet
$a[2] = "aaaaaaaaaa";
check_contents($data);          # nothing written yet
$o->flush;
check_contents("55555$:1$:aaaaaaaaaa$:");

# (19-22) Deferred writing past the end of the file
$o->defer;
$a[4] = "7777777";
check_contents("55555$:1$:aaaaaaaaaa$:");
$o->flush;
check_contents("55555$:1$:aaaaaaaaaa$:$:7777777$:");


# (23-26) Now two long batches
$o->defer;
my %l = qw(0 2  1 3  2 4  4 5  5 4  6 3);
for (0..2, 4..6) {
  $a[$_] = $_ x $l{$_};
}
check_contents("55555$:1$:aaaaaaaaaa$:$:7777777$:");
$o->flush;
check_contents(join $:, "00", "111", "2222", "", "44444", "5555", "666", "");

# (27-30) Now let's make sure that discarded writes are really discarded
# We have a 2Mib buffer here, so we can be sure that we aren't accidentally
# filling it up
$o->defer;
for (0, 3, 7) {
  $a[$_] = "discarded" . $_ x $_;
}
check_contents(join $:, "00", "111", "2222", "", "44444", "5555", "666", "");
$o->discard;
check_contents(join $:, "00", "111", "2222", "", "44444", "5555", "666", "");

################################################################


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

