# Test to make sure object can be instantiated for icmp protocol.
# Root access is required to actually perform icmp testing.

use strict;
use Config;

BEGIN {
  unless (eval "require Socket") {
    print "1..0 \# Skip: no Socket\n";
    exit;
  }
  unless ($Config{d_getpbyname}) {
    print "1..0 \# Skip: no getprotobyname\n";
    exit;
  }
}

use Test::More tests => 2;
BEGIN {use_ok('Net::Ping')};

SKIP: {
  skip "icmp ping requires root privileges.", 1
    unless &Net::Ping::_isroot;
  my $p = new Net::Ping "icmp";
  isa_ok($p, 'Net::Ping', 'object can be instantiated for icmp protocol');
}
