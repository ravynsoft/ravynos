package TAP::Parser::Result;

use strict;
use warnings;

use base 'TAP::Object';

BEGIN {

    # make is_* methods
    my @attrs = qw( plan pragma test comment bailout version unknown yaml );
    no strict 'refs';
    for my $token (@attrs) {
        my $method = "is_$token";
        *$method = sub { return $token eq shift->type };
    }
}

##############################################################################

=head1 NAME

TAP::Parser::Result - Base class for TAP::Parser output objects

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  # abstract class - not meant to be used directly
  # see TAP::Parser::ResultFactory for preferred usage

  # directly:
  use TAP::Parser::Result;
  my $token  = {...};
  my $result = TAP::Parser::Result->new( $token );

=head2 DESCRIPTION

This is a simple base class used by L<TAP::Parser> to store objects that
represent the current bit of test output data from TAP (usually a single
line).  Unless you're subclassing, you probably won't need to use this module
directly.

=head2 METHODS

=head3 C<new>

  # see TAP::Parser::ResultFactory for preferred usage

  # to use directly:
  my $result = TAP::Parser::Result->new($token);

Returns an instance the appropriate class for the test token passed in.

=cut

# new() implementation provided by TAP::Object

sub _initialize {
    my ( $self, $token ) = @_;
    if ($token) {

       # assign to a hash slice to make a shallow copy of the token.
       # I guess we could assign to the hash as (by default) there are not
       # contents, but that seems less helpful if someone wants to subclass us
        @{$self}{ keys %$token } = values %$token;
    }
    return $self;
}

##############################################################################

=head2 Boolean methods

The following methods all return a boolean value and are to be overridden in
the appropriate subclass.

=over 4

=item * C<is_plan>

Indicates whether or not this is the test plan line.

 1..3

=item * C<is_pragma>

Indicates whether or not this is a pragma line.

 pragma +strict

=item * C<is_test>

Indicates whether or not this is a test line.

 ok 1 Is OK!

=item * C<is_comment>

Indicates whether or not this is a comment.

 # this is a comment

=item * C<is_bailout>

Indicates whether or not this is bailout line.

 Bail out! We're out of dilithium crystals.

=item * C<is_version>

Indicates whether or not this is a TAP version line.

 TAP version 4

=item * C<is_unknown>

Indicates whether or not the current line could be parsed.

 ... this line is junk ...

=item * C<is_yaml>

Indicates whether or not this is a YAML chunk.

=back

=cut

##############################################################################

=head3 C<raw>

  print $result->raw;

Returns the original line of text which was parsed.

=cut

sub raw { shift->{raw} }

##############################################################################

=head3 C<type>

  my $type = $result->type;

Returns the "type" of a token, such as C<comment> or C<test>.

=cut

sub type { shift->{type} }

##############################################################################

=head3 C<as_string>

  print $result->as_string;

Prints a string representation of the token.  This might not be the exact
output, however.  Tests will have test numbers added if not present, TODO and
SKIP directives will be capitalized and, in general, things will be cleaned
up.  If you need the original text for the token, see the C<raw> method.

=cut

sub as_string { shift->{raw} }

##############################################################################

=head3 C<is_ok>

  if ( $result->is_ok ) { ... }

Reports whether or not a given result has passed.  Anything which is B<not> a
test result returns true.  This is merely provided as a convenient shortcut.

=cut

sub is_ok {1}

##############################################################################

=head3 C<passed>

Deprecated.  Please use C<is_ok> instead.

=cut

sub passed {
    warn 'passed() is deprecated.  Please use "is_ok()"';
    shift->is_ok;
}

##############################################################################

=head3 C<has_directive>

  if ( $result->has_directive ) {
     ...
  }

Indicates whether or not the given result has a TODO or SKIP directive.

=cut

sub has_directive {
    my $self = shift;
    return ( $self->has_todo || $self->has_skip );
}

##############################################################################

=head3 C<has_todo>

 if ( $result->has_todo ) {
     ...
 }

Indicates whether or not the given result has a TODO directive.

=cut

sub has_todo { 'TODO' eq ( shift->{directive} || '' ) }

##############################################################################

=head3 C<has_skip>

 if ( $result->has_skip ) {
     ...
 }

Indicates whether or not the given result has a SKIP directive.

=cut

sub has_skip { 'SKIP' eq ( shift->{directive} || '' ) }

=head3 C<set_directive>

Set the directive associated with this token. Used internally to fake
TODO tests.

=cut

sub set_directive {
    my ( $self, $dir ) = @_;
    $self->{directive} = $dir;
}

1;

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

Remember: if you want your subclass to be automatically used by the parser,
you'll have to register it with L<TAP::Parser::ResultFactory/register_type>.

If you're creating a completely new result I<type>, you'll probably need to
subclass L<TAP::Parser::Grammar> too, or else it'll never get used.

=head2 Example

  package MyResult;

  use strict;

  use base 'TAP::Parser::Result';

  # register with the factory:
  TAP::Parser::ResultFactory->register_type( 'my_type' => __PACKAGE__ );

  sub as_string { 'My results all look the same' }

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::ResultFactory>,
L<TAP::Parser::Result::Bailout>,
L<TAP::Parser::Result::Comment>,
L<TAP::Parser::Result::Plan>,
L<TAP::Parser::Result::Pragma>,
L<TAP::Parser::Result::Test>,
L<TAP::Parser::Result::Unknown>,
L<TAP::Parser::Result::Version>,
L<TAP::Parser::Result::YAML>,

=cut
