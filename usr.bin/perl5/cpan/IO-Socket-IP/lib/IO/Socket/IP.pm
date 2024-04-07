#  You may distribute under the terms of either the GNU General Public License
#  or the Artistic License (the same terms as Perl itself)
#
#  (C) Paul Evans, 2010-2020 -- leonerd@leonerd.org.uk

package IO::Socket::IP;

use v5;
use strict;
use warnings;

# $VERSION needs to be set before  use base 'IO::Socket'
#  - https://rt.cpan.org/Ticket/Display.html?id=92107
BEGIN {
   our $VERSION = '0.41_01';
   $VERSION = eval $VERSION;
}

use base qw( IO::Socket );

use Carp;

use Socket 1.97 qw(
   getaddrinfo getnameinfo
   sockaddr_family
   AF_INET
   AI_PASSIVE
   IPPROTO_TCP IPPROTO_UDP
   IPPROTO_IPV6 IPV6_V6ONLY
   NI_DGRAM NI_NUMERICHOST NI_NUMERICSERV NIx_NOHOST NIx_NOSERV
   SO_REUSEADDR SO_REUSEPORT SO_BROADCAST SO_ERROR
   SOCK_DGRAM SOCK_STREAM
   SOL_SOCKET
);
my $AF_INET6 = eval { Socket::AF_INET6() }; # may not be defined
my $AI_ADDRCONFIG = eval { Socket::AI_ADDRCONFIG() } || 0;
use POSIX qw( dup2 );
use Errno qw( EINVAL EINPROGRESS EISCONN ENOTCONN ETIMEDOUT EWOULDBLOCK EOPNOTSUPP );

use constant HAVE_MSWIN32 => ( $^O eq "MSWin32" );

# At least one OS (Android) is known not to have getprotobyname()
use constant HAVE_GETPROTOBYNAME => defined eval { getprotobyname( "tcp" ) };

my $IPv6_re = do {
   # translation of RFC 3986 3.2.2 ABNF to re
   my $IPv4address = do {
      my $dec_octet = q<(?:[0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])>;
      qq<$dec_octet(?: \\. $dec_octet){3}>;
   };
   my $IPv6address = do {
      my $h16  = qq<[0-9A-Fa-f]{1,4}>;
      my $ls32 = qq<(?: $h16 : $h16 | $IPv4address)>;
      qq<(?:
                                            (?: $h16 : ){6} $ls32
         |                               :: (?: $h16 : ){5} $ls32
         | (?:                   $h16 )? :: (?: $h16 : ){4} $ls32
         | (?: (?: $h16 : ){0,1} $h16 )? :: (?: $h16 : ){3} $ls32
         | (?: (?: $h16 : ){0,2} $h16 )? :: (?: $h16 : ){2} $ls32
         | (?: (?: $h16 : ){0,3} $h16 )? ::     $h16 :      $ls32
         | (?: (?: $h16 : ){0,4} $h16 )? ::                 $ls32
         | (?: (?: $h16 : ){0,5} $h16 )? ::                 $h16
         | (?: (?: $h16 : ){0,6} $h16 )? ::
      )>
   };
   qr<$IPv6address>xo;
};

=head1 NAME

C<IO::Socket::IP> - Family-neutral IP socket supporting both IPv4 and IPv6

=head1 SYNOPSIS

 use IO::Socket::IP;

 my $sock = IO::Socket::IP->new(
    PeerHost => "www.google.com",
    PeerPort => "http",
    Type     => SOCK_STREAM,
 ) or die "Cannot construct socket - $@";

 my $familyname = ( $sock->sockdomain == PF_INET6 ) ? "IPv6" :
                  ( $sock->sockdomain == PF_INET  ) ? "IPv4" :
                                                      "unknown";

 printf "Connected to google via %s\n", $familyname;

=head1 DESCRIPTION

This module provides a protocol-independent way to use IPv4 and IPv6 sockets,
intended as a replacement for L<IO::Socket::INET>. Most constructor arguments
and methods are provided in a backward-compatible way. For a list of known
differences, see the C<IO::Socket::INET> INCOMPATIBILITES section below.

It uses the C<getaddrinfo(3)> function to convert hostnames and service names
or port numbers into sets of possible addresses to connect to or listen on.
This allows it to work for IPv6 where the system supports it, while still
falling back to IPv4-only on systems which don't.

=head1 REPLACING C<IO::Socket> DEFAULT BEHAVIOUR

By placing C<-register> in the import list to C<IO::Socket::IP>, it will
register itself with L<IO::Socket> as the class that handles C<PF_INET>. It
will also ask to handle C<PF_INET6> as well, provided that constant is
available.

Changing C<IO::Socket>'s default behaviour means that calling the
C<IO::Socket> constructor with either C<PF_INET> or C<PF_INET6> as the
C<Domain> parameter will yield an C<IO::Socket::IP> object.

 use IO::Socket::IP -register;

 my $sock = IO::Socket->new(
    Domain    => PF_INET6,
    LocalHost => "::1",
    Listen    => 1,
 ) or die "Cannot create socket - $@\n";

 print "Created a socket of type " . ref($sock) . "\n";

Note that C<-register> is a global setting that applies to the entire program;
it cannot be applied only for certain callers, removed, or limited by lexical
scope.

=cut

sub import
{
   my $pkg = shift;
   my @symbols;

   foreach ( @_ ) {
      if( $_ eq "-register" ) {
         IO::Socket::IP::_ForINET->register_domain( AF_INET );
         IO::Socket::IP::_ForINET6->register_domain( $AF_INET6 ) if defined $AF_INET6;
      }
      else {
         push @symbols, $_;
      }
   }

   @_ = ( $pkg, @symbols );
   goto &IO::Socket::import;
}

# Convenient capability test function
{
   my $can_disable_v6only;
   sub CAN_DISABLE_V6ONLY
   {
      return $can_disable_v6only if defined $can_disable_v6only;

      socket my $testsock, Socket::PF_INET6(), SOCK_STREAM, 0 or
         die "Cannot socket(PF_INET6) - $!";

      if( setsockopt $testsock, IPPROTO_IPV6, IPV6_V6ONLY, 0 ) {
         if ($^O eq "dragonfly") {
            # dragonflybsd 6.4 lies about successfully turning this off
            if (getsockopt $testsock, IPPROTO_IPV6, IPV6_V6ONLY) {
               return $can_disable_v6only = 0;
            }
         }
         return $can_disable_v6only = 1;
      }
      elsif( $! == EINVAL || $! == EOPNOTSUPP ) {
         return $can_disable_v6only = 0;
      }
      else {
         die "Cannot setsockopt() - $!";
      }
   }
}

=head1 CONSTRUCTORS

=cut

=head2 new

   $sock = IO::Socket::IP->new( %args )

Creates a new C<IO::Socket::IP> object, containing a newly created socket
handle according to the named arguments passed. The recognised arguments are:

=over 8

=item PeerHost => STRING

=item PeerService => STRING

Hostname and service name for the peer to C<connect()> to. The service name
may be given as a port number, as a decimal string.

=item PeerAddr => STRING

=item PeerPort => STRING

For symmetry with the accessor methods and compatibility with
C<IO::Socket::INET>, these are accepted as synonyms for C<PeerHost> and
C<PeerService> respectively.

=item PeerAddrInfo => ARRAY

Alternate form of specifying the peer to C<connect()> to. This should be an
array of the form returned by C<Socket::getaddrinfo>.

This parameter takes precedence over the C<Peer*>, C<Family>, C<Type> and
C<Proto> arguments.

=item LocalHost => STRING

=item LocalService => STRING

Hostname and service name for the local address to C<bind()> to.

=item LocalAddr => STRING

=item LocalPort => STRING

For symmetry with the accessor methods and compatibility with
C<IO::Socket::INET>, these are accepted as synonyms for C<LocalHost> and
C<LocalService> respectively.

=item LocalAddrInfo => ARRAY

Alternate form of specifying the local address to C<bind()> to. This should be
an array of the form returned by C<Socket::getaddrinfo>.

This parameter takes precedence over the C<Local*>, C<Family>, C<Type> and
C<Proto> arguments.

=item Family => INT

The address family to pass to C<getaddrinfo> (e.g. C<AF_INET>, C<AF_INET6>).
Normally this will be left undefined, and C<getaddrinfo> will search using any
address family supported by the system.

=item Type => INT

The socket type to pass to C<getaddrinfo> (e.g. C<SOCK_STREAM>,
C<SOCK_DGRAM>). Normally defined by the caller; if left undefined
C<getaddrinfo> may attempt to infer the type from the service name.

=item Proto => STRING or INT

The IP protocol to use for the socket (e.g. C<'tcp'>, C<IPPROTO_TCP>,
C<'udp'>,C<IPPROTO_UDP>). Normally this will be left undefined, and either
C<getaddrinfo> or the kernel will choose an appropriate value. May be given
either in string name or numeric form.

=item GetAddrInfoFlags => INT

More flags to pass to the C<getaddrinfo()> function. If not supplied, a
default of C<AI_ADDRCONFIG> will be used.

These flags will be combined with C<AI_PASSIVE> if the C<Listen> argument is
given. For more information see the documentation about C<getaddrinfo()> in
the L<Socket> module.

=item Listen => INT

If defined, puts the socket into listening mode where new connections can be
accepted using the C<accept> method. The value given is used as the
C<listen(2)> queue size.

=item ReuseAddr => BOOL

If true, set the C<SO_REUSEADDR> sockopt

=item ReusePort => BOOL

If true, set the C<SO_REUSEPORT> sockopt (not all OSes implement this sockopt)

=item Broadcast => BOOL

If true, set the C<SO_BROADCAST> sockopt

=item Sockopts => ARRAY

An optional array of other socket options to apply after the three listed
above. The value is an ARRAY containing 2- or 3-element ARRAYrefs. Each inner
array relates to a single option, giving the level and option name, and an
optional value. If the value element is missing, it will be given the value of
a platform-sized integer 1 constant (i.e. suitable to enable most of the
common boolean options).

For example, both options given below are equivalent to setting C<ReuseAddr>.

 Sockopts => [
    [ SOL_SOCKET, SO_REUSEADDR ],
    [ SOL_SOCKET, SO_REUSEADDR, pack( "i", 1 ) ],
 ]

=item V6Only => BOOL

If defined, set the C<IPV6_V6ONLY> sockopt when creating C<PF_INET6> sockets
to the given value. If true, a listening-mode socket will only listen on the
C<AF_INET6> addresses; if false it will also accept connections from
C<AF_INET> addresses.

If not defined, the socket option will not be changed, and default value set
by the operating system will apply. For repeatable behaviour across platforms
it is recommended this value always be defined for listening-mode sockets.

Note that not all platforms support disabling this option. Some, at least
OpenBSD and MirBSD, will fail with C<EINVAL> if you attempt to disable it.
To determine whether it is possible to disable, you may use the class method

 if( IO::Socket::IP->CAN_DISABLE_V6ONLY ) {
    ...
 }
 else {
    ...
 }

If your platform does not support disabling this option but you still want to
listen for both C<AF_INET> and C<AF_INET6> connections you will have to create
two listening sockets, one bound to each protocol.

=item MultiHomed

This C<IO::Socket::INET>-style argument is ignored, except if it is defined
but false. See the C<IO::Socket::INET> INCOMPATIBILITES section below.

However, the behaviour it enables is always performed by C<IO::Socket::IP>.

=item Blocking => BOOL

If defined but false, the socket will be set to non-blocking mode. Otherwise
it will default to blocking mode. See the NON-BLOCKING section below for more
detail.

=item Timeout => NUM

If defined, gives a maximum time in seconds to block per C<connect()> call
when in blocking mode. If missing, no timeout is applied other than that
provided by the underlying operating system. When in non-blocking mode this
parameter is ignored.

Note that if the hostname resolves to multiple address candidates, the same
timeout will apply to each connection attempt individually, rather than to the
operation as a whole. Further note that the timeout does not apply to the
initial hostname resolve operation, if connecting by hostname.

This behviour is copied inspired by C<IO::Socket::INET>; for more fine grained
control over connection timeouts, consider performing a nonblocking connect
directly.

=back

If neither C<Type> nor C<Proto> hints are provided, a default of
C<SOCK_STREAM> and C<IPPROTO_TCP> respectively will be set, to maintain
compatibility with C<IO::Socket::INET>. Other named arguments that are not
recognised are ignored.

If neither C<Family> nor any hosts or addresses are passed, nor any
C<*AddrInfo>, then the constructor has no information on which to decide a
socket family to create. In this case, it performs a C<getaddinfo> call with
the C<AI_ADDRCONFIG> flag, no host name, and a service name of C<"0">, and
uses the family of the first returned result.

If the constructor fails, it will set C<$@> to an appropriate error message;
this may be from C<$!> or it may be some other string; not every failure
necessarily has an associated C<errno> value.

=head2 new (one arg)

   $sock = IO::Socket::IP->new( $peeraddr )

As a special case, if the constructor is passed a single argument (as
opposed to an even-sized list of key/value pairs), it is taken to be the value
of the C<PeerAddr> parameter. This is parsed in the same way, according to the
behaviour given in the C<PeerHost> AND C<LocalHost> PARSING section below.

=cut

sub new
{
   my $class = shift;
   my %arg = (@_ == 1) ? (PeerHost => $_[0]) : @_;
   return $class->SUPER::new(%arg);
}

# IO::Socket may call this one; neaten up the arguments from IO::Socket::INET
# before calling our real _configure method
sub configure
{
   my $self = shift;
   my ( $arg ) = @_;

   $arg->{PeerHost} = delete $arg->{PeerAddr}
      if exists $arg->{PeerAddr} && !exists $arg->{PeerHost};

   $arg->{PeerService} = delete $arg->{PeerPort}
      if exists $arg->{PeerPort} && !exists $arg->{PeerService};

   $arg->{LocalHost} = delete $arg->{LocalAddr}
      if exists $arg->{LocalAddr} && !exists $arg->{LocalHost};

   $arg->{LocalService} = delete $arg->{LocalPort}
      if exists $arg->{LocalPort} && !exists $arg->{LocalService};

   for my $type (qw(Peer Local)) {
      my $host    = $type . 'Host';
      my $service = $type . 'Service';

      if( defined $arg->{$host} ) {
         ( $arg->{$host}, my $s ) = $self->split_addr( $arg->{$host} );
         # IO::Socket::INET compat - *Host parsed port always takes precedence
         $arg->{$service} = $s if defined $s;
      }
   }

   $self->_io_socket_ip__configure( $arg );
}

# Avoid simply calling it _configure, as some subclasses of IO::Socket::INET on CPAN already take that
sub _io_socket_ip__configure
{
   my $self = shift;
   my ( $arg ) = @_;

   my %hints;
   my @localinfos;
   my @peerinfos;

   my $listenqueue = $arg->{Listen};
   if( defined $listenqueue and
       ( defined $arg->{PeerHost} || defined $arg->{PeerService} || defined $arg->{PeerAddrInfo} ) ) {
      croak "Cannot Listen with a peer address";
   }

   if( defined $arg->{GetAddrInfoFlags} ) {
      $hints{flags} = $arg->{GetAddrInfoFlags};
   }
   else {
      $hints{flags} = $AI_ADDRCONFIG;
   }

   if( defined( my $family = $arg->{Family} ) ) {
      $hints{family} = $family;
   }

   if( defined( my $type = $arg->{Type} ) ) {
      $hints{socktype} = $type;
   }

   if( defined( my $proto = $arg->{Proto} ) ) {
      unless( $proto =~ m/^\d+$/ ) {
         my $protonum = HAVE_GETPROTOBYNAME
            ? getprotobyname( $proto )
            : eval { Socket->${\"IPPROTO_\U$proto"}() };
         defined $protonum or croak "Unrecognised protocol $proto";
         $proto = $protonum;
      }

      $hints{protocol} = $proto;
   }

   # To maintain compatibility with IO::Socket::INET, imply a default of
   # SOCK_STREAM + IPPROTO_TCP if neither hint is given
   if( !defined $hints{socktype} and !defined $hints{protocol} ) {
      $hints{socktype} = SOCK_STREAM;
      $hints{protocol} = IPPROTO_TCP;
   }

   # Some OSes (NetBSD) don't seem to like just a protocol hint without a
   # socktype hint as well. We'll set a couple of common ones
   if( !defined $hints{socktype} and defined $hints{protocol} ) {
      $hints{socktype} = SOCK_STREAM if $hints{protocol} == IPPROTO_TCP;
      $hints{socktype} = SOCK_DGRAM  if $hints{protocol} == IPPROTO_UDP;
   }

   if( my $info = $arg->{LocalAddrInfo} ) {
      ref $info eq "ARRAY" or croak "Expected 'LocalAddrInfo' to be an ARRAY ref";
      @localinfos = @$info;
   }
   elsif( defined $arg->{LocalHost} or
          defined $arg->{LocalService} or
          HAVE_MSWIN32 and $arg->{Listen} ) {
      # Either may be undef
      my $host = $arg->{LocalHost};
      my $service = $arg->{LocalService};

      unless ( defined $host or defined $service ) {
         $service = 0;
      }

      local $1; # Placate a taint-related bug; [perl #67962]
      defined $service and $service =~ s/\((\d+)\)$// and
         my $fallback_port = $1;

      my %localhints = %hints;
      $localhints{flags} |= AI_PASSIVE;
      ( my $err, @localinfos ) = getaddrinfo( $host, $service, \%localhints );

      if( $err and defined $fallback_port ) {
         ( $err, @localinfos ) = getaddrinfo( $host, $fallback_port, \%localhints );
      }

      if( $err ) {
         $@ = "$err";
         $! = EINVAL;
         return;
      }
   }

   if( my $info = $arg->{PeerAddrInfo} ) {
      ref $info eq "ARRAY" or croak "Expected 'PeerAddrInfo' to be an ARRAY ref";
      @peerinfos = @$info;
   }
   elsif( defined $arg->{PeerHost} or defined $arg->{PeerService} ) {
      defined( my $host = $arg->{PeerHost} ) or
         croak "Expected 'PeerHost'";
      defined( my $service = $arg->{PeerService} ) or
         croak "Expected 'PeerService'";

      local $1; # Placate a taint-related bug; [perl #67962]
      defined $service and $service =~ s/\((\d+)\)$// and
         my $fallback_port = $1;

      ( my $err, @peerinfos ) = getaddrinfo( $host, $service, \%hints );

      if( $err and defined $fallback_port ) {
         ( $err, @peerinfos ) = getaddrinfo( $host, $fallback_port, \%hints );
      }

      if( $err ) {
         $@ = "$err";
         $! = EINVAL;
         return;
      }
   }

   my $INT_1 = pack "i", 1;

   my @sockopts_enabled;
   push @sockopts_enabled, [ SOL_SOCKET, SO_REUSEADDR, $INT_1 ] if $arg->{ReuseAddr};
   push @sockopts_enabled, [ SOL_SOCKET, SO_REUSEPORT, $INT_1 ] if $arg->{ReusePort};
   push @sockopts_enabled, [ SOL_SOCKET, SO_BROADCAST, $INT_1 ] if $arg->{Broadcast};

   if( my $sockopts = $arg->{Sockopts} ) {
      ref $sockopts eq "ARRAY" or croak "Expected 'Sockopts' to be an ARRAY ref";
      foreach ( @$sockopts ) {
         ref $_ eq "ARRAY" or croak "Bad Sockopts item - expected ARRAYref";
         @$_ >= 2 and @$_ <= 3 or
            croak "Bad Sockopts item - expected 2 or 3 elements";

         my ( $level, $optname, $value ) = @$_;
         # TODO: consider more sanity checking on argument values

         defined $value or $value = $INT_1;
         push @sockopts_enabled, [ $level, $optname, $value ];
      }
   }

   my $blocking = $arg->{Blocking};
   defined $blocking or $blocking = 1;

   my $v6only = $arg->{V6Only};

   # IO::Socket::INET defines this key. IO::Socket::IP always implements the
   # behaviour it requests, so we can ignore it, unless the caller is for some
   # reason asking to disable it.
   if( defined $arg->{MultiHomed} and !$arg->{MultiHomed} ) {
      croak "Cannot disable the MultiHomed parameter";
   }

   my @infos;
   foreach my $local ( @localinfos ? @localinfos : {} ) {
      foreach my $peer ( @peerinfos ? @peerinfos : {} ) {
         next if defined $local->{family}   and defined $peer->{family}   and
            $local->{family} != $peer->{family};
         next if defined $local->{socktype} and defined $peer->{socktype} and
            $local->{socktype} != $peer->{socktype};
         next if defined $local->{protocol} and defined $peer->{protocol} and
            $local->{protocol} != $peer->{protocol};

         my $family   = $local->{family}   || $peer->{family}   or next;
         my $socktype = $local->{socktype} || $peer->{socktype} or next;
         my $protocol = $local->{protocol} || $peer->{protocol} || 0;

         push @infos, {
            family    => $family,
            socktype  => $socktype,
            protocol  => $protocol,
            localaddr => $local->{addr},
            peeraddr  => $peer->{addr},
         };
      }
   }

   if( !@infos ) {
      # If there was a Family hint then create a plain unbound, unconnected socket
      if( defined $hints{family} ) {
         @infos = ( {
            family   => $hints{family},
            socktype => $hints{socktype},
            protocol => $hints{protocol},
         } );
      }
      # If there wasn't, use getaddrinfo()'s AI_ADDRCONFIG side-effect to guess a
      # suitable family first.
      else {
         ( my $err, @infos ) = getaddrinfo( "", "0", \%hints );
         if( $err ) {
            $@ = "$err";
            $! = EINVAL;
            return;
         }

         # We'll take all the @infos anyway, because some OSes (HPUX) are known to
         # ignore the AI_ADDRCONFIG hint and return AF_INET6 even if they don't
         # support them
      }
   }

   # In the nonblocking case, caller will be calling ->setup multiple times.
   # Store configuration in the object for the ->setup method
   # Yes, these are messy. Sorry, I can't help that...

   ${*$self}{io_socket_ip_infos} = \@infos;

   ${*$self}{io_socket_ip_idx} = -1;

   ${*$self}{io_socket_ip_sockopts} = \@sockopts_enabled;
   ${*$self}{io_socket_ip_v6only} = $v6only;
   ${*$self}{io_socket_ip_listenqueue} = $listenqueue;
   ${*$self}{io_socket_ip_blocking} = $blocking;

   ${*$self}{io_socket_ip_errors} = [ undef, undef, undef ];

   # ->setup is allowed to return false in nonblocking mode
   $self->setup or !$blocking or return undef;

   return $self;
}

sub setup
{
   my $self = shift;

   while(1) {
      ${*$self}{io_socket_ip_idx}++;
      last if ${*$self}{io_socket_ip_idx} >= @{ ${*$self}{io_socket_ip_infos} };

      my $info = ${*$self}{io_socket_ip_infos}->[${*$self}{io_socket_ip_idx}];

      $self->socket( @{$info}{qw( family socktype protocol )} ) or
         ( ${*$self}{io_socket_ip_errors}[2] = $!, next );

      $self->blocking( 0 ) unless ${*$self}{io_socket_ip_blocking};

      foreach my $sockopt ( @{ ${*$self}{io_socket_ip_sockopts} } ) {
         my ( $level, $optname, $value ) = @$sockopt;
         $self->setsockopt( $level, $optname, $value ) or ( $@ = "$!", return undef );
      }

      if( defined ${*$self}{io_socket_ip_v6only} and defined $AF_INET6 and $info->{family} == $AF_INET6 ) {
         my $v6only = ${*$self}{io_socket_ip_v6only};
         $self->setsockopt( IPPROTO_IPV6, IPV6_V6ONLY, pack "i", $v6only ) or ( $@ = "$!", return undef );
      }

      if( defined( my $addr = $info->{localaddr} ) ) {
         $self->bind( $addr ) or
            ( ${*$self}{io_socket_ip_errors}[1] = $!, next );
      }

      if( defined( my $listenqueue = ${*$self}{io_socket_ip_listenqueue} ) ) {
         $self->listen( $listenqueue ) or ( $@ = "$!", return undef );
      }

      if( defined( my $addr = $info->{peeraddr} ) ) {
         if( $self->connect( $addr ) ) {
            $! = 0;
            return 1;
         }

         if( $! == EINPROGRESS or $! == EWOULDBLOCK ) {
            ${*$self}{io_socket_ip_connect_in_progress} = 1;
            return 0;
         }

         # If connect failed but we have no system error there must be an error
         # at the application layer, like a bad certificate with
         # IO::Socket::SSL.
         # In this case don't continue IP based multi-homing because the problem
         # cannot be solved at the IP layer.
         return 0 if ! $!;

         ${*$self}{io_socket_ip_errors}[0] = $!;
         next;
      }

      return 1;
   }

   # Pick the most appropriate error, stringified
   $! = ( grep defined, @{ ${*$self}{io_socket_ip_errors}} )[0];
   $@ = "$!";
   return undef;
}

sub connect :method
{
   my $self = shift;

   # It seems that IO::Socket hides EINPROGRESS errors, making them look like
   # a success. This is annoying here.
   # Instead of putting up with its frankly-irritating intentional breakage of
   # useful APIs I'm just going to end-run around it and call core's connect()
   # directly

   if( @_ ) {
      my ( $addr ) = @_;

      # Annoyingly IO::Socket's connect() is where the timeout logic is
      # implemented, so we'll have to reinvent it here
      my $timeout = ${*$self}{'io_socket_timeout'};

      return connect( $self, $addr ) unless defined $timeout;

      my $was_blocking = $self->blocking( 0 );

      my $err = defined connect( $self, $addr ) ? 0 : $!+0;

      if( !$err ) {
         # All happy
         $self->blocking( $was_blocking );
         return 1;
      }
      elsif( not( $err == EINPROGRESS or $err == EWOULDBLOCK ) ) {
         # Failed for some other reason
         $self->blocking( $was_blocking );
         return undef;
      }
      elsif( !$was_blocking ) {
         # We shouldn't block anyway
         return undef;
      }

      my $vec = ''; vec( $vec, $self->fileno, 1 ) = 1;
      if( !select( undef, $vec, $vec, $timeout ) ) {
         $self->blocking( $was_blocking );
         $! = ETIMEDOUT;
         return undef;
      }

      # Hoist the error by connect()ing a second time
      $err = $self->getsockopt( SOL_SOCKET, SO_ERROR );
      $err = 0 if $err == EISCONN; # Some OSes give EISCONN

      $self->blocking( $was_blocking );

      $! = $err, return undef if $err;
      return 1;
   }

   return 1 if !${*$self}{io_socket_ip_connect_in_progress};

   # See if a connect attempt has just failed with an error
   if( my $errno = $self->getsockopt( SOL_SOCKET, SO_ERROR ) ) {
      delete ${*$self}{io_socket_ip_connect_in_progress};
      ${*$self}{io_socket_ip_errors}[0] = $! = $errno;
      return $self->setup;
   }

   # No error, so either connect is still in progress, or has completed
   # successfully. We can tell by trying to connect() again; either it will
   # succeed or we'll get EISCONN (connected successfully), or EALREADY
   # (still in progress). This even works on MSWin32.
   my $addr = ${*$self}{io_socket_ip_infos}[${*$self}{io_socket_ip_idx}]{peeraddr};

   if( connect( $self, $addr ) or $! == EISCONN ) {
      delete ${*$self}{io_socket_ip_connect_in_progress};
      $! = 0;
      return 1;
   }
   else {
      $! = EINPROGRESS;
      return 0;
   }
}

sub connected
{
   my $self = shift;
   return defined $self->fileno &&
          !${*$self}{io_socket_ip_connect_in_progress} &&
          defined getpeername( $self ); # ->peername caches, we need to detect disconnection
}

=head1 METHODS

As well as the following methods, this class inherits all the methods in
L<IO::Socket> and L<IO::Handle>.

=cut

sub _get_host_service
{
   my $self = shift;
   my ( $addr, $flags, $xflags ) = @_;

   defined $addr or
      $! = ENOTCONN, return;

   $flags |= NI_DGRAM if $self->socktype == SOCK_DGRAM;

   my ( $err, $host, $service ) = getnameinfo( $addr, $flags, $xflags || 0 );
   croak "getnameinfo - $err" if $err;

   return ( $host, $service );
}

sub _unpack_sockaddr
{
   my ( $addr ) = @_;
   my $family = sockaddr_family $addr;

   if( $family == AF_INET ) {
      return ( Socket::unpack_sockaddr_in( $addr ) )[1];
   }
   elsif( defined $AF_INET6 and $family == $AF_INET6 ) {
      return ( Socket::unpack_sockaddr_in6( $addr ) )[1];
   }
   else {
      croak "Unrecognised address family $family";
   }
}

=head2 sockhost_service

   ( $host, $service ) = $sock->sockhost_service( $numeric )

Returns the hostname and service name of the local address (that is, the
socket address given by the C<sockname> method).

If C<$numeric> is true, these will be given in numeric form rather than being
resolved into names.

The following four convenience wrappers may be used to obtain one of the two
values returned here. If both host and service names are required, this method
is preferable to the following wrappers, because it will call
C<getnameinfo(3)> only once.

=cut

sub sockhost_service
{
   my $self = shift;
   my ( $numeric ) = @_;

   $self->_get_host_service( $self->sockname, $numeric ? NI_NUMERICHOST|NI_NUMERICSERV : 0 );
}

=head2 sockhost

   $addr = $sock->sockhost

Return the numeric form of the local address as a textual representation

=head2 sockport

   $port = $sock->sockport

Return the numeric form of the local port number

=head2 sockhostname

   $host = $sock->sockhostname

Return the resolved name of the local address

=head2 sockservice

   $service = $sock->sockservice

Return the resolved name of the local port number

=cut

sub sockhost { my $self = shift; scalar +( $self->_get_host_service( $self->sockname, NI_NUMERICHOST, NIx_NOSERV ) )[0] }
sub sockport { my $self = shift; scalar +( $self->_get_host_service( $self->sockname, NI_NUMERICSERV, NIx_NOHOST ) )[1] }

sub sockhostname { my $self = shift; scalar +( $self->_get_host_service( $self->sockname, 0, NIx_NOSERV ) )[0] }
sub sockservice  { my $self = shift; scalar +( $self->_get_host_service( $self->sockname, 0, NIx_NOHOST ) )[1] }

=head2 sockaddr

   $addr = $sock->sockaddr

Return the local address as a binary octet string

=cut

sub sockaddr { my $self = shift; _unpack_sockaddr $self->sockname }

=head2 peerhost_service

   ( $host, $service ) = $sock->peerhost_service( $numeric )

Returns the hostname and service name of the peer address (that is, the
socket address given by the C<peername> method), similar to the
C<sockhost_service> method.

The following four convenience wrappers may be used to obtain one of the two
values returned here. If both host and service names are required, this method
is preferable to the following wrappers, because it will call
C<getnameinfo(3)> only once.

=cut

sub peerhost_service
{
   my $self = shift;
   my ( $numeric ) = @_;

   $self->_get_host_service( $self->peername, $numeric ? NI_NUMERICHOST|NI_NUMERICSERV : 0 );
}

=head2 peerhost

   $addr = $sock->peerhost

Return the numeric form of the peer address as a textual representation

=head2 peerport

   $port = $sock->peerport

Return the numeric form of the peer port number

=head2 peerhostname

   $host = $sock->peerhostname

Return the resolved name of the peer address

=head2 peerservice

   $service = $sock->peerservice

Return the resolved name of the peer port number

=cut

sub peerhost { my $self = shift; scalar +( $self->_get_host_service( $self->peername, NI_NUMERICHOST, NIx_NOSERV ) )[0] }
sub peerport { my $self = shift; scalar +( $self->_get_host_service( $self->peername, NI_NUMERICSERV, NIx_NOHOST ) )[1] }

sub peerhostname { my $self = shift; scalar +( $self->_get_host_service( $self->peername, 0, NIx_NOSERV ) )[0] }
sub peerservice  { my $self = shift; scalar +( $self->_get_host_service( $self->peername, 0, NIx_NOHOST ) )[1] }

=head2 peeraddr

   $addr = $peer->peeraddr

Return the peer address as a binary octet string

=cut

sub peeraddr { my $self = shift; _unpack_sockaddr $self->peername }

# This unbelievably dodgy hack works around the bug that IO::Socket doesn't do
# it
#    https://rt.cpan.org/Ticket/Display.html?id=61577
sub accept
{
   my $self = shift;
   my ( $new, $peer ) = $self->SUPER::accept( @_ ) or return;

   ${*$new}{$_} = ${*$self}{$_} for qw( io_socket_domain io_socket_type io_socket_proto );

   return wantarray ? ( $new, $peer )
                    : $new;
}

# This second unbelievably dodgy hack guarantees that $self->fileno doesn't
# change, which is useful during nonblocking connect
sub socket :method
{
   my $self = shift;
   return $self->SUPER::socket(@_) if not defined $self->fileno;

   # I hate core prototypes sometimes...
   socket( my $tmph, $_[0], $_[1], $_[2] ) or return undef;

   dup2( $tmph->fileno, $self->fileno ) or die "Unable to dup2 $tmph onto $self - $!";
}

# Versions of IO::Socket before 1.35 may leave socktype undef if from, say, an
#   ->fdopen call. In this case we'll apply a fix
BEGIN {
   if( eval($IO::Socket::VERSION) < 1.35 ) {
      *socktype = sub {
         my $self = shift;
         my $type = $self->SUPER::socktype;
         if( !defined $type ) {
            $type = $self->sockopt( Socket::SO_TYPE() );
         }
         return $type;
      };
   }
}

=head2 as_inet

   $inet = $sock->as_inet

Returns a new L<IO::Socket::INET> instance wrapping the same filehandle. This
may be useful in cases where it is required, for backward-compatibility, to
have a real object of C<IO::Socket::INET> type instead of C<IO::Socket::IP>.
The new object will wrap the same underlying socket filehandle as the
original, so care should be taken not to continue to use both objects
concurrently. Ideally the original C<$sock> should be discarded after this
method is called.

This method checks that the socket domain is C<PF_INET> and will throw an
exception if it isn't.

=cut

sub as_inet
{
   my $self = shift;
   croak "Cannot downgrade a non-PF_INET socket to IO::Socket::INET" unless $self->sockdomain == AF_INET;
   return IO::Socket::INET->new_from_fd( $self->fileno, "r+" );
}

=head1 NON-BLOCKING

If the constructor is passed a defined but false value for the C<Blocking>
argument then the socket is put into non-blocking mode. When in non-blocking
mode, the socket will not be set up by the time the constructor returns,
because the underlying C<connect(2)> syscall would otherwise have to block.

The non-blocking behaviour is an extension of the C<IO::Socket::INET> API,
unique to C<IO::Socket::IP>, because the former does not support multi-homed
non-blocking connect.

When using non-blocking mode, the caller must repeatedly check for
writeability on the filehandle (for instance using C<select> or C<IO::Poll>).
Each time the filehandle is ready to write, the C<connect> method must be
called, with no arguments. Note that some operating systems, most notably
C<MSWin32> do not report a C<connect()> failure using write-ready; so you must
also C<select()> for exceptional status.

While C<connect> returns false, the value of C<$!> indicates whether it should
be tried again (by being set to the value C<EINPROGRESS>, or C<EWOULDBLOCK> on
MSWin32), or whether a permanent error has occurred (e.g. C<ECONNREFUSED>).

Once the socket has been connected to the peer, C<connect> will return true
and the socket will now be ready to use.

Note that calls to the platform's underlying C<getaddrinfo(3)> function may
block. If C<IO::Socket::IP> has to perform this lookup, the constructor will
block even when in non-blocking mode.

To avoid this blocking behaviour, the caller should pass in the result of such
a lookup using the C<PeerAddrInfo> or C<LocalAddrInfo> arguments. This can be
achieved by using L<Net::LibAsyncNS>, or the C<getaddrinfo(3)> function can be
called in a child process.

 use IO::Socket::IP;
 use Errno qw( EINPROGRESS EWOULDBLOCK );

 my @peeraddrinfo = ... # Caller must obtain the getaddinfo result here

 my $socket = IO::Socket::IP->new(
    PeerAddrInfo => \@peeraddrinfo,
    Blocking     => 0,
 ) or die "Cannot construct socket - $@";

 while( !$socket->connect and ( $! == EINPROGRESS || $! == EWOULDBLOCK ) ) {
    my $wvec = '';
    vec( $wvec, fileno $socket, 1 ) = 1;
    my $evec = '';
    vec( $evec, fileno $socket, 1 ) = 1;

    select( undef, $wvec, $evec, undef ) or die "Cannot select - $!";
 }

 die "Cannot connect - $!" if $!;

 ...

The example above uses C<select()>, but any similar mechanism should work
analogously. C<IO::Socket::IP> takes care when creating new socket filehandles
to preserve the actual file descriptor number, so such techniques as C<poll>
or C<epoll> should be transparent to its reallocation of a different socket
underneath, perhaps in order to switch protocol family between C<PF_INET> and
C<PF_INET6>.

For another example using C<IO::Poll> and C<Net::LibAsyncNS>, see the
F<examples/nonblocking_libasyncns.pl> file in the module distribution.

=cut

=head1 C<PeerHost> AND C<LocalHost> PARSING

To support the C<IO::Socket::INET> API, the host and port information may be
passed in a single string rather than as two separate arguments.

If either C<LocalHost> or C<PeerHost> (or their C<...Addr> synonyms) have any
of the following special forms then special parsing is applied.

The value of the C<...Host> argument will be split to give both the hostname
and port (or service name):

 hostname.example.org:http    # Host name
 192.0.2.1:80                 # IPv4 address
 [2001:db8::1]:80             # IPv6 address

In each case, the port or service name (e.g. C<80>) is passed as the
C<LocalService> or C<PeerService> argument.

Either of C<LocalService> or C<PeerService> (or their C<...Port> synonyms) can
be either a service name, a decimal number, or a string containing both a
service name and number, in a form such as

 http(80)

In this case, the name (C<http>) will be tried first, but if the resolver does
not understand it then the port number (C<80>) will be used instead.

If the C<...Host> argument is in this special form and the corresponding
C<...Service> or C<...Port> argument is also defined, the one parsed from
the C<...Host> argument will take precedence and the other will be ignored.

=head2 split_addr

   ( $host, $port ) = IO::Socket::IP->split_addr( $addr )

Utility method that provides the parsing functionality described above.
Returns a 2-element list, containing either the split hostname and port
description if it could be parsed, or the given address and C<undef> if it was
not recognised.

 IO::Socket::IP->split_addr( "hostname:http" )
                              # ( "hostname",  "http" )

 IO::Socket::IP->split_addr( "192.0.2.1:80" )
                              # ( "192.0.2.1", "80"   )

 IO::Socket::IP->split_addr( "[2001:db8::1]:80" )
                              # ( "2001:db8::1", "80" )

 IO::Socket::IP->split_addr( "something.else" )
                              # ( "something.else", undef )

=cut

sub split_addr
{
   shift;
   my ( $addr ) = @_;

   local ( $1, $2 ); # Placate a taint-related bug; [perl #67962]
   if( $addr =~ m/\A\[($IPv6_re)\](?::([^\s:]*))?\z/ or
       $addr =~ m/\A([^\s:]*):([^\s:]*)\z/ ) {
      return ( $1, $2 ) if defined $2 and length $2;
      return ( $1, undef );
   }

   return ( $addr, undef );
}

=head2 join_addr

   $addr = IO::Socket::IP->join_addr( $host, $port )

Utility method that performs the reverse of C<split_addr>, returning a string
formed by joining the specified host address and port number. The host address
will be wrapped in C<[]> brackets if required (because it is a raw IPv6
numeric address).

This can be especially useful when combined with the C<sockhost_service> or
C<peerhost_service> methods.

 say "Connected to ", IO::Socket::IP->join_addr( $sock->peerhost_service );

=cut

sub join_addr
{
   shift;
   my ( $host, $port ) = @_;

   $host = "[$host]" if $host =~ m/:/;

   return join ":", $host, $port if defined $port;
   return $host;
}

# Since IO::Socket->new( Domain => ... ) will delete the Domain parameter
# before calling ->configure, we need to keep track of which it was

package # hide from indexer
   IO::Socket::IP::_ForINET;
use base qw( IO::Socket::IP );

sub configure
{
   # This is evil
   my $self = shift;
   my ( $arg ) = @_;

   bless $self, "IO::Socket::IP";
   $self->configure( { %$arg, Family => Socket::AF_INET() } );
}

package # hide from indexer
   IO::Socket::IP::_ForINET6;
use base qw( IO::Socket::IP );

sub configure
{
   # This is evil
   my $self = shift;
   my ( $arg ) = @_;

   bless $self, "IO::Socket::IP";
   $self->configure( { %$arg, Family => Socket::AF_INET6() } );
}

=head1 C<IO::Socket::INET> INCOMPATIBILITES

=over 4

=item *

The behaviour enabled by C<MultiHomed> is in fact implemented by
C<IO::Socket::IP> as it is required to correctly support searching for a
useable address from the results of the C<getaddrinfo(3)> call. The
constructor will ignore the value of this argument, except if it is defined
but false. An exception is thrown in this case, because that would request it
disable the C<getaddrinfo(3)> search behaviour in the first place.

=item *

C<IO::Socket::IP> implements both the C<Blocking> and C<Timeout> parameters,
but it implements the interaction of both in a different way.

In C<::INET>, supplying a timeout overrides the non-blocking behaviour,
meaning that the C<connect()> operation will still block despite that the
caller asked for a non-blocking socket. This is not explicitly specified in
its documentation, nor does this author believe that is a useful behaviour -
it appears to come from a quirk of implementation.

In C<::IP> therefore, the C<Blocking> parameter takes precedence - if a
non-blocking socket is requested, no operation will block. The C<Timeout>
parameter here simply defines the maximum time that a blocking C<connect()>
call will wait, if it blocks at all.

In order to specifically obtain the "blocking connect then non-blocking send
and receive" behaviour of specifying this combination of options to C<::INET>
when using C<::IP>, perform first a blocking connect, then afterwards turn the
socket into nonblocking mode.

 my $sock = IO::Socket::IP->new(
    PeerHost => $peer,
    Timeout => 20,
 ) or die "Cannot connect - $@";

 $sock->blocking( 0 );

This code will behave identically under both C<IO::Socket::INET> and
C<IO::Socket::IP>.

=back

=cut

=head1 TODO

=over 4

=item *

Investigate whether C<POSIX::dup2> upsets BSD's C<kqueue> watchers, and if so,
consider what possible workarounds might be applied.

=back

=head1 AUTHOR

Paul Evans <leonerd@leonerd.org.uk>

=cut

0x55AA;
