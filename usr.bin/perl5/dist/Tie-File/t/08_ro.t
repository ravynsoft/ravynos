#!/usr/bin/perl
#
# Make sure it works to open the file in read-only mode
#

use strict;
use warnings;

my $file = "tf08-$$.txt";
$: = Tie::File::_default_recsep();

print "1..13\n";

my $N = 1;
use Tie::File;
use Fcntl 'O_RDONLY';
print "ok $N\n"; $N++;

my @items = qw(Gold Frankincense Myrrh Ivory Apes Peacocks);
init_file(join $:, @items, '');

my @a;
my $o = tie @a, 'Tie::File', $file, mode => O_RDONLY, autochomp => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

$#a == $#items ? print "ok $N\n" : print "not ok $N\n";
$N++;

for my $i (0..$#items) {
  ("$items[$i]$:" eq $a[$i]) ? print "ok $N\n" : print "not ok $N\n";
  $N++;
}

sub init_file {
  my $data = shift;
  open F, '>', $file or die $!;
  binmode F;
  print F $data;
  close F;
}

undef $o; untie @a;
my $badrec = "Malformed";
# (10-13) When a record lacks the record seprator, we sneakily try
# to fix it.  How does that work when the file is read-only?
if (setup_badly_terminated_file(4)) {
  my $good = 1;
  my $warn;
  local $SIG{__WARN__} = sub { $good = 0; ctrlfix($warn = shift); };
  local $^W = 1;
  my $o = tie @a, 'Tie::File', $file, mode => O_RDONLY, autochomp => 0
    or die "Couldn't tie $file: $!";

  print $a[0] eq "Malformed$:" ? "ok $N\n" : "not ok $N\n";  $N++;
  print $good ? "ok $N\n" : "not ok $N # $warn\n";  $good = 1; $N++;
  print $a[0] eq "Malformed$:" ? "ok $N\n" : "not ok $N\n";  $N++;
  print $good ? "ok $N\n" : "not ok $N # $warn\n";  $good = 1; $N++;
}

sub setup_badly_terminated_file {
  my $NTESTS = shift;
  open F, '>', $file or die "Couldn't open $file: $!";
  binmode F;
  print F $badrec;
  close F;
  unless (-s $file == length $badrec) {
    for (1 .. $NTESTS) {
      print "ok $N \# skipped - can't create improperly terminated file\n";
      $N++;
    }
    return;
  }
  return 1;
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

