#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;
use Socket 1.95 qw(
   PF_INET SOCK_STREAM IPPROTO_TCP pack_sockaddr_in INADDR_ANY
   AI_PASSIVE
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

@gai_rets = (
   [ "Service unknown" ],
   [ "", {
         family   => PF_INET,
         socktype => SOCK_STREAM,
         protocol => IPPROTO_TCP,
         addr     => pack_sockaddr_in( 80, INADDR_ANY )
      } ],
);

IO::Socket::IP->new( LocalPort => "zyxxyblarg(80)" );

is_deeply( \@gai_args,
           [ 
              [ undef, "zyxxyblarg", { flags => AI_PASSIVE|$AI_ADDRCONFIG, socktype => SOCK_STREAM, protocol => IPPROTO_TCP } ],
              [ undef, "80",         { flags => AI_PASSIVE|$AI_ADDRCONFIG, socktype => SOCK_STREAM, protocol => IPPROTO_TCP } ],
           ],
           '@gai_args for LocalPort => "zyxxyblarg(80)"' );

done_testing;
