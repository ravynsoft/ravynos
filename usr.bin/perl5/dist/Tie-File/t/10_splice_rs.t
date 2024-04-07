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

use POSIX 'SEEK_SET';

my $file = "tf10-$$.txt";
my $data = "rec0blahrec1blahrec2blah";

print "1..101\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;  # partial credit just for showing up

init_file($data);

my @a;
my $o = tie @a, 'Tie::File', $file, recsep => 'blah';
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

my $n;

# (3-22) splicing at the beginning
splice(@a, 0, 0, "rec4");
check_contents("rec4blah$data");
splice(@a, 0, 1, "rec5");       # same length
check_contents("rec5blah$data");
splice(@a, 0, 1, "record5");    # longer
check_contents("record5blah$data");

splice(@a, 0, 1, "r5");         # shorter
check_contents("r5blah$data");
splice(@a, 0, 1);               # removal
check_contents("$data");
splice(@a, 0, 0);               # no-op
check_contents("$data");
splice(@a, 0, 0, 'r7', 'rec8'); # insert more than one
check_contents("r7blahrec8blah$data");
splice(@a, 0, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check_contents("rec7blahrecord8blahrec9blah$data");

splice(@a, 0, 3, 'record9', 'rec10'); # delete more than insert
check_contents("record9blahrec10blah$data");
splice(@a, 0, 2);               # delete more than one
check_contents("$data");


# (23-42) splicing in the middle
splice(@a, 1, 0, "rec4");
check_contents("rec0blahrec4blahrec1blahrec2blah");
splice(@a, 1, 1, "rec5");       # same length
check_contents("rec0blahrec5blahrec1blahrec2blah");
splice(@a, 1, 1, "record5");    # longer
check_contents("rec0blahrecord5blahrec1blahrec2blah");

splice(@a, 1, 1, "r5");         # shorter
check_contents("rec0blahr5blahrec1blahrec2blah");
splice(@a, 1, 1);               # removal
check_contents("$data");
splice(@a, 1, 0);               # no-op
check_contents("$data");
splice(@a, 1, 0, 'r7', 'rec8'); # insert more than one
check_contents("rec0blahr7blahrec8blahrec1blahrec2blah");
splice(@a, 1, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check_contents("rec0blahrec7blahrecord8blahrec9blahrec1blahrec2blah");

splice(@a, 1, 3, 'record9', 'rec10'); # delete more than insert
check_contents("rec0blahrecord9blahrec10blahrec1blahrec2blah");
splice(@a, 1, 2);               # delete more than one
check_contents("$data");

# (43-62) splicing at the end
splice(@a, 3, 0, "rec4");
check_contents("$ {data}rec4blah");
splice(@a, 3, 1, "rec5");       # same length
check_contents("$ {data}rec5blah");
splice(@a, 3, 1, "record5");    # longer
check_contents("$ {data}record5blah");

splice(@a, 3, 1, "r5");         # shorter
check_contents("$ {data}r5blah");
splice(@a, 3, 1);               # removal
check_contents("$data");
splice(@a, 3, 0);               # no-op
check_contents("$data");
splice(@a, 3, 0, 'r7', 'rec8'); # insert more than one
check_contents("$ {data}r7blahrec8blah");
splice(@a, 3, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check_contents("$ {data}rec7blahrecord8blahrec9blah");

splice(@a, 3, 3, 'record9', 'rec10'); # delete more than insert
check_contents("$ {data}record9blahrec10blah");
splice(@a, 3, 2);               # delete more than one
check_contents("$data");

# (63-82) splicing with negative subscript
splice(@a, -1, 0, "rec4");
check_contents("rec0blahrec1blahrec4blahrec2blah");
splice(@a, -1, 1, "rec5");       # same length
check_contents("rec0blahrec1blahrec4blahrec5blah");
splice(@a, -1, 1, "record5");    # longer
check_contents("rec0blahrec1blahrec4blahrecord5blah");

splice(@a, -1, 1, "r5");         # shorter
check_contents("rec0blahrec1blahrec4blahr5blah");
splice(@a, -1, 1);               # removal
check_contents("rec0blahrec1blahrec4blah");
splice(@a, -1, 0);               # no-op  
check_contents("rec0blahrec1blahrec4blah");
splice(@a, -1, 0, 'r7', 'rec8'); # insert more than one
check_contents("rec0blahrec1blahr7blahrec8blahrec4blah");
splice(@a, -1, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check_contents("rec0blahrec1blahr7blahrec8blahrec7blahrecord8blahrec9blah");

splice(@a, -3, 3, 'record9', 'rec10'); # delete more than insert
check_contents("rec0blahrec1blahr7blahrec8blahrecord9blahrec10blah");
splice(@a, -4, 3);               # delete more than one
check_contents("rec0blahrec1blahrec10blah");

# (83-84) scrub it all out
splice(@a, 0, 3);
check_contents("");

# (85-86) put some back in
splice(@a, 0, 0, "rec0", "rec1");
check_contents("rec0blahrec1blah");

# (87-88) what if we remove too many records?
splice(@a, 0, 17);
check_contents("");

# (89-92) In the past, splicing past the end was not correctly detected
# (0.14)
splice(@a, 89, 3);
check_contents("");
splice(@a, @a, 3);
check_contents("");

# (93-96) Also we did not emulate splice's freaky behavior when inserting
# past the end of the array (1.14)
splice(@a, 89, 0, "I", "like", "pie");
check_contents("Iblahlikeblahpieblah");
splice(@a, 89, 0, "pie pie pie");
check_contents("Iblahlikeblahpieblahpie pie pieblah");

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
check_contents("0blah1blah2blah3blah");
splice @a;
check_contents("");


sub init_file {
  my $data = shift;
  open F, '>', $file or die $!;
  binmode F;
  print F $data;
  close F;
}

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

