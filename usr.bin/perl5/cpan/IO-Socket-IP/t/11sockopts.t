#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;

use Errno qw( EACCES );
use Socket qw( SOL_SOCKET SO_REUSEADDR SO_REUSEPORT SO_BROADCAST );

TODO: {
   local $TODO = "SO_REUSEADDR doesn't appear to work on cygwin smokers" if $^O eq "cygwin";
   # I honestly have no idea why this fails, and people don't seem to be able
   # to reproduce it on a development box. I'll mark it TODO for now until we
   # can gain any more insight into it.

   my $sock = IO::Socket::IP->new(
      LocalHost => "127.0.0.1",
      Type      => SOCK_STREAM,
      Listen    => 1,
      ReuseAddr => 1,
   ) or die "Cannot socket() - $@";

   ok( $sock->getsockopt( SOL_SOCKET, SO_REUSEADDR ), 'SO_REUSEADDR set' );

   $sock = IO::Socket::IP->new(
      LocalHost => "127.0.0.1",
      Type      => SOCK_STREAM,
      Listen    => 1,
      Sockopts  => [
         [ SOL_SOCKET, SO_REUSEADDR ],
      ],
   ) or die "Cannot socket() - $@";

   ok( $sock->getsockopt( SOL_SOCKET, SO_REUSEADDR ), 'SO_REUSEADDR set via Sockopts' );
}

SKIP: {
   # Some OSes don't implement SO_REUSEPORT
   skip "No SO_REUSEPORT constant", 1 unless defined eval { SO_REUSEPORT };
   skip "No support for SO_REUSEPORT", 1 unless defined eval {
      my $s;
      socket( $s, Socket::PF_INET, Socket::SOCK_STREAM, 0 ) and
         setsockopt( $s, SOL_SOCKET, SO_REUSEPORT, 1 ) };

   my $sock = IO::Socket::IP->new(
      LocalHost => "127.0.0.1",
      Type      => SOCK_STREAM,
      Listen    => 1,
      ReusePort => 1,
   ) or die "Cannot socket() - $@";

   ok( $sock->getsockopt( SOL_SOCKET, SO_REUSEPORT ), 'SO_REUSEPORT set' );
}

SKIP: {
   # Some OSes need special privileges to set SO_BROADCAST
   $! = 0;
   my $sock = IO::Socket::IP->new(
      LocalHost => "127.0.0.1",
      Type      => SOCK_DGRAM,
      Broadcast => 1,
   );
   skip "Privileges required to set broadcast on datagram socket", 1 if !$sock and $! == EACCES;
   die "Cannot socket() - $@" unless $sock;

   ok( $sock->getsockopt( SOL_SOCKET, SO_BROADCAST ), 'SO_BROADCAST set' );
}

done_testing;
