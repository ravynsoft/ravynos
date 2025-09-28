#!/usr/bin/perl -w

use strict;
use warnings;

use Test2::Util qw/CAN_THREAD/;
BEGIN {
    unless(CAN_THREAD) {
        require Test::More;
        Test::More->import(skip_all => "threads are not supported");
    }
}
use threads;

use Test::More;

subtest 'simple test with threads on' => sub {
    is( 1+1, 2,   "simple test" );
    is( "a", "a", "another simple test" );
};

pass("Parent retains sharedness");

done_testing(2);
