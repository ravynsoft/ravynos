#!/usr/bin/perl

use strict;
use warnings;

#
# Check behavior of 'autodefer' feature
# Mostly this isn't implemented yet
# This file is primarily here to make sure that the promised ->autodefer
# method doesn't croak.
#

use POSIX 'SEEK_SET';

my $file = "tf31-$$.txt";
$: = Tie::File::_default_recsep();
my $data = "rec0$:rec1$:rec2$:";
my ($o, $n, @a);

print "1..65\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

open F, '>', $file or die $!;
binmode F;
print F $data;
close F;
$o = tie @a, 'Tie::File', $file;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

# I am an undocumented feature
$o->{autodefer_filelen_threshhold} = 0;
# Normally autodeferring only works on large files.  This disables that.

# (3-22) Deferred storage
$a[3] = "rec3";
check_autodeferring('OFF');
$a[4] = "rec4";
check_autodeferring('OFF');
$a[5] = "rec5";
check_autodeferring('ON');
check_contents($data . "rec3$:rec4$:"); # only the first two were written
$a[6] = "rec6";
check_autodeferring('ON');
check_contents($data . "rec3$:rec4$:"); # still nothing written
$a[7] = "rec7";
check_autodeferring('ON');
check_contents($data . "rec3$:rec4$:"); # still nothing written
$a[0] = "recX";
check_autodeferring('OFF');
check_contents("recX$:rec1$:rec2$:rec3$:rec4$:rec5$:rec6$:rec7$:");
$a[1] = "recY";
check_autodeferring('OFF');
check_contents("recX$:recY$:rec2$:rec3$:rec4$:rec5$:rec6$:rec7$:");
$a[2] = "recZ";                 # it kicks in here
check_autodeferring('ON');
check_contents("recX$:recY$:rec2$:rec3$:rec4$:rec5$:rec6$:rec7$:");

# (23-26) Explicitly enabling deferred writing deactivates autodeferring
$o->defer;
check_autodeferring('OFF');
check_contents("recX$:recY$:recZ$:rec3$:rec4$:rec5$:rec6$:rec7$:");
$o->discard;
check_autodeferring('OFF');

# (27-32) Now let's try the CLEAR special case
@a = ("r0" .. "r4");
check_autodeferring('ON');
# The file was extended to the right length, but nothing was actually written.
check_contents("$:$:$:$:$:");
$a[2] = "fish";
check_autodeferring('OFF');
check_contents("r0$:r1$:fish$:r3$:r4$:");

# (33-47) Now let's try the originally intended application:  a 'for' loop.
my $it = 0;
for (@a) {
  $_ = "##$_";
  if ($it == 0) {
    check_autodeferring('OFF');
    check_contents("##r0$:r1$:fish$:r3$:r4$:");
  } elsif ($it == 1) {
    check_autodeferring('OFF');
    check_contents("##r0$:##r1$:fish$:r3$:r4$:");
  } else {
    check_autodeferring('ON');
    check_contents("##r0$:##r1$:fish$:r3$:r4$:");
  }
  $it++;
}

# (48-56) Autodeferring should not become active during explicit defer mode
$o->defer();  # This should flush the pending autodeferred records
              # and deactivate autodeferring
check_autodeferring('OFF');
check_contents("##r0$:##r1$:##fish$:##r3$:##r4$:");
@a = ("s0" .. "s4");
check_autodeferring('OFF');
check_contents("");
$o->flush;
check_autodeferring('OFF');
check_contents("s0$:s1$:s2$:s3$:s4$:");

undef $o; untie @a;

# Limit cache+buffer size to 47 bytes 
my $MAX = 47;
#  -- that's enough space for 5 records, but not 6, on both \n and \r\n systems
my $BUF = 20;
#  -- that's enough space for 2 records, but not 3, on both \n and \r\n systems
# Re-tie the object for more tests
$o = tie @a, 'Tie::File', $file, autodefer => 0;
die $! unless $o;
# I am an undocumented feature
$o->{autodefer_filelen_threshhold} = 0;
# Normally autodeferring only works on large files.  This disables that.

# (57-59) Did the autodefer => 0 option work?
# (If it doesn't, a whole bunch of the other test files will fail.)
@a = (0..3);
check_autodeferring('OFF');
check_contents(join("$:", qw(0 1 2 3), ""));

# (60-62) Does the ->autodefer method work?
$o->autodefer(1);
@a = (10..13);
check_autodeferring('ON');
check_contents("$:$:$:$:");  # This might be unfortunate.

# (63-65) Does the ->autodefer method work?
$o->autodefer(0);
check_autodeferring('OFF');
check_contents(join("$:", qw(10 11 12 13), ""));


sub check_autodeferring {
  my ($x) = shift;
  my $a = $o->{autodeferring} ? 'ON' : 'OFF';
  if ($x eq $a) {
    print "ok $N\n";
  } else {
    print "not ok $N \# Autodeferring was $a, expected it to be $x\n";
  }
  $N++;
}


sub check_contents {
  my $x = shift;
#  for (values %{$o->{cache}}) {
#    print "# cache=$_";    
#  }
  
  my $integrity = $o->_check_integrity($file, $ENV{INTEGRITY});
  local *FH = $o->{fh};
  seek FH, 0, SEEK_SET;
  print $integrity ? "ok $N\n" : "not ok $N\n";
  $N++;
  my $a;
  { local $/; $a = <FH> }
  $a = "" unless defined $a;
  if ($a eq $x) {
    print "ok $N\n";
  } else {
    ctrlfix(my $msg = "# expected <$x>, got <$a>");
    print "not ok $N\n$msg\n";
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

