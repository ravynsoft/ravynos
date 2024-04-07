# Same as 400_ping_sync.t, but port should be included in return
use strict;

BEGIN {
  if ($ENV{NO_NETWORK_TESTING} ||
      ($ENV{PERL_CORE} && !$ENV{PERL_TEST_Net_Ping})) {
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
  unless (getservbyname('http', 'tcp')) {
    print "1..0 \# Skip: no http port\n";
    exit;
  }
}

# Remote network test using syn protocol.
#
# NOTE:
#   Network connectivity will be required for all tests to pass.
#   Firewalls may also cause some tests to fail, so test it
#   on a clear network.  If you know you do not have a direct
#   connection to remote networks, but you still want the tests
#   to pass, use the following:
#
# $ NO_NETWORK_TESTING=1 make test

# Hopefully this is never a routeable host
my $fail_ip = $ENV{NET_PING_FAIL_IP} || "192.0.2.0";

# Try a few remote servers
my %webs;
my @hosts = (
  $fail_ip,

  # Hopefully all these http and https ports are open
  "www.google.com",
  "www.duckduckgo.com",
  "www.microsoft.com",
);

use Test::More tests => 19;

BEGIN {use_ok('Net::Ping')};

my $can_alarm = eval {alarm 0; 1;};

sub Alarm {
    alarm(shift) if $can_alarm;
}

Alarm(50);
$SIG{ALRM} = sub {
    fail('Alarm timed out');
    die "TIMED OUT!";
};

my $p = Net::Ping->new("syn", 10);

isa_ok($p, 'Net::Ping', 'new() worked');

# Change to use the more common web port.
# (Make sure getservbyname works in scalar context.)
cmp_ok(($p->{port_num} = getservbyname("http", "tcp")), '>', 0, 'valid port for http/tcp');

my %contacted;
foreach my $host (@hosts) {
    # ping() does dns resolution and
    # only sends the SYN at this point
    Alarm(50); # (Plenty for a DNS lookup)
    foreach my $port (80, 443) {
        $p->port_number($port);
        is($p->ping($host), 1, "Sent SYN to $host at port $port [" . ($p->{bad}->{$host} || "") . "]");
        $contacted{"$host:$port"} = 1;
    }
}

Alarm(20);
while (my @r = $p->ack()) {
    my %res;
    @res{qw(host ack_time ip port)} = @r;
    my $answered = "$res{host}:$res{port}";
    like($answered, qr/^[\w\.]+:\d+$/, "Supposed to be up: $res{host}:$res{port}");
    delete $contacted{$answered};
}

Alarm(0);
# $fail_ip should not be reachable
is keys %contacted, 2,
  '2 servers did not acknowledge our ping'
  or diag sort keys %contacted;
delete $contacted{$_}
    foreach ("$fail_ip:80","$fail_ip:443", 'www.about.com:443');
is keys %contacted, 0,
    'The servers that did not acknowledge our ping were correct';
