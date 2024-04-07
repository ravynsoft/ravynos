use v5.6.1;
use strict;
use warnings;
use Test::More;

use Socket qw(
    pack_ipv6_mreq unpack_ipv6_mreq
);

# Check that pack/unpack_ipv6_mreq either croak with "Not implemented", or
# roundtrip as identity

my $packed;
eval {
    $packed = pack_ipv6_mreq "ANADDRESSIN16CHR", 123;
};
if( !defined $packed ) {
    plan skip_all => "No pack_ipv6_mreq" if $@ =~ m/ not implemented /;
    die $@;
}

plan tests => 2;

my @unpacked = unpack_ipv6_mreq $packed;

is( $unpacked[0], "ANADDRESSIN16CHR", 'unpack_ipv6_mreq multiaddr' );
is( $unpacked[1], 123,                'unpack_ipv6_mreq ifindex' );
