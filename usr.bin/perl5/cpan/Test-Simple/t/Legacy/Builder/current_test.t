#!/usr/bin/perl -w

# Dave Rolsky found a bug where if current_test() is used and no
# tests are run via Test::Builder it will blow up.

use strict;
use warnings;

use Test::Builder;
my $TB = Test::Builder->new;
$TB->plan(tests => 2);
print "ok 1\n";
print "ok 2\n";
$TB->current_test(2);
