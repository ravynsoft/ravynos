package ExtUtils::Typemaps::Type;
use 5.006001;
use strict;
use warnings;
require ExtUtils::Typemaps;

our $VERSION = '3.51';

=head1 NAME

ExtUtils::Typemaps::Type - Entry in the TYPEMAP section of a typemap

=head1 SYNOPSIS

  use ExtUtils::Typemaps;
  ...
  my $type = $typemap->get_type_map('char*');
  my $input = $typemap->get_input_map($type->xstype);

=head1 DESCRIPTION

Refer to L<ExtUtils::Typemaps> for details.
Object associates C<ctype> with C<xstype>, which is the index
into the in- and output mapping tables.

=head1 METHODS

=cut

=head2 new

Requires C<xstype> and C<ctype> parameters.

Optionally takes C<prototype> parameter.

=cut

sub new {
  my $prot = shift;
  my $class = ref($prot)||$prot;
  my %args = @_;

  if (!ref($prot)) {
    if (not defined $args{xstype} or not defined $args{ctype}) {
      die("Need xstype and ctype parameters");
    }
  }

  my $self = bless(
    (ref($prot) ? {%$prot} : {proto => ''})
    => $class
  );

  $self->{xstype} = $args{xstype} if defined $args{xstype};
  $self->{ctype} = $args{ctype} if defined $args{ctype};
  $self->{tidy_ctype} = ExtUtils::Typemaps::tidy_type($self->{ctype});
  $self->{proto} = $args{'prototype'} if defined $args{'prototype'};

  return $self;
}

=head2 proto

Returns or sets the prototype.

=cut

sub proto {
  $_[0]->{proto} = $_[1] if @_ > 1;
  return $_[0]->{proto};
}

=head2 xstype

Returns the name of the XS type that this C type is associated to.

=cut

sub xstype {
  return $_[0]->{xstype};
}

=head2 ctype

Returns the name of the C type as it was set on construction.

=cut

sub ctype {
  return defined($_[0]->{ctype}) ? $_[0]->{ctype} : $_[0]->{tidy_ctype};
}

=head2 tidy_ctype

Returns the canonicalized name of the C type.

=cut

sub tidy_ctype {
  return $_[0]->{tidy_ctype};
}

=head1 SEE ALSO

L<ExtUtils::Typemaps>

=head1 AUTHOR

Steffen Mueller C<<smueller@cpan.org>>

=head1 COPYRIGHT & LICENSE

Copyright 2009, 2010, 2011, 2012 Steffen Mueller

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut

1;

