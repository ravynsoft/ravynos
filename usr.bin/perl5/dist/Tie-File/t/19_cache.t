#!/usr/bin/perl

use strict;
use warnings;

#
# Tests for various caching errors
#

$|=1;
my $file = "tf19-$$.txt";
$: = Tie::File::_default_recsep();
my $data = join $:, "rec0" .. "rec9", "";
my $V = $ENV{INTEGRITY};        # Verbose integrity checking?

print "1..55\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

open F, '>', $file or die $!;
binmode F;
print F $data;
close F;

my @a;
my $o = tie @a, 'Tie::File', $file;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

# (3) Through 0.18, this 'splice' call would corrupt the cache.
my @z = @a;                     # force cache to contain all ten records
splice @a, 0, 0, "x";
print $o->_check_integrity($file, $V) ? "ok $N\n" : "not ok $N\n";
$N++;

# Here we redo *all* the splice tests, with populate()
# calls before each one, to make sure that splice() does not botch the cache.

# (4-14) splicing at the beginning
check();
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


# (15-24) splicing in the middle
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

# (25-34) splicing at the end
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

# (35-44) splicing with negative subscript
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

# (45) scrub it all out
splice(@a, 0, 3);
check();

# (46) put some back in
splice(@a, 0, 0, "rec0", "rec1");
check();

# (47) what if we remove too many records?
splice(@a, 0, 17);
check();

# (48-49) In the past, splicing past the end was not correctly detected
# (1.14)
splice(@a, 89, 3);
check();
splice(@a, @a, 3);
check();

# (50-51) Also we did not emulate splice's freaky behavior when inserting
# past the end of the array (1.14)
splice(@a, 89, 0, "I", "like", "pie");
check();
splice(@a, 89, 0, "pie pie pie");
check();

# (52-54) Test default arguments
splice @a, 0, 0, (0..11);
check();
splice @a, 4;
check();
splice @a;
check();
    
# (55) This was broken on 20030507 when you moved the cache management
# stuff out of _oadjust back into _splice without also putting it back 
# into _store.
@a = (0..11);
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
  repopulate();
}


sub ctrlfix {
  for (@_) {
    s/\n/\\n/g;
    s/\r/\\r/g;
  }
}

sub repopulate {
  $o->{cache}->empty;
  my @z = @a;                   # refill the cache with correct data
}

END {
  undef $o;
  untie @a;
  1 while unlink $file;
}



