#!/usr/bin/perl

use strict;
use warnings;

use Test::More;

plan tests => 3;

use Errno qw( EINVAL );

# Keep this unit test in a file of its own because we need to override
# connect() globally
BEGIN {
    *CORE::GLOBAL::connect = sub { $! = EINVAL; return undef };
}

my $EINVAL_STR = do { local $! = EINVAL; "$!" };

use IO::Socket;

# test that error strings turn up in both places
my $sock = IO::Socket::INET->new(
    PeerHost => "localhost",
    PeerPort => 1,
);
my $e = $@;

ok(!defined $sock, 'fails to connect with CORE::GLOBAL::connect override');

is($IO::Socket::errstr, "IO::Socket::INET: connect: $EINVAL_STR",
    'error message appears in $IO::Socket::errstr');
is($e, "IO::Socket::INET: connect: $EINVAL_STR",
    'error message appeared in $@');
