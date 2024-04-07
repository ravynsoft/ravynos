#!/usr/bin/perl
#
# Make sure we can fetch a record in the middle of the file
# before we've ever looked at any records before it
#
# Make sure fetching past the end of the file returns the undefined value
#
# (tests _fill_offsets_to() )
#

use strict;
use warnings;

my $file = "tf03-$$.txt";
$: = Tie::File::_default_recsep();
my $data = "rec0$:rec1$:rec2$:";

print "1..8\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

open F, '>', $file or die $!;
binmode F;
print F $data;
close F;

my @a;
my $o = tie @a, 'Tie::File', $file, autochomp => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

$: = $o->{recsep};

my $n;

# 3-5
for (2, 1, 0) {
  my $rec = $a[$_];
  print $rec eq "rec$_$:" ? "ok $N\n" : "not ok $N # rec=<$rec> ?\n";
  $N++;
}

# 6-8
for (3, 4, 6) {
  my $rec = $a[$_];
  print ((not defined $rec) ? "ok $N\n" : "not ok $N # rec=<$rec> is defined\n");
  $N++;
}

END {
  undef $o;
  untie @a;
  1 while unlink $file;
}

