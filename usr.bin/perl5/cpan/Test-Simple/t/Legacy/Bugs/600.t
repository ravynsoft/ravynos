use Test2::API qw/intercept/;
use Test::More;

my $TEST = Test::Builder->new();

sub fake {
    $TEST->use_numbers(0);
    $TEST->no_ending(1);
    $TEST->done_testing(1);    # a computed number of tests from its deferred magic
}

my $events = intercept { fake() };
is(@$events, 1, "only 1 event");
is($events->[0]->max, 1, "Plan set to 1, not 0");

done_testing;
