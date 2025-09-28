#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;

foreach my $socktype (qw( SOCK_STREAM SOCK_DGRAM )) {
   my $testserver = IO::Socket::IP->new(
      ( $socktype eq "SOCK_STREAM" ? ( Listen => 1 ) : () ),
      LocalHost => "127.0.0.1",
      LocalPort => "0",
      Type      => Socket->$socktype,
   ) or die "Cannot listen on PF_INET - $@";

   my $socket = IO::Socket::IP->new(
      PeerHost    => "127.0.0.1",
      PeerService => $testserver->sockport,
      Type        => Socket->$socktype,
   ) or die "Cannot connect on PF_INET - $@";

   my $testclient = ( $socktype eq "SOCK_STREAM" ) ? 
      $testserver->accept : 
      do { $testserver->connect( $socket->sockname ); $testserver };

   is( $testclient->sockport, $socket->peerport, "\$testclient->sockport for $socktype" );
   is( $testclient->peerport, $socket->sockport, "\$testclient->peerport for $socktype" );

   is( $testclient->sockhost, $socket->peerhost, "\$testclient->sockhost for $socktype" );
   is( $testclient->peerhost, $socket->sockhost, "\$testclient->peerhost for $socktype" );

   $socket->write( "Request\n" );
   is( $testclient->getline, "Request\n", "\$socket to \$testclient for $socktype" );

   $testclient->write( "Response\n" );
   is( $socket->getline, "Response\n", "\$testclient to \$socket for $socktype" );
}

done_testing;
