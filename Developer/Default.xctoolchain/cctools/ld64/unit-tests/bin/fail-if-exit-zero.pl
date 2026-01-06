#!/usr/bin/perl -w

use strict;

my $test_name = "";
if ( exists $ENV{UNIT_TEST_NAME} ) {
    $test_name = $ENV{UNIT_TEST_NAME};
}

my $ret = system(@ARGV);
my $exit_value  = $ret >> 8;
my $signal_num  = $ret & 127;
my $dumped_core = $ret & 128;
my $crashed  = $signal_num + $dumped_core;

if(0 == $exit_value || 0 != $crashed)
{
    printf("FAIL $test_name\n");
    exit 1;
}

exit 0;
