#!/usr/bin/perl

use v5;
use strict;
use warnings;

use Test::More;

use IO::Socket::IP;
use Socket qw( AF_INET SOCK_STREAM );

socket( my $tmph, AF_INET, SOCK_STREAM, 0 ) or die "Cannot socket() - $!";

my $socket = IO::Socket::IP->new or die "Cannot create IO::Socket::IP - $@";

$socket->socket( AF_INET, SOCK_STREAM, 0 ) or die "Cannot socket() - $!";
my $fileno = $socket->fileno;

$socket->socket( AF_INET, SOCK_STREAM, 0 ) or die "Cannot socket() - $!";

is( $socket->fileno, $fileno, '$socket->fileno preserved after ->socket' );

close $tmph;

$socket->socket( AF_INET, SOCK_STREAM, 0 ) or die "Cannot socket() - $!";

is( $socket->fileno, $fileno, '$socket->fileno preserved after ->socket with free handle' );

done_testing;
