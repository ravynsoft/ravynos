use strict;
use warnings;

use Test2::API qw/context test2_stack/;

sub done_testing {
    my $ctx = context();

    die "Test Already ended!" if $ctx->hub->ended;
    $ctx->hub->finalize($ctx->trace, 1);
    $ctx->release;
}

sub ok($;$) {
    my ($bool, $name) = @_;
    my $ctx = context();
    $ctx->ok($bool, $name);
    $ctx->release;
}

sub diag {
    my $ctx = context();
    $ctx->diag( join '', @_ );
    $ctx->release;
}

ok(1, "First");

my $filter = test2_stack->top->filter(sub {
    my ($hub, $event) = @_;

    # Turn a diag into a note
    return Test2::Event::Note->new(%$event) if ref($event) eq 'Test2::Event::Diag';

    # Set todo on ok's
    if ($event->isa('Test2::Event::Ok')) {
        $event->set_todo('here be dragons');
        $event->set_effective_pass(1);
    }

    return $event;
});

ok(0, "Second");
diag "should be a note";

test2_stack->top->unfilter($filter);

ok(1, "Third");

done_testing;
