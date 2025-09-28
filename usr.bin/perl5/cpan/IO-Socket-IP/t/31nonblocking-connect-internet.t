#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;

use IO::Socket::INET;
use Errno qw( EINPROGRESS EWOULDBLOCK ECONNREFUSED );

# Chris Williams (BINGOS) has offered cpanidx.org as a TCP testing server here
my $test_host = "cpanidx.org";
my $test_good_port = 80;
my $test_bad_port = 6666;

SKIP: {
   IO::Socket::INET->new(
      PeerHost => $test_host,
      PeerPort => $test_good_port,
      Type     => SOCK_STREAM,
      Timeout  => 3,
   ) or skip "Can't connect to $test_host:$test_good_port", 5;

   my $socket = IO::Socket::IP->new(
      PeerHost    => $test_host,
      PeerService => $test_good_port,
      Type        => SOCK_STREAM,
      Blocking    => 0,
   );

   ok( defined $socket, "defined \$socket for $test_host:$test_good_port" ) or
      diag( "  error was $@" );

   ok( defined $socket->fileno, '$socket has fileno' );

   ok( !$socket->connected, '$socket not yet connected' );

   while( !$socket->connect and ( $! == EINPROGRESS || $! == EWOULDBLOCK ) ) {
      my $wvec = '';
      vec( $wvec, fileno $socket, 1 ) = 1;
      my $evec = '';
      vec( $evec, fileno $socket, 1 ) = 1;

      my $ret = select( undef, $wvec, $evec, 60 );
      defined $ret or die "Cannot select() - $!";
      $ret or die "select() timed out";
   }

   ok( !$!, '->connect eventually succeeds' );

   ok( $socket->connected, '$socket now connected' );
}

SKIP: {
   IO::Socket::INET->new(
      PeerHost => $test_host,
      PeerPort => $test_bad_port,
      Type     => SOCK_STREAM,
      Timeout  => 3,
   ) and skip "Connecting to $test_host:$test_bad_port succeeds", 4;
   $! == ECONNREFUSED or skip "Connecting to $test_host:$test_bad_port doesn't give ECONNREFUSED", 4;

   my $socket = IO::Socket::IP->new(
      PeerHost    => $test_host,
      PeerService => $test_bad_port,
      Type        => SOCK_STREAM,
      Blocking    => 0,
   );

   ok( defined $socket, "defined \$socket for $test_host:$test_bad_port" ) or
      diag( "  error was $@" );

   ok( defined $socket->fileno, '$socket has fileno' );

   ok( !$socket->connected, '$socket not yet connected' );

   while( !$socket->connect and ( $! == EINPROGRESS || $! == EWOULDBLOCK ) ) {
      my $wvec = '';
      vec( $wvec, fileno $socket, 1 ) = 1;
      my $evec = '';
      vec( $evec, fileno $socket, 1 ) = 1;

      my $ret = select( undef, $wvec, $evec, 60 );
      defined $ret or die "Cannot select() - $!";
      $ret or die "select() timed out";
   }

   my $dollarbang = $!;

   ok( $dollarbang == ECONNREFUSED, '->connect eventually fails with ECONNREFUSED' ) or
      diag( "  dollarbang = $dollarbang" );
}

done_testing;
