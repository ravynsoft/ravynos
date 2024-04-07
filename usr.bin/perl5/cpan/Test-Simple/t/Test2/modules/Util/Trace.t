use strict;
use warnings;
use Test2::Tools::Tiny;
use Test2::EventFacet::Trace;

like(
    exception { 'Test2::EventFacet::Trace'->new() },
    qr/The 'frame' attribute is required/,
    "got error"
);

my $one = 'Test2::EventFacet::Trace'->new(frame => ['Foo::Bar', 'foo.t', 5, 'Foo::Bar::foo']);
is_deeply($one->frame,  ['Foo::Bar', 'foo.t', 5, 'Foo::Bar::foo'], "Got frame");
is_deeply([$one->call], ['Foo::Bar', 'foo.t', 5, 'Foo::Bar::foo'], "Got call");
is($one->package, 'Foo::Bar',      "Got package");
is($one->file,    'foo.t',         "Got file");
is($one->line,    5,               "Got line");
is($one->subname, 'Foo::Bar::foo', "got subname");

is($one->debug, "at foo.t line 5", "got trace");
$one->set_detail("yo momma");
is($one->debug, "yo momma", "got detail for trace");
$one->set_detail(undef);

is(
    exception { $one->throw('I died') },
    "I died at foo.t line 5.\n",
    "got exception"
);

is_deeply(
    warnings { $one->alert('I cried') },
    [ "I cried at foo.t line 5.\n" ],
    "alter() warns"
);

my $snap = $one->snapshot;
is_deeply($snap, $one, "identical");
ok($snap != $one, "Not the same instance");

done_testing;
