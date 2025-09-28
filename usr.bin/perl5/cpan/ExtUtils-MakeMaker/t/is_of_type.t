#!/usr/bin/perl -w

# Test _is_of_type()

BEGIN {
    chdir 't' if -d 't';
}

use lib './lib';
use strict;
use warnings;
use ExtUtils::MakeMaker;

use Test::More "no_plan";

my $is_of_type = \&ExtUtils::MakeMaker::_is_of_type;

my @tests = (
    [23,                "",     1],
    [[],                "",     0],
    [{},                "",     0],
    [[],                "HASH", 0],
    [{},                "HASH", 1],
    [bless({}, "Foo"),  "Foo",  1],
    [bless({}, "Bar"),  "Foo",  0],
    [bless([], "Foo"),  "",     0],
    [bless([], "Foo"),  "HASH", 0],
    [bless([], "Foo"),  "ARRAY", 1],
);

for my $test (@tests) {
    my($thing, $type, $want) = @$test;

    # [rt.cpan.org 41060]
    local $SIG{__DIE__} = sub { fail("sigdie should be ignored") };
    is !!$is_of_type->($thing, $type), !!$want, qq[_is_of_type($thing, '$type'): $want];
}
