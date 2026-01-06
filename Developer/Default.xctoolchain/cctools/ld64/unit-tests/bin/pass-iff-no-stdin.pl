#!/usr/bin/perl -w

#
# Usage:
#
#		command | ${PASS_IFF_EMPTY} 
#

use strict;

my $test_name = "";
if ( exists $ENV{UNIT_TEST_NAME} ) {
	$test_name = $ENV{UNIT_TEST_NAME};
}

if( eof STDIN )
{
    printf("PASS $test_name\n");
    exit 0;
}

printf("FAIL $test_name\n");
exit 1;
