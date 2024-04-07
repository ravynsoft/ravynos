#!./perl -w

use Config;
BEGIN {
    require Test::More;
    if (!$Config{'d_fork'}
       # open2/3 supported on win32
       && $^O ne 'MSWin32')
    {
	Test::More->import(skip_all => 'open2/3 not available with MSWin32');
	exit 0;
    }
    # make warnings fatal
    $SIG{__WARN__} = sub { die @_ };
}

use strict;
use IPC::Open2;
use Test::More tests => 15;

my $perl = $^X;

sub cmd_line {
	if ($^O eq 'MSWin32') {
		return qq/"$_[0]"/;
	}
	else {
		return $_[0];
	}
}

STDOUT->autoflush;
STDERR->autoflush;

my $pid = open2('READ', 'WRITE', $perl, '-e', cmd_line('print scalar <STDIN>'));
cmp_ok($pid, '>', 1, 'got a sane process ID');
ok(print WRITE "hi kid\n");
like(<READ>, qr/^hi kid\r?\n$/);
ok(close(WRITE), "closing WRITE: $!");
ok(close(READ), "closing READ: $!");
my $reaped_pid = waitpid $pid, 0;
is($reaped_pid, $pid, "Reaped PID matches");
is($?, 0, '$? should be zero');

{
    package SKREEEK;
    my $pid = IPC::Open2::open2('KAZOP', 'WRITE', $perl, '-e',
				main::cmd_line('print scalar <STDIN>'));
    main::cmp_ok($pid, '>', 1, 'got a sane process ID');
    main::ok(print WRITE "hi kid\n");
    main::like(<KAZOP>, qr/^hi kid\r?\n$/);
    main::ok(close(WRITE), "closing WRITE: $!");
    main::ok(close(KAZOP), "closing READ: $!");
    my $reaped_pid = waitpid $pid, 0;
    main::is($reaped_pid, $pid, "Reaped PID matches");
    main::is($?, 0, '$? should be zero');
}

$pid = eval { open2('READ', '', $perl, '-e', cmd_line('print scalar <STDIN>')) };
like($@, qr/^open2: Modification of a read-only value attempted at /,
     'open2 faults read-only parameters correctly') or do {waitpid $pid, 0};
