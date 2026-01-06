#!/usr/bin/perl -w

#
# Usage:
#
#		command | ${FAIL_IF_EMPTY}
#

use strict;

my $test_name = "";
if ( exists $ENV{UNIT_TEST_NAME} ) {
	$test_name = $ENV{UNIT_TEST_NAME};
}

if( eof STDIN )
{
    printf("FAIL $test_name\n");
    exit 1;
}

exit 0;
