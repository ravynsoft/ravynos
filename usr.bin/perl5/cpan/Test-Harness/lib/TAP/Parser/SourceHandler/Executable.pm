package TAP::Parser::SourceHandler::Executable;

use strict;
use warnings;

use TAP::Parser::IteratorFactory   ();
use TAP::Parser::Iterator::Process ();

use base 'TAP::Parser::SourceHandler';

TAP::Parser::IteratorFactory->register_handler(__PACKAGE__);

=head1 NAME

TAP::Parser::SourceHandler::Executable - Stream output from an executable TAP source

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  use TAP::Parser::Source;
  use TAP::Parser::SourceHandler::Executable;

  my $source = TAP::Parser::Source->new->raw(['/usr/bin/ruby', 'mytest.rb']);
  $source->assemble_meta;

  my $class = 'TAP::Parser::SourceHandler::Executable';
  my $vote  = $class->can_handle( $source );
  my $iter  = $class->make_iterator( $source );

=head1 DESCRIPTION

This is an I<executable> L<TAP::Parser::SourceHandler> - it has 2 jobs:

1. Figure out if the L<TAP::Parser::Source> it's given is an executable
   command (L</can_handle>).

2. Creates an iterator for executable commands (L</make_iterator>).

Unless you're writing a plugin or subclassing L<TAP::Parser>, you
probably won't need to use this module directly.

=head1 METHODS

=head2 Class Methods

=head3 C<can_handle>

  my $vote = $class->can_handle( $source );

Only votes if $source looks like an executable file. Casts the
following votes:

  0.9  if it's a hash with an 'exec' key
  0.8  if it's a .bat file
  0.75 if it's got an execute bit set

=cut

sub can_handle {
    my ( $class, $src ) = @_;
    my $meta = $src->meta;

    if ( $meta->{is_file} ) {
        my $file = $meta->{file};

        return 0.85 if $file->{execute} && $file->{binary};
        return 0.8 if $file->{lc_ext} eq '.bat';
        return 0.25 if $file->{execute};
    }
    elsif ( $meta->{is_hash} ) {
        return 0.9 if $src->raw->{exec};
    }

    return 0;
}

=head3 C<make_iterator>

  my $iterator = $class->make_iterator( $source );

Returns a new L<TAP::Parser::Iterator::Process> for the source.
C<$source-E<gt>raw> must be in one of the following forms:

  { exec => [ @exec ] }

  [ @exec ]

  $file

C<croak>s on error.

=cut

sub make_iterator {
    my ( $class, $source ) = @_;
    my $meta = $source->meta;

    my @command;
    if ( $meta->{is_hash} ) {
        @command = @{ $source->raw->{exec} || [] };
    }
    elsif ( $meta->{is_scalar} ) {
        @command = ${ $source->raw };
    }
    elsif ( $meta->{is_array} ) {
        @command = @{ $source->raw };
    }

    $class->_croak('No command found in $source->raw!') unless @command;

    $class->_autoflush( \*STDOUT );
    $class->_autoflush( \*STDERR );

    push @command, @{ $source->test_args || [] };

    return $class->iterator_class->new(
        {   command => \@command,
            merge   => $source->merge
        }
    );
}

=head3 C<iterator_class>

The class of iterator to use, override if you're sub-classing.  Defaults
to L<TAP::Parser::Iterator::Process>.

=cut

use constant iterator_class => 'TAP::Parser::Iterator::Process';

# Turns on autoflush for the handle passed
sub _autoflush {
    my ( $class, $flushed ) = @_;
    my $old_fh = select $flushed;
    $| = 1;
    select $old_fh;
}

1;

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

=head2 Example

  package MyRubySourceHandler;

  use strict;

  use Carp qw( croak );
  use TAP::Parser::SourceHandler::Executable;

  use base 'TAP::Parser::SourceHandler::Executable';

  # expect $handler->(['mytest.rb', 'cmdline', 'args']);
  sub make_iterator {
    my ($self, $source) = @_;
    my @test_args = @{ $source->test_args };
    my $rb_file   = $test_args[0];
    croak("error: Ruby file '$rb_file' not found!") unless (-f $rb_file);
    return $self->SUPER::raw_source(['/usr/bin/ruby', @test_args]);
  }

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::IteratorFactory>,
L<TAP::Parser::SourceHandler>,
L<TAP::Parser::SourceHandler::Perl>,
L<TAP::Parser::SourceHandler::File>,
L<TAP::Parser::SourceHandler::Handle>,
L<TAP::Parser::SourceHandler::RawTAP>

=cut
