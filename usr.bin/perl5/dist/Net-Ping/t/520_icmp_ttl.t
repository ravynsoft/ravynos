# Test to perform icmp protocol testing.
# Root access is required.

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

use Test::More qw(no_plan);
BEGIN {use_ok('Net::Ping')};

SKIP: {
  skip "icmp ping requires root privileges.", 1
    if !Net::Ping::_isroot() or $^O eq 'MSWin32';
  my $p = new Net::Ping ("icmp",undef,undef,undef,undef,undef);
  isa_ok($p, 'Net::Ping');
  ok $p->ping("127.0.0.1");
  $p->close();
  $p = new Net::Ping ("icmp",undef,undef,undef,undef,0);
  ok $p->ping("127.0.0.1");
  $p->close();
  $p = undef();
  $p = new Net::Ping ("icmp",undef,undef,undef,undef,1);
  isa_ok($p, 'Net::Ping');
  $p = undef();
  $p = eval 'new Net::Ping ("icmp",undef,undef,undef,undef,-1)';
  ok(!defined($p));
  $p = undef();
  $p = eval 'new Net::Ping ("icmp",undef,undef,undef,undef,256)';
  ok(!defined($p));
  $p = new Net::Ping ("icmp",undef,undef,undef,undef,10);
  ok $p->ping("127.0.0.1");
  $p->close();
}
