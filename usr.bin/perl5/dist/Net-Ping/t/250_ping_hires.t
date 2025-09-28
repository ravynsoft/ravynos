# Test to make sure hires feature works.

use strict;

BEGIN {
  if ($ENV{NO_NETWORK_TESTING} ||
      ($ENV{PERL_CORE}) && !$ENV{PERL_TEST_Net_Ping}) {
    print "1..0 \# Skip: network dependent test\n";
    exit;
  } 
  unless (eval "require Socket") {
    print "1..0 \# Skip: no Socket\n";
    exit;
  }
  unless (eval "require Time::HiRes") {
    print "1..0 \# Skip: no Time::HiRes\n";
    exit;
  }
  unless (getservbyname('echo', 'tcp')) {
    print "1..0 \# Skip: no echo port\n";
    exit;
  }
  unless (Socket::getaddrinfo('localhost', &Socket::AF_INET())) {
    print "1..0 \# Skip: no localhost resolver on $^O\n";
    exit;
  }
}

use Test::More tests => 8;
BEGIN {use_ok('Net::Ping');}

my $p = new Net::Ping "tcp";

isa_ok($p, 'Net::Ping', 'new() worked');

is($Net::Ping::hires, 1, 'Default is to use Time::HiRes');

$p -> hires();
isnt($Net::Ping::hires, 0, 'Enabled hires');

$p -> hires(0);
is($Net::Ping::hires, 0, 'Make sure disable works');

$p -> hires(1);
isnt($Net::Ping::hires, 0, 'Enable hires again');

SKIP: {
  skip "unreliable ping localhost on $^O", 2
    if $^O =~ /^(?:hpux|os390|irix|freebsd)$/;

  # Test on the default port
  my ($ret, $duration) = $p -> ping("localhost");

  isnt($ret, 0, 'localhost should always be reachable');

  # It is extremely likely that the duration contains a decimal
  # point if Time::HiRes is functioning properly, except when it
  # is fast enough to be "0", or slow enough to be exactly "1".
  like($duration, qr/\.|^[01]$/, 'returned duration is valid');
}
