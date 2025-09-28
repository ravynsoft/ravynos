use strict;
use warnings;

use Test2::Tools::Tiny;

use Test2::API qw/run_subtest intercept context/;

# Test a subtest that should inherit the trace from the tool that calls it
my ($file, $line) = (__FILE__, __LINE__ + 1);
my $events = intercept { my_tool_inherit() };

is(@$events, 1, "got 1 event");
my $e = shift @$events;
ok($e->isa('Test2::Event::Subtest'), "got a subtest event");
is($e->trace->file, $file, "subtest is at correct file");
is($e->trace->line, $line, "subtest is at correct line");
my $plan = pop @{$e->subevents};
ok($plan->isa('Test2::Event::Plan'), "Removed plan");
for my $se (@{$e->subevents}) {
    is($se->trace->file, $file, "subtest event is at correct file");
    is($se->trace->line, $line, "subtest event is at correct line");
    ok($se->facets->{assert}->pass, "subtest event passed");
}




# Test a subtest that should NOT inherit the trace from the tool that calls it
($file, $line) = (__FILE__, __LINE__ + 1);
$events = intercept { my_tool_no_inherit() };

is(@$events, 1, "got 1 event");
$e = shift @$events;
ok($e->isa('Test2::Event::Subtest'), "got a subtest event");
is($e->trace->file, $file, "subtest is at correct file");
is($e->trace->line, $line, "subtest is at correct line");
$plan = pop @{$e->subevents};
ok($plan->isa('Test2::Event::Plan'), "Removed plan");
for my $se (@{$e->subevents}) {
    ok($se->trace->file ne $file, "subtest event is not in our file");
    ok($se->trace->line ne $line, "subtest event is not on our line");
    ok($se->facets->{assert}->{pass}, "subtest event passed");
}

done_testing;

# Make these tools appear to be in a different file/line
#line 100 'fake.pm'

sub my_tool_inherit {
    my $ctx = context();

    run_subtest(
        'foo',
        sub {
            ok(1, 'a');
            ok(2, 'b');
            is_deeply(\@_, [qw/arg1 arg2/], "got args");
        },
        {buffered => 1, inherit_trace => 1},
        'arg1', 'arg2'
    );

    $ctx->release;
}

sub my_tool_no_inherit {
    my $ctx = context();

    run_subtest(
        'foo',
        sub {
            ok(1, 'a');
            ok(2, 'b');
            is_deeply(\@_, [qw/arg1 arg2/], "got args");
        },
        {buffered => 1, inherit_trace => 0},
        'arg1', 'arg2'
    );

    $ctx->release;
}


