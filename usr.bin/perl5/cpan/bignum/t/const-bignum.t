# -*- mode: perl; -*-

# Binary, octal, and hexadecimal floating point literals were introduced in
# v5.22.0.
#
# - It wasn't until v5.28.0 that binary, octal, and hexadecimal floating point
#   literals were converted to the correct value on perls compiled with quadmath
#   support.
#
# - It wasn't until v5.32.0 that binary and octal floating point literals worked
#   correctly with constant overloading. Before v5.32.0, it seems like the
#   second character is always silently converted to an "x", so, e.g., "0b1.1p8"
#   is passed to the overload::constant subroutine as "0x1.1p8", and "01.1p+8"
#   is passed as "0x.1p+8".
#
# - Octal floating point literals using the "0o" prefix were introduced in
#   v5.34.0.

# Note that all numeric literals that should not be overloaded must be quoted.

use strict;
use warnings;

use Test::More tests => "171";

use bignum;

my $class = "Math::BigFloat";
my $x;

################################################################################
# The following tests should be identical for Math::BigInt, Math::BigFloat and
# Math::BigRat.

# These are handled by "binary".

$x = 0xff;
is($x, "255", "hexadecimal integer literal 0xff");
like(ref($x), qr/^Math::BigInt(::Lite)?$/,
     "value is a Math::BigInt or Math::BigInt::Lite");

SKIP: {
    # Hexadecimal literals using the "0X" prefix require v5.14.0.
    skip "perl v5.14.0 required for hexadecimal integer literals"
      . " with '0X' prefix", "2" if $] < "5.014";

    $x = eval "0XFF";
    is($x, "255", "hexadecimal integer literal 0XFF");
    like(ref($x), qr/^Math::BigInt(::Lite)?$/,
         "value is a Math::BigInt or Math::BigInt::Lite");
}

$x = 0377;
is($x, "255", "octal integer literal 0377");
like(ref($x), qr/^Math::BigInt(::Lite)?$/,
     "value is a Math::BigInt or Math::BigInt::Lite");

SKIP: {
    # Octal literals using the "0o" prefix require v5.34.0.
    skip "perl v5.34.0 required for octal floating point literals"
      . " with '0o' prefix", "4" if $] < "5.034";

    for my $str (qw/ 0o377 0O377 /) {
        $x = eval $str;
        is($x, "255", "octal integer literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }
}

$x = 0b11111111;
is($x, "255", "binary integer literal 0b11111111");
like(ref($x), qr/^Math::BigInt(::Lite)?$/,
     "value is a Math::BigInt or Math::BigInt::Lite");

SKIP: {
    # Binary literals using the "0B" prefix require v5.14.0.
    skip "perl v5.14.0 required for binary integer literals"
      . " with '0B' prefix", "2" if $] < "5.014";

    $x = eval "0B11111111";
    is($x, "255", "binary integer literal 0B11111111");
    like(ref($x), qr/^Math::BigInt(::Lite)?$/,
         "value is a Math::BigInt or Math::BigInt::Lite");
}

# These are handled by "float".

$x = 999999999999999999999999999999999999999999999999999999999999999999999999;
is($x,
   "999999999999999999999999999999999999999999999999999999999999999999999999",
   "decimal integer literal " . ("9" x 72));
like(ref($x), qr/^Math::BigInt(::Lite)?$/,
     "value is a Math::BigInt or Math::BigInt::Lite");

$x = 1e72 - 1;
is($x,
   "999999999999999999999999999999999999999999999999999999999999999999999999",
   "literal 1e72 - 1");
like(ref($x), qr/^Math::BigInt(::Lite)?$/,
     "value is a Math::BigInt or Math::BigInt::Lite");

# These are handled by "float".

SKIP: {
    # Hexadecimal floating point literals require v5.28.0.
    skip "perl v5.28.0 required for hexadecimal floating point literals",
      "6" * "2" + "2" * "2" if $] < "5.028";

    for my $str (qw/ 0x1.3ap+8 0X1.3AP+8
                     0x1.3ap8  0X1.3AP8
                     0x13a0p-4 0X13A0P-4 /)
    {
        $x = eval $str;
        is($x, "314", "hexadecimal floating point literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }

    for my $str (qw/ 0x0.0p+8 0X0.0P+8 /)
    {
        $x = eval $str;
        is($x, "0", "hexadecimal floating point literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }
}

SKIP: {
    # Octal floating point literals using the "0o" prefix require v5.34.0.
    skip "perl v5.34.0 required for octal floating point literals"
      . " with '0o' prefix", "6" * "2" + "6" * "2" if $] < "5.034";

    for my $str (qw/ 0o1.164p+8 0O1.164P+8
                     0o1.164p8  0O1.164P8
                     0o11640p-4 0O11640P-4 /)
    {
        $x = eval $str;
        is($x, "314", "octal floating point literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }

    for my $str (qw/ 0o0.0p+8 0O0.0P+8
                     0o0.0p8  0O0.0P8
                     0o0.0p-8 0O0.0P-8 /)
    {
        $x = eval $str;
        is($x, "0", "octal floating point literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }
}

SKIP: {
    # Octal floating point literals using the "0" prefix require v5.32.0.
    skip "perl v5.32.0 required for octal floating point literals",
      "6" * "2" + "6" * "2" if $] < "5.032";

    for my $str (qw/ 01.164p+8 01.164P+8
                     01.164p8  01.164P8
                     011640p-4 011640P-4 /)
    {
        $x = eval $str;
        is($x, "314", "octal floating point literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }

    for my $str (qw/ 00.0p+8 00.0P+8
                     00.0p8 00.0P8
                     00.0p-8 00.0P-8 /)
    {
        $x = eval $str;
        is($x, "0", "octal floating point literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }
}

SKIP: {
    # Binary floating point literals require v5.32.0.
    skip "perl v5.32.0 required for binary floating point literals",
      "6" * "2" + "6" * "2" if $] < "5.032";

    for my $str (qw/ 0b1.0011101p+8   0B1.0011101P+8
                     0b1.0011101p8    0B1.0011101P8
                     0b10011101000p-2 0B10011101000P-2 /)
    {
        $x = eval $str;
        is($x, "314", "binary floating point literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }

    for my $str (qw/ 0b0p+8 0B0P+8
                     0b0p8 0B0P8
                     0b0p-8 0B0P-8
                   /)
    {
        $x = eval $str;
        is($x, "0", "binary floating point literal $str");
        like(ref($x), qr/^Math::BigInt(::Lite)?$/,
             "value is a Math::BigInt or Math::BigInt::Lite");
    }
}

# These are handled by "integer".

$x = 314;
is($x, "314", "integer literal 314");
like(ref($x), qr/^Math::BigInt(::Lite)?$/,
     "value is a Math::BigInt or Math::BigInt::Lite");

$x = 0;
is($x, "0", "integer literal 0");
like(ref($x), qr/^Math::BigInt(::Lite)?$/,
     "value is a Math::BigInt or Math::BigInt::Lite");

$x = 2 ** 255;
is($x,
   "578960446186580977117854925043439539266"
   . "34992332820282019728792003956564819968",
   "2 ** 255");
like(ref($x), qr/^Math::BigInt(::Lite)?$/,
     "value is a Math::BigInt or Math::BigInt::Lite");

# These are handled by "binary".

{
    no warnings "portable";     # protect against "non-portable" warnings

    # hexadecimal constant
    $x = 0x123456789012345678901234567890;
    is($x,
       "94522879687365475552814062743484560",
       "hexadecimal constant 0x123456789012345678901234567890");
    like(ref($x), qr/^Math::BigInt(::Lite)?$/,
         "value is a Math::BigInt or Math::BigInt::Lite");

    # octal constant
    $x = 012345676543210123456765432101234567654321;
    is($x,
       "1736132869400711976876385488263403729",
       "octal constant 012345676543210123456765432101234567654321");
    like(ref($x), qr/^Math::BigInt(::Lite)?$/,
         "value is a Math::BigInt or Math::BigInt::Lite");

    # binary constant
    $x = 0b01010100011001010110110001110011010010010110000101101101;
    is($x,
       "23755414508757357",
       "binary constant 0b0101010001100101011011000111"
       . "0011010010010110000101101101");
    like(ref($x), qr/^Math::BigInt(::Lite)?$/,
         "value is a Math::BigInt or Math::BigInt::Lite");
}

################################################################################
# The following tests are unique to $class.

# These are handled by "float".

$x = 0.999999999999999999999999999999999999999999999999999999999999999999999999;
is($x,
   "0.999999999999999999999999999999999999999999999999999999999999999999999999",
   "decimal floating point literal 0." . ("9" x 72));
is(ref($x), $class, "value is a $class");

$x = 1e72 - 0.1;
is($x,
   "999999999999999999999999999999999999999999999999999999999999999999999999.9",
   "literal 1e72 - 0.1");
is(ref($x), $class, "value is a $class");

# These are handled by "float".

SKIP: {
    # Hexadecimal floating point literals require v5.28.0.
    skip "perl v5.22.0 required for hexadecimal floating point literals",
      "6" * "2" if $] < "5.028";

    for my $str (qw/ 0x1.92p+1 0X1.92P+1
                     0x1.92p1 0X1.92P1
                     0x19.2p-3 0X19.2P-3 /)
    {
        $x = eval $str;
        is($x, "3.140625", "hexadecimal floating point literal $str");
        is(ref($x), $class, "value is a $class");
    }
}

SKIP: {
    # Octal floating point literals using the "0o" prefix require v5.34.0.
    skip "perl v5.34.0 required for octal floating point literals"
      . " with '0o' prefix", "6" * "2" if $] < "5.034";

    for my $str (qw/ 0o1.444p+1 0O1.444P+1
                     0o1.444p1  0O1.444P1
                     0o14.44p-2 0O14.44P-2 /)
    {
        $x = eval $str;
        is($x, "3.140625", "octal floating point literal $str");
        is(ref($x), $class, "value is a $class");
    }
}

SKIP: {
    # Octal floating point literals using the "0" prefix require v5.32.0.
    skip "perl v5.32.0 required for octal floating point literals",
      "6" * "2" if $] < "5.032";

    for my $str (qw/ 01.444p+1 01.444P+1
                     01.444p1  01.444P1
                     014.44p-2 014.44P-2 /)
    {
        $x = eval $str;
        is($x, "3.140625", "octal floating point literal $str");
        is(ref($x), $class, "value is a $class");
    }
}

SKIP: {
    # Binary floating point literals require v5.32.0.
    skip "perl v5.32.0 required for binary floating point literals",
      "6" * "2" if $] < "5.032";

    for my $str (qw/ 0b1.1001001p+1 0B1.1001001P+1
                     0b1.1001001p1  0B1.1001001P1
                     0b110.01001p-1 0B110.01001P-1 /)
    {
        $x = eval $str;
        is($x, "3.140625", "binary floating point literal $str");
        is(ref($x), $class, "value is a $class");
    }
}

is(1.0 / 3.0, "0.3333333333333333333333333333333333333333",
   "1.0 / 3.0 = 0.3333333333333333333333333333333333333333");
