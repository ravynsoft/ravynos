use Test2::Tools::Tiny;
use Test2::API qw/test2_add_uuid_via context intercept/;

my %CNT;
test2_add_uuid_via(sub {
    my $type = shift;
    $CNT{$type} ||= 1;
    $type . '-' . $CNT{$type}++;
});

my $events = intercept {
    ok(1, "pass");

    sub {
        my $ctx = context();
        ok(1, "pass");
        ok(0, "fail");
        $ctx->release;
    }->();

    tests foo => sub {
        ok(1, "pass");
    };

    warnings {
        require Test::More;
        *subtest = \&Test::More::subtest;
    };

    subtest(foo => sub {
        ok(1, "pass");
    });
};

my $hub = Test2::API::test2_stack->top;
is($hub->uuid, 'hub-1', "First hub got a uuid");

is($events->[0]->uuid, 'event-1', "First event gets first uuid");
is($events->[0]->trace->uuid, 'context-2', "First event has correct context");
is($events->[0]->trace->huuid, 'hub-2', "First event has correct hub");

is($events->[0]->facet_data->{about}->{uuid}, "event-1", "The UUID makes it to facet data");

is($events->[1]->uuid, 'event-2', "Second event gets correct uuid");
is($events->[1]->trace->uuid, 'context-3', "Second event has correct context");
is($events->[1]->trace->huuid, 'hub-2', "Second event has correct hub");

is($events->[2]->uuid, 'event-3', "Third event gets correct uuid");
is($events->[2]->trace->uuid, $events->[1]->trace->uuid, "Third event shares context with event 2");
is($events->[2]->trace->huuid, 'hub-2', "Third event has correct hub");

is($events->[3]->uuid, 'event-6', "subtest event gets correct uuid (not next)");
is($events->[3]->subtest_uuid, 'hub-3', "subtest event gets correct subtest-uuid (next hub uuid)");
is($events->[3]->trace->uuid, 'context-4', "subtest gets next sequential context");
is($events->[3]->trace->huuid, 'hub-2', "subtest event has correct hub");

is($events->[3]->subevents->[0]->uuid, 'event-4', "First subevent gets next event uuid");
is($events->[3]->subevents->[0]->trace->uuid, 'context-5', "First subevent has correct context");
is($events->[3]->subevents->[0]->trace->huuid, 'hub-3', "First subevent has correct hub uuid (subtest hub uuid)");

is($events->[3]->subevents->[1]->uuid, 'event-5', "Second subevent gets next event uuid");
is($events->[3]->subevents->[1]->trace->uuid, $events->[3]->trace->uuid, "Second subevent has same context as subtest itself");
is($events->[3]->subevents->[1]->trace->huuid, 'hub-3', "Second subevent has correct hub uuid (subtest hub uuid)");

is($events->[5]->uuid, 'event-10', "subtest event gets correct uuid (not next)");
is($events->[5]->subtest_uuid, 'hub-4', "subtest event gets correct subtest-uuid (next hub uuid)");
is($events->[5]->trace->uuid, 'context-8', "subtest gets next sequential context");
is($events->[5]->trace->huuid, 'hub-2', "subtest event has correct hub");

is($events->[5]->subevents->[0]->uuid, 'event-8', "First subevent gets next event uuid");
is($events->[5]->subevents->[0]->trace->uuid, 'context-10', "First subevent has correct context");
is($events->[5]->subevents->[0]->trace->huuid, 'hub-4', "First subevent has correct hub uuid (subtest hub uuid)");

is($events->[5]->subevents->[1]->uuid, 'event-9', "Second subevent gets next event uuid");
is($events->[5]->subevents->[1]->trace->uuid, $events->[5]->trace->uuid, "Second subevent has same context as subtest itself");
is($events->[5]->subevents->[1]->trace->huuid, 'hub-2', "Second subevent has correct hub uuid (subtest hub uuid)");

done_testing;
