use strict;
use warnings;

use Test2::Tools::Tiny;

use Test2::API qw/intercept/;

tests in_eval => sub {
    my $events = intercept {
        eval { skip_all "foo" };
        die "Should not see this: $@";
    };

    is(@$events, 1, "got 1 event");
    ok($events->[0]->isa('Test2::Event::Plan'), "Plan is only event");
    is($events->[0]->directive, 'SKIP', "Plan is to skip");
};

tests no_eval => sub {
    my $events = intercept {
        skip_all "foo";
        die "Should not see this: $@";
    };

    is(@$events, 1, "got 1 event");
    ok($events->[0]->isa('Test2::Event::Plan'), "Plan is only event");
    is($events->[0]->directive, 'SKIP', "Plan is to skip");
};

tests in_require => sub {
    my $events = intercept {
        require './t/lib/SkipAll.pm';
        die "Should not see this: $@";
    };

    is(@$events, 1, "got 1 event");
    ok($events->[0]->isa('Test2::Event::Plan'), "Plan is only event");
    is($events->[0]->directive, 'SKIP', "Plan is to skip");
};

done_testing;
