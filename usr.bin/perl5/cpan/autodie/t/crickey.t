#!/usr/bin/perl -w
use strict;
use FindBin;
use Test::More 'no_plan';

use lib "$FindBin::Bin/lib";

use constant NO_SUCH_FILE => "crickey_mate_this_file_isnt_here_either";

use autodie::test::au qw(open);

eval {
    open(my $fh, '<', NO_SUCH_FILE);
};

ok(my $e = $@, 'Strewth!  autodie::test::au should throw an exception on failure');

isa_ok($e, 'autodie::test::au::exception',
    'Yeah mate, that should be our test exception.');

like($e, qr/time for a beer/, "Time for a beer mate?");

like( eval { $e->time_for_a_beer; },
    qr/time for a beer/, "It's always a good time for a beer."
);

ok($e->matches('open'), "Should be a fair dinkum error from open");
