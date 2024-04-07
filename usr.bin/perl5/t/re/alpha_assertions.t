#!./perl

use strict;
use warnings;
no warnings 'once';
no warnings 'experimental::vlb';

# This tests that the alphabetic assertions, like '(*atomic:...) work
# It just sets a flag and calls regexp.t which will run through its test
# suite, modifiying the tests to use the alphabetic synonyms.

BEGIN { $::alpha_assertions = 1; }
for my $file ('./re/regexp.t', './t/re/regexp.t', ':re:regexp.t') {
    if (-r $file) {
	do $file or die $@;
	exit;
    }
}
die "Cannot find ./re/regexp.t or ./t/re/regexp.t\n";
