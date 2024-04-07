use strict;

BEGIN {
  if ($ENV{NO_NETWORK_TESTING} ||
      ($ENV{PERL_CORE}) && !$ENV{PERL_TEST_Net_Ping}) {
    print "1..0 # Skip: network dependent test\n";
    exit;
  }
  unless (eval "require Socket") {
    print "1..0 \# Skip: no Socket\n";
    exit;
  }
  unless (getservbyname('echo', 'tcp')) {
    print "1..0 \# Skip: no echo port\n";
    exit;
  }
}

# Hopefully this is never a routeable host
my $fail_ip = $ENV{NET_PING_FAIL_IP} || "192.0.2.0";

# Remote network test using tcp protocol.
#
# NOTE:
#   Network connectivity will be required for all tests to pass.
#   Firewalls may also cause some tests to fail, so test it
#   on a clear network.  If you know you do not have a direct
#   connection to remote networks, but you still want the tests
#   to pass, use the following:
#
# $ PERL_CORE=1 make test

use Test::More tests => 13;
BEGIN {use_ok('Net::Ping');}

my $p = new Net::Ping "tcp",9;

isa_ok($p, 'Net::Ping', 'new() worked');

# message_type can't be used
eval {
  $p->message_type();
};
like($@, qr/message type only supported on 'icmp' protocol/, "message_type() API only concern 'icmp' protocol");

my $localhost = $p->ping("localhost");
if ($localhost) {
  isnt($p->ping("localhost"), 0, 'Test on the default port');
} else {
  ok(1, "SKIP localhost on the default port on $^O");
}

# Change to use the more common web port.
# This will pull from /etc/services on UNIX.
# (Make sure getservbyname works in scalar context.)
isnt($p->{port_num} = (getservbyname("http", "tcp") || 80), undef, "getservbyname http");

if ($localhost) {
  isnt($p->ping("localhost"), 0, 'Test localhost on the web port');
} else {
  my $result = $p->ping("localhost");
  if ($result) {
    isnt($p->ping("localhost"), 0, "localhost on the web port unexpectedly worked on $^O");
  } else {
    ok(1, "SKIP localhost on the web port on $^O");
  }
}

is($p->ping($fail_ip), 0, "Can't reach $fail_ip");

# Test a few remote servers
# Hopefully they are up when the tests are run.

if ($p->ping('google.com')) { # check for firewall
  foreach (qw(google.com www.google.com www.wisc.edu
              yahoo.com www.yahoo.com www.about.com)) {
    isnt($p->ping($_), 0, "Can ping $_");
  }
} else {
 SKIP: {
    skip "Cannot ping google.com: no TCP connection or firewall", 6;
  }
}
