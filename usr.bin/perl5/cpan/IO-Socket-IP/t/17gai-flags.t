#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;
use Socket 1.95 qw(
   PF_INET SOCK_STREAM IPPROTO_TCP pack_sockaddr_in INADDR_ANY
   AI_PASSIVE AI_NUMERICSERV
);

my $AI_ADDRCONFIG = eval { Socket::AI_ADDRCONFIG() } || 0;

my @gai_args;
my @gai_rets;

no strict 'refs';
no warnings 'redefine';

*{"IO::Socket::IP::getaddrinfo"} = sub {
   push @gai_args, [ @_ ];
   return @{ shift @gai_rets };
};

@gai_args = ();
@gai_rets = (
   [ "", {
         family   => PF_INET,
         socktype => SOCK_STREAM,
         protocol => IPPROTO_TCP,
         addr     => pack_sockaddr_in( 80, INADDR_ANY )
      } ],
);
IO::Socket::IP->new( LocalPort => "80" );

is_deeply( \@gai_args,
           [ 
              [ undef, "80", { flags => AI_PASSIVE|$AI_ADDRCONFIG, socktype => SOCK_STREAM, protocol => IPPROTO_TCP } ],
           ],
           '@gai_args for LocalPort => "80"' );

SKIP: {
   skip "No AI_NUMERICSERV", 1 unless defined eval { AI_NUMERICSERV() };

   @gai_args = ();
   @gai_rets = (
      [ "", {
            family   => PF_INET,
            socktype => SOCK_STREAM,
            protocol => IPPROTO_TCP,
            addr     => pack_sockaddr_in( 80, INADDR_ANY )
         } ],
   );
   IO::Socket::IP->new( LocalPort => "80", GetAddrInfoFlags => AI_NUMERICSERV );

   is_deeply( \@gai_args,
              [ 
                 [ undef, "80", { flags => AI_PASSIVE|AI_NUMERICSERV, socktype => SOCK_STREAM, protocol => IPPROTO_TCP } ],
              ],
              '@gai_args for LocalPort => "80", GetAddrInfoFlags => AI_NUMERICSERV' );
}

done_testing;
