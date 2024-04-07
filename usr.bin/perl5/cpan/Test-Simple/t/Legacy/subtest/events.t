use strict;
use warnings;

use Test::More;
use Test2::API qw/intercept/;

my $events = intercept {
    subtest foo => sub {
        ok(1, "pass");
    };
};

my $st = $events->[-1];
isa_ok($st, 'Test2::Event::Subtest');
ok(my $id = $st->subtest_id, "got an id");
for my $se (@{$st->subevents}) {
    is($se->trace->hid, $id, "set subtest_id on child event");
}

done_testing;
