#! /usr/bin/perl -w

BEGIN {
  my $inc = $ENV{PERL5LIB};
  $inc = $ENV{PERLLIB} unless defined $inc;
  $inc = '' unless defined $inc;
  $ENV{PERL5LIB} = join ';', @INC, split /;/, $inc;
}

use strict;
use Test::More tests => 11;
use OS2::Process;

my $cmd = <<'EOA';
use OS2::Process;
$| = 1;
print for $$, ppid, sidOf;
$SIG{TERM} = $SIG{INT} = sub {exit};
sleep 10;
EOA

#my $PID = open my $fh, '-|', $^X, '-wle', $cmd;
$ENV{CMD_RUN} = $cmd;
my $PID = open my $fh, '-|', "$^X -wle 'eval \$ENV{CMD_RUN} or die'";
ok $PID, 'opened a pipe';
my ($kpid, $kppid, $sid);
$kpid = <$fh>;
$kppid = <$fh>;
$sid = <$fh>;
chomp ($kpid, $kppid, $sid);

# This does not work with the intervening shell...
my $extra_fork = $kppid == $PID; # Temporary implementation of multi-arg open()

print "# us=$$, immediate-pid=$PID, parent-of-kid=$kppid, kid=$kpid\n";
if ($ENV{CMD_RUN}) {	# Two copies of the shell intervene...
  is( ppidOf($kppid), $PID, 'correct pid of the kid or its parent');
  is( ppidOf($PID), $$,  'we know our child\'s parent');
} else {
  is( ($extra_fork ? $kppid : $kpid), $PID, 'correct pid of the kid');
  is( $kppid, ($extra_fork ? $PID : $$), 'kid knows its ppid');
}
ok $sid >= 0, 'kid got its sid';
is($sid, sidOf, 'sid of kid same as our');
is(sidOf($kpid), $sid, 'we know sid of kid');
is(sidOf($PID), $sid, 'we know sid of inter-kid');
is(ppidOf($kpid), $kppid, 'we know ppid of kid');
is(ppidOf($PID), $$, 'we know ppid of inter-kid');

ok kill('TERM', $kpid), 'killed the kid';
#ok( ($PID == $kpid or kill('TERM', $PID)), 'killed the inter-kid');
ok close $fh, 'closed the pipe';	# No kid any more
