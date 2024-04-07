/*    utf8.c
 *
 *    Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
 *    by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'What a fix!' said Sam.  'That's the one place in all the lands we've ever
 *  heard of that we don't want to see any closer; and that's the one place
 *  we're trying to get to!  And that's just where we can't get, nohow.'
 *
 *     [p.603 of _The Lord of the Rings_, IV/I: "The Taming of Sméagol"]
 *
 * 'Well do I understand your speech,' he answered in the same language;
 * 'yet few strangers do so.  Why then do you not speak in the Common Tongue,
 *  as is the custom in the West, if you wish to be answered?'
 *                           --Gandalf, addressing Théoden's door wardens
 *
 *     [p.508 of _The Lord of the Rings_, III/vi: "The King of the Golden Hall"]
 *
 * ...the travellers perceived that the floor was paved with stones of many
 * hues; branching runes and strange devices intertwined beneath their feet.
 *
 *     [p.512 of _The Lord of the Rings_, III/vi: "The King of the Golden Hall"]
 */

#include "EXTERN.h"
#define PERL_IN_UTF8_C
#include "perl.h"
#include "invlist_inline.h"

static const char malformed_text[] = "Malformed UTF-8 character";
static const char unees[] =
                        "Malformed UTF-8 character (unexpected end of string)";

/*
These are various utility functions for manipulating UTF8-encoded
strings.  For the uninitiated, this is a method of representing arbitrary
Unicode characters as a variable number of bytes, in such a way that
characters in the ASCII range are unmodified, and a zero byte never appears
within non-zero characters.
*/

void
Perl__force_out_malformed_utf8_message(pTHX_
            const U8 *const p,      /* First byte in UTF-8 sequence */
            const U8 * const e,     /* Final byte in sequence (may include
                                       multiple chars */
            const U32 flags,        /* Flags to pass to utf8n_to_uvchr(),
                                       usually 0, or some DISALLOW flags */
            const bool die_here)    /* If TRUE, this function does not return */
{
    /* This core-only function is to be called when a malformed UTF-8 character
     * is found, in order to output the detailed information about the
     * malformation before dieing.  The reason it exists is for the occasions
     * when such a malformation is fatal, but warnings might be turned off, so
     * that normally they would not be actually output.  This ensures that they
     * do get output.  Because a sequence may be malformed in more than one
     * way, multiple messages may be generated, so we can't make them fatal, as
     * that would cause the first one to die.
     *
     * Instead we pretend -W was passed to perl, then die afterwards.  The
     * flexibility is here to return to the caller so they can finish up and
     * die themselves */
    U32 errors;

    PERL_ARGS_ASSERT__FORCE_OUT_MALFORMED_UTF8_MESSAGE;

    ENTER;
    SAVEI8(PL_dowarn);
    SAVESPTR(PL_curcop);

    PL_dowarn = G_WARN_ALL_ON|G_WARN_ON;
    if (PL_curcop) {
        SAVECURCOPWARNINGS();
        PL_curcop->cop_warnings = pWARN_ALL;
    }

    (void) utf8n_to_uvchr_error(p, e - p, NULL, flags & ~UTF8_CHECK_ONLY, &errors);

    LEAVE;

    if (! errors) {
        Perl_croak(aTHX_ "panic: _force_out_malformed_utf8_message should"
                         " be called only when there are errors found");
    }

    if (die_here) {
        Perl_croak(aTHX_ "Malformed UTF-8 character (fatal)");
    }
}

STATIC HV *
S_new_msg_hv(pTHX_ const char * const message, /* The message text */
                   U32 categories,  /* Packed warning categories */
                   U32 flag)        /* Flag associated with this message */
{
    /* Creates, populates, and returns an HV* that describes an error message
     * for the translators between UTF8 and code point */

    SV* msg_sv = newSVpv(message, 0);
    SV* category_sv = newSVuv(categories);
    SV* flag_bit_sv = newSVuv(flag);

    HV* msg_hv = newHV();

    PERL_ARGS_ASSERT_NEW_MSG_HV;

    (void) hv_stores(msg_hv, "text", msg_sv);
    (void) hv_stores(msg_hv, "warn_categories",  category_sv);
    (void) hv_stores(msg_hv, "flag_bit", flag_bit_sv);

    return msg_hv;
}

/*
=for apidoc uvoffuni_to_utf8_flags

THIS FUNCTION SHOULD BE USED IN ONLY VERY SPECIALIZED CIRCUMSTANCES.
Instead, B<Almost all code should use L<perlapi/uvchr_to_utf8> or
L<perlapi/uvchr_to_utf8_flags>>.

This function is like them, but the input is a strict Unicode
(as opposed to native) code point.  Only in very rare circumstances should code
not be using the native code point.

For details, see the description for L<perlapi/uvchr_to_utf8_flags>.

=cut
*/

U8 *
Perl_uvoffuni_to_utf8_flags(pTHX_ U8 *d, UV uv, const UV flags)
{
    PERL_ARGS_ASSERT_UVOFFUNI_TO_UTF8_FLAGS;

    return uvoffuni_to_utf8_flags_msgs(d, uv, flags, NULL);
}

/* All these formats take a single UV code point argument */
const char surrogate_cp_format[] = "UTF-16 surrogate U+%04" UVXf;
const char nonchar_cp_format[]   = "Unicode non-character U+%04" UVXf
                                   " is not recommended for open interchange";
const char super_cp_format[]     = "Code point 0x%" UVXf " is not Unicode,"
                                   " may not be portable";

/*  Use shorter names internally in this file */
#define SHIFT   UTF_ACCUMULATION_SHIFT
#undef  MARK
#define MARK    UTF_CONTINUATION_MARK
#define MASK    UTF_CONTINUATION_MASK

/*
=for apidoc uvchr_to_utf8_flags_msgs

THIS FUNCTION SHOULD BE USED IN ONLY VERY SPECIALIZED CIRCUMSTANCES.

Most code should use C<L</uvchr_to_utf8_flags>()> rather than call this directly.

This function is for code that wants any warning and/or error messages to be
returned to the caller rather than be displayed.  All messages that would have
been displayed if all lexical warnings are enabled will be returned.

It is just like C<L</uvchr_to_utf8_flags>> but it takes an extra parameter
placed after all the others, C<msgs>.  If this parameter is 0, this function
behaves identically to C<L</uvchr_to_utf8_flags>>.  Otherwise, C<msgs> should
be a pointer to an C<HV *> variable, in which this function creates a new HV to
contain any appropriate messages.  The hash has three key-value pairs, as
follows:

=over 4

=item C<text>

The text of the message as a C<SVpv>.

=item C<warn_categories>

The warning category (or categories) packed into a C<SVuv>.

=item C<flag>

A single flag bit associated with this message, in a C<SVuv>.
The bit corresponds to some bit in the C<*errors> return value,
such as C<UNICODE_GOT_SURROGATE>.

=back

It's important to note that specifying this parameter as non-null will cause
any warnings this function would otherwise generate to be suppressed, and
instead be placed in C<*msgs>.  The caller can check the lexical warnings state
(or not) when choosing what to do with the returned messages.

The caller, of course, is responsible for freeing any returned HV.

=cut
*/

/* Undocumented; we don't want people using this.  Instead they should use
 * uvchr_to_utf8_flags_msgs() */
U8 *
Perl_uvoffuni_to_utf8_flags_msgs(pTHX_ U8 *d, UV input_uv, UV flags, HV** msgs)
{
    U8 *p;
    UV shifted_uv = input_uv;
    STRLEN utf8_skip = OFFUNISKIP(input_uv);

    PERL_ARGS_ASSERT_UVOFFUNI_TO_UTF8_FLAGS_MSGS;

    if (msgs) {
        *msgs = NULL;
    }

    switch (utf8_skip) {
      case 1:
        *d++ = LATIN1_TO_NATIVE(input_uv);
        return d;

      default:
        if (   UNLIKELY(input_uv > MAX_LEGAL_CP
            && UNLIKELY(! (flags & UNICODE_ALLOW_ABOVE_IV_MAX))))
        {
            Perl_croak(aTHX_ "%s", form_cp_too_large_msg(16, /* Hex output */
                                                         NULL, 0, input_uv));
        }

        if ((flags & (UNICODE_WARN_PERL_EXTENDED|UNICODE_WARN_SUPER))) {
            U32 category = packWARN2(WARN_NON_UNICODE, WARN_PORTABLE);
            const char * format = PL_extended_cp_format;
            if (msgs) {
                *msgs = new_msg_hv(Perl_form(aTHX_ format, input_uv),
                                   category,
                                   UNICODE_GOT_PERL_EXTENDED);
            }
            else {
                Perl_ck_warner_d(aTHX_ category, format, input_uv);
            }

            /* Don't output a 2nd msg */
            flags &= ~UNICODE_WARN_SUPER;
        }

        if (flags & UNICODE_DISALLOW_PERL_EXTENDED) {
            return NULL;
        }

        p = d + utf8_skip - 1;
        while (p >= d + 6 + ONE_IF_EBCDIC_ZERO_IF_NOT) {
            *p-- = I8_TO_NATIVE_UTF8((shifted_uv & MASK) | MARK);
            shifted_uv >>= SHIFT;
        }

        /* FALLTHROUGH */

      case 6 + ONE_IF_EBCDIC_ZERO_IF_NOT:
        d[5 + ONE_IF_EBCDIC_ZERO_IF_NOT]
                                = I8_TO_NATIVE_UTF8((shifted_uv & MASK) | MARK);
        shifted_uv >>= SHIFT;
        /* FALLTHROUGH */

      case 5 + ONE_IF_EBCDIC_ZERO_IF_NOT:
        d[4 + ONE_IF_EBCDIC_ZERO_IF_NOT]
                                = I8_TO_NATIVE_UTF8((shifted_uv & MASK) | MARK);
        shifted_uv >>= SHIFT;
        /* FALLTHROUGH */

      case 4 + ONE_IF_EBCDIC_ZERO_IF_NOT:
        if (UNLIKELY(UNICODE_IS_SUPER(input_uv))) {
            if (flags & UNICODE_WARN_SUPER) {
                U32 category = packWARN(WARN_NON_UNICODE);
                const char * format = super_cp_format;

                if (msgs) {
                    *msgs = new_msg_hv(Perl_form(aTHX_ format, input_uv),
                                       category,
                                       UNICODE_GOT_SUPER);
                }
                else {
                    Perl_ck_warner_d(aTHX_ category, format, input_uv);
                }

                if (flags & UNICODE_DISALLOW_SUPER) {
                    return NULL;
                }
            }
            if (       (flags & UNICODE_DISALLOW_SUPER)
                || (   (flags & UNICODE_DISALLOW_PERL_EXTENDED)
                    &&  UNICODE_IS_PERL_EXTENDED(input_uv)))
            {
                return NULL;
            }
        }

        d[3 + ONE_IF_EBCDIC_ZERO_IF_NOT]
                                = I8_TO_NATIVE_UTF8((shifted_uv & MASK) | MARK);
        shifted_uv >>= SHIFT;
        /* FALLTHROUGH */

      case 3 + ONE_IF_EBCDIC_ZERO_IF_NOT:
        if (isUNICODE_POSSIBLY_PROBLEMATIC(input_uv)) {
            if (UNLIKELY(UNICODE_IS_NONCHAR(input_uv))) {
                if (flags & UNICODE_WARN_NONCHAR) {
                    U32 category = packWARN(WARN_NONCHAR);
                    const char * format = nonchar_cp_format;
                    if (msgs) {
                        *msgs = new_msg_hv(Perl_form(aTHX_ format, input_uv),
                                           category,
                                           UNICODE_GOT_NONCHAR);
                    }
                    else {
                        Perl_ck_warner_d(aTHX_ category, format, input_uv);
                    }
                }
                if (flags & UNICODE_DISALLOW_NONCHAR) {
                    return NULL;
                }
            }
            else if (UNLIKELY(UNICODE_IS_SURROGATE(input_uv))) {
                if (flags & UNICODE_WARN_SURROGATE) {
                    U32 category = packWARN(WARN_SURROGATE);
                    const char * format = surrogate_cp_format;
                    if (msgs) {
                        *msgs = new_msg_hv(Perl_form(aTHX_ format, input_uv),
                                           category,
                                           UNICODE_GOT_SURROGATE);
                    }
                    else {
                        Perl_ck_warner_d(aTHX_ category, format, input_uv);
                    }
                }
                if (flags & UNICODE_DISALLOW_SURROGATE) {
                    return NULL;
                }
            }
        }

        d[2 + ONE_IF_EBCDIC_ZERO_IF_NOT]
                                = I8_TO_NATIVE_UTF8((shifted_uv & MASK) | MARK);
        shifted_uv >>= SHIFT;
        /* FALLTHROUGH */

#ifdef EBCDIC

      case 3:
        d[2] = I8_TO_NATIVE_UTF8((shifted_uv & MASK) | MARK);
        shifted_uv >>= SHIFT;
        /* FALLTHROUGH */

#endif

        /* FALLTHROUGH */
      case 2:
        d[1] = I8_TO_NATIVE_UTF8((shifted_uv & MASK) | MARK);
        shifted_uv >>= SHIFT;
        d[0] = I8_TO_NATIVE_UTF8((shifted_uv & UTF_START_MASK(utf8_skip))
                                             | UTF_START_MARK(utf8_skip));
        break;
    }

    return d + utf8_skip;
}

/*
=for apidoc uvchr_to_utf8

Adds the UTF-8 representation of the native code point C<uv> to the end
of the string C<d>; C<d> should have at least C<UVCHR_SKIP(uv)+1> (up to
C<UTF8_MAXBYTES+1>) free bytes available.  The return value is the pointer to
the byte after the end of the new character.  In other words,

    d = uvchr_to_utf8(d, uv);

is the recommended wide native character-aware way of saying

    *(d++) = uv;

This function accepts any code point from 0..C<IV_MAX> as input.
C<IV_MAX> is typically 0x7FFF_FFFF in a 32-bit word.

It is possible to forbid or warn on non-Unicode code points, or those that may
be problematic by using L</uvchr_to_utf8_flags>.

=cut
*/

/* This is also a macro */
PERL_CALLCONV U8*       Perl_uvchr_to_utf8(pTHX_ U8 *d, UV uv);

U8 *
Perl_uvchr_to_utf8(pTHX_ U8 *d, UV uv)
{
    return uvchr_to_utf8(d, uv);
}

/*
=for apidoc uvchr_to_utf8_flags

Adds the UTF-8 representation of the native code point C<uv> to the end
of the string C<d>; C<d> should have at least C<UVCHR_SKIP(uv)+1> (up to
C<UTF8_MAXBYTES+1>) free bytes available.  The return value is the pointer to
the byte after the end of the new character.  In other words,

    d = uvchr_to_utf8_flags(d, uv, flags);

or, in most cases,

    d = uvchr_to_utf8_flags(d, uv, 0);

This is the Unicode-aware way of saying

    *(d++) = uv;

If C<flags> is 0, this function accepts any code point from 0..C<IV_MAX> as
input.  C<IV_MAX> is typically 0x7FFF_FFFF in a 32-bit word.

Specifying C<flags> can further restrict what is allowed and not warned on, as
follows:

If C<uv> is a Unicode surrogate code point and C<UNICODE_WARN_SURROGATE> is set,
the function will raise a warning, provided UTF8 warnings are enabled.  If
instead C<UNICODE_DISALLOW_SURROGATE> is set, the function will fail and return
NULL.  If both flags are set, the function will both warn and return NULL.

Similarly, the C<UNICODE_WARN_NONCHAR> and C<UNICODE_DISALLOW_NONCHAR> flags
affect how the function handles a Unicode non-character.

And likewise, the C<UNICODE_WARN_SUPER> and C<UNICODE_DISALLOW_SUPER> flags
affect the handling of code points that are above the Unicode maximum of
0x10FFFF.  Languages other than Perl may not be able to accept files that
contain these.

The flag C<UNICODE_WARN_ILLEGAL_INTERCHANGE> selects all three of
the above WARN flags; and C<UNICODE_DISALLOW_ILLEGAL_INTERCHANGE> selects all
three DISALLOW flags.  C<UNICODE_DISALLOW_ILLEGAL_INTERCHANGE> restricts the
allowed inputs to the strict UTF-8 traditionally defined by Unicode.
Similarly, C<UNICODE_WARN_ILLEGAL_C9_INTERCHANGE> and
C<UNICODE_DISALLOW_ILLEGAL_C9_INTERCHANGE> are shortcuts to select the
above-Unicode and surrogate flags, but not the non-character ones, as
defined in
L<Unicode Corrigendum #9|https://www.unicode.org/versions/corrigendum9.html>.
See L<perlunicode/Noncharacter code points>.

Extremely high code points were never specified in any standard, and require an
extension to UTF-8 to express, which Perl does.  It is likely that programs
written in something other than Perl would not be able to read files that
contain these; nor would Perl understand files written by something that uses a
different extension.  For these reasons, there is a separate set of flags that
can warn and/or disallow these extremely high code points, even if other
above-Unicode ones are accepted.  They are the C<UNICODE_WARN_PERL_EXTENDED>
and C<UNICODE_DISALLOW_PERL_EXTENDED> flags.  For more information see
C<L</UTF8_GOT_PERL_EXTENDED>>.  Of course C<UNICODE_DISALLOW_SUPER> will
treat all above-Unicode code points, including these, as malformations.  (Note
that the Unicode standard considers anything above 0x10FFFF to be illegal, but
there are standards predating it that allow up to 0x7FFF_FFFF (2**31 -1))

A somewhat misleadingly named synonym for C<UNICODE_WARN_PERL_EXTENDED> is
retained for backward compatibility: C<UNICODE_WARN_ABOVE_31_BIT>.  Similarly,
C<UNICODE_DISALLOW_ABOVE_31_BIT> is usable instead of the more accurately named
C<UNICODE_DISALLOW_PERL_EXTENDED>.  The names are misleading because on EBCDIC
platforms,these flags can apply to code points that actually do fit in 31 bits.
The new names accurately describe the situation in all cases.

=for apidoc Amnh||UNICODE_DISALLOW_ABOVE_31_BIT
=for apidoc Amnh||UNICODE_DISALLOW_ILLEGAL_C9_INTERCHANGE
=for apidoc Amnh||UNICODE_DISALLOW_ILLEGAL_INTERCHANGE
=for apidoc Amnh||UNICODE_DISALLOW_NONCHAR
=for apidoc Amnh||UNICODE_DISALLOW_PERL_EXTENDED
=for apidoc Amnh||UNICODE_DISALLOW_SUPER
=for apidoc Amnh||UNICODE_DISALLOW_SURROGATE
=for apidoc Amnh||UNICODE_WARN_ABOVE_31_BIT
=for apidoc Amnh||UNICODE_WARN_ILLEGAL_C9_INTERCHANGE
=for apidoc Amnh||UNICODE_WARN_ILLEGAL_INTERCHANGE
=for apidoc Amnh||UNICODE_WARN_NONCHAR
=for apidoc Amnh||UNICODE_WARN_PERL_EXTENDED
=for apidoc Amnh||UNICODE_WARN_SUPER
=for apidoc Amnh||UNICODE_WARN_SURROGATE

=cut
*/

/* This is also a macro */
PERL_CALLCONV U8*       Perl_uvchr_to_utf8_flags(pTHX_ U8 *d, UV uv, UV flags);

U8 *
Perl_uvchr_to_utf8_flags(pTHX_ U8 *d, UV uv, UV flags)
{
    return uvchr_to_utf8_flags(d, uv, flags);
}

PERL_STATIC_INLINE int
S_is_utf8_overlong(const U8 * const s, const STRLEN len)
{
    /* Returns an int indicating whether or not the UTF-8 sequence from 's' to
     * 's' + 'len' - 1 is an overlong.  It returns 1 if it is an overlong; 0 if
     * it isn't, and -1 if there isn't enough information to tell.  This last
     * return value can happen if the sequence is incomplete, missing some
     * trailing bytes that would form a complete character.  If there are
     * enough bytes to make a definitive decision, this function does so.
     * Usually 2 bytes are sufficient.
     *
     * Overlongs can occur whenever the number of continuation bytes changes.
     * That means whenever the number of leading 1 bits in a start byte
     * increases from the next lower start byte.  That happens for start bytes
     * C0, E0, F0, F8, FC, FE, and FF.
     */

    PERL_ARGS_ASSERT_IS_UTF8_OVERLONG;

    /* Each platform has overlongs after the start bytes given above (expressed
     * in I8 for EBCDIC).  The values below were found by manually inspecting
     * the UTF-8 patterns.  See the tables in utf8.h and utfebcdic.h. */

    switch (NATIVE_UTF8_TO_I8(s[0])) {
      default:
        assert(UTF8_IS_START(s[0]));
        return 0;

      case 0xC0:
      case 0xC1:
        return 1;

#ifdef EBCDIC

      case 0xC2:
      case 0xC3:
      case 0xC4:
      case 0xE0:
        return 1;
#else
      case 0xE0:
        return (len < 2) ? -1 : s[1] < 0xA0;
#endif

      case 0xF0:
        return (len < 2)
               ? -1
               : NATIVE_UTF8_TO_I8(s[1]) < UTF_MIN_CONTINUATION_BYTE + 0x10;
      case 0xF8:
        return (len < 2)
               ? -1
               : NATIVE_UTF8_TO_I8(s[1]) < UTF_MIN_CONTINUATION_BYTE + 0x08;
      case 0xFC:
        return (len < 2)
               ? -1
               : NATIVE_UTF8_TO_I8(s[1]) < UTF_MIN_CONTINUATION_BYTE + 0x04;
      case 0xFE:
        return (len < 2)
               ? -1
               : NATIVE_UTF8_TO_I8(s[1]) < UTF_MIN_CONTINUATION_BYTE + 0x02;
      case 0xFF:
        return isFF_overlong(s, len);
    }
}

PERL_STATIC_INLINE int
S_isFF_overlong(const U8 * const s, const STRLEN len)
{
    /* Returns an int indicating whether or not the UTF-8 sequence from 's' to
     * 'e' - 1 is an overlong beginning with \xFF.  It returns 1 if it is; 0 if
     * it isn't, and -1 if there isn't enough information to tell.  This last
     * return value can happen if the sequence is incomplete, missing some
     * trailing bytes that would form a complete character.  If there are
     * enough bytes to make a definitive decision, this function does so. */

    PERL_ARGS_ASSERT_ISFF_OVERLONG;

#ifdef EBCDIC
    /* This works on all three EBCDIC code pages traditionally supported by
     * perl */
#  define FF_OVERLONG_PREFIX "\xfe\x41\x41\x41\x41\x41\x41\x41"
#else
#  define FF_OVERLONG_PREFIX "\xff\x80\x80\x80\x80\x80\x80"
#endif

    /* To be an FF overlong, all the available bytes must match */
    if (LIKELY(memNE(s, FF_OVERLONG_PREFIX,
                     MIN(len, STRLENs(FF_OVERLONG_PREFIX)))))
    {
        return 0;
    }

    /* To be an FF overlong sequence, all the bytes in FF_OVERLONG_PREFIX must
     * be there; what comes after them doesn't matter.  See tables in utf8.h,
     * utfebcdic.h. */
    if (len >= STRLENs(FF_OVERLONG_PREFIX)) {
        return 1;
    }

    /* The missing bytes could cause the result to go one way or the other, so
     * the result is indeterminate */
    return -1;
}

/* At some point we may want to allow core to use up to UV_MAX */

#ifdef EBCDIC     /* Actually is I8 */
#  if defined(UV_IS_QUAD) /* These assume IV_MAX is 2**63-1, UV_MAX 2**64-1 */
#    define HIGHEST_REPRESENTABLE_UTF  "\xFF\xA7"
                              /* UV_MAX "\xFF\xAF" */
#  else      /* These assume IV_MAX is 2**31-1, UV_MAX 2**32-1 */
#    define HIGHEST_REPRESENTABLE_UTF  "\xFF\xA0\xA0\xA0\xA0\xA0\xA0\xA1"
                              /* UV_MAX "\xFF\xA0\xA0\xA0\xA0\xA0\xA0\xA3" */
#  endif
#else
#  if defined(UV_IS_QUAD)
#    define HIGHEST_REPRESENTABLE_UTF  "\xFF\x80\x87"
                              /* UV_MAX "\xFF\x80" */
#  else
#    define HIGHEST_REPRESENTABLE_UTF  "\xFD"
                              /* UV_MAX "\xFE\x83" */
#  endif
#endif

PERL_STATIC_INLINE int
S_does_utf8_overflow(const U8 * const s,
                     const U8 * e,
                     const bool consider_overlongs)
{
    /* Returns an int indicating whether or not the UTF-8 sequence from 's' to
     * 'e' - 1 would overflow an IV on this platform; that is if it represents
     * a code point larger than the highest representable code point.  It
     * returns 1 if it does overflow; 0 if it doesn't, and -1 if there isn't
     * enough information to tell.  This last return value can happen if the
     * sequence is incomplete, missing some trailing bytes that would form a
     * complete character.  If there are enough bytes to make a definitive
     * decision, this function does so.
     *
     * If 'consider_overlongs' is TRUE, the function checks for the possibility
     * that the sequence is an overlong that doesn't overflow.  Otherwise, it
     * assumes the sequence is not an overlong.  This can give different
     * results only on ASCII 32-bit platforms.
     *
     * (For ASCII platforms, we could use memcmp() because we don't have to
     * convert each byte to I8, but it's very rare input indeed that would
     * approach overflow, so the loop below will likely only get executed once.)
     *
     */
    const STRLEN len = e - s;
    const U8 *x;
    const U8 * y = (const U8 *) HIGHEST_REPRESENTABLE_UTF;
    int is_overlong = 0;

    PERL_ARGS_ASSERT_DOES_UTF8_OVERFLOW;

    for (x = s; x < e; x++, y++) {

        /* 'y' is set up to not include the trailing bytes that are all the
         * maximum possible continuation byte.  So when we reach the end of 'y'
         * (known to be NUL terminated), it is impossible for 'x' to contain
         * bytes larger than those omitted bytes, and therefore 'x' can't
         * overflow */
        if (*y == '\0') {
            return 0;
        }

        /* If this byte is less than the corresponding highest non-overflowing
         * UTF-8, the sequence doesn't overflow */
        if (NATIVE_UTF8_TO_I8(*x) < *y) {
            return 0;
        }

        if (UNLIKELY(NATIVE_UTF8_TO_I8(*x) > *y)) {
            goto overflows_if_not_overlong;
        }
    }

    /* Got to the end, and all bytes are the same.  If the input is a whole
     * character, it doesn't overflow.  And if it is a partial character,
     * there's not enough information to tell */
    return (len >= STRLENs(HIGHEST_REPRESENTABLE_UTF)) ? 0 : -1;

  overflows_if_not_overlong:

    /* Here, a well-formed sequence overflows.  If we are assuming
     * well-formedness, return that it overflows. */
    if (! consider_overlongs) {
        return 1;
    }

    /* Here, it could be the overlong malformation, and might not actually
     * overflow if you were to calculate it out.
     *
     * See if it actually is overlong */
    is_overlong = is_utf8_overlong(s, len);

    /* If it isn't overlong, is well-formed, so overflows */
    if (is_overlong == 0) {
        return 1;
    }

    /* Not long enough to determine */
    if (is_overlong < 0) {
        return -1;
    }

    /* Here, it appears to overflow, but it is also overlong */

#if 6 * UTF_CONTINUATION_BYTE_INFO_BITS <= IVSIZE * CHARBITS

    /* On many platforms, it is impossible for an overlong to overflow.  For
     * these, no further work is necessary: we can return immediately that this
     * overlong that is an apparent overflow actually isn't
     *
     * To see why, note that a length_N sequence can represent as overlongs all
     * the code points representable by shorter length sequences, but no
     * higher.  If it could represent a higher code point without being an
     * overlong, we wouldn't have had to increase the sequence length!
     *
     * The highest possible start byte is FF; the next highest is FE.  The
     * highest code point representable as an overlong on the platform is thus
     * the highest code point representable by a non-overlong sequence whose
     * start byte is FE.  If that value doesn't overflow the platform's word
     * size, overlongs can't overflow.
     *
     * FE consists of 7 bytes total; the FE start byte contributes 0 bits of
     * information (the high 7 bits, all ones, say that the sequence is 7 bytes
     * long, and the bottom, zero, bit is s placeholder. That leaves the 6
     * continuation bytes to contribute UTF_CONTINUATION_BYTE_INFO_BITS each.
      If that number of bits doesn't exceed the word size, it can't overflow. */

    return 0;

#else

    /* In practice, only a 32-bit ASCII box gets here.  The FE start byte can
     * represent, as an overlong, the highest code point representable by an FD
     * start byte, which is 5*6 continuation bytes of info plus one bit from
     * the start byte, or 31 bits.  That doesn't overflow.  More explicitly:
     * \xFD\xBF\xBF\xBF\xBF\xBF evaluates to 0x7FFFFFFF = 2*31 - 1.
     *
     * That means only the FF start byte can have an overflowing overlong. */
    if (*s < 0xFF) {
        return 0;
    }

    /* The sequence \xff\x80\x80\x80\x80\x80\x80\x82 is an overlong that
     * evaluates to 2**31, so overflows an IV.  For a UV it's
     *              \xff\x80\x80\x80\x80\x80\x80\x83 = 2**32 */
#  define OVERFLOWS  "\xff\x80\x80\x80\x80\x80\x80\x82"

    if (e - s < (Ptrdiff_t) STRLENs(OVERFLOWS)) {   /* Not enough info */
         return -1;
    }

#  define strnGE(s1,s2,l) (strncmp(s1,s2,l) >= 0)

    return strnGE((const char *) s, OVERFLOWS, STRLENs(OVERFLOWS));

#endif

}

STRLEN
Perl_is_utf8_char_helper_(const U8 * const s, const U8 * e, const U32 flags)
{
    SSize_t len, full_len;

    /* An internal helper function.
     *
     * On input:
     *  's' is a string, which is known to be syntactically valid UTF-8 as far
     *      as (e - 1); e > s must hold.
     *  'e' This function is allowed to look at any byte from 's'...'e-1', but
     *      nowhere else.  The function has to cope as best it can if that
     *      sequence does not form a full character.
     * 'flags' can be 0, or any combination of the UTF8_DISALLOW_foo flags
     *      accepted by L</utf8n_to_uvchr>.  If non-zero, this function returns
     *      0 if it determines the input will match something disallowed.
     * On output:
     *  The return is the number of bytes required to represent the code point
     *  if it isn't disallowed by 'flags'; 0 otherwise.  Be aware that if the
     *  input is for a partial character, a successful return will be larger
     *  than 'e - s'.
     *
     *  If *s..*(e-1) is only for a partial character, the function will return
     *  non-zero if there is any sequence of well-formed UTF-8 that, when
     *  appended to the input sequence, could result in an allowed code point;
     *  otherwise it returns 0.  Non characters cannot be determined based on
     *  partial character input.  But many  of the other excluded types can be
     *  determined with just the first one or two bytes.
     *
     */

    PERL_ARGS_ASSERT_IS_UTF8_CHAR_HELPER_;

    assert(e > s);
    assert(0 == (flags & ~(UTF8_DISALLOW_ILLEGAL_INTERCHANGE
                          |UTF8_DISALLOW_PERL_EXTENDED)));

    full_len = UTF8SKIP(s);

    len = e - s;
    if (len > full_len) {
        e = s + full_len;
        len = full_len;
    }

    switch (full_len) {
        bool is_super;

      default: /* Extended */
        if (flags & UTF8_DISALLOW_PERL_EXTENDED) {
            return 0;
        }

        /* FALLTHROUGH */

      case 6 + ONE_IF_EBCDIC_ZERO_IF_NOT:   /* above Unicode */
      case 5 + ONE_IF_EBCDIC_ZERO_IF_NOT:   /* above Unicode */

        if (flags & UTF8_DISALLOW_SUPER) {
            return 0;                       /* Above Unicode */
        }

        return full_len;

      case 4 + ONE_IF_EBCDIC_ZERO_IF_NOT:
        is_super = (   UNLIKELY(NATIVE_UTF8_TO_I8(s[0]) > UTF_START_BYTE_110000_)
                    || (   len > 1
                        && NATIVE_UTF8_TO_I8(s[0]) == UTF_START_BYTE_110000_
                        && NATIVE_UTF8_TO_I8(s[1])
                                                >= UTF_FIRST_CONT_BYTE_110000_));
        if (is_super) {
            if (flags & UTF8_DISALLOW_SUPER) {
                return 0;
            }
        }
        else if (   (flags & UTF8_DISALLOW_NONCHAR)
                 && len == full_len
                 && UNLIKELY(is_LARGER_NON_CHARS_utf8(s)))
        {
            return 0;
        }

        return full_len;

      case 3 + ONE_IF_EBCDIC_ZERO_IF_NOT:

        if (! isUTF8_POSSIBLY_PROBLEMATIC(s[0]) || len < 2) {
            return full_len;
        }

        if (   (flags & UTF8_DISALLOW_SURROGATE)
            &&  UNLIKELY(is_SURROGATE_utf8(s)))
        {
            return 0;       /* Surrogate */
        }

        if (  (flags & UTF8_DISALLOW_NONCHAR)
            && len == full_len
            && UNLIKELY(is_SHORTER_NON_CHARS_utf8(s)))
        {
            return 0;
        }

        return full_len;

      /* The lower code points don't have any disallowable characters */
#ifdef EBCDIC
      case 3:
        return full_len;
#endif

      case 2:
      case 1:
        return full_len;
    }
}

Size_t
Perl_is_utf8_FF_helper_(const U8 * const s0, const U8 * const e,
                        const bool require_partial)
{
    /* This is called to determine if the UTF-8 sequence starting at s0 and
     * continuing for up to one full character of bytes, but looking no further
     * than 'e - 1', is legal.  *s0 must be 0xFF (or whatever the native
     * equivalent of FF in I8 on EBCDIC platforms is).  This marks it as being
     * for the largest code points recognized by Perl, the ones that require
     * the most UTF-8 bytes per character to represent (somewhat less than
     * twice the size of the next longest kind).  This sequence will only ever
     * be Perl extended UTF-8.
     *
     * The routine returns 0 if the sequence is not fully valid, syntactically
     * or semantically.  That means it checks that everything following the
     * start byte is a continuation byte, and that it doesn't overflow, nor is
     * an overlong representation.
     *
     * If 'require_partial' is FALSE, the routine returns non-zero only if the
     * input (as far as 'e-1') is a full character.  The return is the count of
     * the bytes in the character.
     *
     * If 'require_partial' is TRUE, the routine returns non-zero only if the
     * input as far as 'e-1' is a partial, not full character, with no
     * malformations found before position 'e'.  The return is either just
     * FALSE, or TRUE.  */

    const U8 *s = s0 + 1;
    const U8 *send = e;

    PERL_ARGS_ASSERT_IS_UTF8_FF_HELPER_;

    assert(s0 < e);
    assert(*s0 == I8_TO_NATIVE_UTF8(0xFF));

    send = s + MIN(UTF8_MAXBYTES - 1, e - s);
    while (s < send) {
        if (! UTF8_IS_CONTINUATION(*s)) {
            return 0;
        }

        s++;
    }

    if (0 < does_utf8_overflow(s0, e,
                               FALSE /* Don't consider_overlongs */
    )) {
        return 0;
    }

    if (0 < isFF_overlong(s0, e - s0)) {
        return 0;
    }

    /* Here, the character is valid as far as it got.  Check if got a partial
     * character */
    if (s - s0 < UTF8_MAXBYTES) {
        return (require_partial) ? 1 : 0;
    }

    /* Here, got a full character */
    return (require_partial) ? 0 : UTF8_MAXBYTES;
}

char *
Perl__byte_dump_string(pTHX_ const U8 * const start, const STRLEN len, const bool format)
{
    /* Returns a mortalized C string that is a displayable copy of the 'len'
     * bytes starting at 'start'.  'format' gives how to display each byte.
     * Currently, there are only two formats, so it is currently a bool:
     *      0   \xab
     *      1    ab         (that is a space between two hex digit bytes)
     */

    const STRLEN output_len = 4 * len + 1;  /* 4 bytes per each input, plus a
                                               trailing NUL */
    const U8 * s = start;
    const U8 * const e = start + len;
    char * output;
    char * d;

    PERL_ARGS_ASSERT__BYTE_DUMP_STRING;

    Newx(output, output_len, char);
    SAVEFREEPV(output);

    d = output;
    for (s = start; s < e; s++) {
        const unsigned high_nibble = (*s & 0xF0) >> 4;
        const unsigned low_nibble =  (*s & 0x0F);

        if (format) {
            if (s > start) {
                *d++ = ' ';
            }
        }
        else {
            *d++ = '\\';
            *d++ = 'x';
        }

        if (high_nibble < 10) {
            *d++ = high_nibble + '0';
        }
        else {
            *d++ = high_nibble - 10 + 'a';
        }

        if (low_nibble < 10) {
            *d++ = low_nibble + '0';
        }
        else {
            *d++ = low_nibble - 10 + 'a';
        }
    }

    *d = '\0';
    return output;
}

PERL_STATIC_INLINE char *
S_unexpected_non_continuation_text(pTHX_ const U8 * const s,

                                         /* Max number of bytes to print */
                                         STRLEN print_len,

                                         /* Which one is the non-continuation */
                                         const STRLEN non_cont_byte_pos,

                                         /* How many bytes should there be? */
                                         const STRLEN expect_len)
{
    /* Return the malformation warning text for an unexpected continuation
     * byte. */

    const char * const where = (non_cont_byte_pos == 1)
                               ? "immediately"
                               : Perl_form(aTHX_ "%d bytes",
                                                 (int) non_cont_byte_pos);
    const U8 * x = s + non_cont_byte_pos;
    const U8 * e = s + print_len;

    PERL_ARGS_ASSERT_UNEXPECTED_NON_CONTINUATION_TEXT;

    /* We don't need to pass this parameter, but since it has already been
     * calculated, it's likely faster to pass it; verify under DEBUGGING */
    assert(expect_len == UTF8SKIP(s));

    /* As a defensive coding measure, don't output anything past a NUL.  Such
     * bytes shouldn't be in the middle of a malformation, and could mark the
     * end of the allocated string, and what comes after is undefined */
    for (; x < e; x++) {
        if (*x == '\0') {
            x++;            /* Output this particular NUL */
            break;
        }
    }

    return Perl_form(aTHX_ "%s: %s (unexpected non-continuation byte 0x%02x,"
                           " %s after start byte 0x%02x; need %d bytes, got %d)",
                           malformed_text,
                           _byte_dump_string(s, x - s, 0),
                           *(s + non_cont_byte_pos),
                           where,
                           *s,
                           (int) expect_len,
                           (int) non_cont_byte_pos);
}

/*

=for apidoc utf8n_to_uvchr

THIS FUNCTION SHOULD BE USED IN ONLY VERY SPECIALIZED CIRCUMSTANCES.
Most code should use L</utf8_to_uvchr_buf>() rather than call this
directly.

Bottom level UTF-8 decode routine.
Returns the native code point value of the first character in the string C<s>,
which is assumed to be in UTF-8 (or UTF-EBCDIC) encoding, and no longer than
C<curlen> bytes; C<*retlen> (if C<retlen> isn't NULL) will be set to
the length, in bytes, of that character.

The value of C<flags> determines the behavior when C<s> does not point to a
well-formed UTF-8 character.  If C<flags> is 0, encountering a malformation
causes zero to be returned and C<*retlen> is set so that (S<C<s> + C<*retlen>>)
is the next possible position in C<s> that could begin a non-malformed
character.  Also, if UTF-8 warnings haven't been lexically disabled, a warning
is raised.  Some UTF-8 input sequences may contain multiple malformations.
This function tries to find every possible one in each call, so multiple
warnings can be raised for the same sequence.

Various ALLOW flags can be set in C<flags> to allow (and not warn on)
individual types of malformations, such as the sequence being overlong (that
is, when there is a shorter sequence that can express the same code point;
overlong sequences are expressly forbidden in the UTF-8 standard due to
potential security issues).  Another malformation example is the first byte of
a character not being a legal first byte.  See F<utf8.h> for the list of such
flags.  Even if allowed, this function generally returns the Unicode
REPLACEMENT CHARACTER when it encounters a malformation.  There are flags in
F<utf8.h> to override this behavior for the overlong malformations, but don't
do that except for very specialized purposes.

The C<UTF8_CHECK_ONLY> flag overrides the behavior when a non-allowed (by other
flags) malformation is found.  If this flag is set, the routine assumes that
the caller will raise a warning, and this function will silently just set
C<retlen> to C<-1> (cast to C<STRLEN>) and return zero.

Note that this API requires disambiguation between successful decoding a C<NUL>
character, and an error return (unless the C<UTF8_CHECK_ONLY> flag is set), as
in both cases, 0 is returned, and, depending on the malformation, C<retlen> may
be set to 1.  To disambiguate, upon a zero return, see if the first byte of
C<s> is 0 as well.  If so, the input was a C<NUL>; if not, the input had an
error.  Or you can use C<L</utf8n_to_uvchr_error>>.

Certain code points are considered problematic.  These are Unicode surrogates,
Unicode non-characters, and code points above the Unicode maximum of 0x10FFFF.
By default these are considered regular code points, but certain situations
warrant special handling for them, which can be specified using the C<flags>
parameter.  If C<flags> contains C<UTF8_DISALLOW_ILLEGAL_INTERCHANGE>, all
three classes are treated as malformations and handled as such.  The flags
C<UTF8_DISALLOW_SURROGATE>, C<UTF8_DISALLOW_NONCHAR>, and
C<UTF8_DISALLOW_SUPER> (meaning above the legal Unicode maximum) can be set to
disallow these categories individually.  C<UTF8_DISALLOW_ILLEGAL_INTERCHANGE>
restricts the allowed inputs to the strict UTF-8 traditionally defined by
Unicode.  Use C<UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE> to use the strictness
definition given by
L<Unicode Corrigendum #9|https://www.unicode.org/versions/corrigendum9.html>.
The difference between traditional strictness and C9 strictness is that the
latter does not forbid non-character code points.  (They are still discouraged,
however.)  For more discussion see L<perlunicode/Noncharacter code points>.

The flags C<UTF8_WARN_ILLEGAL_INTERCHANGE>,
C<UTF8_WARN_ILLEGAL_C9_INTERCHANGE>, C<UTF8_WARN_SURROGATE>,
C<UTF8_WARN_NONCHAR>, and C<UTF8_WARN_SUPER> will cause warning messages to be
raised for their respective categories, but otherwise the code points are
considered valid (not malformations).  To get a category to both be treated as
a malformation and raise a warning, specify both the WARN and DISALLOW flags.
(But note that warnings are not raised if lexically disabled nor if
C<UTF8_CHECK_ONLY> is also specified.)

Extremely high code points were never specified in any standard, and require an
extension to UTF-8 to express, which Perl does.  It is likely that programs
written in something other than Perl would not be able to read files that
contain these; nor would Perl understand files written by something that uses a
different extension.  For these reasons, there is a separate set of flags that
can warn and/or disallow these extremely high code points, even if other
above-Unicode ones are accepted.  They are the C<UTF8_WARN_PERL_EXTENDED> and
C<UTF8_DISALLOW_PERL_EXTENDED> flags.  For more information see
C<L</UTF8_GOT_PERL_EXTENDED>>.  Of course C<UTF8_DISALLOW_SUPER> will treat all
above-Unicode code points, including these, as malformations.
(Note that the Unicode standard considers anything above 0x10FFFF to be
illegal, but there are standards predating it that allow up to 0x7FFF_FFFF
(2**31 -1))

A somewhat misleadingly named synonym for C<UTF8_WARN_PERL_EXTENDED> is
retained for backward compatibility: C<UTF8_WARN_ABOVE_31_BIT>.  Similarly,
C<UTF8_DISALLOW_ABOVE_31_BIT> is usable instead of the more accurately named
C<UTF8_DISALLOW_PERL_EXTENDED>.  The names are misleading because these flags
can apply to code points that actually do fit in 31 bits.  This happens on
EBCDIC platforms, and sometimes when the L<overlong
malformation|/C<UTF8_GOT_LONG>> is also present.  The new names accurately
describe the situation in all cases.


All other code points corresponding to Unicode characters, including private
use and those yet to be assigned, are never considered malformed and never
warn.

=for apidoc Amnh||UTF8_CHECK_ONLY
=for apidoc Amnh||UTF8_DISALLOW_ILLEGAL_INTERCHANGE
=for apidoc Amnh||UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE
=for apidoc Amnh||UTF8_DISALLOW_SURROGATE
=for apidoc Amnh||UTF8_DISALLOW_NONCHAR
=for apidoc Amnh||UTF8_DISALLOW_SUPER
=for apidoc Amnh||UTF8_WARN_ILLEGAL_INTERCHANGE
=for apidoc Amnh||UTF8_WARN_ILLEGAL_C9_INTERCHANGE
=for apidoc Amnh||UTF8_WARN_SURROGATE
=for apidoc Amnh||UTF8_WARN_NONCHAR
=for apidoc Amnh||UTF8_WARN_SUPER
=for apidoc Amnh||UTF8_WARN_PERL_EXTENDED
=for apidoc Amnh||UTF8_DISALLOW_PERL_EXTENDED

=cut

Also implemented as a macro in utf8.h
*/

UV
Perl_utf8n_to_uvchr(const U8 *s,
                    STRLEN curlen,
                    STRLEN *retlen,
                    const U32 flags)
{
    PERL_ARGS_ASSERT_UTF8N_TO_UVCHR;

    return utf8n_to_uvchr_error(s, curlen, retlen, flags, NULL);
}

/*

=for apidoc utf8n_to_uvchr_error

THIS FUNCTION SHOULD BE USED IN ONLY VERY SPECIALIZED CIRCUMSTANCES.
Most code should use L</utf8_to_uvchr_buf>() rather than call this
directly.

This function is for code that needs to know what the precise malformation(s)
are when an error is found.  If you also need to know the generated warning
messages, use L</utf8n_to_uvchr_msgs>() instead.

It is like C<L</utf8n_to_uvchr>> but it takes an extra parameter placed after
all the others, C<errors>.  If this parameter is 0, this function behaves
identically to C<L</utf8n_to_uvchr>>.  Otherwise, C<errors> should be a pointer
to a C<U32> variable, which this function sets to indicate any errors found.
Upon return, if C<*errors> is 0, there were no errors found.  Otherwise,
C<*errors> is the bit-wise C<OR> of the bits described in the list below.  Some
of these bits will be set if a malformation is found, even if the input
C<flags> parameter indicates that the given malformation is allowed; those
exceptions are noted:

=over 4

=item C<UTF8_GOT_PERL_EXTENDED>

The input sequence is not standard UTF-8, but a Perl extension.  This bit is
set only if the input C<flags> parameter contains either the
C<UTF8_DISALLOW_PERL_EXTENDED> or the C<UTF8_WARN_PERL_EXTENDED> flags.

Code points above 0x7FFF_FFFF (2**31 - 1) were never specified in any standard,
and so some extension must be used to express them.  Perl uses a natural
extension to UTF-8 to represent the ones up to 2**36-1, and invented a further
extension to represent even higher ones, so that any code point that fits in a
64-bit word can be represented.  Text using these extensions is not likely to
be portable to non-Perl code.  We lump both of these extensions together and
refer to them as Perl extended UTF-8.  There exist other extensions that people
have invented, incompatible with Perl's.

On EBCDIC platforms starting in Perl v5.24, the Perl extension for representing
extremely high code points kicks in at 0x3FFF_FFFF (2**30 -1), which is lower
than on ASCII.  Prior to that, code points 2**31 and higher were simply
unrepresentable, and a different, incompatible method was used to represent
code points between 2**30 and 2**31 - 1.

On both platforms, ASCII and EBCDIC, C<UTF8_GOT_PERL_EXTENDED> is set if
Perl extended UTF-8 is used.

In earlier Perls, this bit was named C<UTF8_GOT_ABOVE_31_BIT>, which you still
may use for backward compatibility.  That name is misleading, as this flag may
be set when the code point actually does fit in 31 bits.  This happens on
EBCDIC platforms, and sometimes when the L<overlong
malformation|/C<UTF8_GOT_LONG>> is also present.  The new name accurately
describes the situation in all cases.

=item C<UTF8_GOT_CONTINUATION>

The input sequence was malformed in that the first byte was a UTF-8
continuation byte.

=item C<UTF8_GOT_EMPTY>

The input C<curlen> parameter was 0.

=item C<UTF8_GOT_LONG>

The input sequence was malformed in that there is some other sequence that
evaluates to the same code point, but that sequence is shorter than this one.

Until Unicode 3.1, it was legal for programs to accept this malformation, but
it was discovered that this created security issues.

=item C<UTF8_GOT_NONCHAR>

The code point represented by the input UTF-8 sequence is for a Unicode
non-character code point.
This bit is set only if the input C<flags> parameter contains either the
C<UTF8_DISALLOW_NONCHAR> or the C<UTF8_WARN_NONCHAR> flags.

=item C<UTF8_GOT_NON_CONTINUATION>

The input sequence was malformed in that a non-continuation type byte was found
in a position where only a continuation type one should be.  See also
C<L</UTF8_GOT_SHORT>>.

=item C<UTF8_GOT_OVERFLOW>

The input sequence was malformed in that it is for a code point that is not
representable in the number of bits available in an IV on the current platform.

=item C<UTF8_GOT_SHORT>

The input sequence was malformed in that C<curlen> is smaller than required for
a complete sequence.  In other words, the input is for a partial character
sequence.


C<UTF8_GOT_SHORT> and C<UTF8_GOT_NON_CONTINUATION> both indicate a too short
sequence.  The difference is that C<UTF8_GOT_NON_CONTINUATION> indicates always
that there is an error, while C<UTF8_GOT_SHORT> means that an incomplete
sequence was looked at.   If no other flags are present, it means that the
sequence was valid as far as it went.  Depending on the application, this could
mean one of three things:

=over

=item *

The C<curlen> length parameter passed in was too small, and the function was
prevented from examining all the necessary bytes.

=item *

The buffer being looked at is based on reading data, and the data received so
far stopped in the middle of a character, so that the next read will
read the remainder of this character.  (It is up to the caller to deal with the
split bytes somehow.)

=item *

This is a real error, and the partial sequence is all we're going to get.

=back

=item C<UTF8_GOT_SUPER>

The input sequence was malformed in that it is for a non-Unicode code point;
that is, one above the legal Unicode maximum.
This bit is set only if the input C<flags> parameter contains either the
C<UTF8_DISALLOW_SUPER> or the C<UTF8_WARN_SUPER> flags.

=item C<UTF8_GOT_SURROGATE>

The input sequence was malformed in that it is for a -Unicode UTF-16 surrogate
code point.
This bit is set only if the input C<flags> parameter contains either the
C<UTF8_DISALLOW_SURROGATE> or the C<UTF8_WARN_SURROGATE> flags.

=back

To do your own error handling, call this function with the C<UTF8_CHECK_ONLY>
flag to suppress any warnings, and then examine the C<*errors> return.

=for apidoc Amnh||UTF8_GOT_PERL_EXTENDED
=for apidoc Amnh||UTF8_GOT_CONTINUATION
=for apidoc Amnh||UTF8_GOT_EMPTY
=for apidoc Amnh||UTF8_GOT_LONG
=for apidoc Amnh||UTF8_GOT_NONCHAR
=for apidoc Amnh||UTF8_GOT_NON_CONTINUATION
=for apidoc Amnh||UTF8_GOT_OVERFLOW
=for apidoc Amnh||UTF8_GOT_SHORT
=for apidoc Amnh||UTF8_GOT_SUPER
=for apidoc Amnh||UTF8_GOT_SURROGATE

=cut

Also implemented as a macro in utf8.h
*/

UV
Perl_utf8n_to_uvchr_error(const U8 *s,
                          STRLEN curlen,
                          STRLEN *retlen,
                          const U32 flags,
                          U32 * errors)
{
    PERL_ARGS_ASSERT_UTF8N_TO_UVCHR_ERROR;

    return utf8n_to_uvchr_msgs(s, curlen, retlen, flags, errors, NULL);
}

/*

=for apidoc utf8n_to_uvchr_msgs

THIS FUNCTION SHOULD BE USED IN ONLY VERY SPECIALIZED CIRCUMSTANCES.
Most code should use L</utf8_to_uvchr_buf>() rather than call this
directly.

This function is for code that needs to know what the precise malformation(s)
are when an error is found, and wants the corresponding warning and/or error
messages to be returned to the caller rather than be displayed.  All messages
that would have been displayed if all lexical warnings are enabled will be
returned.

It is just like C<L</utf8n_to_uvchr_error>> but it takes an extra parameter
placed after all the others, C<msgs>.  If this parameter is 0, this function
behaves identically to C<L</utf8n_to_uvchr_error>>.  Otherwise, C<msgs> should
be a pointer to an C<AV *> variable, in which this function creates a new AV to
contain any appropriate messages.  The elements of the array are ordered so
that the first message that would have been displayed is in the 0th element,
and so on.  Each element is a hash with three key-value pairs, as follows:

=over 4

=item C<text>

The text of the message as a C<SVpv>.

=item C<warn_categories>

The warning category (or categories) packed into a C<SVuv>.

=item C<flag>

A single flag bit associated with this message, in a C<SVuv>.
The bit corresponds to some bit in the C<*errors> return value,
such as C<UTF8_GOT_LONG>.

=back

It's important to note that specifying this parameter as non-null will cause
any warnings this function would otherwise generate to be suppressed, and
instead be placed in C<*msgs>.  The caller can check the lexical warnings state
(or not) when choosing what to do with the returned messages.

If the flag C<UTF8_CHECK_ONLY> is passed, no warnings are generated, and hence
no AV is created.

The caller, of course, is responsible for freeing any returned AV.

=cut
*/

UV
Perl__utf8n_to_uvchr_msgs_helper(const U8 *s,
                               STRLEN curlen,
                               STRLEN *retlen,
                               const U32 flags,
                               U32 * errors,
                               AV ** msgs)
{
    const U8 * const s0 = s;
    const U8 * send = s0 + curlen;
    U32 possible_problems;  /* A bit is set here for each potential problem
                               found as we go along */
    UV uv;
    STRLEN expectlen;     /* How long should this sequence be? */
    STRLEN avail_len;     /* When input is too short, gives what that is */
    U32 discard_errors;   /* Used to save branches when 'errors' is NULL; this
                             gets set and discarded */

    /* The below are used only if there is both an overlong malformation and a
     * too short one.  Otherwise the first two are set to 's0' and 'send', and
     * the third not used at all */
    U8 * adjusted_s0;
    U8 temp_char_buf[UTF8_MAXBYTES + 1]; /* Used to avoid a Newx in this
                                            routine; see [perl #130921] */
    UV uv_so_far;
    dTHX;

    PERL_ARGS_ASSERT__UTF8N_TO_UVCHR_MSGS_HELPER;

    /* Here, is one of: a) malformed; b) a problematic code point (surrogate,
     * non-unicode, or nonchar); or c) on ASCII platforms, one of the Hangul
     * syllables that the dfa doesn't properly handle.  Quickly dispose of the
     * final case. */

    /* Each of the affected Hanguls starts with \xED */

    if (is_HANGUL_ED_utf8_safe(s0, send)) { /* Always false on EBCDIC */
        if (retlen) {
            *retlen = 3;
        }
        if (errors) {
            *errors = 0;
        }
        if (msgs) {
            *msgs = NULL;
        }

        return ((0xED & UTF_START_MASK(3)) << (2 * UTF_ACCUMULATION_SHIFT))
             | ((s0[1] & UTF_CONTINUATION_MASK) << UTF_ACCUMULATION_SHIFT)
             |  (s0[2] & UTF_CONTINUATION_MASK);
    }

    /* In conjunction with the exhaustive tests that can be enabled in
     * APItest/t/utf8_warn_base.pl, this can make sure the dfa does precisely
     * what it is intended to do, and that no flaws in it are masked by
     * dropping down and executing the code below
    assert(! isUTF8_CHAR(s0, send)
          || UTF8_IS_SURROGATE(s0, send)
          || UTF8_IS_SUPER(s0, send)
          || UTF8_IS_NONCHAR(s0,send));
    */

    s = s0;
    possible_problems = 0;
    expectlen = 0;
    avail_len = 0;
    discard_errors = 0;
    adjusted_s0 = (U8 *) s0;
    uv_so_far = 0;

    if (errors) {
        *errors = 0;
    }
    else {
        errors = &discard_errors;
    }

    /* The order of malformation tests here is important.  We should consume as
     * few bytes as possible in order to not skip any valid character.  This is
     * required by the Unicode Standard (section 3.9 of Unicode 6.0); see also
     * https://unicode.org/reports/tr36 for more discussion as to why.  For
     * example, once we've done a UTF8SKIP, we can tell the expected number of
     * bytes, and could fail right off the bat if the input parameters indicate
     * that there are too few available.  But it could be that just that first
     * byte is garbled, and the intended character occupies fewer bytes.  If we
     * blindly assumed that the first byte is correct, and skipped based on
     * that number, we could skip over a valid input character.  So instead, we
     * always examine the sequence byte-by-byte.
     *
     * We also should not consume too few bytes, otherwise someone could inject
     * things.  For example, an input could be deliberately designed to
     * overflow, and if this code bailed out immediately upon discovering that,
     * returning to the caller C<*retlen> pointing to the very next byte (one
     * which is actually part of the overflowing sequence), that could look
     * legitimate to the caller, which could discard the initial partial
     * sequence and process the rest, inappropriately.
     *
     * Some possible input sequences are malformed in more than one way.  This
     * function goes to lengths to try to find all of them.  This is necessary
     * for correctness, as the inputs may allow one malformation but not
     * another, and if we abandon searching for others after finding the
     * allowed one, we could allow in something that shouldn't have been.
     */

    if (UNLIKELY(curlen == 0)) {
        possible_problems |= UTF8_GOT_EMPTY;
        curlen = 0;
        uv = UNICODE_REPLACEMENT;
        goto ready_to_handle_errors;
    }

    /* We now know we can examine the first byte of the input */
    expectlen = UTF8SKIP(s);
    uv = *s;

    /* A well-formed UTF-8 character, as the vast majority of calls to this
     * function will be for, has this expected length.  For efficiency, set
     * things up here to return it.  It will be overridden only in those rare
     * cases where a malformation is found */
    if (retlen) {
        *retlen = expectlen;
    }

    /* A continuation character can't start a valid sequence */
    if (UNLIKELY(UTF8_IS_CONTINUATION(uv))) {
        possible_problems |= UTF8_GOT_CONTINUATION;
        curlen = 1;
        uv = UNICODE_REPLACEMENT;
        goto ready_to_handle_errors;
    }

    /* Here is not a continuation byte, nor an invariant.  The only thing left
     * is a start byte (possibly for an overlong).  (We can't use UTF8_IS_START
     * because it excludes start bytes like \xC0 that always lead to
     * overlongs.) */

    /* Convert to I8 on EBCDIC (no-op on ASCII), then remove the leading bits
     * that indicate the number of bytes in the character's whole UTF-8
     * sequence, leaving just the bits that are part of the value.  */
    uv = NATIVE_UTF8_TO_I8(uv) & UTF_START_MASK(expectlen);

    /* Setup the loop end point, making sure to not look past the end of the
     * input string, and flag it as too short if the size isn't big enough. */
    if (UNLIKELY(curlen < expectlen)) {
        possible_problems |= UTF8_GOT_SHORT;
        avail_len = curlen;
    }
    else {
        send = (U8*) s0 + expectlen;
    }

    /* Now, loop through the remaining bytes in the character's sequence,
     * accumulating each into the working value as we go. */
    for (s = s0 + 1; s < send; s++) {
        if (LIKELY(UTF8_IS_CONTINUATION(*s))) {
            uv = UTF8_ACCUMULATE(uv, *s);
            continue;
        }

        /* Here, found a non-continuation before processing all expected bytes.
         * This byte indicates the beginning of a new character, so quit, even
         * if allowing this malformation. */
        possible_problems |= UTF8_GOT_NON_CONTINUATION;
        break;
    } /* End of loop through the character's bytes */

    /* Save how many bytes were actually in the character */
    curlen = s - s0;

    /* Note that there are two types of too-short malformation.  One is when
     * there is actual wrong data before the normal termination of the
     * sequence.  The other is that the sequence wasn't complete before the end
     * of the data we are allowed to look at, based on the input 'curlen'.
     * This means that we were passed data for a partial character, but it is
     * valid as far as we saw.  The other is definitely invalid.  This
     * distinction could be important to a caller, so the two types are kept
     * separate.
     *
     * A convenience macro that matches either of the too-short conditions.  */
#   define UTF8_GOT_TOO_SHORT (UTF8_GOT_SHORT|UTF8_GOT_NON_CONTINUATION)

    if (UNLIKELY(possible_problems & UTF8_GOT_TOO_SHORT)) {
        uv_so_far = uv;
        uv = UNICODE_REPLACEMENT;
    }

    /* Check for overflow.  The algorithm requires us to not look past the end
     * of the current character, even if partial, so the upper limit is 's' */
    if (UNLIKELY(0 < does_utf8_overflow(s0, s,
                                         1 /* Do consider overlongs */
                                        )))
    {
        possible_problems |= UTF8_GOT_OVERFLOW;
        uv = UNICODE_REPLACEMENT;
    }

    /* Check for overlong.  If no problems so far, 'uv' is the correct code
     * point value.  Simply see if it is expressible in fewer bytes.  Otherwise
     * we must look at the UTF-8 byte sequence itself to see if it is for an
     * overlong */
    if (     (   LIKELY(! possible_problems)
              && UNLIKELY(expectlen > (STRLEN) OFFUNISKIP(uv)))
        || (       UNLIKELY(possible_problems)
            && (   UNLIKELY(! UTF8_IS_START(*s0))
                || (UNLIKELY(0 < is_utf8_overlong(s0, s - s0))))))
    {
        possible_problems |= UTF8_GOT_LONG;

        if (   UNLIKELY(   possible_problems & UTF8_GOT_TOO_SHORT)

                          /* The calculation in the 'true' branch of this 'if'
                           * below won't work if overflows, and isn't needed
                           * anyway.  Further below we handle all overflow
                           * cases */
            &&   LIKELY(! (possible_problems & UTF8_GOT_OVERFLOW)))
        {
            UV min_uv = uv_so_far;
            STRLEN i;

            /* Here, the input is both overlong and is missing some trailing
             * bytes.  There is no single code point it could be for, but there
             * may be enough information present to determine if what we have
             * so far is for an unallowed code point, such as for a surrogate.
             * The code further below has the intelligence to determine this,
             * but just for non-overlong UTF-8 sequences.  What we do here is
             * calculate the smallest code point the input could represent if
             * there were no too short malformation.  Then we compute and save
             * the UTF-8 for that, which is what the code below looks at
             * instead of the raw input.  It turns out that the smallest such
             * code point is all we need. */
            for (i = curlen; i < expectlen; i++) {
                min_uv = UTF8_ACCUMULATE(min_uv,
                                I8_TO_NATIVE_UTF8(UTF_MIN_CONTINUATION_BYTE));
            }

            adjusted_s0 = temp_char_buf;
            (void) uvoffuni_to_utf8_flags(adjusted_s0, min_uv, 0);
        }
    }

    /* Here, we have found all the possible problems, except for when the input
     * is for a problematic code point not allowed by the input parameters. */

                                /* uv is valid for overlongs */
    if (   (   (      LIKELY(! (possible_problems & ~UTF8_GOT_LONG))
                   && isUNICODE_POSSIBLY_PROBLEMATIC(uv))
            || (   UNLIKELY(possible_problems)

                          /* if overflow, we know without looking further
                           * precisely which of the problematic types it is,
                           * and we deal with those in the overflow handling
                           * code */
                && LIKELY(! (possible_problems & UTF8_GOT_OVERFLOW))
                && (   isUTF8_POSSIBLY_PROBLEMATIC(*adjusted_s0)
                    || UNLIKELY(UTF8_IS_PERL_EXTENDED(s0)))))
        && ((flags & ( UTF8_DISALLOW_NONCHAR
                      |UTF8_DISALLOW_SURROGATE
                      |UTF8_DISALLOW_SUPER
                      |UTF8_DISALLOW_PERL_EXTENDED
                      |UTF8_WARN_NONCHAR
                      |UTF8_WARN_SURROGATE
                      |UTF8_WARN_SUPER
                      |UTF8_WARN_PERL_EXTENDED))))
    {
        /* If there were no malformations, or the only malformation is an
         * overlong, 'uv' is valid */
        if (LIKELY(! (possible_problems & ~UTF8_GOT_LONG))) {
            if (UNLIKELY(UNICODE_IS_SURROGATE(uv))) {
                possible_problems |= UTF8_GOT_SURROGATE;
            }
            else if (UNLIKELY(UNICODE_IS_SUPER(uv))) {
                possible_problems |= UTF8_GOT_SUPER;
            }
            else if (UNLIKELY(UNICODE_IS_NONCHAR(uv))) {
                possible_problems |= UTF8_GOT_NONCHAR;
            }
        }
        else {  /* Otherwise, need to look at the source UTF-8, possibly
                   adjusted to be non-overlong */

            if (UNLIKELY(NATIVE_UTF8_TO_I8(*adjusted_s0)
                                                    > UTF_START_BYTE_110000_))
            {
                possible_problems |= UTF8_GOT_SUPER;
            }
            else if (curlen > 1) {
                if (UNLIKELY(   NATIVE_UTF8_TO_I8(*adjusted_s0)
                                                == UTF_START_BYTE_110000_
                             && NATIVE_UTF8_TO_I8(*(adjusted_s0 + 1))
                                                >= UTF_FIRST_CONT_BYTE_110000_))
                {
                    possible_problems |= UTF8_GOT_SUPER;
                }
                else if (UNLIKELY(is_SURROGATE_utf8(adjusted_s0))) {
                    possible_problems |= UTF8_GOT_SURROGATE;
                }
            }

            /* We need a complete well-formed UTF-8 character to discern
             * non-characters, so can't look for them here */
        }
    }

  ready_to_handle_errors:

    /* At this point:
     * curlen               contains the number of bytes in the sequence that
     *                      this call should advance the input by.
     * avail_len            gives the available number of bytes passed in, but
     *                      only if this is less than the expected number of
     *                      bytes, based on the code point's start byte.
     * possible_problems    is 0 if there weren't any problems; otherwise a bit
     *                      is set in it for each potential problem found.
     * uv                   contains the code point the input sequence
     *                      represents; or if there is a problem that prevents
     *                      a well-defined value from being computed, it is
     *                      some substitute value, typically the REPLACEMENT
     *                      CHARACTER.
     * s0                   points to the first byte of the character
     * s                    points to just after where we left off processing
     *                      the character
     * send                 points to just after where that character should
     *                      end, based on how many bytes the start byte tells
     *                      us should be in it, but no further than s0 +
     *                      avail_len
     */

    if (UNLIKELY(possible_problems)) {
        bool disallowed = FALSE;
        const U32 orig_problems = possible_problems;

        if (msgs) {
            *msgs = NULL;
        }

        while (possible_problems) { /* Handle each possible problem */
            U32 pack_warn = 0;
            char * message = NULL;
            U32 this_flag_bit = 0;

            /* Each 'if' clause handles one problem.  They are ordered so that
             * the first ones' messages will be displayed before the later
             * ones; this is kinda in decreasing severity order.  But the
             * overlong must come last, as it changes 'uv' looked at by the
             * others */
            if (possible_problems & UTF8_GOT_OVERFLOW) {

                /* Overflow means also got a super and are using Perl's
                 * extended UTF-8, but we handle all three cases here */
                possible_problems
                  &= ~(UTF8_GOT_OVERFLOW|UTF8_GOT_SUPER|UTF8_GOT_PERL_EXTENDED);
                *errors |= UTF8_GOT_OVERFLOW;

                /* But the API says we flag all errors found */
                if (flags & (UTF8_WARN_SUPER|UTF8_DISALLOW_SUPER)) {
                    *errors |= UTF8_GOT_SUPER;
                }
                if (flags
                        & (UTF8_WARN_PERL_EXTENDED|UTF8_DISALLOW_PERL_EXTENDED))
                {
                    *errors |= UTF8_GOT_PERL_EXTENDED;
                }

                /* Disallow if any of the three categories say to */
                if ( ! (flags &   UTF8_ALLOW_OVERFLOW)
                    || (flags & ( UTF8_DISALLOW_SUPER
                                 |UTF8_DISALLOW_PERL_EXTENDED)))
                {
                    disallowed = TRUE;
                }

                /* Likewise, warn if any say to */
                if (  ! (flags & UTF8_ALLOW_OVERFLOW)
                    ||  (flags & (UTF8_WARN_SUPER|UTF8_WARN_PERL_EXTENDED)))
                {

                    /* The warnings code explicitly says it doesn't handle the
                     * case of packWARN2 and two categories which have
                     * parent-child relationship.  Even if it works now to
                     * raise the warning if either is enabled, it wouldn't
                     * necessarily do so in the future.  We output (only) the
                     * most dire warning */
                    if (! (flags & UTF8_CHECK_ONLY)) {
                        if (msgs || ckWARN_d(WARN_UTF8)) {
                            pack_warn = packWARN(WARN_UTF8);
                        }
                        else if (msgs || ckWARN_d(WARN_NON_UNICODE)) {
                            pack_warn = packWARN(WARN_NON_UNICODE);
                        }
                        if (pack_warn) {
                            message = Perl_form(aTHX_ "%s: %s (overflows)",
                                            malformed_text,
                                            _byte_dump_string(s0, curlen, 0));
                            this_flag_bit = UTF8_GOT_OVERFLOW;
                        }
                    }
                }
            }
            else if (possible_problems & UTF8_GOT_EMPTY) {
                possible_problems &= ~UTF8_GOT_EMPTY;
                *errors |= UTF8_GOT_EMPTY;

                if (! (flags & UTF8_ALLOW_EMPTY)) {

                    /* This so-called malformation is now treated as a bug in
                     * the caller.  If you have nothing to decode, skip calling
                     * this function */
                    assert(0);

                    disallowed = TRUE;
                    if (  (msgs
                        || ckWARN_d(WARN_UTF8)) && ! (flags & UTF8_CHECK_ONLY))
                    {
                        pack_warn = packWARN(WARN_UTF8);
                        message = Perl_form(aTHX_ "%s (empty string)",
                                                   malformed_text);
                        this_flag_bit = UTF8_GOT_EMPTY;
                    }
                }
            }
            else if (possible_problems & UTF8_GOT_CONTINUATION) {
                possible_problems &= ~UTF8_GOT_CONTINUATION;
                *errors |= UTF8_GOT_CONTINUATION;

                if (! (flags & UTF8_ALLOW_CONTINUATION)) {
                    disallowed = TRUE;
                    if ((   msgs
                         || ckWARN_d(WARN_UTF8)) && ! (flags & UTF8_CHECK_ONLY))
                    {
                        pack_warn = packWARN(WARN_UTF8);
                        message = Perl_form(aTHX_
                                "%s: %s (unexpected continuation byte 0x%02x,"
                                " with no preceding start byte)",
                                malformed_text,
                                _byte_dump_string(s0, 1, 0), *s0);
                        this_flag_bit = UTF8_GOT_CONTINUATION;
                    }
                }
            }
            else if (possible_problems & UTF8_GOT_SHORT) {
                possible_problems &= ~UTF8_GOT_SHORT;
                *errors |= UTF8_GOT_SHORT;

                if (! (flags & UTF8_ALLOW_SHORT)) {
                    disallowed = TRUE;
                    if ((   msgs
                         || ckWARN_d(WARN_UTF8)) && ! (flags & UTF8_CHECK_ONLY))
                    {
                        pack_warn = packWARN(WARN_UTF8);
                        message = Perl_form(aTHX_
                             "%s: %s (too short; %d byte%s available, need %d)",
                             malformed_text,
                             _byte_dump_string(s0, send - s0, 0),
                             (int)avail_len,
                             avail_len == 1 ? "" : "s",
                             (int)expectlen);
                        this_flag_bit = UTF8_GOT_SHORT;
                    }
                }

            }
            else if (possible_problems & UTF8_GOT_NON_CONTINUATION) {
                possible_problems &= ~UTF8_GOT_NON_CONTINUATION;
                *errors |= UTF8_GOT_NON_CONTINUATION;

                if (! (flags & UTF8_ALLOW_NON_CONTINUATION)) {
                    disallowed = TRUE;
                    if ((   msgs
                         || ckWARN_d(WARN_UTF8)) && ! (flags & UTF8_CHECK_ONLY))
                    {

                        /* If we don't know for sure that the input length is
                         * valid, avoid as much as possible reading past the
                         * end of the buffer */
                        int printlen = (flags & _UTF8_NO_CONFIDENCE_IN_CURLEN)
                                       ? (int) (s - s0)
                                       : (int) (send - s0);
                        pack_warn = packWARN(WARN_UTF8);
                        message = Perl_form(aTHX_ "%s",
                            unexpected_non_continuation_text(s0,
                                                            printlen,
                                                            s - s0,
                                                            (int) expectlen));
                        this_flag_bit = UTF8_GOT_NON_CONTINUATION;
                    }
                }
            }
            else if (possible_problems & UTF8_GOT_SURROGATE) {
                possible_problems &= ~UTF8_GOT_SURROGATE;

                if (flags & UTF8_WARN_SURROGATE) {
                    *errors |= UTF8_GOT_SURROGATE;

                    if (   ! (flags & UTF8_CHECK_ONLY)
                        && (msgs || ckWARN_d(WARN_SURROGATE)))
                    {
                        pack_warn = packWARN(WARN_SURROGATE);

                        /* These are the only errors that can occur with a
                        * surrogate when the 'uv' isn't valid */
                        if (orig_problems & UTF8_GOT_TOO_SHORT) {
                            message = Perl_form(aTHX_
                                    "UTF-16 surrogate (any UTF-8 sequence that"
                                    " starts with \"%s\" is for a surrogate)",
                                    _byte_dump_string(s0, curlen, 0));
                        }
                        else {
                            message = Perl_form(aTHX_ surrogate_cp_format, uv);
                        }
                        this_flag_bit = UTF8_GOT_SURROGATE;
                    }
                }

                if (flags & UTF8_DISALLOW_SURROGATE) {
                    disallowed = TRUE;
                    *errors |= UTF8_GOT_SURROGATE;
                }
            }
            else if (possible_problems & UTF8_GOT_SUPER) {
                possible_problems &= ~UTF8_GOT_SUPER;

                if (flags & UTF8_WARN_SUPER) {
                    *errors |= UTF8_GOT_SUPER;

                    if (   ! (flags & UTF8_CHECK_ONLY)
                        && (msgs || ckWARN_d(WARN_NON_UNICODE)))
                    {
                        pack_warn = packWARN(WARN_NON_UNICODE);

                        if (orig_problems & UTF8_GOT_TOO_SHORT) {
                            message = Perl_form(aTHX_
                                    "Any UTF-8 sequence that starts with"
                                    " \"%s\" is for a non-Unicode code point,"
                                    " may not be portable",
                                    _byte_dump_string(s0, curlen, 0));
                        }
                        else {
                            message = Perl_form(aTHX_ super_cp_format, uv);
                        }
                        this_flag_bit = UTF8_GOT_SUPER;
                    }
                }

                /* Test for Perl's extended UTF-8 after the regular SUPER ones,
                 * and before possibly bailing out, so that the more dire
                 * warning will override the regular one. */
                if (UNLIKELY(UTF8_IS_PERL_EXTENDED(s0))) {
                    if (  ! (flags & UTF8_CHECK_ONLY)
                        &&  (flags & (UTF8_WARN_PERL_EXTENDED|UTF8_WARN_SUPER))
                        &&  (msgs || (   ckWARN_d(WARN_NON_UNICODE)
                                      || ckWARN(WARN_PORTABLE))))
                    {
                        pack_warn = packWARN2(WARN_NON_UNICODE, WARN_PORTABLE);

                        /* If it is an overlong that evaluates to a code point
                         * that doesn't have to use the Perl extended UTF-8, it
                         * still used it, and so we output a message that
                         * doesn't refer to the code point.  The same is true
                         * if there was a SHORT malformation where the code
                         * point is not valid.  In that case, 'uv' will have
                         * been set to the REPLACEMENT CHAR, and the message
                         * below without the code point in it will be selected
                         * */
                        if (UNICODE_IS_PERL_EXTENDED(uv)) {
                            message = Perl_form(aTHX_
                                            PL_extended_cp_format, uv);
                        }
                        else {
                            message = Perl_form(aTHX_
                                        "Any UTF-8 sequence that starts with"
                                        " \"%s\" is a Perl extension, and"
                                        " so is not portable",
                                        _byte_dump_string(s0, curlen, 0));
                        }
                        this_flag_bit = UTF8_GOT_PERL_EXTENDED;
                    }

                    if (flags & ( UTF8_WARN_PERL_EXTENDED
                                 |UTF8_DISALLOW_PERL_EXTENDED))
                    {
                        *errors |= UTF8_GOT_PERL_EXTENDED;

                        if (flags & UTF8_DISALLOW_PERL_EXTENDED) {
                            disallowed = TRUE;
                        }
                    }
                }

                if (flags & UTF8_DISALLOW_SUPER) {
                    *errors |= UTF8_GOT_SUPER;
                    disallowed = TRUE;
                }
            }
            else if (possible_problems & UTF8_GOT_NONCHAR) {
                possible_problems &= ~UTF8_GOT_NONCHAR;

                if (flags & UTF8_WARN_NONCHAR) {
                    *errors |= UTF8_GOT_NONCHAR;

                    if (  ! (flags & UTF8_CHECK_ONLY)
                        && (msgs || ckWARN_d(WARN_NONCHAR)))
                    {
                        /* The code above should have guaranteed that we don't
                         * get here with errors other than overlong */
                        assert (! (orig_problems
                                        & ~(UTF8_GOT_LONG|UTF8_GOT_NONCHAR)));

                        pack_warn = packWARN(WARN_NONCHAR);
                        message = Perl_form(aTHX_ nonchar_cp_format, uv);
                        this_flag_bit = UTF8_GOT_NONCHAR;
                    }
                }

                if (flags & UTF8_DISALLOW_NONCHAR) {
                    disallowed = TRUE;
                    *errors |= UTF8_GOT_NONCHAR;
                }
            }
            else if (possible_problems & UTF8_GOT_LONG) {
                possible_problems &= ~UTF8_GOT_LONG;
                *errors |= UTF8_GOT_LONG;

                if (flags & UTF8_ALLOW_LONG) {

                    /* We don't allow the actual overlong value, unless the
                     * special extra bit is also set */
                    if (! (flags & (   UTF8_ALLOW_LONG_AND_ITS_VALUE
                                    & ~UTF8_ALLOW_LONG)))
                    {
                        uv = UNICODE_REPLACEMENT;
                    }
                }
                else {
                    disallowed = TRUE;

                    if ((   msgs
                         || ckWARN_d(WARN_UTF8)) && ! (flags & UTF8_CHECK_ONLY))
                    {
                        pack_warn = packWARN(WARN_UTF8);

                        /* These error types cause 'uv' to be something that
                         * isn't what was intended, so can't use it in the
                         * message.  The other error types either can't
                         * generate an overlong, or else the 'uv' is valid */
                        if (orig_problems &
                                        (UTF8_GOT_TOO_SHORT|UTF8_GOT_OVERFLOW))
                        {
                            message = Perl_form(aTHX_
                                    "%s: %s (any UTF-8 sequence that starts"
                                    " with \"%s\" is overlong which can and"
                                    " should be represented with a"
                                    " different, shorter sequence)",
                                    malformed_text,
                                    _byte_dump_string(s0, send - s0, 0),
                                    _byte_dump_string(s0, curlen, 0));
                        }
                        else {
                            U8 tmpbuf[UTF8_MAXBYTES+1];
                            const U8 * const e = uvoffuni_to_utf8_flags(tmpbuf,
                                                                        uv, 0);
                            /* Don't use U+ for non-Unicode code points, which
                             * includes those in the Latin1 range */
                            const char * preface = (   UNICODE_IS_SUPER(uv)
#ifdef EBCDIC
                                                    || uv <= 0xFF
#endif
                                                   )
                                                   ? "0x"
                                                   : "U+";
                            message = Perl_form(aTHX_
                                "%s: %s (overlong; instead use %s to represent"
                                " %s%0*" UVXf ")",
                                malformed_text,
                                _byte_dump_string(s0, send - s0, 0),
                                _byte_dump_string(tmpbuf, e - tmpbuf, 0),
                                preface,
                                ((uv < 256) ? 2 : 4), /* Field width of 2 for
                                                         small code points */
                                UNI_TO_NATIVE(uv));
                        }
                        this_flag_bit = UTF8_GOT_LONG;
                    }
                }
            } /* End of looking through the possible flags */

            /* Display the message (if any) for the problem being handled in
             * this iteration of the loop */
            if (message) {
                if (msgs) {
                    assert(this_flag_bit);

                    if (*msgs == NULL) {
                        *msgs = newAV();
                    }

                    av_push(*msgs, newRV_noinc((SV*) new_msg_hv(message,
                                                                pack_warn,
                                                                this_flag_bit)));
                }
                else if (PL_op)
                    Perl_warner(aTHX_ pack_warn, "%s in %s", message,
                                                 OP_DESC(PL_op));
                else
                    Perl_warner(aTHX_ pack_warn, "%s", message);
            }
        }   /* End of 'while (possible_problems)' */

        /* Since there was a possible problem, the returned length may need to
         * be changed from the one stored at the beginning of this function.
         * Instead of trying to figure out if it has changed, just do it. */
        if (retlen) {
            *retlen = curlen;
        }

        if (disallowed) {
            if (flags & UTF8_CHECK_ONLY && retlen) {
                *retlen = ((STRLEN) -1);
            }
            return 0;
        }
    }

    return UNI_TO_NATIVE(uv);
}

/*
=for apidoc utf8_to_uvchr_buf

Returns the native code point of the first character in the string C<s> which
is assumed to be in UTF-8 encoding; C<send> points to 1 beyond the end of C<s>.
C<*retlen> will be set to the length, in bytes, of that character.

If C<s> does not point to a well-formed UTF-8 character and UTF8 warnings are
enabled, zero is returned and C<*retlen> is set (if C<retlen> isn't
C<NULL>) to -1.  If those warnings are off, the computed value, if well-defined
(or the Unicode REPLACEMENT CHARACTER if not), is silently returned, and
C<*retlen> is set (if C<retlen> isn't C<NULL>) so that (S<C<s> + C<*retlen>>) is
the next possible position in C<s> that could begin a non-malformed character.
See L</utf8n_to_uvchr> for details on when the REPLACEMENT CHARACTER is
returned.

=cut

Also implemented as a macro in utf8.h

*/


UV
Perl_utf8_to_uvchr_buf(pTHX_ const U8 *s, const U8 *send, STRLEN *retlen)
{
    PERL_ARGS_ASSERT_UTF8_TO_UVCHR_BUF;

    return utf8_to_uvchr_buf_helper(s, send, retlen);
}

/*
=for apidoc utf8_length

Returns the number of characters in the sequence of UTF-8-encoded bytes starting
at C<s> and ending at the byte just before C<e>.  If <s> and <e> point to the
same place, it returns 0 with no warning raised.

If C<e E<lt> s> or if the scan would end up past C<e>, it raises a UTF8 warning
and returns the number of valid characters.

=cut

    For long strings we process the input word-at-a-time, and count
    continuations, instead of otherwise counting characters and using UTF8SKIP
    to find the next one.  If our input were 13-byte characters, the per-word
    would be a loser, as we would be doing things in 8 byte chunks (or 4 on a
    32-bit platform).  But the maximum legal Unicode code point is 4 bytes, and
    most text will have a significant number of 1 and 2 byte characters, so the
    per-word is generally a winner.

    There are start-up and finish costs with the per-word method, so we use the
    standard method unless the input has a relatively large length.
*/

STRLEN
Perl_utf8_length(pTHX_ const U8 * const s0, const U8 * const e)
{
    STRLEN continuations = 0;
    STRLEN len = 0;
    const U8 * s = s0;

    PERL_ARGS_ASSERT_UTF8_LENGTH;

    /* For EBCDCIC and short strings, we count the characters.  The boundary
     * was determined by eyeballing the output of Porting/bench.pl and
     * choosing a number where the continuations method gave better results (on
     * a 64 bit system, khw not having access to a 32 bit system with
     * cachegrind).  The number isn't critical, as at these sizes, the total
     * time spent isn't large either way */

#ifndef EBCDIC

    if (e - s0 < 96)

#endif

    {
        while (s < e) { /* Count characters directly */

            /* Take extra care to not exceed 'e' (which would be undefined
             * behavior) should the input be malformed, with a partial
             * character at the end */
            Ptrdiff_t expected_byte_count = UTF8SKIP(s);
            if (UNLIKELY(e - s  < expected_byte_count)) {
                goto warn_and_return;
            }

            len++;
            s += expected_byte_count;
        }

        if (LIKELY(e == s)) {
            return len;
        }

      warn_and_return:
        if (ckWARN_d(WARN_UTF8)) {
            if (PL_op)
                Perl_warner(aTHX_ packWARN(WARN_UTF8),
                             "%s in %s", unees, OP_DESC(PL_op));
            else
                Perl_warner(aTHX_ packWARN(WARN_UTF8), "%s", unees);
        }

        return s - s0;
    }

#ifndef EBCDIC

    /* Count continuations, word-at-a-time.
     *
     * We need to stop before the final start character in order to
     * preserve the limited error checking that's always been done */
    const U8 * e_limit = e - UTF8_MAXBYTES;

    /* Points to the first byte >=s which is positioned at a word boundary.  If
     * s is on a word boundary, it is s, otherwise it is to the next word. */
    const U8 * partial_word_end = s + PERL_WORDSIZE * PERL_IS_SUBWORD_ADDR(s)
                                    - (PTR2nat(s) & PERL_WORD_BOUNDARY_MASK);

    /* Process up to a full word boundary. */
    while (s < partial_word_end) {
        const Size_t skip = UTF8SKIP(s);

        continuations += skip - 1;
        s += skip;
    }

    /* Adjust back down any overshoot */
    continuations -= s - partial_word_end;
    s = partial_word_end;

    do { /* Process per-word */

        /* The idea for counting continuation bytes came from
         * http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html
         * One thing it does that this doesn't is to prefetch the buffer
         *      __builtin_prefetch(&s[256], 0, 0);
         *
         * A continuation byte has the upper 2 bits be '10', and the rest
         * dont-cares.  The VARIANTS mask zeroes out all but the upper bit of
         * each byte in the word.  That gets shifted to the byte's lowest bit,
         * and 'anded' with the complement of the 2nd highest bit of the byte,
         * which has also been shifted to that position.  Hence the bit in that
         * position will be 1 iff the upper bit is 1 and the next one is 0.  We
         * then use the same integer multiplcation and shifting that are used
         * in variant_under_utf8_count() to count how many of those are set in
         * the word. */

        continuations += (((((* (const PERL_UINTMAX_T *) s)
                                            & PERL_VARIANTS_WORD_MASK) >> 7)
                      & (((~ (* (const PERL_UINTMAX_T *) s))) >> 6))
                  * PERL_COUNT_MULTIPLIER)
                >> ((PERL_WORDSIZE - 1) * CHARBITS);
        s += PERL_WORDSIZE;
    } while (s + PERL_WORDSIZE <= e_limit);

    /* Process remainder per-byte */
    while (s < e) {
	if (UTF8_IS_CONTINUATION(*s)) {
            continuations++;
            s++;
            continue;
        }

        /* Here is a starter byte.  Use UTF8SKIP from now on */
        do {
            Ptrdiff_t expected_byte_count = UTF8SKIP(s);
            if (UNLIKELY(e - s  < expected_byte_count)) {
                break;
            }

            continuations += expected_byte_count- 1;
            s += expected_byte_count;
        } while (s < e);

        break;
    }

#  endif

    if (LIKELY(e == s)) {
        return s - s0 - continuations;
    }

    /* Convert to characters */
    s -= continuations;

    goto warn_and_return;
}

/*
=for apidoc bytes_cmp_utf8

Compares the sequence of characters (stored as octets) in C<b>, C<blen> with the
sequence of characters (stored as UTF-8)
in C<u>, C<ulen>.  Returns 0 if they are
equal, -1 or -2 if the first string is less than the second string, +1 or +2
if the first string is greater than the second string.

-1 or +1 is returned if the shorter string was identical to the start of the
longer string.  -2 or +2 is returned if
there was a difference between characters
within the strings.

=cut
*/

int
Perl_bytes_cmp_utf8(pTHX_ const U8 *b, STRLEN blen, const U8 *u, STRLEN ulen)
{
    const U8 *const bend = b + blen;
    const U8 *const uend = u + ulen;

    PERL_ARGS_ASSERT_BYTES_CMP_UTF8;

    while (b < bend && u < uend) {
        U8 c = *u++;
        if (!UTF8_IS_INVARIANT(c)) {
            if (UTF8_IS_DOWNGRADEABLE_START(c)) {
                if (u < uend) {
                    U8 c1 = *u++;
                    if (UTF8_IS_CONTINUATION(c1)) {
                        c = EIGHT_BIT_UTF8_TO_NATIVE(c, c1);
                    } else {
                        /* diag_listed_as: Malformed UTF-8 character%s */
                        Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8),
                              "%s %s%s",
                              unexpected_non_continuation_text(u - 2, 2, 1, 2),
                              PL_op ? " in " : "",
                              PL_op ? OP_DESC(PL_op) : "");
                        return -2;
                    }
                } else {
                    if (PL_op)
                        Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8),
                                         "%s in %s", unees, OP_DESC(PL_op));
                    else
                        Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8), "%s", unees);
                    return -2; /* Really want to return undef :-)  */
                }
            } else {
                return -2;
            }
        }
        if (*b != c) {
            return *b < c ? -2 : +2;
        }
        ++b;
    }

    if (b == bend && u == uend)
        return 0;

    return b < bend ? +1 : -1;
}

/*
=for apidoc utf8_to_bytes

Converts a string C<"s"> of length C<*lenp> from UTF-8 into native byte encoding.
Unlike L</bytes_to_utf8>, this over-writes the original string, and
updates C<*lenp> to contain the new length.
Returns zero on failure (leaving C<"s"> unchanged) setting C<*lenp> to -1.

Upon successful return, the number of variants in the string can be computed by
having saved the value of C<*lenp> before the call, and subtracting the
after-call value of C<*lenp> from it.

If you need a copy of the string, see L</bytes_from_utf8>.

=cut
*/

U8 *
Perl_utf8_to_bytes(pTHX_ U8 *s, STRLEN *lenp)
{
    U8 * first_variant;

    PERL_ARGS_ASSERT_UTF8_TO_BYTES;
    PERL_UNUSED_CONTEXT;

    /* This is a no-op if no variants at all in the input */
    if (is_utf8_invariant_string_loc(s, *lenp, (const U8 **) &first_variant)) {
        return s;
    }

    /* Nothing before 'first_variant' needs to be changed, so start the real
     * work there */

    U8 * const save = s;
    U8 * const send = s + *lenp;
    U8 * d;

#ifndef EBCDIC      /* The below relies on the bit patterns of UTF-8 */

    /* There is some start-up/tear-down overhead with this, so no real gain
     * unless the string is long enough.  The current value is just a
     * guess. */
    if (*lenp > 5 * PERL_WORDSIZE) {

        /* First, go through the string a word at-a-time to verify that it is
         * downgradable.  If it contains any start byte besides C2 and C3, then
         * it isn't. */

        const PERL_UINTMAX_T C0_mask = PERL_COUNT_MULTIPLIER * 0xC0;
        const PERL_UINTMAX_T C2_mask = PERL_COUNT_MULTIPLIER * 0xC2;
        const PERL_UINTMAX_T FE_mask = PERL_COUNT_MULTIPLIER * 0xFE;

        /* Points to the first byte >=s which is positioned at a word boundary.
         * If s is on a word boundary, it is s, otherwise it is the first byte
         * of the next word. */
        U8 * partial_word_end = s + PERL_WORDSIZE * PERL_IS_SUBWORD_ADDR(s)
                                - (PTR2nat(s) & PERL_WORD_BOUNDARY_MASK);

        /* Here there is at least a full word beyond the first word boundary.
         * Process up to that boundary. */
        while (s < partial_word_end) {
            if (! UTF8_IS_INVARIANT(*s)) {
                if (! UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(s, send)) {
                    *lenp = ((STRLEN) -1);
                    return NULL;
                }
                s++;
            }
            s++;
        }

        /* Adjust back down any overshoot */
        s = partial_word_end;

        /* Process per-word */
        do {

            PERL_UINTMAX_T C2_C3_start_bytes;

            /* First find the bytes that are start bytes.  ANDing with
             * C0C0...C0 causes any start byte to become C0; any other byte
             * becomes something else.  Then XORing with C0 causes any start
             * byte to become 0; all other bytes non-zero. */
            PERL_UINTMAX_T start_bytes
                          = ((* (PERL_UINTMAX_T *) s) & C0_mask) ^ C0_mask;

            /* These shifts causes the most significant bit to be set to 1 for
             * any bytes in the word that aren't completely 0.  Hence after
             * these, only the start bytes have 0 in their msb */
            start_bytes |= start_bytes << 1;
            start_bytes |= start_bytes << 2;
            start_bytes |= start_bytes << 4;

            /* When we complement, then AND with 8080...80, the start bytes
             * will have 1 in their msb, and all other bits are 0 */
            start_bytes = ~ start_bytes & PERL_VARIANTS_WORD_MASK;

            /* Now repeat the procedure, but look for bytes that match only
             * C2-C3. */
            C2_C3_start_bytes = ((* (PERL_UINTMAX_T *) s) & FE_mask)
                                                                ^ C2_mask;
            C2_C3_start_bytes |= C2_C3_start_bytes << 1;
            C2_C3_start_bytes |= C2_C3_start_bytes << 2;
            C2_C3_start_bytes |= C2_C3_start_bytes << 4;
            C2_C3_start_bytes = ~ C2_C3_start_bytes
                                & PERL_VARIANTS_WORD_MASK;

            /* Here, start_bytes has a 1 in the msb of each byte that has a
             *                                              start_byte; And
             * C2_C3_start_bytes has a 1 in the msb of each byte that has a
             *                                       start_byte of C2 or C3
             * If they're not equal, there are start bytes that aren't C2
             * nor C3, hence this is not downgradable */
            if (start_bytes != C2_C3_start_bytes) {
                *lenp = ((STRLEN) -1);
                return NULL;
            }

            s += PERL_WORDSIZE;
        } while (s + PERL_WORDSIZE <= send);

        /* If the final byte was a start byte, it means that the character
         * straddles two words, so back off one to start looking below at the
         * first byte of the character  */
        if (s > first_variant && UTF8_IS_START(*(s-1))) {
            s--;
        }
    }

#endif

    /* Do the straggler bytes beyond the final word boundary (or all bytes
     * in the case of EBCDIC) */
    while (s < send) {
        if (! UTF8_IS_INVARIANT(*s)) {
            if (! UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(s, send)) {
                *lenp = ((STRLEN) -1);
                return NULL;
            }
            s++;
        }
        s++;
    }

    /* Here, we passed the tests above.  For the EBCDIC case, everything
     * was well-formed and can be downgraded to non-UTF8.  For non-EBCDIC,
     * it means only that all start bytes were C2 or C3, hence any
     * well-formed sequences are downgradable.  But we didn't test, for
     * example, that there weren't two C2's in a row.  That means that in
     * the loop below, we have to be sure things are well-formed.  Because
     * this is very very likely, and we don't care about having speedy
     * handling of malformed input, the loop proceeds as if well formed,
     * and should a malformed one come along, it undoes what it already has
     * done */

    d = s = first_variant;

    while (s < send) {
        U8 * s1;

        if (UVCHR_IS_INVARIANT(*s)) {
            *d++ = *s++;
            continue;
        }

        /* Here it is two-byte encoded. */
        if (   LIKELY(UTF8_IS_DOWNGRADEABLE_START(*s))
            && LIKELY(UTF8_IS_CONTINUATION((s[1]))))
        {
            U8 first_byte = *s++;
            *d++ = EIGHT_BIT_UTF8_TO_NATIVE(first_byte, *s);
            s++;
            continue;
        }

        /* Here, it is malformed.  This shouldn't happen on EBCDIC, and on
         * ASCII platforms, we know that the only start bytes in the text
         * are C2 and C3, and the code above has made sure that it doesn't
         * end with a start byte.  That means the only malformations that
         * are possible are a start byte without a continuation (either
         * followed by another start byte or an invariant) or an unexpected
         * continuation.
         *
         * We have to undo all we've done before, back down to the first
         * UTF-8 variant.  Note that each 2-byte variant we've done so far
         * (converted to single byte) slides things to the left one byte,
         * and so we have bytes that haven't been written over.
         *
         * Here, 'd' points to the next position to overwrite, and 's'
         * points to the first invalid byte.  That means 'd's contents
         * haven't been changed yet, nor has anything else beyond it in the
         * string.  In restoring to the original contents, we don't need to
         * do anything past (d-1).
         *
         * In particular, the bytes from 'd' to 's' have not been changed.
         * This loop uses a new variable 's1' (to avoid confusing 'source'
         * and 'destination') set to 'd',  and moves 's' and 's1' in lock
         * step back so that afterwards, 's1' points to the first changed
         * byte that will be the source for the first byte (or bytes) at
         * 's' that need to be changed back.  Note that s1 can expand to
         * two bytes */
        s1 = d;
        while (s >= d) {
            s--;
            if (! UVCHR_IS_INVARIANT(*s1)) {
                s--;
            }
            s1--;
        }

        /* Do the changing back */
        while (s1 >= first_variant) {
            if (UVCHR_IS_INVARIANT(*s1)) {
                *s-- = *s1--;
            }
            else {
                *s-- = UTF8_EIGHT_BIT_LO(*s1);
                *s-- = UTF8_EIGHT_BIT_HI(*s1);
                s1--;
            }
        }

        *lenp = ((STRLEN) -1);
        return NULL;
    }

    /* Success! */
    *d = '\0';
    *lenp = d - save;

    return save;
}

/*
=for apidoc bytes_from_utf8

Converts a potentially UTF-8 encoded string C<s> of length C<*lenp> into native
byte encoding.  On input, the boolean C<*is_utf8p> gives whether or not C<s> is
actually encoded in UTF-8.

Unlike L</utf8_to_bytes> but like L</bytes_to_utf8>, this is non-destructive of
the input string.

Do nothing if C<*is_utf8p> is 0, or if there are code points in the string
not expressible in native byte encoding.  In these cases, C<*is_utf8p> and
C<*lenp> are unchanged, and the return value is the original C<s>.

Otherwise, C<*is_utf8p> is set to 0, and the return value is a pointer to a
newly created string containing a downgraded copy of C<s>, and whose length is
returned in C<*lenp>, updated.  The new string is C<NUL>-terminated.  The
caller is responsible for arranging for the memory used by this string to get
freed.

Upon successful return, the number of variants in the string can be computed by
having saved the value of C<*lenp> before the call, and subtracting the
after-call value of C<*lenp> from it.

=cut

There is a macro that avoids this function call, but this is retained for
anyone who calls it with the Perl_ prefix */

U8 *
Perl_bytes_from_utf8(pTHX_ const U8 *s, STRLEN *lenp, bool *is_utf8p)
{
    PERL_ARGS_ASSERT_BYTES_FROM_UTF8;
    PERL_UNUSED_CONTEXT;

    return bytes_from_utf8_loc(s, lenp, is_utf8p, NULL);
}

/*
=for apidoc bytes_from_utf8_loc

Like C<L<perlapi/bytes_from_utf8>()>, but takes an extra parameter, a pointer
to where to store the location of the first character in C<"s"> that cannot be
converted to non-UTF8.

If that parameter is C<NULL>, this function behaves identically to
C<bytes_from_utf8>.

Otherwise if C<*is_utf8p> is 0 on input, the function behaves identically to
C<bytes_from_utf8>, except it also sets C<*first_non_downgradable> to C<NULL>.

Otherwise, the function returns a newly created C<NUL>-terminated string
containing the non-UTF8 equivalent of the convertible first portion of
C<"s">.  C<*lenp> is set to its length, not including the terminating C<NUL>.
If the entire input string was converted, C<*is_utf8p> is set to a FALSE value,
and C<*first_non_downgradable> is set to C<NULL>.

Otherwise, C<*first_non_downgradable> is set to point to the first byte of the
first character in the original string that wasn't converted.  C<*is_utf8p> is
unchanged.  Note that the new string may have length 0.

Another way to look at it is, if C<*first_non_downgradable> is non-C<NULL> and
C<*is_utf8p> is TRUE, this function starts at the beginning of C<"s"> and
converts as many characters in it as possible stopping at the first one it
finds that can't be converted to non-UTF-8.  C<*first_non_downgradable> is
set to point to that.  The function returns the portion that could be converted
in a newly created C<NUL>-terminated string, and C<*lenp> is set to its length,
not including the terminating C<NUL>.  If the very first character in the
original could not be converted, C<*lenp> will be 0, and the new string will
contain just a single C<NUL>.  If the entire input string was converted,
C<*is_utf8p> is set to FALSE and C<*first_non_downgradable> is set to C<NULL>.

Upon successful return, the number of variants in the converted portion of the
string can be computed by having saved the value of C<*lenp> before the call,
and subtracting the after-call value of C<*lenp> from it.

=cut


*/

U8 *
Perl_bytes_from_utf8_loc(const U8 *s, STRLEN *lenp, bool *is_utf8p, const U8** first_unconverted)
{
    U8 *d;
    const U8 *original = s;
    U8 *converted_start;
    const U8 *send = s + *lenp;

    PERL_ARGS_ASSERT_BYTES_FROM_UTF8_LOC;

    if (! *is_utf8p) {
        if (first_unconverted) {
            *first_unconverted = NULL;
        }

        return (U8 *) original;
    }

    Newx(d, (*lenp) + 1, U8);

    converted_start = d;
    while (s < send) {
        U8 c = *s++;
        if (! UTF8_IS_INVARIANT(c)) {

            /* Then it is multi-byte encoded.  If the code point is above 0xFF,
             * have to stop now */
            if (UNLIKELY (! UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(s - 1, send))) {
                if (first_unconverted) {
                    *first_unconverted = s - 1;
                    goto finish_and_return;
                }
                else {
                    Safefree(converted_start);
                    return (U8 *) original;
                }
            }

            c = EIGHT_BIT_UTF8_TO_NATIVE(c, *s);
            s++;
        }
        *d++ = c;
    }

    /* Here, converted the whole of the input */
    *is_utf8p = FALSE;
    if (first_unconverted) {
        *first_unconverted = NULL;
    }

  finish_and_return:
    *d = '\0';
    *lenp = d - converted_start;

    /* Trim unused space */
    Renew(converted_start, *lenp + 1, U8);

    return converted_start;
}

/*
=for apidoc bytes_to_utf8

Converts a string C<s> of length C<*lenp> bytes from the native encoding into
UTF-8.
Returns a pointer to the newly-created string, and sets C<*lenp> to
reflect the new length in bytes.  The caller is responsible for arranging for
the memory used by this string to get freed.

Upon successful return, the number of variants in the string can be computed by
having saved the value of C<*lenp> before the call, and subtracting it from the
after-call value of C<*lenp>.

A C<NUL> character will be written after the end of the string.

If you want to convert to UTF-8 from encodings other than
the native (Latin1 or EBCDIC),
see L</sv_recode_to_utf8>().

=cut
*/

U8*
Perl_bytes_to_utf8(pTHX_ const U8 *s, STRLEN *lenp)
{
    const U8 * const send = s + (*lenp);
    U8 *d;
    U8 *dst;

    PERL_ARGS_ASSERT_BYTES_TO_UTF8;
    PERL_UNUSED_CONTEXT;

    /* 1 for each byte + 1 for each byte that expands to two, + trailing NUL */
    Newx(d, (*lenp) + variant_under_utf8_count(s, send) + 1, U8);
    dst = d;

    while (s < send) {
        append_utf8_from_native_byte(*s, &d);
        s++;
    }

    *d = '\0';
    *lenp = d-dst;

    return dst;
}

/*
 * Convert native UTF-16 to UTF-8. Called via the more public functions
 * utf16_to_utf8() for big-endian and utf16_to_utf8_reversed() for
 * little-endian,
 *
 * 'p' is the UTF-16 input string, passed as a pointer to U8.
 * 'bytelen' is its length (must be even)
 * 'd' is the pointer to the destination buffer.  The caller must ensure that
 *     the space is large enough.  The maximum expansion factor is 2 times
 *     'bytelen'.  1.5 if never going to run on an EBCDIC box.
 * '*newlen' will contain the number of bytes this function filled of 'd'.
 * 'high_byte' is 0 if UTF-16BE; 1 if UTF-16LE
 * 'low_byte' is 1  if UTF-16BE; 0 if UTF-16LE
 *
 * The expansion factor is because UTF-16 requires 2 bytes for every code point
 * below 0x10000; otherwise 4 bytes.  UTF-8 requires 1-3 bytes for every code
 * point below 0x1000; otherwise 4 bytes.  UTF-EBCDIC requires 1-4 bytes for
 * every code point below 0x1000; otherwise 4-5 bytes.
 *
 * The worst case is where every code point is below U+10000, hence requiring 2
 * UTF-16 bytes, but is U+0800 or higher on ASCII platforms, requiring 3 UTF-8
 * bytes; or >= U+4000 on EBCDIC requiring 4 UTF-8 bytes.
 *
 * Do not use in-place. */

U8*
Perl_utf16_to_utf8_base(pTHX_ U8* p, U8* d, Size_t bytelen, Size_t *newlen,
                              const bool high_byte, /* Which of next two bytes is
                                                  high order */
                              const bool low_byte)
{
    U8* pend;
    U8* dstart = d;

    PERL_ARGS_ASSERT_UTF16_TO_UTF8_BASE;

    if (bytelen & 1)
        Perl_croak(aTHX_ "panic: utf16_to_utf8%s: odd bytelen %" UVuf,
                ((high_byte == 0) ? "" : "_reversed"), (UV)bytelen);
    pend = p + bytelen;

    while (p < pend) {

        /* Next 16 bits is what we want.  (The bool is cast to U8 because on
         * platforms where a bool is implemented as a signed char, a compiler
         * warning may be generated) */
        U32 uv = (p[(U8) high_byte] << 8) + p[(U8) low_byte];
        p += 2;

        /* If it's a surrogate, we find the uv that the surrogate pair encodes.
         * */
        if (UNLIKELY(UNICODE_IS_SURROGATE(uv))) {

#define FIRST_HIGH_SURROGATE UNICODE_SURROGATE_FIRST
#define LAST_HIGH_SURROGATE  0xDBFF
#define FIRST_LOW_SURROGATE  0xDC00
#define LAST_LOW_SURROGATE   UNICODE_SURROGATE_LAST
#define FIRST_IN_PLANE1      0x10000

            if (UNLIKELY(p >= pend) || UNLIKELY(uv > LAST_HIGH_SURROGATE)) {
                Perl_croak(aTHX_ "Malformed UTF-16 surrogate");
            }
            else {
                U32 low_surrogate = (p[(U8) high_byte] << 8) + p[(U8) low_byte];
                if (UNLIKELY(! inRANGE(low_surrogate, FIRST_LOW_SURROGATE,
                                                       LAST_LOW_SURROGATE)))
                {
                    Perl_croak(aTHX_ "Malformed UTF-16 surrogate");
                }

                p += 2;

                /* Here uv is the high surrogate.  Combine with low surrogate
                 * just computed to form the actual U32 code point.
                 *
                 * From https://unicode.org/faq/utf_bom.html#utf16-4 */
                uv = FIRST_IN_PLANE1 + (uv << 10) - (FIRST_HIGH_SURROGATE << 10)
                                     + low_surrogate - FIRST_LOW_SURROGATE;
            }
        }

        /* Here, 'uv' is the real U32 we want to find the UTF-8 of */
        d = uvchr_to_utf8(d, uv);
    }

    *newlen = d - dstart;
    return d;
}

U8*
Perl_utf16_to_utf8(pTHX_ U8* p, U8* d, Size_t bytelen, Size_t *newlen)
{
    PERL_ARGS_ASSERT_UTF16_TO_UTF8;

    return utf16_to_utf8(p, d, bytelen, newlen);
}

U8*
Perl_utf16_to_utf8_reversed(pTHX_ U8* p, U8* d, Size_t bytelen, Size_t *newlen)
{
    PERL_ARGS_ASSERT_UTF16_TO_UTF8_REVERSED;

    return utf16_to_utf8_reversed(p, d, bytelen, newlen);
}

/*
 * Convert UTF-8 to native UTF-16. Called via the macros utf8_to_utf16() for
 * big-endian and utf8_to_utf16_reversed() for little-endian,
 *
 * 's' is the UTF-8 input string, passed as a pointer to U8.
 * 'bytelen' is its length
 * 'd' is the pointer to the destination buffer, currently passed as U8 *.  The
 *     caller must ensure that the space is large enough.  The maximum
 *     expansion factor is 2 times 'bytelen'.  This happens when the input is
 *     entirely single-byte ASCII, expanding to two-byte UTF-16.
 * '*newlen' will contain the number of bytes this function filled of 'd'.
 * 'high_byte' is 0 if UTF-16BE; 1 if UTF-16LE
 * 'low_byte'  is 1 if UTF-16BE; 0 if UTF-16LE
 *
 * Do not use in-place. */
U8*
Perl_utf8_to_utf16_base(pTHX_ U8* s, U8* d, Size_t bytelen, Size_t *newlen,
                              const bool high_byte, /* Which of next two bytes
                                                       is high order */
                              const bool low_byte)
{
    U8* send;
    U8* dstart = d;

    PERL_ARGS_ASSERT_UTF8_TO_UTF16_BASE;

    send = s + bytelen;

    while (s < send) {
        STRLEN retlen;
        UV uv = utf8n_to_uvchr(s, send - s, &retlen,
                               /* No surrogates nor above-Unicode */
                               UTF8_DISALLOW_ILLEGAL_C9_INTERCHANGE);

        /* The modern method is to keep going with malformed input,
         * substituting the REPLACEMENT CHARACTER */
        if (UNLIKELY(uv == 0 && *s != '\0')) {
            uv = UNICODE_REPLACEMENT;
        }

        if (uv >= FIRST_IN_PLANE1) {    /* Requires a surrogate pair */

            /* From https://unicode.org/faq/utf_bom.html#utf16-4 */
            U32 high_surrogate = (uv >> 10) - (FIRST_IN_PLANE1 >> 10)
                               + FIRST_HIGH_SURROGATE;

            /* (The bool is cast to U8 because on platforms where a bool is
             * implemented as a signed char, a compiler warning may be
             * generated) */
            d[(U8) high_byte] = high_surrogate >> 8;
            d[(U8) low_byte]  = high_surrogate & nBIT_MASK(8);
            d += 2;

            /* The low surrogate is the lower 10 bits plus the offset */
            uv &= nBIT_MASK(10);
            uv += FIRST_LOW_SURROGATE;

            /* Drop down to output the low surrogate like it were a
             * non-surrogate */
        }

        d[(U8) high_byte] = uv >> 8;
        d[(U8) low_byte] = uv & nBIT_MASK(8);
        d += 2;

        s += retlen;
    }

    *newlen = d - dstart;
    return d;
}

bool
Perl__is_uni_FOO(pTHX_ const U8 classnum, const UV c)
{
    return _invlist_contains_cp(PL_XPosix_ptrs[classnum], c);
}

bool
Perl__is_uni_perl_idcont(pTHX_ UV c)
{
    return _invlist_contains_cp(PL_utf8_perl_idcont, c);
}

bool
Perl__is_uni_perl_idstart(pTHX_ UV c)
{
    return _invlist_contains_cp(PL_utf8_perl_idstart, c);
}

UV
Perl__to_upper_title_latin1(pTHX_ const U8 c, U8* p, STRLEN *lenp,
                                  const char S_or_s)
{
    /* We have the latin1-range values compiled into the core, so just use
     * those, converting the result to UTF-8.  The only difference between upper
     * and title case in this range is that LATIN_SMALL_LETTER_SHARP_S is
     * either "SS" or "Ss".  Which one to use is passed into the routine in
     * 'S_or_s' to avoid a test */

    UV converted = toUPPER_LATIN1_MOD(c);

    PERL_ARGS_ASSERT__TO_UPPER_TITLE_LATIN1;

    assert(S_or_s == 'S' || S_or_s == 's');

    if (UVCHR_IS_INVARIANT(converted)) { /* No difference between the two for
                                             characters in this range */
        *p = (U8) converted;
        *lenp = 1;
        return converted;
    }

    /* toUPPER_LATIN1_MOD gives the correct results except for three outliers,
     * which it maps to one of them, so as to only have to have one check for
     * it in the main case */
    if (UNLIKELY(converted == LATIN_SMALL_LETTER_Y_WITH_DIAERESIS)) {
        switch (c) {
            case LATIN_SMALL_LETTER_Y_WITH_DIAERESIS:
                converted = LATIN_CAPITAL_LETTER_Y_WITH_DIAERESIS;
                break;
            case MICRO_SIGN:
                converted = GREEK_CAPITAL_LETTER_MU;
                break;
#if    UNICODE_MAJOR_VERSION > 2                                        \
   || (UNICODE_MAJOR_VERSION == 2 && UNICODE_DOT_VERSION >= 1           \
                                  && UNICODE_DOT_DOT_VERSION >= 8)
            case LATIN_SMALL_LETTER_SHARP_S:
                *(p)++ = 'S';
                *p = S_or_s;
                *lenp = 2;
                return 'S';
#endif
            default:
                Perl_croak(aTHX_ "panic: to_upper_title_latin1 did not expect"
                                 " '%c' to map to '%c'",
                                 c, LATIN_SMALL_LETTER_Y_WITH_DIAERESIS);
                NOT_REACHED; /* NOTREACHED */
        }
    }

    *(p)++ = UTF8_TWO_BYTE_HI(converted);
    *p = UTF8_TWO_BYTE_LO(converted);
    *lenp = 2;

    return converted;
}

/* If compiled on an early Unicode version, there may not be auxiliary tables
 * */
#ifndef HAS_UC_AUX_TABLES
#  define UC_AUX_TABLE_ptrs     NULL
#  define UC_AUX_TABLE_lengths  NULL
#endif
#ifndef HAS_TC_AUX_TABLES
#  define TC_AUX_TABLE_ptrs     NULL
#  define TC_AUX_TABLE_lengths  NULL
#endif
#ifndef HAS_LC_AUX_TABLES
#  define LC_AUX_TABLE_ptrs     NULL
#  define LC_AUX_TABLE_lengths  NULL
#endif
#ifndef HAS_CF_AUX_TABLES
#  define CF_AUX_TABLE_ptrs     NULL
#  define CF_AUX_TABLE_lengths  NULL
#endif

/* Call the function to convert a UTF-8 encoded character to the specified case.
 * Note that there may be more than one character in the result.
 * 's' is a pointer to the first byte of the input character
 * 'd' will be set to the first byte of the string of changed characters.  It
 *	needs to have space for UTF8_MAXBYTES_CASE+1 bytes
 * 'lenp' will be set to the length in bytes of the string of changed characters
 *
 * The functions return the ordinal of the first character in the string of
 * 'd' */
#define CALL_UPPER_CASE(uv, s, d, lenp)                                     \
                _to_utf8_case(uv, s, d, lenp, PL_utf8_toupper,              \
                                              Uppercase_Mapping_invmap,     \
                                              UC_AUX_TABLE_ptrs,            \
                                              UC_AUX_TABLE_lengths,         \
                                              "uppercase")
#define CALL_TITLE_CASE(uv, s, d, lenp)                                     \
                _to_utf8_case(uv, s, d, lenp, PL_utf8_totitle,              \
                                              Titlecase_Mapping_invmap,     \
                                              TC_AUX_TABLE_ptrs,            \
                                              TC_AUX_TABLE_lengths,         \
                                              "titlecase")
#define CALL_LOWER_CASE(uv, s, d, lenp)                                     \
                _to_utf8_case(uv, s, d, lenp, PL_utf8_tolower,              \
                                              Lowercase_Mapping_invmap,     \
                                              LC_AUX_TABLE_ptrs,            \
                                              LC_AUX_TABLE_lengths,         \
                                              "lowercase")


/* This additionally has the input parameter 'specials', which if non-zero will
 * cause this to use the specials hash for folding (meaning get full case
 * folding); otherwise, when zero, this implies a simple case fold */
#define CALL_FOLD_CASE(uv, s, d, lenp, specials)                            \
        (specials)                                                          \
        ?  _to_utf8_case(uv, s, d, lenp, PL_utf8_tofold,                    \
                                          Case_Folding_invmap,              \
                                          CF_AUX_TABLE_ptrs,                \
                                          CF_AUX_TABLE_lengths,             \
                                          "foldcase")                       \
        : _to_utf8_case(uv, s, d, lenp, PL_utf8_tosimplefold,               \
                                         Simple_Case_Folding_invmap,        \
                                         NULL, NULL,                        \
                                         "foldcase")

UV
Perl_to_uni_upper(pTHX_ UV c, U8* p, STRLEN *lenp)
{
    /* Convert the Unicode character whose ordinal is <c> to its uppercase
     * version and store that in UTF-8 in <p> and its length in bytes in <lenp>.
     * Note that the <p> needs to be at least UTF8_MAXBYTES_CASE+1 bytes since
     * the changed version may be longer than the original character.
     *
     * The ordinal of the first character of the changed version is returned
     * (but note, as explained above, that there may be more.) */

    PERL_ARGS_ASSERT_TO_UNI_UPPER;

    if (c < 256) {
        return _to_upper_title_latin1((U8) c, p, lenp, 'S');
    }

    return CALL_UPPER_CASE(c, NULL, p, lenp);
}

UV
Perl_to_uni_title(pTHX_ UV c, U8* p, STRLEN *lenp)
{
    PERL_ARGS_ASSERT_TO_UNI_TITLE;

    if (c < 256) {
        return _to_upper_title_latin1((U8) c, p, lenp, 's');
    }

    return CALL_TITLE_CASE(c, NULL, p, lenp);
}

STATIC U8
S_to_lower_latin1(const U8 c, U8* p, STRLEN *lenp, const char dummy)
{
    /* We have the latin1-range values compiled into the core, so just use
     * those, converting the result to UTF-8.  Since the result is always just
     * one character, we allow <p> to be NULL */

    U8 converted = toLOWER_LATIN1(c);

    PERL_UNUSED_ARG(dummy);

    if (p != NULL) {
        if (NATIVE_BYTE_IS_INVARIANT(converted)) {
            *p = converted;
            *lenp = 1;
        }
        else {
            /* Result is known to always be < 256, so can use the EIGHT_BIT
             * macros */
            *p = UTF8_EIGHT_BIT_HI(converted);
            *(p+1) = UTF8_EIGHT_BIT_LO(converted);
            *lenp = 2;
        }
    }
    return converted;
}

UV
Perl_to_uni_lower(pTHX_ UV c, U8* p, STRLEN *lenp)
{
    PERL_ARGS_ASSERT_TO_UNI_LOWER;

    if (c < 256) {
        return to_lower_latin1((U8) c, p, lenp, 0 /* 0 is a dummy arg */ );
    }

    return CALL_LOWER_CASE(c, NULL, p, lenp);
}

UV
Perl__to_fold_latin1(const U8 c, U8* p, STRLEN *lenp, const unsigned int flags)
{
    /* Corresponds to to_lower_latin1(); <flags> bits meanings:
     *	    FOLD_FLAGS_NOMIX_ASCII iff non-ASCII to ASCII folds are prohibited
     *	    FOLD_FLAGS_FULL  iff full folding is to be used;
     *
     *	Not to be used for locale folds
     */

    UV converted;

    PERL_ARGS_ASSERT__TO_FOLD_LATIN1;

    assert (! (flags & FOLD_FLAGS_LOCALE));

    if (UNLIKELY(c == MICRO_SIGN)) {
        converted = GREEK_SMALL_LETTER_MU;
    }
#if    UNICODE_MAJOR_VERSION > 3 /* no multifolds in early Unicode */   \
   || (UNICODE_MAJOR_VERSION == 3 && (   UNICODE_DOT_VERSION > 0)       \
                                      || UNICODE_DOT_DOT_VERSION > 0)
    else if (   (flags & FOLD_FLAGS_FULL)
             && UNLIKELY(c == LATIN_SMALL_LETTER_SHARP_S))
    {
        /* If can't cross 127/128 boundary, can't return "ss"; instead return
         * two U+017F characters, as fc("\df") should eq fc("\x{17f}\x{17f}")
         * under those circumstances. */
        if (flags & FOLD_FLAGS_NOMIX_ASCII) {
            *lenp = 2 * STRLENs(LATIN_SMALL_LETTER_LONG_S_UTF8);
            Copy(LATIN_SMALL_LETTER_LONG_S_UTF8 LATIN_SMALL_LETTER_LONG_S_UTF8,
                 p, *lenp, U8);
            return LATIN_SMALL_LETTER_LONG_S;
        }
        else {
            *(p)++ = 's';
            *p = 's';
            *lenp = 2;
            return 's';
        }
    }
#endif
    else { /* In this range the fold of all other characters is their lower
              case */
        converted = toLOWER_LATIN1(c);
    }

    if (UVCHR_IS_INVARIANT(converted)) {
        *p = (U8) converted;
        *lenp = 1;
    }
    else {
        *(p)++ = UTF8_TWO_BYTE_HI(converted);
        *p = UTF8_TWO_BYTE_LO(converted);
        *lenp = 2;
    }

    return converted;
}

UV
Perl__to_uni_fold_flags(pTHX_ UV c, U8* p, STRLEN *lenp, U8 flags)
{

    /* Not currently externally documented, and subject to change
     *  <flags> bits meanings:
     *	    FOLD_FLAGS_FULL  iff full folding is to be used;
     *	    FOLD_FLAGS_LOCALE is set iff the rules from the current underlying
     *	                      locale are to be used.
     *	    FOLD_FLAGS_NOMIX_ASCII iff non-ASCII to ASCII folds are prohibited
     */

    PERL_ARGS_ASSERT__TO_UNI_FOLD_FLAGS;

    if (flags & FOLD_FLAGS_LOCALE) {
        /* Treat a non-Turkic UTF-8 locale as not being in locale at all,
         * except for potentially warning */
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        if (IN_UTF8_CTYPE_LOCALE && ! IN_UTF8_TURKIC_LOCALE) {
            flags &= ~FOLD_FLAGS_LOCALE;
        }
        else {
            goto needs_full_generality;
        }
    }

    if (c < 256) {
        return _to_fold_latin1((U8) c, p, lenp,
                            flags & (FOLD_FLAGS_FULL | FOLD_FLAGS_NOMIX_ASCII));
    }

    /* Here, above 255.  If no special needs, just use the macro */
    if ( ! (flags & (FOLD_FLAGS_LOCALE|FOLD_FLAGS_NOMIX_ASCII))) {
        return CALL_FOLD_CASE(c, NULL, p, lenp, flags & FOLD_FLAGS_FULL);
    }
    else {  /* Otherwise, _toFOLD_utf8_flags has the intelligence to deal with
               the special flags. */
        U8 utf8_c[UTF8_MAXBYTES + 1];

      needs_full_generality:
        uvchr_to_utf8(utf8_c, c);
        return _toFOLD_utf8_flags(utf8_c, utf8_c + C_ARRAY_LENGTH(utf8_c),
                                  p, lenp, flags);
    }
}

PERL_STATIC_INLINE bool
S_is_utf8_common(pTHX_ const U8 *const p, const U8 * const e,
                       SV* const invlist)
{
    /* returns a boolean giving whether or not the UTF8-encoded character that
     * starts at <p>, and extending no further than <e - 1> is in the inversion
     * list <invlist>. */

    UV cp = utf8n_to_uvchr(p, e - p, NULL, 0);

    PERL_ARGS_ASSERT_IS_UTF8_COMMON;

    if (cp == 0 && (p >= e || *p != '\0')) {
        _force_out_malformed_utf8_message(p, e, 0, 1);
        NOT_REACHED; /* NOTREACHED */
    }

    assert(invlist);
    return _invlist_contains_cp(invlist, cp);
}

#if 0	/* Not currently used, but may be needed in the future */
PERLVAR(I, seen_deprecated_macro, HV *)

STATIC void
S_warn_on_first_deprecated_use(pTHX_ U32 category,
                                     const char * const name,
                                     const char * const alternative,
                                     const bool use_locale,
                                     const char * const file,
                                     const unsigned line)
{
    const char * key;

    PERL_ARGS_ASSERT_WARN_ON_FIRST_DEPRECATED_USE;

    if (ckWARN_d(category)) {

        key = Perl_form(aTHX_ "%s;%d;%s;%d", name, use_locale, file, line);
        if (! hv_fetch(PL_seen_deprecated_macro, key, strlen(key), 0)) {
            if (! PL_seen_deprecated_macro) {
                PL_seen_deprecated_macro = newHV();
            }
            if (! hv_store(PL_seen_deprecated_macro, key,
                           strlen(key), &PL_sv_undef, 0))
            {
                Perl_croak(aTHX_ "panic: hv_store() unexpectedly failed");
            }

            if (instr(file, "mathoms.c")) {
                Perl_warner(aTHX_ category,
                            "In %s, line %d, starting in Perl v5.32, %s()"
                            " will be removed.  Avoid this message by"
                            " converting to use %s().\n",
                            file, line, name, alternative);
            }
            else {
                Perl_warner(aTHX_ category,
                            "In %s, line %d, starting in Perl v5.32, %s() will"
                            " require an additional parameter.  Avoid this"
                            " message by converting to use %s().\n",
                            file, line, name, alternative);
            }
        }
    }
}
#endif

bool
Perl__is_utf8_FOO(pTHX_ const U8 classnum, const U8 *p, const U8 * const e)
{
    PERL_ARGS_ASSERT__IS_UTF8_FOO;

    return is_utf8_common(p, e, PL_XPosix_ptrs[classnum]);
}

bool
Perl__is_utf8_perl_idstart(pTHX_ const U8 *p, const U8 * const e)
{
    PERL_ARGS_ASSERT__IS_UTF8_PERL_IDSTART;

    return is_utf8_common(p, e, PL_utf8_perl_idstart);
}

bool
Perl__is_utf8_perl_idcont(pTHX_ const U8 *p, const U8 * const e)
{
    PERL_ARGS_ASSERT__IS_UTF8_PERL_IDCONT;

    return is_utf8_common(p, e, PL_utf8_perl_idcont);
}

STATIC UV
S_to_case_cp_list(pTHX_
                  const UV original,
                  const U32 ** const remaining_list,
                  Size_t * remaining_count,
                  SV *invlist, const I32 * const invmap,
                  const U32 * const * const aux_tables,
                  const U8 * const aux_table_lengths,
                  const char * const normal)
{
    SSize_t index;
    I32 base;

    /* Calculate the changed case of code point 'original'.  The first code
     * point of the changed case is returned.
     *
     * If 'remaining_count' is not NULL, *remaining_count will be set to how
     * many *other* code points are in the changed case.  If non-zero and
     * 'remaining_list' is also not NULL, *remaining_list will be set to point
     * to a non-modifiable array containing the second and potentially third
     * code points in the changed case.  (Unicode guarantees a maximum of 3.)
     * Note that this means that *remaining_list is undefined unless there are
     * multiple code points, and the caller has chosen to find out how many by
     * making 'remaining_count' not NULL.
     *
     * 'normal' is a string to use to name the new case in any generated
     * messages, as a fallback if the operation being used is not available.
     *
     * The casing to use is given by the data structures in the remaining
     * arguments.
     */

    PERL_ARGS_ASSERT_TO_CASE_CP_LIST;

    /* 'index' is guaranteed to be non-negative, as this is an inversion map
     * that covers all possible inputs.  See [perl #133365] */
    index = _invlist_search(invlist, original);
    base = invmap[index];

    /* Most likely, the case change will contain just a single code point */
    if (remaining_count) {
        *remaining_count = 0;
    }

    if (LIKELY(base == 0)) {    /* 0 => original was unchanged by casing */

        /* At this bottom level routine is where we warn about illegal code
         * points */
        if (isUNICODE_POSSIBLY_PROBLEMATIC(original)) {
            if (UNLIKELY(UNICODE_IS_SURROGATE(original))) {
                if (ckWARN_d(WARN_SURROGATE)) {
                    const char* desc = (PL_op) ? OP_DESC(PL_op) : normal;
                    Perl_warner(aTHX_ packWARN(WARN_SURROGATE),
                        "Operation \"%s\" returns its argument for"
                        " UTF-16 surrogate U+%04" UVXf, desc, original);
                }
            }
            else if (UNLIKELY(UNICODE_IS_SUPER(original))) {
                if (UNLIKELY(original > MAX_LEGAL_CP)) {
                    Perl_croak(aTHX_ "%s", form_cp_too_large_msg(16, NULL, 0, original));
                }
                if (ckWARN_d(WARN_NON_UNICODE)) {
                    const char* desc = (PL_op) ? OP_DESC(PL_op) : normal;
                    Perl_warner(aTHX_ packWARN(WARN_NON_UNICODE),
                        "Operation \"%s\" returns its argument for"
                        " non-Unicode code point 0x%04" UVXf, desc, original);
                }
            }

            /* Note that non-characters are perfectly legal, so no warning
             * should be given. */
        }

        return original;
    }

    if (LIKELY(base > 0)) {  /* means original mapped to a single code point,
                                different from itself */
        return base + original - invlist_array(invlist)[index];
    }

    /* Here 'base' is negative.  That means the mapping is 1-to-many, and
     * requires an auxiliary table look up.  abs(base) gives the index into a
     * list of such tables which points to the proper aux table.  And a
     * parallel list gives the length of each corresponding aux table.  Skip
     * the first entry in the *remaining returns, as it is returned by the
     * function. */
    base = -base;
    if (remaining_count) {
        *remaining_count = (Size_t) (aux_table_lengths[base] - 1);

        if (remaining_list) {
            *remaining_list  = aux_tables[base] + 1;
        }
    }

    return (UV) aux_tables[base][0];
}

STATIC UV
S__to_utf8_case(pTHX_ const UV original, const U8 *p,
                      U8* ustrp, STRLEN *lenp,
                      SV *invlist, const I32 * const invmap,
                      const U32 * const * const aux_tables,
                      const U8 * const aux_table_lengths,
                      const char * const normal)
{
    /* Change the case of code point 'original'.  If 'p' is non-NULL, it points to
     * the beginning of the (assumed to be valid) UTF-8 representation of
     * 'original'.  'normal' is a string to use to name the new case in any
     * generated messages, as a fallback if the operation being used is not
     * available.  The new case is given by the data structures in the
     * remaining arguments.
     *
     * On return 'ustrp' points to '*lenp' UTF-8 encoded bytes representing the
     * entire changed case string, and the return value is the first code point
     * in that string
     *
     * Note that the <ustrp> needs to be at least UTF8_MAXBYTES_CASE+1 bytes
     * since the changed version may be longer than the original character. */

    const U32 * remaining_list;
    Size_t remaining_count;
    UV first = to_case_cp_list(original,
                               &remaining_list, &remaining_count,
                               invlist, invmap,
                               aux_tables, aux_table_lengths,
                               normal);

    PERL_ARGS_ASSERT__TO_UTF8_CASE;

    /* If the code point maps to itself and we already have its representation,
     * copy it instead of recalculating */
    if (original == first && p) {
        *lenp = UTF8SKIP(p);

        if (p != ustrp) {   /* Don't copy onto itself */
            Copy(p, ustrp, *lenp, U8);
        }
    }
    else {
        U8 * d = ustrp;
        Size_t i;

        d = uvchr_to_utf8(d, first);

        for (i = 0; i < remaining_count; i++) {
            d = uvchr_to_utf8(d, remaining_list[i]);
        }

        *d = '\0';
        *lenp = d - ustrp;
    }

    return first;
}

Size_t
Perl__inverse_folds(pTHX_ const UV cp, U32 * first_folds_to,
                          const U32 ** remaining_folds_to)
{
    /* Returns the count of the number of code points that fold to the input
     * 'cp' (besides itself).
     *
     * If the return is 0, there is nothing else that folds to it, and
     * '*first_folds_to' is set to 0, and '*remaining_folds_to' is set to NULL.
     *
     * If the return is 1, '*first_folds_to' is set to the single code point,
     * and '*remaining_folds_to' is set to NULL.
     *
     * Otherwise, '*first_folds_to' is set to a code point, and
     * '*remaining_fold_to' is set to an array that contains the others.  The
     * length of this array is the returned count minus 1.
     *
     * The reason for this convolution is to avoid having to deal with
     * allocating and freeing memory.  The lists are already constructed, so
     * the return can point to them, but single code points aren't, so would
     * need to be constructed if we didn't employ something like this API
     *
     * The code points returned by this function are all legal Unicode, which
     * occupy at most 21 bits, and so a U32 is sufficient, and the lists are
     * constructed with this size (to save space and memory), and we return
     * pointers, so they must be this size */

    /* 'index' is guaranteed to be non-negative, as this is an inversion map
     * that covers all possible inputs.  See [perl #133365] */
    SSize_t index = _invlist_search(PL_utf8_foldclosures, cp);
    I32 base = _Perl_IVCF_invmap[index];

    PERL_ARGS_ASSERT__INVERSE_FOLDS;

    if (base == 0) {            /* No fold */
        *first_folds_to = 0;
        *remaining_folds_to = NULL;
        return 0;
    }

#ifndef HAS_IVCF_AUX_TABLES     /* This Unicode version only has 1-1 folds */

    assert(base > 0);

#else

    if (UNLIKELY(base < 0)) {   /* Folds to more than one character */

        /* The data structure is set up so that the absolute value of 'base' is
         * an index into a table of pointers to arrays, with the array
         * corresponding to the index being the list of code points that fold
         * to 'cp', and the parallel array containing the length of the list
         * array */
        *first_folds_to = IVCF_AUX_TABLE_ptrs[-base][0];
        *remaining_folds_to = IVCF_AUX_TABLE_ptrs[-base] + 1;
                                                /* +1 excludes first_folds_to */
        return IVCF_AUX_TABLE_lengths[-base];
    }

#endif

    /* Only the single code point.  This works like 'fc(G) = G - A + a' */
    *first_folds_to = (U32) (base + cp
                                  - invlist_array(PL_utf8_foldclosures)[index]);
    *remaining_folds_to = NULL;
    return 1;
}

STATIC UV
S_check_locale_boundary_crossing(pTHX_ const U8* const p, const UV result,
                                       U8* const ustrp, STRLEN *lenp)
{
    /* This is called when changing the case of a UTF-8-encoded character above
     * the Latin1 range, and the operation is in a non-UTF-8 locale.  If the
     * result contains a character that crosses the 255/256 boundary, disallow
     * the change, and return the original code point.  See L<perlfunc/lc> for
     * why;
     *
     * p	points to the original string whose case was changed; assumed
     *          by this routine to be well-formed
     * result	the code point of the first character in the changed-case string
     * ustrp	points to the changed-case string (<result> represents its
     *          first char)
     * lenp	points to the length of <ustrp> */

    UV original;    /* To store the first code point of <p> */

    PERL_ARGS_ASSERT_CHECK_LOCALE_BOUNDARY_CROSSING;

    assert(UTF8_IS_ABOVE_LATIN1(*p));

    /* We know immediately if the first character in the string crosses the
     * boundary, so can skip testing */
    if (result > 255) {

        /* Look at every character in the result; if any cross the
        * boundary, the whole thing is disallowed */
        U8* s = ustrp + UTF8SKIP(ustrp);
        U8* e = ustrp + *lenp;
        while (s < e) {
            if (! UTF8_IS_ABOVE_LATIN1(*s)) {
                goto bad_crossing;
            }
            s += UTF8SKIP(s);
        }

        /* Here, no characters crossed, result is ok as-is, but we warn. */
        _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(p, p + UTF8SKIP(p));
        return result;
    }

  bad_crossing:

    /* Failed, have to return the original */
    original = valid_utf8_to_uvchr(p, lenp);

    /* diag_listed_as: Can't do %s("%s") on non-UTF-8 locale; resolved to "%s". */
    Perl_ck_warner(aTHX_ packWARN(WARN_LOCALE),
                           "Can't do %s(\"\\x{%" UVXf "}\") on non-UTF-8"
                           " locale; resolved to \"\\x{%" UVXf "}\".",
                           OP_DESC(PL_op),
                           original,
                           original);
    Copy(p, ustrp, *lenp, char);
    return original;
}

STATIC UV
S_turkic_fc(pTHX_ const U8 * const p, const U8 * const e,
                        U8 * ustrp, STRLEN *lenp)
{
    /* Returns 0 if the foldcase of the input UTF-8 encoded sequence from
     * p0..e-1 according to Turkic rules is the same as for non-Turkic.
     * Otherwise, it returns the first code point of the Turkic foldcased
     * sequence, and the entire sequence will be stored in *ustrp.  ustrp will
     * contain *lenp bytes
     *
     * Turkic differs only from non-Turkic in that 'i' and LATIN CAPITAL LETTER
     * I WITH DOT ABOVE form a case pair, as do 'I' and LATIN SMALL LETTER
     * DOTLESS I */

    PERL_ARGS_ASSERT_TURKIC_FC;
    assert(e > p);

    if (UNLIKELY(*p == 'I')) {
        *lenp = 2;
        ustrp[0] = UTF8_TWO_BYTE_HI(LATIN_SMALL_LETTER_DOTLESS_I);
        ustrp[1] = UTF8_TWO_BYTE_LO(LATIN_SMALL_LETTER_DOTLESS_I);
        return LATIN_SMALL_LETTER_DOTLESS_I;
    }

    if (UNLIKELY(memBEGINs(p, e - p,
                           LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE_UTF8)))
    {
        *lenp = 1;
        *ustrp = 'i';
        return 'i';
    }

    return 0;
}

STATIC UV
S_turkic_lc(pTHX_ const U8 * const p0, const U8 * const e,
                        U8 * ustrp, STRLEN *lenp)
{
    /* Returns 0 if the lowercase of the input UTF-8 encoded sequence from
     * p0..e-1 according to Turkic rules is the same as for non-Turkic.
     * Otherwise, it returns the first code point of the Turkic lowercased
     * sequence, and the entire sequence will be stored in *ustrp.  ustrp will
     * contain *lenp bytes */

    PERL_ARGS_ASSERT_TURKIC_LC;
    assert(e > p0);

    /* A 'I' requires context as to what to do */
    if (UNLIKELY(*p0 == 'I')) {
        const U8 * p = p0 + 1;

        /* According to the Unicode SpecialCasing.txt file, a capital 'I'
         * modified by a dot above lowercases to 'i' even in turkic locales. */
        while (p < e) {
            UV cp;

            if (memBEGINs(p, e - p, COMBINING_DOT_ABOVE_UTF8)) {
                ustrp[0] = 'i';
                *lenp = 1;
                return 'i';
            }

            /* For the dot above to modify the 'I', it must be part of a
             * combining sequence immediately following the 'I', and no other
             * modifier with a ccc of 230 may intervene */
            cp = utf8_to_uvchr_buf(p, e, NULL);
            if (! _invlist_contains_cp(PL_CCC_non0_non230, cp)) {
                break;
            }

            /* Here the combining sequence continues */
            p += UTF8SKIP(p);
        }
    }

    /* In all other cases the lc is the same as the fold */
    return turkic_fc(p0, e, ustrp, lenp);
}

STATIC UV
S_turkic_uc(pTHX_ const U8 * const p, const U8 * const e,
                        U8 * ustrp, STRLEN *lenp)
{
    /* Returns 0 if the upper or title-case of the input UTF-8 encoded sequence
     * from p0..e-1 according to Turkic rules is the same as for non-Turkic.
     * Otherwise, it returns the first code point of the Turkic upper or
     * title-cased sequence, and the entire sequence will be stored in *ustrp.
     * ustrp will contain *lenp bytes
     *
     * Turkic differs only from non-Turkic in that 'i' and LATIN CAPITAL LETTER
     * I WITH DOT ABOVE form a case pair, as do 'I' and LATIN SMALL LETTER
     * DOTLESS I */

    PERL_ARGS_ASSERT_TURKIC_UC;
    assert(e > p);

    if (*p == 'i') {
        *lenp = 2;
        ustrp[0] = UTF8_TWO_BYTE_HI(LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE);
        ustrp[1] = UTF8_TWO_BYTE_LO(LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE);
        return LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE;
    }

    if (memBEGINs(p, e - p, LATIN_SMALL_LETTER_DOTLESS_I_UTF8)) {
        *lenp = 1;
        *ustrp = 'I';
        return 'I';
    }

    return 0;
}

/* The process for changing the case is essentially the same for the four case
 * change types, except there are complications for folding.  Otherwise the
 * difference is only which case to change to.  To make sure that they all do
 * the same thing, the bodies of the functions are extracted out into the
 * following two macros.  The functions are written with the same variable
 * names, and these are known and used inside these macros.  It would be
 * better, of course, to have inline functions to do it, but since different
 * macros are called, depending on which case is being changed to, this is not
 * feasible in C (to khw's knowledge).  Two macros are created so that the fold
 * function can start with the common start macro, then finish with its special
 * handling; while the other three cases can just use the common end macro.
 *
 * The algorithm is to use the proper (passed in) macro or function to change
 * the case for code points that are below 256.  The macro is used if using
 * locale rules for the case change; the function if not.  If the code point is
 * above 255, it is computed from the input UTF-8, and another macro is called
 * to do the conversion.  If necessary, the output is converted to UTF-8.  If
 * using a locale, we have to check that the change did not cross the 255/256
 * boundary, see check_locale_boundary_crossing() for further details.
 *
 * The macros are split with the correct case change for the below-256 case
 * stored into 'result', and in the middle of an else clause for the above-255
 * case.  At that point in the 'else', 'result' is not the final result, but is
 * the input code point calculated from the UTF-8.  The fold code needs to
 * realize all this and take it from there.
 *
 * To deal with Turkic locales, the function specified by the parameter
 * 'turkic' is called when appropriate.
 *
 * If you read the two macros as sequential, it's easier to understand what's
 * going on. */
#define CASE_CHANGE_BODY_START(locale_flags, libc_change_function, L1_func,  \
                               L1_func_extra_param, turkic)                  \
                                                                             \
    if (flags & (locale_flags)) {                                            \
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;                                  \
        if (IN_UTF8_CTYPE_LOCALE) {                                          \
            if (UNLIKELY(IN_UTF8_TURKIC_LOCALE)) {                           \
                UV ret = turkic(p, e, ustrp, lenp);                          \
                if (ret) return ret;                                         \
            }                                                                \
                                                                             \
            /* Otherwise, treat a UTF-8 locale as not being in locale at     \
             * all */                                                        \
            flags &= ~(locale_flags);                                        \
        }                                                                    \
    }                                                                        \
                                                                             \
    if (UTF8_IS_INVARIANT(*p)) {                                             \
        if (flags & (locale_flags)) {                                        \
            result = libc_change_function(*p);                               \
        }                                                                    \
        else {                                                               \
            return L1_func(*p, ustrp, lenp, L1_func_extra_param);            \
        }                                                                    \
    }                                                                        \
    else if UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(p, e) {                          \
        U8 c = EIGHT_BIT_UTF8_TO_NATIVE(*p, *(p+1));                         \
        if (flags & (locale_flags)) {                                        \
            result = libc_change_function(c);                                \
        }                                                                    \
        else {                                                               \
            return L1_func(c, ustrp, lenp,  L1_func_extra_param);            \
        }                                                                    \
    }                                                                        \
    else {  /* malformed UTF-8 or ord above 255 */                           \
        STRLEN len_result;                                                   \
        result = utf8n_to_uvchr(p, e - p, &len_result, UTF8_CHECK_ONLY);     \
        if (len_result == (STRLEN) -1) {                                     \
            _force_out_malformed_utf8_message(p, e, 0, 1 /* Die */ );        \
        }

#define CASE_CHANGE_BODY_END(locale_flags, change_macro)                     \
        result = change_macro(result, p, ustrp, lenp);                       \
                                                                             \
        if (flags & (locale_flags)) {                                        \
            result = check_locale_boundary_crossing(p, result, ustrp, lenp); \
        }                                                                    \
        return result;                                                       \
    }                                                                        \
                                                                             \
    /* Here, used locale rules.  Convert back to UTF-8 */                    \
    if (UTF8_IS_INVARIANT(result)) {                                         \
        *ustrp = (U8) result;                                                \
        *lenp = 1;                                                           \
    }                                                                        \
    else {                                                                   \
        *ustrp = UTF8_EIGHT_BIT_HI((U8) result);                             \
        *(ustrp + 1) = UTF8_EIGHT_BIT_LO((U8) result);                       \
        *lenp = 2;                                                           \
    }                                                                        \
                                                                             \
    return result;

/* Not currently externally documented, and subject to change:
 * <flags> is set iff the rules from the current underlying locale are to
 *         be used. */

UV
Perl__to_utf8_upper_flags(pTHX_ const U8 *p,
                                const U8 *e,
                                U8* ustrp,
                                STRLEN *lenp,
                                bool flags)
{
    UV result;

    PERL_ARGS_ASSERT__TO_UTF8_UPPER_FLAGS;

    /* ~0 makes anything non-zero in 'flags' mean we are using locale rules */
    /* 2nd char of uc(U+DF) is 'S' */
    CASE_CHANGE_BODY_START(~0, toupper, _to_upper_title_latin1, 'S',
                                                                    turkic_uc);
    CASE_CHANGE_BODY_END  (~0, CALL_UPPER_CASE);
}

/* Not currently externally documented, and subject to change:
 * <flags> is set iff the rules from the current underlying locale are to be
 *         used.  Since titlecase is not defined in POSIX, for other than a
 *         UTF-8 locale, uppercase is used instead for code points < 256.
 */

UV
Perl__to_utf8_title_flags(pTHX_ const U8 *p,
                                const U8 *e,
                                U8* ustrp,
                                STRLEN *lenp,
                                bool flags)
{
    UV result;

    PERL_ARGS_ASSERT__TO_UTF8_TITLE_FLAGS;

    /* 2nd char of ucfirst(U+DF) is 's' */
    CASE_CHANGE_BODY_START(~0, toupper, _to_upper_title_latin1, 's',
                                                                    turkic_uc);
    CASE_CHANGE_BODY_END  (~0, CALL_TITLE_CASE);
}

/* Not currently externally documented, and subject to change:
 * <flags> is set iff the rules from the current underlying locale are to
 *         be used.
 */

UV
Perl__to_utf8_lower_flags(pTHX_ const U8 *p,
                                const U8 *e,
                                U8* ustrp,
                                STRLEN *lenp,
                                bool flags)
{
    UV result;

    PERL_ARGS_ASSERT__TO_UTF8_LOWER_FLAGS;

    CASE_CHANGE_BODY_START(~0, tolower, to_lower_latin1, 0 /* 0 is dummy */,
                                                                    turkic_lc);
    CASE_CHANGE_BODY_END  (~0, CALL_LOWER_CASE)
}

/* Not currently externally documented, and subject to change,
 * in <flags>
 *	bit FOLD_FLAGS_LOCALE is set iff the rules from the current underlying
 *	                      locale are to be used.
 *      bit FOLD_FLAGS_FULL   is set iff full case folds are to be used;
 *			      otherwise simple folds
 *      bit FOLD_FLAGS_NOMIX_ASCII is set iff folds of non-ASCII to ASCII are
 *			      prohibited
 */

UV
Perl__to_utf8_fold_flags(pTHX_ const U8 *p,
                               const U8 *e,
                               U8* ustrp,
                               STRLEN *lenp,
                               U8 flags)
{
    UV result;

    PERL_ARGS_ASSERT__TO_UTF8_FOLD_FLAGS;

    /* These are mutually exclusive */
    assert (! ((flags & FOLD_FLAGS_LOCALE) && (flags & FOLD_FLAGS_NOMIX_ASCII)));

    assert(p != ustrp); /* Otherwise overwrites */

    CASE_CHANGE_BODY_START(FOLD_FLAGS_LOCALE, tolower, _to_fold_latin1,
                 ((flags) & (FOLD_FLAGS_FULL | FOLD_FLAGS_NOMIX_ASCII)),
                                                                    turkic_fc);

        result = CALL_FOLD_CASE(result, p, ustrp, lenp, flags & FOLD_FLAGS_FULL);

        if (flags & FOLD_FLAGS_LOCALE) {

#           define LONG_S_T      LATIN_SMALL_LIGATURE_LONG_S_T_UTF8
#         ifdef LATIN_CAPITAL_LETTER_SHARP_S_UTF8
#           define CAP_SHARP_S   LATIN_CAPITAL_LETTER_SHARP_S_UTF8

            /* Special case these two characters, as what normally gets
             * returned under locale doesn't work */
            if (memBEGINs((char *) p, e - p, CAP_SHARP_S))
            {
                /* diag_listed_as: Can't do %s("%s") on non-UTF-8 locale; resolved to "%s". */
                Perl_ck_warner(aTHX_ packWARN(WARN_LOCALE),
                              "Can't do fc(\"\\x{1E9E}\") on non-UTF-8 locale; "
                              "resolved to \"\\x{17F}\\x{17F}\".");
                goto return_long_s;
            }
            else
#endif
                 if (memBEGINs((char *) p, e - p, LONG_S_T))
            {
                /* diag_listed_as: Can't do %s("%s") on non-UTF-8 locale; resolved to "%s". */
                Perl_ck_warner(aTHX_ packWARN(WARN_LOCALE),
                              "Can't do fc(\"\\x{FB05}\") on non-UTF-8 locale; "
                              "resolved to \"\\x{FB06}\".");
                goto return_ligature_st;
            }

#if    UNICODE_MAJOR_VERSION   == 3         \
    && UNICODE_DOT_VERSION     == 0         \
    && UNICODE_DOT_DOT_VERSION == 1
#           define DOTTED_I   LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE_UTF8

            /* And special case this on this Unicode version only, for the same
             * reaons the other two are special cased.  They would cross the
             * 255/256 boundary which is forbidden under /l, and so the code
             * wouldn't catch that they are equivalent (which they are only in
             * this release) */
            else if (memBEGINs((char *) p, e - p, DOTTED_I)) {
                /* diag_listed_as: Can't do %s("%s") on non-UTF-8 locale; resolved to "%s". */
                Perl_ck_warner(aTHX_ packWARN(WARN_LOCALE),
                              "Can't do fc(\"\\x{0130}\") on non-UTF-8 locale; "
                              "resolved to \"\\x{0131}\".");
                goto return_dotless_i;
            }
#endif

            return check_locale_boundary_crossing(p, result, ustrp, lenp);
        }
        else if (! (flags & FOLD_FLAGS_NOMIX_ASCII)) {
            return result;
        }
        else {
            /* This is called when changing the case of a UTF-8-encoded
             * character above the ASCII range, and the result should not
             * contain an ASCII character. */

            UV original;    /* To store the first code point of <p> */

            /* Look at every character in the result; if any cross the
            * boundary, the whole thing is disallowed */
            U8* s = ustrp;
            U8* send = ustrp + *lenp;
            while (s < send) {
                if (isASCII(*s)) {
                    /* Crossed, have to return the original */
                    original = valid_utf8_to_uvchr(p, lenp);

                    /* But in these instances, there is an alternative we can
                     * return that is valid */
                    if (original == LATIN_SMALL_LETTER_SHARP_S
#ifdef LATIN_CAPITAL_LETTER_SHARP_S /* not defined in early Unicode releases */
                        || original == LATIN_CAPITAL_LETTER_SHARP_S
#endif
                    ) {
                        goto return_long_s;
                    }
                    else if (original == LATIN_SMALL_LIGATURE_LONG_S_T) {
                        goto return_ligature_st;
                    }
#if    UNICODE_MAJOR_VERSION   == 3         \
    && UNICODE_DOT_VERSION     == 0         \
    && UNICODE_DOT_DOT_VERSION == 1

                    else if (original == LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE) {
                        goto return_dotless_i;
                    }
#endif
                    Copy(p, ustrp, *lenp, char);
                    return original;
                }
                s += UTF8SKIP(s);
            }

            /* Here, no characters crossed, result is ok as-is */
            return result;
        }
    }

    /* Here, used locale rules.  Convert back to UTF-8 */
    if (UTF8_IS_INVARIANT(result)) {
        *ustrp = (U8) result;
        *lenp = 1;
    }
    else {
        *ustrp = UTF8_EIGHT_BIT_HI((U8) result);
        *(ustrp + 1) = UTF8_EIGHT_BIT_LO((U8) result);
        *lenp = 2;
    }

    return result;

  return_long_s:
    /* Certain folds to 'ss' are prohibited by the options, but they do allow
     * folds to a string of two of these characters.  By returning this
     * instead, then, e.g.,
     *      fc("\x{1E9E}") eq fc("\x{17F}\x{17F}")
     * works. */

    *lenp = 2 * STRLENs(LATIN_SMALL_LETTER_LONG_S_UTF8);
    Copy(LATIN_SMALL_LETTER_LONG_S_UTF8   LATIN_SMALL_LETTER_LONG_S_UTF8,
        ustrp, *lenp, U8);
    return LATIN_SMALL_LETTER_LONG_S;

  return_ligature_st:
    /* Two folds to 'st' are prohibited by the options; instead we pick one and
     * have the other one fold to it */

    *lenp = STRLENs(LATIN_SMALL_LIGATURE_ST_UTF8);
    Copy(LATIN_SMALL_LIGATURE_ST_UTF8, ustrp, *lenp, U8);
    return LATIN_SMALL_LIGATURE_ST;

#if    UNICODE_MAJOR_VERSION   == 3         \
    && UNICODE_DOT_VERSION     == 0         \
    && UNICODE_DOT_DOT_VERSION == 1

  return_dotless_i:
    *lenp = STRLENs(LATIN_SMALL_LETTER_DOTLESS_I_UTF8);
    Copy(LATIN_SMALL_LETTER_DOTLESS_I_UTF8, ustrp, *lenp, U8);
    return LATIN_SMALL_LETTER_DOTLESS_I;

#endif

}

bool
Perl_check_utf8_print(pTHX_ const U8* s, const STRLEN len)
{
    /* May change: warns if surrogates, non-character code points, or
     * non-Unicode code points are in 's' which has length 'len' bytes.
     * Returns TRUE if none found; FALSE otherwise.  The only other validity
     * check is to make sure that this won't exceed the string's length nor
     * overflow */

    const U8* const e = s + len;
    bool ok = TRUE;

    PERL_ARGS_ASSERT_CHECK_UTF8_PRINT;

    while (s < e) {
        if (UTF8SKIP(s) > len) {
            Perl_ck_warner_d(aTHX_ packWARN(WARN_UTF8),
                           "%s in %s", unees, PL_op ? OP_DESC(PL_op) : "print");
            return FALSE;
        }
        if (UNLIKELY(isUTF8_POSSIBLY_PROBLEMATIC(*s))) {
            if (UNLIKELY(UTF8_IS_SUPER(s, e))) {
                if (   ckWARN_d(WARN_NON_UNICODE)
                    || UNLIKELY(0 < does_utf8_overflow(s, s + len,
                                               0 /* Don't consider overlongs */
                                               )))
                {
                    /* A side effect of this function will be to warn */
                    (void) utf8n_to_uvchr(s, e - s, NULL, UTF8_WARN_SUPER);
                    ok = FALSE;
                }
            }
            else if (UNLIKELY(UTF8_IS_SURROGATE(s, e))) {
                if (ckWARN_d(WARN_SURROGATE)) {
                    /* This has a different warning than the one the called
                     * function would output, so can't just call it, unlike we
                     * do for the non-chars and above-unicodes */
                    UV uv = utf8_to_uvchr_buf(s, e, NULL);
                    Perl_warner(aTHX_ packWARN(WARN_SURROGATE),
                        "Unicode surrogate U+%04" UVXf " is illegal in UTF-8",
                                             uv);
                    ok = FALSE;
                }
            }
            else if (   UNLIKELY(UTF8_IS_NONCHAR(s, e))
                     && (ckWARN_d(WARN_NONCHAR)))
            {
                /* A side effect of this function will be to warn */
                (void) utf8n_to_uvchr(s, e - s, NULL, UTF8_WARN_NONCHAR);
                ok = FALSE;
            }
        }
        s += UTF8SKIP(s);
    }

    return ok;
}

/*
=for apidoc pv_uni_display

Build to the scalar C<dsv> a displayable version of the UTF-8 encoded string
C<spv>, length C<len>, the displayable version being at most C<pvlim> bytes
long (if longer, the rest is truncated and C<"..."> will be appended).

The C<flags> argument can have C<UNI_DISPLAY_ISPRINT> set to display
C<isPRINT()>able characters as themselves, C<UNI_DISPLAY_BACKSLASH>
to display the C<\\[nrfta\\]> as the backslashed versions (like C<"\n">)
(C<UNI_DISPLAY_BACKSLASH> is preferred over C<UNI_DISPLAY_ISPRINT> for C<"\\">).
C<UNI_DISPLAY_QQ> (and its alias C<UNI_DISPLAY_REGEX>) have both
C<UNI_DISPLAY_BACKSLASH> and C<UNI_DISPLAY_ISPRINT> turned on.

Additionally, there is now C<UNI_DISPLAY_BACKSPACE> which allows C<\b> for a
backspace, but only when C<UNI_DISPLAY_BACKSLASH> also is set.

The pointer to the PV of the C<dsv> is returned.

See also L</sv_uni_display>.

=for apidoc Amnh||UNI_DISPLAY_BACKSLASH
=for apidoc Amnh||UNI_DISPLAY_BACKSPACE
=for apidoc Amnh||UNI_DISPLAY_ISPRINT
=for apidoc Amnh||UNI_DISPLAY_QQ
=for apidoc Amnh||UNI_DISPLAY_REGEX
=cut
*/
char *
Perl_pv_uni_display(pTHX_ SV *dsv, const U8 *spv, STRLEN len, STRLEN pvlim,
                          UV flags)
{
    int truncated = 0;
    const char *s, *e;

    PERL_ARGS_ASSERT_PV_UNI_DISPLAY;

    SvPVCLEAR(dsv);
    SvUTF8_off(dsv);
    for (s = (const char *)spv, e = s + len; s < e; s += UTF8SKIP(s)) {
         UV u;
         bool ok = 0;

         if (pvlim && SvCUR(dsv) >= pvlim) {
              truncated++;
              break;
         }
         u = utf8_to_uvchr_buf((U8*)s, (U8*)e, 0);
         if (u < 256) {
             const U8 c = (U8) u;
             if (flags & UNI_DISPLAY_BACKSLASH) {
                 if (    isMNEMONIC_CNTRL(c)
                     && (   c != '\b'
                         || (flags & UNI_DISPLAY_BACKSPACE)))
                 {
                    const char * mnemonic = cntrl_to_mnemonic(c);
                    sv_catpvn(dsv, mnemonic, strlen(mnemonic));
                    ok = 1;
                 }
                 else if (c == '\\') {
                    sv_catpvs(dsv, "\\\\");
                    ok = 1;
                 }
             }
             /* isPRINT() is the locale-blind version. */
             if (!ok && (flags & UNI_DISPLAY_ISPRINT) && isPRINT(c)) {
                 const char string = c;
                 sv_catpvn(dsv, &string, 1);
                 ok = 1;
             }
         }
         if (!ok)
             Perl_sv_catpvf(aTHX_ dsv, "\\x{%" UVxf "}", u);
    }
    if (truncated)
         sv_catpvs(dsv, "...");

    return SvPVX(dsv);
}

/*
=for apidoc sv_uni_display

Build to the scalar C<dsv> a displayable version of the scalar C<sv>,
the displayable version being at most C<pvlim> bytes long
(if longer, the rest is truncated and "..." will be appended).

The C<flags> argument is as in L</pv_uni_display>().

The pointer to the PV of the C<dsv> is returned.

=cut
*/
char *
Perl_sv_uni_display(pTHX_ SV *dsv, SV *ssv, STRLEN pvlim, UV flags)
{
    const char * const ptr =
        isREGEXP(ssv) ? RX_WRAPPED((REGEXP*)ssv) : SvPVX_const(ssv);

    PERL_ARGS_ASSERT_SV_UNI_DISPLAY;

    return Perl_pv_uni_display(aTHX_ dsv, (const U8*)ptr,
                                SvCUR(ssv), pvlim, flags);
}

/*
=for apidoc foldEQ_utf8

Returns true if the leading portions of the strings C<s1> and C<s2> (either or
both of which may be in UTF-8) are the same case-insensitively; false
otherwise.  How far into the strings to compare is determined by other input
parameters.

If C<u1> is true, the string C<s1> is assumed to be in UTF-8-encoded Unicode;
otherwise it is assumed to be in native 8-bit encoding.  Correspondingly for
C<u2> with respect to C<s2>.

If the byte length C<l1> is non-zero, it says how far into C<s1> to check for
fold equality.  In other words, C<s1>+C<l1> will be used as a goal to reach.
The scan will not be considered to be a match unless the goal is reached, and
scanning won't continue past that goal.  Correspondingly for C<l2> with respect
to C<s2>.

If C<pe1> is non-C<NULL> and the pointer it points to is not C<NULL>, that
pointer is considered an end pointer to the position 1 byte past the maximum
point in C<s1> beyond which scanning will not continue under any circumstances.
(This routine assumes that UTF-8 encoded input strings are not malformed;
malformed input can cause it to read past C<pe1>).  This means that if both
C<l1> and C<pe1> are specified, and C<pe1> is less than C<s1>+C<l1>, the match
will never be successful because it can never
get as far as its goal (and in fact is asserted against).  Correspondingly for
C<pe2> with respect to C<s2>.

At least one of C<s1> and C<s2> must have a goal (at least one of C<l1> and
C<l2> must be non-zero), and if both do, both have to be
reached for a successful match.   Also, if the fold of a character is multiple
characters, all of them must be matched (see tr21 reference below for
'folding').

Upon a successful match, if C<pe1> is non-C<NULL>,
it will be set to point to the beginning of the I<next> character of C<s1>
beyond what was matched.  Correspondingly for C<pe2> and C<s2>.

For case-insensitiveness, the "casefolding" of Unicode is used
instead of upper/lowercasing both the characters, see
L<https://www.unicode.org/reports/tr21/> (Case Mappings).

=for apidoc Cmnh||FOLDEQ_UTF8_NOMIX_ASCII
=for apidoc Cmnh||FOLDEQ_LOCALE
=for apidoc Cmnh||FOLDEQ_S1_ALREADY_FOLDED
=for apidoc Cmnh||FOLDEQ_S1_FOLDS_SANE
=for apidoc Cmnh||FOLDEQ_S2_ALREADY_FOLDED
=for apidoc Cmnh||FOLDEQ_S2_FOLDS_SANE

=cut */

/* A flags parameter has been added which may change, and hence isn't
 * externally documented.  Currently it is:
 *  0 for as-documented above
 *  FOLDEQ_UTF8_NOMIX_ASCII meaning that if a non-ASCII character folds to an
                            ASCII one, to not match
 *  FOLDEQ_LOCALE	    is set iff the rules from the current underlying
 *	                    locale are to be used.
 *  FOLDEQ_S1_ALREADY_FOLDED  s1 has already been folded before calling this
 *                          routine.  This allows that step to be skipped.
 *                          Currently, this requires s1 to be encoded as UTF-8
 *                          (u1 must be true), which is asserted for.
 *  FOLDEQ_S1_FOLDS_SANE    With either NOMIX_ASCII or LOCALE, no folds may
 *                          cross certain boundaries.  Hence, the caller should
 *                          let this function do the folding instead of
 *                          pre-folding.  This code contains an assertion to
 *                          that effect.  However, if the caller knows what
 *                          it's doing, it can pass this flag to indicate that,
 *                          and the assertion is skipped.
 *  FOLDEQ_S2_ALREADY_FOLDED  Similar to FOLDEQ_S1_ALREADY_FOLDED, but applies
 *                          to s2, and s2 doesn't have to be UTF-8 encoded.
 *                          This introduces an asymmetry to save a few branches
 *                          in a loop.  Currently, this is not a problem, as
 *                          never are both inputs pre-folded.  Simply call this
 *                          function with the pre-folded one as the second
 *                          string.
 *  FOLDEQ_S2_FOLDS_SANE
 */

I32
Perl_foldEQ_utf8_flags(pTHX_ const char *s1, char **pe1, UV l1, bool u1,
                             const char *s2, char **pe2, UV l2, bool u2,
                             U32 flags)
{
    const U8 *p1  = (const U8*)s1; /* Point to current char */
    const U8 *p2  = (const U8*)s2;
    const U8 *g1 = NULL;       /* goal for s1 */
    const U8 *g2 = NULL;
    const U8 *e1 = NULL;       /* Don't scan s1 past this */
    U8 *f1 = NULL;             /* Point to current folded */
    const U8 *e2 = NULL;
    U8 *f2 = NULL;
    STRLEN n1 = 0, n2 = 0;              /* Number of bytes in current char */
    U8 foldbuf1[UTF8_MAXBYTES_CASE+1];
    U8 foldbuf2[UTF8_MAXBYTES_CASE+1];
    U8 flags_for_folder = FOLD_FLAGS_FULL;

    PERL_ARGS_ASSERT_FOLDEQ_UTF8_FLAGS;

    assert( ! (             (flags & (FOLDEQ_UTF8_NOMIX_ASCII | FOLDEQ_LOCALE))
               && ((        (flags &  FOLDEQ_S1_ALREADY_FOLDED)
                        && !(flags &  FOLDEQ_S1_FOLDS_SANE))
                    || (    (flags &  FOLDEQ_S2_ALREADY_FOLDED)
                        && !(flags &  FOLDEQ_S2_FOLDS_SANE)))));
    /* The algorithm is to trial the folds without regard to the flags on
     * the first line of the above assert(), and then see if the result
     * violates them.  This means that the inputs can't be pre-folded to a
     * violating result, hence the assert.  This could be changed, with the
     * addition of extra tests here for the already-folded case, which would
     * slow it down.  That cost is more than any possible gain for when these
     * flags are specified, as the flags indicate /il or /iaa matching which
     * is less common than /iu, and I (khw) also believe that real-world /il
     * and /iaa matches are most likely to involve code points 0-255, and this
     * function only under rare conditions gets called for 0-255. */

    if (flags & FOLDEQ_LOCALE) {
        if (IN_UTF8_CTYPE_LOCALE) {
            if (UNLIKELY(IN_UTF8_TURKIC_LOCALE)) {
                flags_for_folder |= FOLD_FLAGS_LOCALE;
            }
            else {
                flags &= ~FOLDEQ_LOCALE;
            }
        }
        else {
            flags_for_folder |= FOLD_FLAGS_LOCALE;
        }
    }
    if (flags & FOLDEQ_UTF8_NOMIX_ASCII) {
        flags_for_folder |= FOLD_FLAGS_NOMIX_ASCII;
    }

    if (pe1) {
        e1 = *(U8**)pe1;
    }

    if (l1) {
        g1 = (const U8*)s1 + l1;
    }

    if (pe2) {
        e2 = *(U8**)pe2;
    }

    if (l2) {
        g2 = (const U8*)s2 + l2;
    }

    /* Must have at least one goal */
    assert(g1 || g2);

    if (g1) {

        /* Will never match if goal is out-of-bounds */
        assert(! e1  || e1 >= g1);

        /* Here, there isn't an end pointer, or it is beyond the goal.  We
        * only go as far as the goal */
        e1 = g1;
    }
    else {
        assert(e1);    /* Must have an end for looking at s1 */
    }

    /* Same for goal for s2 */
    if (g2) {
        assert(! e2  || e2 >= g2);
        e2 = g2;
    }
    else {
        assert(e2);
    }

    /* If both operands are already folded, we could just do a memEQ on the
     * whole strings at once, but it would be better if the caller realized
     * this and didn't even call us */

    /* Look through both strings, a character at a time */
    while (p1 < e1 && p2 < e2) {

        /* If at the beginning of a new character in s1, get its fold to use
         * and the length of the fold. */
        if (n1 == 0) {
            if (flags & FOLDEQ_S1_ALREADY_FOLDED) {
                f1 = (U8 *) p1;
                assert(u1);
                n1 = UTF8SKIP(f1);
            }
            else {
                if (isASCII(*p1) && ! (flags & FOLDEQ_LOCALE)) {

                    /* We have to forbid mixing ASCII with non-ASCII if the
                     * flags so indicate.  And, we can short circuit having to
                     * call the general functions for this common ASCII case,
                     * all of whose non-locale folds are also ASCII, and hence
                     * UTF-8 invariants, so the UTF8ness of the strings is not
                     * relevant. */
                    if ((flags & FOLDEQ_UTF8_NOMIX_ASCII) && ! isASCII(*p2)) {
                        return 0;
                    }
                    n1 = 1;
                    *foldbuf1 = toFOLD(*p1);
                }
                else if (u1) {
                    _toFOLD_utf8_flags(p1, e1, foldbuf1, &n1, flags_for_folder);
                }
                else {  /* Not UTF-8, get UTF-8 fold */
                    _to_uni_fold_flags(*p1, foldbuf1, &n1, flags_for_folder);
                }
                f1 = foldbuf1;
            }
        }

        if (n2 == 0) {    /* Same for s2 */
            if (flags & FOLDEQ_S2_ALREADY_FOLDED) {

                /* Point to the already-folded character.  But for non-UTF-8
                 * variants, convert to UTF-8 for the algorithm below */
                if (UTF8_IS_INVARIANT(*p2)) {
                    f2 = (U8 *) p2;
                    n2 = 1;
                }
                else if (u2) {
                    f2 = (U8 *) p2;
                    n2 = UTF8SKIP(f2);
                }
                else {
                    foldbuf2[0] = UTF8_EIGHT_BIT_HI(*p2);
                    foldbuf2[1] = UTF8_EIGHT_BIT_LO(*p2);
                    f2 = foldbuf2;
                    n2 = 2;
                }
            }
            else {
                if (isASCII(*p2) && ! (flags & FOLDEQ_LOCALE)) {
                    if ((flags & FOLDEQ_UTF8_NOMIX_ASCII) && ! isASCII(*p1)) {
                        return 0;
                    }
                    n2 = 1;
                    *foldbuf2 = toFOLD(*p2);
                }
                else if (u2) {
                    _toFOLD_utf8_flags(p2, e2, foldbuf2, &n2, flags_for_folder);
                }
                else {
                    _to_uni_fold_flags(*p2, foldbuf2, &n2, flags_for_folder);
                }
                f2 = foldbuf2;
            }
        }

        /* Here f1 and f2 point to the beginning of the strings to compare.
         * These strings are the folds of the next character from each input
         * string, stored in UTF-8. */

        /* While there is more to look for in both folds, see if they
        * continue to match */
        while (n1 && n2) {
            U8 fold_length = UTF8SKIP(f1);
            if (fold_length != UTF8SKIP(f2)
                || (fold_length == 1 && *f1 != *f2) /* Short circuit memNE
                                                       function call for single
                                                       byte */
                || memNE((char*)f1, (char*)f2, fold_length))
            {
                return 0; /* mismatch */
            }

            /* Here, they matched, advance past them */
            n1 -= fold_length;
            f1 += fold_length;
            n2 -= fold_length;
            f2 += fold_length;
        }

        /* When reach the end of any fold, advance the input past it */
        if (n1 == 0) {
            p1 += u1 ? UTF8SKIP(p1) : 1;
        }
        if (n2 == 0) {
            p2 += u2 ? UTF8SKIP(p2) : 1;
        }
    } /* End of loop through both strings */

    /* A match is defined by each scan that specified an explicit length
    * reaching its final goal, and the other not having matched a partial
    * character (which can happen when the fold of a character is more than one
    * character). */
    if (! ((g1 == 0 || p1 == g1) && (g2 == 0 || p2 == g2)) || n1 || n2) {
        return 0;
    }

    /* Successful match.  Set output pointers */
    if (pe1) {
        *pe1 = (char*)p1;
    }
    if (pe2) {
        *pe2 = (char*)p2;
    }
    return 1;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
