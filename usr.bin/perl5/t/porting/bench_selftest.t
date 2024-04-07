#!./perl -w

# run Porting/bench.pl's selftest

use strict;

chdir '..' if -f 'test.pl' && -f 'thread_it.pl';
require './t/test.pl';

system "$^X -I. -MTestInit Porting/bench.pl --action=selftest";
