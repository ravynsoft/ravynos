use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::Event::Waiting;

my $waiting = Test2::Event::Waiting->new(
    trace => {},
);

ok($waiting, "Created event");
ok($waiting->global, "waiting is global");

is($waiting->summary, "IPC is waiting for children to finish...", "Got summary");

my $facet_data = $waiting->facet_data;
ok($facet_data->{about}, "Got common facet data");

is_deeply(
    $facet_data->{info},
    [
        {
            tag     => 'INFO',
            debug   => 0,
            details => "IPC is waiting for children to finish...",
        },
    ],
    "Got added info facet"
);

done_testing;
