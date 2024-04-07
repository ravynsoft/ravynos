#!/usr/bin/perl
use strict;
BEGIN {
	$|  = 1;
	$^W = 1;
}

use Test::More tests => 2;

# Check their perl version
ok( $] >= 5.006001, "Your perl is new enough" );

# Does the module load
use_ok( 'ExtUtils::Typemaps'   );
