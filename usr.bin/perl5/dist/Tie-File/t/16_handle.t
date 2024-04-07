#!/usr/bin/perl
#
# Basic operation, initializing the object from an already-open handle
# instead of from a filename

use strict;
use warnings;

my $file = "tf16-$$.txt";
$: = Tie::File::_default_recsep();

if ($^O =~ /vms/i) {
  print "1..0\n";
  exit;
}

print "1..39\n";

my $N = 1;
use Tie::File;
print "ok $N\n"; $N++;

use Fcntl 'O_CREAT', 'O_RDWR';
sysopen F, $file, O_CREAT | O_RDWR 
  or die "Couldn't create temp file $file: $!; aborting";
binmode F;

my @a;
my $o = tie @a, 'Tie::File', \*F, autochomp => 0, autodefer => 0;
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

# 25-38 shortening alterations, including truncation
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

# file with holes
$a[4] = 'rec4';
check_contents("sh0", "sh1", "short2", "", "rec4");
$a[3] = 'rec3';
check_contents("sh0", "sh1", "short2", "rec3", "rec4");

close F;
undef $o;
untie @a;

# (39) Does it correctly detect a non-seekable handle?
{  if ($^O =~ /^(MSWin32|dos|beos)$/) {
     print "ok $N # skipped ($^O has broken pipe semantics)\n";
     last;
   }
   if ($] < 5.006) {
     print "ok $N # skipped - 5.005_03 panics after this test\n";
     last;
   }
   my $pipe_succeeded = eval {pipe *R, *W};
   if ($@) {
     chomp $@;
     print "ok $N # skipped (no pipes: $@)\n";
     last;
   } elsif (! $pipe_succeeded) {
     print "ok $N # skipped (pipe call failed: $!)\n";
     last;
   }
   close R;
   $o = eval {tie @a, 'Tie::File', \*W};
   if ($@) {
     if ($@ =~ /filehandle does not appear to be seekable/) {
       print "ok $N\n";
     } else {
       chomp $@;
       print "not ok $N \# \$\@ is $@\n";
     }
   } else {
       print "not ok $N \# passing pipe to TIEARRAY didn't abort program\n";
   }
   $N++;
}

use POSIX 'SEEK_SET';
sub check_contents {
  my @c = @_;
  my $x = join $:, @c, '';
  local *FH = $o->{fh};
  seek FH, 0, SEEK_SET;
#  my $open = open FH, '<', $file;
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

  # now check FETCH:
  my $good = 1;
  my $msg;
  for (0.. $#c) {
    unless ($a[$_] eq "$c[$_]$:") {
      $msg = "expected $c[$_]$:, got $a[$_]";
      ctrlfix($msg);
      $good = 0;
    }
  }
  print $good ? "ok $N\n" : "not ok $N # $msg\n";
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


