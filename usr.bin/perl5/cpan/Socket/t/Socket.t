#!./perl

use v5.6.1;
use strict;
use warnings;

our %Config;
my $has_alarm;
BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bSocket\b/ && 
        !(($^O eq 'VMS') && $Config{d_socket})) {
	print "1..0\n";
	exit 0;
    }
    $has_alarm = $Config{d_alarm};
}
	
use Socket qw(:all);
use Test::More tests => 6;

my $has_echo = $^O ne 'MSWin32';
my $alarmed = 0;
sub arm      { $alarmed = 0; alarm(shift) if $has_alarm }
sub alarmed  { $alarmed = 1 }
$SIG{ALRM} = 'alarmed'                    if $has_alarm;

SKIP: {
    unless(socket(T, PF_INET, SOCK_STREAM, IPPROTO_TCP)) {
	skip "No PF_INET", 3;
    }

    pass "socket(PF_INET)";

    arm(5);
    my $host = $^O eq 'MacOS' || ($^O eq 'irix' && $Config{osvers} == 5) ?
			 	 '127.0.0.1' : 'localhost';
    my $localhost = inet_aton($host);

    SKIP: {
	unless($has_echo && defined $localhost && connect(T,pack_sockaddr_in(7,$localhost))) {
	    skip "Unable to connect to localhost:7", 2;
	}

	arm(0);

	pass "PF_INET echo localhost connected";

	diag "Connected to " .
		inet_ntoa((unpack_sockaddr_in(getpeername(T)))[1])."\n";

	arm(5);
	syswrite(T,"hello",5);
	arm(0);

	arm(5);
	my $buff;
	my $read = sysread(T,$buff,10);	# Connection may be granted, then closed!
	arm(0);

	while ($read > 0 && length($buff) < 5) {
	    # adjust for fact that TCP doesn't guarantee size of reads/writes
	    arm(5);
	    $read = sysread(T,$buff,10,length($buff));
	    arm(0);
	}

	ok(($read == 0 || $buff eq "hello"), "PF_INET echo localhost reply");
    }
}

SKIP: {
    unless(socket(S, PF_INET, SOCK_STREAM, IPPROTO_TCP)) {
	skip "No PF_INET", 3;
    }

    pass "socket(PF_INET)";

    SKIP: {
	arm(5);
	unless($has_echo && connect(S,pack_sockaddr_in(7,INADDR_LOOPBACK))) {
	    skip "Unable to connect to localhost:7", 2;
	}

        arm(0);

	pass "PF_INET echo INADDR_LOOPBACK connected";

	diag "Connected to " .
		inet_ntoa((unpack_sockaddr_in(getpeername(S)))[1])."\n";

	arm(5);
	syswrite(S,"olleh",5);
	arm(0);

	arm(5);
	my $buff;
	my $read = sysread(S,$buff,10);	# Connection may be granted, then closed!
	arm(0);

	while ($read > 0 && length($buff) < 5) {
	    # adjust for fact that TCP doesn't guarantee size of reads/writes
	    arm(5);
	    $read = sysread(S,$buff,10,length($buff));
	    arm(0);
	}

	ok(($read == 0 || $buff eq "olleh"), "PF_INET echo INADDR_LOOPBACK reply");
    }
}
