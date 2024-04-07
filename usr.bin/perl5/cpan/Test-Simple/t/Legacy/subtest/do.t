#!/usr/bin/perl -w

# Test the idiom of running another test file as a subtest.

use strict;
use Test::More;

pass("First");

my $file = "./t/Legacy/subtest/for_do_t.test";
ok -e $file, "subtest test file exists";

subtest $file => sub { do $file };

pass("Last");

done_testing(4);
