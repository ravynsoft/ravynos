# Test to make sure object can be instantiated for syn protocol.

use strict;
use Config;

BEGIN {
  unless (eval "require Socket") {
    print "1..0 \# Skip: no Socket\n";
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


use Test::More tests => 2;
BEGIN {use_ok 'Net::Ping'};

my $p = new Net::Ping "syn";
isa_ok($p, 'Net::Ping', 'object can be instantiated for syn protocol');
