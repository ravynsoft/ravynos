# Testing service_check method using tcp and syn protocols.

use Config;

BEGIN {
  unless (eval "require IO::Socket") {
    print "1..0 \# Skip: no IO::Socket\n";
    exit;
  }
  unless (getservbyname('echo', 'tcp')) {
    print "1..0 \# Skip: no echo port\n";
    exit;
  }
  unless ($Config{d_getpbyname}) {
    print "1..0 \# Skip: no getprotobyname\n";
    exit;
  }
}

use strict;
use Test::More tests => 26;
BEGIN {use_ok('Net::Ping')};

# I'm lazy so I'll just use IO::Socket
# for the TCP Server stuff instead of doing
# all that direct socket() junk manually.

my $sock1 = new IO::Socket::INET
  LocalAddr => "127.0.0.1",
  Proto => "tcp",
  Listen => 8,
  or warn "bind: $!";

isa_ok($sock1, 'IO::Socket::INET',
       'Start a TCP listen server on ephemeral port');

# Start listening on another ephemeral port
my $sock2 = new IO::Socket::INET
  LocalAddr => "127.0.0.1",
  Proto => "tcp",
  Listen => 8,
  or warn "bind: $!";

isa_ok($sock2, 'IO::Socket::INET',
       'Start a second TCP listen server on ephemeral port');

my $port1 = $sock1->sockport;
cmp_ok($port1, '>', 0);

my $port2 = $sock2->sockport;
cmp_ok($port2, '>', 0);

# 
isnt($port1, $port2, 'Make sure the servers are listening on different ports');

$sock2->close;

# This is how it should be:
# 127.0.0.1:$port1 - service ON
# 127.0.0.1:$port2 - service OFF

#####
# First, we test using the "tcp" protocol.
# (2 seconds should be long enough to connect to loopback.)
my $p = new Net::Ping "tcp", 2;

isa_ok($p, 'Net::Ping', 'new() worked');

# Disable service checking
$p->service_check(0);

# Try on the first port
$p->{port_num} = $port1;

is($p->ping("127.0.0.1"), 1, 'first port is reachable');

# Try on the other port
$p->{port_num} = $port2;

{
    local $TODO = "Believed not to work on $^O" if $^O =~ /^(?:MSWin32|os390)$/;
    is($p->ping("127.0.0.1"), 1, 'second port is reachable');
}

# Enable service checking
$p->service_check(1);

# Try on the first port
$p->{port_num} = $port1;

is($p->ping("127.0.0.1"), 1, 'first service is on');

# Try on the other port
$p->{port_num} = $port2;

isnt($p->ping("127.0.0.1"), 2, 'second service is off');

# test 11 just finished.

#####
# Lastly, we test using the "syn" protocol.
$p = new Net::Ping "syn", 2;

isa_ok($p, 'Net::Ping', 'new() worked');

# Disable service checking
$p->service_check(0);

# Try on the first port
$p->{port_num} = $port1;

is($p->ping("127.0.0.1"), 1, "send SYN to first port") or diag ("ERRNO: $!");

is($p->ack(), '127.0.0.1', 'IP should be reachable');
is($p->ack(), undef, 'No more sockets');

###
# Get a fresh object
$p = new Net::Ping "syn", 2;

isa_ok($p, 'Net::Ping', 'new() worked');

# Disable service checking
$p->service_check(0);

# Try on the other port
$p->{port_num} = $port2;

SKIP: {
  skip "no localhost resolver on $^O", 2
    unless Socket::getaddrinfo('localhost', &Socket::AF_INET);
  is($p->ping("127.0.0.1"), 1, "send SYN to second port") or diag ("ERRNO: $!");

  {
    local $TODO = "Believed not to work on $^O"
      if $^O =~ /^(?:MSWin32|os390)$/;
    is($p->ack(), '127.0.0.1', 'IP should be reachable');
  }
}
is($p->ack(), undef, 'No more sockets');


###
# Get a fresh object
$p = new Net::Ping "syn", 2;

isa_ok($p, 'Net::Ping', 'new() worked');

# Enable service checking
$p->service_check(1);

# Try on the first port
$p->{port_num} = $port1;

is($p->ping("127.0.0.1"), 1, "send SYN to first port") or diag ("ERRNO: $!");

is($p->ack(), '127.0.0.1', 'IP should be reachable');
is($p->ack(), undef, 'No more sockets');


###
# Get a fresh object
$p = new Net::Ping "syn", 2;

isa_ok($p, 'Net::Ping', 'new() worked');

# Enable service checking
$p->service_check(1);

# Try on the other port
$p->{port_num} = $port2;

is($p->ping("127.0.0.1"), 1, "send SYN to second port") or diag ("ERRNO: $!");

is($p->ack(), undef, 'No sockets should have service on');
