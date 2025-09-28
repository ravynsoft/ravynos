use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::Event::Note;
use Test2::EventFacet::Trace;

my $note = Test2::Event::Note->new(
    trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    message => 'foo',
);

is($note->summary, 'foo', "summary is just message");

$note = Test2::Event::Note->new(
    trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    message => undef,
);

is($note->message, 'undef', "set undef message to undef");
is($note->summary, 'undef', "summary is just message even when undef");

$note = Test2::Event::Note->new(
    trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    message => {},
);

like($note->message, qr/^HASH\(.*\)$/, "stringified the input value");

$note = Test2::Event::Note->new(
    trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    message => 'Hi there',
);

my $facet_data = $note->facet_data;
ok($facet_data->{about}, "Got 'about' from common");
ok($facet_data->{trace}, "Got 'trace' from common");

is_deeply(
    $facet_data->{info},
    [{
        tag => 'NOTE',
        debug => 0,
        details => 'Hi there',
    }],
    "Got info facet"
);


done_testing;
