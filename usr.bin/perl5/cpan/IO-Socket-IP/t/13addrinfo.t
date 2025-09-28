#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;

use IO::Socket::INET;
use Socket qw( SOCK_STREAM unpack_sockaddr_in getaddrinfo );

{
   my $testserver = IO::Socket::INET->new(
      Listen    => 1,
      LocalHost => "127.0.0.1",
      Type      => SOCK_STREAM,
   ) or die "Cannot listen on PF_INET - $@";

   my ( $err, @peeraddrinfo ) = getaddrinfo( "127.0.0.1", $testserver->sockport, { socktype => SOCK_STREAM } );
   $err and die "Cannot getaddrinfo 127.0.0.1 - $err";

   my $socket = IO::Socket::IP->new(
      PeerAddrInfo => \@peeraddrinfo,
   );

   ok( defined $socket, 'IO::Socket::IP->new( PeerAddrInfo => ... ) constructs a new socket' ) or
      diag( "  error was $@" );

   is_deeply( [ unpack_sockaddr_in $socket->peername ],
              [ unpack_sockaddr_in $testserver->sockname ],
              '$socket->peername' );
}

{
   my ( $err, @localaddrinfo ) = getaddrinfo( "127.0.0.1", 0, { socktype => SOCK_STREAM } );
   $err and die "Cannot getaddrinfo 127.0.0.1 - $err";

   my $socket = IO::Socket::IP->new(
      Listen => 1,
      LocalAddrInfo => \@localaddrinfo,
   );

   ok( defined $socket, 'IO::Socket::IP->new( LocalAddrInfo => ... ) constructs a new socket' ) or
      diag( "  error was $@" );

   my $testclient = IO::Socket::INET->new(
      PeerHost => "127.0.0.1",
      PeerPort => $socket->sockport,
   ) or die "Cannot connect to localhost - $@";

   is_deeply( [ unpack_sockaddr_in $socket->sockname ],
              [ unpack_sockaddr_in $testclient->peername ],
              '$socket->sockname' );
}

done_testing;
