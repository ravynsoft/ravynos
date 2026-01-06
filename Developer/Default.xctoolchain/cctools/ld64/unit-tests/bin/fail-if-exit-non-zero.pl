#!/usr/bin/perl -w

use strict;

my $test_name = "";
if ( exists $ENV{UNIT_TEST_NAME} ) {
	$test_name = $ENV{UNIT_TEST_NAME};
}

if(system(@ARGV) != 0)
{
    printf("FAIL $test_name\n");
    exit 1;
}

exit 0;
