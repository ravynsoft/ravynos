use Test2::Tools::Tiny;
use strict;
use warnings;

use Test2::Event::Skip;
use Test2::EventFacet::Trace;

my $skip = Test2::Event::Skip->new(
    trace  => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__]),
    name   => 'skip me',
    reason => 'foo',
);

my $facet_data = $skip->facet_data;
ok($facet_data->{about}, "Got basic data");
is_deeply(
    $facet_data->{amnesty},
    [
        {
            tag       => 'skip',
            details   => 'foo',
            inherited => 0,
        }
    ],
    "Added some amnesty for the skip",
);

is($skip->name, 'skip me', "set name");
is($skip->reason, 'foo', "got skip reason");
ok(!$skip->pass, "no default for pass");
ok($skip->effective_pass, "TODO always effectively passes");

is($skip->summary, "skip me (SKIP: foo)", "summary with reason");

$skip->set_reason('');
is($skip->summary, "skip me (SKIP)", "summary without reason");

done_testing;
