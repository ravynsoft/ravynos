#!/usr/bin/perl

use warnings;
use strict;

use Test::More;

eval { require XS::APItest; XS::APItest->import('sv_count'); 1 }
  or plan skip_all => "No XS::APItest::sv_count() available";

plan tests => 1;

sub leak {
    my ($n, $delta, $code, $name) = @_;
    my $sv0 = 0;
    my $sv1 = 0;
    for my $i (1..$n) {
	&$code();
	$sv1 = sv_count();
	$sv0 = $sv1 if $i == 1;
    }
    cmp_ok($sv1-$sv0, '<=', ($n-1)*$delta, $name);
}

# [perl #129788] IO::Poll shouldn't leak on errors
{
    package io_poll_leak;
    use IO::Poll;

    sub TIESCALAR { bless {} }
    sub FETCH { die }

    tie(my $a, __PACKAGE__);
    sub f {eval { IO::Poll::_poll(0, $a, 1) }}

    ::leak(5, 0, \&f, q{IO::Poll::_poll shouldn't leak});
}
