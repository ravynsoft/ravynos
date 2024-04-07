package TAP::Parser::ResultFactory;

use strict;
use warnings;

use TAP::Parser::Result::Bailout ();
use TAP::Parser::Result::Comment ();
use TAP::Parser::Result::Plan    ();
use TAP::Parser::Result::Pragma  ();
use TAP::Parser::Result::Test    ();
use TAP::Parser::Result::Unknown ();
use TAP::Parser::Result::Version ();
use TAP::Parser::Result::YAML    ();

use base 'TAP::Object';

##############################################################################

=head1 NAME

TAP::Parser::ResultFactory - Factory for creating TAP::Parser output objects

=head1 SYNOPSIS

  use TAP::Parser::ResultFactory;
  my $token   = {...};
  my $factory = TAP::Parser::ResultFactory->new;
  my $result  = $factory->make_result( $token );

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head2 DESCRIPTION

This is a simple factory class which returns a L<TAP::Parser::Result> subclass
representing the current bit of test data from TAP (usually a single line).
It is used primarily by L<TAP::Parser::Grammar>.  Unless you're subclassing,
you probably won't need to use this module directly.

=head2 METHODS

=head2 Class Methods

=head3 C<new>

Creates a new factory class.
I<Note:> You currently don't need to instantiate a factory in order to use it.

=head3 C<make_result>

Returns an instance the appropriate class for the test token passed in.

  my $result = TAP::Parser::ResultFactory->make_result($token);

Can also be called as an instance method.

=cut

sub make_result {
    my ( $proto, $token ) = @_;
    my $type = $token->{type};
    return $proto->class_for($type)->new($token);
}

=head3 C<class_for>

Takes one argument: C<$type>.  Returns the class for this $type, or C<croak>s
with an error.

=head3 C<register_type>

Takes two arguments: C<$type>, C<$class>

This lets you override an existing type with your own custom type, or register
a completely new type, eg:

  # create a custom result type:
  package MyResult;
  use strict;
  use base 'TAP::Parser::Result';

  # register with the factory:
  TAP::Parser::ResultFactory->register_type( 'my_type' => __PACKAGE__ );

  # use it:
  my $r = TAP::Parser::ResultFactory->( { type => 'my_type' } );

Your custom type should then be picked up automatically by the L<TAP::Parser>.

=cut

our %CLASS_FOR = (
	plan    => 'TAP::Parser::Result::Plan',
	pragma  => 'TAP::Parser::Result::Pragma',
	test    => 'TAP::Parser::Result::Test',
	comment => 'TAP::Parser::Result::Comment',
	bailout => 'TAP::Parser::Result::Bailout',
	version => 'TAP::Parser::Result::Version',
	unknown => 'TAP::Parser::Result::Unknown',
	yaml    => 'TAP::Parser::Result::YAML',
);

sub class_for {
    my ( $class, $type ) = @_;

    # return target class:
    return $CLASS_FOR{$type} if exists $CLASS_FOR{$type};

    # or complain:
    require Carp;
    Carp::croak("Could not determine class for result type '$type'");
}

sub register_type {
    my ( $class, $type, $rclass ) = @_;

    # register it blindly, assume they know what they're doing
    $CLASS_FOR{$type} = $rclass;
    return $class;
}

1;

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

There are a few things to bear in mind when creating your own
C<ResultFactory>:

=over 4

=item 1

The factory itself is never instantiated (this I<may> change in the future).
This means that C<_initialize> is never called.

=item 2

C<TAP::Parser::Result-E<gt>new> is never called, $tokens are reblessed.
This I<will> change in a future version!

=item 3

L<TAP::Parser::Result> subclasses will register themselves with
L<TAP::Parser::ResultFactory> directly:

  package MyFooResult;
  TAP::Parser::ResultFactory->register_type( foo => __PACKAGE__ );

Of course, it's up to you to decide whether or not to ignore them.

=back

=head2 Example

  package MyResultFactory;

  use strict;

  use MyResult;

  use base 'TAP::Parser::ResultFactory';

  # force all results to be 'MyResult'
  sub class_for {
    return 'MyResult';
  }

  1;

=head1 SEE ALSO

L<TAP::Parser>,
L<TAP::Parser::Result>,
L<TAP::Parser::Grammar>

=cut
