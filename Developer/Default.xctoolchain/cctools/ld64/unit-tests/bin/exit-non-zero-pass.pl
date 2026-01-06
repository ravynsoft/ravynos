#!/usr/bin/perl -w

#
# Usage:
#
#		${PASS_UNLESS} "test name" command
#

use strict;

my $string = shift @ARGV;
my $ret = system(@ARGV);
my $exit_value  = $ret >> 8;
my $signal_num  = $ret & 127;
my $dumped_core = $ret & 128;
my $crashed  = $signal_num + $dumped_core;

if(0 == $exit_value || 0 != $crashed)
{
    printf("FAIL $string\n");
}
else
{
    printf("PASS $string\n");
}

exit 0;
