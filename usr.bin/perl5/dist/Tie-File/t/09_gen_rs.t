#!/usr/bin/perl

use strict;
use warnings;

my $file = "tf09-$$.txt";

print "1..59\n";

use Fcntl 'O_RDONLY';

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

my $RECSEP = 'blah';

my @a;
my $o = tie @a, 'Tie::File', $file, 
    recsep => $RECSEP, autochomp => 0, autodefer => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;


# 3-4 create
$a[0] = 'rec0';
check_contents("rec0");

# 5-8 append
$a[1] = 'rec1';
check_contents("rec0", "rec1");
$a[2] = 'rec2';
check_contents("rec0", "rec1", "rec2");

# 9-14 same-length alterations
$a[0] = 'new0';
check_contents("new0", "rec1", "rec2");
$a[1] = 'new1';
check_contents("new0", "new1", "rec2");
$a[2] = 'new2';
check_contents("new0", "new1", "new2");

# 15-24 lengthening alterations
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

# 25-34 shortening alterations, including truncation
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

# (35-38) file with holes
$a[4] = 'rec4';
check_contents("sh0", "sh1", "short2", "", "rec4");
$a[3] = 'rec3';
check_contents("sh0", "sh1", "short2", "rec3", "rec4");

# (39-40) zero out file
@a = ();
check_contents();

# (41-42) insert into the middle of an empty file
$a[3] = "rec3";
check_contents("", "", "", "rec3");

# (43-47) 20020326 You thought there would be a bug in STORE where if
# a cached record was false, STORE wouldn't see it at all.  Yup, there is,
# and adding the appropriate defined() test fixes the problem.
undef $o;  untie @a;  1 while unlink $file;
$RECSEP = '0';
$o = tie @a, 'Tie::File', $file, 
    recsep => $RECSEP, autochomp => 0, autodefer => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;
$#a = 2;
my $z = $a[1];                  # caches "0"
$a[2] = "oops";
check_contents("", "", "oops");
$a[1] = "bah";
check_contents("", "bah", "oops");
undef $o; untie @a;

# (48-56) 20020331 Make sure we correctly handle the case where the final
# record of the file is not properly terminated, Through version 0.90,
# we would mangle the file.
my $badrec = "Malformed";
$: = $RECSEP = Tie::File::_default_recsep();
# (48-50)
if (setup_badly_terminated_file(3)) {
  $o = tie @a, 'Tie::File', $file,
    recsep => $RECSEP, autochomp => 0, autodefer => 0
    or die "Couldn't tie file: $!";
  my $z = $a[0];
  print $z eq "$badrec$:" ? "ok $N\n" : 
                        "not ok $N \# got $z, expected $badrec\n";
  $N++;
  push @a, "next";
  check_contents($badrec, "next");
}
# (51-52)
if (setup_badly_terminated_file(2)) {
  $o = tie @a, 'Tie::File', $file,
    recsep => $RECSEP, autochomp => 0, autodefer => 0
    or die "Couldn't tie file: $!";
  splice @a, 1, 0, "x", "y";
  check_contents($badrec, "x", "y");
}
# (53-56)
if (setup_badly_terminated_file(4)) {
  $o = tie @a, 'Tie::File', $file,
    recsep => $RECSEP, autochomp => 0, autodefer => 0
    or die "Couldn't tie file: $!";
  my @r = splice @a, 0, 1, "x", "y";
  my $n = @r;
  print $n == 1 ? "ok $N\n" : "not ok $N \# expected 1 elt, got $n\n";
  $N++;
  print $r[0] eq "$badrec$:" ? "ok $N\n"
    : "not ok $N \# expected <$badrec>, got <$r[0]>\n";
  $N++;
  check_contents("x", "y");
}

# (57-58) 20020402 The modification would have failed if $\ were set wrong.
# I hate $\.
if (setup_badly_terminated_file(2)) {
  $o = tie @a, 'Tie::File', $file,
    recsep => $RECSEP, autochomp => 0, autodefer => 0
    or die "Couldn't tie file: $!";
  { local $\ = "I hate \$\\.";
    my $z = $a[0];
  }
  check_contents($badrec);
}

# (59) 20030527 Tom Christiansen pointed out that FETCH returns the wrong
# data on the final record of an unterminated file if the file is opened
# in read-only mode.  Note that the $#a is necessary here.
# There's special-case code to fix the final record when it is read normally.
# But the $#a forces it to be read from the cache, which skips the
# termination.
$badrec = "world${RECSEP}hello";
if (setup_badly_terminated_file(1)) {
  tie(@a, "Tie::File", $file, mode => O_RDONLY, recsep => $RECSEP)
      or die "Couldn't tie file: $!";
  my $z = $#a;
  $z = $a[1];
  print $z eq "hello" ? "ok $N\n" : 
      "not ok $N \# got $z, expected hello\n";
  $N++;
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


use POSIX 'SEEK_SET';
sub check_contents {
  my @c = @_;
  my $x = join $RECSEP, @c, '';
  local *FH = $o->{fh};
  seek FH, 0, SEEK_SET;
  my $a;
  { local $/; $a = <FH> }

  $a = "" unless defined $a;
  if ($a eq $x) {
    print "ok $N\n";
  } else {
    my $msg = "# expected <$x>, got <$a>";
    ctrlfix($msg);
    print "not ok $N $msg\n";
  }
  $N++;

  # now check FETCH:
  my $good = 1;
  my $msg = '';
  for (0.. $#c) {
    unless ($a[$_] eq "$c[$_]$RECSEP") {
      $msg = "expected $c[$_]$RECSEP, got $a[$_]";
      ctrlfix($msg);
      $good = 0;
    }
  }
  print $good ? "ok $N\n" : "not ok $N # fetch $msg\n";
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

