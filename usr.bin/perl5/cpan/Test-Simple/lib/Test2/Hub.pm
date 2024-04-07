package Test2::Hub;
use strict;
use warnings;

our $VERSION = '1.302194';


use Carp qw/carp croak confess/;
use Test2::Util qw/get_tid gen_uid/;

use Scalar::Util qw/weaken/;
use List::Util qw/first/;

use Test2::Util::ExternalMeta qw/meta get_meta set_meta delete_meta/;
use Test2::Util::HashBase qw{
    pid tid hid ipc
    nested buffered
    no_ending
    _filters
    _pre_filters
    _listeners
    _follow_ups
    _formatter
    _context_acquire
    _context_init
    _context_release

    uuid
    active
    count
    failed
    ended
    bailed_out
    _passing
    _plan
    skip_reason
};

my $UUID_VIA;

sub init {
    my $self = shift;

    $self->{+PID} = $$;
    $self->{+TID} = get_tid();
    $self->{+HID} = gen_uid();

    $UUID_VIA ||= Test2::API::_add_uuid_via_ref();
    $self->{+UUID} = ${$UUID_VIA}->('hub') if $$UUID_VIA;

    $self->{+NESTED}   = 0 unless defined $self->{+NESTED};
    $self->{+BUFFERED} = 0 unless defined $self->{+BUFFERED};

    $self->{+COUNT}    = 0;
    $self->{+FAILED}   = 0;
    $self->{+_PASSING} = 1;

    if (my $formatter = delete $self->{formatter}) {
        $self->format($formatter);
    }

    if (my $ipc = $self->{+IPC}) {
        $ipc->add_hub($self->{+HID});
    }
}

sub is_subtest { 0 }

sub _tb_reset {
    my $self = shift;

    # Nothing to do
    return if $self->{+PID} == $$ && $self->{+TID} == get_tid();

    $self->{+PID} = $$;
    $self->{+TID} = get_tid();
    $self->{+HID} = gen_uid();

    if (my $ipc = $self->{+IPC}) {
        $ipc->add_hub($self->{+HID});
    }
}

sub reset_state {
    my $self = shift;

    $self->{+COUNT} = 0;
    $self->{+FAILED} = 0;
    $self->{+_PASSING} = 1;

    delete $self->{+_PLAN};
    delete $self->{+ENDED};
    delete $self->{+BAILED_OUT};
    delete $self->{+SKIP_REASON};
}

sub inherit {
    my $self = shift;
    my ($from, %params) = @_;

    $self->{+NESTED} ||= 0;

    $self->{+_FORMATTER} = $from->{+_FORMATTER}
        unless $self->{+_FORMATTER} || exists($params{formatter});

    if ($from->{+IPC} && !$self->{+IPC} && !exists($params{ipc})) {
        my $ipc = $from->{+IPC};
        $self->{+IPC} = $ipc;
        $ipc->add_hub($self->{+HID});
    }

    if (my $ls = $from->{+_LISTENERS}) {
        push @{$self->{+_LISTENERS}} => grep { $_->{inherit} } @$ls;
    }

    if (my $pfs = $from->{+_PRE_FILTERS}) {
        push @{$self->{+_PRE_FILTERS}} => grep { $_->{inherit} } @$pfs;
    }

    if (my $fs = $from->{+_FILTERS}) {
        push @{$self->{+_FILTERS}} => grep { $_->{inherit} } @$fs;
    }
}

sub format {
    my $self = shift;

    my $old = $self->{+_FORMATTER};
    ($self->{+_FORMATTER}) = @_ if @_;

    return $old;
}

sub is_local {
    my $self = shift;
    return $$ == $self->{+PID}
        && get_tid() == $self->{+TID};
}

sub listen {
    my $self = shift;
    my ($sub, %params) = @_;

    carp "Useless addition of a listener in a child process or thread!"
        if $$ != $self->{+PID} || get_tid() != $self->{+TID};

    croak "listen only takes coderefs for arguments, got '$sub'"
        unless ref $sub && ref $sub eq 'CODE';

    push @{$self->{+_LISTENERS}} => { %params, code => $sub };

    $sub; # Intentional return.
}

sub unlisten {
    my $self = shift;

    carp "Useless removal of a listener in a child process or thread!"
        if $$ != $self->{+PID} || get_tid() != $self->{+TID};

    my %subs = map {$_ => $_} @_;

    @{$self->{+_LISTENERS}} = grep { !$subs{$_->{code}} } @{$self->{+_LISTENERS}};
}

sub filter {
    my $self = shift;
    my ($sub, %params) = @_;

    carp "Useless addition of a filter in a child process or thread!"
        if $$ != $self->{+PID} || get_tid() != $self->{+TID};

    croak "filter only takes coderefs for arguments, got '$sub'"
        unless ref $sub && ref $sub eq 'CODE';

    push @{$self->{+_FILTERS}} => { %params, code => $sub };

    $sub; # Intentional Return
}

sub unfilter {
    my $self = shift;
    carp "Useless removal of a filter in a child process or thread!"
        if $$ != $self->{+PID} || get_tid() != $self->{+TID};
    my %subs = map {$_ => $_} @_;
    @{$self->{+_FILTERS}} = grep { !$subs{$_->{code}} } @{$self->{+_FILTERS}};
}

sub pre_filter {
    my $self = shift;
    my ($sub, %params) = @_;

    croak "pre_filter only takes coderefs for arguments, got '$sub'"
        unless ref $sub && ref $sub eq 'CODE';

    push @{$self->{+_PRE_FILTERS}} => { %params, code => $sub };

    $sub; # Intentional Return
}

sub pre_unfilter {
    my $self = shift;
    my %subs = map {$_ => $_} @_;
    @{$self->{+_PRE_FILTERS}} = grep { !$subs{$_->{code}} } @{$self->{+_PRE_FILTERS}};
}

sub follow_up {
    my $self = shift;
    my ($sub) = @_;

    carp "Useless addition of a follow-up in a child process or thread!"
        if $$ != $self->{+PID} || get_tid() != $self->{+TID};

    croak "follow_up only takes coderefs for arguments, got '$sub'"
        unless ref $sub && ref $sub eq 'CODE';

    push @{$self->{+_FOLLOW_UPS}} => $sub;
}

*add_context_aquire = \&add_context_acquire;
sub add_context_acquire {
    my $self = shift;
    my ($sub) = @_;

    croak "add_context_acquire only takes coderefs for arguments, got '$sub'"
        unless ref $sub && ref $sub eq 'CODE';

    push @{$self->{+_CONTEXT_ACQUIRE}} => $sub;

    $sub; # Intentional return.
}

*remove_context_aquire = \&remove_context_acquire;
sub remove_context_acquire {
    my $self = shift;
    my %subs = map {$_ => $_} @_;
    @{$self->{+_CONTEXT_ACQUIRE}} = grep { !$subs{$_} == $_ } @{$self->{+_CONTEXT_ACQUIRE}};
}

sub add_context_init {
    my $self = shift;
    my ($sub) = @_;

    croak "add_context_init only takes coderefs for arguments, got '$sub'"
        unless ref $sub && ref $sub eq 'CODE';

    push @{$self->{+_CONTEXT_INIT}} => $sub;

    $sub; # Intentional return.
}

sub remove_context_init {
    my $self = shift;
    my %subs = map {$_ => $_} @_;
    @{$self->{+_CONTEXT_INIT}} = grep { !$subs{$_} == $_ } @{$self->{+_CONTEXT_INIT}};
}

sub add_context_release {
    my $self = shift;
    my ($sub) = @_;

    croak "add_context_release only takes coderefs for arguments, got '$sub'"
        unless ref $sub && ref $sub eq 'CODE';

    push @{$self->{+_CONTEXT_RELEASE}} => $sub;

    $sub; # Intentional return.
}

sub remove_context_release {
    my $self = shift;
    my %subs = map {$_ => $_} @_;
    @{$self->{+_CONTEXT_RELEASE}} = grep { !$subs{$_} == $_ } @{$self->{+_CONTEXT_RELEASE}};
}

sub send {
    my $self = shift;
    my ($e) = @_;

    $e->eid;

    $e->add_hub(
        {
            details => ref($self),

            buffered => $self->{+BUFFERED},
            hid      => $self->{+HID},
            nested   => $self->{+NESTED},
            pid      => $self->{+PID},
            tid      => $self->{+TID},
            uuid     => $self->{+UUID},

            ipc => $self->{+IPC} ? 1 : 0,
        }
    );

    $e->set_uuid(${$UUID_VIA}->('event')) if $$UUID_VIA;

    if ($self->{+_PRE_FILTERS}) {
        for (@{$self->{+_PRE_FILTERS}}) {
            $e = $_->{code}->($self, $e);
            return unless $e;
        }
    }

    my $ipc = $self->{+IPC} || return $self->process($e);

    if($e->global) {
        $ipc->send($self->{+HID}, $e, 'GLOBAL');
        return $self->process($e);
    }

    return $ipc->send($self->{+HID}, $e)
        if $$ != $self->{+PID} || get_tid() != $self->{+TID};

    $self->process($e);
}

sub process {
    my $self = shift;
    my ($e) = @_;

    if ($self->{+_FILTERS}) {
        for (@{$self->{+_FILTERS}}) {
            $e = $_->{code}->($self, $e);
            return unless $e;
        }
    }

    # Optimize the most common case
    my $type = ref($e);
    if ($type eq 'Test2::Event::Pass' || ($type eq 'Test2::Event::Ok' && $e->{pass})) {
        my $count = ++($self->{+COUNT});
        $self->{+_FORMATTER}->write($e, $count) if $self->{+_FORMATTER};

        if ($self->{+_LISTENERS}) {
            $_->{code}->($self, $e, $count) for @{$self->{+_LISTENERS}};
        }

        return $e;
    }

    my $f = $e->facet_data;

    my $fail = 0;
    $fail = 1 if $f->{assert} && !$f->{assert}->{pass};
    $fail = 1 if $f->{errors} && grep { $_->{fail} } @{$f->{errors}};
    $fail = 0 if $f->{amnesty};

    $self->{+COUNT}++ if $f->{assert};
    $self->{+FAILED}++ if $fail && $f->{assert};
    $self->{+_PASSING} = 0 if $fail;

    my $code = $f->{control} ? $f->{control}->{terminate} : undef;
    my $count = $self->{+COUNT};

    if (my $plan = $f->{plan}) {
        if ($plan->{skip}) {
            $self->plan('SKIP');
            $self->set_skip_reason($plan->{details} || 1);
            $code ||= 0;
        }
        elsif ($plan->{none}) {
            $self->plan('NO PLAN');
        }
        else {
            $self->plan($plan->{count});
        }
    }

    $e->callback($self) if $f->{control} && $f->{control}->{has_callback};

    $self->{+_FORMATTER}->write($e, $count, $f) if $self->{+_FORMATTER};

    if ($self->{+_LISTENERS}) {
        $_->{code}->($self, $e, $count, $f) for @{$self->{+_LISTENERS}};
    }

    if ($f->{control} && $f->{control}->{halt}) {
        $code ||= 255;
        $self->set_bailed_out($e);
    }

    if (defined $code) {
        $self->{+_FORMATTER}->terminate($e, $f) if $self->{+_FORMATTER};
        $self->terminate($code, $e, $f);
    }

    return $e;
}

sub terminate {
    my $self = shift;
    my ($code) = @_;
    exit($code);
}

sub cull {
    my $self = shift;

    my $ipc = $self->{+IPC} || return;
    return if $self->{+PID} != $$ || $self->{+TID} != get_tid();

    # No need to do IPC checks on culled events
    $self->process($_) for $ipc->cull($self->{+HID});
}

sub finalize {
    my $self = shift;
    my ($trace, $do_plan) = @_;

    $self->cull();

    my $plan   = $self->{+_PLAN};
    my $count  = $self->{+COUNT};
    my $failed = $self->{+FAILED};
    my $active = $self->{+ACTIVE};

    # return if NOTHING was done.
    unless ($active || $do_plan || defined($plan) || $count || $failed) {
        $self->{+_FORMATTER}->finalize($plan, $count, $failed, 0, $self->is_subtest) if $self->{+_FORMATTER};
        return;
    }

    unless ($self->{+ENDED}) {
        if ($self->{+_FOLLOW_UPS}) {
            $_->($trace, $self) for reverse @{$self->{+_FOLLOW_UPS}};
        }

        # These need to be refreshed now
        $plan   = $self->{+_PLAN};
        $count  = $self->{+COUNT};
        $failed = $self->{+FAILED};

        if (($plan && $plan eq 'NO PLAN') || ($do_plan && !$plan)) {
            $self->send(
                Test2::Event::Plan->new(
                    trace => $trace,
                    max => $count,
                )
            );
        }
        $plan = $self->{+_PLAN};
    }

    my $frame = $trace->frame;
    if($self->{+ENDED}) {
        my (undef, $ffile, $fline) = @{$self->{+ENDED}};
        my (undef, $sfile, $sline) = @$frame;

        die <<"        EOT"
Test already ended!
First End:  $ffile line $fline
Second End: $sfile line $sline
        EOT
    }

    $self->{+ENDED} = $frame;
    my $pass = $self->is_passing(); # Generate the final boolean.

    $self->{+_FORMATTER}->finalize($plan, $count, $failed, $pass, $self->is_subtest) if $self->{+_FORMATTER};

    return $pass;
}

sub is_passing {
    my $self = shift;

    ($self->{+_PASSING}) = @_ if @_;

    # If we already failed just return 0.
    my $pass = $self->{+_PASSING} or return 0;
    return $self->{+_PASSING} = 0 if $self->{+FAILED};

    my $count = $self->{+COUNT};
    my $ended = $self->{+ENDED};
    my $plan = $self->{+_PLAN};

    return $pass if !$count && $plan && $plan =~ m/^SKIP$/;

    return $self->{+_PASSING} = 0
        if $ended && (!$count || !$plan);

    return $pass unless $plan && $plan =~ m/^\d+$/;

    if ($ended) {
        return $self->{+_PASSING} = 0 if $count != $plan;
    }
    else {
        return $self->{+_PASSING} = 0 if $count > $plan;
    }

    return $pass;
}

sub plan {
    my $self = shift;

    return $self->{+_PLAN} unless @_;

    my ($plan) = @_;

    confess "You cannot unset the plan"
        unless defined $plan;

    confess "You cannot change the plan"
        if $self->{+_PLAN} && $self->{+_PLAN} !~ m/^NO PLAN$/;

    confess "'$plan' is not a valid plan! Plan must be an integer greater than 0, 'NO PLAN', or 'SKIP'"
        unless $plan =~ m/^(\d+|NO PLAN|SKIP)$/;

    $self->{+_PLAN} = $plan;
}

sub check_plan {
    my $self = shift;

    return undef unless $self->{+ENDED};
    my $plan = $self->{+_PLAN} || return undef;

    return 1 if $plan !~ m/^\d+$/;

    return 1 if $plan == $self->{+COUNT};
    return 0;
}

sub DESTROY {
    my $self = shift;
    my $ipc = $self->{+IPC} || return;
    return unless $$ == $self->{+PID};
    return unless get_tid() == $self->{+TID};
    $ipc->drop_hub($self->{+HID});
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Hub - The conduit through which all events flow.

=head1 SYNOPSIS

    use Test2::Hub;

    my $hub = Test2::Hub->new();
    $hub->send(...);

=head1 DESCRIPTION

The hub is the place where all events get processed and handed off to the
formatter. The hub also tracks test state, and provides several hooks into the
event pipeline.

=head1 COMMON TASKS

=head2 SENDING EVENTS

    $hub->send($event)

The C<send()> method is used to issue an event to the hub. This method will
handle thread/fork sync, filters, listeners, TAP output, etc.

=head2 ALTERING OR REMOVING EVENTS

You can use either C<filter()> or C<pre_filter()>, depending on your
needs. Both have identical syntax, so only C<filter()> is shown here.

    $hub->filter(sub {
        my ($hub, $event) = @_;

        my $action = get_action($event);

        # No action should be taken
        return $event if $action eq 'none';

        # You want your filter to remove the event
        return undef if $action eq 'delete';

        if ($action eq 'do_it') {
            my $new_event = copy_event($event);
            ... Change your copy of the event ...
            return $new_event;
        }

        die "Should not happen";
    });

By default, filters are not inherited by child hubs. That means if you start a
subtest, the subtest will not inherit the filter. You can change this behavior
with the C<inherit> parameter:

    $hub->filter(sub { ... }, inherit => 1);

=head2 LISTENING FOR EVENTS

    $hub->listen(sub {
        my ($hub, $event, $number) = @_;

        ... do whatever you want with the event ...

        # return is ignored
    });

By default listeners are not inherited by child hubs. That means if you start a
subtest, the subtest will not inherit the listener. You can change this behavior
with the C<inherit> parameter:

    $hub->listen(sub { ... }, inherit => 1);


=head2 POST-TEST BEHAVIORS

    $hub->follow_up(sub {
        my ($trace, $hub) = @_;

        ... do whatever you need to ...

        # Return is ignored
    });

follow_up subs are called only once, either when done_testing is called, or in
an END block.

=head2 SETTING THE FORMATTER

By default an instance of L<Test2::Formatter::TAP> is created and used.

    my $old = $hub->format(My::Formatter->new);

Setting the formatter will REPLACE any existing formatter. You may set the
formatter to undef to prevent output. The old formatter will be returned if one
was already set. Only one formatter is allowed at a time.

=head1 METHODS

=over 4

=item $hub->send($event)

This is where all events enter the hub for processing.

=item $hub->process($event)

This is called by send after it does any IPC handling. You can use this to
bypass the IPC process, but in general you should avoid using this.

=item $old = $hub->format($formatter)

Replace the existing formatter instance with a new one. Formatters must be
objects that implement a C<< $formatter->write($event) >> method.

=item $sub = $hub->listen(sub { ... }, %optional_params)

You can use this to record all events AFTER they have been sent to the
formatter. No changes made here will be meaningful, except possibly to other
listeners.

    $hub->listen(sub {
        my ($hub, $event, $number) = @_;

        ... do whatever you want with the event ...

        # return is ignored
    });

Normally listeners are not inherited by child hubs such as subtests. You can
add the C<< inherit => 1 >> parameter to allow a listener to be inherited.

=item $hub->unlisten($sub)

You can use this to remove a listen callback. You must pass in the coderef
returned by the C<listen()> method.

=item $sub = $hub->filter(sub { ... }, %optional_params)

=item $sub = $hub->pre_filter(sub { ... }, %optional_params)

These can be used to add filters. Filters can modify, replace, or remove events
before anything else can see them.

    $hub->filter(
        sub {
            my ($hub, $event) = @_;

            return $event;    # No Changes
            return;           # Remove the event

            # Or you can modify an event before returning it.
            $event->modify;
            return $event;
        }
    );

If you are not using threads, forking, or IPC then the only difference between
a C<filter> and a C<pre_filter> is that C<pre_filter> subs run first. When you
are using threads, forking, or IPC, pre_filters happen to events before they
are sent to their destination proc/thread, ordinary filters happen only in the
destination hub/thread.

You cannot add a regular filter to a hub if the hub was created in another
process or thread. You can always add a pre_filter.

=item $hub->unfilter($sub)

=item $hub->pre_unfilter($sub)

These can be used to remove filters and pre_filters. The C<$sub> argument is
the reference returned by C<filter()> or C<pre_filter()>.

=item $hub->follow_op(sub { ... })

Use this to add behaviors that are called just before the hub is finalized. The
only argument to your codeblock will be a L<Test2::EventFacet::Trace> instance.

    $hub->follow_up(sub {
        my ($trace, $hub) = @_;

        ... do whatever you need to ...

        # Return is ignored
    });

follow_up subs are called only once, ether when done_testing is called, or in
an END block.

=item $sub = $hub->add_context_acquire(sub { ... });

Add a callback that will be called every time someone tries to acquire a
context. It gets a single argument, a reference of the hash of parameters
being used the construct the context. This is your chance to change the
parameters by directly altering the hash.

    test2_add_callback_context_acquire(sub {
        my $params = shift;
        $params->{level}++;
    });

This is a very scary API function. Please do not use this unless you need to.
This is here for L<Test::Builder> and backwards compatibility. This has you
directly manipulate the hash instead of returning a new one for performance
reasons.

B<Note> Using this hook could have a huge performance impact.

The coderef you provide is returned and can be used to remove the hook later.

=item $hub->remove_context_acquire($sub);

This can be used to remove a context acquire hook.

=item $sub = $hub->add_context_init(sub { ... });

This allows you to add callbacks that will trigger every time a new context is
created for the hub. The only argument to the sub will be the
L<Test2::API::Context> instance that was created.

B<Note> Using this hook could have a huge performance impact.

The coderef you provide is returned and can be used to remove the hook later.

=item $hub->remove_context_init($sub);

This can be used to remove a context init hook.

=item $sub = $hub->add_context_release(sub { ... });

This allows you to add callbacks that will trigger every time a context for
this hub is released. The only argument to the sub will be the
L<Test2::API::Context> instance that was released. These will run in reverse
order.

B<Note> Using this hook could have a huge performance impact.

The coderef you provide is returned and can be used to remove the hook later.

=item $hub->remove_context_release($sub);

This can be used to remove a context release hook.

=item $hub->cull()

Cull any IPC events (and process them).

=item $pid = $hub->pid()

Get the process id under which the hub was created.

=item $tid = $hub->tid()

Get the thread id under which the hub was created.

=item $hud = $hub->hid()

Get the identifier string of the hub.

=item $uuid = $hub->uuid()

If UUID tagging is enabled (see L<Test2::API>) then the hub will have a UUID.

=item $ipc = $hub->ipc()

Get the IPC object used by the hub.

=item $hub->set_no_ending($bool)

=item $bool = $hub->no_ending

This can be used to disable auto-ending behavior for a hub. The auto-ending
behavior is triggered by an end block and is used to cull IPC events, and
output the final plan if the plan was 'NO PLAN'.

=item $bool = $hub->active

=item $hub->set_active($bool)

These are used to get/set the 'active' attribute. When true this attribute will
force C<< hub->finalize() >> to take action even if there is no plan, and no
tests have been run. This flag is useful for plugins that add follow-up
behaviors that need to run even if no events are seen.

=back

=head2 STATE METHODS

=over 4

=item $hub->reset_state()

Reset all state to the start. This sets the test count to 0, clears the plan,
removes the failures, etc.

=item $num = $hub->count

Get the number of tests that have been run.

=item $num = $hub->failed

Get the number of failures (Not all failures come from a test fail, so this
number can be larger than the count).

=item $bool = $hub->ended

True if the testing has ended. This MAY return the stack frame of the tool that
ended the test, but that is not guaranteed.

=item $bool = $hub->is_passing

=item $hub->is_passing($bool)

Check if the overall test run is a failure. Can also be used to set the
pass/fail status.

=item $hub->plan($plan)

=item $plan = $hub->plan

Get or set the plan. The plan must be an integer larger than 0, the string
'NO PLAN', or the string 'SKIP'.

=item $bool = $hub->check_plan

Check if the plan and counts match, but only if the tests have ended. If tests
have not ended this will return undef, otherwise it will be a true/false.

=back

=head1 THIRD PARTY META-DATA

This object consumes L<Test2::Util::ExternalMeta> which provides a consistent
way for you to attach meta-data to instances of this class. This is useful for
tools, plugins, and other extensions.

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
