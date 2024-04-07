#!perl -w

# We assume that TestInit has been used.

BEGIN {
      chdir 't' if -d 't';
      require './test.pl';
}

use strict;
use Config;

plan tests => 29;
$| = 1;

watchdog(25);

$SIG{ALRM} = sub {
    die "Alarm!\n";
};

pass('before the first loop');

alarm 2;

eval {
    1 while 1;
};

is($@, "Alarm!\n", 'after the first loop');

pass('before the second loop');

alarm 2;

eval {
    while (1) {
    }
};

is($@, "Alarm!\n", 'after the second loop');

SKIP: {
    skip('We can\'t test blocking without sigprocmask', 17)
	if is_miniperl() || !$Config{d_sigprocmask};
    skip("This doesn\'t work on $^O threaded builds RT#88814", 17)
        if ($^O =~ /cygwin/ && $Config{useithreads});
    skip("This doesn\'t work on $^O version $Config{osvers} RT#88814", 17)
        if ($^O eq "openbsd" && $Config{osvers} < 5.2);

    require POSIX;
    my $pending = POSIX::SigSet->new();
    is POSIX::sigpending($pending), '0 but true', 'sigpending';
    is $pending->ismember(&POSIX::SIGUSR1), 0, 'SIGUSR1 is not pending';
    my $new = POSIX::SigSet->new(&POSIX::SIGUSR1);
    POSIX::sigprocmask(&POSIX::SIG_BLOCK, $new);
    
    my $gotit = 0;
    $SIG{USR1} = sub { $gotit++ };
    kill 'SIGUSR1', $$;
    is $gotit, 0, 'Haven\'t received third signal yet';

    diag "2nd sigpending crashes on cygwin" if $^O eq 'cygwin';
    is POSIX::sigpending($pending), '0 but true', 'sigpending';
    is $pending->ismember(&POSIX::SIGUSR1), 1, 'SIGUSR1 is pending';
    
    my $old = POSIX::SigSet->new();
    POSIX::sigsuspend($old);
    is $gotit, 1, 'Received third signal';
    is POSIX::sigpending($pending), '0 but true', 'sigpending';
    is $pending->ismember(&POSIX::SIGUSR1), 0, 'SIGUSR1 is no longer pending';
    
	{
		kill 'SIGUSR1', $$;
		local $SIG{USR1} = sub { die "FAIL\n" };
		POSIX::sigprocmask(&POSIX::SIG_BLOCK, undef, $old);
		ok $old->ismember(&POSIX::SIGUSR1), 'SIGUSR1 is blocked';
		eval { POSIX::sigsuspend(POSIX::SigSet->new) };
		is $@, "FAIL\n", 'Exception is thrown, so received fourth signal';
		POSIX::sigprocmask(&POSIX::SIG_BLOCK, undef, $old);
TODO:
	    {
		local $::TODO = "Needs investigation" if $^O eq 'VMS';
		ok $old->ismember(&POSIX::SIGUSR1), 'SIGUSR1 is still blocked';
	    }
	}

    POSIX::sigprocmask(&POSIX::SIG_BLOCK, $new);
    kill 'SIGUSR1', $$;
    is $gotit, 1, 'Haven\'t received fifth signal yet';
    POSIX::sigprocmask(&POSIX::SIG_UNBLOCK, $new, $old);
    ok $old->ismember(&POSIX::SIGUSR1), 'SIGUSR1 was still blocked';
    is $gotit, 2, 'Received fifth signal';

    # test unsafe signal handlers in combination with exceptions

    SKIP: {
	# #89718: on old linux kernels, this test hangs. No-ones thought
	# of a reliable way to probe for this, so for now, just skip the
	# tests on production releases
	skip("some OSes hang here", 3) if (int($]*1000) & 1) == 0;
    
  SKIP: {
	skip("Issues on Android", 3) if $^O =~ /android/;
	my $action = POSIX::SigAction->new(sub { $gotit--, die }, POSIX::SigSet->new, 0);
	POSIX::sigaction(&POSIX::SIGALRM, $action);
	eval {
	    alarm 1;
	    my $set = POSIX::SigSet->new;
	    POSIX::sigprocmask(&POSIX::SIG_BLOCK, undef, $set);
	    is $set->ismember(&POSIX::SIGALRM), 0, "SIGALRM is not blocked on attempt $_";
	    POSIX::sigsuspend($set);
	} for 1..2;
	is $gotit, 0, 'Received both signals';
    }
}
}

SKIP: {
    skip("alarm cannot interrupt blocking system calls on $^O", 2)
	if $^O =~ /MSWin32|cygwin|VMS/;
    # RT #88774
    # make sure the signal handler's called in an eval block *before*
    # the eval is popped

    $SIG{'ALRM'} = sub { die "HANDLER CALLED\n" };

    eval {
	alarm(2);
	select(undef,undef,undef,10);
    };
    alarm(0);
    is($@, "HANDLER CALLED\n", 'block eval');

    eval q{
	alarm(2);
	select(undef,undef,undef,10);
    };
    alarm(0);
    is($@, "HANDLER CALLED\n", 'string eval');
}

eval { $SIG{"__WARN__\0"} = sub { 1 } };
like $@, qr/No such hook: __WARN__\\0 at/, q!Fetching %SIG hooks with an extra trailing nul is nul-clean!;

eval { $SIG{"__DIE__\0whoops"} = sub { 1 } };
like $@, qr/No such hook: __DIE__\\0whoops at/;

{
    use warnings;
    my $w;
    local $SIG{__WARN__} = sub { $w = shift };

    $SIG{"KILL\0"} = sub { 1 };
    like $w, qr/No such signal: SIGKILL\\0 at/, 'Arbitrary signal lookup through %SIG is clean';
}

# [perl #45173]
{
    my $int_called;
    local $SIG{INT} = sub { $int_called = 1; };
    $@ = "died";
    is($@, "died");
    kill 'INT', $$;
    # this is needed to ensure signal delivery on MSWin32
    sleep(1);
    is($int_called, 1);
    is($@, "died");
}
