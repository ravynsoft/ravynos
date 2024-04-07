#!/usr/bin/perl
#
# Check FETCHSIZE and SETSIZE functions
# PUSH POP SHIFT UNSHIFT
#

use strict;
use warnings;

use POSIX 'SEEK_SET';

my $file = "tf05-$$.txt";
my ($o, $n);

print "1..16\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

# 2-3 FETCHSIZE 0-length file
open F, '>', $file or die $!;
binmode F;
close F;

my @a;
$o = tie @a, 'Tie::File', $file;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

$: = $o->{recsep};

$n = @a;
print $n == 0 ? "ok $N\n" : "not ok $N # $n, s/b 0\n";
$N++;

# Reset everything
undef $o;
untie @a;

my $data = "rec0$:rec1$:rec2$:";
open F, '>', $file or die $!;
binmode F;
print F $data;
close F;

$o = tie @a, 'Tie::File', $file;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

# 4-5 FETCHSIZE positive-length file
$n = @a;
print $n == 3 ? "ok $N\n" : "not ok $N # $n, s/b 0\n";
$N++;

# STORESIZE
# (6-7) Make it longer:
populate();
$#a = 4;
check_contents("$data$:$:");

# (8-9) Make it longer again:
populate();
$#a = 6;
check_contents("$data$:$:$:$:");

# (10-11) Make it shorter:
populate();
$#a = 4;
check_contents("$data$:$:");

# (12-13) Make it shorter again:
populate();
$#a = 2;
check_contents($data);

# (14-15) Get rid of it completely:
populate();
$#a = -1;
check_contents('');

# (16) 20020324 I have an idea that shortening the array will not
# expunge a cached record at the end if one is present.
$o->defer;
$a[3] = "record";
my $r = $a[3];
$#a = -1;
$r = $a[3];
print (! defined $r ? "ok $N\n" : "not ok $N \# was <$r>; should be UNDEF\n");
# Turns out not to be the case---STORESIZE explicitly removes them later
# 20020326 Well, but happily, this test did fail today.

# In the past, there was a bug in STORESIZE that it didn't correctly
# remove deleted records from the cache.  This wasn't detected
# because these tests were all done with an empty cache.  populate()
# will ensure that the cache is fully populated.
sub populate {
  my $z;
  $z = $a[$_] for 0 .. $#a;
}

sub check_contents {
  my $x = shift;
  local *FH = $o->{fh};
  seek FH, 0, SEEK_SET;
  my $a;
  { local $/; $a = <FH> }
  $a = "" unless defined $a;
  if ($a eq $x) {
    print "ok $N\n";
  } else {
    ctrlfix($a, $x);
    print "not ok $N\n# expected <$x>, got <$a>\n";
  }
  $N++;
  my $integrity = $o->_check_integrity($file, $ENV{INTEGRITY});
  print $integrity ? "ok $N\n" : "not ok $N \# integrity\n";
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

