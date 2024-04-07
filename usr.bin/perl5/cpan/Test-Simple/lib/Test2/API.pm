package Test2::API;
use strict;
use warnings;

use Time::HiRes qw/time/;
use Test2::Util qw/USE_THREADS/;

BEGIN {
    $ENV{TEST_ACTIVE} ||= 1;
    $ENV{TEST2_ACTIVE} = 1;
}

our $VERSION = '1.302194';


my $INST;
my $ENDING = 0;
sub test2_unset_is_end { $ENDING = 0 }
sub test2_get_is_end { $ENDING }

sub test2_set_is_end {
    my $before = $ENDING;
    ($ENDING) = @_ ? @_ : (1);

    # Only send the event in a transition from false to true
    return if $before;
    return unless $ENDING;

    return unless $INST;
    my $stack = $INST->stack or return;
    my $root = $stack->root or return;

    return unless $root->count;

    return unless $$ == $INST->pid;
    return unless get_tid() == $INST->tid;

    my $trace = Test2::EventFacet::Trace->new(
        frame  => [__PACKAGE__, __FILE__, __LINE__, __PACKAGE__ . '::test2_set_is_end'],
    );
    my $ctx = Test2::API::Context->new(
        trace => $trace,
        hub   => $root,
    );

    $ctx->send_ev2(control => { phase => 'END', details => 'Transition to END phase' });

    1;
}

use Test2::API::Instance(\$INST);

# Set the exit status
END {
    test2_set_is_end(); # See gh #16
    $INST->set_exit();
}

sub CLONE {
    my $init = test2_init_done();
    my $load = test2_load_done();

    return if $init && $load;

    require Carp;
    Carp::croak "Test2 must be fully loaded before you start a new thread!\n";
}

# See gh #16
{
    no warnings;
    INIT { eval 'END { test2_set_is_end() }; 1' or die $@ }
}

BEGIN {
    no warnings 'once';
    if($] ge '5.014' || $ENV{T2_CHECK_DEPTH} || $Test2::API::DO_DEPTH_CHECK) {
        *DO_DEPTH_CHECK = sub() { 1 };
    }
    else {
        *DO_DEPTH_CHECK = sub() { 0 };
    }
}

use Test2::EventFacet::Trace();
use Test2::Util::Trace(); # Legacy

use Test2::Hub::Subtest();
use Test2::Hub::Interceptor();
use Test2::Hub::Interceptor::Terminator();

use Test2::Event::Ok();
use Test2::Event::Diag();
use Test2::Event::Note();
use Test2::Event::Plan();
use Test2::Event::Bail();
use Test2::Event::Exception();
use Test2::Event::Waiting();
use Test2::Event::Skip();
use Test2::Event::Subtest();

use Carp qw/carp croak confess/;
use Scalar::Util qw/blessed weaken/;
use Test2::Util qw/get_tid clone_io pkg_to_file gen_uid/;

our @EXPORT_OK = qw{
    context release
    context_do
    no_context
    intercept intercept_deep
    run_subtest

    test2_init_done
    test2_load_done
    test2_load
    test2_start_preload
    test2_stop_preload
    test2_in_preload
    test2_is_testing_done

    test2_set_is_end
    test2_unset_is_end
    test2_get_is_end

    test2_pid
    test2_tid
    test2_stack
    test2_no_wait
    test2_ipc_wait_enable
    test2_ipc_wait_disable
    test2_ipc_wait_enabled

    test2_add_uuid_via

    test2_add_callback_testing_done

    test2_add_callback_context_aquire
    test2_add_callback_context_acquire
    test2_add_callback_context_init
    test2_add_callback_context_release
    test2_add_callback_exit
    test2_add_callback_post_load
    test2_add_callback_pre_subtest
    test2_list_context_aquire_callbacks
    test2_list_context_acquire_callbacks
    test2_list_context_init_callbacks
    test2_list_context_release_callbacks
    test2_list_exit_callbacks
    test2_list_post_load_callbacks
    test2_list_pre_subtest_callbacks

    test2_ipc
    test2_has_ipc
    test2_ipc_disable
    test2_ipc_disabled
    test2_ipc_drivers
    test2_ipc_add_driver
    test2_ipc_polling
    test2_ipc_disable_polling
    test2_ipc_enable_polling
    test2_ipc_get_pending
    test2_ipc_set_pending
    test2_ipc_get_timeout
    test2_ipc_set_timeout

    test2_formatter
    test2_formatters
    test2_formatter_add
    test2_formatter_set

    test2_stdout
    test2_stderr
    test2_reset_io
};
BEGIN { require Exporter; our @ISA = qw(Exporter) }

my $STACK       = $INST->stack;
my $CONTEXTS    = $INST->contexts;
my $INIT_CBS    = $INST->context_init_callbacks;
my $ACQUIRE_CBS = $INST->context_acquire_callbacks;

my $STDOUT = clone_io(\*STDOUT);
my $STDERR = clone_io(\*STDERR);
sub test2_stdout { $STDOUT ||= clone_io(\*STDOUT) }
sub test2_stderr { $STDERR ||= clone_io(\*STDERR) }

sub test2_post_preload_reset {
    test2_reset_io();
    $INST->post_preload_reset;
}

sub test2_reset_io {
    $STDOUT = clone_io(\*STDOUT);
    $STDERR = clone_io(\*STDERR);
}

sub test2_init_done { $INST->finalized }
sub test2_load_done { $INST->loaded }

sub test2_load          { $INST->load }
sub test2_start_preload { $ENV{T2_IN_PRELOAD} = 1; $INST->start_preload }
sub test2_stop_preload  { $ENV{T2_IN_PRELOAD} = 0; $INST->stop_preload }
sub test2_in_preload    { $INST->preload }

sub test2_pid              { $INST->pid }
sub test2_tid              { $INST->tid }
sub test2_stack            { $INST->stack }
sub test2_ipc_wait_enable  { $INST->set_no_wait(0) }
sub test2_ipc_wait_disable { $INST->set_no_wait(1) }
sub test2_ipc_wait_enabled { !$INST->no_wait }

sub test2_is_testing_done {
    # No instance? VERY DONE!
    return 1 unless $INST;

    # No stack? tests must be done, it is created pretty early
    my $stack = $INST->stack or return 1;

    # Nothing on the stack, no root hub yet, likely have not started testing
    return 0 unless @$stack;

    # Stack has a slot for the root hub (see above) but it is undefined, likely
    # garbage collected, test is done
    my $root_hub = $stack->[0] or return 1;

    # If the root hub is ended than testing is done.
    return 1 if $root_hub->ended;

    # Looks like we are still testing!
    return 0;
}

sub test2_no_wait {
    $INST->set_no_wait(@_) if @_;
    $INST->no_wait;
}

sub test2_add_callback_testing_done {
    my $cb = shift;

    test2_add_callback_post_load(sub {
        my $stack = test2_stack();
        $stack->top; # Insure we have a hub
        my ($hub) = Test2::API::test2_stack->all;

        $hub->set_active(1);

        $hub->follow_up($cb);
    });

    return;
}

sub test2_add_callback_context_acquire   { $INST->add_context_acquire_callback(@_) }
sub test2_add_callback_context_aquire    { $INST->add_context_acquire_callback(@_) }
sub test2_add_callback_context_init      { $INST->add_context_init_callback(@_) }
sub test2_add_callback_context_release   { $INST->add_context_release_callback(@_) }
sub test2_add_callback_exit              { $INST->add_exit_callback(@_) }
sub test2_add_callback_post_load         { $INST->add_post_load_callback(@_) }
sub test2_add_callback_pre_subtest       { $INST->add_pre_subtest_callback(@_) }
sub test2_list_context_aquire_callbacks  { @{$INST->context_acquire_callbacks} }
sub test2_list_context_acquire_callbacks { @{$INST->context_acquire_callbacks} }
sub test2_list_context_init_callbacks    { @{$INST->context_init_callbacks} }
sub test2_list_context_release_callbacks { @{$INST->context_release_callbacks} }
sub test2_list_exit_callbacks            { @{$INST->exit_callbacks} }
sub test2_list_post_load_callbacks       { @{$INST->post_load_callbacks} }
sub test2_list_pre_subtest_callbacks     { @{$INST->pre_subtest_callbacks} }

sub test2_add_uuid_via {
    $INST->set_add_uuid_via(@_) if @_;
    $INST->add_uuid_via();
}

sub test2_ipc                 { $INST->ipc }
sub test2_has_ipc             { $INST->has_ipc }
sub test2_ipc_disable         { $INST->ipc_disable }
sub test2_ipc_disabled        { $INST->ipc_disabled }
sub test2_ipc_add_driver      { $INST->add_ipc_driver(@_) }
sub test2_ipc_drivers         { @{$INST->ipc_drivers} }
sub test2_ipc_polling         { $INST->ipc_polling }
sub test2_ipc_enable_polling  { $INST->enable_ipc_polling }
sub test2_ipc_disable_polling { $INST->disable_ipc_polling }
sub test2_ipc_get_pending     { $INST->get_ipc_pending }
sub test2_ipc_set_pending     { $INST->set_ipc_pending(@_) }
sub test2_ipc_set_timeout     { $INST->set_ipc_timeout(@_) }
sub test2_ipc_get_timeout     { $INST->ipc_timeout() }
sub test2_ipc_enable_shm      { 0 }

sub test2_formatter     {
    if ($ENV{T2_FORMATTER} && $ENV{T2_FORMATTER} =~ m/^(\+)?(.*)$/) {
        my $formatter = $1 ? $2 : "Test2::Formatter::$2";
        my $file = pkg_to_file($formatter);
        require $file;
        return $formatter;
    }

    return $INST->formatter;
}

sub test2_formatters    { @{$INST->formatters} }
sub test2_formatter_add { $INST->add_formatter(@_) }
sub test2_formatter_set {
    my ($formatter) = @_;
    croak "No formatter specified" unless $formatter;
    croak "Global Formatter already set" if $INST->formatter_set;
    $INST->set_formatter($formatter);
}

# Private, for use in Test2::API::Context
sub _contexts_ref                  { $INST->contexts }
sub _context_acquire_callbacks_ref { $INST->context_acquire_callbacks }
sub _context_init_callbacks_ref    { $INST->context_init_callbacks }
sub _context_release_callbacks_ref { $INST->context_release_callbacks }
sub _add_uuid_via_ref              { \($INST->{Test2::API::Instance::ADD_UUID_VIA()}) }

# Private, for use in Test2::IPC
sub _set_ipc { $INST->set_ipc(@_) }

sub context_do(&;@) {
    my $code = shift;
    my @args = @_;

    my $ctx = context(level => 1);

    my $want = wantarray;

    my @out;
    my $ok = eval {
        $want          ? @out    = $code->($ctx, @args) :
        defined($want) ? $out[0] = $code->($ctx, @args) :
                                   $code->($ctx, @args) ;
        1;
    };
    my $err = $@;

    $ctx->release;

    die $err unless $ok;

    return @out    if $want;
    return $out[0] if defined $want;
    return;
}

sub no_context(&;$) {
    my ($code, $hid) = @_;
    $hid ||= $STACK->top->hid;

    my $ctx = $CONTEXTS->{$hid};
    delete $CONTEXTS->{$hid};
    my $ok = eval { $code->(); 1 };
    my $err = $@;

    $CONTEXTS->{$hid} = $ctx;
    weaken($CONTEXTS->{$hid});

    die $err unless $ok;

    return;
};

my $UUID_VIA = _add_uuid_via_ref();
sub context {
    # We need to grab these before anything else to ensure they are not
    # changed.
    my ($errno, $eval_error, $child_error, $extended_error) = (0 + $!, $@, $?, $^E);

    my %params = (level => 0, wrapped => 0, @_);

    # If something is getting a context then the sync system needs to be
    # considered loaded...
    $INST->load unless $INST->{loaded};

    croak "context() called, but return value is ignored"
        unless defined wantarray;

    my $stack   = $params{stack} || $STACK;
    my $hub     = $params{hub}   || (@$stack ? $stack->[-1] : $stack->top);

    # Catch an edge case where we try to get context after the root hub has
    # been garbage collected resulting in a stack that has a single undef
    # hub
    if (!$hub && !exists($params{hub}) && @$stack) {
        my $msg = Carp::longmess("Attempt to get Test2 context after testing has completed (did you attempt a testing event after done_testing?)");

        # The error message is usually masked by the global destruction, so we have to print to STDER
        print STDERR $msg;

        # Make sure this is a failure, we are probably already in END, so set $? to change the exit code
        $? = 1;

        # Now we actually die to interrupt the program flow and avoid undefined his warnings
        die $msg;
    }

    my $hid     = $hub->{hid};
    my $current = $CONTEXTS->{$hid};

    $_->(\%params) for @$ACQUIRE_CBS;
    map $_->(\%params), @{$hub->{_context_acquire}} if $hub->{_context_acquire};

    # This is for https://github.com/Test-More/test-more/issues/16
    # and https://rt.perl.org/Public/Bug/Display.html?id=127774
    my $phase = ${^GLOBAL_PHASE} || 'NA';
    my $end_phase = $ENDING || $phase eq 'END' || $phase eq 'DESTRUCT';

    my $level = 1 + $params{level};
    my ($pkg, $file, $line, $sub, @other) = $end_phase ? caller(0) : caller($level);
    unless ($pkg || $end_phase) {
        confess "Could not find context at depth $level" unless $params{fudge};
        ($pkg, $file, $line, $sub, @other) = caller(--$level) while ($level >= 0 && !$pkg);
    }

    my $depth = $level;
    $depth++ while DO_DEPTH_CHECK && !$end_phase && (!$current || $depth <= $current->{_depth} + $params{wrapped}) && caller($depth + 1);
    $depth -= $params{wrapped};
    my $depth_ok = !DO_DEPTH_CHECK || $end_phase || !$current || $current->{_depth} < $depth;

    if ($current && $params{on_release} && $depth_ok) {
        $current->{_on_release} ||= [];
        push @{$current->{_on_release}} => $params{on_release};
    }

    # I know this is ugly....
    ($!, $@, $?, $^E) = ($errno, $eval_error, $child_error, $extended_error) and return bless(
        {
            %$current,
            _is_canon   => undef,
            errno       => $errno,
            eval_error  => $eval_error,
            child_error => $child_error,
            _is_spawn   => [$pkg, $file, $line, $sub],
        },
        'Test2::API::Context'
    ) if $current && $depth_ok;

    # Handle error condition of bad level
    if ($current) {
        unless (${$current->{_aborted}}) {
            _canon_error($current, [$pkg, $file, $line, $sub, $depth])
                unless $current->{_is_canon};

            _depth_error($current, [$pkg, $file, $line, $sub, $depth])
                unless $depth_ok;
        }

        $current->release if $current->{_is_canon};

        delete $CONTEXTS->{$hid};
    }

    # Directly bless the object here, calling new is a noticeable performance
    # hit with how often this needs to be called.
    my $trace = bless(
        {
            frame  => [$pkg, $file, $line, $sub],
            pid    => $$,
            tid    => get_tid(),
            cid    => gen_uid(),
            hid    => $hid,
            nested => $hub->{nested},
            buffered => $hub->{buffered},

            full_caller => [$pkg, $file, $line, $sub, @other],

            $$UUID_VIA ? (
                huuid => $hub->{uuid},
                uuid  => ${$UUID_VIA}->('context'),
            ) : (),
        },
        'Test2::EventFacet::Trace'
    );

    # Directly bless the object here, calling new is a noticeable performance
    # hit with how often this needs to be called.
    my $aborted = 0;
    $current = bless(
        {
            _aborted     => \$aborted,
            stack        => $stack,
            hub          => $hub,
            trace        => $trace,
            _is_canon    => 1,
            _depth       => $depth,
            errno        => $errno,
            eval_error   => $eval_error,
            child_error  => $child_error,
            $params{on_release} ? (_on_release => [$params{on_release}]) : (),
        },
        'Test2::API::Context'
    );

    $CONTEXTS->{$hid} = $current;
    weaken($CONTEXTS->{$hid});

    $_->($current) for @$INIT_CBS;
    map $_->($current), @{$hub->{_context_init}} if $hub->{_context_init};

    $params{on_init}->($current) if $params{on_init};

    ($!, $@, $?, $^E) = ($errno, $eval_error, $child_error, $extended_error);

    return $current;
}

sub _depth_error {
    _existing_error(@_, <<"    EOT");
context() was called to retrieve an existing context, however the existing
context was created in a stack frame at the same, or deeper level. This usually
means that a tool failed to release the context when it was finished.
    EOT
}

sub _canon_error {
    _existing_error(@_, <<"    EOT");
context() was called to retrieve an existing context, however the existing
context has an invalid internal state (!_canon_count). This should not normally
happen unless something is mucking about with internals...
    EOT
}

sub _existing_error {
    my ($ctx, $details, $msg) = @_;
    my ($pkg, $file, $line, $sub, $depth) = @$details;

    my $oldframe = $ctx->{trace}->frame;
    my $olddepth = $ctx->{_depth};

    # Older versions of Carp do not export longmess() function, so it needs to be called with package name
    my $mess = Carp::longmess();

    warn <<"    EOT";
$msg
Old context details:
   File: $oldframe->[1]
   Line: $oldframe->[2]
   Tool: $oldframe->[3]
  Depth: $olddepth

New context details:
   File: $file
   Line: $line
   Tool: $sub
  Depth: $depth

Trace: $mess

Removing the old context and creating a new one...
    EOT
}

sub release($;$) {
    $_[0]->release;
    return $_[1];
}

sub intercept(&) {
    my $code = shift;
    my $ctx = context();

    my $events = _intercept($code, deep => 0);

    $ctx->release;

    return $events;
}

sub intercept_deep(&) {
    my $code = shift;
    my $ctx = context();

    my $events = _intercept($code, deep => 1);

    $ctx->release;

    return $events;
}

sub _intercept {
    my $code = shift;
    my %params = @_;
    my $ctx = context();

    my $ipc;
    if (my $global_ipc = test2_ipc()) {
        my $driver = blessed($global_ipc);
        $ipc = $driver->new;
    }

    my $hub = Test2::Hub::Interceptor->new(
        ipc => $ipc,
        no_ending => 1,
    );

    my @events;
    $hub->listen(sub { push @events => $_[1] }, inherit => $params{deep});

    $ctx->stack->top; # Make sure there is a top hub before we begin.
    $ctx->stack->push($hub);

    my $trace = $ctx->trace;
    my $state = {};
    $hub->clean_inherited(trace => $trace, state => $state);

    my ($ok, $err) = (1, undef);
    T2_SUBTEST_WRAPPER: {
        # Do not use 'try' cause it localizes __DIE__
        $ok = eval { $code->(hub => $hub, context => $ctx->snapshot); 1 };
        $err = $@;

        # They might have done 'BEGIN { skip_all => "whatever" }'
        if (!$ok && $err =~ m/Label not found for "last T2_SUBTEST_WRAPPER"/ || (blessed($err) && $err->isa('Test2::Hub::Interceptor::Terminator'))) {
            $ok  = 1;
            $err = undef;
        }
    }

    $hub->cull;
    $ctx->stack->pop($hub);

    $hub->restore_inherited(trace => $trace, state => $state);

    $ctx->release;

    die $err unless $ok;

    $hub->finalize($trace, 1)
        if $ok
        && !$hub->no_ending
        && !$hub->ended;

    require Test2::API::InterceptResult;
    return Test2::API::InterceptResult->new_from_ref(\@events);
}

sub run_subtest {
    my ($name, $code, $params, @args) = @_;

    $_->($name,$code,@args)
        for Test2::API::test2_list_pre_subtest_callbacks();

    $params = {buffered => $params} unless ref $params;
    my $inherit_trace = delete $params->{inherit_trace};

    my $ctx = context();

    my $parent = $ctx->hub;

    # If a parent is buffered then the child must be as well.
    my $buffered = $params->{buffered} || $parent->{buffered};

    $ctx->note($name) unless $buffered;

    my $stack = $ctx->stack || $STACK;
    my $hub = $stack->new_hub(
        class => 'Test2::Hub::Subtest',
        %$params,
        buffered => $buffered,
    );

    my @events;
    $hub->listen(sub { push @events => $_[1] });

    if ($buffered) {
        if (my $format = $hub->format) {
            my $hide = $format->can('hide_buffered') ? $format->hide_buffered : 1;
            $hub->format(undef) if $hide;
        }
    }

    if ($inherit_trace) {
        my $orig = $code;
        $code = sub {
            my $base_trace = $ctx->trace;
            my $trace = $base_trace->snapshot(nested => 1 + $base_trace->nested);
            my $st_ctx = Test2::API::Context->new(
                trace  => $trace,
                hub    => $hub,
            );
            $st_ctx->do_in_context($orig, @args);
        };
    }

    my $start_stamp = time;

    my ($ok, $err, $finished);
    T2_SUBTEST_WRAPPER: {
        # Do not use 'try' cause it localizes __DIE__
        $ok = eval { $code->(@args); 1 };
        $err = $@;

        # They might have done 'BEGIN { skip_all => "whatever" }'
        if (!$ok && $err =~ m/Label not found for "last T2_SUBTEST_WRAPPER"/ || (blessed($err) && blessed($err) eq 'Test::Builder::Exception')) {
            $ok  = undef;
            $err = undef;
        }
        else {
            $finished = 1;
        }
    }

    my $stop_stamp = time;

    if ($params->{no_fork}) {
        if ($$ != $ctx->trace->pid) {
            warn $ok ? "Forked inside subtest, but subtest never finished!\n" : $err;
            exit 255;
        }

        if (get_tid() != $ctx->trace->tid) {
            warn $ok ? "Started new thread inside subtest, but thread never finished!\n" : $err;
            exit 255;
        }
    }
    elsif (!$parent->is_local && !$parent->ipc) {
        warn $ok ? "A new process or thread was started inside subtest, but IPC is not enabled!\n" : $err;
        exit 255;
    }

    $stack->pop($hub);

    my $trace = $ctx->trace;

    my $bailed = $hub->bailed_out;

    if (!$finished) {
        if ($bailed && !$buffered) {
            $ctx->bail($bailed->reason);
        }
        elsif ($bailed && $buffered) {
            $ok = 1;
        }
        else {
            my $code = $hub->exit_code;
            $ok = !$code;
            $err = "Subtest ended with exit code $code" if $code;
        }
    }

    $hub->finalize($trace->snapshot(huuid => $hub->uuid, hid => $hub->hid, nested => $hub->nested, buffered => $buffered), 1)
        if $ok
        && !$hub->no_ending
        && !$hub->ended;

    my $pass = $ok && $hub->is_passing;
    my $e = $ctx->build_event(
        'Subtest',
        pass         => $pass,
        name         => $name,
        subtest_id   => $hub->id,
        subtest_uuid => $hub->uuid,
        buffered     => $buffered,
        subevents    => \@events,
        start_stamp  => $start_stamp,
        stop_stamp   => $stop_stamp,
    );

    my $plan_ok = $hub->check_plan;

    $ctx->hub->send($e);

    $ctx->failure_diag($e) unless $e->pass;

    $ctx->diag("Caught exception in subtest: $err") unless $ok;

    $ctx->diag("Bad subtest plan, expected " . $hub->plan . " but ran " . $hub->count)
        if defined($plan_ok) && !$plan_ok;

    $ctx->bail($bailed->reason) if $bailed && $buffered;

    $ctx->release;
    return $pass;
}

# There is a use-cycle between API and API/Context. Context needs to use some
# API functions as the package is compiling. Test2::API::context() needs
# Test2::API::Context to be loaded, but we cannot 'require' the module there as
# it causes a very noticeable performance impact with how often context() is
# called.
require Test2::API::Context;

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::API - Primary interface for writing Test2 based testing tools.

=head1 ***INTERNALS NOTE***

B<The internals of this package are subject to change at any time!> The public
methods provided will not change in backwards-incompatible ways (once there is
a stable release), but the underlying implementation details might.
B<Do not break encapsulation here!>

Currently the implementation is to create a single instance of the
L<Test2::API::Instance> Object. All class methods defer to the single
instance. There is no public access to the singleton, and that is intentional.
The class methods provided by this package provide the only functionality
publicly exposed.

This is done primarily to avoid the problems Test::Builder had by exposing its
singleton. We do not want anyone to replace this singleton, rebless it, or
directly muck with its internals. If you need to do something and cannot
because of the restrictions placed here, then please report it as an issue. If
possible, we will create a way for you to implement your functionality without
exposing things that should not be exposed.

=head1 DESCRIPTION

This package exports all the functions necessary to write and/or verify testing
tools. Using these building blocks you can begin writing test tools very
quickly. You are also provided with tools that help you to test the tools you
write.

=head1 SYNOPSIS

=head2 WRITING A TOOL

The C<context()> method is your primary interface into the Test2 framework.

    package My::Ok;
    use Test2::API qw/context/;

    our @EXPORT = qw/my_ok/;
    use base 'Exporter';

    # Just like ok() from Test::More
    sub my_ok($;$) {
        my ($bool, $name) = @_;
        my $ctx = context(); # Get a context
        $ctx->ok($bool, $name);
        $ctx->release; # Release the context
        return $bool;
    }

See L<Test2::API::Context> for a list of methods available on the context object.

=head2 TESTING YOUR TOOLS

The C<intercept { ... }> tool lets you temporarily intercept all events
generated by the test system:

    use Test2::API qw/intercept/;

    use My::Ok qw/my_ok/;

    my $events = intercept {
        # These events are not displayed
        my_ok(1, "pass");
        my_ok(0, "fail");
    };

As of version 1.302178 this now returns an arrayref that is also an instance of
L<Test2::API::InterceptResult>. See the L<Test2::API::InterceptResult>
documentation for details on how to best use it.

=head2 OTHER API FUNCTIONS

    use Test2::API qw{
        test2_init_done
        test2_stack
        test2_set_is_end
        test2_get_is_end
        test2_ipc
        test2_formatter_set
        test2_formatter
        test2_is_testing_done
    };

    my $init  = test2_init_done();
    my $stack = test2_stack();
    my $ipc   = test2_ipc();

    test2_formatter_set($FORMATTER)
    my $formatter = test2_formatter();

    ... And others ...

=head1 MAIN API EXPORTS

All exports are optional. You must specify subs to import.

    use Test2::API qw/context intercept run_subtest/;

This is the list of exports that are most commonly needed. If you are simply
writing a tool, then this is probably all you need. If you need something and
you cannot find it here, then you can also look at L</OTHER API EXPORTS>.

These exports lack the 'test2_' prefix because of how important/common they
are. Exports in the L</OTHER API EXPORTS> section have the 'test2_' prefix to
ensure they stand out.

=head2 context(...)

Usage:

=over 4

=item $ctx = context()

=item $ctx = context(%params)

=back

The C<context()> function will always return the current context. If
there is already a context active, it will be returned. If there is not an
active context, one will be generated. When a context is generated it will
default to using the file and line number where the currently running sub was
called from.

Please see L<Test2::API::Context/"CRITICAL DETAILS"> for important rules about
what you can and cannot do with a context once it is obtained.

B<Note> This function will throw an exception if you ignore the context object
it returns.

B<Note> On perls 5.14+ a depth check is used to insure there are no context
leaks. This cannot be safely done on older perls due to
L<https://rt.perl.org/Public/Bug/Display.html?id=127774>
You can forcefully enable it either by setting C<$ENV{T2_CHECK_DEPTH} = 1> or
C<$Test2::API::DO_DEPTH_CHECK = 1> B<BEFORE> loading L<Test2::API>.

=head3 OPTIONAL PARAMETERS

All parameters to C<context> are optional.

=over 4

=item level => $int

If you must obtain a context in a sub deeper than your entry point you can use
this to tell it how many EXTRA stack frames to look back. If this option is not
provided the default of C<0> is used.

    sub third_party_tool {
        my $sub = shift;
        ... # Does not obtain a context
        $sub->();
        ...
    }

    third_party_tool(sub {
        my $ctx = context(level => 1);
        ...
        $ctx->release;
    });

=item wrapped => $int

Use this if you need to write your own tool that wraps a call to C<context()>
with the intent that it should return a context object.

    sub my_context {
        my %params = ( wrapped => 0, @_ );
        $params{wrapped}++;
        my $ctx = context(%params);
        ...
        return $ctx;
    }

    sub my_tool {
        my $ctx = my_context();
        ...
        $ctx->release;
    }

If you do not do this, then tools you call that also check for a context will
notice that the context they grabbed was created at the same stack depth, which
will trigger protective measures that warn you and destroy the existing
context.

=item stack => $stack

Normally C<context()> looks at the global hub stack. If you are maintaining
your own L<Test2::API::Stack> instance you may pass it in to be used
instead of the global one.

=item hub => $hub

Use this parameter if you want to obtain the context for a specific hub instead
of whatever one happens to be at the top of the stack.

=item on_init => sub { ... }

This lets you provide a callback sub that will be called B<ONLY> if your call
to C<context()> generated a new context. The callback B<WILL NOT> be called if
C<context()> is returning an existing context. The only argument passed into
the callback will be the context object itself.

    sub foo {
        my $ctx = context(on_init => sub { 'will run' });

        my $inner = sub {
            # This callback is not run since we are getting the existing
            # context from our parent sub.
            my $ctx = context(on_init => sub { 'will NOT run' });
            $ctx->release;
        }
        $inner->();

        $ctx->release;
    }

=item on_release => sub { ... }

This lets you provide a callback sub that will be called when the context
instance is released. This callback will be added to the returned context even
if an existing context is returned. If multiple calls to context add callbacks,
then all will be called in reverse order when the context is finally released.

    sub foo {
        my $ctx = context(on_release => sub { 'will run second' });

        my $inner = sub {
            my $ctx = context(on_release => sub { 'will run first' });

            # Neither callback runs on this release
            $ctx->release;
        }
        $inner->();

        # Both callbacks run here.
        $ctx->release;
    }

=back

=head2 release($;$)

Usage:

=over 4

=item release $ctx;

=item release $ctx, ...;

=back

This is intended as a shortcut that lets you release your context and return a
value in one statement. This function will get your context, and an optional
return value. It will release your context, then return your value. Scalar
context is always assumed.

    sub tool {
        my $ctx = context();
        ...

        return release $ctx, 1;
    }

This tool is most useful when you want to return the value you get from calling
a function that needs to see the current context:

    my $ctx = context();
    my $out = some_tool(...);
    $ctx->release;
    return $out;

We can combine the last 3 lines of the above like so:

    my $ctx = context();
    release $ctx, some_tool(...);

=head2 context_do(&;@)

Usage:

    sub my_tool {
        context_do {
            my $ctx = shift;

            my (@args) = @_;

            $ctx->ok(1, "pass");

            ...

            # No need to call $ctx->release, done for you on scope exit.
        } @_;
    }

Using this inside your test tool takes care of a lot of boilerplate for you. It
will ensure a context is acquired. It will capture and rethrow any exception. It
will insure the context is released when you are done. It preserves the
subroutine call context (array, scalar, void).

This is the safest way to write a test tool. The only two downsides to this are a
slight performance decrease, and some extra indentation in your source. If the
indentation is a problem for you then you can take a peek at the next section.

=head2 no_context(&;$)

Usage:

=over 4

=item no_context { ... };

=item no_context { ... } $hid;

    sub my_tool(&) {
        my $code = shift;
        my $ctx = context();
        ...

        no_context {
            # Things in here will not see our current context, they get a new
            # one.

            $code->();
        };

        ...
        $ctx->release;
    };

=back

This tool will hide a context for the provided block of code. This means any
tools run inside the block will get a completely new context if they acquire
one. The new context will be inherited by tools nested below the one that
acquired it.

This will normally hide the current context for the top hub. If you need to
hide the context for a different hub you can pass in the optional C<$hid>
parameter.

=head2 intercept(&)

Usage:

    my $events = intercept {
        ok(1, "pass");
        ok(0, "fail");
        ...
    };

This function takes a codeblock as its only argument, and it has a prototype.
It will execute the codeblock, intercepting any generated events in the
process. It will return an array reference with all the generated event
objects. All events should be subclasses of L<Test2::Event>.

As of version 1.302178 the events array that is returned is blssed as an
L<Test2::API::InterceptResult> instance. L<Test2::API::InterceptResult>
Provides a helpful interface for filtering and/or inspecting the events list
overall, or individual events within the list.

This is intended to help you test your test code. This is not intended for
people simply writing tests.

=head2 run_subtest(...)

Usage:

    run_subtest($NAME, \&CODE, $BUFFERED, @ARGS)

    # or

    run_subtest($NAME, \&CODE, \%PARAMS, @ARGS)

This will run the provided codeblock with the args in C<@args>. This codeblock
will be run as a subtest. A subtest is an isolated test state that is condensed
into a single L<Test2::Event::Subtest> event, which contains all events
generated inside the subtest.

=head3 ARGUMENTS:

=over 4

=item $NAME

The name of the subtest.

=item \&CODE

The code to run inside the subtest.

=item $BUFFERED or \%PARAMS

If this is a simple scalar then it will be treated as a boolean for the
'buffered' setting. If this is a hash reference then it will be used as a
parameters hash. The param hash will be used for hub construction (with the
specified keys removed).

Keys that are removed and used by run_subtest:

=over 4

=item 'buffered' => $bool

Toggle buffered status.

=item 'inherit_trace' => $bool

Normally the subtest hub is pushed and the sub is allowed to generate its own
root context for the hub. When this setting is turned on a root context will be
created for the hub that shares the same trace as the current context.

Set this to true if your tool is producing subtests without user-specified
subs.

=item 'no_fork' => $bool

Defaults to off. Normally forking inside a subtest will actually fork the
subtest, resulting in 2 final subtest events. This parameter will turn off that
behavior, only the original process/thread will return a final subtest event.

=back

=item @ARGS

Any extra arguments you want passed into the subtest code.

=back

=head3 BUFFERED VS UNBUFFERED (OR STREAMED)

Normally all events inside and outside a subtest are sent to the formatter
immediately by the hub. Sometimes it is desirable to hold off sending events
within a subtest until the subtest is complete. This usually depends on the
formatter being used.

=over 4

=item Things not effected by this flag

In both cases events are generated and stored in an array. This array is
eventually used to populate the C<subevents> attribute on the
L<Test2::Event::Subtest> event that is generated at the end of the subtest.
This flag has no effect on this part, it always happens.

At the end of the subtest, the final L<Test2::Event::Subtest> event is sent to
the formatter.

=item Things that are effected by this flag

The C<buffered> attribute of the L<Test2::Event::Subtest> event will be set to
the value of this flag. This means any formatter, listener, etc which looks at
the event will know if it was buffered.

=item Things that are formatter dependant

Events within a buffered subtest may or may not be sent to the formatter as
they happen. If a formatter fails to specify then the default is to B<NOT SEND>
the events as they are generated, instead the formatter can pull them from the
C<subevents> attribute.

A formatter can specify by implementing the C<hide_buffered()> method. If this
method returns true then events generated inside a buffered subtest will not be
sent independently of the final subtest event.

=back

An example of how this is used is the L<Test2::Formatter::TAP> formatter. For
unbuffered subtests the events are rendered as they are generated. At the end
of the subtest, the final subtest event is rendered, but the C<subevents>
attribute is ignored. For buffered subtests the opposite occurs, the events are
NOT rendered as they are generated, instead the C<subevents> attribute is used
to render them all at once. This is useful when running subtests tests in
parallel, since without it the output from subtests would be interleaved
together.

=head1 OTHER API EXPORTS

Exports in this section are not commonly needed. These all have the 'test2_'
prefix to help ensure they stand out. You should look at the L</MAIN API
EXPORTS> section before looking here. This section is one where "Great power
comes with great responsibility". It is possible to break things badly if you
are not careful with these.

All exports are optional. You need to list which ones you want at import time:

    use Test2::API qw/test2_init_done .../;

=head2 STATUS AND INITIALIZATION STATE

These provide access to internal state and object instances.

=over 4

=item $bool = test2_init_done()

This will return true if the stack and IPC instances have already been
initialized. It will return false if they have not. Init happens as late as
possible. It happens as soon as a tool requests the IPC instance, the
formatter, or the stack.

=item $bool = test2_load_done()

This will simply return the boolean value of the loaded flag. If Test2 has
finished loading this will be true, otherwise false. Loading is considered
complete the first time a tool requests a context.

=item test2_set_is_end()

=item test2_set_is_end($bool)

This is used to toggle Test2's belief that the END phase has already started.
With no arguments this will set it to true. With arguments it will set it to
the first argument's value.

This is used to prevent the use of C<caller()> in END blocks which can cause
segfaults. This is only necessary in some persistent environments that may have
multiple END phases.

=item $bool = test2_get_is_end()

Check if Test2 believes it is the END phase.

=item $stack = test2_stack()

This will return the global L<Test2::API::Stack> instance. If this has not
yet been initialized it will be initialized now.

=item $bool = test2_is_testing_done()

This will return true if testing is complete and no other events should be
sent. This is useful in things like warning handlers where you might want to
turn warnings into events, but need them to start acting like normal warnings
when testing is done.

    $SIG{__WARN__} = sub {
        my ($warning) = @_;

        if (test2_is_testing_done()) {
            warn @_;
        }
        else {
            my $ctx = context();
            ...
            $ctx->release
        }
    }

=item test2_ipc_disable

Disable IPC.

=item $bool = test2_ipc_diabled

Check if IPC is disabled.

=item test2_ipc_wait_enable()

=item test2_ipc_wait_disable()

=item $bool = test2_ipc_wait_enabled()

These can be used to turn IPC waiting on and off, or check the current value of
the flag.

Waiting is turned on by default. Waiting will cause the parent process/thread
to wait until all child processes and threads are finished before exiting. You
will almost never want to turn this off.

=item $bool = test2_no_wait()

=item test2_no_wait($bool)

B<DISCOURAGED>: This is a confusing interface, it is better to use
C<test2_ipc_wait_enable()>, C<test2_ipc_wait_disable()> and
C<test2_ipc_wait_enabled()>.

This can be used to get/set the no_wait status. Waiting is turned on by
default. Waiting will cause the parent process/thread to wait until all child
processes and threads are finished before exiting. You will almost never want
to turn this off.

=item $fh = test2_stdout()

=item $fh = test2_stderr()

These functions return the filehandles that test output should be written to.
They are primarily useful when writing a custom formatter and code that turns
events into actual output (TAP, etc.).  They will return a dupe of the original
filehandles that formatted output can be sent to regardless of whatever state
the currently running test may have left STDOUT and STDERR in.

=item test2_reset_io()

Re-dupe the internal filehandles returned by C<test2_stdout()> and
C<test2_stderr()> from the current STDOUT and STDERR.  You shouldn't need to do
this except in very peculiar situations (for example, you're testing a new
formatter and you need control over where the formatter is sending its output.)

=back

=head2 BEHAVIOR HOOKS

These are hooks that allow you to add custom behavior to actions taken by Test2
and tools built on top of it.

=over 4

=item test2_add_callback_exit(sub { ... })

This can be used to add a callback that is called after all testing is done. This
is too late to add additional results, the main use of this callback is to set the
exit code.

    test2_add_callback_exit(
        sub {
            my ($context, $exit, \$new_exit) = @_;
            ...
        }
    );

The C<$context> passed in will be an instance of L<Test2::API::Context>. The
C<$exit> argument will be the original exit code before anything modified it.
C<$$new_exit> is a reference to the new exit code. You may modify this to
change the exit code. Please note that C<$$new_exit> may already be different
from C<$exit>

=item test2_add_callback_post_load(sub { ... })

Add a callback that will be called when Test2 is finished loading. This
means the callback will be run once, the first time a context is obtained.
If Test2 has already finished loading then the callback will be run immediately.

=item test2_add_callback_testing_done(sub { ... })

This adds your coderef as a follow-up to the root hub after Test2 is finished loading.

This is essentially a helper to do the following:

    test2_add_callback_post_load(sub {
        my $stack = test2_stack();
        $stack->top; # Insure we have a hub
        my ($hub) = Test2::API::test2_stack->all;

        $hub->set_active(1);

        $hub->follow_up(sub { ... }); # <-- Your coderef here
    });

=item test2_add_callback_context_acquire(sub { ... })

Add a callback that will be called every time someone tries to acquire a
context. This will be called on EVERY call to C<context()>. It gets a single
argument, a reference to the hash of parameters being used the construct the
context. This is your chance to change the parameters by directly altering the
hash.

    test2_add_callback_context_acquire(sub {
        my $params = shift;
        $params->{level}++;
    });

This is a very scary API function. Please do not use this unless you need to.
This is here for L<Test::Builder> and backwards compatibility. This has you
directly manipulate the hash instead of returning a new one for performance
reasons.

=item test2_add_callback_context_init(sub { ... })

Add a callback that will be called every time a new context is created. The
callback will receive the newly created context as its only argument.

=item test2_add_callback_context_release(sub { ... })

Add a callback that will be called every time a context is released. The
callback will receive the released context as its only argument.

=item test2_add_callback_pre_subtest(sub { ... })

Add a callback that will be called every time a subtest is going to be
run. The callback will receive the subtest name, coderef, and any
arguments.

=item @list = test2_list_context_acquire_callbacks()

Return all the context acquire callback references.

=item @list = test2_list_context_init_callbacks()

Returns all the context init callback references.

=item @list = test2_list_context_release_callbacks()

Returns all the context release callback references.

=item @list = test2_list_exit_callbacks()

Returns all the exit callback references.

=item @list = test2_list_post_load_callbacks()

Returns all the post load callback references.

=item @list = test2_list_pre_subtest_callbacks()

Returns all the pre-subtest callback references.

=item test2_add_uuid_via(sub { ... })

=item $sub = test2_add_uuid_via()

This allows you to provide a UUID generator. If provided UUIDs will be attached
to all events, hubs, and contexts. This is useful for storing, tracking, and
linking these objects.

The sub you provide should always return a unique identifier. Most things will
expect a proper UUID string, however nothing in Test2::API enforces this.

The sub will receive exactly 1 argument, the type of thing being tagged
'context', 'hub', or 'event'. In the future additional things may be tagged, in
which case new strings will be passed in. These are purely informative, you can
(and usually should) ignore them.

=back

=head2 IPC AND CONCURRENCY

These let you access, or specify, the IPC system internals.

=over 4

=item $bool = test2_has_ipc()

Check if IPC is enabled.

=item $ipc = test2_ipc()

This will return the global L<Test2::IPC::Driver> instance. If this has not yet
been initialized it will be initialized now.

=item test2_ipc_add_driver($DRIVER)

Add an IPC driver to the list. This will add the driver to the start of the
list.

=item @drivers = test2_ipc_drivers()

Get the list of IPC drivers.

=item $bool = test2_ipc_polling()

Check if polling is enabled.

=item test2_ipc_enable_polling()

Turn on polling. This will cull events from other processes and threads every
time a context is created.

=item test2_ipc_disable_polling()

Turn off IPC polling.

=item test2_ipc_enable_shm()

Legacy, this is currently a no-op that returns 0;

=item test2_ipc_set_pending($uniq_val)

Tell other processes and events that an event is pending. C<$uniq_val> should
be a unique value no other thread/process will generate.

B<Note:> After calling this C<test2_ipc_get_pending()> will return 1. This is
intentional, and not avoidable.

=item $pending = test2_ipc_get_pending()

This returns -1 if there is no way to check (assume yes)

This returns 0 if there are (most likely) no pending events.

This returns 1 if there are (likely) pending events. Upon return it will reset,
nothing else will be able to see that there were pending events.

=item $timeout = test2_ipc_get_timeout()

=item test2_ipc_set_timeout($timeout)

Get/Set the timeout value for the IPC system. This timeout is how long the IPC
system will wait for child processes and threads to finish before aborting.

The default value is C<30> seconds.

=back

=head2 MANAGING FORMATTERS

These let you access, or specify, the formatters that can/should be used.

=over 4

=item $formatter = test2_formatter

This will return the global formatter class. This is not an instance. By
default the formatter is set to L<Test2::Formatter::TAP>.

You can override this default using the C<T2_FORMATTER> environment variable.

Normally 'Test2::Formatter::' is prefixed to the value in the
environment variable:

    $ T2_FORMATTER='TAP' perl test.t     # Use the Test2::Formatter::TAP formatter
    $ T2_FORMATTER='Foo' perl test.t     # Use the Test2::Formatter::Foo formatter

If you want to specify a full module name you use the '+' prefix:

    $ T2_FORMATTER='+Foo::Bar' perl test.t     # Use the Foo::Bar formatter

=item test2_formatter_set($class_or_instance)

Set the global formatter class. This can only be set once. B<Note:> This will
override anything specified in the 'T2_FORMATTER' environment variable.

=item @formatters = test2_formatters()

Get a list of all loaded formatters.

=item test2_formatter_add($class_or_instance)

Add a formatter to the list. Last formatter added is used at initialization. If
this is called after initialization a warning will be issued.

=back

=head1 OTHER EXAMPLES

See the C</Examples/> directory included in this distribution.

=head1 SEE ALSO

L<Test2::API::Context> - Detailed documentation of the context object.

L<Test2::IPC> - The IPC system used for threading/fork support.

L<Test2::Formatter> - Formatters such as TAP live here.

L<Test2::Event> - Events live in this namespace.

L<Test2::Hub> - All events eventually funnel through a hub. Custom hubs are how
C<intercept()> and C<run_subtest()> are implemented.

=head1 MAGIC

This package has an END block. This END block is responsible for setting the
exit code based on the test results. This end block also calls the callbacks that
can be added to this package.

=head1 SOURCE

The source code repository for Test2 can be found at
F<http://github.com/Test-More/test-more/>.

=head1 MAINTAINERS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 AUTHORS

=over 4

=item Chad Granum E<lt>exodist@cpan.orgE<gt>

=back

=head1 COPYRIGHT

Copyright 2020 Chad Granum E<lt>exodist@cpan.orgE<gt>.

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

See F<http://dev.perl.org/licenses/>

=cut
