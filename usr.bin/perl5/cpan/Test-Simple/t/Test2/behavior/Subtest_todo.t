use strict;
use warnings;

use Test2::Tools::Tiny;

use Test2::API qw/run_subtest intercept/;

my $events = intercept {
    todo 'testing todo', sub {
        run_subtest(
            'fails in todo',
            sub {
                ok(1, 'first passes');
                ok(0, 'second fails');
            }
        );
    };
};

ok($events->[1],                 'Test2::Event::Subtest', 'subtest ran');
ok($events->[1]->effective_pass, 'Test2::Event::Subtest', 'subtest effective_pass is true');
ok($events->[1]->todo,           'testing todo',          'subtest todo is set to expected value');

my $subevents = $events->[1]->subevents;

is(scalar @$subevents, 3, 'got subevents in the subtest');

ok($subevents->[0]->facets->{assert}->pass, 'first event passed');

ok(!$subevents->[1]->facets->{assert}->pass, 'second event failed');
ok(!$subevents->[1]->causes_fail,    'second event does not cause failure');

done_testing;
