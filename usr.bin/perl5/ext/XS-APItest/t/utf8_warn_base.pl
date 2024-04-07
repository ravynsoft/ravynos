#!perl -w

# This is a base file to be used by various .t's in its directory
# It tests various malformed UTF-8 sequences and some code points that are
# "problematic", and verifies that the correct warnings/flags etc are
# generated when using them.  For the code points, it also takes the UTF-8 and
# perturbs it to be malformed in various ways, and tests that this gets
# appropriately detected.

use strict;
use Test::More;

BEGIN {
    use_ok('XS::APItest');
    require 'charset_tools.pl';
    require './t/utf8_setup.pl';
};

$|=1;

use XS::APItest;

my @warnings_gotten;

use warnings 'utf8';
local $SIG{__WARN__} = sub { my @copy = @_;
                             push @warnings_gotten, map { chomp; $_ } @copy;
                           };

my $highest_non_extended_utf8_cp = (isASCII) ? 0x7FFFFFFF : 0x3FFFFFFF;
my $native_lowest_continuation_chr = I8_to_native(chr $::lowest_continuation);

# C5 is chosen as it is valid for both ASCII and EBCDIC platforms
my $known_start_byte = I8_to_native("\xC5");

sub requires_extended_utf8($) {

    # Returns a boolean as to whether or not the code point parameter fits
    # into 31 bits (30 on EBCDIC), subject to the convention that a negative
    # code point stands for one that overflows the word size, so won't fit in
    # 31 bits.

    return shift > $highest_non_extended_utf8_cp;
}

sub is_extended_utf8($) {

    # Returns a boolean as to whether or not the input UTF-8 sequence uses
    # Perl extended UTF-8.

    my $byte = substr(shift, 0, 1);
    return ord $byte >= 0xFE if isASCII;
    return $byte == I8_to_native("\xFF");
}

sub overflow_discern_len($) {

    # Returns how many bytes are needed to tell if a non-overlong UTF-8
    # sequence is for a code point that won't fit in the platform's word size.
    # Only the length of the sequence representing a single code point is
    # needed.

    if (isASCII) {
        return ($::is64bit) ? 3 : 1;

        # Below is needed for code points above IV_MAX
        #return ($::is64bit) ? 3 : ((shift == $::max_bytes)
        #                           ? 1
        #                           : 2);
    }

    return ($::is64bit) ? 2 : 8;
}

sub overlong_discern_len($) {

    # Returns how many bytes are needed to tell if the input UTF-8 sequence
    # for a code point is overlong

    my $string = shift;
    my $length = length $string;
    my $byte = ord native_to_I8(substr($string, 0, 1));
    if (isASCII) {
        return ($byte >= 0xFE)
                ? ((! $::is64bit)
                    ? 1
                    : ($byte == 0xFF) ? 7 : 2)
                : (($length == 2) ? 1 : 2);
        # Below is needed for code points above IV_MAX
        #return ($length == $::max_bytes)
        #          # This is constrained to 1 on 32-bit machines, as it
        #          # overflows there
        #        ? (($::is64bit) ? 7 : 1)
        #        : (($length == 2) ? 1 : 2);
    }

    return ($length == $::max_bytes) ? 8 : (($length <= 3) ? 1 : 2);
}

my @tests;
{
    no warnings qw(portable overflow);
    @tests = (
        # $testname,
        # $bytes,                  UTF-8 string
        # $allowed_uv,             code point $bytes evaluates to; -1 if
        #                          overflows
        # $needed_to_discern_len   optional, how long an initial substring do
        #                          we need to tell that the string must be for
        #                          a code point in the category it falls in,
        #                          like being a surrogate; 0 indicates we need
        #                          the whole string.  Some categories have a
        #                          default that is used if this is omitted.
        [ "orphan continuation byte malformation",
            I8_to_native("$::I8c"),
            0xFFFD,
            1,
        ],
        [ "overlong malformation, lowest 2-byte",
            (isASCII) ? "\xc0\x80" : I8_to_native("\xc0\xa0"),
            0,   # NUL
        ],
        [ "overlong malformation, highest 2-byte",
            (isASCII) ? "\xc1\xbf" : I8_to_native("\xc4\xbf"),
            (isASCII) ? 0x7F : 0xFF,
        ],
        [ "overlong malformation, lowest 3-byte",
            (isASCII) ? "\xe0\x80\x80" : I8_to_native("\xe0\xa0\xa0"),
            0,   # NUL
        ],
        [ "overlong malformation, highest 3-byte",
            (isASCII) ? "\xe0\x9f\xbf" : I8_to_native("\xe0\xbf\xbf"),
            (isASCII) ? 0x7FF : 0x3FF,
        ],
        [ "lowest surrogate",
            (isASCII) ? "\xed\xa0\x80" : I8_to_native("\xf1\xb6\xa0\xa0"),
            0xD800,
        ],
        [ "a middle surrogate",
            (isASCII) ? "\xed\xa4\x8d" : I8_to_native("\xf1\xb6\xa8\xad"),
            0xD90D,
        ],
        [ "highest surrogate",
            (isASCII) ? "\xed\xbf\xbf" : I8_to_native("\xf1\xb7\xbf\xbf"),
            0xDFFF,
        ],
        [ "first of 32 consecutive non-character code points",
            (isASCII) ? "\xef\xb7\x90" : I8_to_native("\xf1\xbf\xae\xb0"),
            0xFDD0,
        ],
        [ "a mid non-character code point of the 32 consecutive ones",
            (isASCII) ? "\xef\xb7\xa0" : I8_to_native("\xf1\xbf\xaf\xa0"),
            0xFDE0,
        ],
        [ "final of 32 consecutive non-character code points",
            (isASCII) ? "\xef\xb7\xaf" : I8_to_native("\xf1\xbf\xaf\xaf"),
            0xFDEF,
        ],
        [ "non-character code point U+FFFE",
            (isASCII) ? "\xef\xbf\xbe" : I8_to_native("\xf1\xbf\xbf\xbe"),
            0xFFFE,
        ],
        [ "non-character code point U+FFFF",
            (isASCII) ? "\xef\xbf\xbf" : I8_to_native("\xf1\xbf\xbf\xbf"),
            0xFFFF,
        ],
        [ "overlong malformation, lowest 4-byte",
            (isASCII) ? "\xf0\x80\x80\x80" : I8_to_native("\xf0\xa0\xa0\xa0"),
            0,   # NUL
        ],
        [ "overlong malformation, highest 4-byte",
            (isASCII) ? "\xf0\x8F\xbf\xbf" : I8_to_native("\xf0\xaf\xbf\xbf"),
            (isASCII) ? 0xFFFF : 0x3FFF,
        ],
        [ "non-character code point U+1FFFE",
            (isASCII) ? "\xf0\x9f\xbf\xbe" : I8_to_native("\xf3\xbf\xbf\xbe"),
            0x1FFFE,
        ],
        [ "non-character code point U+1FFFF",
            (isASCII) ? "\xf0\x9f\xbf\xbf" : I8_to_native("\xf3\xbf\xbf\xbf"),
            0x1FFFF,
        ],
        [ "non-character code point U+2FFFE",
            (isASCII) ? "\xf0\xaf\xbf\xbe" : I8_to_native("\xf5\xbf\xbf\xbe"),
            0x2FFFE,
        ],
        [ "non-character code point U+2FFFF",
            (isASCII) ? "\xf0\xaf\xbf\xbf" : I8_to_native("\xf5\xbf\xbf\xbf"),
            0x2FFFF,
        ],
        [ "non-character code point U+3FFFE",
            (isASCII) ? "\xf0\xbf\xbf\xbe" : I8_to_native("\xf7\xbf\xbf\xbe"),
            0x3FFFE,
        ],
        [ "non-character code point U+3FFFF",
            (isASCII) ? "\xf0\xbf\xbf\xbf" : I8_to_native("\xf7\xbf\xbf\xbf"),
            0x3FFFF,
        ],
        [ "non-character code point U+4FFFE",
            (isASCII)
            ?               "\xf1\x8f\xbf\xbe"
            : I8_to_native("\xf8\xa9\xbf\xbf\xbe"),
            0x4FFFE,
        ],
        [ "non-character code point U+4FFFF",
            (isASCII)
            ?               "\xf1\x8f\xbf\xbf"
            : I8_to_native("\xf8\xa9\xbf\xbf\xbf"),
            0x4FFFF,
        ],
        [ "non-character code point U+5FFFE",
            (isASCII)
            ?              "\xf1\x9f\xbf\xbe"
            : I8_to_native("\xf8\xab\xbf\xbf\xbe"),
            0x5FFFE,
        ],
        [ "non-character code point U+5FFFF",
            (isASCII)
            ?              "\xf1\x9f\xbf\xbf"
            : I8_to_native("\xf8\xab\xbf\xbf\xbf"),
            0x5FFFF,
        ],
        [ "non-character code point U+6FFFE",
            (isASCII)
            ?              "\xf1\xaf\xbf\xbe"
            : I8_to_native("\xf8\xad\xbf\xbf\xbe"),
            0x6FFFE,
        ],
        [ "non-character code point U+6FFFF",
            (isASCII)
            ?              "\xf1\xaf\xbf\xbf"
            : I8_to_native("\xf8\xad\xbf\xbf\xbf"),
            0x6FFFF,
        ],
        [ "non-character code point U+7FFFE",
            (isASCII)
            ?              "\xf1\xbf\xbf\xbe"
            : I8_to_native("\xf8\xaf\xbf\xbf\xbe"),
            0x7FFFE,
        ],
        [ "non-character code point U+7FFFF",
            (isASCII)
            ?              "\xf1\xbf\xbf\xbf"
            : I8_to_native("\xf8\xaf\xbf\xbf\xbf"),
            0x7FFFF,
        ],
        [ "non-character code point U+8FFFE",
            (isASCII)
            ?              "\xf2\x8f\xbf\xbe"
            : I8_to_native("\xf8\xb1\xbf\xbf\xbe"),
            0x8FFFE,
        ],
        [ "non-character code point U+8FFFF",
            (isASCII)
            ?              "\xf2\x8f\xbf\xbf"
            : I8_to_native("\xf8\xb1\xbf\xbf\xbf"),
            0x8FFFF,
        ],
        [ "non-character code point U+9FFFE",
            (isASCII)
            ?              "\xf2\x9f\xbf\xbe"
            : I8_to_native("\xf8\xb3\xbf\xbf\xbe"),
            0x9FFFE,
        ],
        [ "non-character code point U+9FFFF",
            (isASCII)
            ?              "\xf2\x9f\xbf\xbf"
            : I8_to_native("\xf8\xb3\xbf\xbf\xbf"),
            0x9FFFF,
        ],
        [ "non-character code point U+AFFFE",
            (isASCII)
            ?              "\xf2\xaf\xbf\xbe"
            : I8_to_native("\xf8\xb5\xbf\xbf\xbe"),
            0xAFFFE,
        ],
        [ "non-character code point U+AFFFF",
            (isASCII)
            ?              "\xf2\xaf\xbf\xbf"
            : I8_to_native("\xf8\xb5\xbf\xbf\xbf"),
            0xAFFFF,
        ],
        [ "non-character code point U+BFFFE",
            (isASCII)
            ?              "\xf2\xbf\xbf\xbe"
            : I8_to_native("\xf8\xb7\xbf\xbf\xbe"),
            0xBFFFE,
        ],
        [ "non-character code point U+BFFFF",
            (isASCII)
            ?              "\xf2\xbf\xbf\xbf"
            : I8_to_native("\xf8\xb7\xbf\xbf\xbf"),
            0xBFFFF,
        ],
        [ "non-character code point U+CFFFE",
            (isASCII)
            ?              "\xf3\x8f\xbf\xbe"
            : I8_to_native("\xf8\xb9\xbf\xbf\xbe"),
            0xCFFFE,
        ],
        [ "non-character code point U+CFFFF",
            (isASCII)
            ?              "\xf3\x8f\xbf\xbf"
            : I8_to_native("\xf8\xb9\xbf\xbf\xbf"),
            0xCFFFF,
        ],
        [ "non-character code point U+DFFFE",
            (isASCII)
            ?              "\xf3\x9f\xbf\xbe"
            : I8_to_native("\xf8\xbb\xbf\xbf\xbe"),
            0xDFFFE,
        ],
        [ "non-character code point U+DFFFF",
            (isASCII)
            ?              "\xf3\x9f\xbf\xbf"
            : I8_to_native("\xf8\xbb\xbf\xbf\xbf"),
            0xDFFFF,
        ],
        [ "non-character code point U+EFFFE",
            (isASCII)
            ?              "\xf3\xaf\xbf\xbe"
            : I8_to_native("\xf8\xbd\xbf\xbf\xbe"),
            0xEFFFE,
        ],
        [ "non-character code point U+EFFFF",
            (isASCII)
            ?              "\xf3\xaf\xbf\xbf"
            : I8_to_native("\xf8\xbd\xbf\xbf\xbf"),
            0xEFFFF,
        ],
        [ "non-character code point U+FFFFE",
            (isASCII)
            ?              "\xf3\xbf\xbf\xbe"
            : I8_to_native("\xf8\xbf\xbf\xbf\xbe"),
            0xFFFFE,
        ],
        [ "non-character code point U+FFFFF",
            (isASCII)
            ?              "\xf3\xbf\xbf\xbf"
            : I8_to_native("\xf8\xbf\xbf\xbf\xbf"),
            0xFFFFF,
        ],
        [ "non-character code point U+10FFFE",
            (isASCII)
            ?              "\xf4\x8f\xbf\xbe"
            : I8_to_native("\xf9\xa1\xbf\xbf\xbe"),
            0x10FFFE,
        ],
        [ "non-character code point U+10FFFF",
            (isASCII)
            ?              "\xf4\x8f\xbf\xbf"
            : I8_to_native("\xf9\xa1\xbf\xbf\xbf"),
            0x10FFFF,
        ],
        [ "first non_unicode",
            (isASCII)
            ?              "\xf4\x90\x80\x80"
            : I8_to_native("\xf9\xa2\xa0\xa0\xa0"),
            0x110000,
            2,
        ],
        [ "non_unicode whose first byte tells that",
            (isASCII)
            ?              "\xf5\x80\x80\x80"
            : I8_to_native("\xfa\xa0\xa0\xa0\xa0"),
            (isASCII) ? 0x140000 : 0x200000,
            1,
        ],
        [ "overlong malformation, lowest 5-byte",
            (isASCII)
            ?              "\xf8\x80\x80\x80\x80"
            : I8_to_native("\xf8\xa0\xa0\xa0\xa0"),
            0,   # NUL
        ],
        [ "overlong malformation, highest 5-byte",
            (isASCII)
            ?              "\xf8\x87\xbf\xbf\xbf"
            : I8_to_native("\xf8\xa7\xbf\xbf\xbf"),
            (isASCII) ? 0x1FFFFF : 0x3FFFF,
        ],
        [ "overlong malformation, lowest 6-byte",
            (isASCII)
            ?              "\xfc\x80\x80\x80\x80\x80"
            : I8_to_native("\xfc\xa0\xa0\xa0\xa0\xa0"),
            0,   # NUL
        ],
        [ "overlong malformation, highest 6-byte",
            (isASCII)
            ?              "\xfc\x83\xbf\xbf\xbf\xbf"
            : I8_to_native("\xfc\xa3\xbf\xbf\xbf\xbf"),
            (isASCII) ? 0x3FFFFFF : 0x3FFFFF,
        ],
        [ "overlong malformation, lowest 7-byte",
            (isASCII)
            ?              "\xfe\x80\x80\x80\x80\x80\x80"
            : I8_to_native("\xfe\xa0\xa0\xa0\xa0\xa0\xa0"),
            0,   # NUL
        ],
        [ "overlong malformation, highest 7-byte",
            (isASCII)
            ?              "\xfe\x81\xbf\xbf\xbf\xbf\xbf"
            : I8_to_native("\xfe\xa1\xbf\xbf\xbf\xbf\xbf"),
            (isASCII) ? 0x7FFFFFFF : 0x3FFFFFF,
        ],
        [ "highest 31 bit code point",
            (isASCII)
            ?  "\xfd\xbf\xbf\xbf\xbf\xbf"
            : I8_to_native(
               "\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa1\xbf\xbf\xbf\xbf\xbf\xbf"),
            0x7FFFFFFF,
            1,
        ],
        [ "lowest 32 bit code point",
            (isASCII)
            ?  "\xfe\x82\x80\x80\x80\x80\x80"
            : I8_to_native(
                "\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa2\xa0\xa0\xa0\xa0\xa0\xa0"),
            ($::is64bit) ? 0x80000000 : -1,   # Overflows on 32-bit systems
            1,
        ],
        # Used when UV_MAX is allowed as a code point
        #[ "highest 32 bit code point",
        #    (isASCII)
        #    ?  "\xfe\x83\xbf\xbf\xbf\xbf\xbf"
        #    : I8_to_native(
        #       "\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa3\xbf\xbf\xbf\xbf\xbf\xbf"),
        #    0xFFFFFFFF,
        #],
        #[ "Lowest 33 bit code point",
        #    (isASCII)
        #    ?  "\xfe\x84\x80\x80\x80\x80\x80"
        #    : I8_to_native(
        #        "\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa4\xa0\xa0\xa0\xa0\xa0\xa0"),
        #    ($::is64bit) ? 0x100000000 : 0x0,   # Overflows on 32-bit systems
        #],
    );

    if (! $::is64bit) {
        if (isASCII) {
            push @tests,
                [ "overlong malformation, but naively looks like overflow",
                    "\xff\x80\x80\x80\x80\x80\x80\x81\xbf\xbf\xbf\xbf\xbf",
                    0x7FFFFFFF,
                ],
                # Used when above IV_MAX are allowed.
                #[ "overlong malformation, but naively looks like overflow",
                #    "\xff\x80\x80\x80\x80\x80\x80\x83\xbf\xbf\xbf\xbf\xbf",
                #    0xFFFFFFFF,
                #],
                [ "overflow that old algorithm failed to detect",
                    "\xfe\x86\x80\x80\x80\x80\x80",
                    -1,
                ];
        }
    }

    push @tests,
        [ "overlong malformation, lowest max-byte",
            (isASCII)
             ?      "\xff\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
             : I8_to_native(
                    "\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
            0,   # NUL
        ],
        [ "overlong malformation, highest max-byte",
            (isASCII)    # 2**36-1 on ASCII; 2**30-1 on EBCDIC
             ?      "\xff\x80\x80\x80\x80\x80\x80\xbf\xbf\xbf\xbf\xbf\xbf"
             : I8_to_native(
                    "\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xbf\xbf\xbf\xbf\xbf\xbf"),
            (isASCII) ? (($::is64bit) ? 0xFFFFFFFFF : -1) : 0x3FFFFFFF,
        ];

    if (isASCII) {
        push @tests,
            [ "Lowest code point requiring 13 bytes to represent", # 2**36
                "\xff\x80\x80\x80\x80\x80\x81\x80\x80\x80\x80\x80\x80",
                ($::is64bit) ? 0x1000000000 : -1,    # overflows on 32bit
            ],
    };

    if ($::is64bit) {
        push @tests,
            [ "highest 63 bit code point",
              (isASCII)
              ? "\xff\x80\x87\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf"
              : I8_to_native(
                "\xff\xa7\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf"),
              0x7FFFFFFFFFFFFFFF,
            ],
            [ "first 64 bit code point",
              (isASCII)
              ? "\xff\x80\x88\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
              : I8_to_native(
                "\xff\xa8\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
              -1,
            ];
            # Used when UV_MAX is allowed as a code point
            #[ "highest 64 bit code point",
            #  (isASCII)
            #  ? "\xff\x80\x8f\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf"
            #  : I8_to_native(
            #    "\xff\xaf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf"),
            #  0xFFFFFFFFFFFFFFFF,
            #  (isASCII) ? 1 : 2,
            #],
            #[ "first 65 bit code point",
            #  (isASCII)
            #  ? "\xff\x80\x9f\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80"
            #  : I8_to_native(
            #    "\xff\xb0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
            #  0,
            #];
        if (isASCII) {
            push @tests,
                [ "overflow that old algorithm failed to detect",
                    "\xff\x80\x90\x90\x90\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf",
                    -1,
                ];
        }
        else {
            push @tests,    # These could falsely show wrongly in a naive
                            # implementation
                [ "requires at least 32 bits",
                    I8_to_native(
                    "\xff\xa0\xa0\xa0\xa0\xa0\xa1\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
                    0x800000000,
                ],
                [ "requires at least 32 bits",
                    I8_to_native(
                    "\xff\xa0\xa0\xa0\xa0\xa1\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
                    0x10000000000,
                ],
                [ "requires at least 32 bits",
                    I8_to_native(
                    "\xff\xa0\xa0\xa0\xa1\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
                    0x200000000000,
                ],
                [ "requires at least 32 bits",
                    I8_to_native(
                    "\xff\xa0\xa0\xa1\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
                    0x4000000000000,
                ],
                [ "requires at least 32 bits",
                    I8_to_native(
                    "\xff\xa0\xa1\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
                    0x80000000000000,
                ],
                [ "requires at least 32 bits",
                    I8_to_native(
                    "\xff\xa1\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0\xa0"),
                    0x1000000000000000,
                ];
        }
    }
}

sub flags_to_text($$)
{
    my ($flags, $flags_to_text_ref) = @_;

    # Returns a string containing a mnemonic representation of the bits that
    # are set in the $flags.  These are assumed to be flag bits.  The return
    # looks like "FOO|BAR|BAZ".  The second parameter is a reference to an
    # array that gives the textual representation of all the possible flags.
    # Element 0 is the text for the bit 0 flag; element 1 for bit 1; ....  If
    # no bits at all are set the string "0" is returned;

    my @flag_text;
    my $shift = 0;

    return "0" if $flags == 0;

    while ($flags) {
        #diag sprintf "%x", $flags;
        if ($flags & 1) {
            push @flag_text, $flags_to_text_ref->[$shift];
        }
        $shift++;
        $flags >>= 1;
    }

    return join "|", @flag_text;
}

# Possible flag returns from utf8n_to_uvchr_error().  These should have G_,
# instead of A_, D_, but the prefixes will be used in a later commit, so
# minimize churn by having them here.
my @utf8n_flags_to_text =  ( qw(
        A_EMPTY
        A_CONTINUATION
        A_NON_CONTINUATION
        A_SHORT
        A_LONG
        A_LONG_AND_ITS_VALUE
        PLACEHOLDER
        A_OVERFLOW
        D_SURROGATE
        W_SURROGATE
        D_NONCHAR
        W_NONCHAR
        D_SUPER
        W_SUPER
        D_PERL_EXTENDED
        W_PERL_EXTENDED
        CHECK_ONLY
        NO_CONFIDENCE_IN_CURLEN_
    ) );

sub utf8n_display_call($)
{
    # Converts an eval string that calls test_utf8n_to_uvchr into a more human
    # readable form, and returns it.  Doesn't work if the byte string contains
    # an apostrophe.  The return will look something like:
    #   test_utf8n_to_uvchr_error('$bytes', $length, $flags)
    #diag $_[0];

    $_[0] =~ / ^ ( [^(]* \( ) ' ( [^']*? ) ' ( .+ , \D* ) ( \d+ ) \) $ /x;
    my $text1 = $1;     # Everything before the byte string
    my $bytes = $2;
    my $text2 = $3;     # Includes the length
    my $flags = $4;

    return $text1
         . display_bytes($bytes)
         . $text2
         . flags_to_text($flags, \@utf8n_flags_to_text)
         . ')';
}

my @uvchr_flags_to_text =  ( qw(
        W_SURROGATE
        W_NONCHAR
        W_SUPER
        W_PERL_EXTENDED
        D_SURROGATE
        D_NONCHAR
        D_SUPER
        D_PERL_EXTENDED
) );

sub uvchr_display_call($)
{
    # Converts an eval string that calls test_uvchr_to_utf8 into a more human
    # readable form, and returns it.  The return will look something like:
    #   test_uvchr_to_utf8n_flags($uv, $flags)
    #diag $_[0];


    $_[0] =~ / ^ ( [^(]* \( ) ( \d+ ) , \s* ( \d+ ) \) $ /x;
    my $text = $1;
    my $cp = sprintf "%X", $2;
    my $flags = $3;

    return "${text}0x$cp, " . flags_to_text($flags, \@uvchr_flags_to_text) . ')';
}

sub do_warnings_test(@)
{
    my @expected_warnings = @_;

    # Compares the input expected warnings array with @warnings_gotten,
    # generating a pass for each found, removing it from @warnings_gotten.
    # Any discrepancies generate test failures.  Returns TRUE if no
    # discrepcancies; otherwise FALSE.

    my $succeeded = 1;

    if (@expected_warnings == 0) {
        if (! is(@warnings_gotten, 0, "    Expected and got no warnings")) {
            output_warnings(@warnings_gotten);
            $succeeded = 0;
        }
        return $succeeded;
    }

    # Check that we got all the expected warnings,
    # removing each one found
  WARNING:
    foreach my $expected (@expected_warnings) {
        foreach (my $i = 0; $i < @warnings_gotten; $i++) {
            if ($warnings_gotten[$i] =~ $expected) {
                pass("    Expected and got warning: "
                    . " $warnings_gotten[$i]");
                splice @warnings_gotten, $i, 1;
                next WARNING;
            }
        }
        fail("    Expected a warning that matches "
            . $expected . " but didn't get it");
        $succeeded = 0;
    }

    if (! is(@warnings_gotten, 0, "    Got no unexpected warnings")) {
        output_warnings(@warnings_gotten);
        $succeeded = 0;
    }

    return $succeeded;
}

my $min_cont = $::lowest_continuation;
my $continuation_shift = (isASCII) ? 6 : 5;
my $continuation_mask = (1 << $continuation_shift) - 1;

sub isUTF8_CHAR($$) {   # Uses first principals to determine if this I8 input
                        # is legal.  (Doesn't work if overflows)
    my ($native, $length) = @_;
    my $i8 = native_to_I8($native);

    # Uses first principals to calculate if $i8 is legal

    return 0 if $length <= 0;

    my $first = ord substr($i8, 0, 1);

    # Invariant
    return 1 if $length == 1 && $first < $min_cont;

    return 0 if $first < 0xC0;  # Starts with continuation

    # Calculate the number of leading 1 bits
    my $utf8skip = 0;
    my $bits = $first;
    do {
        $utf8skip++;
        $bits = ($bits << 1) & 0xFF;
    } while ($bits & 0x80);

    return 0 if $utf8skip != $length;

    # Accumulate the $code point.  The remaining bits in the start byte count
    # towards it
    my $cp = $bits >> $utf8skip;

    for my $i (1 .. $length - 1) {
        my $ord = ord substr($i8, $i, 1);

        # Wrong if not a continuation
        return 0 if $ord < $min_cont || $ord >= 0xC0;

        $cp = ($cp << $continuation_shift)
            | ($ord & $continuation_mask);
    }

    # If the calculated value can be expressed in fewer bytes than were passed
    # in, is an illegal overlong.  XXX if 'chr' is not working properly, this
    # may not be right
    my $chr = uni_to_native(chr $cp);
    utf8::upgrade($chr);

    use bytes;
    return 0 if length $chr < $length;

    # Also, its possible on EBCDIC platforms that have more illegal start
    # bytes than ASCII ones (like C3, C4) for something to have the same
    # length but still be overlong.  We make sure the first byte isn't smaller
    # than the first byte of the real representation.
    return 0 if substr($native, 0, 1) lt substr($chr, 0, 1);

    return 1;
}

sub start_mark($) {
    my $len = shift;
    return 0xFF if $len >  7;
    return (0xFF & (0xFE << (7 - $len)));
}

sub start_mask($) {
    my $len = shift;
    return 0 if $len >  7;
    return 0x1F >> ($len - 2);
}

# This test is split into this number of files.
my $num_test_files = $ENV{TEST_JOBS} || 1;
$num_test_files = 10 if $num_test_files > 10;

# We only really need to test utf8n_to_uvchr_msgs() once with this flag.
my $tested_CHECK_ONLY = 0;

my $test_count = -1;

# By setting this environment variable to this particular value, we test
# essentially all combinations of potential UTF-8, so that can get a
# comprehensive test of the decoding routine.  This test assumes the routine
# that does the translation from code point to UTF-8 is working.  An assert
# can be used in the routine to make sure that the dfa is working precisely
# correctly, and any flaws in it aren't being masked by the remainder of the
# function.
if ($::TEST_CHUNK == 0
&& $ENV{PERL_DEBUG_FULL_TEST}
&& $ENV{PERL_DEBUG_FULL_TEST} == 97)
{
    # We construct UTF-8 (I8 on EBCDIC platforms converted later to native)

    my $min_cont_mask = $min_cont | 0xF;
    my @bytes = (   0,  # Placeholder to signify to use an empty string ""
                 0x41,  # We assume that all the invariant characters are
                        # properly in the same class, so this is an exemplar
                        # character
                $min_cont .. 0xFF   # But test every non-invariant individually
                );
    my $mark = $min_cont;
    my $mask = (1 << $continuation_shift) - 1;
    for my $byte1 (@bytes) {
        for my $byte2 (@bytes) {
            last if $byte2 && ! $byte1;      # Don't test empty preceding byte

            last if $byte2 && $byte1 < 0xC0; # No need to test more than a
                                             # single byte unless start byte
                                             # indicates those.

            for my $byte3 (@bytes) {
                last if $byte3 && ! $byte2;
                last if $byte3 && $byte1 < 0xE0;    # Only test 3 bytes for
                                                    # 3-byte start byte

                # If the preceding byte is a start byte, it should fail, and
                # there is no need to test illegal bytes that follow.
                # Instead, limit ourselves to just a few legal bytes that
                # could follow.  This cuts down tremendously on the number of
                # tests executed.
                next if $byte2 >= 0xC0
                     && $byte3 >= $min_cont
                     && ($byte3 & $min_cont_mask) != $min_cont;

                for my $byte4 (@bytes) {
                    last if $byte4 && ! $byte3;
                    last if $byte4 && $byte1 < 0xF0;  # Only test 4 bytes for
                                                      # 4 byte strings

                    # Like for byte 3, we limit things that come after a
                    # mispositioned start-byte to just a few things that
                    # otherwise would be legal
                    next if ($byte2 >= 0xC0 || $byte3 >= 0xC0)
                          && $byte4 >= $min_cont
                          && ($byte4 & $min_cont_mask) != $min_cont;

                    for my $byte5 (@bytes) {
                        last if $byte5 && ! $byte4;
                        last if $byte5 && $byte1 < 0xF8;  # Only test 5 bytes for
                                                          # 5 byte strings

                        # Like for byte 4, we limit things that come after a
                        # mispositioned start-byte to just a few things that
                        # otherwise would be legal
                        next if (   $byte2 >= 0xC0
                                 || $byte3 >= 0xC0
                                 || $byte4 >= 0xC0)
                              && $byte4 >= $min_cont
                              && ($byte4 & $min_cont_mask) != $min_cont;

                        my $string = "";
                        $string .= chr $byte1 if $byte1;
                        $string .= chr $byte2 if $byte2;
                        $string .= chr $byte3 if $byte3;
                        $string .= chr $byte4 if $byte4;
                        $string .= chr $byte5 if $byte5;

                        my $length = length $string;
                        next unless $length;
                        last if $byte1 >= ((isASCII) ? 0xF6 : 0xFA);

                        my $native = I8_to_native($string);
                        my $is_valid = isUTF8_CHAR($native, $length);
                        my $got_valid = test_isUTF8_CHAR($native, $length);
                        my $got_strict
                                    = test_isSTRICT_UTF8_CHAR($native, $length);
                        my $got_C9
                                 = test_isC9_STRICT_UTF8_CHAR($native, $length);
                        my $ret = test_utf8n_to_uvchr_msgs($native, $length,
                                            $::UTF8_WARN_ILLEGAL_INTERCHANGE);
                        my $is_strict = $is_valid;
                        my $is_C9 = $is_valid;

                        if ($is_valid) {

                            # Here, is legal UTF-8.  Verify that it returned
                            # the correct code point, and if so, that it
                            # correctly classifies the result.
                            my $cp = $ret->[0];

                            my $should_be_string;
                            if ($length == 1) {
                                $should_be_string = native_to_I8(chr $cp);
                            }
                            else {

                                # Starting with the code point, use first
                                # principals to find the equivalent I8 string
                                my @bytes;
                                my $uv = ord native_to_uni(chr $cp);
                                for (my $i = $length - 1; $i > 0; $i--) {
                                    $bytes[$i] = chr (($uv & $mask) | $mark);
                                    $uv >>= $continuation_shift;
                                }
                                $bytes[0] = chr ($uv & start_mask($length)
                                            | start_mark($length));
                                $should_be_string = join "", @bytes;
                            }

                            # If the original string and the inverse are the
                            # same, it worked.
                            my $test_name = "utf8n_to_uvchr_msgs("
                                          . display_bytes($native)
                                          . ") yields "
                                          . sprintf ("0x%x", $cp)
                                          . "; does its I8 eq original";
                            if (is($should_be_string, $string, $test_name)) {
                                my $is_surrogate = $cp >= 0xD800
                                                && $cp <= 0xDFFF;
                                my $got_surrogate
                                    = ($ret->[2] & $::UTF8_GOT_SURROGATE) != 0;
                                $is_strict = 0 if $is_surrogate;
                                $is_C9 = 0 if $is_surrogate;

                                my $is_super = $cp > 0x10FFFF;
                                my $got_super
                                        = ($ret->[2] & $::UTF8_GOT_SUPER) != 0;
                                $is_strict = 0 if $is_super;
                                $is_C9 = 0 if $is_super;

                                my $is_nonchar = ! $is_super
                                    && (   ($cp & 0xFFFE) == 0xFFFE
                                        || ($cp >= 0xFDD0 && $cp <= 0xFDEF));
                                my $got_nonchar
                                      = ($ret->[2] & $::UTF8_GOT_NONCHAR) != 0;
                                $is_strict = 0 if $is_nonchar;

                                is($got_surrogate, $is_surrogate,
                                    "    And correctly flagged it as"
                                  . ((! $is_surrogate) ? " not" : "")
                                  . " being a surrogate");
                                is($got_super, $is_super,
                                    "    And correctly flagged it as"
                                  . ((! $is_super) ? " not" : "")
                                  . " being above Unicode");
                                is($got_nonchar, $is_nonchar,
                                    "    And correctly flagged it as"
                                  . ((! $is_nonchar) ? " not" : "")
                                  . " being a non-char");
                            }

                            # This is how we exit the loop normally if things
                            # are working.  The fail-safe code above is used
                            # when they aren't.
                            goto done if $cp > 0x140001;
                        }
                        else {
                            is($ret->[0], 0, "utf8n_to_uvchr_msgs("
                                            . display_bytes($native)
                                            . ") correctly returns error");
                            if (! ($ret->[2] & ($::UTF8_GOT_SHORT
                                               |$::UTF8_GOT_NON_CONTINUATION
                                               |$::UTF8_GOT_LONG)))
                            {
                                is($ret->[2] & ( $::UTF8_GOT_NONCHAR
                                                |$::UTF8_GOT_SURROGATE
                                                |$::UTF8_GOT_SUPER), 0,
                                "    And isn't a surrogate, non-char, nor"
                                . " above Unicode");
                             }
                        }

                        is($got_valid == 0, $is_valid == 0,
                           "    And isUTF8_CHAR() correctly returns "
                         . (($got_valid == 0) ? "0" : "non-zero"));
                        is($got_strict == 0, $is_strict == 0,
                           "    And isSTRICT_UTF8_CHAR() correctly returns "
                         . (($got_strict == 0) ? "0" : "non-zero"));
                        is($got_C9 == 0, $is_C9 == 0,
                           "    And isC9_UTF8_CHAR() correctly returns "
                         . (($got_C9 == 0) ? "0" : "non-zero"));
                    }
                }
            }
        }
    }
  done:
}

foreach my $test (@tests) {
  $test_count++;
  next if $test_count % $num_test_files != $::TEST_CHUNK;

  my ($testname, $bytes, $allowed_uv, $needed_to_discern_len) = @$test;

  my $length = length $bytes;
  my $initially_overlong = $testname =~ /overlong/;
  my $initially_orphan   = $testname =~ /orphan/;
  my $will_overflow = $allowed_uv < 0;

  my $uv_string = sprintf(($allowed_uv < 0x100) ? "%02X" : "%04X", $allowed_uv);
  my $display_bytes = display_bytes($bytes);

  my $controlling_warning_category;
  my $utf8n_flag_to_warn;
  my $utf8n_flag_to_disallow;
  my $uvchr_flag_to_warn;
  my $uvchr_flag_to_disallow;

  # We want to test that the independent flags are actually independent.
  # For example, that a surrogate doesn't trigger a non-character warning,
  # and conversely, turning off an above-Unicode flag doesn't suppress a
  # surrogate warning.  Earlier versions of this file used nested loops to
  # test all possible combinations.  But that creates lots of tests, making
  # this run too long.  What is now done instead is to use the complement of
  # the category we are testing to greatly reduce the combinatorial
  # explosion.  For example, if we have a surrogate and we aren't expecting
  # a warning about it, we set all the flags for non-surrogates to raise
  # warnings.  If one shows up, it indicates the flags aren't independent.
  my $utf8n_flag_to_warn_complement;
  my $utf8n_flag_to_disallow_complement;
  my $uvchr_flag_to_warn_complement;
  my $uvchr_flag_to_disallow_complement;

  # Many of the code points being tested are middling in that if code point
  # edge cases work, these are very likely to as well.  Because this test
  # file takes a while to execute, we skip testing the edge effects of code
  # points deemed middling, while testing their basics and continuing to
  # fully test the non-middling code points.
  my $skip_most_tests = 0;

  my $cp_message_qr;      # Pattern that matches the message raised when
                          # that message contains the problematic code
                          # point.  The message is the same (currently) both
                          # when going from/to utf8.
  my $non_cp_trailing_text;   # The suffix text when the message doesn't
                              # contain a code point.  (This is a result of
                              # some sort of malformation that means we
                              # can't get an exact code poin
  my $extended_cp_message_qr = qr/\QCode point 0x$uv_string is not Unicode,\E
                      \Q requires a Perl extension, and so is not\E
                      \Q portable\E/x;
  my $extended_non_cp_trailing_text
                      = "is a Perl extension, and so is not portable";

  # What bytes should have been used to specify a code point that has been
  # specified as an overlong.
  my $correct_bytes_for_overlong;

  # Is this test malformed from the beginning?  If so, we know to generally
  # expect that the tests will show it isn't valid.
  my $initially_malformed = 0;

  if ($initially_overlong || $initially_orphan) {
      $non_cp_trailing_text = "if you see this, there is an error";
      $cp_message_qr = qr/\Q$non_cp_trailing_text\E/;
      $initially_malformed = 1;
      $utf8n_flag_to_warn     = 0;
      $utf8n_flag_to_disallow = 0;

      $utf8n_flag_to_warn_complement =     $::UTF8_WARN_SURROGATE;
      $utf8n_flag_to_disallow_complement = $::UTF8_DISALLOW_SURROGATE;
      if (! $will_overflow && $allowed_uv <= 0x10FFFF) {
          $utf8n_flag_to_warn_complement     |= $::UTF8_WARN_SUPER;
          $utf8n_flag_to_disallow_complement |= $::UTF8_DISALLOW_SUPER;
          if (($allowed_uv & 0xFFFF) != 0xFFFF) {
              $utf8n_flag_to_warn_complement      |= $::UTF8_WARN_NONCHAR;
              $utf8n_flag_to_disallow_complement  |= $::UTF8_DISALLOW_NONCHAR;
          }
      }
      if (! is_extended_utf8($bytes)) {
          $utf8n_flag_to_warn_complement |= $::UTF8_WARN_PERL_EXTENDED;
          $utf8n_flag_to_disallow_complement  |= $::UTF8_DISALLOW_PERL_EXTENDED;
      }

      $controlling_warning_category = 'utf8';

      if ($initially_overlong) {
          if (! defined $needed_to_discern_len) {
              $needed_to_discern_len = overlong_discern_len($bytes);
          }
          $correct_bytes_for_overlong = display_bytes_no_quotes(chr $allowed_uv);
      }
  }
  elsif($will_overflow || $allowed_uv > 0x10FFFF) {

      # Set the SUPER flags; later, we test for PERL_EXTENDED as well.
      $utf8n_flag_to_warn     = $::UTF8_WARN_SUPER;
      $utf8n_flag_to_disallow = $::UTF8_DISALLOW_SUPER;
      $uvchr_flag_to_warn     = $::UNICODE_WARN_SUPER;
      $uvchr_flag_to_disallow = $::UNICODE_DISALLOW_SUPER;;

      # Below, we add the flags for non-perl_extended to the code points
      # that don't fit that category.  Special tests are done for this
      # category in the inner loop.
      $utf8n_flag_to_warn_complement     = $::UTF8_WARN_NONCHAR
                                          |$::UTF8_WARN_SURROGATE;
      $utf8n_flag_to_disallow_complement = $::UTF8_DISALLOW_NONCHAR
                                          |$::UTF8_DISALLOW_SURROGATE;
      $uvchr_flag_to_warn_complement     = $::UNICODE_WARN_NONCHAR
                                          |$::UNICODE_WARN_SURROGATE;
      $uvchr_flag_to_disallow_complement = $::UNICODE_DISALLOW_NONCHAR
                                          |$::UNICODE_DISALLOW_SURROGATE;
      $controlling_warning_category = 'non_unicode';

      if ($will_overflow) {  # This is realy a malformation
          $non_cp_trailing_text = "if you see this, there is an error";
          $cp_message_qr = qr/\Q$non_cp_trailing_text\E/;
          $initially_malformed = 1;
          if (! defined $needed_to_discern_len) {
              $needed_to_discern_len = overflow_discern_len($length);
          }
      }
      elsif (requires_extended_utf8($allowed_uv)) {
          $cp_message_qr = $extended_cp_message_qr;
          $non_cp_trailing_text = $extended_non_cp_trailing_text;
          $needed_to_discern_len = 1 unless defined $needed_to_discern_len;
      }
      else {
          $cp_message_qr = qr/\QCode point 0x$uv_string is not Unicode,\E
                              \Q may not be portable\E/x;
          $non_cp_trailing_text = "is for a non-Unicode code point, may not"
                              . " be portable";
          $utf8n_flag_to_warn_complement     |= $::UTF8_WARN_PERL_EXTENDED;
          $utf8n_flag_to_disallow_complement
                                          |= $::UTF8_DISALLOW_PERL_EXTENDED;
          $uvchr_flag_to_warn_complement |= $::UNICODE_WARN_PERL_EXTENDED;
          $uvchr_flag_to_disallow_complement
                                      |= $::UNICODE_DISALLOW_PERL_EXTENDED;
      }
  }
  elsif ($allowed_uv >= 0xD800 && $allowed_uv <= 0xDFFF) {
      $cp_message_qr = qr/UTF-16 surrogate U\+$uv_string/;
      $non_cp_trailing_text = "is for a surrogate";
      $needed_to_discern_len = 2 unless defined $needed_to_discern_len;
      $skip_most_tests = 1 if $allowed_uv > 0xD800 && $allowed_uv < 0xDFFF;

      $utf8n_flag_to_warn     = $::UTF8_WARN_SURROGATE;
      $utf8n_flag_to_disallow = $::UTF8_DISALLOW_SURROGATE;
      $uvchr_flag_to_warn     = $::UNICODE_WARN_SURROGATE;
      $uvchr_flag_to_disallow = $::UNICODE_DISALLOW_SURROGATE;;

      $utf8n_flag_to_warn_complement     = $::UTF8_WARN_NONCHAR
                                          |$::UTF8_WARN_SUPER
                                          |$::UTF8_WARN_PERL_EXTENDED;
      $utf8n_flag_to_disallow_complement = $::UTF8_DISALLOW_NONCHAR
                                          |$::UTF8_DISALLOW_SUPER
                                          |$::UTF8_DISALLOW_PERL_EXTENDED;
      $uvchr_flag_to_warn_complement     = $::UNICODE_WARN_NONCHAR
                                          |$::UNICODE_WARN_SUPER
                                          |$::UNICODE_WARN_PERL_EXTENDED;
      $uvchr_flag_to_disallow_complement = $::UNICODE_DISALLOW_NONCHAR
                                          |$::UNICODE_DISALLOW_SUPER
                                          |$::UNICODE_DISALLOW_PERL_EXTENDED;
      $controlling_warning_category = 'surrogate';
  }
  elsif (   ($allowed_uv >= 0xFDD0 && $allowed_uv <= 0xFDEF)
          || ($allowed_uv & 0xFFFE) == 0xFFFE)
  {
      $cp_message_qr = qr/\QUnicode non-character U+$uv_string\E
                          \Q is not recommended for open interchange\E/x;
      $non_cp_trailing_text = "if you see this, there is an error";
      $needed_to_discern_len = $length unless defined $needed_to_discern_len;
      if (   ($allowed_uv > 0xFDD0 && $allowed_uv < 0xFDEF)
          || ($allowed_uv > 0xFFFF && $allowed_uv < 0x10FFFE))
      {
          $skip_most_tests = 1;
      }

      $utf8n_flag_to_warn     = $::UTF8_WARN_NONCHAR;
      $utf8n_flag_to_disallow = $::UTF8_DISALLOW_NONCHAR;
      $uvchr_flag_to_warn     = $::UNICODE_WARN_NONCHAR;
      $uvchr_flag_to_disallow = $::UNICODE_DISALLOW_NONCHAR;;

      $utf8n_flag_to_warn_complement     = $::UTF8_WARN_SURROGATE
                                          |$::UTF8_WARN_SUPER
                                          |$::UTF8_WARN_PERL_EXTENDED;
      $utf8n_flag_to_disallow_complement = $::UTF8_DISALLOW_SURROGATE
                                          |$::UTF8_DISALLOW_SUPER
                                          |$::UTF8_DISALLOW_PERL_EXTENDED;
      $uvchr_flag_to_warn_complement     = $::UNICODE_WARN_SURROGATE
                                          |$::UNICODE_WARN_SUPER
                                          |$::UNICODE_WARN_PERL_EXTENDED;
      $uvchr_flag_to_disallow_complement = $::UNICODE_DISALLOW_SURROGATE
                                          |$::UNICODE_DISALLOW_SUPER
                                          |$::UNICODE_DISALLOW_PERL_EXTENDED;

      $controlling_warning_category = 'nonchar';
  }
  else {
      die "Can't figure out what type of warning to test for $testname"
  }

  die 'Didn\'t set $needed_to_discern_len for ' . $testname
                                      unless defined $needed_to_discern_len;

  # We try various combinations of malformations that can occur
  foreach my $short (0, 1) {
    next if $skip_most_tests && $short;
    foreach my $unexpected_noncont (0, 1) {
      next if $skip_most_tests && $unexpected_noncont;
      foreach my $overlong (0, 1) {
        next if $overlong && $skip_most_tests;
        next if $initially_overlong && ! $overlong;

        # If we're creating an overlong, it can't be longer than the
        # maximum length, so skip if we're already at that length.
        next if   (! $initially_overlong && $overlong)
                  &&  $length >= $::max_bytes;

        my $this_cp_message_qr = $cp_message_qr;
        my $this_non_cp_trailing_text = $non_cp_trailing_text;

        foreach my $malformed_allow_type (0..2) {
          # 0 don't allow this malformation; ignored if no malformation
          # 1 allow, with REPLACEMENT CHARACTER returned
          # 2 allow, with intended code point returned.  All malformations
          #   other than overlong can't determine the intended code point,
          #   so this isn't valid for them.
          next if     $malformed_allow_type == 2
                  && ($will_overflow || $short || $unexpected_noncont);
          next if $skip_most_tests && $malformed_allow_type;

          # Here we are in the innermost loop for malformations.  So we
          # know which ones are in effect.  Can now change the input to be
          # appropriately malformed.  We also can set up certain other
          # things now, like whether we expect a return flag from this
          # malformation, and which flag.

          my $this_bytes = $bytes;
          my $this_length = $length;
          my $this_expected_len = $length;
          my $this_needed_to_discern_len = $needed_to_discern_len;

          my @malformation_names;
          my @expected_malformation_warnings;
          my @expected_malformation_return_flags;

          # Contains the flags for any allowed malformations.  Currently no
          # combinations of on/off are tested for.  It's either all are
          # allowed, or none are.
          my $allow_flags = 0;
          my $overlong_is_in_perl_extended_utf8 = 0;
          my $dont_use_overlong_cp = 0;

          if ($initially_orphan) {
              next if $overlong || $short || $unexpected_noncont;
          }

          if ($overlong) {
              if (! $initially_overlong) {
                  my $new_expected_len;

                  # To force this malformation, we convert the original start
                  # byte into a continuation byte with the same data bits as
                  # originally. ...
                  my $start_byte = substr($this_bytes, 0, 1);
                  my $converted_to_continuation_byte
                                          = start_byte_to_cont($start_byte);

                  # ... Then we prepend it with a known overlong sequence.
                  # This should evaluate to the exact same code point as the
                  # original.  We try to avoid an overlong using Perl
                  # extended UTF-8.  The code points are the highest
                  # representable as overlongs on the respective platform
                  # without using extended UTF-8.
                  if (native_to_I8($start_byte) lt "\xFC") {
                      $start_byte = I8_to_native("\xFC");
                      $new_expected_len = 6;
                  }
                  elsif (! isASCII && native_to_I8($start_byte) lt "\xFE") {

                      # FE is not extended UTF-8 on EBCDIC
                      $start_byte = I8_to_native("\xFE");
                      $new_expected_len = 7;
                  }
                  else {  # Must use extended UTF-8.  On ASCII platforms, we
                          # could express some overlongs here starting with
                          # \xFE, but there's no real reason to do so.
                      $overlong_is_in_perl_extended_utf8 = 1;
                      $start_byte = I8_to_native("\xFF");
                      $new_expected_len = $::max_bytes;
                      $this_cp_message_qr = $extended_cp_message_qr;

                      # The warning that gets raised doesn't include the
                      # code point in the message if the code point can be
                      # expressed without using extended UTF-8, but the
                      # particular overlong sequence used is in extended
                      # UTF-8.  To do otherwise would be confusing to the
                      # user, as it would claim the code point requires
                      # extended, when it doesn't.
                      $dont_use_overlong_cp = 1
                                  unless requires_extended_utf8($allowed_uv);
                      $this_non_cp_trailing_text
                                            = $extended_non_cp_trailing_text;
                  }

                  # Splice in the revise continuation byte, preceded by the
                  # start byte and the proper number of the lowest
                  # continuation bytes.
                  $this_bytes =   $start_byte
                              . ($native_lowest_continuation_chr
                                  x (  $new_expected_len
                                      - 1
                                      - length($this_bytes)))
                              .  $converted_to_continuation_byte
                              .  substr($this_bytes, 1);
                  $this_length = length($this_bytes);
                  $this_needed_to_discern_len =    $new_expected_len
                                              - (  $this_expected_len
                                              - $this_needed_to_discern_len);
                  $this_expected_len = $new_expected_len;
              }
          }

          if ($short) {

              # To force this malformation, just tell the test to not look
              # as far as it should into the input.
              $this_length--;
              $this_expected_len--;

              $allow_flags |= $::UTF8_ALLOW_SHORT if $malformed_allow_type;
          }

          if ($unexpected_noncont) {

              # To force this malformation, change the final continuation
              # byte into a start byte.
              my $pos = ($short) ? -2 : -1;
              substr($this_bytes, $pos, 1) = $known_start_byte;
              $this_expected_len--;
          }

          # The whole point of a test that is malformed from the beginning
          # is to test for that malformation.  If we've modified things so
          # much that we don't have enough information to detect that
          # malformation, there's no point in testing.
          next if    $initially_malformed
                  && $this_expected_len < $this_needed_to_discern_len;

          # Here, we've transformed the input with all of the desired
          # non-overflow malformations.  We are now in a position to
          # construct any potential warnings for those malformations.  But
          # it's a pain to get the detailed messages exactly right, so for
          # now XXX, only do so for those that return an explicit code
          # point.

          if ($initially_orphan) {
              push @malformation_names, "orphan continuation";
              push @expected_malformation_return_flags,
                                                  $::UTF8_GOT_CONTINUATION;
              $allow_flags |= $::UTF8_ALLOW_CONTINUATION
                                                  if $malformed_allow_type;
              push @expected_malformation_warnings, qr/unexpected continuation/;
          }

          if ($overlong) {
              push @malformation_names, 'overlong';
              push @expected_malformation_return_flags, $::UTF8_GOT_LONG;

              # If one of the other malformation types is also in effect, we
              # don't know what the intended code point was.
              if ($short || $unexpected_noncont || $will_overflow) {
                  push @expected_malformation_warnings, qr/overlong/;
              }
              else {
                  my $wrong_bytes = display_bytes_no_quotes(
                                        substr($this_bytes, 0, $this_length));
                  if (! defined $correct_bytes_for_overlong) {
                      $correct_bytes_for_overlong
                                          = display_bytes_no_quotes($bytes);
                  }
                  my $prefix = (   $allowed_uv > 0x10FFFF
                                || ! isASCII && $allowed_uv < 256)
                                ? "0x"
                                : "U+";
                  push @expected_malformation_warnings,
                          qr/\QMalformed UTF-8 character: $wrong_bytes\E
                              \Q (overlong; instead use\E
                              \Q $correct_bytes_for_overlong to\E
                              \Q represent $prefix$uv_string)/x;
              }

              if ($malformed_allow_type == 2) {
                  $allow_flags |= $::UTF8_ALLOW_LONG_AND_ITS_VALUE;
              }
              elsif ($malformed_allow_type) {
                  $allow_flags |= $::UTF8_ALLOW_LONG;
              }
          }
          if ($short) {
              push @malformation_names, 'short';
              push @expected_malformation_return_flags, $::UTF8_GOT_SHORT;
              push @expected_malformation_warnings, qr/too short/;
          }
          if ($unexpected_noncont) {
              push @malformation_names, 'unexpected non-continuation';
              push @expected_malformation_return_flags,
                              $::UTF8_GOT_NON_CONTINUATION;
              $allow_flags |= $::UTF8_ALLOW_NON_CONTINUATION
                                                  if $malformed_allow_type;
              push @expected_malformation_warnings,
                                      qr/unexpected non-continuation byte/;
          }

          # The overflow malformation is done differently than other
          # malformations.  It comes from manually typed tests in the test
          # array.  We now make it be treated like one of the other
          # malformations.  But some has to be deferred until the inner loop
          my $overflow_msg_pattern;
          if ($will_overflow) {
              push @malformation_names, 'overflow';

              $overflow_msg_pattern = display_bytes_no_quotes(
                                  substr($this_bytes, 0, $this_expected_len));
              $overflow_msg_pattern = qr/\QMalformed UTF-8 character:\E
                                          \Q $overflow_msg_pattern\E
                                          \Q (overflows)\E/x;
              push @expected_malformation_return_flags, $::UTF8_GOT_OVERFLOW;
              $allow_flags |= $::UTF8_ALLOW_OVERFLOW if $malformed_allow_type;
          }

          # And we can create the malformation-related text for the test
          # names we eventually will generate.
          my $malformations_name = "";
          if (@malformation_names) {
              $malformations_name .= "dis" unless $malformed_allow_type;
              $malformations_name .= "allowed ";
              $malformations_name .= "malformation";
              $malformations_name .= "s" if @malformation_names > 1;
              $malformations_name .= ": ";
              $malformations_name .=  join "/", @malformation_names;
              $malformations_name =  " ($malformations_name)";
          }

          # Done setting up the malformation related stuff

          {   # First test the isFOO calls
              use warnings; # XXX no warnings 'deprecated';   # Make sure these don't raise warnings
              undef @warnings_gotten;

              my $ret = test_isUTF8_CHAR($this_bytes, $this_length);
              my $ret_flags
                      = test_isUTF8_CHAR_flags($this_bytes, $this_length, 0);
              if ($malformations_name) {
                  is($ret, 0, "For $testname$malformations_name: isUTF8_CHAR() returns 0");
                  is($ret_flags, 0, "    And isUTF8_CHAR_flags() returns 0");
              }
              else {
                  is($ret, $this_length, "For $testname: isUTF8_CHAR() returns"
                                        . " expected length: $this_length");
                  is($ret_flags, $this_length,
                      "    And isUTF8_CHAR_flags(...,0) returns expected"
                    . " length: $this_length");
              }
              is(scalar @warnings_gotten, 0,
                  "    And neither isUTF8_CHAR() nor isUTF8_CHAR()_flags"
                . " generated any warnings")
              or output_warnings(@warnings_gotten);

              undef @warnings_gotten;
              $ret = test_isSTRICT_UTF8_CHAR($this_bytes, $this_length);
              if ($malformations_name) {
                  is($ret, 0, "    And isSTRICT_UTF8_CHAR() returns 0");
              }
              else {
                  my $expected_ret
                              = (   $testname =~ /surrogate|non-character/
                                  || $allowed_uv > 0x10FFFF)
                                ? 0
                                : $this_length;
                  is($ret, $expected_ret,
                      "    And isSTRICT_UTF8_CHAR() returns expected"
                    . " length: $expected_ret");
                  $ret = test_isUTF8_CHAR_flags($this_bytes, $this_length,
                                      $::UTF8_DISALLOW_ILLEGAL_INTERCHANGE);
                  is($ret, $expected_ret,
                      "    And isUTF8_CHAR_flags('"
                    . "DISALLOW_ILLEGAL_INTERCHANGE') acts like"
                    . " isSTRICT_UTF8_CHAR");
              }
              is(scalar @warnings_gotten, 0,
                      "    And neither isSTRICT_UTF8_CHAR() nor"
                    . " isUTF8_CHAR_flags generated any warnings")
              or output_warnings(@warnings_gotten);

              undef @warnings_gotten;
              $ret = test_isC9_STRICT_UTF8_CHAR($this_bytes, $this_length);
              if ($malformations_name) {
                  is($ret, 0, "    And isC9_STRICT_UTF8_CHAR() returns 0");
              }
              else {
                  my $expected_ret = (   $testname =~ /surrogate/
                                      || $allowed_uv > 0x10FFFF)
                                      ? 0
                                      : $this_expected_len;
                  is($ret, $expected_ret, "    And isC9_STRICT_UTF8_CHAR()"
                                        . " returns expected length:"
                                        . " $expected_ret");
                  $ret = test_isUTF8_CHAR_flags($this_bytes, $this_length,
                                  $::UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE);
                  is($ret, $expected_ret,
                      "    And isUTF8_CHAR_flags('"
                    . "DISALLOW_ILLEGAL_C9_INTERCHANGE') acts like"
                    . " isC9_STRICT_UTF8_CHAR");
              }
              is(scalar @warnings_gotten, 0,
                      "    And neither isC9_STRICT_UTF8_CHAR() nor"
                    . " isUTF8_CHAR_flags generated any warnings")
              or output_warnings(@warnings_gotten);

              foreach my $disallow_type (0..2) {
                  # 0 is don't disallow this type of code point
                  # 1 is do disallow
                  # 2 is do disallow, but only code points requiring
                  #   perl-extended-UTF8

                  my $disallow_flags;
                  my $expected_ret;

                  if ($malformations_name) {

                      # Malformations are by default disallowed, so testing
                      # with $disallow_type equal to 0 is sufficicient.
                      next if $disallow_type;

                      $disallow_flags = 0;
                      $expected_ret = 0;
                  }
                  elsif ($disallow_type == 1) {
                      $disallow_flags = $utf8n_flag_to_disallow;
                      $expected_ret = 0;
                  }
                  elsif ($disallow_type == 2) {
                      next if ! requires_extended_utf8($allowed_uv);
                      $disallow_flags = $::UTF8_DISALLOW_PERL_EXTENDED;
                      $expected_ret = 0;
                  }
                  else {  # type is 0
                      $disallow_flags = $utf8n_flag_to_disallow_complement;
                      $expected_ret = $this_length;
                  }

                  $ret = test_isUTF8_CHAR_flags($this_bytes, $this_length,
                                                $disallow_flags);
                  is($ret, $expected_ret,
                            "    And isUTF8_CHAR_flags($display_bytes,"
                          . " $disallow_flags) returns $expected_ret")
                    or diag "The flags mean "
                          . flags_to_text($disallow_flags,
                                          \@utf8n_flags_to_text);
                  is(scalar @warnings_gotten, 0,
                          "    And isUTF8_CHAR_flags(...) generated"
                        . " no warnings")
                    or output_warnings(@warnings_gotten);

                  # Test partial character handling, for each byte not a
                  # full character
                  my $did_test_partial = 0;
                  for (my $j = 1; $j < $this_length - 1; $j++) {
                      $did_test_partial = 1;
                      my $partial = substr($this_bytes, 0, $j);
                      my $ret_should_be;
                      my $comment;
                      if ($disallow_type || $malformations_name) {
                          $ret_should_be = 0;
                          $comment = "disallowed";

                          # The number of bytes required to tell if a
                          # sequence has something wrong is the smallest of
                          # all the things wrong with it.  We start with the
                          # number for this type of code point, if that is
                          # disallowed; or the whole length if not.  The
                          # latter is what a couple of the malformations
                          # require.
                          my $needed_to_tell = ($disallow_type)
                                                ? $this_needed_to_discern_len
                                                : $this_expected_len;

                          # Then we see if the malformations that are
                          # detectable early in the string are present.
                          if ($overlong) {
                              my $dl = overlong_discern_len($this_bytes);
                              $needed_to_tell = $dl if $dl < $needed_to_tell;
                          }
                          if ($will_overflow) {
                              my $dl = overflow_discern_len($length);
                              $needed_to_tell = $dl if $dl < $needed_to_tell;
                          }

                          if ($j < $needed_to_tell) {
                              $ret_should_be = 1;
                              $comment .= ", but need $needed_to_tell"
                                        . " bytes to discern:";
                          }
                      }
                      else {
                          $ret_should_be = 1;
                          $comment = "allowed";
                      }

                      undef @warnings_gotten;

                      $ret = test_is_utf8_valid_partial_char_flags($partial,
                                                      $j, $disallow_flags);
                      is($ret, $ret_should_be,
                          "    And is_utf8_valid_partial_char_flags("
                          . display_bytes($partial)
                          . ", $disallow_flags), $comment: returns"
                          . " $ret_should_be")
                      or diag "The flags mean "
                      . flags_to_text($disallow_flags, \@utf8n_flags_to_text);
                  }

                  if ($did_test_partial) {
                      is(scalar @warnings_gotten, 0,
                          "    And is_utf8_valid_partial_char_flags()"
                          . " generated no warnings for any of the lengths")
                        or output_warnings(@warnings_gotten);
                  }
              }
          }

          # Now test the to/from UTF-8 calls.  There are several orthogonal
          # variables involved.  We test most possible combinations

          foreach my $do_disallow (0, 1) {
            if ($do_disallow) {
              next if $initially_overlong || $initially_orphan;
            }
            else {
              next if $skip_most_tests;
            }

            # This tests four functions: utf8n_to_uvchr_error,
            # utf8n_to_uvchr_msgs, uvchr_to_utf8_flags, and
            # uvchr_to_utf8_msgs.  The first two are variants of each other,
            # and the final two also form a pair.  We use a loop 'which_func'
            # to determine which of each pair is being tested.  The main loop
            # tests either the first and third, or the 2nd and fourth.
            # which_func is sets whether we are expecting warnings or not in
            # certain places.  The _msgs() version of the functions expects
            # warnings even if lexical ones are turned off, so by making its
            # which_func == 1, we can say we want warnings; whereas the other
            # one with the value 0, doesn't get them.
            for my $which_func (0, 1) {
              my $utf8_func = ($which_func)
                          ? 'utf8n_to_uvchr_msgs'
                          : 'utf8n_to_uvchr_error';

              # We classify the warnings into certain "interesting" types,
              # described later
              foreach my $warning_type (0..5) {
                next if $skip_most_tests && $warning_type != 1;
                foreach my $use_warn_flag (0, 1) {
                    if ($use_warn_flag) {
                        next if $initially_overlong || $initially_orphan;

                        # Since foo_msgs() expects warnings even when lexical
                        # ones are turned off, we can skip testing it when
                        # they are turned on, with little likelihood of
                        # missing an error case.
                        next if $which_func;
                    }
                    else {
                        next if $skip_most_tests;
                    }

                    # Finally, here is the inner loop

                    my $this_utf8n_flag_to_warn = $utf8n_flag_to_warn;
                    my $this_utf8n_flag_to_disallow = $utf8n_flag_to_disallow;
                    my $this_uvchr_flag_to_warn = $uvchr_flag_to_warn;
                    my $this_uvchr_flag_to_disallow = $uvchr_flag_to_disallow;

                    my $eval_warn;
                    my $expect_regular_warnings;
                    my $expect_warnings_for_malformed;
                    my $expect_warnings_for_overflow;

                    if ($warning_type == 0) {
                        $eval_warn = "use warnings";
                        $expect_regular_warnings = $use_warn_flag;

                        # We ordinarily expect overflow warnings here.  But it
                        # is somewhat more complicated, and the final
                        # determination is deferred to one place in the file
                        # where we handle overflow.
                        $expect_warnings_for_overflow = 1;

                        # We would ordinarily expect malformed warnings in
                        # this case, but not if malformations are allowed.
                        $expect_warnings_for_malformed
                                                = $malformed_allow_type == 0;
                    }
                    elsif ($warning_type == 1) {
                        $eval_warn = "no warnings";
                        $expect_regular_warnings = $which_func;
                        $expect_warnings_for_overflow = $which_func;
                        $expect_warnings_for_malformed = $which_func;
                    }
                    elsif ($warning_type == 2) {
                        $eval_warn = "no warnings; use warnings 'utf8'";
                        $expect_regular_warnings = $use_warn_flag;
                        $expect_warnings_for_overflow = 1;
                        $expect_warnings_for_malformed
                                                = $malformed_allow_type == 0;
                    }
                    elsif ($warning_type == 3) {
                        $eval_warn = "no warnings; use warnings"
                                   . " '$controlling_warning_category'";
                        $expect_regular_warnings = $use_warn_flag;
                        $expect_warnings_for_overflow
                            = $controlling_warning_category eq 'non_unicode';
                        $expect_warnings_for_malformed = $which_func;
                    }
                    elsif ($warning_type =~ /^[45]$/) {
                        # Like type 3, but uses the PERL_EXTENDED flags, and 5
                        # uses PORTABLE warnings;
                        # The complement flags were set up so that the
                        # PERL_EXTENDED flags have been tested that they don't
                        # trigger wrongly for too small code points.  And the
                        # flags have been set up so that those small code
                        # points are tested for being above Unicode.  What's
                        # left to test is that the large code points do
                        # trigger the PERL_EXTENDED flags.
                        next if ! requires_extended_utf8($allowed_uv);
                        next if $controlling_warning_category ne 'non_unicode';
                        $eval_warn = "no warnings;";
                        if ($warning_type == 4) {
                            $eval_warn .= " use warnings 'non_unicode'";
                        }
                        else {
                            $eval_warn .= " use warnings 'portable'";
                        }
                        $expect_regular_warnings = 1;
                        $expect_warnings_for_overflow = 1;
                        $expect_warnings_for_malformed = 0;
                        $this_utf8n_flag_to_warn = $::UTF8_WARN_PERL_EXTENDED;
                        $this_utf8n_flag_to_disallow
                                             = $::UTF8_DISALLOW_PERL_EXTENDED;
                        $this_uvchr_flag_to_warn
                                              = $::UNICODE_WARN_PERL_EXTENDED;
                        $this_uvchr_flag_to_disallow
                                          = $::UNICODE_DISALLOW_PERL_EXTENDED;
                    }
                    else {
                       die "Unexpected warning type '$warning_type'";
                    }

                    # We only need to test the case where all warnings are
                    # enabled (type 0) to see if turning off the warning flag
                    # causes things to not be output.  If those pass, then
                    # turning on some sub-category of warnings, or turning off
                    # warnings altogether are extremely likely to not output
                    # warnings either, given how the warnings subsystem is
                    # supposed to work, and this file assumes it does work.
                    next if $warning_type != 0 && ! $use_warn_flag;

                    # The convention is that the 'got' flag is the same value
                    # as the disallow one.  If this were violated, the tests
                    # here should start failing.
                    my $return_flag = $this_utf8n_flag_to_disallow;

                    # If we aren't expecting warnings/disallow for this, turn
                    # on all the other flags.  That makes sure that they all
                    # are independent of this flag, and so we don't need to
                    # test them individually.
                    my $this_warning_flags
                            = ($use_warn_flag)
                              ? $this_utf8n_flag_to_warn
                              : ($overlong_is_in_perl_extended_utf8
                                ? ($utf8n_flag_to_warn_complement
                                    & ~$::UTF8_WARN_PERL_EXTENDED)
                                :  $utf8n_flag_to_warn_complement);
                    my $this_disallow_flags
                            = ($do_disallow)
                              ? $this_utf8n_flag_to_disallow
                              : ($overlong_is_in_perl_extended_utf8
                                 ? ($utf8n_flag_to_disallow_complement
                                    & ~$::UTF8_DISALLOW_PERL_EXTENDED)
                                 :  $utf8n_flag_to_disallow_complement);
                    my $expected_uv = $allowed_uv;
                    my $this_uv_string = $uv_string;

                    my @expected_return_flags
                                        = @expected_malformation_return_flags;
                    my @expected_warnings;
                    push @expected_warnings, @expected_malformation_warnings
                                            if $expect_warnings_for_malformed;

                    # The overflow malformation is done differently than other
                    # malformations.  It comes from manually typed tests in
                    # the test array, but it also is above Unicode and uses
                    # Perl extended UTF-8, so affects some of the flags being
                    # tested.  We now make it be treated like one of the other
                    # generated malformations.
                    if ($will_overflow) {

                        # An overflow is (way) above Unicode, and overrides
                        # everything else.
                        $expect_regular_warnings = 0;

                        # Earlier, we tentatively calculated whether this
                        # should emit a message or not.  It's tentative
                        # because, even if we ordinarily would output it, we
                        # don't if malformations are allowed -- except an
                        # overflow is also a SUPER and PERL_EXTENDED, and if
                        # warnings for those are enabled, the overflow
                        # warning does get raised.
                        if (   $expect_warnings_for_overflow
                            && (    $malformed_allow_type == 0
                                ||   (   $this_warning_flags
                                      & ($::UTF8_WARN_SUPER
                                        |$::UTF8_WARN_PERL_EXTENDED))))
                        {
                            push @expected_warnings, $overflow_msg_pattern;
                        }
                    }

                    # It may be that the malformations have shortened the
                    # amount of input we look at so much that we can't tell
                    # what the category the code point was in.  Otherwise, set
                    # up the expected return flags based on the warnings and
                    # disallowments.
                    if ($this_expected_len < $this_needed_to_discern_len) {
                        $expect_regular_warnings = 0;
                    }
                    elsif (   ($this_warning_flags & $this_utf8n_flag_to_warn)
                           || (  $this_disallow_flags
                               & $this_utf8n_flag_to_disallow))
                    {
                        push @expected_return_flags, $return_flag;
                    }

                    # Finish setting up the expected warning.
                    if ($expect_regular_warnings) {

                        # So far the array contains warnings generated by
                        # malformations.  Add the expected regular one.
                        unshift @expected_warnings, $this_cp_message_qr;

                        # But it may need to be modified, because either of
                        # these malformations means we can't determine the
                        # expected code point.
                        if (   $short || $unexpected_noncont
                            || $dont_use_overlong_cp)
                        {
                            my $first_byte = substr($this_bytes, 0, 1);
                            $expected_warnings[0] = display_bytes(
                                    substr($this_bytes, 0, $this_expected_len));
                            $expected_warnings[0]
                                = qr/[Aa]\Qny UTF-8 sequence that starts with\E
                                     \Q $expected_warnings[0]\E
                                     \Q $this_non_cp_trailing_text\E/x;
                        }
                    }

                    # Is effectively disallowed if we've set up a malformation
                    # (unless malformations are allowed), even if the flag
                    # indicates it is allowed.  Fix up test name to indicate
                    # this as well
                    my $disallowed = 0;
                    if (   $this_disallow_flags & $this_utf8n_flag_to_disallow
                        && $this_expected_len >= $this_needed_to_discern_len)
                    {
                        $disallowed = 1;
                    }
                    if ($malformations_name) {
                        if ($malformed_allow_type == 0) {
                            $disallowed = 1;
                        }
                        elsif ($malformed_allow_type == 1) {

                            # Even if allowed, the malformation returns the
                            # REPLACEMENT CHARACTER.
                            $expected_uv = 0xFFFD;
                            $this_uv_string = "0xFFFD"
                        }
                    }

                    my $this_name = "$utf8_func() $testname: ";
                    my @scratch_expected_return_flags = @expected_return_flags;
                    if (! $initially_malformed) {
                        $this_name .= ($disallowed)
                                       ? 'disallowed, '
                                       : 'allowed, ';
                    }
                    $this_name .= "$eval_warn";
                    $this_name .= ", " . ((  $this_warning_flags
                                            & $this_utf8n_flag_to_warn)
                                          ? 'with flag for raising warnings'
                                          : 'no flag for raising warnings');
                    $this_name .= $malformations_name;

                    # Do the actual test using an eval
                    undef @warnings_gotten;
                    my $ret_ref;
                    my $this_flags
                        = $allow_flags|$this_warning_flags|$this_disallow_flags;
                    my $eval_text =      "$eval_warn; \$ret_ref"
                            . " = test_$utf8_func("
                            . "'$this_bytes', $this_length, $this_flags)";
                    eval "$eval_text";
                    if (! ok ($@ eq "", "$this_name: eval succeeded"))
                    {
                        diag "\$@='$@'; call was: "
                           . utf8n_display_call($eval_text);
                        next;
                    }

                    if ($disallowed) {
                        is($ret_ref->[0], 0, "    And returns 0")
                          or diag "Call was: " . utf8n_display_call($eval_text);
                    }
                    else {
                        is($ret_ref->[0], $expected_uv,
                                "    And returns expected uv: "
                              . $this_uv_string)
                          or diag "Call was: " . utf8n_display_call($eval_text);
                    }
                    is($ret_ref->[1], $this_expected_len,
                                        "    And returns expected length:"
                                      . " $this_expected_len")
                      or diag "Call was: " . utf8n_display_call($eval_text);

                    my $returned_flags = $ret_ref->[2];

                    for (my $i = @scratch_expected_return_flags - 1;
                         $i >= 0;
                         $i--)
                    {
                      if ($scratch_expected_return_flags[$i] & $returned_flags)
                      {
                          if ($scratch_expected_return_flags[$i]
                                              == $::UTF8_GOT_PERL_EXTENDED)
                          {
                              pass("    Expected and got return flag for"
                                  . " PERL_EXTENDED");
                          }
                                  # The first entries in this are
                                  # malformations
                          elsif ($i > @malformation_names - 1)  {
                              pass("    Expected and got return flag"
                                  . " for " . $controlling_warning_category);
                          }
                          else {
                              pass("    Expected and got return flag for "
                                  . $malformation_names[$i]
                                  . " malformation");
                          }
                          $returned_flags
                                      &= ~$scratch_expected_return_flags[$i];
                          splice @scratch_expected_return_flags, $i, 1;
                      }
                    }

                    if (! is($returned_flags, 0,
                       "    Got no unexpected return flags"))
                    {
                        diag "The unexpected flags gotten were: "
                           . (flags_to_text($returned_flags,
                                            \@utf8n_flags_to_text)
                                # We strip off any prefixes from the flag
                                # names
                             =~ s/ \b [A-Z] _ //xgr);
                        diag "Call was: " . utf8n_display_call($eval_text);
                    }

                    if (! is (scalar @scratch_expected_return_flags, 0,
                        "    Got all expected return flags"))
                    {
                        diag "The expected flags not gotten were: "
                           . (flags_to_text(eval join("|",
                                                @scratch_expected_return_flags),
                                            \@utf8n_flags_to_text)
                                # We strip off any prefixes from the flag
                                # names
                             =~ s/ \b [A-Z] _ //xgr);
                        diag "Call was: " . utf8n_display_call($eval_text);
                    }

                    if ($which_func) {
                        my @returned_warnings;
                        for my $element_ref (@{$ret_ref->[3]}) {
                            push @returned_warnings, $element_ref->{'text'};
                            my $text = $element_ref->{'text'};
                            my $flag = $element_ref->{'flag_bit'};
                            my $category = $element_ref->{'warning_category'};

                            if (! ok(($flag & ($flag-1)) == 0,
                                      "flag for returned msg is a single bit"))
                            {
                              diag sprintf("flags are %x; msg=%s", $flag, $text);
                            }
                            else {
                              if (grep { $_ == $flag } @expected_return_flags) {
                                  pass("flag for returned msg is expected");
                              }
                              else {
                                  fail("flag ("
                                     . flags_to_text($flag, \@utf8n_flags_to_text)
                                     . ") for returned msg is expected");
                              }
                            }

                            # In perl space, don't know the category numbers
                            isnt($category, 0,
                                          "returned category for msg isn't 0");
                        }

                        ok(@warnings_gotten == 0, "$utf8_func raised no warnings;"
                              . " the next tests are for ones in the returned"
                              . " variable")
                            or diag join "\n", "The unexpected warnings were:",
                                                              @warnings_gotten;
                        @warnings_gotten = @returned_warnings;
                    }

                    do_warnings_test(@expected_warnings)
                      or diag "Call was: " . utf8n_display_call($eval_text);
                    undef @warnings_gotten;

                    # Check CHECK_ONLY results when the input is
                    # disallowed.  Do this when actually disallowed,
                    # not just when the $this_disallow_flags is set.  We only
                    # test once utf8n_to_uvchr_msgs() with this.
                    if (   $disallowed
                        && ($which_func == 0 || ! $tested_CHECK_ONLY))
                    {
                        $tested_CHECK_ONLY = 1;
                        my $this_flags = $this_disallow_flags|$::UTF8_CHECK_ONLY;
                        my $eval_text = "use warnings; \$ret_ref ="
                                      . " test_$utf8_func('"
                                      . "$this_bytes', $this_length,"
                                      . " $this_flags)";
                        eval $eval_text;
                        if (! ok ($@ eq "",
                            "    And eval succeeded with CHECK_ONLY"))
                        {
                            diag "\$@='$@'; Call was: "
                               . utf8n_display_call($eval_text);
                            next;
                        }
                        is($ret_ref->[0], 0, "    CHECK_ONLY: Returns 0")
                          or diag "Call was: " . utf8n_display_call($eval_text);
                        is($ret_ref->[1], -1,
                                       "    CHECK_ONLY: returns -1 for length")
                          or diag "Call was: " . utf8n_display_call($eval_text);
                        if (! is(scalar @warnings_gotten, 0,
                                      "    CHECK_ONLY: no warnings generated"))
                        {
                            diag "Call was: " . utf8n_display_call($eval_text);
                            output_warnings(@warnings_gotten);
                        }
                    }

                    # Now repeat some of the above, but for
                    # uvchr_to_utf8_flags().  Since this comes from an
                    # existing code point, it hasn't overflowed, and isn't
                    # malformed.
                    next if @malformation_names;

                    my $uvchr_func = ($which_func)
                                     ? 'uvchr_to_utf8_flags_msgs'
                                     : 'uvchr_to_utf8_flags';

                    $this_warning_flags = ($use_warn_flag)
                                          ? $this_uvchr_flag_to_warn
                                          : 0;
                    $this_disallow_flags = ($do_disallow)
                                           ? $this_uvchr_flag_to_disallow
                                           : 0;

                    $disallowed = $this_disallow_flags
                                & $this_uvchr_flag_to_disallow;
                    $this_name .= ", " . ((  $this_warning_flags
                                           & $this_utf8n_flag_to_warn)
                                          ? 'with flag for raising warnings'
                                          : 'no flag for raising warnings');

                    $this_name = "$uvchr_func() $testname: "
                                        . (($disallowed)
                                           ? 'disallowed'
                                           : 'allowed');
                    $this_name .= ", $eval_warn";
                    $this_name .= ", " . ((  $this_warning_flags
                                           & $this_uvchr_flag_to_warn)
                                        ? 'with warning flag'
                                        : 'no warning flag');

                    undef @warnings_gotten;
                    my $ret;
                    $this_flags = $this_warning_flags|$this_disallow_flags;
                    $eval_text = "$eval_warn; \$ret ="
                            . " test_$uvchr_func("
                            . "$allowed_uv, $this_flags)";
                    eval "$eval_text";
                    if (! ok ($@ eq "", "$this_name: eval succeeded"))
                    {
                        diag "\$@='$@'; call was: "
                           . uvchr_display_call($eval_text);
                        next;
                    }

                    if ($which_func) {
                        if (defined $ret->[1]) {
                            my @returned_warnings;
                            push @returned_warnings, $ret->[1]{'text'};
                            my $text = $ret->[1]{'text'};
                            my $flag = $ret->[1]{'flag_bit'};
                            my $category = $ret->[1]{'warning_category'};

                            if (! ok(($flag & ($flag-1)) == 0,
                                        "flag for returned msg is a single bit"))
                            {
                                diag sprintf("flags are %x; msg=%s", $flag, $text);
                            }
                            else {
                                if ($flag & $this_uvchr_flag_to_disallow) {
                                    pass("flag for returned msg is expected");
                                }
                                else {
                                    fail("flag ("
                                        . flags_to_text($flag, \@utf8n_flags_to_text)
                                        . ") for returned msg is expected");
                                }
                            }

                            # In perl space, don't know the category numbers
                            isnt($category, 0,
                                            "returned category for msg isn't 0");

                            ok(@warnings_gotten == 0, "$uvchr_func raised no warnings;"
                                . " the next tests are for ones in the returned"
                                . " variable")
                                or diag join "\n", "The unexpected warnings were:",
                                                                @warnings_gotten;
                            @warnings_gotten = @returned_warnings;
                        }

                        $ret = $ret->[0];
                    }

                    if ($disallowed) {
                        is($ret, undef, "    And returns undef")
                          or diag "Call was: " . uvchr_display_call($eval_text);
                    }
                    else {
                        is($ret, $this_bytes, "    And returns expected string")
                          or diag "Call was: " . uvchr_display_call($eval_text);
                    }

                    do_warnings_test(@expected_warnings)
                      or diag "Call was: " . uvchr_display_call($eval_text);
                }
              }
            }
          }
        }
      }
    }
  }
}

done_testing;
