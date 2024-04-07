# Test to make sure object can be instantiated for udp protocol.
# I do not know of any servers that support udp echo anymore.

use strict;
use Config;

BEGIN {
  unless (eval "require Socket") {
    print "1..0 \# Skip: no Socket\n";
    exit;
  }
  unless (getservbyname('echo', 'udp')) {
    print "1..0 \# Skip: no echo port\n";
    exit;
  }
  unless ($Config{d_getpbyname}) {
    print "1..0 \# Skip: no getprotobyname\n";
    exit;
  }
}

use Test::More tests => 2;
BEGIN {use_ok 'Net::Ping'};

my $p = new Net::Ping "udp";
isa_ok($p, 'Net::Ping', 'object can be instantiated for udp protocol');
