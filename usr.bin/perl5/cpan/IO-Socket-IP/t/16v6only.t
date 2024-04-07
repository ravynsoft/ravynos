#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use Socket qw(PF_INET6 PF_INET IPPROTO_IPV6 IPV6_V6ONLY);
use IO::Socket::IP;

eval { IO::Socket::IP->new( LocalHost => "::1" ) } or
   plan skip_all => "Unable to bind to ::1";

eval { defined IPV6_V6ONLY } or
   plan skip_all => "IPV6_V6ONLY not available";

# https://rt.cpan.org/Ticket/Display.html?id=102662
$^O eq "irix" and
   plan skip_all => "$^O: IPV6_V6ONLY exists but getnameinfo() fails with EAI_NONAME";

# Don't be locale-sensitive
$! = Errno::ECONNREFUSED;
my $ECONNREFUSED_STR = "$!";

{
   my $listensock = IO::Socket::IP->new(
      Listen    => 1,
      Family    => PF_INET6,
      LocalPort => 0,
      Type      => SOCK_STREAM,
      V6Only    => 1,
      GetAddrInfoFlags => 0, # disable AI_ADDRCONFIG
   ) or die "Cannot listen on PF_INET6 - $@";

   is( $listensock->getsockopt( IPPROTO_IPV6, IPV6_V6ONLY ), 1, 'IPV6_V6ONLY is 1 on $listensock' );

   my $testsock = IO::Socket::IP->new(
      Family   => PF_INET,
      PeerHost => "127.0.0.1",
      PeerPort => $listensock->sockport,
      Type     => SOCK_STREAM,
      GetAddrInfoFlags => 0, # disable AI_ADDRCONFIG
   );
   my $err = "$@";

   ok( !defined $testsock, 'Unable to connect PF_INET socket to PF_INET6 socket with V6Only true' );
   like( $err, qr/\Q$ECONNREFUSED_STR/, 'Socket creation fails with connection refused' );
}

SKIP: {
   skip "This platform does not allow turning IPV6_V6ONLY off", 3 unless IO::Socket::IP->CAN_DISABLE_V6ONLY;

   local $ENV{LANG} = "C"; # avoid locale-dependent error messages

   my $listensock = IO::Socket::IP->new(
      Listen    => 1,
      Family    => PF_INET6,
      LocalPort => 0,
      Type      => SOCK_STREAM,
      V6Only    => 0,
      GetAddrInfoFlags => 0, # disable AI_ADDRCONFIG
   ) or die "Cannot listen on PF_INET6 - $@";

   is( $listensock->getsockopt( IPPROTO_IPV6, IPV6_V6ONLY ), 0, 'IPV6_V6ONLY is 0 on $listensock' );

   my $testsock = IO::Socket::IP->new(
      Family   => PF_INET,
      PeerHost => "127.0.0.1",
      PeerPort => $listensock->sockport,
      Type     => SOCK_STREAM,
      GetAddrInfoFlags => 0, # disable AI_ADDRCONFIG
   );
   my $err = "$@";

   ok( defined $testsock, 'Connected PF_INET socket to PF_INET6 socket with V6Only false' ) or
      diag( "IO::Socket::IP->new failed - $err" );
   is( $testsock->peerport, $listensock->sockport, 'Test socket connected to correct peer port' );
}

done_testing;
