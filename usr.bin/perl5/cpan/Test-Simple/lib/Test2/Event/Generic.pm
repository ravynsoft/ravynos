package Test2::Event::Generic;
use strict;
use warnings;

use Carp qw/croak/;
use Scalar::Util qw/reftype/;

our $VERSION = '1.302194';

BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }
use Test2::Util::HashBase;

my @FIELDS = qw{
    causes_fail increments_count diagnostics no_display callback terminate
    global sets_plan summary facet_data
};
my %DEFAULTS = (
    causes_fail      => 0,
    increments_count => 0,
    diagnostics      => 0,
    no_display       => 0,
);

sub init {
    my $self = shift;

    for my $field (@FIELDS) {
        my $val = defined $self->{$field} ? delete $self->{$field} : $DEFAULTS{$field};
        next unless defined $val;

        my $set = "set_$field";
        $self->$set($val);
    }
}

for my $field (@FIELDS) {
    no strict 'refs';

    *$field = sub { exists $_[0]->{$field} ? $_[0]->{$field} : () }
        unless exists &{$field};

    *{"set_$field"} = sub { $_[0]->{$field} = $_[1] }
        unless exists &{"set_$field"};
}

sub can {
    my $self = shift;
    my ($name) = @_;
    return $self->SUPER::can($name) unless $name eq 'callback';
    return $self->{callback} || \&Test2::Event::callback;
}

sub facet_data {
    my $self = shift;
    return $self->{facet_data} || $self->SUPER::facet_data();
}

sub summary {
    my $self = shift;
    return $self->{summary} if defined $self->{summary};
    $self->SUPER::summary();
}

sub sets_plan {
    my $self = shift;
    return unless $self->{sets_plan};
    return @{$self->{sets_plan}};
}

sub callback {
    my $self = shift;
    my $cb = $self->{callback} || return;
    $self->$cb(@_);
}

sub set_global {
    my $self = shift;
    my ($bool) = @_;

    if(!defined $bool) {
        delete $self->{global};
        return undef;
    }

    $self->{global} = $bool;
}

sub set_callback {
    my $self = shift;
    my ($cb) = @_;

    if(!defined $cb) {
        delete $self->{callback};
        return undef;
    }

    croak "callback must be a code reference"
        unless ref($cb) && reftype($cb) eq 'CODE';

    $self->{callback} = $cb;
}

sub set_terminate {
    my $self = shift;
    my ($exit) = @_;

    if(!defined $exit) {
        delete $self->{terminate};
        return undef;
    }

    croak "terminate must be a positive integer"
       unless $exit =~ m/^\d+$/;

    $self->{terminate} = $exit;
}

sub set_sets_plan {
    my $self = shift;
    my ($plan) = @_;

    if(!defined $plan) {
        delete $self->{sets_plan};
        return undef;
    }

    croak "'sets_plan' must be an array reference"
        unless ref($plan) && reftype($plan) eq 'ARRAY';

    $self->{sets_plan} = $plan;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::Generic - Generic event type.

=head1 DESCRIPTION

This is a generic event that lets you customize all fields in the event API.
This is useful if you have need for a custom event that does not make sense as
a published reusable event subclass.

=head1 SYNOPSIS

    use Test2::API qw/context/;

    sub send_custom_fail {
        my $ctx = shift;

        $ctx->send_event('Generic', causes_fail => 1, summary => 'The sky is falling');

        $ctx->release;
    }

    send_custom_fail();

=head1 METHODS

=over 4

=item $e->facet_data($data)

=item $data = $e->facet_data

Get or set the facet data (see L<Test2::Event>). If no facet_data is set then
C<< Test2::Event->facet_data >> will be called to produce facets from the other
data.

=item $e->callback($hub)

Call the custom callback if one is set, otherwise this does nothing.

=item $e->set_callback(sub { ... })

Set the custom callback. The custom callback must be a coderef. The first
argument to your callback will be the event itself, the second will be the
L<Test2::Event::Hub> that is using the callback.

=item $bool = $e->causes_fail

=item $e->set_causes_fail($bool)

Get/Set the C<causes_fail> attribute. This defaults to C<0>.

=item $bool = $e->diagnostics

=item $e->set_diagnostics($bool)

Get/Set the C<diagnostics> attribute. This defaults to C<0>.

=item $bool_or_undef = $e->global

=item @bool_or_empty = $e->global

=item $e->set_global($bool_or_undef)

Get/Set the C<diagnostics> attribute. This defaults to an empty list which is
undef in scalar context.

=item $bool = $e->increments_count

=item $e->set_increments_count($bool)

Get/Set the C<increments_count> attribute. This defaults to C<0>.

=item $bool = $e->no_display

=item $e->set_no_display($bool)

Get/Set the C<no_display> attribute. This defaults to C<0>.

=item @plan = $e->sets_plan

Get the plan if this event sets one. The plan is a list of up to 3 items:
C<($count, $directive, $reason)>. C<$count> must be defined, the others may be
undef, or may not exist at all.

=item $e->set_sets_plan(\@plan)

Set the plan. You must pass in an arrayref with up to 3 elements.

=item $summary = $e->summary

=item $e->set_summary($summary_or_undef)

Get/Set the summary. This will default to the event package
C<'Test2::Event::Generic'>. You can set it to any value. Setting this to
C<undef> will reset it to the default.

=item $int_or_undef = $e->terminate

=item @int_or_empty = $e->terminate

=item $e->set_terminate($int_or_undef)

This will get/set the C<terminate> attribute. This defaults to undef in scalar
context, or an empty list in list context. Setting this to undef will clear it
completely. This must be set to a positive integer (0 or larger).

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
