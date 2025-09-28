#!./perl -w

BEGIN {
    require Config; import Config;
    if (!$Config{'d_fork'}
       # open2/3 supported on win32
       && $^O ne 'MSWin32')
    {
	print "1..0\n";
	exit 0;
    }
    # make warnings fatal
    $SIG{__WARN__} = sub { die @_ };
}

use strict;
use Test::More tests => 45;

use IO::Handle;
use IPC::Open3;
use POSIX ":sys_wait_h";

my $perl = $^X;

sub cmd_line {
	if ($^O eq 'MSWin32') {
		my $cmd = shift;
		$cmd =~ tr/\r\n//d;
		$cmd =~ s/"/\\"/g;
		return qq/"$cmd"/;
	}
	else {
		return $_[0];
	}
}

my ($pid, $reaped_pid);
STDOUT->autoflush;
STDERR->autoflush;

# basic
$pid = open3 'WRITE', 'READ', 'ERROR', $perl, '-e', cmd_line(<<'EOF');
    $| = 1;
    print scalar <STDIN>;
    print STDERR "hi error\n";
EOF
cmp_ok($pid, '!=', 0);
isnt((print WRITE "hi kid\n"), 0);
like(scalar <READ>, qr/^hi kid\r?\n$/);
like(scalar <ERROR>, qr/^hi error\r?\n$/);
is(close(WRITE), 1) or diag($!);
is(close(READ), 1) or diag($!);
is(close(ERROR), 1) or diag($!);
$reaped_pid = waitpid $pid, 0;
is($reaped_pid, $pid);
is($?, 0);

my $desc = "read and error together, both named";
$pid = open3 'WRITE', 'READ', 'READ', $perl, '-e', cmd_line(<<'EOF');
    $| = 1;
    print scalar <STDIN>;
    print STDERR scalar <STDIN>;
EOF
print WRITE "$desc\n";
like(scalar <READ>, qr/\A$desc\r?\n\z/);
print WRITE "$desc [again]\n";
like(scalar <READ>, qr/\A$desc \[again\]\r?\n\z/);
waitpid $pid, 0;

$desc = "read and error together, error empty";
$pid = open3 'WRITE', 'READ', '', $perl, '-e', cmd_line(<<'EOF');
    $| = 1;
    print scalar <STDIN>;
    print STDERR scalar <STDIN>;
EOF
print WRITE "$desc\n";
like(scalar <READ>, qr/\A$desc\r?\n\z/);
print WRITE "$desc [again]\n";
like(scalar <READ>, qr/\A$desc \[again\]\r?\n\z/);
waitpid $pid, 0;

is(pipe(PIPE_READ, PIPE_WRITE), 1);
$pid = open3 '<&PIPE_READ', 'READ', '',
		    $perl, '-e', cmd_line('print scalar <STDIN>');
close PIPE_READ;
print PIPE_WRITE "dup writer\n";
close PIPE_WRITE;
like(scalar <READ>, qr/\Adup writer\r?\n\z/);
waitpid $pid, 0;

my $TB = Test::Builder->new();
my $test = $TB->current_test;
# dup reader
$pid = open3 'WRITE', '>&STDOUT', 'ERROR',
		    $perl, '-e', cmd_line('print scalar <STDIN>');
++$test;
print WRITE "ok $test\n";
waitpid $pid, 0;

{
    package YAAH;
    $pid = IPC::Open3::open3('QWACK_WAAK_WAAK', '>&STDOUT', 'ERROR',
			     $perl, '-e', main::cmd_line('print scalar <STDIN>'));
    ++$test;
    no warnings 'once';
    print QWACK_WAAK_WAAK "ok $test # filenames qualified to their package\n";
    waitpid $pid, 0;
}

# dup error:  This particular case, duping stderr onto the existing
# stdout but putting stdout somewhere else, is a good case because it
# used not to work.
$pid = open3 'WRITE', 'READ', '>&STDOUT',
		    $perl, '-e', cmd_line('print STDERR scalar <STDIN>');
++$test;
print WRITE "ok $test\n";
waitpid $pid, 0;

foreach (['>&STDOUT', 'both named'],
	 ['', 'error empty'],
	) {
    my ($err, $desc) = @$_;
    $pid = open3 'WRITE', '>&STDOUT', $err, $perl, '-e', cmd_line(<<'EOF');
    $| = 1;
    print STDOUT scalar <STDIN>;
    print STDERR scalar <STDIN>;
EOF
    printf WRITE "ok %d # dup reader and error together, $desc\n", ++$test
	for 0, 1;
    waitpid $pid, 0;
}

# command line in single parameter variant of open3
# for understanding of Config{'sh'} test see exec description in camel book
my $cmd = 'print(scalar(<STDIN>))';
$cmd = $Config{'sh'} =~ /sh/ ? "'$cmd'" : cmd_line($cmd);
$pid = eval { open3 'WRITE', '>&STDOUT', 'ERROR', "$perl -e " . $cmd; };
if ($@) {
	print "error $@\n";
	++$test;
	print WRITE "not ok $test\n";
}
else {
	++$test;
	print WRITE "ok $test\n";
	waitpid $pid, 0;
}
$TB->current_test($test);

# RT 72016
{
    local $::TODO = "$^O returns a pid and doesn't throw an exception"
	if $^O eq 'MSWin32';
    $pid = eval { open3 'WRITE', 'READ', 'ERROR', '/non/existent/program'; };
    isnt($@, '',
	 'open3 of a non existent program fails with an exception in the parent')
	or do {waitpid $pid, 0};
    SKIP: {
	skip 'open3 returned, our responsibility to reap', 1 unless $@;
	is(waitpid(-1, WNOHANG), -1, 'failed exec child is reaped');
    }
}

$pid = eval { open3 'WRITE', '', 'ERROR', '/non/existent/program'; };
like($@, qr/^open3: Modification of a read-only value attempted at /,
     'open3 faults read-only parameters correctly') or do {waitpid $pid, 0};

package NoFetch;

my $fetchcount = 1;

sub TIESCALAR {
  my $class = shift;
  my $instance = shift || undef;
  return bless \$instance => $class;
}

sub FETCH {
    my $cmd; #dont let "@args = @DB::args;" in Carp::caller_info fire this die
    #fetchcount may need to be increased to 2 if this code is being stepped with
    #a perl debugger
    if($fetchcount == 1 && (caller(1))[3] ne 'Carp::caller_info') {
	#Carp croak reports the errors as being in IPC-Open3.t, so it is
	#unacceptable for testing where the FETCH failure occured, we dont want
	#it failing in a $foo = $_[0]; #later# system($foo), where the failure
	#is supposed to be triggered in the inner most syscall, aka system()
	my ($package, $filename, $line, $subroutine) = caller(2);

	die("FETCH not allowed in ".((caller(1))[3])." in ".((caller(2))[3])."\n");
    } else {
	$fetchcount++;
	return tie($cmd, 'NoFetch');
    }
}

package main;

{
    my $cmd;
    tie($cmd, 'NoFetch');

    $pid = eval { open3 'WRITE', 'READ', 'ERROR', $cmd; };
    like($@, qr/^(?:open3: IO::Pipe: Can't spawn-NOWAIT: FETCH not allowed in \(eval\) (?x:
         )in IPC::Open3::spawn_with_handles|FETCH not allowed in \(eval\) in IPC::Open3::_open3)/,
     'dieing inside Tied arg propagates correctly') or do {waitpid $pid, 0};
}

foreach my $handle (qw (DUMMY STDIN STDOUT STDERR)) {
    local $::{$handle};
    my $out = IO::Handle->new();
    my $pid = eval {
	local $SIG{__WARN__} = sub {
	    open my $fh, '>', '/dev/tty';
	    return if "@_" =~ m!^Use of uninitialized value \$fd.*IO/Handle\.pm!;
	    print $fh "@_";
	    die @_
	};
	open3 undef, $out, undef, $perl, '-le', "print q _# ${handle}_"
    };
    is($@, '', "No errors with localised $handle");
    cmp_ok($pid, '>', 0, "Got a pid with localised $handle");
    if ($handle eq 'STDOUT') {
	is(<$out>, undef, "Expected no output with localised $handle");
    } else {
	like(<$out>, qr/\A# $handle\r?\n\z/,
	     "Expected output with localised $handle");
    }
    waitpid $pid, 0;
}

# Test that tied STDIN, STDOUT, and STDERR do not cause open3 any discomfort.
# In particular, tied STDERR used to be able to prevent open3 from working
# correctly.  RT #119843.
SKIP: {
    if (&IPC::Open3::DO_SPAWN) {
      skip "Calling open3 with tied filehandles does not work here", 6
    }

    {	# This just throws things out
	package My::Tied::FH;
	sub TIEHANDLE { bless \my $self }
	sub PRINT {}
	# Note the absence of OPEN and FILENO
    }
    my $message = "japh\n";
    foreach my $handle (*STDIN, *STDOUT, *STDERR) {
	tie $handle, 'My::Tied::FH';
	my ($in, $out);
	my $pid = eval {
	    open3 $in, $out, undef, $perl, '-ne', 'print';
	};
	is($@, '', "no errors calling open3 with tied $handle");
	print $in $message;
	close $in;
	my $japh = <$out>;
	waitpid $pid, 0;
	is($japh, $message, "read input correctly");
	untie $handle;
    }
}
