use strict;
use warnings;
use Test2::Tools::Tiny;

use Test2::Event();
use Test2::EventFacet::Trace();
use Test2::Event::Generic;

use Test2::API qw/context/;
use Scalar::Util qw/reftype/;

tests old_api => sub {
    {
        package My::MockEvent;

        use base 'Test2::Event';
        use Test2::Util::HashBase qw/foo bar baz/;
    }

    ok(My::MockEvent->can($_), "Added $_ accessor") for qw/foo bar baz/;

    my $one = My::MockEvent->new(trace => 'fake');

    ok(!$one->causes_fail, "Events do not cause failures by default");

    ok(!$one->$_, "$_ is false by default") for qw/increments_count terminate global/;

    ok(!$one->get_meta('xxx'), "no meta-data associated for key 'xxx'");

    $one->set_meta('xxx', '123');

    is($one->meta('xxx'), '123', "got meta-data");

    is($one->meta('xxx', '321'), '123', "did not use default");

    is($one->meta('yyy', '1221'), '1221', "got the default");

    is($one->meta('yyy'), '1221', "last call set the value to the default for future use");

    is($one->summary, 'My::MockEvent', "Default summary is event package");

    is($one->diagnostics, 0, "Not diagnostics by default");
};

tests deprecated => sub {
    my $e = Test2::Event->new(trace => Test2::EventFacet::Trace->new(frame => ['foo', 'foo.pl', 42], nested => 2, hid => 'maybe'));

    my $warnings = warnings {
        local $ENV{AUTHOR_TESTING} = 1;
        is($e->nested, 2, "Got nested from the trace");
        is($e->in_subtest, 'maybe', "got hid from trace");

        $e->trace->{nested} = 0;

        local $ENV{AUTHOR_TESTING} = 0;
        is($e->nested, 0, "Not nested");
        is($e->in_subtest, undef, "Did not get hid");
    };

    is(@$warnings, 2, "got warnings once each");
    like($warnings->[0], qr/Use of Test2::Event->nested\(\) is deprecated/, "Warned about deprecation");
    like($warnings->[1], qr/Use of Test2::Event->in_subtest\(\) is deprecated/, "Warned about deprecation");
};

tests facet_data => sub {
    my $e = Test2::Event::Generic->new(
        causes_fail      => 0,
        increments_count => 0,
        diagnostics      => 0,
        no_display       => 0,
        callback         => undef,
        terminate        => undef,
        global           => undef,
        sets_plan        => undef,
        summary          => undef,
        facet_data       => undef,
    );

    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef,
            },
            control => {
                has_callback => 0,
                terminate    => undef,
                global       => 0
            },
        },
        "Facet data has control with only false values, and an about"
    );

    $e->set_trace(Test2::EventFacet::Trace->new(frame => ['foo', 'foo.t', 42]));
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef,
            },
            control => {
                has_callback => 0,
                terminate    => undef,
                global       => 0
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid => $$,
                tid => 0,
            },
        },
        "Got a trace now"
    );

    $e->set_causes_fail(1);
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 0,
                terminate    => undef,
                global       => 0
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            errors => [
                {
                    tag     => 'FAIL',
                    details => 'Test2::Event::Generic',
                    fail    => 1,
                }
            ],
        },
        "Got an error"
    );

    $e->set_increments_count(1);
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 0,
                terminate    => undef,
                global       => 0
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass => 0,
                details => 'Test2::Event::Generic',
            },
        },
        "Got an assert now"
    );

    $e->set_causes_fail(0);
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 0,
                terminate    => undef,
                global       => 0
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass => 1,
                details => 'Test2::Event::Generic',
            },
        },
        "Got a passing assert now"
    );

    $e->set_global(1);
    $e->set_terminate(255);
    $e->set_callback(sub {1});
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 1,
                terminate    => 255,
                global       => 1,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass => 1,
                details => 'Test2::Event::Generic',
            },
        },
        "control fields were altered"
    );

    my $data;
    {
        no warnings 'once';
        local *Test2::Event::Generic::subtest_id = sub { 123 };
        $data = $e->facet_data;
    }
    is_deeply(
        $data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 1,
                terminate    => 255,
                global       => 1,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass     => 1,
                details  => 'Test2::Event::Generic',
            },
            parent => {hid => 123},
        },
        "Added parent"
    );

    $e->set_meta('foo', {a => 1});
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 1,
                terminate    => 255,
                global       => 1,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass     => 1,
                details  => 'Test2::Event::Generic',
            },
            meta => {foo => {a => 1}},
        },
        "Grabbed meta"
    );


    $e->set_sets_plan([5]);
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 1,
                terminate    => 255,
                global       => 1,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass     => 1,
                details  => 'Test2::Event::Generic',
            },
            meta => {foo => {a => 1}},
            plan => { count => 5 },
        },
        "Plan facet added"
    );

    $e->set_terminate(undef);
    $e->set_sets_plan([0, SKIP => 'because']);
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 1,
                terminate    => 0,
                global       => 1,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass     => 1,
                details  => 'Test2::Event::Generic',
            },
            meta => {foo => {a => 1}},
            plan => { count => 0, skip => 1, details => 'because' },
        },
        "Plan set terminate, skip, and details"
    );

    $e->set_sets_plan([0, 'NO PLAN' => 'because']);
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 1,
                terminate    => undef,
                global       => 1,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass     => 1,
                details  => 'Test2::Event::Generic',
            },
            meta => {foo => {a => 1}},
            plan => { count => 0, none => 1, details => 'because' },
        },
        "Plan does not set terminate, but sets 'none' and 'details'"
    );

    $e->add_amnesty({tag => 'foo', details => 'bar'});
    $e->add_amnesty({tag => 'baz', details => 'bat'});
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef
            },
            control => {
                has_callback => 1,
                terminate    => undef,
                global       => 1,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            assert => {
                no_debug => 1,
                pass     => 1,
                details  => 'Test2::Event::Generic',
            },
            meta => {foo => {a => 1}},
            plan => { count => 0, none => 1, details => 'because' },
            amnesty => [
                { tag => 'foo', details => 'bar' },
                { tag => 'baz', details => 'bat' },
            ],
        },
        "Amnesty added"
    );

    $e = Test2::Event::Generic->new();
    $e->set_diagnostics(1);
    $e->set_no_display(1);
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => 1,
            },
            control => {
                has_callback => 0,
                terminate    => undef,
                global       => 0,
            },
        },
        "No Info"
    );

    $e->set_no_display(0);
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'Test2::Event::Generic',
                eid        => $e->eid,
                no_display => undef,
            },
            control => {
                has_callback => 0,
                terminate    => undef,
                global       => 0,
            },
            info => [{
                details => 'Test2::Event::Generic',
                tag => 'DIAG',
                debug => 1,
            }],
        },
        "Got debug Info"
    );

    $e->set_summary("foo bar baz");
    is_deeply(
        $e->facet_data,
        {
            about => {
                package    => 'Test2::Event::Generic',
                details    => 'foo bar baz',
                eid        => $e->eid,
                no_display => undef,
            },
            control => {
                has_callback => 0,
                terminate    => undef,
                global       => 0,
            },
            info => [{
                details => 'foo bar baz',
                tag => 'DIAG',
                debug => 1,
            }],
        },
        "Got debug Info with summary change"
    );
};

tests facets => sub {
    my $data = {
        about => {
            package    => 'Test2::Event::Generic',
            details    => 'Test2::Event::Generic',
            no_display => undef
        },
        control => {
            has_callback => 1,
            terminate    => undef,
            global       => 1,
        },
        trace => {
            frame => ['foo', 'foo.t', 42],
            pid   => $$,
            tid   => 0,
        },
        assert => {
            no_debug => 1,
            pass     => 1,
            details  => 'Test2::Event::Generic',
        },
        meta => {foo => {a => 1}},
        plan    => {count => 0,   none     => 1, details => 'because'},
        parent  => {hid   => 123, children => []},
        amnesty => [
            {tag => 'foo', details => 'bar'},
            {tag => 'baz', details => 'bat'},
        ],
        info => [
            {
                details => 'foo bar baz',
                tag     => 'DIAG',
                debug   => 1,
            }
        ],
        errors => [{
            tag     => 'FAIL',
            details => 'Test2::Event::Generic',
            fail    => 1,
        }],
    };

    my $e = Test2::Event::Generic->new(facet_data => $data);
    is_deeply(
        $e->facet_data,
        $e->facets,
        "Facets and facet_data have the same structure"
    );

    my $facets = $e->facets;

    for my $key (sort keys %$facets) {
        my $type = "Test2::EventFacet::" . ucfirst($key);
        $type =~ s/s$//;
        my $val  = $facets->{$key};
        if ($type->is_list) {
            for my $f (@$val) {
                ok($f->isa('Test2::EventFacet'), "'$key' has a blessed facet");
                ok($f->isa("$type"), "'$key' is a '$type'") or diag("$f");
            }
        }
        else {
            ok($val->isa('Test2::EventFacet'), "'$key' has a blessed facet");
            ok($val->isa($type), "'$key' is a '$type'");
        }
    }
};

tests common_facet_data => sub {
    my $e = Test2::Event::Generic->new(
        causes_fail      => 0,
        increments_count => 0,
        diagnostics      => 0,
        no_display       => 0,
        callback         => undef,
        terminate        => undef,
        global           => undef,
        sets_plan        => undef,
        summary          => undef,
        facet_data       => undef,
    );

    is_deeply(
        $e->common_facet_data,
        {
            about => {
                package => 'Test2::Event::Generic',
                eid     => $e->eid,
            },
        },
        "Facet data has an about"
    );

    $e->set_trace(Test2::EventFacet::Trace->new(frame => ['foo', 'foo.t', 42]));
    is_deeply(
        $e->common_facet_data,
        {
            about => {
                package => 'Test2::Event::Generic',
                eid     => $e->eid,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
        },
        "Got a trace now"
    );

    $e->set_meta('foo', {a => 1});
    is_deeply(
        $e->common_facet_data,
        {
            about => {
                package => 'Test2::Event::Generic',
                eid     => $e->eid,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            meta => {foo => {a => 1}},
        },
        "Grabbed meta"
    );

    $e->add_amnesty({tag => 'foo', details => 'bar'});
    $e->add_amnesty({tag => 'baz', details => 'bat'});
    is_deeply(
        $e->common_facet_data,
        {
            about => {
                package => 'Test2::Event::Generic',
                eid     => $e->eid,
            },
            trace => {
                frame => ['foo', 'foo.t', 42],
                pid   => $$,
                tid   => 0,
            },
            meta    => {foo => {a => 1}},
            amnesty => [
                {tag => 'foo', details => 'bar'},
                {tag => 'baz', details => 'bat'},
            ],
        },
        "Amnesty added"
    );
};

tests related => sub {
    my $ctx = context();
    my $ev_a = $ctx->build_ev2(about => {});
    my $ev_b = $ctx->build_ev2(about => {});
    $ctx->release;

    $ctx = context();
    my $ev_c = $ctx->build_ev2(about => {});
    $ctx->release;

    delete $ev_a->{trace}->{uuid};
    delete $ev_b->{trace}->{uuid};
    delete $ev_c->{trace}->{uuid};

    ok($ev_a->related($ev_b), "Related as they were created with the same context (no uuid)");
    ok(!$ev_a->related($ev_c), "Not related as they were created with a different context (no uuid)");

    $ev_a->{trace}->{uuid} = 'xxx'; # Yes I know it is not valid.
    $ev_b->{trace}->{uuid} = 'yyy'; # Yes I know it is not valid.
    $ev_c->{trace}->{uuid} = 'xxx'; # Yes I know it is not valid.

    ok(!$ev_a->related($ev_b), "Not related, traces have different UUID's");
    ok($ev_a->related($ev_c), "Related, traces have the same UUID's");
};

tests verify_facet_data => sub {
    my $ev1 = Test2::Event::V2->new(
        assert => { pass => 1 },
        info => [{tag => 'NOTE', details => 'oops' }],
        'a custom one' => {},
    );

    is_deeply(
        [$ev1->validate_facet_data],
        [],
        "No errors"
    );

    my $ev2 = Test2::Event::V2->new(
        assert => [{ pass => 1 }],
        info => {tag => 'NOTE', details => 'oops' },
        'a custom one' => {},
    );

    my @errors = $ev2->validate_facet_data;
    is(@errors, 2, "Got 2 errors");
    like($errors[0], qr/^Facet 'assert' should not be a list, but got a a list/, "Got a list for a non-list type");
    like($errors[1], qr/^Facet 'info' should be a list, but got a single item/, "Got a single item when a list is needed");

    @errors = $ev2->validate_facet_data(require_facet_class => 1);
    is(@errors, 3, "Got 3 errors");
    is($errors[0], "Could not find a facet class for facet 'a custom one'", "Classes required");
    like($errors[1], qr/^Facet 'assert' should not be a list, but got a a list/, "Got a list for a non-list type");
    like($errors[2], qr/^Facet 'info' should be a list, but got a single item/, "Got a single item when a list is needed");

    is_deeply(
        [Test2::Event->validate_facet_data($ev1->facet_data)],
        [],
        "No errors"
    );

    @errors = Test2::Event->validate_facet_data($ev2->facet_data);
    is(@errors, 2, "Got 2 errors");
    like($errors[0], qr/^Facet 'assert' should not be a list, but got a a list/, "Got a list for a non-list type");
    like($errors[1], qr/^Facet 'info' should be a list, but got a single item/, "Got a single item when a list is needed");

    @errors = Test2::Event->validate_facet_data($ev2->facet_data, require_facet_class => 1);
    is(@errors, 3, "Got 3 errors");
    is($errors[0], "Could not find a facet class for facet 'a custom one'", "Classes required");
    like($errors[1], qr/^Facet 'assert' should not be a list, but got a a list/, "Got a list for a non-list type");
    like($errors[2], qr/^Facet 'info' should be a list, but got a single item/, "Got a single item when a list is needed");
};

done_testing;
