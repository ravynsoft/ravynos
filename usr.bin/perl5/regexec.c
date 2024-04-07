/*    regexec.c
 */

/*
 *	One Ring to rule them all, One Ring to find them
 *
 *     [p.v of _The Lord of the Rings_, opening poem]
 *     [p.50 of _The Lord of the Rings_, I/iii: "The Shadow of the Past"]
 *     [p.254 of _The Lord of the Rings_, II/ii: "The Council of Elrond"]
 */

/* This file contains functions for executing a regular expression.  See
 * also regcomp.c which funnily enough, contains functions for compiling
 * a regular expression.
 *
 * This file is also copied at build time to ext/re/re_exec.c, where
 * it's built with -DPERL_EXT_RE_BUILD -DPERL_EXT_RE_DEBUG -DPERL_EXT.
 * This causes the main functions to be compiled under new names and with
 * debugging support added, which makes "use re 'debug'" work.
 */

/* NOTE: this is derived from Henry Spencer's regexp code, and should not
 * confused with the original package (see point 3 below).  Thanks, Henry!
 */

/* Additional note: this code is very heavily munged from Henry's version
 * in places.  In some spots I've traded clarity for efficiency, so don't
 * blame Henry for some of the lack of readability.
 */

/* The names of the functions have been changed from regcomp and
 * regexec to  pregcomp and pregexec in order to avoid conflicts
 * with the POSIX routines of the same names.
*/

#ifdef PERL_EXT_RE_BUILD
#include "re_top.h"
#endif

/*
 * pregcomp and pregexec -- regsub and regerror are not used in perl
 *
 *	Copyright (c) 1986 by University of Toronto.
 *	Written by Henry Spencer.  Not derived from licensed software.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to redistribute it freely,
 *	subject to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of
 *		this software, no matter how awful, even if they arise
 *		from defects in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *		by explicit claim or by omission.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *		be misrepresented as being the original software.
 *
 ****    Alterations to Henry's code are...
 ****
 ****    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999,
 ****    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008
 ****    by Larry Wall and others
 ****
 ****    You may distribute under the terms of either the GNU General Public
 ****    License or the Artistic License, as specified in the README file.
 *
 * Beware that some of this code is subtly aware of the way operator
 * precedence is structured in regular expressions.  Serious changes in
 * regular-expression syntax might require a total rethink.
 */
#include "EXTERN.h"
#define PERL_IN_REGEX_ENGINE
#define PERL_IN_REGEXEC_C
#include "perl.h"

#ifdef PERL_IN_XSUB_RE
#  include "re_comp.h"
#else
#  include "regcomp.h"
#endif

#include "invlist_inline.h"
#include "unicode_constants.h"

static const char b_utf8_locale_required[] =
 "Use of \\b{} or \\B{} for non-UTF-8 locale is wrong."
                                                "  Assuming a UTF-8 locale";

#define CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_BOUND                       \
    STMT_START {                                                            \
        if (! IN_UTF8_CTYPE_LOCALE) {                                       \
          Perl_ck_warner(aTHX_ packWARN(WARN_LOCALE),                       \
                                                b_utf8_locale_required);    \
        }                                                                   \
    } STMT_END

static const char sets_utf8_locale_required[] =
      "Use of (?[ ]) for non-UTF-8 locale is wrong.  Assuming a UTF-8 locale";

#define CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_SETS(n)                     \
    STMT_START {                                                            \
        if (! IN_UTF8_CTYPE_LOCALE && (FLAGS(n) & ANYOFL_UTF8_LOCALE_REQD)){\
          Perl_ck_warner(aTHX_ packWARN(WARN_LOCALE),                       \
                                             sets_utf8_locale_required);    \
        }                                                                   \
    } STMT_END

#ifdef DEBUGGING
/* At least one required character in the target string is expressible only in
 * UTF-8. */
static const char non_utf8_target_but_utf8_required[]
                = "Can't match, because target string needs to be in UTF-8\n";
#endif

#define NON_UTF8_TARGET_BUT_UTF8_REQUIRED(target) STMT_START {           \
    DEBUG_EXECUTE_r(Perl_re_printf( aTHX_  "%s", non_utf8_target_but_utf8_required));\
    goto target;                                                         \
} STMT_END

#ifndef STATIC
#define STATIC  static
#endif

/*
 * Forwards.
 */

#define CHR_SVLEN(sv) (utf8_target ? sv_len_utf8(sv) : SvCUR(sv))

#define HOPc(pos,off) \
        (char *)(reginfo->is_utf8_target \
            ? reghop3((U8*)pos, off, \
                    (U8*)(off >= 0 ? reginfo->strend : reginfo->strbeg)) \
            : (U8*)(pos + off))

/* like HOPMAYBE3 but backwards. lim must be +ve. Returns NULL on overshoot */
#define HOPBACK3(pos, off, lim) \
        (reginfo->is_utf8_target                          \
            ? reghopmaybe3((U8*)pos, (SSize_t)0-off, (U8*)(lim)) \
            : (pos - off >= lim)	                         \
                ? (U8*)pos - off		                 \
                : NULL)

#define HOPBACKc(pos, off) ((char*)HOPBACK3(pos, off, reginfo->strbeg))

#define HOP3(pos,off,lim) (reginfo->is_utf8_target  ? reghop3((U8*)(pos), off, (U8*)(lim)) : (U8*)(pos + off))
#define HOP3c(pos,off,lim) ((char*)HOP3(pos,off,lim))

/* lim must be +ve. Returns NULL on overshoot */
#define HOPMAYBE3(pos,off,lim) \
        (reginfo->is_utf8_target                        \
            ? reghopmaybe3((U8*)pos, off, (U8*)(lim))   \
            : ((U8*)pos + off <= lim)                   \
                ? (U8*)pos + off                        \
                : NULL)

/* like HOP3, but limits the result to <= lim even for the non-utf8 case.
 * off must be >=0; args should be vars rather than expressions */
#define HOP3lim(pos,off,lim) (reginfo->is_utf8_target \
    ? reghop3((U8*)(pos), off, (U8*)(lim)) \
    : (U8*)((pos + off) > lim ? lim : (pos + off)))
#define HOP3clim(pos,off,lim) ((char*)HOP3lim(pos,off,lim))

#define HOP4(pos,off,llim, rlim) (reginfo->is_utf8_target \
    ? reghop4((U8*)(pos), off, (U8*)(llim), (U8*)(rlim)) \
    : (U8*)(pos + off))
#define HOP4c(pos,off,llim, rlim) ((char*)HOP4(pos,off,llim, rlim))

#define PLACEHOLDER	/* Something for the preprocessor to grab onto */
/* TODO: Combine JUMPABLE and HAS_TEXT to cache OP(rn) */

/* for use after a quantifier and before an EXACT-like node -- japhy */
/* it would be nice to rework regcomp.sym to generate this stuff. sigh
 *
 * NOTE that *nothing* that affects backtracking should be in here, specifically
 * VERBS must NOT be included. JUMPABLE is used to determine  if we can ignore a
 * node that is in between two EXACT like nodes when ascertaining what the required
 * "follow" character is. This should probably be moved to regex compile time
 * although it may be done at run time because of the REF possibility - more
 * investigation required. -- demerphq
*/
#define JUMPABLE(rn) (                                                             \
    OP(rn) == OPEN ||                                                              \
    (OP(rn) == CLOSE &&                                                            \
     !EVAL_CLOSE_PAREN_IS(cur_eval,PARNO(rn)) ) ||                                 \
    OP(rn) == EVAL ||                                                              \
    OP(rn) == SUSPEND || OP(rn) == IFMATCH ||                                      \
    OP(rn) == PLUS || OP(rn) == MINMOD ||                                          \
    OP(rn) == KEEPS ||                                                             \
    (REGNODE_TYPE(OP(rn)) == CURLY && ARG1i(rn) > 0)                                  \
)
#define IS_EXACT(rn) (REGNODE_TYPE(OP(rn)) == EXACT)

#define HAS_TEXT(rn) ( IS_EXACT(rn) || REGNODE_TYPE(OP(rn)) == REF )

/*
  Search for mandatory following text node; for lookahead, the text must
  follow but for lookbehind (FLAGS(rn) != 0) we skip to the next step.
*/
#define FIND_NEXT_IMPT(rn) STMT_START {                                   \
    while (JUMPABLE(rn)) { \
        const OPCODE type = OP(rn); \
        if (type == SUSPEND || REGNODE_TYPE(type) == CURLY) \
            rn = REGNODE_AFTER_opcode(rn,type); \
        else if (type == PLUS) \
            rn = REGNODE_AFTER_type(rn,tregnode_PLUS); \
        else if (type == IFMATCH) \
            rn = (FLAGS(rn) == 0) ? REGNODE_AFTER_type(rn,tregnode_IFMATCH) : rn + ARG1u(rn); \
        else rn += NEXT_OFF(rn); \
    } \
} STMT_END

#define SLAB_FIRST(s) (&(s)->states[0])
#define SLAB_LAST(s)  (&(s)->states[PERL_REGMATCH_SLAB_SLOTS-1])

static void S_setup_eval_state(pTHX_ regmatch_info *const reginfo);
static void S_cleanup_regmatch_info_aux(pTHX_ void *arg);
static regmatch_state * S_push_slab(pTHX);

#define REGCP_OTHER_ELEMS 3
#define REGCP_FRAME_ELEMS 1
/* REGCP_FRAME_ELEMS are not part of the REGCP_OTHER_ELEMS and
 * are needed for the regexp context stack bookkeeping. */

STATIC CHECKPOINT
S_regcppush(pTHX_ const regexp *rex, I32 parenfloor, U32 maxopenparen comma_pDEPTH)
{
    const int retval = PL_savestack_ix;
    /* Number of bytes about to be stored in the stack */
    const SSize_t paren_bytes_to_push = sizeof(*RXp_OFFSp(rex)) * (maxopenparen - parenfloor);
    /* Number of savestack[] entries to be filled by the paren data */
    /* Rounding is performed in case we are few elements short */
    const int paren_elems_to_push = (paren_bytes_to_push + sizeof(*PL_savestack) - 1) / sizeof(*PL_savestack);
    const UV total_elems = paren_elems_to_push + REGCP_OTHER_ELEMS;
    const UV elems_shifted = total_elems << SAVE_TIGHT_SHIFT;

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGCPPUSH;

    if (paren_elems_to_push < 0)
        Perl_croak(aTHX_ "panic: paren_elems_to_push, %i < 0, maxopenparen: %i parenfloor: %i",
                   (int)paren_elems_to_push, (int)maxopenparen,
                   (int)parenfloor);

    if ((elems_shifted >> SAVE_TIGHT_SHIFT) != total_elems)
        Perl_croak(aTHX_ "panic: paren_elems_to_push offset %" UVuf
                   " out of range (%lu-%ld)",
                   total_elems,
                   (unsigned long)maxopenparen,
                   (long)parenfloor);

    DEBUG_BUFFERS_r(
        if ((int)maxopenparen > (int)parenfloor)
            Perl_re_exec_indentf( aTHX_
                "rex=0x%" UVxf " offs=0x%" UVxf ": saving capture indices:\n",
                depth,
                PTR2UV(rex),
                PTR2UV(RXp_OFFSp(rex))
            );
    );

    SSGROW(total_elems + REGCP_FRAME_ELEMS);
    assert((IV)PL_savestack_max > (IV)(total_elems + REGCP_FRAME_ELEMS));

    /* memcpy the offs inside the stack - it's faster than for loop */
    memcpy(&PL_savestack[PL_savestack_ix], RXp_OFFSp(rex) + parenfloor + 1, paren_bytes_to_push);
    PL_savestack_ix += paren_elems_to_push;

    DEBUG_BUFFERS_r({
	I32 p;
        for (p = parenfloor + 1; p <= (I32)maxopenparen; p++) {
            Perl_re_exec_indentf(aTHX_
                "    \\%" UVuf " %" IVdf " (%" IVdf ") .. %" IVdf " (regcppush)\n",
                depth,
                (UV)p,
                (IV)RXp_OFFSp(rex)[p].start,
                (IV)RXp_OFFSp(rex)[p].start_tmp,
                (IV)RXp_OFFSp(rex)[p].end
            );
        }
    });

/* REGCP_OTHER_ELEMS are pushed in any case, parentheses or no. */
    SSPUSHINT(maxopenparen);
    SSPUSHINT(RXp_LASTPAREN(rex));
    SSPUSHINT(RXp_LASTCLOSEPAREN(rex));
    SSPUSHUV(SAVEt_REGCONTEXT | elems_shifted); /* Magic cookie. */


    DEBUG_BUFFERS_r({
        Perl_re_exec_indentf(aTHX_
                "finished regcppush returning %" IVdf " cur: %" IVdf "\n",
                depth, retval, PL_savestack_ix);
    });

    return retval;
}

/* These are needed since we do not localize EVAL nodes: */
#define REGCP_SET(cp)                                           \
    DEBUG_STATE_r(                                              \
        Perl_re_exec_indentf( aTHX_                             \
            "Setting an EVAL scope, savestack=%" IVdf ",\n",    \
            depth, (IV)PL_savestack_ix                          \
        )                                                       \
    );                                                          \
    cp = PL_savestack_ix

#define REGCP_UNWIND(cp)                                        \
    DEBUG_STATE_r(                                              \
        if (cp != PL_savestack_ix)                              \
            Perl_re_exec_indentf( aTHX_                         \
                "Clearing an EVAL scope, savestack=%"           \
                IVdf "..%" IVdf "\n",                           \
                depth, (IV)(cp), (IV)PL_savestack_ix            \
            )                                                   \
    );                                                          \
    regcpblow(cp)

/* set the start and end positions of capture ix */
#define CLOSE_ANY_CAPTURE(rex, ix, s, e)                                    \
    RXp_OFFSp(rex)[(ix)].start = (s);                                       \
    RXp_OFFSp(rex)[(ix)].end = (e)

#define CLOSE_CAPTURE(rex, ix, s, e)                                        \
    CLOSE_ANY_CAPTURE(rex, ix, s, e);                                       \
    if (ix > RXp_LASTPAREN(rex))                                            \
        RXp_LASTPAREN(rex) = (ix);                                          \
    RXp_LASTCLOSEPAREN(rex) = (ix);                                         \
    DEBUG_BUFFERS_r(Perl_re_exec_indentf( aTHX_                             \
        "CLOSE: rex=0x%" UVxf " offs=0x%" UVxf ": \\%" UVuf ": set %" IVdf " .. %" IVdf " max: %" UVuf "\n", \
        depth,                                                              \
        PTR2UV(rex),                                                        \
        PTR2UV(RXp_OFFSp(rex)),                                             \
        (UV)(ix),                                                           \
        (IV)RXp_OFFSp(rex)[ix].start,                                       \
        (IV)RXp_OFFSp(rex)[ix].end,                                         \
        (UV)RXp_LASTPAREN(rex)                                              \
    ))

/* the lp and lcp args match the relevant members of the
 * regexp structure, but in practice they should all be U16
 * instead as we have a hard limit of U16_MAX parens. See
 * line 4003 or so of regcomp.c where we parse OPEN parens
 * of various types. */
PERL_STATIC_INLINE void
S_unwind_paren(pTHX_ regexp *rex, U32 lp, U32 lcp comma_pDEPTH) {
    PERL_ARGS_ASSERT_UNWIND_PAREN;
    U32 n;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;
    DEBUG_BUFFERS_r(Perl_re_exec_indentf( aTHX_
        "UNWIND_PAREN: rex=0x%" UVxf " offs=0x%" UVxf
        ": invalidate (%" UVuf " .. %" UVuf ") set lcp: %" UVuf "\n",
        depth,
        PTR2UV(rex),
        PTR2UV(RXp_OFFSp(rex)),
        (UV)(lp),
        (UV)(RXp_LASTPAREN(rex)),
        (UV)(lcp)
    ));
    for (n = RXp_LASTPAREN(rex); n > lp; n--) {
        RXp_OFFSp(rex)[n].end = -1;
    }
    RXp_LASTPAREN(rex) = n;
    RXp_LASTCLOSEPAREN(rex) = lcp;
}
#define UNWIND_PAREN(lp,lcp) unwind_paren(rex,lp,lcp)

PERL_STATIC_INLINE void
S_capture_clear(pTHX_ regexp *rex, U16 from_ix, U16 to_ix, const char *str comma_pDEPTH) {
    PERL_ARGS_ASSERT_CAPTURE_CLEAR;
    PERL_UNUSED_ARG(str); /* only used for debugging */
    U16 my_ix;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;
    for ( my_ix = from_ix; my_ix <= to_ix; my_ix++ ) {
        DEBUG_BUFFERS_r(Perl_re_exec_indentf( aTHX_
                "CAPTURE_CLEAR %s \\%" IVdf ": "
                "%" IVdf "(%" IVdf ") .. %" IVdf
                " => "
                "%" IVdf "(%" IVdf ") .. %" IVdf
                "\n",
            depth, str, (IV)my_ix,
            (IV)RXp_OFFSp(rex)[my_ix].start,
            (IV)RXp_OFFSp(rex)[my_ix].start_tmp,
            (IV)RXp_OFFSp(rex)[my_ix].end,
            (IV)-1, (IV)-1, (IV)-1));
        RXp_OFFSp(rex)[my_ix].start = -1;
        RXp_OFFSp(rex)[my_ix].start_tmp = -1;
        RXp_OFFSp(rex)[my_ix].end = -1;
    }
}

#define CAPTURE_CLEAR(from_ix, to_ix, str) \
    if (from_ix) capture_clear(rex,from_ix, to_ix, str)

STATIC void
S_regcppop(pTHX_ regexp *rex, U32 *maxopenparen_p comma_pDEPTH)
{
    UV i;
    U32 paren;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGCPPOP;


    DEBUG_BUFFERS_r({
        Perl_re_exec_indentf(aTHX_
                "starting regcppop at %" IVdf "\n",
                depth, PL_savestack_ix);
    });

    /* Pop REGCP_OTHER_ELEMS before the parentheses loop starts. */
    i = SSPOPUV;
    assert((i & SAVE_MASK) == SAVEt_REGCONTEXT); /* Check that the magic cookie is there. */
    i >>= SAVE_TIGHT_SHIFT; /* Parentheses elements to pop. */
    RXp_LASTCLOSEPAREN(rex) = SSPOPINT;
    RXp_LASTPAREN(rex) = SSPOPINT;
    *maxopenparen_p = SSPOPINT;

    i -= REGCP_OTHER_ELEMS;
    /* Now restore the parentheses context. */
    DEBUG_BUFFERS_r(
        if (i || RXp_LASTPAREN(rex) + 1 <= rex->nparens)
            Perl_re_exec_indentf( aTHX_
                "rex=0x%" UVxf " offs=0x%" UVxf ": restoring capture indices to:\n",
                depth,
                PTR2UV(rex),
                PTR2UV(RXp_OFFSp(rex))
            );
    );
    /* substract remaining elements from the stack */
    PL_savestack_ix -= i;

    /* static assert that offs struc size is not less than stack elem size */
    STATIC_ASSERT_STMT(sizeof(*RXp_OFFSp(rex)) >= sizeof(*PL_savestack));

    /* calculate actual number of offs/capture groups stored */
    /* by doing integer division (leaving potential alignment aside) */
    i = (i * sizeof(*PL_savestack)) / sizeof(*RXp_OFFSp(rex));

    /* calculate paren starting point */
    /* i is our number of entries which we are subtracting from *maxopenparen_p */
    /* and we are storing + 1 this to get the beginning */
    paren = *maxopenparen_p - i + 1;

    /* restore them */
    memcpy(RXp_OFFSp(rex) + paren, &PL_savestack[PL_savestack_ix], i * sizeof(*RXp_OFFSp(rex)));

    DEBUG_BUFFERS_r(
        for (; paren <= *maxopenparen_p; ++paren) {
            Perl_re_exec_indentf(aTHX_
                "    \\%" UVuf " %" IVdf "(%" IVdf ") .. %" IVdf " %s (regcppop)\n",
                depth,
                (UV)paren,
                (IV)RXp_OFFSp(rex)[paren].start,
                (IV)RXp_OFFSp(rex)[paren].start_tmp,
                (IV)RXp_OFFSp(rex)[paren].end,
                (paren > RXp_LASTPAREN(rex) ? "(skipped)" : ""));
        }
    );
#if 1
    /* It would seem that the similar code in regtry()
     * already takes care of this, and in fact it is in
     * a better location to since this code can #if 0-ed out
     * but the code in regtry() is needed or otherwise tests
     * requiring null fields (pat.t#187 and split.t#{13,14}
     * (as of patchlevel 7877)  will fail.  Then again,
     * this code seems to be necessary or otherwise
     * this erroneously leaves $1 defined: "1" =~ /^(?:(\d)x)?\d$/
     * --jhi updated by dapm */
    for (i = RXp_LASTPAREN(rex) + 1; i <= rex->nparens; i++) {
        if (i > *maxopenparen_p) {
            RXp_OFFSp(rex)[i].start = -1;
        }
        RXp_OFFSp(rex)[i].end = -1;
        DEBUG_BUFFERS_r( Perl_re_exec_indentf( aTHX_
            "    \\%" UVuf ": %s   ..-1 undeffing (regcppop)\n",
            depth,
            (UV)i,
            (i > *maxopenparen_p) ? "-1" : "  "
        ));
    }
#endif
    DEBUG_BUFFERS_r({
        Perl_re_exec_indentf(aTHX_
                "finished regcppop at %" IVdf "\n",
                depth, PL_savestack_ix);
    });
}

/* restore the parens and associated vars at savestack position ix,
 * but without popping the stack */

STATIC void
S_regcp_restore(pTHX_ regexp *rex, I32 ix, U32 *maxopenparen_p comma_pDEPTH)
{
    I32 tmpix = PL_savestack_ix;
    PERL_ARGS_ASSERT_REGCP_RESTORE;

    PL_savestack_ix = ix;
    regcppop(rex, maxopenparen_p);
    PL_savestack_ix = tmpix;
}

#define regcpblow(cp) LEAVE_SCOPE(cp)	/* Ignores regcppush()ed data. */

STATIC bool
S_isFOO_lc(pTHX_ const U8 classnum, const U8 character)
{
    /* Returns a boolean as to whether or not 'character' is a member of the
     * Posix character class given by 'classnum' that should be equivalent to a
     * value in the typedef 'char_class_number_'.
     *
     * Ideally this could be replaced by a just an array of function pointers
     * to the C library functions that implement the macros this calls.
     * However, to compile, the precise function signatures are required, and
     * these may vary from platform to platform.  To avoid having to figure
     * out what those all are on each platform, I (khw) am using this method,
     * which adds an extra layer of function call overhead (unless the C
     * optimizer strips it away).  But we don't particularly care about
     * performance with locales anyway. */

    if (IN_UTF8_CTYPE_LOCALE) {
        return cBOOL(generic_isCC_(character, classnum));
    }

    switch ((char_class_number_) classnum) {
        case CC_ENUM_ALPHANUMERIC_: return isU8_ALPHANUMERIC_LC(character);
        case CC_ENUM_ALPHA_:        return    isU8_ALPHA_LC(character);
        case CC_ENUM_ASCII_:        return    isU8_ASCII_LC(character);
        case CC_ENUM_BLANK_:        return    isU8_BLANK_LC(character);
        case CC_ENUM_CASED_:        return    isU8_CASED_LC(character);
        case CC_ENUM_CNTRL_:        return    isU8_CNTRL_LC(character);
        case CC_ENUM_DIGIT_:        return    isU8_DIGIT_LC(character);
        case CC_ENUM_GRAPH_:        return    isU8_GRAPH_LC(character);
        case CC_ENUM_LOWER_:        return    isU8_LOWER_LC(character);
        case CC_ENUM_PRINT_:        return    isU8_PRINT_LC(character);
        case CC_ENUM_PUNCT_:        return    isU8_PUNCT_LC(character);
        case CC_ENUM_SPACE_:        return    isU8_SPACE_LC(character);
        case CC_ENUM_UPPER_:        return    isU8_UPPER_LC(character);
        case CC_ENUM_WORDCHAR_:     return isU8_WORDCHAR_LC(character);
        case CC_ENUM_XDIGIT_:       return   isU8_XDIGIT_LC(character);
        default:    /* VERTSPACE should never occur in locales */
            break;
    }

    Perl_croak(aTHX_
               "panic: isFOO_lc() has an unexpected character class '%d'",
               classnum);

    NOT_REACHED; /* NOTREACHED */
    return FALSE;
}

PERL_STATIC_INLINE I32
S_foldEQ_latin1_s2_folded(pTHX_ const char *s1, const char *s2, I32 len)
{
    /* Compare non-UTF-8 using Unicode (Latin1) semantics.  s2 must already be
     * folded.  Works on all folds representable without UTF-8, except for
     * LATIN_SMALL_LETTER_SHARP_S, and does not check for this.  Nor does it
     * check that the strings each have at least 'len' characters.
     *
     * There is almost an identical API function where s2 need not be folded:
     * Perl_foldEQ_latin1() */

    const U8 *a = (const U8 *)s1;
    const U8 *b = (const U8 *)s2;

    PERL_ARGS_ASSERT_FOLDEQ_LATIN1_S2_FOLDED;

    assert(len >= 0);

    while (len--) {
        assert(! isUPPER_L1(*b));
        if (toLOWER_L1(*a) != *b) {
            return 0;
        }
        a++, b++;
    }
    return 1;
}

STATIC bool
S_isFOO_utf8_lc(pTHX_ const U8 classnum, const U8* character, const U8* e)
{
    /* Returns a boolean as to whether or not the (well-formed) UTF-8-encoded
     * 'character' is a member of the Posix character class given by 'classnum'
     * that should be equivalent to a value in the typedef
     * 'char_class_number_'.
     *
     * This just calls isFOO_lc on the code point for the character if it is in
     * the range 0-255.  Outside that range, all characters use Unicode
     * rules, ignoring any locale.  So use the Unicode function if this class
     * requires an inversion list, and use the Unicode macro otherwise. */


    PERL_ARGS_ASSERT_ISFOO_UTF8_LC;

    if (UTF8_IS_INVARIANT(*character)) {
        return isFOO_lc(classnum, *character);
    }
    else if (UTF8_IS_DOWNGRADEABLE_START(*character)) {
        return isFOO_lc(classnum,
                        EIGHT_BIT_UTF8_TO_NATIVE(*character, *(character + 1)));
    }

    _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(character, e);

    switch ((char_class_number_) classnum) {
        case CC_ENUM_SPACE_:     return is_XPERLSPACE_high(character);
        case CC_ENUM_BLANK_:     return is_HORIZWS_high(character);
        case CC_ENUM_XDIGIT_:    return is_XDIGIT_high(character);
        case CC_ENUM_VERTSPACE_: return is_VERTWS_high(character);
        default:
            return _invlist_contains_cp(PL_XPosix_ptrs[classnum],
                                        utf8_to_uvchr_buf(character, e, NULL));
    }
    NOT_REACHED; /* NOTREACHED */
}

STATIC U8 *
S_find_span_end(U8 * s, const U8 * send, const U8 span_byte)
{
    /* Returns the position of the first byte in the sequence between 's' and
     * 'send-1' inclusive that isn't 'span_byte'; returns 'send' if none found.
     * */

    PERL_ARGS_ASSERT_FIND_SPAN_END;

    assert(send >= s);

    if ((STRLEN) (send - s) >= PERL_WORDSIZE
                          + PERL_WORDSIZE * PERL_IS_SUBWORD_ADDR(s)
                          - (PTR2nat(s) & PERL_WORD_BOUNDARY_MASK))
    {
        PERL_UINTMAX_T span_word;

        /* Process per-byte until reach word boundary.  XXX This loop could be
         * eliminated if we knew that this platform had fast unaligned reads */
        while (PTR2nat(s) & PERL_WORD_BOUNDARY_MASK) {
            if (*s != span_byte) {
                return s;
            }
            s++;
        }

        /* Create a word filled with the bytes we are spanning */
        span_word = PERL_COUNT_MULTIPLIER * span_byte;

        /* Process per-word as long as we have at least a full word left */
        do {

            /* Keep going if the whole word is composed of 'span_byte's */
            if ((* (PERL_UINTMAX_T *) s) == span_word)  {
                s += PERL_WORDSIZE;
                continue;
            }

            /* Here, at least one byte in the word isn't 'span_byte'. */

#ifdef EBCDIC

            break;

#else

            /* This xor leaves 1 bits only in those non-matching bytes */
            span_word ^= * (PERL_UINTMAX_T *) s;

            /* Make sure the upper bit of each non-matching byte is set.  This
             * makes each such byte look like an ASCII platform variant byte */
            span_word |= span_word << 1;
            span_word |= span_word << 2;
            span_word |= span_word << 4;

            /* That reduces the problem to what this function solves */
            return s + variant_byte_number(span_word);

#endif

        } while (s + PERL_WORDSIZE <= send);
    }

    /* Process the straggler bytes beyond the final word boundary */
    while (s < send) {
        if (*s != span_byte) {
            return s;
        }
        s++;
    }

    return s;
}

STATIC U8 *
S_find_next_masked(U8 * s, const U8 * send, const U8 byte, const U8 mask)
{
    /* Returns the position of the first byte in the sequence between 's'
     * and 'send-1' inclusive that when ANDed with 'mask' yields 'byte';
     * returns 'send' if none found.  It uses word-level operations instead of
     * byte to speed up the process */

    PERL_ARGS_ASSERT_FIND_NEXT_MASKED;

    assert(send >= s);
    assert((byte & mask) == byte);

#ifndef EBCDIC

    if ((STRLEN) (send - s) >= PERL_WORDSIZE
                          + PERL_WORDSIZE * PERL_IS_SUBWORD_ADDR(s)
                          - (PTR2nat(s) & PERL_WORD_BOUNDARY_MASK))
    {
        PERL_UINTMAX_T word, mask_word;

        while (PTR2nat(s) & PERL_WORD_BOUNDARY_MASK) {
            if (((*s) & mask) == byte) {
                return s;
            }
            s++;
        }

        word      = PERL_COUNT_MULTIPLIER * byte;
        mask_word = PERL_COUNT_MULTIPLIER * mask;

        do {
            PERL_UINTMAX_T masked = (* (PERL_UINTMAX_T *) s) & mask_word;

            /* If 'masked' contains bytes with the bit pattern of 'byte' within
             * it, xoring with 'word' will leave each of the 8 bits in such
             * bytes be 0, and no byte containing any other bit pattern will be
             * 0. */
            masked ^= word;

            /* This causes the most significant bit to be set to 1 for any
             * bytes in the word that aren't completely 0 */
            masked |= masked << 1;
            masked |= masked << 2;
            masked |= masked << 4;

            /* The msbits are the same as what marks a byte as variant, so we
             * can use this mask.  If all msbits are 1, the word doesn't
             * contain 'byte' */
            if ((masked & PERL_VARIANTS_WORD_MASK) == PERL_VARIANTS_WORD_MASK) {
                s += PERL_WORDSIZE;
                continue;
            }

            /* Here, the msbit of bytes in the word that aren't 'byte' are 1,
             * and any that are, are 0.  Complement and re-AND to swap that */
            masked = ~ masked;
            masked &= PERL_VARIANTS_WORD_MASK;

            /* This reduces the problem to that solved by this function */
            s += variant_byte_number(masked);
            return s;

        } while (s + PERL_WORDSIZE <= send);
    }

#endif

    while (s < send) {
        if (((*s) & mask) == byte) {
            return s;
        }
        s++;
    }

    return s;
}

STATIC U8 *
S_find_span_end_mask(U8 * s, const U8 * send, const U8 span_byte, const U8 mask)
{
    /* Returns the position of the first byte in the sequence between 's' and
     * 'send-1' inclusive that when ANDed with 'mask' isn't 'span_byte'.
     * 'span_byte' should have been ANDed with 'mask' in the call of this
     * function.  Returns 'send' if none found.  Works like find_span_end(),
     * except for the AND */

    PERL_ARGS_ASSERT_FIND_SPAN_END_MASK;

    assert(send >= s);
    assert((span_byte & mask) == span_byte);

    if ((STRLEN) (send - s) >= PERL_WORDSIZE
                          + PERL_WORDSIZE * PERL_IS_SUBWORD_ADDR(s)
                          - (PTR2nat(s) & PERL_WORD_BOUNDARY_MASK))
    {
        PERL_UINTMAX_T span_word, mask_word;

        while (PTR2nat(s) & PERL_WORD_BOUNDARY_MASK) {
            if (((*s) & mask) != span_byte) {
                return s;
            }
            s++;
        }

        span_word = PERL_COUNT_MULTIPLIER * span_byte;
        mask_word = PERL_COUNT_MULTIPLIER * mask;

        do {
            PERL_UINTMAX_T masked = (* (PERL_UINTMAX_T *) s) & mask_word;

            if (masked == span_word) {
                s += PERL_WORDSIZE;
                continue;
            }

#ifdef EBCDIC

            break;

#else

            masked ^= span_word;
            masked |= masked << 1;
            masked |= masked << 2;
            masked |= masked << 4;
            return s + variant_byte_number(masked);

#endif

        } while (s + PERL_WORDSIZE <= send);
    }

    while (s < send) {
        if (((*s) & mask) != span_byte) {
            return s;
        }
        s++;
    }

    return s;
}

/*
 * pregexec and friends
 */

#ifndef PERL_IN_XSUB_RE
/*
 - pregexec - match a regexp against a string
 */
I32
Perl_pregexec(pTHX_ REGEXP * const prog, char* stringarg, char *strend,
         char *strbeg, SSize_t minend, SV *screamer, U32 nosave)
/* stringarg: the point in the string at which to begin matching */
/* strend:    pointer to null at end of string */
/* strbeg:    real beginning of string */
/* minend:    end of match must be >= minend bytes after stringarg. */
/* screamer:  SV being matched: only used for utf8 flag, pos() etc; string
 *            itself is accessed via the pointers above */
/* nosave:    For optimizations. */
{
    PERL_ARGS_ASSERT_PREGEXEC;

    return
        regexec_flags(prog, stringarg, strend, strbeg, minend, screamer, NULL,
                      nosave ? 0 : REXEC_COPY_STR);
}
#endif



/* re_intuit_start():
 *
 * Based on some optimiser hints, try to find the earliest position in the
 * string where the regex could match.
 *
 *   rx:     the regex to match against
 *   sv:     the SV being matched: only used for utf8 flag; the string
 *           itself is accessed via the pointers below. Note that on
 *           something like an overloaded SV, SvPOK(sv) may be false
 *           and the string pointers may point to something unrelated to
 *           the SV itself.
 *   strbeg: real beginning of string
 *   strpos: the point in the string at which to begin matching
 *   strend: pointer to the byte following the last char of the string
 *   flags   currently unused; set to 0
 *   data:   currently unused; set to NULL
 *
 * The basic idea of re_intuit_start() is to use some known information
 * about the pattern, namely:
 *
 *   a) the longest known anchored substring (i.e. one that's at a
 *      constant offset from the beginning of the pattern; but not
 *      necessarily at a fixed offset from the beginning of the
 *      string);
 *   b) the longest floating substring (i.e. one that's not at a constant
 *      offset from the beginning of the pattern);
 *   c) Whether the pattern is anchored to the string; either
 *      an absolute anchor: /^../, or anchored to \n: /^.../m,
 *      or anchored to pos(): /\G/;
 *   d) A start class: a real or synthetic character class which
 *      represents which characters are legal at the start of the pattern;
 *
 * to either quickly reject the match, or to find the earliest position
 * within the string at which the pattern might match, thus avoiding
 * running the full NFA engine at those earlier locations, only to
 * eventually fail and retry further along.
 *
 * Returns NULL if the pattern can't match, or returns the address within
 * the string which is the earliest place the match could occur.
 *
 * The longest of the anchored and floating substrings is called 'check'
 * and is checked first. The other is called 'other' and is checked
 * second. The 'other' substring may not be present.  For example,
 *
 *    /(abc|xyz)ABC\d{0,3}DEFG/
 *
 * will have
 *
 *   check substr (float)    = "DEFG", offset 6..9 chars
 *   other substr (anchored) = "ABC",  offset 3..3 chars
 *   stclass = [ax]
 *
 * Be aware that during the course of this function, sometimes 'anchored'
 * refers to a substring being anchored relative to the start of the
 * pattern, and sometimes to the pattern itself being anchored relative to
 * the string. For example:
 *
 *   /\dabc/:   "abc" is anchored to the pattern;
 *   /^\dabc/:  "abc" is anchored to the pattern and the string;
 *   /\d+abc/:  "abc" is anchored to neither the pattern nor the string;
 *   /^\d+abc/: "abc" is anchored to neither the pattern nor the string,
 *                    but the pattern is anchored to the string.
 */

char *
Perl_re_intuit_start(pTHX_
                    REGEXP * const rx,
                    SV *sv,
                    const char * const strbeg,
                    char *strpos,
                    char *strend,
                    const U32 flags,
                    re_scream_pos_data *data)
{
    struct regexp *const prog = ReANY(rx);
    SSize_t start_shift = prog->check_offset_min;
    /* Should be nonnegative! */
    SSize_t end_shift   = 0;
    /* current lowest pos in string where the regex can start matching */
    char *rx_origin = strpos;
    SV *check;
    const bool utf8_target = (sv && SvUTF8(sv)) ? 1 : 0; /* if no sv we have to assume bytes */
    U8   other_ix = 1 - prog->substrs->check_ix;
    bool ml_anch = 0;
    char *other_last = strpos;/* latest pos 'other' substr already checked to */
    char *check_at = NULL;		/* check substr found at this pos */
    const I32 multiline = prog->extflags & RXf_PMf_MULTILINE;
    RXi_GET_DECL(prog,progi);
    regmatch_info reginfo_buf;  /* create some info to pass to find_byclass */
    regmatch_info *const reginfo = &reginfo_buf;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_RE_INTUIT_START;
    PERL_UNUSED_ARG(flags);
    PERL_UNUSED_ARG(data);

    DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                "Intuit: trying to determine minimum start position...\n"));

    /* for now, assume that all substr offsets are positive. If at some point
     * in the future someone wants to do clever things with lookbehind and
     * -ve offsets, they'll need to fix up any code in this function
     * which uses these offsets. See the thread beginning
     * <20140113145929.GF27210@iabyn.com>
     */
    assert(prog->substrs->data[0].min_offset >= 0);
    assert(prog->substrs->data[0].max_offset >= 0);
    assert(prog->substrs->data[1].min_offset >= 0);
    assert(prog->substrs->data[1].max_offset >= 0);
    assert(prog->substrs->data[2].min_offset >= 0);
    assert(prog->substrs->data[2].max_offset >= 0);

    /* for now, assume that if both present, that the floating substring
     * doesn't start before the anchored substring.
     * If you break this assumption (e.g. doing better optimisations
     * with lookahead/behind), then you'll need to audit the code in this
     * function carefully first
     */
    assert(
            ! (  (prog->anchored_utf8 || prog->anchored_substr)
              && (prog->float_utf8    || prog->float_substr))
           || (prog->float_min_offset >= prog->anchored_offset));

    /* byte rather than char calculation for efficiency. It fails
     * to quickly reject some cases that can't match, but will reject
     * them later after doing full char arithmetic */
    if (prog->minlen > strend - strpos) {
        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                              "  String too short...\n"));
        goto fail;
    }

    RXp_MATCH_UTF8_set(prog, utf8_target);
    reginfo->is_utf8_target = cBOOL(utf8_target);
    reginfo->info_aux = NULL;
    reginfo->strbeg = strbeg;
    reginfo->strend = strend;
    reginfo->is_utf8_pat = cBOOL(RX_UTF8(rx));
    reginfo->intuit = 1;
    /* not actually used within intuit, but zero for safety anyway */
    reginfo->poscache_maxiter = 0;

    if (utf8_target) {
        if ((!prog->anchored_utf8 && prog->anchored_substr)
                || (!prog->float_utf8 && prog->float_substr))
            to_utf8_substr(prog);
        check = prog->check_utf8;
    } else {
        if (!prog->check_substr && prog->check_utf8) {
            if (! to_byte_substr(prog)) {
                NON_UTF8_TARGET_BUT_UTF8_REQUIRED(fail);
            }
        }
        check = prog->check_substr;
    }

    /* dump the various substring data */
    DEBUG_OPTIMISE_MORE_r({
        int i;
        for (i=0; i<=2; i++) {
            SV *sv = (utf8_target ? prog->substrs->data[i].utf8_substr
                                  : prog->substrs->data[i].substr);
            if (!sv)
                continue;

            Perl_re_printf( aTHX_
                "  substrs[%d]: min=%" IVdf " max=%" IVdf " end shift=%" IVdf
                " useful=%" IVdf " utf8=%d [%s]\n",
                i,
                (IV)prog->substrs->data[i].min_offset,
                (IV)prog->substrs->data[i].max_offset,
                (IV)prog->substrs->data[i].end_shift,
                BmUSEFUL(sv),
                utf8_target ? 1 : 0,
                SvPEEK(sv));
        }
    });

    if (prog->intflags & PREGf_ANCH) { /* Match at \G, beg-of-str or after \n */

        /* ml_anch: check after \n?
         *
         * A note about PREGf_IMPLICIT: on an un-anchored pattern beginning
         * with /.*.../, these flags will have been added by the
         * compiler:
         *   /.*abc/, /.*abc/m:  PREGf_IMPLICIT | PREGf_ANCH_MBOL
         *   /.*abc/s:           PREGf_IMPLICIT | PREGf_ANCH_SBOL
         */
        ml_anch =      (prog->intflags & PREGf_ANCH_MBOL)
                   && !(prog->intflags & PREGf_IMPLICIT);

        if (!ml_anch && !(prog->intflags & PREGf_IMPLICIT)) {
            /* we are only allowed to match at BOS or \G */

            /* trivially reject if there's a BOS anchor and we're not at BOS.
             *
             * Note that we don't try to do a similar quick reject for
             * \G, since generally the caller will have calculated strpos
             * based on pos() and gofs, so the string is already correctly
             * anchored by definition; and handling the exceptions would
             * be too fiddly (e.g. REXEC_IGNOREPOS).
             */
            if (   strpos != strbeg
                && (prog->intflags & PREGf_ANCH_SBOL))
            {
                DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                                "  Not at start...\n"));
                goto fail;
            }

            /* in the presence of an anchor, the anchored (relative to the
             * start of the regex) substr must also be anchored relative
             * to strpos. So quickly reject if substr isn't found there.
             * This works for \G too, because the caller will already have
             * subtracted gofs from pos, and gofs is the offset from the
             * \G to the start of the regex. For example, in /.abc\Gdef/,
             * where substr="abcdef", pos()=3, gofs=4, offset_min=1:
             * caller will have set strpos=pos()-4; we look for the substr
             * at position pos()-4+1, which lines up with the "a" */

            if (prog->check_offset_min == prog->check_offset_max) {
                /* Substring at constant offset from beg-of-str... */
                SSize_t slen = SvCUR(check);
                char *s = HOP3c(strpos, prog->check_offset_min, strend);

                DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                    "  Looking for check substr at fixed offset %" IVdf "...\n",
                    (IV)prog->check_offset_min));

                if (SvTAIL(check)) {
                    /* In this case, the regex is anchored at the end too.
                     * Unless it's a multiline match, the lengths must match
                     * exactly, give or take a \n.  NB: slen >= 1 since
                     * the last char of check is \n */
                    if (!multiline
                        && (   strend - s > slen
                            || strend - s < slen - 1
                            || (strend - s == slen && strend[-1] != '\n')))
                    {
                        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                                            "  String too long...\n"));
                        goto fail_finish;
                    }
                    /* Now should match s[0..slen-2] */
                    slen--;
                }
                if (slen && (strend - s < slen
                    || *SvPVX_const(check) != *s
                    || (slen > 1 && (memNE(SvPVX_const(check), s, slen)))))
                {
                    DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                                    "  String not equal...\n"));
                    goto fail_finish;
                }

                check_at = s;
                goto success_at_start;
            }
        }
    }

    end_shift = prog->check_end_shift;

#ifdef DEBUGGING	/* 7/99: reports of failure (with the older version) */
    if (end_shift < 0)
        Perl_croak(aTHX_ "panic: end_shift: %" IVdf " pattern:\n%s\n ",
                   (IV)end_shift, RX_PRECOMP(rx));
#endif

  restart:

    /* This is the (re)entry point of the main loop in this function.
     * The goal of this loop is to:
     * 1) find the "check" substring in the region rx_origin..strend
     *    (adjusted by start_shift / end_shift). If not found, reject
     *    immediately.
     * 2) If it exists, look for the "other" substr too if defined; for
     *    example, if the check substr maps to the anchored substr, then
     *    check the floating substr, and vice-versa. If not found, go
     *    back to (1) with rx_origin suitably incremented.
     * 3) If we find an rx_origin position that doesn't contradict
     *    either of the substrings, then check the possible additional
     *    constraints on rx_origin of /^.../m or a known start class.
     *    If these fail, then depending on which constraints fail, jump
     *    back to here, or to various other re-entry points further along
     *    that skip some of the first steps.
     * 4) If we pass all those tests, update the BmUSEFUL() count on the
     *    substring. If the start position was determined to be at the
     *    beginning of the string  - so, not rejected, but not optimised,
     *    since we have to run regmatch from position 0 - decrement the
     *    BmUSEFUL() count. Otherwise increment it.
     */


    /* first, look for the 'check' substring */

    {
        U8* start_point;
        U8* end_point;

        DEBUG_OPTIMISE_MORE_r({
            Perl_re_printf( aTHX_
                "  At restart: rx_origin=%" IVdf " Check offset min: %" IVdf
                " Start shift: %" IVdf " End shift %" IVdf
                " Real end Shift: %" IVdf "\n",
                (IV)(rx_origin - strbeg),
                (IV)prog->check_offset_min,
                (IV)start_shift,
                (IV)end_shift,
                (IV)prog->check_end_shift);
        });

        end_point = HOPBACK3(strend, end_shift, rx_origin);
        if (!end_point)
            goto fail_finish;
        start_point = HOPMAYBE3(rx_origin, start_shift, end_point);
        if (!start_point)
            goto fail_finish;


        /* If the regex is absolutely anchored to either the start of the
         * string (SBOL) or to pos() (ANCH_GPOS), then
         * check_offset_max represents an upper bound on the string where
         * the substr could start. For the ANCH_GPOS case, we assume that
         * the caller of intuit will have already set strpos to
         * pos()-gofs, so in this case strpos + offset_max will still be
         * an upper bound on the substr.
         */
        if (!ml_anch
            && prog->intflags & PREGf_ANCH
            && prog->check_offset_max != SSize_t_MAX)
        {
            SSize_t check_len = SvCUR(check) - cBOOL(SvTAIL(check));
            const char * const anchor =
                        (prog->intflags & PREGf_ANCH_GPOS ? strpos : strbeg);
            SSize_t targ_len = (char*)end_point - anchor;

            if (check_len > targ_len) {
                DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                              "Target string too short to match required substring...\n"));
                goto fail_finish;
            }

            /* do a bytes rather than chars comparison. It's conservative;
             * so it skips doing the HOP if the result can't possibly end
             * up earlier than the old value of end_point.
             */
            assert(anchor + check_len <= (char *)end_point);
            if (prog->check_offset_max + check_len < targ_len) {
                end_point = HOP3lim((U8*)anchor,
                                prog->check_offset_max,
                                end_point - check_len
                            )
                            + check_len;
                if (end_point < start_point)
                    goto fail_finish;
            }
        }

        check_at = fbm_instr( start_point, end_point,
                      check, multiline ? FBMrf_MULTILINE : 0);

        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
            "  doing 'check' fbm scan, [%" IVdf "..%" IVdf "] gave %" IVdf "\n",
            (IV)((char*)start_point - strbeg),
            (IV)((char*)end_point   - strbeg),
            (IV)(check_at ? check_at - strbeg : -1)
        ));

        /* Update the count-of-usability, remove useless subpatterns,
            unshift s.  */

        DEBUG_EXECUTE_r({
            RE_PV_QUOTED_DECL(quoted, utf8_target, PERL_DEBUG_PAD_ZERO(0),
                SvPVX_const(check), RE_SV_DUMPLEN(check), 30);
            Perl_re_printf( aTHX_  "  %s %s substr %s%s%s",
                              (check_at ? "Found" : "Did not find"),
                (check == (utf8_target ? prog->anchored_utf8 : prog->anchored_substr)
                    ? "anchored" : "floating"),
                quoted,
                RE_SV_TAIL(check),
                (check_at ? " at offset " : "...\n") );
        });

        if (!check_at)
            goto fail_finish;
        /* set rx_origin to the minimum position where the regex could start
         * matching, given the constraint of the just-matched check substring.
         * But don't set it lower than previously.
         */

        if (check_at - rx_origin > prog->check_offset_max)
            rx_origin = HOP3c(check_at, -prog->check_offset_max, rx_origin);
        /* Finish the diagnostic message */
        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
            "%ld (rx_origin now %" IVdf ")...\n",
            (long)(check_at - strbeg),
            (IV)(rx_origin - strbeg)
        ));
    }


    /* now look for the 'other' substring if defined */

    if (prog->substrs->data[other_ix].utf8_substr
        || prog->substrs->data[other_ix].substr)
    {
        /* Take into account the "other" substring. */
        char *last, *last1;
        char *s;
        SV* must;
        struct reg_substr_datum *other;

      do_other_substr:
        other = &prog->substrs->data[other_ix];
        if (!utf8_target && !other->substr) {
            if (!to_byte_substr(prog)) {
                NON_UTF8_TARGET_BUT_UTF8_REQUIRED(fail);
            }
        }

        /* if "other" is anchored:
         * we've previously found a floating substr starting at check_at.
         * This means that the regex origin must lie somewhere
         * between min (rx_origin): HOP3(check_at, -check_offset_max)
         * and max:                 HOP3(check_at, -check_offset_min)
         * (except that min will be >= strpos)
         * So the fixed  substr must lie somewhere between
         *  HOP3(min, anchored_offset)
         *  HOP3(max, anchored_offset) + SvCUR(substr)
         */

        /* if "other" is floating
         * Calculate last1, the absolute latest point where the
         * floating substr could start in the string, ignoring any
         * constraints from the earlier fixed match. It is calculated
         * as follows:
         *
         * strend - prog->minlen (in chars) is the absolute latest
         * position within the string where the origin of the regex
         * could appear. The latest start point for the floating
         * substr is float_min_offset(*) on from the start of the
         * regex.  last1 simply combines thee two offsets.
         *
         * (*) You might think the latest start point should be
         * float_max_offset from the regex origin, and technically
         * you'd be correct. However, consider
         *    /a\d{2,4}bcd\w/
         * Here, float min, max are 3,5 and minlen is 7.
         * This can match either
         *    /a\d\dbcd\w/
         *    /a\d\d\dbcd\w/
         *    /a\d\d\d\dbcd\w/
         * In the first case, the regex matches minlen chars; in the
         * second, minlen+1, in the third, minlen+2.
         * In the first case, the floating offset is 3 (which equals
         * float_min), in the second, 4, and in the third, 5 (which
         * equals float_max). In all cases, the floating string bcd
         * can never start more than 4 chars from the end of the
         * string, which equals minlen - float_min. As the substring
         * starts to match more than float_min from the start of the
         * regex, it makes the regex match more than minlen chars,
         * and the two cancel each other out. So we can always use
         * float_min - minlen, rather than float_max - minlen for the
         * latest position in the string.
         *
         * Note that -minlen + float_min_offset is equivalent (AFAIKT)
         * to CHR_SVLEN(must) - !!SvTAIL(must) + prog->float_end_shift
         */

        assert(prog->minlen >= other->min_offset);
        last1 = HOP3c(strend,
                        other->min_offset - prog->minlen, strbeg);

        if (other_ix) {/* i.e. if (other-is-float) */
            /* last is the latest point where the floating substr could
             * start, *given* any constraints from the earlier fixed
             * match. This constraint is that the floating string starts
             * <= float_max_offset chars from the regex origin (rx_origin).
             * If this value is less than last1, use it instead.
             */
            assert(rx_origin <= last1);
            last =
                /* this condition handles the offset==infinity case, and
                 * is a short-cut otherwise. Although it's comparing a
                 * byte offset to a char length, it does so in a safe way,
                 * since 1 char always occupies 1 or more bytes,
                 * so if a string range is  (last1 - rx_origin) bytes,
                 * it will be less than or equal to  (last1 - rx_origin)
                 * chars; meaning it errs towards doing the accurate HOP3
                 * rather than just using last1 as a short-cut */
                (last1 - rx_origin) < other->max_offset
                    ? last1
                    : (char*)HOP3lim(rx_origin, other->max_offset, last1);
        }
        else {
            assert(strpos + start_shift <= check_at);
            last = HOP4c(check_at, other->min_offset - start_shift,
                        strbeg, strend);
        }

        s = HOP3c(rx_origin, other->min_offset, strend);
        if (s < other_last)	/* These positions already checked */
            s = other_last;

        must = utf8_target ? other->utf8_substr : other->substr;
        assert(SvPOK(must));
        {
            char *from = s;
            char *to   = last + SvCUR(must) - (SvTAIL(must)!=0);

            if (to > strend)
                to = strend;
            if (from > to) {
                s = NULL;
                DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                    "  skipping 'other' fbm scan: %" IVdf " > %" IVdf "\n",
                    (IV)(from - strbeg),
                    (IV)(to   - strbeg)
                ));
            }
            else {
                s = fbm_instr(
                    (unsigned char*)from,
                    (unsigned char*)to,
                    must,
                    multiline ? FBMrf_MULTILINE : 0
                );
                DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                    "  doing 'other' fbm scan, [%" IVdf "..%" IVdf "] gave %" IVdf "\n",
                    (IV)(from - strbeg),
                    (IV)(to   - strbeg),
                    (IV)(s ? s - strbeg : -1)
                ));
            }
        }

        DEBUG_EXECUTE_r({
            RE_PV_QUOTED_DECL(quoted, utf8_target, PERL_DEBUG_PAD_ZERO(0),
                SvPVX_const(must), RE_SV_DUMPLEN(must), 30);
            Perl_re_printf( aTHX_  "  %s %s substr %s%s",
                s ? "Found" : "Contradicts",
                other_ix ? "floating" : "anchored",
                quoted, RE_SV_TAIL(must));
        });


        if (!s) {
            /* last1 is latest possible substr location. If we didn't
             * find it before there, we never will */
            if (last >= last1) {
                DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                                        "; giving up...\n"));
                goto fail_finish;
            }

            /* try to find the check substr again at a later
             * position. Maybe next time we'll find the "other" substr
             * in range too */
            other_last = HOP3c(last, 1, strend) /* highest failure */;
            rx_origin =
                other_ix /* i.e. if other-is-float */
                    ? HOP3c(rx_origin, 1, strend)
                    : HOP4c(last, 1 - other->min_offset, strbeg, strend);
            DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                "; about to retry %s at offset %ld (rx_origin now %" IVdf ")...\n",
                (other_ix ? "floating" : "anchored"),
                (long)(HOP3c(check_at, 1, strend) - strbeg),
                (IV)(rx_origin - strbeg)
            ));
            goto restart;
        }
        else {
            if (other_ix) { /* if (other-is-float) */
                /* other_last is set to s, not s+1, since its possible for
                 * a floating substr to fail first time, then succeed
                 * second time at the same floating position; e.g.:
                 *     "-AB--AABZ" =~ /\wAB\d*Z/
                 * The first time round, anchored and float match at
                 * "-(AB)--AAB(Z)" then fail on the initial \w character
                 * class. Second time round, they match at "-AB--A(AB)(Z)".
                 */
                other_last = s;
            }
            else {
                rx_origin = HOP3c(s, -other->min_offset, strbeg);
                other_last = HOP3c(s, 1, strend);
            }
            DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                " at offset %ld (rx_origin now %" IVdf ")...\n",
                  (long)(s - strbeg),
                (IV)(rx_origin - strbeg)
              ));

        }
    }
    else {
        DEBUG_OPTIMISE_MORE_r(
            Perl_re_printf( aTHX_
                "  Check-only match: offset min:%" IVdf " max:%" IVdf
                " check_at:%" IVdf " rx_origin:%" IVdf " rx_origin-check_at:%" IVdf
                " strend:%" IVdf "\n",
                (IV)prog->check_offset_min,
                (IV)prog->check_offset_max,
                (IV)(check_at-strbeg),
                (IV)(rx_origin-strbeg),
                (IV)(rx_origin-check_at),
                (IV)(strend-strbeg)
            )
        );
    }

  postprocess_substr_matches:

    /* handle the extra constraint of /^.../m if present */

    if (ml_anch && rx_origin != strbeg && rx_origin[-1] != '\n') {
        char *s;

        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                        "  looking for /^/m anchor"));

        /* we have failed the constraint of a \n before rx_origin.
         * Find the next \n, if any, even if it's beyond the current
         * anchored and/or floating substrings. Whether we should be
         * scanning ahead for the next \n or the next substr is debatable.
         * On the one hand you'd expect rare substrings to appear less
         * often than \n's. On the other hand, searching for \n means
         * we're effectively flipping between check_substr and "\n" on each
         * iteration as the current "rarest" candidate string, which
         * means for example that we'll quickly reject the whole string if
         * hasn't got a \n, rather than trying every substr position
         * first
         */

        s = HOP3c(strend, - prog->minlen, strpos);
        if (s <= rx_origin ||
            ! ( rx_origin = (char *)memchr(rx_origin, '\n', s - rx_origin)))
        {
            DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                            "  Did not find /%s^%s/m...\n",
                            PL_colors[0], PL_colors[1]));
            goto fail_finish;
        }

        /* earliest possible origin is 1 char after the \n.
         * (since *rx_origin == '\n', it's safe to ++ here rather than
         * HOP(rx_origin, 1)) */
        rx_origin++;

        if (prog->substrs->check_ix == 0  /* check is anchored */
            || rx_origin >= HOP3c(check_at,  - prog->check_offset_min, strpos))
        {
            /* Position contradicts check-string; either because
             * check was anchored (and thus has no wiggle room),
             * or check was float and rx_origin is above the float range */
            DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                "  Found /%s^%s/m, about to restart lookup for check-string with rx_origin %ld...\n",
                PL_colors[0], PL_colors[1], (long)(rx_origin - strbeg)));
            goto restart;
        }

        /* if we get here, the check substr must have been float,
         * is in range, and we may or may not have had an anchored
         * "other" substr which still contradicts */
        assert(prog->substrs->check_ix); /* check is float */

        if (utf8_target ? prog->anchored_utf8 : prog->anchored_substr) {
            /* whoops, the anchored "other" substr exists, so we still
             * contradict. On the other hand, the float "check" substr
             * didn't contradict, so just retry the anchored "other"
             * substr */
            DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                "  Found /%s^%s/m, rescanning for anchored from offset %" IVdf " (rx_origin now %" IVdf ")...\n",
                PL_colors[0], PL_colors[1],
                (IV)(rx_origin - strbeg + prog->anchored_offset),
                (IV)(rx_origin - strbeg)
            ));
            goto do_other_substr;
        }

        /* success: we don't contradict the found floating substring
         * (and there's no anchored substr). */
        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
            "  Found /%s^%s/m with rx_origin %ld...\n",
            PL_colors[0], PL_colors[1], (long)(rx_origin - strbeg)));
    }
    else {
        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
            "  (multiline anchor test skipped)\n"));
    }

  success_at_start:


    /* if we have a starting character class, then test that extra constraint.
     * (trie stclasses are too expensive to use here, we are better off to
     * leave it to regmatch itself) */

    if (progi->regstclass && REGNODE_TYPE(OP(progi->regstclass))!=TRIE) {
        const U8* const str = (U8*)STRING(progi->regstclass);

        /* XXX this value could be pre-computed */
        const SSize_t cl_l = (REGNODE_TYPE(OP(progi->regstclass)) == EXACT
                    ?  (reginfo->is_utf8_pat
                        ? (SSize_t)utf8_distance(str + STR_LEN(progi->regstclass), str)
                        : (SSize_t)STR_LEN(progi->regstclass))
                    : 1);
        char * endpos;
        char *s;
        /* latest pos that a matching float substr constrains rx start to */
        char *rx_max_float = NULL;

        /* if the current rx_origin is anchored, either by satisfying an
         * anchored substring constraint, or a /^.../m constraint, then we
         * can reject the current origin if the start class isn't found
         * at the current position. If we have a float-only match, then
         * rx_origin is constrained to a range; so look for the start class
         * in that range. if neither, then look for the start class in the
         * whole rest of the string */

        /* XXX DAPM it's not clear what the minlen test is for, and why
         * it's not used in the floating case. Nothing in the test suite
         * causes minlen == 0 here. See <20140313134639.GS12844@iabyn.com>.
         * Here are some old comments, which may or may not be correct:
         *
         *   minlen == 0 is possible if regstclass is \b or \B,
         *   and the fixed substr is ''$.
         *   Since minlen is already taken into account, rx_origin+1 is
         *   before strend; accidentally, minlen >= 1 guaranties no false
         *   positives at rx_origin + 1 even for \b or \B.  But (minlen? 1 :
         *   0) below assumes that regstclass does not come from lookahead...
         *   If regstclass takes bytelength more than 1: If charlength==1, OK.
         *   This leaves EXACTF-ish only, which are dealt with in
         *   find_byclass().
         */

        if (prog->anchored_substr || prog->anchored_utf8 || ml_anch)
            endpos = HOP3clim(rx_origin, (prog->minlen ? cl_l : 0), strend);
        else if (prog->float_substr || prog->float_utf8) {
            rx_max_float = HOP3c(check_at, -start_shift, strbeg);
            endpos = HOP3clim(rx_max_float, cl_l, strend);
        }
        else
            endpos= strend;

        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
            "  looking for class: start_shift: %" IVdf " check_at: %" IVdf
            " rx_origin: %" IVdf " endpos: %" IVdf "\n",
              (IV)start_shift, (IV)(check_at - strbeg),
              (IV)(rx_origin - strbeg), (IV)(endpos - strbeg)));

        s = find_byclass(prog, progi->regstclass, rx_origin, endpos,
                            reginfo);
        if (!s) {
            if (endpos == strend) {
                DEBUG_EXECUTE_r( Perl_re_printf( aTHX_
                                "  Could not match STCLASS...\n") );
                goto fail;
            }
            DEBUG_EXECUTE_r( Perl_re_printf( aTHX_
                               "  This position contradicts STCLASS...\n") );
            if ((prog->intflags & PREGf_ANCH) && !ml_anch
                        && !(prog->intflags & PREGf_IMPLICIT))
                goto fail;

            /* Contradict one of substrings */
            if (prog->anchored_substr || prog->anchored_utf8) {
                if (prog->substrs->check_ix == 1) { /* check is float */
                    /* Have both, check_string is floating */
                    assert(rx_origin + start_shift <= check_at);
                    if (rx_origin + start_shift != check_at) {
                        /* not at latest position float substr could match:
                         * Recheck anchored substring, but not floating.
                         * The condition above is in bytes rather than
                         * chars for efficiency. It's conservative, in
                         * that it errs on the side of doing 'goto
                         * do_other_substr'. In this case, at worst,
                         * an extra anchored search may get done, but in
                         * practice the extra fbm_instr() is likely to
                         * get skipped anyway. */
                        DEBUG_EXECUTE_r( Perl_re_printf( aTHX_
                            "  about to retry anchored at offset %ld (rx_origin now %" IVdf ")...\n",
                            (long)(other_last - strbeg),
                            (IV)(rx_origin - strbeg)
                        ));
                        goto do_other_substr;
                    }
                }
            }
            else {
                /* float-only */

                if (ml_anch) {
                    /* In the presence of ml_anch, we might be able to
                     * find another \n without breaking the current float
                     * constraint. */

                    /* strictly speaking this should be HOP3c(..., 1, ...),
                     * but since we goto a block of code that's going to
                     * search for the next \n if any, its safe here */
                    rx_origin++;
                    DEBUG_EXECUTE_r( Perl_re_printf( aTHX_
                              "  about to look for /%s^%s/m starting at rx_origin %ld...\n",
                              PL_colors[0], PL_colors[1],
                              (long)(rx_origin - strbeg)) );
                    goto postprocess_substr_matches;
                }

                /* strictly speaking this can never be true; but might
                 * be if we ever allow intuit without substrings */
                if (!(utf8_target ? prog->float_utf8 : prog->float_substr))
                    goto fail;

                rx_origin = rx_max_float;
            }

            /* at this point, any matching substrings have been
             * contradicted. Start again... */

            rx_origin = HOP3c(rx_origin, 1, strend);

            /* uses bytes rather than char calculations for efficiency.
             * It's conservative: it errs on the side of doing 'goto restart',
             * where there is code that does a proper char-based test */
            if (rx_origin + start_shift + end_shift > strend) {
                DEBUG_EXECUTE_r( Perl_re_printf( aTHX_
                                       "  Could not match STCLASS...\n") );
                goto fail;
            }
            DEBUG_EXECUTE_r( Perl_re_printf( aTHX_
                "  about to look for %s substr starting at offset %ld (rx_origin now %" IVdf ")...\n",
                (prog->substrs->check_ix ? "floating" : "anchored"),
                (long)(rx_origin + start_shift - strbeg),
                (IV)(rx_origin - strbeg)
            ));
            goto restart;
        }

        /* Success !!! */

        if (rx_origin != s) {
            DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                        "  By STCLASS: moving %ld --> %ld\n",
                                  (long)(rx_origin - strbeg), (long)(s - strbeg))
                   );
        }
        else {
            DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                                  "  Does not contradict STCLASS...\n");
                   );
        }
    }

    /* Decide whether using the substrings helped */

    if (rx_origin != strpos) {
        /* Fixed substring is found far enough so that the match
           cannot start at strpos. */

        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_  "  try at offset...\n"));
        ++BmUSEFUL(utf8_target ? prog->check_utf8 : prog->check_substr);	/* hooray/5 */
    }
    else {
        /* The found rx_origin position does not prohibit matching at
         * strpos, so calling intuit didn't gain us anything. Decrement
         * the BmUSEFUL() count on the check substring, and if we reach
         * zero, free it.  */
        if (!(prog->intflags & PREGf_NAUGHTY)
            && (utf8_target ? (
                prog->check_utf8		/* Could be deleted already */
                && --BmUSEFUL(prog->check_utf8) < 0
                && (prog->check_utf8 == prog->float_utf8)
            ) : (
                prog->check_substr		/* Could be deleted already */
                && --BmUSEFUL(prog->check_substr) < 0
                && (prog->check_substr == prog->float_substr)
            )))
        {
            /* If flags & SOMETHING - do not do it many times on the same match */
            DEBUG_EXECUTE_r(Perl_re_printf( aTHX_  "  ... Disabling check substring...\n"));
            /* XXX Does the destruction order has to change with utf8_target? */
            SvREFCNT_dec(utf8_target ? prog->check_utf8 : prog->check_substr);
            SvREFCNT_dec(utf8_target ? prog->check_substr : prog->check_utf8);
            prog->check_substr = prog->check_utf8 = NULL;	/* disable */
            prog->float_substr = prog->float_utf8 = NULL;	/* clear */
            check = NULL;			/* abort */
            /* XXXX This is a remnant of the old implementation.  It
                    looks wasteful, since now INTUIT can use many
                    other heuristics. */
            prog->extflags &= ~RXf_USE_INTUIT;
        }
    }

    DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
            "Intuit: %sSuccessfully guessed:%s match at offset %ld\n",
             PL_colors[4], PL_colors[5], (long)(rx_origin - strbeg)) );

    return rx_origin;

  fail_finish:				/* Substring not found */
    if (prog->check_substr || prog->check_utf8)		/* could be removed already */
        BmUSEFUL(utf8_target ? prog->check_utf8 : prog->check_substr) += 5; /* hooray */
  fail:
    DEBUG_EXECUTE_r(Perl_re_printf( aTHX_  "%sMatch rejected by optimizer%s\n",
                          PL_colors[4], PL_colors[5]));
    return NULL;
}


#define DECL_TRIE_TYPE(scan) \
    const enum { trie_plain, trie_utf8, trie_utf8_fold, trie_latin_utf8_fold,       \
                 trie_utf8_exactfa_fold, trie_latin_utf8_exactfa_fold,              \
                 trie_utf8l, trie_flu8, trie_flu8_latin }                           \
                    trie_type = ((FLAGS(scan) == EXACT)                             \
                                 ? (utf8_target ? trie_utf8 : trie_plain)           \
                                 : (FLAGS(scan) == EXACTL)                          \
                                    ? (utf8_target ? trie_utf8l : trie_plain)       \
                                    : (FLAGS(scan) == EXACTFAA)                     \
                                      ? (utf8_target                                \
                                         ? trie_utf8_exactfa_fold                   \
                                         : trie_latin_utf8_exactfa_fold)            \
                                      : (FLAGS(scan) == EXACTFLU8                   \
                                         ? (utf8_target                             \
                                           ? trie_flu8                              \
                                           : trie_flu8_latin)                       \
                                         : (utf8_target                             \
                                           ? trie_utf8_fold                         \
                                           : trie_latin_utf8_fold)))

/* 'uscan' is set to foldbuf, and incremented, so below the end of uscan is
 * 'foldbuf+sizeof(foldbuf)' */
#define REXEC_TRIE_READ_CHAR(trie_type, trie, widecharmap, uc, uc_end, uscan, len, uvc, charid, foldlen, foldbuf, uniflags) \
STMT_START {                                                                        \
    STRLEN skiplen;                                                                 \
    U8 flags = FOLD_FLAGS_FULL;                                                     \
    switch (trie_type) {                                                            \
    case trie_flu8:                                                                 \
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;                                         \
        if (UTF8_IS_ABOVE_LATIN1(*uc)) {                                            \
            _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(uc, uc_end);                     \
        }                                                                           \
        goto do_trie_utf8_fold;                                                     \
    case trie_utf8_exactfa_fold:                                                    \
        flags |= FOLD_FLAGS_NOMIX_ASCII;                                            \
        /* FALLTHROUGH */                                                           \
    case trie_utf8_fold:                                                            \
      do_trie_utf8_fold:                                                            \
        if ( foldlen>0 ) {                                                          \
            uvc = utf8n_to_uvchr( (const U8*) uscan, foldlen, &len, uniflags );     \
            foldlen -= len;                                                         \
            uscan += len;                                                           \
            len=0;                                                                  \
        } else {                                                                    \
            uvc = _toFOLD_utf8_flags( (const U8*) uc, uc_end, foldbuf, &foldlen,    \
                                                                            flags); \
            len = UTF8_SAFE_SKIP(uc, uc_end);                                       \
            skiplen = UVCHR_SKIP( uvc );                                            \
            foldlen -= skiplen;                                                     \
            uscan = foldbuf + skiplen;                                              \
        }                                                                           \
        break;                                                                      \
    case trie_flu8_latin:                                                           \
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;                                         \
        goto do_trie_latin_utf8_fold;                                               \
    case trie_latin_utf8_exactfa_fold:                                              \
        flags |= FOLD_FLAGS_NOMIX_ASCII;                                            \
        /* FALLTHROUGH */                                                           \
    case trie_latin_utf8_fold:                                                      \
      do_trie_latin_utf8_fold:                                                      \
        if ( foldlen>0 ) {                                                          \
            uvc = utf8n_to_uvchr( (const U8*) uscan, foldlen, &len, uniflags );     \
            foldlen -= len;                                                         \
            uscan += len;                                                           \
            len=0;                                                                  \
        } else {                                                                    \
            len = 1;                                                                \
            uvc = _to_fold_latin1( (U8) *uc, foldbuf, &foldlen, flags);             \
            skiplen = UVCHR_SKIP( uvc );                                            \
            foldlen -= skiplen;                                                     \
            uscan = foldbuf + skiplen;                                              \
        }                                                                           \
        break;                                                                      \
    case trie_utf8l:                                                                \
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;                                         \
        if (utf8_target && UTF8_IS_ABOVE_LATIN1(*uc)) {                             \
            _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(uc, uc_end);                     \
        }                                                                           \
        /* FALLTHROUGH */                                                           \
    case trie_utf8:                                                                 \
        uvc = utf8n_to_uvchr( (const U8*) uc, uc_end - uc, &len, uniflags );        \
        break;                                                                      \
    case trie_plain:                                                                \
        uvc = (UV)*uc;                                                              \
        len = 1;                                                                    \
    }                                                                               \
    if (uvc < 256) {                                                                \
        charid = trie->charmap[ uvc ];                                              \
    }                                                                               \
    else {                                                                          \
        charid = 0;                                                                 \
        if (widecharmap) {                                                          \
            SV** const svpp = hv_fetch(widecharmap,                                 \
                        (char*)&uvc, sizeof(UV), 0);                                \
            if (svpp)                                                               \
                charid = (U16)SvIV(*svpp);                                          \
        }                                                                           \
    }                                                                               \
} STMT_END

#define DUMP_EXEC_POS(li,s,doutf8,depth)                    \
    dump_exec_pos(li,s,(reginfo->strend),(reginfo->strbeg), \
                startpos, doutf8, depth)

#define GET_ANYOFH_INVLIST(prog, n)                                         \
                        GET_REGCLASS_AUX_DATA(prog, n, TRUE, 0, NULL, NULL)

#define REXEC_FBC_UTF8_SCAN(CODE)                           \
    STMT_START {                                            \
        while (s < strend) {                                \
            CODE                                            \
            s += UTF8_SAFE_SKIP(s, reginfo->strend);        \
        }                                                   \
    } STMT_END

#define REXEC_FBC_NON_UTF8_SCAN(CODE)                       \
    STMT_START {                                            \
        while (s < strend) {                                \
            CODE                                            \
            s++;                                            \
        }                                                   \
    } STMT_END

#define REXEC_FBC_UTF8_CLASS_SCAN(COND)                     \
    STMT_START {                                            \
        while (s < strend) {                                \
            REXEC_FBC_UTF8_CLASS_SCAN_GUTS(COND)            \
        }                                                   \
    } STMT_END

#define REXEC_FBC_NON_UTF8_CLASS_SCAN(COND)                 \
    STMT_START {                                            \
        while (s < strend) {                                \
            REXEC_FBC_NON_UTF8_CLASS_SCAN_GUTS(COND)        \
        }                                                   \
    } STMT_END

#define REXEC_FBC_UTF8_CLASS_SCAN_GUTS(COND)                   \
    if (COND) {                                                \
        FBC_CHECK_AND_TRY                                      \
        s += UTF8_SAFE_SKIP(s, reginfo->strend);               \
        previous_occurrence_end = s;                           \
    }                                                          \
    else {                                                     \
        s += UTF8SKIP(s);                                      \
    }

#define REXEC_FBC_NON_UTF8_CLASS_SCAN_GUTS(COND)               \
    if (COND) {                                                \
        FBC_CHECK_AND_TRY                                      \
        s++;                                                   \
        previous_occurrence_end = s;                           \
    }                                                          \
    else {                                                     \
        s++;                                                   \
    }

/* We keep track of where the next character should start after an occurrence
 * of the one we're looking for.  Knowing that, we can see right away if the
 * next occurrence is adjacent to the previous.  When 'doevery' is FALSE, we
 * don't accept the 2nd and succeeding adjacent occurrences */
#define FBC_CHECK_AND_TRY                                           \
        if (   (   doevery                                          \
                || s != previous_occurrence_end)                    \
            && (   reginfo->intuit                                  \
                || (s <= reginfo->strend && regtry(reginfo, &s))))  \
        {                                                           \
            goto got_it;                                            \
        }


/* These differ from the above macros in that they call a function which
 * returns the next occurrence of the thing being looked for in 's'; and
 * 'strend' if there is no such occurrence.  'f' is something like fcn(a,b,c)
 * */
#define REXEC_FBC_UTF8_FIND_NEXT_SCAN(f)                    \
    while (s < strend) {                                    \
        s = (char *) (f);                                   \
        if (s >= strend) {                                  \
            break;                                          \
        }                                                   \
                                                            \
        FBC_CHECK_AND_TRY                                   \
        s += UTF8SKIP(s);                                   \
        previous_occurrence_end = s;                        \
    }

#define REXEC_FBC_NON_UTF8_FIND_NEXT_SCAN(f)                \
    while (s < strend) {                                    \
        s = (char *) (f);                                   \
        if (s >= strend) {                                  \
            break;                                          \
        }                                                   \
                                                            \
        FBC_CHECK_AND_TRY                                   \
        s++;                                                \
        previous_occurrence_end = s;                        \
    }

/* This is like the above macro except the function returns NULL if there is no
 * occurrence, and there is a further condition that must be matched besides
 * the function */
#define REXEC_FBC_FIND_NEXT_UTF8_SCAN_COND(f, COND)         \
    while (s < strend) {                                    \
        s = (char *) (f);                                     \
        if (s == NULL) {                                    \
            s = (char *) strend;                            \
            break;                                          \
        }                                                   \
                                                            \
        if (COND) {                                         \
            FBC_CHECK_AND_TRY                               \
            s += UTF8_SAFE_SKIP(s, reginfo->strend);        \
            previous_occurrence_end = s;                    \
        }                                                   \
        else {                                              \
            s += UTF8SKIP(s);                               \
        }                                                   \
    }

/* This differs from the above macros in that it is passed a single byte that
 * is known to begin the next occurrence of the thing being looked for in 's'.
 * It does a memchr to find the next occurrence of 'byte', before trying 'COND'
 * at that position. */
#define REXEC_FBC_FIND_NEXT_UTF8_BYTE_SCAN(byte, COND)                  \
    REXEC_FBC_FIND_NEXT_UTF8_SCAN_COND(memchr(s, byte, strend - s),     \
                                              COND)

/* This is like the function above, but takes an entire string to look for
 * instead of a single byte */
#define REXEC_FBC_FIND_NEXT_UTF8_STRING_SCAN(substr, substr_end, COND)      \
    REXEC_FBC_FIND_NEXT_UTF8_SCAN_COND(                                     \
                                     ninstr(s, strend, substr, substr_end), \
                                     COND)

/* The four macros below are slightly different versions of the same logic.
 *
 * The first is for /a and /aa when the target string is UTF-8.  This can only
 * match ascii, but it must advance based on UTF-8.   The other three handle
 * the non-UTF-8 and the more generic UTF-8 cases.   In all four, we are
 * looking for the boundary (or non-boundary) between a word and non-word
 * character.  The utf8 and non-utf8 cases have the same logic, but the details
 * must be different.  Find the "wordness" of the character just prior to this
 * one, and compare it with the wordness of this one.  If they differ, we have
 * a boundary.  At the beginning of the string, pretend that the previous
 * character was a new-line.
 *
 * All these macros uncleanly have side-effects with each other and outside
 * variables.  So far it's been too much trouble to clean-up
 *
 * TEST_NON_UTF8 is the macro or function to call to test if its byte input is
 *               a word character or not.
 * IF_SUCCESS    is code to do if it finds that we are at a boundary between
 *               word/non-word
 * IF_FAIL       is code to do if we aren't at a boundary between word/non-word
 *
 * Exactly one of the two IF_FOO parameters is a no-op, depending on whether we
 * are looking for a boundary or for a non-boundary.  If we are looking for a
 * boundary, we want IF_FAIL to be the no-op, and for IF_SUCCESS to go out and
 * see if this tentative match actually works, and if so, to quit the loop
 * here.  And vice-versa if we are looking for a non-boundary.
 *
 * 'tmp' below in the next four macros in the REXEC_FBC_UTF8_SCAN and
 * REXEC_FBC_UTF8_SCAN loops is a loop invariant, a bool giving the return of
 * TEST_NON_UTF8(s-1).  To see this, note that that's what it is defined to be
 * at entry to the loop, and to get to the IF_FAIL branch, tmp must equal
 * TEST_NON_UTF8(s), and in the opposite branch, IF_SUCCESS, tmp is that
 * complement.  But in that branch we complement tmp, meaning that at the
 * bottom of the loop tmp is always going to be equal to TEST_NON_UTF8(s),
 * which means at the top of the loop in the next iteration, it is
 * TEST_NON_UTF8(s-1) */
#define FBC_UTF8_A(TEST_NON_UTF8, IF_SUCCESS, IF_FAIL)                         \
    tmp = (s != reginfo->strbeg) ? UCHARAT(s - 1) : '\n';                      \
    tmp = TEST_NON_UTF8(tmp);                                                  \
    REXEC_FBC_UTF8_SCAN( /* advances s while s < strend */                     \
        if (tmp == ! TEST_NON_UTF8((U8) *s)) {                                 \
            tmp = !tmp;                                                        \
            IF_SUCCESS; /* Is a boundary if values for s-1 and s differ */     \
        }                                                                      \
        else {                                                                 \
            IF_FAIL;                                                           \
        }                                                                      \
    );                                                                         \

/* Like FBC_UTF8_A, but TEST_UV is a macro which takes a UV as its input, and
 * TEST_UTF8 is a macro that for the same input code points returns identically
 * to TEST_UV, but takes a pointer to a UTF-8 encoded string instead (and an
 * end pointer as well) */
#define FBC_UTF8(TEST_UV, TEST_UTF8, IF_SUCCESS, IF_FAIL)                      \
    if (s == reginfo->strbeg) {                                                \
        tmp = '\n';                                                            \
    }                                                                          \
    else { /* Back-up to the start of the previous character */                \
        U8 * const r = reghop3((U8*)s, -1, (U8*)reginfo->strbeg);              \
        tmp = utf8n_to_uvchr(r, (U8*) reginfo->strend - r,                     \
                                                       0, UTF8_ALLOW_DEFAULT); \
    }                                                                          \
    tmp = TEST_UV(tmp);                                                        \
    REXEC_FBC_UTF8_SCAN(/* advances s while s < strend */                      \
        if (tmp == ! (TEST_UTF8((U8 *) s, (U8 *) reginfo->strend))) {          \
            tmp = !tmp;                                                        \
            IF_SUCCESS;                                                        \
        }                                                                      \
        else {                                                                 \
            IF_FAIL;                                                           \
        }                                                                      \
    );

/* Like the above two macros, for a UTF-8 target string.  UTF8_CODE is the
 * complete code for handling UTF-8.  Common to the BOUND and NBOUND cases,
 * set-up by the FBC_BOUND, etc macros below */
#define FBC_BOUND_COMMON_UTF8(UTF8_CODE, TEST_NON_UTF8, IF_SUCCESS, IF_FAIL)   \
    UTF8_CODE;                                                                 \
    /* Here, things have been set up by the previous code so that tmp is the   \
     * return of TEST_NON_UTF8(s-1).  We also have to check if this matches    \
     * against the EOS, which we treat as a \n */                              \
    if (tmp == ! TEST_NON_UTF8('\n')) {                                        \
        IF_SUCCESS;                                                            \
    }                                                                          \
    else {                                                                     \
        IF_FAIL;                                                               \
    }

/* Same as the macro above, but the target isn't UTF-8 */
#define FBC_BOUND_COMMON_NON_UTF8(TEST_NON_UTF8, IF_SUCCESS, IF_FAIL)       \
    tmp = (s != reginfo->strbeg) ? UCHARAT(s - 1) : '\n';                   \
    tmp = TEST_NON_UTF8(tmp);                                               \
    REXEC_FBC_NON_UTF8_SCAN(/* advances s while s < strend */               \
        if (tmp == ! TEST_NON_UTF8(UCHARAT(s))) {                           \
            IF_SUCCESS;                                                     \
            tmp = !tmp;                                                     \
        }                                                                   \
        else {                                                              \
            IF_FAIL;                                                        \
        }                                                                   \
    );                                                                      \
    /* Here, things have been set up by the previous code so that tmp is    \
     * the return of TEST_NON_UTF8(s-1).   We also have to check if this    \
     * matches against the EOS, which we treat as a \n */                   \
    if (tmp == ! TEST_NON_UTF8('\n')) {                                     \
        IF_SUCCESS;                                                         \
    }                                                                       \
    else {                                                                  \
        IF_FAIL;                                                            \
    }

/* This is the macro to use when we want to see if something that looks like it
 * could match, actually does, and if so exits the loop.  It needs to be used
 * only for bounds checking macros, as it allows for matching beyond the end of
 * string (which should be zero length without having to look at the string
 * contents) */
#define REXEC_FBC_TRYIT                                                     \
    if (reginfo->intuit || (s <= reginfo->strend && regtry(reginfo, &s)))   \
        goto got_it

/* The only difference between the BOUND and NBOUND cases is that
 * REXEC_FBC_TRYIT is called when matched in BOUND, and when non-matched in
 * NBOUND.  This is accomplished by passing it as either the if or else clause,
 * with the other one being empty (PLACEHOLDER is defined as empty).
 *
 * The TEST_FOO parameters are for operating on different forms of input, but
 * all should be ones that return identically for the same underlying code
 * points */

#define FBC_BOUND_UTF8(TEST_NON_UTF8, TEST_UV, TEST_UTF8)                   \
    FBC_BOUND_COMMON_UTF8(                                                  \
          FBC_UTF8(TEST_UV, TEST_UTF8, REXEC_FBC_TRYIT, PLACEHOLDER),       \
          TEST_NON_UTF8, REXEC_FBC_TRYIT, PLACEHOLDER)

#define FBC_BOUND_NON_UTF8(TEST_NON_UTF8)                                   \
    FBC_BOUND_COMMON_NON_UTF8(TEST_NON_UTF8, REXEC_FBC_TRYIT, PLACEHOLDER)

#define FBC_BOUND_A_UTF8(TEST_NON_UTF8)                                     \
    FBC_BOUND_COMMON_UTF8(                                                  \
                    FBC_UTF8_A(TEST_NON_UTF8, REXEC_FBC_TRYIT, PLACEHOLDER),\
                    TEST_NON_UTF8, REXEC_FBC_TRYIT, PLACEHOLDER)

#define FBC_BOUND_A_NON_UTF8(TEST_NON_UTF8)                                 \
    FBC_BOUND_COMMON_NON_UTF8(TEST_NON_UTF8, REXEC_FBC_TRYIT, PLACEHOLDER)

#define FBC_NBOUND_UTF8(TEST_NON_UTF8, TEST_UV, TEST_UTF8)                  \
    FBC_BOUND_COMMON_UTF8(                                                  \
              FBC_UTF8(TEST_UV, TEST_UTF8, PLACEHOLDER, REXEC_FBC_TRYIT),   \
              TEST_NON_UTF8, PLACEHOLDER, REXEC_FBC_TRYIT)

#define FBC_NBOUND_NON_UTF8(TEST_NON_UTF8)                                  \
    FBC_BOUND_COMMON_NON_UTF8(TEST_NON_UTF8, PLACEHOLDER, REXEC_FBC_TRYIT)

#define FBC_NBOUND_A_UTF8(TEST_NON_UTF8)                                    \
    FBC_BOUND_COMMON_UTF8(                                                  \
            FBC_UTF8_A(TEST_NON_UTF8, PLACEHOLDER, REXEC_FBC_TRYIT),        \
            TEST_NON_UTF8, PLACEHOLDER, REXEC_FBC_TRYIT)

#define FBC_NBOUND_A_NON_UTF8(TEST_NON_UTF8)                                \
    FBC_BOUND_COMMON_NON_UTF8(TEST_NON_UTF8, PLACEHOLDER, REXEC_FBC_TRYIT)

#ifdef DEBUGGING
static IV
S_get_break_val_cp_checked(SV* const invlist, const UV cp_in) {
  IV cp_out = _invlist_search(invlist, cp_in);
  assert(cp_out >= 0);
  return cp_out;
}
#  define _generic_GET_BREAK_VAL_CP_CHECKED(invlist, invmap, cp) \
        invmap[S_get_break_val_cp_checked(invlist, cp)]
#else
#  define _generic_GET_BREAK_VAL_CP_CHECKED(invlist, invmap, cp) \
        invmap[_invlist_search(invlist, cp)]
#endif

/* Takes a pointer to an inversion list, a pointer to its corresponding
 * inversion map, and a code point, and returns the code point's value
 * according to the two arrays.  It assumes that all code points have a value.
 * This is used as the base macro for macros for particular properties */
#define _generic_GET_BREAK_VAL_CP(invlist, invmap, cp)              \
        _generic_GET_BREAK_VAL_CP_CHECKED(invlist, invmap, cp)

/* Same as above, but takes begin, end ptrs to a UTF-8 encoded string instead
 * of a code point, returning the value for the first code point in the string.
 * And it takes the particular macro name that finds the desired value given a
 * code point.  Merely convert the UTF-8 to code point and call the cp macro */
#define _generic_GET_BREAK_VAL_UTF8(cp_macro, pos, strend)                     \
             (__ASSERT_(pos < strend)                                          \
                 /* Note assumes is valid UTF-8 */                             \
             (cp_macro(utf8_to_uvchr_buf((pos), (strend), NULL))))

/* Returns the GCB value for the input code point */
#define getGCB_VAL_CP(cp)                                                      \
          _generic_GET_BREAK_VAL_CP(                                           \
                                    PL_GCB_invlist,                            \
                                    _Perl_GCB_invmap,                          \
                                    (cp))

/* Returns the GCB value for the first code point in the UTF-8 encoded string
 * bounded by pos and strend */
#define getGCB_VAL_UTF8(pos, strend)                                           \
    _generic_GET_BREAK_VAL_UTF8(getGCB_VAL_CP, pos, strend)

/* Returns the LB value for the input code point */
#define getLB_VAL_CP(cp)                                                       \
          _generic_GET_BREAK_VAL_CP(                                           \
                                    PL_LB_invlist,                             \
                                    _Perl_LB_invmap,                           \
                                    (cp))

/* Returns the LB value for the first code point in the UTF-8 encoded string
 * bounded by pos and strend */
#define getLB_VAL_UTF8(pos, strend)                                            \
    _generic_GET_BREAK_VAL_UTF8(getLB_VAL_CP, pos, strend)


/* Returns the SB value for the input code point */
#define getSB_VAL_CP(cp)                                                       \
          _generic_GET_BREAK_VAL_CP(                                           \
                                    PL_SB_invlist,                             \
                                    _Perl_SB_invmap,                     \
                                    (cp))

/* Returns the SB value for the first code point in the UTF-8 encoded string
 * bounded by pos and strend */
#define getSB_VAL_UTF8(pos, strend)                                            \
    _generic_GET_BREAK_VAL_UTF8(getSB_VAL_CP, pos, strend)

/* Returns the WB value for the input code point */
#define getWB_VAL_CP(cp)                                                       \
          _generic_GET_BREAK_VAL_CP(                                           \
                                    PL_WB_invlist,                             \
                                    _Perl_WB_invmap,                         \
                                    (cp))

/* Returns the WB value for the first code point in the UTF-8 encoded string
 * bounded by pos and strend */
#define getWB_VAL_UTF8(pos, strend)                                            \
    _generic_GET_BREAK_VAL_UTF8(getWB_VAL_CP, pos, strend)

/* We know what class REx starts with.  Try to find this position... */
/* if reginfo->intuit, its a dryrun */
/* annoyingly all the vars in this routine have different names from their counterparts
   in regmatch. /grrr */
STATIC char *
S_find_byclass(pTHX_ regexp * prog, const regnode *c, char *s,
    const char *strend, regmatch_info *reginfo)
{

    /* TRUE if x+ need not match at just the 1st pos of run of x's */
    const I32 doevery = (prog->intflags & PREGf_SKIP) == 0;

    char *pat_string;   /* The pattern's exactish string */
    char *pat_end;	    /* ptr to end char of pat_string */
    re_fold_t folder;	/* Function for computing non-utf8 folds */
    const U8 *fold_array;   /* array for folding ords < 256 */
    STRLEN ln;
    STRLEN lnc;
    U8 c1;
    U8 c2;
    char *e = NULL;

    /* In some cases we accept only the first occurrence of 'x' in a sequence of
     * them.  This variable points to just beyond the end of the previous
     * occurrence of 'x', hence we can tell if we are in a sequence.  (Having
     * it point to beyond the 'x' allows us to work for UTF-8 without having to
     * hop back.) */
    char * previous_occurrence_end = 0;

    I32 tmp;            /* Scratch variable */
    const bool utf8_target = reginfo->is_utf8_target;
    UV utf8_fold_flags = 0;
    const bool is_utf8_pat = reginfo->is_utf8_pat;
    bool to_complement = FALSE; /* Invert the result?  Taking the xor of this
                                   with a result inverts that result, as 0^1 =
                                   1 and 1^1 = 0 */
    char_class_number_ classnum;

    RXi_GET_DECL(prog,progi);

    PERL_ARGS_ASSERT_FIND_BYCLASS;

    /* We know what class it must start with. The case statements below have
     * encoded the OP, and the UTF8ness of the target ('t8' for is UTF-8; 'tb'
     * for it isn't; 'b' stands for byte), and the UTF8ness of the pattern
     * ('p8' and 'pb'. */
    switch (with_tp_UTF8ness(OP(c), utf8_target, is_utf8_pat)) {
        SV * anyofh_list;

      case ANYOFPOSIXL_t8_pb:
      case ANYOFPOSIXL_t8_p8:
      case ANYOFL_t8_pb:
      case ANYOFL_t8_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_SETS(c);

        /* FALLTHROUGH */

      case ANYOFD_t8_pb:
      case ANYOFD_t8_p8:
      case ANYOF_t8_pb:
      case ANYOF_t8_p8:
        REXEC_FBC_UTF8_CLASS_SCAN(
                reginclass(prog, c, (U8*)s, (U8*) strend, 1 /* is utf8 */));
        break;

      case ANYOFPOSIXL_tb_pb:
      case ANYOFPOSIXL_tb_p8:
      case ANYOFL_tb_pb:
      case ANYOFL_tb_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_SETS(c);

        /* FALLTHROUGH */

      case ANYOFD_tb_pb:
      case ANYOFD_tb_p8:
      case ANYOF_tb_pb:
      case ANYOF_tb_p8:
        if (! ANYOF_FLAGS(c) && ANYOF_MATCHES_NONE_OUTSIDE_BITMAP(c)) {
            /* We know that s is in the bitmap range since the target isn't
             * UTF-8, so what happens for out-of-range values is not relevant,
             * so exclude that from the flags */
            REXEC_FBC_NON_UTF8_CLASS_SCAN(ANYOF_BITMAP_TEST(c, *((U8*)s)));
        }
        else {
            REXEC_FBC_NON_UTF8_CLASS_SCAN(reginclass(prog,c, (U8*)s, (U8*)s+1,
                                                     0));
        }
        break;

      case ANYOFM_tb_pb: /* ARG1u() is the base byte; FLAGS() the mask byte */
      case ANYOFM_tb_p8:
        REXEC_FBC_NON_UTF8_FIND_NEXT_SCAN(
             find_next_masked((U8 *) s, (U8 *) strend, (U8) ARG1u(c), FLAGS(c)));
        break;

      case ANYOFM_t8_pb:
      case ANYOFM_t8_p8:
        /* UTF-8ness doesn't matter because only matches UTF-8 invariants.  But
         * we do anyway for performance reasons, as otherwise we would have to
         * examine all the continuation characters */
        REXEC_FBC_UTF8_FIND_NEXT_SCAN(
             find_next_masked((U8 *) s, (U8 *) strend, (U8) ARG1u(c), FLAGS(c)));
        break;

      case NANYOFM_tb_pb:
      case NANYOFM_tb_p8:
        REXEC_FBC_NON_UTF8_FIND_NEXT_SCAN(
           find_span_end_mask((U8 *) s, (U8 *) strend, (U8) ARG1u(c), FLAGS(c)));
        break;

      case NANYOFM_t8_pb:
      case NANYOFM_t8_p8: /* UTF-8ness does matter because can match UTF-8
                                  variants. */
        REXEC_FBC_UTF8_FIND_NEXT_SCAN(
                        (char *) find_span_end_mask((U8 *) s, (U8 *) strend,
                                                    (U8) ARG1u(c), FLAGS(c)));
        break;

      /* These nodes all require at least one code point to be in UTF-8 to
       * match */
      case ANYOFH_tb_pb:
      case ANYOFH_tb_p8:
      case ANYOFHb_tb_pb:
      case ANYOFHb_tb_p8:
      case ANYOFHbbm_tb_pb:
      case ANYOFHbbm_tb_p8:
      case ANYOFHr_tb_pb:
      case ANYOFHr_tb_p8:
      case ANYOFHs_tb_pb:
      case ANYOFHs_tb_p8:
      case EXACTFLU8_tb_pb:
      case EXACTFLU8_tb_p8:
      case EXACTFU_REQ8_tb_pb:
      case EXACTFU_REQ8_tb_p8:
        break;

      case ANYOFH_t8_pb:
      case ANYOFH_t8_p8:
        anyofh_list = GET_ANYOFH_INVLIST(prog, c);
        REXEC_FBC_UTF8_CLASS_SCAN(
              (   (U8) NATIVE_UTF8_TO_I8(*s) >= ANYOF_FLAGS(c)
               && _invlist_contains_cp(anyofh_list,
                                       utf8_to_uvchr_buf((U8 *) s,
                                                         (U8 *) strend,
                                                         NULL))));
        break;

      case ANYOFHb_t8_pb:
      case ANYOFHb_t8_p8:
        {
            /* We know what the first byte of any matched string should be. */
            U8 first_byte = FLAGS(c);

            anyofh_list = GET_ANYOFH_INVLIST(prog, c);
            REXEC_FBC_FIND_NEXT_UTF8_BYTE_SCAN(first_byte,
                   _invlist_contains_cp(anyofh_list,
                                           utf8_to_uvchr_buf((U8 *) s,
                                                              (U8 *) strend,
                                                              NULL)));
        }
        break;

      case ANYOFHbbm_t8_pb:
      case ANYOFHbbm_t8_p8:
        {
            /* We know what the first byte of any matched string should be. */
            U8 first_byte = FLAGS(c);

            /* And a bitmap defines all the legal 2nd byte matches */
            REXEC_FBC_FIND_NEXT_UTF8_BYTE_SCAN(first_byte,
                               (    s < strend
                                && BITMAP_TEST(((struct regnode_bbm *) c)->bitmap,
                                            (U8) s[1] & UTF_CONTINUATION_MASK)));
        }
        break;

      case ANYOFHr_t8_pb:
      case ANYOFHr_t8_p8:
        anyofh_list = GET_ANYOFH_INVLIST(prog, c);
        REXEC_FBC_UTF8_CLASS_SCAN(
                    (   inRANGE(NATIVE_UTF8_TO_I8(*s),
                                LOWEST_ANYOF_HRx_BYTE(ANYOF_FLAGS(c)),
                                HIGHEST_ANYOF_HRx_BYTE(ANYOF_FLAGS(c)))
                   && _invlist_contains_cp(anyofh_list,
                                           utf8_to_uvchr_buf((U8 *) s,
                                                              (U8 *) strend,
                                                              NULL))));
        break;

      case ANYOFHs_t8_pb:
      case ANYOFHs_t8_p8:
        anyofh_list = GET_ANYOFH_INVLIST(prog, c);
        REXEC_FBC_FIND_NEXT_UTF8_STRING_SCAN(
                        ((struct regnode_anyofhs *) c)->string,
                        /* Note FLAGS is the string length in this regnode */
                        ((struct regnode_anyofhs *) c)->string + FLAGS(c),
                        _invlist_contains_cp(anyofh_list,
                                             utf8_to_uvchr_buf((U8 *) s,
                                                               (U8 *) strend,
                                                               NULL)));
        break;

      case ANYOFR_tb_pb:
      case ANYOFR_tb_p8:
        REXEC_FBC_NON_UTF8_CLASS_SCAN(withinCOUNT((U8) *s,
                                            ANYOFRbase(c), ANYOFRdelta(c)));
        break;

      case ANYOFR_t8_pb:
      case ANYOFR_t8_p8:
        REXEC_FBC_UTF8_CLASS_SCAN(
                            (   NATIVE_UTF8_TO_I8(*s) >= ANYOF_FLAGS(c)
                             && withinCOUNT(utf8_to_uvchr_buf((U8 *) s,
                                                              (U8 *) strend,
                                                              NULL),
                                            ANYOFRbase(c), ANYOFRdelta(c))));
        break;

      case ANYOFRb_tb_pb:
      case ANYOFRb_tb_p8:
        REXEC_FBC_NON_UTF8_CLASS_SCAN(withinCOUNT((U8) *s,
                                            ANYOFRbase(c), ANYOFRdelta(c)));
        break;

      case ANYOFRb_t8_pb:
      case ANYOFRb_t8_p8:
        {   /* We know what the first byte of any matched string should be */
            U8 first_byte = FLAGS(c);

            REXEC_FBC_FIND_NEXT_UTF8_BYTE_SCAN(first_byte,
                                withinCOUNT(utf8_to_uvchr_buf((U8 *) s,
                                                              (U8 *) strend,
                                                              NULL),
                                            ANYOFRbase(c), ANYOFRdelta(c)));
        }
        break;

      case EXACTFAA_tb_pb:

        /* Latin1 folds are not affected by /a, except it excludes the sharp s,
         * which these functions don't handle anyway */
        fold_array = PL_fold_latin1;
        folder = S_foldEQ_latin1_s2_folded;
        goto do_exactf_non_utf8;

      case EXACTF_tb_pb:
        fold_array = PL_fold;
        folder = Perl_foldEQ;
        goto do_exactf_non_utf8;

      case EXACTFL_tb_pb:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;

        if (IN_UTF8_CTYPE_LOCALE) {
            utf8_fold_flags = FOLDEQ_LOCALE;
            goto do_exactf_utf8;
        }

        fold_array = PL_fold_locale;
        folder = Perl_foldEQ_locale;
        goto do_exactf_non_utf8;

      case EXACTFU_tb_pb:
        /* Any 'ss' in the pattern should have been replaced by regcomp, so we
         * don't have to worry here about this single special case in the
         * Latin1 range */
        fold_array = PL_fold_latin1;
        folder = S_foldEQ_latin1_s2_folded;

        /* FALLTHROUGH */

       do_exactf_non_utf8: /* Neither pattern nor string are UTF8, and there
                              are no glitches with fold-length differences
                              between the target string and pattern */

        /* The idea in the non-utf8 EXACTF* cases is to first find the first
         * character of the EXACTF* node and then, if necessary,
         * case-insensitively compare the full text of the node.  c1 is the
         * first character.  c2 is its fold.  This logic will not work for
         * Unicode semantics and the german sharp ss, which hence should not be
         * compiled into a node that gets here. */
        pat_string = STRINGs(c);
        ln  = STR_LENs(c);	/* length to match in octets/bytes */

        /* We know that we have to match at least 'ln' bytes (which is the same
         * as characters, since not utf8).  If we have to match 3 characters,
         * and there are only 2 available, we know without trying that it will
         * fail; so don't start a match past the required minimum number from
         * the far end */
        e = HOP3c(strend, -((SSize_t)ln), s);
        if (e < s)
            break;

        c1 = *pat_string;
        c2 = fold_array[c1];
        if (c1 == c2) { /* If char and fold are the same */
            while (s <= e) {
                s = (char *) memchr(s, c1, e + 1 - s);
                if (s == NULL) {
                    break;
                }

                /* Check that the rest of the node matches */
                if (   (ln == 1 || folder(aTHX_ s + 1, pat_string + 1, ln - 1))
                    && (reginfo->intuit || regtry(reginfo, &s)) )
                {
                    goto got_it;
                }
                s++;
            }
        }
        else {
            U8 bits_differing = c1 ^ c2;

            /* If the folds differ in one bit position only, we can mask to
             * match either of them, and can use this faster find method.  Both
             * ASCII and EBCDIC tend to have their case folds differ in only
             * one position, so this is very likely */
            if (LIKELY(PL_bitcount[bits_differing] == 1)) {
                bits_differing = ~ bits_differing;
                while (s <= e) {
                    s = (char *) find_next_masked((U8 *) s, (U8 *) e + 1,
                                        (c1 & bits_differing), bits_differing);
                    if (s > e) {
                        break;
                    }

                    if (   (ln == 1 || folder(aTHX_ s + 1, pat_string + 1, ln - 1))
                        && (reginfo->intuit || regtry(reginfo, &s)) )
                    {
                        goto got_it;
                    }
                    s++;
                }
            }
            else {  /* Otherwise, stuck with looking byte-at-a-time.  This
                       should actually happen only in EXACTFL nodes */
                while (s <= e) {
                    if (    (*(U8*)s == c1 || *(U8*)s == c2)
                        && (ln == 1 || folder(aTHX_ s + 1, pat_string + 1, ln - 1))
                        && (reginfo->intuit || regtry(reginfo, &s)) )
                    {
                        goto got_it;
                    }
                    s++;
                }
            }
        }
        break;

      case EXACTFAA_tb_p8:
      case EXACTFAA_t8_p8:
        utf8_fold_flags = FOLDEQ_UTF8_NOMIX_ASCII
                         |FOLDEQ_S2_ALREADY_FOLDED
                         |FOLDEQ_S2_FOLDS_SANE;
        goto do_exactf_utf8;

      case EXACTFAA_NO_TRIE_tb_pb:
      case EXACTFAA_NO_TRIE_t8_pb:
      case EXACTFAA_t8_pb:

        /* Here, and elsewhere in this file, the reason we can't consider a
         * non-UTF-8 pattern already folded in the presence of a UTF-8 target
         * is because any MICRO SIGN in the pattern won't be folded.  Since the
         * fold of the MICRO SIGN requires UTF-8 to represent, we can consider
         * a non-UTF-8 pattern folded when matching a non-UTF-8 target */
        utf8_fold_flags = FOLDEQ_UTF8_NOMIX_ASCII;
        goto do_exactf_utf8;

      case EXACTFL_tb_p8:
      case EXACTFL_t8_pb:
      case EXACTFL_t8_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        utf8_fold_flags = FOLDEQ_LOCALE;
        goto do_exactf_utf8;

      case EXACTFLU8_t8_pb:
      case EXACTFLU8_t8_p8:
        utf8_fold_flags =  FOLDEQ_LOCALE | FOLDEQ_S2_ALREADY_FOLDED
                                         | FOLDEQ_S2_FOLDS_SANE;
        goto do_exactf_utf8;

      case EXACTFU_REQ8_t8_p8:
        utf8_fold_flags = FOLDEQ_S2_ALREADY_FOLDED;
        goto do_exactf_utf8;

      case EXACTFU_tb_p8:
      case EXACTFU_t8_pb:
      case EXACTFU_t8_p8:
        utf8_fold_flags = FOLDEQ_S2_ALREADY_FOLDED;
        goto do_exactf_utf8;

      /* The following are problematic even though pattern isn't UTF-8.  Use
       * full functionality normally not done except for UTF-8. */
      case EXACTF_t8_pb:
      case EXACTFUP_tb_pb:
      case EXACTFUP_t8_pb:

       do_exactf_utf8:
        {
            unsigned expansion;

            /* If one of the operands is in utf8, we can't use the simpler
             * folding above, due to the fact that many different characters
             * can have the same fold, or portion of a fold, or different-
             * length fold */
            pat_string = STRINGs(c);
            ln  = STR_LENs(c);	/* length to match in octets/bytes */
            pat_end = pat_string + ln;
            lnc = is_utf8_pat       /* length to match in characters */
                  ? utf8_length((U8 *) pat_string, (U8 *) pat_end)
                  : ln;

            /* We have 'lnc' characters to match in the pattern, but because of
             * multi-character folding, each character in the target can match
             * up to 3 characters (Unicode guarantees it will never exceed
             * this) if it is utf8-encoded; and up to 2 if not (based on the
             * fact that the Latin 1 folds are already determined, and the only
             * multi-char fold in that range is the sharp-s folding to 'ss'.
             * Thus, a pattern character can match as little as 1/3 of a string
             * character.  Adjust lnc accordingly, rounding up, so that if we
             * need to match at least 4+1/3 chars, that really is 5. */
            expansion = (utf8_target) ? UTF8_MAX_FOLD_CHAR_EXPAND : 2;
            lnc = (lnc + expansion - 1) / expansion;

            /* As in the non-UTF8 case, if we have to match 3 characters, and
             * only 2 are left, it's guaranteed to fail, so don't start a match
             * that would require us to go beyond the end of the string */
            e = HOP3c(strend, -((SSize_t)lnc), s);

            /* XXX Note that we could recalculate e to stop the loop earlier,
             * as the worst case expansion above will rarely be met, and as we
             * go along we would usually find that e moves further to the left.
             * This would happen only after we reached the point in the loop
             * where if there were no expansion we should fail.  Unclear if
             * worth the expense */

            while (s <= e) {
                char *my_strend= (char *)strend;
                if (   foldEQ_utf8_flags(s, &my_strend, 0,  utf8_target,
                                         pat_string, NULL, ln, is_utf8_pat,
                                         utf8_fold_flags)
                    && (reginfo->intuit || regtry(reginfo, &s)) )
                {
                    goto got_it;
                }
                s += (utf8_target) ? UTF8_SAFE_SKIP(s, reginfo->strend) : 1;
            }
        }
        break;

      case BOUNDA_tb_pb:
      case BOUNDA_tb_p8:
      case BOUND_tb_pb:  /* /d without utf8 target is /a */
      case BOUND_tb_p8:
        /* regcomp.c makes sure that these only have the traditional \b
         * meaning. */
        assert(FLAGS(c) == TRADITIONAL_BOUND);

        FBC_BOUND_A_NON_UTF8(isWORDCHAR_A);
        break;

      case BOUNDA_t8_pb: /* What /a matches is same under UTF-8 */
      case BOUNDA_t8_p8:
        /* regcomp.c makes sure that these only have the traditional \b
         * meaning. */
        assert(FLAGS(c) == TRADITIONAL_BOUND);

        FBC_BOUND_A_UTF8(isWORDCHAR_A);
        break;

      case NBOUNDA_tb_pb:
      case NBOUNDA_tb_p8:
      case NBOUND_tb_pb: /* /d without utf8 target is /a */
      case NBOUND_tb_p8:
        /* regcomp.c makes sure that these only have the traditional \b
         * meaning. */
        assert(FLAGS(c) == TRADITIONAL_BOUND);

        FBC_NBOUND_A_NON_UTF8(isWORDCHAR_A);
        break;

      case NBOUNDA_t8_pb: /* What /a matches is same under UTF-8 */
      case NBOUNDA_t8_p8:
        /* regcomp.c makes sure that these only have the traditional \b
         * meaning. */
        assert(FLAGS(c) == TRADITIONAL_BOUND);

        FBC_NBOUND_A_UTF8(isWORDCHAR_A);
        break;

      case NBOUNDU_tb_pb:
      case NBOUNDU_tb_p8:
        if ((bound_type) FLAGS(c) == TRADITIONAL_BOUND) {
            FBC_NBOUND_NON_UTF8(isWORDCHAR_L1);
            break;
        }

        to_complement = 1;
        goto do_boundu_non_utf8;

      case NBOUNDL_tb_pb:
      case NBOUNDL_tb_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        if (FLAGS(c) == TRADITIONAL_BOUND) {
            FBC_NBOUND_NON_UTF8(isWORDCHAR_LC);
            break;
        }

        CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_BOUND;

        to_complement = 1;
        goto do_boundu_non_utf8;

      case BOUNDL_tb_pb:
      case BOUNDL_tb_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        if (FLAGS(c) == TRADITIONAL_BOUND) {
            FBC_BOUND_NON_UTF8(isWORDCHAR_LC);
            break;
        }

        CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_BOUND;

        goto do_boundu_non_utf8;

      case BOUNDU_tb_pb:
      case BOUNDU_tb_p8:
        if ((bound_type) FLAGS(c) == TRADITIONAL_BOUND) {
            FBC_BOUND_NON_UTF8(isWORDCHAR_L1);
            break;
        }

      do_boundu_non_utf8:
        if (s == reginfo->strbeg) {
            if (reginfo->intuit || regtry(reginfo, &s))
            {
                goto got_it;
            }

            /* Didn't match.  Try at the next position (if there is one) */
            s++;
            if (UNLIKELY(s >= reginfo->strend)) {
                break;
            }
        }

        switch((bound_type) FLAGS(c)) {
          case TRADITIONAL_BOUND: /* Should have already been handled */
            assert(0);
            break;

          case GCB_BOUND:
            /* Not utf8.  Everything is a GCB except between CR and LF */
            while (s < strend) {
                if ((to_complement ^ (   UCHARAT(s - 1) != '\r'
                                      || UCHARAT(s) != '\n'))
                    && (reginfo->intuit || regtry(reginfo, &s)))
                {
                    goto got_it;
                }
                s++;
            }

            break;

          case LB_BOUND:
            {
                LB_enum before = getLB_VAL_CP((U8) *(s -1));
                while (s < strend) {
                    LB_enum after = getLB_VAL_CP((U8) *s);
                    if (to_complement ^ isLB(before,
                                             after,
                                             (U8*) reginfo->strbeg,
                                             (U8*) s,
                                             (U8*) reginfo->strend,
                                             0 /* target not utf8 */ )
                        && (reginfo->intuit || regtry(reginfo, &s)))
                    {
                        goto got_it;
                    }
                    before = after;
                    s++;
                }
            }

            break;

          case SB_BOUND:
            {
                SB_enum before = getSB_VAL_CP((U8) *(s -1));
                while (s < strend) {
                    SB_enum after = getSB_VAL_CP((U8) *s);
                    if ((to_complement ^ isSB(before,
                                              after,
                                              (U8*) reginfo->strbeg,
                                              (U8*) s,
                                              (U8*) reginfo->strend,
                                             0 /* target not utf8 */ ))
                        && (reginfo->intuit || regtry(reginfo, &s)))
                    {
                        goto got_it;
                    }
                    before = after;
                    s++;
                }
            }

            break;

          case WB_BOUND:
            {
                WB_enum previous = WB_UNKNOWN;
                WB_enum before = getWB_VAL_CP((U8) *(s -1));
                while (s < strend) {
                    WB_enum after = getWB_VAL_CP((U8) *s);
                    if ((to_complement ^ isWB(previous,
                                              before,
                                              after,
                                              (U8*) reginfo->strbeg,
                                              (U8*) s,
                                              (U8*) reginfo->strend,
                                               0 /* target not utf8 */ ))
                        && (reginfo->intuit || regtry(reginfo, &s)))
                    {
                        goto got_it;
                    }
                    previous = before;
                    before = after;
                    s++;
                }
            }
        }

        /* Here are at the final position in the target string, which is a
         * boundary by definition, so matches, depending on other constraints.
         * */
        if (   reginfo->intuit
            || (s <= reginfo->strend && regtry(reginfo, &s)))
        {
            goto got_it;
        }

        break;

      case BOUNDL_t8_pb:
      case BOUNDL_t8_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        if (FLAGS(c) == TRADITIONAL_BOUND) {
            FBC_BOUND_UTF8(isWORDCHAR_LC, isWORDCHAR_LC_uvchr,
                           isWORDCHAR_LC_utf8_safe);
            break;
        }

        CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_BOUND;

        to_complement = 1;
        goto do_boundu_utf8;

      case NBOUNDL_t8_pb:
      case NBOUNDL_t8_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        if (FLAGS(c) == TRADITIONAL_BOUND) {
            FBC_NBOUND_UTF8(isWORDCHAR_LC, isWORDCHAR_LC_uvchr,
                            isWORDCHAR_LC_utf8_safe);
            break;
        }

        CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_BOUND;

        to_complement = 1;
        goto do_boundu_utf8;

      case NBOUND_t8_pb:
      case NBOUND_t8_p8:
        /* regcomp.c makes sure that these only have the traditional \b
         * meaning. */
        assert(FLAGS(c) == TRADITIONAL_BOUND);

        /* FALLTHROUGH */

      case NBOUNDU_t8_pb:
      case NBOUNDU_t8_p8:
        if ((bound_type) FLAGS(c) == TRADITIONAL_BOUND) {
            FBC_NBOUND_UTF8(isWORDCHAR_L1, isWORDCHAR_uni,
                            isWORDCHAR_utf8_safe);
            break;
        }

        to_complement = 1;
        goto do_boundu_utf8;

      case BOUND_t8_pb:
      case BOUND_t8_p8:
        /* regcomp.c makes sure that these only have the traditional \b
         * meaning. */
        assert(FLAGS(c) == TRADITIONAL_BOUND);

        /* FALLTHROUGH */

      case BOUNDU_t8_pb:
      case BOUNDU_t8_p8:
        if ((bound_type) FLAGS(c) == TRADITIONAL_BOUND) {
            FBC_BOUND_UTF8(isWORDCHAR_L1, isWORDCHAR_uni, isWORDCHAR_utf8_safe);
            break;
        }

      do_boundu_utf8:
        if (s == reginfo->strbeg) {
            if (reginfo->intuit || regtry(reginfo, &s))
            {
                goto got_it;
            }

            /* Didn't match.  Try at the next position (if there is one) */
            s += UTF8_SAFE_SKIP(s, reginfo->strend);
            if (UNLIKELY(s >= reginfo->strend)) {
                break;
            }
        }

        switch((bound_type) FLAGS(c)) {
          case TRADITIONAL_BOUND: /* Should have already been handled */
            assert(0);
            break;

          case GCB_BOUND:
            {
                GCB_enum before = getGCB_VAL_UTF8(
                                           reghop3((U8*)s, -1,
                                                   (U8*)(reginfo->strbeg)),
                                           (U8*) reginfo->strend);
                while (s < strend) {
                    GCB_enum after = getGCB_VAL_UTF8((U8*) s,
                                                    (U8*) reginfo->strend);
                    if (   (to_complement ^ isGCB(before,
                                                  after,
                                                  (U8*) reginfo->strbeg,
                                                  (U8*) s,
                                                  1 /* target is utf8 */ ))
                        && (reginfo->intuit || regtry(reginfo, &s)))
                    {
                        goto got_it;
                    }
                    before = after;
                    s += UTF8_SAFE_SKIP(s, reginfo->strend);
                }
            }
            break;

          case LB_BOUND:
            {
                LB_enum before = getLB_VAL_UTF8(reghop3((U8*)s,
                                                        -1,
                                                        (U8*)(reginfo->strbeg)),
                                                   (U8*) reginfo->strend);
                while (s < strend) {
                    LB_enum after = getLB_VAL_UTF8((U8*) s,
                                                   (U8*) reginfo->strend);
                    if (to_complement ^ isLB(before,
                                             after,
                                             (U8*) reginfo->strbeg,
                                             (U8*) s,
                                             (U8*) reginfo->strend,
                                             1 /* target is utf8 */ )
                        && (reginfo->intuit || regtry(reginfo, &s)))
                    {
                        goto got_it;
                    }
                    before = after;
                    s += UTF8_SAFE_SKIP(s, reginfo->strend);
                }
            }

            break;

          case SB_BOUND:
            {
                SB_enum before = getSB_VAL_UTF8(reghop3((U8*)s,
                                                    -1,
                                                    (U8*)(reginfo->strbeg)),
                                                  (U8*) reginfo->strend);
                while (s < strend) {
                    SB_enum after = getSB_VAL_UTF8((U8*) s,
                                                     (U8*) reginfo->strend);
                    if ((to_complement ^ isSB(before,
                                              after,
                                              (U8*) reginfo->strbeg,
                                              (U8*) s,
                                              (U8*) reginfo->strend,
                                              1 /* target is utf8 */ ))
                        && (reginfo->intuit || regtry(reginfo, &s)))
                    {
                        goto got_it;
                    }
                    before = after;
                    s += UTF8_SAFE_SKIP(s, reginfo->strend);
                }
            }

            break;

          case WB_BOUND:
            {
                /* We are at a boundary between char_sub_0 and char_sub_1.
                 * We also keep track of the value for char_sub_-1 as we
                 * loop through the line.   Context may be needed to make a
                 * determination, and if so, this can save having to
                 * recalculate it */
                WB_enum previous = WB_UNKNOWN;
                WB_enum before = getWB_VAL_UTF8(
                                          reghop3((U8*)s,
                                                  -1,
                                                  (U8*)(reginfo->strbeg)),
                                          (U8*) reginfo->strend);
                while (s < strend) {
                    WB_enum after = getWB_VAL_UTF8((U8*) s,
                                                    (U8*) reginfo->strend);
                    if ((to_complement ^ isWB(previous,
                                              before,
                                              after,
                                              (U8*) reginfo->strbeg,
                                              (U8*) s,
                                              (U8*) reginfo->strend,
                                              1 /* target is utf8 */ ))
                        && (reginfo->intuit || regtry(reginfo, &s)))
                    {
                        goto got_it;
                    }
                    previous = before;
                    before = after;
                    s += UTF8_SAFE_SKIP(s, reginfo->strend);
                }
            }
        }

        /* Here are at the final position in the target string, which is a
         * boundary by definition, so matches, depending on other constraints.
         * */

        if (   reginfo->intuit
            || (s <= reginfo->strend && regtry(reginfo, &s)))
        {
            goto got_it;
        }
        break;

      case LNBREAK_t8_pb:
      case LNBREAK_t8_p8:
        REXEC_FBC_UTF8_CLASS_SCAN(is_LNBREAK_utf8_safe(s, strend));
        break;

      case LNBREAK_tb_pb:
      case LNBREAK_tb_p8:
        REXEC_FBC_NON_UTF8_CLASS_SCAN(is_LNBREAK_latin1_safe(s, strend));
        break;

      /* The argument to all the POSIX node types is the class number to pass
       * to generic_isCC_() to build a mask for searching in PL_charclass[] */

      case NPOSIXL_t8_pb:
      case NPOSIXL_t8_p8:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXL_t8_pb:
      case POSIXL_t8_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        REXEC_FBC_UTF8_CLASS_SCAN(
            to_complement ^ cBOOL(isFOO_utf8_lc(FLAGS(c), (U8 *) s,
                                                          (U8 *) strend)));
        break;

      case NPOSIXL_tb_pb:
      case NPOSIXL_tb_p8:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXL_tb_pb:
      case POSIXL_tb_p8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        REXEC_FBC_NON_UTF8_CLASS_SCAN(
                                to_complement ^ cBOOL(isFOO_lc(FLAGS(c), *s)));
        break;

      case NPOSIXA_t8_pb:
      case NPOSIXA_t8_p8:
        /* The complement of something that matches only ASCII matches all
         * non-ASCII, plus everything in ASCII that isn't in the class. */
        REXEC_FBC_UTF8_CLASS_SCAN(   ! isASCII_utf8_safe(s, strend)
                                  || ! generic_isCC_A_(*s, FLAGS(c)));
        break;

      case POSIXA_t8_pb:
      case POSIXA_t8_p8:
        /* Don't need to worry about utf8, as it can match only a single
         * byte invariant character.  But we do anyway for performance reasons,
         * as otherwise we would have to examine all the continuation
         * characters */
        REXEC_FBC_UTF8_CLASS_SCAN(generic_isCC_A_(*s, FLAGS(c)));
        break;

      case NPOSIXD_tb_pb:
      case NPOSIXD_tb_p8:
      case NPOSIXA_tb_pb:
      case NPOSIXA_tb_p8:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXD_tb_pb:
      case POSIXD_tb_p8:
      case POSIXA_tb_pb:
      case POSIXA_tb_p8:
        REXEC_FBC_NON_UTF8_CLASS_SCAN(
                        to_complement ^ cBOOL(generic_isCC_A_(*s, FLAGS(c))));
        break;

      case NPOSIXU_tb_pb:
      case NPOSIXU_tb_p8:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXU_tb_pb:
      case POSIXU_tb_p8:
            REXEC_FBC_NON_UTF8_CLASS_SCAN(
                                 to_complement ^ cBOOL(generic_isCC_(*s,
                                                                    FLAGS(c))));
        break;

      case NPOSIXD_t8_pb:
      case NPOSIXD_t8_p8:
      case NPOSIXU_t8_pb:
      case NPOSIXU_t8_p8:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXD_t8_pb:
      case POSIXD_t8_p8:
      case POSIXU_t8_pb:
      case POSIXU_t8_p8:
        classnum = (char_class_number_) FLAGS(c);
        switch (classnum) {
          default:
            REXEC_FBC_UTF8_CLASS_SCAN(
                        to_complement ^ cBOOL(_invlist_contains_cp(
                                                PL_XPosix_ptrs[classnum],
                                                utf8_to_uvchr_buf((U8 *) s,
                                                                (U8 *) strend,
                                                                NULL))));
            break;

          case CC_ENUM_SPACE_:
            REXEC_FBC_UTF8_CLASS_SCAN(
                        to_complement ^ cBOOL(isSPACE_utf8_safe(s, strend)));
            break;

          case CC_ENUM_BLANK_:
            REXEC_FBC_UTF8_CLASS_SCAN(
                        to_complement ^ cBOOL(isBLANK_utf8_safe(s, strend)));
            break;

          case CC_ENUM_XDIGIT_:
            REXEC_FBC_UTF8_CLASS_SCAN(
                        to_complement ^ cBOOL(isXDIGIT_utf8_safe(s, strend)));
            break;

          case CC_ENUM_VERTSPACE_:
            REXEC_FBC_UTF8_CLASS_SCAN(
                        to_complement ^ cBOOL(isVERTWS_utf8_safe(s, strend)));
            break;

          case CC_ENUM_CNTRL_:
            REXEC_FBC_UTF8_CLASS_SCAN(
                        to_complement ^ cBOOL(isCNTRL_utf8_safe(s, strend)));
            break;
        }
        break;

      case AHOCORASICKC_tb_pb:
      case AHOCORASICKC_tb_p8:
      case AHOCORASICKC_t8_pb:
      case AHOCORASICKC_t8_p8:
      case AHOCORASICK_tb_pb:
      case AHOCORASICK_tb_p8:
      case AHOCORASICK_t8_pb:
      case AHOCORASICK_t8_p8:
        {
            DECL_TRIE_TYPE(c);
            /* what trie are we using right now */
            reg_ac_data *aho = (reg_ac_data*)progi->data->data[ ARG1u( c ) ];
            reg_trie_data *trie = (reg_trie_data*)progi->data->data[aho->trie];
            HV *widecharmap = MUTABLE_HV(progi->data->data[ aho->trie + 1 ]);

            const char *last_start = strend - trie->minlen;
#ifdef DEBUGGING
            const char *real_start = s;
#endif
            STRLEN maxlen = trie->maxlen;
            SV *sv_points;
            U8 **points; /* map of where we were in the input string
                            when reading a given char. For ASCII this
                            is unnecessary overhead as the relationship
                            is always 1:1, but for Unicode, especially
                            case folded Unicode this is not true. */
            U8 foldbuf[ UTF8_MAXBYTES_CASE + 1 ];
            U8 *bitmap=NULL;


            DECLARE_AND_GET_RE_DEBUG_FLAGS;

            /* We can't just allocate points here. We need to wrap it in
             * an SV so it gets freed properly if there is a croak while
             * running the match */
            ENTER;
            SAVETMPS;
            sv_points=newSV(maxlen * sizeof(U8 *));
            SvCUR_set(sv_points,
                maxlen * sizeof(U8 *));
            SvPOK_on(sv_points);
            sv_2mortal(sv_points);
            points=(U8**)SvPV_nolen(sv_points );
            if ( trie_type != trie_utf8_fold
                 && (trie->bitmap || OP(c)==AHOCORASICKC) )
            {
                if (trie->bitmap)
                    bitmap=(U8*)trie->bitmap;
                else
                    bitmap=(U8*)ANYOF_BITMAP(c);
            }
            /* this is the Aho-Corasick algorithm modified a touch
               to include special handling for long "unknown char" sequences.
               The basic idea being that we use AC as long as we are dealing
               with a possible matching char, when we encounter an unknown char
               (and we have not encountered an accepting state) we scan forward
               until we find a legal starting char.
               AC matching is basically that of trie matching, except that when
               we encounter a failing transition, we fall back to the current
               states "fail state", and try the current char again, a process
               we repeat until we reach the root state, state 1, or a legal
               transition. If we fail on the root state then we can either
               terminate if we have reached an accepting state previously, or
               restart the entire process from the beginning if we have not.

             */
            while (s <= last_start) {
                const U32 uniflags = UTF8_ALLOW_DEFAULT;
                U8 *uc = (U8*)s;
                U16 charid = 0;
                U32 base = 1;
                U32 state = 1;
                UV uvc = 0;
                STRLEN len = 0;
                STRLEN foldlen = 0;
                U8 *uscan = (U8*)NULL;
                U8 *leftmost = NULL;
#ifdef DEBUGGING
                U32 accepted_word= 0;
#endif
                U32 pointpos = 0;

                while ( state && uc <= (U8*)strend ) {
                    int failed=0;
                    U32 word = aho->states[ state ].wordnum;

                    if( state==1 ) {
                        if ( bitmap ) {
                            DEBUG_TRIE_EXECUTE_r(
                                if (  uc <= (U8*)last_start
                                    && !BITMAP_TEST(bitmap,*uc) )
                                {
                                    dump_exec_pos( (char *)uc, c, strend,
                                        real_start,
                                        (char *)uc, utf8_target, 0 );
                                    Perl_re_printf( aTHX_
                                        " Scanning for legal start char...\n");
                                }
                            );
                            if (utf8_target) {
                                while (  uc <= (U8*)last_start
                                       && !BITMAP_TEST(bitmap,*uc) )
                                {
                                    uc += UTF8SKIP(uc);
                                }
                            } else {
                                while (  uc <= (U8*)last_start
                                       && ! BITMAP_TEST(bitmap,*uc) )
                                {
                                    uc++;
                                }
                            }
                            s= (char *)uc;
                        }
                        if (uc >(U8*)last_start) break;
                    }

                    if ( word ) {
                        U8 *lpos= points[ (pointpos - trie->wordinfo[word].len)
                                                                    % maxlen ];
                        if (!leftmost || lpos < leftmost) {
                            DEBUG_r(accepted_word=word);
                            leftmost= lpos;
                        }
                        if (base==0) break;

                    }
                    points[pointpos++ % maxlen]= uc;
                    if (foldlen || uc < (U8*)strend) {
                        REXEC_TRIE_READ_CHAR(trie_type, trie, widecharmap, uc,
                                             (U8 *) strend, uscan, len, uvc,
                                             charid, foldlen, foldbuf,
                                             uniflags);
                        DEBUG_TRIE_EXECUTE_r({
                            dump_exec_pos( (char *)uc, c, strend,
                                        real_start, s, utf8_target, 0);
                            Perl_re_printf( aTHX_
                                " Charid:%3u CP:%4" UVxf " ",
                                 charid, uvc);
                        });
                    }
                    else {
                        len = 0;
                        charid = 0;
                    }


                    do {
#ifdef DEBUGGING
                        word = aho->states[ state ].wordnum;
#endif
                        base = aho->states[ state ].trans.base;

                        DEBUG_TRIE_EXECUTE_r({
                            if (failed)
                                dump_exec_pos((char *)uc, c, strend, real_start,
                                    s,   utf8_target, 0 );
                            Perl_re_printf( aTHX_
                                "%sState: %4" UVxf ", word=%" UVxf,
                                failed ? " Fail transition to " : "",
                                (UV)state, (UV)word);
                        });
                        if ( base ) {
                            U32 tmp;
                            I32 offset;
                            if (charid &&
                                 ( ((offset = base + charid
                                    - 1 - trie->uniquecharcount)) >= 0)
                                 && ((U32)offset < trie->lasttrans)
                                 && trie->trans[offset].check == state
                                 && (tmp=trie->trans[offset].next))
                            {
                                DEBUG_TRIE_EXECUTE_r(
                                    Perl_re_printf( aTHX_ " - legal\n"));
                                state = tmp;
                                break;
                            }
                            else {
                                DEBUG_TRIE_EXECUTE_r(
                                    Perl_re_printf( aTHX_ " - fail\n"));
                                failed = 1;
                                state = aho->fail[state];
                            }
                        }
                        else {
                            /* we must be accepting here */
                            DEBUG_TRIE_EXECUTE_r(
                                    Perl_re_printf( aTHX_ " - accepting\n"));
                            failed = 1;
                            break;
                        }
                    } while(state);
                    uc += len;
                    if (failed) {
                        if (leftmost)
                            break;
                        if (!state) state = 1;
                    }
                }
                if ( aho->states[ state ].wordnum ) {
                    U8 *lpos = points[ (pointpos
                                      - trie->wordinfo[aho->states[ state ]
                                                    .wordnum].len) % maxlen ];
                    if (!leftmost || lpos < leftmost) {
                        DEBUG_r(accepted_word=aho->states[ state ].wordnum);
                        leftmost = lpos;
                    }
                }
                if (leftmost) {
                    s = (char*)leftmost;
                    DEBUG_TRIE_EXECUTE_r({
                        Perl_re_printf( aTHX_  "Matches word #%" UVxf
                                        " at position %" IVdf ". Trying full"
                                        " pattern...\n",
                            (UV)accepted_word, (IV)(s - real_start)
                        );
                    });
                    if (reginfo->intuit || regtry(reginfo, &s)) {
                        FREETMPS;
                        LEAVE;
                        goto got_it;
                    }
                    if (s < reginfo->strend) {
                        s = HOPc(s,1);
                    }
                    DEBUG_TRIE_EXECUTE_r({
                        Perl_re_printf( aTHX_
                                       "Pattern failed. Looking for new start"
                                       " point...\n");
                    });
                } else {
                    DEBUG_TRIE_EXECUTE_r(
                        Perl_re_printf( aTHX_ "No match.\n"));
                    break;
                }
            }
            FREETMPS;
            LEAVE;
        }
        break;

      case EXACTFU_REQ8_t8_pb:
      case EXACTFUP_tb_p8:
      case EXACTFUP_t8_p8:
      case EXACTF_tb_p8:
      case EXACTF_t8_p8:   /* This node only generated for non-utf8 patterns */
      case EXACTFAA_NO_TRIE_tb_p8:
      case EXACTFAA_NO_TRIE_t8_p8: /* This node only generated for non-utf8
                                      patterns */
        assert(0);

      default:
        Perl_croak(aTHX_ "panic: unknown regstclass %d", (int)OP(c));
    } /* End of switch on node type */

    return 0;

  got_it:
    return s;
}

/* set RX_SAVED_COPY, RX_SUBBEG etc.
 * flags have same meanings as with regexec_flags() */

static void
S_reg_set_capture_string(pTHX_ REGEXP * const rx,
                            char *strbeg,
                            char *strend,
                            SV *sv,
                            U32 flags,
                            bool utf8_target)
{
    struct regexp *const prog = ReANY(rx);

    if (flags & REXEC_COPY_STR) {
#ifdef PERL_ANY_COW
        if (SvCANCOW(sv)) {
            DEBUG_C(Perl_re_printf( aTHX_
                              "Copy on write: regexp capture, type %d\n",
                                    (int) SvTYPE(sv)));
            /* Create a new COW SV to share the match string and store
             * in saved_copy, unless the current COW SV in saved_copy
             * is valid and suitable for our purpose */
            if ((   RXp_SAVED_COPY(prog)
                 && SvIsCOW(RXp_SAVED_COPY(prog))
                 && SvPOKp(RXp_SAVED_COPY(prog))
                 && SvIsCOW(sv)
                 && SvPOKp(sv)
                 && SvPVX(sv) == SvPVX(RXp_SAVED_COPY(prog))))
            {
                /* just reuse saved_copy SV */
                if (RXp_MATCH_COPIED(prog)) {
                    Safefree(RXp_SUBBEG(prog));
                    RXp_MATCH_COPIED_off(prog);
                }
            }
            else {
                /* create new COW SV to share string */
                RXp_MATCH_COPY_FREE(prog);
                RXp_SAVED_COPY(prog) = sv_setsv_cow(RXp_SAVED_COPY(prog), sv);
            }
            RXp_SUBBEG(prog) = (char *)SvPVX_const(RXp_SAVED_COPY(prog));
            assert (SvPOKp(RXp_SAVED_COPY(prog)));
            RXp_SUBLEN(prog)  = strend - strbeg;
            RXp_SUBOFFSET(prog) = 0;
            RXp_SUBCOFFSET(prog) = 0;
        } else
#endif
        {
            SSize_t min = 0;
            SSize_t max = strend - strbeg;
            SSize_t sublen;

            if (    (flags & REXEC_COPY_SKIP_POST)
                && !(prog->extflags & RXf_PMf_KEEPCOPY) /* //p */
                && !(PL_sawampersand & SAWAMPERSAND_RIGHT)
            ) { /* don't copy $' part of string */
                SSize_t offs_end;
                U32 n = 0;
                max = -1;
                /* calculate the right-most part of the string covered
                 * by a capture. Due to lookahead, this may be to
                 * the right of $&, so we have to scan all captures */
                while (n <= RXp_LASTPAREN(prog)) {
                    if ((offs_end = RXp_OFFS_END(prog,n)) > max)
                        max = offs_end;
                    n++;
                }
                if (max == -1)
                    max = (PL_sawampersand & SAWAMPERSAND_LEFT)
                            ? RXp_OFFS_START(prog,0)
                            : 0;
                assert(max >= 0 && max <= strend - strbeg);
            }

            if (    (flags & REXEC_COPY_SKIP_PRE)
                && !(prog->extflags & RXf_PMf_KEEPCOPY) /* //p */
                && !(PL_sawampersand & SAWAMPERSAND_LEFT)
            ) { /* don't copy $` part of string */
                U32 n = 0;
                min = max;
                /* calculate the left-most part of the string covered
                 * by a capture. Due to lookbehind, this may be to
                 * the left of $&, so we have to scan all captures */
                while (min && n <= RXp_LASTPAREN(prog)) {
                    I32 start = RXp_OFFS_START(prog,n);
                    if (   start != -1
                        && start < min)
                    {
                        min = start;
                    }
                    n++;
                }
                if ((PL_sawampersand & SAWAMPERSAND_RIGHT)
                    && min >  RXp_OFFS_END(prog,0)
                )
                    min = RXp_OFFS_END(prog,0);

            }

            assert(min >= 0 && min <= max && min <= strend - strbeg);
            sublen = max - min;

            if (RXp_MATCH_COPIED(prog)) {
                if (sublen > RXp_SUBLEN(prog))
                    RXp_SUBBEG(prog) =
                            (char*)saferealloc(RXp_SUBBEG(prog), sublen+1);
            }
            else
                RXp_SUBBEG(prog) = (char*)safemalloc(sublen+1);
            Copy(strbeg + min, RXp_SUBBEG(prog), sublen, char);
            RXp_SUBBEG(prog)[sublen] = '\0';
            RXp_SUBOFFSET(prog) = min;
            RXp_SUBLEN(prog) = sublen;
            RXp_MATCH_COPIED_on(prog);
        }
        RXp_SUBCOFFSET(prog) = RXp_SUBOFFSET(prog);
        if (RXp_SUBOFFSET(prog) && utf8_target) {
            /* Convert byte offset to chars.
             * XXX ideally should only compute this if @-/@+
             * has been seen, a la PL_sawampersand ??? */

            /* If there's a direct correspondence between the
             * string which we're matching and the original SV,
             * then we can use the utf8 len cache associated with
             * the SV. In particular, it means that under //g,
             * sv_pos_b2u() will use the previously cached
             * position to speed up working out the new length of
             * subcoffset, rather than counting from the start of
             * the string each time. This stops
             *   $x = "\x{100}" x 1E6; 1 while $x =~ /(.)/g;
             * from going quadratic */
            if (SvPOKp(sv) && SvPVX(sv) == strbeg)
                RXp_SUBCOFFSET(prog) = sv_pos_b2u_flags(sv, RXp_SUBCOFFSET(prog),
                                                SV_GMAGIC|SV_CONST_RETURN);
            else
                RXp_SUBCOFFSET(prog) = utf8_length((U8*)strbeg,
                                    (U8*)(strbeg+RXp_SUBOFFSET(prog)));
        }
    }
    else {
        RXp_MATCH_COPY_FREE(prog);
        RXp_SUBBEG(prog) = strbeg;
        RXp_SUBOFFSET(prog) = 0;
        RXp_SUBCOFFSET(prog) = 0;
        RXp_SUBLEN(prog) = strend - strbeg;
    }
}




/*
 - regexec_flags - match a regexp against a string
 */
I32
Perl_regexec_flags(pTHX_ REGEXP * const rx, char *stringarg, char *strend,
              char *strbeg, SSize_t minend, SV *sv, void *data, U32 flags)
/* stringarg: the point in the string at which to begin matching */
/* strend:    pointer to null at end of string */
/* strbeg:    real beginning of string */
/* minend:    end of match must be >= minend bytes after stringarg. */
/* sv:        SV being matched: only used for utf8 flag, pos() etc; string
 *            itself is accessed via the pointers above */
/* data:      May be used for some additional optimizations.
              Currently unused. */
/* flags:     For optimizations. See REXEC_* in regexp.h */

{
    struct regexp *const prog = ReANY(rx);
    char *s;
    regnode *c;
    char *startpos;
    SSize_t minlen;		/* must match at least this many chars */
    SSize_t dontbother = 0;	/* how many characters not to try at end */
    const bool utf8_target = cBOOL(DO_UTF8(sv));
    I32 multiline;
    RXi_GET_DECL(prog,progi);
    regmatch_info reginfo_buf;  /* create some info to pass to regtry etc */
    regmatch_info *const reginfo = &reginfo_buf;
    regexp_paren_pair *swap = NULL;
    I32 oldsave;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGEXEC_FLAGS;
    PERL_UNUSED_ARG(data);

    /* Be paranoid... */
    if (prog == NULL) {
        Perl_croak(aTHX_ "NULL regexp parameter");
    }

    DEBUG_EXECUTE_r(
        debug_start_match(rx, utf8_target, stringarg, strend,
        "Matching");
    );

    startpos = stringarg;

    /* set these early as they may be used by the HOP macros below */
    reginfo->strbeg = strbeg;
    reginfo->strend = strend;
    reginfo->is_utf8_target = cBOOL(utf8_target);

    if (prog->intflags & PREGf_GPOS_SEEN) {
        MAGIC *mg;

        /* set reginfo->ganch, the position where \G can match */

        reginfo->ganch =
            (flags & REXEC_IGNOREPOS)
            ? stringarg /* use start pos rather than pos() */
            : ((mg = mg_find_mglob(sv)) && mg->mg_len >= 0)
              /* Defined pos(): */
            ? strbeg + MgBYTEPOS(mg, sv, strbeg, strend-strbeg)
            : strbeg; /* pos() not defined; use start of string */

        DEBUG_GPOS_r(Perl_re_printf( aTHX_
            "GPOS ganch set to strbeg[%" IVdf "]\n", (IV)(reginfo->ganch - strbeg)));

        /* in the presence of \G, we may need to start looking earlier in
         * the string than the suggested start point of stringarg:
         * if prog->gofs is set, then that's a known, fixed minimum
         * offset, such as
         * /..\G/:   gofs = 2
         * /ab|c\G/: gofs = 1
         * or if the minimum offset isn't known, then we have to go back
         * to the start of the string, e.g. /w+\G/
         */

        if (prog->intflags & PREGf_ANCH_GPOS) {
            if (prog->gofs) {
                startpos = HOPBACKc(reginfo->ganch, prog->gofs);
                if (!startpos ||
                    ((flags & REXEC_FAIL_ON_UNDERFLOW) && startpos < stringarg))
                {
                    DEBUG_GPOS_r(Perl_re_printf( aTHX_
                            "fail: ganch-gofs before earliest possible start\n"));
                    return 0;
                }
            }
            else
                startpos = reginfo->ganch;
        }
        else if (prog->gofs) {
            startpos = HOPBACKc(startpos, prog->gofs);
            if (!startpos)
                startpos = strbeg;
        }
        else if (prog->intflags & PREGf_GPOS_FLOAT)
            startpos = strbeg;
    }

    minlen = prog->minlen;
    if ((startpos + minlen) > strend || startpos < strbeg) {
        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                        "Regex match can't succeed, so not even tried\n"));
        return 0;
    }

    /* at the end of this function, we'll do a LEAVE_SCOPE(oldsave),
     * which will call destuctors to reset PL_regmatch_state, free higher
     * PL_regmatch_slabs, and clean up regmatch_info_aux and
     * regmatch_info_aux_eval */

    oldsave = PL_savestack_ix;

    s = startpos;

    if ((prog->extflags & RXf_USE_INTUIT)
        && !(flags & REXEC_CHECKED))
    {
        s = re_intuit_start(rx, sv, strbeg, startpos, strend,
                                    flags, NULL);
        if (!s)
            return 0;

        if (prog->extflags & RXf_CHECK_ALL) {
            /* we can match based purely on the result of INTUIT.
             * Set up captures etc just for $& and $-[0]
             * (an intuit-only match wont have $1,$2,..) */
            assert(!prog->nparens);

            /* s/// doesn't like it if $& is earlier than where we asked it to
             * start searching (which can happen on something like /.\G/) */
            if (       (flags & REXEC_FAIL_ON_UNDERFLOW)
                    && (s < stringarg))
            {
                /* this should only be possible under \G */
                assert(prog->intflags & PREGf_GPOS_SEEN);
                DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                    "matched, but failing for REXEC_FAIL_ON_UNDERFLOW\n"));
                goto phooey;
            }

            /* match via INTUIT shouldn't have any captures.
             * Let @-, @+, $^N know */
            RXp_LASTPAREN(prog) = RXp_LASTCLOSEPAREN(prog) = 0;
            RXp_MATCH_UTF8_set(prog, utf8_target);
            SSize_t match_start = s - strbeg;
            SSize_t match_end = utf8_target
                ? (char*)utf8_hop_forward((U8*)s, prog->minlenret, (U8 *) strend) - strbeg
                : s - strbeg + prog->minlenret;
            CLOSE_ANY_CAPTURE(prog, 0, match_start, match_end);
            if ( !(flags & REXEC_NOT_FIRST) )
                S_reg_set_capture_string(aTHX_ rx,
                                        strbeg, strend,
                                        sv, flags, utf8_target);

            return 1;
        }
    }

    multiline = prog->extflags & RXf_PMf_MULTILINE;

    if (strend - s < (minlen+(prog->check_offset_min<0?prog->check_offset_min:0))) {
        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                              "String too short [regexec_flags]...\n"));
        goto phooey;
    }

    /* Check validity of program. */
    if (UCHARAT(progi->program) != REG_MAGIC) {
        Perl_croak(aTHX_ "corrupted regexp program");
    }

    RXp_MATCH_TAINTED_off(prog);
    RXp_MATCH_UTF8_set(prog, utf8_target);

    reginfo->prog = rx;	 /* Yes, sorry that this is confusing.  */
    reginfo->intuit = 0;
    reginfo->is_utf8_pat = cBOOL(RX_UTF8(rx));
    reginfo->warned = FALSE;
    reginfo->sv = sv;
    reginfo->poscache_maxiter = 0; /* not yet started a countdown */
    /* see how far we have to get to not match where we matched before */
    reginfo->till = stringarg + minend;

    if (prog->extflags & RXf_EVAL_SEEN && SvPADTMP(sv)) {
        /* SAVEFREESV, not sv_mortalcopy, as this SV must last until after
           S_cleanup_regmatch_info_aux has executed (registered by
           SAVEDESTRUCTOR_X below).  S_cleanup_regmatch_info_aux modifies
           magic belonging to this SV.
           Not newSVsv, either, as it does not COW.
        */
        reginfo->sv = newSV_type(SVt_NULL);
        SvSetSV_nosteal(reginfo->sv, sv);
        SAVEFREESV(reginfo->sv);
    }

    /* reserve next 2 or 3 slots in PL_regmatch_state:
     * slot N+0: may currently be in use: skip it
     * slot N+1: use for regmatch_info_aux struct
     * slot N+2: use for regmatch_info_aux_eval struct if we have (?{})'s
     * slot N+3: ready for use by regmatch()
     */

    {
        regmatch_state *old_regmatch_state;
        regmatch_slab  *old_regmatch_slab;
        int i, max = (prog->extflags & RXf_EVAL_SEEN) ? 2 : 1;

        /* on first ever match, allocate first slab */
        if (!PL_regmatch_slab) {
            Newx(PL_regmatch_slab, 1, regmatch_slab);
            PL_regmatch_slab->prev = NULL;
            PL_regmatch_slab->next = NULL;
            PL_regmatch_state = SLAB_FIRST(PL_regmatch_slab);
        }

        old_regmatch_state = PL_regmatch_state;
        old_regmatch_slab  = PL_regmatch_slab;

        for (i=0; i <= max; i++) {
            if (i == 1)
                reginfo->info_aux = &(PL_regmatch_state->u.info_aux);
            else if (i ==2)
                reginfo->info_aux_eval =
                reginfo->info_aux->info_aux_eval =
                            &(PL_regmatch_state->u.info_aux_eval);

            if (++PL_regmatch_state >  SLAB_LAST(PL_regmatch_slab))
                PL_regmatch_state = S_push_slab(aTHX);
        }

        /* note initial PL_regmatch_state position; at end of match we'll
         * pop back to there and free any higher slabs */

        reginfo->info_aux->old_regmatch_state = old_regmatch_state;
        reginfo->info_aux->old_regmatch_slab  = old_regmatch_slab;
        reginfo->info_aux->poscache = NULL;

        SAVEDESTRUCTOR_X(S_cleanup_regmatch_info_aux, reginfo->info_aux);

        if ((prog->extflags & RXf_EVAL_SEEN))
            S_setup_eval_state(aTHX_ reginfo);
        else
            reginfo->info_aux_eval = reginfo->info_aux->info_aux_eval = NULL;
    }

    if (PL_curpm && (PM_GETRE(PL_curpm) == rx)) {
        /* We have to be careful. If the previous successful match
           was from this regex we don't want a subsequent partially
           successful match to clobber the old results.
           So when we detect this possibility we add a swap buffer
           to the re, and switch the buffer each match. If we fail,
           we switch it back; otherwise we leave it swapped.
        */
        swap = RXp_OFFSp(prog);
        /* avoid leak if we die, or clean up anyway if match completes */
        SAVEFREEPV(swap);
        Newxz(RXp_OFFSp(prog), (prog->nparens + 1), regexp_paren_pair);
        DEBUG_BUFFERS_r(Perl_re_exec_indentf( aTHX_
            "rex=0x%" UVxf " saving  offs: orig=0x%" UVxf " new=0x%" UVxf "\n",
            0,
            PTR2UV(prog),
            PTR2UV(swap),
            PTR2UV(RXp_OFFSp(prog))
        ));
    }

    if (prog->recurse_locinput)
        Zero(prog->recurse_locinput,prog->nparens + 1, char *);

    /* Simplest case: anchored match (but not \G) need be tried only once,
     * or with MBOL, only at the beginning of each line.
     *
     * Note that /.*.../ sets PREGf_IMPLICIT|MBOL, while /.*.../s sets
     * PREGf_IMPLICIT|SBOL. The idea is that with /.*.../s, if it doesn't
     * match at the start of the string then it won't match anywhere else
     * either; while with /.*.../, if it doesn't match at the beginning,
     * the earliest it could match is at the start of the next line */

    if (prog->intflags & (PREGf_ANCH & ~PREGf_ANCH_GPOS)) {
        char *end;

        if (regtry(reginfo, &s))
            goto got_it;

        if (!(prog->intflags & PREGf_ANCH_MBOL))
            goto phooey;

        /* didn't match at start, try at other newline positions */

        if (minlen)
            dontbother = minlen - 1;
        end = HOP3c(strend, -dontbother, strbeg) - 1;

        /* skip to next newline */

        while (s <= end) { /* note it could be possible to match at the end of the string */
            /* NB: newlines are the same in unicode as they are in latin */
            if (*s++ != '\n')
                continue;
            if (prog->check_substr || prog->check_utf8) {
            /* note that with PREGf_IMPLICIT, intuit can only fail
             * or return the start position, so it's of limited utility.
             * Nevertheless, I made the decision that the potential for
             * quick fail was still worth it - DAPM */
                s = re_intuit_start(rx, sv, strbeg, s, strend, flags, NULL);
                if (!s)
                    goto phooey;
            }
            if (regtry(reginfo, &s))
                goto got_it;
        }
        goto phooey;
    } /* end anchored search */

    /* anchored \G match */
    if (prog->intflags & PREGf_ANCH_GPOS)
    {
        /* PREGf_ANCH_GPOS should never be true if PREGf_GPOS_SEEN is not true */
        assert(prog->intflags & PREGf_GPOS_SEEN);
        /* For anchored \G, the only position it can match from is
         * (ganch-gofs); we already set startpos to this above; if intuit
         * moved us on from there, we can't possibly succeed */
        assert(startpos == HOPBACKc(reginfo->ganch, prog->gofs));
        if (s == startpos && regtry(reginfo, &s))
            goto got_it;
        goto phooey;
    }

    /* Messy cases:  unanchored match. */

    if ((prog->anchored_substr || prog->anchored_utf8) && prog->intflags & PREGf_SKIP) {
        /* we have /x+whatever/ */
        /* it must be a one character string (XXXX Except is_utf8_pat?) */
        char ch;
#ifdef DEBUGGING
        int did_match = 0;
#endif
        if (utf8_target) {
            if (! prog->anchored_utf8) {
                to_utf8_substr(prog);
            }
            ch = SvPVX_const(prog->anchored_utf8)[0];
            REXEC_FBC_UTF8_SCAN(
                if (*s == ch) {
                    DEBUG_EXECUTE_r( did_match = 1 );
                    if (regtry(reginfo, &s)) goto got_it;
                    s += UTF8_SAFE_SKIP(s, strend);
                    while (s < strend && *s == ch)
                        s += UTF8SKIP(s);
                }
            );

        }
        else {
            if (! prog->anchored_substr) {
                if (! to_byte_substr(prog)) {
                    NON_UTF8_TARGET_BUT_UTF8_REQUIRED(phooey);
                }
            }
            ch = SvPVX_const(prog->anchored_substr)[0];
            REXEC_FBC_NON_UTF8_SCAN(
                if (*s == ch) {
                    DEBUG_EXECUTE_r( did_match = 1 );
                    if (regtry(reginfo, &s)) goto got_it;
                    s++;
                    while (s < strend && *s == ch)
                        s++;
                }
            );
        }
        DEBUG_EXECUTE_r(if (!did_match)
                Perl_re_printf( aTHX_
                                  "Did not find anchored character...\n")
               );
    }
    else if (prog->anchored_substr != NULL
              || prog->anchored_utf8 != NULL
              || ((prog->float_substr != NULL || prog->float_utf8 != NULL)
                  && prog->float_max_offset < strend - s)) {
        SV *must;
        SSize_t back_max;
        SSize_t back_min;
        char *last;
        char *last1;		/* Last position checked before */
#ifdef DEBUGGING
        int did_match = 0;
#endif
        if (prog->anchored_substr || prog->anchored_utf8) {
            if (utf8_target) {
                if (! prog->anchored_utf8) {
                    to_utf8_substr(prog);
                }
                must = prog->anchored_utf8;
            }
            else {
                if (! prog->anchored_substr) {
                    if (! to_byte_substr(prog)) {
                        NON_UTF8_TARGET_BUT_UTF8_REQUIRED(phooey);
                    }
                }
                must = prog->anchored_substr;
            }
            back_max = back_min = prog->anchored_offset;
        } else {
            if (utf8_target) {
                if (! prog->float_utf8) {
                    to_utf8_substr(prog);
                }
                must = prog->float_utf8;
            }
            else {
                if (! prog->float_substr) {
                    if (! to_byte_substr(prog)) {
                        NON_UTF8_TARGET_BUT_UTF8_REQUIRED(phooey);
                    }
                }
                must = prog->float_substr;
            }
            back_max = prog->float_max_offset;
            back_min = prog->float_min_offset;
        }

        if (back_min<0) {
            last = strend;
        } else {
            last = HOP3c(strend,	/* Cannot start after this */
                  -(SSize_t)(CHR_SVLEN(must)
                         - (SvTAIL(must) != 0) + back_min), strbeg);
        }
        if (s > reginfo->strbeg)
            last1 = HOPc(s, -1);
        else
            last1 = s - 1;	/* bogus */

        /* XXXX check_substr already used to find "s", can optimize if
           check_substr==must. */
        dontbother = 0;
        strend = HOPc(strend, -dontbother);
        while ( (s <= last) &&
                (s = fbm_instr((unsigned char*)HOP4c(s, back_min, strbeg,  strend),
                                  (unsigned char*)strend, must,
                                  multiline ? FBMrf_MULTILINE : 0)) ) {
            DEBUG_EXECUTE_r( did_match = 1 );
            if (HOPc(s, -back_max) > last1) {
                last1 = HOPc(s, -back_min);
                s = HOPc(s, -back_max);
            }
            else {
                char * const t = (last1 >= reginfo->strbeg)
                                    ? HOPc(last1, 1) : last1 + 1;

                last1 = HOPc(s, -back_min);
                s = t;
            }
            if (utf8_target) {
                while (s <= last1) {
                    if (regtry(reginfo, &s))
                        goto got_it;
                    if (s >= last1) {
                        s++; /* to break out of outer loop */
                        break;
                    }
                    s += UTF8SKIP(s);
                }
            }
            else {
                while (s <= last1) {
                    if (regtry(reginfo, &s))
                        goto got_it;
                    s++;
                }
            }
        }
        DEBUG_EXECUTE_r(if (!did_match) {
            RE_PV_QUOTED_DECL(quoted, utf8_target, PERL_DEBUG_PAD_ZERO(0),
                SvPVX_const(must), RE_SV_DUMPLEN(must), 30);
            Perl_re_printf( aTHX_  "Did not find %s substr %s%s...\n",
                              ((must == prog->anchored_substr || must == prog->anchored_utf8)
                               ? "anchored" : "floating"),
                quoted, RE_SV_TAIL(must));
        });
        goto phooey;
    }
    else if ( (c = progi->regstclass) ) {
        if (minlen) {
            const OPCODE op = OP(progi->regstclass);
            /* don't bother with what can't match */
            if (REGNODE_TYPE(op) != EXACT && REGNODE_TYPE(op) != TRIE)
                strend = HOPc(strend, -(minlen - 1));
        }
        DEBUG_EXECUTE_r({
            SV * const prop = sv_newmortal();
            regprop(prog, prop, c, reginfo, NULL);
            {
                RE_PV_QUOTED_DECL(quoted,utf8_target,PERL_DEBUG_PAD_ZERO(1),
                    s,strend-s,PL_dump_re_max_len);
                Perl_re_printf( aTHX_
                    "Matching stclass %.*s against %s (%d bytes)\n",
                    (int)SvCUR(prop), SvPVX_const(prop),
                     quoted, (int)(strend - s));
            }
        });
        if (find_byclass(prog, c, s, strend, reginfo))
            goto got_it;
        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_  "Contradicts stclass... [regexec_flags]\n"));
    }
    else {
        dontbother = 0;
        if (prog->float_substr != NULL || prog->float_utf8 != NULL) {
            /* Trim the end. */
            char *last= NULL;
            SV* float_real;
            STRLEN len;
            const char *little;

            if (utf8_target) {
                if (! prog->float_utf8) {
                    to_utf8_substr(prog);
                }
                float_real = prog->float_utf8;
            }
            else {
                if (! prog->float_substr) {
                    if (! to_byte_substr(prog)) {
                        NON_UTF8_TARGET_BUT_UTF8_REQUIRED(phooey);
                    }
                }
                float_real = prog->float_substr;
            }

            little = SvPV_const(float_real, len);
            if (SvTAIL(float_real)) {
                    /* This means that float_real contains an artificial \n on
                     * the end due to the presence of something like this:
                     * /foo$/ where we can match both "foo" and "foo\n" at the
                     * end of the string.  So we have to compare the end of the
                     * string first against the float_real without the \n and
                     * then against the full float_real with the string.  We
                     * have to watch out for cases where the string might be
                     * smaller than the float_real or the float_real without
                     * the \n. */
                    char *checkpos= strend - len;
                    DEBUG_OPTIMISE_r(
                        Perl_re_printf( aTHX_
                            "%sChecking for float_real.%s\n",
                            PL_colors[4], PL_colors[5]));
                    if (checkpos + 1 < strbeg) {
                        /* can't match, even if we remove the trailing \n
                         * string is too short to match */
                        DEBUG_EXECUTE_r(
                            Perl_re_printf( aTHX_
                                "%sString shorter than required trailing substring, cannot match.%s\n",
                                PL_colors[4], PL_colors[5]));
                        goto phooey;
                    } else if (memEQ(checkpos + 1, little, len - 1)) {
                        /* can match, the end of the string matches without the
                         * "\n" */
                        last = checkpos + 1;
                    } else if (checkpos < strbeg) {
                        /* cant match, string is too short when the "\n" is
                         * included */
                        DEBUG_EXECUTE_r(
                            Perl_re_printf( aTHX_
                                "%sString does not contain required trailing substring, cannot match.%s\n",
                                PL_colors[4], PL_colors[5]));
                        goto phooey;
                    } else if (!multiline) {
                        /* non multiline match, so compare with the "\n" at the
                         * end of the string */
                        if (memEQ(checkpos, little, len)) {
                            last= checkpos;
                        } else {
                            DEBUG_EXECUTE_r(
                                Perl_re_printf( aTHX_
                                    "%sString does not contain required trailing substring, cannot match.%s\n",
                                    PL_colors[4], PL_colors[5]));
                            goto phooey;
                        }
                    } else {
                        /* multiline match, so we have to search for a place
                         * where the full string is located */
                        goto find_last;
                    }
            } else {
                  find_last:
                    if (len)
                        last = rninstr(s, strend, little, little + len);
                    else
                        last = strend;	/* matching "$" */
            }
            if (!last) {
                /* at one point this block contained a comment which was
                 * probably incorrect, which said that this was a "should not
                 * happen" case.  Even if it was true when it was written I am
                 * pretty sure it is not anymore, so I have removed the comment
                 * and replaced it with this one. Yves */
                DEBUG_EXECUTE_r(
                    Perl_re_printf( aTHX_
                        "%sString does not contain required substring, cannot match.%s\n",
                        PL_colors[4], PL_colors[5]
                    ));
                goto phooey;
            }
            dontbother = strend - last + prog->float_min_offset;
        }
        if (minlen && (dontbother < minlen))
            dontbother = minlen - 1;
        strend -= dontbother; 		   /* this one's always in bytes! */
        /* We don't know much -- general case. */
        if (utf8_target) {
            for (;;) {
                if (regtry(reginfo, &s))
                    goto got_it;
                if (s >= strend)
                    break;
                s += UTF8SKIP(s);
            };
        }
        else {
            do {
                if (regtry(reginfo, &s))
                    goto got_it;
            } while (s++ < strend);
        }
    }

    /* Failure. */
    goto phooey;

  got_it:
    /* s/// doesn't like it if $& is earlier than where we asked it to
     * start searching (which can happen on something like /.\G/) */
    if (       (flags & REXEC_FAIL_ON_UNDERFLOW)
            && (RXp_OFFS_START(prog,0) < stringarg - strbeg))
    {
        /* this should only be possible under \G */
        assert(prog->intflags & PREGf_GPOS_SEEN);
        DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
            "matched, but failing for REXEC_FAIL_ON_UNDERFLOW\n"));
        goto phooey;
    }

    /* clean up; this will trigger destructors that will free all slabs
     * above the current one, and cleanup the regmatch_info_aux
     * and regmatch_info_aux_eval sructs */

    LEAVE_SCOPE(oldsave);

    if (RXp_PAREN_NAMES(prog))
        (void)hv_iterinit(RXp_PAREN_NAMES(prog));

    /* make sure $`, $&, $', and $digit will work later */
    if ( !(flags & REXEC_NOT_FIRST) )
        S_reg_set_capture_string(aTHX_ rx,
                                    strbeg, reginfo->strend,
                                    sv, flags, utf8_target);

    return 1;

  phooey:
    DEBUG_EXECUTE_r(Perl_re_printf( aTHX_  "%sMatch failed%s\n",
                          PL_colors[4], PL_colors[5]));

    if (swap) {
        /* we failed :-( roll it back.
         * Since the swap buffer will be freed on scope exit which follows
         * shortly, restore the old captures by copying 'swap's original
         * data to the new offs buffer
         */
        DEBUG_BUFFERS_r(Perl_re_exec_indentf( aTHX_
            "rex=0x%" UVxf " rolling back offs: 0x%" UVxf " will be freed; restoring data to =0x%" UVxf "\n",
            0,
            PTR2UV(prog),
            PTR2UV(RXp_OFFSp(prog)),
            PTR2UV(swap)
        ));

        Copy(swap, RXp_OFFSp(prog), prog->nparens + 1, regexp_paren_pair);
    }

    /* clean up; this will trigger destructors that will free all slabs
     * above the current one, and cleanup the regmatch_info_aux
     * and regmatch_info_aux_eval sructs */

    LEAVE_SCOPE(oldsave);

    return 0;
}


/* Set which rex is pointed to by PL_reg_curpm, handling ref counting.
 * Do inc before dec, in case old and new rex are the same */
#define SET_reg_curpm(Re2)                          \
    if (reginfo->info_aux_eval) {                   \
        (void)ReREFCNT_inc(Re2);		    \
        ReREFCNT_dec(PM_GETRE(PL_reg_curpm));	    \
        PM_SETRE((PL_reg_curpm), (Re2));	    \
    }


/*
 - regtry - try match at specific point
 */
STATIC bool			/* 0 failure, 1 success */
S_regtry(pTHX_ regmatch_info *reginfo, char **startposp)
{
    CHECKPOINT lastcp;
    REGEXP *const rx = reginfo->prog;
    regexp *const prog = ReANY(rx);
    SSize_t result;
#ifdef DEBUGGING
    U32 depth = 0; /* used by REGCP_SET */
#endif
    RXi_GET_DECL(prog,progi);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGTRY;

    reginfo->cutpoint=NULL;

    RXp_OFFSp(prog)[0].start = *startposp - reginfo->strbeg;
    RXp_LASTPAREN(prog) = 0;
    RXp_LASTCLOSEPAREN(prog) = 0;

    /* XXXX What this code is doing here?!!!  There should be no need
       to do this again and again, RXp_LASTPAREN(prog) should take care of
       this!  --ilya*/

    /* Tests pat.t#187 and split.t#{13,14} seem to depend on this code.
     * Actually, the code in regcppop() (which Ilya may be meaning by
     * RXp_LASTPAREN(prog)), is not needed at all by the test suite
     * (op/regexp, op/pat, op/split), but that code is needed otherwise
     * this erroneously leaves $1 defined: "1" =~ /^(?:(\d)x)?\d$/
     * Meanwhile, this code *is* needed for the
     * above-mentioned test suite tests to succeed.  The common theme
     * on those tests seems to be returning null fields from matches.
     * --jhi updated by dapm */

    /* After encountering a variant of the issue mentioned above I think
     * the point Ilya was making is that if we properly unwind whenever
     * we set lastparen to a smaller value then we should not need to do
     * this every time, only when needed. So if we have tests that fail if
     * we remove this, then it suggests somewhere else we are improperly
     * unwinding the lastparen/paren buffers. See UNWIND_PARENS() and
     * places it is called, and related regcp() routines. - Yves */
#if 1
    if (prog->nparens) {
        regexp_paren_pair *pp = RXp_OFFSp(prog);
        I32 i;
        for (i = prog->nparens; i > (I32)RXp_LASTPAREN(prog); i--) {
            ++pp;
            pp->start = -1;
            pp->end = -1;
        }
    }
#endif
    REGCP_SET(lastcp);
    result = regmatch(reginfo, *startposp, progi->program + 1);
    if (result != -1) {
        RXp_OFFSp(prog)[0].end = result;
        return 1;
    }
    if (reginfo->cutpoint)
        *startposp= reginfo->cutpoint;
    REGCP_UNWIND(lastcp);
    return 0;
}

/* this is used to determine how far from the left messages like
   'failed...' are printed in regexec.c. It should be set such that
   messages are inline with the regop output that created them.
*/
#define REPORT_CODE_OFF 29
#define INDENT_CHARS(depth) ((int)(depth) % 20)
#ifdef DEBUGGING
int
Perl_re_exec_indentf(pTHX_ const char *fmt, U32 depth, ...)
{
    va_list ap;
    int result;
    PerlIO *f= Perl_debug_log;
    PERL_ARGS_ASSERT_RE_EXEC_INDENTF;
    va_start(ap, depth);
    PerlIO_printf(f, "%*s|%4" UVuf "| %*s", REPORT_CODE_OFF, "", (UV)depth, INDENT_CHARS(depth), "" );
    result = PerlIO_vprintf(f, fmt, ap);
    va_end(ap);
    return result;
}
#endif /* DEBUGGING */

/* grab a new slab and return the first slot in it */

STATIC regmatch_state *
S_push_slab(pTHX)
{
    regmatch_slab *s = PL_regmatch_slab->next;
    if (!s) {
        Newx(s, 1, regmatch_slab);
        s->prev = PL_regmatch_slab;
        s->next = NULL;
        PL_regmatch_slab->next = s;
    }
    PL_regmatch_slab = s;
    return SLAB_FIRST(s);
}

#ifdef DEBUGGING

STATIC void
S_debug_start_match(pTHX_ const REGEXP *prog, const bool utf8_target,
    const char *start, const char *end, const char *blurb)
{
    const bool utf8_pat = RX_UTF8(prog) ? 1 : 0;

    PERL_ARGS_ASSERT_DEBUG_START_MATCH;

    if (!PL_colorset)
            reginitcolors();
    {
        RE_PV_QUOTED_DECL(s0, utf8_pat, PERL_DEBUG_PAD_ZERO(0),
            RX_PRECOMP_const(prog), RX_PRELEN(prog), PL_dump_re_max_len);

        RE_PV_QUOTED_DECL(s1, utf8_target, PERL_DEBUG_PAD_ZERO(1),
            start, end - start, PL_dump_re_max_len);

        Perl_re_printf( aTHX_
            "%s%s REx%s %s against %s\n",
                       PL_colors[4], blurb, PL_colors[5], s0, s1);

        if (utf8_target||utf8_pat)
            Perl_re_printf( aTHX_  "UTF-8 %s%s%s...\n",
                utf8_pat ? "pattern" : "",
                utf8_pat && utf8_target ? " and " : "",
                utf8_target ? "string" : ""
            );
    }
}

STATIC void
S_dump_exec_pos(pTHX_ const char *locinput,
                      const regnode *scan,
                      const char *loc_regeol,
                      const char *loc_bostr,
                      const char *loc_reg_starttry,
                      const bool utf8_target,
                      const U32 depth
                )
{
    const int docolor = *PL_colors[0] || *PL_colors[2] || *PL_colors[4];
    const int taill = (docolor ? 10 : 7); /* 3 chars for "> <" */
    int l = (loc_regeol - locinput) > taill ? taill : (loc_regeol - locinput);
    /* The part of the string before starttry has one color
       (pref0_len chars), between starttry and current
       position another one (pref_len - pref0_len chars),
       after the current position the third one.
       We assume that pref0_len <= pref_len, otherwise we
       decrease pref0_len.  */
    int pref_len = (locinput - loc_bostr) > (5 + taill) - l
        ? (5 + taill) - l : locinput - loc_bostr;
    int pref0_len;

    PERL_ARGS_ASSERT_DUMP_EXEC_POS;

    if (utf8_target) {
        while (UTF8_IS_CONTINUATION(*(U8*)(locinput - pref_len))) {
            pref_len++;
        }
    }
    pref0_len = pref_len  - (locinput - loc_reg_starttry);
    if (l + pref_len < (5 + taill) && l < loc_regeol - locinput)
        l = ( loc_regeol - locinput > (5 + taill) - pref_len
              ? (5 + taill) - pref_len : loc_regeol - locinput);
    if (utf8_target) {
        while (UTF8_IS_CONTINUATION(*(U8*)(locinput + l))) {
            l--;
        }
    }
    if (pref0_len < 0)
        pref0_len = 0;
    if (pref0_len > pref_len)
        pref0_len = pref_len;
    {
        const int is_uni = utf8_target ? 1 : 0;

        RE_PV_COLOR_DECL(s0,len0,is_uni,PERL_DEBUG_PAD(0),
            (locinput - pref_len),pref0_len, PL_dump_re_max_len, 4, 5);

        RE_PV_COLOR_DECL(s1,len1,is_uni,PERL_DEBUG_PAD(1),
                    (locinput - pref_len + pref0_len),
                    pref_len - pref0_len, PL_dump_re_max_len, 2, 3);

        RE_PV_COLOR_DECL(s2,len2,is_uni,PERL_DEBUG_PAD(2),
                    locinput, loc_regeol - locinput, 10, 0, 1);

        const STRLEN tlen=len0+len1+len2;
        Perl_re_printf( aTHX_
                    "%4" IVdf " <%.*s%.*s%s%.*s>%*s|%4" UVuf "| ",
                    (IV)(locinput - loc_bostr),
                    len0, s0,
                    len1, s1,
                    (docolor ? "" : "> <"),
                    len2, s2,
                    (int)(tlen > 19 ? 0 :  19 - tlen),
                    "",
                    (UV)depth);
    }
}

#endif

/* reg_check_named_buff_matched()
 * Checks to see if a named buffer has matched. The data array of
 * buffer numbers corresponding to the buffer is expected to reside
 * in the regexp->data->data array in the slot stored in the ARG1u() of
 * node involved. Note that this routine doesn't actually care about the
 * name, that information is not preserved from compilation to execution.
 * Returns the index of the leftmost defined buffer with the given name
 * or 0 if non of the buffers matched.
 */
STATIC I32
S_reg_check_named_buff_matched(const regexp *rex, const regnode *scan)
{
    I32 n;
    RXi_GET_DECL(rex,rexi);
    SV *sv_dat= MUTABLE_SV(rexi->data->data[ ARG1u( scan ) ]);
    I32 *nums=(I32*)SvPVX(sv_dat);

    PERL_ARGS_ASSERT_REG_CHECK_NAMED_BUFF_MATCHED;

    for ( n=0; n<SvIVX(sv_dat); n++ ) {
        if ((I32)RXp_LASTPAREN(rex) >= nums[n] &&
            RXp_OFFS_END(rex,nums[n]) != -1)
        {
            return nums[n];
        }
    }
    return 0;
}

static bool
S_setup_EXACTISH_ST(pTHX_ const regnode * const text_node,
                          struct next_matchable_info * m,
                          regmatch_info *reginfo)
{
    /* This function determines various characteristics about every possible
     * initial match of the passed-in EXACTish <text_node>, and stores them in
     * <*m>.
     *
     * That includes a match string and a parallel mask, such that if you AND
     * the target string with the mask and compare with the match string,
     * you'll have a pretty good idea, perhaps even perfect, if that portion of
     * the target matches or not.
     *
     * The motivation behind this function is to allow the caller to set up
     * tight loops for matching.  Consider patterns like '.*B' or '.*?B' where
     * B is an arbitrary EXACTish node.  To find the end of .*, we look for the
     * beginning oF B, which is the passed in <text_node>  That's where this
     * function comes in.  The values it returns can quickly be used to rule
     * out many, or all, cases of possible matches not actually being the
     * beginning of B, <text_node>.  It is also used in regrepeat() where we
     * have 'A*', for arbitrary 'A'.  This sets up criteria to more efficiently
     * determine where the span of 'A's stop.
     *
     * If <text_node> is of type EXACT, there is only one possible character
     * that can match its first character, and so the situation is quite
     * simple.  But things can get much more complicated if folding is
     * involved.  It may be that the first character of an EXACTFish node
     * doesn't participate in any possible fold, e.g., punctuation, so it can
     * be matched only by itself.  The vast majority of characters that are in
     * folds match just two things, their lower and upper-case equivalents.
     * But not all are like that; some have multiple possible matches, or match
     * sequences of more than one character.  This function sorts all that out.
     *
     * It returns information about all possibilities of what the first
     * character(s) of <text_node> could look like.  Again, if <text_node> is a
     * plain EXACT node, that's just the actual first bytes of the first
     * character; but otherwise it is the bytes, that when masked, match all
     * possible combinations of all the initial bytes of all the characters
     * that could match, folded.  (Actually, this is a slight over promise.  It
     * handles only up to the initial 5 bytes, which is enough for all Unicode
     * characters, but not for all non-Unicode ones.)
     *
     * Here's an example to clarify.  Suppose the first character of
     * <text_node> is the letter 'C', and we are under /i matching.  That means
     * 'c' also matches.  The representations of these two characters differ in
     * just one bit, so the mask would be a zero in that position and ones in
     * the other 7.  And the returned string would be the AND of these two
     * characters, and would be one byte long, since these characters are each
     * a single byte.  ANDing the target <text_node> with this mask will yield
     * the returned string if and only if <text_node> begins with one of these
     * two characters.  So, the function would also return that the definitive
     * length matched is 1 byte.
     *
     * Now, suppose instead of the letter 'C',  <text_node> begins with the
     * letter 'F'.  The situation is much more complicated because there are
     * various ligatures such as LATIN SMALL LIGATURE FF, whose fold also
     * begins with 'f', and hence could match.  We add these into the returned
     * string and mask, but the result isn't definitive; the caller has to
     * check further if its AND and compare pass.  But the failure of that
     * compare will quickly rule out most possible inputs.
     *
     * Much of this could be done in regcomp.c at compile time, except for
     * locale-dependent, and UTF-8 target dependent data.  Extra data fields
     * could be used for one or the other eventualities.
     *
     * If this function determines that no possible character in the target
     * string can match, it returns FALSE; otherwise TRUE.  (The FALSE
     * situation occurs if the first character in <text_node> requires UTF-8 to
     * represent, and the target string isn't in UTF-8.)
     *
     * Some analysis is in GH #18414, located at the time of this writing at:
     * https://github.com/Perl/perl5/issues/18414
     */

    const bool utf8_target = reginfo->is_utf8_target;
    bool utf8_pat = reginfo->is_utf8_pat;

    PERL_UINT_FAST8_T i;

    /* Here and below, '15' is the value of UTF8_MAXBYTES_CASE, which requires at least :e
     */
    U8 matches[MAX_MATCHES][UTF8_MAXBYTES_CASE + 1] = { { 0 } };
    U8 lengths[MAX_MATCHES] = { 0 };

    U8 index_of_longest = 0;

    U8 *pat = (U8*)STRING(text_node);
    Size_t pat_len = STR_LEN(text_node);
    U8 op = OP(text_node);

    U8 byte_mask[5]  = {0};
    U8 byte_anded[5] = {0};

    /* There are some folds in Unicode to multiple characters.  This will hold
     * such characters that could fold to the beginning of 'text_node' */
    UV multi_fold_from = 0;

    /* We may have to create a modified copy of the pattern */
    U8 mod_pat[UTF8_MAXBYTES_CASE + 1] = { '\0' };

    m->max_length = 0;
    m->min_length = 255;
    m->count = 0;

    /* Even if the first character in the node can match something in Latin1,
     * if there is anything in the node that can't, the match must fail */
    if (! utf8_target && isEXACT_REQ8(op)) {
        return FALSE;
    }

/* Define a temporary op for use in this function, using an existing one that
 * should never be a real op during execution */
#define TURKISH  PSEUDO

    /* What to do about these two nodes had to be deferred to runtime (which is
     * now).  If the extra information we now have so indicates, turn them into
     * EXACTFU nodes */
    if (   (op == EXACTF && utf8_target)
        || (op == EXACTFL && IN_UTF8_CTYPE_LOCALE))
    {
        if (op == EXACTFL && IN_UTF8_TURKIC_LOCALE) {
            op = TURKISH;
        }
        else {
            op = EXACTFU;
        }

        /* And certain situations are better handled if we create a modified
         * version of the pattern */
        if (utf8_pat) { /* Here, must have been EXACTFL, so look at the
                           specific problematic characters */
            if (is_PROBLEMATIC_LOCALE_FOLD_utf8(pat)) {

                /* The node could start with characters that are the first ones
                 * of a multi-character fold. */
                multi_fold_from
                          = what_MULTI_CHAR_FOLD_utf8_safe(pat, pat + pat_len);
                if (multi_fold_from) {

                    /* Here, they do form a sequence that matches the fold of a
                     * single character.  That single character then is a
                     * possible match.  Below we will look again at this, but
                     * the code below is expecting every character in the
                     * pattern to be folded, which the input isn't required to
                     * be in this case.  So, just fold the single character,
                     * and the result will be in the expected form. */
                    _to_uni_fold_flags(multi_fold_from, mod_pat, &pat_len,
                                       FOLD_FLAGS_FULL);
                    pat = mod_pat;
                }
                         /* Turkish has a couple extra possibilities. */
                else if (   UNLIKELY(op == TURKISH)
                         &&  pat_len >= 3
                         &&  isALPHA_FOLD_EQ(pat[0], 'f')
                         && (   memBEGINs(pat + 1, pat_len - 1,
                                    LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE_UTF8)
                             || (   pat_len >= 4
                                 && isALPHA_FOLD_EQ(pat[1], 'f')
                                 && memBEGINs(pat + 2, pat_len - 2,
                                    LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE_UTF8)
                ))) {
                    /* The macros for finding a multi-char fold don't include
                     * the Turkish possibilities, in which U+130 folds to 'i'.
                     * Hard-code these.  It's very unlikely that Unicode will
                     * ever add any others.  */
                    if (pat[1] == 'f') {
                        pat_len = 3;
                        Copy("ffi", mod_pat, pat_len, U8);
                    }
                    else {
                        pat_len = 2;
                        Copy("fi", mod_pat, pat_len, U8);
                    }
                    pat = mod_pat;
                }
                else if (    UTF8_IS_DOWNGRADEABLE_START(*pat)
                         &&  LIKELY(memNEs(pat, pat_len, MICRO_SIGN_UTF8))
                         &&  LIKELY(memNEs(pat, pat_len,
                                           LATIN_SMALL_LETTER_SHARP_S_UTF8))
                         && (LIKELY(op != TURKISH || *pat != 'I')))
                {
                    /* For all cases of things between 0-255, except the ones
                     * in the conditional above, the fold is just the lower
                     * case, which is faster than the more general case. */
                    mod_pat[0] = toLOWER_L1(EIGHT_BIT_UTF8_TO_NATIVE(pat[0],
                                                                     pat[1]));
                    pat_len = 1;
                    pat = mod_pat;
                    utf8_pat = FALSE;
                }
                else {  /* Code point above 255, or needs special handling */
                    _to_utf8_fold_flags(pat, pat + pat_len,
                                        mod_pat, &pat_len,
                                        FOLD_FLAGS_FULL|FOLD_FLAGS_LOCALE);
                    pat = mod_pat;
                }
            }
        }
        else if /* Below is not a UTF-8 pattern; there's a somewhat different
                   set of problematic characters */
                ((multi_fold_from
                          = what_MULTI_CHAR_FOLD_latin1_safe(pat, pat + pat_len)))
        {
            /* We may have to canonicalize a multi-char fold, as in the UTF-8
             * case */
            _to_uni_fold_flags(multi_fold_from, mod_pat, &pat_len,
                               FOLD_FLAGS_FULL);
            pat = mod_pat;
        }
        else if (UNLIKELY(*pat == LATIN_SMALL_LETTER_SHARP_S)) {
            mod_pat[0] = mod_pat[1] = 's';
            pat_len = 2;
            utf8_pat = utf8_target; /* UTF-8ness immaterial for invariant
                                       chars, and speeds copying */
            pat = mod_pat;
        }
        else if (LIKELY(op != TURKISH || *pat != 'I')) {
            mod_pat[0] = toLOWER_L1(*pat);
            pat_len = 1;
            pat = mod_pat;
        }
    }
    else if /* Below isn't a node that we convert to UTF-8 */
            (     utf8_target
             && ! utf8_pat
             &&   op == EXACTFAA_NO_TRIE
             &&  *pat == LATIN_SMALL_LETTER_SHARP_S)
    {
        /* A very special case.  Folding U+DF goes to U+17F under /iaa.  We
         * did this at compile time when the pattern was UTF-8 , but otherwise
         * we couldn't do it earlier, because it requires a UTF-8 target for
         * this match to be legal. */
        pat_len = 2 * (sizeof(LATIN_SMALL_LETTER_LONG_S_UTF8) - 1);
        Copy(LATIN_SMALL_LETTER_LONG_S_UTF8
             LATIN_SMALL_LETTER_LONG_S_UTF8, mod_pat, pat_len, U8);
        pat = mod_pat;
        utf8_pat = TRUE;
    }

    /* Here, we have taken care of the initial work for a few very problematic
     * situations, possibly creating a modified pattern.
     *
     * Now ready for the general case.  We build up all the possible things
     * that could match the first character of the pattern into the elements of
     * 'matches[]'
     *
     * Everything generally matches at least itself.  But if there is a
     * UTF8ness mismatch, we have to convert to that of the target string. */
    if (UTF8_IS_INVARIANT(*pat)) {  /* Immaterial if either is in UTF-8 */
        matches[0][0] = pat[0];
        lengths[0] = 1;
        m->count++;
    }
    else if (utf8_target) {
        if (utf8_pat) {
            lengths[0] = UTF8SKIP(pat);
            Copy(pat, matches[0], lengths[0], U8);
            m->count++;
        }
        else {  /* target is UTF-8, pattern isn't */
            matches[0][0] = UTF8_EIGHT_BIT_HI(pat[0]);
            matches[0][1] = UTF8_EIGHT_BIT_LO(pat[0]);
            lengths[0] = 2;
            m->count++;
        }
    }
    else if (! utf8_pat) {  /* Neither is UTF-8 */
        matches[0][0] = pat[0];
        lengths[0] = 1;
        m->count++;
    }
    else     /* target isn't UTF-8; pattern is.  No match possible unless the
                pattern's first character can fit in a byte */
         if (UTF8_IS_DOWNGRADEABLE_START(*pat))
    {
        matches[0][0] = EIGHT_BIT_UTF8_TO_NATIVE(pat[0], pat[1]);
        lengths[0] = 1;
        m->count++;
    }

    /* Here we have taken care of any necessary node-type changes */

    if (m->count) {
        m->max_length = lengths[0];
        m->min_length = lengths[0];
    }

    /* For non-folding nodes, there are no other possible candidate matches,
     * but for foldable ones, we have to look further. */
    if (UNLIKELY(op == TURKISH) || isEXACTFish(op)) { /* A folding node */
        UV folded;  /* The first character in the pattern, folded */
        U32 first_fold_from;    /* A character that folds to it */
        const U32 * remaining_fold_froms;   /* The remaining characters that
                                               fold to it, if any */
        Size_t folds_to_count;  /* The total number of characters that fold to
                                   'folded' */

        /* If the node begins with a sequence of more than one character that
         * together form the fold of a single character, it is called a
         * 'multi-character fold', and the normal functions don't handle this
         * case.  We set 'multi_fold_from' to the single folded-from character,
         * which is handled in an extra iteration below */
        if (utf8_pat) {
            folded = valid_utf8_to_uvchr(pat, NULL);
            multi_fold_from
                          = what_MULTI_CHAR_FOLD_utf8_safe(pat, pat + pat_len);
        }
        else {
            folded = *pat;

            /* This may generate illegal combinations for things like EXACTF,
             * but rather than repeat the logic and exclude them here, all such
             * illegalities are checked for and skipped below in the loop */
            multi_fold_from
                        = what_MULTI_CHAR_FOLD_latin1_safe(pat, pat + pat_len);
        }

        /* Everything matches at least itself; initialize to that because the
         * only the branches below that set it are the ones where the number
         * isn't 1. */
        folds_to_count = 1;

        /* There are a few special cases for locale-dependent nodes, where the
         * run-time context was needed before we could know what matched */
        if (UNLIKELY(op == EXACTFL) && folded < 256)  {
            first_fold_from = PL_fold_locale[folded];
        }
        else if (   op == EXACTFL && utf8_target && utf8_pat
                 && memBEGINs(pat, pat_len, LATIN_SMALL_LETTER_LONG_S_UTF8
                                            LATIN_SMALL_LETTER_LONG_S_UTF8))
        {
            first_fold_from = LATIN_CAPITAL_LETTER_SHARP_S;
        }
        else if (UNLIKELY(    op == TURKISH
                          && (   isALPHA_FOLD_EQ(folded, 'i')
                              || inRANGE(folded,
                                         LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE,
                                         LATIN_SMALL_LETTER_DOTLESS_I))))
        {   /* Turkish folding requires special handling */
            if (folded == 'i')
                first_fold_from = LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE;
            else if (folded == 'I')
                first_fold_from = LATIN_SMALL_LETTER_DOTLESS_I;
            else if (folded == LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE)
                first_fold_from = 'i';
            else first_fold_from = 'I';
        }
        else {
            /* Here, isn't a special case: use the generic function to
             * calculate what folds to this */
          redo_multi:
            /* Look up what code points (besides itself) fold to 'folded';
             * e.g., [ 'K', KELVIN_SIGN ] both fold to 'k'. */
            folds_to_count = _inverse_folds(folded, &first_fold_from,
                                                       &remaining_fold_froms);
        }

        /* Add each character that folds to 'folded' to the list of them,
         * subject to limitations based on the node type and target UTF8ness.
         * If there was a character that folded to multiple characters, do an
         * extra iteration for it.  (Note the extra iteration if there is a
         * multi-character fold) */
        for (i = 0; i < folds_to_count
                      + UNLIKELY(multi_fold_from != 0); i++)
        {
            UV fold_from = 0;

            if (i >= folds_to_count) {  /* Final iteration: handle the
                                           multi-char */
                fold_from = multi_fold_from;
            }
            else if (i == 0) {
                fold_from = first_fold_from;
            }
            else if (i < folds_to_count) {
                fold_from = remaining_fold_froms[i-1];
            }

            if (folded == fold_from) {  /* We already added the character
                                           itself */
                continue;
            }

            /* EXACTF doesn't have any non-ascii folds */
            if (op == EXACTF && (! isASCII(folded) || ! isASCII(fold_from))) {
                continue;
            }

            /* In /iaa nodes, neither or both must be ASCII to be a legal fold
             * */
            if (    isASCII(folded) != isASCII(fold_from)
                &&  inRANGE(op, EXACTFAA, EXACTFAA_NO_TRIE))

            {
                continue;
            }

            /* In /il nodes, can't cross 255/256 boundary (unless in a UTF-8
             * locale, but those have been converted to EXACTFU above) */
            if (   op == EXACTFL
                && (folded < 256) != (fold_from < 256))
            {
                continue;
            }

            /* If this triggers, it likely is because of the unlikely case
             * where a new Unicode standard has changed what MAX_MATCHES should
             * be set to */
            assert(m->count < MAX_MATCHES);

            /* Add this character to the list of possible matches */
            if (utf8_target) {
                uvchr_to_utf8(matches[(U8) m->count], fold_from);
                lengths[m->count] = UVCHR_SKIP(fold_from);
                m->count++;
            }
            else { /* Non-UTF8 target: no code point above 255 can appear in it
                    */
                if (fold_from > 255) {
                    continue;
                }

                matches[m->count][0] = fold_from;
                lengths[m->count] = 1;
                m->count++;
            }

            /* Update min and mlengths */
            if (m->min_length > lengths[m->count-1]) {
                m->min_length = lengths[m->count-1];
            }

            if (m->max_length < lengths[m->count-1]) {
                index_of_longest = m->count - 1;
                m->max_length = lengths[index_of_longest];
            }
        } /* looped through each potential fold */

        /* If there is something that folded to an initial multi-character
         * fold, repeat, using it.  This catches some edge cases.  An example
         * of one is /ss/i when UTF-8 encoded.  The function
         * what_MULTI_CHAR_FOLD_utf8_safe('ss') gets called and returns U+DF
         * (LATIN SMALL SHARP S).  If it returned a list of characters, this
         * code wouldn't be needed.  But since it doesn't, we have to look what
         * folds to the U+DF.  In this case, U+1E9E does, and has to be added.
         * */
        if (multi_fold_from) {
            folded = multi_fold_from;
            multi_fold_from = 0;
            goto redo_multi;
        }
    } /* End of finding things that participate in this fold */

    if (m->count == 0) {    /* If nothing found, can't match */
        m->min_length = 0;
        return FALSE;
    }

    /* Have calculated all possible matches.  Now calculate the mask and AND
     * values */
    m->initial_exact = 0;
    m->initial_definitive = 0;

    {
        unsigned int mask_ones = 0;
        unsigned int possible_ones = 0;
        U8 j;

        /* For each byte that is in all possible matches ... */
        for (j = 0; j < MIN(m->min_length, 5); j++) {

            /* Initialize the accumulator for this byte */
            byte_mask[j] = 0xFF;
            byte_anded[j] = matches[0][j];

            /* Then the rest of the rows (folds).  The mask is based on, like,
             * ~('A' ^ 'a') is a 1 in all bits where these are the same, and 0
             * where they differ. */
            for (i = 1; i < (PERL_UINT_FAST8_T) m->count; i++) {
                byte_mask[j]  &= ~ (byte_anded[j] ^ matches[i][j]);
                byte_anded[j] &= matches[i][j];
            }

            /* Keep track of the number of initial mask bytes that are all one
             * bits.  The code calling this can use this number to know that
             * a string that matches this number of bytes in the pattern is an
             * exact match of that pattern for this number of bytes.  But also
             * counted are the number of initial bytes that in total have a
             * single zero bit.  If a string matches those, masked, it must be
             * one of two possibilites, both of which this function has
             * determined are legal.  (But if that single 0 is one of the
             * initial bits for masking a UTF-8 start byte, that could
             * incorrectly lead to different length strings appearing to be
             * equivalent, so only do this optimization when the matchables are
             * all the same length.  This was uncovered by testing
             * /\x{029E}/i.) */
            if (m->min_length == m->max_length) {
                mask_ones += PL_bitcount[byte_mask[j]];
                possible_ones += 8;
                if (mask_ones + 1 >= possible_ones) {
                    m->initial_definitive++;
                    if (mask_ones >= possible_ones) {
                        m->initial_exact++;
                    }
                }
            }
        }
    }

    /* The first byte is separate for speed */
    m->first_byte_mask = byte_mask[0];
    m->first_byte_anded = byte_anded[0];

    /* Then pack up to the next 4 bytes into a word */
    m->mask32 = m->anded32 = 0;
    for (i = 1; i < MIN(m->min_length, 5); i++) {
        U8 which = i;
        U8 shift = (which - 1) * 8;
        m->mask32  |= (U32) byte_mask[i]  << shift;
        m->anded32 |= (U32) byte_anded[i] << shift;
    }

    /* Finally, take the match strings and place them sequentially into a
     * one-dimensional array.  (This is done to save significant space in the
     * structure.) Sort so the longest (presumably the least likely) is last.
     * XXX When this gets moved to regcomp, may want to fully sort shortest
     * first, but above we generally used the folded code point first, and
     * those tend to be no longer than their upper case values, so this is
     * already pretty well sorted by size.
     *
     * If the asserts fail, it's most likely because a new version of the
     * Unicode standard requires more space; simply increase the declaration
     * size. */
    {
        U8 cur_pos = 0;
        U8 output_index = 0;

        if (m->count > 1) { /* No need to sort a single entry */
            for (i = 0; i < (PERL_UINT_FAST8_T) m->count; i++) {

                /* Keep the same order for all but the longest.  (If the
                 * asserts fail, it could be because m->matches is declared too
                 * short, either because of a new Unicode release, or an
                 * overlooked test case, or it could be a bug.) */
                if (i != index_of_longest) {
                    assert(cur_pos + lengths[i] <= C_ARRAY_LENGTH(m->matches));
                    Copy(matches[i], m->matches + cur_pos, lengths[i], U8);
                    cur_pos += lengths[i];
                    m->lengths[output_index++] = lengths[i];
                }
            }
        }

        assert(cur_pos + lengths[index_of_longest] <= C_ARRAY_LENGTH(m->matches));
        Copy(matches[index_of_longest], m->matches + cur_pos,
             lengths[index_of_longest], U8);

        /* Place the longest match last */
        m->lengths[output_index] = lengths[index_of_longest];
    }


    return TRUE;
}

PERL_STATIC_FORCE_INLINE    /* We want speed at the expense of size */
bool
S_test_EXACTISH_ST(const char * loc,
                   struct next_matchable_info info)
{
    /* This function uses the data set up in setup_EXACTISH_ST() to see if the
     * bytes starting at 'loc' can match based on 'next_matchable_info' */

    U32 input32 = 0;

    /* Check the first byte */
    if (((U8) loc[0] & info.first_byte_mask) != info.first_byte_anded)
        return FALSE;

    /* Pack the next up-to-4 bytes into a 32 bit word */
    switch (info.min_length) {
        default:
            input32 |= (U32) ((U8) loc[4]) << 3 * 8;
            /* FALLTHROUGH */
        case 4:
            input32 |= (U8) loc[3] << 2 * 8;
            /* FALLTHROUGH */
        case 3:
            input32 |= (U8) loc[2] << 1 * 8;
            /* FALLTHROUGH */
        case 2:
            input32 |= (U8) loc[1];
            break;
        case 1:
            return TRUE;    /* We already tested and passed the 0th byte */
        case 0:
            ASSUME(0);
    }

    /* And AND that with the mask and compare that with the assembled ANDED
     * values */
    return (input32 & info.mask32) == info.anded32;
}

STATIC bool
S_isGCB(pTHX_ const GCB_enum before, const GCB_enum after, const U8 * const strbeg, const U8 * const curpos, const bool utf8_target)
{
    /* returns a boolean indicating if there is a Grapheme Cluster Boundary
     * between the inputs.  See https://www.unicode.org/reports/tr29/. */

    PERL_ARGS_ASSERT_ISGCB;

    switch (GCB_table[before][after]) {
        case GCB_BREAKABLE:
            return TRUE;

        case GCB_NOBREAK:
            return FALSE;

        case GCB_RI_then_RI:
            {
                int RI_count = 1;
                U8 * temp_pos = (U8 *) curpos;

                /* Do not break within emoji flag sequences. That is, do not
                 * break between regional indicator (RI) symbols if there is an
                 * odd number of RI characters before the break point.
                 *  GB12   sot (RI RI)* RI × RI
                 *  GB13 [^RI] (RI RI)* RI × RI */

                while (backup_one_GCB(strbeg,
                                    &temp_pos,
                                    utf8_target) == GCB_Regional_Indicator)
                {
                    RI_count++;
                }

                return RI_count % 2 != 1;
            }

        case GCB_EX_then_EM:

            /* GB10  ( E_Base | E_Base_GAZ ) Extend* ×  E_Modifier */
            {
                U8 * temp_pos = (U8 *) curpos;
                GCB_enum prev;

                do {
                    prev = backup_one_GCB(strbeg, &temp_pos, utf8_target);
                }
                while (prev == GCB_Extend);

                return prev != GCB_E_Base && prev != GCB_E_Base_GAZ;
            }

        case GCB_Maybe_Emoji_NonBreak:

            {

            /* Do not break within emoji modifier sequences or emoji zwj sequences.
              GB11 \p{Extended_Pictographic} Extend* ZWJ × \p{Extended_Pictographic}
              */
                U8 * temp_pos = (U8 *) curpos;
                GCB_enum prev;

                do {
                    prev = backup_one_GCB(strbeg, &temp_pos, utf8_target);
                }
                while (prev == GCB_Extend);

                return prev != GCB_ExtPict_XX;
            }

        default:
            break;
    }

#ifdef DEBUGGING
    Perl_re_printf( aTHX_  "Unhandled GCB pair: GCB_table[%d, %d] = %d\n",
                                  before, after, GCB_table[before][after]);
    assert(0);
#endif
    return TRUE;
}

STATIC GCB_enum
S_backup_one_GCB(pTHX_ const U8 * const strbeg, U8 ** curpos, const bool utf8_target)
{
    GCB_enum gcb;

    PERL_ARGS_ASSERT_BACKUP_ONE_GCB;

    if (*curpos < strbeg) {
        return GCB_EDGE;
    }

    if (utf8_target) {
        U8 * prev_char_pos = reghopmaybe3(*curpos, -1, strbeg);
        U8 * prev_prev_char_pos;

        if (! prev_char_pos) {
            return GCB_EDGE;
        }

        if ((prev_prev_char_pos = reghopmaybe3((U8 *) prev_char_pos, -1, strbeg))) {
            gcb = getGCB_VAL_UTF8(prev_prev_char_pos, prev_char_pos);
            *curpos = prev_char_pos;
            prev_char_pos = prev_prev_char_pos;
        }
        else {
            *curpos = (U8 *) strbeg;
            return GCB_EDGE;
        }
    }
    else {
        if (*curpos - 2 < strbeg) {
            *curpos = (U8 *) strbeg;
            return GCB_EDGE;
        }
        (*curpos)--;
        gcb = getGCB_VAL_CP(*(*curpos - 1));
    }

    return gcb;
}

/* Combining marks attach to most classes that precede them, but this defines
 * the exceptions (from TR14) */
#define LB_CM_ATTACHES_TO(prev) ( ! (   prev == LB_EDGE                 \
                                     || prev == LB_Mandatory_Break      \
                                     || prev == LB_Carriage_Return      \
                                     || prev == LB_Line_Feed            \
                                     || prev == LB_Next_Line            \
                                     || prev == LB_Space                \
                                     || prev == LB_ZWSpace))

STATIC bool
S_isLB(pTHX_ LB_enum before,
             LB_enum after,
             const U8 * const strbeg,
             const U8 * const curpos,
             const U8 * const strend,
             const bool utf8_target)
{
    U8 * temp_pos = (U8 *) curpos;
    LB_enum prev = before;

    /* Is the boundary between 'before' and 'after' line-breakable?
     * Most of this is just a table lookup of a generated table from Unicode
     * rules.  But some rules require context to decide, and so have to be
     * implemented in code */

    PERL_ARGS_ASSERT_ISLB;

    /* Rule numbers in the comments below are as of Unicode 9.0 */

  redo:
    before = prev;
    switch (LB_table[before][after]) {
        case LB_BREAKABLE:
            return TRUE;

        case LB_NOBREAK:
        case LB_NOBREAK_EVEN_WITH_SP_BETWEEN:
            return FALSE;

        case LB_SP_foo + LB_BREAKABLE:
        case LB_SP_foo + LB_NOBREAK:
        case LB_SP_foo + LB_NOBREAK_EVEN_WITH_SP_BETWEEN:

            /* When we have something following a SP, we have to look at the
             * context in order to know what to do.
             *
             * SP SP should not reach here because LB7: Do not break before
             * spaces.  (For two spaces in a row there is nothing that
             * overrides that) */
            assert(after != LB_Space);

            /* Here we have a space followed by a non-space.  Mostly this is a
             * case of LB18: "Break after spaces".  But there are complications
             * as the handling of spaces is somewhat tricky.  They are in a
             * number of rules, which have to be applied in priority order, but
             * something earlier in the string can cause a rule to be skipped
             * and a lower priority rule invoked.  A prime example is LB7 which
             * says don't break before a space.  But rule LB8 (lower priority)
             * says that the first break opportunity after a ZW is after any
             * span of spaces immediately after it.  If a ZW comes before a SP
             * in the input, rule LB8 applies, and not LB7.  Other such rules
             * involve combining marks which are rules 9 and 10, but they may
             * override higher priority rules if they come earlier in the
             * string.  Since we're doing random access into the middle of the
             * string, we have to look for rules that should get applied based
             * on both string position and priority.  Combining marks do not
             * attach to either ZW nor SP, so we don't have to consider them
             * until later.
             *
             * To check for LB8, we have to find the first non-space character
             * before this span of spaces */
            do {
                prev = backup_one_LB(strbeg, &temp_pos, utf8_target);
            }
            while (prev == LB_Space);

            /* LB8 Break before any character following a zero-width space,
             * even if one or more spaces intervene.
             *      ZW SP* ÷
             * So if we have a ZW just before this span, and to get here this
             * is the final space in the span. */
            if (prev == LB_ZWSpace) {
                return TRUE;
            }

            /* Here, not ZW SP+.  There are several rules that have higher
             * priority than LB18 and can be resolved now, as they don't depend
             * on anything earlier in the string (except ZW, which we have
             * already handled).  One of these rules is LB11 Do not break
             * before Word joiner, but we have specially encoded that in the
             * lookup table so it is caught by the single test below which
             * catches the other ones. */
            if (LB_table[LB_Space][after] - LB_SP_foo
                                            == LB_NOBREAK_EVEN_WITH_SP_BETWEEN)
            {
                return FALSE;
            }

            /* If we get here, we have to XXX consider combining marks. */
            if (prev == LB_Combining_Mark) {

                /* What happens with these depends on the character they
                 * follow.  */
                do {
                    prev = backup_one_LB(strbeg, &temp_pos, utf8_target);
                }
                while (prev == LB_Combining_Mark);

                /* Most times these attach to and inherit the characteristics
                 * of that character, but not always, and when not, they are to
                 * be treated as AL by rule LB10. */
                if (! LB_CM_ATTACHES_TO(prev)) {
                    prev = LB_Alphabetic;
                }
            }

            /* Here, we have the character preceding the span of spaces all set
             * up.  We follow LB18: "Break after spaces" unless the table shows
             * that is overridden */
            return LB_table[prev][after] != LB_NOBREAK_EVEN_WITH_SP_BETWEEN;

        case LB_CM_ZWJ_foo:

            /* We don't know how to treat the CM except by looking at the first
             * non-CM character preceding it.  ZWJ is treated as CM */
            do {
                prev = backup_one_LB(strbeg, &temp_pos, utf8_target);
            }
            while (prev == LB_Combining_Mark || prev == LB_ZWJ);

            /* Here, 'prev' is that first earlier non-CM character.  If the CM
             * attaches to it, then it inherits the behavior of 'prev'.  If it
             * doesn't attach, it is to be treated as an AL */
            if (! LB_CM_ATTACHES_TO(prev)) {
                prev = LB_Alphabetic;
            }

            goto redo;

        case LB_HY_or_BA_then_foo + LB_BREAKABLE:
        case LB_HY_or_BA_then_foo + LB_NOBREAK:

            /* LB21a Don't break after Hebrew + Hyphen.
             * HL (HY | BA) × */

            if (backup_one_LB(strbeg, &temp_pos, utf8_target)
                                                          == LB_Hebrew_Letter)
            {
                return FALSE;
            }

            return LB_table[prev][after] - LB_HY_or_BA_then_foo == LB_BREAKABLE;

        case LB_PR_or_PO_then_OP_or_HY + LB_BREAKABLE:
        case LB_PR_or_PO_then_OP_or_HY + LB_NOBREAK:

            /* LB25a (PR | PO) × ( OP | HY )? NU */
            if (advance_one_LB(&temp_pos, strend, utf8_target) == LB_Numeric) {
                return FALSE;
            }

            return LB_table[prev][after] - LB_PR_or_PO_then_OP_or_HY
                                                                == LB_BREAKABLE;

        case LB_SY_or_IS_then_various + LB_BREAKABLE:
        case LB_SY_or_IS_then_various + LB_NOBREAK:
        {
            /* LB25d NU (SY | IS)* × (NU | SY | IS | CL | CP ) */

            LB_enum temp = prev;
            do {
                temp = backup_one_LB(strbeg, &temp_pos, utf8_target);
            }
            while (temp == LB_Break_Symbols || temp == LB_Infix_Numeric);
            if (temp == LB_Numeric) {
                return FALSE;
            }

            return LB_table[prev][after] - LB_SY_or_IS_then_various
                                                               == LB_BREAKABLE;
        }

        case LB_various_then_PO_or_PR + LB_BREAKABLE:
        case LB_various_then_PO_or_PR + LB_NOBREAK:
        {
            /* LB25e NU (SY | IS)* (CL | CP)? × (PO | PR) */

            LB_enum temp = prev;
            if (temp == LB_Close_Punctuation || temp == LB_Close_Parenthesis)
            {
                temp = backup_one_LB(strbeg, &temp_pos, utf8_target);
            }
            while (temp == LB_Break_Symbols || temp == LB_Infix_Numeric) {
                temp = backup_one_LB(strbeg, &temp_pos, utf8_target);
            }
            if (temp == LB_Numeric) {
                return FALSE;
            }
            return LB_various_then_PO_or_PR;
        }

        case LB_RI_then_RI + LB_NOBREAK:
        case LB_RI_then_RI + LB_BREAKABLE:
            {
                int RI_count = 1;

                /* LB30a Break between two regional indicator symbols if and
                 * only if there are an even number of regional indicators
                 * preceding the position of the break.
                 *
                 *    sot (RI RI)* RI × RI
                 *  [^RI] (RI RI)* RI × RI */

                while (backup_one_LB(strbeg,
                                     &temp_pos,
                                     utf8_target) == LB_Regional_Indicator)
                {
                    RI_count++;
                }

                return RI_count % 2 == 0;
            }

        default:
            break;
    }

#ifdef DEBUGGING
    Perl_re_printf( aTHX_  "Unhandled LB pair: LB_table[%d, %d] = %d\n",
                                  before, after, LB_table[before][after]);
    assert(0);
#endif
    return TRUE;
}

STATIC LB_enum
S_advance_one_LB(pTHX_ U8 ** curpos, const U8 * const strend, const bool utf8_target)
{

    LB_enum lb;

    PERL_ARGS_ASSERT_ADVANCE_ONE_LB;

    if (*curpos >= strend) {
        return LB_EDGE;
    }

    if (utf8_target) {
        *curpos += UTF8SKIP(*curpos);
        if (*curpos >= strend) {
            return LB_EDGE;
        }
        lb = getLB_VAL_UTF8(*curpos, strend);
    }
    else {
        (*curpos)++;
        if (*curpos >= strend) {
            return LB_EDGE;
        }
        lb = getLB_VAL_CP(**curpos);
    }

    return lb;
}

STATIC LB_enum
S_backup_one_LB(pTHX_ const U8 * const strbeg, U8 ** curpos, const bool utf8_target)
{
    LB_enum lb;

    PERL_ARGS_ASSERT_BACKUP_ONE_LB;

    if (*curpos < strbeg) {
        return LB_EDGE;
    }

    if (utf8_target) {
        U8 * prev_char_pos = reghopmaybe3(*curpos, -1, strbeg);
        U8 * prev_prev_char_pos;

        if (! prev_char_pos) {
            return LB_EDGE;
        }

        if ((prev_prev_char_pos = reghopmaybe3((U8 *) prev_char_pos, -1, strbeg))) {
            lb = getLB_VAL_UTF8(prev_prev_char_pos, prev_char_pos);
            *curpos = prev_char_pos;
            prev_char_pos = prev_prev_char_pos;
        }
        else {
            *curpos = (U8 *) strbeg;
            return LB_EDGE;
        }
    }
    else {
        if (*curpos - 2 < strbeg) {
            *curpos = (U8 *) strbeg;
            return LB_EDGE;
        }
        (*curpos)--;
        lb = getLB_VAL_CP(*(*curpos - 1));
    }

    return lb;
}

STATIC bool
S_isSB(pTHX_ SB_enum before,
             SB_enum after,
             const U8 * const strbeg,
             const U8 * const curpos,
             const U8 * const strend,
             const bool utf8_target)
{
    /* returns a boolean indicating if there is a Sentence Boundary Break
     * between the inputs.  See https://www.unicode.org/reports/tr29/ */

    U8 * lpos = (U8 *) curpos;
    bool has_para_sep = FALSE;
    bool has_sp = FALSE;

    PERL_ARGS_ASSERT_ISSB;

    /* Break at the start and end of text.
        SB1.  sot  ÷
        SB2.  ÷  eot
      But unstated in Unicode is don't break if the text is empty */
    if (before == SB_EDGE || after == SB_EDGE) {
        return before != after;
    }

    /* SB 3: Do not break within CRLF. */
    if (before == SB_CR && after == SB_LF) {
        return FALSE;
    }

    /* Break after paragraph separators.  CR and LF are considered
     * so because Unicode views text as like word processing text where there
     * are no newlines except between paragraphs, and the word processor takes
     * care of wrapping without there being hard line-breaks in the text *./
       SB4.  Sep | CR | LF  ÷ */
    if (before == SB_Sep || before == SB_CR || before == SB_LF) {
        return TRUE;
    }

    /* Ignore Format and Extend characters, except after sot, Sep, CR, or LF.
     * (See Section 6.2, Replacing Ignore Rules.)
        SB5.  X (Extend | Format)*  →  X */
    if (after == SB_Extend || after == SB_Format) {

        /* Implied is that the these characters attach to everything
         * immediately prior to them except for those separator-type
         * characters.  And the rules earlier have already handled the case
         * when one of those immediately precedes the extend char */
        return FALSE;
    }

    if (before == SB_Extend || before == SB_Format) {
        U8 * temp_pos = lpos;
        const SB_enum backup = backup_one_SB(strbeg, &temp_pos, utf8_target);
        if (   backup != SB_EDGE
            && backup != SB_Sep
            && backup != SB_CR
            && backup != SB_LF)
        {
            before = backup;
            lpos = temp_pos;
        }

        /* Here, both 'before' and 'backup' are these types; implied is that we
         * don't break between them */
        if (backup == SB_Extend || backup == SB_Format) {
            return FALSE;
        }
    }

    /* Do not break after ambiguous terminators like period, if they are
     * immediately followed by a number or lowercase letter, if they are
     * between uppercase letters, if the first following letter (optionally
     * after certain punctuation) is lowercase, or if they are followed by
     * "continuation" punctuation such as comma, colon, or semicolon. For
     * example, a period may be an abbreviation or numeric period, and thus may
     * not mark the end of a sentence.

     * SB6. ATerm  ×  Numeric */
    if (before == SB_ATerm && after == SB_Numeric) {
        return FALSE;
    }

    /* SB7.  (Upper | Lower) ATerm  ×  Upper */
    if (before == SB_ATerm && after == SB_Upper) {
        U8 * temp_pos = lpos;
        SB_enum backup = backup_one_SB(strbeg, &temp_pos, utf8_target);
        if (backup == SB_Upper || backup == SB_Lower) {
            return FALSE;
        }
    }

    /* The remaining rules that aren't the final one, all require an STerm or
     * an ATerm after having backed up over some Close* Sp*, and in one case an
     * optional Paragraph separator, although one rule doesn't have any Sp's in it.
     * So do that backup now, setting flags if either Sp or a paragraph
     * separator are found */

    if (before == SB_Sep || before == SB_CR || before == SB_LF) {
        has_para_sep = TRUE;
        before = backup_one_SB(strbeg, &lpos, utf8_target);
    }

    if (before == SB_Sp) {
        has_sp = TRUE;
        do {
            before = backup_one_SB(strbeg, &lpos, utf8_target);
        }
        while (before == SB_Sp);
    }

    while (before == SB_Close) {
        before = backup_one_SB(strbeg, &lpos, utf8_target);
    }

    /* The next few rules apply only when the backed-up-to is an ATerm, and in
     * most cases an STerm */
    if (before == SB_STerm || before == SB_ATerm) {

        /* So, here the lhs matches
         *      (STerm | ATerm) Close* Sp* (Sep | CR | LF)?
         * and we have set flags if we found an Sp, or the optional Sep,CR,LF.
         * The rules that apply here are:
         *
         * SB8    ATerm Close* Sp*  ×  ( ¬(OLetter | Upper | Lower | Sep | CR
                                           | LF | STerm | ATerm) )* Lower
           SB8a  (STerm | ATerm) Close* Sp*  ×  (SContinue | STerm | ATerm)
           SB9   (STerm | ATerm) Close*  ×  (Close | Sp | Sep | CR | LF)
           SB10  (STerm | ATerm) Close* Sp*  ×  (Sp | Sep | CR | LF)
           SB11  (STerm | ATerm) Close* Sp* (Sep | CR | LF)?  ÷
         */

        /* And all but SB11 forbid having seen a paragraph separator */
        if (! has_para_sep) {
            if (before == SB_ATerm) {          /* SB8 */
                U8 * rpos = (U8 *) curpos;
                SB_enum later = after;

                while (    later != SB_OLetter
                        && later != SB_Upper
                        && later != SB_Lower
                        && later != SB_Sep
                        && later != SB_CR
                        && later != SB_LF
                        && later != SB_STerm
                        && later != SB_ATerm
                        && later != SB_EDGE)
                {
                    later = advance_one_SB(&rpos, strend, utf8_target);
                }
                if (later == SB_Lower) {
                    return FALSE;
                }
            }

            if (   after == SB_SContinue    /* SB8a */
                || after == SB_STerm
                || after == SB_ATerm)
            {
                return FALSE;
            }

            if (! has_sp) {     /* SB9 applies only if there was no Sp* */
                if (   after == SB_Close
                    || after == SB_Sp
                    || after == SB_Sep
                    || after == SB_CR
                    || after == SB_LF)
                {
                    return FALSE;
                }
            }

            /* SB10.  This and SB9 could probably be combined some way, but khw
             * has decided to follow the Unicode rule book precisely for
             * simplified maintenance */
            if (   after == SB_Sp
                || after == SB_Sep
                || after == SB_CR
                || after == SB_LF)
            {
                return FALSE;
            }
        }

        /* SB11.  */
        return TRUE;
    }

    /* Otherwise, do not break.
    SB12.  Any  ×  Any */

    return FALSE;
}

STATIC SB_enum
S_advance_one_SB(pTHX_ U8 ** curpos, const U8 * const strend, const bool utf8_target)
{
    SB_enum sb;

    PERL_ARGS_ASSERT_ADVANCE_ONE_SB;

    if (*curpos >= strend) {
        return SB_EDGE;
    }

    if (utf8_target) {
        do {
            *curpos += UTF8SKIP(*curpos);
            if (*curpos >= strend) {
                return SB_EDGE;
            }
            sb = getSB_VAL_UTF8(*curpos, strend);
        } while (sb == SB_Extend || sb == SB_Format);
    }
    else {
        do {
            (*curpos)++;
            if (*curpos >= strend) {
                return SB_EDGE;
            }
            sb = getSB_VAL_CP(**curpos);
        } while (sb == SB_Extend || sb == SB_Format);
    }

    return sb;
}

STATIC SB_enum
S_backup_one_SB(pTHX_ const U8 * const strbeg, U8 ** curpos, const bool utf8_target)
{
    SB_enum sb;

    PERL_ARGS_ASSERT_BACKUP_ONE_SB;

    if (*curpos < strbeg) {
        return SB_EDGE;
    }

    if (utf8_target) {
        U8 * prev_char_pos = reghopmaybe3(*curpos, -1, strbeg);
        if (! prev_char_pos) {
            return SB_EDGE;
        }

        /* Back up over Extend and Format.  curpos is always just to the right
         * of the character whose value we are getting */
        do {
            U8 * prev_prev_char_pos;
            if ((prev_prev_char_pos = reghopmaybe3((U8 *) prev_char_pos, -1,
                                                                      strbeg)))
            {
                sb = getSB_VAL_UTF8(prev_prev_char_pos, prev_char_pos);
                *curpos = prev_char_pos;
                prev_char_pos = prev_prev_char_pos;
            }
            else {
                *curpos = (U8 *) strbeg;
                return SB_EDGE;
            }
        } while (sb == SB_Extend || sb == SB_Format);
    }
    else {
        do {
            if (*curpos - 2 < strbeg) {
                *curpos = (U8 *) strbeg;
                return SB_EDGE;
            }
            (*curpos)--;
            sb = getSB_VAL_CP(*(*curpos - 1));
        } while (sb == SB_Extend || sb == SB_Format);
    }

    return sb;
}

STATIC bool
S_isWB(pTHX_ WB_enum previous,
             WB_enum before,
             WB_enum after,
             const U8 * const strbeg,
             const U8 * const curpos,
             const U8 * const strend,
             const bool utf8_target)
{
    /*  Return a boolean as to if the boundary between 'before' and 'after' is
     *  a Unicode word break, using their published algorithm, but tailored for
     *  Perl by treating spans of white space as one unit.  Context may be
     *  needed to make this determination.  If the value for the character
     *  before 'before' is known, it is passed as 'previous'; otherwise that
     *  should be set to WB_UNKNOWN.  The other input parameters give the
     *  boundaries and current position in the matching of the string.  That
     *  is, 'curpos' marks the position where the character whose wb value is
     *  'after' begins.  See http://www.unicode.org/reports/tr29/ */

    U8 * before_pos = (U8 *) curpos;
    U8 * after_pos = (U8 *) curpos;
    WB_enum prev = before;
    WB_enum next;

    PERL_ARGS_ASSERT_ISWB;

    /* Rule numbers in the comments below are as of Unicode 9.0 */

  redo:
    before = prev;
    switch (WB_table[before][after]) {
        case WB_BREAKABLE:
            return TRUE;

        case WB_NOBREAK:
            return FALSE;

        case WB_hs_then_hs:     /* 2 horizontal spaces in a row */
            next = advance_one_WB(&after_pos, strend, utf8_target,
                                 FALSE /* Don't skip Extend nor Format */ );
            /* A space immediately preceding an Extend or Format is attached
             * to by them, and hence gets separated from previous spaces.
             * Otherwise don't break between horizontal white space */
            return next == WB_Extend || next == WB_Format;

        /* WB4 Ignore Format and Extend characters, except when they appear at
         * the beginning of a region of text.  This code currently isn't
         * general purpose, but it works as the rules are currently and likely
         * to be laid out.  The reason it works is that when 'they appear at
         * the beginning of a region of text', the rule is to break before
         * them, just like any other character.  Therefore, the default rule
         * applies and we don't have to look in more depth.  Should this ever
         * change, we would have to have 2 'case' statements, like in the rules
         * below, and backup a single character (not spacing over the extend
         * ones) and then see if that is one of the region-end characters and
         * go from there */
        case WB_Ex_or_FO_or_ZWJ_then_foo:
            prev = backup_one_WB(&previous, strbeg, &before_pos, utf8_target);
            goto redo;

        case WB_DQ_then_HL + WB_BREAKABLE:
        case WB_DQ_then_HL + WB_NOBREAK:

            /* WB7c  Hebrew_Letter Double_Quote  ×  Hebrew_Letter */

            if (backup_one_WB(&previous, strbeg, &before_pos, utf8_target)
                                                            == WB_Hebrew_Letter)
            {
                return FALSE;
            }

             return WB_table[before][after] - WB_DQ_then_HL == WB_BREAKABLE;

        case WB_HL_then_DQ + WB_BREAKABLE:
        case WB_HL_then_DQ + WB_NOBREAK:

            /* WB7b  Hebrew_Letter  ×  Double_Quote Hebrew_Letter */

            if (advance_one_WB(&after_pos, strend, utf8_target,
                                       TRUE /* Do skip Extend and Format */ )
                                                            == WB_Hebrew_Letter)
            {
                return FALSE;
            }

            return WB_table[before][after] - WB_HL_then_DQ == WB_BREAKABLE;

        case WB_LE_or_HL_then_MB_or_ML_or_SQ + WB_NOBREAK:
        case WB_LE_or_HL_then_MB_or_ML_or_SQ + WB_BREAKABLE:

            /* WB6  (ALetter | Hebrew_Letter)  ×  (MidLetter | MidNumLet
             *       | Single_Quote) (ALetter | Hebrew_Letter) */

            next = advance_one_WB(&after_pos, strend, utf8_target,
                                       TRUE /* Do skip Extend and Format */ );

            if (next == WB_ALetter || next == WB_Hebrew_Letter)
            {
                return FALSE;
            }

            return WB_table[before][after]
                            - WB_LE_or_HL_then_MB_or_ML_or_SQ == WB_BREAKABLE;

        case WB_MB_or_ML_or_SQ_then_LE_or_HL + WB_NOBREAK:
        case WB_MB_or_ML_or_SQ_then_LE_or_HL + WB_BREAKABLE:

            /* WB7  (ALetter | Hebrew_Letter) (MidLetter | MidNumLet
             *       | Single_Quote)  ×  (ALetter | Hebrew_Letter) */

            prev = backup_one_WB(&previous, strbeg, &before_pos, utf8_target);
            if (prev == WB_ALetter || prev == WB_Hebrew_Letter)
            {
                return FALSE;
            }

            return WB_table[before][after]
                            - WB_MB_or_ML_or_SQ_then_LE_or_HL == WB_BREAKABLE;

        case WB_MB_or_MN_or_SQ_then_NU + WB_NOBREAK:
        case WB_MB_or_MN_or_SQ_then_NU + WB_BREAKABLE:

            /* WB11  Numeric (MidNum | (MidNumLet | Single_Quote))  ×  Numeric
             * */

            if (backup_one_WB(&previous, strbeg, &before_pos, utf8_target)
                                                            == WB_Numeric)
            {
                return FALSE;
            }

            return WB_table[before][after]
                                - WB_MB_or_MN_or_SQ_then_NU == WB_BREAKABLE;

        case WB_NU_then_MB_or_MN_or_SQ + WB_NOBREAK:
        case WB_NU_then_MB_or_MN_or_SQ + WB_BREAKABLE:

            /* WB12  Numeric  ×  (MidNum | MidNumLet | Single_Quote) Numeric */

            if (advance_one_WB(&after_pos, strend, utf8_target,
                                       TRUE /* Do skip Extend and Format */ )
                                                            == WB_Numeric)
            {
                return FALSE;
            }

            return WB_table[before][after]
                                - WB_NU_then_MB_or_MN_or_SQ == WB_BREAKABLE;

        case WB_RI_then_RI + WB_NOBREAK:
        case WB_RI_then_RI + WB_BREAKABLE:
            {
                int RI_count = 1;

                /* Do not break within emoji flag sequences. That is, do not
                 * break between regional indicator (RI) symbols if there is an
                 * odd number of RI characters before the potential break
                 * point.
                 *
                 * WB15   sot (RI RI)* RI × RI
                 * WB16 [^RI] (RI RI)* RI × RI */

                while (backup_one_WB(&previous,
                                     strbeg,
                                     &before_pos,
                                     utf8_target) == WB_Regional_Indicator)
                {
                    RI_count++;
                }

                return RI_count % 2 != 1;
            }

        default:
            break;
    }

#ifdef DEBUGGING
    Perl_re_printf( aTHX_  "Unhandled WB pair: WB_table[%d, %d] = %d\n",
                                  before, after, WB_table[before][after]);
    assert(0);
#endif
    return TRUE;
}

STATIC WB_enum
S_advance_one_WB(pTHX_ U8 ** curpos,
                       const U8 * const strend,
                       const bool utf8_target,
                       const bool skip_Extend_Format)
{
    WB_enum wb;

    PERL_ARGS_ASSERT_ADVANCE_ONE_WB;

    if (*curpos >= strend) {
        return WB_EDGE;
    }

    if (utf8_target) {

        /* Advance over Extend and Format */
        do {
            *curpos += UTF8SKIP(*curpos);
            if (*curpos >= strend) {
                return WB_EDGE;
            }
            wb = getWB_VAL_UTF8(*curpos, strend);
        } while (    skip_Extend_Format
                 && (wb == WB_Extend || wb == WB_Format));
    }
    else {
        do {
            (*curpos)++;
            if (*curpos >= strend) {
                return WB_EDGE;
            }
            wb = getWB_VAL_CP(**curpos);
        } while (    skip_Extend_Format
                 && (wb == WB_Extend || wb == WB_Format));
    }

    return wb;
}

STATIC WB_enum
S_backup_one_WB(pTHX_ WB_enum * previous, const U8 * const strbeg, U8 ** curpos, const bool utf8_target)
{
    WB_enum wb;

    PERL_ARGS_ASSERT_BACKUP_ONE_WB;

    /* If we know what the previous character's break value is, don't have
        * to look it up */
    if (*previous != WB_UNKNOWN) {
        wb = *previous;

        /* But we need to move backwards by one */
        if (utf8_target) {
            *curpos = reghopmaybe3(*curpos, -1, strbeg);
            if (! *curpos) {
                *previous = WB_EDGE;
                *curpos = (U8 *) strbeg;
            }
            else {
                *previous = WB_UNKNOWN;
            }
        }
        else {
            (*curpos)--;
            *previous = (*curpos <= strbeg) ? WB_EDGE : WB_UNKNOWN;
        }

        /* And we always back up over these three types */
        if (wb != WB_Extend && wb != WB_Format && wb != WB_ZWJ) {
            return wb;
        }
    }

    if (*curpos < strbeg) {
        return WB_EDGE;
    }

    if (utf8_target) {
        U8 * prev_char_pos = reghopmaybe3(*curpos, -1, strbeg);
        if (! prev_char_pos) {
            return WB_EDGE;
        }

        /* Back up over Extend and Format.  curpos is always just to the right
         * of the character whose value we are getting */
        do {
            U8 * prev_prev_char_pos;
            if ((prev_prev_char_pos = reghopmaybe3((U8 *) prev_char_pos,
                                                   -1,
                                                   strbeg)))
            {
                wb = getWB_VAL_UTF8(prev_prev_char_pos, prev_char_pos);
                *curpos = prev_char_pos;
                prev_char_pos = prev_prev_char_pos;
            }
            else {
                *curpos = (U8 *) strbeg;
                return WB_EDGE;
            }
        } while (wb == WB_Extend || wb == WB_Format || wb == WB_ZWJ);
    }
    else {
        do {
            if (*curpos - 2 < strbeg) {
                *curpos = (U8 *) strbeg;
                return WB_EDGE;
            }
            (*curpos)--;
            wb = getWB_VAL_CP(*(*curpos - 1));
        } while (wb == WB_Extend || wb == WB_Format);
    }

    return wb;
}

/* Macros for regmatch(), using its internal variables */
#define NEXTCHR_EOS -10 /* nextchr has fallen off the end */
#define NEXTCHR_IS_EOS (nextbyte < 0)

#define SET_nextchr \
    nextbyte = ((locinput < reginfo->strend) ? UCHARAT(locinput) : NEXTCHR_EOS)

#define SET_locinput(p) \
    locinput = (p);  \
    SET_nextchr

#define sayYES goto yes
#define sayNO goto no
#define sayNO_SILENT goto no_silent

/* we don't use STMT_START/END here because it leads to
   "unreachable code" warnings, which are bogus, but distracting. */
#define CACHEsayNO \
    if (ST.cache_mask) \
       reginfo->info_aux->poscache[ST.cache_offset] |= ST.cache_mask; \
    sayNO

#define EVAL_CLOSE_PAREN_IS(st,expr)                        \
(                                                           \
    (   ( st )                                         ) && \
    (   ( st )->u.eval.close_paren                     ) && \
    ( ( ( st )->u.eval.close_paren ) == ( (expr) + 1 ) )    \
)

#define EVAL_CLOSE_PAREN_IS_TRUE(st,expr)                   \
(                                                           \
    (   ( st )                                         ) && \
    (   ( st )->u.eval.close_paren                     ) && \
    (   ( expr )                                       ) && \
    ( ( ( st )->u.eval.close_paren ) == ( (expr) + 1 ) )    \
)


#define EVAL_CLOSE_PAREN_SET(st,expr) \
    (st)->u.eval.close_paren = ( (expr) + 1 )

#define EVAL_CLOSE_PAREN_CLEAR(st) \
    (st)->u.eval.close_paren = 0

/* push a new state then goto it */

#define PUSH_STATE_GOTO(state, node, input, eol, sr0)       \
    pushinput = input; \
    pusheol = eol; \
    pushsr0 = sr0; \
    scan = node; \
    st->resume_state = state; \
    goto push_state;

/* push a new state with success backtracking, then goto it */

#define PUSH_YES_STATE_GOTO(state, node, input, eol, sr0)   \
    pushinput = input; \
    pusheol = eol;     \
    pushsr0 = sr0; \
    scan = node; \
    st->resume_state = state; \
    goto push_yes_state;

#define DEBUG_STATE_pp(pp)                                  \
    DEBUG_STATE_r({                                         \
        DUMP_EXEC_POS(locinput, scan, utf8_target,depth);   \
        Perl_re_printf( aTHX_                               \
            "%*s" pp " %s%s%s%s%s\n",                       \
            INDENT_CHARS(depth), "",                        \
            REGNODE_NAME(st->resume_state),                  \
            ((st==yes_state||st==mark_state) ? "[" : ""),   \
            ((st==yes_state) ? "Y" : ""),                   \
            ((st==mark_state) ? "M" : ""),                  \
            ((st==yes_state||st==mark_state) ? "]" : "")    \
        );                                                  \
    });

/*

regmatch() - main matching routine

This is basically one big switch statement in a loop. We execute an op,
set 'next' to point the next op, and continue. If we come to a point which
we may need to backtrack to on failure such as (A|B|C), we push a
backtrack state onto the backtrack stack. On failure, we pop the top
state, and re-enter the loop at the state indicated. If there are no more
states to pop, we return failure.

Sometimes we also need to backtrack on success; for example /A+/, where
after successfully matching one A, we need to go back and try to
match another one; similarly for lookahead assertions: if the assertion
completes successfully, we backtrack to the state just before the assertion
and then carry on.  In these cases, the pushed state is marked as
'backtrack on success too'. This marking is in fact done by a chain of
pointers, each pointing to the previous 'yes' state. On success, we pop to
the nearest yes state, discarding any intermediate failure-only states.
Sometimes a yes state is pushed just to force some cleanup code to be
called at the end of a successful match or submatch; e.g. (??{$re}) uses
it to free the inner regex.

Note that failure backtracking rewinds the cursor position, while
success backtracking leaves it alone.

A pattern is complete when the END op is executed, while a subpattern
such as (?=foo) is complete when the SUCCESS op is executed. Both of these
ops trigger the "pop to last yes state if any, otherwise return true"
behaviour.

A common convention in this function is to use A and B to refer to the two
subpatterns (or to the first nodes thereof) in patterns like /A*B/: so A is
the subpattern to be matched possibly multiple times, while B is the entire
rest of the pattern. Variable and state names reflect this convention.

The states in the main switch are the union of ops and failure/success of
substates associated with that op.  For example, IFMATCH is the op
that does lookahead assertions /(?=A)B/ and so the IFMATCH state means
'execute IFMATCH'; while IFMATCH_A is a state saying that we have just
successfully matched A and IFMATCH_A_fail is a state saying that we have
just failed to match A. Resume states always come in pairs. The backtrack
state we push is marked as 'IFMATCH_A', but when that is popped, we resume
at IFMATCH_A or IFMATCH_A_fail, depending on whether we are backtracking
on success or failure.

The struct that holds a backtracking state is actually a big union, with
one variant for each major type of op. The variable st points to the
top-most backtrack struct. To make the code clearer, within each
block of code we #define ST to alias the relevant union.

Here's a concrete example of a (vastly oversimplified) IFMATCH
implementation:

    switch (state) {
    ....

#define ST st->u.ifmatch

    case IFMATCH: // we are executing the IFMATCH op, (?=A)B
        ST.foo = ...; // some state we wish to save
        ...
        // push a yes backtrack state with a resume value of
        // IFMATCH_A/IFMATCH_A_fail, then continue execution at the
        // first node of A:
        PUSH_YES_STATE_GOTO(IFMATCH_A, A, newinput);
        // NOTREACHED

    case IFMATCH_A: // we have successfully executed A; now continue with B
        next = B;
        bar = ST.foo; // do something with the preserved value
        break;

    case IFMATCH_A_fail: // A failed, so the assertion failed
        ...;   // do some housekeeping, then ...
        sayNO; // propagate the failure

#undef ST

    ...
    }

For any old-timers reading this who are familiar with the old recursive
approach, the code above is equivalent to:

    case IFMATCH: // we are executing the IFMATCH op, (?=A)B
    {
        int foo = ...
        ...
        if (regmatch(A)) {
            next = B;
            bar = foo;
            break;
        }
        ...;   // do some housekeeping, then ...
        sayNO; // propagate the failure
    }

The topmost backtrack state, pointed to by st, is usually free. If you
want to claim it, populate any ST.foo fields in it with values you wish to
save, then do one of

        PUSH_STATE_GOTO(resume_state, node, newinput, new_eol);
        PUSH_YES_STATE_GOTO(resume_state, node, newinput, new_eol);

which sets that backtrack state's resume value to 'resume_state', pushes a
new free entry to the top of the backtrack stack, then goes to 'node'.
On backtracking, the free slot is popped, and the saved state becomes the
new free state. An ST.foo field in this new top state can be temporarily
accessed to retrieve values, but once the main loop is re-entered, it
becomes available for reuse.

Note that the depth of the backtrack stack constantly increases during the
left-to-right execution of the pattern, rather than going up and down with
the pattern nesting. For example the stack is at its maximum at Z at the
end of the pattern, rather than at X in the following:

    /(((X)+)+)+....(Y)+....Z/

The only exceptions to this are lookahead/behind assertions and the cut,
(?>A), which pop all the backtrack states associated with A before
continuing.

Backtrack state structs are allocated in slabs of about 4K in size.
PL_regmatch_state and st always point to the currently active state,
and PL_regmatch_slab points to the slab currently containing
PL_regmatch_state.  The first time regmatch() is called, the first slab is
allocated, and is never freed until interpreter destruction. When the slab
is full, a new one is allocated and chained to the end. At exit from
regmatch(), slabs allocated since entry are freed.

In order to work with variable length lookbehinds, an upper limit is placed on
lookbehinds which is set to where the match position is at the end of where the
lookbehind would get to.  Nothing in the lookbehind should match above that,
except we should be able to look beyond if for things like \b, which need the
next character in the string to be able to determine if this is a boundary or
not.  We also can't match the end of string/line unless we are also at the end
of the entire string, so NEXTCHR_IS_EOS remains the same, and for those OPs
that match a width, we have to add a condition that they are within the legal
bounds of our window into the string.

*/

/* returns -1 on failure, $+[0] on success */
STATIC SSize_t
S_regmatch(pTHX_ regmatch_info *reginfo, char *startpos, regnode *prog)
{
    const bool utf8_target = reginfo->is_utf8_target;
    const U32 uniflags = UTF8_ALLOW_DEFAULT;
    REGEXP *rex_sv = reginfo->prog;
    regexp *rex = ReANY(rex_sv);
    RXi_GET_DECL(rex,rexi);
    /* the current state. This is a cached copy of PL_regmatch_state */
    regmatch_state *st;
    /* cache heavy used fields of st in registers */
    regnode *scan;
    regnode *next;
    U32 n = 0;	/* general value; init to avoid compiler warning */
    U32 utmp = 0;  /* tmp variable - valid for at most one opcode */
    SSize_t ln = 0; /* len or last;  init to avoid compiler warning */
    SSize_t endref = 0; /* offset of end of backref when ln is start */
    char *locinput = startpos;
    char *loceol = reginfo->strend;
    char *pushinput; /* where to continue after a PUSH */
    char *pusheol;   /* where to stop matching (loceol) after a PUSH */
    U8   *pushsr0;   /* save starting pos of script run */
    PERL_INT_FAST16_T nextbyte;   /* is always set to UCHARAT(locinput), or -1
                                     at EOS */

    bool result = 0;	    /* return value of S_regmatch */
    U32 depth = 0;            /* depth of backtrack stack */
    U32 nochange_depth = 0; /* depth of GOSUB recursion with nochange */
    const U32 max_nochange_depth =
        (3 * rex->nparens > MAX_RECURSE_EVAL_NOCHANGE_DEPTH) ?
        3 * rex->nparens : MAX_RECURSE_EVAL_NOCHANGE_DEPTH;
    regmatch_state *yes_state = NULL; /* state to pop to on success of
                                                            subpattern */
    /* mark_state piggy backs on the yes_state logic so that when we unwind
       the stack on success we can update the mark_state as we go */
    regmatch_state *mark_state = NULL; /* last mark state we have seen */
    regmatch_state *cur_eval = NULL; /* most recent EVAL_AB state */
    struct regmatch_state  *cur_curlyx = NULL; /* most recent curlyx */
    U32 state_num;
    bool no_final = 0;      /* prevent failure from backtracking? */
    bool do_cutgroup = 0;   /* no_final only until next branch/trie entry */
    char *startpoint = locinput;
    SV *popmark = NULL;     /* are we looking for a mark? */
    SV *sv_commit = NULL;   /* last mark name seen in failure */
    SV *sv_yes_mark = NULL; /* last mark name we have seen
                               during a successful match */
    U32 lastopen = 0;       /* last open we saw */
    bool has_cutgroup = RXp_HAS_CUTGROUP(rex) ? 1 : 0;
    SV* const oreplsv = GvSVn(PL_replgv);
    /* these three flags are set by various ops to signal information to
     * the very next op. They have a useful lifetime of exactly one loop
     * iteration, and are not preserved or restored by state pushes/pops
     */
    bool sw = 0;	    /* the condition value in (?(cond)a|b) */
    bool minmod = 0;	    /* the next "{n,m}" is a "{n,m}?" */
    int logical = 0;	    /* the following EVAL is:
                                0: (?{...})
                                1: (?(?{...})X|Y)
                                2: (??{...})
                               or the following IFMATCH/UNLESSM is:
                                false: plain (?=foo)
                                true:  used as a condition: (?(?=foo))
                            */
    PAD* last_pad = NULL;
    dMULTICALL;
    U8 gimme = G_SCALAR;
    CV *caller_cv = NULL;	/* who called us */
    CV *last_pushed_cv = NULL;	/* most recently called (?{}) CV */
    U32 maxopenparen = 0;       /* max '(' index seen so far */
    int to_complement;  /* Invert the result? */
    char_class_number_ classnum;
    bool is_utf8_pat = reginfo->is_utf8_pat;
    bool match = FALSE;
    I32 orig_savestack_ix = PL_savestack_ix;
    U8 * script_run_begin = NULL;
    char *match_end= NULL; /* where a match MUST end to be considered successful */
    bool is_accepted = FALSE; /* have we hit an ACCEPT opcode? */
    re_fold_t folder = NULL;  /* used by various EXACTish regops */
    const U8 * fold_array = NULL; /* used by various EXACTish regops */

/* Solaris Studio 12.3 messes up fetching PL_charclass['\n'] */
#if (defined(__SUNPRO_C) && (__SUNPRO_C == 0x5120) && defined(__x86_64) && defined(USE_64_BIT_ALL))
#  define SOLARIS_BAD_OPTIMIZER
    const U32 *pl_charclass_dup = PL_charclass;
#  define PL_charclass pl_charclass_dup
#endif

#ifdef DEBUGGING
    DECLARE_AND_GET_RE_DEBUG_FLAGS;
#endif

    /* protect against undef(*^R) */
    SAVEFREESV(SvREFCNT_inc_simple_NN(oreplsv));

    /* shut up 'may be used uninitialized' compiler warnings for dMULTICALL */
    multicall_oldcatch = 0;
    PERL_UNUSED_VAR(multicall_cop);

    PERL_ARGS_ASSERT_REGMATCH;

    st = PL_regmatch_state;

    /* Note that nextbyte is a byte even in UTF */
    SET_nextchr;
    scan = prog;

    DEBUG_OPTIMISE_r( DEBUG_EXECUTE_r({
            DUMP_EXEC_POS( locinput, scan, utf8_target, depth );
            Perl_re_printf( aTHX_ "regmatch start\n" );
    }));

    while (scan != NULL) {
        next = scan + NEXT_OFF(scan);
        if (next == scan)
            next = NULL;
        state_num = OP(scan);

      reenter_switch:
        DEBUG_EXECUTE_r(
            if (state_num <= REGNODE_MAX) {
                SV * const prop = sv_newmortal();
                regnode *rnext = regnext(scan);

                DUMP_EXEC_POS( locinput, scan, utf8_target, depth );
                regprop(rex, prop, scan, reginfo, NULL);
                Perl_re_printf( aTHX_
                    "%*s%" IVdf ":%s(%" IVdf ")\n",
                    INDENT_CHARS(depth), "",
                    (IV)(scan - rexi->program),
                    SvPVX_const(prop),
                    (REGNODE_TYPE(OP(scan)) == END || !rnext) ?
                        0 : (IV)(rnext - rexi->program));
            }
        );

        to_complement = 0;

        SET_nextchr;
        assert(nextbyte < 256 && (nextbyte >= 0 || nextbyte == NEXTCHR_EOS));

        switch (state_num) {
            SV * anyofh_list;

        case SBOL: /*  /^../ and /\A../  */
            if (locinput == reginfo->strbeg)
                break;
            sayNO;

        case MBOL: /*  /^../m  */
            if (locinput == reginfo->strbeg ||
                (!NEXTCHR_IS_EOS && locinput[-1] == '\n'))
            {
                break;
            }
            sayNO;

        case GPOS: /*  \G  */
            if (locinput == reginfo->ganch)
                break;
            sayNO;

        case KEEPS: /*   \K  */
            /* update the startpoint */
            st->u.keeper.val = RXp_OFFS_START(rex,0);
            RXp_OFFSp(rex)[0].start = locinput - reginfo->strbeg;
            PUSH_STATE_GOTO(KEEPS_next, next, locinput, loceol,
                            script_run_begin);
            NOT_REACHED; /* NOTREACHED */

        case KEEPS_next_fail:
            /* rollback the start point change */
            RXp_OFFSp(rex)[0].start = st->u.keeper.val;
            sayNO_SILENT;
            NOT_REACHED; /* NOTREACHED */

        case MEOL: /* /..$/m  */
            if (!NEXTCHR_IS_EOS && nextbyte != '\n')
                sayNO;
            break;

        case SEOL: /* /..$/  */
            if (!NEXTCHR_IS_EOS && nextbyte != '\n')
                sayNO;
            if (reginfo->strend - locinput > 1)
                sayNO;
            break;

        case EOS: /*  \z  */
            if (!NEXTCHR_IS_EOS)
                sayNO;
            break;

        case SANY: /*  /./s  */
            if (NEXTCHR_IS_EOS || locinput >= loceol)
                sayNO;
            goto increment_locinput;

        case REG_ANY: /*  /./  */
            if (   NEXTCHR_IS_EOS
                || locinput >= loceol
                || nextbyte == '\n')
            {
                sayNO;
            }
            goto increment_locinput;


#undef  ST
#define ST st->u.trie
        case TRIEC: /* (ab|cd) with known charclass */
            /* In this case the charclass data is available inline so
               we can fail fast without a lot of extra overhead.
             */
            if ( !   NEXTCHR_IS_EOS
                &&   locinput < loceol
                && ! ANYOF_BITMAP_TEST(scan, nextbyte))
            {
                DEBUG_EXECUTE_r(
                    Perl_re_exec_indentf( aTHX_  "%sTRIE: failed to match trie start class...%s\n",
                              depth, PL_colors[4], PL_colors[5])
                );
                sayNO_SILENT;
                NOT_REACHED; /* NOTREACHED */
            }
            /* FALLTHROUGH */
        case TRIE:  /* (ab|cd)  */
            /* the basic plan of execution of the trie is:
             * At the beginning, run though all the states, and
             * find the longest-matching word. Also remember the position
             * of the shortest matching word. For example, this pattern:
             *    1  2 3 4    5
             *    ab|a|x|abcd|abc
             * when matched against the string "abcde", will generate
             * accept states for all words except 3, with the longest
             * matching word being 4, and the shortest being 2 (with
             * the position being after char 1 of the string).
             *
             * Then for each matching word, in word order (i.e. 1,2,4,5),
             * we run the remainder of the pattern; on each try setting
             * the current position to the character following the word,
             * returning to try the next word on failure.
             *
             * We avoid having to build a list of words at runtime by
             * using a compile-time structure, wordinfo[].prev, which
             * gives, for each word, the previous accepting word (if any).
             * In the case above it would contain the mappings 1->2, 2->0,
             * 3->0, 4->5, 5->1.  We can use this table to generate, from
             * the longest word (4 above), a list of all words, by
             * following the list of prev pointers; this gives us the
             * unordered list 4,5,1,2. Then given the current word we have
             * just tried, we can go through the list and find the
             * next-biggest word to try (so if we just failed on word 2,
             * the next in the list is 4).
             *
             * Since at runtime we don't record the matching position in
             * the string for each word, we have to work that out for
             * each word we're about to process. The wordinfo table holds
             * the character length of each word; given that we recorded
             * at the start: the position of the shortest word and its
             * length in chars, we just need to move the pointer the
             * difference between the two char lengths. Depending on
             * Unicode status and folding, that's cheap or expensive.
             *
             * This algorithm is optimised for the case where are only a
             * small number of accept states, i.e. 0,1, or maybe 2.
             * With lots of accepts states, and having to try all of them,
             * it becomes quadratic on number of accept states to find all
             * the next words.
             */

            {
                /* what type of TRIE am I? (utf8 makes this contextual) */
                DECL_TRIE_TYPE(scan);

                /* what trie are we using right now */
                reg_trie_data * const trie
                    = (reg_trie_data*)rexi->data->data[ ARG1u( scan ) ];
                ST.before_paren = trie->before_paren;
                ST.after_paren = trie->after_paren;
                assert(ST.before_paren<=rex->nparens);
                assert(ST.after_paren<=rex->nparens);

                HV * widecharmap = MUTABLE_HV(rexi->data->data[ ARG1u( scan ) + 1 ]);
                U32 state = trie->startstate;

                if (FLAGS(scan) == EXACTL || FLAGS(scan) == EXACTFLU8) {
                    CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
                    if (utf8_target
                        && ! NEXTCHR_IS_EOS
                        && UTF8_IS_ABOVE_LATIN1(nextbyte)
                        && FLAGS(scan) == EXACTL)
                    {
                        /* We only output for EXACTL, as we let the folder
                         * output this message for EXACTFLU8 to avoid
                         * duplication */
                        _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(locinput,
                                                               reginfo->strend);
                    }
                }
                if (   trie->bitmap
                    && (     NEXTCHR_IS_EOS
                        ||   locinput >= loceol
                        || ! TRIE_BITMAP_TEST(trie, nextbyte)))
                {
                    if (trie->states[ state ].wordnum) {
                         DEBUG_EXECUTE_r(
                            Perl_re_exec_indentf( aTHX_  "%sTRIE: matched empty string...%s\n",
                                          depth, PL_colors[4], PL_colors[5])
                        );
                        if (!trie->jump)
                            break;
                    } else {
                        DEBUG_EXECUTE_r(
                            Perl_re_exec_indentf( aTHX_  "%sTRIE: failed to match trie start class...%s\n",
                                          depth, PL_colors[4], PL_colors[5])
                        );
                        sayNO_SILENT;
                   }
                }

            {
                U8 *uc = ( U8* )locinput;

                STRLEN len = 0;
                STRLEN foldlen = 0;
                U8 *uscan = (U8*)NULL;
                U8 foldbuf[ UTF8_MAXBYTES_CASE + 1 ];
                U32 charcount = 0; /* how many input chars we have matched */
                U32 accepted = 0; /* have we seen any accepting states? */

                ST.jump = trie->jump;
                ST.j_before_paren = trie->j_before_paren;
                ST.j_after_paren= trie->j_after_paren;
                ST.me = scan;
                ST.firstpos = NULL;
                ST.longfold = FALSE; /* char longer if folded => it's harder */
                ST.nextword = 0;

                /* fully traverse the TRIE; note the position of the
                   shortest accept state and the wordnum of the longest
                   accept state */

                while ( state && uc <= (U8*)(loceol) ) {
                    U32 base = trie->states[ state ].trans.base;
                    UV uvc = 0;
                    U16 charid = 0;
                    U16 wordnum;
                    wordnum = trie->states[ state ].wordnum;

                    if (wordnum) { /* it's an accept state */
                        if (!accepted) {
                            accepted = 1;
                            /* record first match position */
                            if (ST.longfold) {
                                ST.firstpos = (U8*)locinput;
                                ST.firstchars = 0;
                            }
                            else {
                                ST.firstpos = uc;
                                ST.firstchars = charcount;
                            }
                        }
                        if (!ST.nextword || wordnum < ST.nextword)
                            ST.nextword = wordnum;
                        ST.topword = wordnum;
                    }

                    DEBUG_TRIE_EXECUTE_r({
                                DUMP_EXEC_POS( (char *)uc, scan, utf8_target, depth );
                                /* HERE */
                                PerlIO_printf( Perl_debug_log,
                                    "%*s%sTRIE: State: %4" UVxf " Accepted: %c ",
                                    INDENT_CHARS(depth), "", PL_colors[4],
                                    (UV)state, (accepted ? 'Y' : 'N'));
                    });

                    /* read a char and goto next state */
                    if ( base && (foldlen || uc < (U8*)(loceol))) {
                        I32 offset;
                        REXEC_TRIE_READ_CHAR(trie_type, trie, widecharmap, uc,
                                             (U8 *) loceol, uscan,
                                             len, uvc, charid, foldlen,
                                             foldbuf, uniflags);
                        charcount++;
                        if (foldlen>0)
                            ST.longfold = TRUE;
                        if (charid &&
                             ( ((offset =
                              base + charid - 1 - trie->uniquecharcount)) >= 0)

                             && ((U32)offset < trie->lasttrans)
                             && trie->trans[offset].check == state)
                        {
                            state = trie->trans[offset].next;
                        }
                        else {
                            state = 0;
                        }
                        uc += len;

                    }
                    else {
                        state = 0;
                    }
                    DEBUG_TRIE_EXECUTE_r(
                        Perl_re_printf( aTHX_
                            "TRIE: Charid:%3x CP:%4" UVxf " After State: %4" UVxf "%s\n",
                            charid, uvc, (UV)state, PL_colors[5] );
                    );
                }
                if (!accepted)
                   sayNO;

                /* calculate total number of accept states */
                {
                    U16 w = ST.topword;
                    accepted = 0;
                    while (w) {
                        w = trie->wordinfo[w].prev;
                        accepted++;
                    }
                    ST.accepted = accepted;
                }

                DEBUG_EXECUTE_r(
                    Perl_re_exec_indentf( aTHX_  "%sTRIE: got %" IVdf " possible matches%s\n",
                        depth,
                        PL_colors[4], (IV)ST.accepted, PL_colors[5] );
                );
                goto trie_first_try; /* jump into the fail handler */
            }}
            NOT_REACHED; /* NOTREACHED */

        case TRIE_next_fail: /* we failed - try next alternative */
        {
            U8 *uc;
            if (RE_PESSIMISTIC_PARENS) {
                REGCP_UNWIND(ST.lastcp);
                regcppop(rex,&maxopenparen);
            }
            if ( ST.jump ) {
                /* undo any captures done in the tail part of a branch,
                 * e.g.
                 *    /(?:X(.)(.)|Y(.)).../
                 * where the trie just matches X then calls out to do the
                 * rest of the branch */
                REGCP_UNWIND(ST.cp);
                UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
                if (ST.after_paren) {
                    assert(ST.before_paren<=rex->nparens && ST.after_paren<=rex->nparens);
                    CAPTURE_CLEAR(ST.before_paren+1, ST.after_paren, "TRIE_next_fail");
                }
            }
            if (!--ST.accepted) {
                DEBUG_EXECUTE_r({
                    Perl_re_exec_indentf( aTHX_  "%sTRIE failed...%s\n",
                        depth,
                        PL_colors[4],
                        PL_colors[5] );
                });
                sayNO_SILENT;
            }
            {
                /* Find next-highest word to process.  Note that this code
                 * is O(N^2) per trie run (O(N) per branch), so keep tight */
                U16 min = 0;
                U16 word;
                U16 const nextword = ST.nextword;
                reg_trie_wordinfo * const wordinfo
                    = ((reg_trie_data*)rexi->data->data[ARG1u(ST.me)])->wordinfo;
                for (word=ST.topword; word; word=wordinfo[word].prev) {
                    if (word > nextword && (!min || word < min))
                        min = word;
                }
                ST.nextword = min;
            }

          trie_first_try:
            if (do_cutgroup) {
                do_cutgroup = 0;
                no_final = 0;
            }

            if ( ST.jump ) {
                ST.lastparen = RXp_LASTPAREN(rex);
                ST.lastcloseparen = RXp_LASTCLOSEPAREN(rex);
                REGCP_SET(ST.cp);
            }

            /* find start char of end of current word */
            {
                U32 chars; /* how many chars to skip */
                reg_trie_data * const trie
                    = (reg_trie_data*)rexi->data->data[ARG1u(ST.me)];

                assert((trie->wordinfo[ST.nextword].len - trie->prefixlen)
                            >=  ST.firstchars);
                chars = (trie->wordinfo[ST.nextword].len - trie->prefixlen)
                            - ST.firstchars;
                uc = ST.firstpos;

                if (ST.longfold) {
                    /* the hard option - fold each char in turn and find
                     * its folded length (which may be different */
                    U8 foldbuf[UTF8_MAXBYTES_CASE + 1];
                    STRLEN foldlen;
                    STRLEN len;
                    UV uvc;
                    U8 *uscan;

                    while (chars) {
                        if (utf8_target) {
                            /* XXX This assumes the length is well-formed, as
                             * does the UTF8SKIP below */
                            uvc = utf8n_to_uvchr((U8*)uc, UTF8_MAXLEN, &len,
                                                    uniflags);
                            uc += len;
                        }
                        else {
                            uvc = *uc;
                            uc++;
                        }
                        uvc = to_uni_fold(uvc, foldbuf, &foldlen);
                        uscan = foldbuf;
                        while (foldlen) {
                            if (!--chars)
                                break;
                            uvc = utf8n_to_uvchr(uscan, foldlen, &len,
                                                 uniflags);
                            uscan += len;
                            foldlen -= len;
                        }
                    }
                }
                else {
                    if (utf8_target)
                        uc = utf8_hop(uc, chars);
                    else
                        uc += chars;
                }
            }
            if (ST.jump && ST.jump[ST.nextword]) {
                scan = ST.me + ST.jump[ST.nextword];
                ST.before_paren = ST.j_before_paren[ST.nextword];
                assert(ST.before_paren <= rex->nparens);
                ST.after_paren = ST.j_after_paren[ST.nextword];
                assert(ST.after_paren <= rex->nparens);
            } else {
                scan = ST.me + NEXT_OFF(ST.me);
            }


            DEBUG_EXECUTE_r({
                Perl_re_exec_indentf( aTHX_  "%sTRIE matched word #%d, continuing%s\n",
                    depth,
                    PL_colors[4],
                    ST.nextword,
                    PL_colors[5]
                    );
            });

            if ( ST.accepted > 1 || has_cutgroup || ST.jump ) {
                if (RE_PESSIMISTIC_PARENS) {
                    (void)regcppush(rex, 0, maxopenparen);
                    REGCP_SET(ST.lastcp);
                }
                PUSH_STATE_GOTO(TRIE_next, scan, (char*)uc, loceol,
                                script_run_begin);
                NOT_REACHED; /* NOTREACHED */
            }
            /* only one choice left - just continue */
            DEBUG_EXECUTE_r({
                AV *const trie_words
                    = MUTABLE_AV(rexi->data->data[ARG1u(ST.me)+TRIE_WORDS_OFFSET]);
                SV ** const tmp = trie_words
                        ? av_fetch(trie_words, ST.nextword - 1, 0) : NULL;
                SV *sv= tmp ? sv_newmortal() : NULL;

                Perl_re_exec_indentf( aTHX_  "%sTRIE: only one match left, short-circuiting: #%d <%s>%s\n",
                    depth, PL_colors[4],
                    ST.nextword,
                    tmp ? pv_pretty(sv, SvPV_nolen_const(*tmp), SvCUR(*tmp), 0,
                            PL_colors[0], PL_colors[1],
                            (SvUTF8(*tmp) ? PERL_PV_ESCAPE_UNI : 0)|PERL_PV_ESCAPE_NONASCII
                        )
                    : "not compiled under -Dr",
                    PL_colors[5] );
            });

            locinput = (char*)uc;
            continue; /* execute rest of RE */
            /* NOTREACHED */
        }
#undef  ST

        case LEXACT_REQ8:
            if (! utf8_target) {
                sayNO;
            }
            /* FALLTHROUGH */

        case LEXACT:
        {
            char *s;

            s = STRINGl(scan);
            ln = STR_LENl(scan);
            goto join_short_long_exact;

        case EXACTL:             /*  /abc/l       */
            CHECK_AND_WARN_PROBLEMATIC_LOCALE_;

            /* Complete checking would involve going through every character
             * matched by the string to see if any is above latin1.  But the
             * comparison otherwise might very well be a fast assembly
             * language routine, and I (khw) don't think slowing things down
             * just to check for this warning is worth it.  So this just checks
             * the first character */
            if (utf8_target && UTF8_IS_ABOVE_LATIN1(*locinput)) {
                _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(locinput, reginfo->strend);
            }
            goto do_exact;
        case EXACT_REQ8:
            if (! utf8_target) {
                sayNO;
            }
            /* FALLTHROUGH */

        case EXACT:             /*  /abc/        */
          do_exact:
            s = STRINGs(scan);
            ln = STR_LENs(scan);

          join_short_long_exact:
            if (utf8_target != is_utf8_pat) {
                /* The target and the pattern have differing utf8ness. */
                char *l = locinput;
                const char * const e = s + ln;

                if (utf8_target) {
                    /* The target is utf8, the pattern is not utf8.
                     * Above-Latin1 code points can't match the pattern;
                     * invariants match exactly, and the other Latin1 ones need
                     * to be downgraded to a single byte in order to do the
                     * comparison.  (If we could be confident that the target
                     * is not malformed, this could be refactored to have fewer
                     * tests by just assuming that if the first bytes match, it
                     * is an invariant, but there are tests in the test suite
                     * dealing with (??{...}) which violate this) */
                    while (s < e) {
                        if (   l >= loceol
                            || UTF8_IS_ABOVE_LATIN1(* (U8*) l))
                        {
                            sayNO;
                        }
                        if (UTF8_IS_INVARIANT(*(U8*)l)) {
                            if (*l != *s) {
                                sayNO;
                            }
                            l++;
                        }
                        else {
                            if (EIGHT_BIT_UTF8_TO_NATIVE(*l, *(l+1)) != * (U8*) s)
                            {
                                sayNO;
                            }
                            l += 2;
                        }
                        s++;
                    }
                }
                else {
                    /* The target is not utf8, the pattern is utf8. */
                    while (s < e) {
                        if (   l >= loceol
                            || UTF8_IS_ABOVE_LATIN1(* (U8*) s))
                        {
                            sayNO;
                        }
                        if (UTF8_IS_INVARIANT(*(U8*)s)) {
                            if (*s != *l) {
                                sayNO;
                            }
                            s++;
                        }
                        else {
                            if (EIGHT_BIT_UTF8_TO_NATIVE(*s, *(s+1)) != * (U8*) l)
                            {
                                sayNO;
                            }
                            s += 2;
                        }
                        l++;
                    }
                }
                locinput = l;
            }
            else {
                /* The target and the pattern have the same utf8ness. */
                /* Inline the first character, for speed. */
                if (   loceol - locinput < ln
                    || UCHARAT(s) != nextbyte
                    || (ln > 1 && memNE(s, locinput, ln)))
                {
                    sayNO;
                }
                locinput += ln;
            }
            break;
            }

        case EXACTFL:            /*  /abc/il      */
          {
            const char * s;
            U32 fold_utf8_flags;

            CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
            folder = Perl_foldEQ_locale;
            fold_array = PL_fold_locale;
            fold_utf8_flags = FOLDEQ_LOCALE;
            goto do_exactf;

        case EXACTFLU8:           /*  /abc/il; but all 'abc' are above 255, so
                                      is effectively /u; hence to match, target
                                      must be UTF-8. */
            if (! utf8_target) {
                sayNO;
            }
            fold_utf8_flags =  FOLDEQ_LOCALE | FOLDEQ_S2_ALREADY_FOLDED
                                             | FOLDEQ_S2_FOLDS_SANE;
            folder = S_foldEQ_latin1_s2_folded;
            fold_array = PL_fold_latin1;
            goto do_exactf;

        case EXACTFU_REQ8:      /* /abc/iu with something in /abc/ > 255 */
            if (! utf8_target) {
                sayNO;
            }
            assert(is_utf8_pat);
            fold_utf8_flags = FOLDEQ_S2_ALREADY_FOLDED;
#ifdef DEBUGGING
            /* this is only used in an assert check, so we restrict it to DEBUGGING mode.
             * In theory neither of these variables should be used in this mode. */
            folder = NULL;
            fold_array = NULL;
#endif
            goto do_exactf;

        case EXACTFUP:          /*  /foo/iu, and something is problematic in
                                    'foo' so can't take shortcuts. */
            assert(! is_utf8_pat);
            folder = Perl_foldEQ_latin1;
            fold_array = PL_fold_latin1;
            fold_utf8_flags = 0;
            goto do_exactf;

        case EXACTFU:            /*  /abc/iu      */
            folder = S_foldEQ_latin1_s2_folded;
            fold_array = PL_fold_latin1;
            fold_utf8_flags = FOLDEQ_S2_ALREADY_FOLDED;
            goto do_exactf;

        case EXACTFAA_NO_TRIE:   /* This node only generated for non-utf8
                                   patterns */
            assert(! is_utf8_pat);
            /* FALLTHROUGH */
        case EXACTFAA:            /*  /abc/iaa     */
            folder = S_foldEQ_latin1_s2_folded;
            fold_array = PL_fold_latin1;
            fold_utf8_flags = FOLDEQ_UTF8_NOMIX_ASCII;
            if (is_utf8_pat || ! utf8_target) {

                /* The possible presence of a MICRO SIGN in the pattern forbids
                 * us to view a non-UTF-8 pattern as folded when there is a
                 * UTF-8 target */
                fold_utf8_flags |= FOLDEQ_S2_ALREADY_FOLDED
                                  |FOLDEQ_S2_FOLDS_SANE;
            }
            goto do_exactf;


        case EXACTF:             /*  /abc/i    This node only generated for
                                               non-utf8 patterns */
            assert(! is_utf8_pat);
            folder = Perl_foldEQ;
            fold_array = PL_fold;
            fold_utf8_flags = 0;

          do_exactf:
            s = STRINGs(scan);
            ln = STR_LENs(scan);

            if (   utf8_target
                || is_utf8_pat
                || state_num == EXACTFUP
                || (state_num == EXACTFL && IN_UTF8_CTYPE_LOCALE))
            {
              /* Either target or the pattern are utf8, or has the issue where
               * the fold lengths may differ. */
                const char * const l = locinput;
                char *e = loceol;

                if (! foldEQ_utf8_flags(l, &e, 0,  utf8_target,
                                        s, 0,  ln, is_utf8_pat,fold_utf8_flags))
                {
                    sayNO;
                }
                locinput = e;
                break;
            }

            /* Neither the target nor the pattern are utf8 */
            assert(fold_array);
            if (UCHARAT(s) != nextbyte
                && !NEXTCHR_IS_EOS
                && UCHARAT(s) != fold_array[nextbyte])
            {
                sayNO;
            }
            if (loceol - locinput < ln)
                sayNO;
            assert(folder);
            if (ln > 1 && ! folder(aTHX_ locinput, s, ln))
                sayNO;
            locinput += ln;
            break;
        }

        case NBOUNDL: /*  /\B/l  */
            to_complement = 1;
            /* FALLTHROUGH */

        case BOUNDL:  /*  /\b/l  */
        {
            bool b1, b2;
            CHECK_AND_WARN_PROBLEMATIC_LOCALE_;

            if (FLAGS(scan) != TRADITIONAL_BOUND) {
                CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_BOUND;
                goto boundu;
            }

            if (utf8_target) {
                if (locinput == reginfo->strbeg)
                    b1 = isWORDCHAR_LC('\n');
                else {
                    U8 *p = reghop3((U8*)locinput, -1,
                                    (U8*)(reginfo->strbeg));
                    b1 = isWORDCHAR_LC_utf8_safe(p, (U8*)(reginfo->strend));
                }
                b2 = (NEXTCHR_IS_EOS)
                    ? isWORDCHAR_LC('\n')
                    : isWORDCHAR_LC_utf8_safe((U8*) locinput,
                                              (U8*) reginfo->strend);
            }
            else { /* Here the string isn't utf8 */
                b1 = (locinput == reginfo->strbeg)
                     ? isWORDCHAR_LC('\n')
                     : isWORDCHAR_LC(UCHARAT(locinput - 1));
                b2 = (NEXTCHR_IS_EOS)
                    ? isWORDCHAR_LC('\n')
                    : isWORDCHAR_LC(nextbyte);
            }
            if (to_complement ^ (b1 == b2)) {
                sayNO;
            }
            break;
        }

        case NBOUND:  /*  /\B/   */
            to_complement = 1;
            /* FALLTHROUGH */

        case BOUND:   /*  /\b/   */
            if (utf8_target) {
                goto bound_utf8;
            }
            goto bound_ascii_match_only;

        case NBOUNDA: /*  /\B/a  */
            to_complement = 1;
            /* FALLTHROUGH */

        case BOUNDA:  /*  /\b/a  */
        {
            bool b1, b2;

          bound_ascii_match_only:
            /* Here the string isn't utf8, or is utf8 and only ascii characters
             * are to match \w.  In the latter case looking at the byte just
             * prior to the current one may be just the final byte of a
             * multi-byte character.  This is ok.  There are two cases:
             * 1) it is a single byte character, and then the test is doing
             *    just what it's supposed to.
             * 2) it is a multi-byte character, in which case the final byte is
             *    never mistakable for ASCII, and so the test will say it is
             *    not a word character, which is the correct answer. */
            b1 = (locinput == reginfo->strbeg)
                 ? isWORDCHAR_A('\n')
                 : isWORDCHAR_A(UCHARAT(locinput - 1));
            b2 = (NEXTCHR_IS_EOS)
                ? isWORDCHAR_A('\n')
                : isWORDCHAR_A(nextbyte);
            if (to_complement ^ (b1 == b2)) {
                sayNO;
            }
            break;
        }

        case NBOUNDU: /*  /\B/u  */
            to_complement = 1;
            /* FALLTHROUGH */

        case BOUNDU:  /*  /\b/u  */

          boundu:
            if (UNLIKELY(reginfo->strbeg >= reginfo->strend)) {
                match = FALSE;
            }
            else if (utf8_target) {
              bound_utf8:
                switch((bound_type) FLAGS(scan)) {
                    case TRADITIONAL_BOUND:
                    {
                        bool b1, b2;
                        if (locinput == reginfo->strbeg) {
                            b1 = 0 /* isWORDCHAR_L1('\n') */;
                        }
                        else {
                            U8 *p = reghop3((U8*)locinput, -1,
                                            (U8*)(reginfo->strbeg));

                            b1 = isWORDCHAR_utf8_safe(p, (U8*) reginfo->strend);
                        }
                        b2 = (NEXTCHR_IS_EOS)
                            ? 0 /* isWORDCHAR_L1('\n') */
                            : isWORDCHAR_utf8_safe((U8*)locinput,
                                                   (U8*) reginfo->strend);
                        match = cBOOL(b1 != b2);
                        break;
                    }
                    case GCB_BOUND:
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE; /* GCB always matches at begin and
                                             end */
                        }
                        else {
                            /* Find the gcb values of previous and current
                             * chars, then see if is a break point */
                            match = isGCB(getGCB_VAL_UTF8(
                                                reghop3((U8*)locinput,
                                                        -1,
                                                        (U8*)(reginfo->strbeg)),
                                                (U8*) reginfo->strend),
                                          getGCB_VAL_UTF8((U8*) locinput,
                                                        (U8*) reginfo->strend),
                                          (U8*) reginfo->strbeg,
                                          (U8*) locinput,
                                          utf8_target);
                        }
                        break;

                    case LB_BOUND:
                        if (locinput == reginfo->strbeg) {
                            match = FALSE;
                        }
                        else if (NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isLB(getLB_VAL_UTF8(
                                                reghop3((U8*)locinput,
                                                        -1,
                                                        (U8*)(reginfo->strbeg)),
                                                (U8*) reginfo->strend),
                                          getLB_VAL_UTF8((U8*) locinput,
                                                        (U8*) reginfo->strend),
                                          (U8*) reginfo->strbeg,
                                          (U8*) locinput,
                                          (U8*) reginfo->strend,
                                          utf8_target);
                        }
                        break;

                    case SB_BOUND: /* Always matches at begin and end */
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isSB(getSB_VAL_UTF8(
                                                reghop3((U8*)locinput,
                                                        -1,
                                                        (U8*)(reginfo->strbeg)),
                                                (U8*) reginfo->strend),
                                          getSB_VAL_UTF8((U8*) locinput,
                                                        (U8*) reginfo->strend),
                                          (U8*) reginfo->strbeg,
                                          (U8*) locinput,
                                          (U8*) reginfo->strend,
                                          utf8_target);
                        }
                        break;

                    case WB_BOUND:
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isWB(WB_UNKNOWN,
                                         getWB_VAL_UTF8(
                                                reghop3((U8*)locinput,
                                                        -1,
                                                        (U8*)(reginfo->strbeg)),
                                                (U8*) reginfo->strend),
                                          getWB_VAL_UTF8((U8*) locinput,
                                                        (U8*) reginfo->strend),
                                          (U8*) reginfo->strbeg,
                                          (U8*) locinput,
                                          (U8*) reginfo->strend,
                                          utf8_target);
                        }
                        break;
                }
            }
            else {  /* Not utf8 target */
                switch((bound_type) FLAGS(scan)) {
                    case TRADITIONAL_BOUND:
                    {
                        bool b1, b2;
                        b1 = (locinput == reginfo->strbeg)
                            ? 0 /* isWORDCHAR_L1('\n') */
                            : isWORDCHAR_L1(UCHARAT(locinput - 1));
                        b2 = (NEXTCHR_IS_EOS)
                            ? 0 /* isWORDCHAR_L1('\n') */
                            : isWORDCHAR_L1(nextbyte);
                        match = cBOOL(b1 != b2);
                        break;
                    }

                    case GCB_BOUND:
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE; /* GCB always matches at begin and
                                             end */
                        }
                        else {  /* Only CR-LF combo isn't a GCB in 0-255
                                   range */
                            match =    UCHARAT(locinput - 1) != '\r'
                                    || UCHARAT(locinput) != '\n';
                        }
                        break;

                    case LB_BOUND:
                        if (locinput == reginfo->strbeg) {
                            match = FALSE;
                        }
                        else if (NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isLB(getLB_VAL_CP(UCHARAT(locinput -1)),
                                         getLB_VAL_CP(UCHARAT(locinput)),
                                         (U8*) reginfo->strbeg,
                                         (U8*) locinput,
                                         (U8*) reginfo->strend,
                                         utf8_target);
                        }
                        break;

                    case SB_BOUND: /* Always matches at begin and end */
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isSB(getSB_VAL_CP(UCHARAT(locinput -1)),
                                         getSB_VAL_CP(UCHARAT(locinput)),
                                         (U8*) reginfo->strbeg,
                                         (U8*) locinput,
                                         (U8*) reginfo->strend,
                                         utf8_target);
                        }
                        break;

                    case WB_BOUND:
                        if (locinput == reginfo->strbeg || NEXTCHR_IS_EOS) {
                            match = TRUE;
                        }
                        else {
                            match = isWB(WB_UNKNOWN,
                                         getWB_VAL_CP(UCHARAT(locinput -1)),
                                         getWB_VAL_CP(UCHARAT(locinput)),
                                         (U8*) reginfo->strbeg,
                                         (U8*) locinput,
                                         (U8*) reginfo->strend,
                                         utf8_target);
                        }
                        break;
                }
            }

            if (to_complement ^ ! match) {
                sayNO;
            }
            break;

        case ANYOFPOSIXL:
        case ANYOFL:  /*  /[abc]/l      */
            CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
            CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_SETS(scan);

            /* FALLTHROUGH */
        case ANYOFD:  /*   /[abc]/d       */
        case ANYOF:  /*   /[abc]/       */
            if (NEXTCHR_IS_EOS || locinput >= loceol)
                sayNO;
            if (  (! utf8_target || UTF8_IS_INVARIANT(*locinput))
                && ! ANYOF_FLAGS(scan)
                && ANYOF_MATCHES_NONE_OUTSIDE_BITMAP(scan))
            {
                if (! ANYOF_BITMAP_TEST(scan, * (U8 *) (locinput))) {
                    sayNO;
                }
                locinput++;
            }
            else {
                if (!reginclass(rex, scan, (U8*)locinput, (U8*) loceol,
                                                                   utf8_target))
                {
                    sayNO;
                }
                goto increment_locinput;
            }
            break;

        case ANYOFM:
            if (   NEXTCHR_IS_EOS
                || (UCHARAT(locinput) & FLAGS(scan)) != ARG1u(scan)
                || locinput >= loceol)
            {
                sayNO;
            }
            locinput++; /* ANYOFM is always single byte */
            break;

        case NANYOFM:
            if (   NEXTCHR_IS_EOS
                || (UCHARAT(locinput) & FLAGS(scan)) == ARG1u(scan)
                || locinput >= loceol)
            {
                sayNO;
            }
            goto increment_locinput;
            break;

        case ANYOFH:
            if (   ! utf8_target
                ||   NEXTCHR_IS_EOS
                ||   ANYOF_FLAGS(scan) > NATIVE_UTF8_TO_I8(*locinput)
                || ! (anyofh_list = GET_ANYOFH_INVLIST(rex, scan))
                || ! _invlist_contains_cp(anyofh_list,
                                          utf8_to_uvchr_buf((U8 *) locinput,
                                                            (U8 *) loceol,
                                                            NULL)))
            {
                sayNO;
            }
            goto increment_locinput;
            break;

        case ANYOFHb:
            if (   ! utf8_target
                ||   NEXTCHR_IS_EOS
                ||   ANYOF_FLAGS(scan) != (U8) *locinput
                || ! (anyofh_list = GET_ANYOFH_INVLIST(rex, scan))
                || ! _invlist_contains_cp(anyofh_list,
                                          utf8_to_uvchr_buf((U8 *) locinput,
                                                            (U8 *) loceol,
                                                            NULL)))
            {
                sayNO;
            }
            goto increment_locinput;
            break;

        case ANYOFHbbm:
            if (   ! utf8_target
                ||   NEXTCHR_IS_EOS
                ||   ANYOF_FLAGS(scan) != (U8) locinput[0]
                ||   locinput >= reginfo->strend
                || ! BITMAP_TEST(( (struct regnode_bbm *) scan)->bitmap,
                                   (U8) locinput[1] & UTF_CONTINUATION_MASK))
            {
                sayNO;
            }
            goto increment_locinput;
            break;

        case ANYOFHr:
            if (   ! utf8_target
                ||   NEXTCHR_IS_EOS
                || ! inRANGE((U8) NATIVE_UTF8_TO_I8(*locinput),
                             LOWEST_ANYOF_HRx_BYTE(ANYOF_FLAGS(scan)),
                             HIGHEST_ANYOF_HRx_BYTE(ANYOF_FLAGS(scan)))
                || ! (anyofh_list = GET_ANYOFH_INVLIST(rex, scan))
                || ! _invlist_contains_cp(anyofh_list,
                                          utf8_to_uvchr_buf((U8 *) locinput,
                                                            (U8 *) loceol,
                                                            NULL)))
            {
                sayNO;
            }
            goto increment_locinput;
            break;

        case ANYOFHs:
            if (   ! utf8_target
                ||   NEXTCHR_IS_EOS
                ||   loceol - locinput < FLAGS(scan)
                ||   memNE(locinput, ((struct regnode_anyofhs *) scan)->string, FLAGS(scan))
                || ! (anyofh_list = GET_ANYOFH_INVLIST(rex, scan))
                || ! _invlist_contains_cp(anyofh_list,
                                          utf8_to_uvchr_buf((U8 *) locinput,
                                                            (U8 *) loceol,
                                                            NULL)))
            {
                sayNO;
            }
            goto increment_locinput;
            break;

        case ANYOFR:
            if (NEXTCHR_IS_EOS) {
                sayNO;
            }

            if (utf8_target) {
                if (    ANYOF_FLAGS(scan) > NATIVE_UTF8_TO_I8(*locinput)
                   || ! withinCOUNT(utf8_to_uvchr_buf((U8 *) locinput,
                                                (U8 *) reginfo->strend,
                                                NULL),
                                    ANYOFRbase(scan), ANYOFRdelta(scan)))
                {
                    sayNO;
                }
            }
            else {
                if (! withinCOUNT((U8) *locinput,
                                  ANYOFRbase(scan), ANYOFRdelta(scan)))
                {
                    sayNO;
                }
            }
            goto increment_locinput;
            break;

        case ANYOFRb:
            if (NEXTCHR_IS_EOS) {
                sayNO;
            }

            if (utf8_target) {
                if (     ANYOF_FLAGS(scan) != (U8) *locinput
                    || ! withinCOUNT(utf8_to_uvchr_buf((U8 *) locinput,
                                                (U8 *) reginfo->strend,
                                                NULL),
                                     ANYOFRbase(scan), ANYOFRdelta(scan)))
                {
                    sayNO;
                }
            }
            else {
                if (! withinCOUNT((U8) *locinput,
                                  ANYOFRbase(scan), ANYOFRdelta(scan)))
                {
                    sayNO;
                }
            }
            goto increment_locinput;
            break;

        /* The argument (FLAGS) to all the POSIX node types is the class number
         * */

        case NPOSIXL:   /* \W or [:^punct:] etc. under /l */
            to_complement = 1;
            /* FALLTHROUGH */

        case POSIXL:    /* \w or [:punct:] etc. under /l */
            CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
            if (NEXTCHR_IS_EOS || locinput >= loceol)
                sayNO;

            /* Use isFOO_lc() for characters within Latin1.  (Note that
             * UTF8_IS_INVARIANT works even on non-UTF-8 strings, or else
             * wouldn't be invariant) */
            if (UTF8_IS_INVARIANT(nextbyte) || ! utf8_target) {
                if (! (to_complement ^ cBOOL(isFOO_lc(FLAGS(scan), (U8) nextbyte)))) {
                    sayNO;
                }

                locinput++;
                break;
            }

            if (! UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(locinput, reginfo->strend)) {
                /* An above Latin-1 code point, or malformed */
                _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(locinput,
                                                       reginfo->strend);
                goto utf8_posix_above_latin1;
            }

            /* Here is a UTF-8 variant code point below 256 and the target is
             * UTF-8 */
            if (! (to_complement ^ cBOOL(isFOO_lc(FLAGS(scan),
                                            EIGHT_BIT_UTF8_TO_NATIVE(nextbyte,
                                            *(locinput + 1))))))
            {
                sayNO;
            }

            goto increment_locinput;

        case NPOSIXD:   /* \W or [:^punct:] etc. under /d */
            to_complement = 1;
            /* FALLTHROUGH */

        case POSIXD:    /* \w or [:punct:] etc. under /d */
            if (utf8_target) {
                goto utf8_posix;
            }
            goto posixa;

        case NPOSIXA:   /* \W or [:^punct:] etc. under /a */

            if (NEXTCHR_IS_EOS || locinput >= loceol) {
                sayNO;
            }

            /* All UTF-8 variants match */
            if (! UTF8_IS_INVARIANT(nextbyte)) {
                goto increment_locinput;
            }

            to_complement = 1;
            goto join_nposixa;

        case POSIXA:    /* \w or [:punct:] etc. under /a */

          posixa:
            /* We get here through POSIXD, NPOSIXD, and NPOSIXA when not in
             * UTF-8, and also from NPOSIXA even in UTF-8 when the current
             * character is a single byte */

            if (NEXTCHR_IS_EOS || locinput >= loceol) {
                sayNO;
            }

          join_nposixa:

            if (! (to_complement ^ cBOOL(generic_isCC_A_(nextbyte,
                                                                FLAGS(scan)))))
            {
                sayNO;
            }

            /* Here we are either not in utf8, or we matched a utf8-invariant,
             * so the next char is the next byte */
            locinput++;
            break;

        case NPOSIXU:   /* \W or [:^punct:] etc. under /u */
            to_complement = 1;
            /* FALLTHROUGH */

        case POSIXU:    /* \w or [:punct:] etc. under /u */
          utf8_posix:
            if (NEXTCHR_IS_EOS || locinput >= loceol) {
                sayNO;
            }

            /* Use generic_isCC_() for characters within Latin1.  (Note that
             * UTF8_IS_INVARIANT works even on non-UTF-8 strings, or else
             * wouldn't be invariant) */
            if (UTF8_IS_INVARIANT(nextbyte) || ! utf8_target) {
                if (! (to_complement ^ cBOOL(generic_isCC_(nextbyte,
                                                           FLAGS(scan)))))
                {
                    sayNO;
                }
                locinput++;
            }
            else if (UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(locinput, reginfo->strend)) {
                if (! (to_complement
                       ^ cBOOL(generic_isCC_(EIGHT_BIT_UTF8_TO_NATIVE(nextbyte,
                                                               *(locinput + 1)),
                                             FLAGS(scan)))))
                {
                    sayNO;
                }
                locinput += 2;
            }
            else {  /* Handle above Latin-1 code points */
              utf8_posix_above_latin1:
                classnum = (char_class_number_) FLAGS(scan);
                switch (classnum) {
                    default:
                        if (! (to_complement
                           ^ cBOOL(_invlist_contains_cp(
                                      PL_XPosix_ptrs[classnum],
                                      utf8_to_uvchr_buf((U8 *) locinput,
                                                        (U8 *) reginfo->strend,
                                                        NULL)))))
                        {
                            sayNO;
                        }
                        break;
                    case CC_ENUM_SPACE_:
                        if (! (to_complement
                                    ^ cBOOL(is_XPERLSPACE_high(locinput))))
                        {
                            sayNO;
                        }
                        break;
                    case CC_ENUM_BLANK_:
                        if (! (to_complement
                                        ^ cBOOL(is_HORIZWS_high(locinput))))
                        {
                            sayNO;
                        }
                        break;
                    case CC_ENUM_XDIGIT_:
                        if (! (to_complement
                                        ^ cBOOL(is_XDIGIT_high(locinput))))
                        {
                            sayNO;
                        }
                        break;
                    case CC_ENUM_VERTSPACE_:
                        if (! (to_complement
                                        ^ cBOOL(is_VERTWS_high(locinput))))
                        {
                            sayNO;
                        }
                        break;
                    case CC_ENUM_CNTRL_:    /* These can't match above Latin1 */
                    case CC_ENUM_ASCII_:
                        if (! to_complement) {
                            sayNO;
                        }
                        break;
                }
                locinput += UTF8_SAFE_SKIP(locinput, reginfo->strend);
            }
            break;

        case CLUMP: /* Match \X: logical Unicode character.  This is defined as
                       a Unicode extended Grapheme Cluster */
            if (NEXTCHR_IS_EOS || locinput >= loceol)
                sayNO;
            if  (! utf8_target) {

                /* Match either CR LF  or '.', as all the other possibilities
                 * require utf8 */
                locinput++;	    /* Match the . or CR */
                if (nextbyte == '\r' /* And if it was CR, and the next is LF,
                                       match the LF */
                    && locinput <  loceol
                    && UCHARAT(locinput) == '\n')
                {
                    locinput++;
                }
            }
            else {

                /* Get the gcb type for the current character */
                GCB_enum prev_gcb = getGCB_VAL_UTF8((U8*) locinput,
                                                       (U8*) reginfo->strend);

                /* Then scan through the input until we get to the first
                 * character whose type is supposed to be a gcb with the
                 * current character.  (There is always a break at the
                 * end-of-input) */
                locinput += UTF8SKIP(locinput);
                while (locinput < loceol) {
                    GCB_enum cur_gcb = getGCB_VAL_UTF8((U8*) locinput,
                                                         (U8*) reginfo->strend);
                    if (isGCB(prev_gcb, cur_gcb,
                              (U8*) reginfo->strbeg, (U8*) locinput,
                              utf8_target))
                    {
                        break;
                    }

                    prev_gcb = cur_gcb;
                    locinput += UTF8SKIP(locinput);
                }


            }
            break;

        case REFFLN:  /*  /\g{name}/il  */
        {   /* The capture buffer cases.  The ones beginning with N for the
               named buffers just convert to the equivalent numbered and
               pretend they were called as the corresponding numbered buffer
               op.  */
            /* don't initialize these in the declaration, it makes C++
               unhappy */
            const char *s;
            char type;
            re_fold_t folder;
            const U8 *fold_array;
            UV utf8_fold_flags;

            CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
            folder = Perl_foldEQ_locale;
            fold_array = PL_fold_locale;
            type = REFFL;
            utf8_fold_flags = FOLDEQ_LOCALE;
            goto do_nref;

        case REFFAN:  /*  /\g{name}/iaa  */
            folder = Perl_foldEQ_latin1;
            fold_array = PL_fold_latin1;
            type = REFFA;
            utf8_fold_flags = FOLDEQ_UTF8_NOMIX_ASCII;
            goto do_nref;

        case REFFUN:  /*  /\g{name}/iu  */
            folder = Perl_foldEQ_latin1;
            fold_array = PL_fold_latin1;
            type = REFFU;
            utf8_fold_flags = 0;
            goto do_nref;

        case REFFN:  /*  /\g{name}/i  */
            folder = Perl_foldEQ;
            fold_array = PL_fold;
            type = REFF;
            utf8_fold_flags = 0;
            goto do_nref;

        case REFN:  /*  /\g{name}/   */
            type = REF;
            folder = NULL;
            fold_array = NULL;
            utf8_fold_flags = 0;
          do_nref:

            /* For the named back references, find the corresponding buffer
             * number */
            n = reg_check_named_buff_matched(rex,scan);

            if ( ! n ) {
                sayNO;
            }
            goto do_nref_ref_common;

        case REFFL:  /*  /\1/il  */
            CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
            folder = Perl_foldEQ_locale;
            fold_array = PL_fold_locale;
            utf8_fold_flags = FOLDEQ_LOCALE;
            goto do_ref;

        case REFFA:  /*  /\1/iaa  */
            folder = Perl_foldEQ_latin1;
            fold_array = PL_fold_latin1;
            utf8_fold_flags = FOLDEQ_UTF8_NOMIX_ASCII;
            goto do_ref;

        case REFFU:  /*  /\1/iu  */
            folder = Perl_foldEQ_latin1;
            fold_array = PL_fold_latin1;
            utf8_fold_flags = 0;
            goto do_ref;

        case REFF:  /*  /\1/i  */
            folder = Perl_foldEQ;
            fold_array = PL_fold;
            utf8_fold_flags = 0;
            goto do_ref;

#undef  ST
#define ST st->u.backref
        case REF:  /*  /\1/    */
            folder = NULL;
            fold_array = NULL;
            utf8_fold_flags = 0;

          do_ref:
            type = OP(scan);
            n = ARG1u(scan);  /* which paren pair */
            if (rex->logical_to_parno) {
                n = rex->logical_to_parno[n];
                do {
                    if ( RXp_LASTPAREN(rex) < n ||
                         RXp_OFFS_START(rex,n) == -1 ||
                         RXp_OFFS_END(rex,n) == -1
                    ) {
                        n = rex->parno_to_logical_next[n];
                    }
                    else {
                        break;
                    }
                } while(n);
                
                if (!n) /* this means there is nothing that matched */
                    sayNO;
            }

          do_nref_ref_common:
            reginfo->poscache_iter = reginfo->poscache_maxiter; /* Void cache */
            if (RXp_LASTPAREN(rex) < n)
                sayNO;

            ln = RXp_OFFSp(rex)[n].start;
            endref = RXp_OFFSp(rex)[n].end;
            if (ln == -1 || endref == -1)
                sayNO;			/* Do not match unless seen CLOSEn. */

            if (ln == endref)
                goto ref_yes;

            s = reginfo->strbeg + ln;
            if (type != REF	/* REF can do byte comparison */
                && (utf8_target || type == REFFU || type == REFFL))
            {
                char * limit = loceol;

                /* This call case insensitively compares the entire buffer
                    * at s, with the current input starting at locinput, but
                    * not going off the end given by loceol, and
                    * returns in <limit> upon success, how much of the
                    * current input was matched */
                if (! foldEQ_utf8_flags(s, NULL, endref - ln, utf8_target,
                                    locinput, &limit, 0, utf8_target, utf8_fold_flags))
                {
                    sayNO;
                }
                locinput = limit;
                goto ref_yes;
            }

            /* Not utf8:  Inline the first character, for speed. */
            if ( ! NEXTCHR_IS_EOS
                && locinput < loceol
                && UCHARAT(s) != nextbyte
                && (   type == REF
                    || UCHARAT(s) != fold_array[nextbyte]))
            {
                sayNO;
            }
            ln = endref - ln;
            if (locinput + ln > loceol)
                sayNO;
            if (ln > 1 && (type == REF
                           ? memNE(s, locinput, ln)
                           : ! folder(aTHX_ locinput, s, ln)))
                sayNO;
            locinput += ln;
        }
        ref_yes:
            if (FLAGS(scan)) { /* == VOLATILE_REF but only other value is 0 */
                ST.cp = regcppush(rex, ARG2u(scan) - 1, maxopenparen);
                REGCP_SET(ST.lastcp);
                PUSH_STATE_GOTO(REF_next, next, locinput, loceol,
                                script_run_begin);
            }
            break;
            NOT_REACHED; /* NOTREACHED */

        case REF_next:
            sayYES;
            break;

        case REF_next_fail:
            REGCP_UNWIND(ST.lastcp);
            regcppop(rex, &maxopenparen);
            sayNO;
            break;

        case NOTHING: /* null op; e.g. the 'nothing' following
                       * the '*' in m{(a+|b)*}' */
            break;
        case TAIL: /* placeholder while compiling (A|B|C) */
            break;

#undef  ST
#define ST st->u.eval
#define CUR_EVAL cur_eval->u.eval

        {
            SV *ret;
            REGEXP *re_sv;
            regexp *re;
            regexp_internal *rei;
            regnode *startpoint;
            U32 arg;

        case GOSUB: /*    /(...(?1))/   /(...(?&foo))/   */
            arg = ARG1u(scan);
            if (cur_eval && cur_eval->locinput == locinput) {
                if ( ++nochange_depth > max_nochange_depth )
                    Perl_croak(aTHX_
                        "Pattern subroutine nesting without pos change"
                        " exceeded limit in regex");
            } else {
                nochange_depth = 0;
            }
            re_sv = rex_sv;
            re = rex;
            rei = rexi;
            startpoint = scan + ARG2i(scan);
            EVAL_CLOSE_PAREN_SET( st, arg );
            /* Detect infinite recursion
             *
             * A pattern like /(?R)foo/ or /(?<x>(?&y)foo)(?<y>(?&x)bar)/
             * or "a"=~/(.(?2))((?<=(?=(?1)).))/ could recurse forever.
             * So we track the position in the string we are at each time
             * we recurse and if we try to enter the same routine twice from
             * the same position we throw an error.
             */
            if ( rex->recurse_locinput[arg] == locinput ) {
                /* FIXME: we should show the regop that is failing as part
                 * of the error message. */
                Perl_croak(aTHX_ "Infinite recursion in regex");
            } else {
                ST.prev_recurse_locinput= rex->recurse_locinput[arg];
                rex->recurse_locinput[arg]= locinput;

                DEBUG_r({
                    DECLARE_AND_GET_RE_DEBUG_FLAGS;
                    DEBUG_STACK_r({
                        Perl_re_exec_indentf( aTHX_
                            "entering GOSUB, prev_recurse_locinput=%p recurse_locinput[%d]=%p\n",
                            depth, ST.prev_recurse_locinput, arg, rex->recurse_locinput[arg]
                        );
                    });
                });
            }

            /* Save all the positions seen so far. */
            ST.cp = regcppush(rex, 0, maxopenparen);
            REGCP_SET(ST.lastcp);

            /* and then jump to the code we share with EVAL */
            goto eval_recurse_doit;
            /* NOTREACHED */

        case EVAL:  /*   /(?{...})B/   /(??{A})B/  and  /(?(?{...})X|Y)B/   */
            if (logical == 2 && cur_eval && cur_eval->locinput==locinput) {
                if ( ++nochange_depth > max_nochange_depth )
                    Perl_croak(aTHX_ "EVAL without pos change exceeded limit in regex");
            } else {
                nochange_depth = 0;
            }
            {
                /* execute the code in the {...} */

                dSP;
                IV before;
                OP * const oop = PL_op;
                COP * const ocurcop = PL_curcop;
                OP *nop;
                CV *newcv;

                /* save *all* paren positions */
                ST.cp = regcppush(rex, 0, maxopenparen);
                REGCP_SET(ST.lastcp);

                if (!caller_cv)
                    caller_cv = find_runcv(NULL);

                n = ARG1u(scan);

                if (rexi->data->what[n] == 'r') { /* code from an external qr */
                    newcv = (ReANY(
                                    (REGEXP*)(rexi->data->data[n])
                            ))->qr_anoncv;
                    nop = (OP*)rexi->data->data[n+1];
                }
                else if (rexi->data->what[n] == 'l') { /* literal code */
                    newcv = caller_cv;
                    nop = (OP*)rexi->data->data[n];
                    assert(CvDEPTH(newcv));
                }
                else {
                    /* literal with own CV */
                    assert(rexi->data->what[n] == 'L');
                    newcv = rex->qr_anoncv;
                    nop = (OP*)rexi->data->data[n];
                }

                /* Some notes about MULTICALL and the context and save stacks.
                 *
                 * In something like
                 *   /...(?{ my $x)}...(?{ my $y)}...(?{ my $z)}.../
                 * since codeblocks don't introduce a new scope (so that
                 * local() etc accumulate), at the end of a successful
                 * match there will be a SAVEt_CLEARSV on the savestack
                 * for each of $x, $y, $z. If the three code blocks above
                 * happen to have come from different CVs (e.g. via
                 * embedded qr//s), then we must ensure that during any
                 * savestack unwinding, PL_comppad always points to the
                 * right pad at each moment. We achieve this by
                 * interleaving SAVEt_COMPPAD's on the savestack whenever
                 * there is a change of pad.
                 * In theory whenever we call a code block, we should
                 * push a CXt_SUB context, then pop it on return from
                 * that code block. This causes a bit of an issue in that
                 * normally popping a context also clears the savestack
                 * back to cx->blk_oldsaveix, but here we specifically
                 * don't want to clear the save stack on exit from the
                 * code block.
                 * Also for efficiency we don't want to keep pushing and
                 * popping the single SUB context as we backtrack etc.
                 * So instead, we push a single context the first time
                 * we need, it, then hang onto it until the end of this
                 * function. Whenever we encounter a new code block, we
                 * update the CV etc if that's changed. During the times
                 * in this function where we're not executing a code
                 * block, having the SUB context still there is a bit
                 * naughty - but we hope that no-one notices.
                 * When the SUB context is initially pushed, we fake up
                 * cx->blk_oldsaveix to be as if we'd pushed this context
                 * on first entry to S_regmatch rather than at some random
                 * point during the regexe execution. That way if we
                 * croak, popping the context stack will ensure that
                 * *everything* SAVEd by this function is undone and then
                 * the context popped, rather than e.g., popping the
                 * context (and restoring the original PL_comppad) then
                 * popping more of the savestack and restoring a bad
                 * PL_comppad.
                 */

                /* If this is the first EVAL, push a MULTICALL. On
                 * subsequent calls, if we're executing a different CV, or
                 * if PL_comppad has got messed up from backtracking
                 * through SAVECOMPPADs, then refresh the context.
                 */
                if (newcv != last_pushed_cv || PL_comppad != last_pad)
                {
                    U8 flags = (CXp_SUB_RE |
                                ((newcv == caller_cv) ? CXp_SUB_RE_FAKE : 0));
                    SAVECOMPPAD();
                    if (last_pushed_cv) {
                        CHANGE_MULTICALL_FLAGS(newcv, flags);
                    }
                    else {
                        PUSH_MULTICALL_FLAGS(newcv, flags);
                    }
                    /* see notes above */
                    CX_CUR()->blk_oldsaveix = orig_savestack_ix;

                    last_pushed_cv = newcv;
                }
                else {
                    /* these assignments are just to silence compiler
                     * warnings */
                    multicall_cop = NULL;
                }
                last_pad = PL_comppad;

                /* the initial nextstate you would normally execute
                 * at the start of an eval (which would cause error
                 * messages to come from the eval), may be optimised
                 * away from the execution path in the regex code blocks;
                 * so manually set PL_curcop to it initially */
                {
                    OP *o = cUNOPx(nop)->op_first;
                    assert(o->op_type == OP_NULL);
                    if (o->op_targ == OP_SCOPE) {
                        o = cUNOPo->op_first;
                    }
                    else {
                        assert(o->op_targ == OP_LEAVE);
                        o = cUNOPo->op_first;
                        assert(o->op_type == OP_ENTER);
                        o = OpSIBLING(o);
                    }

                    if (o->op_type != OP_STUB) {
                        assert(    o->op_type == OP_NEXTSTATE
                                || o->op_type == OP_DBSTATE
                                || (o->op_type == OP_NULL
                                    &&  (  o->op_targ == OP_NEXTSTATE
                                        || o->op_targ == OP_DBSTATE
                                        )
                                    )
                        );
                        PL_curcop = (COP*)o;
                    }
                }
                nop = nop->op_next;

                DEBUG_STATE_r( Perl_re_printf( aTHX_
                    "  re EVAL PL_op=0x%" UVxf "\n", PTR2UV(nop)) );

                RXp_OFFSp(rex)[0].end = locinput - reginfo->strbeg;
                if (reginfo->info_aux_eval->pos_magic)
                    MgBYTEPOS_set(reginfo->info_aux_eval->pos_magic,
                                  reginfo->sv, reginfo->strbeg,
                                  locinput - reginfo->strbeg);

                if (sv_yes_mark) {
                    SV *sv_mrk = get_sv("REGMARK", 1);
                    sv_setsv(sv_mrk, sv_yes_mark);
                }

                /* we don't use MULTICALL here as we want to call the
                 * first op of the block of interest, rather than the
                 * first op of the sub. Also, we don't want to free
                 * the savestack frame */
                before = (IV)(SP-PL_stack_base);
                PL_op = nop;
                CALLRUNOPS(aTHX);			/* Scalar context. */
                SPAGAIN;
                if ((IV)(SP-PL_stack_base) == before)
                    ret = &PL_sv_undef;   /* protect against empty (?{}) blocks. */
                else {
                    ret = POPs;
                    PUTBACK;
                }

                /* before restoring everything, evaluate the returned
                 * value, so that 'uninit' warnings don't use the wrong
                 * PL_op or pad. Also need to process any magic vars
                 * (e.g. $1) *before* parentheses are restored */

                PL_op = NULL;

                re_sv = NULL;
                if (logical == 0) {       /* /(?{ ... })/ and /(*{ ... })/ */
                    SV *replsv = save_scalar(PL_replgv);
                    sv_setsv(replsv, ret); /* $^R */
                    SvSETMAGIC(replsv);
                }
                else if (logical == 1) { /*   /(?(?{...})X|Y)/    */
                    sw = cBOOL(SvTRUE_NN(ret));
                    logical = 0;
                }
                else {                   /*  /(??{ ... })  */
                    /*  if its overloaded, let the regex compiler handle
                     *  it; otherwise extract regex, or stringify  */
                    if (SvGMAGICAL(ret))
                        ret = sv_mortalcopy(ret);
                    if (!SvAMAGIC(ret)) {
                        SV *sv = ret;
                        if (SvROK(sv))
                            sv = SvRV(sv);
                        if (SvTYPE(sv) == SVt_REGEXP)
                            re_sv = (REGEXP*) sv;
                        else if (SvSMAGICAL(ret)) {
                            MAGIC *mg = mg_find(ret, PERL_MAGIC_qr);
                            if (mg)
                                re_sv = (REGEXP *) mg->mg_obj;
                        }

                        /* force any undef warnings here */
                        if (!re_sv && !SvPOK(ret) && !SvNIOK(ret)) {
                            ret = sv_mortalcopy(ret);
                            (void) SvPV_force_nolen(ret);
                        }
                    }

                }

                /* *** Note that at this point we don't restore
                 * PL_comppad, (or pop the CxSUB) on the assumption it may
                 * be used again soon. This is safe as long as nothing
                 * in the regexp code uses the pad ! */
                PL_op = oop;
                PL_curcop = ocurcop;
                regcp_restore(rex, ST.lastcp, &maxopenparen);
                PL_curpm_under = PL_curpm;
                PL_curpm = PL_reg_curpm;

                if (logical != 2) {
                    PUSH_STATE_GOTO(EVAL_B, next, locinput, loceol,
                                    script_run_begin);
                    /* NOTREACHED */
                }
            }

                /* only /(??{ ... })/  from now on */
                logical = 0;
                {
                    /* extract RE object from returned value; compiling if
                     * necessary */

                    if (re_sv) {
                        re_sv = reg_temp_copy(NULL, re_sv);
                    }
                    else {
                        U32 pm_flags = 0;

                        if (SvUTF8(ret) && IN_BYTES) {
                            /* In use 'bytes': make a copy of the octet
                             * sequence, but without the flag on */
                            STRLEN len;
                            const char *const p = SvPV(ret, len);
                            ret = newSVpvn_flags(p, len, SVs_TEMP);
                        }
                        if (rex->intflags & PREGf_USE_RE_EVAL)
                            pm_flags |= PMf_USE_RE_EVAL;

                        /* if we got here, it should be an engine which
                         * supports compiling code blocks and stuff */
                        assert(rex->engine && rex->engine->op_comp);
                        assert(!(FLAGS(scan) & ~RXf_PMf_COMPILETIME));
                        re_sv = rex->engine->op_comp(aTHX_ &ret, 1, NULL,
                                    rex->engine, NULL, NULL,
                                    /* copy /msixn etc to inner pattern */
                                    ARG2i(scan),
                                    pm_flags);

                        if (!(SvFLAGS(ret)
                              & (SVs_TEMP | SVs_GMG | SVf_ROK))
                         && (!SvPADTMP(ret) || SvREADONLY(ret))) {
                            /* This isn't a first class regexp. Instead, it's
                               caching a regexp onto an existing, Perl visible
                               scalar.  */
                            sv_magic(ret, MUTABLE_SV(re_sv), PERL_MAGIC_qr, 0, 0);
                        }
                    }
                    SAVEFREESV(re_sv);
                    re = ReANY(re_sv);
                }
                RXp_MATCH_COPIED_off(re);
                RXp_SUBBEG(re) = RXp_SUBBEG(rex);
                RXp_SUBLEN(re) = RXp_SUBLEN(rex);
                RXp_SUBOFFSET(re) = RXp_SUBOFFSET(rex);
                RXp_SUBCOFFSET(re) = RXp_SUBCOFFSET(rex);
                RXp_LASTPAREN(re) = 0;
                RXp_LASTCLOSEPAREN(re) = 0;
                rei = RXi_GET(re);
                DEBUG_EXECUTE_r(
                    debug_start_match(re_sv, utf8_target, locinput,
                                    reginfo->strend, "EVAL/GOSUB: Matching embedded");
                );
                startpoint = rei->program + 1;
                EVAL_CLOSE_PAREN_CLEAR(st); /* ST.close_paren = 0;
                                             * close_paren only for GOSUB */
                ST.prev_recurse_locinput= NULL; /* only used for GOSUB */

                /* note we saved the paren state earlier:
                ST.cp = regcppush(rex, 0, maxopenparen);
                REGCP_SET(ST.lastcp);
                */
                /* and set maxopenparen to 0, since we are starting a "fresh" match */
                maxopenparen = 0;
                /* run the pattern returned from (??{...}) */

              eval_recurse_doit: /* Share code with GOSUB below this line
                            * At this point we expect the stack context to be
                            * set up correctly */

                /* invalidate the S-L poscache. We're now executing a
                 * different set of WHILEM ops (and their associated
                 * indexes) against the same string, so the bits in the
                 * cache are meaningless. Setting maxiter to zero forces
                 * the cache to be invalidated and zeroed before reuse.
                 * XXX This is too dramatic a measure. Ideally we should
                 * save the old cache and restore when running the outer
                 * pattern again */
                reginfo->poscache_maxiter = 0;

                /* the new regexp might have a different is_utf8_pat than we do */
                is_utf8_pat = reginfo->is_utf8_pat = cBOOL(RX_UTF8(re_sv));

                ST.prev_rex = rex_sv;
                ST.prev_curlyx = cur_curlyx;
                rex_sv = re_sv;
                SET_reg_curpm(rex_sv);
                rex = re;
                rexi = rei;
                cur_curlyx = NULL;
                ST.B = next;
                ST.prev_eval = cur_eval;
                cur_eval = st;
                /* now continue from first node in postoned RE */
                PUSH_YES_STATE_GOTO(EVAL_postponed_AB, startpoint, locinput,
                                    loceol, script_run_begin);
                NOT_REACHED; /* NOTREACHED */
        }

        case EVAL_postponed_AB: /* cleanup after a successful (??{A})B */
            /* note: this is called twice; first after popping B, then A */
            DEBUG_STACK_r({
                Perl_re_exec_indentf( aTHX_  "EVAL_AB cur_eval=%p prev_eval=%p\n",
                    depth, cur_eval, ST.prev_eval);
            });

#define SET_RECURSE_LOCINPUT(STR,VAL)\
            if ( cur_eval && CUR_EVAL.close_paren ) {\
                DEBUG_STACK_r({ \
                    Perl_re_exec_indentf( aTHX_  STR " GOSUB%d ce=%p recurse_locinput=%p\n",\
                        depth,    \
                        CUR_EVAL.close_paren - 1,\
                        cur_eval, \
                        VAL);     \
                });               \
                rex->recurse_locinput[CUR_EVAL.close_paren - 1] = VAL;\
            }

            SET_RECURSE_LOCINPUT("EVAL_AB[before]", CUR_EVAL.prev_recurse_locinput);

            rex_sv = ST.prev_rex;
            is_utf8_pat = reginfo->is_utf8_pat = cBOOL(RX_UTF8(rex_sv));
            SET_reg_curpm(rex_sv);
            rex = ReANY(rex_sv);
            rexi = RXi_GET(rex);
            {
                /* preserve $^R across LEAVE's. See Bug 121070. */
                SV *save_sv= GvSV(PL_replgv);
                SV *replsv;
                SvREFCNT_inc(save_sv);
                regcpblow(ST.cp); /* LEAVE in disguise */
                /* don't move this initialization up */
                replsv = GvSV(PL_replgv);
                sv_setsv(replsv, save_sv);
                SvSETMAGIC(replsv);
                SvREFCNT_dec(save_sv);
            }
            cur_eval = ST.prev_eval;
            cur_curlyx = ST.prev_curlyx;

            /* Invalidate cache. See "invalidate" comment above. */
            reginfo->poscache_maxiter = 0;
            if ( nochange_depth )
                nochange_depth--;

            SET_RECURSE_LOCINPUT("EVAL_AB[after]", cur_eval->locinput);
            sayYES;


        case EVAL_B_fail: /* unsuccessful B in (?{...})B */
            REGCP_UNWIND(ST.lastcp);
            regcppop(rex, &maxopenparen);
            sayNO;

        case EVAL_postponed_AB_fail: /* unsuccessfully ran A or B in (??{A})B */
            /* note: this is called twice; first after popping B, then A */
            DEBUG_STACK_r({
                Perl_re_exec_indentf( aTHX_  "EVAL_AB_fail cur_eval=%p prev_eval=%p\n",
                    depth, cur_eval, ST.prev_eval);
            });

            SET_RECURSE_LOCINPUT("EVAL_AB_fail[before]", CUR_EVAL.prev_recurse_locinput);

            rex_sv = ST.prev_rex;
            is_utf8_pat = reginfo->is_utf8_pat = cBOOL(RX_UTF8(rex_sv));
            SET_reg_curpm(rex_sv);
            rex = ReANY(rex_sv);
            rexi = RXi_GET(rex);

            REGCP_UNWIND(ST.lastcp);
            regcppop(rex, &maxopenparen);
            cur_eval = ST.prev_eval;
            cur_curlyx = ST.prev_curlyx;

            /* Invalidate cache. See "invalidate" comment above. */
            reginfo->poscache_maxiter = 0;
            if ( nochange_depth )
                nochange_depth--;

            SET_RECURSE_LOCINPUT("EVAL_AB_fail[after]", cur_eval->locinput);
            sayNO_SILENT;
#undef ST

        case OPEN: /*  (  */
            n = PARNO(scan);  /* which paren pair */
            RXp_OFFSp(rex)[n].start_tmp = locinput - reginfo->strbeg;
            if (n > maxopenparen)
                maxopenparen = n;
            DEBUG_BUFFERS_r(Perl_re_exec_indentf( aTHX_
                "OPEN: rex=0x%" UVxf " offs=0x%" UVxf ": \\%" UVuf ": set %" IVdf " tmp; maxopenparen=%" UVuf "\n",
                depth,
                PTR2UV(rex),
                PTR2UV(RXp_OFFSp(rex)),
                (UV)n,
                (IV)RXp_OFFSp(rex)[n].start_tmp,
                (UV)maxopenparen
            ));
            lastopen = n;
            break;

        case SROPEN: /*  (*SCRIPT_RUN:  */
            script_run_begin = (U8 *) locinput;
            break;


        case CLOSE:  /*  )  */
            n = PARNO(scan);  /* which paren pair */
            CLOSE_CAPTURE(rex, n, RXp_OFFSp(rex)[n].start_tmp,
                             locinput - reginfo->strbeg);
            if ( EVAL_CLOSE_PAREN_IS( cur_eval, n ) )
                goto fake_end;

            break;

        case SRCLOSE:  /*  (*SCRIPT_RUN: ... )   */

            if (! isSCRIPT_RUN(script_run_begin, (U8 *) locinput, utf8_target))
            {
                sayNO;
            }

            break;


        case ACCEPT:  /*  (*ACCEPT)  */
            is_accepted = true;
            if (FLAGS(scan))
                sv_yes_mark = MUTABLE_SV(rexi->data->data[ ARG1u( scan ) ]);
            utmp = ARG2u(scan);

            if ( utmp ) {
                regnode *cursor;
                for (
                    cursor = scan;
                    cursor && ( OP(cursor) != END );
                    cursor = (
                               REGNODE_TYPE( OP(cursor) ) == END
                               || REGNODE_TYPE( OP(cursor) ) == WHILEM
                             )
                             ? REGNODE_AFTER(cursor)
                             : regnext(cursor)
                ){
                    if ( OP(cursor) != CLOSE )
                        continue;

                    n = PARNO(cursor);

                    if ( n > lastopen ) /* might be OPEN/CLOSE in the way */
                        continue;       /* so skip this one */

                    CLOSE_CAPTURE(rex, n, RXp_OFFSp(rex)[n].start_tmp,
                                     locinput - reginfo->strbeg);

                    if ( n == utmp || EVAL_CLOSE_PAREN_IS(cur_eval, n) )
                        break;
                }
            }
            goto fake_end;
            /* NOTREACHED */

        case GROUPP:  /*  (?(1))  */
            n = ARG1u(scan);  /* which paren pair */
            sw = cBOOL(RXp_LASTPAREN(rex) >= n && RXp_OFFS_END(rex,n) != -1);
            break;

        case GROUPPN:  /*  (?(<name>))  */
            /* reg_check_named_buff_matched returns 0 for no match */
            sw = cBOOL(0 < reg_check_named_buff_matched(rex,scan));
            break;

        case INSUBP:   /*  (?(R))  */
            n = ARG1u(scan);
            /* this does not need to use EVAL_CLOSE_PAREN macros, as the arg
             * of SCAN is already set up as matches a eval.close_paren */
            sw = cur_eval && (n == 0 || CUR_EVAL.close_paren == n);
            break;

        case DEFINEP:  /*  (?(DEFINE))  */
            sw = 0;
            break;

        case IFTHEN:   /*  (?(cond)A|B)  */
            reginfo->poscache_iter = reginfo->poscache_maxiter; /* Void cache */
            if (sw)
                next = REGNODE_AFTER_type(scan,tregnode_IFTHEN);
            else {
                next = scan + ARG1u(scan);
                if (OP(next) == IFTHEN) /* Fake one. */
                    next = REGNODE_AFTER_type(next,tregnode_IFTHEN);
            }
            break;

        case LOGICAL:  /* modifier for EVAL and IFMATCH */
            logical = FLAGS(scan) & EVAL_FLAGS_MASK; /* reserve a bit for optimistic eval */
            break;

/*******************************************************************

The CURLYX/WHILEM pair of ops handle the most generic case of the /A*B/
pattern, where A and B are subpatterns. (For simple A, CURLYM or
STAR/PLUS/CURLY/CURLYN are used instead.)

A*B is compiled as <CURLYX><A><WHILEM><B>

On entry to the subpattern, CURLYX is called. This pushes a CURLYX
state, which contains the current count, initialised to -1. It also sets
cur_curlyx to point to this state, with any previous value saved in the
state block.

CURLYX then jumps straight to the WHILEM op, rather than executing A,
since the pattern may possibly match zero times (i.e. it's a while {} loop
rather than a do {} while loop).

Each entry to WHILEM represents a successful match of A. The count in the
CURLYX block is incremented, another WHILEM state is pushed, and execution
passes to A or B depending on greediness and the current count.

For example, if matching against the string a1a2a3b (where the aN are
substrings that match /A/), then the match progresses as follows: (the
pushed states are interspersed with the bits of strings matched so far):

    <CURLYX cnt=-1>
    <CURLYX cnt=0><WHILEM>
    <CURLYX cnt=1><WHILEM> a1 <WHILEM>
    <CURLYX cnt=2><WHILEM> a1 <WHILEM> a2 <WHILEM>
    <CURLYX cnt=3><WHILEM> a1 <WHILEM> a2 <WHILEM> a3 <WHILEM>
    <CURLYX cnt=3><WHILEM> a1 <WHILEM> a2 <WHILEM> a3 <WHILEM> b

(Contrast this with something like CURLYM, which maintains only a single
backtrack state:

    <CURLYM cnt=0> a1
    a1 <CURLYM cnt=1> a2
    a1 a2 <CURLYM cnt=2> a3
    a1 a2 a3 <CURLYM cnt=3> b
)

Each WHILEM state block marks a point to backtrack to upon partial failure
of A or B, and also contains some minor state data related to that
iteration.  The CURLYX block, pointed to by cur_curlyx, contains the
overall state, such as the count, and pointers to the A and B ops.

This is complicated slightly by nested CURLYX/WHILEM's. Since cur_curlyx
must always point to the *current* CURLYX block, the rules are:

When executing CURLYX, save the old cur_curlyx in the CURLYX state block,
and set cur_curlyx to point the new block.

When popping the CURLYX block after a successful or unsuccessful match,
restore the previous cur_curlyx.

When WHILEM is about to execute B, save the current cur_curlyx, and set it
to the outer one saved in the CURLYX block.

When popping the WHILEM block after a successful or unsuccessful B match,
restore the previous cur_curlyx.

Here's an example for the pattern (AI* BI)*BO
I and O refer to inner and outer, C and W refer to CURLYX and WHILEM:

cur_
curlyx backtrack stack
------ ---------------
NULL
CO     <CO prev=NULL> <WO>
CI     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai
CO     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi
NULL   <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi <WO prev=CO> bo

At this point the pattern succeeds, and we work back down the stack to
clean up, restoring as we go:

CO     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai <WI prev=CI> bi
CI     <CO prev=NULL> <WO> <CI prev=CO> <WI> ai
CO     <CO prev=NULL> <WO>
NULL

*******************************************************************/

#define ST st->u.curlyx

        case CURLYX:    /* start of /A*B/  (for complex A) */
        {
            /* No need to save/restore up to this paren */
            I32 parenfloor = FLAGS(scan);

            assert(next); /* keep Coverity happy */
            if (OP(REGNODE_BEFORE(next)) == NOTHING) /* LONGJMP */
                next += ARG1u(next);

            /* XXXX Probably it is better to teach regpush to support
               parenfloor > maxopenparen ... */
            if (parenfloor > (I32)RXp_LASTPAREN(rex))
                parenfloor = RXp_LASTPAREN(rex); /* Pessimization... */

            ST.prev_curlyx= cur_curlyx;
            cur_curlyx = st;
            ST.cp = PL_savestack_ix;

            /* these fields contain the state of the current curly.
             * they are accessed by subsequent WHILEMs */
            ST.parenfloor = parenfloor;
            ST.me = scan;
            ST.B = next;
            ST.minmod = minmod;
            minmod = 0;
            ST.count = -1;	/* this will be updated by WHILEM */
            ST.lastloc = NULL;  /* this will be updated by WHILEM */

            PUSH_YES_STATE_GOTO(CURLYX_end, REGNODE_BEFORE(next), locinput, loceol,
                                script_run_begin);
            NOT_REACHED; /* NOTREACHED */
        }

        case CURLYX_end: /* just finished matching all of A*B */
            cur_curlyx = ST.prev_curlyx;
            sayYES;
            NOT_REACHED; /* NOTREACHED */

        case CURLYX_end_fail: /* just failed to match all of A*B */
            regcpblow(ST.cp);
            cur_curlyx = ST.prev_curlyx;
            sayNO;
            NOT_REACHED; /* NOTREACHED */


#undef ST
#define ST st->u.whilem

        case WHILEM:     /* just matched an A in /A*B/  (for complex A) */
        {
            /* see the discussion above about CURLYX/WHILEM */
            I32 n;
            int min, max;
            /* U16 first_paren, last_paren; */
            regnode *A;

            assert(cur_curlyx); /* keep Coverity happy */

            min = ARG1i(cur_curlyx->u.curlyx.me);
            max = ARG2i(cur_curlyx->u.curlyx.me);
            /* first_paren = ARG3a(cur_curlyx->u.curlyx.me); */
            /* last_paren = ARG3b(cur_curlyx->u.curlyx.me);  */
            A = REGNODE_AFTER(cur_curlyx->u.curlyx.me);
            n = ++cur_curlyx->u.curlyx.count; /* how many A's matched */
            ST.save_lastloc = cur_curlyx->u.curlyx.lastloc;
            ST.cache_offset = 0;
            ST.cache_mask = 0;

            DEBUG_EXECUTE_r( Perl_re_exec_indentf( aTHX_  "WHILEM: matched %ld out of %d..%d\n",
                  depth, (long)n, min, max)
            );

            /* First just match a string of min A's. */
            if (n < min) {
                ST.cp = regcppush(rex, cur_curlyx->u.curlyx.parenfloor, maxopenparen);
                cur_curlyx->u.curlyx.lastloc = locinput;
                REGCP_SET(ST.lastcp);

                PUSH_STATE_GOTO(WHILEM_A_pre, A, locinput, loceol,
                                script_run_begin);
                NOT_REACHED; /* NOTREACHED */
            }

            /* If degenerate A matches "", assume A done. */

            if (locinput == cur_curlyx->u.curlyx.lastloc) {
                DEBUG_EXECUTE_r( Perl_re_exec_indentf( aTHX_  "WHILEM: empty match detected, trying continuation...\n",
                   depth)
                );
                goto do_whilem_B_max;
            }

            /* super-linear cache processing.
             *
             * The idea here is that for certain types of CURLYX/WHILEM -
             * principally those whose upper bound is infinity (and
             * excluding regexes that have things like \1 and other very
             * non-regular expressiony things), then if a pattern like
             * /....A*.../ fails and we backtrack to the WHILEM, then we
             * make a note that this particular WHILEM op was at string
             * position 47 (say) when the rest of pattern failed. Then, if
             * we ever find ourselves back at that WHILEM, and at string
             * position 47 again, we can just fail immediately rather than
             * running the rest of the pattern again.
             *
             * This is very handy when patterns start to go
             * 'super-linear', like in (a+)*(a+)*(a+)*, where you end up
             * with a combinatorial explosion of backtracking.
             *
             * The cache is implemented as a bit array, with one bit per
             * string byte position per WHILEM op (up to 16) - so its
             * between 0.25 and 2x the string size.
             *
             * To avoid allocating a poscache buffer every time, we do an
             * initially countdown; only after we have  executed a WHILEM
             * op (string-length x #WHILEMs) times do we allocate the
             * cache.
             *
             * The top 4 bits of FLAGS(scan) byte say how many different
             * relevant CURLLYX/WHILEM op pairs there are, while the
             * bottom 4-bits is the identifying index number of this
             * WHILEM.
             */

            if (FLAGS(scan)) {

                if (!reginfo->poscache_maxiter) {
                    /* start the countdown: Postpone detection until we
                     * know the match is not *that* much linear. */
                    reginfo->poscache_maxiter
                        =    (reginfo->strend - reginfo->strbeg + 1)
                           * (FLAGS(scan)>>4);
                    /* possible overflow for long strings and many CURLYX's */
                    if (reginfo->poscache_maxiter < 0)
                        reginfo->poscache_maxiter = I32_MAX;
                    reginfo->poscache_iter = reginfo->poscache_maxiter;
                }

                if (reginfo->poscache_iter-- == 0) {
                    /* initialise cache */
                    const SSize_t size = (reginfo->poscache_maxiter + 7)/8;
                    regmatch_info_aux *const aux = reginfo->info_aux;
                    if (aux->poscache) {
                        if ((SSize_t)reginfo->poscache_size < size) {
                            Renew(aux->poscache, size, char);
                            reginfo->poscache_size = size;
                        }
                        Zero(aux->poscache, size, char);
                    }
                    else {
                        reginfo->poscache_size = size;
                        Newxz(aux->poscache, size, char);
                    }
                    DEBUG_EXECUTE_r( Perl_re_printf( aTHX_
      "%sWHILEM: Detected a super-linear match, switching on caching%s...\n",
                              PL_colors[4], PL_colors[5])
                    );
                }

                if (reginfo->poscache_iter < 0) {
                    /* have we already failed at this position? */
                    SSize_t offset, mask;

                    reginfo->poscache_iter = -1; /* stop eventual underflow */
                    offset  = (FLAGS(scan) & 0xf) - 1
                                +   (locinput - reginfo->strbeg)
                                  * (FLAGS(scan)>>4);
                    mask    = 1 << (offset % 8);
                    offset /= 8;
                    if (reginfo->info_aux->poscache[offset] & mask) {
                        DEBUG_EXECUTE_r( Perl_re_exec_indentf( aTHX_  "WHILEM: (cache) already tried at this position...\n",
                            depth)
                        );
                        cur_curlyx->u.curlyx.count--;
                        sayNO; /* cache records failure */
                    }
                    ST.cache_offset = offset;
                    ST.cache_mask   = mask;
                }
            }

            /* Prefer B over A for minimal matching. */

            if (cur_curlyx->u.curlyx.minmod) {
                ST.save_curlyx = cur_curlyx;
                cur_curlyx = cur_curlyx->u.curlyx.prev_curlyx;
                PUSH_YES_STATE_GOTO(WHILEM_B_min, ST.save_curlyx->u.curlyx.B,
                                    locinput, loceol, script_run_begin);
                NOT_REACHED; /* NOTREACHED */
            }

            /* Prefer A over B for maximal matching. */

            if (n < max) { /* More greed allowed? */
                ST.cp = regcppush(rex, cur_curlyx->u.curlyx.parenfloor,
                            maxopenparen);
                cur_curlyx->u.curlyx.lastloc = locinput;
                REGCP_SET(ST.lastcp);
                PUSH_STATE_GOTO(WHILEM_A_max, A, locinput, loceol,
                                script_run_begin);
                NOT_REACHED; /* NOTREACHED */
            }
            goto do_whilem_B_max;
        }
        NOT_REACHED; /* NOTREACHED */

        case WHILEM_B_min: /* just matched B in a minimal match */
        case WHILEM_B_max: /* just matched B in a maximal match */
            cur_curlyx = ST.save_curlyx;
            sayYES;
            NOT_REACHED; /* NOTREACHED */

        case WHILEM_B_max_fail: /* just failed to match B in a maximal match */
            cur_curlyx = ST.save_curlyx;
            cur_curlyx->u.curlyx.lastloc = ST.save_lastloc;
            cur_curlyx->u.curlyx.count--;
            CACHEsayNO;
            NOT_REACHED; /* NOTREACHED */

        case WHILEM_A_min_fail: /* just failed to match A in a minimal match */
            /* FALLTHROUGH */
        case WHILEM_A_pre_fail: /* just failed to match even minimal A */
            REGCP_UNWIND(ST.lastcp);
            regcppop(rex, &maxopenparen);
            cur_curlyx->u.curlyx.lastloc = ST.save_lastloc;
            cur_curlyx->u.curlyx.count--;
            CACHEsayNO;
            NOT_REACHED; /* NOTREACHED */

        case WHILEM_A_max_fail: /* just failed to match A in a maximal match */
            REGCP_UNWIND(ST.lastcp);
            regcppop(rex, &maxopenparen); /* Restore some previous $<digit>s? */
            DEBUG_EXECUTE_r(Perl_re_exec_indentf( aTHX_  "WHILEM: failed, trying continuation...\n",
                depth)
            );

          do_whilem_B_max:
            /* now try B */
            ST.save_curlyx = cur_curlyx;
            cur_curlyx = cur_curlyx->u.curlyx.prev_curlyx;
            PUSH_YES_STATE_GOTO(WHILEM_B_max, ST.save_curlyx->u.curlyx.B,
                                locinput, loceol, script_run_begin);
            NOT_REACHED; /* NOTREACHED */

        case WHILEM_B_min_fail: /* just failed to match B in a minimal match */
            cur_curlyx = ST.save_curlyx;

            if (cur_curlyx->u.curlyx.count >= /*max*/ARG2i(cur_curlyx->u.curlyx.me)) {
                /* Maximum greed exceeded */
                cur_curlyx->u.curlyx.count--;
                CACHEsayNO;
            }

            DEBUG_EXECUTE_r(Perl_re_exec_indentf( aTHX_  "WHILEM: B min fail: trying longer...\n", depth)
            );
            /* Try grabbing another A and see if it helps. */
            cur_curlyx->u.curlyx.lastloc = locinput;
            ST.cp = regcppush(rex, cur_curlyx->u.curlyx.parenfloor, maxopenparen);
            REGCP_SET(ST.lastcp);
            PUSH_STATE_GOTO(WHILEM_A_min,
                /*A*/ REGNODE_AFTER(ST.save_curlyx->u.curlyx.me),
                locinput, loceol, script_run_begin);
            NOT_REACHED; /* NOTREACHED */

#undef  ST
#define ST st->u.branch

        case BRANCHJ:	    /*  /(...|A|...)/ with long next pointer */
            next = scan + ARG1u(scan);
            if (next == scan)
                next = NULL;
            ST.before_paren = ARG2a(scan);
            ST.after_paren = ARG2b(scan);
            goto branch_logic;
            NOT_REACHED; /* NOTREACHED */

        case BRANCH:	    /*  /(...|A|...)/ */
            ST.before_paren = ARG1a(scan);
            ST.after_paren = ARG1b(scan);
          branch_logic:
            scan = REGNODE_AFTER_opcode(scan,state_num); /* scan now points to inner node */
            assert(scan);
            ST.lastparen = RXp_LASTPAREN(rex);
            ST.lastcloseparen = RXp_LASTCLOSEPAREN(rex);
            ST.next_branch = next;
            REGCP_SET(ST.cp);
            if (RE_PESSIMISTIC_PARENS) {
                regcppush(rex, 0, maxopenparen);
                REGCP_SET(ST.lastcp);
            }

            /* Now go into the branch */
            if (has_cutgroup) {
                PUSH_YES_STATE_GOTO(BRANCH_next, scan, locinput, loceol,
                                    script_run_begin);
            } else {
                PUSH_STATE_GOTO(BRANCH_next, scan, locinput, loceol,
                                script_run_begin);
            }
            NOT_REACHED; /* NOTREACHED */

        case CUTGROUP:  /*  /(*THEN)/  */
            sv_yes_mark = st->u.mark.mark_name = FLAGS(scan)
                ? MUTABLE_SV(rexi->data->data[ ARG1u( scan ) ])
                : NULL;
            PUSH_STATE_GOTO(CUTGROUP_next, next, locinput, loceol,
                            script_run_begin);
            NOT_REACHED; /* NOTREACHED */

        case CUTGROUP_next_fail:
            do_cutgroup = 1;
            no_final = 1;
            if (st->u.mark.mark_name)
                sv_commit = st->u.mark.mark_name;
            sayNO;
            NOT_REACHED; /* NOTREACHED */

        case BRANCH_next:
            sayYES;
            NOT_REACHED; /* NOTREACHED */

        case BRANCH_next_fail: /* that branch failed; try the next, if any */
            if (do_cutgroup) {
                do_cutgroup = 0;
                no_final = 0;
            }
            if (RE_PESSIMISTIC_PARENS) {
                REGCP_UNWIND(ST.lastcp);
                regcppop(rex,&maxopenparen);
            }
            REGCP_UNWIND(ST.cp);
            UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
            CAPTURE_CLEAR(ST.before_paren+1, ST.after_paren, "BRANCH_next_fail");
            scan = ST.next_branch;
            /* no more branches? */
            if (!scan || (OP(scan) != BRANCH && OP(scan) != BRANCHJ)) {
                DEBUG_EXECUTE_r({
                    Perl_re_exec_indentf( aTHX_  "%sBRANCH failed...%s\n",
                        depth,
                        PL_colors[4],
                        PL_colors[5] );
                });
                sayNO_SILENT;
            }
            continue; /* execute next BRANCH[J] op */
            /* NOTREACHED */

        case MINMOD: /* next op will be non-greedy, e.g. A*?  */
            minmod = 1;
            break;

#undef  ST
#define ST st->u.curlym

        case CURLYM:	/* /A{m,n}B/ where A is fixed-length */

            /* This is an optimisation of CURLYX that enables us to push
             * only a single backtracking state, no matter how many matches
             * there are in {m,n}. It relies on the pattern being constant
             * length, with no parens to influence future backrefs
             */

            ST.me = scan;
            scan = REGNODE_AFTER_type(scan, tregnode_CURLYM);

            ST.lastparen      = RXp_LASTPAREN(rex);
            ST.lastcloseparen = RXp_LASTCLOSEPAREN(rex);

            /* if paren positive, emulate an OPEN/CLOSE around A */
            if (FLAGS(ST.me)) {
                U32 paren = FLAGS(ST.me);
                lastopen = paren;
                if (paren > maxopenparen)
                    maxopenparen = paren;
                scan += NEXT_OFF(scan); /* Skip former OPEN. */
            }
            ST.A = scan;
            ST.B = next;
            ST.alen = 0;
            ST.count = 0;
            ST.minmod = minmod;
            minmod = 0;
            ST.Binfo.count = -1;
            REGCP_SET(ST.cp);

            if (!(ST.minmod ? ARG1i(ST.me) : ARG2i(ST.me))) /* min/max */
                goto curlym_do_B;

          curlym_do_A: /* execute the A in /A{m,n}B/  */
            PUSH_YES_STATE_GOTO(CURLYM_A, ST.A, locinput, loceol, /* match A */
                                script_run_begin);
            NOT_REACHED; /* NOTREACHED */

        case CURLYM_A: /* we've just matched an A */
            ST.count++;
            /* after first match, determine A's length: u.curlym.alen */
            if (ST.count == 1) {
                if (reginfo->is_utf8_target) {
                    char *s = st->locinput;
                    while (s < locinput) {
                        ST.alen++;
                        s += UTF8SKIP(s);
                    }
                }
                else {
                    ST.alen = locinput - st->locinput;
                }
                if (ST.alen == 0)
                    ST.count = ST.minmod ? ARG1i(ST.me) : ARG2i(ST.me);
            }
            DEBUG_EXECUTE_r(
                Perl_re_exec_indentf( aTHX_  "CURLYM now matched %" IVdf " times, len=%" IVdf "...\n",
                          depth, (IV) ST.count, (IV)ST.alen)
            );

            if (FLAGS(ST.me)) {
                /* emulate CLOSE: mark current A as captured */
                U32 paren = (U32)FLAGS(ST.me);
                CLOSE_CAPTURE(rex, paren,
                    HOPc(locinput, -ST.alen) - reginfo->strbeg,
                    locinput - reginfo->strbeg);
            }

            if (EVAL_CLOSE_PAREN_IS_TRUE(cur_eval,(U32)FLAGS(ST.me)))
                goto fake_end;


            if (!is_accepted) {
                I32 max = (ST.minmod ? ARG1i(ST.me) : ARG2i(ST.me));
                if ( max == REG_INFTY || ST.count < max )
                    goto curlym_do_A; /* try to match another A */
            }
            goto curlym_do_B; /* try to match B */

        case CURLYM_A_fail: /* just failed to match an A */
            REGCP_UNWIND(ST.cp);


            if (ST.minmod || ST.count < ARG1i(ST.me) /* min*/
                || EVAL_CLOSE_PAREN_IS_TRUE(cur_eval,(U32)FLAGS(ST.me)))
                sayNO;

          curlym_do_B: /* execute the B in /A{m,n}B/  */
            if (is_accepted)
                goto curlym_close_B;

            if (ST.Binfo.count < 0) {
                /* calculate possible match of 1st char following curly */
                assert(ST.B);
                if (HAS_TEXT(ST.B) || JUMPABLE(ST.B)) {
                    regnode *text_node = ST.B;
                    if (! HAS_TEXT(text_node))
                        FIND_NEXT_IMPT(text_node);
                    if (REGNODE_TYPE(OP(text_node)) == EXACT) {
                        if (! S_setup_EXACTISH_ST(aTHX_ text_node,
                                                        &ST.Binfo, reginfo))
                        {
                            sayNO;
                        }
                    }
                }
            }

            DEBUG_EXECUTE_r(
                Perl_re_exec_indentf( aTHX_  "CURLYM trying tail with matches=%" IVdf "...\n",
                    depth, (IV)ST.count)
            );
            if (! NEXTCHR_IS_EOS && ST.Binfo.count >= 0) {
                assert(ST.Binfo.count > 0);

                /* Do a quick test to hopefully rule out most non-matches */
                if (     locinput + ST.Binfo.min_length > loceol
                    || ! S_test_EXACTISH_ST(locinput, ST.Binfo))
                {
                    DEBUG_OPTIMISE_r(
                        Perl_re_exec_indentf( aTHX_
                            "CURLYM Fast bail next target=0x%X anded==0x%X"
                                                                " mask=0x%X\n",
                            depth,
                            (int) nextbyte, ST.Binfo.first_byte_anded,
                                            ST.Binfo.first_byte_mask)
                    );
                    state_num = CURLYM_B_fail;
                    goto reenter_switch;
                }
            }

          curlym_close_B:
            if (FLAGS(ST.me)) {
                /* emulate CLOSE: mark current A as captured */
                U32 paren = (U32)FLAGS(ST.me);
                if (ST.count || is_accepted) {
                    CLOSE_CAPTURE(rex, paren,
                        HOPc(locinput, -ST.alen) - reginfo->strbeg,
                        locinput - reginfo->strbeg);
                }
                else
                    RXp_OFFSp(rex)[paren].end = -1;

                if (EVAL_CLOSE_PAREN_IS_TRUE(cur_eval,(U32)FLAGS(ST.me)))
                {
                    if (ST.count || is_accepted)
                        goto fake_end;
                    else
                        sayNO;
                }
            }

            if (is_accepted)
                goto fake_end;

            PUSH_STATE_GOTO(CURLYM_B, ST.B, locinput, loceol,   /* match B */
                            script_run_begin);
            NOT_REACHED; /* NOTREACHED */

        case CURLYM_B_fail: /* just failed to match a B */
            REGCP_UNWIND(ST.cp);
            UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
            if (ST.minmod) {
                I32 max = ARG2i(ST.me);
                if (max != REG_INFTY && ST.count == max)
                    sayNO;
                goto curlym_do_A; /* try to match a further A */
            }
            /* backtrack one A */
            if (ST.count == ARG1i(ST.me) /* min */)
                sayNO;
            ST.count--;
            SET_locinput(HOPc(locinput, -ST.alen));
            goto curlym_do_B; /* try to match B */

#undef ST
#define ST st->u.curly

#define CURLY_SETPAREN(paren, success)                                      \
    if (paren) {                                                            \
        if (success) {                                                      \
            CLOSE_CAPTURE(rex, paren, HOPc(locinput, -1) - reginfo->strbeg, \
                                 locinput - reginfo->strbeg);               \
        }                                                                   \
        else {                                                              \
            RXp_OFFSp(rex)[paren].end = -1;                                 \
            RXp_LASTPAREN(rex)  = ST.lastparen;                             \
            RXp_LASTCLOSEPAREN(rex) = ST.lastcloseparen;                    \
        }                                                                   \
    }

        case STAR:		/*  /A*B/ where A is width 1 char */
            ST.paren = 0;
            ST.min = 0;
            ST.max = REG_INFTY;
            scan = REGNODE_AFTER_type(scan,tregnode_STAR);
            goto repeat;

        case PLUS:		/*  /A+B/ where A is width 1 char */
            ST.paren = 0;
            ST.min = 1;
            ST.max = REG_INFTY;
            scan = REGNODE_AFTER_type(scan,tregnode_PLUS);
            goto repeat;

        case CURLYN:		/*  /(A){m,n}B/ where A is width 1 char */
            ST.paren = FLAGS(scan);     /* Which paren to set */
            ST.lastparen      = RXp_LASTPAREN(rex);
            ST.lastcloseparen = RXp_LASTCLOSEPAREN(rex);
            if (ST.paren > maxopenparen)
                maxopenparen = ST.paren;
            ST.min = ARG1i(scan);  /* min to match */
            ST.max = ARG2i(scan);  /* max to match */
            scan = regnext(REGNODE_AFTER_type(scan, tregnode_CURLYN));

            /* handle the single-char capture called as a GOSUB etc */
            if (EVAL_CLOSE_PAREN_IS_TRUE(cur_eval,(U32)ST.paren))
            {
                char *li = locinput;
                if (!regrepeat(rex, &li, scan, loceol, reginfo, 1))
                    sayNO;
                SET_locinput(li);
                goto fake_end;
            }

            goto repeat;

        case CURLY:		/*  /A{m,n}B/ where A is width 1 char */
            ST.paren = 0;
            ST.min = ARG1i(scan);  /* min to match */
            ST.max = ARG2i(scan);  /* max to match */
            scan = REGNODE_AFTER_type(scan, tregnode_CURLY);
          repeat:
            /*
            * Lookahead to avoid useless match attempts
            * when we know what character comes next.
            *
            * Used to only do .*x and .*?x, but now it allows
            * for )'s, ('s and (?{ ... })'s to be in the way
            * of the quantifier and the EXACT-like node.  -- japhy
            */

            assert(ST.min <= ST.max);
            if (! HAS_TEXT(next) && ! JUMPABLE(next)) {
                ST.Binfo.count = 0;
            }
            else {
                regnode *text_node = next;

                if (! HAS_TEXT(text_node))
                    FIND_NEXT_IMPT(text_node);

                if (! HAS_TEXT(text_node))
                    ST.Binfo.count = 0;
                else {
                    if ( REGNODE_TYPE(OP(text_node)) != EXACT ) {
                        ST.Binfo.count = 0;
                    }
                    else {
                        if (! S_setup_EXACTISH_ST(aTHX_ text_node,
                                                        &ST.Binfo, reginfo))
                        {
                            sayNO;
                        }
                    }
                }
            }

            ST.A = scan;
            ST.B = next;
            if (minmod) {
                char *li = locinput;
                minmod = 0;
                if (ST.min &&
                        regrepeat(rex, &li, ST.A, loceol, reginfo, ST.min)
                            < ST.min)
                    sayNO;
                SET_locinput(li);
                ST.count = ST.min;
                REGCP_SET(ST.cp);

                if (ST.Binfo.count <= 0)
                    goto curly_try_B_min;

                ST.oldloc = locinput;

                /* set ST.maxpos to the furthest point along the
                 * string that could possibly match, i.e., that a match could
                 * start at. */
                if  (ST.max == REG_INFTY) {
                    ST.maxpos = loceol - 1;
                    if (utf8_target)
                        while (UTF8_IS_CONTINUATION(*(U8*)ST.maxpos))
                            ST.maxpos--;
                }
                else if (utf8_target) {
                    int m = ST.max - ST.min;
                    for (ST.maxpos = locinput;
                         m >0 && ST.maxpos <  loceol; m--)
                        ST.maxpos += UTF8SKIP(ST.maxpos);
                }
                else {
                    ST.maxpos = locinput + ST.max - ST.min;
                    if (ST.maxpos >=  loceol)
                        ST.maxpos =  loceol - 1;
                }
                goto curly_try_B_min_known;

            }
            else {
                /* avoid taking address of locinput, so it can remain
                 * a register var */
                char *li = locinput;
                if (ST.max)
                    ST.count = regrepeat(rex, &li, ST.A, loceol, reginfo, ST.max);
                else
                    ST.count = 0;
                if (ST.count < ST.min)
                    sayNO;
                SET_locinput(li);
                if ((ST.count > ST.min)
                    && (REGNODE_TYPE(OP(ST.B)) == EOL) && (OP(ST.B) != MEOL))
                {
                    /* A{m,n} must come at the end of the string, there's
                     * no point in backing off ... */
                    ST.min = ST.count;
                    /* ...except that $ and \Z can match before *and* after
                       newline at the end.  Consider "\n\n" =~ /\n+\Z\n/.
                       We may back off by one in this case. */
                    if (UCHARAT(locinput - 1) == '\n' && OP(ST.B) != EOS)
                        ST.min--;
                }
                REGCP_SET(ST.cp);
                goto curly_try_B_max;
            }
            NOT_REACHED; /* NOTREACHED */

        case CURLY_B_min_fail:
            /* failed to find B in a non-greedy match. */
            if (RE_PESSIMISTIC_PARENS) {
                REGCP_UNWIND(ST.lastcp);
                regcppop(rex, &maxopenparen); /* Restore some previous $<digit>s? */
            }
            REGCP_UNWIND(ST.cp);
            if (ST.paren) {
                UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
            }

            if (ST.Binfo.count == 0) {
                /* failed -- move forward one */
                char *li = locinput;
                if (!regrepeat(rex, &li, ST.A, loceol, reginfo, 1)) {
                    sayNO;
                }
                locinput = li;
                ST.count++;
                if (!(   ST.count <= ST.max
                        /* count overflow ? */
                     || (ST.max == REG_INFTY && ST.count > 0))
                )
                    sayNO;
            }
            else {
                int n;
                /* Couldn't or didn't -- move forward. */
                ST.oldloc = locinput;
                if (utf8_target)
                    locinput += UTF8SKIP(locinput);
                else
                    locinput++;
                ST.count++;

              curly_try_B_min_known:
                /* find the next place where 'B' could work, then call B */
                if (locinput + ST.Binfo.initial_exact < loceol) {
                    if (ST.Binfo.initial_exact >= ST.Binfo.max_length) {

                        /* Here, the mask is all 1's for the entire length of
                         * any possible match.  (That actually means that there
                         * is only one possible match.)  Look for the next
                         * occurrence */
                        locinput = ninstr(locinput, loceol,
                                        (char *) ST.Binfo.matches,
                                        (char *) ST.Binfo.matches
                                                    + ST.Binfo.initial_exact);
                        if (locinput == NULL) {
                            sayNO;
                        }
                    }
                    else do {
                        /* If the first byte(s) of the mask are all ones, it
                         * means those bytes must match identically, so can use
                         * ninstr() to find the next possible matchpoint */
                        if (ST.Binfo.initial_exact > 0) {
                            locinput = ninstr(locinput, loceol,
                                              (char *) ST.Binfo.matches,
                                              (char *) ST.Binfo.matches
                                                     + ST.Binfo.initial_exact);
                        }
                        else { /* Otherwise find the next byte that matches,
                                  masked */
                            locinput = (char *) find_next_masked(
                                                (U8 *) locinput, (U8 *) loceol,
                                                ST.Binfo.first_byte_anded,
                                                ST.Binfo.first_byte_mask);
                            /* Advance to the end of a multi-byte character */
                            if (utf8_target) {
                                while (   locinput < loceol
                                    && UTF8_IS_CONTINUATION(*locinput))
                                {
                                    locinput++;
                                }
                            }
                        }
                        if (   locinput == NULL
                            || locinput + ST.Binfo.min_length > loceol)
                        {
                            sayNO;
                        }

                        /* Here, we have found a possible match point; if can't
                         * rule it out, quit the loop so can check fully */
                        if (S_test_EXACTISH_ST(locinput, ST.Binfo)) {
                            break;
                        }

                        locinput += (utf8_target) ? UTF8SKIP(locinput) : 1;

                    } while (locinput <= ST.maxpos);
                }

                if (locinput > ST.maxpos)
                    sayNO;

                n = (utf8_target)
                    ? utf8_length((U8 *) ST.oldloc, (U8 *) locinput)
                    : (STRLEN) (locinput - ST.oldloc);


                /* Here is at the beginning of a character that meets the mask
                 * criteria.  Need to make sure that some real possibility */

                if (n) {
                    /* In /a{m,n}b/, ST.oldloc is at "a" x m, locinput is
                     * at what may be the beginning of b; check that everything
                     * between oldloc and locinput matches */
                    char *li = ST.oldloc;
                    ST.count += n;
                    if (regrepeat(rex, &li, ST.A, loceol, reginfo, n) < n)
                        sayNO;
                    assert(n == REG_INFTY || locinput == li);
                }
            }

          curly_try_B_min:
            if (RE_PESSIMISTIC_PARENS) {
                (void)regcppush(rex, 0, maxopenparen);
                REGCP_SET(ST.lastcp);
            }
            CURLY_SETPAREN(ST.paren, ST.count);
            PUSH_STATE_GOTO(CURLY_B_min, ST.B, locinput, loceol,
                            script_run_begin);
            NOT_REACHED; /* NOTREACHED */


          curly_try_B_max:
            /* a successful greedy match: now try to match B */
            if (        ST.Binfo.count <= 0
                || (    ST.Binfo.count > 0
                    &&  locinput + ST.Binfo.min_length <= loceol
                    &&  S_test_EXACTISH_ST(locinput, ST.Binfo)))
            {
                if (RE_PESSIMISTIC_PARENS) {
                    (void)regcppush(rex, 0, maxopenparen);
                    REGCP_SET(ST.lastcp);
                }
                CURLY_SETPAREN(ST.paren, ST.count);
                PUSH_STATE_GOTO(CURLY_B_max, ST.B, locinput, loceol,
                                script_run_begin);
                NOT_REACHED; /* NOTREACHED */
            }
            goto CURLY_B_all_failed;
            NOT_REACHED; /* NOTREACHED */

        case CURLY_B_max_fail:
            /* failed to find B in a greedy match */

            if (RE_PESSIMISTIC_PARENS) {
                REGCP_UNWIND(ST.lastcp);
                regcppop(rex, &maxopenparen); /* Restore some previous $<digit>s? */
            }
          CURLY_B_all_failed:
            REGCP_UNWIND(ST.cp);
            if (ST.paren) {
                UNWIND_PAREN(ST.lastparen, ST.lastcloseparen);
            }
            /*  back up. */
            if (--ST.count < ST.min)
                sayNO;
            locinput = HOPc(locinput, -1);
            goto curly_try_B_max;

#undef ST

        case END: /*  last op of main pattern  */
          fake_end:
            if (cur_eval) {
                /* we've just finished A in /(??{A})B/; now continue with B */
                is_accepted= false;
                SET_RECURSE_LOCINPUT("FAKE-END[before]", CUR_EVAL.prev_recurse_locinput);
                st->u.eval.prev_rex = rex_sv;		/* inner */

                /* Save *all* the positions. */
                st->u.eval.cp = regcppush(rex, 0, maxopenparen);
                rex_sv = CUR_EVAL.prev_rex;
                is_utf8_pat = reginfo->is_utf8_pat = cBOOL(RX_UTF8(rex_sv));
                SET_reg_curpm(rex_sv);
                rex = ReANY(rex_sv);
                rexi = RXi_GET(rex);

                st->u.eval.prev_curlyx = cur_curlyx;
                cur_curlyx = CUR_EVAL.prev_curlyx;

                REGCP_SET(st->u.eval.lastcp);

                /* Restore parens of the outer rex without popping the
                 * savestack */
                regcp_restore(rex, CUR_EVAL.lastcp, &maxopenparen);

                st->u.eval.prev_eval = cur_eval;
                cur_eval = CUR_EVAL.prev_eval;
                DEBUG_EXECUTE_r(
                    Perl_re_exec_indentf( aTHX_  "END: EVAL trying tail ... (cur_eval=%p)\n",
                                      depth, cur_eval););
                if ( nochange_depth )
                    nochange_depth--;

                SET_RECURSE_LOCINPUT("FAKE-END[after]", cur_eval->locinput);

                PUSH_YES_STATE_GOTO(EVAL_postponed_AB,          /* match B */
                                    st->u.eval.prev_eval->u.eval.B,
                                    locinput, loceol, script_run_begin);
            }

            if (locinput < reginfo->till) {
                DEBUG_EXECUTE_r(Perl_re_printf( aTHX_
                                      "%sEND: Match possible, but length=%ld is smaller than requested=%ld, failing!%s\n",
                                      PL_colors[4],
                                      (long)(locinput - startpos),
                                      (long)(reginfo->till - startpos),
                                      PL_colors[5]));

                sayNO_SILENT;		/* Cannot match: too short. */
            }
            sayYES;			/* Success! */

        case LOOKBEHIND_END: /* validate that *lookbehind* UNLESSM/IFMATCH
                                matches end at the right spot, required for
                                variable length matches. */
            if (match_end && locinput != match_end)
            {
                DEBUG_EXECUTE_r(
                Perl_re_exec_indentf( aTHX_
                    "%sLOOKBEHIND_END: subpattern failed...%s\n",
                    depth, PL_colors[4], PL_colors[5]));
                sayNO;            /* Variable length match didn't line up */
            }
            /* FALLTHROUGH */

        case SUCCEED: /* successful SUSPEND/CURLYM and
                                            *lookahead* IFMATCH/UNLESSM*/
            DEBUG_EXECUTE_r(
            Perl_re_exec_indentf( aTHX_
                "%sSUCCEED: subpattern success...%s\n",
                depth, PL_colors[4], PL_colors[5]));
            sayYES;			/* Success! */

#undef  ST
#define ST st->u.ifmatch

        case SUSPEND:	/* (?>A) */
            ST.wanted = 1;
            ST.start = locinput;
            ST.end = loceol;
            ST.count = 1;
            goto do_ifmatch;

        case UNLESSM:	/* -ve lookaround: (?!A), or with 'flags', (?<!A) */
            ST.wanted = 0;
            goto ifmatch_trivial_fail_test;

        case IFMATCH:	/* +ve lookaround: (?=A), or with 'flags', (?<=A) */
            ST.wanted = 1;
          ifmatch_trivial_fail_test:
            ST.prev_match_end= match_end;
            ST.count = NEXT_OFF(scan) + 1; /* next_off repurposed to be
                                              lookbehind count, requires
                                              non-zero flags */
            if (! FLAGS(scan)) {    /* 'flags' zero means lookahed */

                /* Lookahead starts here and ends at the normal place */
                ST.start = locinput;
                ST.end = loceol;
                match_end = NULL;
            }
            else {
                PERL_UINT_FAST8_T back_count = FLAGS(scan);
                char * s;
                match_end = locinput;

                /* Lookbehind can look beyond the current position */
                ST.end = loceol;

                /* ... and starts at the first place in the input that is in
                 * the range of the possible start positions */
                for (; ST.count > 0; ST.count--, back_count--) {
                    s = HOPBACKc(locinput, back_count);
                    if (s) {
                        ST.start = s;
                        goto do_ifmatch;
                    }
                }

                /* If the lookbehind doesn't start in the actual string, is a
                 * trivial match failure */
                match_end = ST.prev_match_end;
                if (logical) {
                    logical = 0;
                    sw = 1 - cBOOL(ST.wanted);
                }
                else if (ST.wanted)
                    sayNO;

                /* Here, we didn't want it to match, so is actually success */
                next = scan + ARG1u(scan);
                if (next == scan)
                    next = NULL;
                break;
            }

          do_ifmatch:
            ST.me = scan;
            ST.logical = logical;
            logical = 0; /* XXX: reset state of logical once it has been saved into ST */

            /* execute body of (?...A) */
            PUSH_YES_STATE_GOTO(IFMATCH_A, REGNODE_AFTER(scan), ST.start,
                                ST.end, script_run_begin);
            NOT_REACHED; /* NOTREACHED */

        {
            bool matched;

        case IFMATCH_A_fail: /* body of (?...A) failed */
            if (! ST.logical && ST.count > 1) {

                /* It isn't a real failure until we've tried all starting
                 * positions.  Move to the next starting position and retry */
                ST.count--;
                ST.start = HOPc(ST.start, 1);
                scan = ST.me;
                logical = ST.logical;
                goto do_ifmatch;
            }

            /* Here, all starting positions have been tried. */
            matched = FALSE;
            goto ifmatch_done;

        case IFMATCH_A: /* body of (?...A) succeeded */
            matched = TRUE;
          ifmatch_done:
            sw = matched == ST.wanted;
            match_end = ST.prev_match_end;
            if (! ST.logical && !sw) {
                sayNO;
            }

            if (OP(ST.me) != SUSPEND) {
                /* restore old position except for (?>...) */
                locinput = st->locinput;
                loceol = st->loceol;
                script_run_begin = st->sr0;
            }
            scan = ST.me + ARG1u(ST.me);
            if (scan == ST.me)
                scan = NULL;
            continue; /* execute B */
        }

#undef ST

        case LONGJMP: /*  alternative with many branches compiles to
                       * (BRANCHJ; EXACT ...; LONGJMP ) x N */
            next = scan + ARG1u(scan);
            if (next == scan)
                next = NULL;
            break;

        case COMMIT:  /*  (*COMMIT)  */
            reginfo->cutpoint = loceol;
            /* FALLTHROUGH */

        case PRUNE:   /*  (*PRUNE)   */
            if (FLAGS(scan))
                sv_yes_mark = sv_commit = MUTABLE_SV(rexi->data->data[ ARG1u( scan ) ]);
            PUSH_STATE_GOTO(COMMIT_next, next, locinput, loceol,
                            script_run_begin);
            NOT_REACHED; /* NOTREACHED */

        case COMMIT_next_fail:
            no_final = 1;
            /* FALLTHROUGH */
            sayNO;
            NOT_REACHED; /* NOTREACHED */

        case OPFAIL:   /* (*FAIL)  */
            if (FLAGS(scan))
                sv_commit = MUTABLE_SV(rexi->data->data[ ARG1u( scan ) ]);
            if (logical) {
                /* deal with (?(?!)X|Y) properly,
                 * make sure we trigger the no branch
                 * of the trailing IFTHEN structure*/
                sw= 0;
                break;
            } else {
                sayNO;
            }
            NOT_REACHED; /* NOTREACHED */

#define ST st->u.mark
        case MARKPOINT: /*  (*MARK:foo)  */
            ST.prev_mark = mark_state;
            ST.mark_name = sv_commit = sv_yes_mark
                = MUTABLE_SV(rexi->data->data[ ARG1u( scan ) ]);
            mark_state = st;
            ST.mark_loc = locinput;
            PUSH_YES_STATE_GOTO(MARKPOINT_next, next, locinput, loceol,
                                script_run_begin);
            NOT_REACHED; /* NOTREACHED */

        case MARKPOINT_next:
            mark_state = ST.prev_mark;
            sayYES;
            NOT_REACHED; /* NOTREACHED */

        case MARKPOINT_next_fail:
            if (popmark && sv_eq(ST.mark_name,popmark))
            {
                if (ST.mark_loc > startpoint)
                    reginfo->cutpoint = HOPBACKc(ST.mark_loc, 1);
                popmark = NULL; /* we found our mark */
                sv_commit = ST.mark_name;

                DEBUG_EXECUTE_r({
                        Perl_re_exec_indentf( aTHX_  "%sMARKPOINT: next fail: setting cutpoint to mark:%" SVf "...%s\n",
                            depth,
                            PL_colors[4], SVfARG(sv_commit), PL_colors[5]);
                });
            }
            mark_state = ST.prev_mark;
            sv_yes_mark = mark_state ?
                mark_state->u.mark.mark_name : NULL;
            sayNO;
            NOT_REACHED; /* NOTREACHED */

        case SKIP:  /*  (*SKIP)  */
            if (!FLAGS(scan)) {
                /* (*SKIP) : if we fail we cut here*/
                ST.mark_name = NULL;
                ST.mark_loc = locinput;
                PUSH_STATE_GOTO(SKIP_next,next, locinput, loceol,
                                script_run_begin);
            } else {
                /* (*SKIP:NAME) : if there is a (*MARK:NAME) fail where it was,
                   otherwise do nothing.  Meaning we need to scan
                 */
                regmatch_state *cur = mark_state;
                SV *find = MUTABLE_SV(rexi->data->data[ ARG1u( scan ) ]);

                while (cur) {
                    if ( sv_eq( cur->u.mark.mark_name,
                                find ) )
                    {
                        ST.mark_name = find;
                        PUSH_STATE_GOTO( SKIP_next, next, locinput, loceol,
                                         script_run_begin);
                    }
                    cur = cur->u.mark.prev_mark;
                }
            }
            /* Didn't find our (*MARK:NAME) so ignore this (*SKIP:NAME) */
            break;

        case SKIP_next_fail:
            if (ST.mark_name) {
                /* (*CUT:NAME) - Set up to search for the name as we
                   collapse the stack*/
                popmark = ST.mark_name;
            } else {
                /* (*CUT) - No name, we cut here.*/
                if (ST.mark_loc > startpoint)
                    reginfo->cutpoint = HOPBACKc(ST.mark_loc, 1);
                /* but we set sv_commit to latest mark_name if there
                   is one so they can test to see how things lead to this
                   cut */
                if (mark_state)
                    sv_commit=mark_state->u.mark.mark_name;
            }
            no_final = 1;
            sayNO;
            NOT_REACHED; /* NOTREACHED */
#undef ST

        case LNBREAK: /* \R */
            if ((n=is_LNBREAK_safe(locinput, loceol, utf8_target))) {
                locinput += n;
            } else
                sayNO;
            break;

        default:
            PerlIO_printf(Perl_error_log, "%" UVxf " %d\n",
                          PTR2UV(scan), OP(scan));
            Perl_croak(aTHX_ "regexp memory corruption");

        /* this is a point to jump to in order to increment
         * locinput by one character */
          increment_locinput:
            assert(!NEXTCHR_IS_EOS);
            if (utf8_target) {
                locinput += PL_utf8skip[nextbyte];
                /* locinput is allowed to go 1 char off the end (signifying
                 * EOS), but not 2+ */
                if (locinput >  loceol)
                    sayNO;
            }
            else
                locinput++;
            break;

        } /* end switch */

        /* switch break jumps here */
        scan = next; /* prepare to execute the next op and ... */
        continue;    /* ... jump back to the top, reusing st */
        /* NOTREACHED */

      push_yes_state:
        /* push a state that backtracks on success */
        st->u.yes.prev_yes_state = yes_state;
        yes_state = st;
        /* FALLTHROUGH */
      push_state:
        /* push a new regex state, then continue at scan  */
        {
            regmatch_state *newst;
            DECLARE_AND_GET_RE_DEBUG_FLAGS;

            DEBUG_r( /* DEBUG_STACK_r */
              if (DEBUG_v_TEST || RE_DEBUG_FLAG(RE_DEBUG_EXTRA_STACK)) {
                regmatch_state *cur = st;
                regmatch_state *curyes = yes_state;
                U32 i;
                regmatch_slab *slab = PL_regmatch_slab;
                for (i = 0; i < 3 && i <= depth; cur--,i++) {
                    if (cur < SLAB_FIRST(slab)) {
                        slab = slab->prev;
                        cur = SLAB_LAST(slab);
                    }
                    Perl_re_exec_indentf( aTHX_ "%4s #%-3d %-10s %s\n",
                        depth,
                        i ? "    " : "push",
                        depth - i, REGNODE_NAME(cur->resume_state),
                        (curyes == cur) ? "yes" : ""
                    );
                    if (curyes == cur)
                        curyes = cur->u.yes.prev_yes_state;
                }
            } else {
                DEBUG_STATE_pp("push")
            });
            depth++;
            st->locinput = locinput;
            st->loceol = loceol;
            st->sr0 = script_run_begin;
            newst = st+1;
            if (newst >  SLAB_LAST(PL_regmatch_slab))
                newst = S_push_slab(aTHX);
            PL_regmatch_state = newst;

            locinput = pushinput;
            loceol = pusheol;
            script_run_begin = pushsr0;
            st = newst;
            continue;
            /* NOTREACHED */
        }
    }
#ifdef SOLARIS_BAD_OPTIMIZER
#  undef PL_charclass
#endif

    /*
    * We get here only if there's trouble -- normally "case END" is
    * the terminating point.
    */
    Perl_croak(aTHX_ "corrupted regexp pointers");
    NOT_REACHED; /* NOTREACHED */

  yes:
    if (yes_state) {
        /* we have successfully completed a subexpression, but we must now
         * pop to the state marked by yes_state and continue from there */
        assert(st != yes_state);
#ifdef DEBUGGING
        while (st != yes_state) {
            st--;
            if (st < SLAB_FIRST(PL_regmatch_slab)) {
                PL_regmatch_slab = PL_regmatch_slab->prev;
                st = SLAB_LAST(PL_regmatch_slab);
            }
            DEBUG_STATE_r({
                if (no_final) {
                    DEBUG_STATE_pp("pop (no final)");
                } else {
                    DEBUG_STATE_pp("pop (yes)");
                }
            });
            depth--;
        }
#else
        while (yes_state < SLAB_FIRST(PL_regmatch_slab)
            || yes_state > SLAB_LAST(PL_regmatch_slab))
        {
            /* not in this slab, pop slab */
            depth -= (st - SLAB_FIRST(PL_regmatch_slab) + 1);
            PL_regmatch_slab = PL_regmatch_slab->prev;
            st = SLAB_LAST(PL_regmatch_slab);
        }
        depth -= (st - yes_state);
#endif
        st = yes_state;
        yes_state = st->u.yes.prev_yes_state;
        PL_regmatch_state = st;

        if (no_final) {
            locinput= st->locinput;
            loceol= st->loceol;
            script_run_begin = st->sr0;
        }
        state_num = st->resume_state + no_final;
        goto reenter_switch;
    }

    DEBUG_EXECUTE_r(Perl_re_printf( aTHX_  "%sMatch successful!%s\n",
                          PL_colors[4], PL_colors[5]));

    if (reginfo->info_aux_eval) {
        /* each successfully executed (?{...}) block does the equivalent of
         *   local $^R = do {...}
         * When popping the save stack, all these locals would be undone;
         * bypass this by setting the outermost saved $^R to the latest
         * value */
        /* I don't know if this is needed or works properly now.
         * see code related to PL_replgv elsewhere in this file.
         * Yves
         */
        if (oreplsv != GvSV(PL_replgv)) {
            sv_setsv(oreplsv, GvSV(PL_replgv));
            SvSETMAGIC(oreplsv);
        }
    }
    result = 1;
    goto final_exit;

  no:
    DEBUG_EXECUTE_r(
        Perl_re_exec_indentf( aTHX_  "%sfailed...%s\n",
            depth,
            PL_colors[4], PL_colors[5])
        );

  no_silent:
    if (no_final) {
        if (yes_state) {
            goto yes;
        } else {
            goto final_exit;
        }
    }
    if (depth) {
        /* there's a previous state to backtrack to */
        st--;
        if (st < SLAB_FIRST(PL_regmatch_slab)) {
            PL_regmatch_slab = PL_regmatch_slab->prev;
            st = SLAB_LAST(PL_regmatch_slab);
        }
        PL_regmatch_state = st;
        locinput= st->locinput;
        loceol= st->loceol;
        script_run_begin = st->sr0;

        DEBUG_STATE_pp("pop");
        depth--;
        if (yes_state == st)
            yes_state = st->u.yes.prev_yes_state;

        state_num = st->resume_state + 1; /* failure = success + 1 */
        PERL_ASYNC_CHECK();
        goto reenter_switch;
    }
    result = 0;

  final_exit:
    if (rex->intflags & PREGf_VERBARG_SEEN) {
        SV *sv_err = get_sv("REGERROR", 1);
        SV *sv_mrk = get_sv("REGMARK", 1);
        if (result) {
            sv_commit = &PL_sv_no;
            if (!sv_yes_mark)
                sv_yes_mark = &PL_sv_yes;
        } else {
            if (!sv_commit)
                sv_commit = &PL_sv_yes;
            sv_yes_mark = &PL_sv_no;
        }
        assert(sv_err);
        assert(sv_mrk);
        sv_setsv(sv_err, sv_commit);
        sv_setsv(sv_mrk, sv_yes_mark);
    }


    if (last_pushed_cv) {
        dSP;
        /* see "Some notes about MULTICALL" above */
        POP_MULTICALL;
        PERL_UNUSED_VAR(SP);
    }
    else
        LEAVE_SCOPE(orig_savestack_ix);

    assert(!result ||  locinput - reginfo->strbeg >= 0);
    return result ?  locinput - reginfo->strbeg : -1;
}

/*
 - regrepeat - repeatedly match something simple, report how many
 *
 * What 'simple' means is a node which can be the operand of a quantifier like
 * '+', or {1,3}
 *
 * startposp - pointer to a pointer to the start position.  This is updated
 *             to point to the byte following the highest successful
 *             match.
 * p         - the regnode to be repeatedly matched against.
 * loceol    - pointer to the end position beyond which we aren't supposed to
 *             look.
 * reginfo   - struct holding match state, such as utf8_target
 * max       - maximum number of things to match.
 * depth     - (for debugging) backtracking depth.
 */
STATIC I32
S_regrepeat(pTHX_ regexp *prog, char **startposp, const regnode *p,
            char * loceol, regmatch_info *const reginfo, I32 max comma_pDEPTH)
{
    char *scan;     /* Pointer to current position in target string */
    I32 c;
    char *this_eol = loceol;   /* potentially adjusted version. */
    I32 hardcount = 0;  /* How many matches so far */
    bool utf8_target = reginfo->is_utf8_target;
    unsigned int to_complement = 0;  /* Invert the result? */
    char_class_number_ classnum;

    PERL_ARGS_ASSERT_REGREPEAT;

    /* This routine is structured so that we switch on the input OP.  Each OP
     * case: statement contains a loop to repeatedly apply the OP, advancing
     * the input until it fails, or reaches the end of the input, or until it
     * reaches the upper limit of matches. */

    scan = *startposp;
    if (max == REG_INFTY)   /* This is a special marker to go to the platform's
                               max */
        max = I32_MAX;
    else if (! utf8_target && this_eol - scan > max)
        this_eol = scan + max;

    /* Here, for the case of a non-UTF-8 target we have adjusted <this_eol>
     * down to the maximum of how far we should go in it (but leaving it set to
     * the real end if the maximum permissible would take us beyond that).
     * This allows us to make the loop exit condition that we haven't gone past
     * <this_eol> to also mean that we haven't exceeded the max permissible
     * count, saving a test each time through the loop.  But it assumes that
     * the OP matches a single byte, which is true for most of the OPs below
     * when applied to a non-UTF-8 target.  Those relatively few OPs that don't
     * have this characteristic have to compensate.
     *
     * There is no such adjustment for UTF-8 targets, since the number of bytes
     * per character can vary.  OPs will have to test both that the count is
     * less than the max permissible (using <hardcount> to keep track), and
     * that we are still within the bounds of the string (using <this_eol>.  A
     * few OPs match a single byte no matter what the encoding.  They can omit
     * the max test if, for the UTF-8 case, they do the adjustment that was
     * skipped above.
     *
     * Thus, the code above sets things up for the common case; and exceptional
     * cases need extra work; the common case is to make sure <scan> doesn't go
     * past <this_eol>, and for UTF-8 to also use <hardcount> to make sure the
     * count doesn't exceed the maximum permissible */

    switch (with_t_UTF8ness(OP(p), utf8_target)) {
        SV * anyofh_list;

      case REG_ANY_t8:
        while (scan < this_eol && hardcount < max && *scan != '\n') {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case REG_ANY_tb:
        scan = (char *) memchr(scan, '\n', this_eol - scan);
        if (! scan) {
            scan = this_eol;
        }
        break;

      case SANY_t8:
        while (scan < this_eol && hardcount < max) {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case SANY_tb:
        scan = this_eol;
        break;

      case EXACT_REQ8_tb:
      case LEXACT_REQ8_tb:
      case EXACTFU_REQ8_tb:
        break;

      case EXACTL_t8:
        if (UTF8_IS_ABOVE_LATIN1(*scan)) {
            _CHECK_AND_OUTPUT_WIDE_LOCALE_UTF8_MSG(scan, loceol);
        }
        /* FALLTHROUGH */

      case EXACTL_tb:
      case EXACTFL_t8:
      case EXACTFL_tb:
      case EXACTFLU8_t8:
      case EXACTFLU8_tb:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        /* FALLTHROUGH */

      case EXACT_REQ8_t8:
      case LEXACT_REQ8_t8:
      case EXACTFU_REQ8_t8:
      case LEXACT_t8:
      case LEXACT_tb:
      case EXACT_t8:
      case EXACT_tb:
      case EXACTF_t8:
      case EXACTF_tb:
      case EXACTFAA_NO_TRIE_t8:
      case EXACTFAA_NO_TRIE_tb:
      case EXACTFAA_t8:
      case EXACTFAA_tb:
      case EXACTFU_t8:
      case EXACTFU_tb:
      case EXACTFUP_t8:
      case EXACTFUP_tb:

      {
        struct next_matchable_info Binfo;
        PERL_UINT_FAST8_T definitive_len;

        assert(STR_LEN(p) == reginfo->is_utf8_pat ? UTF8SKIP(STRING(p)) : 1);

        /* Set up termination info, and quit if we can rule out that we've
         * gotten a match of the termination criteria */
        if (   ! S_setup_EXACTISH_ST(aTHX_ p, &Binfo, reginfo)
            ||   scan + Binfo.min_length > this_eol
            || ! S_test_EXACTISH_ST(scan, Binfo))
        {
            break;
        }

        definitive_len = Binfo.initial_definitive;

        /* Here there are potential matches, and the first byte(s) matched our
         * filter
         *
         * If we got a definitive match of some initial bytes, there is no
         * possibility of false positives as far as it got */
        if (definitive_len > 0) {

            /* If as far as it got is the maximum possible, there were no false
             * positives at all.  Since we have everything set up, see how many
             * repeats there are. */
            if (definitive_len >= Binfo.max_length) {

                /* We've already found one match */
                scan += definitive_len;
                hardcount++;

                /* If want more than the one match, and there is room for more,
                 * see if there are any */
                if (hardcount < max && scan + definitive_len <= this_eol) {

                    /* If the character is only a single byte long, just span
                     * all such bytes. */
                    if (definitive_len == 1) {
                        const char * orig_scan = scan;

                        if (this_eol - (scan - hardcount) > max) {
                            this_eol = scan - hardcount + max;
                        }

                        /* Use different routines depending on whether it's an
                         * exact match or matches with a mask */
                        if (Binfo.initial_exact == 1) {
                            scan = (char *) find_span_end((U8 *) scan,
                                                          (U8 *) this_eol,
                                                          Binfo.matches[0]);
                        }
                        else {
                            scan = (char *) find_span_end_mask(
                                                       (U8 *) scan,
                                                       (U8 *) this_eol,
                                                       Binfo.first_byte_anded,
                                                       Binfo.first_byte_mask);
                        }

                        hardcount += scan - orig_scan;
                    }
                    else { /* Here, the full character definitive match is more
                              than one byte */
                        while (   hardcount < max
                               && scan + definitive_len <= this_eol
                               && S_test_EXACTISH_ST(scan, Binfo))
                        {
                                scan += definitive_len;
                                hardcount++;
                        }
                    }
                }

                break;
            }   /* End of a full character is definitively matched */

            /* Here, an initial portion of the character matched definitively,
             * and the rest matched as well, but could have false positives */

            do {
                int i;
                U8 * matches = Binfo.matches;

                /* The first bytes were definitive.  Look at the remaining */
                for (i = 0; i < Binfo.count; i++) {
                    if (memEQ(scan + definitive_len,
                              matches + definitive_len,
                              Binfo.lengths[i] - definitive_len))
                    {
                        goto found_a_completion;
                    }

                    matches += Binfo.lengths[i];
                }

                /* Didn't find anything to complete our initial match.  Stop
                 * here */
                break;

              found_a_completion:

                /* Here, matched a full character, Include it in the result,
                 * and then look to see if the next char matches */
                hardcount++;
                scan += Binfo.lengths[i];

            } while (   hardcount < max
                     && scan + definitive_len < this_eol
                     && S_test_EXACTISH_ST(scan, Binfo));

            /* Here, have advanced as far as possible */
            break;
        } /* End of found some initial bytes that definitively matched */

        /* Here, we can't rule out that we have found the beginning of 'B', but
         * there were no initial bytes that could rule out anything
         * definitively. Use brute force to examine all the possibilities */
        while (scan < this_eol && hardcount < max) {
            int i;
            U8 * matches = Binfo.matches;

            for (i = 0; i < Binfo.count; i++) {
                if (memEQ(scan, matches, Binfo.lengths[i])) {
                    goto found1;
                }

                matches += Binfo.lengths[i];
            }

            break;

          found1:
            hardcount++;
            scan += Binfo.lengths[i];
        }

        break;
      }

      case ANYOFPOSIXL_t8:
      case ANYOFL_t8:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_SETS(p);

        /* FALLTHROUGH */
      case ANYOFD_t8:
      case ANYOF_t8:
        while (   hardcount < max
               && scan < this_eol
               && reginclass(prog, p, (U8*)scan, (U8*) this_eol, TRUE))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case ANYOFPOSIXL_tb:
      case ANYOFL_tb:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        CHECK_AND_WARN_NON_UTF8_CTYPE_LOCALE_IN_SETS(p);
        /* FALLTHROUGH */

      case ANYOFD_tb:
      case ANYOF_tb:
        if (ANYOF_FLAGS(p) || ANYOF_HAS_AUX(p)) {
            while (   scan < this_eol
                   && reginclass(prog, p, (U8*)scan, (U8*)scan+1, 0))
                scan++;
        }
        else {
            while (scan < this_eol && ANYOF_BITMAP_TEST(p, *((U8*)scan)))
                scan++;
        }
        break;

      case ANYOFM_t8:
        if (this_eol - scan > max) {

            /* We didn't adjust <this_eol> at the beginning of this routine
             * because is UTF-8, but it is actually ok to do so, since here, to
             * match, 1 char == 1 byte. */
            this_eol = scan + max;
        }
        /* FALLTHROUGH */

      case ANYOFM_tb:
        scan = (char *) find_span_end_mask((U8 *) scan, (U8 *) this_eol,
                                           (U8) ARG1u(p), FLAGS(p));
        break;

      case NANYOFM_t8:
        while (     hardcount < max
               &&   scan < this_eol
               &&  (*scan & FLAGS(p)) != ARG1u(p))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case NANYOFM_tb:
        scan = (char *) find_next_masked((U8 *) scan, (U8 *) this_eol,
                                         (U8) ARG1u(p), FLAGS(p));
        break;

      case ANYOFH_tb: /* ANYOFH only can match UTF-8 targets */
      case ANYOFHb_tb:
      case ANYOFHbbm_tb:
      case ANYOFHr_tb:
      case ANYOFHs_tb:
        break;

      case ANYOFH_t8:
        anyofh_list = GET_ANYOFH_INVLIST(prog, p);
        while (  hardcount < max
               && scan < this_eol
               && NATIVE_UTF8_TO_I8(*scan) >= ANYOF_FLAGS(p)
               && _invlist_contains_cp(anyofh_list,
                                             utf8_to_uvchr_buf((U8 *) scan,
                                                               (U8 *) this_eol,
                                                               NULL)))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case ANYOFHb_t8:
        /* we know the first byte must be the FLAGS field */
        anyofh_list = GET_ANYOFH_INVLIST(prog, p);
        while (   hardcount < max
               && scan < this_eol
               && (U8) *scan == ANYOF_FLAGS(p)
               && _invlist_contains_cp(anyofh_list,
                                             utf8_to_uvchr_buf((U8 *) scan,
                                                               (U8 *) this_eol,
                                                               NULL)))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case ANYOFHbbm_t8:
        while (   hardcount < max
               && scan + 1 < this_eol
               && (U8) *scan == ANYOF_FLAGS(p)
               && BITMAP_TEST(( (struct regnode_bbm *) p)->bitmap,
                                (U8) scan[1] & UTF_CONTINUATION_MASK))
        {
            scan += 2;  /* This node only matces 2-byte UTF-8 */
            hardcount++;
        }
        break;

      case ANYOFHr_t8:
        anyofh_list = GET_ANYOFH_INVLIST(prog, p);
        while (  hardcount < max
               && scan < this_eol
               && inRANGE(NATIVE_UTF8_TO_I8(*scan),
                          LOWEST_ANYOF_HRx_BYTE(ANYOF_FLAGS(p)),
                          HIGHEST_ANYOF_HRx_BYTE(ANYOF_FLAGS(p)))
               && NATIVE_UTF8_TO_I8(*scan) >= ANYOF_FLAGS(p)
               && _invlist_contains_cp(anyofh_list,
                                             utf8_to_uvchr_buf((U8 *) scan,
                                                               (U8 *) this_eol,
                                                               NULL)))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case ANYOFHs_t8:
        anyofh_list = GET_ANYOFH_INVLIST(prog, p);
        while (   hardcount < max
               && scan + FLAGS(p) < this_eol
               && memEQ(scan, ((struct regnode_anyofhs *) p)->string, FLAGS(p))
               && _invlist_contains_cp(anyofh_list,
                                             utf8_to_uvchr_buf((U8 *) scan,
                                                               (U8 *) this_eol,
                                                               NULL)))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case ANYOFR_t8:
        while (   hardcount < max
               && scan < this_eol
               && NATIVE_UTF8_TO_I8(*scan) >= ANYOF_FLAGS(p)
               && withinCOUNT(utf8_to_uvchr_buf((U8 *) scan,
                                            (U8 *) this_eol,
                                            NULL),
                              ANYOFRbase(p), ANYOFRdelta(p)))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case ANYOFR_tb:
        while (   hardcount < max
               && scan < this_eol
               && withinCOUNT((U8) *scan, ANYOFRbase(p), ANYOFRdelta(p)))
        {
            scan++;
            hardcount++;
        }
        break;

      case ANYOFRb_t8:
        while (   hardcount < max
               && scan < this_eol
               && (U8) *scan == ANYOF_FLAGS(p)
               && withinCOUNT(utf8_to_uvchr_buf((U8 *) scan,
                                            (U8 *) this_eol,
                                            NULL),
                              ANYOFRbase(p), ANYOFRdelta(p)))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case ANYOFRb_tb:
        while (   hardcount < max
               && scan < this_eol
               && withinCOUNT((U8) *scan, ANYOFRbase(p), ANYOFRdelta(p)))
        {
            scan++;
            hardcount++;
        }
        break;

    /* The argument (FLAGS) to all the POSIX node types is the class number */

      case NPOSIXL_tb:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXL_tb:
        CHECK_AND_WARN_PROBLEMATIC_LOCALE_;
        while (   scan < this_eol
               && to_complement ^ cBOOL(isFOO_lc(FLAGS(p), *scan)))
        {
            scan++;
        }
        break;

      case NPOSIXL_t8:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXL_t8:
        while (   hardcount < max && scan < this_eol
               && to_complement ^ cBOOL(isFOO_utf8_lc(FLAGS(p),
                                                      (U8 *) scan,
                                                      (U8 *) this_eol)))
        {
            scan += UTF8SKIP(scan);
            hardcount++;
        }
        break;

      case POSIXD_tb:
        /* FALLTHROUGH */

      case POSIXA_t8:
        if (this_eol - scan > max) {

            /* We didn't adjust <this_eol> at the beginning of this routine
             * because is UTF-8, but it is actually ok to do so, since here, to
             * match, 1 char == 1 byte. */
            this_eol = scan + max;
        }
        /* FALLTHROUGH */

      case POSIXA_tb:
        while (scan < this_eol && generic_isCC_A_((U8) *scan, FLAGS(p))) {
            scan++;
        }
        break;

      case NPOSIXD_tb:
        /* FALLTHROUGH */

      case NPOSIXA_tb:
        while (scan < this_eol && ! generic_isCC_A_((U8) *scan, FLAGS(p))) {
            scan++;
        }
        break;

      case NPOSIXA_t8:

        /* The complement of something that matches only ASCII matches all
         * non-ASCII, plus everything in ASCII that isn't in the class. */
        while (   hardcount < max && scan < this_eol
               && (   ! isASCII_utf8_safe(scan, loceol)
                   || ! generic_isCC_A_((U8) *scan, FLAGS(p))))
            {
                scan += UTF8SKIP(scan);
                hardcount++;
            }
        break;

      case NPOSIXU_tb:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXU_tb:
        while (   scan < this_eol
               && to_complement ^ cBOOL(generic_isCC_((U8) *scan, FLAGS(p))))
        {
            scan++;
        }
        break;

      case NPOSIXU_t8:
      case NPOSIXD_t8:
        to_complement = 1;
        /* FALLTHROUGH */

      case POSIXD_t8:
      case POSIXU_t8:
        classnum = (char_class_number_) FLAGS(p);
        switch (classnum) {
          default:
            while (   hardcount < max && scan < this_eol
                   && to_complement
                    ^ cBOOL(_invlist_contains_cp(PL_XPosix_ptrs[classnum],
                       utf8_to_uvchr_buf((U8 *) scan, (U8 *) this_eol, NULL))))
            {
                scan += UTF8SKIP(scan);
                hardcount++;
            }
            break;

            /* For the classes below, the knowledge of how to handle every code
             * point is compiled into Perl via a macro.  This code is written
             * for making the loops as tight as possible.  It could be
             * refactored to save space instead. */

          case CC_ENUM_SPACE_:
            while (   hardcount < max
                   && scan < this_eol
                   && (to_complement
                                   ^ cBOOL(isSPACE_utf8_safe(scan, this_eol))))
            {
                scan += UTF8SKIP(scan);
                hardcount++;
            }
            break;
          case CC_ENUM_BLANK_:
            while (   hardcount < max
                   && scan < this_eol
                   && (to_complement
                                ^ cBOOL(isBLANK_utf8_safe(scan, this_eol))))
            {
                scan += UTF8SKIP(scan);
                hardcount++;
            }
            break;
          case CC_ENUM_XDIGIT_:
            while (   hardcount < max
                   && scan < this_eol
                   && (to_complement
                               ^ cBOOL(isXDIGIT_utf8_safe(scan, this_eol))))
            {
                scan += UTF8SKIP(scan);
                hardcount++;
            }
            break;
          case CC_ENUM_VERTSPACE_:
            while (   hardcount < max
                   && scan < this_eol
                   && (to_complement
                               ^ cBOOL(isVERTWS_utf8_safe(scan, this_eol))))
            {
                scan += UTF8SKIP(scan);
                hardcount++;
            }
            break;
          case CC_ENUM_CNTRL_:
            while (   hardcount < max
                   && scan < this_eol
                   && (to_complement
                               ^ cBOOL(isCNTRL_utf8_safe(scan, this_eol))))
            {
                scan += UTF8SKIP(scan);
                hardcount++;
            }
            break;
        }
        break;

      case LNBREAK_t8:
        while (    hardcount < max && scan < this_eol
               && (c=is_LNBREAK_utf8_safe(scan, this_eol)))
        {
            scan += c;
            hardcount++;
        }
        break;

      case LNBREAK_tb:
        /* LNBREAK can match one or two latin chars, which is ok, but we have
         * to use hardcount in this situation, and throw away the adjustment to
         * <this_eol> done before the switch statement */
        while (
            hardcount < max && scan < loceol
            && (c = is_LNBREAK_latin1_safe(scan, loceol))
        ) {
            scan += c;
            hardcount++;
        }
        break;

    default:
        Perl_croak(aTHX_ "panic: regrepeat() called with unrecognized"
                         " node type %d='%s'", OP(p), REGNODE_NAME(OP(p)));
        NOT_REACHED; /* NOTREACHED */

    }

    if (hardcount)
        c = hardcount;
    else
        c = scan - *startposp;
    *startposp = scan;

    DEBUG_r({
        DECLARE_AND_GET_RE_DEBUG_FLAGS;
        DEBUG_EXECUTE_r({
            SV * const prop = sv_newmortal();
            regprop(prog, prop, p, reginfo, NULL);
            Perl_re_exec_indentf( aTHX_
                        "%s can match %" IVdf " times out of %" IVdf "...\n",
                        depth, SvPVX_const(prop),(IV)c,(IV)max);
        });
    });

    return(c);
}

/*
 - reginclass - determine if a character falls into a character class

  n is the ANYOF-type regnode
  p is the target string
  p_end points to one byte beyond the end of the target string
  utf8_target tells whether p is in UTF-8.

  Returns true if matched; false otherwise.

  Note that this can be a synthetic start class, a combination of various
  nodes, so things you think might be mutually exclusive, such as locale,
  aren't.  It can match both locale and non-locale

 */

STATIC bool
S_reginclass(pTHX_ regexp * const prog, const regnode * const n, const U8* const p, const U8* const p_end, const bool utf8_target)
{
    const char flags = (inRANGE(OP(n), ANYOFH, ANYOFHs))
                        ? 0
                        : ANYOF_FLAGS(n);
    bool match = FALSE;
    UV c = *p;

    PERL_ARGS_ASSERT_REGINCLASS;

    /* If c is not already the code point, get it.  Note that
     * UTF8_IS_INVARIANT() works even if not in UTF-8 */
    if (! UTF8_IS_INVARIANT(c) && utf8_target) {
        STRLEN c_len = 0;
        const U32 utf8n_flags = UTF8_ALLOW_DEFAULT;
        c = utf8n_to_uvchr(p, p_end - p, &c_len, utf8n_flags | UTF8_CHECK_ONLY);
        if (c_len == (STRLEN)-1) {
            _force_out_malformed_utf8_message(p, p_end,
                                              utf8n_flags,
                                              1 /* 1 means die */ );
            NOT_REACHED; /* NOTREACHED */
        }
        if (     c > 255
            &&  (OP(n) == ANYOFL || OP(n) == ANYOFPOSIXL)
            && ! (flags & ANYOFL_UTF8_LOCALE_REQD))
        {
            _CHECK_AND_OUTPUT_WIDE_LOCALE_CP_MSG(c);
        }
    }

    /* If this character is potentially in the bitmap, check it */
    if (c < NUM_ANYOF_CODE_POINTS && ! inRANGE(OP(n), ANYOFH, ANYOFHb)) {
        if (ANYOF_BITMAP_TEST(n, c))
            match = TRUE;
        else if (  (flags & ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared)
                 && OP(n) == ANYOFD
                 && ! utf8_target
                 && ! isASCII(c))
        {
            match = TRUE;
        }
        else if (flags & ANYOF_LOCALE_FLAGS) {
            if (  (flags & ANYOFL_FOLD)
                && c < 256
                && ANYOF_BITMAP_TEST(n, PL_fold_locale[c]))
            {
                match = TRUE;
            }
            else if (   ANYOF_POSIXL_TEST_ANY_SET(n)
                     && c <= U8_MAX  /* param to isFOO_lc() */
            ) {
                /* The data structure is arranged so bits 0, 2, 4, ... are set
                 * if the class includes the Posix character class given by
                 * bit/2; and 1, 3, 5, ... are set if the class includes the
                 * complemented Posix class given by int(bit/2), so the
                 * remainder modulo 2 tells us if to complement or not.
                 *
                 * Note that this code assumes that all the classes are closed
                 * under folding.  For example, if a character matches \w, then
                 * its fold does too; and vice versa.  This should be true for
                 * any well-behaved locale for all the currently defined Posix
                 * classes, except for :lower: and :upper:, which are handled
                 * by the pseudo-class :cased: which matches if either of the
                 * other two does.  To get rid of this assumption, an outer
                 * loop could be used below to iterate over both the source
                 * character, and its fold (if different) */

                U32 posixl_bits = ANYOF_POSIXL_BITMAP(n);

                do {
                    /* Find the next set bit indicating a class to try matching
                     * against */
                    U8 bit_pos = lsbit_pos32(posixl_bits);

                    if (bit_pos % 2 ^ cBOOL(isFOO_lc(bit_pos/2, (U8) c))) {
                        match = TRUE;
                        break;
                    }

                    /* Remove this class from consideration; repeat */
                    POSIXL_CLEAR(posixl_bits, bit_pos);
                } while(posixl_bits != 0);
            }
        }
    }

    /* If the bitmap didn't (or couldn't) match, and something outside the
     * bitmap could match, try that. */
    if (!match) {
        if (      c >= NUM_ANYOF_CODE_POINTS
            &&    ANYOF_ONLY_HAS_BITMAP(n)
            && ! (flags & ANYOF_HAS_EXTRA_RUNTIME_MATCHES))
        {
            /* In this case, the ARG is set up so that the final bit indicates
             * whether it matches or not */
            match = ARG1u(n) & 1;
        }
        else
            /* Here, the main way it could match is if the code point is
             * outside the bitmap and an inversion list exists to handle such
             * things.  The other ways all occur when a flag is set to indicate
             * we need special handling.  That handling doesn't come in to
             * effect for ANYOFD nodes unless the target string is UTF-8 and
             * that matters to code point being matched. */
             if (    c >= NUM_ANYOF_CODE_POINTS
                 || (   (flags & ANYOF_HAS_EXTRA_RUNTIME_MATCHES)
                     && (   UNLIKELY(OP(n) != ANYOFD)
                         || (utf8_target && ! isASCII_uvchr(c)
#                               if NUM_ANYOF_CODE_POINTS > 256
                                                               && c < 256
#                               endif
                                                                         ))))
        {
            /* Here, we have an inversion list for outside-the-bitmap code
             * points and/or the flag is set indicating special handling is
             * needed.  The flag is set when there is auxiliary data beyond the
             * normal inversion list, or if there is the possibility of a
             * special Turkic locale match, even without auxiliary data.
             *
             * First check if there is an inversion list and/or auxiliary data.
             * */
            if (ANYOF_HAS_AUX(n)) {
                SV* only_utf8_locale = NULL;

                /* This call will return in 'definition' the union of the base
                 * non-bitmap inversion list, if any, plus the deferred
                 * definitions of user-defined properties, if any.  It croaks
                 * if there is such a property but which still has no definition
                 * available */
                SV * const definition = GET_REGCLASS_AUX_DATA(prog, n, TRUE, 0,
                                                      &only_utf8_locale, NULL);
                if (definition) {
                    /* Most likely is the outside-the-bitmap inversion list. */
                    if (_invlist_contains_cp(definition, c)) {
                        match = TRUE;
                    }
                    else /* Failing that, hardcode the two tests for a Turkic
                            locale */
                         if (   UNLIKELY(IN_UTF8_TURKIC_LOCALE)
                             && isALPHA_FOLD_EQ(*p, 'i'))
                    {
                        /* Turkish locales have these hard-coded rules
                         * overriding normal ones */
                        if (*p == 'i') {
                            if (_invlist_contains_cp(definition,
                                         LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE))
                            {
                                match = TRUE;
                            }
                        }
                        else if (_invlist_contains_cp(definition,
                                                 LATIN_SMALL_LETTER_DOTLESS_I))
                        {
                            match = TRUE;
                        }
                    }
                }

                if (   UNLIKELY(only_utf8_locale)
                    && UNLIKELY(IN_UTF8_CTYPE_LOCALE)
                    && ! match)
                {
                    match = _invlist_contains_cp(only_utf8_locale, c);
                }
            }

            /* In a Turkic locale under folding, hard-code the I i case pair
             * matches; these wouldn't have the ANYOF_HAS_EXTRA_RUNTIME_MATCHES
             * flag set unless [Ii] were match possibilities */
            if (UNLIKELY(IN_UTF8_TURKIC_LOCALE) && ! match) {
                if (utf8_target) {
                    if (c == LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE) {
                        if (ANYOF_BITMAP_TEST(n, 'i')) {
                            match = TRUE;
                        }
                    }
                    else if (c == LATIN_SMALL_LETTER_DOTLESS_I) {
                        if (ANYOF_BITMAP_TEST(n, 'I')) {
                            match = TRUE;
                        }
                    }
                }

#if NUM_ANYOF_CODE_POINTS > 256
                /* Larger bitmap means these special cases aren't handled
                 * outside the bitmap above. */
                if (*p == 'i') {
                    if (ANYOF_BITMAP_TEST(n,
                                        LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE))
                    {
                        match = TRUE;
                    }
                }
                else if (*p == 'I') {
                    if (ANYOF_BITMAP_TEST(n, LATIN_SMALL_LETTER_DOTLESS_I)) {
                        match = TRUE;
                    }
                }
#endif
            }
        }

        if (   UNICODE_IS_SUPER(c)
            && (flags & ANYOF_WARN_SUPER__shared)
            && OP(n) != ANYOFD
            && ckWARN_d(WARN_NON_UNICODE))
        {
            Perl_warner(aTHX_ packWARN(WARN_NON_UNICODE),
                "Matched non-Unicode code point 0x%04" UVXf " against Unicode property; may not be portable", c);
        }
    }

#if ANYOF_INVERT != 1
    /* Depending on compiler optimization cBOOL takes time, so if don't have to
     * use it, don't */
#   error ANYOF_INVERT needs to be set to 1, or guarded with cBOOL below,
#endif

    /* The xor complements the return if to invert: 1^1 = 0, 1^0 = 1 */
    return (flags & ANYOF_INVERT) ^ match;
}

STATIC U8 *
S_reghop3(U8 *s, SSize_t off, const U8* lim)
{
    /* return the position 'off' UTF-8 characters away from 's', forward if
     * 'off' >= 0, backwards if negative.  But don't go outside of position
     * 'lim', which better be < s  if off < 0 */

    PERL_ARGS_ASSERT_REGHOP3;

    if (off >= 0) {
        while (off-- && s < lim) {
            /* XXX could check well-formedness here */
            U8 *new_s = s + UTF8SKIP(s);
            if (new_s > lim) /* lim may be in the middle of a long character */
                return s;
            s = new_s;
        }
    }
    else {
        while (off++ && s > lim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > lim && UTF8_IS_CONTINUATION(*s))
                    s--;
                if (! UTF8_IS_START(*s)) {
                    Perl_croak_nocontext("Malformed UTF-8 character (fatal)");
                }
            }
            /* XXX could check well-formedness here */
        }
    }
    return s;
}

STATIC U8 *
S_reghop4(U8 *s, SSize_t off, const U8* llim, const U8* rlim)
{
    PERL_ARGS_ASSERT_REGHOP4;

    if (off >= 0) {
        while (off-- && s < rlim) {
            /* XXX could check well-formedness here */
            s += UTF8SKIP(s);
        }
    }
    else {
        while (off++ && s > llim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > llim && UTF8_IS_CONTINUATION(*s))
                    s--;
                if (! UTF8_IS_START(*s)) {
                    Perl_croak_nocontext("Malformed UTF-8 character (fatal)");
                }
            }
            /* XXX could check well-formedness here */
        }
    }
    return s;
}

/* like reghop3, but returns NULL on overrun, rather than returning last
 * char pos */

STATIC U8 *
S_reghopmaybe3(U8* s, SSize_t off, const U8* const lim)
{
    PERL_ARGS_ASSERT_REGHOPMAYBE3;

    if (off >= 0) {
        while (off-- && s < lim) {
            /* XXX could check well-formedness here */
            s += UTF8SKIP(s);
        }
        if (off >= 0)
            return NULL;
    }
    else {
        while (off++ && s > lim) {
            s--;
            if (UTF8_IS_CONTINUED(*s)) {
                while (s > lim && UTF8_IS_CONTINUATION(*s))
                    s--;
                if (! UTF8_IS_START(*s)) {
                    Perl_croak_nocontext("Malformed UTF-8 character (fatal)");
                }
            }
            /* XXX could check well-formedness here */
        }
        if (off <= 0)
            return NULL;
    }
    return s;
}


/* when executing a regex that may have (?{}), extra stuff needs setting
   up that will be visible to the called code, even before the current
   match has finished. In particular:

   * $_ is localised to the SV currently being matched;
   * pos($_) is created if necessary, ready to be updated on each call-out
     to code;
   * a fake PMOP is created that can be set to PL_curpm (normally PL_curpm
     isn't set until the current pattern is successfully finished), so that
     $1 etc of the match-so-far can be seen;
   * save the old values of subbeg etc of the current regex, and  set then
     to the current string (again, this is normally only done at the end
     of execution)
*/

static void
S_setup_eval_state(pTHX_ regmatch_info *const reginfo)
{
    MAGIC *mg;
    regexp *const rex = ReANY(reginfo->prog);
    regmatch_info_aux_eval *eval_state = reginfo->info_aux_eval;

    eval_state->rex = rex;
    eval_state->sv  = reginfo->sv;

    if (reginfo->sv) {
        /* Make $_ available to executed code. */
        if (reginfo->sv != DEFSV) {
            SAVE_DEFSV;
            DEFSV_set(reginfo->sv);
        }
        /* will be dec'd by S_cleanup_regmatch_info_aux */
        SvREFCNT_inc_NN(reginfo->sv);

        if (!(mg = mg_find_mglob(reginfo->sv))) {
            /* prepare for quick setting of pos */
            mg = sv_magicext_mglob(reginfo->sv);
            mg->mg_len = -1;
        }
        eval_state->pos_magic = mg;
        eval_state->pos       = mg->mg_len;
        eval_state->pos_flags = mg->mg_flags;
    }
    else
        eval_state->pos_magic = NULL;

    if (!PL_reg_curpm) {
        /* PL_reg_curpm is a fake PMOP that we can attach the current
         * regex to and point PL_curpm at, so that $1 et al are visible
         * within a /(?{})/. It's just allocated once per interpreter the
         * first time its needed */
        Newxz(PL_reg_curpm, 1, PMOP);
#ifdef USE_ITHREADS
        {
            SV* const repointer = &PL_sv_undef;
            /* this regexp is also owned by the new PL_reg_curpm, which
               will try to free it.  */
            av_push(PL_regex_padav, repointer);
            PL_reg_curpm->op_pmoffset = av_top_index(PL_regex_padav);
            PL_regex_pad = AvARRAY(PL_regex_padav);
        }
#endif
    }
    SET_reg_curpm(reginfo->prog);
    eval_state->curpm = PL_curpm;
    PL_curpm_under = PL_curpm;
    PL_curpm = PL_reg_curpm;
    if (RXp_MATCH_COPIED(rex)) {
        /*  Here is a serious problem: we cannot rewrite subbeg,
            since it may be needed if this match fails.  Thus
            $` inside (?{}) could fail... */
        eval_state->subbeg     = RXp_SUBBEG(rex);
        eval_state->sublen     = RXp_SUBLEN(rex);
        eval_state->suboffset  = RXp_SUBOFFSET(rex);
        eval_state->subcoffset = RXp_SUBCOFFSET(rex);
#ifdef PERL_ANY_COW
        eval_state->saved_copy = RXp_SAVED_COPY(rex);
#endif
        RXp_MATCH_COPIED_off(rex);
    }
    else
        eval_state->subbeg = NULL;
    RXp_SUBBEG(rex) = (char *)reginfo->strbeg;
    RXp_SUBOFFSET(rex) = 0;
    RXp_SUBCOFFSET(rex) = 0;
    RXp_SUBLEN(rex) = reginfo->strend - reginfo->strbeg;
}


/* destructor to clear up regmatch_info_aux and regmatch_info_aux_eval */

static void
S_cleanup_regmatch_info_aux(pTHX_ void *arg)
{
    regmatch_info_aux *aux = (regmatch_info_aux *) arg;
    regmatch_info_aux_eval *eval_state =  aux->info_aux_eval;
    regmatch_slab *s;

    Safefree(aux->poscache);

    if (eval_state) {

        /* undo the effects of S_setup_eval_state() */

        if (eval_state->subbeg) {
            regexp * const rex = eval_state->rex;
            RXp_SUBBEG(rex) = eval_state->subbeg;
            RXp_SUBLEN(rex)     = eval_state->sublen;
            RXp_SUBOFFSET(rex)  = eval_state->suboffset;
            RXp_SUBCOFFSET(rex) = eval_state->subcoffset;
#ifdef PERL_ANY_COW
            RXp_SAVED_COPY(rex) = eval_state->saved_copy;
#endif
            RXp_MATCH_COPIED_on(rex);
        }
        if (eval_state->pos_magic)
        {
            eval_state->pos_magic->mg_len = eval_state->pos;
            eval_state->pos_magic->mg_flags =
                 (eval_state->pos_magic->mg_flags & ~MGf_BYTES)
               | (eval_state->pos_flags & MGf_BYTES);
        }

        PL_curpm = eval_state->curpm;
        SvREFCNT_dec(eval_state->sv);
    }

    PL_regmatch_state = aux->old_regmatch_state;
    PL_regmatch_slab  = aux->old_regmatch_slab;

    /* free all slabs above current one - this must be the last action
     * of this function, as aux and eval_state are allocated within
     * slabs and may be freed here */

    s = PL_regmatch_slab->next;
    if (s) {
        PL_regmatch_slab->next = NULL;
        while (s) {
            regmatch_slab * const osl = s;
            s = s->next;
            Safefree(osl);
        }
    }
}


STATIC void
S_to_utf8_substr(pTHX_ regexp *prog)
{
    /* Converts substr fields in prog from bytes to UTF-8, calling fbm_compile
     * on the converted value */

    int i = 1;

    PERL_ARGS_ASSERT_TO_UTF8_SUBSTR;

    do {
        if (prog->substrs->data[i].substr
            && !prog->substrs->data[i].utf8_substr) {
            SV* const sv = newSVsv(prog->substrs->data[i].substr);
            prog->substrs->data[i].utf8_substr = sv;
            sv_utf8_upgrade(sv);
            if (SvVALID(prog->substrs->data[i].substr)) {
                if (SvTAIL(prog->substrs->data[i].substr)) {
                    /* Trim the trailing \n that fbm_compile added last
                       time.  */
                    SvCUR_set(sv, SvCUR(sv) - 1);
                    /* Whilst this makes the SV technically "invalid" (as its
                       buffer is no longer followed by "\0") when fbm_compile()
                       adds the "\n" back, a "\0" is restored.  */
                    fbm_compile(sv, FBMcf_TAIL);
                } else
                    fbm_compile(sv, 0);
            }
            if (prog->substrs->data[i].substr == prog->check_substr)
                prog->check_utf8 = sv;
        }
    } while (i--);
}

STATIC bool
S_to_byte_substr(pTHX_ regexp *prog)
{
    /* Converts substr fields in prog from UTF-8 to bytes, calling fbm_compile
     * on the converted value; returns FALSE if can't be converted. */

    int i = 1;

    PERL_ARGS_ASSERT_TO_BYTE_SUBSTR;

    do {
        if (prog->substrs->data[i].utf8_substr
            && !prog->substrs->data[i].substr) {
            SV* sv = newSVsv(prog->substrs->data[i].utf8_substr);
            if (! sv_utf8_downgrade(sv, TRUE)) {
                SvREFCNT_dec_NN(sv);
                return FALSE;
            }
            if (SvVALID(prog->substrs->data[i].utf8_substr)) {
                if (SvTAIL(prog->substrs->data[i].utf8_substr)) {
                    /* Trim the trailing \n that fbm_compile added last
                        time.  */
                    SvCUR_set(sv, SvCUR(sv) - 1);
                    fbm_compile(sv, FBMcf_TAIL);
                } else
                    fbm_compile(sv, 0);
            }
            prog->substrs->data[i].substr = sv;
            if (prog->substrs->data[i].utf8_substr == prog->check_utf8)
                prog->check_substr = sv;
        }
    } while (i--);

    return TRUE;
}

#ifndef PERL_IN_XSUB_RE

bool
Perl_is_grapheme(pTHX_ const U8 * strbeg, const U8 * s, const U8 * strend, const UV cp)
{
    /* Temporary helper function for toke.c.  Verify that the code point 'cp'
     * is a stand-alone grapheme.  The UTF-8 for 'cp' begins at position 's' in
     * the larger string bounded by 'strbeg' and 'strend'.
     *
     * 'cp' needs to be assigned (if not, a future version of the Unicode
     * Standard could make it something that combines with adjacent characters,
     * so code using it would then break), and there has to be a GCB break
     * before and after the character. */


    GCB_enum cp_gcb_val, prev_cp_gcb_val, next_cp_gcb_val;
    const U8 * prev_cp_start;

    PERL_ARGS_ASSERT_IS_GRAPHEME;

    if (   UNLIKELY(UNICODE_IS_SUPER(cp))
        || UNLIKELY(UNICODE_IS_NONCHAR(cp)))
    {
        /* These are considered graphemes */
        return TRUE;
    }

    /* Otherwise, unassigned code points are forbidden */
    if (UNLIKELY(! ELEMENT_RANGE_MATCHES_INVLIST(
                                    _invlist_search(PL_Assigned_invlist, cp))))
    {
        return FALSE;
    }

    cp_gcb_val = getGCB_VAL_CP(cp);

    /* Find the GCB value of the previous code point in the input */
    prev_cp_start = utf8_hop_back(s, -1, strbeg);
    if (UNLIKELY(prev_cp_start == s)) {
        prev_cp_gcb_val = GCB_EDGE;
    }
    else {
        prev_cp_gcb_val = getGCB_VAL_UTF8(prev_cp_start, strend);
    }

    /* And check that is a grapheme boundary */
    if (! isGCB(prev_cp_gcb_val, cp_gcb_val, strbeg, s,
                TRUE /* is UTF-8 encoded */ ))
    {
        return FALSE;
    }

    /* Similarly verify there is a break between the current character and the
     * following one */
    s += UTF8SKIP(s);
    if (s >= strend) {
        next_cp_gcb_val = GCB_EDGE;
    }
    else {
        next_cp_gcb_val = getGCB_VAL_UTF8(s, strend);
    }

    return isGCB(cp_gcb_val, next_cp_gcb_val, strbeg, s, TRUE);
}

/*
=for apidoc_section $unicode

=for apidoc isSCRIPT_RUN

Returns a bool as to whether or not the sequence of bytes from C<s> up to but
not including C<send> form a "script run".  C<utf8_target> is TRUE iff the
sequence starting at C<s> is to be treated as UTF-8.  To be precise, except for
two degenerate cases given below, this function returns TRUE iff all code
points in it come from any combination of three "scripts" given by the Unicode
"Script Extensions" property: Common, Inherited, and possibly one other.
Additionally all decimal digits must come from the same consecutive sequence of
10.

For example, if all the characters in the sequence are Greek, or Common, or
Inherited, this function will return TRUE, provided any decimal digits in it
are from the same block of digits in Common.  (These are the ASCII digits
"0".."9" and additionally a block for full width forms of these, and several
others used in mathematical notation.)   For scripts (unlike Greek) that have
their own digits defined this will accept either digits from that set or from
one of the Common digit sets, but not a combination of the two.  Some scripts,
such as Arabic, have more than one set of digits.  All digits must come from
the same set for this function to return TRUE.

C<*ret_script>, if C<ret_script> is not NULL, will on return of TRUE
contain the script found, using the C<SCX_enum> typedef.  Its value will be
C<SCX_INVALID> if the function returns FALSE.

If the sequence is empty, TRUE is returned, but C<*ret_script> (if asked for)
will be C<SCX_INVALID>.

If the sequence contains a single code point which is unassigned to a character
in the version of Unicode being used, the function will return TRUE, and the
script will be C<SCX_Unknown>.  Any other combination of unassigned code points
in the input sequence will result in the function treating the input as not
being a script run.

The returned script will be C<SCX_Inherited> iff all the code points in it are
from the Inherited script.

Otherwise, the returned script will be C<SCX_Common> iff all the code points in
it are from the Inherited or Common scripts.

=cut

*/

bool
Perl_isSCRIPT_RUN(pTHX_ const U8 * s, const U8 * send, const bool utf8_target)
{
    /* Basically, it looks at each character in the sequence to see if the
     * above conditions are met; if not it fails.  It uses an inversion map to
     * find the enum corresponding to the script of each character.  But this
     * is complicated by the fact that a few code points can be in any of
     * several scripts.  The data has been constructed so that there are
     * additional enum values (all negative) for these situations.  The
     * absolute value of those is an index into another table which contains
     * pointers to auxiliary tables for each such situation.  Each aux array
     * lists all the scripts for the given situation.  There is another,
     * parallel, table that gives the number of entries in each aux table.
     * These are all defined in charclass_invlists.h */

    /* XXX Here are the additional things UTS 39 says could be done:
     *
     * Forbid sequences of the same nonspacing mark
     *
     * Check to see that all the characters are in the sets of exemplar
     * characters for at least one language in the Unicode Common Locale Data
     * Repository [CLDR]. */


    /* Things that match /\d/u */
    SV * decimals_invlist = PL_XPosix_ptrs[CC_DIGIT_];
    UV * decimals_array = invlist_array(decimals_invlist);

    /* What code point is the digit '0' of the script run? (0 meaning FALSE if
     * not currently known) */
    UV zero_of_run = 0;

    SCX_enum script_of_run  = SCX_INVALID;   /* Illegal value */
    SCX_enum script_of_char = SCX_INVALID;

    /* If the script remains not fully determined from iteration to iteration,
     * this is the current intersection of the possiblities.  */
    SCX_enum * intersection = NULL;
    PERL_UINT_FAST8_T intersection_len = 0;

    bool retval = TRUE;
    SCX_enum * ret_script = NULL;

    assert(send >= s);

    PERL_ARGS_ASSERT_ISSCRIPT_RUN;

    /* All code points in 0..255 are either Common or Latin, so must be a
     * script run.  We can return immediately unless we need to know which
     * script it is. */
    if (! utf8_target && LIKELY(send > s)) {
        if (ret_script == NULL) {
            return TRUE;
        }

        /* If any character is Latin, the run is Latin */
        while (s < send) {
            if (isALPHA_L1(*s) && LIKELY(*s != MICRO_SIGN_NATIVE)) {
                *ret_script = SCX_Latin;
                return TRUE;
            }
        }

        /* Here, all are Common */
        *ret_script = SCX_Common;
        return TRUE;
    }

    /* Look at each character in the sequence */
    while (s < send) {
        /* If the current character being examined is a digit, this is the code
         * point of the zero for its sequence of 10 */
        UV zero_of_char;

        UV cp;

        /* The code allows all scripts to use the ASCII digits.  This is
         * because they are in the Common script.  Hence any ASCII ones found
         * are ok, unless and until a digit from another set has already been
         * encountered.  digit ranges in Common are not similarly blessed) */
        if (UNLIKELY(isDIGIT(*s))) {
            if (UNLIKELY(script_of_run == SCX_Unknown)) {
                retval = FALSE;
                break;
            }
            if (zero_of_run) {
                if (zero_of_run != '0') {
                    retval = FALSE;
                    break;
                }
            }
            else {
                zero_of_run = '0';
            }
            s++;
            continue;
        }

        /* Here, isn't an ASCII digit.  Find the code point of the character */
        if (! UTF8_IS_INVARIANT(*s)) {
            Size_t len;
            cp = valid_utf8_to_uvchr((U8 *) s, &len);
            s += len;
        }
        else {
            cp = *(s++);
        }

        /* If is within the range [+0 .. +9] of the script's zero, it also is a
         * digit in that script.  We can skip the rest of this code for this
         * character. */
        if (UNLIKELY(zero_of_run && withinCOUNT(cp, zero_of_run, 9))) {
            continue;
        }

        /* Find the character's script.  The correct values are hard-coded here
         * for small-enough code points. */
        if (cp < 0x2B9) {   /* From inspection of Unicode db; extremely
                               unlikely to change */
            if (       cp > 255
                || (   isALPHA_L1(cp)
                    && LIKELY(cp != MICRO_SIGN_NATIVE)))
            {
                script_of_char = SCX_Latin;
            }
            else {
                script_of_char = SCX_Common;
            }
        }
        else {
            script_of_char = _Perl_SCX_invmap[
                                       _invlist_search(PL_SCX_invlist, cp)];
        }

        /* We arbitrarily accept a single unassigned character, but not in
         * combination with anything else, and not a run of them. */
        if (   UNLIKELY(script_of_run == SCX_Unknown)
            || UNLIKELY(   script_of_run != SCX_INVALID
                        && script_of_char == SCX_Unknown))
        {
            retval = FALSE;
            break;
        }

        /* For the first character, or the run is inherited, the run's script
         * is set to the char's */
        if (   UNLIKELY(script_of_run == SCX_INVALID)
            || UNLIKELY(script_of_run == SCX_Inherited))
        {
            script_of_run = script_of_char;
        }

        /* For the character's script to be Unknown, it must be the first
         * character in the sequence (for otherwise a test above would have
         * prevented us from reaching here), and we have set the run's script
         * to it.  Nothing further to be done for this character */
        if (UNLIKELY(script_of_char == SCX_Unknown)) {
            continue;
        }

        /* We accept 'inherited' script characters currently even at the
         * beginning.  (We know that no characters in Inherited are digits, or
         * we'd have to check for that) */
        if (UNLIKELY(script_of_char == SCX_Inherited)) {
            continue;
        }

        /* If the run so far is Common, and the new character isn't, change the
         * run's script to that of this character */
        if (script_of_run == SCX_Common && script_of_char != SCX_Common) {
            script_of_run = script_of_char;
        }

        /* Now we can see if the script of the new character is the same as
         * that of the run */
        if (LIKELY(script_of_char == script_of_run)) {
            /* By far the most common case */
            goto scripts_match;
        }

        /* Here, the script of the run isn't Common.  But characters in Common
         * match any script */
        if (script_of_char == SCX_Common) {
            goto scripts_match;
        }

#ifndef HAS_SCX_AUX_TABLES

        /* Too early a Unicode version to have a code point belonging to more
         * than one script, so, if the scripts don't exactly match, fail */
        PERL_UNUSED_VAR(intersection_len);
        retval = FALSE;
        break;

#else

        /* Here there is no exact match between the character's script and the
         * run's.  And we've handled the special cases of scripts Unknown,
         * Inherited, and Common.
         *
         * Negative script numbers signify that the value may be any of several
         * scripts, and we need to look at auxiliary information to make our
         * determination.  But if both are non-negative, we can fail now */
        if (LIKELY(script_of_char >= 0)) {
            const SCX_enum * search_in;
            PERL_UINT_FAST8_T search_in_len;
            PERL_UINT_FAST8_T i;

            if (LIKELY(script_of_run >= 0)) {
                retval = FALSE;
                break;
            }

            /* Use the previously constructed set of possible scripts, if any.
             * */
            if (intersection) {
                search_in = intersection;
                search_in_len = intersection_len;
            }
            else {
                search_in = SCX_AUX_TABLE_ptrs[-script_of_run];
                search_in_len = SCX_AUX_TABLE_lengths[-script_of_run];
            }

            for (i = 0; i < search_in_len; i++) {
                if (search_in[i] == script_of_char) {
                    script_of_run = script_of_char;
                    goto scripts_match;
                }
            }

            retval = FALSE;
            break;
        }
        else if (LIKELY(script_of_run >= 0)) {
            /* script of character could be one of several, but run is a single
             * script */
            const SCX_enum * search_in = SCX_AUX_TABLE_ptrs[-script_of_char];
            const PERL_UINT_FAST8_T search_in_len
                                     = SCX_AUX_TABLE_lengths[-script_of_char];
            PERL_UINT_FAST8_T i;

            for (i = 0; i < search_in_len; i++) {
                if (search_in[i] == script_of_run) {
                    script_of_char = script_of_run;
                    goto scripts_match;
                }
            }

            retval = FALSE;
            break;
        }
        else {
            /* Both run and char could be in one of several scripts.  If the
             * intersection is empty, then this character isn't in this script
             * run.  Otherwise, we need to calculate the intersection to use
             * for future iterations of the loop, unless we are already at the
             * final character */
            const SCX_enum * search_char = SCX_AUX_TABLE_ptrs[-script_of_char];
            const PERL_UINT_FAST8_T char_len
                                      = SCX_AUX_TABLE_lengths[-script_of_char];
            const SCX_enum * search_run;
            PERL_UINT_FAST8_T run_len;

            SCX_enum * new_overlap = NULL;
            PERL_UINT_FAST8_T i, j;

            if (intersection) {
                search_run = intersection;
                run_len = intersection_len;
            }
            else {
                search_run = SCX_AUX_TABLE_ptrs[-script_of_run];
                run_len = SCX_AUX_TABLE_lengths[-script_of_run];
            }

            intersection_len = 0;

            for (i = 0; i < run_len; i++) {
                for (j = 0; j < char_len; j++) {
                    if (search_run[i] == search_char[j]) {

                        /* Here, the script at i,j matches.  That means this
                         * character is in the run.  But continue on to find
                         * the complete intersection, for the next loop
                         * iteration, and for the digit check after it.
                         *
                         * On the first found common script, we malloc space
                         * for the intersection list for the worst case of the
                         * intersection, which is the minimum of the number of
                         * scripts remaining in each set. */
                        if (intersection_len == 0) {
                            Newx(new_overlap,
                                 MIN(run_len - i, char_len - j),
                                 SCX_enum);
                        }
                        new_overlap[intersection_len++] = search_run[i];
                    }
                }
            }

            /* Here we've looked through everything.  If they have no scripts
             * in common, not a run */
            if (intersection_len == 0) {
                retval = FALSE;
                break;
            }

            /* If there is only a single script in common, set to that.
             * Otherwise, use the intersection going forward */
            Safefree(intersection);
            intersection = NULL;
            if (intersection_len == 1) {
                script_of_run = script_of_char = new_overlap[0];
                Safefree(new_overlap);
                new_overlap = NULL;
            }
            else {
                intersection = new_overlap;
            }
        }

#endif

  scripts_match:

        /* Here, the script of the character is compatible with that of the
         * run.  That means that in most cases, it continues the script run.
         * Either it and the run match exactly, or one or both can be in any of
         * several scripts, and the intersection is not empty.  However, if the
         * character is a decimal digit, it could still mean failure if it is
         * from the wrong sequence of 10.  So, we need to look at if it's a
         * digit.  We've already handled the 10 digits [0-9], and the next
         * lowest one is this one: */
        if (cp < FIRST_NON_ASCII_DECIMAL_DIGIT) {
            continue;   /* Not a digit; this character is part of the run */
        }

        /* If we have a definitive '0' for the script of this character, we
         * know that for this to be a digit, it must be in the range of +0..+9
         * of that zero. */
        if (   script_of_char >= 0
            && (zero_of_char = script_zeros[script_of_char]))
        {
            if (! withinCOUNT(cp, zero_of_char, 9)) {
                continue;   /* Not a digit; this character is part of the run
                             */
            }

        }
        else {  /* Need to look up if this character is a digit or not */
            SSize_t index_of_zero_of_char;
            index_of_zero_of_char = _invlist_search(decimals_invlist, cp);
            if (     UNLIKELY(index_of_zero_of_char < 0)
                || ! ELEMENT_RANGE_MATCHES_INVLIST(index_of_zero_of_char))
            {
                continue;   /* Not a digit; this character is part of the run.
                             */
            }

            zero_of_char = decimals_array[index_of_zero_of_char];
        }

        /* Here, the character is a decimal digit, and the zero of its sequence
         * of 10 is in 'zero_of_char'.  If we already have a zero for this run,
         * they better be the same. */
        if (zero_of_run) {
            if (zero_of_run != zero_of_char) {
                retval = FALSE;
                break;
            }
        }
        else {  /* Otherwise we now have a zero for this run */
            zero_of_run = zero_of_char;
        }
    } /* end of looping through CLOSESR text */

    Safefree(intersection);

    if (ret_script != NULL) {
        if (retval) {
            *ret_script = script_of_run;
        }
        else {
            *ret_script = SCX_INVALID;
        }
    }

    return retval;
}
#endif /* ifndef PERL_IN_XSUB_RE */

/* Buffer logic. */
SV*
Perl_reg_named_buff(pTHX_ REGEXP * const rx, SV * const key, SV * const value,
                    const U32 flags)
{
    PERL_ARGS_ASSERT_REG_NAMED_BUFF;

    PERL_UNUSED_ARG(value);

    if (flags & RXapif_FETCH) {
        return reg_named_buff_fetch(rx, key, flags);
    } else if (flags & (RXapif_STORE | RXapif_DELETE | RXapif_CLEAR)) {
        Perl_croak_no_modify();
        return NULL;
    } else if (flags & RXapif_EXISTS) {
        return reg_named_buff_exists(rx, key, flags)
            ? &PL_sv_yes
            : &PL_sv_no;
    } else if (flags & RXapif_REGNAMES) {
        return reg_named_buff_all(rx, flags);
    } else if (flags & (RXapif_SCALAR | RXapif_REGNAMES_COUNT)) {
        return reg_named_buff_scalar(rx, flags);
    } else {
        Perl_croak(aTHX_ "panic: Unknown flags %d in named_buff", (int)flags);
        return NULL;
    }
}

SV*
Perl_reg_named_buff_iter(pTHX_ REGEXP * const rx, const SV * const lastkey,
                         const U32 flags)
{
    PERL_ARGS_ASSERT_REG_NAMED_BUFF_ITER;
    PERL_UNUSED_ARG(lastkey);

    if (flags & RXapif_FIRSTKEY)
        return reg_named_buff_firstkey(rx, flags);
    else if (flags & RXapif_NEXTKEY)
        return reg_named_buff_nextkey(rx, flags);
    else {
        Perl_croak(aTHX_ "panic: Unknown flags %d in named_buff_iter",
                                            (int)flags);
        return NULL;
    }
}

SV*
Perl_reg_named_buff_fetch(pTHX_ REGEXP * const r, SV * const namesv,
                          const U32 flags)
{
    SV *ret;
    struct regexp *const rx = ReANY(r);

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_FETCH;

    if (rx && RXp_PAREN_NAMES(rx)) {
        HE *he_str = hv_fetch_ent( RXp_PAREN_NAMES(rx), namesv, 0, 0 );
        if (he_str) {
            IV i;
            SV* sv_dat=HeVAL(he_str);
            I32 *nums=(I32*)SvPVX(sv_dat);
            AV * const retarray = (flags & RXapif_ALL) ? newAV_alloc_x(SvIVX(sv_dat)) : NULL;
            for ( i=0; i<SvIVX(sv_dat); i++ ) {
                if ((I32)(rx->nparens) >= nums[i]
                    && RXp_OFFS_VALID(rx,nums[i]))
                {
                    ret = newSVpvs("");
                    Perl_reg_numbered_buff_fetch_flags(aTHX_ r, nums[i], ret, REG_FETCH_ABSOLUTE);
                    if (!retarray)
                        return ret;
                } else {
                    if (retarray)
                        ret = newSV_type(SVt_NULL);
                }
                if (retarray)
                    av_push_simple(retarray, ret);
            }
            if (retarray)
                return newRV_noinc(MUTABLE_SV(retarray));
        }
    }
    return NULL;
}

bool
Perl_reg_named_buff_exists(pTHX_ REGEXP * const r, SV * const key,
                           const U32 flags)
{
    struct regexp *const rx = ReANY(r);

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_EXISTS;

    if (rx && RXp_PAREN_NAMES(rx)) {
        if (flags & RXapif_ALL) {
            return hv_exists_ent(RXp_PAREN_NAMES(rx), key, 0);
        } else {
            SV *sv = CALLREG_NAMED_BUFF_FETCH(r, key, flags);
            if (sv) {
                SvREFCNT_dec_NN(sv);
                return TRUE;
            } else {
                return FALSE;
            }
        }
    } else {
        return FALSE;
    }
}

SV*
Perl_reg_named_buff_firstkey(pTHX_ REGEXP * const r, const U32 flags)
{
    struct regexp *const rx = ReANY(r);

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_FIRSTKEY;

    if ( rx && RXp_PAREN_NAMES(rx) ) {
        (void)hv_iterinit(RXp_PAREN_NAMES(rx));

        return CALLREG_NAMED_BUFF_NEXTKEY(r, NULL, flags & ~RXapif_FIRSTKEY);
    } else {
        return FALSE;
    }
}

SV*
Perl_reg_named_buff_nextkey(pTHX_ REGEXP * const r, const U32 flags)
{
    struct regexp *const rx = ReANY(r);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_NEXTKEY;

    if (rx && RXp_PAREN_NAMES(rx)) {
        HV *hv = RXp_PAREN_NAMES(rx);
        HE *temphe;
        while ( (temphe = hv_iternext_flags(hv, 0)) ) {
            IV i;
            IV parno = 0;
            SV* sv_dat = HeVAL(temphe);
            I32 *nums = (I32*)SvPVX(sv_dat);
            for ( i = 0; i < SvIVX(sv_dat); i++ ) {
                if ((I32)(RXp_LASTPAREN(rx)) >= nums[i] &&
                    RXp_OFFS_VALID(rx,nums[i]))
                {
                    parno = nums[i];
                    break;
                }
            }
            if (parno || flags & RXapif_ALL) {
                return newSVhek(HeKEY_hek(temphe));
            }
        }
    }
    return NULL;
}

SV*
Perl_reg_named_buff_scalar(pTHX_ REGEXP * const r, const U32 flags)
{
    SV *ret;
    AV *av;
    SSize_t length;
    struct regexp *const rx = ReANY(r);

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_SCALAR;

    if (rx && RXp_PAREN_NAMES(rx)) {
        if (flags & (RXapif_ALL | RXapif_REGNAMES_COUNT)) {
            return newSViv(HvTOTALKEYS(RXp_PAREN_NAMES(rx)));
        } else if (flags & RXapif_ONE) {
            ret = CALLREG_NAMED_BUFF_ALL(r, (flags | RXapif_REGNAMES));
            av = MUTABLE_AV(SvRV(ret));
            length = av_count(av);
            SvREFCNT_dec_NN(ret);
            return newSViv(length);
        } else {
            Perl_croak(aTHX_ "panic: Unknown flags %d in named_buff_scalar",
                                                (int)flags);
            return NULL;
        }
    }
    return &PL_sv_undef;
}

SV*
Perl_reg_named_buff_all(pTHX_ REGEXP * const r, const U32 flags)
{
    struct regexp *const rx = ReANY(r);
    AV *av = newAV();

    PERL_ARGS_ASSERT_REG_NAMED_BUFF_ALL;

    if (rx && RXp_PAREN_NAMES(rx)) {
        HV *hv= RXp_PAREN_NAMES(rx);
        HE *temphe;
        (void)hv_iterinit(hv);
        while ( (temphe = hv_iternext_flags(hv, 0)) ) {
            IV i;
            IV parno = 0;
            SV* sv_dat = HeVAL(temphe);
            I32 *nums = (I32*)SvPVX(sv_dat);
            for ( i = 0; i < SvIVX(sv_dat); i++ ) {
                if ((I32)(RXp_LASTPAREN(rx)) >= nums[i] &&
                    RXp_OFFS_VALID(rx,nums[i]))
                {
                    parno = nums[i];
                    break;
                }
            }
            if (parno || flags & RXapif_ALL) {
                av_push_simple(av, newSVhek(HeKEY_hek(temphe)));
            }
        }
    }

    return newRV_noinc(MUTABLE_SV(av));
}

void
Perl_reg_numbered_buff_fetch(pTHX_ REGEXP * const re, const I32 paren,
                             SV * const sv)
{
    PERL_ARGS_ASSERT_REG_NUMBERED_BUFF_FETCH;
    Perl_reg_numbered_buff_fetch_flags(aTHX_ re, paren, sv, 0);
}

#ifndef PERL_IN_XSUB_RE

void
Perl_reg_numbered_buff_fetch_flags(pTHX_ REGEXP * const re, const I32 paren,
                                   SV * const sv, U32 flags)
{
    struct regexp *const rx = ReANY(re);
    char *s = NULL;
    SSize_t i,t = 0;
    SSize_t s1, t1;
    I32 n = paren;
    I32 logical_nparens = rx->logical_nparens ? rx->logical_nparens : rx->nparens;

    PERL_ARGS_ASSERT_REG_NUMBERED_BUFF_FETCH_FLAGS;

    if (      n == RX_BUFF_IDX_CARET_PREMATCH
           || n == RX_BUFF_IDX_CARET_FULLMATCH
           || n == RX_BUFF_IDX_CARET_POSTMATCH
       )
    {
        bool keepcopy = cBOOL(rx->extflags & RXf_PMf_KEEPCOPY);
        if (!keepcopy) {
            /* on something like
             *    $r = qr/.../;
             *    /$qr/p;
             * the KEEPCOPY is set on the PMOP rather than the regex */
            if (PL_curpm && re == PM_GETRE(PL_curpm))
                 keepcopy = cBOOL(PL_curpm->op_pmflags & PMf_KEEPCOPY);
        }
        if (!keepcopy)
            goto ret_undef;
    }

    if (!RXp_SUBBEG(rx))
        goto ret_undef;

    if (n == RX_BUFF_IDX_CARET_FULLMATCH)
        /* no need to distinguish between them any more */
        n = RX_BUFF_IDX_FULLMATCH;

    if ((n == RX_BUFF_IDX_PREMATCH || n == RX_BUFF_IDX_CARET_PREMATCH)
        && (i = RXp_OFFS_START(rx,0)) != -1)
    {
        /* $`, ${^PREMATCH} */
        s = RXp_SUBBEG(rx);
    }
    else
    if ((n == RX_BUFF_IDX_POSTMATCH || n == RX_BUFF_IDX_CARET_POSTMATCH)
        && (t = RXp_OFFS_END(rx,0)) != -1)
    {
        /* $', ${^POSTMATCH} */
        s = RXp_SUBBEG(rx) - RXp_SUBOFFSET(rx) + t;
        i = RXp_SUBLEN(rx) + RXp_SUBOFFSET(rx) - t;
    }
    else /* when flags is true we do an absolute lookup, and compare against rx->nparens */
    if (inRANGE(n, 0, flags ? (I32)rx->nparens : logical_nparens)) {
        I32 *map = (!flags && n) ? rx->logical_to_parno : NULL;
        I32 true_parno = map ? map[n] : n;
        do {
            if (((s1 = RXp_OFFS_START(rx,true_parno)) != -1)  &&
                ((t1 = RXp_OFFS_END(rx,true_parno)) != -1))
            {
                /* $&, ${^MATCH}, $1 ... */
                i = t1 - s1;
                s = RXp_SUBBEG(rx) + s1 - RXp_SUBOFFSET(rx);
                goto found_it;
            }
            else if (map) {
                true_parno = rx->parno_to_logical_next[true_parno];
            }
            else {
                break;
            }
        } while (true_parno);
        goto ret_undef;
    } else {
        goto ret_undef;
    }

  found_it:
    assert(s >= RXp_SUBBEG(rx));
    assert((STRLEN)RXp_SUBLEN(rx) >= (STRLEN)((s - RXp_SUBBEG(rx)) + i) );
    if (i >= 0) {
#ifdef NO_TAINT_SUPPORT
        sv_setpvn(sv, s, i);
#else
        const int oldtainted = TAINT_get;
        TAINT_NOT;
        sv_setpvn(sv, s, i);
        TAINT_set(oldtainted);
#endif
        if (RXp_MATCH_UTF8(rx))
            SvUTF8_on(sv);
        else
            SvUTF8_off(sv);
        if (TAINTING_get) {
            if (RXp_MATCH_TAINTED(rx)) {
                if (SvTYPE(sv) >= SVt_PVMG) {
                    MAGIC* const mg = SvMAGIC(sv);
                    MAGIC* mgt;
                    TAINT;
                    SvMAGIC_set(sv, mg->mg_moremagic);
                    SvTAINT(sv);
                    if ((mgt = SvMAGIC(sv))) {
                        mg->mg_moremagic = mgt;
                        SvMAGIC_set(sv, mg);
                    }
                } else {
                    TAINT;
                    SvTAINT(sv);
                }
            } else
                SvTAINTED_off(sv);
        }
    } else {
      ret_undef:
        sv_set_undef(sv);
        return;
    }
}

#endif

void
Perl_reg_numbered_buff_store(pTHX_ REGEXP * const rx, const I32 paren,
                                                         SV const * const value)
{
    PERL_ARGS_ASSERT_REG_NUMBERED_BUFF_STORE;

    PERL_UNUSED_ARG(rx);
    PERL_UNUSED_ARG(paren);
    PERL_UNUSED_ARG(value);

    if (!PL_localizing)
        Perl_croak_no_modify();
}

I32
Perl_reg_numbered_buff_length(pTHX_ REGEXP * const r, const SV * const sv,
                              const I32 paren)
{
    struct regexp *const rx = ReANY(r);
    I32 i,j;
    I32 s1, t1;
    I32 logical_nparens = rx->logical_nparens ? rx->logical_nparens : rx->nparens;

    PERL_ARGS_ASSERT_REG_NUMBERED_BUFF_LENGTH;

    if (   paren == RX_BUFF_IDX_CARET_PREMATCH
        || paren == RX_BUFF_IDX_CARET_FULLMATCH
        || paren == RX_BUFF_IDX_CARET_POSTMATCH
    )
    {
        bool keepcopy = cBOOL(rx->extflags & RXf_PMf_KEEPCOPY);
        if (!keepcopy) {
            /* on something like
             *    $r = qr/.../;
             *    /$qr/p;
             * the KEEPCOPY is set on the PMOP rather than the regex */
            if (PL_curpm && r == PM_GETRE(PL_curpm))
                 keepcopy = cBOOL(PL_curpm->op_pmflags & PMf_KEEPCOPY);
        }
        if (!keepcopy)
            goto warn_undef;
    }

    /* Some of this code was originally in C<Perl_magic_len> in F<mg.c> */
    switch (paren) {
      case RX_BUFF_IDX_CARET_PREMATCH: /* ${^PREMATCH} */
      case RX_BUFF_IDX_PREMATCH:       /* $` */
        if ( (i = RXp_OFFS_START(rx,0)) != -1) {
            if (i > 0) {
                s1 = 0;
                t1 = i;
                goto getlen;
            }
        }
        return 0;

      case RX_BUFF_IDX_CARET_POSTMATCH: /* ${^POSTMATCH} */
      case RX_BUFF_IDX_POSTMATCH:       /* $' */
        if ( (j = RXp_OFFS_END(rx,0)) != -1 ) {
            i = RXp_SUBLEN(rx) - j;
            if (i > 0) {
                s1 = j;
                t1 = RXp_SUBLEN(rx);
                goto getlen;
            }
        }
        return 0;

      default: /* $& / ${^MATCH}, $1, $2, ... */
        if (paren <= logical_nparens) {
            I32 true_paren = rx->logical_to_parno
                             ? rx->logical_to_parno[paren]
                             : paren;
            do {
                if (((s1 = RXp_OFFS_START(rx,true_paren)) != -1) &&
                    ((t1 = RXp_OFFS_END(rx,true_paren)) != -1))
                {
                    i = t1 - s1;
                    goto getlen;
                } else if (rx->parno_to_logical_next) {
                    true_paren = rx->parno_to_logical_next[true_paren];
                } else {
                    break;
                }
            } while(true_paren);
        }
      warn_undef:
        if (ckWARN(WARN_UNINITIALIZED))
            report_uninit((const SV *)sv);
        return 0;
    }
  getlen:
    if (i > 0 && RXp_MATCH_UTF8(rx)) {
        const char * const s = RXp_SUBBEG(rx) - RXp_SUBOFFSET(rx) + s1;
        const U8 *ep;
        STRLEN el;

        i = t1 - s1;
        if (is_utf8_string_loclen((U8*)s, i, &ep, &el))
            i = el;
    }
    return i;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
