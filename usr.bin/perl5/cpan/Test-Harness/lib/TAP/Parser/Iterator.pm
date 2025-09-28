package TAP::Parser::Iterator;

use strict;
use warnings;

use base 'TAP::Object';

=head1 NAME

TAP::Parser::Iterator - Base class for TAP source iterators

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  # to subclass:
  use TAP::Parser::Iterator ();
  use base 'TAP::Parser::Iterator';
  sub _initialize {
    # see TAP::Object...
  }

  sub next_raw { ... }
  sub wait     { ... }
  sub exit     { ... }

=head1 DESCRIPTION

This is a simple iterator base class that defines L<TAP::Parser>'s iterator
API.  Iterators are typically created from L<TAP::Parser::SourceHandler>s.

=head1 METHODS

=head2 Class Methods

=head3 C<new>

Create an iterator.  Provided by L<TAP::Object>.

=head2 Instance Methods

=head3 C<next>

 while ( my $item = $iter->next ) { ... }

Iterate through it, of course.

=head3 C<next_raw>

B<Note:> this method is abstract and should be overridden.

 while ( my $item = $iter->next_raw ) { ... }

Iterate raw input without applying any fixes for quirky input syntax.

=cut

sub next {
    my $self = shift;
    my $line = $self->next_raw;

    # vms nit:  When encountering 'not ok', vms often has the 'not' on a line
    # by itself:
    #   not
    #   ok 1 - 'I hate VMS'
    if ( defined($line) and $line =~ /^\s*not\s*$/ ) {
        $line .= ( $self->next_raw || '' );
    }

    return $line;
}

sub next_raw {
    require Carp;
    my $msg = Carp::longmess('abstract method called directly!');
    $_[0]->_croak($msg);
}

=head3 C<handle_unicode>

If necessary switch the input stream to handle unicode. This only has
any effect for I/O handle based streams.

The default implementation does nothing.

=cut

sub handle_unicode { }

=head3 C<get_select_handles>

Return a list of filehandles that may be used upstream in a select()
call to signal that this Iterator is ready. Iterators that are not
handle-based should return an empty list.

The default implementation does nothing.

=cut

sub get_select_handles {
    return;
}

=head3 C<wait>

B<Note:> this method is abstract and should be overridden.

 my $wait_status = $iter->wait;

Return the C<wait> status for this iterator.

=head3 C<exit>

B<Note:> this method is abstract and should be overridden.

 my $wait_status = $iter->exit;

Return the C<exit> status for this iterator.

=cut

sub wait {
    require Carp;
    my $msg = Carp::longmess('abstract method called directly!');
    $_[0]->_croak($msg);
}

sub exit {
    require Carp;
    my $msg = Carp::longmess('abstract method called directly!');
    $_[0]->_croak($msg);
}

1;

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

You must override the abstract methods as noted above.

=head2 Example

L<TAP::Parser::Iterator::Array> is probably the easiest example to follow.
There's not much point repeating it here.

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::Iterator::Array>,
L<TAP::Parser::Iterator::Stream>,
L<TAP::Parser::Iterator::Process>,

=cut

