use strict; 
BEGIN {
  if ($ENV{NO_NETWORK_TESTING} ||
      ($ENV{PERL_CORE}) && !$ENV{PERL_TEST_Net_Ping}) {
    print "1..0 \# Skip: network dependent test\n";
    exit;
  } 
  if ($^O eq 'freebsd') {
    print "1..0 \# Skip: unreliable localhost resolver on $^O\n";
    exit;
  }
  unless (eval "require Socket") {
    print "1..0 \# Skip: no Socket\n";
    exit;
  }
  if (my $port = getservbyname('echo', 'tcp')) {
    socket(*ECHO, &Socket::PF_INET(), &Socket::SOCK_STREAM(),
                  (getprotobyname 'tcp')[2]);
    unless (connect(*ECHO,
                    scalar
                      &Socket::sockaddr_in($port,
                                           &Socket::inet_aton("localhost"))))
    {
      print "1..0 \# Skip: loopback tcp echo service is off ($!)\n";
      exit;
    }
    close (*ECHO);
  } else {
    print "1..0 \# Skip: no echo port\n";
    exit;
  }
  unless (Socket::getaddrinfo('localhost', &Socket::AF_INET)) {
    print "1..0 \# Skip: no localhost resolver on $^O\n";
    exit;
  }
}

# Test of stream protocol using loopback interface.
#
# NOTE:
#   The echo service must be enabled on localhost
#   to really test the stream protocol ping.  See
#   the end of this document on how to enable it.

use Test::More tests => 23;
use Net::Ping;

my $p = new Net::Ping "stream";

# new() worked?
isa_ok($p, 'Net::Ping', 'new() worked');

# message_type can't be used
eval {
  $p->message_type();
};
like($@, qr/message type only supported on 'icmp' protocol/, "message_type() API only concern 'icmp' protocol");

is($p->ping("localhost"), 1, 'Attempt to connect to the echo port');

for (1..20) {
  select (undef,undef,undef,0.1);
  is($p->ping("localhost"), 1, 'Try several pings while it is connected');
}

__END__

A simple xinetd configuration to enable the echo service can easily be made.
Just create the following file before restarting xinetd:

/etc/xinetd.d/echo:

# description: An echo server.
service echo
{
        type            = INTERNAL
        id              = echo-stream
        socket_type     = stream
        protocol        = tcp
        user            = root
        wait            = no
        disable         = no
}


Or if you are using inetd, before restarting, add
this line to your /etc/inetd.conf:

echo   stream  tcp     nowait  root    internal
