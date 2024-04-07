#!/usr/bin/perl
#
# Check PUSH, POP, SHIFT, and UNSHIFT 
#
# Each call to 'check_contents' actually performs two tests.
# First, it calls the tied object's own 'check_integrity' method,
# which makes sure that the contents of the read cache and offset tables
# accurately reflect the contents of the file.  
# Then, it checks the actual contents of the file against the expected
# contents.

use strict;
use warnings;

use POSIX 'SEEK_SET';

my $file = "tf15-$$.txt";
1 while unlink $file;
$: = Tie::File::_default_recsep();
my $data = "rec0$:rec1$:rec2$:";

print "1..38\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;  # partial credit just for showing up

my @a;
my $o = tie @a, 'Tie::File', $file, autochomp => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;
my ($n, @r);


# (3-11) PUSH tests
$n = push @a, "rec0", "rec1", "rec2";
check_contents($data);
print $n == 3 ? "ok $N\n" : "not ok $N # size is $n, should be 3\n";
$N++;

$n = push @a, "rec3", "rec4$:";
check_contents("$ {data}rec3$:rec4$:");
print $n == 5 ? "ok $N\n" : "not ok $N # size is $n, should be 5\n";
$N++;

# Trivial push
$n = push @a, ();
check_contents("$ {data}rec3$:rec4$:");
print $n == 5 ? "ok $N\n" : "not ok $N # size is $n, should be 5\n";
$N++;

# (12-20) POP tests
$n = pop @a;
check_contents("$ {data}rec3$:");
print $n eq "rec4$:" ? "ok $N\n" : "not ok $N # last rec is $n, should be rec4\n";
$N++;

# Presumably we have already tested this to death
splice(@a, 1, 3);
$n = pop @a;
check_contents("");
print $n eq "rec0$:" ? "ok $N\n" : "not ok $N # last rec is $n, should be rec0\n";
$N++;

$n = pop @a;
check_contents("");
print ! defined $n ? "ok $N\n" : "not ok $N # last rec should be undef, is $n\n";
$N++;


# (21-29) UNSHIFT tests
$n = unshift @a, "rec0", "rec1", "rec2";
check_contents($data);
print $n == 3 ? "ok $N\n" : "not ok $N # size is $n, should be 3\n";
$N++;

$n = unshift @a, "rec3", "rec4$:";
check_contents("rec3$:rec4$:$data");
print $n == 5 ? "ok $N\n" : "not ok $N # size is $n, should be 5\n";
$N++;

# Trivial unshift
$n = unshift @a, ();
check_contents("rec3$:rec4$:$data");
print $n == 5 ? "ok $N\n" : "not ok $N # size is $n, should be 5\n";
$N++;

# (30-38) SHIFT tests
$n = shift @a;
check_contents("rec4$:$data");
print $n eq "rec3$:" ? "ok $N\n" : "not ok $N # last rec is $n, should be rec3\n";
$N++;

# Presumably we have already tested this to death
splice(@a, 1, 3);
$n = shift @a;
check_contents("");
print $n eq "rec4$:" ? "ok $N\n" : "not ok $N # last rec is $n, should be rec4\n";
$N++;

$n = shift @a;
check_contents("");
print ! defined $n ? "ok $N\n" : "not ok $N # last rec should be undef, is $n\n";
$N++;


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

