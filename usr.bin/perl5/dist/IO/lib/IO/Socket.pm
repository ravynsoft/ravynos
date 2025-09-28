
# IO::Socket.pm
#
# Copyright (c) 1997-8 Graham Barr <gbarr@pobox.com>. All rights reserved.
# This program is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself.

package IO::Socket;

use 5.008_001;

use IO::Handle;
use Socket 1.3;
use Carp;
use strict;
use Exporter;
use Errno;

# legacy

require IO::Socket::INET;
require IO::Socket::UNIX if ($^O ne 'epoc' && $^O ne 'symbian');

our @ISA = qw(IO::Handle);

our $VERSION = "1.52";

our @EXPORT_OK = qw(sockatmark);

our $errstr;

sub import {
    my $pkg = shift;
    if (@_ && $_[0] eq 'sockatmark') { # not very extensible but for now, fast
	Exporter::export_to_level('IO::Socket', 1, $pkg, 'sockatmark');
    } else {
	my $callpkg = caller;
	Exporter::export 'Socket', $callpkg, @_;
    }
}

sub new {
    my($class,%arg) = @_;
    my $sock = $class->SUPER::new();

    $sock->autoflush(1);

    ${*$sock}{'io_socket_timeout'} = delete $arg{Timeout};

    return scalar(%arg) ? $sock->configure(\%arg)
			: $sock;
}

my @domain2pkg;

sub register_domain {
    my($p,$d) = @_;
    $domain2pkg[$d] = $p;
}

sub configure {
    my($sock,$arg) = @_;
    my $domain = delete $arg->{Domain};

    croak 'IO::Socket: Cannot configure a generic socket'
	unless defined $domain;

    croak "IO::Socket: Unsupported socket domain"
	unless defined $domain2pkg[$domain];

    croak "IO::Socket: Cannot configure socket in domain '$domain'"
	unless ref($sock) eq "IO::Socket";

    bless($sock, $domain2pkg[$domain]);
    $sock->configure($arg);
}

sub socket {
    @_ == 4 or croak 'usage: $sock->socket(DOMAIN, TYPE, PROTOCOL)';
    my($sock,$domain,$type,$protocol) = @_;

    socket($sock,$domain,$type,$protocol) or
    	return undef;

    ${*$sock}{'io_socket_domain'} = $domain;
    ${*$sock}{'io_socket_type'}   = $type;

    # "A value of 0 for protocol will let the system select an
    # appropriate protocol"
    # so we need to look up what the system selected,
    # not cache PF_UNSPEC.
    ${*$sock}{'io_socket_proto'} = $protocol if $protocol;

    $sock;
}

sub socketpair {
    @_ == 4 || croak 'usage: IO::Socket->socketpair(DOMAIN, TYPE, PROTOCOL)';
    my($class,$domain,$type,$protocol) = @_;
    my $sock1 = $class->new();
    my $sock2 = $class->new();

    socketpair($sock1,$sock2,$domain,$type,$protocol) or
    	return ();

    ${*$sock1}{'io_socket_type'}  = ${*$sock2}{'io_socket_type'}  = $type;
    ${*$sock1}{'io_socket_proto'} = ${*$sock2}{'io_socket_proto'} = $protocol;

    ($sock1,$sock2);
}

sub connect {
    @_ == 2 or croak 'usage: $sock->connect(NAME)';
    my $sock = shift;
    my $addr = shift;
    my $timeout = ${*$sock}{'io_socket_timeout'};
    my $err;
    my $blocking;

    $blocking = $sock->blocking(0) if $timeout;
    if (!connect($sock, $addr)) {
	if (defined $timeout && ($!{EINPROGRESS} || $!{EWOULDBLOCK})) {
	    require IO::Select;

	    my $sel = IO::Select->new( $sock );

	    undef $!;
	    my($r,$w,$e) = IO::Select::select(undef,$sel,$sel,$timeout);
	    if(@$e[0]) {
		# Windows return from select after the timeout in case of
		# WSAECONNREFUSED(10061) if exception set is not used.
		# This behavior is different from Linux.
		# Using the exception
		# set we now emulate the behavior in Linux
		#    - Karthik Rajagopalan
		$err = $sock->getsockopt(SOL_SOCKET,SO_ERROR);
		$errstr = $@ = "connect: $err";
	    }
	    elsif(!@$w[0]) {
		$err = $! || (exists &Errno::ETIMEDOUT ? &Errno::ETIMEDOUT : 1);
		$errstr = $@ = "connect: timeout";
	    }
	    elsif (!connect($sock,$addr) &&
                not ($!{EISCONN} || ($^O eq 'MSWin32' &&
                ($! == (($] < 5.019004) ? 10022 : Errno::EINVAL))))
            ) {
		# Some systems refuse to re-connect() to
		# an already open socket and set errno to EISCONN.
		# Windows sets errno to WSAEINVAL (10022) (pre-5.19.4) or
		# EINVAL (22) (5.19.4 onwards).
		$err = $!;
		$errstr = $@ = "connect: $!";
	    }
	}
        elsif ($blocking || !($!{EINPROGRESS} || $!{EWOULDBLOCK}))  {
	    $err = $!;
	    $errstr = $@ = "connect: $!";
	}
    }

    $sock->blocking(1) if $blocking;

    $! = $err if $err;

    $err ? undef : $sock;
}

# Enable/disable blocking IO on sockets.
# Without args return the current status of blocking,
# with args change the mode as appropriate, returning the
# old setting, or in case of error during the mode change
# undef.

sub blocking {
    my $sock = shift;

    return $sock->SUPER::blocking(@_)
        if $^O ne 'MSWin32' && $^O ne 'VMS';

    # Windows handles blocking differently
    #
    # http://groups.google.co.uk/group/perl.perl5.porters/browse_thread/thread/b4e2b1d88280ddff/630b667a66e3509f?#630b667a66e3509f
    # http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/ioctlsocket_2.asp
    #
    # 0x8004667e is FIONBIO
    #
    # which is used to set blocking behaviour.

    # NOTE:
    # This is a little confusing, the perl keyword for this is
    # 'blocking' but the OS level behaviour is 'non-blocking', probably
    # because sockets are blocking by default.
    # Therefore internally we have to reverse the semantics.

    my $orig= !${*$sock}{io_sock_nonblocking};

    return $orig unless @_;

    my $block = shift;

    if ( !$block != !$orig ) {
        ${*$sock}{io_sock_nonblocking} = $block ? 0 : 1;
        ioctl($sock, 0x8004667e, pack("L!",${*$sock}{io_sock_nonblocking}))
            or return undef;
    }

    return $orig;
}


sub close {
    @_ == 1 or croak 'usage: $sock->close()';
    my $sock = shift;
    ${*$sock}{'io_socket_peername'} = undef;
    $sock->SUPER::close();
}

sub bind {
    @_ == 2 or croak 'usage: $sock->bind(NAME)';
    my $sock = shift;
    my $addr = shift;

    return bind($sock, $addr) ? $sock
			      : undef;
}

sub listen {
    @_ >= 1 && @_ <= 2 or croak 'usage: $sock->listen([QUEUE])';
    my($sock,$queue) = @_;
    $queue = 5
	unless $queue && $queue > 0;

    return listen($sock, $queue) ? $sock
				 : undef;
}

sub accept {
    @_ == 1 || @_ == 2 or croak 'usage $sock->accept([PKG])';
    my $sock = shift;
    my $pkg = shift || $sock;
    my $timeout = ${*$sock}{'io_socket_timeout'};
    my $new = $pkg->new(Timeout => $timeout);
    my $peer = undef;

    if(defined $timeout) {
	require IO::Select;

	my $sel = IO::Select->new( $sock );

	unless ($sel->can_read($timeout)) {
	    $errstr = $@ = 'accept: timeout';
	    $! = (exists &Errno::ETIMEDOUT ? &Errno::ETIMEDOUT : 1);
	    return;
	}
    }

    $peer = accept($new,$sock)
	or return;

    ${*$new}{$_} = ${*$sock}{$_} for qw( io_socket_domain io_socket_type io_socket_proto );

    return wantarray ? ($new, $peer)
    	      	     : $new;
}

sub sockname {
    @_ == 1 or croak 'usage: $sock->sockname()';
    getsockname($_[0]);
}

sub peername {
    @_ == 1 or croak 'usage: $sock->peername()';
    my($sock) = @_;
    ${*$sock}{'io_socket_peername'} ||= getpeername($sock);
}

sub connected {
    @_ == 1 or croak 'usage: $sock->connected()';
    my($sock) = @_;
    getpeername($sock);
}

sub send {
    @_ >= 2 && @_ <= 4 or croak 'usage: $sock->send(BUF, [FLAGS, [TO]])';
    my $sock  = $_[0];
    my $flags = $_[2] || 0;
    my $peer;

    if ($_[3]) {
        # the caller explicitly requested a TO, so use it
        # this is non-portable for "connected" UDP sockets
        $peer = $_[3];
    }
    elsif (!defined getpeername($sock)) {
        # we're not connected, so we require a peer from somewhere
        $peer = $sock->peername;

	croak 'send: Cannot determine peer address'
	    unless(defined $peer);
    }

    my $r = $peer
      ? send($sock, $_[1], $flags, $peer)
      : send($sock, $_[1], $flags);

    # remember who we send to, if it was successful
    ${*$sock}{'io_socket_peername'} = $peer
	if(@_ == 4 && defined $r);

    $r;
}

sub recv {
    @_ == 3 || @_ == 4 or croak 'usage: $sock->recv(BUF, LEN [, FLAGS])';
    my $sock  = $_[0];
    my $len   = $_[2];
    my $flags = $_[3] || 0;

    # remember who we recv'd from
    ${*$sock}{'io_socket_peername'} = recv($sock, $_[1]='', $len, $flags);
}

sub shutdown {
    @_ == 2 or croak 'usage: $sock->shutdown(HOW)';
    my($sock, $how) = @_;
    ${*$sock}{'io_socket_peername'} = undef;
    shutdown($sock, $how);
}

sub setsockopt {
    @_ == 4 or croak '$sock->setsockopt(LEVEL, OPTNAME, OPTVAL)';
    setsockopt($_[0],$_[1],$_[2],$_[3]);
}

my $intsize = length(pack("i",0));

sub getsockopt {
    @_ == 3 or croak '$sock->getsockopt(LEVEL, OPTNAME)';
    my $r = getsockopt($_[0],$_[1],$_[2]);
    # Just a guess
    $r = unpack("i", $r)
	if(defined $r && length($r) == $intsize);
    $r;
}

sub sockopt {
    my $sock = shift;
    @_ == 1 ? $sock->getsockopt(SOL_SOCKET,@_)
	    : $sock->setsockopt(SOL_SOCKET,@_);
}

sub atmark {
    @_ == 1 or croak 'usage: $sock->atmark()';
    my($sock) = @_;
    sockatmark($sock);
}

sub timeout {
    @_ == 1 || @_ == 2 or croak 'usage: $sock->timeout([VALUE])';
    my($sock,$val) = @_;
    my $r = ${*$sock}{'io_socket_timeout'};

    ${*$sock}{'io_socket_timeout'} = defined $val ? 0 + $val : $val
	if(@_ == 2);

    $r;
}

sub sockdomain {
    @_ == 1 or croak 'usage: $sock->sockdomain()';
    my $sock = shift;
    if (!defined(${*$sock}{'io_socket_domain'})) {
	my $addr = $sock->sockname();
	${*$sock}{'io_socket_domain'} = sockaddr_family($addr)
	    if (defined($addr));
    }
    ${*$sock}{'io_socket_domain'};
}

sub socktype {
    @_ == 1 or croak 'usage: $sock->socktype()';
    my $sock = shift;
    ${*$sock}{'io_socket_type'} = $sock->sockopt(Socket::SO_TYPE)
	if (!defined(${*$sock}{'io_socket_type'}) && defined(eval{Socket::SO_TYPE}));
    ${*$sock}{'io_socket_type'}
}

sub protocol {
    @_ == 1 or croak 'usage: $sock->protocol()';
    my($sock) = @_;
    ${*$sock}{'io_socket_proto'} = $sock->sockopt(Socket::SO_PROTOCOL)
	if (!defined(${*$sock}{'io_socket_proto'}) && defined(eval{Socket::SO_PROTOCOL}));
    ${*$sock}{'io_socket_proto'};
}

1;

__END__

=head1 NAME

IO::Socket - Object interface to socket communications

=head1 SYNOPSIS

    use strict;
    use warnings;

    use IO::Socket qw(AF_INET AF_UNIX);

    # create a new AF_INET socket
    my $sock = IO::Socket->new(Domain => AF_INET);
    # which is the same as
    $sock = IO::Socket::INET->new();

    # create a new AF_UNIX socket
    $sock = IO::Socket->new(Domain => AF_UNIX);
    # which is the same as
    $sock = IO::Socket::UNIX->new();

=head1 DESCRIPTION

C<IO::Socket> provides an object-oriented, L<IO::Handle>-based interface to
creating and using sockets via L<Socket>, which provides a near one-to-one
interface to the C socket library.

C<IO::Socket> is a base class that really only defines methods for those
operations which are common to all types of sockets. Operations which are
specific to a particular socket domain have methods defined in subclasses of
C<IO::Socket>. See L<IO::Socket::INET>, L<IO::Socket::UNIX>, and
L<IO::Socket::IP> for examples of such a subclass.

C<IO::Socket> will export all functions (and constants) defined by L<Socket>.

=head1 CONSTRUCTOR ARGUMENTS

Given that C<IO::Socket> doesn't have attributes in the traditional sense, the
following arguments, rather than attributes, can be passed into the
constructor.

Constructor arguments should be passed in C<< Key => 'Value' >> pairs.

The only required argument is L<IO::Socket/"Domain">.

=head2 Blocking

    my $sock = IO::Socket->new(..., Blocking => 1);
    $sock = IO::Socket->new(..., Blocking => 0);

If defined but false, the socket will be set to non-blocking mode. If not
specified it defaults to C<1> (blocking mode).

=head2 Domain

    my $sock = IO::Socket->new(Domain => IO::Socket::AF_INET);
    $sock = IO::Socket->new(Domain => IO::Socket::AF_UNIX);

The socket domain will define which subclass of C<IO::Socket> to use. The two
options available along with this distribution are C<AF_INET> and C<AF_UNIX>.

C<AF_INET> is for the internet address family of sockets and is handled via
L<IO::Socket::INET>. C<AF_INET> sockets are bound to an internet address and
port.

C<AF_UNIX> is for the unix domain socket and is handled via
L<IO::Socket::UNIX>. C<AF_UNIX> sockets are bound to the file system as their
address name space.

This argument is B<required>. All other arguments are optional.

=head2 Listen

    my $sock = IO::Socket->new(..., Listen => 5);

Listen should be an integer value or left unset.

If provided, this argument will place the socket into listening mode. New
connections can then be accepted using the L<IO::Socket/"accept"> method. The
value given is used as the C<listen(2)> queue size.

If the C<Listen> argument is given, but false, the queue size will be set to
5.

=head2 Timeout

    my $sock = IO::Socket->new(..., Timeout => 5);

The timeout value, in seconds, for this socket connection. How exactly this
value is utilized is defined in the socket domain subclasses that make use of
the value.

=head2 Type

    my $sock = IO::Socket->new(..., Type => IO::Socket::SOCK_STREAM);

The socket type that will be used. These are usually C<SOCK_STREAM>,
C<SOCK_DGRAM>, or C<SOCK_RAW>. If this argument is left undefined an attempt
will be made to infer the type from the service name.

For example, you'll usually use C<SOCK_STREAM> with a C<tcp> connection and
C<SOCK_DGRAM> with a C<udp> connection.

=head1 CONSTRUCTORS

C<IO::Socket> extends the L<IO::Handle> constructor.

=head2 new

    my $sock = IO::Socket->new();

    # get a new IO::Socket::INET instance
    $sock = IO::Socket->new(Domain => IO::Socket::AF_INET);
    # get a new IO::Socket::UNIX instance
    $sock = IO::Socket->new(Domain => IO::Socket::AF_UNIX);

    # Domain is the only required argument
    $sock = IO::Socket->new(
        Domain => IO::Socket::AF_INET, # AF_INET, AF_UNIX
        Type => IO::Socket::SOCK_STREAM, # SOCK_STREAM, SOCK_DGRAM, ...
        Proto => 'tcp', # 'tcp', 'udp', IPPROTO_TCP, IPPROTO_UDP
        # and so on...
    );

Creates an C<IO::Socket>, which is a reference to a newly created symbol (see
the L<Symbol> package). C<new> optionally takes arguments, these arguments
are defined in L<IO::Socket/"CONSTRUCTOR ARGUMENTS">.

Any of the L<IO::Socket/"CONSTRUCTOR ARGUMENTS"> may be passed to the
constructor, but if any arguments are provided, then one of them must be
the L<IO::Socket/"Domain"> argument. The L<IO::Socket/"Domain"> argument can,
by default, be either C<AF_INET> or C<AF_UNIX>. Other domains can be used if a
proper subclass for the domain family is registered. All other arguments will
be passed to the C<configuration> method of the package for that domain.

If the constructor fails it will return C<undef> and set the C<$errstr> package
variable to contain an error message.

    $sock = IO::Socket->new(...)
        or die "Cannot create socket - $IO::Socket::errstr\n";

For legacy reasons the error message is also set into the global C<$@>
variable, and you may still find older code which looks here instead.

    $sock = IO::Socket->new(...)
        or die "Cannot create socket - $@\n";

=head1 METHODS

C<IO::Socket> inherits all methods from L<IO::Handle> and implements the
following new ones.

=head2 accept

    my $client_sock = $sock->accept();
    my $inet_sock = $sock->accept('IO::Socket::INET');

The accept method will perform the system call C<accept> on the socket and
return a new object. The new object will be created in the same class as the
listen socket, unless a specific package name is specified. This object can be
used to communicate with the client that was trying to connect.

This differs slightly from the C<accept> function in L<perlfunc>.

In a scalar context the new socket is returned, or C<undef> upon
failure. In a list context a two-element array is returned containing
the new socket and the peer address; the list will be empty upon failure.

=head2 atmark

    my $integer = $sock->atmark();
    # read in some data on a given socket
    my $data;
    $sock->read($data, 1024) until $sock->atmark;

    # or, export the function to use:
    use IO::Socket 'sockatmark';
    $sock->read($data, 1024) until sockatmark($sock);

True if the socket is currently positioned at the urgent data mark, false
otherwise. If your system doesn't yet implement C<sockatmark> this will throw
an exception.

If your system does not support C<sockatmark>, the C<use> declaration will
fail at compile time.

=head2 autoflush

    # by default, autoflush will be turned on when referenced
    $sock->autoflush(); # turns on autoflush
    # turn off autoflush
    $sock->autoflush(0);
    # turn on autoflush
    $sock->autoflush(1);

This attribute isn't overridden from L<IO::Handle>'s implementation. However,
since we turn it on by default, it's worth mentioning here.

=head2 bind

    use Socket qw(pack_sockaddr_in);
    my $port = 3000;
    my $ip_address = '0.0.0.0';
    my $packed_addr = pack_sockaddr_in($port, $ip_address);
    $sock->bind($packed_addr);

Binds a network address to a socket, just as C<bind(2)> does. Returns true if
it succeeded, false otherwise. You should provide a packed address of the
appropriate type for the socket.

=head2 connected

    my $peer_addr = $sock->connected();
    if ($peer_addr) {
        say "We're connected to $peer_addr";
    }

If the socket is in a connected state, the peer address is returned. If the
socket is not in a connected state, C<undef> is returned.

Note that this method considers a half-open TCP socket to be "in a connected
state".  Specifically, it does not distinguish between the
B<ESTABLISHED> and B<CLOSE-WAIT> TCP states; it returns the peer address,
rather than C<undef>, in either case.  Thus, in general, it cannot
be used to reliably learn whether the peer has initiated a graceful shutdown
because in most cases (see below) the local TCP state machine remains in
B<CLOSE-WAIT> until the local application calls L<IO::Socket/"shutdown"> or
C<close>. Only at that point does this function return C<undef>.

The "in most cases" hedge is because local TCP state machine behavior may
depend on the peer's socket options. In particular, if the peer socket has
C<SO_LINGER> enabled with a zero timeout, then the peer's C<close> will
generate a C<RST> segment. Upon receipt of that segment, the local TCP
transitions immediately to B<CLOSED>, and in that state, this method I<will>
return C<undef>.

=head2 getsockopt

    my $value = $sock->getsockopt(SOL_SOCKET, SO_REUSEADDR);
    my $buf = $socket->getsockopt(SOL_SOCKET, SO_RCVBUF);
    say "Receive buffer is $buf bytes";

Get an option associated with the socket. Levels other than C<SOL_SOCKET>
may be specified here. As a convenience, this method will unpack a byte buffer
of the correct size back into a number.

=head2 listen

    $sock->listen(5);

Does the same thing that the C<listen(2)> system call does. Returns true if it
succeeded, false otherwise. Listens to a socket with a given queue size.

=head2 peername

    my $sockaddr_in = $sock->peername();

Returns the packed C<sockaddr> address of the other end of the socket
connection. It calls C<getpeername>.


=head2 protocol

    my $proto = $sock->protocol();

Returns the number for the protocol being used on the socket, if
known. If the protocol is unknown, as with an C<AF_UNIX> socket, zero
is returned.

=head2 recv

    my $buffer = "";
    my $length = 1024;
    my $flags = 0; # default. optional
    $sock->recv($buffer, $length);
    $sock->recv($buffer, $length, $flags);

Similar in functionality to L<perlfunc/recv>.

Receives a message on a socket. Attempts to receive C<$length> characters of
data into C<$buffer> from the specified socket. C<$buffer> will be grown or
shrunk to the length actually read. Takes the same flags as the system call of
the same name. Returns the address of the sender if socket's protocol supports
this; returns an empty string otherwise. If there's an error, returns
C<undef>. This call is actually implemented in terms of the C<recvfrom(2)>
system call.

Flags are ORed together values, such as C<MSG_BCAST>, C<MSG_OOB>,
C<MSG_TRUNC>. The default value for the flags is C<0>.

The cached value of L<IO::Socket/"peername"> is updated with the result of
C<recv>.

B<Note:> In Perl v5.30 and newer, if the socket has been marked as C<:utf8>,
C<recv> will throw an exception. The C<:encoding(...)> layer implicitly
introduces the C<:utf8> layer. See L<perlfunc/binmode>.

B<Note:> In Perl versions older than v5.30, depending on the status of the
socket, either (8-bit) bytes or characters are received. By default all
sockets operate on bytes, but for example if the socket has been changed
using L<perlfunc/binmode> to operate with the C<:encoding(UTF-8)> I/O layer
(see the L<perlfunc/open> pragma), the I/O will operate on UTF8-encoded
Unicode characters, not bytes. Similarly for the C<:encoding> layer: in
that case pretty much any characters can be read.

=head2 send

    my $message = "Hello, world!";
    my $flags = 0; # defaults to zero
    my $to = '0.0.0.0'; # optional destination
    my $sent = $sock->send($message);
    $sent = $sock->send($message, $flags);
    $sent = $sock->send($message, $flags, $to);

Similar in functionality to L<perlfunc/send>.

Sends a message on a socket. Attempts to send the scalar message to the
socket. Takes the same flags as the system call of the same name. On
unconnected sockets, you must specify a destination to send to, in which case
it does a C<sendto(2)> syscall. Returns the number of characters sent, or
C<undef> on error. The C<sendmsg(2)> syscall is currently unimplemented.

The C<flags> option is optional and defaults to C<0>.

After a successful send with C<$to>, further calls to C<send> on an
unconnected socket without C<$to> will send to the same address, and C<$to>
will be used as the result of L<IO::Socket/"peername">.

B<Note:> In Perl v5.30 and newer, if the socket has been marked as C<:utf8>,
C<send> will throw an exception. The C<:encoding(...)> layer implicitly
introduces the C<:utf8> layer. See L<perlfunc/binmode>.

B<Note:> In Perl versions older than v5.30, depending on the status of the
socket, either (8-bit) bytes or characters are sent. By default all
sockets operate on bytes, but for example if the socket has been changed
using L<perlfunc/binmode> to operate with the C<:encoding(UTF-8)> I/O layer
(see the L<perlfunc/open> pragma), the I/O will operate on UTF8-encoded
Unicode characters, not bytes. Similarly for the C<:encoding> layer: in
that case pretty much any characters can be sent.

=head2 setsockopt

    $sock->setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
    $sock->setsockopt(SOL_SOCKET, SO_RCVBUF, 64*1024);

Set option associated with the socket. Levels other than C<SOL_SOCKET>
may be specified here. As a convenience, this method will convert a number
into a packed byte buffer.

=head2 shutdown

    $sock->shutdown(SHUT_RD); # we stopped reading data
    $sock->shutdown(SHUT_WR); # we stopped writing data
    $sock->shutdown(SHUT_RDWR); # we stopped using this socket

Shuts down a socket connection in the manner indicated by the value passed in,
which has the same interpretation as in the syscall of the same name.

This is useful with sockets when you want to tell the other side you're done
writing but not done reading, or vice versa. It's also a more insistent form
of C<close> because it also disables the file descriptor in any
forked copies in other processes.

Returns C<1> for success; on error, returns C<undef> if the socket is
not a valid filehandle, or returns C<0> and sets C<$!> for any other failure.

=head2 sockdomain

    my $domain = $sock->sockdomain();

Returns the number for the socket domain type. For example, for
an C<AF_INET> socket the value of C<&AF_INET> will be returned.

=head2 socket

    my $sock = IO::Socket->new(); # no values given
    # now let's actually get a socket with the socket method
    # domain, type, and protocol are required
    $sock = $sock->socket(AF_INET, SOCK_STREAM, 'tcp');

Opens a socket of the specified kind and returns it. Domain, type, and
protocol are specified the same as for the syscall of the same name.

=head2 socketpair

    my ($r, $w) = $sock->socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC);
    ($r, $w) = IO::Socket::UNIX
        ->socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC);

Will return a list of two sockets created (read and write), or an empty list
on failure.

Differs slightly from C<socketpair> in L<perlfunc> in that the argument list
is a bit simpler.

=head2 sockname

    my $packed_addr = $sock->sockname();

Returns the packed C<sockaddr> address of this end of the connection. It's the
same as C<getsockname(2)>.

=head2 sockopt

    my $value = $sock->sockopt(SO_REUSEADDR);
    $sock->sockopt(SO_REUSEADDR, 1);

Unified method to both set and get options in the C<SOL_SOCKET> level. If
called with one argument then L<IO::Socket/"getsockopt"> is called, otherwise
L<IO::Socket/"setsockopt"> is called.

=head2 socktype

    my $type = $sock->socktype();

Returns the number for the socket type. For example, for
a C<SOCK_STREAM> socket the value of C<&SOCK_STREAM> will be returned.

=head2 timeout

    my $seconds = $sock->timeout();
    my $old_val = $sock->timeout(5); # set new and return old value

Set or get the timeout value (in seconds) associated with this socket.
If called without any arguments then the current setting is returned. If
called with an argument the current setting is changed and the previous
value returned.

This method is available to all C<IO::Socket> implementations but may or may
not be used by the individual domain subclasses.

=head1 EXAMPLES

Let's create a TCP server on C<localhost:3333>.

    use strict;
    use warnings;
    use feature 'say';

    use IO::Socket qw(AF_INET AF_UNIX SOCK_STREAM SHUT_WR);

    my $server = IO::Socket->new(
        Domain => AF_INET,
        Type => SOCK_STREAM,
        Proto => 'tcp',
        LocalHost => '0.0.0.0',
        LocalPort => 3333,
        ReusePort => 1,
        Listen => 5,
    ) || die "Can't open socket: $IO::Socket::errstr";
    say "Waiting on 3333";

    while (1) {
        # waiting for a new client connection
        my $client = $server->accept();

        # get information about a newly connected client
        my $client_address = $client->peerhost();
        my $client_port = $client->peerport();
        say "Connection from $client_address:$client_port";

        # read up to 1024 characters from the connected client
        my $data = "";
        $client->recv($data, 1024);
        say "received data: $data";

        # write response data to the connected client
        $data = "ok";
        $client->send($data);

        # notify client that response has been sent
        $client->shutdown(SHUT_WR);
    }

    $server->close();

A client for such a server could be

    use strict;
    use warnings;
    use feature 'say';

    use IO::Socket qw(AF_INET AF_UNIX SOCK_STREAM SHUT_WR);

    my $client = IO::Socket->new(
        Domain => AF_INET,
        Type => SOCK_STREAM,
        proto => 'tcp',
        PeerPort => 3333,
        PeerHost => '0.0.0.0',
    ) || die "Can't open socket: $IO::Socket::errstr";

    say "Sending Hello World!";
    my $size = $client->send("Hello World!");
    say "Sent data of length: $size";

    $client->shutdown(SHUT_WR);

    my $buffer;
    $client->recv($buffer, 1024);
    say "Got back $buffer";

    $client->close();


=head1 LIMITATIONS

On some systems, for an IO::Socket object created with C<new_from_fd>,
or created with L<IO::Socket/"accept"> from such an object, the
L<IO::Socket/"protocol">, L<IO::Socket/"sockdomain"> and
L<IO::Socket/"socktype"> methods may return C<undef>.

=head1 SEE ALSO

L<Socket>, L<IO::Handle>, L<IO::Socket::INET>, L<IO::Socket::UNIX>,
L<IO::Socket::IP>

=head1 AUTHOR

Graham Barr.  atmark() by Lincoln Stein.  Currently maintained by the Perl 5
Porters.  Please report all bugs at L<https://github.com/Perl/perl5/issues>.

=head1 COPYRIGHT

Copyright (c) 1997-8 Graham Barr <gbarr@pobox.com>. All rights reserved.
This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

The atmark() implementation: Copyright 2001, Lincoln Stein <lstein@cshl.org>.
This module is distributed under the same terms as Perl itself.
Feel free to use, modify and redistribute it as long as you retain
the correct attribution.

=cut
