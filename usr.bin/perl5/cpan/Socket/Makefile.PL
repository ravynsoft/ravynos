#!perl
use strict;
use warnings;

use ExtUtils::MakeMaker;
use ExtUtils::Constant 0.23 'WriteConstants';
use Config;

my @DEFINES;

my $cb;
my $seq = 0;
sub check_for
{
    my %args = @_;
    return if $ENV{PERL_CORE};
    return if defined $args{confkey} and defined $Config{$args{confkey}};

    require ExtUtils::CBuilder;
    $cb ||= ExtUtils::CBuilder->new( quiet => 1 );

    my $main   = $args{main}   || "";
    my $header = $args{header} || "";

    print "Checking $args{define}...\n";

    my $file_base = "test-$seq"; $seq++;

    my $file_source = "$file_base.c";

    {
	open( my $file_source_fh, ">", $file_source ) or die "Cannot write $file_source - $!";
	print $file_source_fh <<"EOF";
#include <stddef.h>
#include <sys/types.h>
#ifdef WIN32
# include <ws2tcpip.h>
# include <winsock.h>
#else
# include <sys/socket.h>
# include <netdb.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif
$header
int main(int argc, char *argv[])
  {
    (void)argc;
    (void)argv;
    { $main }
    return 0;
  }
EOF
    }

    my $file_obj = eval { $cb->compile( source => $file_source ) };
    unlink $file_source;

    return 0 unless defined $file_obj;

    my $file_exe = eval { $cb->link_executable( objects => $file_obj ) };
    unlink $file_obj;

    return 0 unless defined $file_exe;

    # Don't need to try running it
    unlink $file_exe;

    push @DEFINES, $args{define};
}

sub check_for_func
{
    my %args = @_;
    my $func = delete $args{func};
    check_for( %args, main => "void *p = &$func; if(p == NULL) return 1;" );
}

my %defines = (
    # -Dfoo                func()         $Config{key}
    HAS_GETADDRINFO  => [ "getaddrinfo",  "d_getaddrinfo" ],
    HAS_GETNAMEINFO  => [ "getnameinfo",  "d_getnameinfo" ],
    HAS_GAI_STRERROR => [ "gai_strerror", "d_gai_strerror" ],
    HAS_INET_ATON    => [ "inet_aton",    "d_inetaton" ],
    HAS_INETNTOP     => [ "inet_ntop",    "d_inetntop" ],
    HAS_INETPTON     => [ "inet_pton",    "d_inetpton" ],
);

foreach my $define ( sort keys %defines ) {
    my ( $func, $key ) = @{$defines{$define}};
    check_for_func( 
	confkey => $key,
	define  => $define,
	func    => $func
    );
}

check_for(
    confkey => "d_sockaddr_sa_len",
    define  => "HAS_SOCKADDR_SA_LEN",
    main    => "struct sockaddr sa; sa.sa_len = 0;"
);

check_for(
    confkey => "d_sockaddr_in6",
    define  => "HAS_SOCKADDR_IN6",
    main    => "struct sockaddr_in6 sin6; sin6.sin6_family = AF_INET6;"
);

check_for(
    confkey => "d_sin6_scope_id",
    define  => "HAS_SIN6_SCOPE_ID",
    main    => "struct sockaddr_in6 sin6; sin6.sin6_scope_id = 0;"
);

check_for(
    confkey => "d_ip_mreq",
    define  => "HAS_IP_MREQ",
    main    => "struct ip_mreq mreq; mreq.imr_multiaddr.s_addr = INADDR_ANY;"
);

check_for(
    confkey => "d_ip_mreq_source",
    define  => "HAS_IP_MREQ_SOURCE",
    main    => "struct ip_mreq_source mreq; mreq.imr_multiaddr.s_addr = INADDR_ANY;"
);

check_for(
    confkey => "d_ipv6_mreq",
    define  => "HAS_IPV6_MREQ",
    main    => "struct ipv6_mreq mreq; mreq.ipv6mr_interface = 0;"
);

# TODO: Needs adding to perl5 core before importing dual-life again
check_for(
    confkey => "i_netinet_ip",
    define  => "I_NETINET_IP",
    header  => "#include <netinet/ip.h>",
);

my %makefile_args;

# Since we're providing a later version of a core module, before 5.12 the
# @INC order is wrong so we'll have to go in perl rather than site dirs
$makefile_args{INSTALLDIRS} = "perl" if $] < 5.012;

WriteMakefile(
    NAME	  => 'Socket',
    VERSION_FROM  => 'Socket.pm',
    # ABSTRACT_FROM gets confused by C<Socket>
    ABSTRACT      => 'networking constants and support functions',
   ($Config{libs} =~ /(-lsocks\S*)/ ? (LIBS => [ "$1" ] ) : ()),
    XSPROTOARG    => '-noprototypes', 		# XXX remove later?
    realclean     => {FILES=> 'const-c.inc const-xs.inc'},
    DEFINE        => join( " ", map { "-D$_" } @DEFINES ),
    CONFIGURE_REQUIRES => {
	'ExtUtils::CBuilder' => 0,
	'ExtUtils::Constant' => '0.23',
    },
    MIN_PERL_VERSION => '5.006001',
    LICENSE       => 'perl',
    %makefile_args,
);
my @names = (
    qw(
	AF_802 AF_AAL AF_APPLETALK AF_CCITT AF_CHAOS AF_CTF AF_DATAKIT
	AF_DECnet AF_DLI AF_ECMA AF_GOSIP AF_HYLINK AF_IMPLINK AF_INET AF_INET6
	AF_ISO AF_KEY AF_LAST AF_LAT AF_LINK AF_LOCAL AF_MAX AF_NBS AF_NIT
	AF_NS AF_OSI AF_OSINET AF_PUP AF_ROUTE AF_SNA AF_UNIX AF_UNSPEC AF_USER
	AF_WAN AF_X25

	AI_ADDRCONFIG AI_ALL AI_CANONIDN AI_CANONNAME AI_IDN
	AI_NUMERICHOST AI_NUMERICSERV AI_PASSIVE AI_V4MAPPED

	EAI_ADDRFAMILY EAI_AGAIN EAI_BADFLAGS EAI_BADHINTS EAI_FAIL EAI_FAMILY
	EAI_NODATA EAI_NONAME EAI_PROTOCOL EAI_SERVICE EAI_SOCKTYPE EAI_SYSTEM

	IOV_MAX

	IP_ADD_MEMBERSHIP IP_ADD_SOURCE_MEMBERSHIP IP_BIND_ADDRESS_NO_PORT
	IP_DROP_MEMBERSHIP IP_DROP_SOURCE_MEMBERSHIP IP_FREEBIND IP_HDRINCL
	IP_MULTICAST_ALL IP_MULTICAST_IF IP_MULTICAST_LOOP IP_MULTICAST_TTL
	IP_MTU IP_MTU_DISCOVER IP_NODEFRAG IP_OPTIONS IP_RECVERR IP_RECVOPTS
	IP_RECVRETOPTS IP_RETOPTS IP_TOS IP_TRANSPARENT IP_TTL

	IP_PMTUDISC_DO IP_PMTUDISC_DONT IP_PMTUDISC_PROBE IP_PMTUDISC_WANT

	IPTOS_LOWDELAY IPTOS_THROUGHPUT IPTOS_RELIABILITY IPTOS_MINCOST

	IPV6_ADDRFROM IPV6_ADD_MEMBERSHIP IPV6_DROP_MEMBERSHIP IPV6_JOIN_GROUP
	IPV6_LEAVE_GROUP IPV6_MTU IPV6_MTU_DISCOVER IPV6_MULTICAST_HOPS
	IPV6_MULTICAST_IF IPV6_MULTICAST_LOOP IPV6_RECVERR IPV6_ROUTER_ALERT
	IPV6_UNICAST_HOPS IPV6_V6ONLY

	LOCAL_CONNWAIT LOCAL_CREDS LOCAL_OCREDS LOCAL_PEEREID

	MSG_BCAST MSG_BTAG MSG_CTLFLAGS MSG_CTLIGNORE MSG_DONTWAIT MSG_EOF
	MSG_EOR MSG_ERRQUEUE MSG_ETAG MSG_FASTOPEN MSG_FIN MSG_MAXIOVLEN
	MSG_MCAST MSG_NOSIGNAL MSG_RST MSG_SYN MSG_TRUNC MSG_URG MSG_WAITALL
	MSG_WIRE

	NI_DGRAM NI_IDN NI_NAMEREQD NI_NOFQDN NI_NUMERICHOST NI_NUMERICSERV

	PF_802 PF_AAL PF_APPLETALK PF_CCITT PF_CHAOS PF_CTF PF_DATAKIT
	PF_DECnet PF_DLI PF_ECMA PF_GOSIP PF_HYLINK PF_IMPLINK PF_INET PF_INET6
	PF_ISO PF_KEY PF_LAST PF_LAT PF_LINK PF_LOCAL PF_MAX PF_NBS PF_NIT
	PF_NS PF_OSI PF_OSINET PF_PUP PF_ROUTE PF_SNA PF_UNIX PF_UNSPEC PF_USER
	PF_WAN PF_X25

	SCM_CONNECT SCM_CREDENTIALS SCM_CREDS SCM_TIMESTAMP

	SOCK_DGRAM SOCK_RAW SOCK_RDM SOCK_SEQPACKET SOCK_STREAM
	SOCK_NONBLOCK SOCK_CLOEXEC

	SOL_SOCKET 

	SOMAXCONN

	SO_ACCEPTCONN SO_ATTACH_FILTER SO_BACKLOG SO_BINDTODEVICE SO_BROADCAST
	SO_BSDCOMPAT SO_BUSY_POLL SO_CHAMELEON SO_DEBUG SO_DETACH_FILTER
	SO_DGRAM_ERRIND SO_DOMAIN SO_DONTLINGER SO_DONTROUTE SO_ERROR SO_FAMILY
	SO_INCOMING_CPU SO_INCOMING_NAPI_ID SO_KEEPALIVE SO_LINGER
	SO_LOCK_FILTER SO_MARK SO_OOBINLINE SO_PASSCRED SO_PASSIFNAME
	SO_PEEK_OFF SO_PEERCRED SO_PRIORITY SO_PROTOCOL SO_PROTOTYPE SO_RCVBUF
	SO_RCVBUFFORCE SO_RCVLOWAT SO_RCVTIMEO SO_REUSEADDR SO_REUSEPORT
	SO_RXQ_OVFL SO_SECURITY_AUTHENTICATION SO_SECURITY_ENCRYPTION_NETWORK
	SO_SECURITY_ENCRYPTION_TRANSPORT SO_SNDBUF SO_SNDBUFFORCE SO_SNDLOWAT
	SO_SNDTIMEO SO_STATE SO_TIMESTAMP SO_TYPE SO_USELOOPBACK SO_XOPEN
	SO_XSE

	TCP_CONGESTION TCP_CONNECTIONTIMEOUT TCP_CORK TCP_DEFER_ACCEPT
	TCP_FASTOPEN TCP_INFO TCP_INIT_CWND TCP_KEEPALIVE TCP_KEEPCNT
	TCP_KEEPIDLE TCP_KEEPINTVL TCP_LINGER2 TCP_MAXRT TCP_MAXSEG
	TCP_MD5SIG TCP_NODELAY TCP_NOOPT TCP_NOPUSH TCP_QUICKACK
	TCP_SACK_ENABLE TCP_STDURG TCP_SYNCNT TCP_USER_TIMEOUT
	TCP_WINDOW_CLAMP

	UIO_MAXIOV
    ),
    {name=>"IPPROTO_IP",     type=>"IV", default=>["IV",   0]},
    {name=>"IPPROTO_IPV6",   type=>"IV", default=>["IV",  41]},
    {name=>"IPPROTO_RAW",    type=>"IV", default=>["IV", 255]},
    {name=>"IPPROTO_ICMP",   type=>"IV", default=>["IV",   1]},
    {name=>"IPPROTO_IGMP",   type=>"IV", default=>["IV",   2]},
    {name=>"IPPROTO_TCP",    type=>"IV", default=>["IV",   6]},
    {name=>"IPPROTO_UDP",    type=>"IV", default=>["IV",  17]},
    {name=>"IPPROTO_GRE",    type=>"IV", default=>["IV",  47]},
    {name=>"IPPROTO_ESP",    type=>"IV", default=>["IV",  50]},
    {name=>"IPPROTO_AH",     type=>"IV", default=>["IV",  51]},
    {name=>"IPPROTO_ICMPV6", type=>"IV", default=>["IV",  58]},
    {name=>"IPPROTO_SCTP",   type=>"IV", default=>["IV", 132]},
    {name=>"SHUT_RD",   type=>"IV", default=>["IV", "0"]},
    {name=>"SHUT_WR",   type=>"IV", default=>["IV", "1"]},
    {name=>"SHUT_RDWR", type=>"IV", default=>["IV", "2"]},
);

push @names, {
    name  => $_,
    type  => "IV",
    macro => [ "#if defined($_) || defined(HAS_$_) /* might be an enum */\n",
               "#endif\n" ]
} foreach qw (MSG_CTRUNC MSG_DONTROUTE MSG_OOB MSG_PEEK MSG_PROXY SCM_RIGHTS);

push @names, {
    name  => $_,
    type  => "SV",
    pre   => "struct in_addr ip_address; ip_address.s_addr = htonl($_);",
    value => "newSVpvn_flags((char *)&ip_address,sizeof(ip_address), SVs_TEMP)",
} foreach qw(INADDR_ANY INADDR_LOOPBACK INADDR_NONE INADDR_BROADCAST);

push @names, {
    name  => $_,
    type  => "SV",
    macro => [ "#ifdef ${_}_INIT\n",
               "#endif\n" ],
    pre   => "struct in6_addr ip6_address = ${_}_INIT;",
    value => "newSVpvn_flags((char *)&ip6_address,sizeof(ip6_address), SVs_TEMP)",
} foreach qw(IN6ADDR_ANY IN6ADDR_LOOPBACK);

# Work around an old Perl core bug that affects ExtUtils::Constants on
# pre-5.8.2 Perls.  EU:C should be amended to work around this itself.
if("$]" < 5.008002) {
    require ExtUtils::Constant::ProxySubs;
    no warnings "once";
    $ExtUtils::Constant::ProxySubs::type_to_C_value{$_} = sub { () }
        foreach qw(YES NO UNDEF), "";
}

WriteConstants(
    PROXYSUBS => {autoload => 1},
    NAME => 'Socket',
    NAMES => \@names,
);
