#!./perl

# This tests that the (?[...]) feature doesn't introduce unexpected
# differences from regular bracketed character classes.  It just sets a flag
# and calls regexp.t which will run through its test suite, modifiying the
# tests to use (?[...]) instead wherever the test uses [].

BEGIN { $regex_sets = 1; }
for $file ('./re/regexp.t', './t/re/regexp.t', ':re:regexp.t') {
    if (-r $file) {
	do $file or die $@;
	exit;
    }
}
die "Cannot find ./re/regexp.t or ./t/re/regexp.t\n";
