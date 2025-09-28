use strict;
use warnings;

use Test2::Util qw/CAN_THREAD/;
BEGIN {
    unless(CAN_THREAD) {
        require Test::More;
        Test::More->import(skip_all => "threads are not supported");
    }
}

use threads;
use Test::More;

my $t = threads->create(
    sub {
        local $TODO = "Some good reason";

        fail "Crap";

        42;
    }
);

is(
    $t->join,
    42,
    "Thread exitted successfully"
);

done_testing;
