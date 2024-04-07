#ifndef ENCODE_H
#define ENCODE_H

#ifndef H_PERL
/* check whether we're "in perl" so that we can do data parts without
   getting extern references to the code parts
*/
typedef unsigned char U8;
#endif

typedef struct encpage_s encpage_t;

struct encpage_s
{
    /* fields ordered to pack nicely on 32-bit machines */
    const U8 *const seq;   /* Packed output sequences we generate 
                  if we match */
    const encpage_t *const next;      /* Page to go to if we match */
    const U8   min;        /* Min value of octet to match this entry */
    const U8   max;        /* Max value of octet to match this entry */
    const U8   dlen;       /* destination length - 
                  size of entries in seq */
    const U8   slen;       /* source length - 
                  number of source octets needed */
};

/*
  At any point in a translation there is a page pointer which points
  at an array of the above structures.

  Basic operation :
  get octet from source stream.
  if (octet >= min && octet < max) {
    if slen is 0 then we cannot represent this character.
    if we have less than slen octets (including this one) then 
      we have a partial character.
    otherwise
      copy dlen octets from seq + dlen*(octet-min) to output
      (dlen may be zero if we don't know yet.)
      load page pointer with next to continue.
      (is slen is one this is end of a character)
      get next octet.
  }
  else {
    increment the page pointer to look at next slot in the array
  }

  arrays SHALL be constructed so there is an entry which matches
  ..0xFF at the end, and either maps it or indicates no
  representation.

  if MSB of slen is set then mapping is an approximate "FALLBACK" entry.

*/


typedef struct encode_s encode_t;
struct encode_s
{
    const encpage_t *const t_utf8;  /* Starting table for translation from 
                       the encoding to UTF-8 form */
    const encpage_t *const f_utf8;  /* Starting table for translation 
                       from UTF-8 to the encoding */
    const U8 *const rep;            /* Replacement character in this
                       encoding e.g. "?" */
    int        replen;              /* Number of octets in rep */
    U8         min_el;              /* Minimum octets to represent a
                       character */
    U8         max_el;              /* Maximum octets to represent a
                       character */
    const char *const name[2];      /* name(s) of this encoding */
};

#ifdef H_PERL
/* See comment at top of file for deviousness */

extern int do_encode(const encpage_t *enc, const U8 *src, STRLEN *slen,
                     U8 *dst, STRLEN dlen, STRLEN *dout, int approx,
             const U8 *term, STRLEN tlen);

extern void Encode_DefineEncoding(encode_t *enc);

#endif /* H_PERL */

#define ENCODE_NOSPACE  1
#define ENCODE_PARTIAL  2
#define ENCODE_NOREP    3
#define ENCODE_FALLBACK 4
#define ENCODE_FOUND_TERM 5

/* Use the perl core value if available; it is portable to EBCDIC */
#ifdef REPLACEMENT_CHARACTER_UTF8
#  define FBCHAR_UTF8		REPLACEMENT_CHARACTER_UTF8
#else
#  define FBCHAR_UTF8           "\xEF\xBF\xBD"
#endif

#define  ENCODE_DIE_ON_ERR     0x0001 /* croaks immediately */
#define  ENCODE_WARN_ON_ERR    0x0002 /* warn on error; may proceed */
#define  ENCODE_RETURN_ON_ERR  0x0004 /* immediately returns on NOREP */
#define  ENCODE_LEAVE_SRC      0x0008 /* $src updated unless set */
#define  ENCODE_ONLY_PRAGMA_WARNINGS 0x0010 /* when enabled report only warnings configured by pragma warnings, otherwise report all warnings; no effect without ENCODE_WARN_ON_ERR */
#define  ENCODE_PERLQQ         0x0100 /* perlqq fallback string */
#define  ENCODE_HTMLCREF       0x0200 /* HTML character ref. fb mode */
#define  ENCODE_XMLCREF        0x0400 /* XML  character ref. fb mode */
#define  ENCODE_STOP_AT_PARTIAL 0x0800 /* stop at partial explicitly */

#define  ENCODE_FB_DEFAULT     0x0000
#define  ENCODE_FB_CROAK       0x0001
#define  ENCODE_FB_QUIET       ENCODE_RETURN_ON_ERR
#define  ENCODE_FB_WARN        (ENCODE_RETURN_ON_ERR|ENCODE_WARN_ON_ERR)
#define  ENCODE_FB_PERLQQ      (ENCODE_PERLQQ|ENCODE_LEAVE_SRC)
#define  ENCODE_FB_HTMLCREF    (ENCODE_HTMLCREF|ENCODE_LEAVE_SRC)
#define  ENCODE_FB_XMLCREF     (ENCODE_XMLCREF|ENCODE_LEAVE_SRC)

#define encode_ckWARN(c, w) ((c & ENCODE_WARN_ON_ERR)                         \
                        && (!(c & ENCODE_ONLY_PRAGMA_WARNINGS) || ckWARN(w)))

#ifdef UTF8SKIP
#  ifdef EBCDIC   /* The value on early perls is wrong */
#    undef UTF8_MAXBYTES 
#    define UTF8_MAXBYTES 14
#  endif
#  ifndef UNLIKELY
#    define UNLIKELY(x) (x)
#  endif
#  ifndef LIKELY
#    define LIKELY(x) (x)
#  endif

/* EBCDIC requires a later perl to work, so the next two definitions are for
 * ASCII machines only */
#  ifndef NATIVE_UTF8_TO_I8
#    define NATIVE_UTF8_TO_I8(x) (x)
#  endif
#  ifndef I8_TO_NATIVE_UTF8
#    define I8_TO_NATIVE_UTF8(x)  (x)
#  endif
#  ifndef OFFUNISKIP
#    define OFFUNISKIP(x)  UNISKIP(x)
#  endif
#  ifndef uvoffuni_to_utf8_flags
#    define uvoffuni_to_utf8_flags(a,b,c) uvuni_to_utf8_flags(a,b,c)
#  endif
#  ifndef WARN_SURROGATE    /* Use the overarching category if these
                               subcategories are missing */
#    define WARN_SURROGATE WARN_UTF8
#    define WARN_NONCHAR WARN_UTF8
#    define WARN_NON_UNICODE WARN_UTF8
     /* If there's only one possible category, then packing is a no-op */
#    define encode_ckWARN_packed(c, w) encode_ckWARN(c, w)
#  else
#    define encode_ckWARN_packed(c, w)                                      \
            ((c & ENCODE_WARN_ON_ERR)                                       \
        && (!(c & ENCODE_ONLY_PRAGMA_WARNINGS) || Perl_ckwarn(aTHX_ w)))
#  endif

/* All these formats take a single UV code point argument */
static const char surrogate_cp_format[] = "UTF-16 surrogate U+%04" UVXf;
static const char nonchar_cp_format[]   = "Unicode non-character U+%04" UVXf
                                   " is not recommended for open interchange";
static const char super_cp_format[]     = "Code point 0x%" UVXf " is not Unicode,"
                                   " may not be portable";

/* If the perl doesn't have the 5.28 functions, this file includes
 * stripped-down versions of them but containing enough functionality to be
 * suitable for Encode's needs.  Many of the comments have been removed.  But
 * you can inspect the 5.28 source if you get stuck.
 *
 * These could be put in Devel::PPPort, but Encode is likely the only user */

#if    (defined(IN_ENCODE_XS) || defined(IN_UNICODE_XS))                     \
  && (! defined(utf8n_to_uvchr_msgs) && ! defined(uvchr_to_utf8_flags_msgs))

#  ifndef hv_stores
#    define hv_stores(hv, key, val) hv_store((hv), ("" key ""), (sizeof(key)-1), (val), 0)
#  endif

static HV *
S_new_msg_hv(const char * const message, /* The message text */
                   U32 categories)  /* Packed warning categories */
{
    /* Creates, populates, and returns an HV* that describes an error message
     * for the translators between UTF8 and code point */

    dTHX;
    SV* msg_sv = newSVpv(message, 0);
    SV* category_sv = newSVuv(categories);

    HV* msg_hv = newHV();

    (void) hv_stores(msg_hv, "text", msg_sv);
    (void) hv_stores(msg_hv, "warn_categories",  category_sv);

    return msg_hv;
}

#endif

#if ! defined(utf8n_to_uvchr_msgs)                      \
  && (defined(IN_ENCODE_XS) || defined(IN_UNICODE_XS))

#  undef utf8n_to_uvchr     /* Don't use an earlier version: use the version
                               defined in this file */
#  define utf8n_to_uvchr(a,b,c,d) utf8n_to_uvchr_msgs(a, b, c, d, 0, NULL)

#  undef UTF8_IS_START      /* Early perls wrongly accepted C0 and C1 */
#  define UTF8_IS_START(c)  (((U8)(c)) >= 0xc2)
#  ifndef isUTF8_POSSIBLY_PROBLEMATIC
#    ifdef EBCDIC
#      define isUTF8_POSSIBLY_PROBLEMATIC(c) ((U8) c > ' ')
#    else
#      define isUTF8_POSSIBLY_PROBLEMATIC(c) ((U8) c >= 0xED)
#    endif
#  endif
#  ifndef UTF8_ALLOW_OVERFLOW
#    define UTF8_ALLOW_OVERFLOW (1U<<31)    /* Choose highest bit to avoid
                                               potential conflicts */
#    define UTF8_GOT_OVERFLOW           UTF8_ALLOW_OVERFLOW
#  endif
#  undef UTF8_ALLOW_ANY     /* Early perl definitions don't work properly with
                               the code in this file */
#  define UTF8_ALLOW_ANY ( UTF8_ALLOW_CONTINUATION                              \
                          |UTF8_ALLOW_NON_CONTINUATION                          \
                          |UTF8_ALLOW_SHORT                                     \
                          |UTF8_ALLOW_LONG                                      \
                          |UTF8_ALLOW_OVERFLOW)

/* The meanings of these were complemented at some point, but the functions
 * bundled in this file use the complemented meanings */
#  ifndef UTF8_DISALLOW_SURROGATE
#    define UTF8_DISALLOW_SURROGATE     UTF8_ALLOW_SURROGATE
#    define UTF8_DISALLOW_NONCHAR       UTF8_ALLOW_FFFF
#    define UTF8_DISALLOW_SUPER         UTF8_ALLOW_FE_FF

     /* In the stripped-down implementation in this file, disallowing is not
      * independent of warning */
#    define UTF8_WARN_SURROGATE     UTF8_DISALLOW_SURROGATE
#    define UTF8_WARN_NONCHAR       UTF8_DISALLOW_NONCHAR
#    define UTF8_WARN_SUPER         UTF8_DISALLOW_SUPER
#  endif
#  ifndef UTF8_DISALLOW_ILLEGAL_INTERCHANGE
#    define UTF8_DISALLOW_ILLEGAL_INTERCHANGE                                   \
     (UTF8_DISALLOW_SUPER|UTF8_DISALLOW_SURROGATE|UTF8_DISALLOW_NONCHAR)
#  endif
#  ifndef UTF8_WARN_ILLEGAL_INTERCHANGE
#    define UTF8_WARN_ILLEGAL_INTERCHANGE                                       \
         (UTF8_WARN_SUPER|UTF8_WARN_SURROGATE|UTF8_WARN_NONCHAR)
#  endif
#  ifndef FIRST_START_BYTE_THAT_IS_DEFINITELY_SUPER
#    ifdef EBCDIC   /* On EBCDIC, these are actually I8 bytes */
#      define FIRST_START_BYTE_THAT_IS_DEFINITELY_SUPER  0xFA
#      define IS_UTF8_2_BYTE_SUPER(s0, s1) ((s0) == 0xF9 && (s1) >= 0xA2)

#      define IS_UTF8_2_BYTE_SURROGATE(s0, s1)     ((s0) == 0xF1            \
                                              && ((s1) & 0xFE ) == 0xB6)
#    else
#      define FIRST_START_BYTE_THAT_IS_DEFINITELY_SUPER  0xF5
#      define IS_UTF8_2_BYTE_SUPER(s0, s1)       ((s0) == 0xF4 && (s1) >= 0x90)
#      define IS_UTF8_2_BYTE_SURROGATE(s0, s1)   ((s0) == 0xED && (s1) >= 0xA0)
#    endif
#    ifndef HIGHEST_REPRESENTABLE_UTF8
#      if defined(UV_IS_QUAD) /* These assume IV_MAX is 2**63-1 */
#        ifdef EBCDIC     /* Actually is I8 */
#          define HIGHEST_REPRESENTABLE_UTF8                                    \
                   "\xFF\xA7\xBF\xBF\xBF\xBF\xBF\xBF\xBF\xBF\xBF\xBF\xBF\xBF"
#        else
#          define HIGHEST_REPRESENTABLE_UTF8                                    \
                   "\xFF\x80\x87\xBF\xBF\xBF\xBF\xBF\xBF\xBF\xBF\xBF\xBF"
#        endif
#      endif
#    endif
#  endif

#  ifndef Newx
#    define Newx(v,n,t) New(0,v,n,t)
#  endif

#  ifndef PERL_UNUSED_ARG
#    define PERL_UNUSED_ARG(x) ((void)x)
#  endif

#  ifndef memGT
#    define memGT(s1,s2,l) (memcmp(s1,s2,l) > 0)
#  endif

#  ifndef MIN
#    define MIN(a,b) ((a) < (b) ? (a) : (b))
#  endif

static const char malformed_text[] = "Malformed UTF-8 character";

static char *
_byte_dump_string(const U8 * const start, const STRLEN len)
{
    /* Returns a mortalized C string that is a displayable copy of the 'len' */

    const STRLEN output_len = 4 * len + 1;  /* 4 bytes per each input, plus a
                                               trailing NUL */
    const U8 * s = start;
    const U8 * const e = start + len;
    char * output;
    char * d;
    dTHX;

    Newx(output, output_len, char);
    SAVEFREEPV(output);

    d = output;
    for (s = start; s < e; s++) {
        const unsigned high_nibble = (*s & 0xF0) >> 4;
        const unsigned low_nibble =  (*s & 0x0F);

        *d++ = '\\';
        *d++ = 'x';

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

static char *
S_unexpected_non_continuation_text(const U8 * const s,

                                         /* Max number of bytes to print */
                                         STRLEN print_len,

                                         /* Which one is the non-continuation */
                                         const STRLEN non_cont_byte_pos,

                                         /* How many bytes should there be? */
                                         const STRLEN expect_len)
{
    /* Return the malformation warning text for an unexpected continuation
     * byte. */

    dTHX;
    const char * const where = (non_cont_byte_pos == 1)
                               ? "immediately"
                               : Perl_form(aTHX_ "%d bytes",
                                                 (int) non_cont_byte_pos);
    const U8 * x = s + non_cont_byte_pos;
    const U8 * e = s + print_len;

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
                           _byte_dump_string(s, x - s),
                           *(s + non_cont_byte_pos),
                           where,
                           *s,
                           (int) expect_len,
                           (int) non_cont_byte_pos);
}

static int
S_is_utf8_overlong_given_start_byte_ok(const U8 * const s, const STRLEN len);

static int
S_does_utf8_overflow(const U8 * const s,
                       const U8 * e,
                       const bool consider_overlongs)
{
    /* Returns an int indicating whether or not the UTF-8 sequence from 's' to
     * 'e' - 1 would overflow an IV on this platform. */

#  if ! defined(UV_IS_QUAD)

    const STRLEN len = e - s;
    int is_overlong;

    assert(s <= e && s + UTF8SKIP(s) >= e);
    assert(! UTF8_IS_INVARIANT(*s) && e > s);

#    ifdef EBCDIC

    PERL_UNUSED_ARG(consider_overlongs);

    if (*s != 0xFE) {
        return 0;
    }

    if (len == 1) {
        return -1;
    }

#    else

    if (LIKELY(*s < 0xFE)) {
        return 0;
    }

    if (! consider_overlongs) {
        return 1;
    }

    if (len == 1) {
        return -1;
    }

    is_overlong = S_is_utf8_overlong_given_start_byte_ok(s, len);

    if (is_overlong == 0) {
        return 1;
    }

    if (is_overlong < 0) {
        return -1;
    }

    if (*s == 0xFE) {
        return 0;
    }

#    endif

    /* Here, ASCII and EBCDIC rejoin:
    *  On ASCII:   We have an overlong sequence starting with FF
    *  On EBCDIC:  We have a sequence starting with FE. */

    {   /* For C89, use a block so the declaration can be close to its use */

#    ifdef EBCDIC
        const U8 conts_for_highest_30_bit[] = "\x41\x41\x41\x41\x41\x41\x42";
#    else
        const U8 conts_for_highest_30_bit[] = "\x80\x80\x80\x80\x80\x80\x81";
#    endif
        const STRLEN conts_len = sizeof(conts_for_highest_30_bit) - 1;
        const STRLEN cmp_len = MIN(conts_len, len - 1);

        if (cmp_len >= conts_len || memNE(s + 1,
                                          conts_for_highest_30_bit,
                                          cmp_len))
        {
            return memGT(s + 1, conts_for_highest_30_bit, cmp_len);
        }

        return -1;
    }

#  else /* Below is 64-bit word */

    PERL_UNUSED_ARG(consider_overlongs);

    {
        const STRLEN len = e - s;
        const U8 *x;
        const U8 * y = (const U8 *) HIGHEST_REPRESENTABLE_UTF8;

        for (x = s; x < e; x++, y++) {

            if (UNLIKELY(NATIVE_UTF8_TO_I8(*x) == *y)) {
                continue;
            }
            return NATIVE_UTF8_TO_I8(*x) > *y;
        }

        if (len < sizeof(HIGHEST_REPRESENTABLE_UTF8) - 1) {
            return -1;
        }

        return 0;
    }

#  endif

}

static int
S_isFF_OVERLONG(const U8 * const s, const STRLEN len);

static int
S_is_utf8_overlong_given_start_byte_ok(const U8 * const s, const STRLEN len)
{
    const U8 s0 = NATIVE_UTF8_TO_I8(s[0]);
    const U8 s1 = NATIVE_UTF8_TO_I8(s[1]);

    assert(len > 1 && UTF8_IS_START(*s));

#         ifdef EBCDIC
#             define F0_ABOVE_OVERLONG 0xB0
#             define F8_ABOVE_OVERLONG 0xA8
#             define FC_ABOVE_OVERLONG 0xA4
#             define FE_ABOVE_OVERLONG 0xA2
#             define FF_OVERLONG_PREFIX "\xfe\x41\x41\x41\x41\x41\x41\x41"
#         else

    if (s0 == 0xE0 && UNLIKELY(s1 < 0xA0)) {
        return 1;
    }

#             define F0_ABOVE_OVERLONG 0x90
#             define F8_ABOVE_OVERLONG 0x88
#             define FC_ABOVE_OVERLONG 0x84
#             define FE_ABOVE_OVERLONG 0x82
#             define FF_OVERLONG_PREFIX "\xff\x80\x80\x80\x80\x80\x80"
#         endif

    if (   (s0 == 0xF0 && UNLIKELY(s1 < F0_ABOVE_OVERLONG))
        || (s0 == 0xF8 && UNLIKELY(s1 < F8_ABOVE_OVERLONG))
        || (s0 == 0xFC && UNLIKELY(s1 < FC_ABOVE_OVERLONG))
        || (s0 == 0xFE && UNLIKELY(s1 < FE_ABOVE_OVERLONG)))
    {
        return 1;
    }

    /* Check for the FF overlong */
    return S_isFF_OVERLONG(s, len);
}

int
S_isFF_OVERLONG(const U8 * const s, const STRLEN len)
{
    if (LIKELY(memNE(s, FF_OVERLONG_PREFIX,
                     MIN(len, sizeof(FF_OVERLONG_PREFIX) - 1))))
    {
        return 0;
    }

    if (len >= sizeof(FF_OVERLONG_PREFIX) - 1) {
        return 1;
    }

    return -1;
}

#  ifndef UTF8_GOT_CONTINUATION
#    define UTF8_GOT_CONTINUATION       UTF8_ALLOW_CONTINUATION
#    define UTF8_GOT_EMPTY              UTF8_ALLOW_EMPTY
#    define UTF8_GOT_LONG               UTF8_ALLOW_LONG
#    define UTF8_GOT_NON_CONTINUATION   UTF8_ALLOW_NON_CONTINUATION
#    define UTF8_GOT_SHORT              UTF8_ALLOW_SHORT
#    define UTF8_GOT_SURROGATE          UTF8_DISALLOW_SURROGATE
#    define UTF8_GOT_NONCHAR            UTF8_DISALLOW_NONCHAR
#    define UTF8_GOT_SUPER              UTF8_DISALLOW_SUPER
#  endif

#  ifndef UNICODE_IS_SUPER
#    define UNICODE_IS_SUPER(uv)    ((UV) (uv) > PERL_UNICODE_MAX)
#  endif
#  ifndef UNICODE_IS_32_CONTIGUOUS_NONCHARS
#    define UNICODE_IS_32_CONTIGUOUS_NONCHARS(uv)      ((UV) (uv) >= 0xFDD0   \
                                                   && (UV) (uv) <= 0xFDEF)
#  endif
#  ifndef UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER
#    define UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER(uv)                  \
                                          (((UV) (uv) & 0xFFFE) == 0xFFFE)
#  endif
#  ifndef is_NONCHAR_utf8_safe
#    define is_NONCHAR_utf8_safe(s,e)     /*** GENERATED CODE ***/            \
( ( ( LIKELY((e) > (s)) ) && ( LIKELY(((e) - (s)) >= UTF8SKIP(s)) ) ) ? ( ( 0xEF == ((const U8*)s)[0] ) ?\
	    ( ( 0xB7 == ((const U8*)s)[1] ) ?                               \
		( ( 0x90 <= ((const U8*)s)[2] && ((const U8*)s)[2] <= 0xAF ) ? 3 : 0 )\
	    : ( ( 0xBF == ((const U8*)s)[1] ) && ( ( ((const U8*)s)[2] & 0xFE ) == 0xBE ) ) ? 3 : 0 )\
	: ( 0xF0 == ((const U8*)s)[0] ) ?                                   \
	    ( ( ( ( ((const U8*)s)[1] == 0x9F || ( ( ((const U8*)s)[1] & 0xEF ) == 0xAF ) ) && ( 0xBF == ((const U8*)s)[2] ) ) && ( ( ((const U8*)s)[3] & 0xFE ) == 0xBE ) ) ? 4 : 0 )\
	: ( 0xF1 <= ((const U8*)s)[0] && ((const U8*)s)[0] <= 0xF3 ) ?      \
	    ( ( ( ( ( ((const U8*)s)[1] & 0xCF ) == 0x8F ) && ( 0xBF == ((const U8*)s)[2] ) ) && ( ( ((const U8*)s)[3] & 0xFE ) == 0xBE ) ) ? 4 : 0 )\
	: ( ( ( ( 0xF4 == ((const U8*)s)[0] ) && ( 0x8F == ((const U8*)s)[1] ) ) && ( 0xBF == ((const U8*)s)[2] ) ) && ( ( ((const U8*)s)[3] & 0xFE ) == 0xBE ) ) ? 4 : 0 ) : 0 )
#  endif

#  ifndef UTF8_IS_NONCHAR
#    define UTF8_IS_NONCHAR(s, e) (is_NONCHAR_utf8_safe(s,e) > 0)
#  endif
#  ifndef UNICODE_IS_NONCHAR
#    define UNICODE_IS_NONCHAR(uv)                                    \
    (   UNICODE_IS_32_CONTIGUOUS_NONCHARS(uv)                       \
     || (   LIKELY( ! UNICODE_IS_SUPER(uv))                         \
         && UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER(uv)))
#  endif

#  ifndef UTF8_MAXBYTES
#    define UTF8_MAXBYTES UTF8_MAXLEN
#  endif

static UV
utf8n_to_uvchr_msgs(const U8 *s,
                    STRLEN curlen,
                    STRLEN *retlen,
                    const U32 flags,
                    U32 * errors,
                    AV ** msgs)
{
    const U8 * const s0 = s;
    const U8 * send = NULL;
    U32 possible_problems = 0;
    UV uv = *s;
    STRLEN expectlen   = 0;
    U8 * adjusted_s0 = (U8 *) s0;
    U8 temp_char_buf[UTF8_MAXBYTES + 1];
    UV uv_so_far = 0;
    dTHX;

    assert(errors == NULL); /* This functionality has been stripped */

    if (UNLIKELY(curlen == 0)) {
        possible_problems |= UTF8_GOT_EMPTY;
        curlen = 0;
        uv = UNICODE_REPLACEMENT;
	goto ready_to_handle_errors;
    }

    expectlen = UTF8SKIP(s);

    if (retlen) {
	*retlen = expectlen;
    }

    if (UTF8_IS_INVARIANT(uv)) {
	return uv;
    }

    if (UNLIKELY(UTF8_IS_CONTINUATION(uv))) {
	possible_problems |= UTF8_GOT_CONTINUATION;
        curlen = 1;
        uv = UNICODE_REPLACEMENT;
	goto ready_to_handle_errors;
    }

    uv = NATIVE_UTF8_TO_I8(uv) & UTF_START_MASK(expectlen);

    send = (U8*) s0;
    if (UNLIKELY(curlen < expectlen)) {
        possible_problems |= UTF8_GOT_SHORT;
        send += curlen;
    }
    else {
        send += expectlen;
    }

    for (s = s0 + 1; s < send; s++) {
	if (LIKELY(UTF8_IS_CONTINUATION(*s))) {
	    uv = UTF8_ACCUMULATE(uv, *s);
            continue;
        }

        possible_problems |= UTF8_GOT_NON_CONTINUATION;
        break;
    } /* End of loop through the character's bytes */

    curlen = s - s0;

#     define UTF8_GOT_TOO_SHORT (UTF8_GOT_SHORT|UTF8_GOT_NON_CONTINUATION)

    if (UNLIKELY(possible_problems & UTF8_GOT_TOO_SHORT)) {
        uv_so_far = uv;
        uv = UNICODE_REPLACEMENT;
    }

    if (UNLIKELY(0 < S_does_utf8_overflow(s0, s, 1))) {
        possible_problems |= UTF8_GOT_OVERFLOW;
        uv = UNICODE_REPLACEMENT;
    }

    if (     (   LIKELY(! possible_problems)
              && UNLIKELY(expectlen > (STRLEN) OFFUNISKIP(uv)))
        || (       UNLIKELY(possible_problems)
            && (   UNLIKELY(! UTF8_IS_START(*s0))
                || (   curlen > 1
                    && UNLIKELY(0 < S_is_utf8_overlong_given_start_byte_ok(s0,
                                                                s - s0))))))
    {
        possible_problems |= UTF8_GOT_LONG;

        if (   UNLIKELY(   possible_problems & UTF8_GOT_TOO_SHORT)
            &&   LIKELY(! (possible_problems & UTF8_GOT_OVERFLOW)))
        {
            UV min_uv = uv_so_far;
            STRLEN i;

            for (i = curlen; i < expectlen; i++) {
                min_uv = UTF8_ACCUMULATE(min_uv,
                                     I8_TO_NATIVE_UTF8(UTF_CONTINUATION_MARK));
            }

            adjusted_s0 = temp_char_buf;
            (void) uvoffuni_to_utf8_flags(adjusted_s0, min_uv, 0);
        }
    }

    /* Here, we have found all the possible problems, except for when the input
     * is for a problematic code point not allowed by the input parameters. */

                                /* uv is valid for overlongs */
    if (   (   (      LIKELY(! (possible_problems & ~UTF8_GOT_LONG))
                   && uv >= UNICODE_SURROGATE_FIRST)
            || (   UNLIKELY(possible_problems)
                && isUTF8_POSSIBLY_PROBLEMATIC(*adjusted_s0)))
	&& ((flags & ( UTF8_DISALLOW_NONCHAR
                      |UTF8_DISALLOW_SURROGATE
                      |UTF8_DISALLOW_SUPER))))
    {
        if (LIKELY(! (possible_problems & ~UTF8_GOT_LONG))) {
            if (UNLIKELY(UNICODE_IS_SURROGATE(uv))) {
                possible_problems |= UTF8_GOT_SURROGATE;
            }
            else if (UNLIKELY(uv > PERL_UNICODE_MAX)) {
                possible_problems |= UTF8_GOT_SUPER;
            }
            else if (UNLIKELY(UNICODE_IS_NONCHAR(uv))) {
                possible_problems |= UTF8_GOT_NONCHAR;
            }
        }
        else {
            if (UNLIKELY(NATIVE_UTF8_TO_I8(*adjusted_s0)
                                >= FIRST_START_BYTE_THAT_IS_DEFINITELY_SUPER))
            {
                possible_problems |= UTF8_GOT_SUPER;
            }
            else if (curlen > 1) {
                if (UNLIKELY(IS_UTF8_2_BYTE_SUPER(
                                      NATIVE_UTF8_TO_I8(*adjusted_s0),
                                      NATIVE_UTF8_TO_I8(*(adjusted_s0 + 1)))))
                {
                    possible_problems |= UTF8_GOT_SUPER;
                }
                else if (UNLIKELY(IS_UTF8_2_BYTE_SURROGATE(
                                      NATIVE_UTF8_TO_I8(*adjusted_s0),
                                      NATIVE_UTF8_TO_I8(*(adjusted_s0 + 1)))))
                {
                    possible_problems |= UTF8_GOT_SURROGATE;
                }
            }
        }
    }

  ready_to_handle_errors:

    if (UNLIKELY(possible_problems)) {
        bool disallowed = FALSE;
        const U32 orig_problems = possible_problems;

        if (msgs) {
            *msgs = NULL;
        }

        while (possible_problems) { /* Handle each possible problem */
            UV pack_warn = 0;
            char * message = NULL;
            U32 this_flag_bit = 0;

            /* Each 'if' clause handles one problem.  They are ordered so that
             * the first ones' messages will be displayed before the later
             * ones; this is kinda in decreasing severity order.  But the
             * overlong must come last, as it changes 'uv' looked at by the
             * others */
            if (possible_problems & UTF8_GOT_OVERFLOW) {

                /* Overflow means also got a super; we handle both here */
                possible_problems
                  &= ~(UTF8_GOT_OVERFLOW|UTF8_GOT_SUPER);

                /* Disallow if any of the categories say to */
                if ( ! (flags &  UTF8_ALLOW_OVERFLOW)
                    || (flags &  UTF8_DISALLOW_SUPER))
                {
                    disallowed = TRUE;
                }

                /* Likewise, warn if any say to */
                if (  ! (flags & UTF8_ALLOW_OVERFLOW)) {

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
                                            _byte_dump_string(s0, curlen));
                            this_flag_bit = UTF8_GOT_OVERFLOW;
                        }
                    }
                }
            }
            else if (possible_problems & UTF8_GOT_EMPTY) {
                possible_problems &= ~UTF8_GOT_EMPTY;

                if (! (flags & UTF8_ALLOW_EMPTY)) {
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
                                _byte_dump_string(s0, 1), *s0);
                        this_flag_bit = UTF8_GOT_CONTINUATION;
                    }
                }
            }
            else if (possible_problems & UTF8_GOT_SHORT) {
                possible_problems &= ~UTF8_GOT_SHORT;

                if (! (flags & UTF8_ALLOW_SHORT)) {
                    disallowed = TRUE;
                    if ((   msgs
                         || ckWARN_d(WARN_UTF8)) && ! (flags & UTF8_CHECK_ONLY))
                    {
                        pack_warn = packWARN(WARN_UTF8);
                        message = Perl_form(aTHX_
                             "%s: %s (too short; %d byte%s available, need %d)",
                             malformed_text,
                             _byte_dump_string(s0, send - s0),
                             (int)curlen,
                             curlen == 1 ? "" : "s",
                             (int)expectlen);
                        this_flag_bit = UTF8_GOT_SHORT;
                    }
                }

            }
            else if (possible_problems & UTF8_GOT_NON_CONTINUATION) {
                possible_problems &= ~UTF8_GOT_NON_CONTINUATION;

                if (! (flags & UTF8_ALLOW_NON_CONTINUATION)) {
                    disallowed = TRUE;
                    if ((   msgs
                         || ckWARN_d(WARN_UTF8)) && ! (flags & UTF8_CHECK_ONLY))
                    {
                        int printlen = s - s0;
                        pack_warn = packWARN(WARN_UTF8);
                        message = Perl_form(aTHX_ "%s",
                            S_unexpected_non_continuation_text(s0,
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
                                    _byte_dump_string(s0, curlen));
                        }
                        else {
                            message = Perl_form(aTHX_ surrogate_cp_format, uv);
                        }
                        this_flag_bit = UTF8_GOT_SURROGATE;
                    }
                }

                if (flags & UTF8_DISALLOW_SURROGATE) {
                    disallowed = TRUE;
                }
            }
            else if (possible_problems & UTF8_GOT_SUPER) {
                possible_problems &= ~UTF8_GOT_SUPER;

                if (flags & UTF8_WARN_SUPER) {

                    if (   ! (flags & UTF8_CHECK_ONLY)
                        && (msgs || ckWARN_d(WARN_NON_UNICODE)))
                    {
                        pack_warn = packWARN(WARN_NON_UNICODE);

                        if (orig_problems & UTF8_GOT_TOO_SHORT) {
                            message = Perl_form(aTHX_
                                    "Any UTF-8 sequence that starts with"
                                    " \"%s\" is for a non-Unicode code point,"
                                    " may not be portable",
                                    _byte_dump_string(s0, curlen));
                        }
                        else {
                            message = Perl_form(aTHX_ super_cp_format, uv);
                        }
                        this_flag_bit = UTF8_GOT_SUPER;
                    }
                }

                if (flags & UTF8_DISALLOW_SUPER) {
                    disallowed = TRUE;
                }
            }
            else if (possible_problems & UTF8_GOT_NONCHAR) {
                possible_problems &= ~UTF8_GOT_NONCHAR;

                if (flags & UTF8_WARN_NONCHAR) {

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
                }
            }
            else if (possible_problems & UTF8_GOT_LONG) {
                possible_problems &= ~UTF8_GOT_LONG;

                if (flags & UTF8_ALLOW_LONG) {
                    uv = UNICODE_REPLACEMENT;
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
                                    _byte_dump_string(s0, send - s0),
                                    _byte_dump_string(s0, curlen));
                        }
                        else {
                            U8 tmpbuf[UTF8_MAXBYTES+1];
                            const U8 * const e = uvoffuni_to_utf8_flags(tmpbuf,
                                                                        uv, 0);
                            /* Don't use U+ for non-Unicode code points, which
                             * includes those in the Latin1 range */
                            const char * preface = (    uv > PERL_UNICODE_MAX
#  ifdef EBCDIC
                                                     || uv <= 0xFF
#  endif
                                                    )
                                                   ? "0x"
                                                   : "U+";
                            message = Perl_form(aTHX_
                                "%s: %s (overlong; instead use %s to represent"
                                " %s%0*" UVXf ")",
                                malformed_text,
                                _byte_dump_string(s0, send - s0),
                                _byte_dump_string(tmpbuf, e - tmpbuf),
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

                    av_push(*msgs, newRV_noinc((SV*) S_new_msg_hv(message,
                                                                pack_warn)));
                }
                else if (PL_op)
                    Perl_warner(aTHX_ pack_warn, "%s in %s", message,
                                                 OP_DESC(PL_op));
                else
                    Perl_warner(aTHX_ pack_warn, "%s", message);
            }
        }   /* End of 'while (possible_problems)' */

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

static STRLEN
S_is_utf8_char_helper(const U8 * const s, const U8 * e, const U32 flags)
{
    STRLEN len;
    const U8 *x;

    assert(0 == (flags & ~UTF8_DISALLOW_ILLEGAL_INTERCHANGE));
    assert(! UTF8_IS_INVARIANT(*s));

    if (UNLIKELY(! UTF8_IS_START(*s))) {
        return 0;
    }

    /* Examine a maximum of a single whole code point */
    if (e - s > UTF8SKIP(s)) {
        e = s + UTF8SKIP(s);
    }

    len = e - s;

    if (flags && isUTF8_POSSIBLY_PROBLEMATIC(*s)) {
        const U8 s0 = NATIVE_UTF8_TO_I8(s[0]);

        if (  (flags & UTF8_DISALLOW_SUPER)
            && UNLIKELY(s0 >= FIRST_START_BYTE_THAT_IS_DEFINITELY_SUPER))
        {
            return 0;           /* Above Unicode */
        }

        if (len > 1) {
            const U8 s1 = NATIVE_UTF8_TO_I8(s[1]);

            if (   (flags & UTF8_DISALLOW_SUPER)
                &&  UNLIKELY(IS_UTF8_2_BYTE_SUPER(s0, s1)))
            {
                return 0;       /* Above Unicode */
            }

            if (   (flags & UTF8_DISALLOW_SURROGATE)
                &&  UNLIKELY(IS_UTF8_2_BYTE_SURROGATE(s0, s1)))
            {
                return 0;       /* Surrogate */
            }

            if (  (flags & UTF8_DISALLOW_NONCHAR)
                && UNLIKELY(UTF8_IS_NONCHAR(s, e)))
            {
                return 0;       /* Noncharacter code point */
            }
        }
    }

    for (x = s + 1; x < e; x++) {
        if (UNLIKELY(! UTF8_IS_CONTINUATION(*x))) {
            return 0;
        }
    }

    if (len > 1 && S_is_utf8_overlong_given_start_byte_ok(s, len) > 0) {
        return 0;
    }

    if (0 < S_does_utf8_overflow(s, e, 0)) {
        return 0;
    }

    return UTF8SKIP(s);
}

#  undef is_utf8_valid_partial_char_flags

static bool
is_utf8_valid_partial_char_flags(const U8 * const s, const U8 * const e, const U32 flags)
{

    return S_is_utf8_char_helper(s, e, flags) > 0;
}

#  undef is_utf8_string_loc_flags

static bool
is_utf8_string_loc_flags(const U8 *s, STRLEN len, const U8 **ep, const U32 flags)
{
    const U8* send = s + len;

    assert(0 == (flags & ~UTF8_DISALLOW_ILLEGAL_INTERCHANGE));

    while (s < send) {
        if (UTF8_IS_INVARIANT(*s)) {
            s++;
        }
        else if (     UNLIKELY(send - s < UTF8SKIP(s))
                 || ! S_is_utf8_char_helper(s, send, flags))
        {
            *ep = s;
            return 0;
        }
        else {
            s += UTF8SKIP(s);
        }
    }

    *ep = send;

    return 1;
}

#endif

#if defined(IN_UNICODE_XS) && ! defined(uvchr_to_utf8_flags_msgs)

#  define MY_SHIFT   UTF_ACCUMULATION_SHIFT
#  define MY_MARK    UTF_CONTINUATION_MARK
#  define MY_MASK    UTF_CONTINUATION_MASK

static const char cp_above_legal_max[] =
                        "Use of code point 0x%" UVXf " is not allowed; the"
                        " permissible max is 0x%" UVXf;

/* These two can be dummys, as they are not looked at by the function, which
 * has hard-coded into it what flags it is expecting are */
#  ifndef UNICODE_DISALLOW_ILLEGAL_INTERCHANGE
#    define UNICODE_DISALLOW_ILLEGAL_INTERCHANGE 0
#  endif
#  ifndef UNICODE_WARN_ILLEGAL_INTERCHANGE
#    define UNICODE_WARN_ILLEGAL_INTERCHANGE 0
#  endif

#  ifndef OFFUNI_IS_INVARIANT
#    define OFFUNI_IS_INVARIANT(cp) UNI_IS_INVARIANT(cp)
#  endif
#  ifndef MAX_EXTERNALLY_LEGAL_CP
#    define MAX_EXTERNALLY_LEGAL_CP ((UV) (IV_MAX))
#  endif
#  ifndef LATIN1_TO_NATIVE
#    define LATIN1_TO_NATIVE(a) ASCII_TO_NATIVE(a)
#  endif
#  ifndef I8_TO_NATIVE_UTF8
#    define I8_TO_NATIVE_UTF8(a) NATIVE_TO_UTF(a)
#  endif
#  ifndef MAX_UTF8_TWO_BYTE
#    define MAX_UTF8_TWO_BYTE (32 * (1U << UTF_ACCUMULATION_SHIFT) - 1)
#  endif
#  ifndef UNICODE_IS_32_CONTIGUOUS_NONCHARS
#    define UNICODE_IS_32_CONTIGUOUS_NONCHARS(uv)    ((UV) (uv) >= 0xFDD0   \
                                                 && (UV) (uv) <= 0xFDEF)
#  endif
#  ifndef UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER
#    define UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER(uv)                \
                                          (((UV) (uv) & 0xFFFE) == 0xFFFE)
#  endif
#  ifndef UNICODE_IS_SUPER
#    define UNICODE_IS_SUPER(uv)    ((UV) (uv) > PERL_UNICODE_MAX)
#  endif
#  ifndef OFFUNISKIP
#    define OFFUNISKIP(cp)    UNISKIP(NATIVE_TO_UNI(cp))
#  endif

#  define HANDLE_UNICODE_SURROGATE(uv, flags, msgs)                 \
    STMT_START {                                                    \
        U32 category = packWARN(WARN_SURROGATE);                    \
        const char * format = surrogate_cp_format;                  \
        *msgs = S_new_msg_hv(Perl_form(aTHX_ format, uv),           \
                                 category);                         \
        return NULL;                                                \
    } STMT_END;

#  define HANDLE_UNICODE_NONCHAR(uv, flags, msgs)                   \
    STMT_START {                                                    \
        U32 category = packWARN(WARN_NONCHAR);                      \
        const char * format = nonchar_cp_format;                    \
        *msgs = S_new_msg_hv(Perl_form(aTHX_ format, uv),           \
                                 category);                         \
        return NULL;                                                \
    } STMT_END;

static U8 *
uvchr_to_utf8_flags_msgs(U8 *d, UV uv, const UV flags, HV** msgs)
{
    dTHX;

    assert(msgs);

    PERL_UNUSED_ARG(flags);

    uv = NATIVE_TO_UNI(uv);

    *msgs = NULL;

    if (OFFUNI_IS_INVARIANT(uv)) {
	*d++ = LATIN1_TO_NATIVE(uv);
	return d;
    }

    if (uv <= MAX_UTF8_TWO_BYTE) {
        *d++ = I8_TO_NATIVE_UTF8(( uv >> MY_SHIFT) | UTF_START_MARK(2));
        *d++ = I8_TO_NATIVE_UTF8(( uv   & MY_MASK) | MY_MARK);
        return d;
    }

    /* Not 2-byte; test for and handle 3-byte result.   In the test immediately
     * below, the 16 is for start bytes E0-EF (which are all the possible ones
     * for 3 byte characters).  The 2 is for 2 continuation bytes; these each
     * contribute MY_SHIFT bits.  This yields 0x4000 on EBCDIC platforms, 0x1_0000
     * on ASCII; so 3 bytes covers the range 0x400-0x3FFF on EBCDIC;
     * 0x800-0xFFFF on ASCII */
    if (uv < (16 * (1U << (2 * MY_SHIFT)))) {
	*d++ = I8_TO_NATIVE_UTF8(( uv >> ((3 - 1) * MY_SHIFT)) | UTF_START_MARK(3));
	*d++ = I8_TO_NATIVE_UTF8(((uv >> ((2 - 1) * MY_SHIFT)) & MY_MASK) | MY_MARK);
	*d++ = I8_TO_NATIVE_UTF8(( uv  /* (1 - 1) */           & MY_MASK) | MY_MARK);

#ifndef EBCDIC  /* These problematic code points are 4 bytes on EBCDIC, so
                   aren't tested here */
        /* The most likely code points in this range are below the surrogates.
         * Do an extra test to quickly exclude those. */
        if (UNLIKELY(uv >= UNICODE_SURROGATE_FIRST)) {
            if (UNLIKELY(   UNICODE_IS_32_CONTIGUOUS_NONCHARS(uv)
                         || UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER(uv)))
            {
                HANDLE_UNICODE_NONCHAR(uv, flags, msgs);
            }
            else if (UNLIKELY(UNICODE_IS_SURROGATE(uv))) {
                HANDLE_UNICODE_SURROGATE(uv, flags, msgs);
            }
        }
#endif
	return d;
    }

    /* Not 3-byte; that means the code point is at least 0x1_0000 on ASCII
     * platforms, and 0x4000 on EBCDIC.  There are problematic cases that can
     * happen starting with 4-byte characters on ASCII platforms.  We unify the
     * code for these with EBCDIC, even though some of them require 5-bytes on
     * those, because khw believes the code saving is worth the very slight
     * performance hit on these high EBCDIC code points. */

    if (UNLIKELY(UNICODE_IS_SUPER(uv))) {
        const char * format = super_cp_format;
        U32 category = packWARN(WARN_NON_UNICODE);
        if (UNLIKELY(uv > MAX_EXTERNALLY_LEGAL_CP)) {
            Perl_croak(aTHX_ cp_above_legal_max, uv, MAX_EXTERNALLY_LEGAL_CP);
        }
        *msgs = S_new_msg_hv(Perl_form(aTHX_ format, uv), category);
        return NULL;
    }
    else if (UNLIKELY(UNICODE_IS_END_PLANE_NONCHAR_GIVEN_NOT_SUPER(uv))) {
        HANDLE_UNICODE_NONCHAR(uv, flags, msgs);
    }

    /* Test for and handle 4-byte result.   In the test immediately below, the
     * 8 is for start bytes F0-F7 (which are all the possible ones for 4 byte
     * characters).  The 3 is for 3 continuation bytes; these each contribute
     * MY_SHIFT bits.  This yields 0x4_0000 on EBCDIC platforms, 0x20_0000 on
     * ASCII, so 4 bytes covers the range 0x4000-0x3_FFFF on EBCDIC;
     * 0x1_0000-0x1F_FFFF on ASCII */
    if (uv < (8 * (1U << (3 * MY_SHIFT)))) {
	*d++ = I8_TO_NATIVE_UTF8(( uv >> ((4 - 1) * MY_SHIFT)) | UTF_START_MARK(4));
	*d++ = I8_TO_NATIVE_UTF8(((uv >> ((3 - 1) * MY_SHIFT)) & MY_MASK) | MY_MARK);
	*d++ = I8_TO_NATIVE_UTF8(((uv >> ((2 - 1) * MY_SHIFT)) & MY_MASK) | MY_MARK);
	*d++ = I8_TO_NATIVE_UTF8(( uv  /* (1 - 1) */           & MY_MASK) | MY_MARK);

#ifdef EBCDIC   /* These were handled on ASCII platforms in the code for 3-byte
                   characters.  The end-plane non-characters for EBCDIC were
                   handled just above */
        if (UNLIKELY(UNICODE_IS_32_CONTIGUOUS_NONCHARS(uv))) {
            HANDLE_UNICODE_NONCHAR(uv, flags, msgs);
        }
        else if (UNLIKELY(UNICODE_IS_SURROGATE(uv))) {
            HANDLE_UNICODE_SURROGATE(uv, flags, msgs);
        }
#endif

	return d;
    }

    /* Not 4-byte; that means the code point is at least 0x20_0000 on ASCII
     * platforms, and 0x4000 on EBCDIC.  At this point we switch to a loop
     * format.  The unrolled version above turns out to not save all that much
     * time, and at these high code points (well above the legal Unicode range
     * on ASCII platforms, and well above anything in common use in EBCDIC),
     * khw believes that less code outweighs slight performance gains. */

    {
	STRLEN len  = OFFUNISKIP(uv);
	U8 *p = d+len-1;
	while (p > d) {
	    *p-- = I8_TO_NATIVE_UTF8((uv & MY_MASK) | MY_MARK);
	    uv >>= MY_SHIFT;
	}
	*p = I8_TO_NATIVE_UTF8((uv & UTF_START_MASK(len)) | UTF_START_MARK(len));
	return d+len;
    }
}

#endif  /* End of defining our own uvchr_to_utf8_flags_msgs() */
#endif  /* End of UTF8SKIP */

#endif /* ENCODE_H */
