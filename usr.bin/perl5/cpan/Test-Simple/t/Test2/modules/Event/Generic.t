use strict;
use warnings;

use Test2::Tools::Tiny;
use Test2::EventFacet::Trace;

use Test2::API qw/context intercept/;

sub tool {
    my $ctx = context();
    my $e = $ctx->send_event('Generic', @_);
    $ctx->release;
    return $e;
}

my $e;
intercept { $e = tool() };

ok($e,                               "got event");
ok($e->isa('Test2::Event'),          "It is an event");
ok($e->isa('Test2::Event::Generic'), "It is an event");
delete $e->{trace};
is_deeply(
    $e,
    {
        causes_fail      => 0,
        increments_count => 0,
        diagnostics      => 0,
        no_display       => 0,
        _eid             => $e->eid,
        hubs             => [
            {
                'buffered' => 0,
                'details'  => 'Test2::Hub::Interceptor',
                'hid'      => $e->hubs->[0]->{hid},
                'ipc'      => 0,
                'nested'   => 0,
                'pid'      => $$,
                'tid'      => 0,
                $e->hubs->[0]->{uuid} ? (uuid => $e->hubs->[0]->{uuid}) : (uuid => undef),
            }
        ],
        $e->uuid ? (uuid => $e->uuid) : (),
    },
    "Defaults"
);

for my $f (qw/causes_fail increments_count diagnostics no_display/) {
    is($e->$f, 0, "'$f' is 0");
    is_deeply([$e->$f], [0], "'$f' is 0 is list context as well");

    my $set = "set_$f";
    $e->$set(1);
    is($e->$f, 1, "'$f' was set to 1");
}

for my $f (qw/callback terminate global sets_plan/) {
    is($e->$f, undef, "no $f");
    is_deeply([$e->$f], [], "$f is empty in list context");
}

like($e->summary, qr/Test2::Event::Generic/, "Got base class summary");

like(
    exception { $e->set_sets_plan('bad') },
    qr/'sets_plan' must be an array reference/,
    "Must provide an arrayref"
);

$e->set_sets_plan([0, skip => 'cause']);
is_deeply([$e->sets_plan], [0, skip => 'cause'], "sets_plan returns a list, not a ref");
$e->set_sets_plan(undef);
ok(!exists $e->{sets_plan}, "Removed sets_plan key");
ok(!$e->sets_plan, "sets_plan is cleared");

$e->set_global(0);
is($e->global, 0, "global is off");
$e->set_global(1);
is($e->global, 1, "global is on");
$e->set_global(0);
is($e->global, 0, "global is again");
$e->set_global(undef);
ok(!exists $e->{global}, "removed global key");
is($e->global, undef, "global is not defined");

like(
    exception { $e->set_callback('dogfood') },
    qr/callback must be a code reference/,
    "Callback must be code"
);

my $ran = 0;
$e->set_callback(sub {
    $ran++;
    my $self = shift;
    is($self, $e, "got self");
    is_deeply( \@_, ['a', 'b', 'c'], "Got args" );
    return 'foo';
});
is($e->callback('a', 'b', 'c'), 'foo', "got callback's return");
ok($ran, "ran callback");

$e->set_callback(undef);
ok(!$e->callback, "no callback");
ok(!exists $e->{callback}, "no callback key");

like(
    exception { $e->set_terminate('1.1') },
    qr/terminate must be a positive integer/,
    "terminate only takes integers"
);

like(
    exception { $e->set_terminate('foo') },
    qr/terminate must be a positive integer/,
    "terminate only takes numbers"
);

like(
    exception { $e->set_terminate('-1') },
    qr/terminate must be a positive integer/,
    "terminate only takes positive integers"
);

$e->set_terminate(0),
is($e->terminate, 0, "set to 0, 0 is valid");
$e->set_terminate(1),
is($e->terminate, 1, "set to 1");
$e->set_terminate(123),
is($e->terminate, 123, "set to 123");
$e->set_terminate(0),
is($e->terminate, 0, "set to 0, 0 is valid");

$e->set_terminate(undef);
is($e->terminate, undef, "terminate is not defined");
ok(!exists $e->{terminate}, "no terminate key");

# Test constructor args
intercept { $e = tool(causes_fail => 1, increments_count => 'a') };
is($e->causes_fail, 1, "attr from constructor");
is($e->increments_count, 'a', "attr from constructor");

done_testing;
