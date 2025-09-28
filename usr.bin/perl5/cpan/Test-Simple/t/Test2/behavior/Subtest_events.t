use strict;
use warnings;

use Test2::Tools::Tiny;

use Test2::API qw/run_subtest intercept/;

my $events = intercept {
    my $code = sub { ok(1) };
    run_subtest('blah', $code, 'buffered');
};

ok(!$events->[0]->trace->nested, "main event is not inside a subtest");
ok($events->[0]->subtest_id, "Got subtest id");
is($events->[0]->subevents->[0]->trace->hid, $events->[0]->subtest_id, "nested events are in the subtest");

done_testing;
