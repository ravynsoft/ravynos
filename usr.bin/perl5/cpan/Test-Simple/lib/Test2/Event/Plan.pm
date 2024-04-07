package Test2::Event::Plan;
use strict;
use warnings;

our $VERSION = '1.302194';


BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }
use Test2::Util::HashBase qw{max directive reason};

use Carp qw/confess/;

my %ALLOWED = (
    'SKIP'    => 1,
    'NO PLAN' => 1,
);

sub init {
    if ($_[0]->{+DIRECTIVE}) {
        $_[0]->{+DIRECTIVE} = 'SKIP'    if $_[0]->{+DIRECTIVE} eq 'skip_all';
        $_[0]->{+DIRECTIVE} = 'NO PLAN' if $_[0]->{+DIRECTIVE} eq 'no_plan';

        confess "'" . $_[0]->{+DIRECTIVE} . "' is not a valid plan directive"
            unless $ALLOWED{$_[0]->{+DIRECTIVE}};
    }
    else {
        confess "Cannot have a reason without a directive!"
            if defined $_[0]->{+REASON};

        confess "No number of tests specified"
            unless defined $_[0]->{+MAX};

        confess "Plan test count '" . $_[0]->{+MAX}  . "' does not appear to be a valid positive integer"
            unless $_[0]->{+MAX} =~ m/^\d+$/;

        $_[0]->{+DIRECTIVE} = '';
    }
}

sub sets_plan {
    my $self = shift;
    return (
        $self->{+MAX},
        $self->{+DIRECTIVE},
        $self->{+REASON},
    );
}

sub terminate {
    my $self = shift;
    # On skip_all we want to terminate the hub
    return 0 if $self->{+DIRECTIVE} && $self->{+DIRECTIVE} eq 'SKIP';
    return undef;
}

sub summary {
    my $self = shift;
    my $max = $self->{+MAX};
    my $directive = $self->{+DIRECTIVE};
    my $reason = $self->{+REASON};

    return "Plan is $max assertions"
        if $max || !$directive;

    return "Plan is '$directive', $reason"
        if $reason;

    return "Plan is '$directive'";
}

sub facet_data {
    my $self = shift;

    my $out = $self->common_facet_data;

    $out->{control}->{terminate} = $self->{+DIRECTIVE} eq 'SKIP' ? 0 : undef
        unless defined $out->{control}->{terminate};

    $out->{plan} = {count => $self->{+MAX}};
    $out->{plan}->{details} = $self->{+REASON} if defined $self->{+REASON};

    if (my $dir = $self->{+DIRECTIVE}) {
        $out->{plan}->{skip} = 1 if $dir eq 'SKIP';
        $out->{plan}->{none} = 1 if $dir eq 'NO PLAN';
    }

    return $out;
}


1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Plan - The event of a plan

=head1 DESCRIPTION

Plan events are fired off whenever a plan is declared, done testing is called,
or a subtext completes.

=head1 SYNOPSIS

    use Test2::API qw/context/;
    use Test2::Event::Plan;

    my $ctx = context();

    # Plan for 10 tests to run
    my $event = $ctx->plan(10);

    # Plan to skip all tests (will exit 0)
    $ctx->plan(0, skip_all => "These tests need to be skipped");

=head1 ACCESSORS

=over 4

=item $num = $plan->max

Get the number of expected tests

=item $dir = $plan->directive

Get the directive (such as TODO, skip_all, or no_plan).

=item $reason = $plan->reason

Get the reason for the directive.

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
