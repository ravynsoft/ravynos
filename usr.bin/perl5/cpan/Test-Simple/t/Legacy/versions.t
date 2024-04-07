#!/usr/bin/perl -w
# HARNESS-NO-PRELOAD

# Make sure all the modules have the same version
#
# TBT has its own version system.

use strict;
use Test::More;

require Test::Builder;
require Test::Builder::Module;
require Test::Simple;

my $dist_version = Test::More->VERSION;

like( $dist_version, qr/^ \d+ \. \d+ $/x );

my @modules = qw(
    Test::Simple
    Test::Builder
    Test::Builder::Module
);

for my $module (@modules) {
    is( $dist_version, $module->VERSION, $module );
}

done_testing(4);
