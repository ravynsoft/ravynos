#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket;
use IO::Socket::IP -register;

# AF_INET
{
   my $sock = IO::Socket->new(
      Domain    => AF_INET,
      Type      => SOCK_STREAM,
      LocalHost => "127.0.0.1",
      LocalPort => 0,
      GetAddrInfoFlags => 0, # disable AI_ADDRCONFIG
   );

   isa_ok( $sock, "IO::Socket::IP", 'IO::Socket->new( Domain => AF_INET )' ) or
      diag( "  error was $@" );

   $sock = IO::Socket->new(
      Domain    => AF_INET,
      Type      => SOCK_STREAM,
      LocalHost => "::1",
   );

   ok( !defined $sock, 'Domain => AF_INET, LocalHost => "::1" fails' );
}

SKIP: {
   my $AF_INET6 = eval { Socket::AF_INET6() } ||
                  eval { require Socket6; Socket6::AF_INET6() };
   $AF_INET6 or skip "No AF_INET6", 1;
   eval { IO::Socket::IP->new( LocalHost => "::1" ) } or
      skip "Unable to bind to ::1", 1;

   my $sock = IO::Socket->new(
      Domain    => $AF_INET6,
      Type      => SOCK_STREAM,
      LocalHost => "::1",
      LocalPort => 0,
      GetAddrInfoFlags => 0, # disable AI_ADDRCONFIG
   );

   isa_ok( $sock, "IO::Socket::IP", 'IO::Socket->new( Domain => AF_INET6 )' ) or
      diag( "  error was $@" );

   $sock = IO::Socket->new(
      Domain    => $AF_INET6,
      Type      => SOCK_STREAM,
      LocalHost => "127.0.0.1",
   );

   ok( !defined $sock, 'Domain => AF_INET6, LocalHost => "127.0.0.1" fails' );
}

done_testing;
