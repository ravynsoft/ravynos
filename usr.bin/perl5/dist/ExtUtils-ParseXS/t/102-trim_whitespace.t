#!/usr/bin/perl
use strict;
use warnings;
use Test::More tests =>  5;
use ExtUtils::ParseXS::Utilities qw(
  trim_whitespace
);

my $str;

$str = 'overworked';
trim_whitespace($str);
is( $str, 'overworked', "Got expected value" );

$str = '  overworked';
trim_whitespace($str);
is( $str, 'overworked', "Got expected value" );

$str = 'overworked  ';
trim_whitespace($str);
is( $str, 'overworked', "Got expected value" );

$str = '  overworked  ';
trim_whitespace($str);
is( $str, 'overworked', "Got expected value" );

$str = "\toverworked";
trim_whitespace($str);
is( $str, 'overworked', "Got expected value" );
