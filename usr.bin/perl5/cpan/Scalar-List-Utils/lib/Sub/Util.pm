# Copyright (c) 2014 Paul Evans <leonerd@leonerd.org.uk>. All rights reserved.
# This program is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself.

package Sub::Util;

use strict;
use warnings;

require Exporter;

our @ISA = qw( Exporter );
our @EXPORT_OK = qw(
  prototype set_prototype
  subname set_subname
);

our $VERSION    = "1.63";
$VERSION =~ tr/_//d;

require List::Util; # as it has the XS
List::Util->VERSION( $VERSION ); # Ensure we got the right XS version (RT#100863)

=head1 NAME

Sub::Util - A selection of utility subroutines for subs and CODE references

=head1 SYNOPSIS

    use Sub::Util qw( prototype set_prototype subname set_subname );

=head1 DESCRIPTION

C<Sub::Util> contains a selection of utility subroutines that are useful for
operating on subs and CODE references.

The rationale for inclusion in this module is that the function performs some
work for which an XS implementation is essential because it cannot be
implemented in Pure Perl, and which is sufficiently-widely used across CPAN
that its popularity warrants inclusion in a core module, which this is.

=cut

=head1 FUNCTIONS

=cut

=head2 prototype

    my $proto = prototype( $code )

I<Since version 1.40.>

Returns the prototype of the given C<$code> reference, if it has one, as a
string. This is the same as the C<CORE::prototype> operator; it is included
here simply for symmetry and completeness with the other functions.

=cut

sub prototype
{
  my ( $code ) = @_;
  return CORE::prototype( $code );
}

=head2 set_prototype

    my $code = set_prototype $prototype, $code;

I<Since version 1.40.>

Sets the prototype of the function given by the C<$code> reference, or deletes
it if C<$prototype> is C<undef>. Returns the C<$code> reference itself.

I<Caution>: This function takes arguments in a different order to the previous
copy of the code from C<Scalar::Util>. This is to match the order of
C<set_subname>, and other potential additions in this file. This order has
been chosen as it allows a neat and simple chaining of other
C<Sub::Util::set_*> functions as might become available, such as:

 my $code =
    set_subname   name_here =>
    set_prototype '&@'      =>
    set_attribute ':lvalue' =>
       sub { ...... };

=cut

=head2 subname

    my $name = subname( $code )

I<Since version 1.40.>

Returns the name of the given C<$code> reference, if it has one. Normal named
subs will give a fully-qualified name consisting of the package and the
localname separated by C<::>. Anonymous code references will give C<__ANON__>
as the localname. If the package the code was compiled in has been deleted
(e.g. using C<delete_package> from L<Symbol>), C<__ANON__> will be returned as
the package name. If a name has been set using L</set_subname>, this name will be
returned instead.

This function was inspired by C<sub_fullname> from L<Sub::Identify>. The
remaining functions that C<Sub::Identify> implements can easily be emulated
using regexp operations, such as

 sub get_code_info { return (subname $_[0]) =~ m/^(.+)::(.*?)$/ }
 sub sub_name      { return (get_code_info $_[0])[0] }
 sub stash_name    { return (get_code_info $_[0])[1] }

I<Users of Sub::Name beware>: This function is B<not> the same as
C<Sub::Name::subname>; it returns the existing name of the sub rather than
changing it. To set or change a name, see instead L</set_subname>.

=cut

=head2 set_subname

    my $code = set_subname $name, $code;

I<Since version 1.40.>

Sets the name of the function given by the C<$code> reference. Returns the
C<$code> reference itself. If the C<$name> is unqualified, the package of the
caller is used to qualify it.

This is useful for applying names to anonymous CODE references so that stack
traces and similar situations, to give a useful name rather than having the
default of C<__ANON__>. Note that this name is only used for this situation;
the C<set_subname> will not install it into the symbol table; you will have to
do that yourself if required.

However, since the name is not used by perl except as the return value of
C<caller>, for stack traces or similar, there is no actual requirement that
the name be syntactically valid as a perl function name. This could be used to
attach extra information that could be useful in debugging stack traces.

This function was copied from C<Sub::Name::subname> and renamed to the naming
convention of this module.

=cut

=head1 AUTHOR

The general structure of this module was written by Paul Evans
<leonerd@leonerd.org.uk>.

The XS implementation of L</set_subname> was copied from L<Sub::Name> by
Matthijs van Duin <xmath@cpan.org>

=cut

1;
