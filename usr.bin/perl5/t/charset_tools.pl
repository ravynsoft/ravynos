# Tools to aid testing across platforms with different character sets.

$::IS_ASCII  = ord 'A' ==  65;
$::IS_EBCDIC = ord 'A' == 193;

# The following functions allow tests to work on both EBCDIC and ASCII-ish
# platforms.  They convert string scalars between the native character set and
# the set of 256 characters which is usually called Latin1.  However, they
# will work properly with any character input, not just Latin1.

*native_to_uni = ($::IS_ASCII)
                ? sub { return shift }
                : sub {
    my $string = shift;

    my $output = "";
    for my $i (0 .. length($string) - 1) {
        $output .= chr(utf8::native_to_unicode(ord(substr($string, $i, 1))));
    }
    # Preserve utf8ness of input onto the output, even if it didn't need to be
    # utf8
    utf8::upgrade($output) if utf8::is_utf8($string);

    return $output;
};

*uni_to_native = ($::IS_ASCII)
                ? sub { return shift }
                : sub {
    my $string = shift;

    my $output = "";
    for my $i (0 .. length($string) - 1) {
        $output .= chr(utf8::unicode_to_native(ord(substr($string, $i, 1))));
    }
    # Preserve utf8ness of input onto the output, even if it didn't need to be
    # utf8
    utf8::upgrade($output) if utf8::is_utf8($string);

    return $output;
};

my @utf8_skip;

if ($::IS_EBCDIC) {
    @utf8_skip = (
        # This translates a utf-8-encoded byte into how many bytes the full utf8
        # character occupies.

        # 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 0
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 1
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 2
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 3
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 4
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 5
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 6
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  # 7
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  # 8
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  # 9
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  # A
       -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  # B
       -1,-1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  # C
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  # D
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  # E
        4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7,13,  # F
    );
}

*byte_utf8a_to_utf8n = ($::IS_ASCII)
                ? sub { return shift }
                : sub {
    # Convert a UTF-8 byte sequence into the platform's native UTF-8
    # equivalent, currently only UTF-8 and UTF-EBCDIC.

    my $string = shift;
    die "Input to byte_utf8a-to_utf8n() must not be flagged UTF-8"
                                                    if utf8::is_utf8($string);
    die "Expecting ASCII or EBCDIC" unless $::IS_EBCDIC;

    my $length = length($string);
    #diag($string);
    #diag($length);
    my $out = "";
    for ($i = 0; $i < $length; $i++) {
        my $byte = ord substr($string, $i, 1);
        my $byte_count = $utf8_skip[$byte];
        #diag($byte);
        #diag($byte_count);

        die "Illegal start byte" if $byte_count < 0;
        if ($i + $byte_count > $length) {
            die "Attempt to read " . $i + $byte_count - $length . " beyond end-of-string";
        }

        # Just translate UTF-8 invariants directly.
        if ($byte_count == 1) {
            $out .= chr utf8::unicode_to_native($byte);
            next;
        }

        # Otherwise calculate the code point ordinal represented by the
        # sequence beginning with this byte, using the algorithm adapted from
        # utf8.c.  We absorb each byte in the sequence as we go along
        my $ord = $byte & (0x1F >> ($byte_count - 2));
        my $bytes_remaining = $byte_count - 1;
        while ($bytes_remaining > 0) {
            $byte = ord substr($string, ++$i, 1);
            unless (($byte & 0xC0) == 0x80) {
                die sprintf "byte '%X' is not a valid continuation", $byte;
            }
            $ord = $ord << 6 | ($byte & 0x3f);
            $bytes_remaining--;
        }
        #diag($byte);
        #diag($ord);

        my $expected_bytes = $ord < 0x80
                             ? 1
                             : $ord < 0x800
                               ? 2
                               : $ord < 0x10000
                                 ? 3
                                 : $ord < 0x200000
                                   ? 4
                                   : $ord < 0x4000000
                                     ? 5
                                     : $ord < 0x80000000
                                       ? 6
                                       : 7;
                                       #: (uv) < UTF8_QUAD_MAX ? 7 : 13 )

        # Make sure is not an overlong sequence
        if ($byte_count != $expected_bytes) {
            die sprintf "character U+%X should occupy %d bytes, not %d",
                                            $ord, $expected_bytes, $byte_count;
        }

        # Now that we have found the code point the original UTF-8 meant, we
        # use the native chr function to get its native string equivalent.
        $out .= chr utf8::unicode_to_native($ord);
    }

    utf8::encode($out); # Turn off utf8 flag.
    #diag($out);
    return $out;
};

my @i8_to_native = (    # Only code page 1047 so far.
# _0   _1   _2   _3   _4   _5   _6   _7   _8   _9   _A   _B   _C   _D   _E   _F
0x00,0x01,0x02,0x03,0x37,0x2D,0x2E,0x2F,0x16,0x05,0x15,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x3C,0x3D,0x32,0x26,0x18,0x19,0x3F,0x27,0x1C,0x1D,0x1E,0x1F,
0x40,0x5A,0x7F,0x7B,0x5B,0x6C,0x50,0x7D,0x4D,0x5D,0x5C,0x4E,0x6B,0x60,0x4B,0x61,
0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0x7A,0x5E,0x4C,0x7E,0x6E,0x6F,
0x7C,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,
0xD7,0xD8,0xD9,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xAD,0xE0,0xBD,0x5F,0x6D,
0x79,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x91,0x92,0x93,0x94,0x95,0x96,
0x97,0x98,0x99,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xC0,0x4F,0xD0,0xA1,0x07,
0x20,0x21,0x22,0x23,0x24,0x25,0x06,0x17,0x28,0x29,0x2A,0x2B,0x2C,0x09,0x0A,0x1B,
0x30,0x31,0x1A,0x33,0x34,0x35,0x36,0x08,0x38,0x39,0x3A,0x3B,0x04,0x14,0x3E,0xFF,
0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x51,0x52,0x53,0x54,0x55,0x56,
0x57,0x58,0x59,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x70,0x71,0x72,0x73,
0x74,0x75,0x76,0x77,0x78,0x80,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,0x90,0x9A,0x9B,0x9C,
0x9D,0x9E,0x9F,0xA0,0xAA,0xAB,0xAC,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,
0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBE,0xBF,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xDA,0xDB,
0xDC,0xDD,0xDE,0xDF,0xE1,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xFA,0xFB,0xFC,0xFD,0xFE,
);

my @native_to_i8;
for (my $i = 0; $i < 256; $i++) {
    $native_to_i8[$i8_to_native[$i]] = $i;
}

# Use these to convert to/from UTF-8 bytes.  I8 is the encoding that
# corresponds to UTF-8 with start bytes, continuation bytes, and invariant
# bytes.  UTF-EBCDIC is derived from this by a mapping which causes things
# like the start byte C5 to map to something else, as C5 is actually an 'E' in
# EBCDIC so can't be a real start byte, as it must be an invariant; and it
# maps 0x45 (an ASCII 'E') to C5.
*I8_to_native = ($::IS_ASCII)
                ? sub { return shift }
                : sub { return join "", map { chr $i8_to_native[ord $_] }
                                            split "", shift };
*native_to_I8 = ($::IS_ASCII)
                ? sub { return shift }
                : sub { return join "", map { chr $native_to_i8[ord $_] }
                                            split "", shift };

1
