#!./perl

#
# test the bit operators '&', '|', '^', '~', '<<', and '>>'
#

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
    require "./charset_tools.pl";
    require Config;
}

use warnings;

# Tests don't have names yet.
# If you find tests are failing, please try adding names to tests to track
# down where the failure is, and supply your new names as a patch.
# (Just-in-time test naming)
plan tests => 510;

# numerics
ok ((0xdead & 0xbeef) == 0x9ead);
ok ((0xdead | 0xbeef) == 0xfeef);
ok ((0xdead ^ 0xbeef) == 0x6042);
ok ((~0xdead & 0xbeef) == 0x2042);

# shifts
ok ((257 << 7) == 32896);
ok ((33023 >> 7) == 257);

# signed vs. unsigned
ok ((~0 > 0 && do { use integer; ~0 } == -1));

{   # GH #18639
    my $iv_min = -(~0 >> 1) - 1;
    my $shifted;
    { use integer; $shifted = $iv_min << 0 };
    is($shifted, $iv_min, "IV_MIN << 0 yields IV_MIN under 'use integer'");
}

# GH #18691
# Exercise some corner cases on shifting more bits than the size of IV/UV.
# All these should work even if the shift amount doesn't fit in IV or UV.
is(4 << 2147483648, 0, "4 << 2147483648 yields 0");
is(16 << 4294967295, 0, "16 << 4294967295 yields 0");
is(8 >> 4294967296, 0, "8 >> 4294967296 yields 0");
is(11 << 18446744073709551615, 0, "11 << 18446744073709551615 yields 0");
is(do { use integer; -9 >> 18446744073709551616 }, -1,
   "-9 >> 18446744073709551616 under 'use integer' yields -1");
is(do { use integer; -4 << -2147483648 }, -1,
   "-4 << -2147483648 under 'use integer' yields -1");
# Quotes around -9223372036854775808 below are to make it a single term.
# Without quotes, it will be parsed as an expression with an unary minus
# operator which will clip the result to IV range under "use integer".
is(do { use integer; -5 >> '-9223372036854775808' }, 0,
   "-5 >> -9223372036854775808 under 'use integer' yields 0");

my $bits = 0;
for (my $i = ~0; $i; $i >>= 1) { ++$bits; }
my $cusp = 1 << ($bits - 1);


ok (($cusp & -1) > 0 && do { use integer; $cusp & -1 } < 0);
ok (($cusp | 1) > 0 && do { use integer; $cusp | 1 } < 0);
ok (($cusp ^ 1) > 0 && do { use integer; $cusp ^ 1 } < 0);
ok ((1 << ($bits - 1)) == $cusp &&
    do { use integer; 1 << ($bits - 1) } == -$cusp);
ok (($cusp >> 1) == ($cusp / 2) &&
    do { use integer; abs($cusp >> 1) } == ($cusp / 2));

$Aaz = chr(ord("A") & ord("z"));
$Aoz = chr(ord("A") | ord("z"));
$Axz = chr(ord("A") ^ ord("z"));

# short strings
is (("AAAAA" & "zzzzz"), ($Aaz x 5));
is (("AAAAA" | "zzzzz"), ($Aoz x 5));
is (("AAAAA" ^ "zzzzz"), ($Axz x 5));

# long strings
$foo = "A" x 150;
$bar = "z" x 75;
$zap = "A" x 75;
# & truncates
is (($foo & $bar), ($Aaz x 75 ));
# | does not truncate
is (($foo | $bar), ($Aoz x 75 . $zap));
# ^ does not truncate
is (($foo ^ $bar), ($Axz x 75 . $zap));

# string constants.  These tests expect the bit patterns of these strings in
# ASCII, so convert to that.
sub _and($) { $_[0] & native_to_uni("+0") }
sub _oar($) { $_[0] | native_to_uni("+0") }
sub _xor($) { $_[0] ^ native_to_uni("+0") }
is _and native_to_uni("waf"), native_to_uni('# '),  'str var & const str'; # [perl #20661]
is _and native_to_uni("waf"), native_to_uni('# '),  'str var & const str again'; # [perl #20661]
is _oar native_to_uni("yit"), native_to_uni('{yt'), 'str var | const str';
is _oar native_to_uni("yit"), native_to_uni('{yt'), 'str var | const str again';
is _xor native_to_uni("yit"), native_to_uni('RYt'), 'str var ^ const str';
is _xor native_to_uni("yit"), native_to_uni('RYt'), 'str var ^ const str again';

SKIP: {
    skip "Converting a numeric doesn't work with EBCDIC unlike the above tests",
         3 if $::IS_EBCDIC;
    is _and  0, '0',   'num var & const str';     # [perl #20661]
    is _oar  0, '0',   'num var | const str';
    is _xor  0, '0',   'num var ^ const str';
}

# But don’t mistake a COW for a constant when assigning to it
%h=(150=>1);
$i=(keys %h)[0];
$i |= 105;
is $i, 255, '[perl #108480] $cow |= number';
$i=(keys %h)[0];
$i &= 105;
is $i, 0, '[perl #108480] $cow &= number';
$i=(keys %h)[0];
$i ^= 105;
is $i, 255, '[perl #108480] $cow ^= number';

#
is ("ok \xFF\xFF\n" & "ok 19\n", "ok 19\n");
is ("ok 20\n" | "ok \0\0\n", "ok 20\n");
is ("o\000 \0001\000" ^ "\000k\0002\000\n", "ok 21\n");

#
is ("ok \x{FF}\x{FF}\n" & "ok 22\n", "ok 22\n");
is ("ok 23\n" | "ok \x{0}\x{0}\n", "ok 23\n");
is ("o\x{0} \x{0}4\x{0}" ^ "\x{0}k\x{0}2\x{0}\n", "ok 24\n");

# More variations on 19 and 22.
is ("ok \xFF\x{FF}\n" & "ok 41\n", "ok 41\n");
is ("ok \x{FF}\xFF\n" & "ok 42\n", "ok 42\n");

# Tests to see if you really can do casts negative floats to unsigned properly
$neg1 = -1.0;
ok (~ $neg1 == 0);
$neg7 = -7.0;
ok (~ $neg7 == 6);


# double magic tests

sub TIESCALAR { bless { value => $_[1], orig => $_[1] } }
sub STORE { $_[0]{store}++; $_[0]{value} = $_[1] }
sub FETCH { $_[0]{fetch}++; $_[0]{value} }
sub stores { tied($_[0])->{value} = tied($_[0])->{orig};
             delete(tied($_[0])->{store}) || 0 }
sub fetches { delete(tied($_[0])->{fetch}) || 0 }

# numeric double magic tests

tie $x, "main", 1;
tie $y, "main", 3;

is(($x | $y), 3);
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x & $y), 1);
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x ^ $y), 2);
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x |= $y), 3);
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(($x &= $y), 1);
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(($x ^= $y), 2);
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(~~$y, 3);
is(fetches($y), 1);
is(stores($y), 0);

{ use integer;

is(($x | $y), 3);
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x & $y), 1);
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x ^ $y), 2);
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x |= $y), 3);
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(($x &= $y), 1);
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(($x ^= $y), 2);
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(~$y, -4);
is(fetches($y), 1);
is(stores($y), 0);

} # end of use integer;

# stringwise double magic tests

tie $x, "main", "a";
tie $y, "main", "c";

is(($x | $y), ("a" | "c"));
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x & $y), ("a" & "c"));
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x ^ $y), ("a" ^ "c"));
is(fetches($x), 1);
is(fetches($y), 1);
is(stores($x), 0);
is(stores($y), 0);

is(($x |= $y), ("a" | "c"));
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(($x &= $y), ("a" & "c"));
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(($x ^= $y), ("a" ^ "c"));
is(fetches($x), 2);
is(fetches($y), 1);
is(stores($x), 1);
is(stores($y), 0);

is(~~$y, "c");
is(fetches($y), 1);
is(stores($y), 0);

my $g;
# Note: if the vec() reads are part of the is() calls it's treated as
# in lvalue context, so we save it separately
$g = vec($x, 0, 1);
is($g, (ord("a") & 0x01), "check vec value");
is(fetches($x), 1, "fetches for vec read");
is(stores($x), 0, "stores for vec read");
# similarly here, and code like:
#   $g = (vec($x, 0, 1) = 0)
# results in an extra fetch, since the inner assignment returns the LV
vec($x, 0, 1) = 0;
# one fetch in vec() another when the LV is assigned to
is(fetches($x), 2, "fetches for vec write");
is(stores($x), 1, "stores for vec write");

{
    my $a = "a";
    utf8::upgrade($a);
    tie $x, "main", $a;
    $g = vec($x, 0, 1);
    is($g, (ord("a") & 0x01), "check vec value (utf8)");
    is(fetches($x), 1, "fetches for vec read (utf8)");
    is(stores($x), 0, "stores for vec read (utf8)");
    vec($x, 0, 1) = 0;
    # one fetch in vec() another when the LV is assigned to
    is(fetches($x), 2, "fetches for vec write (utf8)");
    is(stores($x), 1, "stores for vec write (utf8)");
}

$a = "\0\x{100}"; chop($a);
ok(utf8::is_utf8($a)); # make sure UTF8 flag is still there
$a = ~$a;
is($a, "\xFF", "~ works with utf-8");
ok(! utf8::is_utf8($a), "    and turns off the UTF-8 flag");

$a = "\0\x{100}"; chop($a);
undef $b;
$b = $a | "\xFF";
ok(utf8::is_utf8($b), "Verify UTF-8 | non-UTF-8 retains UTF-8 flag");
undef $b;
$b = "\xFF" | $a;
ok(utf8::is_utf8($b), "Verify non-UTF-8 | UTF-8 retains UTF-8 flag");
undef $b;
$b = $a & "\xFF";
ok(utf8::is_utf8($b), "Verify UTF-8 & non-UTF-8 retains UTF-8 flag");
undef $b;
$b = "\xFF" & $a;
ok(utf8::is_utf8($b), "Verify non-UTF-8 & UTF-8 retains UTF-8 flag");
undef $b;
$b = $a ^ "\xFF";
ok(utf8::is_utf8($b), "Verify UTF-8 ^ non-UTF-8 retains UTF-8 flag");
undef $b;
$b = "\xFF" ^ $a;
ok(utf8::is_utf8($b), "Verify non-UTF-8 ^ UTF-8 retains UTF-8 flag");


# [rt.perl.org 33003]
# This would cause a segfault without malloc wrap
SKIP: {
  skip "No malloc wrap checks" unless $Config::Config{usemallocwrap};
  like( runperl(prog => 'eval q($#a>>=1); print 1'), qr/^1\n?/ );
}

# [perl #37616] Bug in &= (string) and/or m//
{
    $a = "aa";
    $a &= "a";
    ok($a =~ /a+$/, 'ASCII "a" is NUL-terminated');

    $b = "bb\x{FF}";
    utf8::upgrade($b);
    $b &= "b";
    ok($b =~ /b+$/, 'Unicode "b" is NUL-terminated');
}

# New string- and number-specific bitwise ops
{
  use feature "bitwise";
  no warnings "experimental::bitwise";
  is "22" & "66", 2,    'numeric & with strings';
  is "22" | "66", 86,   'numeric | with strings';
  is "22" ^ "66", 84,   'numeric ^ with strings';
  is ~"22" & 0xff, 233, 'numeric ~ with string';
  is 22 &. 66, 22,     '&. with numbers';
  is 22 |. 66, 66,     '|. with numbers';
  is 22 ^. 66, "\4\4", '^. with numbers';
  if ($::IS_EBCDIC) {
    # ord('2') is 0xF2 on EBCDIC
    is ~.22, "\x0d\x0d", '~. with number';
  }
  else {
    # ord('2') is 0x32 on ASCII
    is ~.22, "\xcd\xcd", '~. with number';
  }
  $_ = "22";
  is $_ &= "66", 2,  'numeric &= with strings';
  $_ = "22";
  is $_ |= "66", 86, 'numeric |= with strings';
  $_ = "22";
  is $_ ^= "66", 84, 'numeric ^= with strings';
  $_ = 22;
  is $_ &.= 66, 22,     '&.= with numbers';
  $_ = 22;
  is $_ |.= 66, 66,     '|.= with numbers';
  $_ = 22;
  is $_ ^.= 66, "\4\4", '^.= with numbers';

 # signed vs. unsigned
 ok ((~0 > 0 && do { use integer; ~0 } == -1));

 my $bits = 0;
 for (my $i = ~0; $i; $i >>= 1) { ++$bits; }
 my $cusp = 1 << ($bits - 1);

 ok (($cusp & -1) > 0 && do { use integer; $cusp & -1 } < 0);
 ok (($cusp | 1) > 0 && do { use integer; $cusp | 1 } < 0);
 ok (($cusp ^ 1) > 0 && do { use integer; $cusp ^ 1 } < 0);
 ok ((1 << ($bits - 1)) == $cusp &&
     do { use integer; 1 << ($bits - 1) } == -$cusp);
 ok (($cusp >> 1) == ($cusp / 2) &&
    do { use integer; abs($cusp >> 1) } == ($cusp / 2));
}
# Repeat some of those, with 'use v5.27'
{
  use v5.27;

  is "22" & "66", 2,    'numeric & with strings';
  is "22" | "66", 86,   'numeric | with strings';
  is "22" ^ "66", 84,   'numeric ^ with strings';
  is ~"22" & 0xff, 233, 'numeric ~ with string';
  is 22 &. 66, 22,     '&. with numbers';
  is 22 |. 66, 66,     '|. with numbers';
  is 22 ^. 66, "\4\4", '^. with numbers';
  if ($::IS_EBCDIC) {
    # ord('2') is 0xF2 on EBCDIC
    is ~.22, "\x0d\x0d", '~. with number';
  }
  else {
    # ord('2') is 0x32 on ASCII
    is ~.22, "\xcd\xcd", '~. with number';
  }
  $_ = "22";
  is $_ &= "66", 2,  'numeric &= with strings';
  $_ = "22";
  is $_ |= "66", 86, 'numeric |= with strings';
  $_ = "22";
  is $_ ^= "66", 84, 'numeric ^= with strings';
  $_ = 22;
  is $_ &.= 66, 22,     '&.= with numbers';
  $_ = 22;
  is $_ |.= 66, 66,     '|.= with numbers';
  $_ = 22;
  is $_ ^.= 66, "\4\4", '^.= with numbers';
}

# ref tests

my %res;

for my $str ("x", "\x{B6}") {
    utf8::upgrade($str) if $str !~ /x/;
    for my $chr (qw/S A H G X ( * F/) {
        for my $op (qw/| & ^/) {
            my $co = ord $chr;
            my $so = ord $str;
            $res{"$chr$op$str"} = eval qq/chr($co $op $so)/;
        }
    }
    $res{"undef|$str"} = $str;
    $res{"undef&$str"} = "";
    $res{"undef^$str"} = $str;
}

sub PVBM () { "X" }
1 if index "foo", PVBM;

my $warn = 0;
local $^W = 1;
local $SIG{__WARN__} = sub { $warn++ };

sub is_first {
    my ($got, $orig, $op, $str, $name) = @_;
    is(substr($got, 0, 1), $res{"$orig$op$str"}, $name);
}

for (
    # [object to test, first char of stringification, name]
    [undef,             "undef",    "undef"         ],
    [\1,                "S",        "scalar ref"    ],
    [[],                "A",        "array ref"     ],
    [{},                "H",        "hash ref"      ],
    [qr/x/,             "(",        "qr//"          ],
    [*foo,              "*",        "glob"          ],
    [\*foo,             "G",        "glob ref"      ],
    [PVBM,              "X",        "PVBM"          ],
    [\PVBM,             "S",        "PVBM ref"      ],
    [bless([], "Foo"),  "F",        "object"        ],
) {
    my ($val, $orig, $type) = @$_;

    for (["x", "string"], ["\x{B6}", "utf8"]) {
        my ($str, $desc) = @$_;
        utf8::upgrade($str) if $desc =~ /utf8/;

        $warn = 0;

        is_first($val | $str, $orig, "|", $str, "$type | $desc");
        is_first($val & $str, $orig, "&", $str, "$type & $desc");
        is_first($val ^ $str, $orig, "^", $str, "$type ^ $desc");

        is_first($str | $val, $orig, "|", $str, "$desc | $type");
        is_first($str & $val, $orig, "&", $str, "$desc & $type");
        is_first($str ^ $val, $orig, "^", $str, "$desc ^ $type");

        my $new;
        ($new = $val) |= $str;
        is_first($new, $orig, "|", $str, "$type |= $desc");
        ($new = $val) &= $str;
        is_first($new, $orig, "&", $str, "$type &= $desc");
        ($new = $val) ^= $str;
        is_first($new, $orig, "^", $str, "$type ^= $desc");

        ($new = $str) |= $val;
        is_first($new, $orig, "|", $str, "$desc |= $type");
        ($new = $str) &= $val;
        is_first($new, $orig, "&", $str, "$desc &= $type");
        ($new = $str) ^= $val;
        is_first($new, $orig, "^", $str, "$desc ^= $type");

        if ($orig eq "undef") {
            # undef |= and undef ^= don't warn
            is($warn, 10, "no duplicate warnings");
        }
        else {
            is($warn, 0, "no warnings");
        }
    }
}

delete $SIG{__WARN__};

my $strval;

{
    package Bar;
    use overload q/""/ => sub { $strval };

    package Baz;
    use overload q/|/ => sub { "y" };
}

ok(!eval { 1 if bless([], "Bar") | "x"; 1 },"string overload can't use |");
like($@, qr/no method found/,               "correct error");
is(eval { bless([], "Baz") | "x" }, "y",    "| overload works");

my $obj = bless [], "Bar";
$strval = "x";
eval { $obj |= "Q" };
$strval = "z";
is("$obj", "z", "|= doesn't break string overload");

# [perl #29070]
$^A .= new version ~$_ for eval sprintf('"\\x%02x"', 0xff - ord("1")),
                           $::IS_EBCDIC ? v13 : v205, # 255 - ord('2')
                           eval sprintf('"\\x%02x"', 0xff - ord("3"));
is $^A, "123", '~v0 clears vstring magic on retval';

{
    my $w = $Config::Config{ivsize} * 8;

    fail("unexpected w $w") unless $w == 32 || $w == 64;

    is(1 << 1, 2, "UV 1 left shift 1");
    is(1 >> 1, 0, "UV 1 right shift 1");

    is(0x7b << -4, 0x007, "UV left negative shift == right shift");
    is(0x7b >> -4, 0x7b0, "UV right negative shift == left shift");

    is(0x7b <<  0, 0x07b, "UV left  zero shift == identity");
    is(0x7b >>  0, 0x07b, "UV right zero shift == identity");

    is(0x0 << -1, 0x0, "zero left  negative shift == zero");
    is(0x0 >> -1, 0x0, "zero right negative shift == zero");

    cmp_ok(1 << $w - 1, '==', 2 ** ($w - 1), # not is() because NV stringify.
       "UV left $w - 1 shift == 2 ** ($w - 1)");
    is(1 << $w,     0, "UV left shift $w     == zero");
    is(1 << $w + 1, 0, "UV left shift $w + 1 == zero");

    is(1 >> $w - 1, 0, "UV right shift $w - 1 == zero");
    is(1 >> $w,     0, "UV right shift $w     == zero");
    is(1 >> $w + 1, 0, "UV right shift $w + 1 == zero");

    # Negative shiftees get promoted to UVs before shifting.  This is
    # not necessarily the ideal behavior, but that is what is happening.
    if ($w == 64) {
        no warnings "portable";
        no warnings "overflow"; # prevent compile-time warning for ivsize=4
        is(-1 << 1, 0xFFFF_FFFF_FFFF_FFFE,
           "neg UV (sic) left shift  = 0xFF..E");
        is(-1 >> 1, 0x7FFF_FFFF_FFFF_FFFF,
           "neg UV (sic) right shift = 0x7F..F");
    } elsif ($w == 32) {
        no warnings "portable";
        is(-1 << 1, 0xFFFF_FFFE, "neg left shift  == 0xFF..E");
        is(-1 >> 1, 0x7FFF_FFFF, "neg right shift == 0x7F..F");
    }

    {
        # 'use integer' means use IVs instead of UVs.
        use integer;

        # No surprises here.
        is(1 << 1, 2, "IV 1 left shift 1  == 2");
        is(1 >> 1, 0, "IV 1 right shift 1 == 0");

        # The left overshift should behave like without 'use integer',
        # that is, return zero.
        is(1 << $w,     0, "IV 1 left shift $w     == 0");
        is(1 << $w + 1, 0, "IV 1 left shift $w + 1 == 0");
        is(-1 << $w,     0, "IV -1 left shift $w     == 0");
        is(-1 << $w + 1, 0, "IV -1 left shift $w + 1 == 0");

        # Even for negative IVs, left shift is multiplication.
        # But right shift should display the stuckiness to -1.
        is(-1 <<      1, -2, "IV -1 left shift       1 == -2");
        is(-1 >>      1, -1, "IV -1 right shift      1 == -1");

        # As for UVs, negative shifting means the reverse shift.
        is(-1 <<     -1, -1, "IV -1 left shift      -1 == -1");
        is(-1 >>     -1, -2, "IV -1 right shift     -1 == -2");

        # Test also at and around wordsize, expect stuckiness to -1.
        is(-1 >> $w - 1, -1, "IV -1 right shift $w - 1 == -1");
        is(-1 >> $w,     -1, "IV -1 right shift $w     == -1");
        is(-1 >> $w + 1, -1, "IV -1 right shift $w + 1 == -1");
    }
}

# [perl #129287] UTF8 & was not providing a trailing null byte.
# This test is a bit convoluted, as we want to make sure that the string
# allocated for &’s target contains memory initialised to something other
# than a null byte.  Uninitialised memory does not make for a reliable
# test.  So we do &. on a longer non-utf8 string first.
for (["aaa","aaa"],[substr ("a\x{100}",0,1), "a"]) {
    use feature "bitwise";
    no warnings "experimental::bitwise", "pack";
    $byte = substr unpack("P2", pack "P", $$_[0] &. $$_[1]), -1;
}
is $byte, "\0", "utf8 &. appends null byte";

# only visible under sanitize
fresh_perl_is('$x = "UUUUUUUV"; $y = "xxxxxxx"; $x |= $y; print $x',
              ( $::IS_EBCDIC) ? 'XXXXXXXV' : '}}}}}}}V',
              {}, "[perl #129995] access to freed memory");


#
# Using code points above 0xFF is fatal
#
foreach my $op_info ([and => "&"], [or => "|"], [xor => "^"]) {
    my ($op_name, $op) = @$op_info;
    local $@;
    eval '$_ = "\xFF" ' . $op . ' "\x{100}";';
    like $@, qr /^Use of strings with code points over 0xFF as arguments (?#
                 )to bitwise $op_name \Q($op)\E operator is not allowed/,
         "Use of code points above 0xFF as arguments to bitwise " .
         "$op_name ($op) is not allowed";
}

{
    local $@;
    eval '$_ = ~ "\x{100}";';
    like $@, qr /^Use of strings with code points over 0xFF as arguments (?#
                 )to 1's complement \(~\) operator is not allowed/,
         "Use of code points above 0xFF as argument to 1's complement " .
         "(~) is not allowed";
}

{
    # RT 134140 fatalizations
    my %op_pairs = (
        and => { low => 'and', high => '&', regex => qr/&/  },
        or  => { low => 'or',  high => '|', regex => qr/\|/ },
        xor => { low => 'xor', high => '^', regex => qr/\^/ },
    );
    my @combos = (
        { string  => '"abc" & "abc\x{100}"',  op_pair => $op_pairs{and} },
        { string  => '"abc" | "abc\x{100}"',  op_pair => $op_pairs{or}  },
        { string  => '"abc" ^ "abc\x{100}"',  op_pair => $op_pairs{xor} },
        { string  => '"abc\x{100}" & "abc"',  op_pair => $op_pairs{and} },
        { string  => '"abc\x{100}" | "abc"',  op_pair => $op_pairs{or}  },
        { string  => '"abc\x{100}" ^ "abc"',  op_pair => $op_pairs{xor} },

    );

    # Use of strings with code points over 0xFF as arguments to %s operator is not allowed
    for my $h (@combos) {
        my $s1 = "Use of strings with code points over 0xFF as arguments to bitwise";
        my $s2 = "operator is not allowed";
        my $expected  = qr/$s1 $h->{op_pair}->{low} \($h->{op_pair}->{regex}\) $s2/;
        my $description = "$s1 $h->{op_pair}->{low} ($h->{op_pair}->{high}) operator is not allowed";
        local $@;
        eval $h->{string};
        like $@, $expected, $description;
    }
}

{
    # perl #17844 - only visible with valgrind/ASAN
    fresh_perl_is(<<'EOS',
formline X000n^\\0,\\0^\\0for\0,0..10
EOS
                  '',
                  {}, "[perl #17844] access beyond end of block");
}
