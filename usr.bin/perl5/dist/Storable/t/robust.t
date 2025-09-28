#!/usr/bin/perl

# This test script checks that Storable will load properly if someone
# is incorrectly messing with %INC to hide Log::Agent.  No, no-one should
# really be doing this, but, then, it *used* to work!

use Test::More;
plan tests => 1;

$INC{'Log/Agent.pm'} = '#ignore#';
require Storable;
pass;
