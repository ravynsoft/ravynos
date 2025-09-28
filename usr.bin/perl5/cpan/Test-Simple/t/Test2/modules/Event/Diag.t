use strict;
use warnings;
use Test2::Tools::Tiny;
use Test2::Event::Diag;
use Test2::EventFacet::Trace;

my $diag = Test2::Event::Diag->new(
    trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    message => 'foo',
);

is($diag->summary, 'foo', "summary is just message");

$diag = Test2::Event::Diag->new(
    trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    message => undef,
);

is($diag->message, 'undef', "set undef message to undef");
is($diag->summary, 'undef', "summary is just message even when undef");

$diag = Test2::Event::Diag->new(
    trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    message => {},
);

like($diag->message, qr/^HASH\(.*\)$/, "stringified the input value");

ok($diag->diagnostics, "Diag events are counted as diagnostics");

$diag = Test2::Event::Diag->new(
    trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    message => "Hi there",
);

my $facet_data = $diag->facet_data;
ok($facet_data->{about}, "Got 'about' from common");
ok($facet_data->{trace}, "Got 'trace' from common");

is_deeply(
    $facet_data->{info},
    [{
        tag => 'DIAG',
        debug => 1,
        details => 'Hi there',
    }],
    "Got info facet"
);

done_testing;
