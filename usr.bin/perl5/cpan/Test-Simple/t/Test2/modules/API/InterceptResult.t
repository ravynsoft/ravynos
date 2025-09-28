use strict;
use warnings;

use Test::Builder;
use Test2::Tools::Tiny;
use Test2::API::InterceptResult;
use Scalar::Util qw/reftype/;
use Test2::API qw/intercept context/;

my $CLASS = 'Test2::API::InterceptResult';

tests construction => sub {
    my $one = $CLASS->new('a');
    ok($one->isa($CLASS), "Got an instance");
    is(reftype($one), 'ARRAY', "Blessed arrayref");
    is_deeply($one, ['a'], "Ref looks good.");

    my $two = $CLASS->new_from_ref(['a']);
    ok($two->isa($CLASS), "Got an instance");
    is(reftype($two), 'ARRAY', "Blessed arrayref");
    is_deeply($two, ['a'], "Ref looks good.");

    my $three = $two->clone;
    ok($three->isa($CLASS), "Got an instance");
    is(reftype($three), 'ARRAY', "Blessed arrayref");
    is_deeply($three, ['a'], "Ref looks good.");

    push @$two => 'b';
    is_deeply($two, ['a', 'b'], "Modified two");
    is_deeply($three, ['a'], "three was not changed");

    my $four = intercept {
        ok(1, "Pass");
    };

    ok($four->isa($CLASS), "Intercept returns an instance");
};

tests event_list => sub {
    my $one = $CLASS->new('a', 'b');
    is_deeply([$one->event_list], ['a', 'b'], "event_list is essentially \@{\$self}");
};

tests _upgrade => sub {
    require Test2::Event::Pass;
    my $event = Test2::Event::Pass->new(name => 'soup for you', trace => {frame => ['foo', 'foo.pl', 42]});
    ok($event->isa('Test2::Event'), "Start with an event");

    my $one = $CLASS->new;
    my $up = $one->_upgrade($event);
    ok($up->isa('Test2::API::InterceptResult::Event'), "Upgraded the event");
    is($up->result_class, $CLASS, "set the result class");

    is_deeply($event->facet_data, $up->facet_data, "Facet data is identical");

    $up->facet_data->{trace}->{frame}->[2] = 43;
    is($up->trace_line, 43, "Modified the facet data in the upgraded clone");
    is($event->facet_data->{trace}->{frame}->[2], 42, "Did nto modify the original");

    my $up2 = $one->_upgrade($up);
    is("$up2", "$up", "Returned the ref unmodified because it is already an upgraded item");

    require Test2::Event::V2;
    my $subtest = 'Test2::Event::V2'->new(
        trace => {frame => ['foo', 'foo.pl', 42]},
        assert => {pass => 1, details => 'pass'},
        parent => {
            hid => 1,
            children => [ $event ],
        },
    );

    my $subup = $one->_upgrade($subtest);
    ok($subup->the_subtest->{children}->isa($CLASS), "Blessed subtest subevents");
    ok(
        $subup->the_subtest->{children}->[0]->isa('Test2::API::InterceptResult::Event'),
        "Upgraded the children"
    );
};

tests hub => sub {
    my $one = intercept {
        ok(1, "pass");
        ok(0, "fail");
        plan 2;
    };

    my $hub = $one->hub;
    ok($hub->isa('Test2::Hub'), "Hub is a proper instance");
    ok($hub->check_plan, "Had a plan and followed it");
    is($hub->count, 2, "saw both events");
    is($hub->failed, 1, "saw a failure");
    ok($hub->ended, "Hub ended");

    is_deeply(
        $one->state,
        {
            count => 2,
            failed => 1,
            is_passing => 0,
            plan => 2,
            bailed_out => undef,
            skip_reason => undef,
            follows_plan => 1,
        },
        "Got the hub state"
    );
};

tests upgrade => sub {
    my $one = intercept {
        require Test::More;
        Test::More::ok(1, "pass");
        Test::More::ok(1, "pass");
    };

    ok($one->[0]->isa('Test2::Event::Ok'), "Original event is not upgraded 0");
    ok($one->[1]->isa('Test2::Event::Ok'), "Original event is not upgraded 1");

    my $two = $one->upgrade;
    ok($one->[0]->isa('Test2::Event::Ok'), "Did not modify original 0");
    ok($one->[0]->isa('Test2::Event::Ok'), "Did not modify original 1");
    ok($two->[0]->isa('Test2::API::InterceptResult::Event'), "Upgraded copy 0");
    ok($two->[1]->isa('Test2::API::InterceptResult::Event'), "Upgraded copy 1");

    my $three = $two->upgrade;
    ok("$two->[0]" ne "$three->[0]", "Upgrade on an already upgraded instance returns copies of the events, not originals");

    like(
        exception { $one->upgrade() },
        qr/Called a method that creates a new instance in void context/,
        "Calling upgrade() without keeping the result is a bug"
    );

    $one->upgrade(in_place => 1);
    ok($one->[0]->isa('Test2::API::InterceptResult::Event'), "Upgraded in place 0");
    ok($one->[1]->isa('Test2::API::InterceptResult::Event'), "Upgraded in place 1");
};

tests squash_info => sub {
    my $one = intercept {
        diag "isolated 1";
        note "isolated 2";
        sub {
            my $ctx = context();
            diag "inline 1";
            note "inline 2";
            $ctx->fail;
            diag "inline 3";
            note "inline 4";
            $ctx->release;
        }->();
        diag "isolated 3";
        note "isolated 4";
    };

    my $new = $one->squash_info;
    $one->squash_info(in_place => 1);
    is_deeply(
        $new,
        $one,
        "Squash and squash in place produce the same result"
    );

    is(@$one, 5, "5 events after squash");
    is_deeply([$one->[0]->info_messages], ['isolated 1'], "First event not modified");
    is_deeply([$one->[1]->info_messages], ['isolated 2'], "Second event not modified");
    is_deeply([$one->[3]->info_messages], ['isolated 3'], "second to last event not modified");
    is_deeply([$one->[4]->info_messages], ['isolated 4'], "last event not modified");
    is_deeply(
        [$one->[2]->info_messages],
        [
            'inline 1',
            'inline 2',
            'inline 3',
            'inline 4',
        ],
        "Assertion collected info generated in the same context"
    );
    ok($one->[2]->has_assert, "Assertion is still an assertion");


    my $two = intercept {

    };
};

tests messages => sub {
    my $one = intercept {
        note "foo";
        diag "bar";

        ok(1);

        sub {
            my $ctx = context();

            $ctx->send_ev2(
                errors => [
                    {tag => 'error', details => "Error 1" },
                    {tag => 'error', details => "Error 2" },
                ],
                info => [
                    {tag => 'DIAG', details => 'Diag 1'},
                    {tag => 'DIAG', details => 'Diag 2'},
                    {tag => 'NOTE', details => 'Note 1'},
                    {tag => 'NOTE', details => 'Note 2'},
                ],
            );

            $ctx->release;
        }->();

        note "baz";
        diag "bat";
    };

    is_deeply(
        $one->diag_messages,
        ['bar', 'Diag 1', 'Diag 2', 'bat'],
        "Got diags"
    );

    is_deeply(
        $one->note_messages,
        ['foo', 'Note 1', 'Note 2', 'baz'],
        "Got Notes"
    );

    is_deeply(
        $one->error_messages,
        ['Error 1', 'Error 2'],
        "Got errors"
    );
};

tests grep => sub {
    my $one = intercept {
        ok(1),                          # 0
        note "A Note";                  # 1
        diag "A Diag";                  # 2
        tests foo => sub { ok(1) };   # 3

        sub {                           # 4
            my $ctx = context();
            $ctx->send_ev2(errors => [{tag => 'error', details => "Error 1"}]);
            $ctx->release;
        }->();                          # 4

        plan 2;                         # 5
    };

    $one->upgrade(in_place => 1);

    is_deeply($one->asserts, [$one->[0], $one->[3]], "Got the asserts");
    is_deeply($one->subtests, [$one->[3]], "Got the subtests");
    is_deeply($one->diags, [$one->[2]], "Got the diags");
    is_deeply($one->notes, [$one->[1]], "Got the notes");
    is_deeply($one->errors, [$one->[4]], "Got the errors");
    is_deeply($one->plans, [$one->[5]], "Got the plans");

    $one->asserts(in_place => 1);
    is(@$one, 2, "2 events");
    ok($_->has_assert, "Is an assert") for @$one;
};

tests map => sub {
    my $one = intercept { ok(1); ok(2) };
    $one->upgrade(in_place => 1);

    is_deeply(
        $one->flatten,
        [ $one->[0]->flatten, $one->[1]->flatten ],
        "Flattened both events"
    );

    is_deeply(
        $one->briefs,
        [ $one->[0]->brief, $one->[1]->brief ],
        "Brief of both events"
    );

    is_deeply(
        $one->summaries,
        [ $one->[0]->summary, $one->[1]->summary ],
        "Summaries of both events"
    );

    my $two = intercept {
        tests foo => sub { ok(1) };
        ok(1);
        tests bar => sub { ok(1) };
    }->upgrade;

    is_deeply(
        $two->subtest_results,
        [ $two->[0]->subtest_result, $two->[2]->subtest_result ],
        "Got subtest results"
    );
};

done_testing;
