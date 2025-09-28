#!/usr/bin/perl

use strict;
use warnings;

#
# Check ->defer and ->flush methods
#
# This is the old version, which you used in the past when
# there was a defer buffer separate from the read cache.  
# There isn't any longer.
#

use POSIX 'SEEK_SET';
my $file = "tf30-$$.txt";
$: = Tie::File::_default_recsep();
my $data = "rec0$:rec1$:rec2$:";
my ($o, $n);

print "1..79\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

open F, '>', $file or die $!;
binmode F;
print F $data;
close F;

my @a;
$o = tie @a, 'Tie::File', $file;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

# (3-6) Deferred storage
$o->defer;
$a[3] = "rec3";
check_contents($data);          # nothing written yet
$a[4] = "rec4";
check_contents($data);          # nothing written yet

# (7-8) Flush
$o->flush;
check_contents($data . "rec3$:rec4$:");          # now it's written

# (9-12) Deferred writing disabled?
$a[3] = "rec9";
check_contents("${data}rec9$:rec4$:");
$a[4] = "rec8";
check_contents("${data}rec9$:rec8$:");

# (13-18) Now let's try two batches of records
$#a = 2;
$o->defer;
$a[0] = "record0";
check_contents($data);          # nothing written yet
$a[2] = "record2";
check_contents($data);          # nothing written yet
$o->flush;
check_contents("record0$:rec1$:record2$:");

# (19-22) Deferred writing past the end of the file
$o->defer;
$a[4] = "record4";
check_contents("record0$:rec1$:record2$:");
$o->flush;
check_contents("record0$:rec1$:record2$:$:record4$:");


# (23-26) Now two long batches
$o->defer;
for (0..2, 4..6) {
  $a[$_] = "r$_";
}
check_contents("record0$:rec1$:record2$:$:record4$:");
$o->flush;
check_contents(join $:, "r0".."r2", "", "r4".."r6", "");

# (27-30) Now let's make sure that discarded writes are really discarded
# We have a 2Mib buffer here, so we can be sure that we aren't accidentally
# filling it up
$o->defer;
for (0, 3, 7) {
  $a[$_] = "discarded$_";
}
check_contents(join $:, "r0".."r2", "", "r4".."r6", "");
$o->discard;
check_contents(join $:, "r0".."r2", "", "r4".."r6", "");

################################################################
#
# Now we're going to test the results of a small memory limit
#
# 
undef $o;  untie @a;
$data = join "$:", map("record$_", 0..7), "";  # records are 8 or 9 bytes long
open F, '>', $file or die $!;
binmode F;
print F $data;
close F;

# Limit cache+buffer size to 47 bytes 
my $MAX = 47;
#  -- that's enough space for 5 records, but not 6, on both \n and \r\n systems
my $BUF = 20;
#  -- that's enough space for 2 records, but not 3, on both \n and \r\n systems
$o = tie @a, 'Tie::File', $file, memory => $MAX, dw_size => $BUF;
print $o ? "ok $N\n" : "not ok $N\n";
$N++;

# (31-32) Fill up the read cache
my @z;
@z = @a;                        
# the cache now contains records 3,4,5,6,7.
check_caches({map(($_ => "record$_$:"), 3..7)}, 
             {});

# (33-44) See if overloading the defer starts by flushing the read cache
# and then flushes out the defer
$o->defer;
$a[0] = "recordA";              # That should flush record 3 from the cache
check_caches({map(($_ => "record$_$:"), 4..7)}, 
             {0 => "recordA$:"});
check_contents($data);

$a[1] = "recordB";              # That should flush record 4 from the cache
check_caches({map(($_ => "record$_$:"), 5..7)}, 
             {0 => "recordA$:",
              1 => "recordB$:"});
check_contents($data);

$a[2] = "recordC";              # That should flush the whole darn defer
# This shouldn't change the cache contents
check_caches({map(($_ => "record$_$:"), 5..7)}, 
             {});               # URRRP
check_contents(join("$:", qw(recordA recordB recordC 
                             record3 record4 record5 record6 record7)) . "$:");

$a[3] = "recordD";         # even though we flushed, deferring is STILL ENABLED
check_caches({map(($_ => "record$_$:"), 5..7)},
             {3 => "recordD$:"}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             record3 record4 record5 record6 record7)) . "$:");

# Check readcache-deferbuffer interactions

# (45-47) This should remove outdated data from the read cache
$a[5] = "recordE";
check_caches({6 => "record6$:", 7 => "record7$:"},
             {3 => "recordD$:", 5 => "recordE$:"}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             record3 record4 record5 record6 record7)) . "$:");

# (48-51) This should read back out of the defer buffer
# without adding anything to the read cache
my $z;
$z = $a[5];
print $z eq "recordE" ? "ok $N\n" : "not ok $N\n";  $N++;
check_caches({6 => "record6$:", 7 => "record7$:"},
             {3 => "recordD$:", 5 => "recordE$:"}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             record3 record4 record5 record6 record7)) . "$:");

# (52-55) This should repopulate the read cache with a new record
$z = $a[0];
print $z eq "recordA" ? "ok $N\n" : "not ok $N\n";  $N++;
check_caches({0 => "recordA$:", 6 => "record6$:", 7 => "record7$:"},
             {3 => "recordD$:", 5 => "recordE$:"}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             record3 record4 record5 record6 record7)) . "$:");

# (56-59) This should flush the LRU record from the read cache
$z = $a[4];
print $z eq "record4" ? "ok $N\n" : "not ok $N\n";  $N++;
check_caches({7 => "record7$:", 0 => "recordA$:", 4 => "record4$:"},
             {3 => "recordD$:", 5 => "recordE$:"}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             record3 record4 record5 record6 record7)) . "$:");

# (60-63) This should FLUSH the deferred buffer
$z = splice @a, 3, 1, "recordZ";
print $z eq "recordD" ? "ok $N\n" : "not ok $N\n";  $N++;
check_caches({7 => "record7$:", 0 => "recordA$:", 4 => "record4$:", 3 => "recordZ$:"},
             {}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             recordZ record4 recordE record6 record7)) . "$:");

# (64-66) We should STILL be in deferred writing mode
$a[5] = "recordX";
check_caches({7 => "record7$:", 0 => "recordA$:", 4 => "record4$:", 3 => "recordZ$:"},
             {5 => "recordX$:"}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             recordZ record4 recordE record6 record7)) . "$:");

# Fill up the defer buffer again
$a[4] = "recordP";
# (67-69) This should OVERWRITE the existing deferred record 
# and NOT flush the buffer
$a[5] = "recordQ";   
check_caches({7 => "record7$:", 0 => "recordA$:", 3 => "recordZ$:"},
             {5 => "recordQ$:", 4 => "recordP$:"}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             recordZ record4 recordE record6 record7)) . "$:");

# (70-72) Discard should just dump the whole deferbuffer
$o->discard;
check_caches({7 => "record7$:", 0 => "recordA$:", 3 => "recordZ$:"},
             {}); 
check_contents(join("$:", qw(recordA recordB recordC 
                             recordZ record4 recordE record6 record7)) . "$:");

# (73-75) NOW we are out of deferred writing mode
$a[0] = "recordF";
check_caches({7 => "record7$:", 0 => "recordF$:", 3 => "recordZ$:"},
             {}); 
check_contents(join("$:", qw(recordF recordB recordC
                             recordZ record4 recordE record6 record7)) . "$:");

# (76-79) Last call--untying the array should flush the deferbuffer
$o->defer;
$a[0] = "flushed";
check_caches({7 => "record7$:",                   3 => "recordZ$:"},
             {0 => "flushed$:" }); 
check_contents(join("$:", qw(recordF recordB recordC
                             recordZ record4 recordE record6 record7)) . "$:");
undef $o;
untie @a;
# (79) We can't use check_contents any more, because the object is dead
open F, '<', $file or die;
binmode F;
{ local $/ ; $z = <F> }
close F;
my $x = join("$:", qw(flushed recordB recordC
                      recordZ record4 recordE record6 record7)) . "$:";
if ($z eq $x) {
  print "ok $N\n";
} else {
  my $msg = ctrlfix("expected <$x>, got <$z>");
  print "not ok $N \# $msg\n";
}
$N++;

################################################################


sub check_caches {
  my ($xcache, $xdefer) = @_;

#  my $integrity = $o->_check_integrity($file, $ENV{INTEGRITY});
#  print $integrity ? "ok $N\n" : "not ok $N\n";
#  $N++;

  my $good = 1;

  # Copy the contents of the cache into a regular hash
  my %cache;
  for my $k ($o->{cache}->ckeys) {
    $cache{$k} = $o->{cache}->_produce($k);
  }

  $good &&= hash_equal(\%cache, $xcache, "true cache", "expected cache");
  $good &&= hash_equal($o->{deferred}, $xdefer, "true defer", "expected defer");
  print $good ? "ok $N\n" : "not ok $N\n";
  $N++;
}

sub hash_equal {
  my ($a, $b, $ha, $hb) = @_;
  $ha = 'first hash'  unless defined $ha;
  $hb = 'second hash' unless defined $hb;

  my $good = 1;
  my %b_seen;

  for my $k (keys %$a) {
    if (! exists $b->{$k}) {
      print ctrlfix("# Key $k is in $ha but not $hb"), "\n";
      $good = 0;
    } elsif ($b->{$k} ne $a->{$k}) {
      print ctrlfix("# Key $k is <$a->{$k}> in $ha but <$b->{$k}> in $hb"), "\n";
      $b_seen{$k} = 1;
      $good = 0;
    } else {
      $b_seen{$k} = 1;
    }
  }

  for my $k (keys %$b) {
    unless ($b_seen{$k}) {
      print ctrlfix("# Key $k is in $hb but not $ha"), "\n";
      $good = 0;
    }
  }

  $good;
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
    my $msg = ctrlfix("# expected <$x>, got <$a>");
    print "not ok $N\n$msg\n";
  }
  $N++;
}

sub ctrlfix {
  local $_ = shift;
  s/\n/\\n/g;
  s/\r/\\r/g;
  $_;
}

END {
  undef $o;
  untie @a if tied @a;
  1 while unlink $file;
}

