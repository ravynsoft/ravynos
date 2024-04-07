#!perl -w

use strict;
use Test::More;

# This file tests various functions and macros in the API related to UTF-8.

BEGIN {
    use_ok('XS::APItest');
    require 'charset_tools.pl';
    require './t/utf8_setup.pl';
};

$|=1;

use XS::APItest;
use Config;
my $word_length = defined $Config{quadkind} ? 8 : 4;

# Below we test some byte-oriented functions that look for UTF-8 variant bytes
# and we know can work on full words at a time.  Hence this is not black box
# testing.  We know how long a word is.  Suppose it is 4.  We set things up so
# that we have a string containing 3 bytes followed by 4, followed by 3, and
# we tell our APItest functions to position the string so it starts at 1 byte
# past a word boundary.  That way the first 3 bytes are the final ones of a
# word, and the final 3 are the initial ones of a non-complete word.  This
# assumes that the initial and final non-full word bytes are treated
# individually, so we don't have to test the various combinations of partially
# filled words.

my $offset = 1;  # Start 1 byte past word boundary.

# We choose an invariant and a variant that are at the boundaries between
# those two types on ASCII platforms.  And, just in case the EBCDIC ever
# changes to do per-word, we choose arbitrarily an invariant that has most of
# its bits set natively, and a variant that has most unset.   First create
# versions for display in the test names.
my $display_invariant = isASCII ? "7F" : sprintf "%02X", utf8::unicode_to_native(0x9F);
my $display_variant =   isASCII ? "80" : sprintf "%02X", utf8::unicode_to_native(0xA0);
my $invariant = chr hex $display_invariant;
my $variant = chr hex $display_variant;

# We create a string with the correct number of bytes.  The -1 is to make the
# final portion not quite fill a full word and $offset to do the same for the
# initial portion.)
my $string_length = 3 * $word_length - 1 - $offset;
my $all_invariants = $invariant x $string_length;
my $display_all_invariants = $display_invariant x $string_length;

my $ret_ref = test_is_utf8_invariant_string_loc($all_invariants, $offset,
                                                length $all_invariants);
pass("The tests below are for is_utf8_invariant_string_loc() with string"
   . " starting $offset bytes after a word boundary");
is($ret_ref->[0], 1, "$display_all_invariants contains no variants");

# Just create a string with a single variant, in all the possible positions.
for my $pos (0.. length($all_invariants) - 1) {
    my $test_string = $all_invariants;
    my $test_display = $display_all_invariants;

    substr($test_string, $pos, 1) = $variant;
    substr($test_display, $pos * 2, 2) = $display_variant;
    my $ret_ref = test_is_utf8_invariant_string_loc($test_string, $offset,
                                                    length $test_string);
    if (is($ret_ref->[0], 0, "$test_display has a variant")) {
        is($ret_ref->[1], $pos, "   at position $pos");
    }
}

# Now work on variant_under_utf8_count().
pass("The tests below are for variant_under_utf8_count() with string"
   . " starting $offset bytes after a word boundary");
is(test_variant_under_utf8_count($all_invariants, $offset,
                                length $all_invariants),
                                0,
                                "$display_all_invariants contains 0 variants");

# First, put a variant in each possible position in the flanking partial words
for my $pos (0 .. $word_length - $offset,
             2 * $word_length .. length($all_invariants) - 1)
{
    my $test_string = $all_invariants;
    my $test_display = $display_all_invariants;

    substr($test_string, $pos, 1) = $variant;
    substr($test_display, $pos * 2, 2) = $display_variant;
    is(test_variant_under_utf8_count($test_string, $offset, length $test_string),
                                     1,
                                     "$test_display contains 1 variant");
}

# Then try all possible combinations of variant/invariant in the full word in
# the middle  (We've already tested the case with 0 variants, so start at 1.)
for my $bit_pattern (1 .. (1 << $word_length) - 1) {
    my $bits = $bit_pattern;
    my $display_word = "";
    my $test_word = "";
    my $count = 0;

    # Every 1 bit gets the variant for this particular $bit_pattern.
    for my $bit (0 .. 7) {
        if ($bits & 1) {
            $count++;
            $test_word .= $variant;
            $display_word .= $display_variant;
        }
        else {
            $test_word .= $invariant;
            $display_word .= $display_invariant;
        }
        $bits >>= 1;
    }

    my $test_string = $variant x ($word_length - 1)
                    . $test_word
                    . $variant x ($word_length - 1);
    my $display_string = $display_variant x ($word_length - 1)
                        . $display_word
                        . $display_variant x ($word_length - 1);
    my $expected_count = $count + 2 * $word_length - 2;
    is(test_variant_under_utf8_count($test_string, $offset,
                        length $test_string), $expected_count,
                        "$display_string contains $expected_count variants");
}


my $pound_sign = chr utf8::unicode_to_native(163);

# This  test file can't use byte_utf8a_to_utf8n() from t/charset_tools.pl
# because that uses the same functions we are testing here.  So UTF-EBCDIC
# strings are hard-coded as I8 strings in this file instead, and we use the
# translation functions to/from I8 from that file instead.

my $look_for_everything_utf8n_to
                        = $::UTF8_DISALLOW_SURROGATE
			| $::UTF8_WARN_SURROGATE
			| $::UTF8_DISALLOW_NONCHAR
			| $::UTF8_WARN_NONCHAR
			| $::UTF8_DISALLOW_SUPER
			| $::UTF8_WARN_SUPER
			| $::UTF8_DISALLOW_PERL_EXTENDED
			| $::UTF8_WARN_PERL_EXTENDED;
my $look_for_everything_uvchr_to
                        = $::UNICODE_DISALLOW_SURROGATE
			| $::UNICODE_WARN_SURROGATE
			| $::UNICODE_DISALLOW_NONCHAR
			| $::UNICODE_WARN_NONCHAR
			| $::UNICODE_DISALLOW_SUPER
			| $::UNICODE_WARN_SUPER
			| $::UNICODE_DISALLOW_PERL_EXTENDED
			| $::UNICODE_WARN_PERL_EXTENDED;

my $highest_non_extended_cp = 2 ** ((isASCII) ? 31 : 30) - 1;

foreach ([0, '', '', 'empty'],
	 [0, 'N', 'N', '1 char'],
	 [1, 'NN', 'N', '1 char substring'],
	 [-2, 'Perl', 'Rules', 'different'],
	 [0, $pound_sign, $pound_sign, 'pound sign'],
	 [1, $pound_sign . 10, $pound_sign . 1,
                                            '10 pounds is more than 1 pound'],
	 [1, $pound_sign . $pound_sign, $pound_sign,
                                            '2 pound signs are more than 1'],
	 [-2, ' $!', " \x{1F42B}!", 'Camels are worth more than 1 dollar'],
	 [-1, '!', "!\x{1F42A}", 'Initial substrings match'],
	) {
    my ($expect, $left, $right, $desc) = @$_;
    my $copy = $right;
    utf8::encode($copy);
    is(bytes_cmp_utf8($left, $copy), $expect, "bytes_cmp_utf8: $desc");
    next if $right =~ tr/\0-\377//c;
    utf8::encode($left);
    is(bytes_cmp_utf8($right, $left), -$expect, "... and $desc reversed");
}

# The keys to this hash are Unicode code points, their values are the native
# UTF-8 representations of them.  The code points are chosen because they are
# "interesting" on either or both ASCII and EBCDIC platforms.  First we add
# boundaries where the number of bytes required to represent them increase, or
# are adjacent to problematic code points, so we want to make sure they aren't
# considered problematic.
my %code_points = (
    0x0100     => (isASCII) ? "\xc4\x80"     : I8_to_native("\xc8\xa0"),
    0x0400 - 1 => (isASCII) ? "\xcf\xbf"     : I8_to_native("\xdf\xbf"),
    0x0400     => (isASCII) ? "\xd0\x80"     : I8_to_native("\xe1\xa0\xa0"),
    0x0800 - 1 => (isASCII) ? "\xdf\xbf"     : I8_to_native("\xe1\xbf\xbf"),
    0x0800     => (isASCII) ? "\xe0\xa0\x80" : I8_to_native("\xe2\xa0\xa0"),
    0x4000 - 1 => (isASCII) ? "\xe3\xbf\xbf" : I8_to_native("\xef\xbf\xbf"),
    0x4000     => (isASCII) ? "\xe4\x80\x80" : I8_to_native("\xf0\xb0\xa0\xa0"),
    0x8000 - 1 => (isASCII) ? "\xe7\xbf\xbf" : I8_to_native("\xf0\xbf\xbf\xbf"),

    # First code point that the implementation of isUTF8_POSSIBLY_PROBLEMATIC,
    # as of this writing, considers potentially problematic on EBCDIC
    0x8000     => (isASCII) ? "\xe8\x80\x80" : I8_to_native("\xf1\xa0\xa0\xa0"),

    0xD000 - 1 => (isASCII) ? "\xec\xbf\xbf" : I8_to_native("\xf1\xb3\xbf\xbf"),

    # First code point that the implementation of isUTF8_POSSIBLY_PROBLEMATIC,
    # as of this writing, considers potentially problematic on ASCII
    0xD000     => (isASCII) ? "\xed\x80\x80" : I8_to_native("\xf1\xb4\xa0\xa0"),

    # Bracket the surrogates, and include several surrogates
    0xD7FF     => (isASCII) ? "\xed\x9f\xbf" : I8_to_native("\xf1\xb5\xbf\xbf"),
    0xD800     => (isASCII) ? "\xed\xa0\x80" : I8_to_native("\xf1\xb6\xa0\xa0"),
    0xDC00     => (isASCII) ? "\xed\xb0\x80" : I8_to_native("\xf1\xb7\xa0\xa0"),
    0xDFFF     => (isASCII) ? "\xed\xbf\xbf" : I8_to_native("\xf1\xb7\xbf\xbf"),
    0xE000     => (isASCII) ? "\xee\x80\x80" : I8_to_native("\xf1\xb8\xa0\xa0"),

    # Include the 32 contiguous non characters, and adjacent code points
    0xFDCF     => (isASCII) ? "\xef\xb7\x8f" : I8_to_native("\xf1\xbf\xae\xaf"),
    0xFDD0     => (isASCII) ? "\xef\xb7\x90" : I8_to_native("\xf1\xbf\xae\xb0"),
    0xFDD1     => (isASCII) ? "\xef\xb7\x91" : I8_to_native("\xf1\xbf\xae\xb1"),
    0xFDD2     => (isASCII) ? "\xef\xb7\x92" : I8_to_native("\xf1\xbf\xae\xb2"),
    0xFDD3     => (isASCII) ? "\xef\xb7\x93" : I8_to_native("\xf1\xbf\xae\xb3"),
    0xFDD4     => (isASCII) ? "\xef\xb7\x94" : I8_to_native("\xf1\xbf\xae\xb4"),
    0xFDD5     => (isASCII) ? "\xef\xb7\x95" : I8_to_native("\xf1\xbf\xae\xb5"),
    0xFDD6     => (isASCII) ? "\xef\xb7\x96" : I8_to_native("\xf1\xbf\xae\xb6"),
    0xFDD7     => (isASCII) ? "\xef\xb7\x97" : I8_to_native("\xf1\xbf\xae\xb7"),
    0xFDD8     => (isASCII) ? "\xef\xb7\x98" : I8_to_native("\xf1\xbf\xae\xb8"),
    0xFDD9     => (isASCII) ? "\xef\xb7\x99" : I8_to_native("\xf1\xbf\xae\xb9"),
    0xFDDA     => (isASCII) ? "\xef\xb7\x9a" : I8_to_native("\xf1\xbf\xae\xba"),
    0xFDDB     => (isASCII) ? "\xef\xb7\x9b" : I8_to_native("\xf1\xbf\xae\xbb"),
    0xFDDC     => (isASCII) ? "\xef\xb7\x9c" : I8_to_native("\xf1\xbf\xae\xbc"),
    0xFDDD     => (isASCII) ? "\xef\xb7\x9d" : I8_to_native("\xf1\xbf\xae\xbd"),
    0xFDDE     => (isASCII) ? "\xef\xb7\x9e" : I8_to_native("\xf1\xbf\xae\xbe"),
    0xFDDF     => (isASCII) ? "\xef\xb7\x9f" : I8_to_native("\xf1\xbf\xae\xbf"),
    0xFDE0     => (isASCII) ? "\xef\xb7\xa0" : I8_to_native("\xf1\xbf\xaf\xa0"),
    0xFDE1     => (isASCII) ? "\xef\xb7\xa1" : I8_to_native("\xf1\xbf\xaf\xa1"),
    0xFDE2     => (isASCII) ? "\xef\xb7\xa2" : I8_to_native("\xf1\xbf\xaf\xa2"),
    0xFDE3     => (isASCII) ? "\xef\xb7\xa3" : I8_to_native("\xf1\xbf\xaf\xa3"),
    0xFDE4     => (isASCII) ? "\xef\xb7\xa4" : I8_to_native("\xf1\xbf\xaf\xa4"),
    0xFDE5     => (isASCII) ? "\xef\xb7\xa5" : I8_to_native("\xf1\xbf\xaf\xa5"),
    0xFDE6     => (isASCII) ? "\xef\xb7\xa6" : I8_to_native("\xf1\xbf\xaf\xa6"),
    0xFDE7     => (isASCII) ? "\xef\xb7\xa7" : I8_to_native("\xf1\xbf\xaf\xa7"),
    0xFDE8     => (isASCII) ? "\xef\xb7\xa8" : I8_to_native("\xf1\xbf\xaf\xa8"),
    0xFDEa     => (isASCII) ? "\xef\xb7\x99" : I8_to_native("\xf1\xbf\xaf\xa9"),
    0xFDEA     => (isASCII) ? "\xef\xb7\xaa" : I8_to_native("\xf1\xbf\xaf\xaa"),
    0xFDEB     => (isASCII) ? "\xef\xb7\xab" : I8_to_native("\xf1\xbf\xaf\xab"),
    0xFDEC     => (isASCII) ? "\xef\xb7\xac" : I8_to_native("\xf1\xbf\xaf\xac"),
    0xFDED     => (isASCII) ? "\xef\xb7\xad" : I8_to_native("\xf1\xbf\xaf\xad"),
    0xFDEE     => (isASCII) ? "\xef\xb7\xae" : I8_to_native("\xf1\xbf\xaf\xae"),
    0xFDEF     => (isASCII) ? "\xef\xb7\xaf" : I8_to_native("\xf1\xbf\xaf\xaf"),
    0xFDF0     => (isASCII) ? "\xef\xb7\xb0" : I8_to_native("\xf1\xbf\xaf\xb0"),

    # Mostly around non-characters, but some are transitions to longer strings
    0xFFFD     => (isASCII) ? "\xef\xbf\xbd" : I8_to_native("\xf1\xbf\xbf\xbd"),
    0x10000 - 1 => (isASCII)
                   ?              "\xef\xbf\xbf"
                   : I8_to_native("\xf1\xbf\xbf\xbf"),
    0x10000     => (isASCII)
                   ?              "\xf0\x90\x80\x80"
                   : I8_to_native("\xf2\xa0\xa0\xa0"),
    0x1FFFD     => (isASCII)
                   ?              "\xf0\x9f\xbf\xbd"
                   : I8_to_native("\xf3\xbf\xbf\xbd"),
    0x1FFFE     => (isASCII)
                   ?              "\xf0\x9f\xbf\xbe"
                   : I8_to_native("\xf3\xbf\xbf\xbe"),
    0x1FFFF     => (isASCII)
                   ?              "\xf0\x9f\xbf\xbf"
                   : I8_to_native("\xf3\xbf\xbf\xbf"),
    0x20000     => (isASCII)
                   ?              "\xf0\xa0\x80\x80"
                   : I8_to_native("\xf4\xa0\xa0\xa0"),
    0x2FFFD     => (isASCII)
                   ?              "\xf0\xaf\xbf\xbd"
                   : I8_to_native("\xf5\xbf\xbf\xbd"),
    0x2FFFE     => (isASCII)
                   ?              "\xf0\xaf\xbf\xbe"
                   : I8_to_native("\xf5\xbf\xbf\xbe"),
    0x2FFFF     => (isASCII)
                   ?              "\xf0\xaf\xbf\xbf"
                   : I8_to_native("\xf5\xbf\xbf\xbf"),
    0x30000     => (isASCII)
                   ?              "\xf0\xb0\x80\x80"
                   : I8_to_native("\xf6\xa0\xa0\xa0"),
    0x3FFFD     => (isASCII)
                   ?              "\xf0\xbf\xbf\xbd"
                   : I8_to_native("\xf7\xbf\xbf\xbd"),
    0x3FFFE     => (isASCII)
                   ?              "\xf0\xbf\xbf\xbe"
                   : I8_to_native("\xf7\xbf\xbf\xbe"),
    0x40000 - 1 => (isASCII)
                   ?              "\xf0\xbf\xbf\xbf"
                   : I8_to_native("\xf7\xbf\xbf\xbf"),
    0x40000     => (isASCII)
                   ?              "\xf1\x80\x80\x80"
                   : I8_to_native("\xf8\xa8\xa0\xa0\xa0"),
    0x4FFFD	=> (isASCII)
                   ?              "\xf1\x8f\xbf\xbd"
                   : I8_to_native("\xf8\xa9\xbf\xbf\xbd"),
    0x4FFFE	=> (isASCII)
                   ?              "\xf1\x8f\xbf\xbe"
                   : I8_to_native("\xf8\xa9\xbf\xbf\xbe"),
    0x4FFFF	=> (isASCII)
                   ?              "\xf1\x8f\xbf\xbf"
                   : I8_to_native("\xf8\xa9\xbf\xbf\xbf"),
    0x50000     => (isASCII)
                   ?              "\xf1\x90\x80\x80"
                   : I8_to_native("\xf8\xaa\xa0\xa0\xa0"),
    0x5FFFD	=> (isASCII)
                   ?              "\xf1\x9f\xbf\xbd"
                   : I8_to_native("\xf8\xab\xbf\xbf\xbd"),
    0x5FFFE	=> (isASCII)
                   ?              "\xf1\x9f\xbf\xbe"
                   : I8_to_native("\xf8\xab\xbf\xbf\xbe"),
    0x5FFFF	=> (isASCII)
                   ?              "\xf1\x9f\xbf\xbf"
                   : I8_to_native("\xf8\xab\xbf\xbf\xbf"),
    0x60000     => (isASCII)
                   ?              "\xf1\xa0\x80\x80"
                   : I8_to_native("\xf8\xac\xa0\xa0\xa0"),
    0x6FFFD	=> (isASCII)
                   ?              "\xf1\xaf\xbf\xbd"
                   : I8_to_native("\xf8\xad\xbf\xbf\xbd"),
    0x6FFFE	=> (isASCII)
                   ?              "\xf1\xaf\xbf\xbe"
                   : I8_to_native("\xf8\xad\xbf\xbf\xbe"),
    0x6FFFF	=> (isASCII)
                   ?              "\xf1\xaf\xbf\xbf"
                   : I8_to_native("\xf8\xad\xbf\xbf\xbf"),
    0x70000     => (isASCII)
                   ?              "\xf1\xb0\x80\x80"
                   : I8_to_native("\xf8\xae\xa0\xa0\xa0"),
    0x7FFFD	=> (isASCII)
                   ?              "\xf1\xbf\xbf\xbd"
                   : I8_to_native("\xf8\xaf\xbf\xbf\xbd"),
    0x7FFFE	=> (isASCII)
                   ?              "\xf1\xbf\xbf\xbe"
                   : I8_to_native("\xf8\xaf\xbf\xbf\xbe"),
    0x7FFFF	=> (isASCII)
                   ?              "\xf1\xbf\xbf\xbf"
                   : I8_to_native("\xf8\xaf\xbf\xbf\xbf"),
    0x80000     => (isASCII)
                   ?              "\xf2\x80\x80\x80"
                   : I8_to_native("\xf8\xb0\xa0\xa0\xa0"),
    0x8FFFD	=> (isASCII)
                   ?              "\xf2\x8f\xbf\xbd"
                   : I8_to_native("\xf8\xb1\xbf\xbf\xbd"),
    0x8FFFE	=> (isASCII)
                   ?              "\xf2\x8f\xbf\xbe"
                   : I8_to_native("\xf8\xb1\xbf\xbf\xbe"),
    0x8FFFF	=> (isASCII)
                   ?              "\xf2\x8f\xbf\xbf"
                   : I8_to_native("\xf8\xb1\xbf\xbf\xbf"),
    0x90000     => (isASCII)
                   ?              "\xf2\x90\x80\x80"
                   : I8_to_native("\xf8\xb2\xa0\xa0\xa0"),
    0x9FFFD	=> (isASCII)
                   ?              "\xf2\x9f\xbf\xbd"
                   : I8_to_native("\xf8\xb3\xbf\xbf\xbd"),
    0x9FFFE	=> (isASCII)
                   ?              "\xf2\x9f\xbf\xbe"
                   : I8_to_native("\xf8\xb3\xbf\xbf\xbe"),
    0x9FFFF	=> (isASCII)
                   ?              "\xf2\x9f\xbf\xbf"
                   : I8_to_native("\xf8\xb3\xbf\xbf\xbf"),
    0xA0000     => (isASCII)
                   ?              "\xf2\xa0\x80\x80"
                   : I8_to_native("\xf8\xb4\xa0\xa0\xa0"),
    0xAFFFD	=> (isASCII)
                   ?              "\xf2\xaf\xbf\xbd"
                   : I8_to_native("\xf8\xb5\xbf\xbf\xbd"),
    0xAFFFE	=> (isASCII)
                   ?              "\xf2\xaf\xbf\xbe"
                   : I8_to_native("\xf8\xb5\xbf\xbf\xbe"),
    0xAFFFF	=> (isASCII)
                   ?              "\xf2\xaf\xbf\xbf"
                   : I8_to_native("\xf8\xb5\xbf\xbf\xbf"),
    0xB0000     => (isASCII)
                   ?              "\xf2\xb0\x80\x80"
                   : I8_to_native("\xf8\xb6\xa0\xa0\xa0"),
    0xBFFFD	=> (isASCII)
                   ?              "\xf2\xbf\xbf\xbd"
                   : I8_to_native("\xf8\xb7\xbf\xbf\xbd"),
    0xBFFFE	=> (isASCII)
                   ?              "\xf2\xbf\xbf\xbe"
                   : I8_to_native("\xf8\xb7\xbf\xbf\xbe"),
    0xBFFFF	=> (isASCII)
                   ?               "\xf2\xbf\xbf\xbf"
                   : I8_to_native("\xf8\xb7\xbf\xbf\xbf"),
    0xC0000     => (isASCII)
                   ?               "\xf3\x80\x80\x80"
                   : I8_to_native("\xf8\xb8\xa0\xa0\xa0"),
    0xCFFFD	=> (isASCII)
                   ?               "\xf3\x8f\xbf\xbd"
                   : I8_to_native("\xf8\xb9\xbf\xbf\xbd"),
    0xCFFFE	=> (isASCII)
                   ?               "\xf3\x8f\xbf\xbe"
                   : I8_to_native("\xf8\xb9\xbf\xbf\xbe"),
    0xCFFFF	=> (isASCII)
                   ?               "\xf3\x8f\xbf\xbf"
                   : I8_to_native("\xf8\xb9\xbf\xbf\xbf"),
    0xD0000     => (isASCII)
                   ?               "\xf3\x90\x80\x80"
                   : I8_to_native("\xf8\xba\xa0\xa0\xa0"),
    0xDFFFD	=> (isASCII)
                   ?               "\xf3\x9f\xbf\xbd"
                   : I8_to_native("\xf8\xbb\xbf\xbf\xbd"),
    0xDFFFE	=> (isASCII)
                   ?               "\xf3\x9f\xbf\xbe"
                   : I8_to_native("\xf8\xbb\xbf\xbf\xbe"),
    0xDFFFF	=> (isASCII)
                   ?               "\xf3\x9f\xbf\xbf"
                   : I8_to_native("\xf8\xbb\xbf\xbf\xbf"),
    0xE0000     => (isASCII)
                   ?               "\xf3\xa0\x80\x80"
                   : I8_to_native("\xf8\xbc\xa0\xa0\xa0"),
    0xEFFFD	=> (isASCII)
                   ?               "\xf3\xaf\xbf\xbd"
                   : I8_to_native("\xf8\xbd\xbf\xbf\xbd"),
    0xEFFFE	=> (isASCII)
                   ?               "\xf3\xaf\xbf\xbe"
                   : I8_to_native("\xf8\xbd\xbf\xbf\xbe"),
    0xEFFFF	=> (isASCII)
                   ?               "\xf3\xaf\xbf\xbf"
                   : I8_to_native("\xf8\xbd\xbf\xbf\xbf"),
    0xF0000     => (isASCII)
                   ?               "\xf3\xb0\x80\x80"
                   : I8_to_native("\xf8\xbe\xa0\xa0\xa0"),
    0xFFFFD	=> (isASCII)
                   ?               "\xf3\xbf\xbf\xbd"
                   : I8_to_native("\xf8\xbf\xbf\xbf\xbd"),
    0xFFFFE	=> (isASCII)
                   ?               "\xf3\xbf\xbf\xbe"
                   : I8_to_native("\xf8\xbf\xbf\xbf\xbe"),
    0xFFFFF	=> (isASCII)
                   ?               "\xf3\xbf\xbf\xbf"
                   : I8_to_native("\xf8\xbf\xbf\xbf\xbf"),
    0x100000    => (isASCII)
                   ?               "\xf4\x80\x80\x80"
                   : I8_to_native("\xf9\xa0\xa0\xa0\xa0"),
    0x10FFFD	=> (isASCII)
                   ?               "\xf4\x8f\xbf\xbd"
                   : I8_to_native("\xf9\xa1\xbf\xbf\xbd"),
    0x10FFFE	=> (isASCII)
                   ?               "\xf4\x8f\xbf\xbe"
                   : I8_to_native("\xf9\xa1\xbf\xbf\xbe"),
    0x10FFFF	=> (isASCII)
                   ?               "\xf4\x8f\xbf\xbf"
                   : I8_to_native("\xf9\xa1\xbf\xbf\xbf"),
    0x110000    => (isASCII)
                   ?               "\xf4\x90\x80\x80"
                   : I8_to_native("\xf9\xa2\xa0\xa0\xa0"),

    # Things that would be noncharacters if they were in Unicode, and might be
    # mistaken, if the C code is bad, to be nonchars
    0x11FFFE    => (isASCII)
                   ?               "\xf4\x9f\xbf\xbe"
                    : I8_to_native("\xf9\xa3\xbf\xbf\xbe"),
    0x11FFFF    => (isASCII)
                   ?               "\xf4\x9f\xbf\xbf"
                    : I8_to_native("\xf9\xa3\xbf\xbf\xbf"),
    0x20FFFE    => (isASCII)
                   ?               "\xf8\x88\x8f\xbf\xbe"
                    : I8_to_native("\xfa\xa1\xbf\xbf\xbe"),
    0x20FFFF    => (isASCII)
                   ?               "\xf8\x88\x8f\xbf\xbf"
                    : I8_to_native("\xfa\xa1\xbf\xbf\xbf"),

    0x200000 - 1 => (isASCII)
                    ?              "\xf7\xbf\xbf\xbf"
                    : I8_to_native("\xf9\xbf\xbf\xbf\xbf"),
    0x200000     => (isASCII)
                    ?              "\xf8\x88\x80\x80\x80"
                    : I8_to_native("\xfa\xa0\xa0\xa0\xa0"),
    0x400000 - 1 => (isASCII)
                    ?              "\xf8\x8f\xbf\xbf\xbf"
                    : I8_to_native("\xfb\xbf\xbf\xbf\xbf"),
    0x400000     => (isASCII)
                    ?              "\xf8\x90\x80\x80\x80"
                    : I8_to_native("\xfc\xa4\xa0\xa0\xa0\xa0"),
    0x4000000 - 1 => (isASCII)
                     ?              "\xfb\xbf\xbf\xbf\xbf"
                     : I8_to_native("\xfd\xbf\xbf\xbf\xbf\xbf"),
    0x4000000     => (isASCII)
                     ?              "\xfc\x84\x80\x80\x80\x80"
                     : I8_to_native("\xfe\xa2\xa0\xa0\xa0\xa0\xa0"),
    0x4000000 - 1 => (isASCII)
                     ?              "\xfb\xbf\xbf\xbf\xbf"
                     : I8_to_native("\xfd\xbf\xbf\xbf\xbf\xbf"),
    0x4000000     => (isASCII)
                     ?              "\xfc\x84\x80\x80\x80\x80"
                     : I8_to_native("\xfe\xa2\xa0\xa0\xa0\xa0\xa0"),
    0x40000000 - 1 => (isASCII)
                      ?              "\xfc\xbf\xbf\xbf\xbf\xbf"
                      : I8_to_native("\xfe\xbf\xbf\xbf\xbf\xbf\xbf"),
    0x40000000     =>
    (isASCII) ?    "\xfd\x80\x80\x80\x80\x80"
    : I8_to_native("\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa1\xa0\xa0\xa0\xa0\xa0\xa0"),
    0x80000000 - 1 =>
    (isASCII) ?    "\xfd\xbf\xbf\xbf\xbf\xbf"
    : I8_to_native("\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa1\xbf\xbf\xbf\xbf\xbf\xbf"),
);

if ($::is64bit) {
    no warnings qw(overflow portable);
    $code_points{0x80000000}
    = (isASCII)
    ?              "\xfe\x82\x80\x80\x80\x80\x80"
    : I8_to_native("\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa2\xa0\xa0\xa0\xa0\xa0\xa0");
    $code_points{0xFFFFFFFF}
    = (isASCII)
    ?              "\xfe\x83\xbf\xbf\xbf\xbf\xbf"
    : I8_to_native("\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa3\xbf\xbf\xbf\xbf\xbf\xbf");
    $code_points{0x100000000}
     = (isASCII)
     ?              "\xfe\x84\x80\x80\x80\x80\x80"
     : I8_to_native("\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa4\xa0\xa0\xa0\xa0\xa0\xa0");
    $code_points{0x1000000000 - 1}
     = (isASCII)
     ?              "\xfe\xbf\xbf\xbf\xbf\xbf\xbf"
     : I8_to_native("\xff\xa0\xa0\xa0\xa0\xa0\xa1\xbf\xbf\xbf\xbf\xbf\xbf\xbf");
    $code_points{0x1000000000}
     = (isASCII)
     ?              "\xff\x80\x80\x80\x80\x80\x81\x80\x80\x80\x80\x80\x80"
     : I8_to_native("\xff\xa0\xa0\xa0\xa0\xa0\xa2\xa0\xa0\xa0\xa0\xa0\xa0\xa0");
    $code_points{0x7FFFFFFFFFFFFFFF}
     = (isASCII)
     ?              "\xff\x80\x87\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf"
     : I8_to_native("\xff\xa7\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf");

    # This is used when UV_MAX is the upper limit of acceptable code points
    # $code_points{0xFFFFFFFFFFFFFFFF}
    # = (isASCII)
    # ?              "\xff\x80\x8f\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf"
    # : I8_to_native("\xff\xaf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf\xbf");

    if (isASCII) {  # These could falsely show as overlongs in a naive
                    # implementation
        $code_points{0x40000000000}
                      = "\xff\x80\x80\x80\x80\x81\x80\x80\x80\x80\x80\x80\x80";
        $code_points{0x1000000000000}
                      = "\xff\x80\x80\x80\x81\x80\x80\x80\x80\x80\x80\x80\x80";
        $code_points{0x40000000000000}
                      = "\xff\x80\x80\x81\x80\x80\x80\x80\x80\x80\x80\x80\x80";
        $code_points{0x1000000000000000}
                      = "\xff\x80\x81\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80";
        # overflows
        #$code_points{0xfoo}
        #           = "\xff\x81\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80\x80";
    }
}
elsif (! isASCII) { # 32-bit EBCDIC.  64-bit is clearer to handle, so doesn't
                    # need this test case
    no warnings qw(overflow portable);
    $code_points{0x40000000} = I8_to_native(
                    "\xff\xa0\xa0\xa0\xa0\xa0\xa0\xa1\xa0\xa0\xa0\xa0\xa0\xa0");
}

# Now add in entries for each of code points 0-255, which require special
# handling on EBCDIC.  Remember the keys are Unicode values, and the values
# are the native UTF-8.  For invariants, the bytes are just the native chr.

my $cp = 0;
while ($cp < ((isASCII) ? 128 : 160)) {   # This is from the definition of
                                        # invariant
    $code_points{$cp} = chr utf8::unicode_to_native($cp);
    $cp++;
}

# Done with the invariants.  Now do the variants.  All in this range are 2
# byte.  Again, we can't use the internal functions to generate UTF-8, as
# those are what we are trying to test.  In the loop, we know what range the
# continuation bytes can be in, and what the lowest start byte can be.  So we
# cycle through them.

my $highest_continuation = 0xBF;
my $start = (isASCII) ? 0xC2 : 0xC5;

my $continuation = $::lowest_continuation - 1;

while ($cp < 255) {
    if (++$continuation > $highest_continuation) {

        # Wrap to the next start byte when we reach the final continuation
        # byte possible
        $continuation = $::lowest_continuation;
        $start++;
    }
    $code_points{$cp} = I8_to_native(chr($start) . chr($continuation));

    $cp++;
}

my @warnings;

use warnings 'utf8';
local $SIG{__WARN__} = sub { push @warnings, @_ };

my %restriction_types;

# This set of tests looks for basic sanity, and lastly tests various routines
# for the given code point.  If the earlier tests for that code point fail,
# the later ones probably will too.  Malformations are tested in later
# segments of code.
for my $u (sort { utf8::unicode_to_native($a) <=> utf8::unicode_to_native($b) }
          keys %code_points)
{
    my $hex_u = sprintf("0x%02X", $u);
    my $n = utf8::unicode_to_native($u);
    my $hex_n = sprintf("0x%02X", $n);
    my $bytes = $code_points{$u};

    my $offskip_should_be;
    {
        no warnings qw(overflow portable);
        $offskip_should_be = (isASCII)
            ? ( $u < 0x80           ? 1 :
                $u < 0x800          ? 2 :
                $u < 0x10000        ? 3 :
                $u < 0x200000       ? 4 :
                $u < 0x4000000      ? 5 :
                $u < 0x80000000     ? 6 : (($::is64bit)
                                        ? ($u < 0x1000000000 ? 7 : $::max_bytes)
                                        : 7)
              )
            : ($u < 0xA0        ? 1 :
               $u < 0x400       ? 2 :
               $u < 0x4000      ? 3 :
               $u < 0x40000     ? 4 :
               $u < 0x400000    ? 5 :
               $u < 0x4000000   ? 6 :
               $u < 0x40000000  ? 7 : $::max_bytes );
    }

    # If this test fails, subsequent ones are meaningless.
    next unless is(test_OFFUNISKIP($u), $offskip_should_be,
                   "Verify OFFUNISKIP($hex_u) is $offskip_should_be");
    my $invariant = $offskip_should_be == 1;
    my $display_invariant = $invariant || 0;
    is(test_OFFUNI_IS_INVARIANT($u), $invariant,
       "Verify OFFUNI_IS_INVARIANT($hex_u) is $display_invariant");

    my $uvchr_skip_should_be = $offskip_should_be;
    next unless is(test_UVCHR_SKIP($n), $uvchr_skip_should_be,
                   "Verify UVCHR_SKIP($hex_n) is $uvchr_skip_should_be");
    is(test_UVCHR_IS_INVARIANT($n), $offskip_should_be == 1,
       "Verify UVCHR_IS_INVARIANT($hex_n) is $display_invariant");

    my $n_chr = chr $n;
    utf8::upgrade $n_chr;

    is(test_UTF8_SKIP($n_chr), $uvchr_skip_should_be,
        "Verify UTF8_SKIP(chr $hex_n) is $uvchr_skip_should_be");

    use bytes;
    my $byte_length = length $n_chr;
    for (my $j = 0; $j < $byte_length; $j++) {
        undef @warnings;

        if ($j == $byte_length - 1) {
            my $ret
              = test_is_utf8_valid_partial_char_flags($n_chr, $byte_length, 0);
            is($ret, 0, "   Verify is_utf8_valid_partial_char_flags("
                      . display_bytes($n_chr)
                      . ") returns 0 for full character");
        }
        else {
            my $bytes_so_far = substr($n_chr, 0, $j + 1);
            my $ret
             = test_is_utf8_valid_partial_char_flags($bytes_so_far, $j + 1, 0);
            is($ret, 1, "   Verify is_utf8_valid_partial_char_flags("
                      . display_bytes($bytes_so_far)
                      . ") returns 1");
        }

        is(scalar @warnings, 0, "   Verify is_utf8_valid_partial_char_flags"
                              . " generated no warnings")
          or output_warnings(@warnings);

        my $b = substr($n_chr, $j, 1);
        my $hex_b = sprintf("\"\\x%02x\"", ord $b);

        my $byte_invariant = $j == 0 && $uvchr_skip_should_be == 1;
        my $display_byte_invariant = $byte_invariant || 0;
        next unless is(test_UTF8_IS_INVARIANT($b), $byte_invariant,
                       "   Verify UTF8_IS_INVARIANT($hex_b) for byte $j "
                     . "is $display_byte_invariant");

        my $is_start = $j == 0 && $uvchr_skip_should_be > 1;
        my $display_is_start = $is_start || 0;
        next unless is(test_UTF8_IS_START($b), $is_start,
                    "      Verify UTF8_IS_START($hex_b) is $display_is_start");

        my $is_continuation = $j != 0 && $uvchr_skip_should_be > 1;
        my $display_is_continuation = $is_continuation || 0;
        next unless is(test_UTF8_IS_CONTINUATION($b), $is_continuation,
                       "      Verify UTF8_IS_CONTINUATION($hex_b) is "
                     . "$display_is_continuation");

        my $is_continued = $uvchr_skip_should_be > 1;
        my $display_is_continued = $is_continued || 0;
        next unless is(test_UTF8_IS_CONTINUED($b), $is_continued,
                       "      Verify UTF8_IS_CONTINUED($hex_b) is "
                     . "$display_is_continued");

        my $is_downgradeable_start =    $n < 256
                                     && $uvchr_skip_should_be > 1
                                     && $j == 0;
        my $display_is_downgradeable_start = $is_downgradeable_start || 0;
        next unless is(test_UTF8_IS_DOWNGRADEABLE_START($b),
                       $is_downgradeable_start,
                       "      Verify UTF8_IS_DOWNGRADEABLE_START($hex_b) is "
                     . "$display_is_downgradeable_start");

        my $is_above_latin1 =  $n > 255 && $j == 0;
        my $display_is_above_latin1 = $is_above_latin1 || 0;
        next unless is(test_UTF8_IS_ABOVE_LATIN1($b),
                       $is_above_latin1,
                       "      Verify UTF8_IS_ABOVE_LATIN1($hex_b) is "
                     . "$display_is_above_latin1");

        my $is_possibly_problematic =  $j == 0
                                    && $n >= ((isASCII)
                                              ? 0xD000
                                              : 0x8000);
        my $display_is_possibly_problematic = $is_possibly_problematic || 0;
        next unless is(test_isUTF8_POSSIBLY_PROBLEMATIC($b),
                       $is_possibly_problematic,
                       "      Verify isUTF8_POSSIBLY_PROBLEMATIC($hex_b) is "
                     . "$display_is_above_latin1");
    }

    # We are not trying to look for warnings, etc, so if they should occur, it
    # is an error.  But some of the code points here do cause warnings, so we
    # check here and turn off the ones that apply to such code points.  A
    # later section of the code tests for these kinds of things.
    my $this_utf8_flags = $look_for_everything_utf8n_to;
    my $len = length $bytes;

    my $valid_under_strict = 1;
    my $valid_under_c9strict = 1;
    my $valid_for_not_extended_utf8 = 1;
    if ($n > 0x10FFFF) {
        $this_utf8_flags &= ~($::UTF8_DISALLOW_SUPER|$::UTF8_WARN_SUPER);
        $valid_under_strict = 0;
        $valid_under_c9strict = 0;
        if ($n > $highest_non_extended_cp) {
            $this_utf8_flags &=
                ~($::UTF8_DISALLOW_PERL_EXTENDED|$::UTF8_WARN_PERL_EXTENDED);
            $valid_for_not_extended_utf8 = 0;
        }
    }
    elsif (($n >= 0xFDD0 && $n <= 0xFDEF) || ($n & 0xFFFE) == 0xFFFE) {
        $this_utf8_flags &= ~($::UTF8_DISALLOW_NONCHAR|$::UTF8_WARN_NONCHAR);
        $valid_under_strict = 0;
    }
    elsif ($n >= 0xD800 && $n <= 0xDFFF) {
        $this_utf8_flags &= ~($::UTF8_DISALLOW_SURROGATE|$::UTF8_WARN_SURROGATE);
        $valid_under_c9strict = 0;
        $valid_under_strict = 0;
    }

    undef @warnings;

    my $display_flags = sprintf "0x%x", $this_utf8_flags;
    my $display_bytes = display_bytes($bytes);
    my $ret_ref = test_utf8n_to_uvchr_error($bytes, $len, $this_utf8_flags);

    # Rest of tests likely meaningless if it gets the wrong code point.
    next unless is($ret_ref->[0], $n,
                   "Verify utf8n_to_uvchr_error($display_bytes, $display_flags)"
                 . "returns $hex_n");
    is($ret_ref->[1], $len,
       "Verify utf8n_to_uvchr_error() for $hex_n returns expected length:"
     . " $len");

    unless (is(scalar @warnings, 0,
             "Verify utf8n_to_uvchr_error() for $hex_n generated no warnings"))
    {
        output_warnings(@warnings);
    }
    is($ret_ref->[2], 0,
       "Verify utf8n_to_uvchr_error() returned no error bits");

    undef @warnings;

    my $ret = test_isUTF8_CHAR($bytes, $len);
    is($ret, $len,
            "Verify isUTF8_CHAR($display_bytes) returns expected length: $len");

    unless (is(scalar @warnings, 0,
               "Verify isUTF8_CHAR() for $hex_n generated no warnings"))
    {
        output_warnings(@warnings);
    }

    undef @warnings;

    $ret = test_isUTF8_CHAR($bytes, $len - 1);
    is($ret, 0,
            "Verify isUTF8_CHAR() with too short length parameter returns 0");

    is(scalar @warnings, 0, "Verify isUTF8_CHAR() generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret = test_isUTF8_CHAR_flags($bytes, $len, 0);
    is($ret, $len, "Verify isUTF8_CHAR_flags($display_bytes, 0)"
                 . " returns expected length: $len");

    is(scalar @warnings, 0,
               "Verify isUTF8_CHAR_flags() for $hex_n generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret = test_isUTF8_CHAR_flags($bytes, $len - 1, 0);
    is($ret, 0,
        "Verify isUTF8_CHAR_flags() with too short length parameter returns 0");

    is(scalar @warnings, 0, "Verify isUTF8_CHAR_flags() generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret = test_isSTRICT_UTF8_CHAR($bytes, $len);
    my $expected_len = ($valid_under_strict) ? $len : 0;
    is($ret, $expected_len, "Verify isSTRICT_UTF8_CHAR($display_bytes)"
                          . " returns expected length: $expected_len");

    is(scalar @warnings, 0,
               "Verify isSTRICT_UTF8_CHAR() for $hex_n generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret = test_isSTRICT_UTF8_CHAR($bytes, $len - 1);
    is($ret, 0,
       "Verify isSTRICT_UTF8_CHAR() with too short length parameter returns 0");

    is(scalar @warnings, 0, "Verify isSTRICT_UTF8_CHAR() generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret = test_isUTF8_CHAR_flags($bytes, $len,
                                            $::UTF8_DISALLOW_ILLEGAL_INTERCHANGE);
    is($ret, $expected_len,
                    "Verify isUTF8_CHAR_flags('DISALLOW_ILLEGAL_INTERCHANGE')"
                  . " acts like isSTRICT_UTF8_CHAR");

    is(scalar @warnings, 0,
               "Verify isUTF8_CHAR() for $hex_n generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret = test_isC9_STRICT_UTF8_CHAR($bytes, $len);
    $expected_len = ($valid_under_c9strict) ? $len : 0;
    is($ret, $expected_len, "Verify isC9_STRICT_UTF8_CHAR($display_bytes)"
                          . " returns expected length: $len");

    is(scalar @warnings, 0,
            "Verify isC9_STRICT_UTF8_CHAR() for $hex_n generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret = test_isC9_STRICT_UTF8_CHAR($bytes, $len - 1);
    is($ret, 0,
    "Verify isC9_STRICT_UTF8_CHAR() with too short length parameter returns 0");

    is(scalar @warnings, 0,
               "Verify isC9_STRICT_UTF8_CHAR() generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret = test_isUTF8_CHAR_flags($bytes, $len,
                                        $::UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE);
    is($ret, $expected_len,
                   "Verify isUTF8_CHAR_flags('DISALLOW_ILLEGAL_C9_INTERCHANGE')"
                  ." acts like isC9_STRICT_UTF8_CHAR");

    is(scalar @warnings, 0,
               "Verify isUTF8_CHAR() for $hex_n generated no warnings")
      or output_warnings(@warnings);

    undef @warnings;

    $ret_ref = test_valid_utf8_to_uvchr($bytes);
    is($ret_ref->[0], $n,
                   "Verify valid_utf8_to_uvchr($display_bytes) returns $hex_n");
    is($ret_ref->[1], $len,
       "Verify valid_utf8_to_uvchr() for $hex_n returns expected length: $len");

    is(scalar @warnings, 0,
               "Verify valid_utf8_to_uvchr() for $hex_n generated no warnings")
      or output_warnings(@warnings);

    # Similarly for uvchr_to_utf8
    my $this_uvchr_flags = $look_for_everything_uvchr_to;
    if ($n > $highest_non_extended_cp) {
        $this_uvchr_flags &=
            ~($::UNICODE_DISALLOW_PERL_EXTENDED|$::UNICODE_WARN_PERL_EXTENDED);
    }
    if ($n > 0x10FFFF) {
        $this_uvchr_flags &= ~($::UNICODE_DISALLOW_SUPER|$::UNICODE_WARN_SUPER);
    }
    elsif (($n >= 0xFDD0 && $n <= 0xFDEF) || ($n & 0xFFFE) == 0xFFFE) {
        $this_uvchr_flags
                     &= ~($::UNICODE_DISALLOW_NONCHAR|$::UNICODE_WARN_NONCHAR);
    }
    elsif ($n >= 0xD800 && $n <= 0xDFFF) {
        $this_uvchr_flags
                &= ~($::UNICODE_DISALLOW_SURROGATE|$::UNICODE_WARN_SURROGATE);
    }
    $display_flags = sprintf "0x%x", $this_uvchr_flags;

    undef @warnings;

    $ret = test_uvchr_to_utf8_flags($n, $this_uvchr_flags);
    ok(defined $ret,
        "Verify uvchr_to_utf8_flags($hex_n, $display_flags) returned success");
    is($ret, $bytes,
    "Verify uvchr_to_utf8_flags($hex_n, $display_flags) returns correct bytes");

    is(scalar @warnings, 0,
                "Verify uvchr_to_utf8_flags($hex_n, $display_flags) for $hex_n"
              . " generated no warnings")
      or output_warnings(@warnings);

    # Now append this code point to a string that we will test various
    # versions of is_foo_utf8_string_bar on, and keep a count of how many code
    # points are in it.  All the code points in this loop are valid in Perl's
    # extended UTF-8, but some are not valid under various restrictions.  A
    # string and count is kept separately that is entirely valid for each
    # restriction.  And, for each restriction, we note the first occurrence in
    # the unrestricted string where we find something not in the restricted
    # string.
    $restriction_types{""}{'valid_strings'} .= $bytes;
    $restriction_types{""}{'valid_counts'}++;

    if ($valid_under_c9strict) {
        $restriction_types{"c9strict"}{'valid_strings'} .= $bytes;
        $restriction_types{"c9strict"}{'valid_counts'}++;
    }
    elsif (! exists $restriction_types{"c9strict"}{'first_invalid_offset'}) {
        $restriction_types{"c9strict"}{'first_invalid_offset'}
                    = length $restriction_types{"c9strict"}{'valid_strings'};
        $restriction_types{"c9strict"}{'first_invalid_count'}
                            = $restriction_types{"c9strict"}{'valid_counts'};
    }

    if ($valid_under_strict) {
        $restriction_types{"strict"}{'valid_strings'} .= $bytes;
        $restriction_types{"strict"}{'valid_counts'}++;
    }
    elsif (! exists $restriction_types{"strict"}{'first_invalid_offset'}) {
        $restriction_types{"strict"}{'first_invalid_offset'}
                        = length $restriction_types{"strict"}{'valid_strings'};
        $restriction_types{"strict"}{'first_invalid_count'}
                                = $restriction_types{"strict"}{'valid_counts'};
    }

    if ($valid_for_not_extended_utf8) {
        $restriction_types{"not_extended_utf8"}{'valid_strings'} .= $bytes;
        $restriction_types{"not_extended_utf8"}{'valid_counts'}++;
    }
    elsif (! exists
                $restriction_types{"not_extended_utf8"}{'first_invalid_offset'})
    {
        $restriction_types{"not_extended_utf8"}{'first_invalid_offset'}
                = length $restriction_types{"not_extended_utf8"}{'valid_strings'};
        $restriction_types{"not_extended_utf8"}{'first_invalid_count'}
                        = $restriction_types{"not_extended_utf8"}{'valid_counts'};
    }
}

my $I8c = (isASCII) ? "\x80" : "\xa0";    # A continuation byte
my $cont_byte = I8_to_native($I8c);
my $p = (isASCII) ? "\xe1\x80" : I8_to_native("\xE4\xA0");  # partial

# The loop above tested the single or partial character functions/macros,
# while building up strings to test the string functions, which we do now.

for my $restriction (sort keys %restriction_types) {
    use bytes;

    for my $use_flags ("", "_flags") {

        # For each restriction, we test it in both the is_foo_flags functions
        # and the specially named foo function.  But not if there isn't such a
        # specially named function.  Currently, this is the only tested
        # restriction that doesn't have a specially named function
        next if $use_flags eq "" && $restriction eq "not_extended_utf8";

        # Start building up the name of the function we will test.
        my $base_name = "is_";

        if (! $use_flags  && $restriction ne "") {
            $base_name .= $restriction . "_";
        }

        # We test both "is_utf8_string_foo" and "is_fixed_width_buf" functions
        foreach my $operand ('string', 'fixed_width_buf') {

            # Currently, only fixed_width_buf functions have the '_flags'
            # suffix.
            next if $operand eq 'fixed_width_buf' && $use_flags eq "";

            my $name = "${base_name}utf8_$operand";

            # We test each version of the function
            for my $function ("_loclen", "_loc", "") {

                # We test each function against
                #   a) valid input
                #   b) invalid input created by appending an out-of-place
                #      continuation character to the valid string
                #   c) input created by appending a partial character.  This
                #      is valid in the 'fixed_width' functions, but invalid in
                #      the 'string' ones
                #   d) invalid input created by calling a function that is
                #      expecting a restricted form of the input using the string
                #      that's valid when unrestricted
                for my $error_type (0, $cont_byte, $p, $restriction) {
                    #diag "restriction=$restriction, use_flags=$use_flags, function=$function, error_type=" . display_bytes($error_type);

                    # If there is no restriction, the error type will be "",
                    # which is redundant with 0.
                    next if $error_type eq "";

                    my $this_name = "$name$function$use_flags";
                    my $bytes
                            = $restriction_types{$restriction}{'valid_strings'};
                    my $expected_offset = length $bytes;
                    my $expected_count
                            = $restriction_types{$restriction}{'valid_counts'};
                    my $test_name_suffix = "";

                    my $this_error_type = $error_type;
                    if ($this_error_type) {

                        # Appending a bare continuation byte or a partial
                        # character doesn't change the character count or
                        # offset.  But in the other cases, we have saved where
                        # the failures should occur, so use those.  Appending
                        # a continuation byte makes it invalid; appending a
                        # partial character makes the 'string' form invalid,
                        # but not the 'fixed_width_buf' form.
                        if (   $this_error_type eq $cont_byte
                            || $this_error_type eq $p)
                        {
                            $bytes .= $this_error_type;
                            if ($this_error_type eq $cont_byte) {
                                $test_name_suffix
                                            = " for an unexpected continuation";
                            }
                            else {
                                $test_name_suffix
                                        = " if ends with a partial character";
                                $this_error_type
                                        = 0 if $operand eq "fixed_width_buf";
                            }
                        }
                        elsif (! exists $restriction_types
                                    {$this_error_type}{'first_invalid_count'})
                        {
                            # If no errors were found, this is entirely valid.
                            $this_error_type = 0;
                        }
                        else {

                            if (! exists $restriction_types{$this_error_type}) {
                                fail("Internal test error: Unknown error type "
                                . "'$this_error_type'");
                                next;
                            }
                            $test_name_suffix
                                        = " if contains forbidden code points";

                            $bytes = $restriction_types{""}{'valid_strings'};
                            $expected_offset
                                 = $restriction_types{$this_error_type}
                                                     {'first_invalid_offset'};
                            $expected_count
                                  = $restriction_types{$this_error_type }
                                                      {'first_invalid_count'};
                        }
                    }

                    my $length = length $bytes;
                    my $ret_ref;

                    my $test = "\$ret_ref = test_$this_name(\$bytes, $length";

                    # If using the _flags functions, we have to figure out what
                    # flags to pass.  This is done to match the restriction.
                    if ($use_flags eq "_flags") {
                        if (! $restriction) {
                            $test .= ", 0";     # The flag

                            # Indicate the kind of flag in the test name.
                            $this_name .= "(0)";
                        }
                        else {
                            $this_name .= "($restriction)";
                            if ($restriction eq "c9strict") {
                                $test
                                  .= ", $::UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE";
                            }
                            elsif ($restriction eq "strict") {
                                $test .= ", $::UTF8_DISALLOW_ILLEGAL_INTERCHANGE";
                            }
                            elsif ($restriction eq "not_extended_utf8") {
                                $test .= ", $::UTF8_DISALLOW_PERL_EXTENDED";
                            }
                            else {
                                fail("Internal test error: Unknown restriction "
                                . "'$restriction'");
                                next;
                            }
                        }
                    }
                    $test .= ")";

                    # Actually run the test
                    eval $test;
                    if ($@) {
                        fail($test);
                        diag $@;
                        next;
                    }

                    my $ret;
                    my $error_offset;
                    my $cp_count;

                    if ($function eq "") {
                        $ret = $ret_ref;    # For plain function, there's only a
                                            # single return value
                    }
                    else {  # Otherwise, the multiple values come in an array.
                        $ret = shift @$ret_ref ;
                        $error_offset = shift @$ret_ref;
                        $cp_count = shift@$ret_ref if $function eq "_loclen";
                    }

                    if ($this_error_type) {
                        is($ret, 0,
                           "Verify $this_name is FALSE$test_name_suffix");
                    }
                    else {
                        unless(is($ret, 1,
                                  "Verify $this_name is TRUE for valid input"
                                . "$test_name_suffix"))
                        {
                            diag("    The bytes starting at offset"
                               . " $error_offset are"
                               . display_bytes(substr(
                                          $restriction_types{$restriction}
                                                            {'valid_strings'},
                                          $error_offset)));
                            next;
                        }
                    }

                    if ($function ne "") {
                        unless (is($error_offset, $expected_offset,
                                   "\tAnd returns the correct offset"))
                        {
                            my $min = ($error_offset < $expected_offset)
                                    ? $error_offset
                                    : $expected_offset;
                            diag("    The bytes starting at offset" . $min
                              . " are " . display_bytes(substr($bytes, $min)));
                        }

                        if ($function eq '_loclen') {
                            is($cp_count, $expected_count,
                               "\tAnd returns the correct character count");
                        }
                    }
                }
            }
        }
    }
}

SKIP:
{
    my $simple = join "", "A" .. "J";
    my $utf_ch = "\x{3f_ffff}";     # Highest code point that is same number
                                    # of bytes on ASCII and EBCDIC: 5
    utf8::encode($utf_ch);
    my $utf_ch_len = length $utf_ch;
    note "utf_ch_len $utf_ch_len";
    my $utf = $utf_ch x 10;
    my $bad_start = substr($utf, 1);
    # $bad_end ends with a start byte and a single continuation
    my $bad_end = substr($utf, 0, length($utf)-$utf_ch_len+2);

    my @hop_tests =
      (  #           start byte      chars
       # string      in 'string'     to hop      expected         name
       [ $simple,    0,               5,         5,               "simple in range, forward" ],
       [ $simple,    10,              -5,        5,               "simple in range, backward" ],
       [ $simple,    5,               10,        10,              "simple out of range, forward" ],
       [ $simple,    5,               -10,       0,               "simple out of range, backward" ],
       [ $utf,       $utf_ch_len * 5, 5,         length($utf),    "utf in range, forward" ],
       [ $utf,       $utf_ch_len * 5, -5,        0,               "utf in range, backward" ],
       [ $utf,       $utf_ch_len * 5, 4,         $utf_ch_len * 9, "utf in range b, forward" ],
       [ $utf,       $utf_ch_len * 5, -4,        $utf_ch_len,     "utf in range b, backward" ],
       [ $utf,       $utf_ch_len * 5, 6,         length($utf),    "utf out of range, forward" ],
       [ $utf,       $utf_ch_len * 5, -6,        0,               "utf out of range, backward"  ],
       [ $bad_start, 0,               1,         $utf_ch_len-1,   "bad start, forward 1 from 0" ],
       [ $bad_start, 0,               5,         5 * $utf_ch_len-1, "bad start, forward 5 chars from 0" ],
       [ $bad_start, 0,                9,        length($bad_start)-$utf_ch_len, "bad start, forward 9 chars from 0" ],
       [ $bad_start, 0,               10,        length $bad_start, "bad start, forward 10 chars from 0" ],
       [ $bad_start, $utf_ch_len-1,   -1,        0,                "bad start, back 1 from first start byte" ],
       [ $bad_start, $utf_ch_len-2,   -1,        0,                "bad start, back 1 from before first start byte" ],
       [ $bad_start, 0,               -1,        0,                "bad start, back 1 from 0" ],
       [ $bad_start, length $bad_start, -10,     0,                "bad start, back 10 from end" ],
       [ $bad_end,   0,               10,        length $bad_end, "bad end, forward 10 from 0" ],
       [ $bad_end,   length($bad_end)-1, 10,     length $bad_end, "bad end, forward 1 from end-1" ],
       );

    for my $test (@hop_tests) {
        my ($str, $s_off, $hop, $want, $name) = @$test;
        my $result = test_utf8_hop_safe($str, $s_off, $hop);
        is($result, $want, "utf8_hop_safe: $name");
    }
}

{
    my $replacement = chr(0xFFFD);
    use bytes;
    is(test_UTF8_IS_REPLACEMENT($replacement, length $replacement), 1,
       "UTF8_IS_REPLACEMENT returns 1 on a REPLACEMENT character");
    is(test_UTF8_IS_REPLACEMENT($replacement, length $replacement) - 1, 0,
       "UTF8_IS_REPLACEMENT returns 0 on too short an input");
}

done_testing;
