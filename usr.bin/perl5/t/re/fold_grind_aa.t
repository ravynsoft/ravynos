#!./perl

# Call fold_grind with /aa

use strict;
use warnings;
no warnings 'once';

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    require './loc_tools.pl';
}

$::TEST_CHUNK = 'aa';

do './re/fold_grind.pl';
print STDERR "$@\n" if $@;
print STDERR "$!\n" if $!;
