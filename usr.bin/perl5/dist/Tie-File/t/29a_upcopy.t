#!/usr/bin/perl

use strict;
use warnings;

#
# Unit tests of _upcopy function
#
# _upcopy($self, $source, $dest, $len)
#
# Take a block of data of leength $len at $source and copy it
# to $dest, which must be <= $source but which need not be <= $source - $len
# (That is, this will only copy a block to a position earlier in the file,
# but the source and destination regions may overlap.)


my $file = "tf29a-$$.txt";

print "1..55\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

$: = Tie::File::_default_recsep();

my @subtests = qw(x <x x> x><x <x> <x><x x><x> <x><x> <x><x><x> 0);

my $FLEN = 40970;  # 2410 records of 17 chars each

# (2-7) Trivial non-moves at start of file
try(0, 0, 0);
try(0, 0, 10);
try(0, 0, 100);
try(0, 0, 1000);
try(0, 0, 10000);
try(0, 0, 20000);

# (8-13) Trivial non-moves in middle of file
try(100, 100, 0);
try(100, 100, 10);
try(100, 100, 100);
try(100, 100, 1000);
try(100, 100, 10000);
try(100, 100, 20000);

# (14) Trivial non-move at end of file
try($FLEN, $FLEN, 0);

# (15-17) Trivial non-move of tail of file
try(0, 0, undef);
try(100, 100, undef);
try($FLEN, $FLEN, undef);

# (18-24) Moves to start of file
try(100, 0, 0);
try(100, 0, 10);
try(100, 0, 100);
try(100, 0, 1000);
try(100, 0, 10000);
try(100, 0, 20000);
try(100, 0, undef);

# (25-31) Moves in middle of file
try(200, 100, 0);
try(200, 100, 10);
try(200, 100, 100);
try(200, 100, 1000);
try(200, 100, 10000);
try(200, 100, 20000);
try(200, 100, undef);

# (32-43) Moves from end of file
try($FLEN, 10000, 0);
try($FLEN-10, 10000, 10);
try($FLEN-100, 10000, 100);
try($FLEN-1000, 200, 1000);
try($FLEN-10000, 200, 10000);
try($FLEN-20000, 200, 20000);
try($FLEN, 10000, undef);
try($FLEN-10, 10000, undef);
try($FLEN-100, 10000, undef);
try($FLEN-1000, 200, undef);
try($FLEN-10000, 200, undef);
try($FLEN-20000, 200, undef);

$FLEN = 40960;

# (44-55) Moves from end of file when file ends on a block boundary
try($FLEN, 10000, 0);
try($FLEN-10, 10000, 10);
try($FLEN-100, 10000, 100);
try($FLEN-1000, 200, 1000);
try($FLEN-10000, 200, 10000);
try($FLEN-20000, 200, 20000);
try($FLEN, 10000, undef);
try($FLEN-10, 10000, undef);
try($FLEN-100, 10000, undef);
try($FLEN-1000, 200, undef);
try($FLEN-10000, 200, undef);
try($FLEN-20000, 200, undef);

sub try {
  my ($src, $dst, $len) = @_;
  open F, '>', $file or die "Couldn't open file $file: $!";
  binmode F;

  # The record has exactly 17 characters.  This will help ensure that
  # even if _upcopy screws up, the data doesn't coincidentally
  # look good because the remainder accidentally lines up.
  my $d = substr("0123456789abcdef$:", -17);
  my $recs = defined($FLEN) ?
    int($FLEN/length($d))+1 : # enough to make up at least $FLEN
    int(8192*5/length($d))+1; # at least 5 blocks' worth
  my $oldfile = $d x $recs;
  my $flen = defined($FLEN) ? $FLEN : $recs * 17;
  substr($oldfile, $FLEN) = "" if defined $FLEN;  # truncate
  print F $oldfile;
  close F;

  die "wrong length!" unless -s $file == $flen;

  # If len is specified, use that.  If it's undef,
  # then behave *as if* we had specified the whole rest of the file
  my $expected = $oldfile;
  if (defined $len) {
    substr($expected, $dst, $len) = substr($expected, $src, $len);
  } else {
    substr($expected, $dst) = substr($expected, $src);
  }

  my $o = tie my @lines, 'Tie::File', $file or die $!;
  # allocate more time for the test if we are running parallel tests
  my $alarm_time = ($ENV{TEST_JOBS} || $ENV{HARNESS_OPTIONS}) ? 20 : 10;
  local $SIG{ALRM} = sub { die "Alarm clock" };
  my $a_retval = eval { alarm($alarm_time) unless $^P; $o->_upcopy($src, $dst, $len) };
  my $err = $@;
  undef $o; untie @lines; alarm(0);
  if ($err) {
    if ($err =~ /^Alarm clock/) {
      print STDERR "# $0 Timeout after $alarm_time seconds at test $N\n";
      print "not ok $N\n"; $N++;
      return;
    } else {
      $@ = $err;
      die;
    }
  }

  open F, '<', $file or die "Couldn't open file $file: $!";
  binmode F;
  my $actual;
  { local $/;
    $actual = <F>;
  }
  close F;

  my ($alen, $xlen) = (length $actual, length $expected);
  unless ($alen == $xlen) {
    print "# try(@_) expected file length $xlen, actual $alen!\n";
  }
  my $desc = sprintf "try(%d, %d, %s)",
                $src, $dst, (defined $len ? $len : "undef");
  print $actual eq $expected ? "ok $N - $desc\n" : "not ok $N - $desc\n";
  $N++;
}

sub ctrlfix {
  for (@_) {
    s/\n/\\n/g;
    s/\r/\\r/g;
  }
}

END {
  1 while unlink $file;
}
