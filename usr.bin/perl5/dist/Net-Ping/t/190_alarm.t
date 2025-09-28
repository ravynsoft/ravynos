# Test to make sure alarm / SIGALM does not interfere
# with Net::Ping.  (This test was derived to ensure
# compatibility with the "spamassassin" utility.)
# Based on code written by radu@netsoft.ro (Radu Greab).

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
  unless (eval {alarm 0; 1;}) {
    print "1..0 \# Skip: alarm borks on $^O $^X $] ?\n";
    exit;
  }
  unless (getservbyname('echo', 'tcp')) {
    print "1..0 \# Skip: no echo port\n";
    exit;
  }
}

use strict;
use Test::More tests => 6;
BEGIN {use_ok 'Net::Ping'};

# Hopefully this is never a routeable host
my $fail_ip = $ENV{NET_PING_FAIL_IP} || "192.0.2.0";

eval {
  my $timeout = 11;

  pass('In eval');
  local $SIG{ALRM} = sub { die "alarm works" };
  pass('SIGALRM can be set on this platform');
  alarm $timeout;
  pass('alarm() can be set on this platform');

  my $start = time;
  while (1) {
    my $ping = Net::Ping->new("tcp", 2);
    # It does not matter if alive or not
    $ping->ping("127.0.0.1");
    $ping->ping($fail_ip);
    die "alarm failed" if time > $start + $timeout + 1;
  }
};
pass('Got out of "infinite loop" okay');

like($@, qr/alarm works/, 'Make sure it died for a good excuse');

alarm 0; # Reset alarm
