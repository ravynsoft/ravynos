/*    regexp.h
 *
 *    Copyright (C) 1993, 1994, 1996, 1997, 1999, 2000, 2001, 2003,
 *    2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */
#ifndef PLUGGABLE_RE_EXTENSION
/* we don't want to include this stuff if we are inside of
   an external regex engine based on the core one - like re 'debug'*/

#  include "utf8.h"

typedef SSize_t regnode_offset;

struct regnode_meta {
    U8 type;
    U8 arg_len;
    U8 arg_len_varies;
    U8 off_by_arg;
};

/* this ensures that on alignment sensitive platforms
 * this struct is aligned on 32 bit boundaries */
union regnode_head {
    struct {
        union {
            U8 flags;
            U8 str_len_u8;
            U8 first_byte;
        } u_8;
        U8  type;
        U16 next_off;
    } data;
    U32 data_u32;
};

struct regnode {
    union regnode_head head;
};

typedef struct regnode regnode;

struct reg_substr_data;

struct reg_data;

struct regexp_engine;
struct regexp;

struct reg_substr_datum {
    SSize_t min_offset; /* min pos (in chars) that substr must appear */
    SSize_t max_offset; /* max pos (in chars) that substr must appear */
    SV *substr;		/* non-utf8 variant */
    SV *utf8_substr;	/* utf8 variant */
    SSize_t end_shift;  /* how many fixed chars must end the string */
};
struct reg_substr_data {
    U8      check_ix;   /* index into data[] of check substr */
    struct reg_substr_datum data[3];	/* Actual array */
};

#  ifdef PERL_ANY_COW
#    define SV_SAVED_COPY   SV *saved_copy; /* If non-NULL, SV which is COW from original */
#  else
#    define SV_SAVED_COPY
#  endif

/* offsets within a string of a particular /(.)/ capture
 * if you change this by adding new non-temporary fields
 * then be sure to update Perl_rxres_save() in pp_ctl.c */
typedef struct regexp_paren_pair {
    SSize_t start;
    SSize_t end;

    /* 'start_tmp' records a new opening position before the matching end
     * has been found, so that the old start and end values are still
     * valid, e.g.
     *	  "abc" =~ /(.(?{print "[$1]"}))+/
     *outputs [][a][b]
     * This field is not part of the API.  */
    SSize_t start_tmp;
} regexp_paren_pair;

#  if defined(PERL_IN_REGCOMP_ANY) || defined(PERL_IN_UTF8_C)
#    define _invlist_union(a, b, output) _invlist_union_maybe_complement_2nd(a, b, FALSE, output)
#    define _invlist_intersection(a, b, output) _invlist_intersection_maybe_complement_2nd(a, b, FALSE, output)

/* Subtracting b from a leaves in a everything that was there that isn't in b,
 * that is the intersection of a with b's complement */
#    define _invlist_subtract(a, b, output) _invlist_intersection_maybe_complement_2nd(a, b, TRUE, output)
#  endif

/* record the position of a (?{...}) within a pattern */

struct reg_code_block {
    STRLEN start;
    STRLEN end;
    OP     *block;
    REGEXP *src_regex;
};

/* array of reg_code_block's plus header info */

struct reg_code_blocks {
    int refcnt; /* we may be pointed to from a regex and from the savestack */
    int  count;    /* how many code blocks */
    struct reg_code_block *cb; /* array of reg_code_block's */
};


/*
= for apidoc AyT||regexp
  The regexp/REGEXP struct, see L<perlreapi> for further documentation
  on the individual fields. The struct is ordered so that the most
  commonly used fields are placed at the start.

  Any patch that adds items to this struct will need to include
  changes to F<sv.c> (C<Perl_re_dup()>) and F<regcomp.c>
  (C<pregfree()>). This involves freeing or cloning items in the
  regexp's data array based on the data item's type.
*/

typedef struct regexp {
    _XPV_HEAD;
    const struct regexp_engine* engine; /* what engine created this regexp? */
    REGEXP *mother_re; /* what re is this a lightweight copy of? */
    HV *paren_names;   /* Optional hash of paren names */

    /*----------------------------------------------------------------------
     * Information about the match that the perl core uses to manage things
     */

    /* see comment in regcomp_internal.h about branch reset to understand
       the distinction between physical and logical capture buffers */
    U32 nparens;                    /* physical number of capture buffers */
    U32 logical_nparens;            /* logical_number of capture buffers */
    I32 *logical_to_parno;          /* map logical parno to first physcial */
    I32 *parno_to_logical;          /* map every physical parno to logical */
    I32 *parno_to_logical_next;     /* map every physical parno to the next
                                       physical with the same logical id */

    U32 extflags;      /* Flags used both externally and internally */
    SSize_t maxlen;    /* maximum possible number of chars in string to match */
    SSize_t minlen;    /* minimum possible number of chars in string to match */
    SSize_t minlenret; /* minimum possible number of chars in $& */
    STRLEN gofs;       /* chars left of pos that we search from */
                       /* substring data about strings that must appear in
                        * the final match, used for optimisations */

    struct reg_substr_data *substrs;

    /* private engine specific data */

    void *pprivate;    /* Data private to the regex engine which
                        * created this object. */
    U32 intflags;      /* Engine Specific Internal flags */

    /*----------------------------------------------------------------------
     * Data about the last/current match. These are modified during matching
     */

    U32 lastparen;           /* highest close paren matched ($+) */
    regexp_paren_pair *offs; /* Array of offsets for (@-) and (@+) */
    char **recurse_locinput; /* used to detect infinite recursion, XXX: move to internal */
    U32 lastcloseparen;      /* last close paren matched ($^N) */


    /*---------------------------------------------------------------------- */

    /* offset from wrapped to the start of precomp */
    PERL_BITFIELD32 pre_prefix:4;

    /* original flags used to compile the pattern, may differ from
     * extflags in various ways */
    PERL_BITFIELD32 compflags:9;

    /*---------------------------------------------------------------------- */

    char *subbeg;       /* saved or original string so \digit works forever. */
    SV_SAVED_COPY       /* If non-NULL, SV which is COW from original */
    SSize_t sublen;     /* Length of string pointed by subbeg */
    SSize_t suboffset;  /* byte offset of subbeg from logical start of str */
    SSize_t subcoffset; /* suboffset equiv, but in chars (for @-/@+) */

    /*---------------------------------------------------------------------- */


    CV *qr_anoncv;      /* the anon sub wrapped round qr/(?{..})/ */
} regexp;


#define RXp_PAREN_NAMES(rx) ((rx)->paren_names)

#define RXp_OFFS_START(rx,n) \
     RXp_OFFSp(rx)[(n)].start 

#define RXp_OFFS_END(rx,n) \
     RXp_OFFSp(rx)[(n)].end 

#define RXp_OFFS_VALID(rx,n) \
     (RXp_OFFSp(rx)[(n)].end != -1 && RXp_OFFSp(rx)[(n)].start != -1 )

#define RX_OFFS_START(rx_sv,n)  RXp_OFFS_START(ReANY(rx_sv),n)
#define RX_OFFS_END(rx_sv,n)    RXp_OFFS_END(ReANY(rx_sv),n)
#define RX_OFFS_VALID(rx_sv,n)  RXp_OFFS_VALID(ReANY(rx_sv),n)

/* used for high speed searches */
typedef struct re_scream_pos_data_s
{
    char **scream_olds;		/* match pos */
    SSize_t *scream_pos;	/* Internal iterator of scream. */
} re_scream_pos_data;

/* regexp_engine structure. This is the dispatch table for regexes.
 * Any regex engine implementation must be able to build one of these.
 */
typedef struct regexp_engine {
    REGEXP* (*comp) (pTHX_ SV * const pattern, U32 flags);
    I32     (*exec) (pTHX_ REGEXP * const rx, char* stringarg, char* strend,
                     char* strbeg, SSize_t minend, SV* sv,
                     void* data, U32 flags);
    char*   (*intuit) (pTHX_
                        REGEXP * const rx,
                        SV *sv,
                        const char * const strbeg,
                        char *strpos,
                        char *strend,
                        const U32 flags,
                       re_scream_pos_data *data);
    SV*     (*checkstr) (pTHX_ REGEXP * const rx);
    void    (*rxfree) (pTHX_ REGEXP * const rx);
    void    (*numbered_buff_FETCH) (pTHX_ REGEXP * const rx, const I32 paren,
                                    SV * const sv);
    void    (*numbered_buff_STORE) (pTHX_ REGEXP * const rx, const I32 paren,
                                   SV const * const value);
    I32     (*numbered_buff_LENGTH) (pTHX_ REGEXP * const rx, const SV * const sv,
                                    const I32 paren);
    SV*     (*named_buff) (pTHX_ REGEXP * const rx, SV * const key,
                           SV * const value, const U32 flags);
    SV*     (*named_buff_iter) (pTHX_ REGEXP * const rx, const SV * const lastkey,
                                const U32 flags);
    SV*     (*qr_package)(pTHX_ REGEXP * const rx);
#  ifdef USE_ITHREADS
    void*   (*dupe) (pTHX_ REGEXP * const rx, CLONE_PARAMS *param);
#  endif
    REGEXP* (*op_comp) (pTHX_ SV ** const patternp, int pat_count,
                    OP *expr, const struct regexp_engine* eng,
                    REGEXP *old_re,
                    bool *is_bare_re, U32 orig_rx_flags, U32 pm_flags);
} regexp_engine;

/*
  These are passed to the numbered capture variable callbacks as the
  paren name. >= 1 is reserved for actual numbered captures, i.e. $1,
  $2 etc.
*/
#  define RX_BUFF_IDX_CARET_PREMATCH  -5 /* ${^PREMATCH}  */
#  define RX_BUFF_IDX_CARET_POSTMATCH -4 /* ${^POSTMATCH} */
#  define RX_BUFF_IDX_CARET_FULLMATCH -3 /* ${^MATCH}     */
#  define RX_BUFF_IDX_PREMATCH        -2 /* $` */
#  define RX_BUFF_IDX_POSTMATCH       -1 /* $' */
#  define RX_BUFF_IDX_FULLMATCH        0 /* $& */

/*
  Flags that are passed to the named_buff and named_buff_iter
  callbacks above. Those routines are called from universal.c via the
  Tie::Hash::NamedCapture interface for %+ and %- and the re::
  functions in the same file.
*/

/* The Tie::Hash::NamedCapture operation this is part of, if any */
#  define RXapif_FETCH     0x0001
#  define RXapif_STORE     0x0002
#  define RXapif_DELETE    0x0004
#  define RXapif_CLEAR     0x0008
#  define RXapif_EXISTS    0x0010
#  define RXapif_SCALAR    0x0020
#  define RXapif_FIRSTKEY  0x0040
#  define RXapif_NEXTKEY   0x0080

/* Whether %+ or %- is being operated on */
#  define RXapif_ONE       0x0100 /* %+ */
#  define RXapif_ALL       0x0200 /* %- */

/* Whether this is being called from a re:: function */
#  define RXapif_REGNAME         0x0400
#  define RXapif_REGNAMES        0x0800
#  define RXapif_REGNAMES_COUNT  0x1000

/*
=for apidoc Am|REGEXP *|SvRX|SV *sv

Convenience macro to get the REGEXP from a SV.  This is approximately
equivalent to the following snippet:

    if (SvMAGICAL(sv))
        mg_get(sv);
    if (SvROK(sv))
        sv = MUTABLE_SV(SvRV(sv));
    if (SvTYPE(sv) == SVt_REGEXP)
        return (REGEXP*) sv;

C<NULL> will be returned if a REGEXP* is not found.

=for apidoc Am|bool|SvRXOK|SV* sv

Returns a boolean indicating whether the SV (or the one it references)
is a REGEXP.

If you want to do something with the REGEXP* later use SvRX instead
and check for NULL.

=cut
*/

#  define SvRX(sv)   (Perl_get_re_arg(aTHX_ sv))
#  define SvRXOK(sv) cBOOL(Perl_get_re_arg(aTHX_ sv))


/* Flags stored in regexp->extflags
 * These are used by code external to the regexp engine
 *
 * Note that the flags whose names start with RXf_PMf_ are defined in
 * op_reg_common.h, being copied from the parallel flags of op_pmflags
 *
 * NOTE: if you modify any RXf flags you should run regen.pl or
 * regen/regcomp.pl so that regnodes.h is updated with the changes.
 *
 */

#  include "op_reg_common.h"

#  define RXf_PMf_STD_PMMOD	(RXf_PMf_MULTILINE|RXf_PMf_SINGLELINE|RXf_PMf_FOLD|RXf_PMf_EXTENDED|RXf_PMf_EXTENDED_MORE|RXf_PMf_NOCAPTURE)

#  define CASE_STD_PMMOD_FLAGS_PARSE_SET(pmfl, x_count)                       \
    case IGNORE_PAT_MOD:    *(pmfl) |= RXf_PMf_FOLD;       break;           \
    case MULTILINE_PAT_MOD: *(pmfl) |= RXf_PMf_MULTILINE;  break;           \
    case SINGLE_PAT_MOD:    *(pmfl) |= RXf_PMf_SINGLELINE; break;           \
    case XTENDED_PAT_MOD:   if (x_count == 0) {                             \
                                *(pmfl) |= RXf_PMf_EXTENDED;                \
                                *(pmfl) &= ~RXf_PMf_EXTENDED_MORE;          \
                            }                                               \
                            else {                                          \
                                *(pmfl) |= RXf_PMf_EXTENDED                 \
                                          |RXf_PMf_EXTENDED_MORE;           \
                            }                                               \
                            (x_count)++; break;                             \
    case NOCAPTURE_PAT_MOD: *(pmfl) |= RXf_PMf_NOCAPTURE; break;

/* Note, includes charset ones, assumes 0 is the default for them */
#  define STD_PMMOD_FLAGS_CLEAR(pmfl)                        \
    *(pmfl) &= ~(RXf_PMf_FOLD|RXf_PMf_MULTILINE|RXf_PMf_SINGLELINE|RXf_PMf_EXTENDED|RXf_PMf_EXTENDED_MORE|RXf_PMf_CHARSET|RXf_PMf_NOCAPTURE)

/* chars and strings used as regex pattern modifiers
 * Singular is a 'c'har, plural is a "string"
 *
 * NOTE, KEEPCOPY was originally 'k', but was changed to 'p' for preserve
 * for compatibility reasons with Regexp::Common which highjacked (?k:...)
 * for its own uses. So 'k' is out as well.
 */
#  define DEFAULT_PAT_MOD      '^'    /* Short for all the default modifiers */
#  define EXEC_PAT_MOD         'e'
#  define KEEPCOPY_PAT_MOD     'p'
#  define NOCAPTURE_PAT_MOD    'n'
#  define ONCE_PAT_MOD         'o'
#  define GLOBAL_PAT_MOD       'g'
#  define CONTINUE_PAT_MOD     'c'
#  define MULTILINE_PAT_MOD    'm'
#  define SINGLE_PAT_MOD       's'
#  define IGNORE_PAT_MOD       'i'
#  define XTENDED_PAT_MOD      'x'
#  define NONDESTRUCT_PAT_MOD  'r'
#  define LOCALE_PAT_MOD       'l'
#  define UNICODE_PAT_MOD      'u'
#  define DEPENDS_PAT_MOD      'd'
#  define ASCII_RESTRICT_PAT_MOD 'a'

#  define ONCE_PAT_MODS        "o"
#  define KEEPCOPY_PAT_MODS    "p"
#  define NOCAPTURE_PAT_MODS   "n"
#  define EXEC_PAT_MODS        "e"
#  define LOOP_PAT_MODS        "gc"
#  define NONDESTRUCT_PAT_MODS "r"
#  define LOCALE_PAT_MODS      "l"
#  define UNICODE_PAT_MODS     "u"
#  define DEPENDS_PAT_MODS     "d"
#  define ASCII_RESTRICT_PAT_MODS "a"
#  define ASCII_MORE_RESTRICT_PAT_MODS "aa"

/* This string is expected by regcomp.c to be ordered so that the first
 * character is the flag in bit RXf_PMf_STD_PMMOD_SHIFT of extflags; the next
 * character is bit +1, etc. */
#  define STD_PAT_MODS        "msixxn"

#  define CHARSET_PAT_MODS    ASCII_RESTRICT_PAT_MODS DEPENDS_PAT_MODS LOCALE_PAT_MODS UNICODE_PAT_MODS

/* This string is expected by XS_re_regexp_pattern() in universal.c to be ordered
 * so that the first character is the flag in bit RXf_PMf_STD_PMMOD_SHIFT of
 * extflags; the next character is in bit +1, etc. */
#  define INT_PAT_MODS    STD_PAT_MODS    KEEPCOPY_PAT_MODS

#  define EXT_PAT_MODS    ONCE_PAT_MODS   KEEPCOPY_PAT_MODS  NOCAPTURE_PAT_MODS
#  define QR_PAT_MODS     STD_PAT_MODS    EXT_PAT_MODS	   CHARSET_PAT_MODS
#  define M_PAT_MODS      QR_PAT_MODS     LOOP_PAT_MODS
#  define S_PAT_MODS      M_PAT_MODS      EXEC_PAT_MODS      NONDESTRUCT_PAT_MODS

/*
 * NOTE: if you modify any RXf flags you should run regen.pl or
 * regen/regcomp.pl so that regnodes.h is updated with the changes.
 *
 */

/*
  Set in Perl_pmruntime for a split. Will be used by regex engines to
  check whether they should set RXf_SKIPWHITE
*/
#  define RXf_SPLIT   RXf_PMf_SPLIT

/* Currently the regex flags occupy a single 32-bit word.  Not all bits are
 * currently used.  The lower bits are shared with their corresponding PMf flag
 * bits, up to but not including _RXf_PMf_SHIFT_NEXT.  The unused bits
 * immediately follow; finally the used RXf-only (unshared) bits, so that the
 * highest bit in the word is used.  This gathers all the unused bits as a pool
 * in the middle, like so: 11111111111111110000001111111111
 * where the '1's represent used bits, and the '0's unused.  This design allows
 * us to allocate off one end of the pool if we need to add a shared bit, and
 * off the other end if we need a non-shared bit, without disturbing the other
 * bits.  This maximizes the likelihood of being able to change things without
 * breaking binary compatibility.
 *
 * To add shared bits, do so in op_reg_common.h.  This should change
 * _RXf_PMf_SHIFT_NEXT so that things won't compile.  Then come to regexp.h and
 * op.h and adjust the constant adders in the definitions of RXf_BASE_SHIFT and
 * Pmf_BASE_SHIFT down by the number of shared bits you added.  That's it.
 * Things should be binary compatible.  But if either of these gets to having
 * to subtract rather than add, leave at 0 and instead adjust all the entries
 * that are in terms of it.  But if the first one of those is already
 * RXf_BASE_SHIFT+0, there are no bits left, and a redesign is in order.
 *
 * To remove unshared bits, just delete its entry.  If you're where breaking
 * binary compatibility is ok to do, you might want to adjust things to move
 * the newly opened space so that it gets absorbed into the common pool.
 *
 * To add unshared bits, first use up any gaps in the middle.  Otherwise,
 * allocate off the low end until you get to RXf_BASE_SHIFT+0.  If that isn't
 * enough, move RXf_BASE_SHIFT down (if possible) and add the new bit at the
 * other end instead; this preserves binary compatibility.
 *
 * For the regexp bits, PL_reg_extflags_name[] in regnodes.h has a comment
 * giving which bits are used/unused */

#  define RXf_BASE_SHIFT (_RXf_PMf_SHIFT_NEXT + 2)

/* What we have seen */
#  define RXf_NO_INPLACE_SUBST  (1U<<(RXf_BASE_SHIFT+2))
#  define RXf_EVAL_SEEN   	(1U<<(RXf_BASE_SHIFT+3))

/* Special */
#  define RXf_UNBOUNDED_QUANTIFIER_SEEN   (1U<<(RXf_BASE_SHIFT+4))
#  define RXf_CHECK_ALL   	(1U<<(RXf_BASE_SHIFT+5))

/* UTF8 related */
#  define RXf_MATCH_UTF8  	(1U<<(RXf_BASE_SHIFT+6)) /* $1 etc are utf8 */

/* Intuit related */
#  define RXf_USE_INTUIT_NOML	(1U<<(RXf_BASE_SHIFT+7))
#  define RXf_USE_INTUIT_ML	(1U<<(RXf_BASE_SHIFT+8))
#  define RXf_INTUIT_TAIL 	(1U<<(RXf_BASE_SHIFT+9))
#  define RXf_USE_INTUIT        (RXf_USE_INTUIT_NOML|RXf_USE_INTUIT_ML)

/* Do we have some sort of anchor? */
#  define RXf_IS_ANCHORED       (1U<<(RXf_BASE_SHIFT+10))

/* Copy and tainted info */
#  define RXf_COPY_DONE   	(1U<<(RXf_BASE_SHIFT+11))

/* post-execution: $1 et al are tainted */
#  define RXf_TAINTED_SEEN	(1U<<(RXf_BASE_SHIFT+12))
/* this pattern was tainted during compilation */
#  define RXf_TAINTED		(1U<<(RXf_BASE_SHIFT+13))

/* Flags indicating special patterns */
#  define RXf_START_ONLY        (1U<<(RXf_BASE_SHIFT+14)) /* Pattern is /^/ */
#  define RXf_SKIPWHITE         (1U<<(RXf_BASE_SHIFT+15)) /* Pattern is for a */
                                                          /* split " " */
#  define RXf_WHITE		(1U<<(RXf_BASE_SHIFT+16)) /* Pattern is /\s+/ */
#  define RXf_NULL		(1U<<(RXf_BASE_SHIFT+17)) /* Pattern is // */

/* See comments at the beginning of these defines about adding bits.  The
 * highest bit position should be used, so that if RXf_BASE_SHIFT gets
 * increased, the #error below will be triggered so that you will be reminded
 * to adjust things at the other end to keep the bit positions unchanged */
#  if RXf_BASE_SHIFT+17 > 31
#     error Too many RXf_PMf bits used.  See comments at beginning of these for what to do
#  endif

/*
 * NOTE: if you modify any RXf flags you should run regen.pl or
 * regen/regcomp.pl so that regnodes.h is updated with the changes.
 *
 */

#  ifdef NO_TAINT_SUPPORT
#    define RX_ISTAINTED(rx_sv)           0
#    define RXp_ISTAINTED(prog)           0
#    define RX_TAINT_on(rx_sv)            NOOP
#    define RXp_MATCH_TAINTED(prog)       0
#    define RX_MATCH_TAINTED(rx_sv)       0
#    define RXp_MATCH_TAINTED_on(prog)    NOOP
#    define RX_MATCH_TAINTED_on(rx_sv)    NOOP
#    define RXp_MATCH_TAINTED_off(prog)   NOOP
#    define RX_MATCH_TAINTED_off(rx_sv)   NOOP
#  else
#    define RX_ISTAINTED(rx_sv)           (RX_EXTFLAGS(rx_sv) & RXf_TAINTED)
#    define RXp_ISTAINTED(prog)           (RXp_EXTFLAGS(prog) & RXf_TAINTED)
#    define RX_TAINT_on(rx_sv)            (RX_EXTFLAGS(rx_sv) |= RXf_TAINTED)
#    define RXp_MATCH_TAINTED(prog)       (RXp_EXTFLAGS(prog) & RXf_TAINTED_SEEN)
#    define RX_MATCH_TAINTED(rx_sv)       (RX_EXTFLAGS(rx_sv) & RXf_TAINTED_SEEN)
#    define RXp_MATCH_TAINTED_on(prog)    (RXp_EXTFLAGS(prog) |= RXf_TAINTED_SEEN)
#    define RX_MATCH_TAINTED_on(rx_sv)    (RX_EXTFLAGS(rx_sv) |= RXf_TAINTED_SEEN)
#    define RXp_MATCH_TAINTED_off(prog)   (RXp_EXTFLAGS(prog) &= ~RXf_TAINTED_SEEN)
#    define RX_MATCH_TAINTED_off(rx_sv)   (RX_EXTFLAGS(rx_sv) &= ~RXf_TAINTED_SEEN)
#  endif

#  define RXp_HAS_CUTGROUP(prog)          ((prog)->intflags & PREGf_CUTGROUP_SEEN)

#  define RX_MATCH_TAINTED_set(rx_sv, t)  ((t) \
                                        ? RX_MATCH_TAINTED_on(rx_sv) \
                                        : RX_MATCH_TAINTED_off(rx_sv))

#  define RXp_MATCH_COPIED(prog)          (RXp_EXTFLAGS(prog) & RXf_COPY_DONE)
#  define RX_MATCH_COPIED(rx_sv)          (RX_EXTFLAGS(rx_sv) & RXf_COPY_DONE)
#  define RXp_MATCH_COPIED_on(prog)       (RXp_EXTFLAGS(prog) |= RXf_COPY_DONE)
#  define RX_MATCH_COPIED_on(rx_sv)       (RX_EXTFLAGS(rx_sv) |= RXf_COPY_DONE)
#  define RXp_MATCH_COPIED_off(prog)      (RXp_EXTFLAGS(prog) &= ~RXf_COPY_DONE)
#  define RX_MATCH_COPIED_off(rx_sv)      (RX_EXTFLAGS(rx_sv) &= ~RXf_COPY_DONE)
#  define RX_MATCH_COPIED_set(rx_sv,t)    ((t) \
                                         ? RX_MATCH_COPIED_on(rx_sv) \
                                         : RX_MATCH_COPIED_off(rx_sv))

#  define RXp_EXTFLAGS(rx)                ((rx)->extflags)
#  define RXp_COMPFLAGS(rx)               ((rx)->compflags)

/* For source compatibility. We used to store these explicitly.  */
#  define RX_PRECOMP(rx_sv)              (RX_WRAPPED(rx_sv) \
                                            + ReANY(rx_sv)->pre_prefix)
#  define RX_PRECOMP_const(rx_sv)        (RX_WRAPPED_const(rx_sv) \
                                            + ReANY(rx_sv)->pre_prefix)
/* FIXME? Are we hardcoding too much here and constraining plugin extension
   writers? Specifically, the value 1 assumes that the wrapped version always
   has exactly one character at the end, a ')'. Will that always be true?  */
#  define RX_PRELEN(rx_sv)                (RX_WRAPLEN(rx_sv) \
                                            - ReANY(rx_sv)->pre_prefix - 1)

#  define RX_WRAPPED(rx_sv)               SvPVX(rx_sv)
#  define RX_WRAPPED_const(rx_sv)         SvPVX_const(rx_sv)
#  define RX_WRAPLEN(rx_sv)               SvCUR(rx_sv)
#  define RX_CHECK_SUBSTR(rx_sv)          (ReANY(rx_sv)->check_substr)
#  define RX_REFCNT(rx_sv)                SvREFCNT(rx_sv)
#  define RX_EXTFLAGS(rx_sv)              RXp_EXTFLAGS(ReANY(rx_sv))
#  define RX_COMPFLAGS(rx_sv)             RXp_COMPFLAGS(ReANY(rx_sv))
#  define RXp_ENGINE(prog)                ((prog)->engine)
#  define RX_ENGINE(rx_sv)                (RXp_ENGINE(ReANY(rx_sv)))
#  define RXp_SUBBEG(prog)                ((prog)->subbeg)
#  define RX_SUBBEG(rx_sv)                (RXp_SUBBEG(ReANY(rx_sv)))
#  define RXp_SUBOFFSET(prog)             ((prog)->suboffset)
#  define RX_SUBOFFSET(rx_sv)             (RXp_SUBOFFSET(ReANY(rx_sv)))
#  define RXp_SUBCOFFSET(prog)            ((prog)->subcoffset)
#  define RX_SUBCOFFSET(rx_sv)            (RXp_SUBCOFFSET(ReANY(rx_sv)))
#  define RXp_OFFSp(prog)                 ((prog)->offs)
#  define RX_OFFSp(rx_sv)                 (RXp_OFFSp(ReANY(rx_sv)))
#  define RXp_LOGICAL_NPARENS(prog)       ((prog)->logical_nparens)
#  define RX_LOGICAL_NPARENS(rx_sv)       (RXp_LOGICAL_NPARENS(ReANY(rx_sv)))
#  define RXp_LOGICAL_TO_PARNO(prog)      ((prog)->logical_to_parno)
#  define RX_LOGICAL_TO_PARNO(rx_sv)      (RXp_LOGICAL_TO_PARNO(ReANY(rx_sv)))
#  define RXp_PARNO_TO_LOGICAL(prog)      ((prog)->parno_to_logical)
#  define RX_PARNO_TO_LOGICAL(rx_sv)      (RXp_PARNO_TO_LOGICAL(ReANY(rx_sv)))
#  define RXp_PARNO_TO_LOGICAL_NEXT(prog) ((prog)->parno_to_logical_next)
#  define RX_PARNO_TO_LOGICAL_NEXT(rx_sv) (RXp_PARNO_TO_LOGICAL_NEXT(ReANY(rx_sv)))
#  define RXp_NPARENS(prog)               ((prog)->nparens)
#  define RX_NPARENS(rx_sv)               (RXp_NPARENS(ReANY(rx_sv)))
#  define RXp_SUBLEN(prog)                ((prog)->sublen)
#  define RX_SUBLEN(rx_sv)                (RXp_SUBLEN(ReANY(rx_sv)))
#  define RXp_MINLEN(prog)                ((prog)->minlen)
#  define RX_MINLEN(rx_sv)                (RXp_MINLEN(ReANY(rx_sv)))
#  define RXp_MINLENRET(prog)             ((prog)->minlenret)
#  define RX_MINLENRET(rx_sv)             (RXp_MINLENRET(ReANY(rx_sv)))
#  define RXp_GOFS(prog)                  ((prog)->gofs)
#  define RX_GOFS(rx_sv)                  (RXp_GOFS(ReANY(rx_sv)))
#  define RXp_LASTPAREN(prog)             ((prog)->lastparen)
#  define RX_LASTPAREN(rx_sv)             (RXp_LASTPAREN(ReANY(rx_sv)))
#  define RXp_LASTCLOSEPAREN(prog)        ((prog)->lastcloseparen)
#  define RX_LASTCLOSEPAREN(rx_sv)        (RXp_LASTCLOSEPAREN(ReANY(rx_sv)))
#  define RXp_SAVED_COPY(prog)            ((prog)->saved_copy)
#  define RX_SAVED_COPY(rx_sv)            (RXp_SAVED_COPY(ReANY(rx_sv)))
#  define RXp_SUBSTRS(prog)               ((prog)->substrs)
#  define RX_SUBSTRS(rx_sv)               (RXp_SUBSTRS(ReANY(rx_sv)))
#  define RXp_PPRIVATE(prog)              ((prog)->pprivate)
#  define RX_PPRIVATE(rx_sv)              (RXp_PPRIVATE(ReANY(rx_sv)))
#  define RXp_QR_ANONCV(prog)             ((prog)->qr_anoncv)
#  define RX_QR_ANONCV(rx_sv)             (RXp_QR_ANONCV(ReANY(rx_sv)))
#  define RXp_MOTHER_RE(prog)             ((prog)->mother_re)
#  define RX_MOTHER_RE(rx_sv)             (RXp_MOTHER_RE(ReANY(rx_sv)))
#  define RXp_PRE_PREFIX(prog)            ((prog)->pre_prefix)
#  define RX_PRE_PREFIX(rx_sv)            (RXp_PRE_PREFIX(ReANY(rx_sv)))

/* last match was zero-length */
#  define RXp_ZERO_LEN(prog) \
        (RXp_OFFS_START(prog,0) + (SSize_t)RXp_GOFS(prog) \
          == RXp_OFFS_END(prog,0))
#  define RX_ZERO_LEN(rx_sv)              (RXp_ZERO_LEN(ReANY(rx_sv)))

#endif /* PLUGGABLE_RE_EXTENSION */

/* Stuff that needs to be included in the pluggable extension goes below here */

#ifdef PERL_ANY_COW
#  define RXp_MATCH_COPY_FREE(prog)                                 \
    STMT_START {                                                    \
        if (RXp_SAVED_COPY(prog)) {                                 \
            SV_CHECK_THINKFIRST_COW_DROP(RXp_SAVED_COPY(prog));     \
        }                                                           \
        if (RXp_MATCH_COPIED(prog)) {                               \
            Safefree(RXp_SUBBEG(prog));                             \
            RXp_MATCH_COPIED_off(prog);                             \
        }                                                           \
    } STMT_END
#else
#  define RXp_MATCH_COPY_FREE(prog)                     \
    STMT_START {                                        \
        if (RXp_MATCH_COPIED(prog)) {                   \
            Safefree(RXp_SUBBEG(prog));                 \
            RXp_MATCH_COPIED_off(prog);                 \
        }                                               \
    } STMT_END
#endif
#define RX_MATCH_COPY_FREE(rx_sv)       RXp_MATCH_COPY_FREE(ReANY(rx_sv))

#define RXp_MATCH_UTF8(prog)            (RXp_EXTFLAGS(prog) & RXf_MATCH_UTF8)
#define RX_MATCH_UTF8(rx_sv)            (RX_EXTFLAGS(rx_sv) & RXf_MATCH_UTF8)
#define RXp_MATCH_UTF8_on(prog)         (RXp_EXTFLAGS(prog) |= RXf_MATCH_UTF8)
#define RX_MATCH_UTF8_on(rx_sv)         (RXp_MATCH_UTF8_on(ReANY(rx_sv)))
#define RXp_MATCH_UTF8_off(prog)        (RXp_EXTFLAGS(prog) &= ~RXf_MATCH_UTF8)
#define RX_MATCH_UTF8_off(rx_sv)        (RXp_MATCH_UTF8_off(ReANY(rx_sv))
#define RXp_MATCH_UTF8_set(prog, t)     ((t) \
                                        ? RXp_MATCH_UTF8_on(prog) \
                                        : RXp_MATCH_UTF8_off(prog))
#define RX_MATCH_UTF8_set(rx_sv, t)     (RXp_MATCH_UTF8_set(ReANY(rx_sv), t))

/* Whether the pattern stored at RX_WRAPPED is in UTF-8  */
#define RX_UTF8(rx_sv)                  SvUTF8(rx_sv)


/* bits in flags arg of Perl_regexec_flags() */

#define REXEC_COPY_STR  0x01    /* Need to copy the string for captures. */
#define REXEC_CHECKED   0x02    /* re_intuit_start() already called. */
#define REXEC_SCREAM    0x04    /* currently unused. */
#define REXEC_IGNOREPOS 0x08    /* use stringarg, not pos(), for \G match */
#define REXEC_NOT_FIRST 0x10    /* This is another iteration of //g:
                                   no need to copy string again */

                                     /* under REXEC_COPY_STR, it's ok for the
                                        engine (modulo PL_sawamperand etc)
                                        to skip copying: ... */
#define REXEC_COPY_SKIP_PRE  0x20    /* ...the $` part of the string, or */
#define REXEC_COPY_SKIP_POST 0x40    /* ...the $' part of the string */
#define REXEC_FAIL_ON_UNDERFLOW 0x80 /* fail the match if $& would start before
                                        the start pos (so s/.\G// would fail
                                        on second iteration */

#if defined(PERL_USE_GCC_BRACE_GROUPS)
#  define ReREFCNT_inc(re)						\
    ({									\
        /* This is here to generate a casting warning if incorrect.  */	\
        REGEXP *const _rerefcnt_inc = (re);				\
        assert(SvTYPE(_rerefcnt_inc) == SVt_REGEXP);			\
        SvREFCNT_inc(_rerefcnt_inc);					\
        _rerefcnt_inc;							\
    })
#  define ReREFCNT_dec(re)						\
    ({									\
        /* This is here to generate a casting warning if incorrect.  */	\
        REGEXP *const _rerefcnt_dec = (re);				\
        SvREFCNT_dec(_rerefcnt_dec);					\
    })
#else
#  define ReREFCNT_dec(re)	SvREFCNT_dec(re)
#  define ReREFCNT_inc(re)	((REGEXP *) SvREFCNT_inc(re))
#endif
#define ReANY(re)		Perl_ReANY((const REGEXP *)(re))

/* FIXME for plugins. */

#define FBMcf_TAIL_DOLLAR	1
#define FBMcf_TAIL_DOLLARM	2
#define FBMcf_TAIL_Z		4
#define FBMcf_TAIL_z		8
#define FBMcf_TAIL		(FBMcf_TAIL_DOLLAR|FBMcf_TAIL_DOLLARM|FBMcf_TAIL_Z|FBMcf_TAIL_z)

#define FBMrf_MULTILINE	1

struct regmatch_state;
struct regmatch_slab;

/* like regmatch_info_aux, but contains extra fields only needed if the
 * pattern contains (?{}). If used, is snuck into the second slot in the
 * regmatch_state stack at the start of execution */

typedef struct {
    regexp *rex;
    PMOP    *curpm;     /* saved PL_curpm */
#ifdef PERL_ANY_COW
    SV      *saved_copy; /* saved saved_copy field from rex */
#endif
    char    *subbeg;    /* saved subbeg     field from rex */
    STRLEN  sublen;     /* saved sublen     field from rex */
    STRLEN  suboffset;  /* saved suboffset  field from rex */
    STRLEN  subcoffset; /* saved subcoffset field from rex */
    SV      *sv;        /* $_  during (?{}) */
    MAGIC   *pos_magic; /* pos() magic attached to $_ */
    SSize_t pos;        /* the original value of pos() in pos_magic */
    U8      pos_flags;  /* flags to be restored; currently only MGf_BYTES*/
} regmatch_info_aux_eval;


/* fields that logically  live in regmatch_info, but which need cleaning
 * up on croak(), and so are instead are snuck into the first slot in
 * the regmatch_state stack at the start of execution */

typedef struct {
    regmatch_info_aux_eval *info_aux_eval;
    struct regmatch_state *old_regmatch_state; /* saved PL_regmatch_state */
    struct regmatch_slab  *old_regmatch_slab;  /* saved PL_regmatch_slab */
    char *poscache;	/* S-L cache of fail positions of WHILEMs */
} regmatch_info_aux;


/*
=for apidoc Ay||regmatch_info
Some basic information about the current match that is created by
Perl_regexec_flags and then passed to regtry(), regmatch() etc.
It is allocated as a local var on the stack, so nothing should be
stored in it that needs preserving or clearing up on croak().
For that, see the aux_info and aux_info_eval members of the
regmatch_state union.

=cut
*/

typedef struct {
    REGEXP *prog;        /* the regex being executed */
    const char * strbeg; /* real start of string */
    char *strend;        /* one byte beyond last char of match string */
    char *till;          /* matches shorter than this fail (see minlen arg) */
    SV *sv;              /* the SV string currently being matched */
    char *ganch;         /* position of \G anchor */
    char *cutpoint;      /* (*COMMIT) position (if any) */
    regmatch_info_aux      *info_aux; /* extra fields that need cleanup */
    regmatch_info_aux_eval *info_aux_eval; /* extra saved state for (?{}) */
    I32  poscache_maxiter; /* how many whilems todo before S-L cache kicks in */
    I32  poscache_iter;    /* current countdown from _maxiter to zero */
    STRLEN poscache_size;  /* size of regmatch_info_aux.poscache */
    bool intuit;    /* re_intuit_start() is the top-level caller */
    bool is_utf8_pat;    /* regex is utf8 */
    bool is_utf8_target; /* string being matched is utf8 */
    bool warned; /* we have issued a recursion warning; no need for more */
} regmatch_info;


/* structures for holding and saving the state maintained by regmatch() */

#ifndef MAX_RECURSE_EVAL_NOCHANGE_DEPTH
#  define MAX_RECURSE_EVAL_NOCHANGE_DEPTH 10
#endif

/* The +1 is because everything matches itself, which isn't included in
 * MAX_FOLD_FROMS; the +2 is based on the current Unicode standards needs, and
 * is unlikely to change.  An assertion should fail in regexec.c if it is too
 * low.  It is needed for certain edge cases involving multi-character folds
 * when the first component also participates in a fold individually. */
#define MAX_MATCHES (MAX_FOLD_FROMS + 1 + 2)

struct next_matchable_info {
    U8     first_byte_mask;
    U8     first_byte_anded;
    U32    mask32;
    U32    anded32;
    PERL_INT_FAST8_T count; /* Negative means not initialized */
    PERL_UINT_FAST8_T min_length;
    PERL_UINT_FAST8_T max_length;
    PERL_UINT_FAST8_T initial_definitive;
    PERL_UINT_FAST8_T initial_exact;
    PERL_UINT_FAST8_T lengths[MAX_MATCHES];

    /* The size is from trial and error, and could change with new Unicode
     * standards, in which case there is an assertion that should start
     * failing.  This size could be calculated in one of the regen scripts
     * dealing with Unicode, but khw thinks the likelihood of it changing is
     * low enough that it isn't worth the effort. */
    U8 matches[18];
};

typedef I32 CHECKPOINT;

typedef struct regmatch_state {
    int resume_state;		/* where to jump to on return */
    char *locinput;		/* where to backtrack in string on failure */
    char *loceol;
    U8 *sr0;                    /* position of start of script run, or NULL */

    union {

        /* the 'info_aux' and 'info_aux_eval' union members are cuckoos in
         * the nest. They aren't saved backtrack state; rather they
         * represent one or two extra chunks of data that need allocating
         * at the start of a match. These fields would logically live in
         * the regmatch_info struct, except that is allocated on the
         * C stack, and these fields are all things that require cleanup
         * after a croak(), when the stack is lost.
         * As a convenience, we just use the first 1 or 2 regmatch_state
         * slots to store this info, as we will be allocating a slab of
         * these anyway. Otherwise we'd have to malloc and then free them,
         * or allocate them on the save stack (where they will get
         * realloced if the save stack grows).
         * info_aux contains the extra fields that are always needed;
         * info_aux_eval contains extra fields that only needed if
         * the pattern contains code blocks
         * We split them into two separate structs to avoid increasing
         * the size of the union.
         */

        regmatch_info_aux info_aux;

        regmatch_info_aux_eval info_aux_eval;

        /* this is a fake union member that matches the first element
         * of each member that needs to store positive backtrack
         * information */
        struct {
            struct regmatch_state *prev_yes_state;
        } yes;


        /* NOTE: Regarding 'cp' and 'lastcp' in the following structs...
         *
         * In the majority of cases we use 'cp' for the "normal"
         * checkpoint for paren saves, and 'lastcp' for the addtional
         * paren saves that are done only under RE_PESSIMISTIC_PARENS.
         *
         * There may be a few cases where both are used always.
         * Regardless they tend be used something like this:
         *
         *   ST.cp = regcppush(rex, 0, maxopenparen);
         *   REGCP_SET(ST.lastcp);
         *
         * thus ST.cp holds the checkpoint from before we push parens,
         * and ST.lastcp holds the checkpoint from afterwards.
         */

        /* branchlike members */
        /* this is a fake union member that matches the first elements
         * of each member that needs to behave like a branch */
        struct {
            /* this first element must match u.yes */
            struct regmatch_state *prev_yes_state;
            U32         lastparen;
            U32         lastcloseparen;
            CHECKPOINT  cp;         /* see note above "struct branchlike" */
            CHECKPOINT  lastcp;     /* see note above "struct branchlike" */
            U16         before_paren;
            U16         after_paren;

        } branchlike;

        struct {
            /* the first elements must match u.branchlike */
            struct regmatch_state *prev_yes_state;
            U32         lastparen;
            U32         lastcloseparen;
            CHECKPOINT  cp;         /* see note above "struct branchlike" */
            CHECKPOINT  lastcp;     /* see note above "struct branchlike" */
            U16         before_paren;
            U16         after_paren;

            regnode *next_branch;   /* next branch node */
        } branch;

        struct {
            /* the first elements must match u.branchlike */
            struct regmatch_state *prev_yes_state;
            U32         lastparen;
            U32         lastcloseparen;
            CHECKPOINT  cp;         /* see note above "struct branchlike" */
            CHECKPOINT  lastcp;     /* see note above "struct branchlike" */
            U16         before_paren;
            U16         after_paren;

            U32         accepted;   /* how many accepting states left */
            bool        longfold;   /* saw a fold with a 1->n char mapping */
            U16         *jump;      /* positive offsets from me */
            U16         *j_before_paren;
            U16         *j_after_paren;
            regnode     *me;        /* Which node am I - needed for jump tries*/
            U8          *firstpos;  /* pos in string of first trie match */
            U32         firstchars; /* len in chars of firstpos from start */
            U16         nextword;   /* next word to try */
            U16         topword;    /* longest accepted word */
        } trie;

        /* special types - these members are used to store state for special
           regops like eval, if/then, lookaround and the markpoint state */
        struct {
            /* this first element must match u.yes */
            struct regmatch_state *prev_yes_state;
            struct regmatch_state *prev_curlyx;
            struct regmatch_state *prev_eval;
            REGEXP	*prev_rex;
            CHECKPOINT  cp;             /* see note above "struct branchlike" */
            CHECKPOINT  lastcp;         /* see note above "struct branchlike" */
            U32         close_paren;    /* which close bracket is our end (+1) */
            regnode     *B;             /* the node following us  */
            char        *prev_recurse_locinput;
        } eval;

        struct {
            /* this first element must match u.yes */
            struct regmatch_state *prev_yes_state;
            I32     wanted;
            I32     logical;    /* saved copy of 'logical' var */
            U8      count;      /* number of beginning positions */
            char    *start;
            char    *end;
            regnode *me;        /* the IFMATCH/SUSPEND/UNLESSM node  */
            char    *prev_match_end;
        } ifmatch;              /* and SUSPEND/UNLESSM */

        struct {
            /* this first element must match u.yes */
            struct regmatch_state *prev_yes_state;
            struct regmatch_state *prev_mark;
            SV      *mark_name;
            char    *mark_loc;
        } mark;

        struct {
            int val;
        } keeper;

        /* quantifiers - these members are used for storing state for
           the regops used to implement quantifiers */
        struct {
            /* this first element must match u.yes */
            struct regmatch_state *prev_yes_state;
            struct regmatch_state *prev_curlyx; /* previous cur_curlyx */
            regnode     *me;        /* the CURLYX node  */
            regnode     *B;         /* the B node in /A*B/  */
            CHECKPOINT  cp;         /* see note above "struct branchlike" */
            CHECKPOINT  lastcp;     /* see note above "struct branchlike" */
            bool	minmod;
            int         parenfloor; /* how far back to strip paren data */

            /* these two are modified by WHILEM */
            int         count;      /* how many instances of A we've matched */
            char        *lastloc;   /* where previous A matched (0-len detect) */
        } curlyx;

        struct {
            /* this first element must match u.yes */
            struct regmatch_state *prev_yes_state;
            struct regmatch_state *save_curlyx;
            CHECKPOINT  cp;             /* see note above "struct branchlike" */
            CHECKPOINT  lastcp;         /* see note above "struct branchlike" */
            char        *save_lastloc;  /* previous curlyx.lastloc */
            I32		cache_offset;
            I32		cache_mask;
        } whilem;

        struct {
            /* this first element must match u.yes */
            struct regmatch_state *prev_yes_state;
            U32         lastparen;
            U32         lastcloseparen;
            CHECKPOINT  cp;         /* see note above "struct branchlike" */
            CHECKPOINT  lastcp;     /* see note above "struct branchlike" */
            I32         alen;       /* length of first-matched A string */
            I32         count;
            bool        minmod;
            regnode     *A, *B;     /* the nodes corresponding to /A*B/  */
            regnode     *me;        /* the curlym node */
            struct next_matchable_info Binfo;
        } curlym;

        struct {
            U32         paren;
            U32         lastparen;
            U32         lastcloseparen;
            CHECKPOINT  cp;         /* see note above "struct branchlike" */
            CHECKPOINT  lastcp;     /* see note above "struct branchlike" */
            char        *maxpos;    /* highest possible point in string to match */
            char        *oldloc;    /* the previous locinput */
            int         count;
            int         min, max;   /* {m,n} */
            regnode     *A, *B;     /* the nodes corresponding to /A*B/  */
            struct next_matchable_info Binfo;
        } curly; /* and CURLYN/PLUS/STAR */

        struct {
            CHECKPOINT  cp;
            CHECKPOINT  lastcp;
        } backref; /* REF and friends */
    } u;
} regmatch_state;



/* how many regmatch_state structs to allocate as a single slab.
 * We do it in 4K blocks for efficiency. The "3" is 2 for the next/prev
 * pointers, plus 1 for any mythical malloc overhead. */

#define PERL_REGMATCH_SLAB_SLOTS \
    ((4096 - 3 * sizeof (void*)) / sizeof(regmatch_state))

typedef struct regmatch_slab {
    regmatch_state states[PERL_REGMATCH_SLAB_SLOTS];
    struct regmatch_slab *prev, *next;
} regmatch_slab;


#define REG_FETCH_ABSOLUTE 1

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
