use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::API::InterceptResult::Squasher;
use Test2::API::InterceptResult::Event;

my $CLASS = 'Test2::API::InterceptResult::Squasher';

my $trace1 = {pid => $$, tid => 0, cid => 1, frame => ['Foo::Bar', 'Foo/Bar.pm', 42, 'ok']};
my $trace2 = {pid => $$, tid => 0, cid => 2, frame => ['Foo::Bar', 'Foo/Bar.pm', 43, 'note']};
my $trace3 = {pid => $$, tid => 0, cid => 3, frame => ['Foo::Bar', 'Foo/Bar.pm', 44, 'subtest']};
my $trace4 = {pid => $$, tid => 0, cid => 4, frame => ['Foo::Bar', 'Foo/Bar.pm', 45, 'diag']};

my @raw = (
    # These 4 should merge
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace1,
        info => [{tag => 'DIAG', details => 'about to fail'}],
    }),
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace1,
        assert => { pass => 0, details => 'fail' },
    }),
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace1,
        info => [{tag => 'DIAG', details => 'it failed'}],
    }),
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace1,
        info => [{tag => 'DIAG', details => 'it failed part 2'}],
    }),

    # Same trace, but should not merge as it has an assert
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace1,
        assert => { pass => 0, details => 'fail again' },
        info => [{tag => 'DIAG', details => 'it failed again'}],
    }),

    # Stand alone note
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace2,
        info => [{tag => 'NOTE', details => 'Take Note!'}],
    }),

    # Subtest, note, assert, diag as 3 events, should be merged
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace3,
        info => [{tag => 'NOTE', details => 'About to start subtest'}],
    }),
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace3,
        assert => { pass => 0, details => 'failed subtest' },
        parent => { details => 'foo', state => {}, children => [] },
    }),
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace3,
        info => [{tag => 'DIAG', details => 'Subtest failed'}],
    }),

    # Stand alone diag
    Test2::API::InterceptResult::Event->new(facet_data => {
        trace => $trace4,
        info => [{tag => 'DIAG', details => 'Diagnosis: Murder'}],
    }),
);

my @events;
my $squasher = $CLASS->new(events => \@events);
ok($squasher->isa($CLASS), "Got an instanct");
$squasher->process($_) for @raw;
$squasher = undef;

is_deeply(
    [map { $_->facet_data } @events],
    [
        {
            trace  => $trace1,
            assert => {pass => 0, details => 'fail'},
            info   => [
                {tag => 'DIAG', details => 'about to fail'},
                {tag => 'DIAG', details => 'it failed'},
                {tag => 'DIAG', details => 'it failed part 2'},
            ],
        },

        {
            trace  => $trace1,
            assert => {pass => 0, details => 'fail again'},
            info   => [{tag => 'DIAG', details => 'it failed again'}],
        },

        {
            trace => $trace2,
            info  => [{tag => 'NOTE', details => 'Take Note!'}],
        },

        {
            trace  => $trace3,
            assert => {pass => 0, details => 'failed subtest'},
            parent => {details => 'foo', state => {}, children => []},
            info   => [
                {tag => 'NOTE', details => 'About to start subtest'},
                {tag => 'DIAG', details => 'Subtest failed'},
            ],
        },

        {
            trace => $trace4,
            info  => [{tag => 'DIAG', details => 'Diagnosis: Murder'}],
        },
    ],
    "Squashed events as expected"
);

done_testing;
