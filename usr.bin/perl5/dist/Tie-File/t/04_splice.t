#!/usr/bin/perl

#
# Check SPLICE function's effect on the file
# (07_rv_splice.t checks its return value)
#
# Each call to 'check_contents' actually performs two tests.
# First, it calls the tied object's own 'check_integrity' method,
# which makes sure that the contents of the read cache and offset tables
# accurately reflect the contents of the file.  
# Then, it checks the actual contents of the file against the expected
# contents.


use strict;
use warnings;

$| = 1;
my $file = "tf04-$$.txt";
$: = Tie::File::_default_recsep();
my $data = "rec0$:rec1$:rec2$:";
print "1..118\n";

init_file($data);

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;  # partial credit just for showing up

my @a;
my $o = tie @a, 'Tie::File', $file;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

$: = $o->{recsep};
my $n;

# (3-22) splicing at the beginning
splice(@a, 0, 0, "rec4");
check_contents("rec4$:$data");
splice(@a, 0, 1, "rec5");       # same length
check_contents("rec5$:$data");
splice(@a, 0, 1, "record5");    # longer
check_contents("record5$:$data");

splice(@a, 0, 1, "r5");         # shorter
check_contents("r5$:$data");
splice(@a, 0, 1);               # removal
check_contents("$data");
splice(@a, 0, 0);               # no-op
check_contents("$data");
splice(@a, 0, 0, 'r7', 'rec8'); # insert more than one
check_contents("r7$:rec8$:$data");
splice(@a, 0, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check_contents("rec7$:record8$:rec9$:$data");

splice(@a, 0, 3, 'record9', 'rec10'); # delete more than insert
check_contents("record9$:rec10$:$data");
splice(@a, 0, 2);               # delete more than one
check_contents("$data");


# (23-42) splicing in the middle
splice(@a, 1, 0, "rec4");
check_contents("rec0$:rec4$:rec1$:rec2$:");
splice(@a, 1, 1, "rec5");       # same length
check_contents("rec0$:rec5$:rec1$:rec2$:");
splice(@a, 1, 1, "record5");    # longer
check_contents("rec0$:record5$:rec1$:rec2$:");

splice(@a, 1, 1, "r5");         # shorter
check_contents("rec0$:r5$:rec1$:rec2$:");
splice(@a, 1, 1);               # removal
check_contents("$data");
splice(@a, 1, 0);               # no-op
check_contents("$data");
splice(@a, 1, 0, 'r7', 'rec8'); # insert more than one
check_contents("rec0$:r7$:rec8$:rec1$:rec2$:");
splice(@a, 1, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check_contents("rec0$:rec7$:record8$:rec9$:rec1$:rec2$:");

splice(@a, 1, 3, 'record9', 'rec10'); # delete more than insert
check_contents("rec0$:record9$:rec10$:rec1$:rec2$:");
splice(@a, 1, 2);               # delete more than one
check_contents("$data");

# (43-62) splicing at the end
splice(@a, 3, 0, "rec4");
check_contents("$ {data}rec4$:");
splice(@a, 3, 1, "rec5");       # same length
check_contents("$ {data}rec5$:");
splice(@a, 3, 1, "record5");    # longer
check_contents("$ {data}record5$:");

splice(@a, 3, 1, "r5");         # shorter
check_contents("$ {data}r5$:");
splice(@a, 3, 1);               # removal
check_contents("$data");
splice(@a, 3, 0);               # no-op
check_contents("$data");
splice(@a, 3, 0, 'r7', 'rec8'); # insert more than one
check_contents("$ {data}r7$:rec8$:");
splice(@a, 3, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check_contents("$ {data}rec7$:record8$:rec9$:");

splice(@a, 3, 3, 'record9', 'rec10'); # delete more than insert
check_contents("$ {data}record9$:rec10$:");
splice(@a, 3, 2);               # delete more than one
check_contents("$data");

# (63-82) splicing with negative subscript
splice(@a, -1, 0, "rec4");
check_contents("rec0$:rec1$:rec4$:rec2$:");
splice(@a, -1, 1, "rec5");       # same length
check_contents("rec0$:rec1$:rec4$:rec5$:");
splice(@a, -1, 1, "record5");    # longer
check_contents("rec0$:rec1$:rec4$:record5$:");

splice(@a, -1, 1, "r5");         # shorter
check_contents("rec0$:rec1$:rec4$:r5$:");
splice(@a, -1, 1);               # removal
check_contents("rec0$:rec1$:rec4$:");
splice(@a, -1, 0);               # no-op  
check_contents("rec0$:rec1$:rec4$:");
splice(@a, -1, 0, 'r7', 'rec8'); # insert more than one
check_contents("rec0$:rec1$:r7$:rec8$:rec4$:");
splice(@a, -1, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check_contents("rec0$:rec1$:r7$:rec8$:rec7$:record8$:rec9$:");

splice(@a, -3, 3, 'record9', 'rec10'); # delete more than insert
check_contents("rec0$:rec1$:r7$:rec8$:record9$:rec10$:");
splice(@a, -4, 3);               # delete more than one
check_contents("rec0$:rec1$:rec10$:");

# (83-84) scrub it all out
splice(@a, 0, 3);
check_contents("");

# (85-86) put some back in
splice(@a, 0, 0, "rec0", "rec1");
check_contents("rec0$:rec1$:");

# (87-88) what if we remove too many records?
splice(@a, 0, 17);
check_contents("");

# (89-92) In the past, splicing past the end was not correctly detected
# (1.14)
splice(@a, 89, 3);
check_contents("");
splice(@a, @a, 3);
check_contents("");

# (93-96) Also we did not emulate splice's freaky behavior when inserting
# past the end of the array (1.14)
splice(@a, 89, 0, "I", "like", "pie");
check_contents("I$:like$:pie$:");
splice(@a, 89, 0, "pie pie pie");
check_contents("I$:like$:pie$:pie pie pie$:");

# (97) Splicing with too large a negative number should be fatal
# This test ignored because it causes 5.6.1 and 5.7.3 to dump core
# It also garbles the stack under 5.005_03 (20020401)
# NOT MY FAULT
if ($] > 5.007003) {
  eval { splice(@a, -7, 0) };
  print $@ =~ /^Modification of non-creatable array value attempted, subscript -7/
      ? "ok $N\n" : "not ok $N \# \$\@ was '$@'\n";
} else { 
  print "ok $N \# skipped (versions through 5.7.3 dump core here.)\n";
}
$N++;
       
# (98-101) Test default arguments
splice @a, 0, 0, (0..11);
splice @a, 4;
check_contents("0$:1$:2$:3$:");
splice @a;
check_contents("");

# (102-103) I think there's a bug here---it will fail to clear the EOF flag
@a = (0..11);
splice @a, -1, 1000;
check_contents("0$:1$:2$:3$:4$:5$:6$:7$:8$:9$:10$:");

# (104-106) make sure that undefs are treated correctly---they should
# be converted to empty records, and should not raise any warnings.
# (Some of these failed in 0.90.  The change to _fixrec fixed them.)
# 20020331
{
  my $good = 1; my $warn;
  # If any of these raise warnings, we have a problem.
  local $SIG{__WARN__} = sub { $good = 0; $warn = shift(); ctrlfix($warn)};
  local $^W = 1;
  @a = (1);
  splice @a, 1, 0, undef, undef, undef;
  print $good ? "ok $N\n" : "not ok $N # $warn\n";
  $N++; $good = 1;
  print defined($a[2]) ? "ok $N\n" : "not ok $N\n";
  $N++; $good = 1;
  my @r = splice @a, 2;
  print defined($r[0]) ? "ok $N\n" : "not ok $N\n";
  $N++; $good = 1;
}

# (107-118) splice with negative length was treated wrong
# 20020402 Reported by Juerd Waalboer
@a = (0..8) ;
splice @a, 0, -3;
check_contents("6$:7$:8$:");
@a = (0..8) ;
splice @a, 1, -3;
check_contents("0$:6$:7$:8$:");
@a = (0..8) ;
splice @a, 7, -3;
check_contents("0$:1$:2$:3$:4$:5$:6$:7$:8$:");
@a = (0..2) ;
splice @a, 0, -3;
check_contents("0$:1$:2$:");
@a = (0..2) ;
splice @a, 1, -3;
check_contents("0$:1$:2$:");
@a = (0..2) ;
splice @a, 7, -3;
check_contents("0$:1$:2$:");

sub init_file {
  my $data = shift;
  open F, '>', $file or die $!;
  binmode F;
  print F $data;
  close F;
}

use POSIX 'SEEK_SET';
sub check_contents {
  my $x = shift;
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
    ctrlfix($a, $x);
    print "not ok $N\n# expected <$x>, got <$a>\n";
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

