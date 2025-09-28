use Test::More;
use strict;
use warnings;

use Test2::API qw/intercept/;
my @events;

intercept {
    local $TODO = "broken";

    Test2::API::test2_stack->top->listen(sub { push @events => $_[1] }, inherit => 1);

    subtest foo => sub {
        subtest bar => sub {
            ok(0, 'oops');
        };
    };
};

my ($event) = grep { $_->trace->line == 16 && ref($_) eq 'Test::Builder::TodoDiag'} @events;
ok($event, "nested todo diag on line 16 was changed to TodoDiag (STDOUT instead of STDERR)");

done_testing;
