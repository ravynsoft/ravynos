package TAP::Parser::SourceHandler::File;

use strict;
use warnings;

use TAP::Parser::IteratorFactory  ();
use TAP::Parser::Iterator::Stream ();

use base 'TAP::Parser::SourceHandler';

TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);

=head1 NAME

TAP::Parser::SourceHandler::File - Stream TAP from a text file.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  use TAP::Parser::Source;
  use TAP::Parser::SourceHandler::File;

  my $source = TAP::Parser::Source->new->raw( \'file.tap' );
  $source->assemble_meta;

  my $class = 'TAP::Parser::SourceHandler::File';
  my $vote  = $class->can_handle( $source );
  my $iter  = $class->make_iterator( $source );

=head1 DESCRIPTION

This is a I<raw TAP stored in a file> L<TAP::Parser::SourceHandler> - it has 2 jobs:

1. Figure out if the I<raw> source it's given is a file containing raw TAP
output.  See L<TAP::Parser::IteratorFactory> for more details.

2. Takes raw TAP from the text file given, and converts into an iterator.

Unless you're writing a plugin or subclassing L<TAP::Parser>, you probably
won't need to use this module directly.

=head1 METHODS

=head2 Class Methods

=head3 C<can_handle>

  my $vote = $class->can_handle( $source );

Only votes if $source looks like a regular file.  Casts the following votes:

  0.9 if it's a .tap file
  0.9 if it has an extension matching any given in user config.

=cut

sub can_handle {
    my ( $class, $src ) = @_;
    my $meta   = $src->meta;
    my $config = $src->config_for($class);

    return 0 unless $meta->{is_file};
    my $file = $meta->{file};
    return 0.9 if $file->{lc_ext} eq '.tap';

    if ( my $exts = $config->{extensions} ) {
        my @exts = ref $exts eq 'ARRAY' ? @$exts : $exts;
        return 0.9 if grep { lc($_) eq $file->{lc_ext} } @exts;
    }

    return 0;
}

=head3 C<make_iterator>

  my $iterator = $class->make_iterator( $source );

Returns a new L<TAP::Parser::Iterator::Stream> for the source.  C<croak>s
on error.

=cut

sub make_iterator {
    my ( $class, $source ) = @_;

    $class->_croak('$source->raw must be a scalar ref')
      unless $source->meta->{is_scalar};

    my $file = ${ $source->raw };
    my $fh;
    open( $fh, '<', $file )
      or $class->_croak("error opening TAP source file '$file': $!");
    return $class->iterator_class->new($fh);
}

=head3 C<iterator_class>

The class of iterator to use, override if you're sub-classing.  Defaults
to L<TAP::Parser::Iterator::Stream>.

=cut

use constant iterator_class => 'TAP::Parser::Iterator::Stream';

1;

__END__

=head1 CONFIGURATION

  {
   extensions => [ @case_insensitive_exts_to_match ]
  }

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::SourceHandler>,
L<TAP::Parser::SourceHandler::Executable>,
L<TAP::Parser::SourceHandler::Perl>,
L<TAP::Parser::SourceHandler::Handle>,
L<TAP::Parser::SourceHandler::RawTAP>

=cut
