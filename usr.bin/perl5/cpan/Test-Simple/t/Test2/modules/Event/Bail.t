use strict;
use warnings;
use Test2::Tools::Tiny;
use Test2::Event::Bail;
use Test2::EventFacet::Trace;

my $bail = Test2::Event::Bail->new(
    trace => Test2::EventFacet::Trace->new(frame => ['foo', 'foo.t', 42]),
    reason => 'evil',
);

ok($bail->causes_fail, "bailout always causes fail.");

is($bail->terminate, 255, "Bail will cause the test to exit.");
is($bail->global, 1, "Bail is global, everything should bail");

is($bail->summary, "Bail out!  evil", "Summary includes reason");
$bail->set_reason("");
is($bail->summary, "Bail out!", "Summary has no reason");

ok($bail->diagnostics, "Bail events are counted as diagnostics");

is_deeply(
    $bail->facet_data,
    {
        about => {
            package => 'Test2::Event::Bail',
            eid     => $bail->eid,
        },
        control => {
            global    => 1,
            terminate => 255,
            details   => '',
            halt      => 1
        },
        trace => {
            frame => [
                'foo',
                'foo.t',
                '42',
            ],
            pid => $$,
            tid => 0
        },
    },
    "Got facet data",
);

$bail->set_reason('uhg');
is_deeply(
    $bail->facet_data,
    {
        about => {
            package => 'Test2::Event::Bail',
            eid     => $bail->eid,
        },
        control => {
            global    => 1,
            terminate => 255,
            details   => 'uhg',
            halt      => 1
        },
        trace => {
            frame => [
                'foo',
                'foo.t',
                '42',
            ],
            pid => $$,
            tid => 0
        },
    },
    "Got facet data with reason",
);

done_testing;
