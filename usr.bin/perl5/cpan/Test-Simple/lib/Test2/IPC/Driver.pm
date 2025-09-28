package Test2::IPC::Driver;
use strict;
use warnings;

our $VERSION = '1.302194';


use Carp qw/confess/;
use Test2::Util::HashBase qw{no_fatal no_bail};

use Test2::API qw/test2_ipc_add_driver/;

my %ADDED;
sub import {
    my $class = shift;
    return if $class eq __PACKAGE__;
    return if $ADDED{$class}++;
    test2_ipc_add_driver($class);
}

sub pending { -1 }
sub set_pending { -1 }

for my $meth (qw/send cull add_hub drop_hub waiting is_viable/) {
    no strict 'refs';
    *$meth = sub {
        my $thing = shift;
        confess "'$thing' did not define the required method '$meth'."
    };
}

# Print the error and call exit. We are not using 'die' cause this is a
# catastrophic error that should never be caught. If we get here it
# means some serious shit has happened in a child process, the only way
# to inform the parent may be to exit false.

sub abort {
    my $self = shift;
    chomp(my ($msg) = @_);

    $self->driver_abort($msg) if $self->can('driver_abort');

    print STDERR "IPC Fatal Error: $msg\n";
    print STDOUT "Bail out! IPC Fatal Error: $msg\n" unless $self->no_bail;

    CORE::exit(255) unless $self->no_fatal;
}

sub abort_trace {
    my $self = shift;
    my ($msg) = @_;
    # Older versions of Carp do not export longmess() function, so it needs to be called with package name
    $self->abort(Carp::longmess($msg));
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::IPC::Driver - Base class for Test2 IPC drivers.

=head1 SYNOPSIS

    package Test2::IPC::Driver::MyDriver;

    use base 'Test2::IPC::Driver';

    ...

=head1 METHODS

=over 4

=item $self->abort($msg)

If an IPC encounters a fatal error it should use this. This will print the
message to STDERR with C<'IPC Fatal Error: '> prefixed to it, then it will
forcefully exit 255. IPC errors may occur in threads or processes other than
the main one, this method provides the best chance of the harness noticing the
error.

=item $self->abort_trace($msg)

This is the same as C<< $ipc->abort($msg) >> except that it uses
C<Carp::longmess> to add a stack trace to the message.

=back

=head1 LOADING DRIVERS

Test2::IPC::Driver has an C<import()> method. All drivers inherit this import
method. This import method registers the driver.

In most cases you just need to load the desired IPC driver to make it work. You
should load this driver as early as possible. A warning will be issued if you
load it too late for it to be effective.

    use Test2::IPC::Driver::MyDriver;
    ...

=head1 WRITING DRIVERS

    package Test2::IPC::Driver::MyDriver;
    use strict;
    use warnings;

    use base 'Test2::IPC::Driver';

    sub is_viable {
        return 0 if $^O eq 'win32'; # Will not work on windows.
        return 1;
    }

    sub add_hub {
        my $self = shift;
        my ($hid) = @_;

        ... # Make it possible to contact the hub
    }

    sub drop_hub {
        my $self = shift;
        my ($hid) = @_;

        ... # Nothing should try to reach the hub anymore.
    }

    sub send {
        my $self = shift;
        my ($hid, $e, $global) = @_;

        ... # Send the event to the proper hub.

        # This may notify other procs/threads that there is a pending event.
        Test2::API::test2_ipc_set_pending($uniq_val);
    }

    sub cull {
        my $self = shift;
        my ($hid) = @_;

        my @events = ...; # Here is where you get the events for the hub

        return @events;
    }

    sub waiting {
        my $self = shift;

        ... # Notify all listening procs and threads that the main
        ... # process/thread is waiting for them to finish.
    }

    1;

=head2 METHODS SUBCLASSES MUST IMPLEMENT

=over 4

=item $ipc->is_viable

This should return true if the driver works in the current environment. This
should return false if it does not. This is a CLASS method.

=item $ipc->add_hub($hid)

This is used to alert the driver that a new hub is expecting events. The driver
should keep track of the process and thread ids, the hub should only be dropped
by the proc+thread that started it.

    sub add_hub {
        my $self = shift;
        my ($hid) = @_;

        ... # Make it possible to contact the hub
    }

=item $ipc->drop_hub($hid)

This is used to alert the driver that a hub is no longer accepting events. The
driver should keep track of the process and thread ids, the hub should only be
dropped by the proc+thread that started it (This is the drivers responsibility
to enforce).

    sub drop_hub {
        my $self = shift;
        my ($hid) = @_;

        ... # Nothing should try to reach the hub anymore.
    }

=item $ipc->send($hid, $event);

=item $ipc->send($hid, $event, $global);

Used to send events from the current process/thread to the specified hub in its
process+thread.

    sub send {
        my $self = shift;
        my ($hid, $e) = @_;

        ... # Send the event to the proper hub.

        # This may notify other procs/threads that there is a pending event.
        Test2::API::test2_ipc_set_pending($uniq_val);
    }

If C<$global> is true then the driver should send the event to all hubs in all
processes and threads.

=item @events = $ipc->cull($hid)

Used to collect events that have been sent to the specified hub.

    sub cull {
        my $self = shift;
        my ($hid) = @_;

        my @events = ...; # Here is where you get the events for the hub

        return @events;
    }

=item $ipc->waiting()

This is called in the parent process when it is complete and waiting for all
child processes and threads to complete.

    sub waiting {
        my $self = shift;

        ... # Notify all listening procs and threads that the main
        ... # process/thread is waiting for them to finish.
    }

=back

=head2 METHODS SUBCLASSES MAY IMPLEMENT OR OVERRIDE

=over 4

=item $ipc->driver_abort($msg)

This is a hook called by C<< Test2::IPC::Driver->abort() >>. This is your
chance to cleanup when an abort happens. You cannot prevent the abort, but you
can gracefully except it.

=back

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
