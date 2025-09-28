# Test to perform ICMPv6 protocol testing.
# Root access is required.
# In the core test suite it calls itself via sudo -n (no password) to test it.

use strict;
use Config;
use Net::Ping;
use Test::More;
use Cwd;
use File::Spec;

BEGIN {
  unless (eval "require Socket") {
    plan skip_all => 'no Socket';
  }
  unless ($Config{d_getpbyname}) {
    plan skip_all => 'no getprotobyname';
  }
}

my $is_devel = $ENV{PERL_CORE} || -d ".git" ? 1 : 0;
if (0 && !Net::Ping::_isroot()) {
    my $file = __FILE__;
    my $lib = $ENV{PERL_CORE} ? '-I../../lib' : '-Mblib';
    # -n prevents from asking for a password. rather fail then
    # A technical problem is with leak-detectors, like asan, which
    # require PERL_DESTRUCT_LEVEL=2 to be set in the root env.
    my $env = "PERL_DESTRUCT_LEVEL=2";
    if ($ENV{TEST_PING6_HOST}) {
      $env .= " TEST_PING6_HOST=$ENV{TEST_PING6_HOST}";
    }
    if ($ENV{PERL_CORE} && $Config{ldlibpthname}) {
      my $up = File::Spec->updir();
      my $dir = Cwd::abs_path(File::Spec->catdir($up, $up));
      $env .= " $Config{ldlibpthname}=\"$dir\"";
    }
    if ($is_devel and
        system("sudo -n $env \"$^X\" $lib $file") == 0) {
      exit;
    } else {
      plan skip_all => 'no sudo/failed';
    }
}


SKIP: {
  skip "icmpv6 ping requires root privileges.", 1
    if !Net::Ping::_isroot() or $^O eq 'MSWin32';
  my $p;
  eval { $p = new Net::Ping "icmpv6"; };
  if ($@) {
    plan skip_all => "no icmpv6 on this machine $@";
  }
  # message_type can't be used
  eval {
    $p->message_type();
  };
  like($@, qr/message type only supported on 'icmp' protocol/, "message_type() API only concern 'icmp' protocol");
  my $rightip = "2001:4860:4860::8888"; # pingable ip of google's dnsserver
  # for a firewalled ipv6 network try an optional local ipv6 host
  $rightip = $ENV{TEST_PING6_HOST} if $ENV{TEST_PING6_HOST};
  my $wrongip = "2001:4860:4860::1234"; # non existing ip
  # diag "Pinging existing IPv6 ";
  my $result = $p->ping($rightip);
  if ($result == 1) {
    ok($result, "icmpv6 ping $rightip");
    # diag "Pinging wrong IPv6 ";
    ok(!$p->ping($wrongip), "icmpv6 ping $wrongip");
  } else {
  TODO: {
      local $TODO = "icmpv6 firewalled?";
      is($result, 1, "icmpv6 ping $rightip");
    }
  }
}

done_testing;
