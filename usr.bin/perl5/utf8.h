/*    utf8.h
 *
 * This file contains definitions for use with the UTF-8 encoding.  It
 * actually also works with the variant UTF-8 encoding called UTF-EBCDIC, and
 * hides almost all of the differences between these from the caller.  In other
 * words, someone should #include this file, and if the code is being compiled
 * on an EBCDIC platform, things should mostly just work.
 *
 *    Copyright (C) 2000, 2001, 2002, 2005, 2006, 2007, 2009,
 *    2010, 2011 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * A note on nomenclature:  The term UTF-8 is used loosely and inconsistently
 * in Perl documentation.  For one, perl uses an extension of UTF-8 to
 * represent code points that Unicode considers illegal.  For another, ASCII
 * platform UTF-8 is usually conflated with EBCDIC platform UTF-EBCDIC, because
 * outside some of the macros in this this file, the differences are hopefully
 * invisible at the semantic level.
 *
 * UTF-EBCDIC has an isomorphic translation named I8 (for "Intermediate eight")
 * which differs from UTF-8 only in a few details.  It is often useful to
 * translate UTF-EBCDIC into this form for processing.  In general, macros and
 * functions that are expecting their inputs to be either in I8 or UTF-8 are
 * named UTF_foo (without an '8'), to indicate this.
 *
 * Unfortunately there are inconsistencies.
 *
 */

#ifndef PERL_UTF8_H_      /* Guard against recursive inclusion */
#define PERL_UTF8_H_ 1

/*
=for apidoc Ay||utf8ness_t

This typedef is used by several core functions that return PV strings, to
indicate the UTF-8ness of those strings.

(If you write a new function, you probably should instead return the PV in an
SV with the UTF-8 flag of the SV properly set, rather than use this mechanism.)

The possible values this can be are:

=over

=item C<UTF8NESS_YES>

This means the string definitely should be treated as a sequence of
UTF-8-encoded characters.

Most code that needs to handle this typedef should be of the form:

 if (utf8ness_flag == UTF8NESS_YES) {
     treat as utf8;  // like turning on an SV UTF-8 flag
 }

=item C<UTF8NESS_NO>

This means the string definitely should be treated as a sequence of bytes, not
encoded as UTF-8.

=item C<UTF8NESS_IMMATERIAL>

This means it is equally valid to treat the string as bytes, or as UTF-8
characters; use whichever way you want.  This happens when the string consists
entirely of characters which have the same representation whether encoded in
UTF-8 or not.

=item C<UTF8NESS_UNKNOWN>

This means it is unknown how the string should be treated.  No core function
will ever return this value to a non-core caller.  Instead, it is used by the
caller to initialize a variable to a non-legal value.  A typical call will look like:

 utf8ness_t string_is_utf8 = UTF8NESS_UNKNOWN
 const char * string = foo(arg1, arg2, ..., &string_is_utf8);
 if (string_is_utf8 == UTF8NESS_YES) {
    do something for UTF-8;
 }

=back

The following relationships hold between the enum values:

=over

=item S<C<0 E<lt>= I<enum value> E<lt>= UTF8NESS_IMMATERIAL>>

the string may be treated in code as non-UTF8

=item S<C<UTF8NESS_IMMATERIAL E<lt>= <I<enum value>>>

the string may be treated in code as encoded in UTF-8

=back

=cut
*/

typedef enum {
    UTF8NESS_NO               =  0,  /* Definitely not UTF-8 */
    UTF8NESS_IMMATERIAL       =  1,  /* Representation is the same in UTF-8 as
                                        not, so the UTF8ness doesn't actually
                                        matter */
    UTF8NESS_YES              =  2,  /* Defintely is UTF-8, wideness
                                        unspecified */
    UTF8NESS_UNKNOWN          = -1,  /* Undetermined so far */
} utf8ness_t;

/* Use UTF-8 as the default script encoding?
 * Turning this on will break scripts having non-UTF-8 binary
 * data (such as Latin-1) in string literals. */
#ifdef USE_UTF8_SCRIPTS
#    define USE_UTF8_IN_NAMES (!IN_BYTES)
#else
#    define USE_UTF8_IN_NAMES (PL_hints & HINT_UTF8)
#endif

#include "regcharclass.h"
#include "unicode_constants.h"

/* For to_utf8_fold_flags, q.v. */
#define FOLD_FLAGS_LOCALE       0x1
#define FOLD_FLAGS_FULL         0x2
#define FOLD_FLAGS_NOMIX_ASCII  0x4

/*
=for apidoc is_ascii_string

This is a misleadingly-named synonym for L</is_utf8_invariant_string>.
On ASCII-ish platforms, the name isn't misleading: the ASCII-range characters
are exactly the UTF-8 invariants.  But EBCDIC machines have more invariants
than just the ASCII characters, so C<is_utf8_invariant_string> is preferred.

=for apidoc is_invariant_string

This is a somewhat misleadingly-named synonym for L</is_utf8_invariant_string>.
C<is_utf8_invariant_string> is preferred, as it indicates under what conditions
the string is invariant.

=cut
*/
#define is_ascii_string(s, len)     is_utf8_invariant_string(s, len)
#define is_invariant_string(s, len) is_utf8_invariant_string(s, len)

#define uvoffuni_to_utf8_flags(d,uv,flags)                                     \
                               uvoffuni_to_utf8_flags_msgs(d, uv, flags, 0)
#define uvchr_to_utf8(a,b)          uvchr_to_utf8_flags(a,b,0)
#define uvchr_to_utf8_flags(d,uv,flags)                                        \
                                    uvchr_to_utf8_flags_msgs(d,uv,flags, 0)
#define uvchr_to_utf8_flags_msgs(d,uv,flags,msgs)                              \
                uvoffuni_to_utf8_flags_msgs(d,NATIVE_TO_UNI(uv),flags, msgs)
#define utf8_to_uvchr_buf(s, e, lenp)                                          \
            utf8_to_uvchr_buf_helper((const U8 *) (s), (const U8 *) e, lenp)
#define utf8n_to_uvchr(s, len, lenp, flags)                                    \
                                utf8n_to_uvchr_error(s, len, lenp, flags, 0)
#define utf8n_to_uvchr_error(s, len, lenp, flags, errors)                      \
                        utf8n_to_uvchr_msgs(s, len, lenp, flags, errors, 0)

#define utf16_to_utf8(p, d, bytelen, newlen)                                \
                            utf16_to_utf8_base(p, d, bytelen, newlen, 0, 1)
#define utf16_to_utf8_reversed(p, d, bytelen, newlen)                       \
                            utf16_to_utf8_base(p, d, bytelen, newlen, 1, 0)
#define utf8_to_utf16(p, d, bytelen, newlen)                                \
                            utf8_to_utf16_base(p, d, bytelen, newlen, 0, 1)
#define utf8_to_utf16_reversed(p, d, bytelen, newlen)                       \
                            utf8_to_utf16_base(p, d, bytelen, newlen, 1, 0)

#define to_uni_fold(c, p, lenp) _to_uni_fold_flags(c, p, lenp, FOLD_FLAGS_FULL)

#define foldEQ_utf8(s1, pe1, l1, u1, s2, pe2, l2, u2) \
                    foldEQ_utf8_flags(s1, pe1, l1, u1, s2, pe2, l2, u2, 0)
#define FOLDEQ_UTF8_NOMIX_ASCII   (1 << 0)
#define FOLDEQ_LOCALE             (1 << 1)
#define FOLDEQ_S1_ALREADY_FOLDED  (1 << 2)
#define FOLDEQ_S2_ALREADY_FOLDED  (1 << 3)
#define FOLDEQ_S1_FOLDS_SANE      (1 << 4)
#define FOLDEQ_S2_FOLDS_SANE      (1 << 5)

/* This will be described more fully below, but it turns out that the
 * fundamental difference between UTF-8 and UTF-EBCDIC is that the former has
 * the upper 2 bits of a continuation byte be '10', and the latter has the
 * upper 3 bits be '101', leaving 6 and 5 significant bits respectively.
 *
 * It is helpful to know the EBCDIC value on ASCII platforms, mainly to avoid
 * some #ifdef's */
#define UTF_EBCDIC_CONTINUATION_BYTE_INFO_BITS 5

/* See explanation below at 'UTF8_MAXBYTES' */
#define ASCII_PLATFORM_UTF8_MAXBYTES 13

#ifdef EBCDIC

/* The equivalent of the next few macros but implementing UTF-EBCDIC are in the
 * following header file: */
#  include "utfebcdic.h"

#  else	/* ! EBCDIC */

START_EXTERN_C

#  ifndef DOINIT
EXTCONST unsigned char PL_utf8skip[];
#  else
EXTCONST unsigned char PL_utf8skip[] = {
/* 0x00 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
/* 0x10 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
/* 0x20 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
/* 0x30 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
/* 0x40 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
/* 0x50 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
/* 0x60 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
/* 0x70 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* ascii */
/* 0x80 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* bogus: continuation byte */
/* 0x90 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* bogus: continuation byte */
/* 0xA0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* bogus: continuation byte */
/* 0xB0 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* bogus: continuation byte */
/* 0xC0 */ 2,2,				    /* overlong */
/* 0xC2 */     2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* U+0080 to U+03FF */
/* 0xD0 */ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* U+0400 to U+07FF */
/* 0xE0 */ 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, /* U+0800 to U+FFFF */
/* 0xF0 */ 4,4,4,4,4,4,4,4,5,5,5,5,6,6,	    /* above BMP to 2**31 - 1 */
           /* Perl extended (never was official UTF-8).  Up to 36 bit */
/* 0xFE */                             7,
           /* More extended, Up to 72 bits (64-bit + reserved) */
/* 0xFF */                               ASCII_PLATFORM_UTF8_MAXBYTES
};
#  endif

END_EXTERN_C

/*

=for apidoc Am|U8|NATIVE_TO_LATIN1|U8 ch

Returns the Latin-1 (including ASCII and control characters) equivalent of the
input native code point given by C<ch>.  Thus, C<NATIVE_TO_LATIN1(193)> on
EBCDIC platforms returns 65.  These each represent the character C<"A"> on
their respective platforms.  On ASCII platforms no conversion is needed, so
this macro expands to just its input, adding no time nor space requirements to
the implementation.

For conversion of code points potentially larger than will fit in a character,
use L</NATIVE_TO_UNI>.

=for apidoc Am|U8|LATIN1_TO_NATIVE|U8 ch

Returns the native  equivalent of the input Latin-1 code point (including ASCII
and control characters) given by C<ch>.  Thus, C<LATIN1_TO_NATIVE(66)> on
EBCDIC platforms returns 194.  These each represent the character C<"B"> on
their respective platforms.  On ASCII platforms no conversion is needed, so
this macro expands to just its input, adding no time nor space requirements to
the implementation.

For conversion of code points potentially larger than will fit in a character,
use L</UNI_TO_NATIVE>.

=for apidoc Am|UV|NATIVE_TO_UNI|UV ch

Returns the Unicode  equivalent of the input native code point given by C<ch>.
Thus, C<NATIVE_TO_UNI(195)> on EBCDIC platforms returns 67.  These each
represent the character C<"C"> on their respective platforms.  On ASCII
platforms no conversion is needed, so this macro expands to just its input,
adding no time nor space requirements to the implementation.

=for apidoc Am|UV|UNI_TO_NATIVE|UV ch

Returns the native  equivalent of the input Unicode code point  given by C<ch>.
Thus, C<UNI_TO_NATIVE(68)> on EBCDIC platforms returns 196.  These each
represent the character C<"D"> on their respective platforms.  On ASCII
platforms no conversion is needed, so this macro expands to just its input,
adding no time nor space requirements to the implementation.

=cut
*/

#define NATIVE_TO_LATIN1(ch)  (__ASSERT_(FITS_IN_8_BITS(ch)) ((U8) (ch)))
#define LATIN1_TO_NATIVE(ch)  (__ASSERT_(FITS_IN_8_BITS(ch)) ((U8) (ch)))

/* I8 is an intermediate version of UTF-8 used only in UTF-EBCDIC.  We thus
 * consider it to be identical to UTF-8 on ASCII platforms.  Strictly speaking
 * UTF-8 and UTF-EBCDIC are two different things, but we often conflate them
 * because they are 8-bit encodings that serve the same purpose in Perl, and
 * rarely do we need to distinguish them.  The term "NATIVE_UTF8" applies to
 * whichever one is applicable on the current platform */
#define NATIVE_UTF8_TO_I8(ch)  (__ASSERT_(FITS_IN_8_BITS(ch)) ((U8) (ch)))
#define I8_TO_NATIVE_UTF8(ch)  (__ASSERT_(FITS_IN_8_BITS(ch)) ((U8) (ch)))

#define UNI_TO_NATIVE(ch)        ((UV) ASSERT_NOT_PTR(ch))
#define NATIVE_TO_UNI(ch)        ((UV) ASSERT_NOT_PTR(ch))

/*

 The following table is from Unicode 3.2, plus the Perl extensions for above
 U+10FFFF

 Code Points		1st Byte  2nd Byte  3rd    4th     5th     6th       7th   8th-13th

   U+0000..U+007F	00..7F
   U+0080..U+07FF     * C2..DF    80..BF
   U+0800..U+0FFF	E0      * A0..BF  80..BF
   U+1000..U+CFFF       E1..EC    80..BF  80..BF
   U+D000..U+D7FF       ED        80..9F  80..BF
   U+D800..U+DFFF       ED        A0..BF  80..BF  (surrogates)
   U+E000..U+FFFF       EE..EF    80..BF  80..BF
  U+10000..U+3FFFF	F0      * 90..BF  80..BF  80..BF
  U+40000..U+FFFFF	F1..F3    80..BF  80..BF  80..BF
 U+100000..U+10FFFF	F4        80..8F  80..BF  80..BF
    Below are above-Unicode code points
 U+110000..U+13FFFF	F4        90..BF  80..BF  80..BF
 U+110000..U+1FFFFF	F5..F7    80..BF  80..BF  80..BF
 U+200000..U+FFFFFF     F8      * 88..BF  80..BF  80..BF  80..BF
U+1000000..U+3FFFFFF    F9..FB    80..BF  80..BF  80..BF  80..BF
U+4000000..U+3FFFFFFF    FC     * 84..BF  80..BF  80..BF  80..BF  80..BF
U+40000000..U+7FFFFFFF   FD       80..BF  80..BF  80..BF  80..BF  80..BF
U+80000000..U+FFFFFFFFF  FE     * 82..BF  80..BF  80..BF  80..BF  80..BF    80..BF
U+1000000000..           FF       80..BF  80..BF  80..BF  80..BF  80..BF  * 81..BF  80..BF

Note the gaps before several of the byte entries above marked by '*'.  These are
caused by legal UTF-8 avoiding non-shortest encodings: it is technically
possible to UTF-8-encode a single code point in different ways, but that is
explicitly forbidden, and the shortest possible encoding should always be used
(and that is what Perl does).  The non-shortest ones are called 'overlongs'.

Another way to look at it, as bits:

                  Code Points      1st Byte   2nd Byte   3rd Byte   4th Byte

                        0aaa aaaa  0aaa aaaa
              0000 0bbb bbaa aaaa  110b bbbb  10aa aaaa
              cccc bbbb bbaa aaaa  1110 cccc  10bb bbbb  10aa aaaa
 00 000d ddcc cccc bbbb bbaa aaaa  1111 0ddd  10cc cccc  10bb bbbb  10aa aaaa

As you can see, the continuation bytes all begin with C<10>, and the
leading bits of the start byte tell how many bytes there are in the
encoded character.

Perl's extended UTF-8 means we can have start bytes up through FF, though any
beginning with FF yields a code point that is too large for 32-bit ASCII
platforms.  FF signals to use 13 bytes for the encoded character.  This breaks
the paradigm that the number of leading bits gives how many total bytes there
are in the character. */

/* This is the number of low-order bits a continuation byte in a UTF-8 encoded
 * sequence contributes to the specification of the code point.  In the bit
 * maps above, you see that the first 2 bits are a constant '10', leaving 6 of
 * real information */
#  define UTF_CONTINUATION_BYTE_INFO_BITS 6

/* ^? is defined to be DEL on ASCII systems.  See the definition of toCTRL()
 * for more */
#  define QUESTION_MARK_CTRL  DEL_NATIVE

#endif /* EBCDIC vs ASCII */

/* It turns out that in a number of cases, that handling ASCII vs EBCDIC is a
 * matter of being off-by-one.  So this is a convenience macro, used to avoid
 * some #ifdefs. */
#define ONE_IF_EBCDIC_ZERO_IF_NOT                                           \
 (UTF_CONTINUATION_BYTE_INFO_BITS == UTF_EBCDIC_CONTINUATION_BYTE_INFO_BITS)

/* Since the significant bits in a continuation byte are stored in the
 * least-significant positions, we often find ourselves shifting by that
 * amount.  This is a clearer name in such situations */
#define UTF_ACCUMULATION_SHIFT  UTF_CONTINUATION_BYTE_INFO_BITS

/* 2**info_bits - 1.  This masks out all but the bits that carry real
 * information in a continuation byte.  This turns out to be 0x3F in UTF-8,
 * 0x1F in UTF-EBCDIC. */
#define UTF_CONTINUATION_MASK                                               \
                        ((U8) nBIT_MASK(UTF_CONTINUATION_BYTE_INFO_BITS))

/* For use in UTF8_IS_CONTINUATION().  This turns out to be 0xC0 in UTF-8,
 * E0 in UTF-EBCDIC */
#define UTF_IS_CONTINUATION_MASK    ((U8) (0xFF << UTF_ACCUMULATION_SHIFT))

/* This defines the bits that are to be in the continuation bytes of a
 * multi-byte UTF-8 encoded character that mark it is a continuation byte.
 * This turns out to be 0x80 in UTF-8, 0xA0 in UTF-EBCDIC.  (khw doesn't know
 * the underlying reason that B0 works here, except it just happens to work.
 * One could solve for two linear equations and come up with it.) */
#define UTF_CONTINUATION_MARK       (UTF_IS_CONTINUATION_MASK & 0xB0)

/* This value is clearer in some contexts */
#define UTF_MIN_CONTINUATION_BYTE  UTF_CONTINUATION_MARK

/* Is the byte 'c' part of a multi-byte UTF8-8 encoded sequence, and not the
 * first byte thereof? */
#define UTF8_IS_CONTINUATION(c)     (__ASSERT_(FITS_IN_8_BITS(c))           \
            (((NATIVE_UTF8_TO_I8(c) & UTF_IS_CONTINUATION_MASK)             \
                                                == UTF_CONTINUATION_MARK)))

/* Is the representation of the Unicode code point 'cp' the same regardless of
 * being encoded in UTF-8 or not? This is a fundamental property of
 * UTF-8,EBCDIC */
#define OFFUNI_IS_INVARIANT(c)                                              \
                        (((WIDEST_UTYPE)(c)) < UTF_MIN_CONTINUATION_BYTE)

/*
=for apidoc Am|bool|UVCHR_IS_INVARIANT|UV cp

Evaluates to 1 if the representation of code point C<cp> is the same whether or
not it is encoded in UTF-8; otherwise evaluates to 0.  UTF-8 invariant
characters can be copied as-is when converting to/from UTF-8, saving time.
C<cp> is Unicode if above 255; otherwise is platform-native.

=cut
 */
#define UVCHR_IS_INVARIANT(cp)  (OFFUNI_IS_INVARIANT(NATIVE_TO_UNI(cp)))

/* This defines the 1-bits that are to be in the first byte of a multi-byte
 * UTF-8 encoded character that mark it as a start byte and give the number of
 * bytes that comprise the character. 'len' is that number.
 *
 * To illustrate: len = 2 => ((U8) ~ 0b0011_1111) or 1100_0000
 *                      7 => ((U8) ~ 0b0000_0001) or 1111_1110
 *                    > 7 =>  0xFF
 *
 * This is not to be used on a single-byte character.  As in many places in
 * perl, U8 must be 8 bits
 */
#define UTF_START_MARK(len) ((U8) ~(0xFF >> (len)))

/* Masks out the initial one bits in a start byte, leaving the following 0 bit
 * and the real data bits.  'len' is the number of bytes in the multi-byte
 * sequence that comprises the character.
 *
 * To illustrate: len = 2 => 0b0011_1111 works on start byte 110xxxxx
 *                      6 => 0b0000_0011 works on start byte 1111110x
 *                   >= 7 => There are no data bits in the start byte
 * Note that on ASCII platforms, this can be passed a len=1 byte; and all the
 * real data bits will be returned:
                  len = 1 => 0b0111_1111
 * This isn't true on EBCDIC platforms, where some len=1 bytes are of the form
 * 0b101x_xxxx, so this can't be used there on single-byte characters. */
#define UTF_START_MASK(len) (0xFF >> (len))

/*

=for apidoc AmnU|STRLEN|UTF8_MAXBYTES

The maximum width of a single UTF-8 encoded character, in bytes.

NOTE: Strictly speaking Perl's UTF-8 should not be called UTF-8 since UTF-8
is an encoding of Unicode, and Unicode's upper limit, 0x10FFFF, can be
expressed with 4 bytes.  However, Perl thinks of UTF-8 as a way to encode
non-negative integers in a binary format, even those above Unicode.

=cut

The start byte 0xFE, never used in any ASCII platform UTF-8 specification, has
an obvious meaning, namely it has its upper 7 bits set, so it should start a
sequence of 7 bytes.  And in fact, this is exactly what standard UTF-EBCDIC
does.

The start byte FF, on the other hand could have several different plausible
meanings:
  1) The meaning in standard UTF-EBCDIC, namely as an FE start byte, with the
     bottom bit that should be a fixed '0' to form FE, instead acting as an
     info bit, 0 or 1.
  2) That the sequence should have exactly 8 bytes.
  3) That the next byte is to be treated as a sort of extended start byte,
     which in combination with this one gives the total length of the sequence.
     There are published UTF-8 extensions that do this, some string together
     multiple initial FF start bytes to achieve arbitrary precision.
  4) That the sequence has exactly n bytes, where n is what the implementation
     chooses.

Perl has chosen 4).
The goal is to be able to represent 64-bit values in UTF-8 or UTF-EBCDIC.  That
rules out items 1) and 2).  Item 3) has the deal-breaking disadvantage of
requiring one to read more than one byte to determine the total length of the
sequence.  So in Perl, a start byte of FF indicates a UTF-8 string consisting
of the start byte, plus enough continuation bytes to encode a 64 bit value.
This turns out to be 13 total bytes in UTF-8 and 14 in UTF-EBCDIC.  This is
because we get zero info bits from the start byte, plus
    12 * 6 bits of info per continuation byte (could encode 72-bit numbers) on
                UTF-8 (khw knows not why 11, which would encode 66 bits wasn't
                chosen instead); and
    13 * 5 bits of info per byte (could encode 65-bit numbers) on UTF-EBCDIC

The disadvantages of this method are:
  1) There's potentially a lot of wasted bytes for all but the largest values.
     For example, something that could be represented by 7 continuation bytes,
     instead requires the full 12 or 13.
  2) There would be problems should larger values, 128-bit say, ever need to be
     represented.

WARNING: This number must be in sync with the value in
regen/charset_translations.pl. */
#define UTF8_MAXBYTES                                                       \
                (ASCII_PLATFORM_UTF8_MAXBYTES + ONE_IF_EBCDIC_ZERO_IF_NOT)

/* Calculate how many bytes are necessary to represent a value whose most
 * significant 1 bit is in bit position 'pos' of the word.  For 0x1, 'pos would
 * be 0; and for 0x400, 'pos' would be 10, and the result would be:
 *  EBCDIC floor((-1 + (10 + 5 - 1 - 1)) / (5 - 1))
 *       = floor((-1 + (13)) / 4)
 *       = floor(12 / 4)
 *       = 3
 *  ASCII  floor(( 0 + (10 + 6 - 1 - 1)) / (6 - 1))
 *       = floor(14 / 5)
 *       = 2
 * The reason this works is because the number of bits needed to represent a
 * value is proportional to (UTF_CONTINUATION_BYTE_INFO_BITS - 1).  The -1 is
 * because each new continuation byte removes one bit of information from the
 * start byte.
 *
 * This is a step function (we need to allocate a full extra byte if we
 * overflow by just a single bit)
 *
 * The caller is responsible for making sure 'pos' is at least 8 (occupies 9
 * bits), as it breaks down at the lower edge.  At the high end, if it returns
 * 8 or more, Perl instead anomalously uses MAX_BYTES, so this would be wrong.
 * */
#define UNISKIP_BY_MSB_(pos)                                                \
  ( ( -ONE_IF_EBCDIC_ZERO_IF_NOT  /* platform break pos's are off-by-one */ \
     + (pos) + ((UTF_CONTINUATION_BYTE_INFO_BITS - 1) - 1))  /* Step fcn */ \
   / (UTF_CONTINUATION_BYTE_INFO_BITS - 1))             /* take floor of */

/* Compute the number of UTF-8 bytes required for representing the input uv,
 * which must be a Unicode, not native value.
 *
 * This uses msbit_pos() which doesn't work on NUL, and UNISKIP_BY_MSB_ breaks
 * down for small code points.  So first check if the input is invariant to get
 * around that, and use a helper for high code points to accommodate the fact
 * that above 7 btyes, the value is anomalous.  The helper is empty on
 * platforms that don't go that high */
#define OFFUNISKIP(uv)                                                      \
    ((OFFUNI_IS_INVARIANT(uv))                                              \
     ? 1                                                                    \
     : (OFFUNISKIP_helper_(uv) UNISKIP_BY_MSB_(msbit_pos(uv))))

/* We need to go to MAX_BYTES when we can't represent 'uv' by the number of
 * information bits in 6 continuation bytes (when we get to 6, the start byte
 * has no information bits to add to the total).  But on 32-bit ASCII
 * platforms, that doesn't happen until 6*6 bits, so on those platforms, this
 * will always be false */
#if UVSIZE * CHARBITS > (6 * UTF_CONTINUATION_BYTE_INFO_BITS)
#  define HAS_EXTRA_LONG_UTF8
#  define OFFUNISKIP_helper_(uv)                                            \
     UNLIKELY(uv > nBIT_UMAX(6 * UTF_CONTINUATION_BYTE_INFO_BITS))          \
      ? UTF8_MAXBYTES :
#else
#  define OFFUNISKIP_helper_(uv)
#endif

/*

=for apidoc Am|STRLEN|UVCHR_SKIP|UV cp
returns the number of bytes required to represent the code point C<cp> when
encoded as UTF-8.  C<cp> is a native (ASCII or EBCDIC) code point if less than
255; a Unicode code point otherwise.

=cut
 */
#define UVCHR_SKIP(uv)  OFFUNISKIP(NATIVE_TO_UNI(uv))

#define NATIVE_SKIP(uv) UVCHR_SKIP(uv)  /* Old terminology */

/* Most code which says UNISKIP is really thinking in terms of native code
 * points (0-255) plus all those beyond.  This is an imprecise term, but having
 * it means existing code continues to work.  For precision, use UVCHR_SKIP,
 * NATIVE_SKIP, or OFFUNISKIP */
#define UNISKIP(uv)   UVCHR_SKIP(uv)

/* Compute the start byte for a given code point.  This requires the log2 of
 * the code point, which is hard to compute at compile time, which this macro
 * wants to be.  (Perhaps deBruijn sequences could be used.)  So a parameter
 * for the number of bits the value occupies is passed in, which the programmer
 * has had to figure out to get compile-time effect.  And asserts are used to
 * make sure the value is correct.
 *
 * Since we are interested only in the start byte, we ignore the lower bits
 * accounted for by the continuation bytes.  Each continuation byte eats up
 * UTF_CONTINUATION_BYTE_INFO_BITS bits, so the number of continuation bytes
 * needed is floor(bits / UTF_CONTINUATION_BYTE_INFO_BITS).  That number is fed
 * to UTF_START_MARK() to get the upper part of the start byte.  The left over
 * bits form the lower part which is OR'd with the mark
 *
 * Note that on EBCDIC platforms, this is actually the I8 */
#define UTF_START_BYTE(uv, bits)                                            \
           (__ASSERT_((uv) >> ((bits) - 1)) /* At least 'bits' */           \
            __ASSERT_(((uv) & ~nBIT_MASK(bits)) == 0) /* No extra bits */   \
              UTF_START_MARK(UNISKIP_BY_MSB_((bits) - 1))                   \
            | ((uv) >> (((bits) / UTF_CONTINUATION_BYTE_INFO_BITS)          \
                                * UTF_CONTINUATION_BYTE_INFO_BITS)))

/* Compute the first continuation byte for a given code point.  This is mostly
 * for compile-time, so how many bits it occupies is also passed in).
 *
 * We are interested in the first continuation byte, so we ignore the lower
 * bits accounted for by the rest of the continuation bytes by right shifting
 * out their info bit, and mask out the higher bits that will go into the start
 * byte.
 *
 * Note that on EBCDIC platforms, this is actually the I8 */
#define UTF_FIRST_CONT_BYTE(uv, bits)                                       \
   (__ASSERT_((uv) >> ((bits) - 1)) /* At least 'bits' */                   \
    __ASSERT_(((uv) & ~nBIT_MASK(bits)) == 0) /* No extra bits */           \
       UTF_CONTINUATION_MARK                                                \
    | (   UTF_CONTINUATION_MASK                                             \
       & ((uv) >> ((((bits) / UTF_CONTINUATION_BYTE_INFO_BITS) - 1)         \
                            * UTF_CONTINUATION_BYTE_INFO_BITS))))

#define UTF_MIN_START_BYTE  UTF_START_BYTE(UTF_MIN_CONTINUATION_BYTE, 8)

/* Is the byte 'c' the first byte of a multi-byte UTF8-8 encoded sequence?
 * This excludes invariants (they are single-byte).  It also excludes the
 * illegal overlong sequences that begin with C0 and C1 on ASCII platforms, and
 * C0-C4 I8 start bytes on EBCDIC ones.  On EBCDIC E0 can't start a
 * non-overlong sequence, so we define a base macro and for those platforms,
 * extend it to also exclude E0 */
#define UTF8_IS_START_base(c)    (__ASSERT_(FITS_IN_8_BITS(c))              \
                             (NATIVE_UTF8_TO_I8(c) >= UTF_MIN_START_BYTE))
#ifdef EBCDIC
#  define UTF8_IS_START(c)                                                  \
                (UTF8_IS_START_base(c) && (c) != I8_TO_NATIVE_UTF8(0xE0))
#else
#  define UTF8_IS_START(c)  UTF8_IS_START_base(c)
#endif

#define UTF_MIN_ABOVE_LATIN1_BYTE  UTF_START_BYTE(0x100, 9)

/* Is the UTF8-encoded byte 'c' the first byte of a sequence of bytes that
 * represent a code point > 255? */
#define UTF8_IS_ABOVE_LATIN1(c)     (__ASSERT_(FITS_IN_8_BITS(c))           \
                        (NATIVE_UTF8_TO_I8(c) >= UTF_MIN_ABOVE_LATIN1_BYTE))

/* Is the UTF8-encoded byte 'c' the first byte of a two byte sequence?  Use
 * UTF8_IS_NEXT_CHAR_DOWNGRADEABLE() instead if the input isn't known to
 * be well-formed. */
#define UTF8_IS_DOWNGRADEABLE_START(c)	(__ASSERT_(FITS_IN_8_BITS(c))       \
                inRANGE_helper_(U8, NATIVE_UTF8_TO_I8(c),                   \
                        UTF_MIN_START_BYTE, UTF_MIN_ABOVE_LATIN1_BYTE - 1))

/* The largest code point representable by two UTF-8 bytes on this platform.
 * The binary for that code point is:
 *      1101_1111 10xx_xxxx in UTF-8, and
 *      1101_1111 101y_yyyy in UTF-EBCDIC I8.
 * where both x and y are 1, and shown this way to indicate there is one more x
 * than there is y.  The number of x and y bits are their platform's respective
 * UTF_CONTINUATION_BYTE_INFO_BITS.  Squeezing out the bits that don't
 * contribute to the value, these evaluate to:
 *      1_1111 xx_xxxx in UTF-8, and
 *      1_1111 y_yyyy in UTF-EBCDIC I8.
 * or, the maximum value of an unsigned with (5 + info_bit_count) bits */
#define MAX_UTF8_TWO_BYTE  nBIT_UMAX(5 + UTF_CONTINUATION_BYTE_INFO_BITS)

/* The largest code point representable by two UTF-8 bytes on any platform that
 * Perl runs on. */
#define MAX_PORTABLE_UTF8_TWO_BYTE                                          \
                nBIT_UMAX(5 + MIN(       UTF_CONTINUATION_BYTE_INFO_BITS,   \
                                  UTF_EBCDIC_CONTINUATION_BYTE_INFO_BITS))

/*

=for apidoc AmnU|STRLEN|UTF8_MAXBYTES_CASE

The maximum number of UTF-8 bytes a single Unicode character can
uppercase/lowercase/titlecase/fold into.

=cut

 * Unicode guarantees that the maximum expansion is UTF8_MAX_FOLD_CHAR_EXPAND
 * characters, but any above-Unicode code point will fold to itself, so we only
 * have to look at the expansion of the maximum Unicode code point.  But this
 * number may be less than the space occupied by a very large code point under
 * Perl's extended UTF-8.  We have to make it large enough to fit any single
 * character.  (It turns out that ASCII and EBCDIC differ in which is larger)
 *
=cut
*/
#define UTF8_MAXBYTES_CASE	                                            \
        MAX(UTF8_MAXBYTES, UTF8_MAX_FOLD_CHAR_EXPAND * UNISKIP_BY_MSB_(20))

/* Rest of these are attributes of Unicode and perl's internals rather than the
 * encoding, or happen to be the same in both ASCII and EBCDIC (at least at
 * this level; the macros that some of these call may have different
 * definitions in the two encodings */

/* In domain restricted to ASCII, these may make more sense to the reader than
 * the ones with Latin1 in the name */
#define NATIVE_TO_ASCII(ch)      NATIVE_TO_LATIN1(ch)
#define ASCII_TO_NATIVE(ch)      LATIN1_TO_NATIVE(ch)

/* More or less misleadingly-named defines, retained for back compat */
#define NATIVE_TO_UTF(ch)        NATIVE_UTF8_TO_I8(ch)
#define NATIVE_TO_I8(ch)         NATIVE_UTF8_TO_I8(ch)
#define UTF_TO_NATIVE(ch)        I8_TO_NATIVE_UTF8(ch)
#define I8_TO_NATIVE(ch)         I8_TO_NATIVE_UTF8(ch)
#define NATIVE8_TO_UNI(ch)       NATIVE_TO_LATIN1(ch)

/* Adds a UTF8 continuation byte 'new' of information to a running total code
 * point 'old' of all the continuation bytes so far.  This is designed to be
 * used in a loop to convert from UTF-8 to the code point represented.  Note
 * that this is asymmetric on EBCDIC platforms, in that the 'new' parameter is
 * the UTF-EBCDIC byte, whereas the 'old' parameter is a Unicode (not EBCDIC)
 * code point in process of being generated */
#define UTF8_ACCUMULATE(old, new) (__ASSERT_(FITS_IN_8_BITS(new))              \
                                   ((old) << UTF_ACCUMULATION_SHIFT)           \
                                   | ((NATIVE_UTF8_TO_I8(new))                 \
                                       & UTF_CONTINUATION_MASK))

/* This works in the face of malformed UTF-8. */
#define UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(s, e)                                 \
                                       (   ( (e) - (s) > 1)                   \
                                        && UTF8_IS_DOWNGRADEABLE_START(*(s))  \
                                        && UTF8_IS_CONTINUATION(*((s)+1)))

/* Longer, but more accurate name */
#define UTF8_IS_ABOVE_LATIN1_START(c)     UTF8_IS_ABOVE_LATIN1(c)

/* Convert a UTF-8 variant Latin1 character to a native code point value.
 * Needs just one iteration of accumulate.  Should be used only if it is known
 * that the code point is < 256, and is not UTF-8 invariant.  Use the slower
 * but more general TWO_BYTE_UTF8_TO_NATIVE() which handles any code point
 * representable by two bytes (which turns out to be up through
 * MAX_PORTABLE_UTF8_TWO_BYTE).  The two parameters are:
 *  HI: a downgradable start byte;
 *  LO: continuation.
 * */
#define EIGHT_BIT_UTF8_TO_NATIVE(HI, LO)                                        \
    ( __ASSERT_(UTF8_IS_DOWNGRADEABLE_START(HI))                                \
      __ASSERT_(UTF8_IS_CONTINUATION(LO))                                       \
     LATIN1_TO_NATIVE(UTF8_ACCUMULATE((                                         \
                            NATIVE_UTF8_TO_I8(HI) & UTF_START_MASK(2)), (LO))))

/* Convert a two (not one) byte utf8 character to a native code point value.
 * Needs just one iteration of accumulate.  Should not be used unless it is
 * known that the two bytes are legal: 1) two-byte start, and 2) continuation.
 * Note that the result can be larger than 255 if the input character is not
 * downgradable */
#define TWO_BYTE_UTF8_TO_NATIVE(HI, LO) \
    (__ASSERT_(FITS_IN_8_BITS(HI))                                              \
     __ASSERT_(FITS_IN_8_BITS(LO))                                              \
     __ASSERT_(PL_utf8skip[(U8) HI] == 2)                                            \
     __ASSERT_(UTF8_IS_CONTINUATION(LO))                                        \
     UNI_TO_NATIVE(UTF8_ACCUMULATE((NATIVE_UTF8_TO_I8(HI) & UTF_START_MASK(2)), \
                                   (LO))))

/* Should never be used, and be deprecated */
#define TWO_BYTE_UTF8_TO_UNI(HI, LO) NATIVE_TO_UNI(TWO_BYTE_UTF8_TO_NATIVE(HI, LO))

/*

=for apidoc Am|STRLEN|UTF8SKIP|char* s
returns the number of bytes a non-malformed UTF-8 encoded character whose first
(perhaps only) byte is pointed to by C<s>.

If there is a possibility of malformed input, use instead:

=over

=item C<L</UTF8_SAFE_SKIP>> if you know the maximum ending pointer in the
buffer pointed to by C<s>; or

=item C<L</UTF8_CHK_SKIP>> if you don't know it.

=back

It is better to restructure your code so the end pointer is passed down so that
you know what it actually is at the point of this call, but if that isn't
possible, C<L</UTF8_CHK_SKIP>> can minimize the chance of accessing beyond the end
of the input buffer.

=cut
 */
#define UTF8SKIP(s)  PL_utf8skip[*(const U8*)(ASSERT_IS_PTR(s))]

/*
=for apidoc Am|STRLEN|UTF8_SKIP|char* s
This is a synonym for C<L</UTF8SKIP>>

=cut
*/

#define UTF8_SKIP(s) UTF8SKIP(s)

/*
=for apidoc Am|STRLEN|UTF8_CHK_SKIP|char* s

This is a safer version of C<L</UTF8SKIP>>, but still not as safe as
C<L</UTF8_SAFE_SKIP>>.  This version doesn't blindly assume that the input
string pointed to by C<s> is well-formed, but verifies that there isn't a NUL
terminating character before the expected end of the next character in C<s>.
The length C<UTF8_CHK_SKIP> returns stops just before any such NUL.

Perl tends to add NULs, as an insurance policy, after the end of strings in
SV's, so it is likely that using this macro will prevent inadvertent reading
beyond the end of the input buffer, even if it is malformed UTF-8.

This macro is intended to be used by XS modules where the inputs could be
malformed, and it isn't feasible to restructure to use the safer
C<L</UTF8_SAFE_SKIP>>, for example when interfacing with a C library.

=cut
*/

#define UTF8_CHK_SKIP(s)                                                       \
            (UNLIKELY(s[0] == '\0') ? 1 : MIN(UTF8SKIP(s),                     \
                                    my_strnlen((char *) (s), UTF8SKIP(s))))
/*

=for apidoc Am|STRLEN|UTF8_SAFE_SKIP|char* s|char* e
returns 0 if S<C<s E<gt>= e>>; otherwise returns the number of bytes in the
UTF-8 encoded character whose first  byte is pointed to by C<s>.  But it never
returns beyond C<e>.  On DEBUGGING builds, it asserts that S<C<s E<lt>= e>>.

=cut
 */
#define UTF8_SAFE_SKIP(s, e)  (__ASSERT_((e) >= (s))                \
                              UNLIKELY(((e) - (s)) <= 0)            \
                               ? 0                                  \
                               : MIN(((e) - (s)), UTF8_SKIP(s)))

/* Most code that says 'UNI_' really means the native value for code points up
 * through 255 */
#define UNI_IS_INVARIANT(cp)   UVCHR_IS_INVARIANT(cp)

/*
=for apidoc Am|bool|UTF8_IS_INVARIANT|char c

Evaluates to 1 if the byte C<c> represents the same character when encoded in
UTF-8 as when not; otherwise evaluates to 0.  UTF-8 invariant characters can be
copied as-is when converting to/from UTF-8, saving time.

In spite of the name, this macro gives the correct result if the input string
from which C<c> comes is not encoded in UTF-8.

See C<L</UVCHR_IS_INVARIANT>> for checking if a UV is invariant.

=cut

The reason it works on both UTF-8 encoded strings and non-UTF-8 encoded, is
that it returns TRUE in each for the exact same set of bit patterns.  It is
valid on a subset of what UVCHR_IS_INVARIANT is valid on, so can just use that;
and the compiler should optimize out anything extraneous given the
implementation of the latter. */
#define UTF8_IS_INVARIANT(c)	UVCHR_IS_INVARIANT(ASSERT_NOT_PTR(c))

/* Like the above, but its name implies a non-UTF8 input, which as the comments
 * above show, doesn't matter as to its implementation */
#define NATIVE_BYTE_IS_INVARIANT(c)	UVCHR_IS_INVARIANT(c)

/* Misleadingly named: is the UTF8-encoded byte 'c' part of a variant sequence
 * in UTF-8?  This is the inverse of UTF8_IS_INVARIANT. */
#define UTF8_IS_CONTINUED(c)  (__ASSERT_(FITS_IN_8_BITS(c))                 \
                               (! UTF8_IS_INVARIANT(c)))

/* The macros in the next 4 sets are used to generate the two utf8 or utfebcdic
 * bytes from an ordinal that is known to fit into exactly two (not one) bytes;
 * it must be less than 0x3FF to work across both encodings. */

/* These two are helper macros for the other three sets, and should not be used
 * directly anywhere else.  'translate_function' is either NATIVE_TO_LATIN1
 * (which works for code points up through 0xFF) or NATIVE_TO_UNI which works
 * for any code point */
#define __BASE_TWO_BYTE_HI(c, translate_function)                               \
           (__ASSERT_(! UVCHR_IS_INVARIANT(c))                                  \
            I8_TO_NATIVE_UTF8((translate_function(c) >> UTF_ACCUMULATION_SHIFT) \
                              | UTF_START_MARK(2)))
#define __BASE_TWO_BYTE_LO(c, translate_function)                               \
             (__ASSERT_(! UVCHR_IS_INVARIANT(c))                                \
              I8_TO_NATIVE_UTF8((translate_function(c) & UTF_CONTINUATION_MASK) \
                                 | UTF_CONTINUATION_MARK))

/* The next two macros should not be used.  They were designed to be usable as
 * the case label of a switch statement, but this doesn't work for EBCDIC.  Use
 * regen/unicode_constants.pl instead */
#define UTF8_TWO_BYTE_HI_nocast(c)  __BASE_TWO_BYTE_HI(c, NATIVE_TO_UNI)
#define UTF8_TWO_BYTE_LO_nocast(c)  __BASE_TWO_BYTE_LO(c, NATIVE_TO_UNI)

/* The next two macros are used when the source should be a single byte
 * character; checked for under DEBUGGING */
#define UTF8_EIGHT_BIT_HI(c) (__ASSERT_(FITS_IN_8_BITS(c))                    \
                             ( __BASE_TWO_BYTE_HI(c, NATIVE_TO_LATIN1)))
#define UTF8_EIGHT_BIT_LO(c) (__ASSERT_(FITS_IN_8_BITS(c))                    \
                             (__BASE_TWO_BYTE_LO(c, NATIVE_TO_LATIN1)))

/* These final two macros in the series are used when the source can be any
 * code point whose UTF-8 is known to occupy 2 bytes; they are less efficient
 * than the EIGHT_BIT versions on EBCDIC platforms.  We use the logical '~'
 * operator instead of "<=" to avoid getting compiler warnings.
 * MAX_UTF8_TWO_BYTE should be exactly all one bits in the lower few
 * places, so the ~ works */
#define UTF8_TWO_BYTE_HI(c)                                                    \
       (__ASSERT_((sizeof(c) ==  1)                                            \
                  || !(((WIDEST_UTYPE)(c)) & ~MAX_UTF8_TWO_BYTE))              \
        (__BASE_TWO_BYTE_HI(c, NATIVE_TO_UNI)))
#define UTF8_TWO_BYTE_LO(c)                                                    \
       (__ASSERT_((sizeof(c) ==  1)                                            \
                  || !(((WIDEST_UTYPE)(c)) & ~MAX_UTF8_TWO_BYTE))              \
        (__BASE_TWO_BYTE_LO(c, NATIVE_TO_UNI)))

/* This is illegal in any well-formed UTF-8 in both EBCDIC and ASCII
 * as it is only in overlongs. */
#define ILLEGAL_UTF8_BYTE   I8_TO_NATIVE_UTF8(0xC1)

/*
 * 'UTF' is whether or not p is encoded in UTF8.  The names 'foo_lazy_if' stem
 * from an earlier version of these macros in which they didn't call the
 * foo_utf8() macros (i.e. were 'lazy') unless they decided that *p is the
 * beginning of a utf8 character.  Now that foo_utf8() determines that itself,
 * no need to do it again here
 */
#define isIDFIRST_lazy_if_safe(p, e, UTF)                                   \
                   ((IN_BYTES || !UTF)                                      \
                     ? isIDFIRST(*(p))                                      \
                     : isIDFIRST_utf8_safe(p, e))
#define isWORDCHAR_lazy_if_safe(p, e, UTF)                                  \
                   ((IN_BYTES || !UTF)                                      \
                     ? isWORDCHAR(*(p))                                     \
                     : isWORDCHAR_utf8_safe((U8 *) p, (U8 *) e))
#define isALNUM_lazy_if_safe(p, e, UTF) isWORDCHAR_lazy_if_safe(p, e, UTF)

#define UTF8_MAXLEN  UTF8_MAXBYTES

/* A Unicode character can fold to up to 3 characters */
#define UTF8_MAX_FOLD_CHAR_EXPAND  3

#define IN_BYTES  UNLIKELY(CopHINTS_get(PL_curcop) & HINT_BYTES)

/*

=for apidoc Am|bool|DO_UTF8|SV* sv
Returns a bool giving whether or not the PV in C<sv> is to be treated as being
encoded in UTF-8.

You should use this I<after> a call to C<SvPV()> or one of its variants, in
case any call to string overloading updates the internal UTF-8 encoding flag.

=cut
*/
#define DO_UTF8(sv) (SvUTF8(sv) && !IN_BYTES)

/* Should all strings be treated as Unicode, and not just UTF-8 encoded ones?
 * Is so within 'feature unicode_strings' or 'locale :not_characters', and not
 * within 'use bytes'.  UTF-8 locales are not tested for here, because it gets
 * complicated by the probability of having categories in different locales. */
#define IN_UNI_8_BIT                                                    \
            ((    (      (CopHINTS_get(PL_curcop) & HINT_UNI_8_BIT))    \
                   || (   CopHINTS_get(PL_curcop) & HINT_LOCALE_PARTIAL \
                            /* -1 below is for :not_characters */       \
                       && _is_in_locale_category(FALSE, -1)))           \
              && (! IN_BYTES))

#define UNICODE_SURROGATE_FIRST		0xD800
#define UNICODE_SURROGATE_LAST		0xDFFF

/*
=for apidoc Am|bool|UNICODE_IS_SURROGATE|const UV uv

Returns a boolean as to whether or not C<uv> is one of the Unicode surrogate
code points

=for apidoc Am|bool|UTF8_IS_SURROGATE|const U8 *s|const U8 *e

Evaluates to non-zero if the first few bytes of the string starting at C<s> and
looking no further than S<C<e - 1>> are well-formed UTF-8 that represents one
of the Unicode surrogate code points; otherwise it evaluates to 0.  If
non-zero, the value gives how many bytes starting at C<s> comprise the code
point's representation.

=cut
 */

#define UNICODE_IS_SURROGATE(uv) UNLIKELY(inRANGE(uv, UNICODE_SURROGATE_FIRST,  \
                                                      UNICODE_SURROGATE_LAST))
#define UTF8_IS_SURROGATE(s, e)      is_SURROGATE_utf8_safe(s, e)

/*

=for apidoc AmnU|UV|UNICODE_REPLACEMENT

Evaluates to 0xFFFD, the code point of the Unicode REPLACEMENT CHARACTER

=for apidoc Am|bool|UNICODE_IS_REPLACEMENT|const UV uv

Returns a boolean as to whether or not C<uv> is the Unicode REPLACEMENT
CHARACTER

=for apidoc Am|bool|UTF8_IS_REPLACEMENT|const U8 *s|const U8 *e

Evaluates to non-zero if the first few bytes of the string starting at C<s> and
looking no further than S<C<e - 1>> are well-formed UTF-8 that represents the
Unicode REPLACEMENT CHARACTER; otherwise it evaluates to 0.  If non-zero, the
value gives how many bytes starting at C<s> comprise the code point's
representation.

=cut
 */
#define UNICODE_REPLACEMENT		0xFFFD
#define UNICODE_IS_REPLACEMENT(uv)  UNLIKELY((UV) (uv) == UNICODE_REPLACEMENT)
#define UTF8_IS_REPLACEMENT(s, send)                                         \
    UNLIKELY(                                                                \
        ((send) - (s)) >= ((SSize_t)(sizeof(REPLACEMENT_CHARACTER_UTF8) - 1))\
             && memEQ((s), REPLACEMENT_CHARACTER_UTF8,                       \
                      sizeof(REPLACEMENT_CHARACTER_UTF8) - 1))

/* Max legal code point according to Unicode */
#define PERL_UNICODE_MAX	0x10FFFF

/*

=for apidoc Am|bool|UNICODE_IS_SUPER|const UV uv

Returns a boolean as to whether or not C<uv> is above the maximum legal Unicode
code point of U+10FFFF.

=cut
*/

#define UNICODE_IS_SUPER(uv)  UNLIKELY((UV) (uv) > PERL_UNICODE_MAX)

/*
=for apidoc Am|bool|UTF8_IS_SUPER|const U8 *s|const U8 *e

Recall that Perl recognizes an extension to UTF-8 that can encode code
points larger than the ones defined by Unicode, which are 0..0x10FFFF.

This macro evaluates to non-zero if the first few bytes of the string starting
at C<s> and looking no further than S<C<e - 1>> are from this UTF-8 extension;
otherwise it evaluates to 0.  If non-zero, the return is how many bytes
starting at C<s> comprise the code point's representation.

0 is returned if the bytes are not well-formed extended UTF-8, or if they
represent a code point that cannot fit in a UV on the current platform.  Hence
this macro can give different results when run on a 64-bit word machine than on
one with a 32-bit word size.

Note that it is illegal in Perl to have code points that are larger than what can
fit in an IV on the current machine; and illegal in Unicode to have any that
this macro matches

=cut

 *		  ASCII		     EBCDIC I8
 * U+10FFFF: \xF4\x8F\xBF\xBF	\xF9\xA1\xBF\xBF\xBF	max legal Unicode
 * U+110000: \xF4\x90\x80\x80	\xF9\xA2\xA0\xA0\xA0
 * U+110001: \xF4\x90\x80\x81	\xF9\xA2\xA0\xA0\xA1
 */
#define UTF_START_BYTE_110000_  UTF_START_BYTE(PERL_UNICODE_MAX + 1, 21)
#define UTF_FIRST_CONT_BYTE_110000_                                         \
                          UTF_FIRST_CONT_BYTE(PERL_UNICODE_MAX + 1, 21)
#define UTF8_IS_SUPER(s, e)                                                 \
    (   ((e) - (s)) >= UNISKIP_BY_MSB_(20)                                  \
     && (       NATIVE_UTF8_TO_I8(s[0]) >= UTF_START_BYTE_110000_           \
         && (   NATIVE_UTF8_TO_I8(s[0]) >  UTF_START_BYTE_110000_           \
             || NATIVE_UTF8_TO_I8(s[1]) >= UTF_FIRST_CONT_BYTE_110000_)))   \
     ? isUTF8_CHAR(s, e)                                                    \
     : 0

/*
=for apidoc Am|bool|UNICODE_IS_NONCHAR|const UV uv

Returns a boolean as to whether or not C<uv> is one of the Unicode
non-character code points

=cut
*/

/* Is 'uv' one of the 32 contiguous-range noncharacters? */
#define UNICODE_IS_32_CONTIGUOUS_NONCHARS(uv)                               \
                                    UNLIKELY(inRANGE(uv, 0xFDD0, 0xFDEF))

/* Is 'uv' one of the 34 plane-ending noncharacters 0xFFFE, 0xFFFF, 0x1FFFE,
 * 0x1FFFF, ... 0x10FFFE, 0x10FFFF, given that we know that 'uv' is not above
 * the Unicode legal max */
#define UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER(uv)                    \
                                  UNLIKELY(((UV) (uv) & 0xFFFE) == 0xFFFE)

#define UNICODE_IS_NONCHAR(uv)                                              \
    (       UNLIKELY(UNICODE_IS_32_CONTIGUOUS_NONCHARS(uv))                 \
     || (   UNLIKELY(UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER(uv))      \
         && LIKELY(! UNICODE_IS_SUPER(uv))))

/*
=for apidoc Am|bool|UTF8_IS_NONCHAR|const U8 *s|const U8 *e

Evaluates to non-zero if the first few bytes of the string starting at C<s> and
looking no further than S<C<e - 1>> are well-formed UTF-8 that represents one
of the Unicode non-character code points; otherwise it evaluates to 0.  If
non-zero, the value gives how many bytes starting at C<s> comprise the code
point's representation.

=cut
*/
#define UTF8_IS_NONCHAR(s, e)  is_NONCHAR_utf8_safe(s,e)

/* This is now machine generated, and the 'given' clause is no longer
 * applicable */
#define UTF8_IS_NONCHAR_GIVEN_THAT_NON_SUPER_AND_GE_PROBLEMATIC(s, e)       \
                                                UTF8_IS_NONCHAR(s, e)

/* Surrogates, non-character code points and above-Unicode code points are
 * problematic in some contexts.  These macros allow code that needs to check
 * for those to quickly exclude the vast majority of code points it will
 * encounter.
 *
 * The lowest such code point is the smallest surrogate, U+D800.  We calculate
 * the start byte of that.  0xD800 occupies 16 bits. */
#define isUNICODE_POSSIBLY_PROBLEMATIC(uv) ((uv) >= UNICODE_SURROGATE_FIRST)
#define isUTF8_POSSIBLY_PROBLEMATIC(c)                                      \
    (NATIVE_UTF8_TO_I8(c) >= UTF_START_BYTE(UNICODE_SURROGATE_FIRST, 16))

/* Perl extends Unicode so that it is possible to encode (as extended UTF-8 or
 * UTF-EBCDIC) any 64-bit value.  No standard known to khw ever encoded higher
 * than a 31 bit value.  On ASCII platforms this just meant arbitrarily saying
 * nothing could be higher than this.  On these the start byte FD gets you to
 * 31 bits, and FE and FF are forbidden as start bytes.  On EBCDIC platforms,
 * FD gets you only to 26 bits; adding FE to mean 7 total bytes gets you to 30
 * bits.  To get to 31 bits, they treated an initial FF byte idiosyncratically.
 * It was considered to be the start byte FE meaning it had 7 total bytes, and
 * the final 1 was treated as an information bit, getting you to 31 bits.
 *
 * Perl used to accept this idiosyncratic interpretation of FF, but now rejects
 * it in order to get to being able to encode 64 bits.  The bottom line is that
 * it is a Perl extension to use the start bytes FE and FF on ASCII platforms,
 * and the start byte FF on EBCDIC ones.  That translates into that it is a
 * Perl extension to represent anything occupying more than 31 bits on ASCII
 * platforms; 30 bits on EBCDIC. */
#define UNICODE_IS_PERL_EXTENDED(uv)                                        \
          UNLIKELY((UV) (uv) > nBIT_UMAX(31 - ONE_IF_EBCDIC_ZERO_IF_NOT))
#define UTF8_IS_PERL_EXTENDED(s)                                            \
                           (UTF8SKIP(s) > 6 + ONE_IF_EBCDIC_ZERO_IF_NOT)

/* Largest code point we accept from external sources */
#define MAX_LEGAL_CP  ((UV)IV_MAX)

#define UTF8_ALLOW_EMPTY		0x0001	/* Allow a zero length string */
#define UTF8_GOT_EMPTY                  UTF8_ALLOW_EMPTY

/* Allow first byte to be a continuation byte */
#define UTF8_ALLOW_CONTINUATION		0x0002
#define UTF8_GOT_CONTINUATION		UTF8_ALLOW_CONTINUATION

/* Unexpected non-continuation byte */
#define UTF8_ALLOW_NON_CONTINUATION	0x0004
#define UTF8_GOT_NON_CONTINUATION	UTF8_ALLOW_NON_CONTINUATION

/* expecting more bytes than were available in the string */
#define UTF8_ALLOW_SHORT		0x0008
#define UTF8_GOT_SHORT		        UTF8_ALLOW_SHORT

/* Overlong sequence; i.e., the code point can be specified in fewer bytes.
 * First one will convert the overlong to the REPLACEMENT CHARACTER; second
 * will return what the overlong evaluates to */
#define UTF8_ALLOW_LONG                 0x0010
#define UTF8_ALLOW_LONG_AND_ITS_VALUE   (UTF8_ALLOW_LONG|0x0020)
#define UTF8_GOT_LONG                   UTF8_ALLOW_LONG

#define UTF8_ALLOW_OVERFLOW             0x0080
#define UTF8_GOT_OVERFLOW               UTF8_ALLOW_OVERFLOW

#define UTF8_DISALLOW_SURROGATE		0x0100	/* Unicode surrogates */
#define UTF8_GOT_SURROGATE		UTF8_DISALLOW_SURROGATE
#define UTF8_WARN_SURROGATE		0x0200

/* Unicode non-character  code points */
#define UTF8_DISALLOW_NONCHAR           0x0400
#define UTF8_GOT_NONCHAR                UTF8_DISALLOW_NONCHAR
#define UTF8_WARN_NONCHAR               0x0800

/* Super-set of Unicode: code points above the legal max */
#define UTF8_DISALLOW_SUPER		0x1000
#define UTF8_GOT_SUPER		        UTF8_DISALLOW_SUPER
#define UTF8_WARN_SUPER		        0x2000

/* The original UTF-8 standard did not define UTF-8 with start bytes of 0xFE or
 * 0xFF, though UTF-EBCDIC did.  This allowed both versions to represent code
 * points up to 2 ** 31 - 1.  Perl extends UTF-8 so that 0xFE and 0xFF are
 * usable on ASCII platforms, and 0xFF means something different than
 * UTF-EBCDIC defines.  These changes allow code points of 64 bits (actually
 * somewhat more) to be represented on both platforms.  But these are Perl
 * extensions, and not likely to be interchangeable with other languages.  Note
 * that on ASCII platforms, FE overflows a signed 32-bit word, and FF an
 * unsigned one. */
#define UTF8_DISALLOW_PERL_EXTENDED     0x4000
#define UTF8_GOT_PERL_EXTENDED          UTF8_DISALLOW_PERL_EXTENDED
#define UTF8_WARN_PERL_EXTENDED         0x8000

/* For back compat, these old names are misleading for overlongs and
 * UTF_EBCDIC. */
#define UTF8_DISALLOW_ABOVE_31_BIT      UTF8_DISALLOW_PERL_EXTENDED
#define UTF8_GOT_ABOVE_31_BIT           UTF8_GOT_PERL_EXTENDED
#define UTF8_WARN_ABOVE_31_BIT          UTF8_WARN_PERL_EXTENDED
#define UTF8_DISALLOW_FE_FF             UTF8_DISALLOW_PERL_EXTENDED
#define UTF8_WARN_FE_FF                 UTF8_WARN_PERL_EXTENDED

#define UTF8_CHECK_ONLY			0x10000
#define _UTF8_NO_CONFIDENCE_IN_CURLEN   0x20000  /* Internal core use only */

/* For backwards source compatibility.  They do nothing, as the default now
 * includes what they used to mean.  The first one's meaning was to allow the
 * just the single non-character 0xFFFF */
#define UTF8_ALLOW_FFFF 0
#define UTF8_ALLOW_FE_FF 0
#define UTF8_ALLOW_SURROGATE 0

/* C9 refers to Unicode Corrigendum #9: allows but discourages non-chars */
#define UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE                                    \
                                 (UTF8_DISALLOW_SUPER|UTF8_DISALLOW_SURROGATE)
#define UTF8_WARN_ILLEGAL_C9_INTERCHANGE (UTF8_WARN_SUPER|UTF8_WARN_SURROGATE)

#define UTF8_DISALLOW_ILLEGAL_INTERCHANGE                                       \
                  (UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE|UTF8_DISALLOW_NONCHAR)
#define UTF8_WARN_ILLEGAL_INTERCHANGE \
                          (UTF8_WARN_ILLEGAL_C9_INTERCHANGE|UTF8_WARN_NONCHAR)

/* This is typically used for code that processes UTF-8 input and doesn't want
 * to have to deal with any malformations that might be present.  All such will
 * be safely replaced by the REPLACEMENT CHARACTER, unless other flags
 * overriding this are also present. */
#define UTF8_ALLOW_ANY ( UTF8_ALLOW_CONTINUATION                                \
                        |UTF8_ALLOW_NON_CONTINUATION                            \
                        |UTF8_ALLOW_SHORT                                       \
                        |UTF8_ALLOW_LONG                                        \
                        |UTF8_ALLOW_OVERFLOW)

/* Accept any Perl-extended UTF-8 that evaluates to any UV on the platform, but
 * not any malformed.  This is the default. */
#define UTF8_ALLOW_ANYUV   0
#define UTF8_ALLOW_DEFAULT UTF8_ALLOW_ANYUV

#define UNICODE_WARN_SURROGATE         0x0001	/* UTF-16 surrogates */
#define UNICODE_WARN_NONCHAR           0x0002	/* Non-char code points */
#define UNICODE_WARN_SUPER             0x0004	/* Above 0x10FFFF */
#define UNICODE_WARN_PERL_EXTENDED     0x0008	/* Above 0x7FFF_FFFF */
#define UNICODE_WARN_ABOVE_31_BIT      UNICODE_WARN_PERL_EXTENDED
#define UNICODE_DISALLOW_SURROGATE     0x0010
#define UNICODE_DISALLOW_NONCHAR       0x0020
#define UNICODE_DISALLOW_SUPER         0x0040
#define UNICODE_DISALLOW_PERL_EXTENDED 0x0080

#ifdef PERL_CORE
#  define UNICODE_ALLOW_ABOVE_IV_MAX   0x0100
#endif
#define UNICODE_DISALLOW_ABOVE_31_BIT  UNICODE_DISALLOW_PERL_EXTENDED

#define UNICODE_GOT_SURROGATE       UNICODE_DISALLOW_SURROGATE
#define UNICODE_GOT_NONCHAR         UNICODE_DISALLOW_NONCHAR
#define UNICODE_GOT_SUPER           UNICODE_DISALLOW_SUPER
#define UNICODE_GOT_PERL_EXTENDED   UNICODE_DISALLOW_PERL_EXTENDED

#define UNICODE_WARN_ILLEGAL_C9_INTERCHANGE                                   \
                                  (UNICODE_WARN_SURROGATE|UNICODE_WARN_SUPER)
#define UNICODE_WARN_ILLEGAL_INTERCHANGE                                      \
                   (UNICODE_WARN_ILLEGAL_C9_INTERCHANGE|UNICODE_WARN_NONCHAR)
#define UNICODE_DISALLOW_ILLEGAL_C9_INTERCHANGE                               \
                          (UNICODE_DISALLOW_SURROGATE|UNICODE_DISALLOW_SUPER)
#define UNICODE_DISALLOW_ILLEGAL_INTERCHANGE                                  \
           (UNICODE_DISALLOW_ILLEGAL_C9_INTERCHANGE|UNICODE_DISALLOW_NONCHAR)

/* For backward source compatibility, as are now the default */
#define UNICODE_ALLOW_SURROGATE 0
#define UNICODE_ALLOW_SUPER	0
#define UNICODE_ALLOW_ANY	0

#define UNICODE_BYTE_ORDER_MARK		0xFEFF
#define UNICODE_IS_BYTE_ORDER_MARK(uv)	UNLIKELY((UV) (uv)                  \
                                                == UNICODE_BYTE_ORDER_MARK)

#define LATIN_SMALL_LETTER_SHARP_S      LATIN_SMALL_LETTER_SHARP_S_NATIVE
#define LATIN_SMALL_LETTER_Y_WITH_DIAERESIS                                  \
                                LATIN_SMALL_LETTER_Y_WITH_DIAERESIS_NATIVE
#define MICRO_SIGN      MICRO_SIGN_NATIVE
#define LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE                               \
                            LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE_NATIVE
#define LATIN_SMALL_LETTER_A_WITH_RING_ABOVE                                 \
                                LATIN_SMALL_LETTER_A_WITH_RING_ABOVE_NATIVE
#define UNICODE_GREEK_CAPITAL_LETTER_SIGMA	0x03A3
#define UNICODE_GREEK_SMALL_LETTER_FINAL_SIGMA	0x03C2
#define UNICODE_GREEK_SMALL_LETTER_SIGMA	0x03C3
#define GREEK_SMALL_LETTER_MU                   0x03BC
#define GREEK_CAPITAL_LETTER_MU                 0x039C	/* Upper and title case
                                                           of MICRON */
#define LATIN_CAPITAL_LETTER_Y_WITH_DIAERESIS   0x0178	/* Also is title case */
#ifdef LATIN_CAPITAL_LETTER_SHARP_S_UTF8
#   define LATIN_CAPITAL_LETTER_SHARP_S	        0x1E9E
#endif
#define LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE   0x130
#define LATIN_SMALL_LETTER_DOTLESS_I            0x131
#define LATIN_SMALL_LETTER_LONG_S               0x017F
#define LATIN_SMALL_LIGATURE_LONG_S_T           0xFB05
#define LATIN_SMALL_LIGATURE_ST                 0xFB06
#define KELVIN_SIGN                             0x212A
#define ANGSTROM_SIGN                           0x212B

#define UNI_DISPLAY_ISPRINT	0x0001
#define UNI_DISPLAY_BACKSLASH	0x0002
#define UNI_DISPLAY_BACKSPACE	0x0004  /* Allow \b when also
                                           UNI_DISPLAY_BACKSLASH */
#define UNI_DISPLAY_QQ		(UNI_DISPLAY_ISPRINT                \
                                |UNI_DISPLAY_BACKSLASH              \
                                |UNI_DISPLAY_BACKSPACE)

/* Character classes could also allow \b, but not patterns in general */
#define UNI_DISPLAY_REGEX	(UNI_DISPLAY_ISPRINT|UNI_DISPLAY_BACKSLASH)

/* Should be removed; maybe deprecated, but not used in CPAN */
#define SHARP_S_SKIP 2

#define is_utf8_char_buf(buf, buf_end) isUTF8_CHAR(buf, buf_end)
#define bytes_from_utf8(s, lenp, is_utf8p)                                  \
                            bytes_from_utf8_loc(s, lenp, is_utf8p, 0)

/* Do not use; should be deprecated.  Use isUTF8_CHAR() instead; this is
 * retained solely for backwards compatibility */
#define IS_UTF8_CHAR(p, n)      (isUTF8_CHAR(p, (p) + (n)) == n)

#endif /* PERL_UTF8_H_ */

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
