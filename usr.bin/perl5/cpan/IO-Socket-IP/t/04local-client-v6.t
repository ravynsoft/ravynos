#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;
use Socket qw( inet_pton inet_ntop pack_sockaddr_in6 unpack_sockaddr_in6 IN6ADDR_LOOPBACK );

my $AF_INET6 = eval { Socket::AF_INET6() } or
   plan skip_all => "No AF_INET6";

# Some odd locations like BSD jails might not like IN6ADDR_LOOPBACK. We'll
# establish a baseline first to test against
my $IN6ADDR_LOOPBACK = eval {
   socket my $sockh, Socket::PF_INET6(), SOCK_STREAM, 0 or die "Cannot socket(PF_INET6) - $!";
   bind $sockh, pack_sockaddr_in6( 0, inet_pton( $AF_INET6, "::1" ) ) or die "Cannot bind() - $!";
   ( unpack_sockaddr_in6( getsockname $sockh ) )[1];
} or plan skip_all => "Unable to bind to ::1 - $@";
my $IN6ADDR_LOOPBACK_HOST = inet_ntop( $AF_INET6, $IN6ADDR_LOOPBACK );
if( $IN6ADDR_LOOPBACK ne IN6ADDR_LOOPBACK ) {
   diag( "Testing with IN6ADDR_LOOPBACK=$IN6ADDR_LOOPBACK_HOST; this may be because of odd networking" );
}
my $IN6ADDR_LOOPBACK_HEX = unpack "H*", $IN6ADDR_LOOPBACK;

# Unpack just ip6_addr and port because other fields might not match end to end
sub unpack_sockaddr_in6_addrport { 
   return ( Socket::unpack_sockaddr_in6( shift ) )[0,1];
}

foreach my $socktype (qw( SOCK_STREAM SOCK_DGRAM )) {
   my $testserver = IO::Socket->new;
   $testserver->socket( $AF_INET6, Socket->$socktype, 0 )
      or die "Cannot socket() - $!";

   my ( $err, $ai ) = Socket::getaddrinfo( "::1", 0, { family => $AF_INET6, socktype => Socket->$socktype } );
   die "getaddrinfo() - $err" if $err;

   $testserver->bind( $ai->{addr} ) or die "Cannot bind() - $!";

   if( $socktype eq "SOCK_STREAM" ) {
      $testserver->listen( 1 ) or die "Cannot listen() - $!";
   }

   my $testport = ( Socket::unpack_sockaddr_in6 $testserver->sockname )[0];

   my $socket = IO::Socket::IP->new(
      PeerHost    => "::1",
      PeerService => $testport,
      Type        => Socket->$socktype,
      GetAddrInfoFlags => 0, # disable AI_ADDRCONFIG
   );

   ok( defined $socket, "IO::Socket::IP->new constructs a $socktype socket" ) or
      diag( "  error was $@" );

   is( $socket->sockdomain, $AF_INET6,         "\$socket->sockdomain for $socktype" );
   is( $socket->socktype,   Socket->$socktype, "\$socket->socktype for $socktype" );

   my $testclient = ( $socktype eq "SOCK_STREAM" ) ? 
      $testserver->accept : 
      do { $testserver->connect( $socket->sockname ); $testserver };

   ok( defined $testclient, "accepted test $socktype client" );

   ok( $socket->connected, "\$socket is connected for $socktype" );

   is_deeply( [ unpack_sockaddr_in6_addrport( $socket->sockname ) ],
              [ unpack_sockaddr_in6_addrport( $testclient->peername ) ],
              "\$socket->sockname for $socktype" );

   is_deeply( [ unpack_sockaddr_in6_addrport( $socket->peername ) ],
              [ unpack_sockaddr_in6_addrport( $testclient->sockname ) ],
              "\$socket->peername for $socktype" );

   is( $socket->peerhost, $IN6ADDR_LOOPBACK_HOST, "\$socket->peerhost for $socktype" );
   is( $socket->peerport, $testport,              "\$socket->peerport for $socktype" );

   # Unpack just so it pretty prints without wrecking the terminal if it fails
   is( unpack("H*", $socket->peeraddr), $IN6ADDR_LOOPBACK_HEX, "\$testclient->peeraddr for $socktype" );
   if( $socktype eq "SOCK_STREAM" ) {
      # Some OSes don't update sockaddr with a local bind() on SOCK_DGRAM sockets
      is( unpack("H*", $socket->sockaddr), $IN6ADDR_LOOPBACK_HEX, "\$testclient->sockaddr for $socktype" );
   }

   # Can't easily test the non-numeric versions without relying on the system's
   # ability to resolve the name "localhost"

   $socket->close;
   ok( !$socket->connected, "\$socket not connected after close for $socktype" );
}

done_testing;
