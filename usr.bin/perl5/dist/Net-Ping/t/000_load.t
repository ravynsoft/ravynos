#!perl -T
use 5.006;
use strict;
use warnings FATAL => 'all';
use Test::More;

plan tests => 3;

BEGIN {
    use_ok( 'Socket' )      || print "No Socket!\n";
    use_ok( 'Time::HiRes' ) || print "No Time::HiRes!\n";
    use_ok( 'Net::Ping' )   || print "No Net::Ping!\n";
}

note( "Testing Net::Ping $Net::Ping::VERSION, Perl $], $^X" );

