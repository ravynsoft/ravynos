# Test to perform icmp protocol testing.
# Root access is required.
# In the core test suite it calls itself via sudo -n (no password) to test it.

use strict;
use Config;

use Test::More;
use Net::Ping;
use Cwd;
use File::Spec;
BEGIN {
  unless (eval "require Socket;") {
    plan skip_all => 'no Socket';
  }
  unless ($Config{d_getpbyname}) {
    plan skip_all => 'no getprotobyname';
  }
}

my $is_devel = $ENV{PERL_CORE} || -d ".git" ? 1 : 0;
# Note this rawsocket test code is considered anti-social in p5p and was removed in
# their variant.
# See http://nntp.perl.org/group/perl.perl5.porters/240707
# Problem is that ping_icmp needs root perms, and previous bugs were
# never caught. So I rather execute it via sudo in the core test suite
# and on devel CPAN dirs, than not at all and risk further bitrot of this API.
if ( 0 && !Net::Ping::_isroot()) { # disable in blead via 7bfdd8260c
    my $file = __FILE__;
    my $lib = $ENV{PERL_CORE} ? '-I../../lib' : '-Mblib';
    if ($is_devel and $Config{ccflags} =~ /fsanitize=address/ and $^O eq 'linux') {
      plan skip_all => 'asan leak detector';
    }
    # -n prevents from asking for a password. rather fail then
    # A technical problem is with leak-detectors, like asan, which
    # require PERL_DESTRUCT_LEVEL=2 to be set in the root env.
    my $env = "PERL_DESTRUCT_LEVEL=2";
    if ($ENV{TEST_PING_HOST}) {
      $env .= " TEST_PING_HOST=$ENV{TEST_PING_HOST}";
    }
    if ($ENV{PERL_CORE} && $Config{ldlibpthname}) {
      my $up = File::Spec->updir();
      my $dir = Cwd::abs_path(File::Spec->catdir($up, $up));
      $env .= " $Config{ldlibpthname}=\"$dir\"";
    }
    if ($is_devel and
        system("sudo -n $env \"$^X\" $lib $file") == 0)
    {
      exit;
    } else {
      plan skip_all => 'no sudo/failed';
    }
}

SKIP: {
  skip "icmp ping requires root privileges.", 2
    if !Net::Ping::_isroot() or $^O eq 'MSWin32';
  my $p = new Net::Ping "icmp";
  is($p->message_type(), 'echo', "default icmp message type is 'echo'");
  # message_type fails on wrong message type
  eval {
    $p->message_type('information');
  };
  like($@, qr/icmp message type are limited to 'echo' and 'timestamp'/, "Failure on wrong message type");
  my $result = $p->ping("127.0.0.1");
  if ($result == 1) {
    is($result, 1, "icmp ping 127.0.0.1");
  } else {
  TODO: {
      local $TODO = "localhost icmp firewalled?";
      if (exists $ENV{TEST_PING_HOST}) {
        my $result = $p->ping($ENV{TEST_PING_HOST});
        is($result, 1, "icmp ping $ENV{TEST_PING_HOST}");
      } else {
        is($result, 1, "icmp ping 127.0.0.1");
      }
    }
  }
}

done_testing;
