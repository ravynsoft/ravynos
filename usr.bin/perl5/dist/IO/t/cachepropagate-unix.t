#!/usr/bin/perl

use warnings;
use strict;

use File::Temp qw(tempdir);
use File::Spec::Functions;
use IO::Socket;
use IO::Socket::UNIX;
use Socket;
use Config;
use Test::More;

plan skip_all => "UNIX domain sockets not implemented on $^O"
  if ($^O =~ m/^(?:qnx|nto|vos|MSWin32|VMS)$/);

my $socketpath = catfile(tempdir( CLEANUP => 1 ), 'testsock');

# check the socketpath fits in sun_path.
#
# pack_sockaddr_un() just truncates the path, this may change, but how
# it will handle such a condition is undetermined (and we might need
# to work with older versions of Socket outside of a perl build)
# https://rt.cpan.org/Ticket/Display.html?id=116819

my $name = eval { pack_sockaddr_un($socketpath) };
if (defined $name) {
    my ($packed_name) = eval { unpack_sockaddr_un($name) };
    if (!defined $packed_name || $packed_name ne $socketpath) {
        plan skip_all => "socketpath too long for sockaddr_un";
    }
}

plan tests => 15;

# start testing stream sockets:
my $listener = IO::Socket::UNIX->new(Type => SOCK_STREAM,
				     Listen => 1,
				     Local => $socketpath);
ok(defined($listener), 'stream socket created');

my $p = $listener->protocol();
{
    # the value of protocol isn't well defined for AF_UNIX, when we
    # create the socket we supply 0, which leaves it up to the implementation
    # to select a protocol, so we (now) don't save a 0 protocol during socket
    # creation.  This test then breaks if the implementation doesn't support
    # SO_SOCKET (at least on AF_UNIX).
    # This specifically includes NetBSD, Darwin and cygwin.
    # This is a TODO instead of a skip so if these ever implement SO_PROTOCOL
    # we'll be notified about the passing TODO so the test can be updated.
    local $TODO = "$^O doesn't support SO_PROTOCOL on AF_UNIX"
        if $^O =~ /^(netbsd|darwin|cygwin|hpux|solaris|dragonfly|os390|gnu)$/;
    ok(defined($p), 'protocol defined');
}
my $d = $listener->sockdomain();
ok(defined($d), 'domain defined');
my $s = $listener->socktype();
ok(defined($s), 'type defined');

SKIP: {
    skip "fork not available", 4
	unless $Config{d_fork} || $Config{d_pseudofork};

    my $cpid = fork();
    if (0 == $cpid) {
	# the child:
	sleep(1);
	my $connector = IO::Socket::UNIX->new(Peer => $socketpath);
	exit(0);
    } else {
	ok(defined($cpid), 'spawned a child');
    }

    my $new = $listener->accept();

    is($new->sockdomain(), $d, 'domain match');
  SKIP: {
      skip "no Socket::SO_PROTOCOL", 1 if !defined(eval { Socket::SO_PROTOCOL });
      skip "SO_PROTOCOL defined but not implemented", 1
         if !defined $new->sockopt(Socket::SO_PROTOCOL);
      is($new->protocol(), $p, 'protocol match');
    }
  SKIP: {
      skip "no Socket::SO_TYPE", 1 if !defined(eval { Socket::SO_TYPE });
      skip "SO_TYPE defined but not implemented", 1
         if !defined $new->sockopt(Socket::SO_TYPE);
      is($new->socktype(), $s, 'type match');
    }

    unlink($socketpath);
    wait();
}

undef $TODO;
SKIP: {
    skip "datagram unix sockets not supported on $^O", 7
      if $^O eq "haiku";
    # now test datagram sockets:
    $listener = IO::Socket::UNIX->new(Type => SOCK_DGRAM,
				      Local => $socketpath);
    ok(defined($listener), 'datagram socket created');

    $p = $listener->protocol();
    {
        # see comment above
        local $TODO = "$^O doesn't support SO_PROTOCOL on AF_UNIX"
            if $^O =~ /^(netbsd|darwin|cygwin|hpux|solaris|dragonfly|os390|gnu)$/;
        ok(defined($p), 'protocol defined');
    }
    $d = $listener->sockdomain();
    ok(defined($d), 'domain defined');
    $s = $listener->socktype();
    ok(defined($s), 'type defined');

    my $new = IO::Socket::UNIX->new_from_fd($listener->fileno(), 'r+');

    is($new->sockdomain(), $d, 'domain match');
    SKIP: {
      skip "no Socket::SO_PROTOCOL", 1 if !defined(eval { Socket::SO_PROTOCOL });
      skip "SO_PROTOCOL defined but not implemented", 1
         if !defined $new->sockopt(Socket::SO_PROTOCOL);
      is($new->protocol(), $p, 'protocol match');
    }
    SKIP: {
      skip "AIX: getsockopt(SO_TYPE) is badly broken on UDP/UNIX sockets", 1
         if $^O eq "aix";
      skip "no Socket::SO_TYPE", 1 if !defined(eval { Socket::SO_TYPE });
      skip "SO_TYPE defined but not implemented", 1
         if !defined $new->sockopt(Socket::SO_TYPE);
      is($new->socktype(), $s, 'type match');
    }
}
unlink($socketpath);
