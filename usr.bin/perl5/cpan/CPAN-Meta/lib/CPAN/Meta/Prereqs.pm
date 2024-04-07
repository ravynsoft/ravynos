use 5.006;
use strict;
use warnings;
package CPAN::Meta::Prereqs;

our $VERSION = '2.150010';

#pod =head1 DESCRIPTION
#pod
#pod A CPAN::Meta::Prereqs object represents the prerequisites for a CPAN
#pod distribution or one of its optional features.  Each set of prereqs is
#pod organized by phase and type, as described in L<CPAN::Meta::Prereqs>.
#pod
#pod =cut

use Carp qw(confess);
use Scalar::Util qw(blessed);
use CPAN::Meta::Requirements 2.121;

#pod =method new
#pod
#pod   my $prereq = CPAN::Meta::Prereqs->new( \%prereq_spec );
#pod
#pod This method returns a new set of Prereqs.  The input should look like the
#pod contents of the C<prereqs> field described in L<CPAN::Meta::Spec>, meaning
#pod something more or less like this:
#pod
#pod   my $prereq = CPAN::Meta::Prereqs->new({
#pod     runtime => {
#pod       requires => {
#pod         'Some::Module' => '1.234',
#pod         ...,
#pod       },
#pod       ...,
#pod     },
#pod     ...,
#pod   });
#pod
#pod You can also construct an empty set of prereqs with:
#pod
#pod   my $prereqs = CPAN::Meta::Prereqs->new;
#pod
#pod This empty set of prereqs is useful for accumulating new prereqs before finally
#pod dumping the whole set into a structure or string.
#pod
#pod =cut

# note we also accept anything matching /\Ax_/i
sub __legal_phases { qw(configure build test runtime develop)   }
sub __legal_types  { qw(requires recommends suggests conflicts) }

# expect a prereq spec from META.json -- rjbs, 2010-04-11
sub new {
  my ($class, $prereq_spec) = @_;
  $prereq_spec ||= {};

  my %is_legal_phase = map {; $_ => 1 } $class->__legal_phases;
  my %is_legal_type  = map {; $_ => 1 } $class->__legal_types;

  my %guts;
  PHASE: for my $phase (keys %$prereq_spec) {
    next PHASE unless $phase =~ /\Ax_/i or $is_legal_phase{$phase};

    my $phase_spec = $prereq_spec->{ $phase };
    next PHASE unless keys %$phase_spec;

    TYPE: for my $type (keys %$phase_spec) {
      next TYPE unless $type =~ /\Ax_/i or $is_legal_type{$type};

      my $spec = $phase_spec->{ $type };

      next TYPE unless keys %$spec;

      $guts{prereqs}{$phase}{$type} = CPAN::Meta::Requirements->from_string_hash(
        $spec
      );
    }
  }

  return bless \%guts => $class;
}

#pod =method requirements_for
#pod
#pod   my $requirements = $prereqs->requirements_for( $phase, $type );
#pod
#pod This method returns a L<CPAN::Meta::Requirements> object for the given
#pod phase/type combination.  If no prerequisites are registered for that
#pod combination, a new CPAN::Meta::Requirements object will be returned, and it may
#pod be added to as needed.
#pod
#pod If C<$phase> or C<$type> are undefined or otherwise invalid, an exception will
#pod be raised.
#pod
#pod =cut

sub requirements_for {
  my ($self, $phase, $type) = @_;

  confess "requirements_for called without phase" unless defined $phase;
  confess "requirements_for called without type"  unless defined $type;

  unless ($phase =~ /\Ax_/i or grep { $phase eq $_ } $self->__legal_phases) {
    confess "requested requirements for unknown phase: $phase";
  }

  unless ($type =~ /\Ax_/i or grep { $type eq $_ } $self->__legal_types) {
    confess "requested requirements for unknown type: $type";
  }

  my $req = ($self->{prereqs}{$phase}{$type} ||= CPAN::Meta::Requirements->new);

  $req->finalize if $self->is_finalized;

  return $req;
}

#pod =method phases
#pod
#pod   my @phases = $prereqs->phases;
#pod
#pod This method returns the list of all phases currently populated in the prereqs
#pod object, suitable for iterating.
#pod
#pod =cut

sub phases {
  my ($self) = @_;

  my %is_legal_phase = map {; $_ => 1 } $self->__legal_phases;
  grep { /\Ax_/i or $is_legal_phase{$_} } keys %{ $self->{prereqs} };
}

#pod =method types_in
#pod
#pod   my @runtime_types = $prereqs->types_in('runtime');
#pod
#pod This method returns the list of all types currently populated in the prereqs
#pod object for the provided phase, suitable for iterating.
#pod
#pod =cut

sub types_in {
  my ($self, $phase) = @_;

  return unless $phase =~ /\Ax_/i or grep { $phase eq $_ } $self->__legal_phases;

  my %is_legal_type  = map {; $_ => 1 } $self->__legal_types;
  grep { /\Ax_/i or $is_legal_type{$_} } keys %{ $self->{prereqs}{$phase} };
}

#pod =method with_merged_prereqs
#pod
#pod   my $new_prereqs = $prereqs->with_merged_prereqs( $other_prereqs );
#pod
#pod   my $new_prereqs = $prereqs->with_merged_prereqs( \@other_prereqs );
#pod
#pod This method returns a new CPAN::Meta::Prereqs objects in which all the
#pod other prerequisites given are merged into the current set.  This is primarily
#pod provided for combining a distribution's core prereqs with the prereqs of one of
#pod its optional features.
#pod
#pod The new prereqs object has no ties to the originals, and altering it further
#pod will not alter them.
#pod
#pod =cut

sub with_merged_prereqs {
  my ($self, $other) = @_;

  my @other = blessed($other) ? $other : @$other;

  my @prereq_objs = ($self, @other);

  my %new_arg;

  for my $phase (__uniq(map { $_->phases } @prereq_objs)) {
    for my $type (__uniq(map { $_->types_in($phase) } @prereq_objs)) {

      my $req = CPAN::Meta::Requirements->new;

      for my $prereq (@prereq_objs) {
        my $this_req = $prereq->requirements_for($phase, $type);
        next unless $this_req->required_modules;

        $req->add_requirements($this_req);
      }

      next unless $req->required_modules;

      $new_arg{ $phase }{ $type } = $req->as_string_hash;
    }
  }

  return (ref $self)->new(\%new_arg);
}

#pod =method merged_requirements
#pod
#pod     my $new_reqs = $prereqs->merged_requirements( \@phases, \@types );
#pod     my $new_reqs = $prereqs->merged_requirements( \@phases );
#pod     my $new_reqs = $prereqs->merged_requirements();
#pod
#pod This method joins together all requirements across a number of phases
#pod and types into a new L<CPAN::Meta::Requirements> object.  If arguments
#pod are omitted, it defaults to "runtime", "build" and "test" for phases
#pod and "requires" and "recommends" for types.
#pod
#pod =cut

sub merged_requirements {
  my ($self, $phases, $types) = @_;
  $phases = [qw/runtime build test/] unless defined $phases;
  $types = [qw/requires recommends/] unless defined $types;

  confess "merged_requirements phases argument must be an arrayref"
    unless ref $phases eq 'ARRAY';
  confess "merged_requirements types argument must be an arrayref"
    unless ref $types eq 'ARRAY';

  my $req = CPAN::Meta::Requirements->new;

  for my $phase ( @$phases ) {
    unless ($phase =~ /\Ax_/i or grep { $phase eq $_ } $self->__legal_phases) {
        confess "requested requirements for unknown phase: $phase";
    }
    for my $type ( @$types ) {
      unless ($type =~ /\Ax_/i or grep { $type eq $_ } $self->__legal_types) {
          confess "requested requirements for unknown type: $type";
      }
      $req->add_requirements( $self->requirements_for($phase, $type) );
    }
  }

  $req->finalize if $self->is_finalized;

  return $req;
}


#pod =method as_string_hash
#pod
#pod This method returns a hashref containing structures suitable for dumping into a
#pod distmeta data structure.  It is made up of hashes and strings, only; there will
#pod be no Prereqs, CPAN::Meta::Requirements, or C<version> objects inside it.
#pod
#pod =cut

sub as_string_hash {
  my ($self) = @_;

  my %hash;

  for my $phase ($self->phases) {
    for my $type ($self->types_in($phase)) {
      my $req = $self->requirements_for($phase, $type);
      next unless $req->required_modules;

      $hash{ $phase }{ $type } = $req->as_string_hash;
    }
  }

  return \%hash;
}

#pod =method is_finalized
#pod
#pod This method returns true if the set of prereqs has been marked "finalized," and
#pod cannot be altered.
#pod
#pod =cut

sub is_finalized { $_[0]{finalized} }

#pod =method finalize
#pod
#pod Calling C<finalize> on a Prereqs object will close it for further modification.
#pod Attempting to make any changes that would actually alter the prereqs will
#pod result in an exception being thrown.
#pod
#pod =cut

sub finalize {
  my ($self) = @_;

  $self->{finalized} = 1;

  for my $phase (keys %{ $self->{prereqs} }) {
    $_->finalize for values %{ $self->{prereqs}{$phase} };
  }
}

#pod =method clone
#pod
#pod   my $cloned_prereqs = $prereqs->clone;
#pod
#pod This method returns a Prereqs object that is identical to the original object,
#pod but can be altered without affecting the original object.  Finalization does
#pod not survive cloning, meaning that you may clone a finalized set of prereqs and
#pod then modify the clone.
#pod
#pod =cut

sub clone {
  my ($self) = @_;

  my $clone = (ref $self)->new( $self->as_string_hash );
}

sub __uniq {
  my (%s, $u);
  grep { defined($_) ? !$s{$_}++ : !$u++ } @_;
}

1;

# ABSTRACT: a set of distribution prerequisites by phase and type

=pod

=encoding UTF-8

=head1 NAME

CPAN::Meta::Prereqs - a set of distribution prerequisites by phase and type

=head1 VERSION

version 2.150010

=head1 DESCRIPTION

A CPAN::Meta::Prereqs object represents the prerequisites for a CPAN
distribution or one of its optional features.  Each set of prereqs is
organized by phase and type, as described in L<CPAN::Meta::Prereqs>.

=head1 METHODS

=head2 new

  my $prereq = CPAN::Meta::Prereqs->new( \%prereq_spec );

This method returns a new set of Prereqs.  The input should look like the
contents of the C<prereqs> field described in L<CPAN::Meta::Spec>, meaning
something more or less like this:

  my $prereq = CPAN::Meta::Prereqs->new({
    runtime => {
      requires => {
        'Some::Module' => '1.234',
        ...,
      },
      ...,
    },
    ...,
  });

You can also construct an empty set of prereqs with:

  my $prereqs = CPAN::Meta::Prereqs->new;

This empty set of prereqs is useful for accumulating new prereqs before finally
dumping the whole set into a structure or string.

=head2 requirements_for

  my $requirements = $prereqs->requirements_for( $phase, $type );

This method returns a L<CPAN::Meta::Requirements> object for the given
phase/type combination.  If no prerequisites are registered for that
combination, a new CPAN::Meta::Requirements object will be returned, and it may
be added to as needed.

If C<$phase> or C<$type> are undefined or otherwise invalid, an exception will
be raised.

=head2 phases

  my @phases = $prereqs->phases;

This method returns the list of all phases currently populated in the prereqs
object, suitable for iterating.

=head2 types_in

  my @runtime_types = $prereqs->types_in('runtime');

This method returns the list of all types currently populated in the prereqs
object for the provided phase, suitable for iterating.

=head2 with_merged_prereqs

  my $new_prereqs = $prereqs->with_merged_prereqs( $other_prereqs );

  my $new_prereqs = $prereqs->with_merged_prereqs( \@other_prereqs );

This method returns a new CPAN::Meta::Prereqs objects in which all the
other prerequisites given are merged into the current set.  This is primarily
provided for combining a distribution's core prereqs with the prereqs of one of
its optional features.

The new prereqs object has no ties to the originals, and altering it further
will not alter them.

=head2 merged_requirements

    my $new_reqs = $prereqs->merged_requirements( \@phases, \@types );
    my $new_reqs = $prereqs->merged_requirements( \@phases );
    my $new_reqs = $prereqs->merged_requirements();

This method joins together all requirements across a number of phases
and types into a new L<CPAN::Meta::Requirements> object.  If arguments
are omitted, it defaults to "runtime", "build" and "test" for phases
and "requires" and "recommends" for types.

=head2 as_string_hash

This method returns a hashref containing structures suitable for dumping into a
distmeta data structure.  It is made up of hashes and strings, only; there will
be no Prereqs, CPAN::Meta::Requirements, or C<version> objects inside it.

=head2 is_finalized

This method returns true if the set of prereqs has been marked "finalized," and
cannot be altered.

=head2 finalize

Calling C<finalize> on a Prereqs object will close it for further modification.
Attempting to make any changes that would actually alter the prereqs will
result in an exception being thrown.

=head2 clone

  my $cloned_prereqs = $prereqs->clone;

This method returns a Prereqs object that is identical to the original object,
but can be altered without affecting the original object.  Finalization does
not survive cloning, meaning that you may clone a finalized set of prereqs and
then modify the clone.

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
