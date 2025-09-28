use strict;
use warnings;
use Test2::Tools::Tiny;

use Test2::API qw/context intercept/;

use Test2::Event::V2();

my $CLASS = 'Test2::Event::V2';

ok($CLASS->isa('Test2::Event'), "Subclass of Test2::Event");

is_deeply(
    [Test2::Event::V2->non_facet_keys],
    ['uuid', '_meta'],
    "Got non-facet keys"
);

ok($CLASS->can($_), "has method $_") for qw{
    causes_fail diagnostics global increments_count no_display sets_plan
    subtest_id summary terminate
    uuid set_uuid
    meta
    facet_data
    about
};

ok(!exception { $CLASS->new(uuid => 2, about => {uuid => 2}) }, "Can have matching uuids");

like(
    exception { $CLASS->new(uuid => 1, about => {uuid => 2}) },
    qr/uuid '1' passed to constructor, but uuid '2' is already set in the 'about' facet/,
    "Cannot have a uuid mismatch"
);

my $one = $CLASS->new(uuid => 123);
is($one->about->{uuid}, 123, "Set uuid in about facet");

$one = $CLASS->new(about => { uuid => 123 });
is($one->uuid, 123, "set uuid attribute");

my $trace = {frame => ['main', 'file.t', 42, 'foo'], tid => 0, pid => $$};
$one = $CLASS->new(trace => $trace);
ok($trace != $one->trace, "Did not keep or modify the original trace ref");
ok($one->trace->isa('Test2::EventFacet::Trace'), "Blessed the trace");
is_deeply($one->trace, $trace, "Trace has all data");

$one = $CLASS->new;
ok(!$one->uuid, "no uuid attribute");
ok(!($one->about && $one->about->{uuid}), "no uuid in about facet");
$one->set_uuid(123);
is($one->about->{uuid}, 123, "Set uuid in about facet");
is($one->uuid, 123, "set uuid attribute");


$one = $CLASS->new(
    uuid => '123',
    trace => $trace,
    assert => {pass => 1, details => 'pass'},
    info => [{tag => 'NOTE', details => 'a note'}],
);

$one->set_meta('foo' => {'xyz' => 1});

$one->{_custom_sttr} = 'xxx';

is_deeply(
    $one->facet_data,
    {
        trace  => $trace,
        assert => {pass => 1, details => 'pass'},
        info   => [{tag => 'NOTE', details => 'a note'}],
        meta  => {foo  => {'xyz' => 1}},
        about => {uuid => 123},
    },
    "Facet data has everything we want, and nothing we do not"
);

sub my_tool {
    my $ctx = context();

    my $event = $ctx->send_ev2(info => [{tag => 'NOTE', details => "This is a note"}]);

    $ctx->release;

    return $event;
}

my $events = intercept {
    my_tool();
};

is(@$events, 1, "Got 1 event");
ok($events->[0]->isa($CLASS), "Created the right type of event");
is_deeply(
    $events->[0]->facet_data->{info},
    [{tag => 'NOTE', details => "This is a note"}],
    "Got the specified info facet"
);

done_testing;
