#!/usr/bin/perl -w
use strict;
use warnings;
use Test::More tests => 3;

use constant NO_SUCH_FILE => 'this_file_had_better_not_exist';

# Keep this test alone in its file as it can be hidden by using autodie outside
# the eval.

# Just to make sure we're absolutely not encountering any weird $@ clobbering
# events, we'll capture a result from our string eval.

my $result = eval q{
    use autodie "open";

    open(my $fh, '<', NO_SUCH_FILE);

    1;
};

ok( ! $result, "Eval should fail with autodie/no such file");
ok($@, "enabling autodie in string eval should throw an exception");
isa_ok($@, 'autodie::exception');
