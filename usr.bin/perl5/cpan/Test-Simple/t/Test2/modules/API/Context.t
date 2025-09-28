use strict;
use warnings;

BEGIN { $Test2::API::DO_DEPTH_CHECK = 1 }
use Test2::Tools::Tiny;

use Test2::API qw{
    context intercept
    test2_stack
    test2_add_callback_context_acquire
    test2_add_callback_context_init
    test2_add_callback_context_release
};

my $error = exception { context(); 1 };
my $exception = "context() called, but return value is ignored at " . __FILE__ . ' line ' . (__LINE__ - 1);
like($error, qr/^\Q$exception\E/, "Got the exception" );

my $ref;
my $frame;
sub wrap(&) {
    my $ctx = context();
    my ($pkg, $file, $line, $sub) = caller(0);
    $frame = [$pkg, $file, $line, $sub];

    $_[0]->($ctx);

    $ref = "$ctx";

    $ctx->release;
}

wrap {
    my $ctx = shift;
    ok($ctx->hub, "got hub");
    delete $ctx->trace->frame->[4];
    is_deeply($ctx->trace->frame, $frame, "Found place to report errors");
};

wrap {
    my $ctx = shift;
    ok("$ctx" ne "$ref", "Got a new context");
    my $new = context();
    my @caller = caller(0);
    is_deeply(
        $new,
        {%$ctx, _is_canon => undef, _is_spawn => [@caller[0,1,2,3]]},
        "Additional call to context gets spawn"
    );
    delete $ctx->trace->frame->[4];
    is_deeply($ctx->trace->frame, $frame, "Found place to report errors");
    $new->release;
};

wrap {
    my $ctx = shift;
    my $snap = $ctx->snapshot;

    is_deeply(
        $snap,
        {%$ctx, _is_canon => undef, _is_spawn => undef, _aborted => undef},
        "snapshot is identical except for canon/spawn/aborted"
    );
    ok($ctx != $snap, "snapshot is a new instance");
};

my $end_ctx;
{ # Simulate an END block...
    local *END = sub { local *__ANON__ = 'END'; context() };
    my $ctx = END();
    $frame = [ __PACKAGE__, __FILE__, __LINE__ - 1, 'main::END' ];
    # "__LINE__ - 1" on the preceding line forces the value to be an IV
    # (even though __LINE__ on its own is a PV), just as (caller)[2] is.
    $end_ctx = $ctx->snapshot;
    $ctx->release;
}
delete $end_ctx->trace->frame->[4];
is_deeply( $end_ctx->trace->frame, $frame, 'context is ok in an end block');

# Test event generation
{
    package My::Formatter;

    sub write {
        my $self = shift;
        my ($e) = @_;
        push @$self => $e;
    }
}
my $events = bless [], 'My::Formatter';
my $hub = Test2::Hub->new(
    formatter => $events,
);
my $trace = Test2::EventFacet::Trace->new(
    frame => [ 'Foo::Bar', 'foo_bar.t', 42, 'Foo::Bar::baz' ],
);
my $ctx = Test2::API::Context->new(
    trace => $trace,
    hub   => $hub,
);

my $e = $ctx->build_event('Ok', pass => 1, name => 'foo');
is($e->pass, 1, "Pass");
is($e->name, 'foo', "got name");
is_deeply($e->trace, $trace, "Got the trace info");
ok(!@$events, "No events yet");

$e = $ctx->send_event('Ok', pass => 1, name => 'foo');
is($e->pass, 1, "Pass");
is($e->name, 'foo', "got name");
is_deeply($e->trace, $trace, "Got the trace info");
is(@$events, 1, "1 event");
is_deeply($events, [$e], "Hub saw the event");
pop @$events;

$e = $ctx->ok(1, 'foo');
is($e->pass, 1, "Pass");
is($e->name, 'foo', "got name");
is_deeply($e->trace, $trace, "Got the trace info");
is(@$events, 1, "1 event");
is_deeply($events, [$e], "Hub saw the event");
pop @$events;

$e = $ctx->note('foo');
is($e->message, 'foo', "got message");
is_deeply($e->trace, $trace, "Got the trace info");
is(@$events, 1, "1 event");
is_deeply($events, [$e], "Hub saw the event");
pop @$events;

$e = $ctx->diag('foo');
is($e->message, 'foo', "got message");
is_deeply($e->trace, $trace, "Got the trace info");
is(@$events, 1, "1 event");
is_deeply($events, [$e], "Hub saw the event");
pop @$events;

$e = $ctx->plan(100);
is($e->max, 100, "got max");
is_deeply($e->trace, $trace, "Got the trace info");
is(@$events, 1, "1 event");
is_deeply($events, [$e], "Hub saw the event");
pop @$events;

$e = $ctx->skip('foo', 'because');
is($e->name, 'foo', "got name");
is($e->reason, 'because', "got reason");
ok($e->pass, "skip events pass by default");
is_deeply($e->trace, $trace, "Got the trace info");
is(@$events, 1, "1 event");
is_deeply($events, [$e], "Hub saw the event");
pop @$events;

$e = $ctx->skip('foo', 'because', pass => 0);
ok(!$e->pass, "can override skip params");
pop @$events;

# Test hooks

my @hooks;
$hub =  test2_stack()->top;
my $ref1 = $hub->add_context_init(sub {    die "Bad Arg" unless ref($_[0]) eq 'Test2::API::Context'; push @hooks => 'hub_init'       });
my $ref2 = $hub->add_context_release(sub { die "Bad Arg" unless ref($_[0]) eq 'Test2::API::Context'; push @hooks => 'hub_release'    });
test2_add_callback_context_init(sub {      die "Bad Arg" unless ref($_[0]) eq 'Test2::API::Context'; push @hooks => 'global_init'    });
test2_add_callback_context_release(sub {   die "Bad Arg" unless ref($_[0]) eq 'Test2::API::Context'; push @hooks => 'global_release' });

my $ref3 = $hub->add_context_acquire(sub { die "Bad Arg" unless ref($_[0]) eq 'HASH'; push @hooks => 'hub_acquire'     });
test2_add_callback_context_acquire(sub {   die "Bad Arg" unless ref($_[0]) eq 'HASH'; push @hooks => 'global_acquire'  });

sub {
    push @hooks => 'start';
    my $ctx = context(on_init => sub { push @hooks => 'ctx_init' }, on_release => sub { push @hooks => 'ctx_release' });
    push @hooks => 'deep';
    my $ctx2 = sub {
        context(on_init => sub { push @hooks => 'ctx_init_deep' }, on_release => sub { push @hooks => 'ctx_release_deep' });
    }->();
    push @hooks => 'release_deep';
    $ctx2->release;
    push @hooks => 'release_parent';
    $ctx->release;
    push @hooks => 'released_all';

    push @hooks => 'new';
    $ctx = context(on_init => sub { push @hooks => 'ctx_init2' }, on_release => sub { push @hooks => 'ctx_release2' });
    push @hooks => 'release_new';
    $ctx->release;
    push @hooks => 'done';
}->();

$hub->remove_context_init($ref1);
$hub->remove_context_release($ref2);
$hub->remove_context_acquire($ref3);
@{Test2::API::_context_init_callbacks_ref()} = ();
@{Test2::API::_context_release_callbacks_ref()} = ();
@{Test2::API::_context_acquire_callbacks_ref()} = ();

is_deeply(
    \@hooks,
    [qw{
        start
        global_acquire
        hub_acquire
        global_init
        hub_init
        ctx_init
        deep
        global_acquire
        hub_acquire
        release_deep
        release_parent
        ctx_release_deep
        ctx_release
        hub_release
        global_release
        released_all
        new
        global_acquire
        hub_acquire
        global_init
        hub_init
        ctx_init2
        release_new
        ctx_release2
        hub_release
        global_release
        done
    }],
    "Got all hook in correct order"
);

{
    my $ctx = context(level => -1);

    my $one = Test2::API::Context->new(
        trace => Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__, 'blah']),
        hub => test2_stack()->top,
    );
    is($one->_depth, 0, "default depth");

    my $ran = 0;
    my $doit = sub {
        is_deeply(\@_, [qw/foo bar/], "got args");
        $ran++;
        die "Make sure old context is restored";
    };

    eval { $one->do_in_context($doit, 'foo', 'bar') };

    my $spawn = context(level => -1, wrapped => -2);
    is($spawn->trace, $ctx->trace, "Old context restored");
    $spawn->release;
    $ctx->release;

    ok(!exception { $one->do_in_context(sub {1}) }, "do_in_context works without an original")
}

{
    like(exception { Test2::API::Context->new() }, qr/The 'trace' attribute is required/, "need to have trace");

    my $trace = Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__, 'foo']);
    like(exception { Test2::API::Context->new(trace => $trace) }, qr/The 'hub' attribute is required/, "need to have hub");

    my $hub = test2_stack()->top;
    my $ctx = Test2::API::Context->new(trace => $trace, hub => $hub);
    is($ctx->{_depth}, 0, "depth set to 0 when not defined.");

    $ctx = Test2::API::Context->new(trace => $trace, hub => $hub, _depth => 1);
    is($ctx->{_depth}, 1, "Do not reset depth");

    like(
        exception { $ctx->release },
        qr/release\(\) should not be called on context that is neither canon nor a child/,
        "Non canonical context, do not release"
    );
}

sub {
    like(
        exception { my $ctx = context(level => 20) },
        qr/Could not find context at depth 21/,
        "Level sanity"
    );

    ok(
        !exception {
            my $ctx = context(level => 20, fudge => 1);
            $ctx->release;
        },
        "Was able to get context when fudging level"
    );
}->();

sub {
    my ($ctx1, $ctx2);
    sub { $ctx1 = context() }->();

    my @warnings;
    {
        local $SIG{__WARN__} = sub { push @warnings => @_ };
        $ctx2 = context();
        $ctx1 = undef;
    }

    $ctx2->release;

    is(@warnings, 1, "1 warning");
    like(
        $warnings[0],
        qr/^context\(\) was called to retrieve an existing context, however the existing/,
        "Got expected warning"
    );
}->();

sub {
    my $ctx = context();
    my $e = exception { $ctx->throw('xxx') };
    like($e, qr/xxx/, "got exception");

    $ctx = context();
    my $warnings = warnings { $ctx->alert('xxx') };
    like($warnings->[0], qr/xxx/, "got warning");
    $ctx->release;
}->();

sub {
    my $ctx = context;

    is($ctx->_parse_event('Ok'), 'Test2::Event::Ok', "Got the Ok event class");
    is($ctx->_parse_event('+Test2::Event::Ok'), 'Test2::Event::Ok', "Got the +Ok event class");

    like(
        exception { $ctx->_parse_event('+DFASGFSDFGSDGSD') },
        qr/Could not load event module 'DFASGFSDFGSDGSD': Can't locate DFASGFSDFGSDGSD\.pm/,
        "Bad event type"
    );
}->();

{
    my ($e1, $e2);
    my $events = intercept {
        my $ctx = context();
        $e1 = $ctx->ok(0, 'foo', ['xxx']);
        $e2 = $ctx->ok(0, 'foo');
        $ctx->release;
    };

    ok($e1->isa('Test2::Event::Ok'), "returned ok event");
    ok($e2->isa('Test2::Event::Ok'), "returned ok event");

    is($events->[0], $e1, "got ok event 1");
    is($events->[3], $e2, "got ok event 2");

    is($events->[2]->message, 'xxx', "event 1 diag 2");
    ok($events->[2]->isa('Test2::Event::Diag'), "event 1 diag 2 is diag");

    is($events->[3], $e2, "got ok event 2");
}

sub {
    local $! = 100;
    local $@ = 'foobarbaz';
    local $? = 123;

    my $ctx = context();

    is($ctx->errno,       100,         "saved errno");
    is($ctx->eval_error,  'foobarbaz', "saved eval error");
    is($ctx->child_error, 123,         "saved child exit");

    $! = 22;
    $@ = 'xyz';
    $? = 33;

    is(0 + $!, 22,    "altered \$! in tool");
    is($@,     'xyz', "altered \$@ in tool");
    is($?,     33,    "altered \$? in tool");

    sub {
        my $ctx2 = context();

        $! = 42;
        $@ = 'app';
        $? = 43;

        is(0 + $!, 42,    "altered \$! in tool (nested)");
        is($@,     'app', "altered \$@ in tool (nested)");
        is($?,     43,    "altered \$? in tool (nested)");

        $ctx2->release;

        is(0 + $!, 22,    "restored the nested \$! in tool");
        is($@,     'xyz', "restored the nested \$@ in tool");
        is($?,     33,    "restored the nested \$? in tool");
    }->();

    sub {
        my $ctx2 = context();

        $! = 42;
        $@ = 'app';
        $? = 43;

        is(0 + $!, 42,    "altered \$! in tool (nested)");
        is($@,     'app', "altered \$@ in tool (nested)");
        is($?,     43,    "altered \$? in tool (nested)");

        # Will not warn since $@ is changed
        $ctx2 = undef;

        is(0 + $!, 42,    'Destroy does not reset $!');
        is($@,     'app', 'Destroy does not reset $@');
        is($?,     43,    'Destroy does not reset $?');
    }->();

    $ctx->release;

    is($ctx->errno,       100,         "restored errno");
    is($ctx->eval_error,  'foobarbaz', "restored eval error");
    is($ctx->child_error, 123,         "restored child exit");
}->();


sub {
    local $! = 100;
    local $@ = 'foobarbaz';
    local $? = 123;

    my $ctx = context();

    is($ctx->errno,       100,         "saved errno");
    is($ctx->eval_error,  'foobarbaz', "saved eval error");
    is($ctx->child_error, 123,         "saved child exit");

    $! = 22;
    $@ = 'xyz';
    $? = 33;

    is(0 + $!, 22,    "altered \$! in tool");
    is($@,     'xyz', "altered \$@ in tool");
    is($?,     33,    "altered \$? in tool");

    # Will not warn since $@ is changed
    $ctx = undef;

    is(0 + $!, 22,    "Destroy does not restore \$!");
    is($@,     'xyz', "Destroy does not restore \$@");
    is($?,     33,    "Destroy does not restore \$?");
}->();

sub {
    require Test2::EventFacet::Info::Table;

    my $events = intercept {
        my $ctx = context();

        $ctx->fail('foo', 'bar', Test2::EventFacet::Info::Table->new(rows => [['a', 'b']]));
        $ctx->fail_and_release('foo', 'bar', Test2::EventFacet::Info::Table->new(rows => [['a', 'b']], as_string => 'a, b'));
    };

    is(@$events, 2, "got 2 events");

    is($events->[0]->{info}->[0]->{details}, 'bar', "got first diag");
    is($events->[0]->{info}->[1]->{details}, '<TABLE NOT DISPLAYED>', "second diag has default details");
    is_deeply(
        $events->[0]->{info}->[1]->{table},
        {rows => [['a', 'b']]},
        "Got the table rows"
    );

    is($events->[1]->{info}->[0]->{details}, 'bar', "got first diag");
    is($events->[1]->{info}->[1]->{details}, 'a, b', "second diag has custom details");
    is_deeply(
        $events->[1]->{info}->[1]->{table},
        {rows => [['a', 'b']]},
        "Got the table rows"
    );

}->();

sub ctx_destroy_test {
    my (undef, undef, $line1) = caller();
    my (@warn, $line2);
    local $SIG{__WARN__} = sub { push @warn => $_[0] };

    { my $ctx = context(); $ctx = undef } $line2 = __LINE__;

    use Data::Dumper;
#    print Dumper(@warn);

    like($warn[0], qr/context appears to have been destroyed without first calling release/, "Is normal context warning");
    like($warn[0], qr{\QContext destroyed at ${ \__FILE__ } line $line2\E}, "Reported context destruction trace");

    my $created = <<"    EOT";
Here are the context creation details, just in case a tool forgot to call
release():
  File: ${ \__FILE__ }
  Line: $line1
  Tool: main::ctx_destroy_test
    EOT

    like($warn[0], qr{\Q$created\E}, "Reported context creation details");
};

ctx_destroy_test();

done_testing;
