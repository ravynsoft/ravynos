use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::API qw/intercept context/;
use Test2::Util qw/get_tid/;

my $line;
my $events = intercept {
    $line = __LINE__ + 1;
    ok(1, "pass");
    sub {
        my $ctx = context;
        $ctx->pass;
        $ctx->pass;
        $ctx->release;
    }->();
};

my $sigpass = $events->[0]->trace->signature;
my $sigfail = $events->[1]->trace->signature;

ok($sigpass ne $sigfail, "Each tool got a new signature");

is($events->[$_]->trace->signature, $sigfail, "Diags share failed ok's signature") for 2 .. $#$events;

like($sigpass, qr/$$~${ \get_tid() }~\d+~\d+:$$:\Q${ \get_tid() }:${ \__FILE__ }:$line\E$/, "signature is sane");

my $trace = Test2::EventFacet::Trace->new(frame => ['main', 'foo.t', 42, 'xxx']);
delete $trace->{cid};
is($trace->signature, undef, "No signature without a cid");

is($events->[0]->related($events->[1]), 0, "event 0 is not related to event 1");
is($events->[1]->related($events->[2]), 1, "event 1 is related to event 2");

my $e = Test2::Event::Ok->new(pass => 1);
is($e->related($events->[0]), undef, "Cannot check relation, invalid trace");

$e = Test2::Event::Ok->new(pass => 1, trace => Test2::EventFacet::Trace->new(frame => ['', '', '', '']));
is($e->related($events->[0]), undef, "Cannot check relation, incomplete trace");

$e = Test2::Event::Ok->new(pass => 1, trace => Test2::EventFacet::Trace->new(frame => []));
is($e->related($events->[0]), undef, "Cannot check relation, incomplete trace");

done_testing;
