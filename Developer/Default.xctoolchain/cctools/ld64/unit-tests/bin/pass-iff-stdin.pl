#!/usr/bin/perl -w

#
# Usage:
#
#		command | ${PASS_IFF_STDIN} 
#

use strict;

my $test_name = "";
if ( exists $ENV{UNIT_TEST_NAME} ) {
	$test_name = $ENV{UNIT_TEST_NAME};
}

if( eof STDIN )
{
    printf("FAIL $test_name\n");
    exit 1
}

printf("PASS $test_name\n");
exit 0;

