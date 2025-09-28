#!./perl

#####################################################################
#
# Test for process id return value from open
# Ronald Schmidt (The Software Path) RonaldWS@software-path.com
#
#####################################################################

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 10;
watchdog(15, $^O eq 'MSWin32' ? "alarm" : '');

use Config;
$| = 1;
$SIG{PIPE} = 'IGNORE';
# reset the handler in case the shell has set a broken default
$SIG{HUP} = 'DEFAULT';
$SIG{HUP} = 'IGNORE' if $^O eq 'interix';

my $perl = which_perl();
$perl .= qq[ "-I../lib"];

my @perl = ( which_perl(), "-I../lib" );

#
# commands run 4 perl programs.  Two of these programs write a
# short message to STDOUT and exit.  Two of these programs
# read from STDIN.  One reader never exits and must be killed.
# the other reader reads one line, waits a few seconds and then
# exits to test the waitpid function.
#
# Using 4+ arg open for the children that sleep so that we're
# killing the perl process instead of an intermediate shell, this
# allows harness to see the file handles closed sooner.  I didn't
# convert them all since I wanted 3-arg open to continue to be
# exercised here.
#
# VMS does not have the list form of piped open, but it also would
# not create a separate process for an intermediate shell.
if ($^O eq 'VMS') {
    $cmd1 = qq/$perl -e "\$|=1; print qq[first process\\n]; sleep 30;"/;
    $cmd2 = qq/$perl -e "\$|=1; print qq[second process\\n]; sleep 30;"/;
}
else {
    @cmd1 = ( @perl, "-e", "\$|=1; print qq[first process\\n]; sleep 30;" );
    @cmd2 = ( @perl, "-e", "\$|=1; print qq[second process\\n]; sleep 30;" );
}
$cmd3 = qq/$perl -e "print <>;"/; # hangs waiting for end of STDIN
$cmd4 = qq/$perl -e "print scalar <>;"/;

#warn "#@cmd1\n#@cmd2\n#$cmd3\n#$cmd4\n";

# start the processes
if ($^O eq 'VMS') {
    ok( $pid1 = open(FH1, "$cmd1 |"), 'first process started');
    ok( $pid2 = open(FH2, "$cmd2 |"), '    second' );
}
else {
    ok( $pid1 = open(FH1, "-|", @cmd1), 'first process started');
    ok( $pid2 = open(FH2, "-|", @cmd2), '    second' );
}
{
    no warnings 'once';
    ok( $pid3 = open(FH3, "| $cmd3"), '    third'  );
}
ok( $pid4 = open(FH4, "| $cmd4"), '    fourth' );

print "# pids were $pid1, $pid2, $pid3, $pid4\n";

my $killsig = 'HUP';
$killsig = 1 unless $Config{sig_name} =~ /\bHUP\b/;

# get message from first process and kill it
chomp($from_pid1 = scalar(<FH1>));
is( $from_pid1, 'first process',    'message from first process' );

$kill_cnt = kill $killsig, $pid1;
is( $kill_cnt, 1,   'first process killed' ) ||
  print "# errno == $!\n";

# get message from second process and kill second process and reader process
chomp($from_pid2 = scalar(<FH2>));
is( $from_pid2, 'second process',   'message from second process' );

$kill_cnt = kill $killsig, $pid2, $pid3;
is( $kill_cnt, 2,   'killing procs 2 & 3' ) ||
  print "# errno == $!\n";


# send one expected line of text to child process and then wait for it
select(FH4); $| = 1; select(STDOUT);

printf FH4 "ok %d - text sent to fourth process\n", curr_test();
next_test();
print "# waiting for process $pid4 to exit\n";
$reap_pid = waitpid $pid4, 0;
is( $reap_pid, $pid4, 'fourth process reaped' );

