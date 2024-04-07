#!./perl

# Test that getppid() follows UNIX semantics: when the parent process
# dies, the child is reparented to the init process
# The init process is usually 1, but doesn't have to be, and there's no
# standard way to find out what it is, so the only portable way to go it so
# attempt 2 reparentings and see if the PID both orphaned grandchildren get is
# the same. (and not ours)
#
# NOTE: Docker and Linux containers set parent to 0 on orphaned tests.
# We have to adjust to this below.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(../lib) );
}

use strict;

skip_all_without_config(qw(d_pipe d_fork d_waitpid d_getppid));
plan (8);

# No, we don't want any zombies. kill 0, $ppid spots zombies :-(
$SIG{CHLD} = 'IGNORE';

sub fork_and_retrieve {
    my $which = shift;
    pipe my ($r, $w) or die "pipe: $!\n";
    my $pid = fork; defined $pid or die "fork: $!\n";

    if ($pid) {
	# parent
	close $w or die "close: $!\n";
	$_ = <$r>;
	chomp;
	die "Garbled output '$_'"
	    unless my ($how, $first, $second) = /^([a-z]+),(\d+),(\d+)\z/;
	cmp_ok ($first, '>=', 1, "Parent of $which grandchild");

    my $message = "grandchild waited until '$how'";
    my $min_getppid_result = is_linux_container() ? 0 : 1;
	cmp_ok ($second, '>=', $min_getppid_result, "New parent of orphaned $which grandchild")
	    ? note ($message) : diag ($message);

	SKIP: {
	    skip("Orphan processes are not reparented on QNX", 1)
		if $^O eq 'nto';
	    isnt($first, $second,
                 "Orphaned $which grandchild got a new parent");
	}
	return $second;
    }
    else {
	# child
	# Prevent test.pl from thinking that we failed to run any tests.
	$::NO_ENDING = 1;
	close $r or die "close: $!\n";

	pipe my ($r2, $w2) or die "pipe: $!\n";
	pipe my ($r3, $w3) or die "pipe: $!\n";
	my $pid2 = fork; defined $pid2 or die "fork: $!\n";
	if ($pid2) {
	    close $w or die "close: $!\n";
	    close $w2 or die "close: $!\n";
	    close $r3 or die "close: $!\n";
	    # Wait for our child to signal that it's read our PID:
	    <$r2>;
	    # Implicit close of $w3:
	    exit 0;
	}
	else {
	    # grandchild
	    close $r2 or die "close: $!\n";
	    close $w3 or die "close: $!\n";
	    my $ppid1 = getppid();
	    # kill 0 isn't portable:
	    my $can_kill0 = eval {
		kill 0, $ppid1;
	    };
	    my $how = $can_kill0 ? 'undead' : 'sleep';

	    # Tell immediate parent to exit:
	    close $w2 or die "close: $!\n";
	    # Wait for it to (start to) exit:
	    <$r3>;
	    # Which sadly isn't enough to be sure that it has exited - often we
	    # get switched in during its shutdown, after $w3 closes but before
	    # it exits and we get reparented.
	    if ($can_kill0) {
		# use kill 0 where possible. Try 10 times, then give up:
		for (0..9) {
		    my $got = kill 0, $ppid1;
		    die "kill: $!" unless defined $got;
		    if (!$got) {
			$how = 'kill';
			last;
		    }
		    sleep 1;
		}
	    } else {
		# Fudge it by waiting a bit more:
		sleep 2;
	    }
	    my $ppid2 = getppid();
	    print $w "$how,$ppid1,$ppid2\n";
	}
	exit 0;
    }
}

my $first = fork_and_retrieve("first");
my $second = fork_and_retrieve("second");
SKIP: {
    skip ("Orphan processes are not reparented on QNX", 1) if $^O eq 'nto';
    is ($first, $second, "Both orphaned grandchildren get the same new parent");
}
isnt ($first, $$, "And that new parent isn't this process");

