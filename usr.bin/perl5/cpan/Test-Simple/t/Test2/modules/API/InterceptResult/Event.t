use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::API::InterceptResult::Event;

my $CLASS = 'Test2::API::InterceptResult::Event';

tests facet_map => sub {
    ok(!$CLASS->can('plugins'), "Did not expose 'plugins' sub");

    my $fm = $CLASS->facet_map;

    is_deeply($fm->{__GENERIC__}, {class => 'Test2::API::InterceptResult::Facet', loaded => 1}, "Generic '__GENERIC__'");

    is_deeply($CLASS->facet_info('about'),   {class => 'Test2::EventFacet::About',   list => 0, loaded => 1}, "Found 'about' facet");
    is_deeply($CLASS->facet_info('amnesty'), {class => 'Test2::EventFacet::Amnesty', list => 1, loaded => 1}, "Found 'amnesty' facet");
    is_deeply($CLASS->facet_info('assert'),  {class => 'Test2::EventFacet::Assert',  list => 0, loaded => 1}, "Found 'assert' facet");
    is_deeply($CLASS->facet_info('control'), {class => 'Test2::EventFacet::Control', list => 0, loaded => 1}, "Found 'control' facet");
    is_deeply($CLASS->facet_info('errors'),  {class => 'Test2::EventFacet::Error',   list => 1, loaded => 1}, "Found 'errors' facet");
    is_deeply($CLASS->facet_info('hubs'),    {class => 'Test2::EventFacet::Hub',     list => 1, loaded => 1}, "Found 'hubs' facet");
    is_deeply($CLASS->facet_info('info'),    {class => 'Test2::EventFacet::Info',    list => 1, loaded => 1}, "Found 'info' facet");
    is_deeply($CLASS->facet_info('meta'),    {class => 'Test2::EventFacet::Meta',    list => 0, loaded => 1}, "Found 'meta' facet");
    is_deeply($CLASS->facet_info('parent'),  {class => 'Test2::EventFacet::Parent',  list => 0, loaded => 1}, "Found 'parent' facet");
    is_deeply($CLASS->facet_info('plan'),    {class => 'Test2::EventFacet::Plan',    list => 0, loaded => 1}, "Found 'plan' facet");
    is_deeply($CLASS->facet_info('render'),  {class => 'Test2::EventFacet::Render',  list => 1, loaded => 1}, "Found 'render' facet");
    is_deeply($CLASS->facet_info('trace'),   {class => 'Test2::EventFacet::Trace',   list => 0, loaded => 1}, "Found 'trace' facet");
};

tests init => sub {
    # This is just here to make sure the later test is meaningful. If this
    # starts to fail it probably means this test needs to be changed.
    ok(!$INC{'Test2/API/InterceptResult.pm'}, "Did not load result class yes");
    my $one = $CLASS->new();
    ok($one->isa($CLASS), "Got an instance");
    is_deeply($one->facet_data, {}, "Got empty data");
    is($one->result_class, 'Test2::API::InterceptResult', "Got default result class");
    ok($INC{'Test2/API/InterceptResult.pm'}, "Loaded result class");

    like(
        exception { $CLASS->new(facet_data => {assert => [{}]}) },
        qr/^Facet 'assert' is an only-one facet, but got 'ARRAY' instead of a hashref/,
        "Check list vs non-list when we can (check for single)"
    );

    like(
        exception { $CLASS->new(facet_data => {info => {}}) },
        qr/^Facet 'info' is a list facet, but got 'HASH' instead of an arrayref/,
        "Check list vs non-list when we can (check for list)"
    );

    like(
        exception { $CLASS->new(facet_data => {info => [{},[]]}) },
        qr/Got item type 'ARRAY' in list-facet 'info', all items must be hashrefs/,
        "Check each item in a list facet is a hashref"
    );

    my $two = $CLASS->new(facet_data => {assert => {}, info => [{}]});
    ok($two->isa($CLASS), "Got an instance with some actual facets");
};

tests facet => sub {
    my $one = $CLASS->new(facet_data => {
        other_single => {},
        other_list   => [{}],
        assert => {pass => 1, details => 'xxx'},
        info => [
            {tag => 'DIAG', details => 'xxx'},
            {tag => 'NOTE', details => 'xxx'},
        ],
    });

    ok(($one->facet('assert'))[0]->isa('Test2::EventFacet::Assert'),                "Bless the assert facet");
    ok(($one->facet('other_list'))[0]->isa('Test2::EventFacet'),                    "Bless the other_list as generic");
    ok(($one->facet('other_single'))[0]->isa('Test2::EventFacet'),                  "Bless the other_single as generic");
    ok(($one->facet('other_list'))[0]->isa('Test2::API::InterceptResult::Facet'),   "Bless the other_list as generic");
    ok(($one->facet('other_single'))[0]->isa('Test2::API::InterceptResult::Facet'), "Bless the other_single as generic");

    is(($one->facet('other_list'))[0]->foo, undef, "Generic gives us autoload for field access");

    is_deeply(
        [$one->facet('xxx')],
        [],
        "Got an empty list when facet is not present",
    );

    is_deeply(
        [$one->facet('assert')],
        [{pass => 1, details => 'xxx'}],
        "One item list for non-list facets",
    );

    is_deeply(
        [$one->facet('info')],
        [
            {tag => 'DIAG', details => 'xxx'},
            {tag => 'NOTE', details => 'xxx'},
        ],
        "Full list for list facets"
    );
};

tests the_facet => sub {
    my $one = $CLASS->new(facet_data => {
        other_single => {},
        other_list   => [{}],
        assert => {pass => 1, details => 'xxx'},
        info => [
            {tag => 'DIAG', details => 'xxx'},
            {tag => 'NOTE', details => 'xxx'},
        ],
    });

    ok($one->the_facet('assert')->isa('Test2::EventFacet::Assert'),                "Bless the assert facet");
    ok($one->the_facet('other_list')->isa('Test2::EventFacet'),                    "Bless the other_list as generic");
    ok($one->the_facet('other_single')->isa('Test2::EventFacet'),                  "Bless the other_single as generic");
    ok($one->the_facet('other_list')->isa('Test2::API::InterceptResult::Facet'),   "Bless the other_list as generic");
    ok($one->the_facet('other_single')->isa('Test2::API::InterceptResult::Facet'), "Bless the other_single as generic");

    is($one->the_facet('other_list')->foo, undef, "Generic gives us autoload for field access");

    is_deeply(
        $one->the_facet('xxx'),
        undef,
        "Got an undef when facet is not present",
    );

    is_deeply(
        $one->the_facet('assert'),
        {pass => 1, details => 'xxx'},
        "One item",
    );

    like(
        exception { $one->the_facet('info') },
        qr/'the_facet' called for facet 'info', but 'info' has '2' items/,
        "the_facet dies if there are more than one"
    );
};

tests causes_failure => sub {
    my $one = $CLASS->new(facet_data => { assert => {pass => 1, details => 'xxx'}});
    ok(!$one->causes_fail, "No failure for passing test");
    ok(!$one->causes_failure, "No failure for passing test (alt name)");

    my $two = $CLASS->new(facet_data => { assert => {pass => 0, details => 'xxx'}});
    ok($two->causes_fail, "Failure for failing test");
    ok($two->causes_failure, "Failure for failing test (alt name)");

    my $three = $CLASS->new(
        facet_data => {
            assert  => {pass => 0, details => 'xxx'},
            amnesty => [{tag => 'TODO', details => 'a todo'}],
        }
    );
    ok(!$three->causes_fail,    "No failure for failing test (with amnesty)");
    ok(!$three->causes_failure, "No failure for failing test (with amnesty) (alt name)");
};

tests trace => sub {
    my $one = $CLASS->new;
    is($one->trace,         undef, "No trace to get");
    is($one->frame,         undef, "No frame to get");
    is($one->trace_details, undef, "No trace to get trace_details from");
    is($one->trace_file,    undef, "No trace to get trace_file from");
    is($one->trace_line,    undef, "No trace to get trace_line from");
    is($one->trace_package, undef, "No trace to get trace_package from");
    is($one->trace_subname, undef, "No trace to get trace_subname from");
    is($one->trace_tool,    undef, "No trace to get trace_tool from");

    my $two = $CLASS->new(
        facet_data => {
            trace => {
                frame => [],
                details => 'xxx',
                pid => 1,
                tid => 1,
            },
        }
    );
    is_deeply($two->the_trace, {details => 'xxx', frame => [], pid => 1, tid => 1}, "Got trace");
    is_deeply([$two->trace], [{details => 'xxx', frame => [], pid => 1, tid => 1}], "Got trace");
    is($two->trace_details, 'xxx', "get trace_details");
    is_deeply($two->frame,         [], "No frame to get");
    is($two->trace_file,    undef, "No frame to get trace_file from");
    is($two->trace_line,    undef, "No frame to get trace_line from");
    is($two->trace_package, undef, "No frame to get trace_package from");
    is($two->trace_subname, undef, "No frame to get trace_subname from");
    is($two->trace_tool,    undef, "No frame to get trace_tool from");

    my $three = $CLASS->new(
        facet_data => {
            trace => {
                details => 'xxx',
                frame   => ['Foo::Bar', 'Foo/Bar.pm', 42, 'ok'],
                pid => 1,
                tid => 1,
            },
        }
    );
    is_deeply($three->the_trace, {details => 'xxx', frame => ['Foo::Bar', 'Foo/Bar.pm', 42, 'ok'], pid => 1, tid => 1}, "Got trace");
    is($three->trace_details, 'xxx', "get trace_details");
    is_deeply($three->frame, ['Foo::Bar', 'Foo/Bar.pm', 42, 'ok'], "Got frame");
    is($three->trace_file,    'Foo/Bar.pm', "Got trace_file");
    is($three->trace_line,    42,           "Got trace_line");
    is($three->trace_package, 'Foo::Bar',   "Got trace_package");
    is($three->trace_subname, 'ok',         "Got trace_subname");
    is($three->trace_tool,    'ok',         "Got trace_tool");
};

tests brief => sub {
    my $one = $CLASS->new(
        facet_data => {
            control => {halt => 1, details => "some reason to bail out"},
            errors  => [{tag => 'ERROR', details => "some kind of error"}],
            assert  => {pass => 1, details => "some passing assert"},
            plan    => {count => 42},
        }
    );

    is($one->brief, $one->bailout_brief, "bail-out is used when present");
    delete $one->{facet_data}->{control};

    is($one->brief, $one->error_brief, "error is next");
    delete $one->{facet_data}->{errors};

    is($one->brief, $one->assert_brief, "assert is next");
    delete $one->{facet_data}->{assert};

    is($one->brief, $one->plan_brief, "plan is last");
    delete $one->{facet_data}->{plan};

    is_deeply(
        [$one->brief],
        [],
        "Empty list if no briefs are available."
    );
};

tests summary => sub {
    my $one = $CLASS->new();

    is_deeply(
        $one->summary,
        {
            brief => '',

            causes_failure => 0,

            trace_line    => undef,
            trace_file    => undef,
            trace_tool    => undef,
            trace_details => undef,

            facets => [],
        },
        "Got summary for empty event"
    );

    my $two = $CLASS->new(facet_data => {
        assert => {pass => 0},
        trace => {frame => ['Foo::Bar', 'Foo/Bar.pm', 42, 'ok'], details => 'a trace'},
        parent => {},
        plan => {count => 1},
        control => {halt => 1, details => "bailout wins"},
        info => [
            {tag => 'DIAG', details => 'diag 1'},
            {tag => 'DIAG', details => 'diag 2'},
            {tag => 'NOTE', details => 'note 1'},
            {tag => 'NOTE', details => 'note 2'},
            {tag => 'OTHER', details => 'other 1'},
            {tag => 'OTHER', details => 'other 2'},
        ],
    });

    is_deeply(
        $two->summary,
        {
            brief => 'BAILED OUT: bailout wins',

            causes_failure => 1,

            trace_line    => 42,
            trace_file    => 'Foo/Bar.pm',
            trace_tool    => 'ok',
            trace_details => 'a trace',

            facets => [qw{ assert control info parent plan trace }],
        },
        "Got summary for lots"
    );

    is_deeply(
        $two->summary(fields => [qw/trace_line trace_file/]),
        {
            trace_line    => 42,
            trace_file    => 'Foo/Bar.pm',
        },
        "Got summary, specific fields"
    );

    is_deeply(
        $two->summary(remove => [qw/brief facets/]),
        {
            causes_failure => 1,

            trace_line    => 42,
            trace_file    => 'Foo/Bar.pm',
            trace_tool    => 'ok',
            trace_details => 'a trace',
        },
        "Got summary, removed some fields"
    );
};

tests assert => sub {
    my $one = $CLASS->new();
    ok(!$one->has_assert, "Not an assert");
    is_deeply([$one->assert],         [], "empty list for assert()");
    is_deeply([$one->assert_brief],   [], "empty list for assert_brief()");

    my $two = $CLASS->new(facet_data => {assert => {pass => 1, details => 'foo'}});
    ok($two->has_assert, "Is an assert");
    is_deeply([$two->assert], [{pass => 1, details => 'foo'}], "got assert item");
    is($two->assert_brief, "PASS", "got PASS for assert_brief()");

    my $three = $CLASS->new(facet_data => {
        assert => {pass => 0, details => 'foo'},
        amnesty => [
            {tag => 'TODO', details => 'todo 1'},
            {tag => 'SKIP', details => 'skip 1'},
            {tag => 'OOPS', details => 'oops 1'},
            {tag => 'TODO', details => 'todo 2'},
            {tag => 'SKIP', details => 'skip 2'},
            {tag => 'OOPS', details => 'oops 2'},
        ],
    });
    ok($three->has_assert, "Is an assert");
    is_deeply([$three->assert], [{pass => 0, details => 'foo'}], "got assert item");
    is($three->assert_brief, "FAIL with amnesty", "Fail with amnesty");

    my $four = $CLASS->new(facet_data => {
        assert => {pass => 0, details => 'foo'},
        amnesty => [
            {tag => 'TODO'},
            {tag => 'SKIP'},
            {tag => 'OOPS'},
        ],
    });
    ok($four->has_assert, "Is an assert");
    is_deeply([$four->assert], [{pass => 0, details => 'foo'}], "got assert item");
    is($four->assert_brief, "FAIL with amnesty", "Fail with amnesty");
};

tests subtest => sub {
    my $one = $CLASS->new();
    ok(!$one->has_subtest, "Not a subtest");
    is_deeply([$one->subtest],         [], "subtest() returns empty list");
    is_deeply([$one->subtest_result],  [], "subtest_result returns an empty list");

    my $two = $CLASS->new(
        facet_data => {
            parent => {
                hid      => '1234',
                children => [],
                state    => {
                    bailed_out   => undef,
                    count        => 5,
                    failed       => 1,
                    follows_plan => 1,
                    is_passing   => 0,
                    nested       => 1,
                    skip_reason  => undef,
                },
            },
        }
    );

    ok($two->has_subtest, "has a subtest");
    is_deeply([$two->subtest], [$two->facet_data->{parent}], "subtest() returns 1 item list");

    my $res = $two->subtest_result;
    ok($res->isa('Test2::API::InterceptResult'), "Got a result instance");
};

tests flatten => sub {
    my $one = $CLASS->new();
    is_deeply(
        $one->flatten,
        {
            causes_failure => 0,
            trace_file     => undef,
            trace_line     => undef
        },
        "Empty event flattens to almost nothing"
    );

    my $two = $CLASS->new(
        facet_data => {
            hubs    => [{details => "DO NOT SHOW"}],
            meta    => {details => "DO NOT SHOW"},
            control => {details => "A control"},
            assert  => {pass => 1, details => "Test Name"},

            trace => {
                frame   => ['Foo::Bar', 'Foo/Bar.pm', 42, 'Test2::Tools::Tiny::ok'],
                details => "Trace Details",
            },

            parent => {
                details => "A Subtest",
                children => [
                    $CLASS->new(facet_data => {assert => {pass  => 1, details => 'nested assertion'}}),
                    $CLASS->new(facet_data => {plan   => {count => 1}}),
                ],
            },

            errors => [
                {tag => 'error', fail => 0, details => "not a fatal error"},
                {tag => 'error', fail => 1, details => "a fatal error"},
            ],

            info => [
                {tag => 'DIAG', details => 'diag 1'},
                {tag => 'DIAG', details => 'diag 2'},
                {tag => 'NOTE', details => 'note 1'},
                {tag => 'NOTE', details => 'note 2'},
                {tag => 'INFO', details => 'info 1'},
                {tag => 'INFO', details => 'info 2'},
            ],
            amnesty => [
                {tag => 'TODO', details => 'todo 1'},
                {tag => 'TODO', details => 'todo 2'},
                {tag => 'SKIP', details => 'skip 1'},
                {tag => 'SKIP', details => 'skip 2'},
                {tag => 'OKOK', details => 'okok 1'},
                {tag => 'OKOK', details => 'okok 2'},
            ],

            other_single => {details => 'other single'},
            other_multi  => [{details => 'other multi'}],
        },
    );

    is_deeply(
        $two->flatten(include_subevents => 1),
        {
            # Summaries
            causes_failure => 0,
            trace_details  => 'Trace Details',
            trace_file     => 'Foo/Bar.pm',
            trace_line     => 42,

            # Info
            diag => ['diag 1', 'diag 2'],
            info => ['info 1', 'info 2'],
            note => ['note 1', 'note 2'],

            # Amnesty
            okok => ['okok 1', 'okok 2'],
            skip => ['skip 1', 'skip 2'],
            todo => ['todo 1', 'todo 2'],

            # Errors
            error => ['not a fatal error', 'FATAL: a fatal error'],

            # Assert
            name => 'Test Name',
            pass => 1,

            # Control
            control => 'A control',

            # Other
            other_multi  => ['other multi'],
            other_single => 'other single',

            # Subtest related
            subtest => {
                follows_plan => 1,
                is_passing   => 1,
                count        => 1,
                failed       => 0,
                plan         => 1,
            },

            subevents => [
                {
                    name           => 'nested assertion',
                    trace_line     => undef,
                    causes_failure => 0,
                    pass           => 1,
                    trace_file     => undef,
                },
                {
                    trace_file     => undef,
                    plan           => '1',
                    trace_line     => undef,
                    causes_failure => 0,
                }
            ],
        },
        "Very full flattening, with subevents"
    );

    is_deeply(
        $two->flatten(),
        {
            # Summaries
            causes_failure => 0,
            trace_details  => 'Trace Details',
            trace_file     => 'Foo/Bar.pm',
            trace_line     => 42,

            # Info
            diag => ['diag 1', 'diag 2'],
            info => ['info 1', 'info 2'],
            note => ['note 1', 'note 2'],

            # Amnesty
            okok => ['okok 1', 'okok 2'],
            skip => ['skip 1', 'skip 2'],
            todo => ['todo 1', 'todo 2'],

            # Errors
            error => ['not a fatal error', 'FATAL: a fatal error'],

            # Assert
            name => 'Test Name',
            pass => 1,

            # Control
            control => 'A control',

            # Other
            other_multi  => ['other multi'],
            other_single => 'other single',

            # Subtest related
            subtest => {
                follows_plan => 1,
                is_passing   => 1,
                count        => 1,
                failed       => 0,
                plan         => 1,
            },
        },
        "Very full flattening, no subevents"
    );

    my $three = $CLASS->new(
        facet_data => {
            trace => {
                frame => ['Foo::Bar', 'Foo/Bar.pm', 42, 'Test2::Tools::Tiny::ok'],
            },

            control => {halt => 1, details => "need to bail dude!"},

            amnesty => [{tag => 'TODO', details => 'todo 1'}],
        },
    );

    is_deeply(
        $three->flatten(include_subevents => 1),
        {
            # Summaries
            causes_failure => 0,

            trace_file => 'Foo/Bar.pm',
            trace_line => 42,

            bailed_out => "need to bail dude!",

            # Amnesty does not show without an assert or parent
        },
        "Bail-out test"
    );

    my $four = $CLASS->new(
        facet_data => {
            trace   => {frame => ['Foo::Bar', 'Foo/Bar.pm', 42, 'Test2::Tools::Tiny::ok']},
            errors  => [{tag => 'ERROR', details => 'an error', fail => 1}],
            amnesty => [{tag => 'TODO', details => 'todo 1'}],
        },
    );

    is_deeply(
        $four->flatten(),
        {
            # Summaries
            causes_failure => 0,

            trace_file => 'Foo/Bar.pm',
            trace_line => 42,

            todo  => ['todo 1'],
            error => ['FATAL: an error'],
        },
        "Include amnesty when there is a fatal error"
    );

    is_deeply(
        $four->flatten(fields => [qw/trace_file trace_line/]),
        {
            trace_file => 'Foo/Bar.pm',
            trace_line => 42,
        },
        "Filtered to only specific fields"
    );

    is_deeply(
        $four->flatten(remove => [qw/todo error/]),
        {
            # Summaries
            causes_failure => 0,

            trace_file => 'Foo/Bar.pm',
            trace_line => 42,
        },
        "Remove specific fields"
    );

};

tests bailout => sub {
    my $one = $CLASS->new();
    ok(!$one->has_bailout, "No bailout");
    is_deeply([$one->bailout],        [], "no bailout");
    is_deeply([$one->bailout_brief],  [], "no bailout");
    is_deeply([$one->bailout_reason], [], "no bailout");

    my $two = $CLASS->new(
        facet_data => {
            trace => {
                frame => ['Foo::Bar', 'Foo/Bar.pm', 42, 'Test2::Tools::Tiny::ok'],
            },

            control => {halt => 1, details => "need to bail dude!"},
        },
    );

    ok($two->has_bailout, "did bail out");
    is_deeply([$two->bailout],        [{halt => 1, details => "need to bail dude!"}], "Got the bailout");
    is_deeply([$two->bailout_brief],  ["BAILED OUT: need to bail dude!"],             "Got the bailout brief");
    is_deeply([$two->bailout_reason], ["need to bail dude!"],                         "Got the bailout reason");
};

tests plan => sub {
    my $one = $CLASS->new;
    ok(!$one->has_plan, "No plan");
    is_deeply([$one->plan], [], "No plan");
    is_deeply([$one->plan_brief], [], "No plan");

    my $two = $CLASS->new(facet_data => {plan => { count => 42 }});
    ok($two->has_plan, "Got a plan");
    is_deeply([$two->plan], [{ count => 42 }], "Got the plan facet");
    is_deeply([$two->plan_brief], ["PLAN 42"], "Got the brief");

    $two->{facet_data}->{plan}->{details} = "foo bar baz";
    is_deeply([$two->plan_brief], ["PLAN 42: foo bar baz"], "Got the brief with details");

    $two->{facet_data}->{plan}->{count} = 0;
    is_deeply([$two->plan_brief], ["SKIP ALL: foo bar baz"], "Got the skip form no count with details");

    $two->{facet_data}->{plan}->{count} = 1;
    $two->{facet_data}->{plan}->{skip} = 1;
    is_deeply([$two->plan_brief], ["SKIP ALL: foo bar baz"], "Got the skip with details");

    $two->{facet_data}->{plan}->{skip} = 0;
    $two->{facet_data}->{plan}->{none} = 1;
    is_deeply([$two->plan_brief], ["NO PLAN: foo bar baz"], "Got the 'NO PLAN' with details");
};

tests amnesty => sub {
    my $one = $CLASS->new();

    ok(!$one->has_amnesty,       "No amnesty");
    ok(!$one->has_todos,         "No todos");
    ok(!$one->has_skips,         "No skips");
    ok(!$one->has_other_amnesty, "No other amnesty");

    is_deeply([$one->amnesty],       [], "amnesty list is empty");
    is_deeply([$one->todos],         [], "todos list is empty");
    is_deeply([$one->skips],         [], "skips list is empty");
    is_deeply([$one->other_amnesty], [], "other_amnesty list is empty");

    is_deeply([$one->amnesty_reasons],       [], "amnesty_reasons list is empty");
    is_deeply([$one->todo_reasons],          [], "todo_reasons list is empty");
    is_deeply([$one->skip_reasons],          [], "skip_reasons list is empty");
    is_deeply([$one->other_amnesty_reasons], [], "other_amnesty_reasons list is empty");

    my $two = $CLASS->new(
        facet_data => {
            amnesty => [
                {tag => 'TODO', details => 'todo 1'},
                {tag => 'TODO', details => 'todo 2'},
                {tag => 'SKIP', details => 'skip 1'},
                {tag => 'SKIP', details => 'skip 2'},
                {tag => 'OKOK', details => 'okok 1'},
                {tag => 'OKOK', details => 'okok 2'},
            ],
        },
    );

    ok($two->has_amnesty,       "amnesty");
    ok($two->has_todos,         "todos");
    ok($two->has_skips,         "skips");
    ok($two->has_other_amnesty, "other amnesty");

    is_deeply(
        [$two->amnesty],
        [
            {tag => 'TODO', details => 'todo 1'},
            {tag => 'TODO', details => 'todo 2'},
            {tag => 'SKIP', details => 'skip 1'},
            {tag => 'SKIP', details => 'skip 2'},
            {tag => 'OKOK', details => 'okok 1'},
            {tag => 'OKOK', details => 'okok 2'},
        ],
        "amnesty list",
    );
    is_deeply(
        [$two->todos],
        [
            {tag => 'TODO', details => 'todo 1'},
            {tag => 'TODO', details => 'todo 2'},
        ],
        "todos list",
    );
    is_deeply(
        [$two->skips],
        [
            {tag => 'SKIP', details => 'skip 1'},
            {tag => 'SKIP', details => 'skip 2'},
        ],
        "skips list",
    );
    is_deeply(
        [$two->other_amnesty],
        [
            {tag => 'OKOK', details => 'okok 1'},
            {tag => 'OKOK', details => 'okok 2'},
        ],
        "other_amnesty list",
    );

    is_deeply(
        [$two->amnesty_reasons],
        [
            'todo 1',
            'todo 2',
            'skip 1',
            'skip 2',
            'okok 1',
            'okok 2',
        ],
        "amnesty_reasons list is empty"
    );
    is_deeply(
        [$two->todo_reasons],
        [
            'todo 1',
            'todo 2',
        ],
        "todo_reasons list is empty"
    );
    is_deeply(
        [$two->skip_reasons],
        [
            'skip 1',
            'skip 2',
        ],
        "skip_reasons list is empty"
    );
    is_deeply(
        [$two->other_amnesty_reasons],
        [
            'okok 1',
            'okok 2',
        ],
        "other_amnesty_reasons list is empty"
    );
};

tests errors => sub {
    my $one = $CLASS->new();
    ok(!$one->has_errors, "No errors");
    is_deeply([$one->errors], [], "No errors");
    is_deeply([$one->error_messages], [], "No errors");
    is_deeply([$one->error_brief], [], "No errors");

    my $two = $CLASS->new(facet_data => {
        errors => [{tag => 'error', details => 'a non fatal error'}],
    });
    ok($two->has_errors, "Got errors");
    is_deeply([$two->errors], [{tag => 'error', details => 'a non fatal error'}], "Got the error");
    is_deeply([$two->error_messages], ['a non fatal error'], "Got the message");
    is_deeply([$two->error_brief], ['ERROR: a non fatal error'], "Got the brief");

    my $three = $CLASS->new(facet_data => {
        errors => [{tag => 'error', details => "a non fatal\nerror"}],
    });
    ok($three->has_errors, "Got errors");
    is_deeply([$three->errors], [{tag => 'error', details => "a non fatal\nerror"}], "Got the error");
    is_deeply([$three->error_messages], ["a non fatal\nerror"], "Got the message");
    is_deeply([$three->error_brief], ["ERROR: a non fatal [...]"], "Got the brief");

    my $four = $CLASS->new(facet_data => {
        errors => [
            {tag => 'error', details => "a fatal error", fail => 1},
            {tag => 'error', details => "a non fatal error", fail => 0},
        ],
    });

    ok($four->has_errors, "Got errors");

    is_deeply(
        [$four->errors],
        [
            {tag => 'error', details => "a fatal error", fail => 1},
            {tag => 'error', details => "a non fatal error", fail => 0},
        ],
        "Got the error"
    );

    is_deeply(
        [$four->error_messages],
        [
            "a fatal error",
            "a non fatal error",
        ],
        "Got the message"
    );

    is_deeply([$four->error_brief], ['ERRORS: a fatal error [...]'], "Got the brief");

};

tests info => sub {
    my $one = $CLASS->new();

    ok(!$one->has_info,       "No info");
    ok(!$one->has_diags,         "No diags");
    ok(!$one->has_notes,         "No notes");
    ok(!$one->has_other_info, "No other info");

    is_deeply([$one->info],       [], "info list is empty");
    is_deeply([$one->diags],         [], "diags list is empty");
    is_deeply([$one->notes],         [], "notes list is empty");
    is_deeply([$one->other_info], [], "other_info list is empty");

    is_deeply([$one->info_messages],       [], "info_messages list is empty");
    is_deeply([$one->diag_messages],          [], "diag_messages list is empty");
    is_deeply([$one->note_messages],          [], "note_messages list is empty");
    is_deeply([$one->other_info_messages], [], "other_info_messages list is empty");

    my $two = $CLASS->new(
        facet_data => {
            info => [
                {tag => 'DIAG', details => 'diag 1'},
                {tag => 'DIAG', details => 'diag 2'},
                {tag => 'NOTE', details => 'note 1'},
                {tag => 'NOTE', details => 'note 2'},
                {tag => 'INFO', details => 'info 1'},
                {tag => 'INFO', details => 'info 2'},
            ],
        },
    );

    ok($two->has_info,       "info");
    ok($two->has_diags,         "diags");
    ok($two->has_notes,         "notes");
    ok($two->has_other_info, "other info");

    is_deeply(
        [$two->info],
        [
            {tag => 'DIAG', details => 'diag 1'},
            {tag => 'DIAG', details => 'diag 2'},
            {tag => 'NOTE', details => 'note 1'},
            {tag => 'NOTE', details => 'note 2'},
            {tag => 'INFO', details => 'info 1'},
            {tag => 'INFO', details => 'info 2'},
        ],
        "info list",
    );
    is_deeply(
        [$two->diags],
        [
            {tag => 'DIAG', details => 'diag 1'},
            {tag => 'DIAG', details => 'diag 2'},
        ],
        "diags list",
    );
    is_deeply(
        [$two->notes],
        [
            {tag => 'NOTE', details => 'note 1'},
            {tag => 'NOTE', details => 'note 2'},
        ],
        "notes list",
    );
    is_deeply(
        [$two->other_info],
        [
            {tag => 'INFO', details => 'info 1'},
            {tag => 'INFO', details => 'info 2'},
        ],
        "other_info list",
    );

    is_deeply(
        [$two->info_messages],
        [
            'diag 1',
            'diag 2',
            'note 1',
            'note 2',
            'info 1',
            'info 2',
        ],
        "info_messages list is empty"
    );
    is_deeply(
        [$two->diag_messages],
        [
            'diag 1',
            'diag 2',
        ],
        "diag_messages list is empty"
    );
    is_deeply(
        [$two->note_messages],
        [
            'note 1',
            'note 2',
        ],
        "note_messages list is empty"
    );
    is_deeply(
        [$two->other_info_messages],
        [
            'info 1',
            'info 2',
        ],
        "other_info_messages list is empty"
    );
};

done_testing;
