#!./perl

use v5.6.1;
use strict;
use warnings;

use Socket qw(
    AF_INET
    inet_ntoa inet_aton inet_ntop inet_pton
    pack_sockaddr_in unpack_sockaddr_in sockaddr_in
    pack_sockaddr_un unpack_sockaddr_un
    sockaddr_family
    sockaddr_un
);
use Test::More tests => 50;

# inet_aton, inet_ntoa
{
    is(join(".", unpack("C*",inet_aton("10.20.30.40"))), "10.20.30.40", 'inet_aton returns packed bytes');

    is(inet_ntoa(v10.20.30.40), "10.20.30.40", 'inet_ntoa from v-string');

    is(inet_ntoa(inet_aton("10.20.30.40")), "10.20.30.40", 'inet_aton->inet_ntoa roundtrip');

    local $@;
    eval { inet_ntoa(v10.20.30.400) };
    like($@, qr/^Wide character in Socket::inet_ntoa at/, 'inet_ntoa warns about wide characters');
}

# inet_ntop, inet_pton
SKIP: {
    skip "No inet_ntop", 5 unless defined eval { inet_pton(AF_INET, "10.20.30.40") };

    is(join(".", unpack("C*",inet_pton(AF_INET, "10.20.30.40"))), "10.20.30.40", 'inet_pton AF_INET returns packed bytes');

    is(inet_ntop(AF_INET, v10.20.30.40), "10.20.30.40", 'inet_ntop AF_INET from v-string');

    is(inet_ntop(AF_INET, inet_pton(AF_INET, "10.20.30.40")), "10.20.30.40", 'inet_pton->inet_ntop AF_INET roundtrip');
    is(inet_ntop(AF_INET, inet_aton("10.20.30.40")), "10.20.30.40", 'inet_aton->inet_ntop AF_INET roundtrip');

    local $@;
    eval { inet_ntop(AF_INET, v10.20.30.400) };
    like($@, qr/^Wide character in Socket::inet_ntop at/, 'inet_ntop warns about wide characters');
}

SKIP: {
    skip "No AF_INET6", 3 unless my $AF_INET6 = eval { Socket::AF_INET6() };
    skip "No inet_ntop", 3 unless defined eval { inet_pton($AF_INET6, "2460::1") };

    is(uc unpack("H*",inet_pton($AF_INET6, "2001:503:BA3E::2:30")), "20010503BA3E00000000000000020030",
        'inet_pton AF_INET6 returns packed bytes');

    is(uc inet_ntop($AF_INET6, "\x20\x01\x05\x03\xBA\x3E\x00\x00\x00\x00\x00\x00\x00\x02\x00\x30"), "2001:503:BA3E::2:30",
        'inet_ntop AF_INET6 from octet string');

    is(lc inet_ntop($AF_INET6, inet_pton($AF_INET6, "2001:503:BA3E::2:30")), "2001:503:ba3e::2:30",
        'inet_pton->inet_ntop AF_INET6 roundtrip');
}

# sockaddr_family
{
    local $@;
    eval { sockaddr_family("") };
    like($@, qr/^Bad arg length for Socket::sockaddr_family, length is 0, should be at least \d+/, 'sockaddr_family warns about argument length');
}

# pack_sockaddr_in, unpack_sockaddr_in
# sockaddr_in
{
    my $sin = pack_sockaddr_in 100, inet_aton("10.20.30.40");
    ok(defined $sin, 'pack_sockaddr_in defined');

    is(sockaddr_family($sin), AF_INET, 'sockaddr_family of pack_sockaddr_in' );

    is(          (unpack_sockaddr_in($sin))[0] , 100,           'pack_sockaddr_in->unpack_sockaddr_in port');
    is(inet_ntoa((unpack_sockaddr_in($sin))[1]), "10.20.30.40", 'pack_sockaddr_in->unpack_sockaddr_in addr');

    is(inet_ntoa(scalar unpack_sockaddr_in($sin)), "10.20.30.40", 'unpack_sockaddr_in in scalar context yields addr');

    is_deeply( [ sockaddr_in($sin) ], [ unpack_sockaddr_in($sin) ],
        'sockaddr_in in list context unpacks' );

    is(sockaddr_family(scalar sockaddr_in(200,v10.30.50.70)), AF_INET,
        'sockaddr_in in scalar context packs');

    my $warnings = "";
    local $SIG{__WARN__} = sub { $warnings .= $_[0]; };
    ok( !eval { pack_sockaddr_in 0, undef; 1 },
        'pack_sockaddr_in undef addr is fatal' );
    ok( !eval { unpack_sockaddr_in undef; 1 },
        'unpack_sockaddr_in undef is fatal' );

    ok( eval { pack_sockaddr_in undef, "\0\0\0\0"; 1 },
        'pack_sockaddr_in undef port is allowed' );

    is( $warnings, "", 'undefined values produced no warnings' );

    ok( eval { pack_sockaddr_in 98765, "\0\0\0\0"; 1 },
        'pack_sockaddr_in oversized port is allowed' );
    like( $warnings, qr/^Port number above 0xFFFF, will be truncated to 33229 for Socket::pack_sockaddr_in at /,
        'pack_sockaddr_in oversized port warning' );
}

# pack_sockaddr_in6, unpack_sockaddr_in6
# sockaddr_in6
SKIP: {
    skip "No AF_INET6", 15 unless my $AF_INET6 = eval { Socket::AF_INET6() };
    skip "Cannot pack_sockaddr_in6()", 15 unless my $sin6 = eval { Socket::pack_sockaddr_in6(0x1234, "0123456789abcdef", 0, 89) };

    ok(defined $sin6, 'pack_sockaddr_in6 defined');

    is(sockaddr_family($sin6), $AF_INET6, 'sockaddr_family of pack_sockaddr_in6');

    is((Socket::unpack_sockaddr_in6($sin6))[0], 0x1234,             'pack_sockaddr_in6->unpack_sockaddr_in6 port');
    is((Socket::unpack_sockaddr_in6($sin6))[1], "0123456789abcdef", 'pack_sockaddr_in6->unpack_sockaddr_in6 addr');
    is((Socket::unpack_sockaddr_in6($sin6))[2], 0,                  'pack_sockaddr_in6->unpack_sockaddr_in6 scope_id');
    is((Socket::unpack_sockaddr_in6($sin6))[3], 89,                 'pack_sockaddr_in6->unpack_sockaddr_in6 flowinfo');

    is(scalar Socket::unpack_sockaddr_in6($sin6), "0123456789abcdef", 'unpack_sockaddr_in6 in scalar context yields addr');

    is_deeply( [ Socket::sockaddr_in6($sin6) ], [ Socket::unpack_sockaddr_in6($sin6) ],
        'sockaddr_in6 in list context unpacks' );

    is(sockaddr_family(scalar Socket::sockaddr_in6(0x1357, "02468ace13579bdf")), $AF_INET6,
        'sockaddr_in6 in scalar context packs' );

    my $warnings = "";
    local $SIG{__WARN__} = sub { $warnings .= $_[0]; };
    ok( !eval { Socket::pack_sockaddr_in6( 0, undef ); 1 },
        'pack_sockaddr_in6 undef addr is fatal' );
    ok( !eval { Socket::unpack_sockaddr_in6( undef ); 1 },
        'unpack_sockaddr_in6 undef is fatal' );

    ok( eval { Socket::pack_sockaddr_in6( undef, "\0"x16 ); 1 },
        'pack_sockaddr_in6 undef port is allowed' );

    is( $warnings, "", 'undefined values produced no warnings' );

    ok( eval { Socket::pack_sockaddr_in6( 98765, "\0"x16 ); 1 },
        'pack_sockaddr_in6 oversized port is allowed' );
    like( $warnings, qr/^Port number above 0xFFFF, will be truncated to 33229 for Socket::pack_sockaddr_in6 at /,
        'pack_sockaddr_in6 oversized port warning' );
}

# sockaddr_un on abstract paths
SKIP: {
    # see if we can handle abstract sockets
    skip "Abstract AF_UNIX paths unsupported", 7 unless $^O eq "linux";

    my $test_abstract_socket = chr(0) . '/org/perl/hello'. chr(0) . 'world';
    my $addr = sockaddr_un ($test_abstract_socket);
    my ($path) = sockaddr_un ($addr);
    is($path, $test_abstract_socket, 'sockaddr_un can handle abstract AF_UNIX paths');

    # see if we calculate the address structure length correctly
    is(length ($test_abstract_socket) + 2, length $addr, 'sockaddr_un abstract address length');

    my $warnings = 0;
    local $SIG{__WARN__} = sub { $warnings++ };
    ok( !eval { pack_sockaddr_un( undef ); 1 },
        'pack_sockaddr_un undef path is fatal' );
    ok( !eval { unpack_sockaddr_un( undef ); 1 },
        'unpack_sockaddr_un undef is fatal' );

    is( $warnings, 0, 'undefined values produced no warnings' );

    ok( eval { pack_sockaddr_un( "x" x 0x10000 ); 1 },
        'pack_sockaddr_un(very long path) succeeds' ) or diag( "Died: $@" );
    is( $warnings, 1, 'pack_sockaddr_in(very long path) warns' );
}

# warnings
{
    my $w = 0;
    local $SIG{__WARN__} = sub {
	++ $w if $_[0] =~ /^6-ARG sockaddr_in call is deprecated/ ;
    };

    no warnings 'Socket';
    sockaddr_in(1,2,3,4,5,6) ;
    is($w, 0, "sockaddr_in deprecated form doesn't warn without lexical warnings");

    use warnings 'Socket';
    sockaddr_in(1,2,3,4,5,6) ;
    is($w, 1, "sockaddr_in deprecated form warns with lexical warnings");
}
