use 5.006;
use strict;
use warnings;
package CPAN::Meta::Feature;

our $VERSION = '2.150010';

use CPAN::Meta::Prereqs;

#pod =head1 DESCRIPTION
#pod
#pod A CPAN::Meta::Feature object describes an optional feature offered by a CPAN
#pod distribution and specified in the distribution's F<META.json> (or F<META.yml>)
#pod file.
#pod
#pod For the most part, this class will only be used when operating on the result of
#pod the C<feature> or C<features> methods on a L<CPAN::Meta> object.
#pod
#pod =method new
#pod
#pod   my $feature = CPAN::Meta::Feature->new( $identifier => \%spec );
#pod
#pod This returns a new Feature object.  The C<%spec> argument to the constructor
#pod should be the same as the value of the C<optional_feature> entry in the
#pod distmeta.  It must contain entries for C<description> and C<prereqs>.
#pod
#pod =cut

sub new {
  my ($class, $identifier, $spec) = @_;

  my %guts = (
    identifier  => $identifier,
    description => $spec->{description},
    prereqs     => CPAN::Meta::Prereqs->new($spec->{prereqs}),
  );

  bless \%guts => $class;
}

#pod =method identifier
#pod
#pod This method returns the feature's identifier.
#pod
#pod =cut

sub identifier  { $_[0]{identifier}  }

#pod =method description
#pod
#pod This method returns the feature's long description.
#pod
#pod =cut

sub description { $_[0]{description} }

#pod =method prereqs
#pod
#pod This method returns the feature's prerequisites as a L<CPAN::Meta::Prereqs>
#pod object.
#pod
#pod =cut

sub prereqs     { $_[0]{prereqs} }

1;

# ABSTRACT: an optional feature provided by a CPAN distribution

=pod

=encoding UTF-8

=head1 NAME

CPAN::Meta::Feature - an optional feature provided by a CPAN distribution

=head1 VERSION

version 2.150010

=head1 DESCRIPTION

A CPAN::Meta::Feature object describes an optional feature offered by a CPAN
distribution and specified in the distribution's F<META.json> (or F<META.yml>)
file.

For the most part, this class will only be used when operating on the result of
the C<feature> or C<features> methods on a L<CPAN::Meta> object.

=head1 METHODS

=head2 new

  my $feature = CPAN::Meta::Feature->new( $identifier => \%spec );

This returns a new Feature object.  The C<%spec> argument to the constructor
should be the same as the value of the C<optional_feature> entry in the
distmeta.  It must contain entries for C<description> and C<prereqs>.

=head2 identifier

This method returns the feature's identifier.

=head2 description

This method returns the feature's long description.

=head2 prereqs

This method returns the feature's prerequisites as a L<CPAN::Meta::Prereqs>
object.

=head1 BUGS

Please report any bugs or feature using the CPAN Request Tracker.
Bugs can be submitted through the web interface at
L<http://rt.cpan.org/Dist/Display.html?Queue=CPAN-Meta>

When submitting a bug or request, please include a test-file or a patch to an
existing test-file that illustrates the bug or desired feature.

=head1 AUTHORS

=over 4

=item *

David Golden <dagolden@cpan.org>

=item *

Ricardo Signes <rjbs@cpan.org>

=item *

Adam Kennedy <adamk@cpan.org>

=back

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2010 by David Golden, Ricardo Signes, Adam Kennedy and Contributors.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut

__END__


# vim: ts=2 sts=2 sw=2 et :
