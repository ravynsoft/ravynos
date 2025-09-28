use Test2::Tools::Tiny;
use Test2::API qw/intercept context/;

{
    $INC{'My/Event.pm'} = 1;

    package My::Event;
    use base 'Test2::Event';

    use Test2::Util::Facets2Legacy ':ALL';

    sub facet_data {
        my $self = shift;

        my $out = $self->common_facet_data;

        $out->{errors} = [{tag => 'OOPS', fail => !$ENV{FAILURE_DO_PASS}, details => "An error occurred"}];

        return $out;
    }
}

sub error {
    my $ctx = context();
    my $e = $ctx->send_event('+My::Event');
    $ctx->release;
    return $e;
}

my $events = intercept {
    tests foo => sub {
        ok(1, "need at least 1 assertion");
        error();
    };
};

ok(!$events->[0]->pass, "Subtest did not pass");

my ($passing_a, $passing_b);
intercept {
    my $hub = Test2::API::test2_stack->top;

    $passing_a = $hub->is_passing;

    error();

    $passing_b = $hub->is_passing;
};

ok($passing_a, "Passign before error");
ok(!$passing_b, "Not passing after error");

done_testing;
