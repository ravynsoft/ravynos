use v5.6.1;
use strict;
use warnings;
use Test::More tests => 13;

use Socket qw(:addrinfo AF_INET pack_sockaddr_in inet_aton);

my ( $err, $host, $service );

( $err, $host, $service ) = getnameinfo( pack_sockaddr_in( 80, inet_aton( "127.0.0.1" ) ), NI_NUMERICHOST|NI_NUMERICSERV );
cmp_ok( $err, "==", 0, '$err == 0 for {family=AF_INET,port=80,sinaddr=127.0.0.1}/NI_NUMERICHOST|NI_NUMERICSERV' );
cmp_ok( $err, "eq", "", '$err eq "" for {family=AF_INET,port=80,sinaddr=127.0.0.1}/NI_NUMERICHOST|NI_NUMERICSERV' );

is( $host, "127.0.0.1", '$host is 127.0.0.1 for NH/NS' );
is( $service, "80", '$service is 80 for NH/NS' );

( $err, $host, $service ) = getnameinfo( pack_sockaddr_in( 80, inet_aton( "127.0.0.1" ) ), NI_NUMERICHOST|NI_NUMERICSERV, NIx_NOHOST );
is( $host, undef, '$host is undef for NIx_NOHOST' );
is( $service, "80", '$service is 80 for NS, NIx_NOHOST' );

( $err, $host, $service ) = getnameinfo( pack_sockaddr_in( 80, inet_aton( "127.0.0.1" ) ), NI_NUMERICHOST|NI_NUMERICSERV, NIx_NOSERV );
is( $host, "127.0.0.1", '$host is 127.0.0.1 for NIx_NOSERV' );
is( $service, undef, '$service is undef for NS, NIx_NOSERV' );

( $err, $host, $service ) = getnameinfo( pack_sockaddr_in( 80, inet_aton( "127.0.0.1" ) ), NI_NUMERICSERV );
cmp_ok( $err, "==", 0, '$err == 0 for {family=AF_INET,port=80,sinaddr=127.0.0.1}/NI_NUMERICSERV' );

# We can't meaningfully compare '$host' with anything specific, all we can be
# sure is it's not empty
ok( length $host, '$host is nonzero length for NS' );

( $err, $host, $service ) = getnameinfo( pack_sockaddr_in( 80, inet_aton( "127.0.0.1" ) ), NI_NUMERICHOST | NI_NUMERICSERV );
cmp_ok( $err, "==", 0, '$err == 0 for {family=AF_INET,port=80,sinaddr=127.0.0.1}/NI_NUMERICHOST' );

ok( length $service, '$service is nonzero length for NH' );

# RT79557
pack_sockaddr_in( 80, inet_aton( "127.0.0.1" ) ) =~ m/^(.*)$/s;
( $err, $host, $service ) = getnameinfo( $1, NI_NUMERICHOST|NI_NUMERICSERV );
cmp_ok( $err, "==", 0, '$err == 0 for $1' ) or diag( '$err was: ' . $err );
