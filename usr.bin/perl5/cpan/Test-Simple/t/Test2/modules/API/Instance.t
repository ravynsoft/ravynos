use strict;
use warnings;

use Test2::IPC;
use Test2::Tools::Tiny;
use Test2::Util qw/CAN_THREAD CAN_REALLY_FORK USE_THREADS get_tid/;

ok(1, "Just to get things initialized.");

# We need to control this env var for this test
$ENV{T2_NO_IPC} = 0;
# This test relies on TAP being the default formatter for non-canon instances
$ENV{T2_FORMATTER} = 'TAP';

my $CLASS = 'Test2::API::Instance';

my $one = $CLASS->new;
is_deeply(
    $one,
    {
        contexts => {},

        finalized => undef,
        ipc       => undef,
        formatter => undef,

        add_uuid_via => undef,

        ipc_polling    => undef,
        ipc_drivers    => [],
        ipc_timeout    => 30,
        ipc_disabled   => 0,

        formatters => [],

        no_wait => 0,
        loaded  => 0,

        exit_callbacks            => [],
        post_load_callbacks       => [],
        context_acquire_callbacks => [],
        context_init_callbacks    => [],
        context_release_callbacks => [],
        pre_subtest_callbacks     => [],

        stack => [],
    },
    "Got initial settings"
);

%$one = ();
is_deeply($one, {}, "wiped object");

$one->reset;
is_deeply(
    $one,
    {
        contexts => {},

        ipc_polling  => undef,
        ipc_drivers  => [],
        ipc_timeout  => 30,
        ipc_disabled => 0,

        add_uuid_via => undef,

        formatters => [],

        finalized => undef,
        ipc       => undef,
        formatter => undef,

        no_wait => 0,
        loaded  => 0,

        exit_callbacks            => [],
        post_load_callbacks       => [],
        context_acquire_callbacks => [],
        context_init_callbacks    => [],
        context_release_callbacks => [],
        pre_subtest_callbacks     => [],

        stack => [],
    },
    "Reset Object"
);

ok(!$one->formatter_set, "no formatter set");
$one->set_formatter('Foo');
ok($one->formatter_set, "formatter set");
$one->reset;

my $ran = 0;
my $callback = sub { $ran++ };
$one->add_post_load_callback($callback);
ok(!$ran, "did not run yet");
is_deeply($one->post_load_callbacks, [$callback], "stored callback for later");

ok(!$one->loaded, "not loaded");
$one->load;
ok($one->loaded, "loaded");
is($ran, 1, "ran the callback");

$one->load;
is($ran, 1, "Did not run the callback again");

$one->add_post_load_callback($callback);
is($ran, 2, "ran the new callback");
is_deeply($one->post_load_callbacks, [$callback, $callback], "stored callback for the record");

like(
    exception { $one->add_post_load_callback({}) },
    qr/Post-load callbacks must be coderefs/,
    "Post-load callbacks must be coderefs"
);

$one->reset;
ok($one->ipc, 'got ipc');
ok($one->finalized, "calling ipc finalized the object");

$one->reset;
ok($one->stack, 'got stack');
ok(!$one->finalized, "calling stack did not finaliz the object");

$one->reset;
ok($one->formatter, 'Got formatter');
ok($one->finalized, "calling format finalized the object");

$one->reset;
$one->set_formatter('Foo');
is($one->formatter, 'Foo', "got specified formatter");
ok($one->finalized, "calling format finalized the object");

{
    local $ENV{T2_FORMATTER} = 'TAP';
    my $one = $CLASS->new;
    is($one->formatter, 'Test2::Formatter::TAP', "got specified formatter");
    ok($one->finalized, "calling format finalized the object");

    local $ENV{T2_FORMATTER} = '+Test2::Formatter::TAP';
    $one->reset;
    is($one->formatter, 'Test2::Formatter::TAP', "got specified formatter");
    ok($one->finalized, "calling format finalized the object");

    local $ENV{T2_FORMATTER} = '+A::Fake::Module::That::Should::Not::Exist';
    $one->reset;
    like(
        exception { $one->formatter },
        qr/COULD NOT LOAD FORMATTER 'A::Fake::Module::That::Should::Not::Exist' \(set by the 'T2_FORMATTER' environment variable\)/,
        "Bad formatter"
    );
}

$ran = 0;
$one->reset;
$one->add_exit_callback($callback);
is(@{$one->exit_callbacks}, 1, "added an exit callback");
$one->add_exit_callback($callback);
is(@{$one->exit_callbacks}, 2, "added another exit callback");

like(
    exception { $one->add_exit_callback({}) },
    qr/End callbacks must be coderefs/,
    "Exit callbacks must be coderefs"
);

$one->reset;
$one->add_pre_subtest_callback($callback);
is(@{$one->pre_subtest_callbacks}, 1, "added a pre-subtest callback");
$one->add_pre_subtest_callback($callback);
is(@{$one->pre_subtest_callbacks}, 2, "added another pre-subtest callback");

like(
    exception { $one->add_pre_subtest_callback({}) },
    qr/Pre-subtest callbacks must be coderefs/,
    "Pre-subtest callbacks must be coderefs"
);

if (CAN_REALLY_FORK) {
    my $one = $CLASS->new;
    my $pid = fork;
    die "Failed to fork!" unless defined $pid;
    unless($pid) { exit 0 }

    is(Test2::API::Instance::_ipc_wait, 0, "No errors");

    $pid = fork;
    die "Failed to fork!" unless defined $pid;
    unless($pid) { exit 255 }
    my @warnings;
    {
        local $SIG{__WARN__} = sub { push @warnings => @_ };
        is(Test2::API::Instance::_ipc_wait, 255, "Process exited badly");
    }
    like($warnings[0], qr/Process .* did not exit cleanly \(wstat: \S+, exit: 255, sig: 0\)/, "Warn about exit");

    $pid = fork;
    die "Failed to fork!" unless defined $pid;
    unless($pid) { sleep 20; exit 0 }
    kill('TERM', $pid) or die "Failed to send signal";
    @warnings = ();
    {
        local $SIG{__WARN__} = sub { push @warnings => @_ };
        is(Test2::API::Instance::_ipc_wait, 255, "Process exited badly");
    }
    like($warnings[0], qr/Process .* did not exit cleanly \(wstat: \S+, exit: 0, sig: 15\)/, "Warn about exit");
}

if (CAN_THREAD && $] ge '5.010') {
    require threads;
    my $one = $CLASS->new;

    threads->new(sub { 1 });
    is(Test2::API::Instance::_ipc_wait, 0, "No errors");

    if (threads->can('error')) {
        threads->new(sub {
            close(STDERR);
            close(STDOUT);
            die "xxx"
        });
        my @warnings;
        {
            local $SIG{__WARN__} = sub { push @warnings => @_ };
            is(Test2::API::Instance::_ipc_wait, 255, "Thread exited badly");
        }
        like($warnings[0], qr/Thread .* did not end cleanly: xxx/, "Warn about exit");
    }
}

{
    my $one = $CLASS->new;
    local $? = 0;
    $one->set_exit;
    is($?, 0, "no errors on exit");
}

{
    my $one = $CLASS->new;
    $one->set__tid(1);
    local $? = 0;
    $one->set_exit;
    is($?, 0, "no errors on exit");
}

{
    my $one = $CLASS->new;
    $one->stack->top;
    $one->no_wait(1);
    local $? = 0;
    $one->set_exit;
    is($?, 0, "no errors on exit");
}

{
    my $one = $CLASS->new;
    $one->stack->top->set_no_ending(1);
    local $? = 0;
    $one->set_exit;
    is($?, 0, "no errors on exit");
}

{
    my $one = $CLASS->new;
    $one->load();
    $one->stack->top->set_failed(2);
    local $? = 0;
    $one->set_exit;
    is($?, 2, "number of failures");
}

{
    my $one = $CLASS->new;
    $one->load();
    local $? = 500;
    $one->set_exit;
    is($?, 255, "set exit code to a sane number");
}

{
    local %INC = %INC;
    delete $INC{'Test2/IPC.pm'};
    my $one = $CLASS->new;
    $one->load();
    my @events;
    $one->stack->top->filter(sub { push @events => $_[1]; undef});
    $one->stack->new_hub;
    local $? = 0;
    $one->set_exit;
    is($?, 255, "errors on exit");
    like($events[0]->message, qr/Test ended with extra hubs on the stack!/, "got diag");
}

SKIP: {
    last SKIP if $] lt "5.008";
    my $one = $CLASS->new;
    my $stderr = "";
    {
        local $INC{'Test/Builder.pm'} = __FILE__;
        local $Test2::API::VERSION    = '0.002';
        local $Test::Builder::VERSION = '0.001';
        local *STDERR;
        open(STDERR, '>', \$stderr) or print "Failed to open new STDERR";

        $one->set_exit;
    }

    is($stderr, <<'    EOT', "Got warning about version mismatch");

********************************************************************************
*                                                                              *
*            Test::Builder -- Test2::API version mismatch detected             *
*                                                                              *
********************************************************************************
   Test2::API Version: 0.002
Test::Builder Version: 0.001

This is not a supported configuration, you will have problems.

    EOT
}

SKIP: {
    last SKIP if $] lt "5.008";
    require Test2::API::Breakage;
    no warnings qw/redefine once/;
    my $ran = 0;
    local *Test2::API::Breakage::report = sub { $ran++; return "foo" };
    use warnings qw/redefine once/;
    my $one = $CLASS->new;
    $one->load();

    my $stderr = "";
    {
        local *STDERR;
        open(STDERR, '>', \$stderr) or print "Failed to open new STDERR";
        local $? = 255;
        $one->set_exit;
    }

    is($stderr, <<"    EOT", "Reported bad modules");

You have loaded versions of test modules known to have problems with Test2.
This could explain some test failures.
foo

    EOT
}


{
    my $one = $CLASS->new;
    $one->load();
    my @events;
    $one->stack->top->filter(sub { push @events => $_[1]; undef});
    $one->stack->new_hub;
    ok($one->stack->top->ipc, "Have IPC");
    $one->stack->new_hub;
    ok($one->stack->top->ipc, "Have IPC");
    $one->stack->top->set_ipc(undef);
    ok(!$one->stack->top->ipc, "no IPC");
    $one->stack->new_hub;
    local $? = 0;
    $one->set_exit;
    is($?, 255, "errors on exit");
    like($events[0]->message, qr/Test ended with extra hubs on the stack!/, "got diag");
}

if (CAN_REALLY_FORK) {
    local $SIG{__WARN__} = sub { };
    my $one = $CLASS->new;
    my $pid = fork;
    die "Failed to fork!" unless defined $pid;
    unless ($pid) { exit 255 }
    $one->_finalize;
    $one->stack->top;

    local $? = 0;
    $one->set_exit;
    is($?, 255, "errors on exit");

    $one->reset();
    $pid = fork;
    die "Failed to fork!" unless defined $pid;
    unless ($pid) { exit 255 }
    $one->_finalize;
    $one->stack->top;

    local $? = 122;
    $one->set_exit;
    is($?, 122, "kept original exit");
}

{
    my $one = $CLASS->new;
    my $ctx = bless {
        trace => Test2::EventFacet::Trace->new(frame => ['Foo::Bar', 'Foo/Bar.pm', 42, 'xxx']),
        hub => Test2::Hub->new(),
    }, 'Test2::API::Context';
    $one->contexts->{1234} = $ctx;

    local $? = 500;
    my $warnings = warnings { $one->set_exit };
    is($?, 255, "set exit code to a sane number");

    is_deeply(
        $warnings,
        [
            "context object was never released! This means a testing tool is behaving very badly at Foo/Bar.pm line 42.\n"
        ],
        "Warned about unfreed context"
    );
    $one->set_no_wait(0);
}

{
    local %INC = %INC;
    delete $INC{'Test2/IPC.pm'};
    delete $INC{'threads.pm'};
    ok(!USE_THREADS, "Sanity Check");

    my $one = $CLASS->new;
    ok(!$one->ipc, 'IPC not loaded, no IPC object');
    ok($one->finalized, "calling ipc finalized the object");
    is($one->ipc_polling, undef, "no polling defined");
    ok(!@{$one->ipc_drivers}, "no driver");

    if (CAN_THREAD) {
        local $INC{'threads.pm'} = 1;
        no warnings 'once';
        local *threads::tid = sub { 0 } unless threads->can('tid');
        $one->reset;
        ok($one->ipc, 'IPC loaded if threads are');
        ok($one->finalized, "calling ipc finalized the object");
        ok($one->ipc_polling, "polling on by default");
        is($one->ipc_drivers->[0], 'Test2::IPC::Driver::Files', "default driver");
    }

    {
        local $INC{'Test2/IPC.pm'} = 1;
        $one->reset;
        ok($one->ipc, 'IPC loaded if Test2::IPC is');
        ok($one->finalized, "calling ipc finalized the object");
        ok($one->ipc_polling, "polling on by default");
        is($one->ipc_drivers->[0], 'Test2::IPC::Driver::Files', "default driver");
    }

    require Test2::IPC::Driver::Files;
    $one->reset;
    $one->add_ipc_driver('Test2::IPC::Driver::Files');
    ok($one->ipc, 'IPC loaded if drivers have been added');
    ok($one->finalized, "calling ipc finalized the object");
    ok($one->ipc_polling, "polling on by default");

    my $file = __FILE__;
    my $line = __LINE__ + 1;
    my $warnings = warnings { $one->add_ipc_driver('Test2::IPC::Driver::Files') };
    like(
        $warnings->[0],
        qr{^IPC driver Test2::IPC::Driver::Files loaded too late to be used as the global ipc driver at \Q$file\E line $line},
        "Got warning at correct frame"
    );

    $one->reset;
    $one->add_ipc_driver('Fake::Fake::XXX');
    is(
        exception { $one->ipc },
        "IPC has been requested, but no viable drivers were found. Aborting...\n",
        "Failed without viable IPC driver"
    );
}

{
    my $one = $CLASS->new;
    $one->{ipc} = Test2::IPC::Driver::Files->new;

    ok(!@{$one->context_init_callbacks}, "no callbacks");
    is($one->ipc_polling, undef, "no polling, undef");

    $one->disable_ipc_polling;
    ok(!@{$one->context_init_callbacks}, "no callbacks");
    is($one->ipc_polling, undef, "no polling, still undef");

    my $cull = 0;
    no warnings 'once';
    local *Fake::Hub::cull = sub { $cull++ };
    use warnings;

    $one->enable_ipc_polling;
    ok(defined($one->{_pid}), "pid is defined");
    ok(defined($one->{_tid}), "tid is defined");
    is(@{$one->context_init_callbacks}, 1, "added the callback");
    is($one->ipc_polling, 1, "polling on");
    $one->context_init_callbacks->[0]->({'hub' => 'Fake::Hub'});
    is($cull, 1, "called cull once");
    $cull = 0;

    $one->disable_ipc_polling;
    is(@{$one->context_init_callbacks}, 1, "kept the callback");
    is($one->ipc_polling, 0, "no polling, set to 0");
    $one->context_init_callbacks->[0]->({'hub' => 'Fake::Hub'});
    is($cull, 0, "did not call cull");
    $cull = 0;

    $one->enable_ipc_polling;
    is(@{$one->context_init_callbacks}, 1, "did not add the callback");
    is($one->ipc_polling, 1, "polling on");
    $one->context_init_callbacks->[0]->({'hub' => 'Fake::Hub'});
    is($cull, 1, "called cull once");
}

{
    require Test2::IPC::Driver::Files;

    local $ENV{T2_NO_IPC} = 1;
    my $one = $CLASS->new;
    $one->add_ipc_driver('Test2::IPC::Driver::Files');
    ok($one->ipc_disabled, "IPC is disabled by env var");
    ok(!$one->ipc, 'IPC not loaded');

    local $ENV{T2_NO_IPC} = 0;
    $one->reset;
    ok(!$one->ipc_disabled, "IPC is not disabled by env var");
    ok($one->ipc, 'IPC loaded');
    like(
        exception { $one->ipc_disable },
        qr/Attempt to disable IPC after it has been initialized/,
        "Cannot diable IPC once it is initialized"
    );

    $one->reset;
    ok(!$one->ipc_disabled, "IPC is not disabled by env var");
    $one->ipc_disable;
    ok($one->ipc_disabled, "IPC is disabled directly");
}

Test2::API::test2_ipc_wait_enable();

done_testing;
