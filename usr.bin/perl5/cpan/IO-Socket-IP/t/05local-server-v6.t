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
   my $testserver = IO::Socket::IP->new(
      ( $socktype eq "SOCK_STREAM" ? ( Listen => 1 ) : () ),
      LocalHost => "::1",
      LocalPort => "0",
      Type      => Socket->$socktype,
      GetAddrInfoFlags => 0, # disable AI_ADDRCONFIG
   );

   ok( defined $testserver, "IO::Socket::IP->new constructs a $socktype socket" ) or
      diag( "  error was $@" );

   is( $testserver->sockdomain, $AF_INET6,         "\$testserver->sockdomain for $socktype" );
   is( $testserver->socktype,   Socket->$socktype, "\$testserver->socktype for $socktype" );

   is( $testserver->sockhost, $IN6ADDR_LOOPBACK_HOST, "\$testserver->sockhost for $socktype" );
   like( $testserver->sockport, qr/^\d+$/,            "\$testserver->sockport for $socktype" );

   ok( eval { $testserver->peerport; 1 }, "\$testserver->peerport does not die for $socktype" )
      or do { chomp( my $e = $@ ); diag( "Exception was: $e" ) };

   my $socket = IO::Socket->new;
   $socket->socket( $AF_INET6, Socket->$socktype, 0 )
      or die "Cannot socket() - $!";

   my ( $err, $ai ) = Socket::getaddrinfo( "::1", $testserver->sockport, { family => $AF_INET6, socktype => Socket->$socktype } );
   die "getaddrinfo() - $err" if $err;

   $socket->connect( $ai->{addr} ) or die "Cannot connect() - $!";

   my $testclient = ( $socktype eq "SOCK_STREAM" ) ? 
      $testserver->accept : 
      do { $testserver->connect( $socket->sockname ); $testserver };

   ok( defined $testclient, "accepted test $socktype client" );
   isa_ok( $testclient, "IO::Socket::IP", "\$testclient for $socktype" );

   is( $testclient->sockdomain, $AF_INET6,         "\$testclient->sockdomain for $socktype" );
   is( $testclient->socktype,   Socket->$socktype, "\$testclient->socktype for $socktype" );

   is_deeply( [ unpack_sockaddr_in6_addrport( $socket->sockname ) ],
              [ unpack_sockaddr_in6_addrport( $testclient->peername ) ],
              "\$socket->sockname for $socktype" );

   is_deeply( [ unpack_sockaddr_in6_addrport( $socket->peername ) ],
              [ unpack_sockaddr_in6_addrport( $testclient->sockname ) ],
              "\$socket->peername for $socktype" );

   my $peerport = ( Socket::unpack_sockaddr_in6 $socket->peername )[0];
   my $sockport = ( Socket::unpack_sockaddr_in6 $socket->sockname )[0];

   is( $testclient->sockport, $peerport, "\$testclient->sockport for $socktype" );
   is( $testclient->peerport, $sockport, "\$testclient->peerport for $socktype" );

   # Unpack just so it pretty prints without wrecking the terminal if it fails
   is( unpack("H*", $testclient->peeraddr), $IN6ADDR_LOOPBACK_HEX, "\$testclient->peeraddr for $socktype" );
   if( $socktype eq "SOCK_STREAM" ) {
      # Some OSes don't update sockaddr with a local bind() on SOCK_DGRAM sockets
      is( unpack("H*", $testclient->sockaddr), $IN6ADDR_LOOPBACK_HEX, "\$testclient->sockaddr for $socktype" );
   }
}

done_testing;
