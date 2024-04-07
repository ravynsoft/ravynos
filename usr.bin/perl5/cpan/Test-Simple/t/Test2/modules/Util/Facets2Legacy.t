use strict;
use warnings;
use Test2::Tools::Tiny;

use Test2::Util::Facets2Legacy ':ALL';

my $CLASS;
BEGIN {
    $CLASS = 'Test2::Util::Facets2Legacy';

    # This private function is not exported, but we want to test it anyway
    *_get_facet_data = $CLASS->can('_get_facet_data');
}

tests _get_facet_data => sub {
    my $pass = Test2::Event::Pass->new(name => 'xxx');
    is_deeply(
        _get_facet_data($pass),
        {
            about  => {package => 'Test2::Event::Pass', details => 'pass', eid => $pass->eid},
            assert => {pass    => 1,                    details => 'xxx'},
        },
        "Got facet data from event"
    );

    is_deeply(
        _get_facet_data({assert => {pass => 1}}),
        {assert => {pass => 1}},
        "Facet data gets passed through"
    );

    my $file = __FILE__;
    my $line;
    like(
        exception { $line = __LINE__; _get_facet_data([]) },
        qr/'ARRAY\(.*\)' Does not appear to be either a Test::Event or an EventFacet hashref at \Q$file\E line $line/,
        "Must provide sane input data"
    );

    {
        package Fake::Event;
        use base 'Test2::Event';
        use Test2::Util::Facets2Legacy qw/causes_fail/;
    }

    my $e = Fake::Event->new();
    like(
        exception { $line = __LINE__; $e->causes_fail },
        qr/Cycle between Facets2Legacy and Fake::Event=HASH\(.*\)->facet_data\(\) \(Did you forget to override the facet_data\(\) method\?\)/,
        "Cannot depend on legacy facet_data and Facets2Legacy"
    );
};

tests causes_fail => sub {
    is(causes_fail({errors => [{fail => 1}]}), 1, "Fatal errors cause failure");

    is(causes_fail({control => {terminate => 0}}), 0, "defined but 0 termination does not cause failure");
    is(causes_fail({control => {terminate => 1}}), 1, "non-zero defined termination causes failure");
    is(causes_fail({control => {halt      => 1}}), 1, "A halt causes failure");
    is(causes_fail({assert  => {pass      => 0}}), 1, "non-passign assert causes failure");

    is(causes_fail({assert => {pass => 0}, amnesty => [{}]}), 0, "amnesty prevents assertion failure");

    is(causes_fail({}), 0, "Default is no failure");
};

tests diagnostics => sub {
    is(diagnostics({}), 0, "Default is no");

    is(diagnostics({errors => [{}]}), 1, "Errors mean diagnostics");
    is(diagnostics({info   => [{}]}), 0, "Info alone does not make diagnostics");

    is(diagnostics({info => [{debug => 1}]}), 1, "Debug flag makes info diagnostics");
};

tests global => sub {
    is(global({}), 0, "not global by default");
    is(global({control => {global => 0}}), 0, "global not set");
    is(global({control => {global => 1}}), 1, "global is set");
};

tests increments_count => sub {
    is(increments_count({}), 0, "No count bump without an assertion");
    is(increments_count({assert => {}}), 1, "count bump with assertion");
};

tests no_display => sub {
    is(no_display({}), 0, "default is no");
    is(no_display({about => {no_display => 0}}), 0, "set to off");
    is(no_display({about => {no_display => 1}}), 1, "set to on");
};

tests subtest_id => sub {
    is(subtest_id({}), undef, "none by default");
    is(subtest_id({parent => {hid => 123}}), 123, "use parent hid when present");
};

tests summary => sub {
    is(summary({}), '', "no summary without about->details");
    is(summary({about => {details => 'foo'}}), 'foo', "got about->details");
};

tests terminate => sub {
    is(terminate({}), undef, "undef by default");
    is(terminate({control => {terminate => undef}}), undef, "undef by choice");
    is(terminate({control => {terminate => 100}}), 100, "got the terminate value");
    is(terminate({control => {terminate => 0}}), 0, "0 is passed through");
};

tests sets_plan => sub {
    is_deeply( [sets_plan({})], [], "No plan by default");

    is_deeply(
        [sets_plan({plan => {}})],
        [0],
        "Empty plan means count of 0, nothing extra"
    );

    is_deeply(
        [sets_plan({plan => {count => 100}})],
        [100],
        "Got simple count"
    );

    is_deeply(
        [sets_plan({plan => {count => 0, none => 1}})],
        [0, 'NO PLAN'],
        "No Plan"
    );

    is_deeply(
        [sets_plan({plan => {count => 0, skip => 1}})],
        [0, 'SKIP'],
        "Skip"
    );

    is_deeply(
        [sets_plan({plan => {count => 0, skip => 1, details => 'foo bar'}})],
        [0, 'SKIP', 'foo bar'],
        "Skip with reason"
    );
};

done_testing;
