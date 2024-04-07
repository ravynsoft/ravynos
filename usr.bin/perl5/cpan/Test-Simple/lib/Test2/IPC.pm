package Test2::IPC;
use strict;
use warnings;

our $VERSION = '1.302194';


use Test2::API::Instance;
use Test2::Util qw/get_tid/;
use Test2::API qw{
    test2_in_preload
    test2_init_done
    test2_ipc
    test2_has_ipc
    test2_ipc_enable_polling
    test2_pid
    test2_stack
    test2_tid
    context
};

# Make sure stuff is finalized before anyone tried to fork or start a new thread.
{
    # Avoid warnings if things are loaded at run-time
    no warnings 'void';
    INIT {
        use warnings 'void';
        context()->release() unless test2_in_preload();
    }
}

use Carp qw/confess/;

our @EXPORT_OK = qw/cull/;
BEGIN { require Exporter; our @ISA = qw(Exporter) }

sub unimport { Test2::API::test2_ipc_disable() }

sub import {
    goto &Exporter::import if test2_has_ipc || !test2_init_done();

    confess "IPC is disabled" if Test2::API::test2_ipc_disabled();
    confess "Cannot add IPC in a child process (" . test2_pid() . " vs $$)" if test2_pid() != $$;
    confess "Cannot add IPC in a child thread (" . test2_tid() . " vs " . get_tid() . ")"  if test2_tid() != get_tid();

    Test2::API::_set_ipc(_make_ipc());
    apply_ipc(test2_stack());

    goto &Exporter::import;
}

sub _make_ipc {
    # Find a driver
    my ($driver) = Test2::API::test2_ipc_drivers();
    unless ($driver) {
        require Test2::IPC::Driver::Files;
        $driver = 'Test2::IPC::Driver::Files';
    }

    return $driver->new();
}

sub apply_ipc {
    my $stack = shift;

    my ($root) = @$stack;

    return unless $root;

    confess "Cannot add IPC in a child process" if $root->pid != $$;
    confess "Cannot add IPC in a child thread"  if $root->tid != get_tid();

    my $ipc = $root->ipc || test2_ipc() || _make_ipc();

    # Add the IPC to all hubs
    for my $hub (@$stack) {
        my $has = $hub->ipc;
        confess "IPC Mismatch!" if $has && $has != $ipc;
        next if $has;
        $hub->set_ipc($ipc);
        $ipc->add_hub($hub->hid);
    }

    test2_ipc_enable_polling();

    return $ipc;
}

sub cull {
    my $ctx = context();
    $ctx->hub->cull;
    $ctx->release;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::IPC - Turn on IPC for threading or forking support.

=head1 SYNOPSIS

You should C<use Test2::IPC;> as early as possible in your test file. If you
import this module after API initialization it will attempt to retrofit IPC
onto the existing hubs.

=head2 DISABLING IT

You can use C<no Test2::IPC;> to disable IPC for good. You can also use the
T2_NO_IPC env var.

=head1 EXPORTS

All exports are optional.

=over 4

=item cull()

Cull allows you to collect results from other processes or threads on demand.

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
