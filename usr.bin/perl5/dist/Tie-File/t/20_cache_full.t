#!/usr/bin/perl

use strict;
use warnings;

#
# Tests for various caching errors
#

my $file = "tf20-$$.txt";
$: = Tie::File::_default_recsep();
my $data = join $:, "record0" .. "record9", "";
my $V = $ENV{INTEGRITY};        # Verbose integrity checking?

print "1..111\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

open F, '>', $file or die $!;
binmode F;
print F $data;
close F;

# Limit cache size to 30 bytes 
my $MAX = 30;
#  -- that's enough space for 3 records, but not 4, on both \n and \r\n systems
my @a;
my $o = tie @a, 'Tie::File', $file, memory => $MAX, autodefer => 0;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

# (3-5) Let's see if data was properly expired from the cache
my @z = @a;                     # force cache to contain all ten records
# It should now contain only the *last* three records, 7, 8, and 9
{
  my $x = "7 8 9";
  my $a = join " ", sort $o->{cache}->ckeys;
  if ($a eq $x) { print "ok $N\n" }
  else { print "not ok $N # cache keys were <$a>; expected <$x>\n" }
  $N++;
}
check();

# Here we redo *all* the splice tests, with populate()
# calls before each one, to make sure that splice() does not botch the cache.

# (6-25) splicing at the beginning
splice(@a, 0, 0, "rec4");
check();
splice(@a, 0, 1, "rec5");       # same length
check();
splice(@a, 0, 1, "record5");    # longer
check();
splice(@a, 0, 1, "r5");         # shorter
check();
splice(@a, 0, 1);               # removal
check();
splice(@a, 0, 0);               # no-op
check();

splice(@a, 0, 0, 'r7', 'rec8'); # insert more than one
check();
splice(@a, 0, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check();
splice(@a, 0, 3, 'record9', 'rec10'); # delete more than insert
check();
splice(@a, 0, 2);               # delete more than one
check();


# (26-45) splicing in the middle
splice(@a, 1, 0, "rec4");
check();
splice(@a, 1, 1, "rec5");       # same length
check();
splice(@a, 1, 1, "record5");    # longer
check();
splice(@a, 1, 1, "r5");         # shorter
check();
splice(@a, 1, 1);               # removal
check();
splice(@a, 1, 0);               # no-op
check();

splice(@a, 1, 0, 'r7', 'rec8'); # insert more than one
check();
splice(@a, 1, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check();
splice(@a, 1, 3, 'record9', 'rec10'); # delete more than insert
check();
splice(@a, 1, 2);               # delete more than one
check();

# (46-65) splicing at the end
splice(@a, 3, 0, "rec4");
check();
splice(@a, 3, 1, "rec5");       # same length
check();
splice(@a, 3, 1, "record5");    # longer
check();
splice(@a, 3, 1, "r5");         # shorter
check();
splice(@a, 3, 1);               # removal
check();
splice(@a, 3, 0);               # no-op
check();

splice(@a, 3, 0, 'r7', 'rec8'); # insert more than one
check();
splice(@a, 3, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check();
splice(@a, 3, 3, 'record9', 'rec10'); # delete more than insert
check();
splice(@a, 3, 2);               # delete more than one
check();

# (66-85) splicing with negative subscript
splice(@a, -1, 0, "rec4");
check();
splice(@a, -1, 1, "rec5");       # same length
check();
splice(@a, -1, 1, "record5");    # longer
check();
splice(@a, -1, 1, "r5");         # shorter
check();
splice(@a, -1, 1);               # removal
check();
splice(@a, -1, 0);               # no-op  
check();

splice(@a, -1, 0, 'r7', 'rec8'); # insert more than one
check();
splice(@a, -1, 2, 'rec7', 'record8', 'rec9'); # insert more than delete
check();
splice(@a, -3, 3, 'record9', 'rec10'); # delete more than insert
check();
splice(@a, -4, 3);               # delete more than one
check();

# (86-87) scrub it all out
splice(@a, 0, 3);
check();

# (88-89) put some back in
splice(@a, 0, 0, "rec0", "rec1");
check();

# (90-91) what if we remove too many records?
splice(@a, 0, 17);
check();

# (92-95) In the past, splicing past the end was not correctly detected
# (1.14)
splice(@a, 89, 3);
check();
splice(@a, @a, 3);
check();

# (96-99) Also we did not emulate splice's freaky behavior when inserting
# past the end of the array (1.14)
splice(@a, 89, 0, "I", "like", "pie");
check();
splice(@a, 89, 0, "pie pie pie");
check();

# (100-105) Test default arguments
splice @a, 0, 0, (0..11);
check();
splice @a, 4;
check();
splice @a;
check();

# (106-111) One last set of tests.  I don't know what state the cache
# is in now.  But if I read any three records, those three records are
# what should be in the cache, and nothing else.
@a = "record0" .. "record9";
check(); # In 0.18 #107 fails here--STORE was not flushing the cache when
         # replacing an old cached record with a longer one
for (5, 6, 1) { my $z = $a[$_] }
{
  my $x = "5 6 1";
  my $a = join " ", $o->{cache}->_produce_lru;
  if ($a eq $x) { print "ok $N\n" }
  else { print "not ok $N # LRU was <$a>; expected <$x>\n" }
  $N++;
  $x = "1 5 6";
  $a = join " ", sort $o->{cache}->ckeys;
  if ($a eq $x) { print "ok $N\n" }
  else { print "not ok $N # cache keys were <$a>; expected <$x>\n" }
  $N++;
}
check();


sub init_file {
  my $data = shift;
  open F, '>', $file or die $!;
  binmode F;
  print F $data;
  close F;
}

sub check {
  my $integrity = $o->_check_integrity($file, $ENV{INTEGRITY});
  print $integrity ? "ok $N\n" : "not ok $N\n";
  $N++;

  my $b = $o->{cache}->bytes;
  print $b <= $MAX 
    ? "ok $N\n" 
    : "not ok $N # $b bytes cached, should be <= $MAX\n";
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



