use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::Event::Subtest;
my $st = 'Test2::Event::Subtest';

my $trace = Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__, 'xxx']);
my $one = $st->new(
    trace     => $trace,
    pass      => 1,
    buffered  => 1,
    name      => 'foo',
    subtest_id => "1-1-1",
);

ok($one->isa('Test2::Event::Ok'), "Inherit from Ok");
is_deeply($one->subevents, [], "subevents is an arrayref");

is($one->summary, "foo", "simple summary");
$one->set_todo('');
is($one->summary, "foo (TODO)", "simple summary + TODO");
$one->set_todo('foo');
is($one->summary, "foo (TODO: foo)", "simple summary + TODO + Reason");

$one->set_todo(undef);
$one->set_name('');
is($one->summary, "Nameless Subtest", "unnamed summary");

require Test2::Event::Pass;
push @{$one->subevents} => Test2::Event::Pass->new(name => 'xxx');

my $facet_data = $one->facet_data;
ok($facet_data->{about}, "got parent facet data");

is_deeply(
    $facet_data->{parent},
    {
        hid      => "1-1-1",
        buffered => 1,
        children => [
            {
                about => {
                    details => 'pass',
                    package => 'Test2::Event::Pass',
                    eid     => $one->subevents->[0]->eid,
                },
                assert => {
                    details => 'xxx',
                    pass    => 1
                },
            }
        ],
    },
    "Got facet data"
);

$one->{start_stamp} = 123;
$one->{stop_stamp} = 456;

$facet_data = $one->facet_data;
is_deeply(
    $facet_data->{parent},
    {
        hid      => "1-1-1",
        buffered => 1,
        start_stamp => 123,
        stop_stamp  => 456,
        children => [
            {
                about => {
                    details => 'pass',
                    package => 'Test2::Event::Pass',
                    eid     => $one->subevents->[0]->eid,
                },
                assert => {
                    details => 'xxx',
                    pass    => 1
                },
            }
        ],
    },
    "Got facet data with stamps"
);


done_testing;
