package TAP::Parser::SourceHandler::RawTAP;

use strict;
use warnings;

use TAP::Parser::IteratorFactory ();
use TAP::Parser::Iterator::Array ();

use base 'TAP::Parser::SourceHandler';

TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);

=head1 NAME

TAP::Parser::SourceHandler::RawTAP - Stream output from raw TAP in a scalar/array ref.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  use TAP::Parser::Source;
  use TAP::Parser::SourceHandler::RawTAP;

  my $source = TAP::Parser::Source->new->raw( \"1..1\nok 1\n" );
  $source->assemble_meta;

  my $class = 'TAP::Parser::SourceHandler::RawTAP';
  my $vote  = $class->can_handle( $source );
  my $iter  = $class->make_iterator( $source );

=head1 DESCRIPTION

This is a I<raw TAP output> L<TAP::Parser::SourceHandler> - it has 2 jobs:

1. Figure out if the L<TAP::Parser::Source> it's given is raw TAP output
(L</can_handle>).

2. Creates an iterator for raw TAP output (L</make_iterator>).

Unless you're writing a plugin or subclassing L<TAP::Parser>, you probably
won't need to use this module directly.

=head1 METHODS

=head2 Class Methods

=head3 C<can_handle>

  my $vote = $class->can_handle( $source );

Only votes if $source is an array, or a scalar with newlines.  Casts the
following votes:

  0.9  if it's a scalar with '..' in it
  0.7  if it's a scalar with 'ok' in it
  0.3  if it's just a scalar with newlines
  0.5  if it's an array

=cut

sub can_handle {
    my ( $class, $src ) = @_;
    my $meta = $src->meta;

    return 0 if $meta->{file};
    if ( $meta->{is_scalar} ) {
        return 0 unless $meta->{has_newlines};
        return 0.9 if ${ $src->raw } =~ /\d\.\.\d/;
        return 0.7 if ${ $src->raw } =~ /ok/;
        return 0.3;
    }
    elsif ( $meta->{is_array} ) {
        return 0.5;
    }
    return 0;
}

=head3 C<make_iterator>

  my $iterator = $class->make_iterator( $source );

Returns a new L<TAP::Parser::Iterator::Array> for the source.
C<$source-E<gt>raw> must be an array ref, or a scalar ref.

C<croak>s on error.

=cut

sub make_iterator {
    my ( $class, $src ) = @_;
    my $meta = $src->meta;

    my $tap_array;
    if ( $meta->{is_scalar} ) {
        $tap_array = [ split "\n" => ${ $src->raw } ];
    }
    elsif ( $meta->{is_array} ) {
        $tap_array = $src->raw;
    }

    $class->_croak('No raw TAP found in $source->raw')
      unless scalar $tap_array;

    return TAP::Parser::Iterator::Array->new($tap_array);
}

1;

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::IteratorFactory>,
L<TAP::Parser::SourceHandler>,
L<TAP::Parser::SourceHandler::Executable>,
L<TAP::Parser::SourceHandler::Perl>,
L<TAP::Parser::SourceHandler::File>,
L<TAP::Parser::SourceHandler::Handle>

=cut
