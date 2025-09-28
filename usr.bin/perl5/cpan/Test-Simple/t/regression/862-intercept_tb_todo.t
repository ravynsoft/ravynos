use strict;
use warnings;

use Test::More;
use Test2::API qw/intercept/;

my $events;
{
    local $TODO = "main-outer-todo";

    package Foo;

    our $TODO;
    local $TODO = "foo-outer-todo";

    $events = main::intercept(sub {
        main::ok(1, "assertion 1");

        {
            local $main::TODO = "main-inner-todo";
            main::ok(1, "assertion 2");
        }

        {
            local $Foo::TODO = "foo-inner-todo";
            main::ok(1, "assertion 3");
        }

        main::ok(1, "assertion 4");
    });

    # Cannot use intercept, so make a failing test, the overall test file
    # should still pass because this is todo. If this is not todo we know we
    # broke something by the test failing overall.
    main::ok(0, "Verifying todo, this should be a failed todo test");
}

@$events = grep { $_->facet_data->{assert} } @$events;

ok(!$events->[0]->facet_data->{amnesty}, "No amnesty for the first event, \$TODO was cleaned");

is_deeply(
    $events->[1]->facet_data->{amnesty},
    [{
        tag     => 'TODO',
        details => 'main-inner-todo',
    }],
    "The second event had the expected amnesty applied",
);

is_deeply(
    $events->[2]->facet_data->{amnesty},
    [{
        tag     => 'TODO',
        details => 'foo-inner-todo',
    }],
    "The third event had the expected amnesty applied",
);

ok(!$events->[3]->facet_data->{amnesty}, "No amnesty for the fourth event, \$TODO was cleaned");

done_testing;
