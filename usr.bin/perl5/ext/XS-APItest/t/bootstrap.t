#!perl -w
#
# check that .bs files are loaded and executed.
# During build of  XS::APItest, the presence of APItest_BS should
# cause a non-empty APItest.bs file to auto-generated. When loading
# APItest.so, the .bs should be automatically executed, which should
# set $::bs_file_got_executed.

use strict;

use Test::More;
use XS::APItest;

is $::bs_file_got_executed, 1, "BS file was executed once";

done_testing();

