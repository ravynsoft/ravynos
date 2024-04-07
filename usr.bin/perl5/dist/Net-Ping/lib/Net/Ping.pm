package Net::Ping;

require 5.002;
require Exporter;

use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK $VERSION
            $def_timeout $def_proto $def_factor $def_family
            $max_datasize $pingstring $hires $source_verify $syn_forking);
use Fcntl qw( F_GETFL F_SETFL O_NONBLOCK );
use Socket 2.007;
use Socket qw( SOCK_DGRAM SOCK_STREAM SOCK_RAW AF_INET PF_INET IPPROTO_TCP
	       SOL_SOCKET SO_ERROR SO_BROADCAST
               IPPROTO_IP IP_TOS IP_TTL
               inet_ntoa inet_aton getnameinfo sockaddr_in );
use POSIX qw( ENOTCONN ECONNREFUSED ECONNRESET EINPROGRESS EWOULDBLOCK EAGAIN
	      WNOHANG );
use FileHandle;
use Carp;
use Time::HiRes;

@ISA = qw(Exporter);
@EXPORT = qw(pingecho);
@EXPORT_OK = qw(wakeonlan);
$VERSION = "2.76";

# Globals

$def_timeout = 5;           # Default timeout to wait for a reply
$def_proto = "tcp";         # Default protocol to use for pinging
$def_factor = 1.2;          # Default exponential backoff rate.
$def_family = AF_INET;      # Default family.
$max_datasize = 65535;      # Maximum data bytes. recommended: 1472 (Ethernet MTU: 1500)
# The data we exchange with the server for the stream protocol
$pingstring = "pingschwingping!\n";
$source_verify = 1;         # Default is to verify source endpoint
$syn_forking = 0;

# Constants

my $AF_INET6  = eval { Socket::AF_INET6() } || 30;
my $AF_UNSPEC = eval { Socket::AF_UNSPEC() };
my $AI_NUMERICHOST = eval { Socket::AI_NUMERICHOST() } || 4;
my $NI_NUMERICHOST = eval { Socket::NI_NUMERICHOST() } || 2;
my $IPPROTO_IPV6   = eval { Socket::IPPROTO_IPV6() }   || 41;
my $NIx_NOSERV = eval { Socket::NIx_NOSERV() } || 2;
#my $IPV6_HOPLIMIT  = eval { Socket::IPV6_HOPLIMIT() };  # ping6 -h 0-255
my $qr_family = qr/^(?:(?:(:?ip)?v?(?:4|6))|${\AF_INET}|$AF_INET6)$/;
my $qr_family4 = qr/^(?:(?:(:?ip)?v?4)|${\AF_INET})$/;
my $Socket_VERSION = eval $Socket::VERSION;

if ($^O =~ /Win32/i) {
  # Hack to avoid this Win32 spewage:
  # Your vendor has not defined POSIX macro ECONNREFUSED
  my @pairs = (ECONNREFUSED => 10061, # "Unknown Error" Special Win32 Response?
	       ENOTCONN     => 10057,
	       ECONNRESET   => 10054,
	       EINPROGRESS  => 10036,
	       EWOULDBLOCK  => 10035,
	  );
  while (my $name = shift @pairs) {
    my $value = shift @pairs;
    # When defined, these all are non-zero
    unless (eval $name) {
      no strict 'refs';
      *{$name} = defined prototype \&{$name} ? sub () {$value} : sub {$value};
    }
  }
#  $syn_forking = 1;    # XXX possibly useful in < Win2K ?
};

# Description:  The pingecho() subroutine is provided for backward
# compatibility with the original Net::Ping.  It accepts a host
# name/IP and an optional timeout in seconds.  Create a tcp ping
# object and try pinging the host.  The result of the ping is returned.

sub pingecho
{
  my ($host,              # Name or IP number of host to ping
      $timeout            # Optional timeout in seconds
      ) = @_;
  my ($p);                # A ping object

  $p = Net::Ping->new("tcp", $timeout);
  $p->ping($host);        # Going out of scope closes the connection
}

# Description:  The new() method creates a new ping object.  Optional
# parameters may be specified for the protocol to use, the timeout in
# seconds and the size in bytes of additional data which should be
# included in the packet.
#   After the optional parameters are checked, the data is constructed
# and a socket is opened if appropriate.  The object is returned.

sub new
{
  my ($this,
      $proto,             # Optional protocol to use for pinging
      $timeout,           # Optional timeout in seconds
      $data_size,         # Optional additional bytes of data
      $device,            # Optional device to use
      $tos,               # Optional ToS to set
      $ttl,               # Optional TTL to set
      $family,            # Optional address family (AF_INET)
      ) = @_;
  my  $class = ref($this) || $this;
  my  $self = {};
  my ($cnt,               # Count through data bytes
      $min_datasize       # Minimum data bytes required
      );

  bless($self, $class);
  if (ref $proto eq 'HASH') { # support named args
    for my $k (qw(proto timeout data_size device tos ttl family
                  gateway host port bind retrans pingstring source_verify
                  econnrefused dontfrag
                  IPV6_USE_MIN_MTU IPV6_RECVPATHMTU IPV6_HOPLIMIT))
    {
      if (exists $proto->{$k}) {
        $self->{$k} = $proto->{$k};
        # some are still globals
        if ($k eq 'pingstring') { $pingstring = $proto->{$k} }
        if ($k eq 'source_verify') { $source_verify = $proto->{$k} }
        # and some are local
        $timeout = $proto->{$k}   if ($k eq 'timeout');
        $data_size = $proto->{$k} if ($k eq 'data_size');
        $device = $proto->{$k}    if ($k eq 'device');
        $tos = $proto->{$k}       if ($k eq 'tos');
        $ttl = $proto->{$k}       if ($k eq 'ttl');
        $family = $proto->{$k}    if ($k eq 'family');
        delete $proto->{$k};
      }
    }
    if (%$proto) {
      croak("Invalid named argument: ",join(" ",keys (%$proto)));
    }
    $proto = $self->{'proto'};
  }

  $proto = $def_proto unless $proto;          # Determine the protocol
  croak('Protocol for ping must be "icmp", "icmpv6", "udp", "tcp", "syn", "stream" or "external"')
    unless $proto =~ m/^(icmp|icmpv6|udp|tcp|syn|stream|external)$/;
  $self->{proto} = $proto;

  $timeout = $def_timeout unless defined $timeout;    # Determine the timeout
  croak("Default timeout for ping must be greater than 0 seconds")
    if $timeout <= 0;
  $self->{timeout} = $timeout;

  $self->{device} = $device;

  $self->{tos} = $tos;

  if ($self->{'host'}) {
    my $host = $self->{'host'};
    my $ip = $self->_resolv($host) or
      carp("could not resolve host $host");
    $self->{host} = $ip;
    $self->{family} = $ip->{family};
  }

  if ($self->{bind}) {
    my $addr = $self->{bind};
    my $ip = $self->_resolv($addr)
      or carp("could not resolve local addr $addr");
    $self->{local_addr} = $ip;
  } else {
    $self->{local_addr} = undef;              # Don't bind by default
  }

  if ($self->{proto} eq 'icmp') {
    croak('TTL must be from 0 to 255')
      if ($ttl && ($ttl < 0 || $ttl > 255));
    $self->{ttl} = $ttl;
  }

  if ($family) {
    if ($family =~ $qr_family) {
      if ($family =~ $qr_family4) {
        $self->{family} = AF_INET;
      } else {
        $self->{family} = $AF_INET6;
      }
    } else {
      croak('Family must be "ipv4" or "ipv6"')
    }
  } else {
    if ($self->{proto} eq 'icmpv6') {
      $self->{family} = $AF_INET6;
    } else {
      $self->{family} = $def_family;
    }
  }

  $min_datasize = ($proto eq "udp") ? 1 : 0;  # Determine data size
  $data_size = $min_datasize unless defined($data_size) && $proto ne "tcp";
  # allow for fragmented packets if data_size>1472 (MTU 1500)
  croak("Data for ping must be from $min_datasize to $max_datasize bytes")
    if ($data_size < $min_datasize) || ($data_size > $max_datasize);
  $data_size-- if $self->{proto} eq "udp";  # We provide the first byte
  $self->{data_size} = $data_size;

  $self->{data} = "";                       # Construct data bytes
  for ($cnt = 0; $cnt < $self->{data_size}; $cnt++)
  {
    $self->{data} .= chr($cnt % 256);
  }

  # Default exponential backoff rate
  $self->{retrans} = $def_factor unless exists $self->{retrans};
  # Default Connection refused behavior
  $self->{econnrefused} = undef unless exists $self->{econnrefused};

  $self->{seq} = 0;                         # For counting packets
  if ($self->{proto} eq "udp")              # Open a socket
  {
    $self->{proto_num} = eval { (getprotobyname('udp'))[2] } ||
      croak("Can't udp protocol by name");
    $self->{port_num} = $self->{port}
      || (getservbyname('echo', 'udp'))[2]
      || croak("Can't get udp echo port by name");
    $self->{fh} = FileHandle->new();
    socket($self->{fh}, PF_INET, SOCK_DGRAM,
           $self->{proto_num}) ||
             croak("udp socket error - $!");
    $self->_setopts();
  }
  elsif ($self->{proto} eq "icmp")
  {
    croak("icmp ping requires root privilege") if !_isroot();
    $self->{proto_num} = eval { (getprotobyname('icmp'))[2] } ||
      croak("Can't get icmp protocol by name");
    $self->{pid} = $$ & 0xffff;           # Save lower 16 bits of pid
    $self->{fh} = FileHandle->new();
    socket($self->{fh}, PF_INET, SOCK_RAW, $self->{proto_num}) ||
      croak("icmp socket error - $!");
    $self->_setopts();
    if ($self->{'ttl'}) {
      setsockopt($self->{fh}, IPPROTO_IP, IP_TTL, pack("I*", $self->{'ttl'}))
        or croak "error configuring ttl to $self->{'ttl'} $!";
    }
  }
  elsif ($self->{proto} eq "icmpv6")
  {
    #croak("icmpv6 ping requires root privilege") if !_isroot();
    croak("Wrong family $self->{family} for icmpv6 protocol")
      if $self->{family} and $self->{family} != $AF_INET6;
    $self->{family} = $AF_INET6;
    $self->{proto_num} = eval { (getprotobyname('ipv6-icmp'))[2] } ||
      croak("Can't get ipv6-icmp protocol by name"); # 58
    $self->{pid} = $$ & 0xffff;           # Save lower 16 bits of pid
    $self->{fh} = FileHandle->new();
    socket($self->{fh}, $AF_INET6, SOCK_RAW, $self->{proto_num}) ||
      croak("icmp socket error - $!");
    $self->_setopts();
    if ($self->{'gateway'}) {
      my $g = $self->{gateway};
      my $ip = $self->_resolv($g)
        or croak("nonexistent gateway $g");
      $self->{family} eq $AF_INET6
        or croak("gateway requires the AF_INET6 family");
      $ip->{family} eq $AF_INET6
        or croak("gateway address needs to be IPv6");
      my $IPV6_NEXTHOP = eval { Socket::IPV6_NEXTHOP() } || 48; # IPV6_3542NEXTHOP, or 21
      setsockopt($self->{fh}, $IPPROTO_IPV6, $IPV6_NEXTHOP, _pack_sockaddr_in($ip))
        or croak "error configuring gateway to $g NEXTHOP $!";
    }
    if (exists $self->{IPV6_USE_MIN_MTU}) {
      my $IPV6_USE_MIN_MTU = eval { Socket::IPV6_USE_MIN_MTU() } || 42;
      setsockopt($self->{fh}, $IPPROTO_IPV6, $IPV6_USE_MIN_MTU,
                 pack("I*", $self->{'IPV6_USE_MIN_MT'}))
        or croak "error configuring IPV6_USE_MIN_MT} $!";
    }
    if (exists $self->{IPV6_RECVPATHMTU}) {
      my $IPV6_RECVPATHMTU = eval { Socket::IPV6_RECVPATHMTU() } || 43;
      setsockopt($self->{fh}, $IPPROTO_IPV6, $IPV6_RECVPATHMTU,
                 pack("I*", $self->{'RECVPATHMTU'}))
        or croak "error configuring IPV6_RECVPATHMTU $!";
    }
    if ($self->{'tos'}) {
      my $proto = $self->{family} == AF_INET ? IPPROTO_IP : $IPPROTO_IPV6;
      setsockopt($self->{fh}, $proto, IP_TOS, pack("I*", $self->{'tos'}))
        or croak "error configuring tos to $self->{'tos'} $!";
    }
    if ($self->{'ttl'}) {
      my $proto = $self->{family} == AF_INET ? IPPROTO_IP : $IPPROTO_IPV6;
      setsockopt($self->{fh}, $proto, IP_TTL, pack("I*", $self->{'ttl'}))
        or croak "error configuring ttl to $self->{'ttl'} $!";
    }
  }
  elsif ($self->{proto} eq "tcp" || $self->{proto} eq "stream")
  {
    $self->{proto_num} = eval { (getprotobyname('tcp'))[2] } ||
      croak("Can't get tcp protocol by name");
    $self->{port_num} = $self->{port}
      || (getservbyname('echo', 'tcp'))[2]
      ||  croak("Can't get tcp echo port by name");
    $self->{fh} = FileHandle->new();
  }
  elsif ($self->{proto} eq "syn")
  {
    $self->{proto_num} = eval { (getprotobyname('tcp'))[2] } ||
      croak("Can't get tcp protocol by name");
    $self->{port_num} = (getservbyname('echo', 'tcp'))[2] ||
      croak("Can't get tcp echo port by name");
    if ($syn_forking) {
      $self->{fork_rd} = FileHandle->new();
      $self->{fork_wr} = FileHandle->new();
      pipe($self->{fork_rd}, $self->{fork_wr});
      $self->{fh} = FileHandle->new();
      $self->{good} = {};
      $self->{bad} = {};
    } else {
      $self->{wbits} = "";
      $self->{bad} = {};
    }
    $self->{syn} = {};
    $self->{stop_time} = 0;
  }

  return($self);
}

# Description: Set the local IP address from which pings will be sent.
# For ICMP, UDP and TCP pings, just saves the address to be used when 
# the socket is opened.  Returns non-zero if successful; croaks on error.
sub bind
{
  my ($self,
      $local_addr         # Name or IP number of local interface
      ) = @_;
  my ($ip,                # Hash of addr (string), addr_in (packed), family
      $h		  # resolved hash
      );

  croak("Usage: \$p->bind(\$local_addr)") unless @_ == 2;
  croak("already bound") if defined($self->{local_addr}) &&
    ($self->{proto} eq "udp" || $self->{proto} eq "icmp");

  $ip = $self->_resolv($local_addr);
  carp("nonexistent local address $local_addr") unless defined($ip);
  $self->{local_addr} = $ip;

  if (($self->{proto} ne "udp") && 
      ($self->{proto} ne "icmp") && 
      ($self->{proto} ne "tcp") && 
      ($self->{proto} ne "syn"))
  {
    croak("Unknown protocol \"$self->{proto}\" in bind()");
  }

  return 1;
}

# Description: A select() wrapper that compensates for platform
# peculiarities.
sub mselect
{
    if ($_[3] > 0 and $^O eq 'MSWin32') {
	# On windows, select() doesn't process the message loop,
	# but sleep() will, allowing alarm() to interrupt the latter.
	# So we chop up the timeout into smaller pieces and interleave
	# select() and sleep() calls.
	my $t = $_[3];
	my $gran = 0.5;  # polling granularity in seconds
	my @args = @_;
	while (1) {
	    $gran = $t if $gran > $t;
	    my $nfound = select($_[0], $_[1], $_[2], $gran);
	    undef $nfound if $nfound == -1;
	    $t -= $gran;
	    return $nfound if $nfound or !defined($nfound) or $t <= 0;

	    sleep(0);
	    ($_[0], $_[1], $_[2]) = @args;
	}
    }
    else {
	my $nfound = select($_[0], $_[1], $_[2], $_[3]);
	undef $nfound if $nfound == -1;
	return $nfound;
    }
}

# Description: Allow UDP source endpoint comparison to be
#              skipped for those remote interfaces that do
#              not response from the same endpoint.

sub source_verify
{
  my $self = shift;
  $source_verify = 1 unless defined
    ($source_verify = ((defined $self) && (ref $self)) ? shift() : $self);
}

# Description: Set whether or not the connect
# behavior should enforce remote service
# availability as well as reachability.

sub service_check
{
  my $self = shift;
  $self->{econnrefused} = 1 unless defined
    ($self->{econnrefused} = shift());
}

sub tcp_service_check
{
  service_check(@_);
}

# Description: Set exponential backoff for retransmission.
# Should be > 1 to retain exponential properties.
# If set to 0, retransmissions are disabled.

sub retrans
{
  my $self = shift;
  $self->{retrans} = shift;
}

sub _IsAdminUser {
  return unless $^O eq 'MSWin32' or $^O eq "cygwin";
  return unless eval { require Win32 };
  return unless defined &Win32::IsAdminUser;
  return Win32::IsAdminUser();
}

sub _isroot {
  if (($> and $^O ne 'VMS' and $^O ne 'cygwin')
    or (($^O eq 'MSWin32' or $^O eq 'cygwin')
        and !_IsAdminUser())
    or ($^O eq 'VMS'
        and (`write sys\$output f\$privilege("SYSPRV")` =~ m/FALSE/))) {
      return 0;
  }
  else {
    return 1;
  }
}

# Description: Sets ipv6 reachability
# REACHCONF was removed in RFC3542, ping6 -R supports it. requires root.

sub IPV6_REACHCONF
{
  my $self = shift;
  my $on = shift;
  if ($on) {
    my $reachconf = eval { Socket::IPV6_REACHCONF() };
    if (!$reachconf) {
      carp "IPV6_REACHCONF not supported on this platform";
      return 0;
    }
    if (!_isroot()) {
      carp "IPV6_REACHCONF requires root permissions";
      return 0;
    }
    $self->{IPV6_REACHCONF} = 1;
  }
  else {
    return $self->{IPV6_REACHCONF};
  }
}

# Description: set it on or off.

sub IPV6_USE_MIN_MTU
{
  my $self = shift;
  my $on = shift;
  if (defined $on) {
    my $IPV6_USE_MIN_MTU = eval { Socket::IPV6_USE_MIN_MTU() } || 43;
    #if (!$IPV6_USE_MIN_MTU) {
    #  carp "IPV6_USE_MIN_MTU not supported on this platform";
    #  return 0;
    #}
    $self->{IPV6_USE_MIN_MTU} = $on ? 1 : 0;
    setsockopt($self->{fh}, $IPPROTO_IPV6, $IPV6_USE_MIN_MTU,
               pack("I*", $self->{'IPV6_USE_MIN_MT'}))
      or croak "error configuring IPV6_USE_MIN_MT} $!";
  }
  else {
    return $self->{IPV6_USE_MIN_MTU};
  }
}

# Description: notify an according MTU

sub IPV6_RECVPATHMTU
{
  my $self = shift;
  my $on = shift;
  if ($on) {
    my $IPV6_RECVPATHMTU = eval { Socket::IPV6_RECVPATHMTU() } || 43;
    #if (!$RECVPATHMTU) {
    #  carp "IPV6_RECVPATHMTU not supported on this platform";
    #  return 0;
    #}
    $self->{IPV6_RECVPATHMTU} = 1;
    setsockopt($self->{fh}, $IPPROTO_IPV6, $IPV6_RECVPATHMTU,
               pack("I*", $self->{'IPV6_RECVPATHMTU'}))
      or croak "error configuring IPV6_RECVPATHMTU} $!";
  }
  else {
    return $self->{IPV6_RECVPATHMTU};
  }
}

# Description: allows the module to use milliseconds as returned by
# the Time::HiRes module

$hires = 1;
sub hires
{
  my $self = shift;
  $hires = 1 unless defined
    ($hires = ((defined $self) && (ref $self)) ? shift() : $self);
}

sub time
{
  return $hires ? Time::HiRes::time() : CORE::time();
}

# Description: Sets or clears the O_NONBLOCK flag on a file handle.
sub socket_blocking_mode
{
  my ($self,
      $fh,              # the file handle whose flags are to be modified
      $block) = @_;     # if true then set the blocking
                        # mode (clear O_NONBLOCK), otherwise
                        # set the non-blocking mode (set O_NONBLOCK)

  my $flags;
  if ($^O eq 'MSWin32' || $^O eq 'VMS') {
      # FIONBIO enables non-blocking sockets on windows and vms.
      # FIONBIO is (0x80000000|(4<<16)|(ord('f')<<8)|126), as per winsock.h, ioctl.h
      my $f = 0x8004667e;
      my $v = pack("L", $block ? 0 : 1);
      ioctl($fh, $f, $v) or croak("ioctl failed: $!");
      return;
  }
  if ($flags = fcntl($fh, F_GETFL, 0)) {
    $flags = $block ? ($flags & ~O_NONBLOCK) : ($flags | O_NONBLOCK);
    if (!fcntl($fh, F_SETFL, $flags)) {
      croak("fcntl F_SETFL: $!");
    }
  } else {
    croak("fcntl F_GETFL: $!");
  }
}

# Description: Ping a host name or IP number with an optional timeout.
# First lookup the host, and return undef if it is not found.  Otherwise
# perform the specific ping method based on the protocol.  Return the
# result of the ping.

sub ping
{
  my ($self,
      $host,              # Name or IP number of host to ping
      $timeout,           # Seconds after which ping times out
      $family,            # Address family
      ) = @_;
  my ($ip,                # Hash of addr (string), addr_in (packed), family
      $ret,               # The return value
      $ping_time,         # When ping began
      );

  $host = $self->{host} if !defined $host and $self->{host};
  croak("Usage: \$p->ping([ \$host [, \$timeout [, \$family]]])") if @_ > 4 or !$host;
  $timeout = $self->{timeout} unless $timeout;
  croak("Timeout must be greater than 0 seconds") if $timeout <= 0;

  if ($family) {
    if ($family =~ $qr_family) {
      if ($family =~ $qr_family4) {
        $self->{family_local} = AF_INET;
      } else {
        $self->{family_local} = $AF_INET6;
      }
    } else {
      croak('Family must be "ipv4" or "ipv6"')
    }
  } else {
    $self->{family_local} = $self->{family};
  }
  
  $ip = $self->_resolv($host);
  return () unless defined($ip);      # Does host exist?

  # Dispatch to the appropriate routine.
  $ping_time = &time();
  if ($self->{proto} eq "external") {
    $ret = $self->ping_external($ip, $timeout);
  }
  elsif ($self->{proto} eq "udp") {
    $ret = $self->ping_udp($ip, $timeout);
  }
  elsif ($self->{proto} eq "icmp") {
    $ret = $self->ping_icmp($ip, $timeout);
  }
  elsif ($self->{proto} eq "icmpv6") {
    $ret = $self->ping_icmpv6($ip, $timeout);
  }
  elsif ($self->{proto} eq "tcp") {
    $ret = $self->ping_tcp($ip, $timeout);
  }
  elsif ($self->{proto} eq "stream") {
    $ret = $self->ping_stream($ip, $timeout);
  }
  elsif ($self->{proto} eq "syn") {
    $ret = $self->ping_syn($host, $ip, $ping_time, $ping_time+$timeout);
  } else {
    croak("Unknown protocol \"$self->{proto}\" in ping()");
  }

  return wantarray ? ($ret, &time() - $ping_time, $self->ntop($ip)) : $ret;
}

# Uses Net::Ping::External to do an external ping.
sub ping_external {
  my ($self,
      $ip,                # Hash of addr (string), addr_in (packed), family
      $timeout,           # Seconds after which ping times out
      $family
     ) = @_;

  $ip = $self->{host} if !defined $ip and $self->{host};
  $timeout = $self->{timeout} if !defined $timeout and $self->{timeout};
  my @addr = exists $ip->{addr_in}
    ? ('ip' => $ip->{addr_in})
    : ('host' => $ip->{host});

  eval {
    local @INC = @INC;
    pop @INC if $INC[-1] eq '.';
    require Net::Ping::External;
  } or croak('Protocol "external" not supported on your system: Net::Ping::External not found');
  return Net::Ping::External::ping(@addr, timeout => $timeout,
                                   family => $family);
}

# h2ph "asm/socket.h"
# require "asm/socket.ph";
use constant SO_BINDTODEVICE  => 25;
use constant ICMP_ECHOREPLY   => 0;   # ICMP packet types
use constant ICMPv6_ECHOREPLY => 129; # ICMP packet types
use constant ICMP_UNREACHABLE => 3;   # ICMP packet types
use constant ICMPv6_UNREACHABLE => 1; # ICMP packet types
use constant ICMPv6_NI_REPLY => 140;  # ICMP packet types
use constant ICMP_ECHO        => 8;
use constant ICMPv6_ECHO      => 128;
use constant ICMP_TIME_EXCEEDED => 11; # ICMP packet types
use constant ICMP_PARAMETER_PROBLEM => 12; # ICMP packet types
use constant ICMP_TIMESTAMP   => 13;
use constant ICMP_TIMESTAMP_REPLY => 14;
use constant ICMP_STRUCT      => "C2 n3 A"; # Structure of a minimal ICMP packet
use constant ICMP_TIMESTAMP_STRUCT => "C2 n3 N3"; # Structure of a minimal timestamp ICMP packet
use constant SUBCODE          => 0; # No ICMP subcode for ECHO and ECHOREPLY
use constant ICMP_FLAGS       => 0; # No special flags for send or recv
use constant ICMP_PORT        => 0; # No port with ICMP
use constant IP_MTU_DISCOVER  => 10; # linux only

sub message_type
{
  my ($self,
      $type
      ) = @_;

  croak "Setting message type only supported on 'icmp' protocol"
    unless $self->{proto} eq 'icmp';

  return $self->{message_type} || 'echo'
    unless defined($type);

  croak "Supported icmp message type are limited to 'echo' and 'timestamp': '$type' not supported"
    unless $type =~ /^echo|timestamp$/i;

  $self->{message_type} = lc($type);
}

sub ping_icmp
{
  my ($self,
      $ip,                # Hash of addr (string), addr_in (packed), family
      $timeout            # Seconds after which ping times out
      ) = @_;

  my ($saddr,             # sockaddr_in with port and ip
      $checksum,          # Checksum of ICMP packet
      $msg,               # ICMP packet to send
      $len_msg,           # Length of $msg
      $rbits,             # Read bits, filehandles for reading
      $nfound,            # Number of ready filehandles found
      $finish_time,       # Time ping should be finished
      $done,              # set to 1 when we are done
      $ret,               # Return value
      $recv_msg,          # Received message including IP header
      $recv_msg_len,      # Length of recevied message, less any additional data
      $from_saddr,        # sockaddr_in of sender
      $from_port,         # Port packet was sent from
      $from_ip,           # Packed IP of sender
      $timestamp_msg,     # ICMP timestamp message type
      $from_type,         # ICMP type
      $from_subcode,      # ICMP subcode
      $from_chk,          # ICMP packet checksum
      $from_pid,          # ICMP packet id
      $from_seq,          # ICMP packet sequence
      $from_msg           # ICMP message
      );

  $ip = $self->{host} if !defined $ip and $self->{host};
  $timeout = $self->{timeout} if !defined $timeout and $self->{timeout};
  $timestamp_msg = $self->{message_type} && $self->{message_type} eq 'timestamp' ? 1 : 0;

  socket($self->{fh}, $ip->{family}, SOCK_RAW, $self->{proto_num}) ||
    croak("icmp socket error - $!");

  if (defined $self->{local_addr} &&
      !CORE::bind($self->{fh}, _pack_sockaddr_in(0, $self->{local_addr}))) {
    croak("icmp bind error - $!");
  }
  $self->_setopts();

  $self->{seq} = ($self->{seq} + 1) % 65536; # Increment sequence
  $checksum = 0;                          # No checksum for starters
  if ($ip->{family} == AF_INET) {
    if ($timestamp_msg) {
      $msg = pack(ICMP_TIMESTAMP_STRUCT, ICMP_TIMESTAMP, SUBCODE,
                  $checksum, $self->{pid}, $self->{seq}, 0, 0, 0);
    } else {
      $msg = pack(ICMP_STRUCT . $self->{data_size}, ICMP_ECHO, SUBCODE,
                  $checksum, $self->{pid}, $self->{seq}, $self->{data});
    }
  } else {
                                          # how to get SRC
    my $pseudo_header = pack('a16a16Nnn', $ip->{addr_in}, $ip->{addr_in}, 8+length($self->{data}), 0, 0x003a);
    $msg = pack(ICMP_STRUCT . $self->{data_size}, ICMPv6_ECHO, SUBCODE,
                $checksum, $self->{pid}, $self->{seq}, $self->{data});
    $msg = $pseudo_header.$msg
  }
  $checksum = Net::Ping->checksum($msg);
  if ($ip->{family} == AF_INET) {
    if ($timestamp_msg) {
      $msg = pack(ICMP_TIMESTAMP_STRUCT, ICMP_TIMESTAMP, SUBCODE,
                  $checksum, $self->{pid}, $self->{seq}, 0, 0, 0);
    } else {
      $msg = pack(ICMP_STRUCT . $self->{data_size}, ICMP_ECHO, SUBCODE,
                  $checksum, $self->{pid}, $self->{seq}, $self->{data});
    }
  } else {
    $msg = pack(ICMP_STRUCT . $self->{data_size}, ICMPv6_ECHO, SUBCODE,
                $checksum, $self->{pid}, $self->{seq}, $self->{data});
  }
  $len_msg = length($msg);
  $saddr = _pack_sockaddr_in(ICMP_PORT, $ip);
  $self->{from_ip} = undef;
  $self->{from_type} = undef;
  $self->{from_subcode} = undef;
  send($self->{fh}, $msg, ICMP_FLAGS, $saddr); # Send the message

  $rbits = "";
  vec($rbits, $self->{fh}->fileno(), 1) = 1;
  $ret = 0;
  $done = 0;
  $finish_time = &time() + $timeout;      # Must be done by this time
  while (!$done && $timeout > 0)          # Keep trying if we have time
  {
    $nfound = mselect((my $rout=$rbits), undef, undef, $timeout); # Wait for packet
    $timeout = $finish_time - &time();    # Get remaining time
    if (!defined($nfound))                # Hmm, a strange error
    {
      $ret = undef;
      $done = 1;
    }
    elsif ($nfound)                     # Got a packet from somewhere
    {
      $recv_msg = "";
      $from_pid = -1;
      $from_seq = -1;
      $from_saddr = recv($self->{fh}, $recv_msg, 1500, ICMP_FLAGS);
      $recv_msg_len = length($recv_msg) - length($self->{data});
      ($from_port, $from_ip) = _unpack_sockaddr_in($from_saddr, $ip->{family});
      # ICMP echo includes the header and ICMPv6 doesn't.
      # IPv4 length($recv_msg) is 28 (20 header + 8 payload)
      # while IPv6 length is only 8 (sans header).
      my $off = ($ip->{family} == AF_INET) ? 20 : 0; # payload offset
      ($from_type, $from_subcode) = unpack("C2", substr($recv_msg, $off, 2));
      if ($from_type == ICMP_TIMESTAMP_REPLY) {
        ($from_pid, $from_seq) = unpack("n3", substr($recv_msg, $off + 4, 4))
          if length $recv_msg >= $off + 8;
      } elsif ($from_type == ICMP_ECHOREPLY || $from_type == ICMPv6_ECHOREPLY) {
        #warn "ICMP_ECHOREPLY: ", $ip->{family}, " ",$recv_msg, ":", length($recv_msg);
        ($from_pid, $from_seq) = unpack("n2", substr($recv_msg, $off + 4, 4))
          if $recv_msg_len == $off + 8;
      } elsif ($from_type == ICMPv6_NI_REPLY) {
        ($from_pid, $from_seq) = unpack("n2", substr($recv_msg, 4, 4))
          if ($ip->{family} == $AF_INET6 && length $recv_msg == 8);
      } else {
        #warn "ICMP: ", $from_type, " ",$ip->{family}, " ",$recv_msg, ":", length($recv_msg);
        ($from_pid, $from_seq) = unpack("n2", substr($recv_msg, $off + 32, 4))
          if length $recv_msg >= $off + 36;
      }
      $self->{from_ip} = $from_ip;
      $self->{from_type} = $from_type;
      $self->{from_subcode} = $from_subcode;
      next if ($from_pid != $self->{pid});
      next if ($from_seq != $self->{seq});
      if (! $source_verify || ($self->ntop($from_ip) eq $self->ntop($ip))) { # Does the packet check out?
        if (!$timestamp_msg && (($from_type == ICMP_ECHOREPLY) || ($from_type == ICMPv6_ECHOREPLY))) {
          $ret = 1;
          $done = 1;
        } elsif ($timestamp_msg && $from_type == ICMP_TIMESTAMP_REPLY) {
          $ret = 1;
          $done = 1;
        } elsif (($from_type == ICMP_UNREACHABLE) || ($from_type == ICMPv6_UNREACHABLE)) {
          $done = 1;
        } elsif ($from_type == ICMP_TIME_EXCEEDED) {
          $ret = 0;
          $done = 1;
        }
      }
    } else {     # Oops, timed out
      $done = 1;
    }
  }
  return $ret;
}

sub ping_icmpv6
{
  shift->ping_icmp(@_);
}

sub icmp_result {
  my ($self) = @_;
  my $addr = $self->{from_ip} || "";
  $addr = "\0\0\0\0" unless 4 == length $addr;
  return ($self->ntop($addr),($self->{from_type} || 0), ($self->{from_subcode} || 0));
}

# Description:  Do a checksum on the message.  Basically sum all of
# the short words and fold the high order bits into the low order bits.

sub checksum
{
  my ($class,
      $msg            # The message to checksum
      ) = @_;
  my ($len_msg,       # Length of the message
      $num_short,     # The number of short words in the message
      $short,         # One short word
      $chk            # The checksum
      );

  $len_msg = length($msg);
  $num_short = int($len_msg / 2);
  $chk = 0;
  foreach $short (unpack("n$num_short", $msg))
  {
    $chk += $short;
  }                                           # Add the odd byte in
  $chk += (unpack("C", substr($msg, $len_msg - 1, 1)) << 8) if $len_msg % 2;
  $chk = ($chk >> 16) + ($chk & 0xffff);      # Fold high into low
  return(~(($chk >> 16) + $chk) & 0xffff);    # Again and complement
}


# Description:  Perform a tcp echo ping.  Since a tcp connection is
# host specific, we have to open and close each connection here.  We
# can't just leave a socket open.  Because of the robust nature of
# tcp, it will take a while before it gives up trying to establish a
# connection.  Therefore, we use select() on a non-blocking socket to
# check against our timeout.  No data bytes are actually
# sent since the successful establishment of a connection is proof
# enough of the reachability of the remote host.  Also, tcp is
# expensive and doesn't need our help to add to the overhead.

sub ping_tcp
{
  my ($self,
      $ip,                # Hash of addr (string), addr_in (packed), family
      $timeout            # Seconds after which ping times out
      ) = @_;
  my ($ret                # The return value
      );

  $ip = $self->{host} if !defined $ip and $self->{host};
  $timeout = $self->{timeout} if !defined $timeout and $self->{timeout};

  $! = 0;
  $ret = $self -> tcp_connect( $ip, $timeout);
  if (!$self->{econnrefused} &&
      $! == ECONNREFUSED) {
    $ret = 1;  # "Connection refused" means reachable
  }
  $self->{fh}->close();
  return $ret;
}

sub tcp_connect
{
  my ($self,
      $ip,                # Hash of addr (string), addr_in (packed), family
      $timeout            # Seconds after which connect times out
      ) = @_;
  my ($saddr);            # Packed IP and Port

  $ip = $self->{host} if !defined $ip and $self->{host};
  $timeout = $self->{timeout} if !defined $timeout and $self->{timeout};

  $saddr = _pack_sockaddr_in($self->{port_num}, $ip);

  my $ret = 0;            # Default to unreachable

  my $do_socket = sub {
    socket($self->{fh}, $ip->{family}, SOCK_STREAM, $self->{proto_num}) ||
      croak("tcp socket error - $!");
    if (defined $self->{local_addr} &&
        !CORE::bind($self->{fh}, _pack_sockaddr_in(0, $self->{local_addr}))) {
      croak("tcp bind error - $!");
    }
    $self->_setopts();
  };
  my $do_connect = sub {
    $self->{ip} = $ip->{addr_in};
    # ECONNREFUSED is 10061 on MSWin32. If we pass it as child error through $?,
    # we'll get (10061 & 255) = 77, so we cannot check it in the parent process.
    return ($ret = connect($self->{fh}, $saddr) || ($! == ECONNREFUSED && !$self->{econnrefused}));
  };
  my $do_connect_nb = sub {
    # Set O_NONBLOCK property on filehandle
    $self->socket_blocking_mode($self->{fh}, 0);

    # start the connection attempt
    if (!connect($self->{fh}, $saddr)) {
      if ($! == ECONNREFUSED) {
        $ret = 1 unless $self->{econnrefused};
      } elsif ($! != EINPROGRESS && ($^O ne 'MSWin32' || $! != EWOULDBLOCK)) {
        # EINPROGRESS is the expected error code after a connect()
        # on a non-blocking socket.  But if the kernel immediately
        # determined that this connect() will never work,
        # Simply respond with "unreachable" status.
        # (This can occur on some platforms with errno
        # EHOSTUNREACH or ENETUNREACH.)
        return 0;
      } else {
        # Got the expected EINPROGRESS.
        # Just wait for connection completion...
        my ($wbits, $wout, $wexc);
        $wout = $wexc = $wbits = "";
        vec($wbits, $self->{fh}->fileno, 1) = 1;

        my $nfound = mselect(undef,
			    ($wout = $wbits),
			    ($^O eq 'MSWin32' ? ($wexc = $wbits) : undef),
			    $timeout);
        warn("select: $!") unless defined $nfound;

        if ($nfound && vec($wout, $self->{fh}->fileno, 1)) {
          # the socket is ready for writing so the connection
          # attempt completed. test whether the connection
          # attempt was successful or not

          if (getpeername($self->{fh})) {
            # Connection established to remote host
            $ret = 1;
          } else {
            # TCP ACK will never come from this host
            # because there was an error connecting.

            # This should set $! to the correct error.
            my $char;
            sysread($self->{fh},$char,1);
            $! = ECONNREFUSED if ($! == EAGAIN && $^O =~ /cygwin/i);

            $ret = 1 if (!$self->{econnrefused}
                         && $! == ECONNREFUSED);
          }
        } else {
          # the connection attempt timed out (or there were connect
	  # errors on Windows)
	  if ($^O =~ 'MSWin32') {
	      # If the connect will fail on a non-blocking socket,
	      # winsock reports ECONNREFUSED as an exception, and we
	      # need to fetch the socket-level error code via getsockopt()
	      # instead of using the thread-level error code that is in $!.
	      if ($nfound && vec($wexc, $self->{fh}->fileno, 1)) {
		  $! = unpack("i", getsockopt($self->{fh}, SOL_SOCKET,
			                      SO_ERROR));
	      }
	  }
        }
      }
    } else {
      # Connection established to remote host
      $ret = 1;
    }

    # Unset O_NONBLOCK property on filehandle
    $self->socket_blocking_mode($self->{fh}, 1);
    $self->{ip} = $ip->{addr_in};
    return $ret;
  };

  if ($syn_forking) {
    # Buggy Winsock API doesn't allow nonblocking connect.
    # Hence, if our OS is Windows, we need to create a separate
    # process to do the blocking connect attempt.
    # XXX Above comments are not true at least for Win2K, where
    # nonblocking connect works.

    $| = 1; # Clear buffer prior to fork to prevent duplicate flushing.
    $self->{'tcp_chld'} = fork;
    if (!$self->{'tcp_chld'}) {
      if (!defined $self->{'tcp_chld'}) {
        # Fork did not work
        warn "Fork error: $!";
        return 0;
      }
      &{ $do_socket }();

      # Try a slow blocking connect() call
      # and report the status to the parent.
      if ( &{ $do_connect }() ) {
        $self->{fh}->close();
        # No error
        exit 0;
      } else {
        # Pass the error status to the parent
        # Make sure that $! <= 255
        exit($! <= 255 ? $! : 255);
      }
    }

    &{ $do_socket }();

    my $patience = &time() + $timeout;

    my ($child, $child_errno);
    $? = 0; $child_errno = 0;
    # Wait up to the timeout
    # And clean off the zombie
    do {
      $child = waitpid($self->{'tcp_chld'}, &WNOHANG());
      $child_errno = $? >> 8;
      select(undef, undef, undef, 0.1);
    } while &time() < $patience && $child != $self->{'tcp_chld'};

    if ($child == $self->{'tcp_chld'}) {
      if ($self->{proto} eq "stream") {
        # We need the socket connected here, in parent
        # Should be safe to connect because the child finished
        # within the timeout
        &{ $do_connect }();
      }
      # $ret cannot be set by the child process
      $ret = !$child_errno;
    } else {
      # Time must have run out.
      # Put that choking client out of its misery
      kill "KILL", $self->{'tcp_chld'};
      # Clean off the zombie
      waitpid($self->{'tcp_chld'}, 0);
      $ret = 0;
    }
    delete $self->{'tcp_chld'};
    $! = $child_errno;
  } else {
    # Otherwise don't waste the resources to fork

    &{ $do_socket }();

    &{ $do_connect_nb }();
  }

  return $ret;
}

sub DESTROY {
  my $self = shift;
  if ($self->{'proto'} && ($self->{'proto'} eq 'tcp') && $self->{'tcp_chld'}) {
    # Put that choking client out of its misery
    kill "KILL", $self->{'tcp_chld'};
    # Clean off the zombie
    waitpid($self->{'tcp_chld'}, 0);
  }
}

# This writes the given string to the socket and then reads it
# back.  It returns 1 on success, 0 on failure.
sub tcp_echo
{
  my ($self, $timeout, $pingstring) = @_;

  $timeout = $self->{timeout} if !defined $timeout and $self->{timeout};
  $pingstring = $self->{pingstring} if !defined $pingstring and $self->{pingstring};

  my $ret = undef;
  my $time = &time();
  my $wrstr = $pingstring;
  my $rdstr = "";

  eval <<'EOM';
    do {
      my $rin = "";
      vec($rin, $self->{fh}->fileno(), 1) = 1;

      my $rout = undef;
      if($wrstr) {
        $rout = "";
        vec($rout, $self->{fh}->fileno(), 1) = 1;
      }

      if(mselect($rin, $rout, undef, ($time + $timeout) - &time())) {

        if($rout && vec($rout,$self->{fh}->fileno(),1)) {
          my $num = syswrite($self->{fh}, $wrstr, length $wrstr);
          if($num) {
            # If it was a partial write, update and try again.
            $wrstr = substr($wrstr,$num);
          } else {
            # There was an error.
            $ret = 0;
          }
        }

        if(vec($rin,$self->{fh}->fileno(),1)) {
          my $reply;
          if(sysread($self->{fh},$reply,length($pingstring)-length($rdstr))) {
            $rdstr .= $reply;
            $ret = 1 if $rdstr eq $pingstring;
          } else {
            # There was an error.
            $ret = 0;
          }
        }

      }
    } until &time() > ($time + $timeout) || defined($ret);
EOM

  return $ret;
}

# Description: Perform a stream ping.  If the tcp connection isn't
# already open, it opens it.  It then sends some data and waits for
# a reply.  It leaves the stream open on exit.

sub ping_stream
{
  my ($self,
      $ip,                # Hash of addr (string), addr_in (packed), family
      $timeout            # Seconds after which ping times out
      ) = @_;

  # Open the stream if it's not already open
  if(!defined $self->{fh}->fileno()) {
    $self->tcp_connect($ip, $timeout) or return 0;
  }

  croak "tried to switch servers while stream pinging"
    if $self->{ip} ne $ip->{addr_in};

  return $self->tcp_echo($timeout, $pingstring);
}

# Description: opens the stream.  You would do this if you want to
# separate the overhead of opening the stream from the first ping.

sub open
{
  my ($self,
      $host,              # Host or IP address
      $timeout,           # Seconds after which open times out
      $family
      ) = @_;
  my $ip;                 # Hash of addr (string), addr_in (packed), family
  $host = $self->{host} unless defined $host;

  if ($family) {
    if ($family =~ $qr_family) {
      if ($family =~ $qr_family4) {
        $self->{family_local} = AF_INET;
      } else {
        $self->{family_local} = $AF_INET6;
      }
    } else {
      croak('Family must be "ipv4" or "ipv6"')
    }
  } else {
    $self->{family_local} = $self->{family};
  }

  $timeout = $self->{timeout} unless $timeout;
  $ip = $self->_resolv($host);

  if ($self->{proto} eq "stream") {
    if (defined($self->{fh}->fileno())) {
      croak("socket is already open");
    } else {
      return () unless $ip;
      $self->tcp_connect($ip, $timeout);
    }
  }
}

sub _dontfrag {
  my $self = shift;
  # bsd solaris
  my $IP_DONTFRAG = eval { Socket::IP_DONTFRAG() };
  if ($IP_DONTFRAG) {
    my $i = 1;
    setsockopt($self->{fh}, IPPROTO_IP, $IP_DONTFRAG, pack("I*", $i))
      or croak "error configuring IP_DONTFRAG $!";
    # Linux needs more: Path MTU Discovery as defined in RFC 1191
    # For non SOCK_STREAM sockets it is the user's responsibility to packetize
    # the data in MTU sized chunks and to do the retransmits if necessary.
    # The kernel will reject packets that are bigger than the known path
    # MTU if this flag is set (with EMSGSIZE).
    if ($^O eq 'linux') {
      my $i = 2; # IP_PMTUDISC_DO
      setsockopt($self->{fh}, IPPROTO_IP, IP_MTU_DISCOVER, pack("I*", $i))
        or croak "error configuring IP_MTU_DISCOVER $!";
    }
  }
}

# SO_BINDTODEVICE + IP_TOS
sub _setopts {
  my $self = shift;
  if ($self->{'device'}) {
    setsockopt($self->{fh}, SOL_SOCKET, SO_BINDTODEVICE, pack("Z*", $self->{'device'}))
      or croak "error binding to device $self->{'device'} $!";
  }
  if ($self->{'tos'}) { # need to re-apply ToS (RT #6706)
    setsockopt($self->{fh}, IPPROTO_IP, IP_TOS, pack("I*", $self->{'tos'}))
      or croak "error applying tos to $self->{'tos'} $!";
  }
  if ($self->{'dontfrag'}) {
    $self->_dontfrag;
  }
}  


# Description:  Perform a udp echo ping.  Construct a message of
# at least the one-byte sequence number and any additional data bytes.
# Send the message out and wait for a message to come back.  If we
# get a message, make sure all of its parts match.  If they do, we are
# done.  Otherwise go back and wait for the message until we run out
# of time.  Return the result of our efforts.

use constant UDP_FLAGS => 0; # Nothing special on send or recv
sub ping_udp
{
  my ($self,
      $ip,                # Hash of addr (string), addr_in (packed), family
      $timeout            # Seconds after which ping times out
      ) = @_;

  my ($saddr,             # sockaddr_in with port and ip
      $ret,               # The return value
      $msg,               # Message to be echoed
      $finish_time,       # Time ping should be finished
      $flush,             # Whether socket needs to be disconnected
      $connect,           # Whether socket needs to be connected
      $done,              # Set to 1 when we are done pinging
      $rbits,             # Read bits, filehandles for reading
      $nfound,            # Number of ready filehandles found
      $from_saddr,        # sockaddr_in of sender
      $from_msg,          # Characters echoed by $host
      $from_port,         # Port message was echoed from
      $from_ip            # Packed IP number of sender
      );

  $saddr = _pack_sockaddr_in($self->{port_num}, $ip);
  $self->{seq} = ($self->{seq} + 1) % 256;    # Increment sequence
  $msg = chr($self->{seq}) . $self->{data};   # Add data if any

  socket($self->{fh}, $ip->{family}, SOCK_DGRAM,
         $self->{proto_num}) ||
           croak("udp socket error - $!");

  if (defined $self->{local_addr} &&
      !CORE::bind($self->{fh}, _pack_sockaddr_in(0, $self->{local_addr}))) {
    croak("udp bind error - $!");
  }

  $self->_setopts();

  if ($self->{connected}) {
    if ($self->{connected} ne $saddr) {
      # Still connected to wrong destination.
      # Need to flush out the old one.
      $flush = 1;
    }
  } else {
    # Not connected yet.
    # Need to connect() before send()
    $connect = 1;
  }

  # Have to connect() and send() instead of sendto()
  # in order to pick up on the ECONNREFUSED setting
  # from recv() or double send() errno as utilized in
  # the concept by rdw @ perlmonks.  See:
  # http://perlmonks.thepen.com/42898.html
  if ($flush) {
    # Need to socket() again to flush the descriptor
    # This will disconnect from the old saddr.
    socket($self->{fh}, $ip->{family}, SOCK_DGRAM,
           $self->{proto_num});
    $self->_setopts();
  }
  # Connect the socket if it isn't already connected
  # to the right destination.
  if ($flush || $connect) {
    connect($self->{fh}, $saddr);               # Tie destination to socket
    $self->{connected} = $saddr;
  }
  send($self->{fh}, $msg, UDP_FLAGS);           # Send it

  $rbits = "";
  vec($rbits, $self->{fh}->fileno(), 1) = 1;
  $ret = 0;                   # Default to unreachable
  $done = 0;
  my $retrans = 0.01;
  my $factor = $self->{retrans};
  $finish_time = &time() + $timeout;       # Ping needs to be done by then
  while (!$done && $timeout > 0)
  {
    if ($factor > 1)
    {
      $timeout = $retrans if $timeout > $retrans;
      $retrans*= $factor; # Exponential backoff
    }
    $nfound  = mselect((my $rout=$rbits), undef, undef, $timeout); # Wait for response
    my $why = $!;
    $timeout = $finish_time - &time();   # Get remaining time

    if (!defined($nfound))  # Hmm, a strange error
    {
      $ret = undef;
      $done = 1;
    }
    elsif ($nfound)         # A packet is waiting
    {
      $from_msg = "";
      $from_saddr = recv($self->{fh}, $from_msg, 1500, UDP_FLAGS);
      if (!$from_saddr) {
        # For example an unreachable host will make recv() fail.
        if (!$self->{econnrefused} &&
            ($! == ECONNREFUSED ||
             $! == ECONNRESET)) {
          # "Connection refused" means reachable
          # Good, continue
          $ret = 1;
        }
        $done = 1;
      } else {
        ($from_port, $from_ip) = _unpack_sockaddr_in($from_saddr, $ip->{family});
        my $addr_in = ref($ip) eq "HASH" ? $ip->{addr_in} : $ip;
        if (!$source_verify ||
            (($from_ip eq $addr_in) &&        # Does the packet check out?
             ($from_port == $self->{port_num}) &&
             ($from_msg eq $msg)))
        {
          $ret = 1;       # It's a winner
          $done = 1;
        }
      }
    }
    elsif ($timeout <= 0)              # Oops, timed out
    {
      $done = 1;
    }
    else
    {
      # Send another in case the last one dropped
      if (send($self->{fh}, $msg, UDP_FLAGS)) {
        # Another send worked?  The previous udp packet
        # must have gotten lost or is still in transit.
        # Hopefully this new packet will arrive safely.
      } else {
        if (!$self->{econnrefused} &&
            $! == ECONNREFUSED) {
          # "Connection refused" means reachable
          # Good, continue
          $ret = 1;
        }
        $done = 1;
      }
    }
  }
  return $ret;
}

# Description: Send a TCP SYN packet to host specified.
sub ping_syn
{
  my $self = shift;
  my $host = shift;
  my $ip = shift;
  my $start_time = shift;
  my $stop_time = shift;

  if ($syn_forking) {
    return $self->ping_syn_fork($host, $ip, $start_time, $stop_time);
  }

  my $fh = FileHandle->new();
  my $saddr = _pack_sockaddr_in($self->{port_num}, $ip);

  # Create TCP socket
  if (!socket ($fh, $ip->{family}, SOCK_STREAM, $self->{proto_num})) {
    croak("tcp socket error - $!");
  }

  if (defined $self->{local_addr} &&
      !CORE::bind($fh, _pack_sockaddr_in(0, $self->{local_addr}))) {
    croak("tcp bind error - $!");
  }

  $self->_setopts();
  # Set O_NONBLOCK property on filehandle
  $self->socket_blocking_mode($fh, 0);

  # Attempt the non-blocking connect
  # by just sending the TCP SYN packet
  if (connect($fh, $saddr)) {
    # Non-blocking, yet still connected?
    # Must have connected very quickly,
    # or else it wasn't very non-blocking.
    #warn "WARNING: Nonblocking connect connected anyway? ($^O)";
  } else {
    # Error occurred connecting.
    if ($! == EINPROGRESS || ($^O eq 'MSWin32' && $! == EWOULDBLOCK)) {
      # The connection is just still in progress.
      # This is the expected condition.
    } else {
      # Just save the error and continue on.
      # The ack() can check the status later.
      $self->{bad}->{$host} = $!;
    }
  }

  my $entry = [ $host, $ip, $fh, $start_time, $stop_time, $self->{port_num} ];
  $self->{syn}->{$fh->fileno} = $entry;
  if ($self->{stop_time} < $stop_time) {
    $self->{stop_time} = $stop_time;
  }
  vec($self->{wbits}, $fh->fileno, 1) = 1;

  return 1;
}

sub ping_syn_fork {
  my ($self, $host, $ip, $start_time, $stop_time) = @_;

  # Buggy Winsock API doesn't allow nonblocking connect.
  # Hence, if our OS is Windows, we need to create a separate
  # process to do the blocking connect attempt.
  my $pid = fork();
  if (defined $pid) {
    if ($pid) {
      # Parent process
      my $entry = [ $host, $ip, $pid, $start_time, $stop_time ];
      $self->{syn}->{$pid} = $entry;
      if ($self->{stop_time} < $stop_time) {
        $self->{stop_time} = $stop_time;
      }
    } else {
      # Child process
      my $saddr = _pack_sockaddr_in($self->{port_num}, $ip);

      # Create TCP socket
      if (!socket ($self->{fh}, $ip->{family}, SOCK_STREAM, $self->{proto_num})) {
        croak("tcp socket error - $!");
      }

      if (defined $self->{local_addr} &&
          !CORE::bind($self->{fh}, _pack_sockaddr_in(0, $self->{local_addr}))) {
        croak("tcp bind error - $!");
      }

      $self->_setopts();

      $!=0;
      # Try to connect (could take a long time)
      connect($self->{fh}, $saddr);
      # Notify parent of connect error status
      my $err = $!+0;
      my $wrstr = "$$ $err";
      # Force to 16 chars including \n
      $wrstr .= " "x(15 - length $wrstr). "\n";
      syswrite($self->{fork_wr}, $wrstr, length $wrstr);
      exit;
    }
  } else {
    # fork() failed?
    die "fork: $!";
  }
  return 1;
}

# Description: Wait for TCP ACK from host specified
# from ping_syn above.  If no host is specified, wait
# for TCP ACK from any of the hosts in the SYN queue.
sub ack
{
  my $self = shift;

  if ($self->{proto} eq "syn") {
    if ($syn_forking) {
      my @answer = $self->ack_unfork(shift);
      return wantarray ? @answer : $answer[0];
    }
    my $wbits = "";
    my $stop_time = 0;
    if (my $host = shift or $self->{host}) {
      # Host passed as arg or as option to new
      $host = $self->{host} unless defined $host;
      if (exists $self->{bad}->{$host}) {
        if (!$self->{econnrefused} &&
            $self->{bad}->{ $host } &&
            (($! = ECONNREFUSED)>0) &&
            $self->{bad}->{ $host } eq "$!") {
          # "Connection refused" means reachable
          # Good, continue
        } else {
          # ECONNREFUSED means no good
          return ();
        }
      }
      my $host_fd = undef;
      foreach my $fd (keys %{ $self->{syn} }) {
        my $entry = $self->{syn}->{$fd};
        if ($entry->[0] eq $host) {
          $host_fd = $fd;
          $stop_time = $entry->[4]
            || croak("Corrupted SYN entry for [$host]");
          last;
        }
      }
      croak("ack called on [$host] without calling ping first!")
        unless defined $host_fd;
      vec($wbits, $host_fd, 1) = 1;
    } else {
      # No $host passed so scan all hosts
      # Use the latest stop_time
      $stop_time = $self->{stop_time};
      # Use all the bits
      $wbits = $self->{wbits};
    }

    while ($wbits !~ /^\0*\z/) {
      my $timeout = $stop_time - &time();
      # Force a minimum of 10 ms timeout.
      $timeout = 0.01 if $timeout <= 0.01;

      my $winner_fd = undef;
      my $wout = $wbits;
      my $fd = 0;
      # Do "bad" fds from $wbits first
      while ($wout !~ /^\0*\z/) {
        if (vec($wout, $fd, 1)) {
          # Wipe it from future scanning.
          vec($wout, $fd, 1) = 0;
          if (my $entry = $self->{syn}->{$fd}) {
            if ($self->{bad}->{ $entry->[0] }) {
              $winner_fd = $fd;
              last;
            }
          }
        }
        $fd++;
      }

      if (defined($winner_fd) or my $nfound = mselect(undef, ($wout=$wbits), undef, $timeout)) {
        if (defined $winner_fd) {
          $fd = $winner_fd;
        } else {
          # Done waiting for one of the ACKs
          $fd = 0;
          # Determine which one
          while ($wout !~ /^\0*\z/ &&
                 !vec($wout, $fd, 1)) {
            $fd++;
          }
        }
        if (my $entry = $self->{syn}->{$fd}) {
          # Wipe it from future scanning.
          delete $self->{syn}->{$fd};
          vec($self->{wbits}, $fd, 1) = 0;
          vec($wbits, $fd, 1) = 0;
          if (!$self->{econnrefused} &&
              $self->{bad}->{ $entry->[0] } &&
              (($! = ECONNREFUSED)>0) &&
              $self->{bad}->{ $entry->[0] } eq "$!") {
            # "Connection refused" means reachable
            # Good, continue
          } elsif (getpeername($entry->[2])) {
            # Connection established to remote host
            # Good, continue
          } else {
            # TCP ACK will never come from this host
            # because there was an error connecting.

            # This should set $! to the correct error.
            my $char;
            sysread($entry->[2],$char,1);
            # Store the excuse why the connection failed.
            $self->{bad}->{$entry->[0]} = $!;
            if (!$self->{econnrefused} &&
                (($! == ECONNREFUSED) ||
                 ($! == EAGAIN && $^O =~ /cygwin/i))) {
              # "Connection refused" means reachable
              # Good, continue
            } else {
              # No good, try the next socket...
              next;
            }
          }
          # Everything passed okay, return the answer
          return wantarray ?
            ($entry->[0], &time() - $entry->[3], $self->ntop($entry->[1]), $entry->[5])
            : $entry->[0];
        } else {
          warn "Corrupted SYN entry: unknown fd [$fd] ready!";
          vec($wbits, $fd, 1) = 0;
          vec($self->{wbits}, $fd, 1) = 0;
        }
      } elsif (defined $nfound) {
        # Timed out waiting for ACK
        foreach my $fd (keys %{ $self->{syn} }) {
          if (vec($wbits, $fd, 1)) {
            my $entry = $self->{syn}->{$fd};
            $self->{bad}->{$entry->[0]} = "Timed out";
            vec($wbits, $fd, 1) = 0;
            vec($self->{wbits}, $fd, 1) = 0;
            delete $self->{syn}->{$fd};
          }
        }
      } else {
        # Weird error occurred with select()
        warn("select: $!");
        $self->{syn} = {};
        $wbits = "";
      }
    }
  }
  return ();
}

sub ack_unfork {
  my ($self,$host) = @_;
  my $stop_time = $self->{stop_time};
  if ($host) {
    # Host passed as arg
    if (my $entry = $self->{good}->{$host}) {
      delete $self->{good}->{$host};
      return ($entry->[0], &time() - $entry->[3], $self->ntop($entry->[1]));
    }
  }

  my $rbits = "";
  my $timeout;

  if (keys %{ $self->{syn} }) {
    # Scan all hosts that are left
    vec($rbits, fileno($self->{fork_rd}), 1) = 1;
    $timeout = $stop_time - &time();
    # Force a minimum of 10 ms timeout.
    $timeout = 0.01 if $timeout < 0.01;
  } else {
    # No hosts left to wait for
    $timeout = 0;
  }

  if ($timeout > 0) {
    my $nfound;
    while ( keys %{ $self->{syn} } and
           $nfound = mselect((my $rout=$rbits), undef, undef, $timeout)) {
      # Done waiting for one of the ACKs
      if (!sysread($self->{fork_rd}, $_, 16)) {
        # Socket closed, which means all children are done.
        return ();
      }
      my ($pid, $how) = split;
      if ($pid) {
        # Flush the zombie
        waitpid($pid, 0);
        if (my $entry = $self->{syn}->{$pid}) {
          # Connection attempt to remote host is done
          delete $self->{syn}->{$pid};
          if (!$how || # If there was no error connecting
              (!$self->{econnrefused} &&
               $how == ECONNREFUSED)) {  # "Connection refused" means reachable
            if ($host && $entry->[0] ne $host) {
              # A good connection, but not the host we need.
              # Move it from the "syn" hash to the "good" hash.
              $self->{good}->{$entry->[0]} = $entry;
              # And wait for the next winner
              next;
            }
            return ($entry->[0], &time() - $entry->[3], $self->ntop($entry->[1]));
          }
        } else {
          # Should never happen
          die "Unknown ping from pid [$pid]";
        }
      } else {
        die "Empty response from status socket?";
      }
    }
    if (defined $nfound) {
      # Timed out waiting for ACK status
    } else {
      # Weird error occurred with select()
      warn("select: $!");
    }
  }
  if (my @synners = keys %{ $self->{syn} }) {
    # Kill all the synners
    kill 9, @synners;
    foreach my $pid (@synners) {
      # Wait for the deaths to finish
      # Then flush off the zombie
      waitpid($pid, 0);
    }
  }
  $self->{syn} = {};
  return ();
}

# Description:  Tell why the ack() failed
sub nack {
  my $self = shift;
  my $host = shift || croak('Usage> nack($failed_ack_host)');
  return $self->{bad}->{$host} || undef;
}

# Description:  Close the connection.

sub close
{
  my ($self) = @_;

  if ($self->{proto} eq "syn") {
    delete $self->{syn};
  } elsif ($self->{proto} eq "tcp") {
    # The connection will already be closed
  } elsif ($self->{proto} eq "external") {
    # Nothing to close
  } else {
    $self->{fh}->close();
  }
}

sub port_number {
   my $self = shift;
   if(@_) {
       $self->{port_num} = shift @_;
       $self->service_check(1);
   }
   return $self->{port_num};
}

sub ntop {
    my($self, $ip) = @_;

    # Vista doesn't define a inet_ntop.  It has InetNtop instead.
    # Not following ANSI... priceless.  getnameinfo() is defined
    # for Windows 2000 and later, so that may be the choice.

    # Any port will work, even undef, but this will work for now.
    # Socket warns when undef is passed in, but it still works.
    my $port = getservbyname('echo', 'udp');
    my $sockaddr = _pack_sockaddr_in($port, $ip);
    my ($error, $address) = getnameinfo($sockaddr, $NI_NUMERICHOST);
    croak $error if $error;
    return $address;
}

sub wakeonlan {
  my ($mac_addr, $host, $port) = @_;

  # use the discard service if $port not passed in
  if (! defined $host) { $host = '255.255.255.255' }
  if (! defined $port || $port !~ /^\d+$/ ) { $port = 9 }

  require IO::Socket::INET;
  my $sock = IO::Socket::INET->new(Proto=>'udp') || return undef;

  my $ip_addr = inet_aton($host);
  my $sock_addr = sockaddr_in($port, $ip_addr);
  $mac_addr =~ s/://g;
  my $packet = pack('C6H*', 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, $mac_addr x 16);

  setsockopt($sock, SOL_SOCKET, SO_BROADCAST, 1);
  send($sock, $packet, 0, $sock_addr);
  $sock->close;

  return 1;
}

########################################################
# DNS hostname resolution
# return:
#   $h->{name}    = host - as passed in
#   $h->{host}    = host - as passed in without :port
#   $h->{port}    = OPTIONAL - if :port, then value of port
#   $h->{addr}    = resolved numeric address
#   $h->{addr_in} = aton/pton result
#   $h->{family}  = AF_INET/6
############################
sub _resolv {
  my ($self,
      $name,
      ) = @_;

  my %h;
  $h{name} = $name;
  my $family = $self->{family};

  if (defined($self->{family_local})) {
    $family = $self->{family_local}
  }

# START - host:port
  my $cnt = 0;

  # Count ":"
  $cnt++ while ($name =~ m/:/g);

  # 0 = hostname or IPv4 address
  if ($cnt == 0) {
    $h{host} = $name
  # 1 = IPv4 address with port
  } elsif ($cnt == 1) {
    ($h{host}, $h{port}) = split /:/, $name
  # >=2 = IPv6 address
  } elsif ($cnt >= 2) {
    #IPv6 with port - [2001::1]:port
    if ($name =~ /^\[.*\]:\d{1,5}$/) {
      ($h{host}, $h{port}) = split /:([^:]+)$/, $name # split after last :
    # IPv6 without port
    } else {
      $h{host} = $name
    }
  }

  # Clean up host
  $h{host} =~ s/\[//g;
  $h{host} =~ s/\]//g;
  # Clean up port
  if (defined($h{port}) && (($h{port} !~ /^\d{1,5}$/) || ($h{port} < 1) || ($h{port} > 65535))) {
    croak("Invalid port `$h{port}' in `$name'");
    return undef;
  }
# END - host:port

  # address check
  # new way
  if ($Socket_VERSION > 1.94) {
    my %hints = (
      family   => $AF_UNSPEC,
      protocol => IPPROTO_TCP,
      flags => $AI_NUMERICHOST
    );

    # numeric address, return
    my ($err, @getaddr) = Socket::getaddrinfo($h{host}, undef, \%hints);
    if (defined($getaddr[0])) {
      $h{addr} = $h{host};
      $h{family} = $getaddr[0]->{family};
      if ($h{family} == AF_INET) {
        (undef, $h{addr_in}, undef, undef) = Socket::unpack_sockaddr_in $getaddr[0]->{addr};
      } else {
        (undef, $h{addr_in}, undef, undef) = Socket::unpack_sockaddr_in6 $getaddr[0]->{addr};
      }
      return \%h
    }
  # old way
  } else {
    # numeric address, return
    my $ret = gethostbyname($h{host});
    if (defined($ret) && (_inet_ntoa($ret) eq $h{host})) {
      $h{addr} = $h{host};
      $h{addr_in} = $ret;
      $h{family} = AF_INET;
      return \%h
    }
  }

  # resolve
  # new way
  if ($Socket_VERSION >= 1.94) {
    my %hints = (
      family   => $family,
      protocol => IPPROTO_TCP
    );

    my ($err, @getaddr) = Socket::getaddrinfo($h{host}, undef, \%hints);
    if (defined($getaddr[0])) {
      my ($err, $address) = Socket::getnameinfo($getaddr[0]->{addr}, $NI_NUMERICHOST, $NIx_NOSERV);
      if (defined($address)) {
        $h{addr} = $address;
        $h{addr} =~ s/\%(.)*$//; # remove %ifID if IPv6
        $h{family} = $getaddr[0]->{family};
        if ($h{family} == AF_INET) {
          (undef, $h{addr_in}, undef, undef) = Socket::unpack_sockaddr_in $getaddr[0]->{addr};
        } else {
          (undef, $h{addr_in}, undef, undef) = Socket::unpack_sockaddr_in6 $getaddr[0]->{addr};
        }
        return \%h;
      } else {
        carp("getnameinfo($getaddr[0]->{addr}) failed - $err");
        return undef;
      }
    } else {
      warn(sprintf("getaddrinfo($h{host},,%s) failed - $err",
                    $family == AF_INET ? "AF_INET" : "AF_INET6"));
      return undef;
    }
  # old way
  } else {
    if ($family == $AF_INET6) {
      croak("Socket >= 1.94 required for IPv6 - found Socket $Socket::VERSION");
      return undef;
    }

    my @gethost = gethostbyname($h{host});
    if (defined($gethost[4])) {
      $h{addr} = inet_ntoa($gethost[4]);
      $h{addr_in} = $gethost[4];
      $h{family} = AF_INET;
      return \%h
    } else {
      carp("gethostbyname($h{host}) failed - $^E");
      return undef;
    }
  }
  return undef;
}

sub _pack_sockaddr_in($$) {
  my ($port,
      $ip,
      ) = @_;

  my $addr = ref($ip) eq "HASH" ? $ip->{addr_in} : $ip;
  if (length($addr) <= 4 ) {
    return Socket::pack_sockaddr_in($port, $addr);
  } else {
    return Socket::pack_sockaddr_in6($port, $addr);
  }
}

sub _unpack_sockaddr_in($;$) {
  my ($addr,
      $family,
      ) = @_;

  my ($port, $host);
  if ($family == AF_INET || (!defined($family) and length($addr) <= 16 )) {
    ($port, $host) = Socket::unpack_sockaddr_in($addr);
  } else {
    ($port, $host) = Socket::unpack_sockaddr_in6($addr);
  }
  return $port, $host
}

sub _inet_ntoa {
  my ($addr
      ) = @_;

  my $ret;
  if ($Socket_VERSION >= 1.94) {
    my ($err, $address) = Socket::getnameinfo($addr, $NI_NUMERICHOST);
    if (defined($address)) {
      $ret = $address;
    } else {
      carp("getnameinfo($addr) failed - $err");
    }
  } else {
    $ret = inet_ntoa($addr)
  }
    
  return $ret
}

1;
__END__

=head1 NAME

Net::Ping - check a remote host for reachability

=head1 SYNOPSIS

    use Net::Ping;

    my $p = Net::Ping->new();
    print "$host is alive.\n" if $p->ping($host);
    $p->close();

    my $p = Net::Ping->new("icmp");
    $p->bind($my_addr); # Specify source interface of pings
    foreach my $host (@host_array)
    {
        print "$host is ";
        print "NOT " unless $p->ping($host, 2);
        print "reachable.\n";
        sleep(1);
    }
    $p->close();

    my $p = Net::Ping->new("icmpv6");
    my $ip = "[fd00:dead:beef::4e]";
    print "$ip is alive.\n" if $p->ping($ip);

    my $p = Net::Ping->new("tcp", 2);
    # Try connecting to the www port instead of the echo port
    $p->port_number(scalar(getservbyname("http", "tcp")));
    while ($stop_time > time())
    {
        print "$host not reachable ", scalar(localtime()), "\n"
            unless $p->ping($host);
        sleep(300);
    }
    undef($p);

    # Like tcp protocol, but with many hosts
    my $p = Net::Ping->new("syn");
    $p->port_number(getservbyname("http", "tcp"));
    foreach my $host (@host_array) {
      $p->ping($host);
    }
    while (my ($host, $rtt, $ip) = $p->ack) {
      print "HOST: $host [$ip] ACKed in $rtt seconds.\n";
    }

    # High precision syntax (requires Time::HiRes)
    my $p = Net::Ping->new();
    $p->hires();
    my ($ret, $duration, $ip) = $p->ping($host, 5.5);
    printf("$host [ip: $ip] is alive (packet return time: %.2f ms)\n",
            1000 * $duration)
      if $ret;
    $p->close();

    # For backward compatibility
    print "$host is alive.\n" if pingecho($host);

=head1 DESCRIPTION

This module contains methods to test the reachability of remote
hosts on a network.  A ping object is first created with optional
parameters, a variable number of hosts may be pinged multiple
times and then the connection is closed.

You may choose one of six different protocols to use for the
ping. The "tcp" protocol is the default. Note that a live remote host
may still fail to be pingable by one or more of these protocols. For
example, www.microsoft.com is generally alive but not "icmp" pingable.

With the "tcp" protocol the ping() method attempts to establish a
connection to the remote host's echo port.  If the connection is
successfully established, the remote host is considered reachable.  No
data is actually echoed.  This protocol does not require any special
privileges but has higher overhead than the "udp" and "icmp" protocols.

Specifying the "udp" protocol causes the ping() method to send a udp
packet to the remote host's echo port.  If the echoed packet is
received from the remote host and the received packet contains the
same data as the packet that was sent, the remote host is considered
reachable.  This protocol does not require any special privileges.
It should be borne in mind that, for a udp ping, a host
will be reported as unreachable if it is not running the
appropriate echo service.  For Unix-like systems see L<inetd(8)>
for more information.

If the "icmp" protocol is specified, the ping() method sends an icmp
echo message to the remote host, which is what the UNIX ping program
does.  If the echoed message is received from the remote host and
the echoed information is correct, the remote host is considered
reachable.  Specifying the "icmp" protocol requires that the program
be run as root or that the program be setuid to root.

If the "external" protocol is specified, the ping() method attempts to
use the C<Net::Ping::External> module to ping the remote host.
C<Net::Ping::External> interfaces with your system's default C<ping>
utility to perform the ping, and generally produces relatively
accurate results. If C<Net::Ping::External> if not installed on your
system, specifying the "external" protocol will result in an error.

If the "syn" protocol is specified, the L</ping> method will only
send a TCP SYN packet to the remote host then immediately return.
If the syn packet was sent successfully, it will return a true value,
otherwise it will return false.  NOTE: Unlike the other protocols,
the return value does NOT determine if the remote host is alive or
not since the full TCP three-way handshake may not have completed
yet.  The remote host is only considered reachable if it receives
a TCP ACK within the timeout specified.  To begin waiting for the
ACK packets, use the L</ack> method as explained below.  Use the
"syn" protocol instead the "tcp" protocol to determine reachability
of multiple destinations simultaneously by sending parallel TCP
SYN packets.  It will not block while testing each remote host.
This protocol does not require any special privileges.

=head2 Functions

=over 4

=item Net::Ping->new([proto, timeout, bytes, device, tos, ttl, family,
                      host, port, bind, gateway, retrans, pingstring,
                      source_verify econnrefused dontfrag
                      IPV6_USE_MIN_MTU IPV6_RECVPATHMTU])
X<new>

Create a new ping object.  All of the parameters are optional and can
be passed as hash ref.  All options besides the first 7 must be passed
as hash ref.

C<proto> specifies the protocol to use when doing a ping.  The current
choices are "tcp", "udp", "icmp", "icmpv6", "stream", "syn", or
"external".  The default is "tcp".

If a C<timeout> in seconds is provided, it is used
when a timeout is not given to the ping() method (below).  The timeout
must be greater than 0 and the default, if not specified, is 5 seconds.

If the number of data bytes (C<bytes>) is given, that many data bytes
are included in the ping packet sent to the remote host. The number of
data bytes is ignored if the protocol is "tcp".  The minimum (and
default) number of data bytes is 1 if the protocol is "udp" and 0
otherwise.  The maximum number of data bytes that can be specified is
65535, but staying below the MTU (1472 bytes for ICMP) is recommended.
Many small devices cannot deal with fragmented ICMP packets.

If C<device> is given, this device is used to bind the source endpoint
before sending the ping packet.  I believe this only works with
superuser privileges and with udp and icmp protocols at this time.

If <tos> is given, this ToS is configured into the socket.

For icmp, C<ttl> can be specified to set the TTL of the outgoing packet.

Valid C<family> values for IPv4:

   4, v4, ip4, ipv4, AF_INET (constant)

Valid C<family> values for IPv6:

   6, v6, ip6, ipv6, AF_INET6 (constant)

The C<host> argument implicitly specifies the family if the family
argument is not given.

The C<port> argument is only valid for a udp, tcp or stream ping, and will not
do what you think it does. ping returns true when we get a "Connection refused"!
The default is the echo port.

The C<bind> argument specifies the local_addr to bind to.
By specifying a bind argument you don't need the bind method.

The C<gateway> argument is only valid for IPv6, and requires a IPv6
address.

The C<retrans> argument the exponential backoff rate, default 1.2.
It matches the $def_factor global.

The C<dontfrag> argument sets the IP_DONTFRAG bit, but note that
IP_DONTFRAG is not yet defined by Socket, and not available on many
systems. Then it is ignored. On linux it also sets IP_MTU_DISCOVER to
IP_PMTUDISC_DO but need we don't chunk oversized packets. You need to
set $data_size manually.

=item $p->ping($host [, $timeout [, $family]]);
X<ping>

Ping the remote host and wait for a response.  $host can be either the
hostname or the IP number of the remote host.  The optional timeout
must be greater than 0 seconds and defaults to whatever was specified
when the ping object was created.  Returns a success flag.  If the
hostname cannot be found or there is a problem with the IP number, the
success flag returned will be undef.  Otherwise, the success flag will
be 1 if the host is reachable and 0 if it is not.  For most practical
purposes, undef and 0 and can be treated as the same case.  In array
context, the elapsed time as well as the string form of the ip the
host resolved to are also returned.  The elapsed time value will
be a float, as returned by the Time::HiRes::time() function, if hires()
has been previously called, otherwise it is returned as an integer.

=item $p->source_verify( { 0 | 1 } );
X<source_verify>

Allows source endpoint verification to be enabled or disabled.
This is useful for those remote destinations with multiples
interfaces where the response may not originate from the same
endpoint that the original destination endpoint was sent to.
This only affects udp and icmp protocol pings.

This is enabled by default.

=item $p->service_check( { 0 | 1 } );
X<service_check>

Set whether or not the connect behavior should enforce
remote service availability as well as reachability.  Normally,
if the remote server reported ECONNREFUSED, it must have been
reachable because of the status packet that it reported.
With this option enabled, the full three-way tcp handshake
must have been established successfully before it will
claim it is reachable.  NOTE:  It still does nothing more
than connect and disconnect.  It does not speak any protocol
(i.e., HTTP or FTP) to ensure the remote server is sane in
any way.  The remote server CPU could be grinding to a halt
and unresponsive to any clients connecting, but if the kernel
throws the ACK packet, it is considered alive anyway.  To
really determine if the server is responding well would be
application specific and is beyond the scope of Net::Ping.
For udp protocol, enabling this option demands that the
remote server replies with the same udp data that it was sent
as defined by the udp echo service.

This affects the "udp", "tcp", and "syn" protocols.

This is disabled by default.

=item $p->tcp_service_check( { 0 | 1 } );
X<tcp_service_check>

Deprecated method, but does the same as service_check() method.

=item $p->hires( { 0 | 1 } );
X<hires>

With 1 causes this module to use Time::HiRes module, allowing milliseconds
to be returned by subsequent calls to ping().

=item $p->time
X<time>

The current time, hires or not.

=item $p->socket_blocking_mode( $fh, $mode );
X<socket_blocking_mode>

Sets or clears the O_NONBLOCK flag on a file handle.

=item $p->IPV6_USE_MIN_MTU
X<IPV6_USE_MIN_MTU>

With argument sets the option.
Without returns the option value.

=item $p->IPV6_RECVPATHMTU
X<IPV6_RECVPATHMTU>

Notify an according IPv6 MTU.

With argument sets the option.
Without returns the option value.

=item $p->IPV6_HOPLIMIT
X<IPV6_HOPLIMIT>

With argument sets the option.
Without returns the option value.

=item $p->IPV6_REACHCONF I<NYI>
X<IPV6_REACHCONF>

Sets ipv6 reachability
IPV6_REACHCONF was removed in RFC3542. ping6 -R supports it.
IPV6_REACHCONF requires root/admin permissions.

With argument sets the option.
Without returns the option value.

Not yet implemented.

=item $p->bind($local_addr);
X<bind>

Sets the source address from which pings will be sent.  This must be
the address of one of the interfaces on the local host.  $local_addr
may be specified as a hostname or as a text IP address such as
"192.168.1.1".

If the protocol is set to "tcp", this method may be called any
number of times, and each call to the ping() method (below) will use
the most recent $local_addr.  If the protocol is "icmp" or "udp",
then bind() must be called at most once per object, and (if it is
called at all) must be called before the first call to ping() for that
object.

The bind() call can be omitted when specifying the C<bind> option to
new().

=item $p->message_type([$ping_type]);
X<message_type>

When you are using the "icmp" protocol, this call permit to change the
message type to 'echo' or 'timestamp' (only for IPv4, see RFC 792).

Without argument, it returns the currently used icmp protocol message type.
By default, it returns 'echo'.

=item $p->open($host);
X<open>

When you are using the "stream" protocol, this call pre-opens the
tcp socket.  It's only necessary to do this if you want to
provide a different timeout when creating the connection, or
remove the overhead of establishing the connection from the
first ping.  If you don't call C<open()>, the connection is
automatically opened the first time C<ping()> is called.
This call simply does nothing if you are using any protocol other
than stream.

The $host argument can be omitted when specifying the C<host> option to
new().

=item $p->ack( [ $host ] );
X<ack>

When using the "syn" protocol, use this method to determine
the reachability of the remote host.  This method is meant
to be called up to as many times as ping() was called.  Each
call returns the host (as passed to ping()) that came back
with the TCP ACK.  The order in which the hosts are returned
may not necessarily be the same order in which they were
SYN queued using the ping() method.  If the timeout is
reached before the TCP ACK is received, or if the remote
host is not listening on the port attempted, then the TCP
connection will not be established and ack() will return
undef.  In list context, the host, the ack time, the dotted ip 
string, and the port number will be returned instead of just the host.
If the optional C<$host> argument is specified, the return
value will be pertaining to that host only.
This call simply does nothing if you are using any protocol
other than "syn".

When L</new> had a host option, this host will be used.
Without C<$host> argument, all hosts are scanned.

=item $p->nack( $failed_ack_host );
X<nack>

The reason that C<host $failed_ack_host> did not receive a
valid ACK.  Useful to find out why when C<ack($fail_ack_host)>
returns a false value.

=item $p->ack_unfork($host)
X<ack_unfork>

The variant called by L</ack> with the "syn" protocol and C<$syn_forking>
enabled.

=item $p->ping_icmp([$host, $timeout, $family])
X<ping_icmp>

The L</ping> method used with the icmp protocol.

=item $p->ping_icmpv6([$host, $timeout, $family])
X<ping_icmpv6>

The L</ping> method used with the icmpv6 protocol.

=item $p->ping_stream([$host, $timeout, $family])
X<ping_stream>

The L</ping> method used with the stream protocol.

Perform a stream ping.  If the tcp connection isn't
already open, it opens it.  It then sends some data and waits for
a reply.  It leaves the stream open on exit.

=item $p->ping_syn([$host, $ip, $start_time, $stop_time])
X<ping_syn>

The L</ping> method used with the syn protocol.
Sends a TCP SYN packet to host specified.

=item $p->ping_syn_fork([$host, $timeout, $family])
X<ping_syn_fork>

The L</ping> method used with the forking syn protocol.

=item $p->ping_tcp([$host, $timeout, $family])
X<ping_tcp>

The L</ping> method used with the tcp protocol.

=item $p->ping_udp([$host, $timeout, $family])
X<ping_udp>

The L</ping> method used with the udp protocol.

Perform a udp echo ping.  Construct a message of
at least the one-byte sequence number and any additional data bytes.
Send the message out and wait for a message to come back.  If we
get a message, make sure all of its parts match.  If they do, we are
done.  Otherwise go back and wait for the message until we run out
of time.  Return the result of our efforts.

=item $p->ping_external([$host, $timeout, $family])
X<ping_external>

The L</ping> method used with the external protocol.
Uses L<Net::Ping::External> to do an external ping.

=item $p->tcp_connect([$ip, $timeout])
X<tcp_connect>

Initiates a TCP connection, for a tcp ping.

=item $p->tcp_echo([$ip, $timeout, $pingstring])
X<tcp_echo>

Performs a TCP echo.
It writes the given string to the socket and then reads it
back.  It returns 1 on success, 0 on failure.

=item $p->close();
X<close>

Close the network connection for this ping object.  The network
connection is also closed by "undef $p".  The network connection is
automatically closed if the ping object goes out of scope (e.g. $p is
local to a subroutine and you leave the subroutine).

=item $p->port_number([$port_number])
X<port_number>

When called with a port number, the port number used to ping is set to
C<$port_number> rather than using the echo port.  It also has the effect
of calling C<$p-E<gt>service_check(1)> causing a ping to return a successful
response only if that specific port is accessible.  This function returns
the value of the port that L</ping> will connect to.

=item $p->mselect
X<mselect>

A C<select()> wrapper that compensates for platform
peculiarities.

=item $p->ntop
X<ntop>

Platform abstraction over C<inet_ntop()>

=item $p->checksum($msg)
X<checksum>

Do a checksum on the message.  Basically sum all of
the short words and fold the high order bits into the low order bits.

=item $p->icmp_result
X<icmp_result>

Returns a list of addr, type, subcode.

=item pingecho($host [, $timeout]);
X<pingecho>

To provide backward compatibility with the previous version of
L<Net::Ping>, a C<pingecho()> subroutine is available with the same
functionality as before.  C<pingecho()> uses the tcp protocol.  The
return values and parameters are the same as described for the L</ping>
method.  This subroutine is obsolete and may be removed in a future
version of L<Net::Ping>.

=item wakeonlan($mac, [$host, [$port]])
X<wakeonlan>

Emit the popular wake-on-lan magic udp packet to wake up a local
device.  See also L<Net::Wake>, but this has the mac address as 1st arg.
C<$host> should be the local gateway. Without it will broadcast.

Default host: '255.255.255.255'
Default port: 9

  perl -MNet::Ping=wakeonlan -e'wakeonlan "e0:69:95:35:68:d2"'

=back

=head1 NOTES

There will be less network overhead (and some efficiency in your
program) if you specify either the udp or the icmp protocol.  The tcp
protocol will generate 2.5 times or more traffic for each ping than
either udp or icmp.  If many hosts are pinged frequently, you may wish
to implement a small wait (e.g. 25ms or more) between each ping to
avoid flooding your network with packets.

The icmp and icmpv6 protocols requires that the program be run as root
or that it be setuid to root.  The other protocols do not require
special privileges, but not all network devices implement tcp or udp
echo.

Local hosts should normally respond to pings within milliseconds.
However, on a very congested network it may take up to 3 seconds or
longer to receive an echo packet from the remote host.  If the timeout
is set too low under these conditions, it will appear that the remote
host is not reachable (which is almost the truth).

Reachability doesn't necessarily mean that the remote host is actually
functioning beyond its ability to echo packets.  tcp is slightly better
at indicating the health of a system than icmp because it uses more
of the networking stack to respond.

Because of a lack of anything better, this module uses its own
routines to pack and unpack ICMP packets.  It would be better for a
separate module to be written which understands all of the different
kinds of ICMP packets.

=head1 INSTALL

The latest source tree is available via git:

  git clone https://github.com/rurban/Net-Ping.git
  cd Net-Ping

The tarball can be created as follows:

  perl Makefile.PL ; make ; make dist

The latest Net::Ping releases are included in cperl and perl5.

=head1 BUGS

For a list of known issues, visit:

L<https://rt.cpan.org/NoAuth/Bugs.html?Dist=Net-Ping>
and
L<https://github.com/rurban/Net-Ping/issues>

To report a new bug, visit:

L<https://github.com/rurban/Net-Ping/issues>

=head1 AUTHORS

  Current maintainers:
    perl11 (for cperl, with IPv6 support and more)
    p5p    (for perl5)

  Previous maintainers:
    bbb@cpan.org (Rob Brown)
    Steve Peters

  External protocol:
    colinm@cpan.org (Colin McMillen)

  Stream protocol:
    bronson@trestle.com (Scott Bronson)

  Wake-on-lan:
    1999-2003 Clinton Wong

  Original pingecho():
    karrer@bernina.ethz.ch (Andreas Karrer)
    pmarquess@bfsec.bt.co.uk (Paul Marquess)

  Original Net::Ping author:
    mose@ns.ccsn.edu (Russell Mosemann)

=head1 COPYRIGHT

Copyright (c) 2017-2020, Reini Urban.  All rights reserved.

Copyright (c) 2016, cPanel Inc.  All rights reserved.

Copyright (c) 2012, Steve Peters.  All rights reserved.

Copyright (c) 2002-2003, Rob Brown.  All rights reserved.

Copyright (c) 2001, Colin McMillen.  All rights reserved.

This program is free software; you may redistribute it and/or
modify it under the same terms as Perl itself.

=cut
