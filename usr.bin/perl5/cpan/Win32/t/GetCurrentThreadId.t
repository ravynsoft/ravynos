use strict;
use Config qw(%Config);
use Test;
use Win32;

my $fork_emulation = $Config{ccflags} =~ /PERL_IMPLICIT_SYS/;

my $tests = $fork_emulation ? 4 : 2;
plan tests => $tests;

my $pid = $$+0; # make sure we don't copy any magic to $pid

if ($^O eq "cygwin") {
    skip(!defined &Cygwin::pid_to_winpid,
	 Cygwin::pid_to_winpid($pid),
	 Win32::GetCurrentProcessId());
}
else {
    ok($pid, Win32::GetCurrentProcessId());
}

if ($fork_emulation) {
    # This test relies on the implementation detail that the fork() emulation
    # uses the negative value of the thread id as a pseudo process id.
    if (my $child = fork) {
	waitpid($child, 0);
	exit 0;
    }
    ok(-$$, Win32::GetCurrentThreadId());

    # GetCurrentProcessId() should still return the real PID
    ok($pid, Win32::GetCurrentProcessId());
    ok($$ != Win32::GetCurrentProcessId());
}
else {
    # here we just want to see something.
    ok(Win32::GetCurrentThreadId() > 0);
}
