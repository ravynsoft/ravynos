#!/usr/bin/perl
#
# Make sure we can fetch a record in the middle of the file
# before we've ever looked at any records before it
#
# (tests _fill_offsets_to() )
#

use strict;
use warnings;

my $file = "tf12-$$.txt";
my $data = "rec0blahrec1blahrec2blah";

print "1..5\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

open F, '>', $file or die $!;
binmode F;
print F $data;
close F;

my @a;
my $o = tie @a, 'Tie::File', $file, autochomp => 0, recsep => 'blah';
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

my $n;

# 3-5
for (2, 1, 0) {
  print $a[$_] eq "rec${_}blah" ? "ok $N\n" : "not ok $N # rec=$a[$_] ?\n";
  $N++;
}

END {
  undef $o;
  untie @a;
  1 while unlink $file;
}

