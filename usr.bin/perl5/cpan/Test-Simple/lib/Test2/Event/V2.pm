package Test2::Event::V2;
use strict;
use warnings;

our $VERSION = '1.302194';

use Scalar::Util qw/reftype/;
use Carp qw/croak/;

BEGIN { require Test2::Event; our @ISA = qw(Test2::Event) }

use Test2::Util::Facets2Legacy qw{
    causes_fail diagnostics global increments_count no_display sets_plan
    subtest_id summary terminate
};

use Test2::Util::HashBase qw/-about/;

sub non_facet_keys {
    return (
        +UUID,
        Test2::Util::ExternalMeta::META_KEY(),
    );
}

sub init {
    my $self = shift;

    my $uuid;
    if ($uuid = $self->{+UUID}) {
        croak "uuid '$uuid' passed to constructor, but uuid '$self->{+ABOUT}->{uuid}' is already set in the 'about' facet"
            if $self->{+ABOUT}->{uuid} && $self->{+ABOUT}->{uuid} ne $uuid;

        $self->{+ABOUT}->{uuid} = $uuid;
    }
    elsif ($self->{+ABOUT} && $self->{+ABOUT}->{uuid}) {
        $uuid = $self->{+ABOUT}->{uuid};
        $self->SUPER::set_uuid($uuid);
    }

    # Clone the trace, make sure it is blessed
    if (my $trace = $self->{+TRACE}) {
        $self->{+TRACE} = Test2::EventFacet::Trace->new(%$trace);
    }
}

sub set_uuid {
    my $self = shift;
    my ($uuid) = @_;
    $self->{+ABOUT}->{uuid} = $uuid;
    $self->SUPER::set_uuid($uuid);
}

sub facet_data {
    my $self = shift;
    my $f = { %{$self} };

    delete $f->{$_} for $self->non_facet_keys;

    my %out;
    for my $k (keys %$f) {
        next if substr($k, 0, 1) eq '_';

        my $data = $f->{$k} or next; # Key is there, but no facet
        my $is_list = 'ARRAY' eq (reftype($data) || '');
        $out{$k} = $is_list ? [ map { {%{$_}} } @$data ] : {%$data};
    }

    if (my $meta = $self->meta_facet_data) {
        $out{meta} = {%$meta, %{$out{meta} || {}}};
    }

    return \%out;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::Event::V2 - Second generation event.

=head1 DESCRIPTION

This is the event type that should be used instead of L<Test2::Event> or its
legacy subclasses.

=head1 SYNOPSIS

=head2 USING A CONTEXT

    use Test2::API qw/context/;

    sub my_tool {
        my $ctx = context();

        my $event = $ctx->send_ev2(info => [{tag => 'NOTE', details => "This is a note"}]);

        $ctx->release;

        return $event;
    }

=head2 USING THE CONSTRUCTOR

    use Test2::Event::V2;

    my $e = Test2::Event::V2->new(
        trace => {frame => [$PKG, $FILE, $LINE, $SUBNAME]},
        info  => [{tag => 'NOTE', details => "This is a note"}],
    );

=head1 METHODS

This class inherits from L<Test2::Event>.

=over 4

=item $fd = $e->facet_data()

This will return a hashref of facet data. Each facet hash will be a shallow
copy of the original.

=item $about = $e->about()

This will return the 'about' facet hashref.

B<NOTE:> This will return the internal hashref, not a copy.

=item $trace = $e->trace()

This will return the 'trace' facet, normally blessed (but this is not enforced
when the trace is set using C<set_trace()>.

B<NOTE:> This will return the internal trace, not a copy.

=back

=head2 MUTATION

=over 4

=item $e->add_amnesty({...})

Inherited from L<Test2::Event>. This can be used to add 'amnesty' facets to an
existing event. Each new item is added to the B<END> of the list.

B<NOTE:> Items B<ARE> blessed when added.

=item $e->add_hub({...})

Inherited from L<Test2::Event>. This is used by hubs to stamp events as they
pass through. New items are added to the B<START> of the list.

B<NOTE:> Items B<ARE NOT> blessed when added.

=item $e->set_uuid($UUID)

Inherited from L<Test2::Event>, overridden to also vivify/mutate the 'about'
facet.

=item $e->set_trace($trace)

Inherited from L<Test2::Event> which allows you to change the trace.

B<Note:> This method does not bless/clone the trace for you. Many things will
expect the trace to be blessed, so you should probably do that.

=back

=head2 LEGACY SUPPORT METHODS

These are all imported from L<Test2::Util::Facets2Legacy>, see that module or
L<Test2::Event> for documentation on what they do.

=over 4

=item causes_fail

=item diagnostics

=item global

=item increments_count

=item no_display

=item sets_plan

=item subtest_id

=item summary

=item terminate

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
