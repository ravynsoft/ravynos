#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;
use Socket qw( SOCK_STREAM AF_INET );

# Compatibility test for
# IO::Socket::INET->new( Blocking => 0 ) still creates a defined filehandle

{
   my $sock = IO::Socket::IP->new( Family => AF_INET );
   my $save_exc = $@;
   ok( defined $sock, 'Constructor yields handle for Family => AF_INET' ) or
      diag( "Exception was $save_exc" );

   ok( defined $sock->fileno, '$sock->fileno for Family => AF_INET' );
   is( $sock->sockdomain, AF_INET, '$sock->sockdomain for Family => AF_INET' );
   is( $sock->socktype, SOCK_STREAM, '$sock->socktype for Family => AF_INET' );
}

SKIP: {
   my $AF_INET6 = eval { require Socket and Socket::AF_INET6() } or
      skip "No AF_INET6", 4;

   eval { IO::Socket::IP->new( LocalHost => "::1" ) } or
      skip "Unable to bind to ::1", 4;

   my $sock = IO::Socket::IP->new( Family => $AF_INET6 );
   my $save_exc = $@;
   ok( defined $sock, 'Constructor yields handle for Family => AF_INET6' ) or
      diag( "Exception was $save_exc" );

   ok( defined $sock->fileno, '$sock->fileno for Family => AF_INET6' );
   is( $sock->sockdomain, $AF_INET6, '$sock->sockdomain for Family => AF_INET6' );
   is( $sock->socktype, SOCK_STREAM, '$sock->socktype for Family => AF_INET6' );
}

# Lack of even a Family hint - _a_ socket is created but we don't guarantee
# what family
{
   my $sock = IO::Socket::IP->new( Type => SOCK_STREAM );
   my $save_exc = $@;
   ok( defined $sock, 'Constructor yields handle for Type => SOCK_STREAM' ) or
      diag( "Exception was $save_exc" );

   ok( defined $sock->fileno, '$sock->fileno for Type => SOCK_STREAM' );
   is( $sock->socktype, SOCK_STREAM, '$sock->socktype for Type => SOCK_STREAM' );
}

done_testing;
