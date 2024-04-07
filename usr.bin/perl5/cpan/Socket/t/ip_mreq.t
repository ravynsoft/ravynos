use v5.6.1;
use strict;
use warnings;
use Test::More;

use Socket qw(
    INADDR_ANY
    pack_ip_mreq unpack_ip_mreq
    pack_ip_mreq_source unpack_ip_mreq_source
);

# Check that pack/unpack_ip_mreq either croak with "Not implemented", or
# roundtrip as identity

my $packed;
eval {
    $packed = pack_ip_mreq "\xe0\0\0\1", INADDR_ANY;
};
if( !defined $packed ) {
    plan skip_all => "No pack_ip_mreq" if $@ =~ m/ not implemented /;
    die $@;
}

plan tests => 6;

my @unpacked = unpack_ip_mreq $packed;

is( $unpacked[0], "\xe0\0\0\1", 'unpack_ip_mreq multiaddr' );
is( $unpacked[1], INADDR_ANY,   'unpack_ip_mreq interface' );

is( (unpack_ip_mreq pack_ip_mreq "\xe0\0\0\1")[1], INADDR_ANY, 'pack_ip_mreq interface defaults to INADDR_ANY' );

SKIP: {
    my $mreq;
    skip "No pack_ip_mreq_source", 3 unless defined eval { $mreq = pack_ip_mreq_source "\xe0\0\0\2", "\x0a\0\0\1", INADDR_ANY };

    @unpacked = unpack_ip_mreq_source $mreq;

    is( $unpacked[0], "\xe0\0\0\2", 'unpack_ip_mreq_source multiaddr' );
    is( $unpacked[1], "\x0a\0\0\1", 'unpack_ip_mreq_source source' );
    is( $unpacked[2], INADDR_ANY,   'unpack_ip_mreq_source interface' );
}
