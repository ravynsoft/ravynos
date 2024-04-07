#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;

sub arguments_is {
   my ($arg, $exp, $name) = @_;

   $arg = [$arg]
   unless ref $arg;

   $name ||= join ' ', map { defined $_ ? $_ : 'undef' } @$arg;

   my $got = do {
      no warnings 'redefine';
      my $args;

      local *IO::Socket::IP::_io_socket_ip__configure = sub {
         $args = $_[1];
         return $_[0];
      };

      IO::Socket::IP->new(@$arg);

      $args;
   };

   is_deeply($got, $exp, $name);
}

my @tests = (
   [ [ '[::1]:80'               ], { PeerHost  => '::1',           PeerService => '80'    } ],
   [ [ '[::1]:http'             ], { PeerHost  => '::1',           PeerService => 'http'  } ],
   [ [ '[::1]'                  ], { PeerHost  => '::1',                                  } ],
   [ [ '[::1]:'                 ], { PeerHost  => '::1',                                  } ],
   [ [ '127.0.0.1:80'           ], { PeerHost  => '127.0.0.1',     PeerService => '80'    } ],
   [ [ '127.0.0.1:http'         ], { PeerHost  => '127.0.0.1',     PeerService => 'http'  } ],
   [ [ '127.0.0.1'              ], { PeerHost  => '127.0.0.1',                            } ],
   [ [ '127.0.0.1:'             ], { PeerHost  => '127.0.0.1',                            } ],
   [ [ 'localhost:80'           ], { PeerHost  => 'localhost',     PeerService => '80'    } ],
   [ [ 'localhost:http'         ], { PeerHost  => 'localhost',     PeerService => 'http'  } ],
   [ [ PeerHost  => '[::1]:80'  ], { PeerHost  => '::1',           PeerService => '80'    } ],
   [ [ PeerHost  => '[::1]'     ], { PeerHost  => '::1'                                   } ],
   [ [ LocalHost => '[::1]:80'  ], { LocalHost => '::1',           LocalService => '80'   } ],
   [ [ LocalHost => undef       ], { LocalHost => undef                                   } ],

   # IO::Socket::INET is happy to take port from the *Host argument even if a *Port argument
   # exists
   [ [ PeerHost => '127.0.0.1:80', PeerPort => '80' ], { PeerHost => '127.0.0.1', PeerService => '80' } ],
   # *Host argument should take precedence over *Service if both exist
   [ [ PeerHost => '127.0.0.1:443', PeerPort => '80' ], { PeerHost => '127.0.0.1', PeerService => '443' } ],
);

is_deeply( [ IO::Socket::IP->split_addr( "hostname:http" ) ],
           [ "hostname",  "http" ],
           "split_addr hostname:http" );

is_deeply( [ IO::Socket::IP->split_addr( "192.0.2.1:80" ) ],
           [ "192.0.2.1", "80"   ],
           "split_addr 192.0.2.1:80" );

is_deeply( [ IO::Socket::IP->split_addr( "[2001:db8::1]:80" ) ],
           [ "2001:db8::1", "80" ],
           "split_addr [2001:db8::1]:80" );

is_deeply( [ IO::Socket::IP->split_addr( "something.else" ) ],
           [ "something.else", undef ],
           "split_addr something.else" );

is( IO::Socket::IP->join_addr( "hostname", "http" ),
    "hostname:http",
    'join_addr hostname:http' );

is( IO::Socket::IP->join_addr( "192.0.2.1", 80 ),
    "192.0.2.1:80",
    'join_addr 192.0.2.1:80' );

is( IO::Socket::IP->join_addr( "2001:db8::1", 80 ),
    "[2001:db8::1]:80",
    'join_addr [2001:db8::1]:80' );

is( IO::Socket::IP->join_addr( "something.else", undef ),
    "something.else",
    'join_addr something.else' );

arguments_is(@$_) for @tests;

done_testing;
