package TAP::Parser::Iterator::Stream;

use strict;
use warnings;

use base 'TAP::Parser::Iterator';

=head1 NAME

TAP::Parser::Iterator::Stream - Iterator for filehandle-based TAP sources

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  use TAP::Parser::Iterator::Stream;
  open( TEST, 'test.tap' );
  my $it   = TAP::Parser::Iterator::Stream->new(\*TEST);
  my $line = $it->next;

=head1 DESCRIPTION

This is a simple iterator wrapper for reading from filehandles, used by
L<TAP::Parser>.  Unless you're writing a plugin or subclassing, you probably
won't need to use this module directly.

=head1 METHODS

=head2 Class Methods

=head3 C<new>

Create an iterator.  Expects one argument containing a filehandle.

=cut

# new() implementation supplied by TAP::Object

sub _initialize {
    my ( $self, $thing ) = @_;
    $self->{fh} = $thing;
    return $self;
}

=head2 Instance Methods

=head3 C<next>

Iterate through it, of course.

=head3 C<next_raw>

Iterate raw input without applying any fixes for quirky input syntax.

=head3 C<wait>

Get the wait status for this iterator. Always returns zero.

=head3 C<exit>

Get the exit status for this iterator. Always returns zero.

=cut

sub wait { shift->exit }
sub exit { shift->{fh} ? () : 0 }

sub next_raw {
    my $self = shift;
    my $fh   = $self->{fh};

    if ( defined( my $line = <$fh> ) ) {
        chomp $line;
        return $line;
    }
    else {
        $self->_finish;
        return;
    }
}

sub _finish {
    my $self = shift;
    close delete $self->{fh};
}

sub get_select_handles {
    my $self = shift;

    # return our handle in case it's a socket or pipe (select()-able)
    return ( $self->{fh}, )
        if (-S $self->{fh} || -p $self->{fh});

    return;
}

1;

=head1 ATTRIBUTION

Originally ripped off from L<Test::Harness>.

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::Iterator>,

=cut

