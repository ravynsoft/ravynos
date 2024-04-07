package TAP::Parser::IteratorFactory;

use strict;
use warnings;

use Carp qw( confess );
use File::Basename qw( fileparse );

use base 'TAP::Object';

use constant handlers => [];

=head1 NAME

TAP::Parser::IteratorFactory - Figures out which SourceHandler objects to use for a given Source

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  use TAP::Parser::IteratorFactory;
  my $factory = TAP::Parser::IteratorFactory->new({ %config });
  my $iterator  = $factory->make_iterator( $filename );

=head1 DESCRIPTION

This is a factory class that takes a L<TAP::Parser::Source> and runs it through all the
registered L<TAP::Parser::SourceHandler>s to see which one should handle the source.

If you're a plugin author, you'll be interested in how to L</register_handler>s,
how L</detect_source> works.

=head1 METHODS

=head2 Class Methods

=head3 C<new>

Creates a new factory class:

  my $sf = TAP::Parser::IteratorFactory->new( $config );

C<$config> is optional.  If given, sets L</config> and calls L</load_handlers>.

=cut

sub _initialize {
    my ( $self, $config ) = @_;
    $self->config( $config || {} )->load_handlers;
    return $self;
}

=head3 C<register_handler>

Registers a new L<TAP::Parser::SourceHandler> with this factory.

  __PACKAGE__->register_handler( $handler_class );

=head3 C<handlers>

List of handlers that have been registered.

=cut

sub register_handler {
    my ( $class, $dclass ) = @_;

    confess("$dclass must implement can_handle & make_iterator methods!")
      unless UNIVERSAL::can( $dclass, 'can_handle' )
          && UNIVERSAL::can( $dclass, 'make_iterator' );

    my $handlers = $class->handlers;
    push @{$handlers}, $dclass
      unless grep { $_ eq $dclass } @{$handlers};

    return $class;
}

##############################################################################

=head2 Instance Methods

=head3 C<config>

 my $cfg = $sf->config;
 $sf->config({ Perl => { %config } });

Chaining getter/setter for the configuration of the available source handlers.
This is a hashref keyed on handler class whose values contain config to be passed
onto the handlers during detection & creation.  Class names may be fully qualified
or abbreviated, eg:

  # these are equivalent
  $sf->config({ 'TAP::Parser::SourceHandler::Perl' => { %config } });
  $sf->config({ 'Perl' => { %config } });

=cut

sub config {
    my $self = shift;
    return $self->{config} unless @_;
    unless ( 'HASH' eq ref $_[0] ) {
        $self->_croak('Argument to &config must be a hash reference');
    }
    $self->{config} = shift;
    return $self;
}

sub _last_handler {
    my $self = shift;
    return $self->{last_handler} unless @_;
    $self->{last_handler} = shift;
    return $self;
}

sub _testing {
    my $self = shift;
    return $self->{testing} unless @_;
    $self->{testing} = shift;
    return $self;
}

##############################################################################

=head3 C<load_handlers>

 $sf->load_handlers;

Loads the handler classes defined in L</config>.  For example, given a config:

  $sf->config({
    MySourceHandler => { some => 'config' },
  });

C<load_handlers> will attempt to load the C<MySourceHandler> class by looking in
C<@INC> for it in this order:

  TAP::Parser::SourceHandler::MySourceHandler
  MySourceHandler

C<croak>s on error.

=cut

sub load_handlers {
    my ($self) = @_;
    for my $handler ( keys %{ $self->config } ) {
        my $sclass = $self->_load_handler($handler);

        # TODO: store which class we loaded anywhere?
    }
    return $self;
}

sub _load_handler {
    my ( $self, $handler ) = @_;

    my @errors;
    for my $dclass ( "TAP::Parser::SourceHandler::$handler", $handler ) {
        return $dclass
          if UNIVERSAL::can( $dclass, 'can_handle' )
              && UNIVERSAL::can( $dclass, 'make_iterator' );

        eval "use $dclass";
        if ( my $e = $@ ) {
            push @errors, $e;
            next;
        }

        return $dclass
          if UNIVERSAL::can( $dclass, 'can_handle' )
              && UNIVERSAL::can( $dclass, 'make_iterator' );
        push @errors,
          "handler '$dclass' does not implement can_handle & make_iterator";
    }

    $self->_croak(
        "Cannot load handler '$handler': " . join( "\n", @errors ) );
}

##############################################################################

=head3 C<make_iterator>

  my $iterator = $src_factory->make_iterator( $source );

Given a L<TAP::Parser::Source>, finds the most suitable L<TAP::Parser::SourceHandler>
to use to create a L<TAP::Parser::Iterator> (see L</detect_source>).  Dies on error.

=cut

sub make_iterator {
    my ( $self, $source ) = @_;

    $self->_croak('no raw source defined!') unless defined $source->raw;

    $source->config( $self->config )->assemble_meta;

    # is the raw source already an object?
    return $source->raw
      if ( $source->meta->{is_object}
        && UNIVERSAL::isa( $source->raw, 'TAP::Parser::SourceHandler' ) );

    # figure out what kind of source it is
    my $sd_class = $self->detect_source($source);
    $self->_last_handler($sd_class);

    return if $self->_testing;

    # create it
    my $iterator = $sd_class->make_iterator($source);

    return $iterator;
}

=head3 C<detect_source>

Given a L<TAP::Parser::Source>, detects what kind of source it is and
returns I<one> L<TAP::Parser::SourceHandler> (the most confident one).  Dies
on error.

The detection algorithm works something like this:

  for (@registered_handlers) {
    # ask them how confident they are about handling this source
    $confidence{$handler} = $handler->can_handle( $source )
  }
  # choose the most confident handler

Ties are handled by choosing the first handler.

=cut

sub detect_source {
    my ( $self, $source ) = @_;

    confess('no raw source ref defined!') unless defined $source->raw;

    # find a list of handlers that can handle this source:
    my %confidence_for;
    for my $handler ( @{ $self->handlers } ) {
        my $confidence = $handler->can_handle($source);
        # warn "handler: $handler: $confidence\n";
        $confidence_for{$handler} = $confidence if $confidence;
    }

    if ( !%confidence_for ) {
        # error: can't detect source
        my $raw_source_short = substr( ${ $source->raw }, 0, 50 );
        confess("Cannot detect source of '$raw_source_short'!");
        return;
    }

    # if multiple handlers can handle it, choose the most confident one
    my @handlers =
          sort { $confidence_for{$b} <=> $confidence_for{$a} }
          keys %confidence_for;

    # Check for a tie.
    if( @handlers > 1 &&
        $confidence_for{$handlers[0]} == $confidence_for{$handlers[1]}
    ) {
        my $filename = $source->meta->{file}{basename};
        die("There is a tie between $handlers[0] and $handlers[1].\n".
            "Both voted $confidence_for{$handlers[0]} on $filename.\n");
    }

    # this is really useful for debugging handlers:
    if ( $ENV{TAP_HARNESS_SOURCE_FACTORY_VOTES} ) {
        warn(
            "votes: ",
            join( ', ', map {"$_: $confidence_for{$_}"} @handlers ),
            "\n"
        );
    }

    # return 1st
    return $handlers[0];
}

1;

__END__

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

=head2 Example

If we've done things right, you'll probably want to write a new source,
rather than sub-classing this (see L<TAP::Parser::SourceHandler> for that).

But in case you find the need to...

  package MyIteratorFactory;

  use strict;

  use base 'TAP::Parser::IteratorFactory';

  # override source detection algorithm
  sub detect_source {
    my ($self, $raw_source_ref, $meta) = @_;
    # do detective work, using $meta and whatever else...
  }

  1;

=head1 AUTHORS

Steve Purkis

=head1 ATTRIBUTION

Originally ripped off from L<Test::Harness>.

Moved out of L<TAP::Parser> & converted to a factory class to support
extensible TAP source detective work by Steve Purkis.

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::SourceHandler>,
L<TAP::Parser::SourceHandler::File>,
L<TAP::Parser::SourceHandler::Perl>,
L<TAP::Parser::SourceHandler::RawTAP>,
L<TAP::Parser::SourceHandler::Handle>,
L<TAP::Parser::SourceHandler::Executable>

=cut

