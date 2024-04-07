/*    pp_pack.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * He still hopefully carried some of his gear in his pack: a small tinder-box,
 * two small shallow pans, the smaller fitting into the larger; inside them a
 * wooden spoon, a short two-pronged fork and some skewers were stowed; and
 * hidden at the bottom of the pack in a flat wooden box a dwindling treasure,
 * some salt.
 *
 *     [p.653 of _The Lord of the Rings_, IV/iv: "Of Herbs and Stewed Rabbit"]
 */

/* This file contains pp ("push/pop") functions that
 * execute the opcodes that make up a perl program. A typical pp function
 * expects to find its arguments on the stack, and usually pushes its
 * results onto the stack, hence the 'pp' terminology. Each OP structure
 * contains a pointer to the relevant pp_foo() function.
 *
 * This particular file just contains pp_pack() and pp_unpack(). See the
 * other pp*.c files for the rest of the pp_ functions.
 */

#include "EXTERN.h"
#define PERL_IN_PP_PACK_C
#include "perl.h"

/* Types used by pack/unpack */ 
typedef enum {
  e_no_len,     /* no length  */
  e_number,     /* number, [] */
  e_star        /* asterisk   */
} howlen_t;

typedef struct tempsym {
  const char*    patptr;   /* current template char */
  const char*    patend;   /* one after last char   */
  const char*    grpbeg;   /* 1st char of ()-group  */
  const char*    grpend;   /* end of ()-group       */
  I32      code;     /* template code (!<>)   */
  U32      flags;    /* /=4, comma=2, pack=1  */
                     /*   and group modifiers */
  SSize_t  length;   /* length/repeat count   */
  howlen_t howlen;   /* how length is given   */ 
  int      level;    /* () nesting level      */
  STRLEN   strbeg;   /* offset of group start */
  struct tempsym *previous; /* previous group */
} tempsym_t;

#define TEMPSYM_INIT(symptr, p, e, f) \
    STMT_START {	\
        (symptr)->patptr   = (p);	\
        (symptr)->patend   = (e);	\
        (symptr)->grpbeg   = NULL;	\
        (symptr)->grpend   = NULL;	\
        (symptr)->grpend   = NULL;	\
        (symptr)->code     = 0;		\
        (symptr)->length   = 0;		\
        (symptr)->howlen   = e_no_len;	\
        (symptr)->level    = 0;		\
        (symptr)->flags    = (f);	\
        (symptr)->strbeg   = 0;		\
        (symptr)->previous = NULL;	\
   } STMT_END

typedef union {
    NV nv;
    U8 bytes[sizeof(NV)];
} NV_bytes;

#if defined(HAS_LONG_DOUBLE)
typedef union {
    long double ld;
    U8 bytes[sizeof(long double)];
} ld_bytes;
#endif

#ifndef CHAR_BIT
# define CHAR_BIT	8
#endif
/* Maximum number of bytes to which a byte can grow due to upgrade */
#define UTF8_EXPAND	2

/*
 * Offset for integer pack/unpack.
 *
 * On architectures where I16 and I32 aren't really 16 and 32 bits,
 * which for now are all Crays, pack and unpack have to play games.
 */

/*
 * These values are required for portability of pack() output.
 * If they're not right on your machine, then pack() and unpack()
 * wouldn't work right anyway; you'll need to apply the Cray hack.
 * (I'd like to check them with #if, but you can't use sizeof() in
 * the preprocessor.)  --???
 */
/*
    The appropriate SHORTSIZE, INTSIZE, LONGSIZE, and LONGLONGSIZE
    defines are now in config.h.  --Andy Dougherty  April 1998
 */
#define SIZE16 2
#define SIZE32 4

/* CROSSCOMPILE and MULTIARCH are going to affect pp_pack() and pp_unpack().
   --jhi Feb 1999 */

#if U16SIZE <= SIZE16 && U32SIZE <= SIZE32
#  define OFF16(p)     ((char *) (p))
#  define OFF32(p)     ((char *) (p))
#elif BYTEORDER == 0x1234 || BYTEORDER == 0x12345678    /* little-endian */
#  define OFF16(p)	((char*)(p))
#  define OFF32(p)	((char*)(p))
#elif BYTEORDER == 0x4321 || BYTEORDER == 0x87654321  /* big-endian */
#  define OFF16(p)	((char*)(p) + (sizeof(U16) - SIZE16))
#  define OFF32(p)	((char*)(p) + (sizeof(U32) - SIZE32))
#else
#  error "bad cray byte order"
#endif

#define PUSH16(utf8, cur, p, needs_swap)                        \
       PUSH_BYTES(utf8, cur, OFF16(p), SIZE16, needs_swap)
#define PUSH32(utf8, cur, p, needs_swap)                        \
       PUSH_BYTES(utf8, cur, OFF32(p), SIZE32, needs_swap)

#if BYTEORDER == 0x4321 || BYTEORDER == 0x87654321  /* big-endian */
#  define NEEDS_SWAP(d)     (TYPE_ENDIANNESS(d) == TYPE_IS_LITTLE_ENDIAN)
#elif BYTEORDER == 0x1234 || BYTEORDER == 0x12345678  /* little-endian */
#  define NEEDS_SWAP(d)     (TYPE_ENDIANNESS(d) == TYPE_IS_BIG_ENDIAN)
#else
#  error "Unsupported byteorder"
        /* Need to add code here to re-instate mixed endian support.
           NEEDS_SWAP would need to hold a flag indicating which action to
           take, and S_reverse_copy and the code in S_utf8_to_bytes would need
           logic adding to deal with any mixed-endian transformations needed.
        */
#endif

/* Only to be used inside a loop (see the break) */
#define SHIFT_BYTES(utf8, s, strend, buf, len, datumtype, needs_swap)	\
STMT_START {						\
    if (UNLIKELY(utf8)) {                               \
        if (!S_utf8_to_bytes(aTHX_ &s, strend,		\
          (char *) (buf), len, datumtype)) break;	\
    } else {						\
        if (UNLIKELY(needs_swap))                       \
            S_reverse_copy(s, (char *) (buf), len);     \
        else                                            \
            Copy(s, (char *) (buf), len, char);		\
        s += len;					\
    }							\
} STMT_END

#define SHIFT16(utf8, s, strend, p, datumtype, needs_swap)              \
       SHIFT_BYTES(utf8, s, strend, OFF16(p), SIZE16, datumtype, needs_swap)

#define SHIFT32(utf8, s, strend, p, datumtype, needs_swap)              \
       SHIFT_BYTES(utf8, s, strend, OFF32(p), SIZE32, datumtype, needs_swap)

#define SHIFT_VAR(utf8, s, strend, var, datumtype, needs_swap)          \
       SHIFT_BYTES(utf8, s, strend, &(var), sizeof(var), datumtype, needs_swap)

#define PUSH_VAR(utf8, aptr, var, needs_swap)           \
       PUSH_BYTES(utf8, aptr, &(var), sizeof(var), needs_swap)

/* Avoid stack overflow due to pathological templates. 100 should be plenty. */
#define MAX_SUB_TEMPLATE_LEVEL 100

/* flags (note that type modifiers can also be used as flags!) */
#define FLAG_WAS_UTF8	      0x40
#define FLAG_PARSE_UTF8       0x20	/* Parse as utf8 */
#define FLAG_UNPACK_ONLY_ONE  0x10
#define FLAG_DO_UTF8          0x08	/* The underlying string is utf8 */
#define FLAG_SLASH            0x04
#define FLAG_COMMA            0x02
#define FLAG_PACK             0x01

STATIC SV *
S_mul128(pTHX_ SV *sv, U8 m)
{
  STRLEN          len;
  char           *s = SvPV(sv, len);
  char           *t;

  PERL_ARGS_ASSERT_MUL128;

  if (! memBEGINs(s, len, "0000")) {  /* need to grow sv */
    SV * const tmpNew = newSVpvs("0000000000");

    sv_catsv(tmpNew, sv);
    SvREFCNT_dec(sv);		/* free old sv */
    sv = tmpNew;
    s = SvPV(sv, len);
  }
  t = s + len - 1;
  while (!*t)                   /* trailing '\0'? */
    t--;
  while (t > s) {
    const U32 i = ((*t - '0') << 7) + m;
    *(t--) = '0' + (char)(i % 10);
    m = (char)(i / 10);
  }
  return (sv);
}

/* Explosives and implosives. */

#define ISUUCHAR(ch)    inRANGE(NATIVE_TO_LATIN1(ch),               \
                                NATIVE_TO_LATIN1(' '),              \
                                NATIVE_TO_LATIN1('a') - 1)

/* type modifiers */
#define TYPE_IS_SHRIEKING	0x100
#define TYPE_IS_BIG_ENDIAN	0x200
#define TYPE_IS_LITTLE_ENDIAN	0x400
#define TYPE_IS_PACK		0x800
#define TYPE_ENDIANNESS_MASK	(TYPE_IS_BIG_ENDIAN|TYPE_IS_LITTLE_ENDIAN)
#define TYPE_MODIFIERS(t)	((t) & ~0xFF)
#define TYPE_NO_MODIFIERS(t)	((U8) (t))

# define TYPE_ENDIANNESS(t)	((t) & TYPE_ENDIANNESS_MASK)
# define TYPE_NO_ENDIANNESS(t)	((t) & ~TYPE_ENDIANNESS_MASK)

# define ENDIANNESS_ALLOWED_TYPES   "sSiIlLqQjJfFdDpP("

#define PACK_SIZE_CANNOT_CSUM		0x80
#define PACK_SIZE_UNPREDICTABLE		0x40	/* Not a fixed size element */
#define PACK_SIZE_MASK			0x3F

#include "packsizetables.inc"

static void
S_reverse_copy(const char *src, char *dest, STRLEN len)
{
    dest += len;
    while (len--)
        *--dest = *src++;
}

STATIC U8
utf8_to_byte(pTHX_ const char **s, const char *end, I32 datumtype)
{
    STRLEN retlen;
    UV val;

    if (*s >= end) {
        goto croak;
    }
    val = utf8n_to_uvchr((U8 *) *s, end-*s, &retlen,
                         ckWARN(WARN_UTF8) ? 0 : UTF8_ALLOW_ANY);
    if (retlen == (STRLEN) -1)
      croak:
        Perl_croak(aTHX_ "Malformed UTF-8 string in '%c' format in unpack",
                   (int) TYPE_NO_MODIFIERS(datumtype));
    if (val >= 0x100) {
        Perl_ck_warner(aTHX_ packWARN(WARN_UNPACK),
                       "Character in '%c' format wrapped in unpack",
                       (int) TYPE_NO_MODIFIERS(datumtype));
        val = (U8) val;
    }
    *s += retlen;
    return (U8)val;
}

#define SHIFT_BYTE(utf8, s, strend, datumtype) ((utf8) ? \
        utf8_to_byte(aTHX_ &(s), (strend), (datumtype)) : \
        *(U8 *)(s)++)

STATIC bool
S_utf8_to_bytes(pTHX_ const char **s, const char *end, const char *buf, SSize_t buf_len, I32 datumtype)
{
    UV val;
    STRLEN retlen;
    const char *from = *s;
    int bad = 0;
    const U32 flags = ckWARN(WARN_UTF8) ?
        UTF8_CHECK_ONLY : (UTF8_CHECK_ONLY | UTF8_ALLOW_ANY);
    const bool needs_swap = NEEDS_SWAP(datumtype);

    if (UNLIKELY(needs_swap))
        buf += buf_len;

    for (;buf_len > 0; buf_len--) {
        if (from >= end) return FALSE;
        val = utf8n_to_uvchr((U8 *) from, end-from, &retlen, flags);
        if (retlen == (STRLEN) -1) {
            from += UTF8_SAFE_SKIP(from, end);
            bad |= 1;
        } else from += retlen;
        if (val >= 0x100) {
            bad |= 2;
            val = (U8) val;
        }
        if (UNLIKELY(needs_swap))
            *(U8 *)--buf = (U8)val;
        else
            *(U8 *)buf++ = (U8)val;
    }
    /* We have enough characters for the buffer. Did we have problems ? */
    if (bad) {
        if (bad & 1) {
            /* Rewalk the string fragment while warning */
            const char *ptr;
            const U32 flags = ckWARN(WARN_UTF8) ? 0 : UTF8_ALLOW_ANY;
            for (ptr = *s; ptr < from; ptr += UTF8SKIP(ptr)) {
                if (ptr >= end) break;
                utf8n_to_uvchr((U8 *) ptr, end-ptr, &retlen, flags);
            }
            if (from > end) from = end;
        }
        if ((bad & 2))
            Perl_ck_warner(aTHX_ packWARN(datumtype & TYPE_IS_PACK ?
                                       WARN_PACK : WARN_UNPACK),
                           "Character(s) in '%c' format wrapped in %s",
                           (int) TYPE_NO_MODIFIERS(datumtype),
                           datumtype & TYPE_IS_PACK ? "pack" : "unpack");
    }
    *s = from;
    return TRUE;
}

STATIC char *
S_my_bytes_to_utf8(const U8 *start, STRLEN len, char *dest, const bool needs_swap) {
    PERL_ARGS_ASSERT_MY_BYTES_TO_UTF8;

    if (UNLIKELY(needs_swap)) {
        const U8 *p = start + len;
        while (p-- > start) {
            append_utf8_from_native_byte(*p, (U8 **) & dest);
        }
    } else {
        const U8 * const end = start + len;
        while (start < end) {
            append_utf8_from_native_byte(*start, (U8 **) & dest);
            start++;
        }
    }
    return dest;
}

#define PUSH_BYTES(utf8, cur, buf, len, needs_swap)             \
STMT_START {							\
    if (UNLIKELY(utf8))	                                        \
        (cur) = my_bytes_to_utf8((U8 *) buf, len, (cur), needs_swap);       \
    else {							\
        if (UNLIKELY(needs_swap))                               \
            S_reverse_copy((char *)(buf), cur, len);            \
        else                                                    \
            Copy(buf, cur, len, char);				\
        (cur) += (len);						\
    }								\
} STMT_END

#define SAFE_UTF8_EXPAND(var)	\
STMT_START {				\
    if ((var) > SSize_t_MAX / UTF8_EXPAND) \
        Perl_croak(aTHX_ "%s", "Out of memory during pack()"); \
    (var) = (var) * UTF8_EXPAND; \
} STMT_END

#define GROWING2(utf8, cat, start, cur, item_size, item_count)	\
STMT_START {							\
    if (SSize_t_MAX / (item_size) < (item_count))		\
        Perl_croak(aTHX_ "%s", "Out of memory during pack()");	\
    GROWING((utf8), (cat), (start), (cur), (item_size) * (item_count)); \
} STMT_END

#define GROWING(utf8, cat, start, cur, in_len)	\
STMT_START {					\
    STRLEN glen = (in_len);			\
    STRLEN catcur = (STRLEN)((cur) - (start));	\
    if (utf8) SAFE_UTF8_EXPAND(glen);		\
    if (SSize_t_MAX - glen < catcur)		\
        Perl_croak(aTHX_ "%s", "Out of memory during pack()"); \
    if (catcur + glen >= SvLEN(cat)) {	\
        (start) = sv_exp_grow(cat, glen);	\
        (cur) = (start) + SvCUR(cat);		\
    }						\
} STMT_END

#define PUSH_GROWING_BYTES(utf8, cat, start, cur, buf, in_len) \
STMT_START {					\
    const STRLEN glen = (in_len);		\
    STRLEN gl = glen;				\
    if (utf8) SAFE_UTF8_EXPAND(gl);		\
    if ((cur) + gl >= (start) + SvLEN(cat)) {	\
        *cur = '\0';				\
        SvCUR_set((cat), (cur) - (start));	\
        (start) = sv_exp_grow(cat, gl);		\
        (cur) = (start) + SvCUR(cat);		\
    }						\
    PUSH_BYTES(utf8, cur, buf, glen, 0);        \
} STMT_END

#define PUSH_BYTE(utf8, s, byte)		\
STMT_START {					\
    if (utf8) {					\
        const U8 au8 = (byte);			\
        (s) = my_bytes_to_utf8(&au8, 1, (s), 0);\
    } else *(U8 *)(s)++ = (byte);		\
} STMT_END

/* Only to be used inside a loop (see the break) */
#define NEXT_UNI_VAL(val, cur, str, end, utf8_flags)		\
STMT_START {							\
    STRLEN retlen;						\
    if (str >= end) break;					\
    val = utf8n_to_uvchr((U8 *) str, end-str, &retlen, utf8_flags);	\
    if (retlen == (STRLEN) -1) {			        \
        *cur = '\0';						\
        Perl_croak(aTHX_ "Malformed UTF-8 string in pack");	\
    }								\
    str += retlen;						\
} STMT_END

static const char *_action( const tempsym_t* symptr )
{
    return (const char *)(( symptr->flags & FLAG_PACK ) ? "pack" : "unpack");
}

/* Returns the sizeof() struct described by pat */
STATIC SSize_t
S_measure_struct(pTHX_ tempsym_t* symptr)
{
    SSize_t total = 0;

    PERL_ARGS_ASSERT_MEASURE_STRUCT;

    while (next_symbol(symptr)) {
        SSize_t len, size;

        switch (symptr->howlen) {
          case e_star:
            Perl_croak(aTHX_ "Within []-length '*' not allowed in %s",
                        _action( symptr ) );

          default:
            /* e_no_len and e_number */
            len = symptr->length;
            break;
        }

        size = packprops[TYPE_NO_ENDIANNESS(symptr->code)] & PACK_SIZE_MASK;
        if (!size) {
            SSize_t star;
            /* endianness doesn't influence the size of a type */
            switch(TYPE_NO_ENDIANNESS(symptr->code)) {
            default:
                /* diag_listed_as: Invalid type '%s' in %s */
                Perl_croak(aTHX_ "Invalid type '%c' in %s",
                           (int)TYPE_NO_MODIFIERS(symptr->code),
                           _action( symptr ) );
            case '.' | TYPE_IS_SHRIEKING:
            case '@' | TYPE_IS_SHRIEKING:
            case '@':
            case '.':
            case '/':
            case 'U':			/* XXXX Is it correct? */
            case 'w':
            case 'u':
                Perl_croak(aTHX_ "Within []-length '%c' not allowed in %s",
                           (int) TYPE_NO_MODIFIERS(symptr->code),
                           _action( symptr ) );
            case '%':
                size = 0;
                break;
            case '(':
            {
                tempsym_t savsym = *symptr;
                symptr->patptr = savsym.grpbeg;
                symptr->patend = savsym.grpend;
                /* XXXX Theoretically, we need to measure many times at
                   different positions, since the subexpression may contain
                   alignment commands, but be not of aligned length.
                   Need to detect this and croak().  */
                size = measure_struct(symptr);
                *symptr = savsym;
                break;
            }
            case 'X' | TYPE_IS_SHRIEKING:
                /* XXXX Is this useful?  Then need to treat MEASURE_BACKWARDS.
                 */
                if (!len)		/* Avoid division by 0 */
                    len = 1;
                len = total % len;	/* Assumed: the start is aligned. */
                /* FALLTHROUGH */
            case 'X':
                size = -1;
                if (total < len)
                    Perl_croak(aTHX_ "'X' outside of string in %s", _action( symptr ) );
                break;
            case 'x' | TYPE_IS_SHRIEKING:
                if (!len)		/* Avoid division by 0 */
                    len = 1;
                star = total % len;	/* Assumed: the start is aligned. */
                if (star)		/* Other portable ways? */
                    len = len - star;
                else
                    len = 0;
                /* FALLTHROUGH */
            case 'x':
            case 'A':
            case 'Z':
            case 'a':
                size = 1;
                break;
            case 'B':
            case 'b':
                len = (len + 7)/8;
                size = 1;
                break;
            case 'H':
            case 'h':
                len = (len + 1)/2;
                size = 1;
                break;

            case 'P':
                len = 1;
                size = sizeof(char*);
                break;
            }
        }
        total += len * size;
    }
    return total;
}


/* locate matching closing parenthesis or bracket
 * returns char pointer to char after match, or NULL
 */
STATIC const char *
S_group_end(pTHX_ const char *patptr, const char *patend, char ender)
{
    PERL_ARGS_ASSERT_GROUP_END;
    Size_t opened = 0;  /* number of pending opened brackets */

    while (patptr < patend) {
        const char c = *patptr++;

        if (opened == 0 && c == ender)
            return patptr-1;
        else if (c == '#') {
            while (patptr < patend && *patptr != '\n')
                patptr++;
            continue;
        } else if (c == '(' || c == '[')
            ++opened;
        else if (c == ')' || c == ']') {
            if (opened == 0)
                Perl_croak(aTHX_ "Mismatched brackets in template");
            --opened;
        }
    }
    Perl_croak(aTHX_ "No group ending character '%c' found in template",
               ender);
    NOT_REACHED; /* NOTREACHED */
}


/* Convert unsigned decimal number to binary.
 * Expects a pointer to the first digit and address of length variable
 * Advances char pointer to 1st non-digit char and returns number
 */
STATIC const char *
S_get_num(pTHX_ const char *patptr, SSize_t *lenptr )
{
  SSize_t len = *patptr++ - '0';

  PERL_ARGS_ASSERT_GET_NUM;

  while (isDIGIT(*patptr)) {
    SSize_t nlen = (len * 10) + (*patptr++ - '0');
    if (nlen < 0 || nlen/10 != len)
      Perl_croak(aTHX_ "pack/unpack repeat count overflow");
    len = nlen;
  }
  *lenptr = len;
  return patptr;
}

/* The marvellous template parsing routine: Using state stored in *symptr,
 * locates next template code and count
 */
STATIC bool
S_next_symbol(pTHX_ tempsym_t* symptr )
{
  const char* patptr = symptr->patptr;
  const char* const patend = symptr->patend;

  PERL_ARGS_ASSERT_NEXT_SYMBOL;

  symptr->flags &= ~FLAG_SLASH;

  while (patptr < patend) {
    if (isSPACE(*patptr))
      patptr++;
    else if (*patptr == '#') {
      patptr++;
      while (patptr < patend && *patptr != '\n')
        patptr++;
      if (patptr < patend)
        patptr++;
    } else {
      /* We should have found a template code */
      I32 code = (U8) *patptr++;
      U32 inherited_modifiers = 0;

      /* unrecognised characters in pack/unpack formats were made fatal in
       * 5.004, with an exception added in 5.004_04 for ',' to "just" warn: */
      if (code == ','){
        if (((symptr->flags & FLAG_COMMA) == 0) && ckWARN(WARN_UNPACK)){
          symptr->flags |= FLAG_COMMA;
          /* diag_listed_as: Invalid type '%s' in %s */
          Perl_warner(aTHX_ packWARN(WARN_UNPACK),
                      "Invalid type ',' in %s", _action( symptr ) );
        }
        continue;
      }

      /* for '(', skip to ')' */
      if (code == '(') {
        if( isDIGIT(*patptr) || *patptr == '*' || *patptr == '[' )
          Perl_croak(aTHX_ "()-group starts with a count in %s",
                        _action( symptr ) );
        symptr->grpbeg = patptr;
        patptr = 1 + ( symptr->grpend = group_end(patptr, patend, ')') );
        if( symptr->level >= MAX_SUB_TEMPLATE_LEVEL )
          Perl_croak(aTHX_ "Too deeply nested ()-groups in %s",
                        _action( symptr ) );
      }

      /* look for group modifiers to inherit */
      if (TYPE_ENDIANNESS(symptr->flags)) {
        if (strchr(ENDIANNESS_ALLOWED_TYPES, TYPE_NO_MODIFIERS(code)))
          inherited_modifiers |= TYPE_ENDIANNESS(symptr->flags);
      }

      /* look for modifiers */
      while (patptr < patend) {
        const char *allowed;
        I32 modifier;
        switch (*patptr) {
          case '!':
            modifier = TYPE_IS_SHRIEKING;
            allowed = "sSiIlLxXnNvV@.";
            break;
          case '>':
            modifier = TYPE_IS_BIG_ENDIAN;
            allowed = ENDIANNESS_ALLOWED_TYPES;
            break;
          case '<':
            modifier = TYPE_IS_LITTLE_ENDIAN;
            allowed = ENDIANNESS_ALLOWED_TYPES;
            break;
          default:
            allowed = "";
            modifier = 0;
            break;
        }

        if (modifier == 0)
          break;

        if (!strchr(allowed, TYPE_NO_MODIFIERS(code)))
          Perl_croak(aTHX_ "'%c' allowed only after types %s in %s", *patptr,
                        allowed, _action( symptr ) );

        if (TYPE_ENDIANNESS(code | modifier) == TYPE_ENDIANNESS_MASK)
          Perl_croak(aTHX_ "Can't use both '<' and '>' after type '%c' in %s",
                     (int) TYPE_NO_MODIFIERS(code), _action( symptr ) );
        else if (TYPE_ENDIANNESS(code | modifier | inherited_modifiers) ==
                 TYPE_ENDIANNESS_MASK)
          Perl_croak(aTHX_ "Can't use '%c' in a group with different byte-order in %s",
                     *patptr, _action( symptr ) );

        if ((code & modifier)) {
            Perl_ck_warner(aTHX_ packWARN(WARN_UNPACK),
                           "Duplicate modifier '%c' after '%c' in %s",
                           *patptr, (int) TYPE_NO_MODIFIERS(code),
                           _action( symptr ) );
        }

        code |= modifier;
        patptr++;
      }

      /* inherit modifiers */
      code |= inherited_modifiers;

      /* look for count and/or / */
      if (patptr < patend) {
        if (isDIGIT(*patptr)) {
          patptr = get_num( patptr, &symptr->length );
          symptr->howlen = e_number;

        } else if (*patptr == '*') {
          patptr++;
          symptr->howlen = e_star;

        } else if (*patptr == '[') {
          const char* lenptr = ++patptr;
          symptr->howlen = e_number;
          patptr = group_end( patptr, patend, ']' ) + 1;
          /* what kind of [] is it? */
          if (isDIGIT(*lenptr)) {
            lenptr = get_num( lenptr, &symptr->length );
            if( *lenptr != ']' )
              Perl_croak(aTHX_ "Malformed integer in [] in %s",
                            _action( symptr ) );
          } else {
            tempsym_t savsym = *symptr;
            symptr->patend = patptr-1;
            symptr->patptr = lenptr;
            savsym.length = measure_struct(symptr);
            *symptr = savsym;
          }
        } else {
          symptr->howlen = e_no_len;
          symptr->length = 1;
        }

        /* try to find / */
        while (patptr < patend) {
          if (isSPACE(*patptr))
            patptr++;
          else if (*patptr == '#') {
            patptr++;
            while (patptr < patend && *patptr != '\n')
              patptr++;
            if (patptr < patend)
              patptr++;
          } else {
            if (*patptr == '/') {
              symptr->flags |= FLAG_SLASH;
              patptr++;
              if (patptr < patend &&
                  (isDIGIT(*patptr) || *patptr == '*' || *patptr == '['))
                Perl_croak(aTHX_ "'/' does not take a repeat count in %s",
                            _action( symptr ) );
            }
            break;
          }
        }
      } else {
        /* at end - no count, no / */
        symptr->howlen = e_no_len;
        symptr->length = 1;
      }

      symptr->code = code;
      symptr->patptr = patptr;
      return TRUE;
    }
  }
  symptr->patptr = patptr;
  return FALSE;
}

/*
   There is no way to cleanly handle the case where we should process the
   string per byte in its upgraded form while it's really in downgraded form
   (e.g. estimates like strend-s as an upper bound for the number of
   characters left wouldn't work). So if we foresee the need of this
   (pattern starts with U or contains U0), we want to work on the encoded
   version of the string. Users are advised to upgrade their pack string
   themselves if they need to do a lot of unpacks like this on it
*/
STATIC bool
need_utf8(const char *pat, const char *patend)
{
    bool first = TRUE;

    PERL_ARGS_ASSERT_NEED_UTF8;

    while (pat < patend) {
        if (pat[0] == '#') {
            pat++;
            pat = (const char *) memchr(pat, '\n', patend-pat);
            if (!pat) return FALSE;
        } else if (pat[0] == 'U') {
            if (first || pat[1] == '0') return TRUE;
        } else first = FALSE;
        pat++;
    }
    return FALSE;
}

STATIC char
first_symbol(const char *pat, const char *patend) {
    PERL_ARGS_ASSERT_FIRST_SYMBOL;

    while (pat < patend) {
        if (pat[0] != '#') return pat[0];
        pat++;
        pat = (const char *) memchr(pat, '\n', patend-pat);
        if (!pat) return 0;
        pat++;
    }
    return 0;
}

/*

=for apidoc unpackstring

The engine implementing the C<unpack()> Perl function.

Using the template C<pat..patend>, this function unpacks the string
C<s..strend> into a number of mortal SVs, which it pushes onto the perl
argument (C<@_>) stack (so you will need to issue a C<PUTBACK> before and
C<SPAGAIN> after the call to this function).  It returns the number of
pushed elements.

The C<strend> and C<patend> pointers should point to the byte following the
last character of each string.

Although this function returns its values on the perl argument stack, it
doesn't take any parameters from that stack (and thus in particular
there's no need to do a C<PUSHMARK> before calling it, unlike L</call_pv> for
example).

=cut */

SSize_t
Perl_unpackstring(pTHX_ const char *pat, const char *patend, const char *s, const char *strend, U32 flags)
{
    tempsym_t sym;

    PERL_ARGS_ASSERT_UNPACKSTRING;

    if (flags & FLAG_DO_UTF8) flags |= FLAG_WAS_UTF8;
    else if (need_utf8(pat, patend)) {
        /* We probably should try to avoid this in case a scalar context call
           wouldn't get to the "U0" */
        STRLEN len = strend - s;
        s = (char *) bytes_to_utf8((U8 *) s, &len);
        SAVEFREEPV(s);
        strend = s + len;
        flags |= FLAG_DO_UTF8;
    }

    if (first_symbol(pat, patend) != 'U' && (flags & FLAG_DO_UTF8))
        flags |= FLAG_PARSE_UTF8;

    TEMPSYM_INIT(&sym, pat, patend, flags);

    return unpack_rec(&sym, s, s, strend, NULL );
}

STATIC SSize_t
S_unpack_rec(pTHX_ tempsym_t* symptr, const char *s, const char *strbeg, const char *strend, const char **new_s )
{
    dSP;
    SV *sv = NULL;
    const SSize_t start_sp_offset = SP - PL_stack_base;
    howlen_t howlen;
    SSize_t checksum = 0;
    UV cuv = 0;
    NV cdouble = 0.0;
    const SSize_t bits_in_uv = CHAR_BIT * sizeof(cuv);
    bool beyond = FALSE;
    bool explicit_length;
    const bool unpack_only_one = (symptr->flags & FLAG_UNPACK_ONLY_ONE) != 0;
    bool utf8 = (symptr->flags & FLAG_PARSE_UTF8) ? 1 : 0;

    PERL_ARGS_ASSERT_UNPACK_REC;

    symptr->strbeg = s - strbeg;

    while (next_symbol(symptr)) {
        packprops_t props;
        SSize_t len;
        I32 datumtype = symptr->code;
        bool needs_swap;
        /* do first one only unless in list context
           / is implemented by unpacking the count, then popping it from the
           stack, so must check that we're not in the middle of a /  */
        if ( unpack_only_one
             && (SP - PL_stack_base == start_sp_offset + 1)
             && (datumtype != '/') )   /* XXX can this be omitted */
            break;

        switch (howlen = symptr->howlen) {
          case e_star:
            len = strend - strbeg;	/* long enough */
            break;
          default:
            /* e_no_len and e_number */
            len = symptr->length;
            break;
        }

        explicit_length = TRUE;
      redo_switch:
        beyond = s >= strend;

        props = packprops[TYPE_NO_ENDIANNESS(datumtype)];
        if (props) {
            /* props nonzero means we can process this letter. */
            const SSize_t size = props & PACK_SIZE_MASK;
            const SSize_t howmany = (strend - s) / size;
            if (len > howmany)
                len = howmany;

            if (!checksum || (props & PACK_SIZE_CANNOT_CSUM)) {
                if (len && unpack_only_one) len = 1;
                EXTEND(SP, len);
                EXTEND_MORTAL(len);
            }
        }

        needs_swap = NEEDS_SWAP(datumtype);

        switch(TYPE_NO_ENDIANNESS(datumtype)) {
        default:
            /* diag_listed_as: Invalid type '%s' in %s */
            Perl_croak(aTHX_ "Invalid type '%c' in unpack", (int)TYPE_NO_MODIFIERS(datumtype) );

        case '%':
            if (howlen == e_no_len)
                len = 16;		/* len is not specified */
            checksum = len;
            cuv = 0;
            cdouble = 0;
            continue;

        case '(':
        {
            tempsym_t savsym = *symptr;
            const U32 group_modifiers = TYPE_MODIFIERS(datumtype & ~symptr->flags);
            symptr->flags |= group_modifiers;
            symptr->patend = savsym.grpend;
            /* cppcheck-suppress autoVariables */
            symptr->previous = &savsym;
            symptr->level++;
            PUTBACK;
            if (len && unpack_only_one) len = 1;
            while (len--) {
                symptr->patptr = savsym.grpbeg;
                if (utf8) symptr->flags |=  FLAG_PARSE_UTF8;
                else      symptr->flags &= ~FLAG_PARSE_UTF8;
                unpack_rec(symptr, s, strbeg, strend, &s);
                if (s == strend && savsym.howlen == e_star)
                    break; /* No way to continue */
            }
            SPAGAIN;
            savsym.flags = symptr->flags & ~group_modifiers;
            *symptr = savsym;
            break;
        }
        case '.' | TYPE_IS_SHRIEKING:
        case '.': {
            const char *from;
            SV *sv;
            const bool u8 = utf8 && !(datumtype & TYPE_IS_SHRIEKING);
            if (howlen == e_star) from = strbeg;
            else if (len <= 0) from = s;
            else {
                tempsym_t *group = symptr;

                while (--len && group) group = group->previous;
                from = group ? strbeg + group->strbeg : strbeg;
            }
            sv = from <= s ?
                newSVuv(  u8 ? (UV) utf8_length((const U8*)from, (const U8*)s) : (UV) (s-from)) :
                newSViv(-(u8 ? (IV) utf8_length((const U8*)s, (const U8*)from) : (IV) (from-s)));
            mXPUSHs(sv);
            break;
        }
        case '@' | TYPE_IS_SHRIEKING:
        case '@':
            s = strbeg + symptr->strbeg;
            if (utf8  && !(datumtype & TYPE_IS_SHRIEKING))
            {
                while (len > 0) {
                    if (s >= strend)
                        Perl_croak(aTHX_ "'@' outside of string in unpack");
                    s += UTF8SKIP(s);
                    len--;
                }
                if (s > strend)
                    Perl_croak(aTHX_ "'@' outside of string with malformed UTF-8 in unpack");
            } else {
                if (strend-s < len)
                    Perl_croak(aTHX_ "'@' outside of string in unpack");
                s += len;
            }
            break;
        case 'X' | TYPE_IS_SHRIEKING:
            if (!len)			/* Avoid division by 0 */
                len = 1;
            if (utf8) {
                const char *hop, *last;
                SSize_t l = len;
                hop = last = strbeg;
                while (hop < s) {
                    hop += UTF8SKIP(hop);
                    if (--l == 0) {
                        last = hop;
                        l = len;
                    }
                }
                if (last > s)
                    Perl_croak(aTHX_ "Malformed UTF-8 string in unpack");
                s = last;
                break;
            }
            len = (s - strbeg) % len;
            /* FALLTHROUGH */
        case 'X':
            if (utf8) {
                while (len > 0) {
                    if (s <= strbeg)
                        Perl_croak(aTHX_ "'X' outside of string in unpack");
                    while (--s, UTF8_IS_CONTINUATION(*s)) {
                        if (s <= strbeg)
                            Perl_croak(aTHX_ "'X' outside of string in unpack");
                    }
                    len--;
                }
            } else {
                if (len > s - strbeg)
                    Perl_croak(aTHX_ "'X' outside of string in unpack" );
                s -= len;
            }
            break;
        case 'x' | TYPE_IS_SHRIEKING: {
            SSize_t ai32;
            if (!len)			/* Avoid division by 0 */
                len = 1;
            if (utf8) ai32 = utf8_length((U8 *) strbeg, (U8 *) s) % len;
            else      ai32 = (s - strbeg)                         % len;
            if (ai32 == 0) break;
            len -= ai32;
            }
            /* FALLTHROUGH */
        case 'x':
            if (utf8) {
                while (len>0) {
                    if (s >= strend)
                        Perl_croak(aTHX_ "'x' outside of string in unpack");
                    s += UTF8SKIP(s);
                    len--;
                }
            } else {
                if (len > strend - s)
                    Perl_croak(aTHX_ "'x' outside of string in unpack");
                s += len;
            }
            break;
        case '/':
            Perl_croak(aTHX_ "'/' must follow a numeric type in unpack");

        case 'A':
        case 'Z':
        case 'a':
            if (checksum) {
                /* Preliminary length estimate is assumed done in 'W' */
                if (len > strend - s) len = strend - s;
                goto W_checksum;
            }
            if (utf8) {
                SSize_t l;
                const char *hop;
                for (l=len, hop=s; l>0; l--, hop += UTF8SKIP(hop)) {
                    if (hop >= strend) {
                        if (hop > strend)
                            Perl_croak(aTHX_ "Malformed UTF-8 string in unpack");
                        break;
                    }
                }
                if (hop > strend)
                    Perl_croak(aTHX_ "Malformed UTF-8 string in unpack");
                len = hop - s;
            } else if (len > strend - s)
                len = strend - s;

            if (datumtype == 'Z') {
                /* 'Z' strips stuff after first null */
                const char *ptr, *end;
                end = s + len;
                for (ptr = s; ptr < end; ptr++) if (*ptr == 0) break;
                sv = newSVpvn(s, ptr-s);
                if (howlen == e_star) /* exact for 'Z*' */
                    len = ptr-s + (ptr != strend ? 1 : 0);
            } else if (datumtype == 'A') {
                /* 'A' strips both nulls and spaces */
                const char *ptr;
                if (utf8 && (symptr->flags & FLAG_WAS_UTF8)) {
                    for (ptr = s+len-1; ptr >= s; ptr--) {
                        if (   *ptr != 0
                            && !UTF8_IS_CONTINUATION(*ptr)
                            && !isSPACE_utf8_safe(ptr, strend))
                        {
                            break;
                        }
                    }
                    if (ptr >= s) ptr += UTF8SKIP(ptr);
                    else ptr++;
                    if (ptr > s+len)
                        Perl_croak(aTHX_ "Malformed UTF-8 string in unpack");
                } else {
                    for (ptr = s+len-1; ptr >= s; ptr--)
                        if (*ptr != 0 && !isSPACE(*ptr)) break;
                    ptr++;
                }
                sv = newSVpvn(s, ptr-s);
            } else sv = newSVpvn(s, len);

            if (utf8) {
                SvUTF8_on(sv);
                /* Undo any upgrade done due to need_utf8() */
                if (!(symptr->flags & FLAG_WAS_UTF8))
                    sv_utf8_downgrade(sv, 0);
            }
            mXPUSHs(sv);
            s += len;
            break;
        case 'B':
        case 'b': {
            char *str;
            if (howlen == e_star || len > (strend - s) * 8)
                len = (strend - s) * 8;
            if (checksum) {
                if (utf8)
                    while (len >= 8 && s < strend) {
                        cuv += PL_bitcount[utf8_to_byte(aTHX_ &s, strend, datumtype)];
                        len -= 8;
                    }
                else
                    while (len >= 8) {
                        cuv += PL_bitcount[*(U8 *)s++];
                        len -= 8;
                    }
                if (len && s < strend) {
                    U8 bits;
                    bits = SHIFT_BYTE(utf8, s, strend, datumtype);
                    if (datumtype == 'b')
                        while (len-- > 0) {
                            if (bits & 1) cuv++;
                            bits >>= 1;
                        }
                    else
                        while (len-- > 0) {
                            if (bits & 0x80) cuv++;
                            bits <<= 1;
                        }
                }
                break;
            }

            sv = sv_2mortal(newSV(len ? len : 1));
            SvPOK_on(sv);
            str = SvPVX(sv);
            if (datumtype == 'b') {
                U8 bits = 0;
                const SSize_t ai32 = len;
                for (len = 0; len < ai32; len++) {
                    if (len & 7) bits >>= 1;
                    else if (utf8) {
                        if (s >= strend) break;
                        bits = utf8_to_byte(aTHX_ &s, strend, datumtype);
                    } else bits = *(U8 *) s++;
                    *str++ = bits & 1 ? '1' : '0';
                }
            } else {
                U8 bits = 0;
                const SSize_t ai32 = len;
                for (len = 0; len < ai32; len++) {
                    if (len & 7) bits <<= 1;
                    else if (utf8) {
                        if (s >= strend) break;
                        bits = utf8_to_byte(aTHX_ &s, strend, datumtype);
                    } else bits = *(U8 *) s++;
                    *str++ = bits & 0x80 ? '1' : '0';
                }
            }
            *str = '\0';
            SvCUR_set(sv, str - SvPVX_const(sv));
            XPUSHs(sv);
            break;
        }
        case 'H':
        case 'h': {
            char *str = NULL;
            /* Preliminary length estimate, acceptable for utf8 too */
            if (howlen == e_star || len > (strend - s) * 2)
                len = (strend - s) * 2;
            if (!checksum) {
                sv = sv_2mortal(newSV(len ? len : 1));
                SvPOK_on(sv);
                str = SvPVX(sv);
            }
            if (datumtype == 'h') {
                U8 bits = 0;
                SSize_t ai32 = len;
                for (len = 0; len < ai32; len++) {
                    if (len & 1) bits >>= 4;
                    else if (utf8) {
                        if (s >= strend) break;
                        bits = utf8_to_byte(aTHX_ &s, strend, datumtype);
                    } else bits = * (U8 *) s++;
                    if (!checksum)
                        *str++ = PL_hexdigit[bits & 15];
                }
            } else {
                U8 bits = 0;
                const SSize_t ai32 = len;
                for (len = 0; len < ai32; len++) {
                    if (len & 1) bits <<= 4;
                    else if (utf8) {
                        if (s >= strend) break;
                        bits = utf8_to_byte(aTHX_ &s, strend, datumtype);
                    } else bits = *(U8 *) s++;
                    if (!checksum)
                        *str++ = PL_hexdigit[(bits >> 4) & 15];
                }
            }
            if (!checksum) {
                *str = '\0';
                SvCUR_set(sv, str - SvPVX_const(sv));
                XPUSHs(sv);
            }
            break;
        }
        case 'C':
            if (len == 0) {
                if (explicit_length)
                    /* Switch to "character" mode */
                    utf8 = (symptr->flags & FLAG_DO_UTF8) ? 1 : 0;
                break;
            }
            /* FALLTHROUGH */
        case 'c':
            while (len-- > 0 && s < strend) {
                int aint;
                if (utf8)
                  {
                    STRLEN retlen;
                    aint = utf8n_to_uvchr((U8 *) s, strend-s, &retlen,
                                 ckWARN(WARN_UTF8) ? 0 : UTF8_ALLOW_ANY);
                    if (retlen == (STRLEN) -1)
                        Perl_croak(aTHX_ "Malformed UTF-8 string in unpack");
                    s += retlen;
                  }
                else
                  aint = *(U8 *)(s)++;
                if (aint >= 128 && datumtype != 'C')	/* fake up signed chars */
                    aint -= 256;
                if (!checksum)
                    mPUSHi(aint);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)aint;
                else
                    cuv += aint;
            }
            break;
        case 'W':
          W_checksum:
            if (utf8) {
                while (len-- > 0 && s < strend) {
                    STRLEN retlen;
                    const UV val = utf8n_to_uvchr((U8 *) s, strend-s, &retlen,
                                         ckWARN(WARN_UTF8) ? 0 : UTF8_ALLOW_ANY);
                    if (retlen == (STRLEN) -1)
                        Perl_croak(aTHX_ "Malformed UTF-8 string in unpack");
                    s += retlen;
                    if (!checksum)
                        mPUSHu(val);
                    else if (checksum > bits_in_uv)
                        cdouble += (NV) val;
                    else
                        cuv += val;
                }
            } else if (!checksum)
                while (len-- > 0) {
                    const U8 ch = *(U8 *) s++;
                    mPUSHu(ch);
            }
            else if (checksum > bits_in_uv)
                while (len-- > 0) cdouble += (NV) *(U8 *) s++;
            else
                while (len-- > 0) cuv += *(U8 *) s++;
            break;
        case 'U':
            if (len == 0) {
                if (explicit_length && howlen != e_star) {
                    /* Switch to "bytes in UTF-8" mode */
                    if (symptr->flags & FLAG_DO_UTF8) utf8 = 0;
                    else
                        /* Should be impossible due to the need_utf8() test */
                        Perl_croak(aTHX_ "U0 mode on a byte string");
                }
                break;
            }
            if (len > strend - s) len = strend - s;
            if (!checksum) {
                if (len && unpack_only_one) len = 1;
                EXTEND(SP, len);
                EXTEND_MORTAL(len);
            }
            while (len-- > 0 && s < strend) {
                STRLEN retlen;
                UV auv;
                if (utf8) {
                    U8 result[UTF8_MAXLEN+1];
                    const char *ptr = s;
                    STRLEN len;
                    /* Bug: warns about bad utf8 even if we are short on bytes
                       and will break out of the loop */
                    if (!S_utf8_to_bytes(aTHX_ &ptr, strend, (char *) result, 1,
                                      'U'))
                        break;
                    len = UTF8SKIP(result);
                    if (!S_utf8_to_bytes(aTHX_ &ptr, strend,
                                      (char *) &result[1], len-1, 'U')) break;
                    auv = utf8n_to_uvchr(result, len, &retlen,
                                         UTF8_ALLOW_DEFAULT);
                    s = ptr;
                } else {
                    auv = utf8n_to_uvchr((U8*)s, strend - s, &retlen,
                                         UTF8_ALLOW_DEFAULT);
                    if (retlen == (STRLEN) -1)
                        Perl_croak(aTHX_ "Malformed UTF-8 string in unpack");
                    s += retlen;
                }
                if (!checksum)
                    mPUSHu(auv);
                else if (checksum > bits_in_uv)
                    cdouble += (NV) auv;
                else
                    cuv += auv;
            }
            break;
        case 's' | TYPE_IS_SHRIEKING:
#if SHORTSIZE != SIZE16
            while (len-- > 0) {
                short ashort;
                SHIFT_VAR(utf8, s, strend, ashort, datumtype, needs_swap);
                if (!checksum)
                    mPUSHi(ashort);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)ashort;
                else
                    cuv += ashort;
            }
            break;
#else
            /* FALLTHROUGH */
#endif
        case 's':
            while (len-- > 0) {
                I16 ai16;

#if U16SIZE > SIZE16
                ai16 = 0;
#endif
                SHIFT16(utf8, s, strend, &ai16, datumtype, needs_swap);
#if U16SIZE > SIZE16
                if (ai16 > 32767)
                    ai16 -= 65536;
#endif
                if (!checksum)
                    mPUSHi(ai16);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)ai16;
                else
                    cuv += ai16;
            }
            break;
        case 'S' | TYPE_IS_SHRIEKING:
#if SHORTSIZE != SIZE16
            while (len-- > 0) {
                unsigned short aushort;
                SHIFT_VAR(utf8, s, strend, aushort, datumtype, needs_swap);
                if (!checksum)
                    mPUSHu(aushort);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)aushort;
                else
                    cuv += aushort;
            }
            break;
#else
            /* FALLTHROUGH */
#endif
        case 'v':
        case 'n':
        case 'S':
            while (len-- > 0) {
                U16 au16;
#if U16SIZE > SIZE16
                au16 = 0;
#endif
                SHIFT16(utf8, s, strend, &au16, datumtype, needs_swap);
                if (datumtype == 'n')
                    au16 = PerlSock_ntohs(au16);
                if (datumtype == 'v')
                    au16 = vtohs(au16);
                if (!checksum)
                    mPUSHu(au16);
                else if (checksum > bits_in_uv)
                    cdouble += (NV) au16;
                else
                    cuv += au16;
            }
            break;
        case 'v' | TYPE_IS_SHRIEKING:
        case 'n' | TYPE_IS_SHRIEKING:
            while (len-- > 0) {
                I16 ai16;
# if U16SIZE > SIZE16
                ai16 = 0;
# endif
                SHIFT16(utf8, s, strend, &ai16, datumtype, needs_swap);
                /* There should never be any byte-swapping here.  */
                assert(!TYPE_ENDIANNESS(datumtype));
                if (datumtype == ('n' | TYPE_IS_SHRIEKING))
                    ai16 = (I16) PerlSock_ntohs((U16) ai16);
                if (datumtype == ('v' | TYPE_IS_SHRIEKING))
                    ai16 = (I16) vtohs((U16) ai16);
                if (!checksum)
                    mPUSHi(ai16);
                else if (checksum > bits_in_uv)
                    cdouble += (NV) ai16;
                else
                    cuv += ai16;
            }
            break;
        case 'i':
        case 'i' | TYPE_IS_SHRIEKING:
            while (len-- > 0) {
                int aint;
                SHIFT_VAR(utf8, s, strend, aint, datumtype, needs_swap);
                if (!checksum)
                    mPUSHi(aint);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)aint;
                else
                    cuv += aint;
            }
            break;
        case 'I':
        case 'I' | TYPE_IS_SHRIEKING:
            while (len-- > 0) {
                unsigned int auint;
                SHIFT_VAR(utf8, s, strend, auint, datumtype, needs_swap);
                if (!checksum)
                    mPUSHu(auint);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)auint;
                else
                    cuv += auint;
            }
            break;
        case 'j':
            while (len-- > 0) {
                IV aiv;
                SHIFT_VAR(utf8, s, strend, aiv, datumtype, needs_swap);
                if (!checksum)
                    mPUSHi(aiv);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)aiv;
                else
                    cuv += aiv;
            }
            break;
        case 'J':
            while (len-- > 0) {
                UV auv;
                SHIFT_VAR(utf8, s, strend, auv, datumtype, needs_swap);
                if (!checksum)
                    mPUSHu(auv);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)auv;
                else
                    cuv += auv;
            }
            break;
        case 'l' | TYPE_IS_SHRIEKING:
#if LONGSIZE != SIZE32
            while (len-- > 0) {
                long along;
                SHIFT_VAR(utf8, s, strend, along, datumtype, needs_swap);
                if (!checksum)
                    mPUSHi(along);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)along;
                else
                    cuv += along;
            }
            break;
#else
            /* FALLTHROUGH */
#endif
        case 'l':
            while (len-- > 0) {
                I32 ai32;
#if U32SIZE > SIZE32
                ai32 = 0;
#endif
                SHIFT32(utf8, s, strend, &ai32, datumtype, needs_swap);
#if U32SIZE > SIZE32
                if (ai32 > 2147483647) ai32 -= 4294967296;
#endif
                if (!checksum)
                    mPUSHi(ai32);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)ai32;
                else
                    cuv += ai32;
            }
            break;
        case 'L' | TYPE_IS_SHRIEKING:
#if LONGSIZE != SIZE32
            while (len-- > 0) {
                unsigned long aulong;
                SHIFT_VAR(utf8, s, strend, aulong, datumtype, needs_swap);
                if (!checksum)
                    mPUSHu(aulong);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)aulong;
                else
                    cuv += aulong;
            }
            break;
#else
            /* FALLTHROUGH */
#endif
        case 'V':
        case 'N':
        case 'L':
            while (len-- > 0) {
                U32 au32;
#if U32SIZE > SIZE32
                au32 = 0;
#endif
                SHIFT32(utf8, s, strend, &au32, datumtype, needs_swap);
                if (datumtype == 'N')
                    au32 = PerlSock_ntohl(au32);
                if (datumtype == 'V')
                    au32 = vtohl(au32);
                if (!checksum)
                    mPUSHu(au32);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)au32;
                else
                    cuv += au32;
            }
            break;
        case 'V' | TYPE_IS_SHRIEKING:
        case 'N' | TYPE_IS_SHRIEKING:
            while (len-- > 0) {
                I32 ai32;
#if U32SIZE > SIZE32
                ai32 = 0;
#endif
                SHIFT32(utf8, s, strend, &ai32, datumtype, needs_swap);
                /* There should never be any byte swapping here.  */
                assert(!TYPE_ENDIANNESS(datumtype));
                if (datumtype == ('N' | TYPE_IS_SHRIEKING))
                    ai32 = (I32)PerlSock_ntohl((U32)ai32);
                if (datumtype == ('V' | TYPE_IS_SHRIEKING))
                    ai32 = (I32)vtohl((U32)ai32);
                if (!checksum)
                    mPUSHi(ai32);
                else if (checksum > bits_in_uv)
                    cdouble += (NV)ai32;
                else
                    cuv += ai32;
            }
            break;
        case 'p':
            while (len-- > 0) {
                const char *aptr;
                SHIFT_VAR(utf8, s, strend, aptr, datumtype, needs_swap);
                /* newSVpv generates undef if aptr is NULL */
                mPUSHs(newSVpv(aptr, 0));
            }
            break;
        case 'w':
            {
                UV auv = 0;
                size_t bytes = 0;

                while (len > 0 && s < strend) {
                    U8 ch;
                    ch = SHIFT_BYTE(utf8, s, strend, datumtype);
                    auv = (auv << 7) | (ch & 0x7f);
                    /* UTF8_IS_XXXXX not right here because this is a BER, not
                     * UTF-8 format - using constant 0x80 */
                    if (ch < 0x80) {
                        bytes = 0;
                        mPUSHu(auv);
                        len--;
                        auv = 0;
                        continue;
                    }
                    if (++bytes >= sizeof(UV)) {	/* promote to string */
                        const char *t;

                        sv = Perl_newSVpvf(aTHX_ "%.*" UVuf,
                                                 (int)TYPE_DIGITS(UV), auv);
                        while (s < strend) {
                            ch = SHIFT_BYTE(utf8, s, strend, datumtype);
                            sv = mul128(sv, (U8)(ch & 0x7f));
                            if (!(ch & 0x80)) {
                                bytes = 0;
                                break;
                            }
                        }
                        t = SvPV_nolen_const(sv);
                        while (*t == '0')
                            t++;
                        sv_chop(sv, t);
                        mPUSHs(sv);
                        len--;
                        auv = 0;
                    }
                }
                if ((s >= strend) && bytes)
                    Perl_croak(aTHX_ "Unterminated compressed integer in unpack");
            }
            break;
        case 'P':
            if (symptr->howlen == e_star)
                Perl_croak(aTHX_ "'P' must have an explicit size in unpack");
            EXTEND(SP, 1);
            if (s + sizeof(char*) <= strend) {
                char *aptr;
                SHIFT_VAR(utf8, s, strend, aptr, datumtype, needs_swap);
                /* newSVpvn generates undef if aptr is NULL */
                PUSHs(newSVpvn_flags(aptr, len, SVs_TEMP));
            }
            break;
#if defined(HAS_QUAD) && IVSIZE >= 8
        case 'q':
            while (len-- > 0) {
                Quad_t aquad;
                SHIFT_VAR(utf8, s, strend, aquad, datumtype, needs_swap);
                if (!checksum)
                    mPUSHs(newSViv((IV)aquad));
                else if (checksum > bits_in_uv)
                    cdouble += (NV)aquad;
                else
                    cuv += aquad;
            }
            break;
        case 'Q':
            while (len-- > 0) {
                Uquad_t auquad;
                SHIFT_VAR(utf8, s, strend, auquad, datumtype, needs_swap);
                if (!checksum)
                    mPUSHs(newSVuv((UV)auquad));
                else if (checksum > bits_in_uv)
                    cdouble += (NV)auquad;
                else
                    cuv += auquad;
            }
            break;
#endif
        /* float and double added gnb@melba.bby.oz.au 22/11/89 */
        case 'f':
            while (len-- > 0) {
                float afloat;
                SHIFT_VAR(utf8, s, strend, afloat, datumtype, needs_swap);
                if (!checksum)
                    mPUSHn(afloat);
                else
                    cdouble += afloat;
            }
            break;
        case 'd':
            while (len-- > 0) {
                double adouble;
                SHIFT_VAR(utf8, s, strend, adouble, datumtype, needs_swap);
                if (!checksum)
                    mPUSHn(adouble);
                else
                    cdouble += adouble;
            }
            break;
        case 'F':
            while (len-- > 0) {
                NV_bytes anv;
                SHIFT_BYTES(utf8, s, strend, anv.bytes, sizeof(anv.bytes),
                            datumtype, needs_swap);
                if (!checksum)
                    mPUSHn(anv.nv);
                else
                    cdouble += anv.nv;
            }
            break;
#if defined(HAS_LONG_DOUBLE)
        case 'D':
            while (len-- > 0) {
                ld_bytes aldouble;
                SHIFT_BYTES(utf8, s, strend, aldouble.bytes,
                            sizeof(aldouble.bytes), datumtype, needs_swap);
                /* The most common long double format, the x86 80-bit
                 * extended precision, has either 2 or 6 unused bytes,
                 * which may contain garbage, which may contain
                 * unintentional data.  While we do zero the bytes of
                 * the long double data in pack(), here in unpack() we
                 * don't, because it's really hard to envision that
                 * reading the long double off aldouble would be
                 * affected by the unused bytes.
                 *
                 * Note that trying to unpack 'long doubles' of 'long
                 * doubles' packed in another system is in the general
                 * case doomed without having more detail. */
                if (!checksum)
                    mPUSHn(aldouble.ld);
                else
                    cdouble += aldouble.ld;
            }
            break;
#endif
        case 'u':
            if (!checksum) {
                const STRLEN l = (STRLEN) (strend - s) * 3 / 4;
                sv = sv_2mortal(newSV(l));
                if (l) {
                    SvPOK_on(sv);
                    *SvEND(sv) = '\0';
                }
            }

            /* Note that all legal uuencoded strings are ASCII printables, so
             * have the same representation under UTF-8 vs not.  This means we
             * can ignore UTF8ness on legal input.  For illegal we stop at the
             * first failure, and don't report where/what that is, so again we
             * can ignore UTF8ness */

            while (s < strend && *s != ' ' && ISUUCHAR(*s)) {
                I32 a, b, c, d;
                char hunk[3];

                len = PL_uudmap[*(U8*)s++] & 077;
                while (len > 0) {
                    if (s < strend && ISUUCHAR(*s))
                        a = PL_uudmap[*(U8*)s++] & 077;
                    else
                        a = 0;
                    if (s < strend && ISUUCHAR(*s))
                        b = PL_uudmap[*(U8*)s++] & 077;
                    else
                        b = 0;
                    if (s < strend && ISUUCHAR(*s))
                        c = PL_uudmap[*(U8*)s++] & 077;
                    else
                        c = 0;
                    if (s < strend && ISUUCHAR(*s))
                        d = PL_uudmap[*(U8*)s++] & 077;
                    else
                        d = 0;
                    hunk[0] = (char)((a << 2) | (b >> 4));
                    hunk[1] = (char)((b << 4) | (c >> 2));
                    hunk[2] = (char)((c << 6) | d);
                    if (!checksum)
                        sv_catpvn(sv, hunk, (len > 3) ? 3 : len);
                    len -= 3;
                }
                if (*s == '\n')
                    s++;
                else	/* possible checksum byte */
                    if (s + 1 < strend && s[1] == '\n')
                        s += 2;
            }
            if (!checksum)
                XPUSHs(sv);
            break;
        } /* End of switch */

        if (checksum) {
            if (memCHRs("fFdD", TYPE_NO_MODIFIERS(datumtype)) ||
              (checksum > bits_in_uv &&
               memCHRs("cCsSiIlLnNUWvVqQjJ", TYPE_NO_MODIFIERS(datumtype))) ) {
                NV trouble, anv;

                anv = (NV) (1 << (checksum & 15));
                while (checksum >= 16) {
                    checksum -= 16;
                    anv *= 65536.0;
                }
                while (cdouble < 0.0)
                    cdouble += anv;
                cdouble = Perl_modf(cdouble / anv, &trouble);
#ifdef LONGDOUBLE_DOUBLEDOUBLE
                /* Workaround for powerpc doubledouble modfl bug:
                 * close to 1.0L and -1.0L cdouble is 0, and trouble
                 * is cdouble / anv. */
                if (trouble != Perl_ceil(trouble)) {
                  cdouble = trouble;
                  if (cdouble >  1.0L) cdouble -= 1.0L;
                  if (cdouble < -1.0L) cdouble += 1.0L;
                }
#endif
                cdouble *= anv;
                sv = newSVnv(cdouble);
            }
            else {
                if (checksum < bits_in_uv) {
                    UV mask = nBIT_MASK(checksum);
                    cuv &= mask;
                }
                sv = newSVuv(cuv);
            }
            mXPUSHs(sv);
            checksum = 0;
        }

        if (symptr->flags & FLAG_SLASH){
            if (SP - PL_stack_base - start_sp_offset <= 0)
                break;
            if( next_symbol(symptr) ){
              if( symptr->howlen == e_number )
                Perl_croak(aTHX_ "Count after length/code in unpack" );
              if( beyond ){
                /* ...end of char buffer then no decent length available */
                Perl_croak(aTHX_ "length/code after end of string in unpack" );
              } else {
                /* take top of stack (hope it's numeric) */
                len = POPi;
                if( len < 0 )
                    Perl_croak(aTHX_ "Negative '/' count in unpack" );
              }
            } else {
                Perl_croak(aTHX_ "Code missing after '/' in unpack" );
            }
            datumtype = symptr->code;
            explicit_length = FALSE;
            goto redo_switch;
        }
    }

    if (new_s)
        *new_s = s;
    PUTBACK;
    return SP - PL_stack_base - start_sp_offset;
}

PP(pp_unpack)
{
    dSP;
    dPOPPOPssrl;
    U8 gimme = GIMME_V;
    STRLEN llen;
    STRLEN rlen;
    const char *pat = SvPV_const(left,  llen);
    const char *s   = SvPV_const(right, rlen);
    const char *strend = s + rlen;
    const char *patend = pat + llen;
    SSize_t cnt;

    PUTBACK;
    cnt = unpackstring(pat, patend, s, strend,
                     ((gimme == G_SCALAR) ? FLAG_UNPACK_ONLY_ONE : 0)
                     | (DO_UTF8(right) ? FLAG_DO_UTF8 : 0));

    SPAGAIN;
    if ( !cnt && gimme == G_SCALAR )
       PUSHs(&PL_sv_undef);
    RETURN;
}

STATIC U8 *
doencodes(U8 *h, const U8 *s, SSize_t len)
{
    *h++ = PL_uuemap[len];
    while (len > 2) {
        *h++ = PL_uuemap[(077 & (s[0] >> 2))];
        *h++ = PL_uuemap[(077 & (((s[0] << 4) & 060) | ((s[1] >> 4) & 017)))];
        *h++ = PL_uuemap[(077 & (((s[1] << 2) & 074) | ((s[2] >> 6) & 03)))];
        *h++ = PL_uuemap[(077 & (s[2] & 077))];
        s += 3;
        len -= 3;
    }
    if (len > 0) {
        const U8 r = (len > 1 ? s[1] : '\0');
        *h++ = PL_uuemap[(077 & (s[0] >> 2))];
        *h++ = PL_uuemap[(077 & (((s[0] << 4) & 060) | ((r >> 4) & 017)))];
        *h++ = PL_uuemap[(077 & ((r << 2) & 074))];
        *h++ = PL_uuemap[0];
    }
    *h++ = '\n';
    return h;
}

STATIC SV *
S_is_an_int(pTHX_ const char *s, STRLEN l)
{
  SV *result = newSVpvn(s, l);
  char *const result_c = SvPV_nolen(result);	/* convenience */
  char *out = result_c;
  bool skip = 1;
  bool ignore = 0;

  PERL_ARGS_ASSERT_IS_AN_INT;

  while (*s) {
    switch (*s) {
    case ' ':
      break;
    case '+':
      if (!skip) {
        SvREFCNT_dec(result);
        return (NULL);
      }
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      skip = 0;
      if (!ignore) {
        *(out++) = *s;
      }
      break;
    case '.':
      ignore = 1;
      break;
    default:
      SvREFCNT_dec(result);
      return (NULL);
    }
    s++;
  }
  *(out++) = '\0';
  SvCUR_set(result, out - result_c);
  return (result);
}

/* pnum must be '\0' terminated */
STATIC int
S_div128(pTHX_ SV *pnum, bool *done)
{
    STRLEN len;
    char * const s = SvPV(pnum, len);
    char *t = s;
    int m = 0;

    PERL_ARGS_ASSERT_DIV128;

    *done = 1;
    while (*t) {
        const int i = m * 10 + (*t - '0');
        const int r = (i >> 7); /* r < 10 */
        m = i & 0x7F;
        if (r) {
            *done = 0;
        }
        *(t++) = '0' + r;
    }
    *(t++) = '\0';
    SvCUR_set(pnum, (STRLEN) (t - s));
    return (m);
}

/*
=for apidoc packlist

The engine implementing C<pack()> Perl function.

=cut
*/

void
Perl_packlist(pTHX_ SV *cat, const char *pat, const char *patend, SV **beglist, SV **endlist )
{
    tempsym_t sym;

    PERL_ARGS_ASSERT_PACKLIST;

    TEMPSYM_INIT(&sym, pat, patend, FLAG_PACK);

    /* We're going to do changes through SvPVX(cat). Make sure it's valid.
       Also make sure any UTF8 flag is loaded */
    SvPV_force_nolen(cat);
    if (DO_UTF8(cat))
        sym.flags |= FLAG_PARSE_UTF8 | FLAG_DO_UTF8;

    (void)pack_rec( cat, &sym, beglist, endlist );
}

/* like sv_utf8_upgrade, but also repoint the group start markers */
STATIC void
marked_upgrade(pTHX_ SV *sv, tempsym_t *sym_ptr) {
    STRLEN len;
    tempsym_t *group;
    const char *from_ptr, *from_start, *from_end, **marks, **m;
    char *to_start, *to_ptr;

    if (SvUTF8(sv)) return;

    from_start = SvPVX_const(sv);
    from_end = from_start + SvCUR(sv);
    for (from_ptr = from_start; from_ptr < from_end; from_ptr++)
        if (!NATIVE_BYTE_IS_INVARIANT(*from_ptr)) break;
    if (from_ptr == from_end) {
        /* Simple case: no character needs to be changed */
        SvUTF8_on(sv);
        return;
    }

    len = (from_end-from_ptr)*UTF8_EXPAND+(from_ptr-from_start)+1;
    Newx(to_start, len, char);
    Copy(from_start, to_start, from_ptr-from_start, char);
    to_ptr = to_start + (from_ptr-from_start);

    Newx(marks, sym_ptr->level+2, const char *);
    for (group=sym_ptr; group; group = group->previous)
        marks[group->level] = from_start + group->strbeg;
    marks[sym_ptr->level+1] = from_end+1;
    for (m = marks; *m < from_ptr; m++)
        *m = to_start + (*m-from_start);

    for (;from_ptr < from_end; from_ptr++) {
        while (*m == from_ptr) *m++ = to_ptr;
        to_ptr = (char *) uvchr_to_utf8((U8 *) to_ptr, *(U8 *) from_ptr);
    }
    *to_ptr = 0;

    while (*m == from_ptr) *m++ = to_ptr;
    if (m != marks + sym_ptr->level+1) {
        Safefree(marks);
        Safefree(to_start);
        Perl_croak(aTHX_ "panic: marks beyond string end, m=%p, marks=%p, "
                   "level=%d", m, marks, sym_ptr->level);
    }
    for (group=sym_ptr; group; group = group->previous)
        group->strbeg = marks[group->level] - to_start;
    Safefree(marks);

    if (SvOOK(sv)) {
        if (SvIVX(sv)) {
            SvLEN_set(sv, SvLEN(sv) + SvIVX(sv));
            from_start -= SvIVX(sv);
            SvIV_set(sv, 0);
        }
        SvFLAGS(sv) &= ~SVf_OOK;
    }
    if (SvLEN(sv) != 0)
        Safefree(from_start);
    SvPV_set(sv, to_start);
    SvCUR_set(sv, to_ptr - to_start);
    SvLEN_set(sv, len);
    SvUTF8_on(sv);
}

/* Exponential string grower. Makes string extension effectively O(n)
   needed says how many extra bytes we need (not counting the final '\0')
   Only grows the string if there is an actual lack of space
*/
STATIC char *
S_sv_exp_grow(pTHX_ SV *sv, STRLEN needed) {
    const STRLEN cur = SvCUR(sv);
    const STRLEN len = SvLEN(sv);
    STRLEN extend;

    PERL_ARGS_ASSERT_SV_EXP_GROW;

    if (len - cur > needed) return SvPVX(sv);
    extend = needed > len ? needed : len;
    return SvGROW(sv, len+extend+1);
}

static SV *
S_sv_check_infnan(pTHX_ SV *sv, I32 datumtype)
{
    SvGETMAGIC(sv);
    if (UNLIKELY(SvAMAGIC(sv)))
        sv = sv_2num(sv);
    if (UNLIKELY(isinfnansv(sv))) {
        const I32 c = TYPE_NO_MODIFIERS(datumtype);
        const NV nv = SvNV_nomg(sv);
        if (c == 'w')
            Perl_croak(aTHX_ "Cannot compress %" NVgf " in pack", nv);
        else
            Perl_croak(aTHX_ "Cannot pack %" NVgf " with '%c'", nv, (int) c);
    }
    return sv;
}

#define SvIV_no_inf(sv,d) \
        ((sv) = S_sv_check_infnan(aTHX_ sv,d), SvIV_nomg(sv))
#define SvUV_no_inf(sv,d) \
        ((sv) = S_sv_check_infnan(aTHX_ sv,d), SvUV_nomg(sv))

STATIC
SV **
S_pack_rec(pTHX_ SV *cat, tempsym_t* symptr, SV **beglist, SV **endlist )
{
    tempsym_t lookahead;
    SSize_t items  = endlist - beglist;
    bool found = next_symbol(symptr);
    bool utf8 = (symptr->flags & FLAG_PARSE_UTF8) ? 1 : 0;
    bool warn_utf8 = ckWARN(WARN_UTF8);
    char* from;

    PERL_ARGS_ASSERT_PACK_REC;

    if (symptr->level == 0 && found && symptr->code == 'U') {
        marked_upgrade(aTHX_ cat, symptr);
        symptr->flags |= FLAG_DO_UTF8;
        utf8 = 0;
    }
    symptr->strbeg = SvCUR(cat);

    while (found) {
        SV *fromstr;
        STRLEN fromlen;
        SSize_t len;
        SV *lengthcode = NULL;
        I32 datumtype = symptr->code;
        howlen_t howlen = symptr->howlen;
        char *start = SvPVX(cat);
        char *cur   = start + SvCUR(cat);
        bool needs_swap;

#define NEXTFROM (lengthcode ? lengthcode : items > 0 ? (--items, *beglist++) : &PL_sv_no)
#define PEEKFROM (lengthcode ? lengthcode : items > 0 ? *beglist : &PL_sv_no)

        switch (howlen) {
          case e_star:
            len = memCHRs("@Xxu", TYPE_NO_MODIFIERS(datumtype)) ?
                0 : items;
            break;
          default:
            /* e_no_len and e_number */
            len = symptr->length;
            break;
        }

        if (len) {
            packprops_t props = packprops[TYPE_NO_ENDIANNESS(datumtype)];

            if (props && !(props & PACK_SIZE_UNPREDICTABLE)) {
                /* We can process this letter. */
                STRLEN size = props & PACK_SIZE_MASK;
                GROWING2(utf8, cat, start, cur, size, (STRLEN)len);
            }
        }

        /* Look ahead for next symbol. Do we have code/code? */
        lookahead = *symptr;
        found = next_symbol(&lookahead);
        if (symptr->flags & FLAG_SLASH) {
            IV count;
            if (!found) Perl_croak(aTHX_ "Code missing after '/' in pack");
            if (memCHRs("aAZ", lookahead.code)) {
                if (lookahead.howlen == e_number) count = lookahead.length;
                else {
                    if (items > 0) {
                        count = sv_len_utf8(*beglist);
                    }
                    else count = 0;
                    if (lookahead.code == 'Z') count++;
                }
            } else {
                if (lookahead.howlen == e_number && lookahead.length < items)
                    count = lookahead.length;
                else count = items;
            }
            lookahead.howlen = e_number;
            lookahead.length = count;
            lengthcode = sv_2mortal(newSViv(count));
        }

        needs_swap = NEEDS_SWAP(datumtype);

        /* Code inside the switch must take care to properly update
           cat (CUR length and '\0' termination) if it updated *cur and
           doesn't simply leave using break */
        switch (TYPE_NO_ENDIANNESS(datumtype)) {
        default:
            /* diag_listed_as: Invalid type '%s' in %s */
            Perl_croak(aTHX_ "Invalid type '%c' in pack",
                       (int) TYPE_NO_MODIFIERS(datumtype));
        case '%':
            Perl_croak(aTHX_ "'%%' may not be used in pack");

        case '.' | TYPE_IS_SHRIEKING:
        case '.':
            if (howlen == e_star) from = start;
            else if (len == 0) from = cur;
            else {
                tempsym_t *group = symptr;

                while (--len && group) group = group->previous;
                from = group ? start + group->strbeg : start;
            }
            fromstr = NEXTFROM;
            len = SvIV_no_inf(fromstr, datumtype);
            goto resize;
        case '@' | TYPE_IS_SHRIEKING:
        case '@':
            from = start + symptr->strbeg;
          resize:
            if (utf8  && !(datumtype & TYPE_IS_SHRIEKING))
                if (len >= 0) {
                    while (len && from < cur) {
                        from += UTF8SKIP(from);
                        len--;
                    }
                    if (from > cur)
                        Perl_croak(aTHX_ "Malformed UTF-8 string in pack");
                    if (len) {
                        /* Here we know from == cur */
                      grow:
                        GROWING(0, cat, start, cur, len);
                        Zero(cur, len, char);
                        cur += len;
                    } else if (from < cur) {
                        len = cur - from;
                        goto shrink;
                    } else goto no_change;
                } else {
                    cur = from;
                    len = -len;
                    goto utf8_shrink;
                }
            else {
                len -= cur - from;
                if (len > 0) goto grow;
                if (len == 0) goto no_change;
                len = -len;
                goto shrink;
            }
            break;

        case '(': {
            tempsym_t savsym = *symptr;
            U32 group_modifiers = TYPE_MODIFIERS(datumtype & ~symptr->flags);
            symptr->flags |= group_modifiers;
            symptr->patend = savsym.grpend;
            symptr->level++;
            /* cppcheck-suppress autoVariables */
            symptr->previous = &lookahead;
            while (len--) {
                U32 was_utf8;
                if (utf8) symptr->flags |=  FLAG_PARSE_UTF8;
                else      symptr->flags &= ~FLAG_PARSE_UTF8;
                was_utf8 = SvUTF8(cat);
                symptr->patptr = savsym.grpbeg;
                beglist = pack_rec(cat, symptr, beglist, endlist);
                if (SvUTF8(cat) != was_utf8)
                    /* This had better be an upgrade while in utf8==0 mode */
                    utf8 = 1;

                if (savsym.howlen == e_star && beglist == endlist)
                    break;		/* No way to continue */
            }
            items = endlist - beglist;
            lookahead.flags  = symptr->flags & ~group_modifiers;
            goto no_change;
        }
        case 'X' | TYPE_IS_SHRIEKING:
            if (!len)			/* Avoid division by 0 */
                len = 1;
            if (utf8) {
                char *hop, *last;
                SSize_t l = len;
                hop = last = start;
                while (hop < cur) {
                    hop += UTF8SKIP(hop);
                    if (--l == 0) {
                        last = hop;
                        l = len;
                    }
                }
                if (last > cur)
                    Perl_croak(aTHX_ "Malformed UTF-8 string in pack");
                cur = last;
                break;
            }
            len = (cur-start) % len;
            /* FALLTHROUGH */
        case 'X':
            if (utf8) {
                if (len < 1) goto no_change;
              utf8_shrink:
                while (len > 0) {
                    if (cur <= start)
                        Perl_croak(aTHX_ "'%c' outside of string in pack",
                                   (int) TYPE_NO_MODIFIERS(datumtype));
                    while (--cur, UTF8_IS_CONTINUATION(*cur)) {
                        if (cur <= start)
                            Perl_croak(aTHX_ "'%c' outside of string in pack",
                                       (int) TYPE_NO_MODIFIERS(datumtype));
                    }
                    len--;
                }
            } else {
              shrink:
                if (cur - start < len)
                    Perl_croak(aTHX_ "'%c' outside of string in pack",
                               (int) TYPE_NO_MODIFIERS(datumtype));
                cur -= len;
            }
            if (cur < start+symptr->strbeg) {
                /* Make sure group starts don't point into the void */
                tempsym_t *group;
                const STRLEN length = cur-start;
                for (group = symptr;
                     group && length < group->strbeg;
                     group = group->previous) group->strbeg = length;
                lookahead.strbeg = length;
            }
            break;
        case 'x' | TYPE_IS_SHRIEKING: {
            SSize_t ai32;
            if (!len)			/* Avoid division by 0 */
                len = 1;
            if (utf8) ai32 = utf8_length((U8 *) start, (U8 *) cur) % len;
            else      ai32 = (cur - start) % len;
            if (ai32 == 0) goto no_change;
            len -= ai32;
        }
        /* FALLTHROUGH */
        case 'x':
            goto grow;
        case 'A':
        case 'Z':
        case 'a': {
            const char *aptr;

            fromstr = NEXTFROM;
            aptr = SvPV_const(fromstr, fromlen);
            if (DO_UTF8(fromstr)) {
                const char *end, *s;

                if (!utf8 && !SvUTF8(cat)) {
                    marked_upgrade(aTHX_ cat, symptr);
                    lookahead.flags |= FLAG_DO_UTF8;
                    lookahead.strbeg = symptr->strbeg;
                    utf8 = 1;
                    start = SvPVX(cat);
                    cur = start + SvCUR(cat);
                }
                if (howlen == e_star) {
                    if (utf8) goto string_copy;
                    len = fromlen+1;
                }
                s = aptr;
                end = aptr + fromlen;
                fromlen = datumtype == 'Z' ? len-1 : len;
                while ((SSize_t) fromlen > 0 && s < end) {
                    s += UTF8SKIP(s);
                    fromlen--;
                }
                if (s > end)
                    Perl_croak(aTHX_ "Malformed UTF-8 string in pack");
                if (utf8) {
                    len = fromlen;
                    if (datumtype == 'Z') len++;
                    fromlen = s-aptr;
                    len += fromlen;

                    goto string_copy;
                }
                fromlen = len - fromlen;
                if (datumtype == 'Z') fromlen--;
                if (howlen == e_star) {
                    len = fromlen;
                    if (datumtype == 'Z') len++;
                }
                GROWING(0, cat, start, cur, len);
                if (!S_utf8_to_bytes(aTHX_ &aptr, end, cur, fromlen,
                                  datumtype | TYPE_IS_PACK))
                    Perl_croak(aTHX_ "panic: predicted utf8 length not available, "
                               "for '%c', aptr=%p end=%p cur=%p, fromlen=%zu",
                               (int)datumtype, aptr, end, cur, fromlen);
                cur += fromlen;
                len -= fromlen;
            } else if (utf8) {
                if (howlen == e_star) {
                    len = fromlen;
                    if (datumtype == 'Z') len++;
                }
                if (len <= (SSize_t) fromlen) {
                    fromlen = len;
                    if (datumtype == 'Z' && fromlen > 0) fromlen--;
                }
                /* assumes a byte expands to at most UTF8_EXPAND bytes on
                   upgrade, so:
                   expected_length <= from_len*UTF8_EXPAND + (len-from_len) */
                GROWING(0, cat, start, cur, fromlen*(UTF8_EXPAND-1)+len);
                len -= fromlen;
                while (fromlen > 0) {
                    cur = (char *) uvchr_to_utf8((U8 *) cur, * (U8 *) aptr);
                    aptr++;
                    fromlen--;
                }
            } else {
              string_copy:
                if (howlen == e_star) {
                    len = fromlen;
                    if (datumtype == 'Z') len++;
                }
                if (len <= (SSize_t) fromlen) {
                    fromlen = len;
                    if (datumtype == 'Z' && fromlen > 0) fromlen--;
                }
                GROWING(0, cat, start, cur, len);
                Copy(aptr, cur, fromlen, char);
                cur += fromlen;
                len -= fromlen;
            }
            memset(cur, datumtype == 'A' ? ' ' : '\0', len);
            cur += len;
            SvTAINT(cat);
            break;
        }
        case 'B':
        case 'b': {
            const char *str, *end;
            SSize_t l, field_len;
            U8 bits;
            bool utf8_source;
            U32 utf8_flags;

            fromstr = NEXTFROM;
            str = SvPV_const(fromstr, fromlen);
            end = str + fromlen;
            if (DO_UTF8(fromstr)) {
                utf8_source = TRUE;
                utf8_flags  = warn_utf8 ? 0 : UTF8_ALLOW_ANY;
            } else {
                utf8_source = FALSE;
                utf8_flags  = 0; /* Unused, but keep compilers happy */
            }
            if (howlen == e_star) len = fromlen;
            field_len = (len+7)/8;
            GROWING(utf8, cat, start, cur, field_len);
            if (len > (SSize_t)fromlen) len = fromlen;
            bits = 0;
            l = 0;
            if (datumtype == 'B')
                while (l++ < len) {
                    if (utf8_source) {
                        UV val = 0;
                        NEXT_UNI_VAL(val, cur, str, end, utf8_flags);
                        bits |= val & 1;
                    } else bits |= *str++ & 1;
                    if (l & 7) bits <<= 1;
                    else {
                        PUSH_BYTE(utf8, cur, bits);
                        bits = 0;
                    }
                }
            else
                /* datumtype == 'b' */
                while (l++ < len) {
                    if (utf8_source) {
                        UV val = 0;
                        NEXT_UNI_VAL(val, cur, str, end, utf8_flags);
                        if (val & 1) bits |= 0x80;
                    } else if (*str++ & 1)
                        bits |= 0x80;
                    if (l & 7) bits >>= 1;
                    else {
                        PUSH_BYTE(utf8, cur, bits);
                        bits = 0;
                    }
                }
            l--;
            if (l & 7) {
                if (datumtype == 'B')
                    bits <<= 7 - (l & 7);
                else
                    bits >>= 7 - (l & 7);
                PUSH_BYTE(utf8, cur, bits);
                l += 7;
            }
            /* Determine how many chars are left in the requested field */
            l /= 8;
            if (howlen == e_star) field_len = 0;
            else field_len -= l;
            Zero(cur, field_len, char);
            cur += field_len;
            break;
        }
        case 'H':
        case 'h': {
            const char *str, *end;
            SSize_t l, field_len;
            U8 bits;
            bool utf8_source;
            U32 utf8_flags;

            fromstr = NEXTFROM;
            str = SvPV_const(fromstr, fromlen);
            end = str + fromlen;
            if (DO_UTF8(fromstr)) {
                utf8_source = TRUE;
                utf8_flags  = warn_utf8 ? 0 : UTF8_ALLOW_ANY;
            } else {
                utf8_source = FALSE;
                utf8_flags  = 0; /* Unused, but keep compilers happy */
            }
            if (howlen == e_star) len = fromlen;
            field_len = (len+1)/2;
            GROWING(utf8, cat, start, cur, field_len);
            if (!utf8_source && len > (SSize_t)fromlen) len = fromlen;
            bits = 0;
            l = 0;
            if (datumtype == 'H')
                while (l++ < len) {
                    if (utf8_source) {
                        UV val = 0;
                        NEXT_UNI_VAL(val, cur, str, end, utf8_flags);
                        if (val < 256 && isALPHA(val))
                            bits |= (val + 9) & 0xf;
                        else
                            bits |= val & 0xf;
                    } else if (isALPHA(*str))
                        bits |= (*str++ + 9) & 0xf;
                    else
                        bits |= *str++ & 0xf;
                    if (l & 1) bits <<= 4;
                    else {
                        PUSH_BYTE(utf8, cur, bits);
                        bits = 0;
                    }
                }
            else
                while (l++ < len) {
                    if (utf8_source) {
                        UV val = 0;
                        NEXT_UNI_VAL(val, cur, str, end, utf8_flags);
                        if (val < 256 && isALPHA(val))
                            bits |= ((val + 9) & 0xf) << 4;
                        else
                            bits |= (val & 0xf) << 4;
                    } else if (isALPHA(*str))
                        bits |= ((*str++ + 9) & 0xf) << 4;
                    else
                        bits |= (*str++ & 0xf) << 4;
                    if (l & 1) bits >>= 4;
                    else {
                        PUSH_BYTE(utf8, cur, bits);
                        bits = 0;
                    }
                }
            l--;
            if (l & 1) {
                PUSH_BYTE(utf8, cur, bits);
                l++;
            }
            /* Determine how many chars are left in the requested field */
            l /= 2;
            if (howlen == e_star) field_len = 0;
            else field_len -= l;
            Zero(cur, field_len, char);
            cur += field_len;
            break;
        }
        case 'c':
            while (len-- > 0) {
                IV aiv;
                fromstr = NEXTFROM;
                aiv = SvIV_no_inf(fromstr, datumtype);
                if ((-128 > aiv || aiv > 127))
                    Perl_ck_warner(aTHX_ packWARN(WARN_PACK),
                                   "Character in 'c' format wrapped in pack");
                PUSH_BYTE(utf8, cur, (U8)aiv);
            }
            break;
        case 'C':
            if (len == 0) {
                utf8 = (symptr->flags & FLAG_DO_UTF8) ? 1 : 0;
                break;
            }
            while (len-- > 0) {
                IV aiv;
                fromstr = NEXTFROM;
                aiv = SvIV_no_inf(fromstr, datumtype);
                if ((0 > aiv || aiv > 0xff))
                    Perl_ck_warner(aTHX_ packWARN(WARN_PACK),
                                   "Character in 'C' format wrapped in pack");
                PUSH_BYTE(utf8, cur, (U8)aiv);
            }
            break;
        case 'W': {
            char *end;
            U8 in_bytes = (U8)IN_BYTES;

            end = start+SvLEN(cat)-1;
            if (utf8) end -= UTF8_MAXLEN-1;
            while (len-- > 0) {
                UV auv;
                fromstr = NEXTFROM;
                auv = SvUV_no_inf(fromstr, datumtype);
                if (in_bytes) auv = auv % 0x100;
                if (utf8) {
                  W_utf8:
                    if (cur >= end) {
                        *cur = '\0';
                        SvCUR_set(cat, cur - start);

                        GROWING(0, cat, start, cur, len+UTF8_MAXLEN);
                        end = start+SvLEN(cat)-UTF8_MAXLEN;
                    }
                    cur = (char *) uvchr_to_utf8_flags((U8 *) cur, auv, 0);
                } else {
                    if (auv >= 0x100) {
                        if (!SvUTF8(cat)) {
                            *cur = '\0';
                            SvCUR_set(cat, cur - start);
                            marked_upgrade(aTHX_ cat, symptr);
                            lookahead.flags |= FLAG_DO_UTF8;
                            lookahead.strbeg = symptr->strbeg;
                            utf8 = 1;
                            start = SvPVX(cat);
                            cur = start + SvCUR(cat);
                            end = start+SvLEN(cat)-UTF8_MAXLEN;
                            goto W_utf8;
                        }
                        Perl_ck_warner(aTHX_ packWARN(WARN_PACK),
                                       "Character in 'W' format wrapped in pack");
                        auv = (U8) auv;
                    }
                    if (cur >= end) {
                        *cur = '\0';
                        SvCUR_set(cat, cur - start);
                        GROWING(0, cat, start, cur, len+1);
                        end = start+SvLEN(cat)-1;
                    }
                    *(U8 *) cur++ = (U8)auv;
                }
            }
            break;
        }
        case 'U': {
            char *end;

            if (len == 0) {
                if (!(symptr->flags & FLAG_DO_UTF8)) {
                    marked_upgrade(aTHX_ cat, symptr);
                    lookahead.flags |= FLAG_DO_UTF8;
                    lookahead.strbeg = symptr->strbeg;
                }
                utf8 = 0;
                goto no_change;
            }

            end = start+SvLEN(cat);
            if (!utf8) end -= UTF8_MAXLEN;
            while (len-- > 0) {
                UV auv;
                fromstr = NEXTFROM;
                auv = SvUV_no_inf(fromstr, datumtype);
                if (utf8) {
                    U8 buffer[UTF8_MAXLEN+1], *endb;
                    endb = uvchr_to_utf8_flags(buffer, auv, 0);
                    if (cur+(endb-buffer)*UTF8_EXPAND >= end) {
                        *cur = '\0';
                        SvCUR_set(cat, cur - start);
                        GROWING(0, cat, start, cur,
                                len+(endb-buffer)*UTF8_EXPAND);
                        end = start+SvLEN(cat);
                    }
                    cur = my_bytes_to_utf8(buffer, endb-buffer, cur, 0);
                } else {
                    if (cur >= end) {
                        *cur = '\0';
                        SvCUR_set(cat, cur - start);
                        GROWING(0, cat, start, cur, len+UTF8_MAXLEN);
                        end = start+SvLEN(cat)-UTF8_MAXLEN;
                    }
                    cur = (char *) uvchr_to_utf8_flags((U8 *) cur, auv, 0);
                }
            }
            break;
        }
        /* Float and double added by gnb@melba.bby.oz.au  22/11/89 */
        case 'f':
            while (len-- > 0) {
                float afloat;
                NV anv;
                fromstr = NEXTFROM;
                anv = SvNV(fromstr);
# if (defined(VMS) && !defined(_IEEE_FP)) || defined(DOUBLE_IS_VAX_FLOAT)
                /* IEEE fp overflow shenanigans are unavailable on VAX and optional
                 * on Alpha; fake it if we don't have them.
                 */
                if (anv > FLT_MAX)
                    afloat = FLT_MAX;
                else if (anv < -FLT_MAX)
                    afloat = -FLT_MAX;
                else afloat = (float)anv;
# else
#  if defined(NAN_COMPARE_BROKEN) && defined(Perl_isnan)
                if(Perl_isnan(anv))
                    afloat = (float)NV_NAN;
                else
#  endif
#  ifdef NV_INF
                /* a simple cast to float is undefined if outside
                 * the range of values that can be represented */
                afloat = (float)(anv >  FLT_MAX ?  NV_INF :
                                 anv < -FLT_MAX ? -NV_INF : anv);
#  endif
# endif
                PUSH_VAR(utf8, cur, afloat, needs_swap);
            }
            break;
        case 'd':
            while (len-- > 0) {
                double adouble;
                NV anv;
                fromstr = NEXTFROM;
                anv = SvNV(fromstr);
# if (defined(VMS) && !defined(_IEEE_FP)) || defined(DOUBLE_IS_VAX_FLOAT)
                /* IEEE fp overflow shenanigans are unavailable on VAX and optional
                 * on Alpha; fake it if we don't have them.
                 */
                if (anv > DBL_MAX)
                    adouble = DBL_MAX;
                else if (anv < -DBL_MAX)
                    adouble = -DBL_MAX;
                else adouble = (double)anv;
# else
                adouble = (double)anv;
# endif
                PUSH_VAR(utf8, cur, adouble, needs_swap);
            }
            break;
        case 'F': {
            NV_bytes anv;
            Zero(&anv, 1, NV); /* can be long double with unused bits */
            while (len-- > 0) {
                fromstr = NEXTFROM;
#ifdef __GNUC__
                /* to work round a gcc/x86 bug; don't use SvNV */
                anv.nv = sv_2nv(fromstr);
#    if defined(LONGDOUBLE_X86_80_BIT) && defined(USE_LONG_DOUBLE) \
         && LONG_DOUBLESIZE > 10
                /* GCC sometimes overwrites the padding in the
                   assignment above */
                Zero(anv.bytes+10, sizeof(anv.bytes) - 10, U8);
#    endif
#else
                anv.nv = SvNV(fromstr);
#endif
                PUSH_BYTES(utf8, cur, anv.bytes, sizeof(anv.bytes), needs_swap);
            }
            break;
        }
#if defined(HAS_LONG_DOUBLE)
        case 'D': {
            ld_bytes aldouble;
            /* long doubles can have unused bits, which may be nonzero */
            Zero(&aldouble, 1, long double);
            while (len-- > 0) {
                fromstr = NEXTFROM;
#  ifdef __GNUC__
                /* to work round a gcc/x86 bug; don't use SvNV */
                aldouble.ld = (long double)sv_2nv(fromstr);
#    if defined(LONGDOUBLE_X86_80_BIT) && LONG_DOUBLESIZE > 10
                /* GCC sometimes overwrites the padding in the
                   assignment above */
                Zero(aldouble.bytes+10, sizeof(aldouble.bytes) - 10, U8);
#    endif
#  else
                aldouble.ld = (long double)SvNV(fromstr);
#  endif
                PUSH_BYTES(utf8, cur, aldouble.bytes, sizeof(aldouble.bytes),
                           needs_swap);
            }
            break;
        }
#endif
        case 'n' | TYPE_IS_SHRIEKING:
        case 'n':
            while (len-- > 0) {
                I16 ai16;
                fromstr = NEXTFROM;
                ai16 = (I16)SvIV_no_inf(fromstr, datumtype);
                ai16 = PerlSock_htons(ai16);
                PUSH16(utf8, cur, &ai16, FALSE);
            }
            break;
        case 'v' | TYPE_IS_SHRIEKING:
        case 'v':
            while (len-- > 0) {
                I16 ai16;
                fromstr = NEXTFROM;
                ai16 = (I16)SvIV_no_inf(fromstr, datumtype);
                ai16 = htovs(ai16);
                PUSH16(utf8, cur, &ai16, FALSE);
            }
            break;
        case 'S' | TYPE_IS_SHRIEKING:
#if SHORTSIZE != SIZE16
            while (len-- > 0) {
                unsigned short aushort;
                fromstr = NEXTFROM;
                aushort = SvUV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, aushort, needs_swap);
            }
            break;
#else
            /* FALLTHROUGH */
#endif
        case 'S':
            while (len-- > 0) {
                U16 au16;
                fromstr = NEXTFROM;
                au16 = (U16)SvUV_no_inf(fromstr, datumtype);
                PUSH16(utf8, cur, &au16, needs_swap);
            }
            break;
        case 's' | TYPE_IS_SHRIEKING:
#if SHORTSIZE != SIZE16
            while (len-- > 0) {
                short ashort;
                fromstr = NEXTFROM;
                ashort = SvIV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, ashort, needs_swap);
            }
            break;
#else
            /* FALLTHROUGH */
#endif
        case 's':
            while (len-- > 0) {
                I16 ai16;
                fromstr = NEXTFROM;
                ai16 = (I16)SvIV_no_inf(fromstr, datumtype);
                PUSH16(utf8, cur, &ai16, needs_swap);
            }
            break;
        case 'I':
        case 'I' | TYPE_IS_SHRIEKING:
            while (len-- > 0) {
                unsigned int auint;
                fromstr = NEXTFROM;
                auint = SvUV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, auint, needs_swap);
            }
            break;
        case 'j':
            while (len-- > 0) {
                IV aiv;
                fromstr = NEXTFROM;
                aiv = SvIV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, aiv, needs_swap);
            }
            break;
        case 'J':
            while (len-- > 0) {
                UV auv;
                fromstr = NEXTFROM;
                auv = SvUV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, auv, needs_swap);
            }
            break;
        case 'w':
            while (len-- > 0) {
                NV anv;
                fromstr = NEXTFROM;
                S_sv_check_infnan(aTHX_ fromstr, datumtype);
                anv = SvNV_nomg(fromstr);

                if (anv < 0) {
                    *cur = '\0';
                    SvCUR_set(cat, cur - start);
                    Perl_croak(aTHX_ "Cannot compress negative numbers in pack");
                }

                /* 0xFFFFFFFFFFFFFFFF may cast to 18446744073709551616.0,
                   which is == UV_MAX_P1. IOK is fine (instead of UV_only), as
                   any negative IVs will have already been got by the croak()
                   above. IOK is untrue for fractions, so we test them
                   against UV_MAX_P1.  */
                if (SvIOK(fromstr) || anv < UV_MAX_P1) {
                    char   buf[(sizeof(UV)*CHAR_BIT)/7+1];
                    char  *in = buf + sizeof(buf);
                    UV     auv = SvUV_nomg(fromstr);

                    do {
                        *--in = (char)((auv & 0x7f) | 0x80);
                        auv >>= 7;
                    } while (auv);
                    buf[sizeof(buf) - 1] &= 0x7f; /* clear continue bit */
                    PUSH_GROWING_BYTES(utf8, cat, start, cur,
                                       in, (buf + sizeof(buf)) - in);
                } else if (SvPOKp(fromstr))
                    goto w_string;
                else if (SvNOKp(fromstr)) {
                    /* 10**NV_MAX_10_EXP is the largest power of 10
                       so 10**(NV_MAX_10_EXP+1) is definitely unrepresentable
                       given 10**(NV_MAX_10_EXP+1) == 128 ** x solve for x:
                       x = (NV_MAX_10_EXP+1) * log (10) / log (128)
                       And with that many bytes only Inf can overflow.
                       Some C compilers are strict about integral constant
                       expressions so we conservatively divide by a slightly
                       smaller integer instead of multiplying by the exact
                       floating-point value.
                    */
#ifdef NV_MAX_10_EXP
                    /* char   buf[1 + (int)((NV_MAX_10_EXP + 1) * 0.47456)]; -- invalid C */
                    char   buf[1 + (int)((NV_MAX_10_EXP + 1) / 2)]; /* valid C */
#else
                    /* char   buf[1 + (int)((308 + 1) * 0.47456)]; -- invalid C */
                    char   buf[1 + (int)((308 + 1) / 2)]; /* valid C */
#endif
                    char  *in = buf + sizeof(buf);

                    anv = Perl_floor(anv);
                    do {
                        const NV next = Perl_floor(anv / 128);
                        if (in <= buf)  /* this cannot happen ;-) */
                            Perl_croak(aTHX_ "Cannot compress integer in pack");
                        *--in = (unsigned char)(anv - (next * 128)) | 0x80;
                        anv = next;
                    } while (anv > 0);
                    buf[sizeof(buf) - 1] &= 0x7f; /* clear continue bit */
                    PUSH_GROWING_BYTES(utf8, cat, start, cur,
                                       in, (buf + sizeof(buf)) - in);
                } else {
                    const char     *from;
                    char           *result, *in;
                    SV             *norm;
                    STRLEN          len;
                    bool            done;

                  w_string:
                    /* Copy string and check for compliance */
                    from = SvPV_nomg_const(fromstr, len);
                    if ((norm = is_an_int(from, len)) == NULL)
                        Perl_croak(aTHX_ "Can only compress unsigned integers in pack");

                    Newx(result, len, char);
                    in = result + len;
                    done = FALSE;
                    while (!done) *--in = div128(norm, &done) | 0x80;
                    result[len - 1] &= 0x7F; /* clear continue bit */
                    PUSH_GROWING_BYTES(utf8, cat, start, cur,
                                       in, (result + len) - in);
                    Safefree(result);
                    SvREFCNT_dec(norm);	/* free norm */
                }
            }
            break;
        case 'i':
        case 'i' | TYPE_IS_SHRIEKING:
            while (len-- > 0) {
                int aint;
                fromstr = NEXTFROM;
                aint = SvIV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, aint, needs_swap);
            }
            break;
        case 'N' | TYPE_IS_SHRIEKING:
        case 'N':
            while (len-- > 0) {
                U32 au32;
                fromstr = NEXTFROM;
                au32 = SvUV_no_inf(fromstr, datumtype);
                au32 = PerlSock_htonl(au32);
                PUSH32(utf8, cur, &au32, FALSE);
            }
            break;
        case 'V' | TYPE_IS_SHRIEKING:
        case 'V':
            while (len-- > 0) {
                U32 au32;
                fromstr = NEXTFROM;
                au32 = SvUV_no_inf(fromstr, datumtype);
                au32 = htovl(au32);
                PUSH32(utf8, cur, &au32, FALSE);
            }
            break;
        case 'L' | TYPE_IS_SHRIEKING:
#if LONGSIZE != SIZE32
            while (len-- > 0) {
                unsigned long aulong;
                fromstr = NEXTFROM;
                aulong = SvUV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, aulong, needs_swap);
            }
            break;
#else
            /* Fall though! */
#endif
        case 'L':
            while (len-- > 0) {
                U32 au32;
                fromstr = NEXTFROM;
                au32 = SvUV_no_inf(fromstr, datumtype);
                PUSH32(utf8, cur, &au32, needs_swap);
            }
            break;
        case 'l' | TYPE_IS_SHRIEKING:
#if LONGSIZE != SIZE32
            while (len-- > 0) {
                long along;
                fromstr = NEXTFROM;
                along = SvIV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, along, needs_swap);
            }
            break;
#else
            /* Fall though! */
#endif
        case 'l':
            while (len-- > 0) {
                I32 ai32;
                fromstr = NEXTFROM;
                ai32 = SvIV_no_inf(fromstr, datumtype);
                PUSH32(utf8, cur, &ai32, needs_swap);
            }
            break;
#if defined(HAS_QUAD) && IVSIZE >= 8
        case 'Q':
            while (len-- > 0) {
                Uquad_t auquad;
                fromstr = NEXTFROM;
                auquad = (Uquad_t) SvUV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, auquad, needs_swap);
            }
            break;
        case 'q':
            while (len-- > 0) {
                Quad_t aquad;
                fromstr = NEXTFROM;
                aquad = (Quad_t)SvIV_no_inf(fromstr, datumtype);
                PUSH_VAR(utf8, cur, aquad, needs_swap);
            }
            break;
#endif
        case 'P':
            len = 1;		/* assume SV is correct length */
            GROWING(utf8, cat, start, cur, sizeof(char *));
            /* FALLTHROUGH */
        case 'p':
            while (len-- > 0) {
                const char *aptr;

                fromstr = NEXTFROM;
                SvGETMAGIC(fromstr);
                if (!SvOK(fromstr)) aptr = NULL;
                else {
                    /* XXX better yet, could spirit away the string to
                     * a safe spot and hang on to it until the result
                     * of pack() (and all copies of the result) are
                     * gone.
                     */
                    if (((SvTEMP(fromstr) && SvREFCNT(fromstr) == 1)
                         || (SvPADTMP(fromstr) &&
                             !SvREADONLY(fromstr)))) {
                        Perl_ck_warner(aTHX_ packWARN(WARN_PACK),
                                       "Attempt to pack pointer to temporary value");
                    }
                    if (SvPOK(fromstr) || SvNIOK(fromstr))
                        aptr = SvPV_nomg_const_nolen(fromstr);
                    else
                        aptr = SvPV_force_flags_nolen(fromstr, 0);
                }
                PUSH_VAR(utf8, cur, aptr, needs_swap);
            }
            break;
        case 'u': {
            const char *aptr, *aend;
            bool from_utf8;

            fromstr = NEXTFROM;
            if (len <= 2) len = 45;
            else len = len / 3 * 3;
            if (len >= 64) {
                Perl_ck_warner(aTHX_ packWARN(WARN_PACK),
                               "Field too wide in 'u' format in pack");
                len = 63;
            }
            aptr = SvPV_const(fromstr, fromlen);
            from_utf8 = DO_UTF8(fromstr);
            if (from_utf8) {
                aend = aptr + fromlen;
                fromlen = sv_len_utf8_nomg(fromstr);
            } else aend = NULL; /* Unused, but keep compilers happy */
            GROWING(utf8, cat, start, cur, (fromlen+2) / 3 * 4 + (fromlen+len-1)/len * 2);
            while (fromlen > 0) {
                U8 *end;
                SSize_t todo;
                U8 hunk[1+63/3*4+1];

                if ((SSize_t)fromlen > len)
                    todo = len;
                else
                    todo = fromlen;
                if (from_utf8) {
                    char buffer[64];
                    if (!S_utf8_to_bytes(aTHX_ &aptr, aend, buffer, todo,
                                      'u' | TYPE_IS_PACK)) {
                        *cur = '\0';
                        SvCUR_set(cat, cur - start);
                        Perl_croak(aTHX_ "panic: string is shorter than advertised, "
                                   "aptr=%p, aend=%p, buffer=%p, todo=%zd",
                                   aptr, aend, buffer, todo);
                    }
                    end = doencodes(hunk, (const U8 *)buffer, todo);
                } else {
                    end = doencodes(hunk, (const U8 *)aptr, todo);
                    aptr += todo;
                }
                PUSH_BYTES(utf8, cur, hunk, end-hunk, 0);
                fromlen -= todo;
            }
            break;
        }
        }
        *cur = '\0';
        SvCUR_set(cat, cur - start);
      no_change:
        *symptr = lookahead;
    }
    return beglist;
}
#undef NEXTFROM


PP(pp_pack)
{
    dSP; dMARK; dORIGMARK; dTARGET;
    SV *cat = TARG;
    STRLEN fromlen;
    SV *pat_sv = *++MARK;
    const char *pat = SvPV_const(pat_sv, fromlen);
    const char *patend = pat + fromlen;

    MARK++;
    SvPVCLEAR(cat);
    SvUTF8_off(cat);

    packlist(cat, pat, patend, MARK, SP + 1);

    if (SvUTF8(cat)) {
        STRLEN result_len;
        const char * result = SvPV_nomg(cat, result_len);
        const U8 * error_pos;

        if (! is_utf8_string_loc((U8 *) result, result_len, &error_pos)) {
            _force_out_malformed_utf8_message(error_pos,
                                              (U8 *) result + result_len,
                                              0, /* no flags */
                                              1 /* Die */
                                            );
            NOT_REACHED; /* NOTREACHED */
        }
    }

    SvSETMAGIC(cat);
    SP = ORIGMARK;
    PUSHs(cat);
    RETURN;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
