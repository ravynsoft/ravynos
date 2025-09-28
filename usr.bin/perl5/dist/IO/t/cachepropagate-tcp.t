#!/usr/bin/perl

use warnings;
use strict;

use IO::Socket;
use IO::Socket::INET;
use Socket;
use Test::More;
use Config;

plan tests => 9;

my $listener = IO::Socket::INET->new(Listen => 1,
                                     LocalAddr => '127.0.0.1',
                                     Proto => 'tcp');
ok(defined($listener), 'socket created');

my $port = $listener->sockport();

my $p = $listener->protocol();
ok(defined($p), 'protocol defined');
my $d = $listener->sockdomain();
ok(defined($d), 'domain defined');
my $s = $listener->socktype();
ok(defined($s), 'type defined');

SKIP: {
    skip "fork not available", 5
	unless $Config{d_fork} || $Config{d_pseudofork};

    my $cpid = fork();
    if (0 == $cpid) {
	# the child:
	sleep(1);
	my $connector = IO::Socket::INET->new(PeerAddr => '127.0.0.1',
					      PeerPort => $port,
					      Proto => 'tcp');
        if ($connector) {
            my $buf;
            # wait for parent to close its end
            $connector->read($buf, 1);
        }
        else {
            diag "child failed to connect to parent: $@";
        }
	exit(0);
    } else {;
	    ok(defined($cpid), 'spawned a child');
    }

    my $new = $listener->accept();

    ok($new, "got a socket from accept")
      or diag "accept failed: $@";

    is($new->sockdomain(), $d, 'domain match');
  SKIP: {
      skip "no Socket::SO_PROTOCOL", 1 if !defined(eval { Socket::SO_PROTOCOL });
      is($new->protocol(), $p, 'protocol match');
    }
  SKIP: {
      skip "no Socket::SO_TYPE", 1 if !defined(eval { Socket::SO_TYPE });
      is($new->socktype(), $s, 'type match');
    }
    $new->close;

    wait();
}
