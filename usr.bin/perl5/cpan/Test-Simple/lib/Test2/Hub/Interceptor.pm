package Test2::Hub::Interceptor;
use strict;
use warnings;

our $VERSION = '1.302194';


use Test2::Hub::Interceptor::Terminator();

BEGIN { require Test2::Hub; our @ISA = qw(Test2::Hub) }
use Test2::Util::HashBase;

sub init {
    my $self = shift;
    $self->SUPER::init();
    $self->{+NESTED} = 0;
}

sub inherit {
    my $self = shift;
    my ($from, %params) = @_;

    $self->{+NESTED} = 0;

    if ($from->{+IPC} && !$self->{+IPC} && !exists($params{ipc})) {
        my $ipc = $from->{+IPC};
        $self->{+IPC} = $ipc;
        $ipc->add_hub($self->{+HID});
    }

    if (my $ls = $from->{+_LISTENERS}) {
        push @{$self->{+_LISTENERS}} => grep { $_->{intercept_inherit} } @$ls;
    }

    if (my $pfs = $from->{+_PRE_FILTERS}) {
        push @{$self->{+_PRE_FILTERS}} => grep { $_->{intercept_inherit} } @$pfs;
    }

    if (my $fs = $from->{+_FILTERS}) {
        push @{$self->{+_FILTERS}} => grep { $_->{intercept_inherit} } @$fs;
    }
}

sub clean_inherited {
    my $self = shift;
    my %params = @_;

    my @sets = (
        $self->{+_LISTENERS},
        $self->{+_PRE_FILTERS},
        $self->{+_FILTERS},
    );

    for my $set (@sets) {
        next unless $set;

        for my $i (@$set) {
            my $cbs = $i->{intercept_inherit} or next;
            next unless ref($cbs) eq 'HASH';
            my $cb = $cbs->{clean} or next;
            $cb->(%params);
        }
    }
}

sub restore_inherited {
    my $self = shift;
    my %params = @_;

    my @sets = (
        $self->{+_FILTERS},
        $self->{+_PRE_FILTERS},
        $self->{+_LISTENERS},
    );

    for my $set (@sets) {
        next unless $set;

        for my $i (@$set) {
            my $cbs = $i->{intercept_inherit} or next;
            next unless ref($cbs) eq 'HASH';
            my $cb = $cbs->{restore} or next;
            $cb->(%params);
        }
    }
}

sub terminate {
    my $self = shift;
    my ($code) = @_;

    eval {
        no warnings 'exiting';
        last T2_SUBTEST_WRAPPER;
    };
    my $err = $@;

    # Fallback
    die bless(\$err, 'Test2::Hub::Interceptor::Terminator');
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Hub::Interceptor - Hub used by interceptor to grab results.

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
