package Test2::EventFacet::Meta;
use strict;
use warnings;

our $VERSION = '1.302194';

BEGIN { require Test2::EventFacet; our @ISA = qw(Test2::EventFacet) }
use vars qw/$AUTOLOAD/;

# replace set_details
{
    no warnings 'redefine';
    sub set_details { $_[0]->{'set_details'} }
}

sub can {
    my $self = shift;
    my ($name) = @_;

    my $existing = $self->SUPER::can($name);
    return $existing if $existing;

    # Only vivify when called on an instance, do not vivify for a class. There
    # are a lot of magic class methods used in things like serialization (or
    # the forks.pm module) which cause problems when vivified.
    return undef unless ref($self);

    my $sub = sub { $_[0]->{$name} };
    {
        no strict 'refs';
        *$name = $sub;
    }

    return $sub;
}

sub AUTOLOAD {
    my $name = $AUTOLOAD;
    $name =~ s/^.*:://g;
    my $sub = $_[0]->can($name);
    goto &$sub;
}

1;

__END__

=pod

=encoding UTF-8

=head1 NAME

Test2::EventFacet::Meta - Facet for meta-data

=head1 DESCRIPTION

This facet can contain any random meta-data that has been attached to the
event.

=head1 METHODS AND FIELDS

Any/all fields and accessors are autovivified into existence. There is no way
to know what metadata may be added, so any is allowed.

=over 4

=item $anything = $meta->{anything}

=item $anything = $meta->anything()

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
