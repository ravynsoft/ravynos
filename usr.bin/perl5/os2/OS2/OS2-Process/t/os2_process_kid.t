#! /usr/bin/perl -w

use strict;
use OS2::Process; 		# qw(P_SESSION P_UNRELATED P_NOWAIT);

my $pl = $0;
$pl =~ s/_kid\.t$/.t/i;
die "Can't find the kid script" unless -r $pl;

my $inc = $ENV{PERL5LIB};
$inc = $ENV{PERLLIB} unless defined $inc;
$inc = '' unless defined $inc;
$ENV{PERL5LIB} = join ';', @INC, split /;/, $inc;

# The thest in $pl modify the session too bad.  We run the tests
# in a different session to keep the current session cleaner

# Apparently, this affects things at open() time, not at system() time
$^F = 40;

# These do not work...  Apparently, the kid "interprets" file handles
# open to CON as output to *its* CON (shortcut in the kernel via the
# device flags?).

#my @fh = ('<&STDIN', '>&STDOUT', '>&STDERR');
#my @nfd;
#open $nfd[$_], $fh[$_] or die "Cannot remap FH" for 0..2;
#my @fn = map fileno $_, @nfd;
#$ENV{NEW_FD} = "@fn";

my ($stdout_r,$stdout_w,$stderr_r,$stderr_w);
pipe $stderr_r, $stderr_w or die;

# Duper for $stderr_r to STDERR
my ($e_r, $e_w) = map fileno $_, $stderr_r,  $stderr_w;
my $k = system P_NOWAIT, $^X, '-we', <<'EOS', $e_r, $e_w or die "Cannot start a STDERR duper";
  my ($e_r, $e_w) = @ARGV;
  # close the other end by the implicit close:
  { open my $closeit, ">&=$e_w" or die "kid: open >&=$e_w: $!, `$^E'" }
  open IN, "<&=$e_r" or die "kid: open <&=$e_r: $!, `$^E'";
  select STDERR; $| = 1; print while sysread IN, $_, 1<<16;
EOS
close $stderr_r or die;		# Now the kid is the owner

pipe $stdout_r, $stdout_w or die;

my @fn = (map fileno $_, $stdout_w, $stderr_w);
$ENV{NEW_FD} = "@fn";
# print "# fns=@fn\n";

$ENV{OS2_PROCESS_TEST_SEPARATE_SESSION} = 1;
my $pid = system P_SESSION, $^X, $pl, @ARGV or die;
close $stderr_w or die;		# Leave these two FH to the kid only
close $stdout_w or die;

# Duplicate the STDOUT of the kid:
# These are workarounds for bug in sysread: it is reading in binary...
binmode $stdout_r;
binmode STDOUT;
$| = 1;  print while sysread $stdout_r, $_, 1<<16;

waitpid($pid, 0) >= 0 or die;

# END { print "# parent finished\r\n" }
