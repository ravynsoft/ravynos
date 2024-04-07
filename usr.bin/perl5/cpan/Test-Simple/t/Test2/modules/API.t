use strict;
use warnings;

BEGIN { no warnings 'once'; $main::cleanup1 = bless {}, 'My::Cleanup' }

use Test2::API qw/context/;

my ($LOADED, $INIT);
BEGIN {
    $INIT   = Test2::API::test2_init_done;
    $LOADED = Test2::API::test2_load_done;
};

use Test2::IPC;
use Test2::Tools::Tiny;
use Test2::Util qw/get_tid/;
my $CLASS = 'Test2::API';

# Ensure we do not break backcompat later by removing anything
ok(Test2::API->can($_), "$_ method is present") for qw{
    context_do
    no_context

    test2_init_done
    test2_load_done

    test2_pid
    test2_tid
    test2_stack
    test2_no_wait
    test2_is_testing_done

    test2_add_callback_context_init
    test2_add_callback_context_release
    test2_add_callback_exit
    test2_add_callback_post_load
    test2_list_context_init_callbacks
    test2_list_context_release_callbacks
    test2_list_exit_callbacks
    test2_list_post_load_callbacks

    test2_ipc
    test2_ipc_disable
    test2_ipc_disabled
    test2_ipc_drivers
    test2_ipc_add_driver
    test2_ipc_polling
    test2_ipc_disable_polling
    test2_ipc_enable_polling

    test2_formatter
    test2_formatters
    test2_formatter_add
    test2_formatter_set
};

ok(!$LOADED, "Was not load_done right away");
ok(!$INIT, "Init was not done right away");
ok(Test2::API::test2_load_done, "We loaded it");

# Note: This is a check that stuff happens in an END block.
{
    {
        package FOLLOW;

        sub DESTROY {
            return if $_[0]->{fixed};
            print "not ok - Did not run end ($_[0]->{name})!";
            $? = 255;
            exit 255;
        }
    }

    our $kill1 = bless {fixed => 0, name => "Custom Hook"}, 'FOLLOW';
    Test2::API::test2_add_callback_exit(
        sub {
            print "# Running END hook\n";
            $kill1->{fixed} = 1;
        }
    );

    our $kill2 = bless {fixed => 0, name => "set exit"}, 'FOLLOW';
    my $old = Test2::API::Instance->can('set_exit');
    no warnings 'redefine';
    *Test2::API::Instance::set_exit = sub {
        $kill2->{fixed} = 1;
        print "# Running set_exit\n";
        $old->(@_);
    };
}

ok($CLASS->can('test2_init_done')->(), "init is done.");
ok($CLASS->can('test2_load_done')->(), "Test2 is finished loading");

is($CLASS->can('test2_pid')->(), $$, "got pid");
is($CLASS->can('test2_tid')->(), get_tid(), "got tid");

ok($CLASS->can('test2_stack')->(), 'got stack');
is($CLASS->can('test2_stack')->(), $CLASS->can('test2_stack')->(), "always get the same stack");

ok($CLASS->can('test2_ipc')->(), 'got ipc');
is($CLASS->can('test2_ipc')->(), $CLASS->can('test2_ipc')->(), "always get the same IPC");

is_deeply([$CLASS->can('test2_ipc_drivers')->()], [qw/Test2::IPC::Driver::Files/], "Got driver list");

# Verify it reports to the correct file/line, there was some trouble with this...
my $file = __FILE__;
my $line = __LINE__ + 1;
my $warnings = warnings { $CLASS->can('test2_ipc_add_driver')->('fake') };
my $sub1 = sub {
like(
    $warnings->[0],
    qr{^IPC driver fake loaded too late to be used as the global ipc driver at \Q$file\E line $line},
    "got warning about adding driver too late"
);
};
if ($] le "5.006002") {
    todo("TODO known to fail on $]", $sub1);
} else {
    $sub1->();
}

is_deeply([$CLASS->can('test2_ipc_drivers')->()], [qw/fake Test2::IPC::Driver::Files/], "Got updated list");

ok($CLASS->can('test2_ipc_polling')->(), "Polling is on");
$CLASS->can('test2_ipc_disable_polling')->();
ok(!$CLASS->can('test2_ipc_polling')->(), "Polling is off");
$CLASS->can('test2_ipc_enable_polling')->();
ok($CLASS->can('test2_ipc_polling')->(), "Polling is on");

ok($CLASS->can('test2_formatter')->(), "Got a formatter");
is($CLASS->can('test2_formatter')->(), $CLASS->can('test2_formatter')->(), "always get the same Formatter (class name)");

my $ran = 0;
$CLASS->can('test2_add_callback_post_load')->(sub { $ran++ });
is($ran, 1, "ran the post-load");

like(
    exception { $CLASS->can('test2_formatter_set')->() },
    qr/No formatter specified/,
    "formatter_set requires an argument"
);

like(
    exception { $CLASS->can('test2_formatter_set')->('fake') },
    qr/Global Formatter already set/,
    "formatter_set doesn't work after initialization",
);

ok(!$CLASS->can('test2_no_wait')->(), "no_wait is not set");
$CLASS->can('test2_no_wait')->(1);
ok($CLASS->can('test2_no_wait')->(), "no_wait is set");
$CLASS->can('test2_no_wait')->(undef);
ok(!$CLASS->can('test2_no_wait')->(), "no_wait is not set");

ok($CLASS->can('test2_ipc_wait_enabled')->(), "IPC waiting enabled");
$CLASS->can('test2_ipc_wait_disable')->();
ok(!$CLASS->can('test2_ipc_wait_enabled')->(), "IPC waiting disabled");
$CLASS->can('test2_ipc_wait_enable')->();
ok($CLASS->can('test2_ipc_wait_enabled')->(), "IPC waiting enabled");

my $pctx;
sub tool_a($;$) {
    Test2::API::context_do {
        my $ctx = shift;
        my ($bool, $name) = @_;
        $pctx = wantarray;
        die "xyz" unless $bool;
        $ctx->ok($bool, $name);
        return unless defined $pctx;
        return (1, 2) if $pctx;
        return 'a';
    } @_;
}

$pctx = 'x';
tool_a(1, "void context test");
ok(!defined($pctx), "void context");

my $x = tool_a(1, "scalar context test");
ok(defined($pctx) && $pctx == 0, "scalar context");
is($x, 'a', "got scalar return");

my @x = tool_a(1, "array context test");
ok($pctx, "array context");
is_deeply(\@x, [1, 2], "Got array return");

like(
    exception { tool_a(0) },
    qr/^xyz/,
    "got exception"
);

sub {
    my $outer = context();
    sub {
        my $middle = context();
        is($outer->trace, $middle->trace, "got the same context before calling no_context");

        Test2::API::no_context {
            my $inner = context();
            ok($inner->trace != $outer->trace, "Got a different context inside of no_context()");
            $inner->release;
        };

        $middle->release;
    }->();

    $outer->release;
}->();

sub {
    my $outer = context();
    sub {
        my $middle = context();
        is($outer->trace, $middle->trace, "got the same context before calling no_context");

        Test2::API::no_context {
            my $inner = context();
            ok($inner->trace != $outer->trace, "Got a different context inside of no_context({}, hid)");
            $inner->release;
        } $outer->hub->hid;

        $middle->release;
    }->();

    $outer->release;
}->();

sub {
    my @warnings;
    my $outer = context();
    sub {
        my $middle = context();
        is($outer->trace, $middle->trace, "got the same context before calling no_context");

        local $SIG{__WARN__} = sub { push @warnings => @_ };
        Test2::API::no_context {
            my $inner = context();
            ok($inner->trace != $outer->trace, "Got a different context inside of no_context({}, hid)");
        } $outer->hub->hid;

        $middle->release;
    }->();

    $outer->release;

    is(@warnings, 1, "1 warning");
    like(
        $warnings[0],
        qr/A context appears to have been destroyed without first calling release/,
        "Got warning about unreleased context"
    );
}->();


sub {
    my $hub = Test2::Hub->new();
    my $ctx = context(hub => $hub);
    is($ctx->hub,$hub, 'got the hub of context() argument');
    $ctx->release;
}->();


my $sub = sub { };

Test2::API::test2_add_callback_context_acquire($sub);
Test2::API::test2_add_callback_context_init($sub);
Test2::API::test2_add_callback_context_release($sub);
Test2::API::test2_add_callback_exit($sub);
Test2::API::test2_add_callback_post_load($sub);

is((grep { $_ == $sub } Test2::API::test2_list_context_acquire_callbacks()), 1, "got the one instance of the hook");
is((grep { $_ == $sub } Test2::API::test2_list_context_init_callbacks()),    1, "got the one instance of the hook");
is((grep { $_ == $sub } Test2::API::test2_list_context_release_callbacks()), 1, "got the one instance of the hook");
is((grep { $_ == $sub } Test2::API::test2_list_exit_callbacks()),            1, "got the one instance of the hook");
is((grep { $_ == $sub } Test2::API::test2_list_post_load_callbacks()),       1, "got the one instance of the hook");

Test2::API::test2_add_callback_context_acquire($sub);
Test2::API::test2_add_callback_context_init($sub);
Test2::API::test2_add_callback_context_release($sub);
Test2::API::test2_add_callback_exit($sub);
Test2::API::test2_add_callback_post_load($sub);

is((grep { $_ == $sub } Test2::API::test2_list_context_acquire_callbacks()), 2, "got the two instances of the hook");
is((grep { $_ == $sub } Test2::API::test2_list_context_init_callbacks()),    2, "got the two instances of the hook");
is((grep { $_ == $sub } Test2::API::test2_list_context_release_callbacks()), 2, "got the two instances of the hook");
is((grep { $_ == $sub } Test2::API::test2_list_exit_callbacks()),            2, "got the two instances of the hook");
is((grep { $_ == $sub } Test2::API::test2_list_post_load_callbacks()),       2, "got the two instances of the hook");

ok(!Test2::API::test2_is_testing_done(), "Testing is not done");

done_testing;

die "Testing should be done, but it is not!" unless Test2::API::test2_is_testing_done();

{
    package My::Cleanup;

    sub DESTROY {
        return if Test2::API::test2_is_testing_done();
        print "not ok - Testing should be done, but it is not!\n";
        warn "Testing should be done, but it is not!";
        eval "END { $? = 255 }; 1" or die $@;
        exit 255;
    }
}

# This should destroy the thing
END { no warnings 'once'; $main::cleanup2 = bless {}, 'My::Cleanup' }
