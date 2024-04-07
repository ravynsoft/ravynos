package Socket;

use strict;
{ use v5.6.1; }

our $VERSION = '2.036';

=head1 NAME

C<Socket> - networking constants and support functions

=head1 SYNOPSIS

C<Socket> a low-level module used by, among other things, the L<IO::Socket>
family of modules. The following examples demonstrate some low-level uses but
a practical program would likely use the higher-level API provided by
C<IO::Socket> or similar instead.

 use Socket qw(PF_INET SOCK_STREAM pack_sockaddr_in inet_aton);

 socket(my $socket, PF_INET, SOCK_STREAM, 0)
     or die "socket: $!";

 my $port = getservbyname "echo", "tcp";
 connect($socket, pack_sockaddr_in($port, inet_aton("localhost")))
     or die "connect: $!";

 print $socket "Hello, world!\n";
 print <$socket>;

See also the L</EXAMPLES> section.

=head1 DESCRIPTION

This module provides a variety of constants, structure manipulators and other
functions related to socket-based networking. The values and functions
provided are useful when used in conjunction with Perl core functions such as
socket(), setsockopt() and bind(). It also provides several other support
functions, mostly for dealing with conversions of network addresses between
human-readable and native binary forms, and for hostname resolver operations.

Some constants and functions are exported by default by this module; but for
backward-compatibility any recently-added symbols are not exported by default
and must be requested explicitly. When an import list is provided to the
C<use Socket> line, the default exports are not automatically imported. It is
therefore best practice to always to explicitly list all the symbols required.

Also, some common socket "newline" constants are provided: the constants
C<CR>, C<LF>, and C<CRLF>, as well as C<$CR>, C<$LF>, and C<$CRLF>, which map
to C<\015>, C<\012>, and C<\015\012>. If you do not want to use the literal
characters in your programs, then use the constants provided here. They are
not exported by default, but can be imported individually, and with the
C<:crlf> export tag:

 use Socket qw(:DEFAULT :crlf);

 $sock->print("GET / HTTP/1.0$CRLF");

The entire getaddrinfo() subsystem can be exported using the tag C<:addrinfo>;
this exports the getaddrinfo() and getnameinfo() functions, and all the
C<AI_*>, C<NI_*>, C<NIx_*> and C<EAI_*> constants.

=cut

=head1 CONSTANTS

In each of the following groups, there may be many more constants provided
than just the ones given as examples in the section heading. If the heading
ends C<...> then this means there are likely more; the exact constants
provided will depend on the OS and headers found at compile-time.

=cut

=head2 PF_INET, PF_INET6, PF_UNIX, ...

Protocol family constants to use as the first argument to socket() or the
value of the C<SO_DOMAIN> or C<SO_FAMILY> socket option.

=head2 AF_INET, AF_INET6, AF_UNIX, ...

Address family constants used by the socket address structures, to pass to
such functions as inet_pton() or getaddrinfo(), or are returned by such
functions as sockaddr_family().

=head2 SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, ...

Socket type constants to use as the second argument to socket(), or the value
of the C<SO_TYPE> socket option.

=head2 SOCK_NONBLOCK. SOCK_CLOEXEC

Linux-specific shortcuts to specify the C<O_NONBLOCK> and C<FD_CLOEXEC> flags
during a C<socket(2)> call.

 socket( my $sockh, PF_INET, SOCK_DGRAM|SOCK_NONBLOCK, 0 )

=head2 SOL_SOCKET

Socket option level constant for setsockopt() and getsockopt().

=head2 SO_ACCEPTCONN, SO_BROADCAST, SO_ERROR, ...

Socket option name constants for setsockopt() and getsockopt() at the
C<SOL_SOCKET> level.

=head2 IP_OPTIONS, IP_TOS, IP_TTL, ...

Socket option name constants for IPv4 socket options at the C<IPPROTO_IP>
level.

=head2 IP_PMTUDISC_WANT, IP_PMTUDISC_DONT, ...

Socket option value constants for C<IP_MTU_DISCOVER> socket option.

=head2 IPTOS_LOWDELAY, IPTOS_THROUGHPUT, IPTOS_RELIABILITY, ...

Socket option value constants for C<IP_TOS> socket option.

=head2 MSG_BCAST, MSG_OOB, MSG_TRUNC, ...

Message flag constants for send() and recv().

=head2 SHUT_RD, SHUT_RDWR, SHUT_WR

Direction constants for shutdown().

=head2 INADDR_ANY, INADDR_BROADCAST, INADDR_LOOPBACK, INADDR_NONE

Constants giving the special C<AF_INET> addresses for wildcard, broadcast,
local loopback, and invalid addresses.

Normally equivalent to inet_aton('0.0.0.0'), inet_aton('255.255.255.255'),
inet_aton('localhost') and inet_aton('255.255.255.255') respectively.

=head2 IPPROTO_IP, IPPROTO_IPV6, IPPROTO_TCP, ...

IP protocol constants to use as the third argument to socket(), the level
argument to getsockopt() or setsockopt(), or the value of the C<SO_PROTOCOL>
socket option.

=head2 TCP_CORK, TCP_KEEPALIVE, TCP_NODELAY, ...

Socket option name constants for TCP socket options at the C<IPPROTO_TCP>
level.

=head2 IN6ADDR_ANY, IN6ADDR_LOOPBACK

Constants giving the special C<AF_INET6> addresses for wildcard and local
loopback.

Normally equivalent to inet_pton(AF_INET6, "::") and
inet_pton(AF_INET6, "::1") respectively.

=head2 IPV6_ADD_MEMBERSHIP, IPV6_MTU, IPV6_V6ONLY, ...

Socket option name constants for IPv6 socket options at the C<IPPROTO_IPV6>
level.

=cut

# Still undocumented: SCM_*, SOMAXCONN, IOV_MAX, UIO_MAXIOV

=head1 STRUCTURE MANIPULATORS

The following functions convert between lists of Perl values and packed binary
strings representing structures.

=cut

=head2 $family = sockaddr_family $sockaddr

Takes a packed socket address (as returned by pack_sockaddr_in(),
pack_sockaddr_un() or the perl builtin functions getsockname() and
getpeername()). Returns the address family tag. This will be one of the
C<AF_*> constants, such as C<AF_INET> for a C<sockaddr_in> addresses or
C<AF_UNIX> for a C<sockaddr_un>. It can be used to figure out what unpack to
use for a sockaddr of unknown type.

=head2 $sockaddr = pack_sockaddr_in $port, $ip_address

Takes two arguments, a port number and an opaque string (as returned by
inet_aton(), or a v-string). Returns the C<sockaddr_in> structure with those
arguments packed in and C<AF_INET> filled in. For Internet domain sockets,
this structure is normally what you need for the arguments in bind(),
connect(), and send().

An undefined $port argument is taken as zero; an undefined $ip_address is
considered a fatal error.

=head2 ($port, $ip_address) = unpack_sockaddr_in $sockaddr

Takes a C<sockaddr_in> structure (as returned by pack_sockaddr_in(),
getpeername() or recv()). Returns a list of two elements: the port and an
opaque string representing the IP address (you can use inet_ntoa() to convert
the address to the four-dotted numeric format). Will croak if the structure
does not represent an C<AF_INET> address.

In scalar context will return just the IP address.

=head2 $sockaddr = sockaddr_in $port, $ip_address

=head2 ($port, $ip_address) = sockaddr_in $sockaddr

A wrapper of pack_sockaddr_in() or unpack_sockaddr_in(). In list context,
unpacks its argument and returns a list consisting of the port and IP address.
In scalar context, packs its port and IP address arguments as a C<sockaddr_in>
and returns it.

Provided largely for legacy compatibility; it is better to use
pack_sockaddr_in() or unpack_sockaddr_in() explicitly.

=head2 $sockaddr = pack_sockaddr_in6 $port, $ip6_address, [$scope_id, [$flowinfo]]

Takes two to four arguments, a port number, an opaque string (as returned by
inet_pton()), optionally a scope ID number, and optionally a flow label
number. Returns the C<sockaddr_in6> structure with those arguments packed in
and C<AF_INET6> filled in. IPv6 equivalent of pack_sockaddr_in().

An undefined $port argument is taken as zero; an undefined $ip6_address is
considered a fatal error.

=head2 ($port, $ip6_address, $scope_id, $flowinfo) = unpack_sockaddr_in6 $sockaddr

Takes a C<sockaddr_in6> structure. Returns a list of four elements: the port
number, an opaque string representing the IPv6 address, the scope ID, and the
flow label. (You can use inet_ntop() to convert the address to the usual
string format). Will croak if the structure does not represent an C<AF_INET6>
address.

In scalar context will return just the IP address.

=head2 $sockaddr = sockaddr_in6 $port, $ip6_address, [$scope_id, [$flowinfo]]

=head2 ($port, $ip6_address, $scope_id, $flowinfo) = sockaddr_in6 $sockaddr

A wrapper of pack_sockaddr_in6() or unpack_sockaddr_in6(). In list context,
unpacks its argument according to unpack_sockaddr_in6(). In scalar context,
packs its arguments according to pack_sockaddr_in6().

Provided largely for legacy compatibility; it is better to use
pack_sockaddr_in6() or unpack_sockaddr_in6() explicitly.

=head2 $sockaddr = pack_sockaddr_un $path

Takes one argument, a pathname. Returns the C<sockaddr_un> structure with that
path packed in with C<AF_UNIX> filled in. For C<PF_UNIX> sockets, this
structure is normally what you need for the arguments in bind(), connect(),
and send().

=head2 ($path) = unpack_sockaddr_un $sockaddr

Takes a C<sockaddr_un> structure (as returned by pack_sockaddr_un(),
getpeername() or recv()). Returns a list of one element: the pathname. Will
croak if the structure does not represent an C<AF_UNIX> address.

=head2 $sockaddr = sockaddr_un $path

=head2 ($path) = sockaddr_un $sockaddr

A wrapper of pack_sockaddr_un() or unpack_sockaddr_un(). In a list context,
unpacks its argument and returns a list consisting of the pathname. In a
scalar context, packs its pathname as a C<sockaddr_un> and returns it.

Provided largely for legacy compatibility; it is better to use
pack_sockaddr_un() or unpack_sockaddr_un() explicitly.

These are only supported if your system has E<lt>F<sys/un.h>E<gt>.

=head2 $ip_mreq = pack_ip_mreq $multiaddr, $interface

Takes an IPv4 multicast address and optionally an interface address (or
C<INADDR_ANY>). Returns the C<ip_mreq> structure with those arguments packed
in. Suitable for use with the C<IP_ADD_MEMBERSHIP> and C<IP_DROP_MEMBERSHIP>
sockopts.

=head2 ($multiaddr, $interface) = unpack_ip_mreq $ip_mreq

Takes an C<ip_mreq> structure. Returns a list of two elements; the IPv4
multicast address and interface address.

=head2 $ip_mreq_source = pack_ip_mreq_source $multiaddr, $source, $interface

Takes an IPv4 multicast address, source address, and optionally an interface
address (or C<INADDR_ANY>). Returns the C<ip_mreq_source> structure with those
arguments packed in. Suitable for use with the C<IP_ADD_SOURCE_MEMBERSHIP>
and C<IP_DROP_SOURCE_MEMBERSHIP> sockopts.

=head2 ($multiaddr, $source, $interface) = unpack_ip_mreq_source $ip_mreq

Takes an C<ip_mreq_source> structure. Returns a list of three elements; the
IPv4 multicast address, source address and interface address.

=head2 $ipv6_mreq = pack_ipv6_mreq $multiaddr6, $ifindex

Takes an IPv6 multicast address and an interface number. Returns the
C<ipv6_mreq> structure with those arguments packed in. Suitable for use with
the C<IPV6_ADD_MEMBERSHIP> and C<IPV6_DROP_MEMBERSHIP> sockopts.

=head2 ($multiaddr6, $ifindex) = unpack_ipv6_mreq $ipv6_mreq

Takes an C<ipv6_mreq> structure. Returns a list of two elements; the IPv6
address and an interface number.

=cut

=head1 FUNCTIONS

=cut

=head2 $ip_address = inet_aton $string

Takes a string giving the name of a host, or a textual representation of an IP
address and translates that to an packed binary address structure suitable to
pass to pack_sockaddr_in(). If passed a hostname that cannot be resolved,
returns C<undef>. For multi-homed hosts (hosts with more than one address),
the first address found is returned.

For portability do not assume that the result of inet_aton() is 32 bits wide,
in other words, that it would contain only the IPv4 address in network order.

This IPv4-only function is provided largely for legacy reasons. Newly-written
code should use getaddrinfo() or inet_pton() instead for IPv6 support.

=head2 $string = inet_ntoa $ip_address

Takes a packed binary address structure such as returned by
unpack_sockaddr_in() (or a v-string representing the four octets of the IPv4
address in network order) and translates it into a string of the form
C<d.d.d.d> where the C<d>s are numbers less than 256 (the normal
human-readable four dotted number notation for Internet addresses).

This IPv4-only function is provided largely for legacy reasons. Newly-written
code should use getnameinfo() or inet_ntop() instead for IPv6 support.

=head2 $address = inet_pton $family, $string

Takes an address family (such as C<AF_INET> or C<AF_INET6>) and a string
containing a textual representation of an address in that family and
translates that to an packed binary address structure.

See also getaddrinfo() for a more powerful and flexible function to look up
socket addresses given hostnames or textual addresses.

=head2 $string = inet_ntop $family, $address

Takes an address family and a packed binary address structure and translates
it into a human-readable textual representation of the address; typically in
C<d.d.d.d> form for C<AF_INET> or C<hhhh:hhhh::hhhh> form for C<AF_INET6>.

See also getnameinfo() for a more powerful and flexible function to turn
socket addresses into human-readable textual representations.

=head2 ($err, @result) = getaddrinfo $host, $service, [$hints]

Given both a hostname and service name, this function attempts to resolve the
host name into a list of network addresses, and the service name into a
protocol and port number, and then returns a list of address structures
suitable to connect() to it.

Given just a host name, this function attempts to resolve it to a list of
network addresses, and then returns a list of address structures giving these
addresses.

Given just a service name, this function attempts to resolve it to a protocol
and port number, and then returns a list of address structures that represent
it suitable to bind() to. This use should be combined with the C<AI_PASSIVE>
flag; see below.

Given neither name, it generates an error.

If present, $hints should be a reference to a hash, where the following keys
are recognised:

=over 4

=item flags => INT

A bitfield containing C<AI_*> constants; see below.

=item family => INT

Restrict to only generating addresses in this address family

=item socktype => INT

Restrict to only generating addresses of this socket type

=item protocol => INT

Restrict to only generating addresses for this protocol

=back

The return value will be a list; the first value being an error indication,
followed by a list of address structures (if no error occurred).

The error value will be a dualvar; comparable to the C<EAI_*> error constants,
or printable as a human-readable error message string. If no error occurred it
will be zero numerically and an empty string.

Each value in the results list will be a hash reference containing the following
fields:

=over 4

=item family => INT

The address family (e.g. C<AF_INET>)

=item socktype => INT

The socket type (e.g. C<SOCK_STREAM>)

=item protocol => INT

The protocol (e.g. C<IPPROTO_TCP>)

=item addr => STRING

The address in a packed string (such as would be returned by
pack_sockaddr_in())

=item canonname => STRING

The canonical name for the host if the C<AI_CANONNAME> flag was provided, or
C<undef> otherwise. This field will only be present on the first returned
address.

=back

The following flag constants are recognised in the $hints hash. Other flag
constants may exist as provided by the OS.

=over 4

=item AI_PASSIVE

Indicates that this resolution is for a local bind() for a passive (i.e.
listening) socket, rather than an active (i.e. connecting) socket.

=item AI_CANONNAME

Indicates that the caller wishes the canonical hostname (C<canonname>) field
of the result to be filled in.

=item AI_NUMERICHOST

Indicates that the caller will pass a numeric address, rather than a hostname,
and that getaddrinfo() must not perform a resolve operation on this name. This
flag will prevent a possibly-slow network lookup operation, and instead return
an error if a hostname is passed.

=back

=head2 ($err, $hostname, $servicename) = getnameinfo $sockaddr, [$flags, [$xflags]]

Given a packed socket address (such as from getsockname(), getpeername(), or
returned by getaddrinfo() in a C<addr> field), returns the hostname and
symbolic service name it represents. $flags may be a bitmask of C<NI_*>
constants, or defaults to 0 if unspecified.

The return value will be a list; the first value being an error condition,
followed by the hostname and service name.

The error value will be a dualvar; comparable to the C<EAI_*> error constants,
or printable as a human-readable error message string. The host and service
names will be plain strings.

The following flag constants are recognised as $flags. Other flag constants may
exist as provided by the OS.

=over 4

=item NI_NUMERICHOST

Requests that a human-readable string representation of the numeric address be
returned directly, rather than performing a name resolve operation that may
convert it into a hostname. This will also avoid potentially-blocking network
IO.

=item NI_NUMERICSERV

Requests that the port number be returned directly as a number representation
rather than performing a name resolve operation that may convert it into a
service name.

=item NI_NAMEREQD

If a name resolve operation fails to provide a name, then this flag will cause
getnameinfo() to indicate an error, rather than returning the numeric
representation as a human-readable string.

=item NI_DGRAM

Indicates that the socket address relates to a C<SOCK_DGRAM> socket, for the
services whose name differs between TCP and UDP protocols.

=back

The following constants may be supplied as $xflags.

=over 4

=item NIx_NOHOST

Indicates that the caller is not interested in the hostname of the result, so
it does not have to be converted. C<undef> will be returned as the hostname.

=item NIx_NOSERV

Indicates that the caller is not interested in the service name of the result,
so it does not have to be converted. C<undef> will be returned as the service
name.

=back

=head1 getaddrinfo() / getnameinfo() ERROR CONSTANTS

The following constants may be returned by getaddrinfo() or getnameinfo().
Others may be provided by the OS.

=over 4

=item EAI_AGAIN

A temporary failure occurred during name resolution. The operation may be
successful if it is retried later.

=item EAI_BADFLAGS

The value of the C<flags> hint to getaddrinfo(), or the $flags parameter to
getnameinfo() contains unrecognised flags.

=item EAI_FAMILY

The C<family> hint to getaddrinfo(), or the family of the socket address
passed to getnameinfo() is not supported.

=item EAI_NODATA

The host name supplied to getaddrinfo() did not provide any usable address
data.

=item EAI_NONAME

The host name supplied to getaddrinfo() does not exist, or the address
supplied to getnameinfo() is not associated with a host name and the
C<NI_NAMEREQD> flag was supplied.

=item EAI_SERVICE

The service name supplied to getaddrinfo() is not available for the socket
type given in the $hints.

=back

=cut

=head1 EXAMPLES

=head2 Lookup for connect()

The getaddrinfo() function converts a hostname and a service name into a list
of structures, each containing a potential way to connect() to the named
service on the named host.

 use IO::Socket;
 use Socket qw(SOCK_STREAM getaddrinfo);

 my %hints = (socktype => SOCK_STREAM);
 my ($err, @res) = getaddrinfo("localhost", "echo", \%hints);
 die "Cannot getaddrinfo - $err" if $err;

 my $sock;

 foreach my $ai (@res) {
     my $candidate = IO::Socket->new();

     $candidate->socket($ai->{family}, $ai->{socktype}, $ai->{protocol})
         or next;

     $candidate->connect($ai->{addr})
         or next;

     $sock = $candidate;
     last;
 }

 die "Cannot connect to localhost:echo" unless $sock;

 $sock->print("Hello, world!\n");
 print <$sock>;

Because a list of potential candidates is returned, the C<while> loop tries
each in turn until it finds one that succeeds both the socket() and connect()
calls.

This function performs the work of the legacy functions gethostbyname(),
getservbyname(), inet_aton() and pack_sockaddr_in().

In practice this logic is better performed by L<IO::Socket::IP>.

=head2 Making a human-readable string out of an address

The getnameinfo() function converts a socket address, such as returned by
getsockname() or getpeername(), into a pair of human-readable strings
representing the address and service name.

 use IO::Socket::IP;
 use Socket qw(getnameinfo);

 my $server = IO::Socket::IP->new(LocalPort => 12345, Listen => 1) or
     die "Cannot listen - $@";

 my $socket = $server->accept or die "accept: $!";

 my ($err, $hostname, $servicename) = getnameinfo($socket->peername);
 die "Cannot getnameinfo - $err" if $err;

 print "The peer is connected from $hostname\n";

Since in this example only the hostname was used, the redundant conversion of
the port number into a service name may be omitted by passing the
C<NIx_NOSERV> flag.

 use Socket qw(getnameinfo NIx_NOSERV);

 my ($err, $hostname) = getnameinfo($socket->peername, 0, NIx_NOSERV);

This function performs the work of the legacy functions unpack_sockaddr_in(),
inet_ntoa(), gethostbyaddr() and getservbyport().

In practice this logic is better performed by L<IO::Socket::IP>.

=head2 Resolving hostnames into IP addresses

To turn a hostname into a human-readable plain IP address use getaddrinfo()
to turn the hostname into a list of socket structures, then getnameinfo() on
each one to make it a readable IP address again.

 use Socket qw(:addrinfo SOCK_RAW);

 my ($err, @res) = getaddrinfo($hostname, "", {socktype => SOCK_RAW});
 die "Cannot getaddrinfo - $err" if $err;

 while( my $ai = shift @res ) {
     my ($err, $ipaddr) = getnameinfo($ai->{addr}, NI_NUMERICHOST, NIx_NOSERV);
     die "Cannot getnameinfo - $err" if $err;

     print "$ipaddr\n";
 }

The C<socktype> hint to getaddrinfo() filters the results to only include one
socket type and protocol. Without this most OSes return three combinations,
for C<SOCK_STREAM>, C<SOCK_DGRAM> and C<SOCK_RAW>, resulting in triplicate
output of addresses. The C<NI_NUMERICHOST> flag to getnameinfo() causes it to
return a string-formatted plain IP address, rather than reverse resolving it
back into a hostname.

This combination performs the work of the legacy functions gethostbyname()
and inet_ntoa().

=head2 Accessing socket options

The many C<SO_*> and other constants provide the socket option names for
getsockopt() and setsockopt().

 use IO::Socket::INET;
 use Socket qw(SOL_SOCKET SO_RCVBUF IPPROTO_IP IP_TTL);

 my $socket = IO::Socket::INET->new(LocalPort => 0, Proto => 'udp')
     or die "Cannot create socket: $@";

 $socket->setsockopt(SOL_SOCKET, SO_RCVBUF, 64*1024) or
     die "setsockopt: $!";

 print "Receive buffer is ", $socket->getsockopt(SOL_SOCKET, SO_RCVBUF),
     " bytes\n";

 print "IP TTL is ", $socket->getsockopt(IPPROTO_IP, IP_TTL), "\n";

As a convenience, L<IO::Socket>'s setsockopt() method will convert a number
into a packed byte buffer, and getsockopt() will unpack a byte buffer of the
correct size back into a number.

=cut

=head1 AUTHOR

This module was originally maintained in Perl core by the Perl 5 Porters.

It was extracted to dual-life on CPAN at version 1.95 by
Paul Evans <leonerd@leonerd.org.uk>

=cut

use Carp;
use warnings::register;

require Exporter;
require XSLoader;
our @ISA = qw(Exporter);

# <@Nicholas> you can't change @EXPORT without breaking the implicit API
# Please put any new constants in @EXPORT_OK!

# List re-ordered to match documentation above. Try to keep the ordering
# consistent so it's easier to see which ones are or aren't documented.
our @EXPORT = qw(
	PF_802 PF_AAL PF_APPLETALK PF_CCITT PF_CHAOS PF_CTF PF_DATAKIT
	PF_DECnet PF_DLI PF_ECMA PF_GOSIP PF_HYLINK PF_IMPLINK PF_INET PF_INET6
	PF_ISO PF_KEY PF_LAST PF_LAT PF_LINK PF_MAX PF_NBS PF_NIT PF_NS PF_OSI
	PF_OSINET PF_PUP PF_ROUTE PF_SNA PF_UNIX PF_UNSPEC PF_USER PF_WAN
	PF_X25

	AF_802 AF_AAL AF_APPLETALK AF_CCITT AF_CHAOS AF_CTF AF_DATAKIT
	AF_DECnet AF_DLI AF_ECMA AF_GOSIP AF_HYLINK AF_IMPLINK AF_INET AF_INET6
	AF_ISO AF_KEY AF_LAST AF_LAT AF_LINK AF_MAX AF_NBS AF_NIT AF_NS AF_OSI
	AF_OSINET AF_PUP AF_ROUTE AF_SNA AF_UNIX AF_UNSPEC AF_USER AF_WAN
	AF_X25

	SOCK_DGRAM SOCK_RAW SOCK_RDM SOCK_SEQPACKET SOCK_STREAM

	SOL_SOCKET

	SO_ACCEPTCONN SO_ATTACH_FILTER SO_BACKLOG SO_BROADCAST SO_CHAMELEON
	SO_DEBUG SO_DETACH_FILTER SO_DGRAM_ERRIND SO_DOMAIN SO_DONTLINGER
	SO_DONTROUTE SO_ERROR SO_FAMILY SO_KEEPALIVE SO_LINGER SO_OOBINLINE
	SO_PASSCRED SO_PASSIFNAME SO_PEERCRED SO_PROTOCOL SO_PROTOTYPE
	SO_RCVBUF SO_RCVLOWAT SO_RCVTIMEO SO_REUSEADDR SO_REUSEPORT
	SO_SECURITY_AUTHENTICATION SO_SECURITY_ENCRYPTION_NETWORK
	SO_SECURITY_ENCRYPTION_TRANSPORT SO_SNDBUF SO_SNDLOWAT SO_SNDTIMEO
	SO_STATE SO_TYPE SO_USELOOPBACK SO_XOPEN SO_XSE

	IP_HDRINCL IP_OPTIONS IP_RECVOPTS IP_RECVRETOPTS IP_RETOPTS IP_TOS
	IP_TTL

	MSG_BCAST MSG_BTAG MSG_CTLFLAGS MSG_CTLIGNORE MSG_CTRUNC MSG_DONTROUTE
	MSG_DONTWAIT MSG_EOF MSG_EOR MSG_ERRQUEUE MSG_ETAG MSG_FASTOPEN MSG_FIN
	MSG_MAXIOVLEN MSG_MCAST MSG_NOSIGNAL MSG_OOB MSG_PEEK MSG_PROXY MSG_RST
	MSG_SYN MSG_TRUNC MSG_URG MSG_WAITALL MSG_WIRE

	SHUT_RD SHUT_RDWR SHUT_WR

	INADDR_ANY INADDR_BROADCAST INADDR_LOOPBACK INADDR_NONE

	SCM_CONNECT SCM_CREDENTIALS SCM_CREDS SCM_RIGHTS SCM_TIMESTAMP

	SOMAXCONN

	IOV_MAX
	UIO_MAXIOV

	sockaddr_family
	pack_sockaddr_in  unpack_sockaddr_in  sockaddr_in
	pack_sockaddr_in6 unpack_sockaddr_in6 sockaddr_in6
	pack_sockaddr_un  unpack_sockaddr_un  sockaddr_un 

	inet_aton inet_ntoa
);

# List re-ordered to match documentation above. Try to keep the ordering
# consistent so it's easier to see which ones are or aren't documented.
our @EXPORT_OK = qw(
	CR LF CRLF $CR $LF $CRLF

	SOCK_NONBLOCK SOCK_CLOEXEC

	IP_ADD_MEMBERSHIP IP_ADD_SOURCE_MEMBERSHIP IP_BIND_ADDRESS_NO_PORT
	IP_DROP_MEMBERSHIP IP_DROP_SOURCE_MEMBERSHIP IP_FREEBIND
	IP_MULTICAST_ALL IP_MULTICAST_IF IP_MULTICAST_LOOP IP_MULTICAST_TTL
	IP_MTU IP_MTU_DISCOVER IP_NODEFRAG IP_RECVERR IP_TRANSPARENT

	IPPROTO_IP IPPROTO_IPV6 IPPROTO_RAW IPPROTO_ICMP IPPROTO_IGMP
	IPPROTO_TCP IPPROTO_UDP IPPROTO_GRE IPPROTO_ESP IPPROTO_AH
	IPPROTO_ICMPV6 IPPROTO_SCTP

	IP_PMTUDISC_DO IP_PMTUDISC_DONT IP_PMTUDISC_PROBE IP_PMTUDISC_WANT

	IPTOS_LOWDELAY IPTOS_THROUGHPUT IPTOS_RELIABILITY IPTOS_MINCOST

	TCP_CONGESTION TCP_CONNECTIONTIMEOUT TCP_CORK TCP_DEFER_ACCEPT
	TCP_FASTOPEN TCP_INFO TCP_INIT_CWND TCP_KEEPALIVE TCP_KEEPCNT
	TCP_KEEPIDLE TCP_KEEPINTVL TCP_LINGER2 TCP_MAXRT TCP_MAXSEG
	TCP_MD5SIG TCP_NODELAY TCP_NOOPT TCP_NOPUSH TCP_QUICKACK
	TCP_SACK_ENABLE TCP_STDURG TCP_SYNCNT TCP_USER_TIMEOUT
	TCP_WINDOW_CLAMP

	IN6ADDR_ANY IN6ADDR_LOOPBACK

	IPV6_ADDRFROM IPV6_ADD_MEMBERSHIP IPV6_DROP_MEMBERSHIP IPV6_JOIN_GROUP
	IPV6_LEAVE_GROUP IPV6_MTU IPV6_MTU_DISCOVER IPV6_MULTICAST_HOPS
	IPV6_MULTICAST_IF IPV6_MULTICAST_LOOP IPV6_RECVERR IPV6_ROUTER_ALERT
	IPV6_UNICAST_HOPS IPV6_V6ONLY

	SO_INCOMING_CPU SO_INCOMING_NAPI_ID SO_LOCK_FILTER SO_RCVBUFFORCE
	SO_SNDBUFFORCE

	pack_ip_mreq unpack_ip_mreq pack_ip_mreq_source unpack_ip_mreq_source

	pack_ipv6_mreq unpack_ipv6_mreq

	inet_pton inet_ntop

	getaddrinfo getnameinfo

	AI_ADDRCONFIG AI_ALL AI_CANONIDN AI_CANONNAME AI_IDN
	AI_IDN_ALLOW_UNASSIGNED AI_IDN_USE_STD3_ASCII_RULES AI_NUMERICHOST
	AI_NUMERICSERV AI_PASSIVE AI_V4MAPPED

	NI_DGRAM NI_IDN NI_IDN_ALLOW_UNASSIGNED NI_IDN_USE_STD3_ASCII_RULES
	NI_NAMEREQD NI_NOFQDN NI_NUMERICHOST NI_NUMERICSERV

	NIx_NOHOST NIx_NOSERV

	EAI_ADDRFAMILY EAI_AGAIN EAI_BADFLAGS EAI_BADHINTS EAI_FAIL EAI_FAMILY
	EAI_NODATA EAI_NONAME EAI_PROTOCOL EAI_SERVICE EAI_SOCKTYPE EAI_SYSTEM
);

our %EXPORT_TAGS = (
    crlf     => [qw(CR LF CRLF $CR $LF $CRLF)],
    addrinfo => [qw(getaddrinfo getnameinfo), grep m/^(?:AI|NI|NIx|EAI)_/, @EXPORT_OK],
    all      => [@EXPORT, @EXPORT_OK],
);

BEGIN {
    sub CR   () {"\015"}
    sub LF   () {"\012"}
    sub CRLF () {"\015\012"}

    # These are not gni() constants; they're extensions for the perl API
    # The definitions in Socket.pm and Socket.xs must match
    sub NIx_NOHOST() {1 << 0}
    sub NIx_NOSERV() {1 << 1}
}

*CR   = \CR();
*LF   = \LF();
*CRLF = \CRLF();

# The four deprecated addrinfo constants
foreach my $name (qw( AI_IDN_ALLOW_UNASSIGNED AI_IDN_USE_STD3_ASCII_RULES NI_IDN_ALLOW_UNASSIGNED NI_IDN_USE_STD3_ASCII_RULES )) {
    no strict 'refs';
    *$name = sub {
	croak "The addrinfo constant $name is deprecated";
    };
}

sub sockaddr_in {
    if (@_ == 6 && !wantarray) { # perl5.001m compat; use this && die
	my($af, $port, @quad) = @_;
	warnings::warn "6-ARG sockaddr_in call is deprecated" 
	    if warnings::enabled();
	pack_sockaddr_in($port, inet_aton(join('.', @quad)));
    } elsif (wantarray) {
	croak "usage:   (port,iaddr) = sockaddr_in(sin_sv)" unless @_ == 1;
        unpack_sockaddr_in(@_);
    } else {
	croak "usage:   sin_sv = sockaddr_in(port,iaddr))" unless @_ == 2;
        pack_sockaddr_in(@_);
    }
}

sub sockaddr_in6 {
    if (wantarray) {
	croak "usage:   (port,in6addr,scope_id,flowinfo) = sockaddr_in6(sin6_sv)" unless @_ == 1;
	unpack_sockaddr_in6(@_);
    }
    else {
	croak "usage:   sin6_sv = sockaddr_in6(port,in6addr,[scope_id,[flowinfo]])" unless @_ >= 2 and @_ <= 4;
	pack_sockaddr_in6(@_);
    }
}

sub sockaddr_un {
    if (wantarray) {
	croak "usage:   (filename) = sockaddr_un(sun_sv)" unless @_ == 1;
        unpack_sockaddr_un(@_);
    } else {
	croak "usage:   sun_sv = sockaddr_un(filename)" unless @_ == 1;
        pack_sockaddr_un(@_);
    }
}

XSLoader::load(__PACKAGE__, $VERSION);

my %errstr;

if( defined &getaddrinfo ) {
    # These are not part of the API, nothing uses them, and deleting them
    # reduces the size of %Socket:: by about 12K
    delete $Socket::{fake_getaddrinfo};
    delete $Socket::{fake_getnameinfo};
} else {
    require Scalar::Util;

    *getaddrinfo = \&fake_getaddrinfo;
    *getnameinfo = \&fake_getnameinfo;

    # These numbers borrowed from GNU libc's implementation, but since
    # they're only used by our emulation, it doesn't matter if the real
    # platform's values differ
    my %constants = (
	AI_PASSIVE     => 1,
	AI_CANONNAME   => 2,
	AI_NUMERICHOST => 4,
	AI_V4MAPPED    => 8,
	AI_ALL         => 16,
	AI_ADDRCONFIG  => 32,
	# RFC 2553 doesn't define this but Linux does - lets be nice and
	# provide it since we can
	AI_NUMERICSERV => 1024,

	EAI_BADFLAGS   => -1,
	EAI_NONAME     => -2,
	EAI_NODATA     => -5,
	EAI_FAMILY     => -6,
	EAI_SERVICE    => -8,

	NI_NUMERICHOST => 1,
	NI_NUMERICSERV => 2,
	NI_NOFQDN      => 4,
	NI_NAMEREQD    => 8,
	NI_DGRAM       => 16,

	# Constants we don't support. Export them, but croak if anyone tries to
	# use them
	AI_IDN      => 64,
	AI_CANONIDN => 128,
	NI_IDN      => 32,

	# Error constants we'll never return, so it doesn't matter what value
	# these have, nor that we don't provide strings for them
	EAI_SYSTEM   => -11,
	EAI_BADHINTS => -1000,
	EAI_PROTOCOL => -1001
    );

    foreach my $name ( keys %constants ) {
	my $value = $constants{$name};

	no strict 'refs';
	defined &$name or *$name = sub () { $value };
    }

    %errstr = (
	# These strings from RFC 2553
	EAI_BADFLAGS()   => "invalid value for ai_flags",
	EAI_NONAME()     => "nodename nor servname provided, or not known",
	EAI_NODATA()     => "no address associated with nodename",
	EAI_FAMILY()     => "ai_family not supported",
	EAI_SERVICE()    => "servname not supported for ai_socktype",
    );
}

# The following functions are used if the system does not have a
# getaddrinfo(3) function in libc; and are used to emulate it for the AF_INET
# family

# Borrowed from Regexp::Common::net
my $REGEXP_IPv4_DECIMAL = qr/25[0-5]|2[0-4][0-9]|[0-1]?[0-9]{1,2}/;
my $REGEXP_IPv4_DOTTEDQUAD = qr/$REGEXP_IPv4_DECIMAL\.$REGEXP_IPv4_DECIMAL\.$REGEXP_IPv4_DECIMAL\.$REGEXP_IPv4_DECIMAL/;

sub fake_makeerr
{
    my ( $errno ) = @_;
    my $errstr = $errno == 0 ? "" : ( $errstr{$errno} || $errno );
    return Scalar::Util::dualvar( $errno, $errstr );
}

sub fake_getaddrinfo
{
    my ( $node, $service, $hints ) = @_;

    $node = "" unless defined $node;

    $service = "" unless defined $service;

    my ( $family, $socktype, $protocol, $flags ) = @$hints{qw( family socktype protocol flags )};

    $family ||= Socket::AF_INET(); # 0 == AF_UNSPEC, which we want too
    $family == Socket::AF_INET() or return fake_makeerr( EAI_FAMILY() );

    $socktype ||= 0;

    $protocol ||= 0;

    $flags ||= 0;

    my $flag_passive     = $flags & AI_PASSIVE();     $flags &= ~AI_PASSIVE();
    my $flag_canonname   = $flags & AI_CANONNAME();   $flags &= ~AI_CANONNAME();
    my $flag_numerichost = $flags & AI_NUMERICHOST(); $flags &= ~AI_NUMERICHOST();
    my $flag_numericserv = $flags & AI_NUMERICSERV(); $flags &= ~AI_NUMERICSERV();

    # These constants don't apply to AF_INET-only lookups, so we might as well
    # just ignore them. For AI_ADDRCONFIG we just presume the host has ability
    # to talk AF_INET. If not we'd have to return no addresses at all. :)
    $flags &= ~(AI_V4MAPPED()|AI_ALL()|AI_ADDRCONFIG());

    $flags & (AI_IDN()|AI_CANONIDN()) and
	croak "Socket::getaddrinfo() does not support IDN";

    $flags == 0 or return fake_makeerr( EAI_BADFLAGS() );

    $node eq "" and $service eq "" and return fake_makeerr( EAI_NONAME() );

    my $canonname;
    my @addrs;
    if( $node ne "" ) {
	return fake_makeerr( EAI_NONAME() ) if( $flag_numerichost and $node !~ m/^$REGEXP_IPv4_DOTTEDQUAD$/ );
	( $canonname, undef, undef, undef, @addrs ) = gethostbyname( $node );
	defined $canonname or return fake_makeerr( EAI_NONAME() );

	undef $canonname unless $flag_canonname;
    }
    else {
	$addrs[0] = $flag_passive ? Socket::inet_aton( "0.0.0.0" )
				  : Socket::inet_aton( "127.0.0.1" );
    }

    my @ports; # Actually ARRAYrefs of [ socktype, protocol, port ]
    my $protname = "";
    if( $protocol ) {
	$protname = eval { getprotobynumber( $protocol ) };
    }

    if( $service ne "" and $service !~ m/^\d+$/ ) {
	return fake_makeerr( EAI_NONAME() ) if( $flag_numericserv );
	getservbyname( $service, $protname ) or return fake_makeerr( EAI_SERVICE() );
    }

    foreach my $this_socktype ( Socket::SOCK_STREAM(), Socket::SOCK_DGRAM(), Socket::SOCK_RAW() ) {
	next if $socktype and $this_socktype != $socktype;

	my $this_protname = "raw";
	$this_socktype == Socket::SOCK_STREAM() and $this_protname = "tcp";
	$this_socktype == Socket::SOCK_DGRAM()  and $this_protname = "udp";

	next if $protname and $this_protname ne $protname;

	my $port;
	if( $service ne "" ) {
	    if( $service =~ m/^\d+$/ ) {
		$port = "$service";
	    }
	    else {
		( undef, undef, $port, $this_protname ) = getservbyname( $service, $this_protname );
		next unless defined $port;
	    }
	}
	else {
	    $port = 0;
	}

	push @ports, [ $this_socktype, eval { scalar getprotobyname( $this_protname ) } || 0, $port ];
    }

    my @ret;
    foreach my $addr ( @addrs ) {
	foreach my $portspec ( @ports ) {
	    my ( $socktype, $protocol, $port ) = @$portspec;
	    push @ret, {
		family    => $family,
		socktype  => $socktype,
		protocol  => $protocol,
		addr      => Socket::pack_sockaddr_in( $port, $addr ),
		canonname => undef,
	    };
	}
    }

    # Only supply canonname for the first result
    if( defined $canonname ) {
	$ret[0]->{canonname} = $canonname;
    }

    return ( fake_makeerr( 0 ), @ret );
}

sub fake_getnameinfo
{
    my ( $addr, $flags, $xflags ) = @_;

    my ( $port, $inetaddr );
    eval { ( $port, $inetaddr ) = Socket::unpack_sockaddr_in( $addr ) }
	or return fake_makeerr( EAI_FAMILY() );

    my $family = Socket::AF_INET();

    $flags ||= 0;

    my $flag_numerichost = $flags & NI_NUMERICHOST(); $flags &= ~NI_NUMERICHOST();
    my $flag_numericserv = $flags & NI_NUMERICSERV(); $flags &= ~NI_NUMERICSERV();
    my $flag_nofqdn      = $flags & NI_NOFQDN();      $flags &= ~NI_NOFQDN();
    my $flag_namereqd    = $flags & NI_NAMEREQD();    $flags &= ~NI_NAMEREQD();
    my $flag_dgram       = $flags & NI_DGRAM()   ;    $flags &= ~NI_DGRAM();

    $flags & NI_IDN() and
	croak "Socket::getnameinfo() does not support IDN";

    $flags == 0 or return fake_makeerr( EAI_BADFLAGS() );

    $xflags ||= 0;

    my $node;
    if( $xflags & NIx_NOHOST ) {
	$node = undef;
    }
    elsif( $flag_numerichost ) {
	$node = Socket::inet_ntoa( $inetaddr );
    }
    else {
	$node = gethostbyaddr( $inetaddr, $family );
	if( !defined $node ) {
	    return fake_makeerr( EAI_NONAME() ) if $flag_namereqd;
	    $node = Socket::inet_ntoa( $inetaddr );
	}
	elsif( $flag_nofqdn ) {
	    my ( $shortname ) = split m/\./, $node;
	    my ( $fqdn ) = gethostbyname $shortname;
	    $node = $shortname if defined $fqdn and $fqdn eq $node;
	}
    }

    my $service;
    if( $xflags & NIx_NOSERV ) {
	$service = undef;
    }
    elsif( $flag_numericserv ) {
	$service = "$port";
    }
    else {
	my $protname = $flag_dgram ? "udp" : "";
	$service = getservbyport( $port, $protname );
	if( !defined $service ) {
	    $service = "$port";
	}
    }

    return ( fake_makeerr( 0 ), $node, $service );
}

1;
