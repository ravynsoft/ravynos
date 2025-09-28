use Test2::Tools::Tiny;
use strict;
use warnings;

use Test2::API qw/context run_subtest intercept/;

sub do_it {
    my $ctx = context();

    run_subtest foo =>  sub {
        ok(1, "pass");
    }, {inherit_trace => 1};

    $ctx->release;
}

do_it();
do_it();

my $events = intercept {
    do_it();
    do_it();
};

for my $st (@$events) {
    next unless $st->isa('Test2::Event::Subtest');

    is($st->trace->nested, 0, "base subtest is not nested");

    is($_->trace->nested, 1, "subevent is nested") for @{$st->subevents};
}

done_testing;
