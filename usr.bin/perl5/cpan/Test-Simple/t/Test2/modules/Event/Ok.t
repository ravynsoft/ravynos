use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::EventFacet::Trace;
use Test2::Event::Ok;
use Test2::Event::Diag;

use Test2::API qw/context/;

my $trace;
sub before_each {
    # Make sure there is a fresh trace object for each group
    $trace = Test2::EventFacet::Trace->new(
        frame => ['main_foo', 'foo.t', 42, 'main_foo::flubnarb'],
    );
}

tests Passing => sub {
    my $ok = Test2::Event::Ok->new(
        trace => $trace,
        pass  => 1,
        name  => 'the_test',
    );
    ok($ok->increments_count, "Bumps the count");
    ok(!$ok->causes_fail, "Passing 'OK' event does not cause failure");
    is($ok->pass, 1, "got pass");
    is($ok->name, 'the_test', "got name");
    is($ok->effective_pass, 1, "effective pass");
    is($ok->summary, "the_test", "Summary is just the name of the test");

    my $facet_data = $ok->facet_data;
    ok($facet_data->{about}, "got common facet data");
    ok(!$facet_data->{amnesty}, "No amnesty by default");
    is_deeply(
        $facet_data->{assert},
        {
            no_debug => 1,
            pass => 1,
            details => 'the_test',
        },
        "Got assert facet",
    );


    $ok = Test2::Event::Ok->new(
        trace => $trace,
        pass  => 1,
        name  => '',
    );
    is($ok->summary, "Nameless Assertion", "Nameless test");

    $facet_data = $ok->facet_data;
    ok($facet_data->{about}, "got common facet data");
    ok(!$facet_data->{amnesty}, "No amnesty by default");
    is_deeply(
        $facet_data->{assert},
        {
            no_debug => 1,
            pass => 1,
            details => '',
        },
        "Got assert facet",
    );
};

tests Failing => sub {
    local $ENV{HARNESS_ACTIVE} = 1;
    local $ENV{HARNESS_IS_VERBOSE} = 1;
    my $ok = Test2::Event::Ok->new(
        trace => $trace,
        pass  => 0,
        name  => 'the_test',
    );
    ok($ok->increments_count, "Bumps the count");
    ok($ok->causes_fail, "A failing test causes failures");
    is($ok->pass, 0, "got pass");
    is($ok->name, 'the_test', "got name");
    is($ok->effective_pass, 0, "effective pass");
    is($ok->summary, "the_test", "Summary is just the name of the test");

    my $facet_data = $ok->facet_data;
    ok($facet_data->{about}, "got common facet data");
    ok(!$facet_data->{amnesty}, "No amnesty by default");
    is_deeply(
        $facet_data->{assert},
        {
            no_debug => 1,
            pass => 0,
            details => 'the_test',
        },
        "Got assert facet",
    );
};

tests "Failing TODO" => sub {
    local $ENV{HARNESS_ACTIVE} = 1;
    local $ENV{HARNESS_IS_VERBOSE} = 1;
    my $ok = Test2::Event::Ok->new(
        trace => $trace,
        pass  => 0,
        name  => 'the_test',
        todo  => 'A Todo',
    );
    ok($ok->increments_count, "Bumps the count");
    is($ok->pass, 0, "got pass");
    is($ok->name, 'the_test', "got name");
    is($ok->effective_pass, 1, "effective pass is true from todo");
    is($ok->summary, "the_test (TODO: A Todo)", "Summary is just the name of the test + todo");

    my $facet_data = $ok->facet_data;
    ok($facet_data->{about}, "got common facet data");
    is_deeply(
        $facet_data->{assert},
        {
            no_debug => 1,
            pass => 0,
            details => 'the_test',
        },
        "Got assert facet",
    );
    is_deeply(
        $facet_data->{amnesty},
        [{
            tag => 'TODO',
            details => 'A Todo',
        }],
        "Got amnesty facet",
    );


    $ok = Test2::Event::Ok->new(
        trace => $trace,
        pass  => 0,
        name  => 'the_test2',
        todo  => '',
    );
    ok($ok->effective_pass, "empty string todo is still a todo");
    is($ok->summary, "the_test2 (TODO)", "Summary is just the name of the test + todo");

    $facet_data = $ok->facet_data;
    ok($facet_data->{about}, "got common facet data");
    is_deeply(
        $facet_data->{assert},
        {
            no_debug => 1,
            pass => 0,
            details => 'the_test2',
        },
        "Got assert facet",
    );
    is_deeply(
        $facet_data->{amnesty},
        [{
            tag => 'TODO',
            details => '',
        }],
        "Got amnesty facet",
    );

};

tests init => sub {
    my $ok = Test2::Event::Ok->new(
        trace => $trace,
        pass  => 1,
    );
    is($ok->effective_pass, 1, "set effective pass");
};

done_testing;
