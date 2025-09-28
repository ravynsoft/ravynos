#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;

my $server = IO::Socket::IP->new(
   Listen    => 1,
   LocalHost => "127.0.0.1",
   LocalPort => 0,
) or die "Cannot listen on PF_INET - $!";

my $client = IO::Socket::IP->new(
   PeerHost => $server->sockhost,
   PeerPort => $server->sockport,
   Timeout  => 0.1,
) or die "Cannot connect on PF_INET - $!";

ok( defined $client, 'client constructed with Timeout' );
ok( $client->blocking, 'client is in blocking mode after connect' );

my $accepted = $server->accept
   or die "Cannot accept - $!";

ok( defined $accepted, 'accepted a client' );

done_testing;
