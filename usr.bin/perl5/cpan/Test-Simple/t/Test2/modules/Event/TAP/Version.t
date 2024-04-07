use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::Event::TAP::Version';
my $CLASS = 'Test2::Event::TAP::Version';

like(
    exception { $CLASS->new() },
    qr/'version' is a required attribute/,
    "Must specify the version"
);

my $one = $CLASS->new(version => 13);
is($one->version, 13, "Got version");
is($one->summary, "TAP version 13", "Got summary");

is_deeply(
    $one->facet_data,
    {
        about => { package => $CLASS, details => "TAP version 13", eid => $one->eid},
        info => [{tag => 'INFO', debug => 0, details => "TAP version 13"}],
    },
    "Got facet data"
);

done_testing;
