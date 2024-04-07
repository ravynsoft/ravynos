#!/usr/bin/perl -w
use strict;
use warnings;
use Test::More 'no_plan';
use autodie;

use constant NO_SUCH_FILE => 'this_file_had_better_not_exist';
use constant MAGIC_STRING => 'xyzzy';

# Opening an eval clears $@, so it's important that we set it
# inside the eval block to see if it's successfully captured.

eval {
    $@ = MAGIC_STRING;
    is($@, MAGIC_STRING, 'Sanity check on start conditions');
    open(my $fh, '<', NO_SUCH_FILE);
};

isa_ok($@, 'autodie::exception');
is($@->eval_error, MAGIC_STRING, 'Previous $@ should be captured');
