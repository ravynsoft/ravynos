#!/usr/bin/perl

use strict;
use warnings;

use Test::More;

BEGIN {
    eval 'use Import::Into 1.002004';
    plan skip_all => 'Test needs Import::Into >= 1.002004' if $@;
}

use FindBin;
use lib "$FindBin::Bin/lib";

use my::pragma qw(open);

plan tests => 1;

my::pragma->dont_die();

eval {
    open(my $fd, '<', 'random-file');
};
ok($@, 'my::pragma can use import::into');

