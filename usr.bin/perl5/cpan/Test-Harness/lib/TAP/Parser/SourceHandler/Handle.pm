package TAP::Parser::SourceHandler::Handle;

use strict;
use warnings;

use TAP::Parser::IteratorFactory  ();
use TAP::Parser::Iterator::Stream ();

use base 'TAP::Parser::SourceHandler';

TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);

=head1 NAME

TAP::Parser::SourceHandler::Handle - Stream TAP from an IO::Handle or a GLOB.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  use TAP::Parser::Source;
  use TAP::Parser::SourceHandler::Executable;

  my $source = TAP::Parser::Source->new->raw( \*TAP_FILE );
  $source->assemble_meta;

  my $class = 'TAP::Parser::SourceHandler::Handle';
  my $vote  = $class->can_handle( $source );
  my $iter  = $class->make_iterator( $source );

=head1 DESCRIPTION

This is a I<raw TAP stored in an IO Handle> L<TAP::Parser::SourceHandler> class.  It
has 2 jobs:

1. Figure out if the L<TAP::Parser::Source> it's given is an L<IO::Handle> or
GLOB containing raw TAP output (L</can_handle>).

2. Creates an iterator for IO::Handle's & globs (L</make_iterator>).

Unless you're writing a plugin or subclassing L<TAP::Parser>, you probably
won't need to use this module directly.

=head1 METHODS

=head2 Class Methods

=head3 C<can_handle>

  my $vote = $class->can_handle( $source );

Casts the following votes:

  0.9 if $source is an IO::Handle
  0.8 if $source is a glob

=cut

sub can_handle {
    my ( $class, $src ) = @_;
    my $meta = $src->meta;

    return 0.9
      if $meta->{is_object}
          && UNIVERSAL::isa( $src->raw, 'IO::Handle' );

    return 0.8 if $meta->{is_glob};

    return 0;
}

=head3 C<make_iterator>

  my $iterator = $class->make_iterator( $source );

Returns a new L<TAP::Parser::Iterator::Stream> for the source.

=cut

sub make_iterator {
    my ( $class, $source ) = @_;

    $class->_croak('$source->raw must be a glob ref or an IO::Handle')
      unless $source->meta->{is_glob}
          || UNIVERSAL::isa( $source->raw, 'IO::Handle' );

    return $class->iterator_class->new( $source->raw );
}

=head3 C<iterator_class>

The class of iterator to use, override if you're sub-classing.  Defaults
to L<TAP::Parser::Iterator::Stream>.

=cut

use constant iterator_class => 'TAP::Parser::Iterator::Stream';

1;

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::Iterator>,
L<TAP::Parser::Iterator::Stream>,
L<TAP::Parser::IteratorFactory>,
L<TAP::Parser::SourceHandler>,
L<TAP::Parser::SourceHandler::Executable>,
L<TAP::Parser::SourceHandler::Perl>,
L<TAP::Parser::SourceHandler::File>,
L<TAP::Parser::SourceHandler::RawTAP>

=cut
