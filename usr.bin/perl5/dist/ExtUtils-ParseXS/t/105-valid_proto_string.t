#!/usr/bin/perl
use strict;
use warnings;
use Test::More tests =>  6;
use ExtUtils::ParseXS::Utilities qw(
  valid_proto_string
);

my ($input, $output);

$input = '[\$]';
$output = valid_proto_string($input);
is( $output, $input, "Got expected value for <$input>" );

$input = '[$]';
$output = valid_proto_string($input);
is( $output, $input, "Got expected value for <$input>" );

$input = '[\$\@]';
$output = valid_proto_string($input);
is( $output, $input, "Got expected value for <$input>" );

$input = '[\$alpha]';
$output = valid_proto_string($input);
is( $output, 0, "Got expected value for <$input>" );

$input = '[alpha]';
$output = valid_proto_string($input);
is( $output, 0, "Got expected value for <$input>" );

$input = '[_]';
$output = valid_proto_string($input);
is( $output, $input, "Got expected value for <$input>" );

