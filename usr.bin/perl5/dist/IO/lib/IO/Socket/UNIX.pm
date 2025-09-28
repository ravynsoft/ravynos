# IO::Socket::UNIX.pm
#
# Copyright (c) 1997-8 Graham Barr <gbarr@pobox.com>. All rights reserved.
# This program is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself.

package IO::Socket::UNIX;

use strict;
use IO::Socket;
use Carp;

our @ISA = qw(IO::Socket);
our $VERSION = "1.52";

IO::Socket::UNIX->register_domain( AF_UNIX );

sub new {
    my $class = shift;
    unshift(@_, "Peer") if @_ == 1;
    return $class->SUPER::new(@_);
}

sub configure {
    my($sock,$arg) = @_;
    my($bport,$cport);

    my $type = $arg->{Type} || SOCK_STREAM;

    $sock->socket(AF_UNIX, $type, 0) or
	return undef;

    if(exists $arg->{Blocking}) {
        $sock->blocking($arg->{Blocking}) or
	    return undef;
    }
    if(exists $arg->{Local}) {
	my $addr = sockaddr_un($arg->{Local});
	$sock->bind($addr) or
	    return undef;
    }
    if(exists $arg->{Listen} && $type != SOCK_DGRAM) {
	$sock->listen($arg->{Listen} || 5) or
	    return undef;
    }
    elsif(exists $arg->{Peer}) {
	my $addr = sockaddr_un($arg->{Peer});
	$sock->connect($addr) or
	    return undef;
    }

    $sock;
}

sub hostpath {
    @_ == 1 or croak 'usage: $sock->hostpath()';
    my $n = $_[0]->sockname || return undef;
    (sockaddr_un($n))[0];
}

sub peerpath {
    @_ == 1 or croak 'usage: $sock->peerpath()';
    my $n = $_[0]->peername || return undef;
    (sockaddr_un($n))[0];
}

1; # Keep require happy

__END__

=head1 NAME

IO::Socket::UNIX - Object interface for AF_UNIX domain sockets

=head1 SYNOPSIS

    use IO::Socket::UNIX;

    my $SOCK_PATH = "$ENV{HOME}/unix-domain-socket-test.sock";

    # Server:
    my $server = IO::Socket::UNIX->new(
        Type => SOCK_STREAM(),
        Local => $SOCK_PATH,
        Listen => 1,
    );

    my $count = 1;
    while (my $conn = $server->accept()) {
        $conn->print("Hello " . ($count++) . "\n");
    }

    # Client:
    my $client = IO::Socket::UNIX->new(
        Type => SOCK_STREAM(),
        Peer => $SOCK_PATH,
    );

    # Now read and write from $client

=head1 DESCRIPTION

C<IO::Socket::UNIX> provides an object interface to creating and using sockets
in the AF_UNIX domain. It is built upon the L<IO::Socket> interface and
inherits all the methods defined by L<IO::Socket>.

=head1 CONSTRUCTOR

=over 4

=item new ( [ARGS] )

Creates an C<IO::Socket::UNIX> object, which is a reference to a
newly created symbol (see the L<Symbol> package). C<new>
optionally takes arguments, these arguments are in key-value pairs.

In addition to the key-value pairs accepted by L<IO::Socket>,
C<IO::Socket::UNIX> provides.

    Type    	Type of socket (eg SOCK_STREAM or SOCK_DGRAM)
    Local   	Path to local fifo
    Peer    	Path to peer fifo
    Listen  	Queue size for listen

If the constructor is only passed a single argument, it is assumed to
be a C<Peer> specification.

If the C<Listen> argument is given, but false, the queue size will be set to 5.

If the constructor fails it will return C<undef> and set the
C<$IO::Socket::errstr> package variable to contain an error message.

    $sock = IO::Socket::UNIX->new(...)
        or die "Cannot create socket - $IO::Socket::errstr\n";

For legacy reasons the error message is also set into the global C<$@>
variable, and you may still find older code which looks here instead.

    $sock = IO::Socket::UNIX->new(...)
        or die "Cannot create socket - $@\n";

=back

=head1 METHODS

=over 4

=item hostpath()

Returns the pathname to the fifo at the local end

=item peerpath()

Returns the pathanme to the fifo at the peer end

=back

=head1 SEE ALSO

L<Socket>, L<IO::Socket>

=head1 AUTHOR

Graham Barr. Currently maintained by the Perl Porters.  Please report all
bugs at L<https://github.com/Perl/perl5/issues>.

=head1 COPYRIGHT

Copyright (c) 1996-8 Graham Barr <gbarr@pobox.com>. All rights reserved.
This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut
