package Test2::API::InterceptResult::Squasher;
use strict;
use warnings;

our $VERSION = '1.302194';

use Carp qw/croak/;
use List::Util qw/first/;

use Test2::Util::HashBase qw{
    <events

    +down_sig +down_buffer

    +up_into +up_sig +up_clear
};

sub init {
    my $self = shift;

    croak "'events' is a required attribute"  unless $self->{+EVENTS};
}

sub can_squash {
    my $self = shift;
    my ($event) = @_;

    # No info, no squash
    return unless $event->has_info;

    # Do not merge up if one of these is true
    return if first { $event->$_ } 'causes_fail', 'has_assert', 'has_bailout', 'has_errors', 'has_plan', 'has_subtest';

    # Signature if we can squash
    return $event->trace_signature;
}

sub process {
    my $self = shift;
    my ($event) = @_;

    return if $self->squash_up($event);
    return if $self->squash_down($event);

    $self->flush_down($event);

    push @{$self->{+EVENTS}} => $event;

    return;
}

sub squash_down {
    my $self = shift;
    my ($event) = @_;

    my $sig = $self->can_squash($event)
        or return;

    $self->flush_down()
        if $self->{+DOWN_SIG} && $self->{+DOWN_SIG} ne $sig;

    $self->{+DOWN_SIG} ||= $sig;
    push @{$self->{+DOWN_BUFFER}} => $event;

    return 1;
}

sub flush_down {
    my $self = shift;
    my ($into) = @_;

    my $sig    = delete $self->{+DOWN_SIG};
    my $buffer = delete $self->{+DOWN_BUFFER};

    return unless $buffer && @$buffer;

    my $fsig = $into ? $into->trace_signature : undef;

    if ($fsig && $fsig eq $sig) {
        $self->squash($into, @$buffer);
    }
    else {
        push @{$self->{+EVENTS}} => @$buffer if $buffer;
    }
}

sub clear_up {
    my $self = shift;

    return unless $self->{+UP_CLEAR};

    delete $self->{+UP_INTO};
    delete $self->{+UP_SIG};
    delete $self->{+UP_CLEAR};
}

sub squash_up {
    my $self = shift;
    my ($event) = @_;
    no warnings 'uninitialized';

    $self->clear_up;

    if ($event->has_assert) {
        if(my $sig = $event->trace_signature) {
            $self->{+UP_INTO}  = $event;
            $self->{+UP_SIG}   = $sig;
            $self->{+UP_CLEAR} = 0;
        }
        else {
            $self->{+UP_CLEAR} = 1;
            $self->clear_up;
        }

        return;
    }

    my $into = $self->{+UP_INTO} or return;

    # Next iteration should clear unless something below changes that
    $self->{+UP_CLEAR} = 1;

    # Only merge into matching trace signatres
    my $sig = $self->can_squash($event);
    return unless $sig eq $self->{+UP_SIG};

    # OK Merge! Do not clear merge in case the return event is also a matching sig diag-only
    $self->{+UP_CLEAR} = 0;

    $self->squash($into, $event);

    return 1;
}

sub squash {
    my $self = shift;
    my ($into, @from) = @_;
    push @{$into->facet_data->{info}} => $_->info for @from;
}

sub DESTROY {
    my $self = shift;

    return unless $self->{+EVENTS};
    $self->flush_down();
    return;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::API::InterceptResult::Squasher - Encapsulation of the algorithm that
squashes diags into assertions.

=head1 DESCRIPTION

Internal use only, please ignore.

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
