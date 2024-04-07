package TAP::Parser::SourceHandler;

use strict;
use warnings;

use TAP::Parser::Iterator ();
use base 'TAP::Object';

=head1 NAME

TAP::Parser::SourceHandler - Base class for different TAP source handlers

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  # abstract class - don't use directly!
  # see TAP::Parser::IteratorFactory for general usage

  # must be sub-classed for use
  package MySourceHandler;
  use base 'TAP::Parser::SourceHandler';
  sub can_handle    { return $confidence_level }
  sub make_iterator { return $iterator }

  # see example below for more details

=head1 DESCRIPTION

This is an abstract base class for L<TAP::Parser::Source> handlers / handlers.

A C<TAP::Parser::SourceHandler> does whatever is necessary to produce & capture
a stream of TAP from the I<raw> source, and package it up in a
L<TAP::Parser::Iterator> for the parser to consume.

C<SourceHandlers> must implement the I<source detection & handling> interface
used by L<TAP::Parser::IteratorFactory>.  At 2 methods, the interface is pretty
simple: L</can_handle> and L</make_source>.

Unless you're writing a new L<TAP::Parser::SourceHandler>, a plugin, or
subclassing L<TAP::Parser>, you probably won't need to use this module directly.

=head1 METHODS

=head2 Class Methods

=head3 C<can_handle>

I<Abstract method>.

  my $vote = $class->can_handle( $source );

C<$source> is a L<TAP::Parser::Source>.

Returns a number between C<0> & C<1> reflecting how confidently the raw source
can be handled.  For example, C<0> means the source cannot handle it, C<0.5>
means it may be able to, and C<1> means it definitely can.  See
L<TAP::Parser::IteratorFactory/detect_source> for details on how this is used.

=cut

sub can_handle {
    my ( $class, $args ) = @_;
    $class->_croak(
        "Abstract method 'can_handle' not implemented for $class!");
    return;
}

=head3 C<make_iterator>

I<Abstract method>.

  my $iterator = $class->make_iterator( $source );

C<$source> is a L<TAP::Parser::Source>.

Returns a new L<TAP::Parser::Iterator> object for use by the L<TAP::Parser>.
C<croak>s on error.

=cut

sub make_iterator {
    my ( $class, $args ) = @_;
    $class->_croak(
        "Abstract method 'make_iterator' not implemented for $class!");
    return;
}
1;

__END__

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview, and any
of the subclasses that ship with this module as an example.  What follows is
a quick overview.

Start by familiarizing yourself with L<TAP::Parser::Source> and
L<TAP::Parser::IteratorFactory>.  L<TAP::Parser::SourceHandler::RawTAP> is
the easiest sub-class to use as an example.

It's important to point out that if you want your subclass to be automatically
used by L<TAP::Parser> you'll have to and make sure it gets loaded somehow.
If you're using L<prove> you can write an L<App::Prove> plugin.  If you're
using L<TAP::Parser> or L<TAP::Harness> directly (e.g. through a custom script,
L<ExtUtils::MakeMaker>, or L<Module::Build>) you can use the C<config> option
which will cause L<TAP::Parser::IteratorFactory/load_sources> to load your
subclass).

Don't forget to register your class with
L<TAP::Parser::IteratorFactory/register_handler>.

=head2 Example

  package MySourceHandler;

  use strict;

  use MySourceHandler; # see TAP::Parser::SourceHandler
  use TAP::Parser::IteratorFactory;

  use base 'TAP::Parser::SourceHandler';

  TAP::Parser::IteratorFactory->register_handler( __PACKAGE__ );

  sub can_handle {
      my ( $class, $src ) = @_;
      my $meta   = $src->meta;
      my $config = $src->config_for( $class );

      if ($config->{accept_all}) {
          return 1.0;
      } elsif (my $file = $meta->{file}) {
          return 0.0 unless $file->{exists};
          return 1.0 if $file->{lc_ext} eq '.tap';
          return 0.9 if $file->{shebang} && $file->{shebang} =~ /^#!.+tap/;
          return 0.5 if $file->{text};
          return 0.1 if $file->{binary};
      } elsif ($meta->{scalar}) {
          return 0.8 if $$raw_source_ref =~ /\d\.\.\d/;
          return 0.6 if $meta->{has_newlines};
      } elsif ($meta->{array}) {
          return 0.8 if $meta->{size} < 5;
          return 0.6 if $raw_source_ref->[0] =~ /foo/;
          return 0.5;
      } elsif ($meta->{hash}) {
          return 0.6 if $raw_source_ref->{foo};
          return 0.2;
      }

      return 0;
  }

  sub make_iterator {
      my ($class, $source) = @_;
      # this is where you manipulate the source and
      # capture the stream of TAP in an iterator
      # either pick a TAP::Parser::Iterator::* or write your own...
      my $iterator = TAP::Parser::Iterator::Array->new([ 'foo', 'bar' ]);
      return $iterator;
  }

  1;

=head1 AUTHORS

TAPx Developers.

Source detection stuff added by Steve Purkis

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::Source>,
L<TAP::Parser::Iterator>,
L<TAP::Parser::IteratorFactory>,
L<TAP::Parser::SourceHandler::Executable>,
L<TAP::Parser::SourceHandler::Perl>,
L<TAP::Parser::SourceHandler::File>,
L<TAP::Parser::SourceHandler::Handle>,
L<TAP::Parser::SourceHandler::RawTAP>

=cut

