use strict;
use warnings;

use Test2::Tools::Tiny;

use ok 'Test2::Event::Encoding';
my $CLASS = 'Test2::Event::Encoding';

like(
    exception { $CLASS->new() },
    qr/'encoding' is a required attribute/,
    "Must specify the encoding"
);

my $one = $CLASS->new(encoding => 'utf8');
is($one->encoding, 'utf8', "Got encoding");
is($one->summary, "Encoding set to utf8", "Got summary");

is_deeply(
    $one->facet_data,
    {
        about => {
            package => $CLASS,
            details => "Encoding set to utf8",
            eid     => $one->eid,
        },
        control => { encoding => 'utf8' },
    },
    "Got facet data"
);

done_testing;
