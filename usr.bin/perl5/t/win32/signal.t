#!./perl
# Tests for signal emulation

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';

    # only used for skip_all, the forking confuses test.pl
    require "./test.pl";
}

BEGIN {
    unless ($^O =~ /^MSWin/) {
        skip_all('windows specific test');
    }
}

use strict;
use Config;

skip_all("requires compilation with the fork emulation")
  unless $Config{'d_pseudofork'};

++$|;

# manual test counting because the forks confuse test.pl
print "1..4\n";

# find a safe signal, the implementation shouldn't be doing anything
# funky with NUMdd signals
my ($sig) = grep /^NUM/, split ' ', $Config{sig_name};

# otherwise, hope CONT is safe
$sig ||= "CONT";

SKIP:
{
    # perl #85104
    use warnings;
    my $pid = fork;

    unless (defined $pid) {
	print <<EOS;
not ok 1 # fork failed: $!
ok 2 # SKIP
ok 3 # SKIP
ok 4 # SKIP
EOS
        last SKIP;
    }
    if ($pid) {
	print "ok 1 # pseudo-forked\n";
	sleep 2; # give the child a chance to setup
	kill $sig, $pid;
	waitpid($pid, 0);
    }
    else {
	my $signalled;
	$SIG{$sig} = sub {
	    $! = 1;
	    $^E = 1000;
	    print "ok 2 # $sig signal handler called\n";
	    ++$signalled;
	};
	$! = 0;
	$^E = 0;
	# wait for the signal
	my $count = 0;
	while (!$signalled && ++$count < 10) {
	    sleep 1;
	}
	print "# signaled after $count loops\n";
	print $! != 0 ? "not " : "", "ok 3 # \$! preserved\n";
	print $^E != 0 ? "not " : "", "ok 4 # \$^E preserved\n"
	    or print STDERR "# \$^E = ", 0+$^E, "\n";
	exit;
    }
}
