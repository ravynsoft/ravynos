BEGIN {
    use Config;
    unless ($Config{d_fork}) {
	print "1..0 # Skip: no fork\n";
	exit 0;
    }
    eval 'use POSIX qw(sys_wait_h)';
    if ($@) {
	print "1..0 # Skip: no POSIX sys_wait_h\n";
	exit 0;
    }
    eval 'use Time::HiRes qw(time)';
    if ($@) {
	print "1..0 # Skip: no Time::HiRes\n";
	exit 0;
    }
}

use warnings;
use strict;

$| = 1;

use Test::More tests => 3;

sub NEG1_PROHIBITED () { 0x01 }
sub NEG1_REQUIRED   () { 0x02 }

my $count     = 0;
my $max_count = 9;
my $state     = NEG1_PROHIBITED;

my $child_pid = fork();
fail("fork failed") unless defined $child_pid;

# Parent receives a nonzero child PID.

if ($child_pid) {
    my @problems;

    while ($count++ < $max_count) {   
	my $begin_time = time();        
	my $ret = waitpid( -1, WNOHANG );          
	my $elapsed_time = time() - $begin_time;
	
	printf( "# waitpid(-1,WNOHANG) returned %d after %.2f seconds\n",
		$ret, $elapsed_time );
	if ($elapsed_time > 0.5) {
	    push @problems,
		sprintf "%.2f seconds in non-blocking waitpid is too long!\n",
		    $elapsed_time;
	    last;
	}
	
	if ($state & NEG1_PROHIBITED) { 
	    if ($ret == -1) {
		push @problems, "waitpid should not have returned -1 here!\n";
		last;
	    }
	    elsif ($ret == $child_pid) {
		$state = NEG1_REQUIRED;
		is(WIFEXITED(${^CHILD_ERROR_NATIVE}), 1, 'child exited cleanly');
		is(WEXITSTATUS(${^CHILD_ERROR_NATIVE}), 0,
		   'child exited with 0 (the return value of its sleep(3) call)');

	    }
	}
	elsif ($state & NEG1_REQUIRED) {
	    unless ($ret == -1) {
		push @problems, "waitpid should have returned -1 here!\n";
	    }
	    last;
	}
	
	sleep(1);
    }
    is("@problems", "", 'no problems');
    POSIX::exit(0); # parent
    fail("Should have exited");
} else {
    # Child receives a zero PID and can request parent's PID with
    # getppid().
    POSIX::_exit(POSIX::sleep(3));
}
