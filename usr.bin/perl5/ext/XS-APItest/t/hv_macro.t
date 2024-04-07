#!perl -w

use strict;
use warnings;
use Test::More;
use Devel::Peek;
use Config;

BEGIN { use_ok('XS::APItest') }

my $is_wide = $Config{ivsize} == 8;

sub test_rot {
    my ( $fnc, $n, $r, $max ) = @_;
    my %seen;
    my @seq;
    while ( @seq < $max and !$seen{$n}++ ) {
        push @seq, $n;
        $n = $fnc->( $n, $r );
    }
    return \@seq;
}

for my $test (
    [
        # source string:
        "\x{12}\x{34}\x{56}\x{78}\x{9A}\x{BC}\x{DE}\x{F0}" x 2,

        #results:
        #16         32              64
        "0x3412", "0x78563412", "0xf0debc9a78563412",
        "0x5634", "0x9A785634", "0x12f0debc9a785634",
        "0x7856", "0xBC9A7856", "0x3412f0debc9a7856",
        "0x9A78", "0xDEBC9A78", "0x563412f0debc9a78",
        "0xBC9A", "0xF0DEBC9A", "0x78563412f0debc9a",
        "0xDEBC", "0x12F0DEBC", "0x9a78563412f0debc",
        "0xF0DE", "0x3412F0DE", "0xbc9a78563412f0de",
        "0x12F0", "0x563412F0", "0xdebc9a78563412f0",
    ],
    [
        # source string:
        "\x{F0}\x{E1}\x{D2}\x{C3}\x{B4}\x{A5}\x{96}\x{87}" x 2,

        #results:
        #16         32              64
        "0xe1f0", "0xc3d2e1f0", "0x8796a5b4c3d2e1f0",
        "0xd2e1", "0xb4c3d2e1", "0xf08796a5b4c3d2e1",
        "0xc3d2", "0xa5b4c3d2", "0xe1f08796a5b4c3d2",
        "0xb4c3", "0x96a5b4c3", "0xd2e1f08796a5b4c3",
        "0xa5b4", "0x8796a5b4", "0xc3d2e1f08796a5b4",
        "0x96a5", "0xf08796a5", "0xb4c3d2e1f08796a5",
        "0x8796", "0xe1f08796", "0xa5b4c3d2e1f08796",
        "0xf087", "0xd2e1f087", "0x96a5b4c3d2e1f087",
    ],
  )
{
    my $str = $test->[0];
    for my $ofs ( 0 .. 7 ) {
        my $n = ( $ofs * 3 ) + 1;
        my ( $want16, $want32, $want64 ) = @{$test}[ $n .. ( $n + 2 ) ];
        my $input = join " ", map { sprintf "%02x", ord($_) } split //,
          substr $str, $ofs, 8;
        my $hex16 = sprintf "0x%04x",
          XS::APItest::HvMacro::u8_to_u16_le( $str, $ofs );
        is( $hex16, lc($want16),
                "U8TO16_LE works as expected (hex bytes:"
              . substr( $input, 0, 4 + 1 )
              . ")" );
        my $hex32 = sprintf "0x%08x",
          XS::APItest::HvMacro::u8_to_u32_le( $str, $ofs );
        is( $hex32, lc($want32),
                "U8TO32_LE works as expected (hex bytes:"
              . substr( $input, 0, 8 + 3 )
              . ")" );
        next unless $is_wide;
        my $hex64 = sprintf "0x%016x",
          XS::APItest::HvMacro::u8_to_u64_le( $str, $ofs );
        is( $hex64, lc($want64),
                "U8TO64_LE works as expected (hex bytes:"
              . substr( $input, 0, 16 + 7 )
              . ")" );
    }
}
{
    my $seq_l32 = test_rot( \&XS::APItest::HvMacro::rotl32, 1, 1, 33 );
    is( 0 + @$seq_l32, 32, "rotl32(n,1) works as expected" );
    is_deeply(
        $seq_l32,
        [
            1,         2,         4,          8,
            16,        32,        64,         128,
            256,       512,       1024,       2048,
            4096,      8192,      16384,      32768,
            65536,     131072,    262144,     524288,
            1048576,   2097152,   4194304,    8388608,
            16777216,  33554432,  67108864,   134217728,
            268435456, 536870912, 1073741824, 2147483648
        ],
        "rotl32(n,1) returned expected results"
    );
    my $seq_r32 = test_rot( \&XS::APItest::HvMacro::rotr32, 1, 1, 33 );
    is( 0 + @$seq_r32, 32, "rotr32(n,1) works as expected" );
    is_deeply(
        $seq_r32,
        [
            1,         2147483648, 1073741824, 536870912,
            268435456, 134217728,  67108864,   33554432,
            16777216,  8388608,    4194304,    2097152,
            1048576,   524288,     262144,     131072,
            65536,     32768,      16384,      8192,
            4096,      2048,       1024,       512,
            256,       128,        64,         32,
            16,        8,          4,          2
        ],
        "rotr32(n,1) returned expected"
    );
    isnt( "@$seq_l32", "@$seq_r32",
        "rotl32(n,1) and rotr32(n,1) return different results" );
}
if ($is_wide) {
    my $seq_l64 = test_rot( \&XS::APItest::HvMacro::rotl64, 1, 1, 65 );
    is( 0 + @$seq_l64, 64, "rotl64(n,1) works as expected" );
    is_deeply(
        $seq_l64,
        [
            1,                     2,
            4,                     8,
            16,                    32,
            64,                    128,
            256,                   512,
            1024,                  2048,
            4096,                  8192,
            16384,                 32768,
            65536,                 131072,
            262144,                524288,
            1048576,               2097152,
            4194304,               8388608,
            16777216,              33554432,
            67108864,              134217728,
            268435456,             536870912,
            1073741824,            2147483648,
            4294967296,            8589934592,
            '17179869184',         '34359738368',
            '68719476736',         '137438953472',
            '274877906944',        '549755813888',
            '1099511627776',       '2199023255552',
            '4398046511104',       '8796093022208',
            '17592186044416',      '35184372088832',
            '70368744177664',      '140737488355328',
            '281474976710656',     '562949953421312',
            '1125899906842624',    '2251799813685248',
            '4503599627370496',    '9007199254740992',
            '18014398509481984',   '36028797018963968',
            '72057594037927936',   '144115188075855872',
            '288230376151711744',  '576460752303423488',
            '1152921504606846976', '2305843009213693952',
            '4611686018427387904', '9223372036854775808'
        ],
        "rotl64(n,1) returned expected results"
    );
    my $seq_r64 = test_rot( \&XS::APItest::HvMacro::rotr64, 1, 1, 65 );
    is( 0 + @$seq_r64, 64, "rotr64(n,1) works as expected" );
    is_deeply(
        $seq_r64,
        [
            1,                     '9223372036854775808',
            '4611686018427387904', '2305843009213693952',
            '1152921504606846976', '576460752303423488',
            '288230376151711744',  '144115188075855872',
            '72057594037927936',   '36028797018963968',
            '18014398509481984',   '9007199254740992',
            '4503599627370496',    '2251799813685248',
            '1125899906842624',    '562949953421312',
            '281474976710656',     '140737488355328',
            '70368744177664',      '35184372088832',
            '17592186044416',      '8796093022208',
            '4398046511104',       '2199023255552',
            '1099511627776',       '549755813888',
            '274877906944',        '137438953472',
            '68719476736',         '34359738368',
            '17179869184',         8589934592,
            4294967296,            2147483648,
            1073741824,            536870912,
            268435456,             134217728,
            67108864,              33554432,
            16777216,              8388608,
            4194304,               2097152,
            1048576,               524288,
            262144,                131072,
            65536,                 32768,
            16384,                 8192,
            4096,                  2048,
            1024,                  512,
            256,                   128,
            64,                    32,
            16,                    8,
            4,                     2
        ],
        "rotr64(n,1) returned expected results"
    );
    isnt( "@$seq_l64", "@$seq_r64",
        "rotl64(n,1) and rotr64(n,1) return different results" );
}
if ($is_wide) {
    push @INC, '../../t';
    require 'charset_tools.pl';

    # The values here are from the ASCII/Unicode code points; so if on EBCDIC
    # we need # to convert from native to uni to get the same values

    my $seed  = native_to_uni("perl is for good");
    my $state = XS::APItest::HvMacro::siphash_seed_state($seed);
    is(
        sprintf( "%016x",
            XS::APItest::HvMacro::siphash24( $state, native_to_uni("Larry wall is BDFL")) ),
        "71a11e065cefc12c",
        "Siphash24 seems to work"
    );
    is(
        sprintf( "%016x",
            XS::APItest::HvMacro::siphash13( $state, native_to_uni("Larry wall is BDFL" ))),
        "adee71f47e49757a",
        "Siphash13 seems to work"
    );
    is( XS::APItest::HvMacro::test_siphash24(), 0, "siphash24 test vectors check" );
    is( XS::APItest::HvMacro::test_siphash13(), 0, "siphash13 test vectors check" );
}
done_testing();

