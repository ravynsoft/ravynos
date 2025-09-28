#!/usr/bin/perl -Tw

use strict;
use warnings;
use lib 't/lib';

use Test::More tests => 2;
use Test::Harness;

my $ver = $ENV{HARNESS_VERSION} or die "HARNESS_VERSION not set";
ok( $ver =~ /^[23].\d\d(_\d\d)?$/, "Version is proper format" );
is( $ver, $Test::Harness::VERSION );
