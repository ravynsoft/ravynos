/*    regcomp.c
 */

/*
 * 'A fair jaw-cracker dwarf-language must be.'            --Samwise Gamgee
 *
 *     [p.285 of _The Lord of the Rings_, II/iii: "The Ring Goes South"]
 */

/* This file contains functions for compiling a regular expression.  See
 * also regexec.c which funnily enough, contains functions for executing
 * a regular expression.
 *
 * This file is also copied at build time to ext/re/re_comp.c, where
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
 * regexec to pregcomp and pregexec in order to avoid conflicts
 * with the POSIX routines of the same names.
*/

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

/* Note on debug output:
 *
 * This is set up so that -Dr turns on debugging like all other flags that are
 * enabled by -DDEBUGGING.  -Drv gives more verbose output.  This applies to
 * all regular expressions encountered in a program, and gives a huge amount of
 * output for all but the shortest programs.
 *
 * The ability to output pattern debugging information lexically, and with much
 * finer grained control was added, with 'use re qw(Debug ....);' available even
 * in non-DEBUGGING builds.  This is accomplished by copying the contents of
 * regcomp.c to ext/re/re_comp.c, and regexec.c is copied to ext/re/re_exec.c.
 * Those files are compiled and linked into the perl executable, and they are
 * compiled essentially as if DEBUGGING were enabled, and controlled by calls
 * to re.pm.
 *
 * That would normally mean linking errors when two functions of the same name
 * are attempted to be placed into the same executable.  That is solved in one
 * of four ways:
 *  1)  Static functions aren't known outside the file they are in, so for the
 *      many functions of that type in this file, it just isn't a problem.
 *  2)  Most externally known functions are enclosed in
 *          #ifndef PERL_IN_XSUB_RE
 *          ...
 *          #endif
 *      blocks, so there is only one definition for them in the whole
 *      executable, the one in regcomp.c (or regexec.c).  The implication of
 *      that is any debugging info that comes from them is controlled only by
 *      -Dr.  Further, any static function they call will also be the version
 *      in regcomp.c (or regexec.c), so its debugging will also be by -Dr.
 *  3)  About a dozen external functions are re-#defined in ext/re/re_top.h, to
 *      have different names, so that what gets loaded in the executable is
 *      'Perl_foo' from regcomp.c (and regexec.c), and the identical function
 *      from re_comp.c (and re_exec.c), but with the name 'my_foo'  Debugging
 *      in the 'Perl_foo' versions is controlled by -Dr, but the 'my_foo'
 *      versions and their callees are under control of re.pm.   The catch is
 *      that references to all these go through the regexp_engine structure,
 *      which is initialized in regcomp.h to the Perl_foo versions, and
 *      substituted out in lexical scopes where 'use re' is in effect to the
 *      'my_foo' ones.   That structure is public API, so it would be a hard
 *      sell to add any additional members.
 *  4)  For functions in regcomp.c and re_comp.c that are called only from,
 *      respectively, regexec.c and re_exec.c, they can have two different
 *      names, depending on #ifdef'ing PERL_IN_XSUB_RE, in both regexec.c and
 *      embed.fnc.
 *
 * The bottom line is that if you add code to one of the public functions
 * listed in ext/re/re_top.h, debugging automagically works.  But if you write
 * a new function that needs to do debugging or there is a chain of calls from
 * it that need to do debugging, all functions in the chain should use options
 * 2) or 4) above.
 *
 * A function may have to be split so that debugging stuff is static, but it
 * calls out to some other function that only gets compiled in regcomp.c to
 * access data that we don't want to duplicate.
 */

#ifdef PERL_EXT_RE_BUILD
#include "re_top.h"
#endif

#include "EXTERN.h"
#define PERL_IN_REGEX_ENGINE
#define PERL_IN_REGCOMP_ANY
#define PERL_IN_REGCOMP_C
#include "perl.h"

#ifdef PERL_IN_XSUB_RE
#  include "re_comp.h"
EXTERN_C const struct regexp_engine my_reg_engine;
EXTERN_C const struct regexp_engine wild_reg_engine;
#else
#  include "regcomp.h"
#endif

#include "invlist_inline.h"
#include "unicode_constants.h"
#include "regcomp_internal.h"

/* =========================================================
 * BEGIN edit_distance stuff.
 *
 * This calculates how many single character changes of any type are needed to
 * transform a string into another one.  It is taken from version 3.1 of
 *
 * https://metacpan.org/pod/Text::Levenshtein::Damerau::XS
 */

/* Our unsorted dictionary linked list.   */
/* Note we use UVs, not chars. */

struct dictionary{
  UV key;
  UV value;
  struct dictionary* next;
};
typedef struct dictionary item;


PERL_STATIC_INLINE item*
push(UV key, item* curr)
{
    item* head;
    Newx(head, 1, item);
    head->key = key;
    head->value = 0;
    head->next = curr;
    return head;
}


PERL_STATIC_INLINE item*
find(item* head, UV key)
{
    item* iterator = head;
    while (iterator){
        if (iterator->key == key){
            return iterator;
        }
        iterator = iterator->next;
    }

    return NULL;
}

PERL_STATIC_INLINE item*
uniquePush(item* head, UV key)
{
    item* iterator = head;

    while (iterator){
        if (iterator->key == key) {
            return head;
        }
        iterator = iterator->next;
    }

    return push(key, head);
}

PERL_STATIC_INLINE void
dict_free(item* head)
{
    item* iterator = head;

    while (iterator) {
        item* temp = iterator;
        iterator = iterator->next;
        Safefree(temp);
    }

    head = NULL;
}

/* End of Dictionary Stuff */

/* All calculations/work are done here */
STATIC int
S_edit_distance(const UV* src,
                const UV* tgt,
                const STRLEN x,             /* length of src[] */
                const STRLEN y,             /* length of tgt[] */
                const SSize_t maxDistance
)
{
    item *head = NULL;
    UV swapCount, swapScore, targetCharCount, i, j;
    UV *scores;
    UV score_ceil = x + y;

    PERL_ARGS_ASSERT_EDIT_DISTANCE;

    /* initialize matrix start values */
    Newx(scores, ( (x + 2) * (y + 2)), UV);
    scores[0] = score_ceil;
    scores[1 * (y + 2) + 0] = score_ceil;
    scores[0 * (y + 2) + 1] = score_ceil;
    scores[1 * (y + 2) + 1] = 0;
    head = uniquePush(uniquePush(head, src[0]), tgt[0]);

    /* work loops    */
    /* i = src index */
    /* j = tgt index */
    for (i=1;i<=x;i++) {
        if (i < x)
            head = uniquePush(head, src[i]);
        scores[(i+1) * (y + 2) + 1] = i;
        scores[(i+1) * (y + 2) + 0] = score_ceil;
        swapCount = 0;

        for (j=1;j<=y;j++) {
            if (i == 1) {
                if(j < y)
                head = uniquePush(head, tgt[j]);
                scores[1 * (y + 2) + (j + 1)] = j;
                scores[0 * (y + 2) + (j + 1)] = score_ceil;
            }

            targetCharCount = find(head, tgt[j-1])->value;
            swapScore = scores[targetCharCount * (y + 2) + swapCount] + i - targetCharCount - 1 + j - swapCount;

            if (src[i-1] != tgt[j-1]){
                scores[(i+1) * (y + 2) + (j + 1)] = MIN(swapScore,(MIN(scores[i * (y + 2) + j], MIN(scores[(i+1) * (y + 2) + j], scores[i * (y + 2) + (j + 1)])) + 1));
            }
            else {
                swapCount = j;
                scores[(i+1) * (y + 2) + (j + 1)] = MIN(scores[i * (y + 2) + j], swapScore);
            }
        }

        find(head, src[i-1])->value = i;
    }

    {
        IV score = scores[(x+1) * (y + 2) + (y + 1)];
        dict_free(head);
        Safefree(scores);
        return (maxDistance != 0 && maxDistance < score)?(-1):score;
    }
}

/* END of edit_distance() stuff
 * ========================================================= */

/* add a data member to the struct reg_data attached to this regex, it should
 * always return a non-zero return. the 's' argument is the type of the items
 * being added and the n is the number of items. The length of 's' should match
 * the number of items. */
U32
Perl_reg_add_data(RExC_state_t* const pRExC_state, const char* const s, const U32 n)
{
    U32 count = RExC_rxi->data ? RExC_rxi->data->count : 1;

    PERL_ARGS_ASSERT_REG_ADD_DATA;

    /* in the below expression we have (count + n - 1), the minus one is there
     * because the struct that we allocate already contains a slot for 1 data
     * item, so we do not need to allocate it the first time. IOW, the
     * sizeof(*RExC_rxi->data) already accounts for one of the elements we need
     * to allocate. See struct reg_data in regcomp.h
     */
    Renewc(RExC_rxi->data,
           sizeof(*RExC_rxi->data) + (sizeof(void*) * (count + n - 1)),
           char, struct reg_data);
    /* however in the data->what expression we use (count + n) and do not
     * subtract one from the result because the data structure contains a
     * pointer to an array, and does not allocate the first element as part of
     * the data struct. */
    if (count > 1)
        Renew(RExC_rxi->data->what, (count + n), U8);
    else {
        /* when count == 1 it means we have not initialized anything.
         * we always fill the 0 slot of the data array with a '%' entry, which
         * means "zero" (all the other types are letters) which exists purely
         * so the return from reg_add_data is ALWAYS true, so we can tell it apart
         * from a "no value" idx=0 in places where we would return an index
         * into reg_add_data.  This is particularly important with the new "single
         * pass, usually, but not always" strategy that we use, where the code
         * will use a 0 to represent "not able to compute this yet".
         */
        Newx(RExC_rxi->data->what, n+1, U8);
        /* fill in the placeholder slot of 0 with a what of '%', we use
         * this because it sorta looks like a zero (0/0) and it is not a letter
         * like any of the other "whats", this type should never be created
         * any other way but here. '%' happens to also not appear in this
         * file for any other reason (at the time of writing this comment)*/
        RExC_rxi->data->what[0]= '%';
        RExC_rxi->data->data[0]= NULL;
    }
    RExC_rxi->data->count = count + n;
    Copy(s, RExC_rxi->data->what + count, n, U8);
    assert(count>0);
    return count;
}

/*XXX: todo make this not included in a non debugging perl, but appears to be
 * used anyway there, in 'use re' */
#ifndef PERL_IN_XSUB_RE
void
Perl_reginitcolors(pTHX)
{
    const char * const s = PerlEnv_getenv("PERL_RE_COLORS");
    if (s) {
        char *t = savepv(s);
        int i = 0;
        PL_colors[0] = t;
        while (++i < 6) {
            t = strchr(t, '\t');
            if (t) {
                *t = '\0';
                PL_colors[i] = ++t;
            }
            else
                PL_colors[i] = t = (char *)"";
        }
    } else {
        int i = 0;
        while (i < 6)
            PL_colors[i++] = (char *)"";
    }
    PL_colorset = 1;
}
#endif


#ifdef TRIE_STUDY_OPT
/* search for "restudy" in this file for a detailed explanation */
#define CHECK_RESTUDY_GOTO_butfirst(dOsomething)            \
    STMT_START {                                            \
        if (                                                \
              (data.flags & SCF_TRIE_RESTUDY)               \
              && ! restudied++                              \
        ) {                                                 \
            dOsomething;                                    \
            goto reStudy;                                   \
        }                                                   \
    } STMT_END
#else
#define CHECK_RESTUDY_GOTO_butfirst
#endif

/*
 * pregcomp - compile a regular expression into internal code
 *
 * Decides which engine's compiler to call based on the hint currently in
 * scope
 */

#ifndef PERL_IN_XSUB_RE

/* return the currently in-scope regex engine (or the default if none)  */

regexp_engine const *
Perl_current_re_engine(pTHX)
{
    if (IN_PERL_COMPILETIME) {
        HV * const table = GvHV(PL_hintgv);
        SV **ptr;

        if (!table || !(PL_hints & HINT_LOCALIZE_HH))
            return &PL_core_reg_engine;
        ptr = hv_fetchs(table, "regcomp", FALSE);
        if ( !(ptr && SvIOK(*ptr) && SvIV(*ptr)))
            return &PL_core_reg_engine;
        return INT2PTR(regexp_engine*, SvIV(*ptr));
    }
    else {
        SV *ptr;
        if (!PL_curcop->cop_hints_hash)
            return &PL_core_reg_engine;
        ptr = cop_hints_fetch_pvs(PL_curcop, "regcomp", 0);
        if ( !(ptr && SvIOK(ptr) && SvIV(ptr)))
            return &PL_core_reg_engine;
        return INT2PTR(regexp_engine*, SvIV(ptr));
    }
}


REGEXP *
Perl_pregcomp(pTHX_ SV * const pattern, const U32 flags)
{
    regexp_engine const *eng = current_re_engine();
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_PREGCOMP;

    /* Dispatch a request to compile a regexp to correct regexp engine. */
    DEBUG_COMPILE_r({
        Perl_re_printf( aTHX_  "Using engine %" UVxf "\n",
                        PTR2UV(eng));
    });
    return CALLREGCOMP_ENG(eng, pattern, flags);
}
#endif

/*
=for apidoc re_compile

Compile the regular expression pattern C<pattern>, returning a pointer to the
compiled object for later matching with the internal regex engine.

This function is typically used by a custom regexp engine C<.comp()> function
to hand off to the core regexp engine those patterns it doesn't want to handle
itself (typically passing through the same flags it was called with).  In
almost all other cases, a regexp should be compiled by calling L</C<pregcomp>>
to compile using the currently active regexp engine.

If C<pattern> is already a C<REGEXP>, this function does nothing but return a
pointer to the input.  Otherwise the PV is extracted and treated like a string
representing a pattern.  See L<perlre>.

The possible flags for C<rx_flags> are documented in L<perlreapi>.  Their names
all begin with C<RXf_>.

=cut

 * public entry point for the perl core's own regex compiling code.
 * It's actually a wrapper for Perl_re_op_compile that only takes an SV
 * pattern rather than a list of OPs, and uses the internal engine rather
 * than the current one */

REGEXP *
Perl_re_compile(pTHX_ SV * const pattern, U32 rx_flags)
{
    SV *pat = pattern; /* defeat constness! */

    PERL_ARGS_ASSERT_RE_COMPILE;

    return Perl_re_op_compile(aTHX_ &pat, 1, NULL,
#ifdef PERL_IN_XSUB_RE
                                &my_reg_engine,
#else
                                &PL_core_reg_engine,
#endif
                                NULL, NULL, rx_flags, 0);
}

static void
S_free_codeblocks(pTHX_ struct reg_code_blocks *cbs)
{
    int n;

    if (--cbs->refcnt > 0)
        return;
    for (n = 0; n < cbs->count; n++) {
        REGEXP *rx = cbs->cb[n].src_regex;
        if (rx) {
            cbs->cb[n].src_regex = NULL;
            SvREFCNT_dec_NN(rx);
        }
    }
    Safefree(cbs->cb);
    Safefree(cbs);
}


static struct reg_code_blocks *
S_alloc_code_blocks(pTHX_  int ncode)
{
     struct reg_code_blocks *cbs;
    Newx(cbs, 1, struct reg_code_blocks);
    cbs->count = ncode;
    cbs->refcnt = 1;
    SAVEDESTRUCTOR_X(S_free_codeblocks, cbs);
    if (ncode)
        Newx(cbs->cb, ncode, struct reg_code_block);
    else
        cbs->cb = NULL;
    return cbs;
}


/* upgrade pattern pat_p of length plen_p to UTF8, and if there are code
 * blocks, recalculate the indices. Update pat_p and plen_p in-place to
 * point to the realloced string and length.
 *
 * This is essentially a copy of Perl_bytes_to_utf8() with the code index
 * stuff added */

static void
S_pat_upgrade_to_utf8(pTHX_ RExC_state_t * const pRExC_state,
                    char **pat_p, STRLEN *plen_p, int num_code_blocks)
{
    U8 *const src = (U8*)*pat_p;
    U8 *dst, *d;
    int n=0;
    STRLEN s = 0;
    bool do_end = 0;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    DEBUG_PARSE_r(Perl_re_printf( aTHX_
        "UTF8 mismatch! Converting to utf8 for resizing and compile\n"));

    /* 1 for each byte + 1 for each byte that expands to two, + trailing NUL */
    Newx(dst, *plen_p + variant_under_utf8_count(src, src + *plen_p) + 1, U8);
    d = dst;

    while (s < *plen_p) {
        append_utf8_from_native_byte(src[s], &d);

        if (n < num_code_blocks) {
            assert(pRExC_state->code_blocks);
            if (!do_end && pRExC_state->code_blocks->cb[n].start == s) {
                pRExC_state->code_blocks->cb[n].start = d - dst - 1;
                assert(*(d - 1) == '(');
                do_end = 1;
            }
            else if (do_end && pRExC_state->code_blocks->cb[n].end == s) {
                pRExC_state->code_blocks->cb[n].end = d - dst - 1;
                assert(*(d - 1) == ')');
                do_end = 0;
                n++;
            }
        }
        s++;
    }
    *d = '\0';
    *plen_p = d - dst;
    *pat_p = (char*) dst;
    SAVEFREEPV(*pat_p);
    RExC_orig_utf8 = RExC_utf8 = 1;
}



/* S_concat_pat(): concatenate a list of args to the pattern string pat,
 * while recording any code block indices, and handling overloading,
 * nested qr// objects etc.  If pat is null, it will allocate a new
 * string, or just return the first arg, if there's only one.
 *
 * Returns the malloced/updated pat.
 * patternp and pat_count is the array of SVs to be concatted;
 * oplist is the optional list of ops that generated the SVs;
 * recompile_p is a pointer to a boolean that will be set if
 *   the regex will need to be recompiled.
 * delim, if non-null is an SV that will be inserted between each element
 */

static SV*
S_concat_pat(pTHX_ RExC_state_t * const pRExC_state,
                SV *pat, SV ** const patternp, int pat_count,
                OP *oplist, bool *recompile_p, SV *delim)
{
    SV **svp;
    int n = 0;
    bool use_delim = FALSE;
    bool alloced = FALSE;

    /* if we know we have at least two args, create an empty string,
     * then concatenate args to that. For no args, return an empty string */
    if (!pat && pat_count != 1) {
        pat = newSVpvs("");
        SAVEFREESV(pat);
        alloced = TRUE;
    }

    for (svp = patternp; svp < patternp + pat_count; svp++) {
        SV *sv;
        SV *rx  = NULL;
        STRLEN orig_patlen = 0;
        bool code = 0;
        SV *msv = use_delim ? delim : *svp;
        if (!msv) msv = &PL_sv_undef;

        /* if we've got a delimiter, we go round the loop twice for each
         * svp slot (except the last), using the delimiter the second
         * time round */
        if (use_delim) {
            svp--;
            use_delim = FALSE;
        }
        else if (delim)
            use_delim = TRUE;

        if (SvTYPE(msv) == SVt_PVAV) {
            /* we've encountered an interpolated array within
             * the pattern, e.g. /...@a..../. Expand the list of elements,
             * then recursively append elements.
             * The code in this block is based on S_pushav() */

            AV *const av = (AV*)msv;
            const SSize_t maxarg = AvFILL(av) + 1;
            SV **array;

            if (oplist) {
                assert(oplist->op_type == OP_PADAV
                    || oplist->op_type == OP_RV2AV);
                oplist = OpSIBLING(oplist);
            }

            if (SvRMAGICAL(av)) {
                SSize_t i;

                Newx(array, maxarg, SV*);
                SAVEFREEPV(array);
                for (i=0; i < maxarg; i++) {
                    SV ** const svp = av_fetch(av, i, FALSE);
                    array[i] = svp ? *svp : &PL_sv_undef;
                }
            }
            else
                array = AvARRAY(av);

            if (maxarg > 0) {
                pat = S_concat_pat(aTHX_ pRExC_state, pat,
                                   array, maxarg, NULL, recompile_p,
                                   /* $" */
                                   GvSV((gv_fetchpvs("\"", GV_ADDMULTI, SVt_PV))));
            }
            else if (!pat) {
                pat = newSVpvs_flags("", SVs_TEMP);
            }

            continue;
        }


        /* we make the assumption here that each op in the list of
         * op_siblings maps to one SV pushed onto the stack,
         * except for code blocks, with have both an OP_NULL and
         * an OP_CONST.
         * This allows us to match up the list of SVs against the
         * list of OPs to find the next code block.
         *
         * Note that       PUSHMARK PADSV PADSV ..
         * is optimised to
         *                 PADRANGE PADSV  PADSV  ..
         * so the alignment still works. */

        if (oplist) {
            if (oplist->op_type == OP_NULL
                && (oplist->op_flags & OPf_SPECIAL))
            {
                assert(n < pRExC_state->code_blocks->count);
                pRExC_state->code_blocks->cb[n].start = pat ? SvCUR(pat) : 0;
                pRExC_state->code_blocks->cb[n].block = oplist;
                pRExC_state->code_blocks->cb[n].src_regex = NULL;
                n++;
                code = 1;
                oplist = OpSIBLING(oplist); /* skip CONST */
                assert(oplist);
            }
            oplist = OpSIBLING(oplist);;
        }

        /* apply magic and QR overloading to arg */

        SvGETMAGIC(msv);
        if (SvROK(msv) && SvAMAGIC(msv)) {
            SV *sv = AMG_CALLunary(msv, regexp_amg);
            if (sv) {
                if (SvROK(sv))
                    sv = SvRV(sv);
                if (SvTYPE(sv) != SVt_REGEXP)
                    Perl_croak(aTHX_ "Overloaded qr did not return a REGEXP");
                msv = sv;
            }
        }

        /* try concatenation overload ... */
        if (pat && (SvAMAGIC(pat) || SvAMAGIC(msv)) &&
                (sv = amagic_call(pat, msv, concat_amg, AMGf_assign)))
        {
            sv_setsv(pat, sv);
            /* overloading involved: all bets are off over literal
             * code. Pretend we haven't seen it */
            if (n)
                pRExC_state->code_blocks->count -= n;
            n = 0;
        }
        else {
            /* ... or failing that, try "" overload */
            while (SvAMAGIC(msv)
                    && (sv = AMG_CALLunary(msv, string_amg))
                    && sv != msv
                    &&  !(   SvROK(msv)
                          && SvROK(sv)
                          && SvRV(msv) == SvRV(sv))
            ) {
                msv = sv;
                SvGETMAGIC(msv);
            }
            if (SvROK(msv) && SvTYPE(SvRV(msv)) == SVt_REGEXP)
                msv = SvRV(msv);

            if (pat) {
                /* this is a partially unrolled
                 *     sv_catsv_nomg(pat, msv);
                 * that allows us to adjust code block indices if
                 * needed */
                STRLEN dlen;
                char *dst = SvPV_force_nomg(pat, dlen);
                orig_patlen = dlen;
                if (SvUTF8(msv) && !SvUTF8(pat)) {
                    S_pat_upgrade_to_utf8(aTHX_ pRExC_state, &dst, &dlen, n);
                    sv_setpvn(pat, dst, dlen);
                    SvUTF8_on(pat);
                }
                sv_catsv_nomg(pat, msv);
                rx = msv;
            }
            else {
                /* We have only one SV to process, but we need to verify
                 * it is properly null terminated or we will fail asserts
                 * later. In theory we probably shouldn't get such SV's,
                 * but if we do we should handle it gracefully. */
                if ( SvTYPE(msv) != SVt_PV || (SvLEN(msv) > SvCUR(msv) && *(SvEND(msv)) == 0) || SvIsCOW_shared_hash(msv) ) {
                    /* not a string, or a string with a trailing null */
                    pat = msv;
                } else {
                    /* a string with no trailing null, we need to copy it
                     * so it has a trailing null */
                    pat = sv_2mortal(newSVsv(msv));
                }
            }

            if (code)
                pRExC_state->code_blocks->cb[n-1].end = SvCUR(pat)-1;
        }

        /* extract any code blocks within any embedded qr//'s */
        if (rx && SvTYPE(rx) == SVt_REGEXP
            && RX_ENGINE((REGEXP*)rx)->op_comp)
        {

            RXi_GET_DECL(ReANY((REGEXP *)rx), ri);
            if (ri->code_blocks && ri->code_blocks->count) {
                int i;
                /* the presence of an embedded qr// with code means
                 * we should always recompile: the text of the
                 * qr// may not have changed, but it may be a
                 * different closure than last time */
                *recompile_p = 1;
                if (pRExC_state->code_blocks) {
                    int new_count = pRExC_state->code_blocks->count
                            + ri->code_blocks->count;
                    Renew(pRExC_state->code_blocks->cb,
                            new_count, struct reg_code_block);
                    pRExC_state->code_blocks->count = new_count;
                }
                else
                    pRExC_state->code_blocks = S_alloc_code_blocks(aTHX_
                                                    ri->code_blocks->count);

                for (i=0; i < ri->code_blocks->count; i++) {
                    struct reg_code_block *src, *dst;
                    STRLEN offset =  orig_patlen
                        + ReANY((REGEXP *)rx)->pre_prefix;
                    assert(n < pRExC_state->code_blocks->count);
                    src = &ri->code_blocks->cb[i];
                    dst = &pRExC_state->code_blocks->cb[n];
                    dst->start	    = src->start + offset;
                    dst->end	    = src->end   + offset;
                    dst->block	    = src->block;
                    dst->src_regex  = (REGEXP*) SvREFCNT_inc( (SV*)
                                            src->src_regex
                                                ? src->src_regex
                                                : (REGEXP*)rx);
                    n++;
                }
            }
        }
    }
    /* avoid calling magic multiple times on a single element e.g. =~ $qr */
    if (alloced)
        SvSETMAGIC(pat);

    return pat;
}



/* see if there are any run-time code blocks in the pattern.
 * False positives are allowed */

static bool
S_has_runtime_code(pTHX_ RExC_state_t * const pRExC_state,
                    char *pat, STRLEN plen)
{
    int n = 0;
    STRLEN s;

    PERL_UNUSED_CONTEXT;

    for (s = 0; s < plen; s++) {
        if (   pRExC_state->code_blocks
            && n < pRExC_state->code_blocks->count
            && s == pRExC_state->code_blocks->cb[n].start)
        {
            s = pRExC_state->code_blocks->cb[n].end;
            n++;
            continue;
        }
        /* TODO ideally should handle [..], (#..), /#.../x to reduce false
         * positives here */
        if (pat[s] == '(' && s+2 <= plen && pat[s+1] == '?' &&
            (pat[s+2] == '{'
                || (s + 2 <= plen && pat[s+2] == '?' && pat[s+3] == '{'))
        )
            return 1;
    }
    return 0;
}

/* Handle run-time code blocks. We will already have compiled any direct
 * or indirect literal code blocks. Now, take the pattern 'pat' and make a
 * copy of it, but with any literal code blocks blanked out and
 * appropriate chars escaped; then feed it into
 *
 *    eval "qr'modified_pattern'"
 *
 * For example,
 *
 *       a\bc(?{"this was literal"})def'ghi\\jkl(?{"this is runtime"})mno
 *
 * becomes
 *
 *    qr'a\\bc_______________________def\'ghi\\\\jkl(?{"this is runtime"})mno'
 *
 * After eval_sv()-ing that, grab any new code blocks from the returned qr
 * and merge them with any code blocks of the original regexp.
 *
 * If the pat is non-UTF8, while the evalled qr is UTF8, don't merge;
 * instead, just save the qr and return FALSE; this tells our caller that
 * the original pattern needs upgrading to utf8.
 */

static bool
S_compile_runtime_code(pTHX_ RExC_state_t * const pRExC_state,
    char *pat, STRLEN plen)
{
    SV *qr;

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    if (pRExC_state->runtime_code_qr) {
        /* this is the second time we've been called; this should
         * only happen if the main pattern got upgraded to utf8
         * during compilation; re-use the qr we compiled first time
         * round (which should be utf8 too)
         */
        qr = pRExC_state->runtime_code_qr;
        pRExC_state->runtime_code_qr = NULL;
        assert(RExC_utf8 && SvUTF8(qr));
    }
    else {
        int n = 0;
        STRLEN s;
        char *p, *newpat;
        int newlen = plen + 7; /* allow for "qr''xx\0" extra chars */
        SV *sv, *qr_ref;
        dSP;

        /* determine how many extra chars we need for ' and \ escaping */
        for (s = 0; s < plen; s++) {
            if (pat[s] == '\'' || pat[s] == '\\')
                newlen++;
        }

        Newx(newpat, newlen, char);
        p = newpat;
        *p++ = 'q'; *p++ = 'r'; *p++ = '\'';

        for (s = 0; s < plen; s++) {
            if (   pRExC_state->code_blocks
                && n < pRExC_state->code_blocks->count
                && s == pRExC_state->code_blocks->cb[n].start)
            {
                /* blank out literal code block so that they aren't
                 * recompiled: eg change from/to:
                 *     /(?{xyz})/
                 *     /(?=====)/
                 * and
                 *     /(??{xyz})/
                 *     /(?======)/
                 * and
                 *     /(?(?{xyz}))/
                 *     /(?(?=====))/
                */
                assert(pat[s]   == '(');
                assert(pat[s+1] == '?');
                *p++ = '(';
                *p++ = '?';
                s += 2;
                while (s < pRExC_state->code_blocks->cb[n].end) {
                    *p++ = '=';
                    s++;
                }
                *p++ = ')';
                n++;
                continue;
            }
            if (pat[s] == '\'' || pat[s] == '\\')
                *p++ = '\\';
            *p++ = pat[s];
        }
        *p++ = '\'';
        if (pRExC_state->pm_flags & RXf_PMf_EXTENDED) {
            *p++ = 'x';
            if (pRExC_state->pm_flags & RXf_PMf_EXTENDED_MORE) {
                *p++ = 'x';
            }
        }
        *p++ = '\0';
        DEBUG_COMPILE_r({
            Perl_re_printf( aTHX_
                "%sre-parsing pattern for runtime code:%s %s\n",
                PL_colors[4], PL_colors[5], newpat);
        });

        sv = newSVpvn_flags(newpat, p-newpat-1, RExC_utf8 ? SVf_UTF8 : 0);
        Safefree(newpat);

        ENTER;
        SAVETMPS;
        save_re_context();
        PUSHSTACKi(PERLSI_REQUIRE);
        /* G_RE_REPARSING causes the toker to collapse \\ into \ when
         * parsing qr''; normally only q'' does this. It also alters
         * hints handling */
        eval_sv(sv, G_SCALAR|G_RE_REPARSING);
        SvREFCNT_dec_NN(sv);
        SPAGAIN;
        qr_ref = POPs;
        PUTBACK;
        {
            SV * const errsv = ERRSV;
            if (SvTRUE_NN(errsv))
                /* use croak_sv ? */
                Perl_croak_nocontext("%" SVf, SVfARG(errsv));
        }
        assert(SvROK(qr_ref));
        qr = SvRV(qr_ref);
        assert(SvTYPE(qr) == SVt_REGEXP && RX_ENGINE((REGEXP*)qr)->op_comp);
        /* the leaving below frees the tmp qr_ref.
         * Give qr a life of its own */
        SvREFCNT_inc(qr);
        POPSTACK;
        FREETMPS;
        LEAVE;

    }

    if (!RExC_utf8 && SvUTF8(qr)) {
        /* first time through; the pattern got upgraded; save the
         * qr for the next time through */
        assert(!pRExC_state->runtime_code_qr);
        pRExC_state->runtime_code_qr = qr;
        return 0;
    }


    /* extract any code blocks within the returned qr//  */


    /* merge the main (r1) and run-time (r2) code blocks into one */
    {
        RXi_GET_DECL(ReANY((REGEXP *)qr), r2);
        struct reg_code_block *new_block, *dst;
        RExC_state_t * const r1 = pRExC_state; /* convenient alias */
        int i1 = 0, i2 = 0;
        int r1c, r2c;

        if (!r2->code_blocks || !r2->code_blocks->count) /* we guessed wrong */
        {
            SvREFCNT_dec_NN(qr);
            return 1;
        }

        if (!r1->code_blocks)
            r1->code_blocks = S_alloc_code_blocks(aTHX_ 0);

        r1c = r1->code_blocks->count;
        r2c = r2->code_blocks->count;

        Newx(new_block, r1c + r2c, struct reg_code_block);

        dst = new_block;

        while (i1 < r1c || i2 < r2c) {
            struct reg_code_block *src;
            bool is_qr = 0;

            if (i1 == r1c) {
                src = &r2->code_blocks->cb[i2++];
                is_qr = 1;
            }
            else if (i2 == r2c)
                src = &r1->code_blocks->cb[i1++];
            else if (  r1->code_blocks->cb[i1].start
                     < r2->code_blocks->cb[i2].start)
            {
                src = &r1->code_blocks->cb[i1++];
                assert(src->end < r2->code_blocks->cb[i2].start);
            }
            else {
                assert(  r1->code_blocks->cb[i1].start
                       > r2->code_blocks->cb[i2].start);
                src = &r2->code_blocks->cb[i2++];
                is_qr = 1;
                assert(src->end < r1->code_blocks->cb[i1].start);
            }

            assert(pat[src->start] == '(');
            assert(pat[src->end]   == ')');
            dst->start	    = src->start;
            dst->end	    = src->end;
            dst->block	    = src->block;
            dst->src_regex  = is_qr ? (REGEXP*) SvREFCNT_inc( (SV*) qr)
                                    : src->src_regex;
            dst++;
        }
        r1->code_blocks->count += r2c;
        Safefree(r1->code_blocks->cb);
        r1->code_blocks->cb = new_block;
    }

    SvREFCNT_dec_NN(qr);
    return 1;
}


STATIC bool
S_setup_longest(pTHX_ RExC_state_t *pRExC_state,
                      struct reg_substr_datum  *rsd,
                      struct scan_data_substrs *sub,
                      STRLEN longest_length)
{
    /* This is the common code for setting up the floating and fixed length
     * string data extracted from Perl_re_op_compile() below.  Returns a boolean
     * as to whether succeeded or not */

    I32 t;
    SSize_t ml;
    bool eol  = cBOOL(sub->flags & SF_BEFORE_EOL);
    bool meol = cBOOL(sub->flags & SF_BEFORE_MEOL);

    if (! (longest_length
           || (eol /* Can't have SEOL and MULTI */
               && (! meol || (RExC_flags & RXf_PMf_MULTILINE)))
          )
            /* See comments for join_exact for why REG_UNFOLDED_MULTI_SEEN */
        || (RExC_seen & REG_UNFOLDED_MULTI_SEEN))
    {
        return FALSE;
    }

    /* copy the information about the longest from the reg_scan_data
        over to the program. */
    if (SvUTF8(sub->str)) {
        rsd->substr      = NULL;
        rsd->utf8_substr = sub->str;
    } else {
        rsd->substr      = sub->str;
        rsd->utf8_substr = NULL;
    }
    /* end_shift is how many chars that must be matched that
        follow this item. We calculate it ahead of time as once the
        lookbehind offset is added in we lose the ability to correctly
        calculate it.*/
    ml = sub->minlenp ? *(sub->minlenp) : (SSize_t)longest_length;
    rsd->end_shift = ml - sub->min_offset
        - longest_length
            /* XXX SvTAIL is always false here - did you mean FBMcf_TAIL
             * intead? - DAPM
            + (SvTAIL(sub->str) != 0)
            */
        + sub->lookbehind;

    t = (eol/* Can't have SEOL and MULTI */
         && (! meol || (RExC_flags & RXf_PMf_MULTILINE)));
    fbm_compile(sub->str, t ? FBMcf_TAIL : 0);

    return TRUE;
}

STATIC void
S_set_regex_pv(pTHX_ RExC_state_t *pRExC_state, REGEXP *Rx)
{
    /* Calculates and sets in the compiled pattern 'Rx' the string to compile,
     * properly wrapped with the right modifiers */

    bool has_p     = ((RExC_rx->extflags & RXf_PMf_KEEPCOPY) == RXf_PMf_KEEPCOPY);
    bool has_charset = RExC_utf8 || (get_regex_charset(RExC_rx->extflags)
                                                != REGEX_DEPENDS_CHARSET);

    /* The caret is output if there are any defaults: if not all the STD
        * flags are set, or if no character set specifier is needed */
    bool has_default =
                (((RExC_rx->extflags & RXf_PMf_STD_PMMOD) != RXf_PMf_STD_PMMOD)
                || ! has_charset);
    bool has_runon = ((RExC_seen & REG_RUN_ON_COMMENT_SEEN)
                                                == REG_RUN_ON_COMMENT_SEEN);
    U8 reganch = (U8)((RExC_rx->extflags & RXf_PMf_STD_PMMOD)
                        >> RXf_PMf_STD_PMMOD_SHIFT);
    const char *fptr = STD_PAT_MODS;        /*"msixxn"*/
    char *p;
    STRLEN pat_len = RExC_precomp_end - RExC_precomp;

    /* We output all the necessary flags; we never output a minus, as all
        * those are defaults, so are
        * covered by the caret */
    const STRLEN wraplen = pat_len + has_p + has_runon
        + has_default       /* If needs a caret */
        + PL_bitcount[reganch] /* 1 char for each set standard flag */

            /* If needs a character set specifier */
        + ((has_charset) ? MAX_CHARSET_NAME_LENGTH : 0)
        + (sizeof("(?:)") - 1);

    PERL_ARGS_ASSERT_SET_REGEX_PV;

    /* make sure PL_bitcount bounds not exceeded */
    STATIC_ASSERT_STMT(sizeof(STD_PAT_MODS) <= 8);

    p = sv_grow(MUTABLE_SV(Rx), wraplen + 1); /* +1 for the ending NUL */
    SvPOK_on(Rx);
    if (RExC_utf8)
        SvFLAGS(Rx) |= SVf_UTF8;
    *p++='('; *p++='?';

    /* If a default, cover it using the caret */
    if (has_default) {
        *p++= DEFAULT_PAT_MOD;
    }
    if (has_charset) {
        STRLEN len;
        const char* name;

        name = get_regex_charset_name(RExC_rx->extflags, &len);
        if (strEQ(name, DEPENDS_PAT_MODS)) {  /* /d under UTF-8 => /u */
            assert(RExC_utf8);
            name = UNICODE_PAT_MODS;
            len = sizeof(UNICODE_PAT_MODS) - 1;
        }
        Copy(name, p, len, char);
        p += len;
    }
    if (has_p)
        *p++ = KEEPCOPY_PAT_MOD; /*'p'*/
    {
        char ch;
        while((ch = *fptr++)) {
            if(reganch & 1)
                *p++ = ch;
            reganch >>= 1;
        }
    }

    *p++ = ':';
    Copy(RExC_precomp, p, pat_len, char);
    assert ((RX_WRAPPED(Rx) - p) < 16);
    RExC_rx->pre_prefix = p - RX_WRAPPED(Rx);
    p += pat_len;

    /* Adding a trailing \n causes this to compile properly:
            my $R = qr / A B C # D E/x; /($R)/
        Otherwise the parens are considered part of the comment */
    if (has_runon)
        *p++ = '\n';
    *p++ = ')';
    *p = 0;
    SvCUR_set(Rx, p - RX_WRAPPED(Rx));
}

/*
 * Perl_re_op_compile - the perl internal RE engine's function to compile a
 * regular expression into internal code.
 * The pattern may be passed either as:
 *    a list of SVs (patternp plus pat_count)
 *    a list of OPs (expr)
 * If both are passed, the SV list is used, but the OP list indicates
 * which SVs are actually pre-compiled code blocks
 *
 * The SVs in the list have magic and qr overloading applied to them (and
 * the list may be modified in-place with replacement SVs in the latter
 * case).
 *
 * If the pattern hasn't changed from old_re, then old_re will be
 * returned.
 *
 * eng is the current engine. If that engine has an op_comp method, then
 * handle directly (i.e. we assume that op_comp was us); otherwise, just
 * do the initial concatenation of arguments and pass on to the external
 * engine.
 *
 * If is_bare_re is not null, set it to a boolean indicating whether the
 * arg list reduced (after overloading) to a single bare regex which has
 * been returned (i.e. /$qr/).
 *
 * orig_rx_flags contains RXf_* flags. See perlreapi.pod for more details.
 *
 * pm_flags contains the PMf_* flags, typically based on those from the
 * pm_flags field of the related PMOP. Currently we're only interested in
 * PMf_HAS_CV, PMf_IS_QR, PMf_USE_RE_EVAL, PMf_WILDCARD.
 *
 * For many years this code had an initial sizing pass that calculated
 * (sometimes incorrectly, leading to security holes) the size needed for the
 * compiled pattern.  That was changed by commit
 * 7c932d07cab18751bfc7515b4320436273a459e2 in 5.29, which reallocs the size, a
 * node at a time, as parsing goes along.  Patches welcome to fix any obsolete
 * references to this sizing pass.
 *
 * Now, an initial crude guess as to the size needed is made, based on the
 * length of the pattern.  Patches welcome to improve that guess.  That amount
 * of space is malloc'd and then immediately freed, and then clawed back node
 * by node.  This design is to minimize, to the extent possible, memory churn
 * when doing the reallocs.
 *
 * A separate parentheses counting pass may be needed in some cases.
 * (Previously the sizing pass did this.)  Patches welcome to reduce the number
 * of these cases.
 *
 * The existence of a sizing pass necessitated design decisions that are no
 * longer needed.  There are potential areas of simplification.
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.  [I'll say.]
 */

REGEXP *
Perl_re_op_compile(pTHX_ SV ** const patternp, int pat_count,
                    OP *expr, const regexp_engine* eng, REGEXP *old_re,
                     bool *is_bare_re, const U32 orig_rx_flags, const U32 pm_flags)
{
    REGEXP *Rx;         /* Capital 'R' means points to a REGEXP */
    STRLEN plen;
    char *exp;
    regnode *scan;
    I32 flags;
    SSize_t minlen = 0;
    U32 rx_flags;
    SV *pat;
    SV** new_patternp = patternp;

    /* these are all flags - maybe they should be turned
     * into a single int with different bit masks */
    I32 sawlookahead = 0;
    I32 sawplus = 0;
    I32 sawopen = 0;
    I32 sawminmod = 0;

    regex_charset initial_charset = get_regex_charset(orig_rx_flags);
    bool recompile = 0;
    bool runtime_code = 0;
    scan_data_t data;
    RExC_state_t RExC_state;
    RExC_state_t * const pRExC_state = &RExC_state;
#ifdef TRIE_STUDY_OPT
    /* search for "restudy" in this file for a detailed explanation */
    int restudied = 0;
    RExC_state_t copyRExC_state;
#endif
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_RE_OP_COMPILE;

    DEBUG_r(if (!PL_colorset) reginitcolors());


    pRExC_state->warn_text = NULL;
    pRExC_state->unlexed_names = NULL;
    pRExC_state->code_blocks = NULL;

    if (is_bare_re)
        *is_bare_re = FALSE;

    if (expr && (expr->op_type == OP_LIST ||
                (expr->op_type == OP_NULL && expr->op_targ == OP_LIST))) {
        /* allocate code_blocks if needed */
        OP *o;
        int ncode = 0;

        for (o = cLISTOPx(expr)->op_first; o; o = OpSIBLING(o))
            if (o->op_type == OP_NULL && (o->op_flags & OPf_SPECIAL))
                ncode++; /* count of DO blocks */

        if (ncode)
            pRExC_state->code_blocks = S_alloc_code_blocks(aTHX_ ncode);
    }

    if (!pat_count) {
        /* compile-time pattern with just OP_CONSTs and DO blocks */

        int n;
        OP *o;

        /* find how many CONSTs there are */
        assert(expr);
        n = 0;
        if (expr->op_type == OP_CONST)
            n = 1;
        else
            for (o = cLISTOPx(expr)->op_first; o; o = OpSIBLING(o)) {
                if (o->op_type == OP_CONST)
                    n++;
            }

        /* fake up an SV array */

        assert(!new_patternp);
        Newx(new_patternp, n, SV*);
        SAVEFREEPV(new_patternp);
        pat_count = n;

        n = 0;
        if (expr->op_type == OP_CONST)
            new_patternp[n] = cSVOPx_sv(expr);
        else
            for (o = cLISTOPx(expr)->op_first; o; o = OpSIBLING(o)) {
                if (o->op_type == OP_CONST)
                    new_patternp[n++] = cSVOPo_sv;
            }

    }

    DEBUG_PARSE_r(Perl_re_printf( aTHX_
        "Assembling pattern from %d elements%s\n", pat_count,
            orig_rx_flags & RXf_SPLIT ? " for split" : ""));

    /* set expr to the first arg op */

    if (pRExC_state->code_blocks && pRExC_state->code_blocks->count
         && expr->op_type != OP_CONST)
    {
            expr = cLISTOPx(expr)->op_first;
            assert(   expr->op_type == OP_PUSHMARK
                   || (expr->op_type == OP_NULL && expr->op_targ == OP_PUSHMARK)
                   || expr->op_type == OP_PADRANGE);
            expr = OpSIBLING(expr);
    }

    pat = S_concat_pat(aTHX_ pRExC_state, NULL, new_patternp, pat_count,
                        expr, &recompile, NULL);

    /* handle bare (possibly after overloading) regex: foo =~ $re */
    {
        SV *re = pat;
        if (SvROK(re))
            re = SvRV(re);
        if (SvTYPE(re) == SVt_REGEXP) {
            if (is_bare_re)
                *is_bare_re = TRUE;
            SvREFCNT_inc(re);
            DEBUG_PARSE_r(Perl_re_printf( aTHX_
                "Precompiled pattern%s\n",
                    orig_rx_flags & RXf_SPLIT ? " for split" : ""));

            return (REGEXP*)re;
        }
    }

    exp = SvPV_nomg(pat, plen);

    if (!eng->op_comp) {
        if ((SvUTF8(pat) && IN_BYTES)
                || SvGMAGICAL(pat) || SvAMAGIC(pat))
        {
            /* make a temporary copy; either to convert to bytes,
             * or to avoid repeating get-magic / overloaded stringify */
            pat = newSVpvn_flags(exp, plen, SVs_TEMP |
                                        (IN_BYTES ? 0 : SvUTF8(pat)));
        }
        return CALLREGCOMP_ENG(eng, pat, orig_rx_flags);
    }

    /* ignore the utf8ness if the pattern is 0 length */
    RExC_utf8 = RExC_orig_utf8 = (plen == 0 || IN_BYTES) ? 0 : SvUTF8(pat);
    RExC_uni_semantics = 0;
    RExC_contains_locale = 0;
    RExC_strict = cBOOL(pm_flags & RXf_PMf_STRICT);
    RExC_in_script_run = 0;
    RExC_study_started = 0;
    pRExC_state->runtime_code_qr = NULL;
    RExC_frame_head= NULL;
    RExC_frame_last= NULL;
    RExC_frame_count= 0;
    RExC_latest_warn_offset = 0;
    RExC_use_BRANCHJ = 0;
    RExC_warned_WARN_EXPERIMENTAL__VLB = 0;
    RExC_warned_WARN_EXPERIMENTAL__REGEX_SETS = 0;
    RExC_logical_total_parens = 0;
    RExC_total_parens = 0;
    RExC_logical_to_parno = NULL;
    RExC_parno_to_logical = NULL;
    RExC_open_parens = NULL;
    RExC_close_parens = NULL;
    RExC_paren_names = NULL;
    RExC_size = 0;
    RExC_seen_d_op = FALSE;
#ifdef DEBUGGING
    RExC_paren_name_list = NULL;
#endif

    DEBUG_r({
        RExC_mysv1= sv_newmortal();
        RExC_mysv2= sv_newmortal();
    });

    DEBUG_COMPILE_r({
            SV *dsv= sv_newmortal();
            RE_PV_QUOTED_DECL(s, RExC_utf8, dsv, exp, plen, PL_dump_re_max_len);
            Perl_re_printf( aTHX_  "%sCompiling REx%s %s\n",
                          PL_colors[4], PL_colors[5], s);
        });

    /* we jump here if we have to recompile, e.g., from upgrading the pattern
     * to utf8 */

    if ((pm_flags & PMf_USE_RE_EVAL)
                /* this second condition covers the non-regex literal case,
                 * i.e.  $foo =~ '(?{})'. */
                || (IN_PERL_COMPILETIME && (PL_hints & HINT_RE_EVAL))
    )
        runtime_code = S_has_runtime_code(aTHX_ pRExC_state, exp, plen);

  redo_parse:
    /* return old regex if pattern hasn't changed */
    /* XXX: note in the below we have to check the flags as well as the
     * pattern.
     *
     * Things get a touch tricky as we have to compare the utf8 flag
     * independently from the compile flags.  */

    if (   old_re
        && !recompile
        && cBOOL(RX_UTF8(old_re)) == cBOOL(RExC_utf8)
        && ( RX_COMPFLAGS(old_re) == ( orig_rx_flags & RXf_PMf_FLAGCOPYMASK ) )
        && RX_PRELEN(old_re) == plen
        && memEQ(RX_PRECOMP(old_re), exp, plen)
        && !runtime_code /* with runtime code, always recompile */ )
    {
        DEBUG_COMPILE_r({
            SV *dsv= sv_newmortal();
            RE_PV_QUOTED_DECL(s, RExC_utf8, dsv, exp, plen, PL_dump_re_max_len);
            Perl_re_printf( aTHX_  "%sSkipping recompilation of unchanged REx%s %s\n",
                          PL_colors[4], PL_colors[5], s);
        });
        return old_re;
    }

    /* Allocate the pattern's SV */
    RExC_rx_sv = Rx = (REGEXP*) newSV_type(SVt_REGEXP);
    RExC_rx = ReANY(Rx);
    if ( RExC_rx == NULL )
        FAIL("Regexp out of space");

    rx_flags = orig_rx_flags;

    if (   toUSE_UNI_CHARSET_NOT_DEPENDS
        && initial_charset == REGEX_DEPENDS_CHARSET)
    {

        /* Set to use unicode semantics if the pattern is in utf8 and has the
         * 'depends' charset specified, as it means unicode when utf8  */
        set_regex_charset(&rx_flags, REGEX_UNICODE_CHARSET);
        RExC_uni_semantics = 1;
    }

    RExC_pm_flags = pm_flags;

    if (runtime_code) {
        assert(TAINTING_get || !TAINT_get);
        if (TAINT_get)
            Perl_croak(aTHX_ "Eval-group in insecure regular expression");

        if (!S_compile_runtime_code(aTHX_ pRExC_state, exp, plen)) {
            /* whoops, we have a non-utf8 pattern, whilst run-time code
             * got compiled as utf8. Try again with a utf8 pattern */
            S_pat_upgrade_to_utf8(aTHX_ pRExC_state, &exp, &plen,
                pRExC_state->code_blocks ? pRExC_state->code_blocks->count : 0);
            goto redo_parse;
        }
    }
    assert(!pRExC_state->runtime_code_qr);

    RExC_sawback = 0;

    RExC_seen = 0;
    RExC_maxlen = 0;
    RExC_in_lookaround = 0;
    RExC_seen_zerolen = *exp == '^' ? -1 : 0;
    RExC_recode_x_to_native = 0;
    RExC_in_multi_char_class = 0;

    RExC_start = RExC_copy_start_in_constructed = RExC_copy_start_in_input = RExC_precomp = exp;
    RExC_precomp_end = RExC_end = exp + plen;
    RExC_nestroot = 0;
    RExC_whilem_seen = 0;
    RExC_end_op = NULL;
    RExC_recurse = NULL;
    RExC_study_chunk_recursed = NULL;
    RExC_study_chunk_recursed_bytes= 0;
    RExC_recurse_count = 0;
    RExC_sets_depth = 0;
    pRExC_state->code_index = 0;

    /* Initialize the string in the compiled pattern.  This is so that there is
     * something to output if necessary */
    set_regex_pv(pRExC_state, Rx);

    DEBUG_PARSE_r({
        Perl_re_printf( aTHX_
            "Starting parse and generation\n");
        RExC_lastnum=0;
        RExC_lastparse=NULL;
    });

    /* Allocate space and zero-initialize. Note, the two step process
       of zeroing when in debug mode, thus anything assigned has to
       happen after that */
    if (!  RExC_size) {

        /* On the first pass of the parse, we guess how big this will be.  Then
         * we grow in one operation to that amount and then give it back.  As
         * we go along, we re-allocate what we need.
         *
         * XXX Currently the guess is essentially that the pattern will be an
         * EXACT node with one byte input, one byte output.  This is crude, and
         * better heuristics are welcome.
         *
         * On any subsequent passes, we guess what we actually computed in the
         * latest earlier pass.  Such a pass probably didn't complete so is
         * missing stuff.  We could improve those guesses by knowing where the
         * parse stopped, and use the length so far plus apply the above
         * assumption to what's left. */
        RExC_size = STR_SZ(RExC_end - RExC_start);
    }

    Newxc(RExC_rxi, sizeof(regexp_internal) + RExC_size, char, regexp_internal);
    if ( RExC_rxi == NULL )
        FAIL("Regexp out of space");

    Zero(RExC_rxi, sizeof(regexp_internal) + RExC_size, char);
    RXi_SET( RExC_rx, RExC_rxi );

    /* We start from 0 (over from 0 in the case this is a reparse.  The first
     * node parsed will give back any excess memory we have allocated so far).
     * */
    RExC_size = 0;

    /* non-zero initialization begins here */
    RExC_rx->engine= eng;
    RExC_rx->extflags = rx_flags;
    RXp_COMPFLAGS(RExC_rx) = orig_rx_flags & RXf_PMf_FLAGCOPYMASK;

    if (pm_flags & PMf_IS_QR) {
        RExC_rxi->code_blocks = pRExC_state->code_blocks;
        if (RExC_rxi->code_blocks) {
            RExC_rxi->code_blocks->refcnt++;
        }
    }

    RExC_rx->intflags = 0;

    RExC_flags = rx_flags;	/* don't let top level (?i) bleed */
    RExC_parse_set(exp);

    /* This NUL is guaranteed because the pattern comes from an SV*, and the sv
     * code makes sure the final byte is an uncounted NUL.  But should this
     * ever not be the case, lots of things could read beyond the end of the
     * buffer: loops like
     *      while(isFOO(*RExC_parse)) RExC_parse_inc_by(1);
     *      strchr(RExC_parse, "foo");
     * etc.  So it is worth noting. */
    assert(*RExC_end == '\0');

    RExC_naughty = 0;
    RExC_npar = 1;
    RExC_logical_npar = 1;
    RExC_parens_buf_size = 0;
    RExC_emit_start = RExC_rxi->program;
    pRExC_state->code_index = 0;

    *((char*) RExC_emit_start) = (char) REG_MAGIC;
    RExC_emit = NODE_STEP_REGNODE;

    /* Do the parse */
    if (reg(pRExC_state, 0, &flags, 1)) {

        /* Success!, But we may need to redo the parse knowing how many parens
         * there actually are */
        if (IN_PARENS_PASS) {
            flags |= RESTART_PARSE;
        }

        /* We have that number in RExC_npar */
        RExC_total_parens = RExC_npar;
        RExC_logical_total_parens = RExC_logical_npar;
    }
    else if (! MUST_RESTART(flags)) {
        ReREFCNT_dec(Rx);
        Perl_croak(aTHX_ "panic: reg returned failure to re_op_compile, flags=%#" UVxf, (UV) flags);
    }

    /* Here, we either have success, or we have to redo the parse for some reason */
    if (MUST_RESTART(flags)) {

        /* It's possible to write a regexp in ascii that represents Unicode
        codepoints outside of the byte range, such as via \x{100}. If we
        detect such a sequence we have to convert the entire pattern to utf8
        and then recompile, as our sizing calculation will have been based
        on 1 byte == 1 character, but we will need to use utf8 to encode
        at least some part of the pattern, and therefore must convert the whole
        thing.
        -- dmq */
        if (flags & NEED_UTF8) {

            /* We have stored the offset of the final warning output so far.
             * That must be adjusted.  Any variant characters between the start
             * of the pattern and this warning count for 2 bytes in the final,
             * so just add them again */
            if (UNLIKELY(RExC_latest_warn_offset > 0)) {
                RExC_latest_warn_offset +=
                            variant_under_utf8_count((U8 *) exp, (U8 *) exp
                                                + RExC_latest_warn_offset);
            }
            S_pat_upgrade_to_utf8(aTHX_ pRExC_state, &exp, &plen,
            pRExC_state->code_blocks ? pRExC_state->code_blocks->count : 0);
            DEBUG_PARSE_r(Perl_re_printf( aTHX_ "Need to redo parse after upgrade\n"));
        }
        else {
            DEBUG_PARSE_r(Perl_re_printf( aTHX_ "Need to redo parse\n"));
        }

        if (ALL_PARENS_COUNTED) {
            /* Make enough room for all the known parens, and zero it */
            Renew(RExC_open_parens, RExC_total_parens, regnode_offset);
            Zero(RExC_open_parens, RExC_total_parens, regnode_offset);
            RExC_open_parens[0] = 1;    /* +1 for REG_MAGIC */

            Renew(RExC_close_parens, RExC_total_parens, regnode_offset);
            Zero(RExC_close_parens, RExC_total_parens, regnode_offset);
            /* we do NOT reinitialize  RExC_logical_to_parno and
             * RExC_parno_to_logical here. We need their data on the second
             * pass */
        }
        else { /* Parse did not complete.  Reinitialize the parentheses
                  structures */
            RExC_total_parens = 0;
            if (RExC_open_parens) {
                Safefree(RExC_open_parens);
                RExC_open_parens = NULL;
            }
            if (RExC_close_parens) {
                Safefree(RExC_close_parens);
                RExC_close_parens = NULL;
            }
            if (RExC_logical_to_parno) {
                Safefree(RExC_logical_to_parno);
                RExC_logical_to_parno = NULL;
            }
            if (RExC_parno_to_logical) {
                Safefree(RExC_parno_to_logical);
                RExC_parno_to_logical = NULL;
            }
        }

        /* Clean up what we did in this parse */
        SvREFCNT_dec_NN(RExC_rx_sv);

        goto redo_parse;
    }

    /* Here, we have successfully parsed and generated the pattern's program
     * for the regex engine.  We are ready to finish things up and look for
     * optimizations. */

    /* Update the string to compile, with correct modifiers, etc */
    set_regex_pv(pRExC_state, Rx);

    RExC_rx->nparens = RExC_total_parens - 1;
    RExC_rx->logical_nparens = RExC_logical_total_parens - 1;

    /* Uses the upper 4 bits of the FLAGS field, so keep within that size */
    if (RExC_whilem_seen > 15)
        RExC_whilem_seen = 15;

    DEBUG_PARSE_r({
        Perl_re_printf( aTHX_
            "Required size %" IVdf " nodes\n", (IV)RExC_size);
        RExC_lastnum=0;
        RExC_lastparse=NULL;
    });

    SetProgLen(RExC_rxi,RExC_size);

    DEBUG_DUMP_PRE_OPTIMIZE_r({
        SV * const sv = sv_newmortal();
        RXi_GET_DECL(RExC_rx, ri);
        DEBUG_RExC_seen();
        Perl_re_printf( aTHX_ "Program before optimization:\n");

        (void)dumpuntil(RExC_rx, ri->program, ri->program + 1, NULL, NULL,
                        sv, 0, 0);
    });

    DEBUG_OPTIMISE_r(
        Perl_re_printf( aTHX_  "Starting post parse optimization\n");
    );

    /* XXXX To minimize changes to RE engine we always allocate
       3-units-long substrs field. */
    Newx(RExC_rx->substrs, 1, struct reg_substr_data);
    if (RExC_recurse_count) {
        Newx(RExC_recurse, RExC_recurse_count, regnode *);
        SAVEFREEPV(RExC_recurse);
    }

    if (RExC_seen & REG_RECURSE_SEEN) {
        /* Note, RExC_total_parens is 1 + the number of parens in a pattern.
         * So its 1 if there are no parens. */
        RExC_study_chunk_recursed_bytes= (RExC_total_parens >> 3) +
                                         ((RExC_total_parens & 0x07) != 0);
        Newx(RExC_study_chunk_recursed,
             RExC_study_chunk_recursed_bytes * RExC_total_parens, U8);
        SAVEFREEPV(RExC_study_chunk_recursed);
    }

  reStudy:
    RExC_rx->minlen = minlen = sawlookahead = sawplus = sawopen = sawminmod = 0;
    DEBUG_r(
        RExC_study_chunk_recursed_count= 0;
    );
    Zero(RExC_rx->substrs, 1, struct reg_substr_data);
    if (RExC_study_chunk_recursed) {
        Zero(RExC_study_chunk_recursed,
             RExC_study_chunk_recursed_bytes * RExC_total_parens, U8);
    }


#ifdef TRIE_STUDY_OPT
    /* search for "restudy" in this file for a detailed explanation */
    if (!restudied) {
        StructCopy(&zero_scan_data, &data, scan_data_t);
        copyRExC_state = RExC_state;
    } else {
        U32 seen=RExC_seen;
        DEBUG_OPTIMISE_r(Perl_re_printf( aTHX_ "Restudying\n"));

        RExC_state = copyRExC_state;
        if (seen & REG_TOP_LEVEL_BRANCHES_SEEN)
            RExC_seen |= REG_TOP_LEVEL_BRANCHES_SEEN;
        else
            RExC_seen &= ~REG_TOP_LEVEL_BRANCHES_SEEN;
        StructCopy(&zero_scan_data, &data, scan_data_t);
    }
#else
    StructCopy(&zero_scan_data, &data, scan_data_t);
#endif

    /* Dig out information for optimizations. */
    RExC_rx->extflags = RExC_flags; /* was pm_op */
    /*dmq: removed as part of de-PMOP: pm->op_pmflags = RExC_flags; */

    if (UTF)
        SvUTF8_on(Rx);	/* Unicode in it? */
    RExC_rxi->regstclass = NULL;
    if (RExC_naughty >= TOO_NAUGHTY)	/* Probably an expensive pattern. */
        RExC_rx->intflags |= PREGf_NAUGHTY;
    scan = RExC_rxi->program + 1;		/* First BRANCH. */

    /* testing for BRANCH here tells us whether there is "must appear"
       data in the pattern. If there is then we can use it for optimisations */
    if (!(RExC_seen & REG_TOP_LEVEL_BRANCHES_SEEN)) { /*  Only one top-level choice.
                                                  */
        SSize_t fake_deltap;
        STRLEN longest_length[2];
        regnode_ssc ch_class; /* pointed to by data */
        int stclass_flag;
        SSize_t last_close = 0; /* pointed to by data */
        regnode *first= scan;
        regnode *first_next= regnext(first);
        regnode *last_close_op= NULL;
        int i;

        /*
         * Skip introductions and multiplicators >= 1
         * so that we can extract the 'meat' of the pattern that must
         * match in the large if() sequence following.
         * NOTE that EXACT is NOT covered here, as it is normally
         * picked up by the optimiser separately.
         *
         * This is unfortunate as the optimiser isnt handling lookahead
         * properly currently.
         *
         */
        while (1)
        {
            if (OP(first) == OPEN)
                sawopen = 1;
            else
            if (OP(first) == IFMATCH && !FLAGS(first))
                /* for now we can't handle lookbehind IFMATCH */
                sawlookahead = 1;
            else
            if (OP(first) == PLUS)
                sawplus = 1;
            else
            if (OP(first) == MINMOD)
                sawminmod = 1;
            else
            if (!(
                /* An OR of *one* alternative - should not happen now. */
                (OP(first) == BRANCH && OP(first_next) != BRANCH) ||
                /* An {n,m} with n>0 */
                (REGNODE_TYPE(OP(first)) == CURLY && ARG1i(first) > 0) ||
                (OP(first) == NOTHING && REGNODE_TYPE(OP(first_next)) != END)
            )){
                break;
            }

            first = REGNODE_AFTER(first);
            first_next= regnext(first);
        }

        /* Starting-point info. */
      again:
        DEBUG_PEEP("first:", first, 0, 0);
        /* Ignore EXACT as we deal with it later. */
        if (REGNODE_TYPE(OP(first)) == EXACT) {
            if (! isEXACTFish(OP(first))) {
                NOOP;	/* Empty, get anchored substr later. */
            }
            else
                RExC_rxi->regstclass = first;
        }
#ifdef TRIE_STCLASS
        else if (REGNODE_TYPE(OP(first)) == TRIE &&
                ((reg_trie_data *)RExC_rxi->data->data[ ARG1u(first) ])->minlen>0)
        {
            /* this can happen only on restudy
             * Search for "restudy" in this file to find
             * a comment with details. */
            RExC_rxi->regstclass = construct_ahocorasick_from_trie(pRExC_state, (regnode *)first, 0);
        }
#endif
        else if (REGNODE_SIMPLE(OP(first)))
            RExC_rxi->regstclass = first;
        else if (REGNODE_TYPE(OP(first)) == BOUND ||
                 REGNODE_TYPE(OP(first)) == NBOUND)
            RExC_rxi->regstclass = first;
        else if (REGNODE_TYPE(OP(first)) == BOL) {
            RExC_rx->intflags |= (OP(first) == MBOL
                           ? PREGf_ANCH_MBOL
                           : PREGf_ANCH_SBOL);
            first = REGNODE_AFTER(first);
            goto again;
        }
        else if (OP(first) == GPOS) {
            RExC_rx->intflags |= PREGf_ANCH_GPOS;
            first = REGNODE_AFTER_type(first,tregnode_GPOS);
            goto again;
        }
        else if ((!sawopen || !RExC_sawback) &&
            !sawlookahead &&
            (OP(first) == STAR &&
            REGNODE_TYPE(OP(REGNODE_AFTER(first))) == REG_ANY) &&
            !(RExC_rx->intflags & PREGf_ANCH) && !(RExC_seen & REG_PESSIMIZE_SEEN))
        {
            /* turn .* into ^.* with an implied $*=1 */
            const int type =
                (OP(REGNODE_AFTER(first)) == REG_ANY)
                    ? PREGf_ANCH_MBOL
                    : PREGf_ANCH_SBOL;
            RExC_rx->intflags |= (type | PREGf_IMPLICIT);
            first = REGNODE_AFTER(first);
            goto again;
        }
        if (sawplus && !sawminmod && !sawlookahead
            && (!sawopen || !RExC_sawback)
            && !(RExC_seen & REG_PESSIMIZE_SEEN)) /* May examine pos and $& */
            /* x+ must match at the 1st pos of run of x's */
            RExC_rx->intflags |= PREGf_SKIP;

        /* Scan is after the zeroth branch, first is atomic matcher. */
#ifdef TRIE_STUDY_OPT
        /* search for "restudy" in this file for a detailed explanation */
        DEBUG_PARSE_r(
            if (!restudied)
                Perl_re_printf( aTHX_  "first at %" IVdf "\n",
                              (IV)(first - scan + 1))
        );
#else
        DEBUG_PARSE_r(
            Perl_re_printf( aTHX_  "first at %" IVdf "\n",
                (IV)(first - scan + 1))
        );
#endif


        /*
        * If there's something expensive in the r.e., find the
        * longest literal string that must appear and make it the
        * regmust.  Resolve ties in favor of later strings, since
        * the regstart check works with the beginning of the r.e.
        * and avoiding duplication strengthens checking.  Not a
        * strong reason, but sufficient in the absence of others.
        * [Now we resolve ties in favor of the earlier string if
        * it happens that c_offset_min has been invalidated, since the
        * earlier string may buy us something the later one won't.]
        */

        data.substrs[0].str = newSVpvs("");
        data.substrs[1].str = newSVpvs("");
        data.last_found = newSVpvs("");
        data.cur_is_floating = 0; /* initially any found substring is fixed */
        ENTER_with_name("study_chunk");
        SAVEFREESV(data.substrs[0].str);
        SAVEFREESV(data.substrs[1].str);
        SAVEFREESV(data.last_found);
        first = scan;
        if (!RExC_rxi->regstclass) {
            ssc_init(pRExC_state, &ch_class);
            data.start_class = &ch_class;
            stclass_flag = SCF_DO_STCLASS_AND;
        } else				/* XXXX Check for BOUND? */
            stclass_flag = 0;
        data.last_closep = &last_close;
        data.last_close_opp = &last_close_op;

        DEBUG_RExC_seen();
        /*
         * MAIN ENTRY FOR study_chunk() FOR m/PATTERN/
         * (NO top level branches)
         */
        minlen = study_chunk(pRExC_state, &first, &minlen, &fake_deltap,
                             scan + RExC_size, /* Up to end */
            &data, -1, 0, NULL,
            SCF_DO_SUBSTR | SCF_WHILEM_VISITED_POS | stclass_flag
                          | (restudied ? SCF_TRIE_DOING_RESTUDY : 0),
            0, TRUE);
        /* search for "restudy" in this file for a detailed explanation
         * of 'restudied' and SCF_TRIE_DOING_RESTUDY */


        CHECK_RESTUDY_GOTO_butfirst(LEAVE_with_name("study_chunk"));


        if ( RExC_total_parens == 1 && !data.cur_is_floating
             && data.last_start_min == 0 && data.last_end > 0
             && !RExC_seen_zerolen
             && !(RExC_seen & REG_VERBARG_SEEN)
             && !(RExC_seen & REG_GPOS_SEEN)
        ){
            RExC_rx->extflags |= RXf_CHECK_ALL;
        }
        scan_commit(pRExC_state, &data,&minlen, 0);


        /* XXX this is done in reverse order because that's the way the
         * code was before it was parameterised. Don't know whether it
         * actually needs doing in reverse order. DAPM */
        for (i = 1; i >= 0; i--) {
            longest_length[i] = CHR_SVLEN(data.substrs[i].str);

            if (   !(   i
                     && SvCUR(data.substrs[0].str)  /* ok to leave SvCUR */
                     &&    data.substrs[0].min_offset
                        == data.substrs[1].min_offset
                     &&    SvCUR(data.substrs[0].str)
                        == SvCUR(data.substrs[1].str)
                    )
                && S_setup_longest (aTHX_ pRExC_state,
                                        &(RExC_rx->substrs->data[i]),
                                        &(data.substrs[i]),
                                        longest_length[i]))
            {
                RExC_rx->substrs->data[i].min_offset =
                        data.substrs[i].min_offset - data.substrs[i].lookbehind;

                RExC_rx->substrs->data[i].max_offset = data.substrs[i].max_offset;
                /* Don't offset infinity */
                if (data.substrs[i].max_offset < OPTIMIZE_INFTY)
                    RExC_rx->substrs->data[i].max_offset -= data.substrs[i].lookbehind;
                SvREFCNT_inc_simple_void_NN(data.substrs[i].str);
            }
            else {
                RExC_rx->substrs->data[i].substr      = NULL;
                RExC_rx->substrs->data[i].utf8_substr = NULL;
                longest_length[i] = 0;
            }
        }

        LEAVE_with_name("study_chunk");

        if (RExC_rxi->regstclass
            && (OP(RExC_rxi->regstclass) == REG_ANY || OP(RExC_rxi->regstclass) == SANY))
            RExC_rxi->regstclass = NULL;

        if ((!(RExC_rx->substrs->data[0].substr || RExC_rx->substrs->data[0].utf8_substr)
              || RExC_rx->substrs->data[0].min_offset)
            && stclass_flag
            && ! (ANYOF_FLAGS(data.start_class) & SSC_MATCHES_EMPTY_STRING)
            && is_ssc_worth_it(pRExC_state, data.start_class))
        {
            const U32 n = reg_add_data(pRExC_state, STR_WITH_LEN("f"));

            ssc_finalize(pRExC_state, data.start_class);

            Newx(RExC_rxi->data->data[n], 1, regnode_ssc);
            StructCopy(data.start_class,
                       (regnode_ssc*)RExC_rxi->data->data[n],
                       regnode_ssc);
            RExC_rxi->regstclass = (regnode*)RExC_rxi->data->data[n];
            RExC_rx->intflags &= ~PREGf_SKIP;	/* Used in find_byclass(). */
            DEBUG_COMPILE_r({ SV *sv = sv_newmortal();
                      regprop(RExC_rx, sv, (regnode*)data.start_class, NULL, pRExC_state);
                      Perl_re_printf( aTHX_
                                    "synthetic stclass \"%s\".\n",
                                    SvPVX_const(sv));});
            data.start_class = NULL;
        }

        /* A temporary algorithm prefers floated substr to fixed one of
         * same length to dig more info. */
        i = (longest_length[0] <= longest_length[1]);
        RExC_rx->substrs->check_ix = i;
        RExC_rx->check_end_shift  = RExC_rx->substrs->data[i].end_shift;
        RExC_rx->check_substr     = RExC_rx->substrs->data[i].substr;
        RExC_rx->check_utf8       = RExC_rx->substrs->data[i].utf8_substr;
        RExC_rx->check_offset_min = RExC_rx->substrs->data[i].min_offset;
        RExC_rx->check_offset_max = RExC_rx->substrs->data[i].max_offset;
        if (!i && (RExC_rx->intflags & (PREGf_ANCH_SBOL|PREGf_ANCH_GPOS)))
            RExC_rx->intflags |= PREGf_NOSCAN;

        if ((RExC_rx->check_substr || RExC_rx->check_utf8) ) {
            RExC_rx->extflags |= RXf_USE_INTUIT;
            if (SvTAIL(RExC_rx->check_substr ? RExC_rx->check_substr : RExC_rx->check_utf8))
                RExC_rx->extflags |= RXf_INTUIT_TAIL;
        }

        /* XXX Unneeded? dmq (shouldn't as this is handled elsewhere)
        if ( (STRLEN)minlen < longest_length[1] )
            minlen= longest_length[1];
        if ( (STRLEN)minlen < longest_length[0] )
            minlen= longest_length[0];
        */
    }
    else {
        /* Several toplevels. Best we can is to set minlen. */
        SSize_t fake_deltap;
        regnode_ssc ch_class;
        SSize_t last_close = 0;
        regnode *last_close_op = NULL;

        DEBUG_PARSE_r(Perl_re_printf( aTHX_  "\nMulti Top Level\n"));

        scan = RExC_rxi->program + 1;
        ssc_init(pRExC_state, &ch_class);
        data.start_class = &ch_class;
        data.last_closep = &last_close;
        data.last_close_opp = &last_close_op;

        DEBUG_RExC_seen();
        /*
         * MAIN ENTRY FOR study_chunk() FOR m/P1|P2|.../
         * (patterns WITH top level branches)
         */
        minlen = study_chunk(pRExC_state,
            &scan, &minlen, &fake_deltap, scan + RExC_size, &data, -1, 0, NULL,
            SCF_DO_STCLASS_AND|SCF_WHILEM_VISITED_POS|(restudied
                                                      ? SCF_TRIE_DOING_RESTUDY
                                                      : 0),
            0, TRUE);
        /* search for "restudy" in this file for a detailed explanation
         * of 'restudied' and SCF_TRIE_DOING_RESTUDY */

        CHECK_RESTUDY_GOTO_butfirst(NOOP);

        RExC_rx->check_substr = NULL;
        RExC_rx->check_utf8 = NULL;
        RExC_rx->substrs->data[0].substr      = NULL;
        RExC_rx->substrs->data[0].utf8_substr = NULL;
        RExC_rx->substrs->data[1].substr      = NULL;
        RExC_rx->substrs->data[1].utf8_substr = NULL;

        if (! (ANYOF_FLAGS(data.start_class) & SSC_MATCHES_EMPTY_STRING)
            && is_ssc_worth_it(pRExC_state, data.start_class))
        {
            const U32 n = reg_add_data(pRExC_state, STR_WITH_LEN("f"));

            ssc_finalize(pRExC_state, data.start_class);

            Newx(RExC_rxi->data->data[n], 1, regnode_ssc);
            StructCopy(data.start_class,
                       (regnode_ssc*)RExC_rxi->data->data[n],
                       regnode_ssc);
            RExC_rxi->regstclass = (regnode*)RExC_rxi->data->data[n];
            RExC_rx->intflags &= ~PREGf_SKIP;	/* Used in find_byclass(). */
            DEBUG_COMPILE_r({ SV* sv = sv_newmortal();
                      regprop(RExC_rx, sv, (regnode*)data.start_class, NULL, pRExC_state);
                      Perl_re_printf( aTHX_
                                    "synthetic stclass \"%s\".\n",
                                    SvPVX_const(sv));});
            data.start_class = NULL;
        }
    }

    if (RExC_seen & REG_UNBOUNDED_QUANTIFIER_SEEN) {
        RExC_rx->extflags |= RXf_UNBOUNDED_QUANTIFIER_SEEN;
        RExC_rx->maxlen = REG_INFTY;
    }
    else {
        RExC_rx->maxlen = RExC_maxlen;
    }

    /* Guard against an embedded (?=) or (?<=) with a longer minlen than
       the "real" pattern. */
    DEBUG_OPTIMISE_r({
        Perl_re_printf( aTHX_ "minlen: %" IVdf " RExC_rx->minlen:%" IVdf " maxlen:%" IVdf "\n",
                      (IV)minlen, (IV)RExC_rx->minlen, (IV)RExC_maxlen);
    });
    RExC_rx->minlenret = minlen;
    if (RExC_rx->minlen < minlen)
        RExC_rx->minlen = minlen;

    if (RExC_seen & REG_RECURSE_SEEN ) {
        RExC_rx->intflags |= PREGf_RECURSE_SEEN;
        Newx(RExC_rx->recurse_locinput, RExC_rx->nparens + 1, char *);
    }
    if (RExC_seen & REG_GPOS_SEEN)
        RExC_rx->intflags |= PREGf_GPOS_SEEN;

    if (RExC_seen & REG_PESSIMIZE_SEEN)
        RExC_rx->intflags |= PREGf_PESSIMIZE_SEEN;

    if (RExC_seen & REG_LOOKBEHIND_SEEN)
        RExC_rx->extflags |= RXf_NO_INPLACE_SUBST; /* inplace might break the
                                                lookbehind */
    if (pRExC_state->code_blocks)
        RExC_rx->extflags |= RXf_EVAL_SEEN;

    if (RExC_seen & REG_VERBARG_SEEN) {
        RExC_rx->intflags |= PREGf_VERBARG_SEEN;
        RExC_rx->extflags |= RXf_NO_INPLACE_SUBST; /* don't understand this! Yves */
    }

    if (RExC_seen & REG_CUTGROUP_SEEN)
        RExC_rx->intflags |= PREGf_CUTGROUP_SEEN;

    if (pm_flags & PMf_USE_RE_EVAL)
        RExC_rx->intflags |= PREGf_USE_RE_EVAL;

    if (RExC_paren_names)
        RXp_PAREN_NAMES(RExC_rx) = MUTABLE_HV(SvREFCNT_inc(RExC_paren_names));
    else
        RXp_PAREN_NAMES(RExC_rx) = NULL;

    /* If we have seen an anchor in our pattern then we set the extflag RXf_IS_ANCHORED
     * so it can be used in pp.c */
    if (RExC_rx->intflags & PREGf_ANCH)
        RExC_rx->extflags |= RXf_IS_ANCHORED;


    {
        /* this is used to identify "special" patterns that might result
         * in Perl NOT calling the regex engine and instead doing the match "itself",
         * particularly special cases in split//. By having the regex compiler
         * do this pattern matching at a regop level (instead of by inspecting the pattern)
         * we avoid weird issues with equivalent patterns resulting in different behavior,
         * AND we allow non Perl engines to get the same optimizations by the setting the
         * flags appropriately - Yves */
        regnode *first = RExC_rxi->program + 1;
        U8 fop = OP(first);
        regnode *next = NULL;
        U8 nop = 0;
        if (fop == NOTHING || fop == MBOL || fop == SBOL || fop == PLUS) {
            next = REGNODE_AFTER(first);
            nop = OP(next);
        }
        /* It's safe to read through *next only if OP(first) is a regop of
         * the right type (not EXACT, for example).
         */
        if (REGNODE_TYPE(fop) == NOTHING && nop == END)
            RExC_rx->extflags |= RXf_NULL;
        else if ((fop == MBOL || (fop == SBOL && !FLAGS(first))) && nop == END)
            /* when fop is SBOL first->flags will be true only when it was
             * produced by parsing /\A/, and not when parsing /^/. This is
             * very important for the split code as there we want to
             * treat /^/ as /^/m, but we do not want to treat /\A/ as /^/m.
             * See rt #122761 for more details. -- Yves */
            RExC_rx->extflags |= RXf_START_ONLY;
        else if (fop == PLUS
                 && REGNODE_TYPE(nop) == POSIXD && FLAGS(next) == CC_SPACE_
                 && OP(regnext(first)) == END)
            RExC_rx->extflags |= RXf_WHITE;
        else if ( RExC_rx->extflags & RXf_SPLIT
                  && (REGNODE_TYPE(fop) == EXACT && ! isEXACTFish(fop))
                  && STR_LEN(first) == 1
                  && *(STRING(first)) == ' '
                  && OP(regnext(first)) == END )
            RExC_rx->extflags |= (RXf_SKIPWHITE|RXf_WHITE);

    }

    if (RExC_contains_locale) {
        RXp_EXTFLAGS(RExC_rx) |= RXf_TAINTED;
    }

#ifdef DEBUGGING
    if (RExC_paren_names) {
        RExC_rxi->name_list_idx = reg_add_data( pRExC_state, STR_WITH_LEN("a"));
        RExC_rxi->data->data[RExC_rxi->name_list_idx]
                                   = (void*)SvREFCNT_inc(RExC_paren_name_list);
    } else
#endif
    RExC_rxi->name_list_idx = 0;

    while ( RExC_recurse_count > 0 ) {
        const regnode *scan = RExC_recurse[ --RExC_recurse_count ];
        /*
         * This data structure is set up in study_chunk() and is used
         * to calculate the distance between a GOSUB regopcode and
         * the OPEN/CURLYM (CURLYM's are special and can act like OPEN's)
         * it refers to.
         *
         * If for some reason someone writes code that optimises
         * away a GOSUB opcode then the assert should be changed to
         * an if(scan) to guard the ARG2i_SET() - Yves
         *
         */
        assert(scan && OP(scan) == GOSUB);
        ARG2i_SET( scan, RExC_open_parens[ARG1u(scan)] - REGNODE_OFFSET(scan));
    }
    if (RExC_logical_total_parens != RExC_total_parens) {
        Newxz(RExC_parno_to_logical_next, RExC_total_parens, I32);
        /* we rebuild this below */
        Zero(RExC_logical_to_parno, RExC_total_parens, I32);
        for( int parno = RExC_total_parens-1 ; parno > 0 ; parno-- ) {
            int logical_parno= RExC_parno_to_logical[parno];
            assert(logical_parno);
            RExC_parno_to_logical_next[parno]= RExC_logical_to_parno[logical_parno];
            RExC_logical_to_parno[logical_parno] = parno;
        }
        RExC_rx->logical_to_parno = RExC_logical_to_parno;
        RExC_rx->parno_to_logical = RExC_parno_to_logical;
        RExC_rx->parno_to_logical_next = RExC_parno_to_logical_next;
        RExC_logical_to_parno = NULL;
        RExC_parno_to_logical = NULL;
        RExC_parno_to_logical_next = NULL;
    } else {
        RExC_rx->logical_to_parno = NULL;
        RExC_rx->parno_to_logical = NULL;
        RExC_rx->parno_to_logical_next = NULL;
    }

    Newxz(RXp_OFFSp(RExC_rx), RExC_total_parens, regexp_paren_pair);
    /* assume we don't need to swap parens around before we match */
    DEBUG_TEST_r({
        Perl_re_printf( aTHX_ "study_chunk_recursed_count: %lu\n",
            (unsigned long)RExC_study_chunk_recursed_count);
    });
    DEBUG_DUMP_r({
        DEBUG_RExC_seen();
        Perl_re_printf( aTHX_ "Final program:\n");
        regdump(RExC_rx);
    });

    if (RExC_open_parens) {
        Safefree(RExC_open_parens);
        RExC_open_parens = NULL;
    }
    if (RExC_close_parens) {
        Safefree(RExC_close_parens);
        RExC_close_parens = NULL;
    }
    if (RExC_logical_to_parno) {
        Safefree(RExC_logical_to_parno);
        RExC_logical_to_parno = NULL;
    }
    if (RExC_parno_to_logical) {
        Safefree(RExC_parno_to_logical);
        RExC_parno_to_logical = NULL;
    }

#ifdef USE_ITHREADS
    /* under ithreads the ?pat? PMf_USED flag on the pmop is simulated
     * by setting the regexp SV to readonly-only instead. If the
     * pattern's been recompiled, the USEDness should remain. */
    if (old_re && SvREADONLY(old_re))
        SvREADONLY_on(Rx);
#endif
    return Rx;
}



SV*
Perl_reg_qr_package(pTHX_ REGEXP * const rx)
{
    PERL_ARGS_ASSERT_REG_QR_PACKAGE;
        PERL_UNUSED_ARG(rx);
        if (0)
            return NULL;
        else
            return newSVpvs("Regexp");
}

/* Scans the name of a named buffer from the pattern.
 * If flags is REG_RSN_RETURN_NULL returns null.
 * If flags is REG_RSN_RETURN_NAME returns an SV* containing the name
 * If flags is REG_RSN_RETURN_DATA returns the data SV* corresponding
 * to the parsed name as looked up in the RExC_paren_names hash.
 * If there is an error throws a vFAIL().. type exception.
 */

#define REG_RSN_RETURN_NULL    0
#define REG_RSN_RETURN_NAME    1
#define REG_RSN_RETURN_DATA    2

STATIC SV*
S_reg_scan_name(pTHX_ RExC_state_t *pRExC_state, U32 flags)
{
    char *name_start = RExC_parse;
    SV* sv_name;

    PERL_ARGS_ASSERT_REG_SCAN_NAME;

    assert (RExC_parse <= RExC_end);
    if (RExC_parse == RExC_end) NOOP;
    else if (isIDFIRST_lazy_if_safe(RExC_parse, RExC_end, UTF)) {
         /* Note that the code here assumes well-formed UTF-8.  Skip IDFIRST by
          * using do...while */
        if (UTF)
            do {
                RExC_parse_inc_utf8();
            } while (   RExC_parse < RExC_end
                     && isWORDCHAR_utf8_safe((U8*)RExC_parse, (U8*) RExC_end));
        else
            do {
                RExC_parse_inc_by(1);
            } while (RExC_parse < RExC_end && isWORDCHAR(*RExC_parse));
    } else {
        RExC_parse_inc_by(1); /* so the <- from the vFAIL is after the offending
                         character */
        vFAIL("Group name must start with a non-digit word character");
    }
    sv_name = newSVpvn_flags(name_start, (int)(RExC_parse - name_start),
                             SVs_TEMP | (UTF ? SVf_UTF8 : 0));
    if ( flags == REG_RSN_RETURN_NAME)
        return sv_name;
    else if (flags==REG_RSN_RETURN_DATA) {
        HE *he_str = NULL;
        SV *sv_dat = NULL;
        if ( ! sv_name )      /* should not happen*/
            Perl_croak(aTHX_ "panic: no svname in reg_scan_name");
        if (RExC_paren_names)
            he_str = hv_fetch_ent( RExC_paren_names, sv_name, 0, 0 );
        if ( he_str )
            sv_dat = HeVAL(he_str);
        if ( ! sv_dat ) {   /* Didn't find group */

            /* It might be a forward reference; we can't fail until we
                * know, by completing the parse to get all the groups, and
                * then reparsing */
            if (ALL_PARENS_COUNTED)  {
                vFAIL("Reference to nonexistent named group");
            }
            else {
                REQUIRE_PARENS_PASS;
            }
        }
        return sv_dat;
    }

    Perl_croak(aTHX_ "panic: bad flag %lx in reg_scan_name",
                     (unsigned long) flags);
}

#define DEBUG_PARSE_MSG(funcname)     DEBUG_PARSE_r({           \
    if (RExC_lastparse!=RExC_parse) {                           \
        Perl_re_printf( aTHX_  "%s",                            \
            Perl_pv_pretty(aTHX_ RExC_mysv1, RExC_parse,        \
                RExC_end - RExC_parse, 16,                      \
                "", "",                                         \
                PERL_PV_ESCAPE_UNI_DETECT |                     \
                PERL_PV_PRETTY_ELLIPSES   |                     \
                PERL_PV_PRETTY_LTGT       |                     \
                PERL_PV_ESCAPE_RE         |                     \
                PERL_PV_PRETTY_EXACTSIZE                        \
            )                                                   \
        );                                                      \
    } else                                                      \
        Perl_re_printf( aTHX_ "%16s","");                       \
                                                                \
    if (RExC_lastnum!=RExC_emit)                                \
       Perl_re_printf( aTHX_ "|%4zu", RExC_emit);                \
    else                                                        \
       Perl_re_printf( aTHX_ "|%4s","");                        \
    Perl_re_printf( aTHX_ "|%*s%-4s",                           \
        (int)((depth*2)), "",                                   \
        (funcname)                                              \
    );                                                          \
    RExC_lastnum=RExC_emit;                                     \
    RExC_lastparse=RExC_parse;                                  \
})



#define DEBUG_PARSE(funcname)     DEBUG_PARSE_r({           \
    DEBUG_PARSE_MSG((funcname));                            \
    Perl_re_printf( aTHX_ "%4s","\n");                                  \
})
#define DEBUG_PARSE_FMT(funcname,fmt,args)     DEBUG_PARSE_r({\
    DEBUG_PARSE_MSG((funcname));                            \
    Perl_re_printf( aTHX_ fmt "\n",args);                               \
})


STATIC void
S_parse_lparen_question_flags(pTHX_ RExC_state_t *pRExC_state)
{
    /* This parses the flags that are in either the '(?foo)' or '(?foo:bar)'
     * constructs, and updates RExC_flags with them.  On input, RExC_parse
     * should point to the first flag; it is updated on output to point to the
     * final ')' or ':'.  There needs to be at least one flag, or this will
     * abort */

    /* for (?g), (?gc), and (?o) warnings; warning
       about (?c) will warn about (?g) -- japhy    */

#define WASTED_O  0x01
#define WASTED_G  0x02
#define WASTED_C  0x04
#define WASTED_GC (WASTED_G|WASTED_C)
    I32 wastedflags = 0x00;
    U32 posflags = 0, negflags = 0;
    U32 *flagsp = &posflags;
    char has_charset_modifier = '\0';
    regex_charset cs;
    bool has_use_defaults = FALSE;
    const char* const seqstart = RExC_parse - 1; /* Point to the '?' */
    int x_mod_count = 0;

    PERL_ARGS_ASSERT_PARSE_LPAREN_QUESTION_FLAGS;

    /* '^' as an initial flag sets certain defaults */
    if (UCHARAT(RExC_parse) == '^') {
        RExC_parse_inc_by(1);
        has_use_defaults = TRUE;
        STD_PMMOD_FLAGS_CLEAR(&RExC_flags);
        cs = (toUSE_UNI_CHARSET_NOT_DEPENDS)
             ? REGEX_UNICODE_CHARSET
             : REGEX_DEPENDS_CHARSET;
        set_regex_charset(&RExC_flags, cs);
    }
    else {
        cs = get_regex_charset(RExC_flags);
        if (   cs == REGEX_DEPENDS_CHARSET
            && (toUSE_UNI_CHARSET_NOT_DEPENDS))
        {
            cs = REGEX_UNICODE_CHARSET;
        }
    }

    while (RExC_parse < RExC_end) {
        /* && memCHRs("iogcmsx", *RExC_parse) */
        /* (?g), (?gc) and (?o) are useless here
           and must be globally applied -- japhy */
        if ((RExC_pm_flags & PMf_WILDCARD)) {
            if (flagsp == & negflags) {
                if (*RExC_parse == 'm') {
                    RExC_parse_inc_by(1);
                    /* diag_listed_as: Use of %s is not allowed in Unicode
                       property wildcard subpatterns in regex; marked by <--
                       HERE in m/%s/ */
                    vFAIL("Use of modifier '-m' is not allowed in Unicode"
                          " property wildcard subpatterns");
                }
            }
            else {
                if (*RExC_parse == 's') {
                    goto modifier_illegal_in_wildcard;
                }
            }
        }

        switch (*RExC_parse) {

            /* Code for the imsxn flags */
            CASE_STD_PMMOD_FLAGS_PARSE_SET(flagsp, x_mod_count);

            case LOCALE_PAT_MOD:
                if (has_charset_modifier) {
                    goto excess_modifier;
                }
                else if (flagsp == &negflags) {
                    goto neg_modifier;
                }
                cs = REGEX_LOCALE_CHARSET;
                has_charset_modifier = LOCALE_PAT_MOD;
                break;
            case UNICODE_PAT_MOD:
                if (has_charset_modifier) {
                    goto excess_modifier;
                }
                else if (flagsp == &negflags) {
                    goto neg_modifier;
                }
                cs = REGEX_UNICODE_CHARSET;
                has_charset_modifier = UNICODE_PAT_MOD;
                break;
            case ASCII_RESTRICT_PAT_MOD:
                if (flagsp == &negflags) {
                    goto neg_modifier;
                }
                if (has_charset_modifier) {
                    if (cs != REGEX_ASCII_RESTRICTED_CHARSET) {
                        goto excess_modifier;
                    }
                    /* Doubled modifier implies more restricted */
                    cs = REGEX_ASCII_MORE_RESTRICTED_CHARSET;
                }
                else {
                    cs = REGEX_ASCII_RESTRICTED_CHARSET;
                }
                has_charset_modifier = ASCII_RESTRICT_PAT_MOD;
                break;
            case DEPENDS_PAT_MOD:
                if (has_use_defaults) {
                    goto fail_modifiers;
                }
                else if (flagsp == &negflags) {
                    goto neg_modifier;
                }
                else if (has_charset_modifier) {
                    goto excess_modifier;
                }

                /* The dual charset means unicode semantics if the
                 * pattern (or target, not known until runtime) are
                 * utf8, or something in the pattern indicates unicode
                 * semantics */
                cs = (toUSE_UNI_CHARSET_NOT_DEPENDS)
                     ? REGEX_UNICODE_CHARSET
                     : REGEX_DEPENDS_CHARSET;
                has_charset_modifier = DEPENDS_PAT_MOD;
                break;
              excess_modifier:
                RExC_parse_inc_by(1);
                if (has_charset_modifier == ASCII_RESTRICT_PAT_MOD) {
                    vFAIL2("Regexp modifier \"%c\" may appear a maximum of twice", ASCII_RESTRICT_PAT_MOD);
                }
                else if (has_charset_modifier == *(RExC_parse - 1)) {
                    vFAIL2("Regexp modifier \"%c\" may not appear twice",
                                        *(RExC_parse - 1));
                }
                else {
                    vFAIL3("Regexp modifiers \"%c\" and \"%c\" are mutually exclusive", has_charset_modifier, *(RExC_parse - 1));
                }
                NOT_REACHED; /*NOTREACHED*/
              neg_modifier:
                RExC_parse_inc_by(1);
                vFAIL2("Regexp modifier \"%c\" may not appear after the \"-\"",
                                    *(RExC_parse - 1));
                NOT_REACHED; /*NOTREACHED*/
            case GLOBAL_PAT_MOD: /* 'g' */
                if (RExC_pm_flags & PMf_WILDCARD) {
                    goto modifier_illegal_in_wildcard;
                }
                /*FALLTHROUGH*/
            case ONCE_PAT_MOD: /* 'o' */
                if (ckWARN(WARN_REGEXP)) {
                    const I32 wflagbit = *RExC_parse == 'o'
                                         ? WASTED_O
                                         : WASTED_G;
                    if (! (wastedflags & wflagbit) ) {
                        wastedflags |= wflagbit;
                        /* diag_listed_as: Useless (?-%s) - don't use /%s modifier in regex; marked by <-- HERE in m/%s/ */
                        vWARN5(
                            RExC_parse + 1,
                            "Useless (%s%c) - %suse /%c modifier",
                            flagsp == &negflags ? "?-" : "?",
                            *RExC_parse,
                            flagsp == &negflags ? "don't " : "",
                            *RExC_parse
                        );
                    }
                }
                break;

            case CONTINUE_PAT_MOD: /* 'c' */
                if (RExC_pm_flags & PMf_WILDCARD) {
                    goto modifier_illegal_in_wildcard;
                }
                if (ckWARN(WARN_REGEXP)) {
                    if (! (wastedflags & WASTED_C) ) {
                        wastedflags |= WASTED_GC;
                        /* diag_listed_as: Useless (?-%s) - don't use /%s modifier in regex; marked by <-- HERE in m/%s/ */
                        vWARN3(
                            RExC_parse + 1,
                            "Useless (%sc) - %suse /gc modifier",
                            flagsp == &negflags ? "?-" : "?",
                            flagsp == &negflags ? "don't " : ""
                        );
                    }
                }
                break;
            case KEEPCOPY_PAT_MOD: /* 'p' */
                if (RExC_pm_flags & PMf_WILDCARD) {
                    goto modifier_illegal_in_wildcard;
                }
                if (flagsp == &negflags) {
                    ckWARNreg(RExC_parse + 1,"Useless use of (?-p)");
                } else {
                    *flagsp |= RXf_PMf_KEEPCOPY;
                }
                break;
            case '-':
                /* A flag is a default iff it is following a minus, so
                 * if there is a minus, it means will be trying to
                 * re-specify a default which is an error */
                if (has_use_defaults || flagsp == &negflags) {
                    goto fail_modifiers;
                }
                flagsp = &negflags;
                wastedflags = 0;  /* reset so (?g-c) warns twice */
                x_mod_count = 0;
                break;
            case ':':
            case ')':

                if (  (RExC_pm_flags & PMf_WILDCARD)
                    && cs != REGEX_ASCII_MORE_RESTRICTED_CHARSET)
                {
                    RExC_parse_inc_by(1);
                    /* diag_listed_as: Use of %s is not allowed in Unicode
                       property wildcard subpatterns in regex; marked by <--
                       HERE in m/%s/ */
                    vFAIL2("Use of modifier '%c' is not allowed in Unicode"
                           " property wildcard subpatterns",
                           has_charset_modifier);
                }

                if ((posflags & (RXf_PMf_EXTENDED|RXf_PMf_EXTENDED_MORE)) == RXf_PMf_EXTENDED) {
                    negflags |= RXf_PMf_EXTENDED_MORE;
                }
                RExC_flags |= posflags;

                if (negflags & RXf_PMf_EXTENDED) {
                    negflags |= RXf_PMf_EXTENDED_MORE;
                }
                RExC_flags &= ~negflags;
                set_regex_charset(&RExC_flags, cs);

                return;
            default:
              fail_modifiers:
                RExC_parse_inc_if_char();
                /* diag_listed_as: Sequence (?%s...) not recognized in regex; marked by <-- HERE in m/%s/ */
                vFAIL2utf8f("Sequence (%" UTF8f "...) not recognized",
                      UTF8fARG(UTF, RExC_parse-seqstart, seqstart));
                NOT_REACHED; /*NOTREACHED*/
        }

        RExC_parse_inc();
    }

    vFAIL("Sequence (?... not terminated");

  modifier_illegal_in_wildcard:
    RExC_parse_inc_by(1);
    /* diag_listed_as: Use of %s is not allowed in Unicode property wildcard
       subpatterns in regex; marked by <-- HERE in m/%s/ */
    vFAIL2("Use of modifier '%c' is not allowed in Unicode property wildcard"
           " subpatterns", *(RExC_parse - 1));
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */

STATIC regnode_offset
S_handle_named_backref(pTHX_ RExC_state_t *pRExC_state,
                             I32 *flagp,
                             char * backref_parse_start,
                             char ch
                      )
{
    regnode_offset ret;
    char* name_start = RExC_parse;
    U32 num = 0;
    SV *sv_dat = reg_scan_name(pRExC_state, REG_RSN_RETURN_DATA);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_HANDLE_NAMED_BACKREF;

    if (RExC_parse != name_start && ch == '}') {
        while (isBLANK(*RExC_parse)) {
            RExC_parse_inc_by(1);
        }
    }
    if (RExC_parse == name_start || *RExC_parse != ch) {
        /* diag_listed_as: Sequence \%s... not terminated in regex; marked by <-- HERE in m/%s/ */
        vFAIL2("Sequence %.3s... not terminated", backref_parse_start);
    }

    if (sv_dat) {
        num = reg_add_data( pRExC_state, STR_WITH_LEN("S"));
        RExC_rxi->data->data[num]=(void*)sv_dat;
        SvREFCNT_inc_simple_void_NN(sv_dat);
    }
    RExC_sawback = 1;
    ret = reg2node(pRExC_state,
                   ((! FOLD)
                     ? REFN
                     : (ASCII_FOLD_RESTRICTED)
                       ? REFFAN
                       : (AT_LEAST_UNI_SEMANTICS)
                         ? REFFUN
                         : (LOC)
                           ? REFFLN
                           : REFFN),
                    num, RExC_nestroot);
    if (RExC_nestroot && num >= (U32)RExC_nestroot)
        FLAGS(REGNODE_p(ret)) = VOLATILE_REF;
    *flagp |= HASWIDTH;

    nextchar(pRExC_state);
    return ret;
}

/* reg_la_NOTHING()
 *
 * Maybe parse a parenthesized lookaround construct that is equivalent to a
 * NOTHING regop when the construct is empty.
 *
 * Calls skip_to_be_ignored_text() before checking if the construct is empty.
 *
 * Checks for unterminated constructs and throws a "not terminated" error
 * with the appropriate type if necessary
 *
 * Assuming it does not throw an exception increments RExC_seen_zerolen.
 *
 * If the construct is empty generates a NOTHING op and returns its
 * regnode_offset, which the caller would then return to its caller.
 *
 * If the construct is not empty increments RExC_in_lookaround, and turns
 * on any flags provided in RExC_seen, and then returns 0 to signify
 * that parsing should continue.
 *
 * PS: I would have called this reg_parse_lookaround_NOTHING() but then
 * any use of it would have had to be broken onto multiple lines, hence
 * the abbreviation.
 */
STATIC regnode_offset
S_reg_la_NOTHING(pTHX_ RExC_state_t *pRExC_state, U32 flags,
    const char *type)
{

    PERL_ARGS_ASSERT_REG_LA_NOTHING;

    /* false below so we do not force /x */
    skip_to_be_ignored_text(pRExC_state, &RExC_parse, FALSE);

    if (RExC_parse >= RExC_end)
        vFAIL2("Sequence (%s... not terminated", type);

    /* Always increment as NOTHING regops are zerolen */
    RExC_seen_zerolen++;

    if (*RExC_parse == ')') {
        regnode_offset ret= reg_node(pRExC_state, NOTHING);
        nextchar(pRExC_state);
        return ret;
    }

    RExC_seen |= flags;
    RExC_in_lookaround++;
    return 0; /* keep parsing! */
}

/* reg_la_OPFAIL()
 *
 * Maybe parse a parenthesized lookaround construct that is equivalent to a
 * OPFAIL regop when the construct is empty.
 *
 * Calls skip_to_be_ignored_text() before checking if the construct is empty.
 *
 * Checks for unterminated constructs and throws a "not terminated" error
 * if necessary.
 *
 * If the construct is empty generates an OPFAIL op and returns its
 * regnode_offset which the caller should then return to its caller.
 *
 * If the construct is not empty increments RExC_in_lookaround, and also
 * increments RExC_seen_zerolen, and turns on the flags provided in
 * RExC_seen, and then returns 0 to signify that parsing should continue.
 *
 * PS: I would have called this reg_parse_lookaround_OPFAIL() but then
 * any use of it would have had to be broken onto multiple lines, hence
 * the abbreviation.
 */

STATIC regnode_offset
S_reg_la_OPFAIL(pTHX_ RExC_state_t *pRExC_state, U32 flags,
    const char *type)
{

    PERL_ARGS_ASSERT_REG_LA_OPFAIL;

    /* FALSE so we don't force to /x below */;
    skip_to_be_ignored_text(pRExC_state, &RExC_parse, FALSE);

    if (RExC_parse >= RExC_end)
        vFAIL2("Sequence (%s... not terminated", type);

    if (*RExC_parse == ')') {
        regnode_offset ret= reg1node(pRExC_state, OPFAIL, 0);
        nextchar(pRExC_state);
        return ret; /* return produced regop */
    }

    /* only increment zerolen *after* we check if we produce an OPFAIL
     * as an OPFAIL does not match a zero length construct, as it
     * does not match ever. */
    RExC_seen_zerolen++;
    RExC_seen |= flags;
    RExC_in_lookaround++;
    return 0; /* keep parsing! */
}

/* Below are the main parsing routines.
 *
 * S_reg()      parses a whole pattern or subpattern.  It itself handles things
 *              like the 'xyz' in '(?xyz:...)', and calls S_regbranch for each
 *              alternation '|' in the '...' pattern.
 * S_regbranch() effectively implements the concatenation operator, handling
 *              one alternative of '|', repeatedly calling S_regpiece on each
 *              segment of the input.
 * S_regpiece() calls S_regatom to handle the next atomic chunk of the input,
 *              and then adds any quantifier for that chunk.
 * S_regatom()  parses the next chunk of the input, returning when it
 *              determines it has found a complete atomic chunk.  The chunk may
 *              be a nested subpattern, in which case S_reg is called
 *              recursively
 *
 * The functions generate regnodes as they go along, appending each to the
 * pattern data structure so far.  They return the offset of the current final
 * node into that structure, or 0 on failure.
 *
 * There are three parameters common to all of them:
 *   pRExC_state    is a structure with much information about the current
 *                  state of the parse.  It's easy to add new elements to
 *                  convey new information, but beware that an error return may
 *                  require clearing the element.
 *   flagp          is a pointer to bit flags set in a lower level to pass up
 *                  to higher levels information, such as the cause of a
 *                  failure, or some characteristic about the generated node
 *   depth          is roughly the recursion depth, mostly unused except for
 *                  pretty printing debugging info.
 *
 * There are ancillary functions that these may farm work out to, using the
 * same parameters.
 *
 * The protocol for handling flags is that each function will, before
 * returning, add into *flagp the flags it needs to pass up.  Each function has
 * a second flags variable, typically named 'flags', which it sets and clears
 * at will.  Flag bits in it are used in that function, and it calls the next
 * layer down with its 'flagp' parameter set to '&flags'.  Thus, upon return,
 * 'flags' will contain whatever it had before the call, plus whatever that
 * function passed up.  If it wants to pass any of these up to its caller, it
 * has to add them to its *flagp.  This means that it takes extra steps to keep
 * passing a flag upwards, and otherwise the flag bit is cleared for higher
 * functions.
 */

/* On success, returns the offset at which any next node should be placed into
 * the regex engine program being compiled.
 *
 * Returns 0 otherwise, with *flagp set to indicate why:
 *  TRYAGAIN        at the end of (?) that only sets flags.
 *  RESTART_PARSE   if the parse needs to be restarted, or'd with
 *                  NEED_UTF8 if the pattern needs to be upgraded to UTF-8.
 *  Otherwise would only return 0 if regbranch() returns 0, which cannot
 *  happen.  */
STATIC regnode_offset
S_reg(pTHX_ RExC_state_t *pRExC_state, I32 paren, I32 *flagp, U32 depth)
    /* paren: Parenthesized? 0=top; 1,2=inside '(': changed to letter.
     * 2 is like 1, but indicates that nextchar() has been called to advance
     * RExC_parse beyond the '('.  Things like '(?' are indivisible tokens, and
     * this flag alerts us to the need to check for that */
{
    regnode_offset ret = 0;    /* Will be the head of the group. */
    regnode_offset br;
    regnode_offset lastbr;
    regnode_offset ender = 0;
    I32 logical_parno = 0;
    I32 parno = 0;
    I32 flags;
    U32 oregflags = RExC_flags;
    bool have_branch = 0;
    bool is_open = 0;
    I32 freeze_paren = 0;
    I32 after_freeze = 0;
    I32 num; /* numeric backreferences */
    SV * max_open;  /* Max number of unclosed parens */
    I32 was_in_lookaround = RExC_in_lookaround;
    I32 fake_eval = 0; /* matches paren */

    /* The difference between the following variables can be seen with  *
     * the broken pattern /(?:foo/ where segment_parse_start will point *
     * at the 'f', and reg_parse_start will point at the '('            */

    /* the following is used for unmatched '(' errors */
    char * const reg_parse_start = RExC_parse;

    /* the following is used to track where various segments of
     * the pattern that we parse out started. */
    char * segment_parse_start = RExC_parse;

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REG;
    DEBUG_PARSE("reg ");

    max_open = get_sv(RE_COMPILE_RECURSION_LIMIT, GV_ADD);
    assert(max_open);
    if (!SvIOK(max_open)) {
        sv_setiv(max_open, RE_COMPILE_RECURSION_INIT);
    }
    if (depth > 4 * (UV) SvIV(max_open)) { /* We increase depth by 4 for each
                                              open paren */
        vFAIL("Too many nested open parens");
    }

    *flagp = 0;				/* Initialize. */

    /* Having this true makes it feasible to have a lot fewer tests for the
     * parse pointer being in scope.  For example, we can write
     *      while(isFOO(*RExC_parse)) RExC_parse_inc_by(1);
     * instead of
     *      while(RExC_parse < RExC_end && isFOO(*RExC_parse)) RExC_parse_inc_by(1);
     */
    assert(*RExC_end == '\0');

    /* Make an OPEN node, if parenthesized. */
    if (paren) {

        /* Under /x, space and comments can be gobbled up between the '(' and
         * here (if paren ==2).  The forms '(*VERB' and '(?...' disallow such
         * intervening space, as the sequence is a token, and a token should be
         * indivisible */
        bool has_intervening_patws = (paren == 2)
                                  && *(RExC_parse - 1) != '(';

        if (RExC_parse >= RExC_end) {
            vFAIL("Unmatched (");
        }

        if (paren == 'r') {     /* Atomic script run */
            paren = '>';
            goto parse_rest;
        }
        else if ( *RExC_parse == '*') { /* (*VERB:ARG), (*construct:...) */
            if (RExC_parse[1] == '{') { /* (*{ ... }) optimistic EVAL */
                fake_eval = '{';
                goto handle_qmark;
            }

            char *start_verb = RExC_parse + 1;
            STRLEN verb_len;
            char *start_arg = NULL;
            unsigned char op = 0;
            int arg_required = 0;
            int internal_argval = -1; /* if > -1 no argument allowed */
            bool has_upper = FALSE;
            U32 seen_flag_set = 0; /* RExC_seen flags we must set */

            if (has_intervening_patws) {
                RExC_parse_inc_by(1);   /* past the '*' */

                /* For strict backwards compatibility, don't change the message
                 * now that we also have lowercase operands */
                if (isUPPER(*RExC_parse)) {
                    vFAIL("In '(*VERB...)', the '(' and '*' must be adjacent");
                }
                else {
                    vFAIL("In '(*...)', the '(' and '*' must be adjacent");
                }
            }
            while (RExC_parse < RExC_end && *RExC_parse != ')' ) {
                if ( *RExC_parse == ':' ) {
                    start_arg = RExC_parse + 1;
                    break;
                }
                else if (! UTF) {
                    if (isUPPER(*RExC_parse)) {
                        has_upper = TRUE;
                    }
                    RExC_parse_inc_by(1);
                }
                else {
                    RExC_parse_inc_utf8();
                }
            }
            verb_len = RExC_parse - start_verb;
            if ( start_arg ) {
                if (RExC_parse >= RExC_end) {
                    goto unterminated_verb_pattern;
                }

                RExC_parse_inc();
                while ( RExC_parse < RExC_end && *RExC_parse != ')' ) {
                    RExC_parse_inc();
                }
                if ( RExC_parse >= RExC_end || *RExC_parse != ')' ) {
                  unterminated_verb_pattern:
                    if (has_upper) {
                        vFAIL("Unterminated verb pattern argument");
                    }
                    else {
                        vFAIL("Unterminated '(*...' argument");
                    }
                }
            } else {
                if ( RExC_parse >= RExC_end || *RExC_parse != ')' ) {
                    if (has_upper) {
                        vFAIL("Unterminated verb pattern");
                    }
                    else {
                        vFAIL("Unterminated '(*...' construct");
                    }
                }
            }

            /* Here, we know that RExC_parse < RExC_end */

            switch ( *start_verb ) {
            case 'A':  /* (*ACCEPT) */
                if ( memEQs(start_verb, verb_len,"ACCEPT") ) {
                    op = ACCEPT;
                    internal_argval = RExC_nestroot;
                }
                break;
            case 'C':  /* (*COMMIT) */
                if ( memEQs(start_verb, verb_len,"COMMIT") )
                    op = COMMIT;
                break;
            case 'F':  /* (*FAIL) */
                if ( verb_len==1 || memEQs(start_verb, verb_len,"FAIL") ) {
                    op = OPFAIL;
                }
                break;
            case ':':  /* (*:NAME) */
            case 'M':  /* (*MARK:NAME) */
                if ( verb_len==0 || memEQs(start_verb, verb_len,"MARK") ) {
                    op = MARKPOINT;
                    arg_required = 1;
                }
                break;
            case 'P':  /* (*PRUNE) */
                if ( memEQs(start_verb, verb_len,"PRUNE") )
                    op = PRUNE;
                break;
            case 'S':   /* (*SKIP) */
                if ( memEQs(start_verb, verb_len,"SKIP") )
                    op = SKIP;
                break;
            case 'T':  /* (*THEN) */
                /* [19:06] <TimToady> :: is then */
                if ( memEQs(start_verb, verb_len,"THEN") ) {
                    op = CUTGROUP;
                    RExC_seen |= REG_CUTGROUP_SEEN;
                }
                break;
            case 'a':
                if (   memEQs(start_verb, verb_len, "asr")
                    || memEQs(start_verb, verb_len, "atomic_script_run"))
                {
                    paren = 'r';        /* Mnemonic: recursed run */
                    goto script_run;
                }
                else if (memEQs(start_verb, verb_len, "atomic")) {
                    paren = 't';    /* AtOMIC */
                    goto alpha_assertions;
                }
                break;
            case 'p':
                if (   memEQs(start_verb, verb_len, "plb")
                    || memEQs(start_verb, verb_len, "positive_lookbehind"))
                {
                    paren = 'b';
                    goto lookbehind_alpha_assertions;
                }
                else if (   memEQs(start_verb, verb_len, "pla")
                         || memEQs(start_verb, verb_len, "positive_lookahead"))
                {
                    paren = 'a';
                    goto alpha_assertions;
                }
                break;
            case 'n':
                if (   memEQs(start_verb, verb_len, "nlb")
                    || memEQs(start_verb, verb_len, "negative_lookbehind"))
                {
                    paren = 'B';
                    goto lookbehind_alpha_assertions;
                }
                else if (   memEQs(start_verb, verb_len, "nla")
                         || memEQs(start_verb, verb_len, "negative_lookahead"))
                {
                    paren = 'A';
                    goto alpha_assertions;
                }
                break;
            case 's':
                if (   memEQs(start_verb, verb_len, "sr")
                    || memEQs(start_verb, verb_len, "script_run"))
                {
                    regnode_offset atomic;

                    paren = 's';

                   script_run:

                    /* This indicates Unicode rules. */
                    REQUIRE_UNI_RULES(flagp, 0);

                    if (! start_arg) {
                        goto no_colon;
                    }

                    RExC_parse_set(start_arg);

                    if (RExC_in_script_run) {

                        /*  Nested script runs are treated as no-ops, because
                         *  if the nested one fails, the outer one must as
                         *  well.  It could fail sooner, and avoid (??{} with
                         *  side effects, but that is explicitly documented as
                         *  undefined behavior. */

                        ret = 0;

                        if (paren == 's') {
                            paren = ':';
                            goto parse_rest;
                        }

                        /* But, the atomic part of a nested atomic script run
                         * isn't a no-op, but can be treated just like a '(?>'
                         * */
                        paren = '>';
                        goto parse_rest;
                    }

                    if (paren == 's') {
                        /* Here, we're starting a new regular script run */
                        ret = reg_node(pRExC_state, SROPEN);
                        RExC_in_script_run = 1;
                        is_open = 1;
                        goto parse_rest;
                    }

                    /* Here, we are starting an atomic script run.  This is
                     * handled by recursing to deal with the atomic portion
                     * separately, enclosed in SROPEN ... SRCLOSE nodes */

                    ret = reg_node(pRExC_state, SROPEN);

                    RExC_in_script_run = 1;

                    atomic = reg(pRExC_state, 'r', &flags, depth);
                    if (flags & (RESTART_PARSE|NEED_UTF8)) {
                        *flagp = flags & (RESTART_PARSE|NEED_UTF8);
                        return 0;
                    }

                    if (! REGTAIL(pRExC_state, ret, atomic)) {
                        REQUIRE_BRANCHJ(flagp, 0);
                    }

                    if (! REGTAIL(pRExC_state, atomic, reg_node(pRExC_state,
                                                                SRCLOSE)))
                    {
                        REQUIRE_BRANCHJ(flagp, 0);
                    }

                    RExC_in_script_run = 0;
                    return ret;
                }

                break;

            lookbehind_alpha_assertions:
                seen_flag_set = REG_LOOKBEHIND_SEEN;
                /*FALLTHROUGH*/

            alpha_assertions:

                if ( !start_arg ) {
                    goto no_colon;
                }

                if ( RExC_parse == start_arg ) {
                    if ( paren == 'A' || paren == 'B' ) {
                        /* An empty negative lookaround assertion is failure.
                         * See also: S_reg_la_OPFAIL() */

                        /* Note: OPFAIL is *not* zerolen. */
                        ret = reg1node(pRExC_state, OPFAIL, 0);
                        nextchar(pRExC_state);
                        return ret;
                    }
                    else
                    if ( paren == 'a' || paren == 'b' ) {
                        /* An empty positive lookaround assertion is success.
                         * See also: S_reg_la_NOTHING() */

                        /* Note: NOTHING is zerolen, so increment here */
                        RExC_seen_zerolen++;
                        ret = reg_node(pRExC_state, NOTHING);
                        nextchar(pRExC_state);
                        return ret;
                    }
                }

                RExC_seen_zerolen++;
                RExC_in_lookaround++;
                RExC_seen |= seen_flag_set;

                RExC_parse_set(start_arg);
                goto parse_rest;

              no_colon:
                vFAIL2utf8f( "'(*%" UTF8f "' requires a terminating ':'",
                    UTF8fARG(UTF, verb_len, start_verb));
                NOT_REACHED; /*NOTREACHED*/

            } /* End of switch */
            if ( ! op ) {
                RExC_parse_inc_safe();
                if (has_upper || verb_len == 0) {
                    vFAIL2utf8f( "Unknown verb pattern '%" UTF8f "'",
                        UTF8fARG(UTF, verb_len, start_verb));
                }
                else {
                    vFAIL2utf8f( "Unknown '(*...)' construct '%" UTF8f "'",
                        UTF8fARG(UTF, verb_len, start_verb));
                }
            }
            if ( RExC_parse == start_arg ) {
                start_arg = NULL;
            }
            if ( arg_required && !start_arg ) {
                vFAIL3( "Verb pattern '%.*s' has a mandatory argument",
                    (int) verb_len, start_verb);
            }
            if (internal_argval == -1) {
                ret = reg1node(pRExC_state, op, 0);
            } else {
                ret = reg2node(pRExC_state, op, 0, internal_argval);
            }
            RExC_seen |= REG_VERBARG_SEEN;
            if (start_arg) {
                SV *sv = newSVpvn( start_arg, RExC_parse - start_arg);
                ARG1u(REGNODE_p(ret)) = reg_add_data( pRExC_state,
                                        STR_WITH_LEN("S"));
                RExC_rxi->data->data[ARG1u(REGNODE_p(ret))]=(void*)sv;
                FLAGS(REGNODE_p(ret)) = 1;
            } else {
                FLAGS(REGNODE_p(ret)) = 0;
            }
            if ( internal_argval != -1 )
                ARG2i_SET(REGNODE_p(ret), internal_argval);
            nextchar(pRExC_state);
            return ret;
        }
        else if (*RExC_parse == '?') { /* (?...) */
          handle_qmark:
            ; /* make sure the label has a statement associated with it*/
            bool is_logical = 0, is_optimistic = 0;
            const char * const seqstart = RExC_parse;
            const char * endptr;
            const char non_existent_group_msg[]
                                            = "Reference to nonexistent group";
            const char impossible_group[] = "Invalid reference to group";

            if (has_intervening_patws) {
                RExC_parse_inc_by(1);
                vFAIL("In '(?...)', the '(' and '?' must be adjacent");
            }

            RExC_parse_inc_by(1);   /* past the '?' */
            if (!fake_eval) {
                paren = *RExC_parse;    /* might be a trailing NUL, if not
                                           well-formed */
                is_optimistic = 0;
            } else {
                is_optimistic = 1;
                paren = fake_eval;
            }
            RExC_parse_inc();
            if (RExC_parse > RExC_end) {
                paren = '\0';
            }
            ret = 0;			/* For look-ahead/behind. */
            switch (paren) {

            case 'P':	/* (?P...) variants for those used to PCRE/Python */
                paren = *RExC_parse;
                if ( paren == '<') {    /* (?P<...>) named capture */
                    RExC_parse_inc_by(1);
                    if (RExC_parse >= RExC_end) {
                        vFAIL("Sequence (?P<... not terminated");
                    }
                    goto named_capture;
                }
                else if (paren == '>') {   /* (?P>name) named recursion */
                    RExC_parse_inc_by(1);
                    if (RExC_parse >= RExC_end) {
                        vFAIL("Sequence (?P>... not terminated");
                    }
                    goto named_recursion;
                }
                else if (paren == '=') {   /* (?P=...)  named backref */
                    RExC_parse_inc_by(1);
                    return handle_named_backref(pRExC_state, flagp,
                                                segment_parse_start, ')');
                }
                RExC_parse_inc_if_char();
                /* diag_listed_as: Sequence (?%s...) not recognized in regex; marked by <-- HERE in m/%s/ */
                vFAIL3("Sequence (%.*s...) not recognized",
                                (int) (RExC_parse - seqstart), seqstart);
                NOT_REACHED; /*NOTREACHED*/
            case '<':           /* (?<...) */
                /* If you want to support (?<*...), first reconcile with GH #17363 */
                if (*RExC_parse == '!') {
                    paren = ','; /* negative lookbehind (?<! ... ) */
                    RExC_parse_inc_by(1);
                    if ((ret= reg_la_OPFAIL(pRExC_state,REG_LB_SEEN,"?<!")))
                        return ret;
                    break;
                }
                else
                if (*RExC_parse == '=') {
                    /* paren = '<' - negative lookahead (?<= ... ) */
                    RExC_parse_inc_by(1);
                    if ((ret= reg_la_NOTHING(pRExC_state,REG_LB_SEEN,"?<=")))
                        return ret;
                    break;
                }
                else
              named_capture:
                {               /* (?<...>) */
                    char *name_start;
                    SV *svname;
                    paren= '>';
                /* FALLTHROUGH */
            case '\'':          /* (?'...') */
                    name_start = RExC_parse;
                    svname = reg_scan_name(pRExC_state, REG_RSN_RETURN_NAME);
                    if (   RExC_parse == name_start
                        || RExC_parse >= RExC_end
                        || *RExC_parse != paren)
                    {
                        vFAIL2("Sequence (?%c... not terminated",
                            paren=='>' ? '<' : (char) paren);
                    }
                    {
                        HE *he_str;
                        SV *sv_dat = NULL;
                        if (!svname) /* shouldn't happen */
                            Perl_croak(aTHX_
                                "panic: reg_scan_name returned NULL");
                        if (!RExC_paren_names) {
                            RExC_paren_names= newHV();
                            sv_2mortal(MUTABLE_SV(RExC_paren_names));
#ifdef DEBUGGING
                            RExC_paren_name_list= newAV();
                            sv_2mortal(MUTABLE_SV(RExC_paren_name_list));
#endif
                        }
                        he_str = hv_fetch_ent( RExC_paren_names, svname, 1, 0 );
                        if ( he_str )
                            sv_dat = HeVAL(he_str);
                        if ( ! sv_dat ) {
                            /* croak baby croak */
                            Perl_croak(aTHX_
                                "panic: paren_name hash element allocation failed");
                        } else if ( SvPOK(sv_dat) ) {
                            /* (?|...) can mean we have dupes so scan to check
                               its already been stored. Maybe a flag indicating
                               we are inside such a construct would be useful,
                               but the arrays are likely to be quite small, so
                               for now we punt -- dmq */
                            IV count = SvIV(sv_dat);
                            I32 *pv = (I32*)SvPVX(sv_dat);
                            IV i;
                            for ( i = 0 ; i < count ; i++ ) {
                                if ( pv[i] == RExC_npar ) {
                                    count = 0;
                                    break;
                                }
                            }
                            if ( count ) {
                                pv = (I32*)SvGROW(sv_dat,
                                                SvCUR(sv_dat) + sizeof(I32)+1);
                                SvCUR_set(sv_dat, SvCUR(sv_dat) + sizeof(I32));
                                pv[count] = RExC_npar;
                                SvIV_set(sv_dat, SvIVX(sv_dat) + 1);
                            }
                        } else {
                            (void)SvUPGRADE(sv_dat, SVt_PVNV);
                            sv_setpvn(sv_dat, (char *)&(RExC_npar),
                                                                sizeof(I32));
                            SvIOK_on(sv_dat);
                            SvIV_set(sv_dat, 1);
                        }
#ifdef DEBUGGING
                        /* No, this does not cause a memory leak under
                         * debugging. RExC_paren_name_list is freed later
                         * on in the dump process. - Yves
                         */
                        if (!av_store(RExC_paren_name_list,
                                      RExC_npar, SvREFCNT_inc_NN(svname)))
                            SvREFCNT_dec_NN(svname);
#endif

                    }
                    nextchar(pRExC_state);
                    paren = 1;
                    goto capturing_parens;
                }
                NOT_REACHED; /*NOTREACHED*/
            case '=':           /* (?=...) */
                if ((ret= reg_la_NOTHING(pRExC_state, 0, "?=")))
                    return ret;
                break;
            case '!':           /* (?!...) */
                if ((ret= reg_la_OPFAIL(pRExC_state, 0, "?!")))
                    return ret;
                break;
            case '|':           /* (?|...) */
                /* branch reset, behave like a (?:...) except that
                   buffers in alternations share the same numbers */
                paren = ':';
                after_freeze = freeze_paren = RExC_logical_npar;

                /* XXX This construct currently requires an extra pass.
                 * Investigation would be required to see if that could be
                 * changed */
                REQUIRE_PARENS_PASS;
                break;
            case ':':           /* (?:...) */
            case '>':           /* (?>...) */
                break;
            case '$':           /* (?$...) */
            case '@':           /* (?@...) */
                vFAIL2("Sequence (?%c...) not implemented", (int)paren);
                break;
            case '0' :           /* (?0) */
            case 'R' :           /* (?R) */
                if (RExC_parse == RExC_end || *RExC_parse != ')')
                    FAIL("Sequence (?R) not terminated");
                num = 0;
                RExC_seen |= REG_RECURSE_SEEN;

                /* XXX These constructs currently require an extra pass.
                 * It probably could be changed */
                REQUIRE_PARENS_PASS;

                *flagp |= POSTPONED;
                goto gen_recurse_regop;
                /*notreached*/
            /* named and numeric backreferences */
            case '&':            /* (?&NAME) */
                segment_parse_start = RExC_parse - 1;
              named_recursion:
                {
                    SV *sv_dat = reg_scan_name(pRExC_state,
                                               REG_RSN_RETURN_DATA);
                   num = sv_dat ? *((I32 *)SvPVX(sv_dat)) : 0;
                }
                if (RExC_parse >= RExC_end || *RExC_parse != ')')
                    vFAIL("Sequence (?&... not terminated");
                goto gen_recurse_regop;
                /* NOTREACHED */
            case '+':
                if (! inRANGE(RExC_parse[0], '1', '9')) {
                    RExC_parse_inc_by(1);
                    vFAIL("Illegal pattern");
                }
                goto parse_recursion;
                /* NOTREACHED*/
            case '-': /* (?-1) */
                if (! inRANGE(RExC_parse[0], '1', '9')) {
                    RExC_parse--; /* rewind to let it be handled later */
                    goto parse_flags;
                }
                /* FALLTHROUGH */
            case '1': case '2': case '3': case '4': /* (?1) */
            case '5': case '6': case '7': case '8': case '9':
                RExC_parse_set((char *) seqstart + 1);  /* Point to the digit */
              parse_recursion:
                {
                    bool is_neg = FALSE;
                    UV unum;
                    segment_parse_start = RExC_parse - 1;
                    if (*RExC_parse == '-') {
                        RExC_parse_inc_by(1);
                        is_neg = TRUE;
                    }
                    endptr = RExC_end;
                    if (grok_atoUV(RExC_parse, &unum, &endptr)
                        && unum <= I32_MAX
                    ) {
                        num = (I32)unum;
                        RExC_parse_set((char*)endptr);
                    }
                    else {  /* Overflow, or something like that.  Position
                               beyond all digits for the message */
                        while (RExC_parse < RExC_end && isDIGIT(*RExC_parse))  {
                            RExC_parse_inc_by(1);
                        }
                        vFAIL(impossible_group);
                    }
                    if (is_neg) {
                        /* -num is always representable on 1 and 2's complement
                         * machines */
                        num = -num;
                    }
                }
                if (*RExC_parse!=')')
                    vFAIL("Expecting close bracket");

                if (paren == '-' || paren == '+') {

                    /* Don't overflow */
                    if (UNLIKELY(I32_MAX - RExC_npar < num)) {
                        RExC_parse_inc_by(1);
                        vFAIL(impossible_group);
                    }

                    /*
                    Diagram of capture buffer numbering.
                    Top line is the normal capture buffer numbers
                    Bottom line is the negative indexing as from
                    the X (the (?-2))

                        1 2    3 4 5 X   Y      6 7
                       /(a(x)y)(a(b(c(?+2)d)e)f)(g(h))/
                       /(a(x)y)(a(b(c(?-2)d)e)f)(g(h))/
                    -   5 4    3 2 1 X   Y      x x

                    Resolve to absolute group.  Recall that RExC_npar is +1 of
                    the actual parenthesis group number.  For lookahead, we
                    have to compensate for that.  Using the above example, when
                    we get to Y in the parse, num is 2 and RExC_npar is 6.  We
                    want 7 for +2, and 4 for -2.
                    */
                    if ( paren == '+' ) {
                        num--;
                    }

                    num += RExC_npar;

                    if (paren == '-' && num < 1) {
                        RExC_parse_inc_by(1);
                        vFAIL(non_existent_group_msg);
                    }
                }
                else
                if (num && num < RExC_logical_npar) {
                    num = RExC_logical_to_parno[num];
                }
                else
                if (ALL_PARENS_COUNTED) {
                    if (num < RExC_logical_total_parens) {
                        num = RExC_logical_to_parno[num];
                    }
                    else {
                        RExC_parse_inc_by(1);
                        vFAIL(non_existent_group_msg);
                    }
                }
                else {
                    REQUIRE_PARENS_PASS;
                }


              gen_recurse_regop:
                if (num >= RExC_npar) {

                    /* It might be a forward reference; we can't fail until we
                     * know, by completing the parse to get all the groups, and
                     * then reparsing */
                    if (ALL_PARENS_COUNTED)  {
                        if (num >= RExC_total_parens) {
                            RExC_parse_inc_by(1);
                            vFAIL(non_existent_group_msg);
                        }
                    }
                    else {
                        REQUIRE_PARENS_PASS;
                    }
                }

                /* We keep track how many GOSUB items we have produced.
                   To start off the ARG2i() of the GOSUB holds its "id",
                   which is used later in conjunction with RExC_recurse
                   to calculate the offset we need to jump for the GOSUB,
                   which it will store in the final representation.
                   We have to defer the actual calculation until much later
                   as the regop may move.
                 */
                ret = reg2node(pRExC_state, GOSUB, num, RExC_recurse_count);
                RExC_recurse_count++;
                DEBUG_OPTIMISE_MORE_r(Perl_re_printf( aTHX_
                    "%*s%*s Recurse #%" UVuf " to %" IVdf "\n",
                            22, "|    |", (int)(depth * 2 + 1), "",
                            (UV)ARG1u(REGNODE_p(ret)),
                            (IV)ARG2i(REGNODE_p(ret))));
                RExC_seen |= REG_RECURSE_SEEN;

                *flagp |= POSTPONED;
                assert(*RExC_parse == ')');
                nextchar(pRExC_state);
                return ret;

            /* NOTREACHED */

            case '?':           /* (??...) */
                is_logical = 1;
                if (*RExC_parse != '{') {
                    RExC_parse_inc_if_char();
                    /* diag_listed_as: Sequence (?%s...) not recognized in regex; marked by <-- HERE in m/%s/ */
                    vFAIL2utf8f(
                        "Sequence (%" UTF8f "...) not recognized",
                        UTF8fARG(UTF, RExC_parse-seqstart, seqstart));
                    NOT_REACHED; /*NOTREACHED*/
                }
                *flagp |= POSTPONED;
                paren = '{';
                RExC_parse_inc_by(1);
                /* FALLTHROUGH */
            case '{':           /* (?{...}) */
            {
                U32 n = 0;
                struct reg_code_block *cb;
                OP * o;

                RExC_seen_zerolen++;

                if (   !pRExC_state->code_blocks
                    || pRExC_state->code_index
                                        >= pRExC_state->code_blocks->count
                    || pRExC_state->code_blocks->cb[pRExC_state->code_index].start
                        != (STRLEN)((RExC_parse -3 - (is_logical ? 1 : 0))
                            - RExC_start)
                ) {
                    if (RExC_pm_flags & PMf_USE_RE_EVAL)
                        FAIL("panic: Sequence (?{...}): no code block found\n");
                    FAIL("Eval-group not allowed at runtime, use re 'eval'");
                }
                /* this is a pre-compiled code block (?{...}) */
                cb = &pRExC_state->code_blocks->cb[pRExC_state->code_index];
                RExC_parse_set(RExC_start + cb->end);
                o = cb->block;
                if (cb->src_regex) {
                    n = reg_add_data(pRExC_state, STR_WITH_LEN("rl"));
                    RExC_rxi->data->data[n] =
                        (void*)SvREFCNT_inc((SV*)cb->src_regex);
                    RExC_rxi->data->data[n+1] = (void*)o;
                }
                else {
                    n = reg_add_data(pRExC_state,
                            (RExC_pm_flags & PMf_HAS_CV) ? "L" : "l", 1);
                    RExC_rxi->data->data[n] = (void*)o;
                }
                pRExC_state->code_index++;
                nextchar(pRExC_state);
                if (!is_optimistic)
                    RExC_seen |= REG_PESSIMIZE_SEEN;

                if (is_logical) {
                    regnode_offset eval;
                    ret = reg_node(pRExC_state, LOGICAL);
                    FLAGS(REGNODE_p(ret)) = 2;

                    eval = reg2node(pRExC_state, EVAL,
                                       n,

                                       /* for later propagation into (??{})
                                        * return value */
                                       RExC_flags & RXf_PMf_COMPILETIME
                                      );
                    FLAGS(REGNODE_p(eval)) = is_optimistic * EVAL_OPTIMISTIC_FLAG;
                    if (! REGTAIL(pRExC_state, ret, eval)) {
                        REQUIRE_BRANCHJ(flagp, 0);
                    }
                    return ret;
                }
                ret = reg2node(pRExC_state, EVAL, n, 0);
                FLAGS(REGNODE_p(ret)) = is_optimistic * EVAL_OPTIMISTIC_FLAG;

                return ret;
            }
            case '(':           /* (?(?{...})...) and (?(?=...)...) */
            {
                int is_define= 0;
                const int DEFINE_len = sizeof("DEFINE") - 1;
                if (    RExC_parse < RExC_end - 1
                    && (   (       RExC_parse[0] == '?'        /* (?(?...)) */
                            && (   RExC_parse[1] == '='
                                || RExC_parse[1] == '!'
                                || RExC_parse[1] == '<'
                                || RExC_parse[1] == '{'))
                        || (       RExC_parse[0] == '*'        /* (?(*...)) */
                            && (   RExC_parse[1] == '{'
                            || (   memBEGINs(RExC_parse + 1,
                                         (Size_t) (RExC_end - (RExC_parse + 1)),
                                         "pla:")
                                || memBEGINs(RExC_parse + 1,
                                         (Size_t) (RExC_end - (RExC_parse + 1)),
                                         "plb:")
                                || memBEGINs(RExC_parse + 1,
                                         (Size_t) (RExC_end - (RExC_parse + 1)),
                                         "nla:")
                                || memBEGINs(RExC_parse + 1,
                                         (Size_t) (RExC_end - (RExC_parse + 1)),
                                         "nlb:")
                                || memBEGINs(RExC_parse + 1,
                                         (Size_t) (RExC_end - (RExC_parse + 1)),
                                         "positive_lookahead:")
                                || memBEGINs(RExC_parse + 1,
                                         (Size_t) (RExC_end - (RExC_parse + 1)),
                                         "positive_lookbehind:")
                                || memBEGINs(RExC_parse + 1,
                                         (Size_t) (RExC_end - (RExC_parse + 1)),
                                         "negative_lookahead:")
                                || memBEGINs(RExC_parse + 1,
                                         (Size_t) (RExC_end - (RExC_parse + 1)),
                                         "negative_lookbehind:")))))
                ) { /* Lookahead or eval. */
                    I32 flag;
                    regnode_offset tail;

                    ret = reg_node(pRExC_state, LOGICAL);
                    FLAGS(REGNODE_p(ret)) = 1;

                    tail = reg(pRExC_state, 1, &flag, depth+1);
                    RETURN_FAIL_ON_RESTART(flag, flagp);
                    if (! REGTAIL(pRExC_state, ret, tail)) {
                        REQUIRE_BRANCHJ(flagp, 0);
                    }
                    goto insert_if;
                }
                else if (   RExC_parse[0] == '<'     /* (?(<NAME>)...) */
                         || RExC_parse[0] == '\'' ) /* (?('NAME')...) */
                {
                    char ch = RExC_parse[0] == '<' ? '>' : '\'';
                    char *name_start= RExC_parse;
                    RExC_parse_inc_by(1);
                    U32 num = 0;
                    SV *sv_dat=reg_scan_name(pRExC_state, REG_RSN_RETURN_DATA);
                    if (   RExC_parse == name_start
                        || RExC_parse >= RExC_end
                        || *RExC_parse != ch)
                    {
                        vFAIL2("Sequence (?(%c... not terminated",
                            (ch == '>' ? '<' : ch));
                    }
                    RExC_parse_inc_by(1);
                    if (sv_dat) {
                        num = reg_add_data( pRExC_state, STR_WITH_LEN("S"));
                        RExC_rxi->data->data[num]=(void*)sv_dat;
                        SvREFCNT_inc_simple_void_NN(sv_dat);
                    }
                    ret = reg1node(pRExC_state, GROUPPN, num);
                    goto insert_if_check_paren;
                }
                else if (memBEGINs(RExC_parse,
                                   (STRLEN) (RExC_end - RExC_parse),
                                   "DEFINE"))
                {
                    ret = reg1node(pRExC_state, DEFINEP, 0);
                    RExC_parse_inc_by(DEFINE_len);
                    is_define = 1;
                    goto insert_if_check_paren;
                }
                else if (RExC_parse[0] == 'R') {
                    RExC_parse_inc_by(1);
                    /* parno == 0 => /(?(R)YES|NO)/  "in any form of recursion OR eval"
                     * parno == 1 => /(?(R0)YES|NO)/ "in GOSUB (?0) / (?R)"
                     * parno == 2 => /(?(R1)YES|NO)/ "in GOSUB (?1) (parno-1)"
                     */
                    parno = 0;
                    if (RExC_parse[0] == '0') {
                        parno = 1;
                        RExC_parse_inc_by(1);
                    }
                    else if (inRANGE(RExC_parse[0], '1', '9')) {
                        UV uv;
                        endptr = RExC_end;
                        if (grok_atoUV(RExC_parse, &uv, &endptr)
                            && uv <= I32_MAX
                        ) {
                            parno = (I32)uv + 1;
                            RExC_parse_set((char*)endptr);
                        }
                        /* else "Switch condition not recognized" below */
                    } else if (RExC_parse[0] == '&') {
                        SV *sv_dat;
                        RExC_parse_inc_by(1);
                        sv_dat = reg_scan_name(pRExC_state,
                                               REG_RSN_RETURN_DATA);
                        if (sv_dat)
                            parno = 1 + *((I32 *)SvPVX(sv_dat));
                    }
                    ret = reg1node(pRExC_state, INSUBP, parno);
                    goto insert_if_check_paren;
                }
                else if (inRANGE(RExC_parse[0], '1', '9')) {
                    /* (?(1)...) */
                    char c;
                    UV uv;
                    endptr = RExC_end;
                    if (grok_atoUV(RExC_parse, &uv, &endptr)
                        && uv <= I32_MAX
                    ) {
                        parno = (I32)uv;
                        RExC_parse_set((char*)endptr);
                    }
                    else {
                        vFAIL("panic: grok_atoUV returned FALSE");
                    }
                    ret = reg1node(pRExC_state, GROUPP, parno);

                 insert_if_check_paren:
                    if (UCHARAT(RExC_parse) != ')') {
                        RExC_parse_inc_safe();
                        vFAIL("Switch condition not recognized");
                    }
                    nextchar(pRExC_state);
                  insert_if:
                    if (! REGTAIL(pRExC_state, ret, reg1node(pRExC_state,
                                                             IFTHEN, 0)))
                    {
                        REQUIRE_BRANCHJ(flagp, 0);
                    }
                    br = regbranch(pRExC_state, &flags, 1, depth+1);
                    if (br == 0) {
                        RETURN_FAIL_ON_RESTART(flags,flagp);
                        FAIL2("panic: regbranch returned failure, flags=%#" UVxf,
                              (UV) flags);
                    } else
                    if (! REGTAIL(pRExC_state, br, reg1node(pRExC_state,
                                                             LONGJMP, 0)))
                    {
                        REQUIRE_BRANCHJ(flagp, 0);
                    }
                    c = UCHARAT(RExC_parse);
                    nextchar(pRExC_state);
                    if (flags&HASWIDTH)
                        *flagp |= HASWIDTH;
                    if (c == '|') {
                        if (is_define)
                            vFAIL("(?(DEFINE)....) does not allow branches");

                        /* Fake one for optimizer.  */
                        lastbr = reg1node(pRExC_state, IFTHEN, 0);

                        if (!regbranch(pRExC_state, &flags, 1, depth+1)) {
                            RETURN_FAIL_ON_RESTART(flags, flagp);
                            FAIL2("panic: regbranch returned failure, flags=%#" UVxf,
                                  (UV) flags);
                        }
                        if (! REGTAIL(pRExC_state, ret, lastbr)) {
                            REQUIRE_BRANCHJ(flagp, 0);
                        }
                        if (flags&HASWIDTH)
                            *flagp |= HASWIDTH;
                        c = UCHARAT(RExC_parse);
                        nextchar(pRExC_state);
                    }
                    else
                        lastbr = 0;
                    if (c != ')') {
                        if (RExC_parse >= RExC_end)
                            vFAIL("Switch (?(condition)... not terminated");
                        else
                            vFAIL("Switch (?(condition)... contains too many branches");
                    }
                    ender = reg_node(pRExC_state, TAIL);
                    if (! REGTAIL(pRExC_state, br, ender)) {
                        REQUIRE_BRANCHJ(flagp, 0);
                    }
                    if (lastbr) {
                        if (! REGTAIL(pRExC_state, lastbr, ender)) {
                            REQUIRE_BRANCHJ(flagp, 0);
                        }
                        if (! REGTAIL(pRExC_state,
                                      REGNODE_OFFSET(
                                        REGNODE_AFTER(REGNODE_p(lastbr))),
                                      ender))
                        {
                            REQUIRE_BRANCHJ(flagp, 0);
                        }
                    }
                    else
                        if (! REGTAIL(pRExC_state, ret, ender)) {
                            REQUIRE_BRANCHJ(flagp, 0);
                        }
#if 0  /* Removing this doesn't cause failures in the test suite -- khw */
                    RExC_size++; /* XXX WHY do we need this?!!
                                    For large programs it seems to be required
                                    but I can't figure out why. -- dmq*/
#endif
                    return ret;
                }
                RExC_parse_inc_safe();
                vFAIL("Unknown switch condition (?(...))");
            }
            case '[':           /* (?[ ... ]) */
                return handle_regex_sets(pRExC_state, NULL, flagp, depth+1);
            case 0: /* A NUL */
                RExC_parse--; /* for vFAIL to print correctly */
                vFAIL("Sequence (? incomplete");
                break;

            case ')':
                if (RExC_strict) {  /* [perl #132851] */
                    ckWARNreg(RExC_parse, "Empty (?) without any modifiers");
                }
                /* FALLTHROUGH */
            case '*': /* If you want to support (?*...), first reconcile with GH #17363 */
            /* FALLTHROUGH */
            default: /* e.g., (?i) */
                RExC_parse_set((char *) seqstart + 1);
              parse_flags:
                parse_lparen_question_flags(pRExC_state);
                if (UCHARAT(RExC_parse) != ':') {
                    if (RExC_parse < RExC_end)
                        nextchar(pRExC_state);
                    *flagp = TRYAGAIN;
                    return 0;
                }
                paren = ':';
                nextchar(pRExC_state);
                ret = 0;
                goto parse_rest;
            } /* end switch */
        }
        else if (!(RExC_flags & RXf_PMf_NOCAPTURE)) {   /* (...) */
          capturing_parens:
            parno = RExC_npar;
            RExC_npar++;
            if (RExC_npar >= U16_MAX)
                FAIL2("Too many capture groups (limit is %" UVuf ")", (UV)RExC_npar);

            logical_parno = RExC_logical_npar;
            RExC_logical_npar++;
            if (! ALL_PARENS_COUNTED) {
                /* If we are in our first pass through (and maybe only pass),
                 * we  need to allocate memory for the capturing parentheses
                 * data structures.
                 */

                if (!RExC_parens_buf_size) {
                    /* first guess at number of parens we might encounter */
                    RExC_parens_buf_size = 10;

                    /* setup RExC_open_parens, which holds the address of each
                     * OPEN tag, and to make things simpler for the 0 index the
                     * start of the program - this is used later for offsets */
                    Newxz(RExC_open_parens, RExC_parens_buf_size,
                            regnode_offset);
                    RExC_open_parens[0] = 1;    /* +1 for REG_MAGIC */

                    /* setup RExC_close_parens, which holds the address of each
                     * CLOSE tag, and to make things simpler for the 0 index
                     * the end of the program - this is used later for offsets
                     * */
                    Newxz(RExC_close_parens, RExC_parens_buf_size,
                            regnode_offset);
                    /* we don't know where end op starts yet, so we don't need to
                     * set RExC_close_parens[0] like we do RExC_open_parens[0]
                     * above */

                    Newxz(RExC_logical_to_parno, RExC_parens_buf_size, I32);
                    Newxz(RExC_parno_to_logical, RExC_parens_buf_size, I32);
                }
                else if (RExC_npar > RExC_parens_buf_size) {
                    I32 old_size = RExC_parens_buf_size;

                    RExC_parens_buf_size *= 2;

                    Renew(RExC_open_parens, RExC_parens_buf_size,
                            regnode_offset);
                    Zero(RExC_open_parens + old_size,
                            RExC_parens_buf_size - old_size, regnode_offset);

                    Renew(RExC_close_parens, RExC_parens_buf_size,
                            regnode_offset);
                    Zero(RExC_close_parens + old_size,
                            RExC_parens_buf_size - old_size, regnode_offset);

                    Renew(RExC_logical_to_parno, RExC_parens_buf_size, I32);
                    Zero(RExC_logical_to_parno + old_size,
                         RExC_parens_buf_size - old_size, I32);

                    Renew(RExC_parno_to_logical, RExC_parens_buf_size, I32);
                    Zero(RExC_parno_to_logical + old_size,
                         RExC_parens_buf_size - old_size, I32);
                }
            }

            ret = reg1node(pRExC_state, OPEN, parno);
            if (!RExC_nestroot)
                RExC_nestroot = parno;
            if (RExC_open_parens && !RExC_open_parens[parno])
            {
                DEBUG_OPTIMISE_MORE_r(Perl_re_printf( aTHX_
                    "%*s%*s Setting open paren #%" IVdf " to %zu\n",
                    22, "|    |", (int)(depth * 2 + 1), "",
                    (IV)parno, ret));
                RExC_open_parens[parno]= ret;
            }
            if (RExC_parno_to_logical) {
                RExC_parno_to_logical[parno] = logical_parno;
                if (RExC_logical_to_parno && !RExC_logical_to_parno[logical_parno])
                    RExC_logical_to_parno[logical_parno] = parno;
            }
            is_open = 1;
        } else {
            /* with RXf_PMf_NOCAPTURE treat (...) as (?:...) */
            paren = ':';
            ret = 0;
        }
    }
    else                        /* ! paren */
        ret = 0;

   parse_rest:
    /* Pick up the branches, linking them together. */
    segment_parse_start = RExC_parse;
    I32 npar_before_regbranch = RExC_npar - 1;
    br = regbranch(pRExC_state, &flags, 1, depth+1);

    /*     branch_len = (paren != 0); */

    if (br == 0) {
        RETURN_FAIL_ON_RESTART(flags, flagp);
        FAIL2("panic: regbranch returned failure, flags=%#" UVxf, (UV) flags);
    }
    if (*RExC_parse == '|') {
        if (RExC_use_BRANCHJ) {
            reginsert(pRExC_state, BRANCHJ, br, depth+1);
            ARG2a_SET(REGNODE_p(br), npar_before_regbranch);
            ARG2b_SET(REGNODE_p(br), (U16)RExC_npar - 1);
        }
        else {
            reginsert(pRExC_state, BRANCH, br, depth+1);
            ARG1a_SET(REGNODE_p(br), (U16)npar_before_regbranch);
            ARG1b_SET(REGNODE_p(br), (U16)RExC_npar - 1);
        }
        have_branch = 1;
    }
    else if (paren == ':') {
        *flagp |= flags&SIMPLE;
    }
    if (is_open) {				/* Starts with OPEN. */
        if (! REGTAIL(pRExC_state, ret, br)) {  /* OPEN -> first. */
            REQUIRE_BRANCHJ(flagp, 0);
        }
    }
    else if (paren != '?')		/* Not Conditional */
        ret = br;
    *flagp |= flags & (HASWIDTH | POSTPONED);
    lastbr = br;
    while (*RExC_parse == '|') {
        if (RExC_use_BRANCHJ) {
            bool shut_gcc_up;

            ender = reg1node(pRExC_state, LONGJMP, 0);

            /* Append to the previous. */
            shut_gcc_up = REGTAIL(pRExC_state,
                         REGNODE_OFFSET(REGNODE_AFTER(REGNODE_p(lastbr))),
                         ender);
            PERL_UNUSED_VAR(shut_gcc_up);
        }
        nextchar(pRExC_state);
        if (freeze_paren) {
            if (RExC_logical_npar > after_freeze)
                after_freeze = RExC_logical_npar;
            RExC_logical_npar = freeze_paren;
        }
        br = regbranch(pRExC_state, &flags, 0, depth+1);

        if (br == 0) {
            RETURN_FAIL_ON_RESTART(flags, flagp);
            FAIL2("panic: regbranch returned failure, flags=%#" UVxf, (UV) flags);
        }
        if (!  REGTAIL(pRExC_state, lastbr, br)) {  /* BRANCH -> BRANCH. */
            REQUIRE_BRANCHJ(flagp, 0);
        }
        assert(OP(REGNODE_p(br)) == BRANCH || OP(REGNODE_p(br))==BRANCHJ);
        assert(OP(REGNODE_p(lastbr)) == BRANCH || OP(REGNODE_p(lastbr))==BRANCHJ);
        if (OP(REGNODE_p(br)) == BRANCH) {
            if (OP(REGNODE_p(lastbr)) == BRANCH)
                ARG1b_SET(REGNODE_p(lastbr),ARG1a(REGNODE_p(br)));
            else
                ARG2b_SET(REGNODE_p(lastbr),ARG1a(REGNODE_p(br)));
        }
        else
        if (OP(REGNODE_p(br)) == BRANCHJ) {
            if (OP(REGNODE_p(lastbr)) == BRANCH)
                ARG1b_SET(REGNODE_p(lastbr),ARG2a(REGNODE_p(br)));
            else
                ARG2b_SET(REGNODE_p(lastbr),ARG2a(REGNODE_p(br)));
        }

        lastbr = br;
        *flagp |= flags & (HASWIDTH | POSTPONED);
    }

    if (have_branch || paren != ':') {
        regnode * br;

        /* Make a closing node, and hook it on the end. */
        switch (paren) {
        case ':':
            ender = reg_node(pRExC_state, TAIL);
            break;
        case 1: case 2:
            ender = reg1node(pRExC_state, CLOSE, parno);
            if ( RExC_close_parens ) {
                DEBUG_OPTIMISE_MORE_r(Perl_re_printf( aTHX_
                        "%*s%*s Setting close paren #%" IVdf " to %zu\n",
                        22, "|    |", (int)(depth * 2 + 1), "",
                        (IV)parno, ender));
                RExC_close_parens[parno]= ender;
                if (RExC_nestroot == parno)
                    RExC_nestroot = 0;
            }
            break;
        case 's':
            ender = reg_node(pRExC_state, SRCLOSE);
            RExC_in_script_run = 0;
            break;
        /* LOOKBEHIND ops (not sure why these are duplicated - Yves) */
        case 'b': /* (*positive_lookbehind: ... ) (*plb: ... ) */
        case 'B': /* (*negative_lookbehind: ... ) (*nlb: ... ) */
        case '<': /* (?<= ... ) */
        case ',': /* (?<! ... ) */
            *flagp &= ~HASWIDTH;
            ender = reg_node(pRExC_state, LOOKBEHIND_END);
            break;
        /* LOOKAHEAD ops (not sure why these are duplicated - Yves) */
        case 'a':
        case 'A':
        case '=':
        case '!':
            *flagp &= ~HASWIDTH;
            /* FALLTHROUGH */
        case 't':   /* aTomic */
        case '>':
            ender = reg_node(pRExC_state, SUCCEED);
            break;
        case 0:
            ender = reg_node(pRExC_state, END);
            assert(!RExC_end_op); /* there can only be one! */
            RExC_end_op = REGNODE_p(ender);
            if (RExC_close_parens) {
                DEBUG_OPTIMISE_MORE_r(Perl_re_printf( aTHX_
                    "%*s%*s Setting close paren #0 (END) to %zu\n",
                    22, "|    |", (int)(depth * 2 + 1), "",
                    ender));

                RExC_close_parens[0]= ender;
            }
            break;
        }
        DEBUG_PARSE_r({
            DEBUG_PARSE_MSG("lsbr");
            regprop(RExC_rx, RExC_mysv1, REGNODE_p(lastbr), NULL, pRExC_state);
            regprop(RExC_rx, RExC_mysv2, REGNODE_p(ender), NULL, pRExC_state);
            Perl_re_printf( aTHX_  "~ tying lastbr %s (%" IVdf ") to ender %s (%" IVdf ") offset %" IVdf "\n",
                          SvPV_nolen_const(RExC_mysv1),
                          (IV)lastbr,
                          SvPV_nolen_const(RExC_mysv2),
                          (IV)ender,
                          (IV)(ender - lastbr)
            );
        });
        if (OP(REGNODE_p(lastbr)) == BRANCH) {
            ARG1b_SET(REGNODE_p(lastbr),(U16)RExC_npar-1);
        }
        else
        if (OP(REGNODE_p(lastbr)) == BRANCHJ) {
            ARG2b_SET(REGNODE_p(lastbr),(U16)RExC_npar-1);
        }

        if (! REGTAIL(pRExC_state, lastbr, ender)) {
            REQUIRE_BRANCHJ(flagp, 0);
        }

        if (have_branch) {
            char is_nothing= 1;
            if (depth==1)
                RExC_seen |= REG_TOP_LEVEL_BRANCHES_SEEN;

            /* Hook the tails of the branches to the closing node. */
            for (br = REGNODE_p(ret); br; br = regnext(br)) {
                const U8 op = REGNODE_TYPE(OP(br));
                regnode *nextoper = REGNODE_AFTER(br);
                if (op == BRANCH) {
                    if (! REGTAIL_STUDY(pRExC_state,
                                        REGNODE_OFFSET(nextoper),
                                        ender))
                    {
                        REQUIRE_BRANCHJ(flagp, 0);
                    }
                    if ( OP(nextoper) != NOTHING
                         || regnext(nextoper) != REGNODE_p(ender))
                        is_nothing= 0;
                }
                else if (op == BRANCHJ) {
                    bool shut_gcc_up = REGTAIL_STUDY(pRExC_state,
                                        REGNODE_OFFSET(nextoper),
                                        ender);
                    PERL_UNUSED_VAR(shut_gcc_up);
                    /* for now we always disable this optimisation * /
                    regnode *nopr= REGNODE_AFTER_type(br,tregnode_BRANCHJ);
                    if ( OP(nopr) != NOTHING
                         || regnext(nopr) != REGNODE_p(ender))
                    */
                        is_nothing= 0;
                }
            }
            if (is_nothing) {
                regnode * ret_as_regnode = REGNODE_p(ret);
                br= REGNODE_TYPE(OP(ret_as_regnode)) != BRANCH
                               ? regnext(ret_as_regnode)
                               : ret_as_regnode;
                DEBUG_PARSE_r({
                    DEBUG_PARSE_MSG("NADA");
                    regprop(RExC_rx, RExC_mysv1, ret_as_regnode,
                                     NULL, pRExC_state);
                    regprop(RExC_rx, RExC_mysv2, REGNODE_p(ender),
                                     NULL, pRExC_state);
                    Perl_re_printf( aTHX_  "~ converting ret %s (%" IVdf ") to ender %s (%" IVdf ") offset %" IVdf "\n",
                                  SvPV_nolen_const(RExC_mysv1),
                                  (IV)REG_NODE_NUM(ret_as_regnode),
                                  SvPV_nolen_const(RExC_mysv2),
                                  (IV)ender,
                                  (IV)(ender - ret)
                    );
                });
                OP(br)= NOTHING;
                if (OP(REGNODE_p(ender)) == TAIL) {
                    NEXT_OFF(br)= 0;
                    RExC_emit= REGNODE_OFFSET(br) + NODE_STEP_REGNODE;
                } else {
                    regnode *opt;
                    for ( opt= br + 1; opt < REGNODE_p(ender) ; opt++ )
                        OP(opt)= OPTIMIZED;
                    NEXT_OFF(br)= REGNODE_p(ender) - br;
                }
            }
        }
    }

    {
        const char *p;
         /* Even/odd or x=don't care: 010101x10x */
        static const char parens[] = "=!aA<,>Bbt";
         /* flag below is set to 0 up through 'A'; 1 for larger */

        if (paren && (p = strchr(parens, paren))) {
            U8 node = ((p - parens) % 2) ? UNLESSM : IFMATCH;
            int flag = (p - parens) > 3;

            if (paren == '>' || paren == 't') {
                node = SUSPEND, flag = 0;
            }

            reginsert(pRExC_state, node, ret, depth+1);
            FLAGS(REGNODE_p(ret)) = flag;
            if (! REGTAIL_STUDY(pRExC_state, ret, reg_node(pRExC_state, TAIL)))
            {
                REQUIRE_BRANCHJ(flagp, 0);
            }
        }
    }

    /* Check for proper termination. */
    if (paren) {
        /* restore original flags, but keep (?p) and, if we've encountered
         * something in the parse that changes /d rules into /u, keep the /u */
        RExC_flags = oregflags | (RExC_flags & RXf_PMf_KEEPCOPY);
        if (DEPENDS_SEMANTICS && toUSE_UNI_CHARSET_NOT_DEPENDS) {
            set_regex_charset(&RExC_flags, REGEX_UNICODE_CHARSET);
        }
        if (RExC_parse >= RExC_end || UCHARAT(RExC_parse) != ')') {
            RExC_parse_set(reg_parse_start);
            vFAIL("Unmatched (");
        }
        nextchar(pRExC_state);
    }
    else if (!paren && RExC_parse < RExC_end) {
        if (*RExC_parse == ')') {
            RExC_parse_inc_by(1);
            vFAIL("Unmatched )");
        }
        else
            FAIL("Junk on end of regexp");	/* "Can't happen". */
        NOT_REACHED; /* NOTREACHED */
    }

    if (after_freeze > RExC_logical_npar)
        RExC_logical_npar = after_freeze;

    RExC_in_lookaround = was_in_lookaround;

    return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 *
 * On success, returns the offset at which any next node should be placed into
 * the regex engine program being compiled.
 *
 * Returns 0 otherwise, setting flagp to RESTART_PARSE if the parse needs
 * to be restarted, or'd with NEED_UTF8 if the pattern needs to be upgraded to
 * UTF-8
 */
STATIC regnode_offset
S_regbranch(pTHX_ RExC_state_t *pRExC_state, I32 *flagp, I32 first, U32 depth)
{
    regnode_offset ret;
    regnode_offset chain = 0;
    regnode_offset latest;
    regnode *branch_node = NULL;
    I32 flags = 0, c = 0;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGBRANCH;

    DEBUG_PARSE("brnc");

    if (first)
        ret = 0;
    else {
        if (RExC_use_BRANCHJ) {
            ret = reg2node(pRExC_state, BRANCHJ, 0, 0);
            branch_node = REGNODE_p(ret);
            ARG2a_SET(branch_node, (U16)RExC_npar-1);
        } else {
            ret = reg1node(pRExC_state, BRANCH, 0);
            branch_node = REGNODE_p(ret);
            ARG1a_SET(branch_node, (U16)RExC_npar-1);
        }
    }

    *flagp = 0;			/* Initialize. */

    skip_to_be_ignored_text(pRExC_state, &RExC_parse,
                            FALSE /* Don't force to /x */ );
    while (RExC_parse < RExC_end && *RExC_parse != '|' && *RExC_parse != ')') {
        flags &= ~TRYAGAIN;
        latest = regpiece(pRExC_state, &flags, depth+1);
        if (latest == 0) {
            if (flags & TRYAGAIN)
                continue;
            RETURN_FAIL_ON_RESTART(flags, flagp);
            FAIL2("panic: regpiece returned failure, flags=%#" UVxf, (UV) flags);
        }
        else if (ret == 0)
            ret = latest;
        *flagp |= flags&(HASWIDTH|POSTPONED);
        if (chain != 0) {
            /* FIXME adding one for every branch after the first is probably
             * excessive now we have TRIE support. (hv) */
            MARK_NAUGHTY(1);
            if (! REGTAIL(pRExC_state, chain, latest)) {
                /* XXX We could just redo this branch, but figuring out what
                 * bookkeeping needs to be reset is a pain, and it's likely
                 * that other branches that goto END will also be too large */
                REQUIRE_BRANCHJ(flagp, 0);
            }
        }
        chain = latest;
        c++;
    }
    if (chain == 0) {	/* Loop ran zero times. */
        chain = reg_node(pRExC_state, NOTHING);
        if (ret == 0)
            ret = chain;
    }
    if (c == 1) {
        *flagp |= flags & SIMPLE;
    }
    return ret;
}

#define RBRACE  0
#define MIN_S   1
#define MIN_E   2
#define MAX_S   3
#define MAX_E   4

#ifndef PERL_IN_XSUB_RE
bool
Perl_regcurly(const char *s, const char *e, const char * result[5])
{
    /* This function matches a {m,n} quantifier.  When called with a NULL final
     * argument, it simply parses the input from 's' up through 'e-1', and
     * returns a boolean as to whether or not this input is syntactically a
     * {m,n} quantifier.
     *
     * When called with a non-NULL final parameter, and when the function
     * returns TRUE, it additionally stores information into the array
     * specified by that parameter about what it found in the parse.  The
     * parameter must be a pointer into a 5 element array of 'const char *'
     * elements.  The returned information is as follows:
     *   result[RBRACE]  points to the closing brace
     *   result[MIN_S]   points to the first byte of the lower bound
     *   result[MIN_E]   points to one beyond the final byte of the lower bound
     *   result[MAX_S]   points to the first byte of the upper bound
     *   result[MAX_E]   points to one beyond the final byte of the upper bound
     *
     * If the quantifier is of the form {m,} (meaning an infinite upper
     * bound), result[MAX_E] is set to result[MAX_S]; what they actually point
     * to is irrelevant, just that it's the same place
     *
     * If instead the quantifier is of the form {m} there is actually only
     * one bound, and both the upper and lower result[] elements are set to
     * point to it.
     *
     * This function checks only for syntactic validity; it leaves checking for
     * semantic validity and raising any diagnostics to the caller.  This
     * function is called in multiple places to check for syntax, but only from
     * one for semantics.  It makes it as simple as possible for the
     * syntax-only callers, while furnishing just enough information for the
     * semantic caller.
     */

    const char * min_start = NULL;
    const char * max_start = NULL;
    const char * min_end = NULL;
    const char * max_end = NULL;

    bool has_comma = FALSE;

    PERL_ARGS_ASSERT_REGCURLY;

    if (s >= e || *s++ != '{')
        return FALSE;

    while (s < e && isBLANK(*s)) {
        s++;
    }

    if isDIGIT(*s) {
        min_start = s;
        do {
            s++;
        } while (s < e && isDIGIT(*s));
        min_end = s;
    }

    while (s < e && isBLANK(*s)) {
        s++;
    }

    if (*s == ',') {
        has_comma = TRUE;
        s++;

        while (s < e && isBLANK(*s)) {
            s++;
        }

        if isDIGIT(*s) {
            max_start = s;
            do {
                s++;
            } while (s < e && isDIGIT(*s));
            max_end = s;
        }
    }

    while (s < e && isBLANK(*s)) {
        s++;
    }
                               /* Need at least one number */
    if (s >= e || *s != '}' || (! min_start && ! max_end)) {
        return FALSE;
    }

    if (result) {

        result[RBRACE] = s;

        result[MIN_S] = min_start;
        result[MIN_E] = min_end;
        if (has_comma) {
            if (max_start) {
                result[MAX_S] = max_start;
                result[MAX_E] = max_end;
            }
            else {
                /* Having no value after the comma is signalled by setting
                 * start and end to the same value.  What that value is isn't
                 * relevant; NULL is chosen simply because it will fail if the
                 * caller mistakenly uses it */
                result[MAX_S] = result[MAX_E] = NULL;
            }
        }
        else {  /* No comma means lower and upper bounds are the same */
            result[MAX_S] = min_start;
            result[MAX_E] = min_end;
        }
    }

    return TRUE;
}
#endif

U32
S_get_quantifier_value(pTHX_ RExC_state_t *pRExC_state,
                       const char * start, const char * end)
{
    /* This is a helper function for regpiece() to compute, given the
     * quantifier {m,n}, the value of either m or n, based on the starting
     * position 'start' in the string, through the byte 'end-1', returning it
     * if valid, and failing appropriately if not.  It knows the restrictions
     * imposed on quantifier values */

    UV uv;
    STATIC_ASSERT_DECL(REG_INFTY <= U32_MAX);

    PERL_ARGS_ASSERT_GET_QUANTIFIER_VALUE;

    if (grok_atoUV(start, &uv, &end)) {
        if (uv < REG_INFTY) {   /* A valid, small-enough number */
            return (U32) uv;
        }
    }
    else if (*start == '0') { /* grok_atoUV() fails for only two reasons:
                                 leading zeros or overflow */
        RExC_parse_set((char * ) end);

        /* Perhaps too generic a msg for what is only failure from having
         * leading zeros, but this is how it's always behaved. */
        vFAIL("Invalid quantifier in {,}");
        NOT_REACHED; /*NOTREACHED*/
    }

    /* Here, found a quantifier, but was too large; either it overflowed or was
     * too big a legal number */
    RExC_parse_set((char * ) end);
    vFAIL2("Quantifier in {,} bigger than %d", REG_INFTY - 1);

    NOT_REACHED; /*NOTREACHED*/
    return U32_MAX; /* Perhaps some compilers will be expecting a return */
}

/*
 - regpiece - something followed by possible quantifier * + ? {n,m}
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 *
 * On success, returns the offset at which any next node should be placed into
 * the regex engine program being compiled.
 *
 * Returns 0 otherwise, with *flagp set to indicate why:
 *  TRYAGAIN        if regatom() returns 0 with TRYAGAIN.
 *  RESTART_PARSE   if the parse needs to be restarted, or'd with
 *                  NEED_UTF8 if the pattern needs to be upgraded to UTF-8.
 */
STATIC regnode_offset
S_regpiece(pTHX_ RExC_state_t *pRExC_state, I32 *flagp, U32 depth)
{
    regnode_offset ret;
    char op;
    I32 flags;
    const char * const origparse = RExC_parse;
    I32 min;
    I32 max = REG_INFTY;
    I32 npar_before = RExC_npar-1;

    /* Save the original in case we change the emitted regop to a FAIL. */
    const regnode_offset orig_emit = RExC_emit;

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGPIECE;

    DEBUG_PARSE("piec");

    ret = regatom(pRExC_state, &flags, depth+1);
    if (ret == 0) {
        RETURN_FAIL_ON_RESTART_OR_FLAGS(flags, flagp, TRYAGAIN);
        FAIL2("panic: regatom returned failure, flags=%#" UVxf, (UV) flags);
    }
    I32 npar_after = RExC_npar-1;

    op = *RExC_parse;
    switch (op) {
        const char * regcurly_return[5];

      case '*':
        nextchar(pRExC_state);
        min = 0;
        break;

      case '+':
        nextchar(pRExC_state);
        min = 1;
        break;

      case '?':
        nextchar(pRExC_state);
        min = 0; max = 1;
        break;

      case '{':  /* A '{' may or may not indicate a quantifier; call regcurly()
                    to determine which */
        if (regcurly(RExC_parse, RExC_end, regcurly_return)) {
            const char * min_start = regcurly_return[MIN_S];
            const char * min_end   = regcurly_return[MIN_E];
            const char * max_start = regcurly_return[MAX_S];
            const char * max_end   = regcurly_return[MAX_E];

            if (min_start) {
                min = get_quantifier_value(pRExC_state, min_start, min_end);
            }
            else {
                min = 0;
            }

            if (max_start == max_end) {     /* Was of the form {m,} */
                max = REG_INFTY;
            }
            else if (max_start == min_start) {  /* Was of the form {m} */
                max = min;
            }
            else {  /* Was of the form {m,n} */
                assert(max_end >= max_start);

                max = get_quantifier_value(pRExC_state, max_start, max_end);
            }

            RExC_parse_set((char *) regcurly_return[RBRACE]);
            nextchar(pRExC_state);

            if (max < min) {    /* If can't match, warn and optimize to fail
                                   unconditionally */
                reginsert(pRExC_state, OPFAIL, orig_emit, depth+1);
                ckWARNreg(RExC_parse, "Quantifier {n,m} with n > m can't match");
                NEXT_OFF(REGNODE_p(orig_emit)) =
                                    REGNODE_ARG_LEN(OPFAIL) + NODE_STEP_REGNODE;
                return ret;
            }
            else if (min == max && *RExC_parse == '?') {
                ckWARN2reg(RExC_parse + 1,
                           "Useless use of greediness modifier '%c'",
                           *RExC_parse);
            }

            break;
        } /* End of is {m,n} */

        /* Here was a '{', but what followed it didn't form a quantifier. */
        /* FALLTHROUGH */

      default:
        *flagp = flags;
        return(ret);
        NOT_REACHED; /*NOTREACHED*/
    }

    /* Here we have a quantifier, and have calculated 'min' and 'max'.
     *
     * Check and possibly adjust a zero width operand */
    if (! (flags & (HASWIDTH|POSTPONED))) {
        if (max > REG_INFTY/3) {
            ckWARN2reg(RExC_parse,
                       "%" UTF8f " matches null string many times",
                       UTF8fARG(UTF, (RExC_parse >= origparse
                                     ? RExC_parse - origparse
                                     : 0),
                       origparse));
        }

        /* There's no point in trying to match something 0 length more than
         * once except for extra side effects, which we don't have here since
         * not POSTPONED */
        if (max > 1) {
            max = 1;
            if (min > max) {
                min = max;
            }
        }
    }

    /* If this is a code block pass it up */
    *flagp |= (flags & POSTPONED);

    if (max > 0) {
        *flagp |= (flags & HASWIDTH);
        if (max == REG_INFTY)
            RExC_seen |= REG_UNBOUNDED_QUANTIFIER_SEEN;
    }

    /* 'SIMPLE' operands don't require full generality */
    if ((flags&SIMPLE)) {
        if (max == REG_INFTY) {
            if (min == 0) {
                if (UNLIKELY(RExC_pm_flags & PMf_WILDCARD)) {
                    goto min0_maxINF_wildcard_forbidden;
                }

                reginsert(pRExC_state, STAR, ret, depth+1);
                MARK_NAUGHTY(4);
                goto done_main_op;
            }
            else if (min == 1) {
                reginsert(pRExC_state, PLUS, ret, depth+1);
                MARK_NAUGHTY(3);
                goto done_main_op;
            }
        }

        /* Here, SIMPLE, but not the '*' and '+' special cases */

        MARK_NAUGHTY_EXP(2, 2);
        reginsert(pRExC_state, CURLY, ret, depth+1);
    }
    else {  /* not SIMPLE */
        const regnode_offset w = reg_node(pRExC_state, WHILEM);

        FLAGS(REGNODE_p(w)) = 0;
        if (!  REGTAIL(pRExC_state, ret, w)) {
            REQUIRE_BRANCHJ(flagp, 0);
        }
        if (RExC_use_BRANCHJ) {
            reginsert(pRExC_state, LONGJMP, ret, depth+1);
            reginsert(pRExC_state, NOTHING, ret, depth+1);
            REGNODE_STEP_OVER(ret,tregnode_NOTHING,tregnode_LONGJMP);
        }
        reginsert(pRExC_state, CURLYX, ret, depth+1);
        if (RExC_use_BRANCHJ)
            /* Go over NOTHING to LONGJMP. */
            REGNODE_STEP_OVER(ret,tregnode_CURLYX,tregnode_NOTHING);

        if (! REGTAIL(pRExC_state, ret, reg_node(pRExC_state,
                                                  NOTHING)))
        {
            REQUIRE_BRANCHJ(flagp, 0);
        }
        RExC_whilem_seen++;
        MARK_NAUGHTY_EXP(1, 4);     /* compound interest */
    }

    /* Finish up the CURLY/CURLYX case */
    FLAGS(REGNODE_p(ret)) = 0;

    ARG1i_SET(REGNODE_p(ret), min);
    ARG2i_SET(REGNODE_p(ret), max);

    /* if we had a npar_after then we need to increment npar_before,
     * we want to track the range of parens we need to reset each iteration
     */
    if (npar_after!=npar_before) {
        ARG3a_SET(REGNODE_p(ret), (U16)npar_before+1);
        ARG3b_SET(REGNODE_p(ret), (U16)npar_after);
    } else {
        ARG3a_SET(REGNODE_p(ret), 0);
        ARG3b_SET(REGNODE_p(ret), 0);
    }

  done_main_op:

    /* Process any greediness modifiers */
    if (*RExC_parse == '?') {
        nextchar(pRExC_state);
        reginsert(pRExC_state, MINMOD, ret, depth+1);
        if (! REGTAIL(pRExC_state, ret, ret + NODE_STEP_REGNODE)) {
            REQUIRE_BRANCHJ(flagp, 0);
        }
    }
    else if (*RExC_parse == '+') {
        regnode_offset ender;
        nextchar(pRExC_state);
        ender = reg_node(pRExC_state, SUCCEED);
        if (! REGTAIL(pRExC_state, ret, ender)) {
            REQUIRE_BRANCHJ(flagp, 0);
        }
        reginsert(pRExC_state, SUSPEND, ret, depth+1);
        ender = reg_node(pRExC_state, TAIL);
        if (! REGTAIL(pRExC_state, ret, ender)) {
            REQUIRE_BRANCHJ(flagp, 0);
        }
    }

    /* Forbid extra quantifiers */
    if (isQUANTIFIER(RExC_parse, RExC_end)) {
        RExC_parse_inc_by(1);
        vFAIL("Nested quantifiers");
    }

    return(ret);

  min0_maxINF_wildcard_forbidden:

    /* Here we are in a wildcard match, and the minimum match length is 0, and
     * the max could be infinity.  This is currently forbidden.  The only
     * reason is to make it harder to write patterns that take a long long time
     * to halt, and because the use of this construct isn't necessary in
     * matching Unicode property values */
    RExC_parse_inc_by(1);
    /* diag_listed_as: Use of %s is not allowed in Unicode property wildcard
       subpatterns in regex; marked by <-- HERE in m/%s/
     */
    vFAIL("Use of quantifier '*' is not allowed in Unicode property wildcard"
          " subpatterns");

    /* Note, don't need to worry about the input being '{0,}', as a '}' isn't
     * legal at all in wildcards, so can't get this far */

    NOT_REACHED; /*NOTREACHED*/
}

STATIC bool
S_grok_bslash_N(pTHX_ RExC_state_t *pRExC_state,
                regnode_offset * node_p,
                UV * code_point_p,
                int * cp_count,
                I32 * flagp,
                const bool strict,
                const U32 depth
    )
{
 /* This routine teases apart the various meanings of \N and returns
  * accordingly.  The input parameters constrain which meaning(s) is/are valid
  * in the current context.
  *
  * Exactly one of <node_p> and <code_point_p> must be non-NULL.
  *
  * If <code_point_p> is not NULL, the context is expecting the result to be a
  * single code point.  If this \N instance turns out to a single code point,
  * the function returns TRUE and sets *code_point_p to that code point.
  *
  * If <node_p> is not NULL, the context is expecting the result to be one of
  * the things representable by a regnode.  If this \N instance turns out to be
  * one such, the function generates the regnode, returns TRUE and sets *node_p
  * to point to the offset of that regnode into the regex engine program being
  * compiled.
  *
  * If this instance of \N isn't legal in any context, this function will
  * generate a fatal error and not return.
  *
  * On input, RExC_parse should point to the first char following the \N at the
  * time of the call.  On successful return, RExC_parse will have been updated
  * to point to just after the sequence identified by this routine.  Also
  * *flagp has been updated as needed.
  *
  * When there is some problem with the current context and this \N instance,
  * the function returns FALSE, without advancing RExC_parse, nor setting
  * *node_p, nor *code_point_p, nor *flagp.
  *
  * If <cp_count> is not NULL, the caller wants to know the length (in code
  * points) that this \N sequence matches.  This is set, and the input is
  * parsed for errors, even if the function returns FALSE, as detailed below.
  *
  * There are 6 possibilities here, as detailed in the next 6 paragraphs.
  *
  * Probably the most common case is for the \N to specify a single code point.
  * *cp_count will be set to 1, and *code_point_p will be set to that code
  * point.
  *
  * Another possibility is for the input to be an empty \N{}.  This is no
  * longer accepted, and will generate a fatal error.
  *
  * Another possibility is for a custom charnames handler to be in effect which
  * translates the input name to an empty string.  *cp_count will be set to 0.
  * *node_p will be set to a generated NOTHING node.
  *
  * Still another possibility is for the \N to mean [^\n]. *cp_count will be
  * set to 0. *node_p will be set to a generated REG_ANY node.
  *
  * The fifth possibility is that \N resolves to a sequence of more than one
  * code points.  *cp_count will be set to the number of code points in the
  * sequence. *node_p will be set to a generated node returned by this
  * function calling S_reg().
  *
  * The sixth and final possibility is that it is premature to be calling this
  * function; the parse needs to be restarted.  This can happen when this
  * changes from /d to /u rules, or when the pattern needs to be upgraded to
  * UTF-8.  The latter occurs only when the fifth possibility would otherwise
  * be in effect, and is because one of those code points requires the pattern
  * to be recompiled as UTF-8.  The function returns FALSE, and sets the
  * RESTART_PARSE and NEED_UTF8 flags in *flagp, as appropriate.  When this
  * happens, the caller needs to desist from continuing parsing, and return
  * this information to its caller.  This is not set for when there is only one
  * code point, as this can be called as part of an ANYOF node, and they can
  * store above-Latin1 code points without the pattern having to be in UTF-8.
  *
  * For non-single-quoted regexes, the tokenizer has resolved character and
  * sequence names inside \N{...} into their Unicode values, normalizing the
  * result into what we should see here: '\N{U+c1.c2...}', where c1... are the
  * hex-represented code points in the sequence.  This is done there because
  * the names can vary based on what charnames pragma is in scope at the time,
  * so we need a way to take a snapshot of what they resolve to at the time of
  * the original parse. [perl #56444].
  *
  * That parsing is skipped for single-quoted regexes, so here we may get
  * '\N{NAME}', which is parsed now.  If the single-quoted regex is something
  * like '\N{U+41}', that code point is Unicode, and has to be translated into
  * the native character set for non-ASCII platforms.  The other possibilities
  * are already native, so no translation is done. */

    char * endbrace;    /* points to '}' following the name */
    char * e;           /* points to final non-blank before endbrace */
    char* p = RExC_parse; /* Temporary */

    SV * substitute_parse = NULL;
    char *orig_end;
    char *save_start;
    I32 flags;

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_GROK_BSLASH_N;

    assert(cBOOL(node_p) ^ cBOOL(code_point_p));  /* Exactly one should be set */
    assert(! (node_p && cp_count));               /* At most 1 should be set */

    if (cp_count) {     /* Initialize return for the most common case */
        *cp_count = 1;
    }

    /* The [^\n] meaning of \N ignores spaces and comments under the /x
     * modifier.  The other meanings do not (except blanks adjacent to and
     * within the braces), so use a temporary until we find out which we are
     * being called with */
    skip_to_be_ignored_text(pRExC_state, &p,
                            FALSE /* Don't force to /x */ );

    /* Disambiguate between \N meaning a named character versus \N meaning
     * [^\n].  The latter is assumed when the {...} following the \N is a legal
     * quantifier, or if there is no '{' at all */
    if (*p != '{' || regcurly(p, RExC_end, NULL)) {
        RExC_parse_set(p);
        if (cp_count) {
            *cp_count = -1;
        }

        if (! node_p) {
            return FALSE;
        }

        *node_p = reg_node(pRExC_state, REG_ANY);
        *flagp |= HASWIDTH|SIMPLE;
        MARK_NAUGHTY(1);
        return TRUE;
    }

    /* The test above made sure that the next real character is a '{', but
     * under the /x modifier, it could be separated by space (or a comment and
     * \n) and this is not allowed (for consistency with \x{...} and the
     * tokenizer handling of \N{NAME}). */
    if (*RExC_parse != '{') {
        vFAIL("Missing braces on \\N{}");
    }

    RExC_parse_inc_by(1);       /* Skip past the '{' */

    endbrace = (char *) memchr(RExC_parse, '}', RExC_end - RExC_parse);
    if (! endbrace) { /* no trailing brace */
        vFAIL2("Missing right brace on \\%c{}", 'N');
    }

    /* Here, we have decided it should be a named character or sequence.  These
     * imply Unicode semantics */
    REQUIRE_UNI_RULES(flagp, FALSE);

    /* \N{_} is what toke.c returns to us to indicate a name that evaluates to
     * nothing at all (not allowed under strict) */
    if (endbrace - RExC_parse == 1 && *RExC_parse == '_') {
        RExC_parse_set(endbrace);
        if (strict) {
            RExC_parse_inc_by(1);   /* Position after the "}" */
            vFAIL("Zero length \\N{}");
        }

        if (cp_count) {
            *cp_count = 0;
        }
        nextchar(pRExC_state);
        if (! node_p) {
            return FALSE;
        }

        *node_p = reg_node(pRExC_state, NOTHING);
        return TRUE;
    }

    while (isBLANK(*RExC_parse)) {
        RExC_parse_inc_by(1);
    }

    e = endbrace;
    while (RExC_parse < e && isBLANK(*(e-1))) {
        e--;
    }

    if (e - RExC_parse < 2 || ! strBEGINs(RExC_parse, "U+")) {

        /* Here, the name isn't of the form  U+....  This can happen if the
         * pattern is single-quoted, so didn't get evaluated in toke.c.  Now
         * is the time to find out what the name means */

        const STRLEN name_len = e - RExC_parse;
        SV *  value_sv;     /* What does this name evaluate to */
        SV ** value_svp;
        const U8 * value;   /* string of name's value */
        STRLEN value_len;   /* and its length */

        /*  RExC_unlexed_names is a hash of names that weren't evaluated by
         *  toke.c, and their values. Make sure is initialized */
        if (! RExC_unlexed_names) {
            RExC_unlexed_names = newHV();
        }

        /* If we have already seen this name in this pattern, use that.  This
         * allows us to only call the charnames handler once per name per
         * pattern.  A broken or malicious handler could return something
         * different each time, which could cause the results to vary depending
         * on if something gets added or subtracted from the pattern that
         * causes the number of passes to change, for example */
        if ((value_svp = hv_fetch(RExC_unlexed_names, RExC_parse,
                                                      name_len, 0)))
        {
            value_sv = *value_svp;
        }
        else { /* Otherwise we have to go out and get the name */
            const char * error_msg = NULL;
            value_sv = get_and_check_backslash_N_name(RExC_parse, e,
                                                      UTF,
                                                      &error_msg);
            if (error_msg) {
                RExC_parse_set(endbrace);
                vFAIL(error_msg);
            }

            /* If no error message, should have gotten a valid return */
            assert (value_sv);

            /* Save the name's meaning for later use */
            if (! hv_store(RExC_unlexed_names, RExC_parse, name_len,
                           value_sv, 0))
            {
                Perl_croak(aTHX_ "panic: hv_store() unexpectedly failed");
            }
        }

        /* Here, we have the value the name evaluates to in 'value_sv' */
        value = (U8 *) SvPV(value_sv, value_len);

        /* See if the result is one code point vs 0 or multiple */
        if (inRANGE(value_len, 1, ((UV) SvUTF8(value_sv)
                                  ? UTF8SKIP(value)
                                  : 1)))
        {
            /* Here, exactly one code point.  If that isn't what is wanted,
             * fail */
            if (! code_point_p) {
                RExC_parse_set(p);
                return FALSE;
            }

            /* Convert from string to numeric code point */
            *code_point_p = (SvUTF8(value_sv))
                            ? valid_utf8_to_uvchr(value, NULL)
                            : *value;

            /* Have parsed this entire single code point \N{...}.  *cp_count
             * has already been set to 1, so don't do it again. */
            RExC_parse_set(endbrace);
            nextchar(pRExC_state);
            return TRUE;
        } /* End of is a single code point */

        /* Count the code points, if caller desires.  The API says to do this
         * even if we will later return FALSE */
        if (cp_count) {
            *cp_count = 0;

            *cp_count = (SvUTF8(value_sv))
                        ? utf8_length(value, value + value_len)
                        : value_len;
        }

        /* Fail if caller doesn't want to handle a multi-code-point sequence.
         * But don't back the pointer up if the caller wants to know how many
         * code points there are (they need to handle it themselves in this
         * case).  */
        if (! node_p) {
            if (! cp_count) {
                RExC_parse_set(p);
            }
            return FALSE;
        }

        /* Convert this to a sub-pattern of the form "(?: ... )", and then call
         * reg recursively to parse it.  That way, it retains its atomicness,
         * while not having to worry about any special handling that some code
         * points may have. */

        substitute_parse = newSVpvs("?:");
        sv_catsv(substitute_parse, value_sv);
        sv_catpv(substitute_parse, ")");

        /* The value should already be native, so no need to convert on EBCDIC
         * platforms.*/
        assert(! RExC_recode_x_to_native);

    }
    else {   /* \N{U+...} */
        Size_t count = 0;   /* code point count kept internally */

        /* We can get to here when the input is \N{U+...} or when toke.c has
         * converted a name to the \N{U+...} form.  This include changing a
         * name that evaluates to multiple code points to \N{U+c1.c2.c3 ...} */

        RExC_parse_inc_by(2);    /* Skip past the 'U+' */

        /* Code points are separated by dots.  The '}' terminates the whole
         * thing. */

        do {    /* Loop until the ending brace */
            I32 flags = PERL_SCAN_SILENT_OVERFLOW
                      | PERL_SCAN_SILENT_ILLDIGIT
                      | PERL_SCAN_NOTIFY_ILLDIGIT
                      | PERL_SCAN_ALLOW_MEDIAL_UNDERSCORES
                      | PERL_SCAN_DISALLOW_PREFIX;
            STRLEN len = e - RExC_parse;
            NV overflow_value;
            char * start_digit = RExC_parse;
            UV cp = grok_hex(RExC_parse, &len, &flags, &overflow_value);

            if (len == 0) {
                RExC_parse_inc_by(1);
              bad_NU:
                vFAIL("Invalid hexadecimal number in \\N{U+...}");
            }

            RExC_parse_inc_by(len);

            if (cp > MAX_LEGAL_CP) {
                vFAIL(form_cp_too_large_msg(16, start_digit, len, 0));
            }

            if (RExC_parse >= e) { /* Got to the closing '}' */
                if (count) {
                    goto do_concat;
                }

                /* Here, is a single code point; fail if doesn't want that */
                if (! code_point_p) {
                    RExC_parse_set(p);
                    return FALSE;
                }

                /* A single code point is easy to handle; just return it */
                *code_point_p = UNI_TO_NATIVE(cp);
                RExC_parse_set(endbrace);
                nextchar(pRExC_state);
                return TRUE;
            }

            /* Here, the parse stopped bfore the ending brace.  This is legal
             * only if that character is a dot separating code points, like a
             * multiple character sequence (of the form "\N{U+c1.c2. ... }".
             * So the next character must be a dot (and the one after that
             * can't be the ending brace, or we'd have something like
             * \N{U+100.} )
             * */
            if (*RExC_parse != '.' || RExC_parse + 1 >= e) {
                /*point to after 1st invalid */
                RExC_parse_incf(RExC_orig_utf8);
                /*Guard against malformed utf8*/
                RExC_parse_set(MIN(e, RExC_parse));
                goto bad_NU;
            }

            /* Here, looks like its really a multiple character sequence.  Fail
             * if that's not what the caller wants.  But continue with counting
             * and error checking if they still want a count */
            if (! node_p && ! cp_count) {
                return FALSE;
            }

            /* What is done here is to convert this to a sub-pattern of the
             * form \x{char1}\x{char2}...  and then call reg recursively to
             * parse it (enclosing in "(?: ... )" ).  That way, it retains its
             * atomicness, while not having to worry about special handling
             * that some code points may have.  We don't create a subpattern,
             * but go through the motions of code point counting and error
             * checking, if the caller doesn't want a node returned. */

            if (node_p && ! substitute_parse) {
                substitute_parse = newSVpvs("?:");
            }

          do_concat:

            if (node_p) {
                /* Convert to notation the rest of the code understands */
                sv_catpvs(substitute_parse, "\\x{");
                sv_catpvn(substitute_parse, start_digit,
                                            RExC_parse - start_digit);
                sv_catpvs(substitute_parse, "}");
            }

            /* Move to after the dot (or ending brace the final time through.)
             * */
            RExC_parse_inc_by(1);
            count++;

        } while (RExC_parse < e);

        if (! node_p) { /* Doesn't want the node */
            assert (cp_count);

            *cp_count = count;
            return FALSE;
        }

        sv_catpvs(substitute_parse, ")");

        /* The values are Unicode, and therefore have to be converted to native
         * on a non-Unicode (meaning non-ASCII) platform. */
        SET_recode_x_to_native(1);
    }

    /* Here, we have the string the name evaluates to, ready to be parsed,
     * stored in 'substitute_parse' as a series of valid "\x{...}\x{...}"
     * constructs.  This can be called from within a substitute parse already.
     * The error reporting mechanism doesn't work for 2 levels of this, but the
     * code above has validated this new construct, so there should be no
     * errors generated by the below.  And this isn't an exact copy, so the
     * mechanism to seamlessly deal with this won't work, so turn off warnings
     * during it */
    save_start = RExC_start;
    orig_end = RExC_end;

    RExC_start = SvPVX(substitute_parse);
    RExC_parse_set(RExC_start);
    RExC_end = RExC_parse + SvCUR(substitute_parse);
    TURN_OFF_WARNINGS_IN_SUBSTITUTE_PARSE;

    *node_p = reg(pRExC_state, 1, &flags, depth+1);

    /* Restore the saved values */
    RESTORE_WARNINGS;
    RExC_start = save_start;
    RExC_parse_set(endbrace);
    RExC_end = orig_end;
    SET_recode_x_to_native(0);

    SvREFCNT_dec_NN(substitute_parse);

    if (! *node_p) {
        RETURN_FAIL_ON_RESTART(flags, flagp);
        FAIL2("panic: reg returned failure to grok_bslash_N, flags=%#" UVxf,
            (UV) flags);
    }
    *flagp |= flags&(HASWIDTH|SIMPLE|POSTPONED);

    nextchar(pRExC_state);

    return TRUE;
}


STATIC U8
S_compute_EXACTish(RExC_state_t *pRExC_state)
{
    U8 op;

    PERL_ARGS_ASSERT_COMPUTE_EXACTISH;

    if (! FOLD) {
        return (LOC)
                ? EXACTL
                : EXACT;
    }

    op = get_regex_charset(RExC_flags);
    if (op >= REGEX_ASCII_RESTRICTED_CHARSET) {
        op--; /* /a is same as /u, and map /aa's offset to what /a's would have
                 been, so there is no hole */
    }

    return op + EXACTF;
}

/* Parse backref decimal value, unless it's too big to sensibly be a backref,
 * in which case return I32_MAX (rather than possibly 32-bit wrapping) */

static I32
S_backref_value(char *p, char *e)
{
    const char* endptr = e;
    UV val;
    if (grok_atoUV(p, &val, &endptr) && val <= I32_MAX)
        return (I32)val;
    return I32_MAX;
}


/*
 - regatom - the lowest level

   Try to identify anything special at the start of the current parse position.
   If there is, then handle it as required. This may involve generating a
   single regop, such as for an assertion; or it may involve recursing, such as
   to handle a () structure.

   If the string doesn't start with something special then we gobble up
   as much literal text as we can.  If we encounter a quantifier, we have to
   back off the final literal character, as that quantifier applies to just it
   and not to the whole string of literals.

   Once we have been able to handle whatever type of thing started the
   sequence, we return the offset into the regex engine program being compiled
   at which any  next regnode should be placed.

   Returns 0, setting *flagp to TRYAGAIN if reg() returns 0 with TRYAGAIN.
   Returns 0, setting *flagp to RESTART_PARSE if the parse needs to be
   restarted, or'd with NEED_UTF8 if the pattern needs to be upgraded to UTF-8
   Otherwise does not return 0.

   Note: we have to be careful with escapes, as they can be both literal
   and special, and in the case of \10 and friends, context determines which.

   A summary of the code structure is:

   switch (first_byte) {
        cases for each special:
            handle this special;
            break;
        case '\\':
            switch (2nd byte) {
                cases for each unambiguous special:
                    handle this special;
                    break;
                cases for each ambiguous special/literal:
                    disambiguate;
                    if (special)  handle here
                    else goto defchar;
                default: // unambiguously literal:
                    goto defchar;
            }
        default:  // is a literal char
            // FALL THROUGH
        defchar:
            create EXACTish node for literal;
            while (more input and node isn't full) {
                switch (input_byte) {
                   cases for each special;
                       make sure parse pointer is set so that the next call to
                           regatom will see this special first
                       goto loopdone; // EXACTish node terminated by prev. char
                   default:
                       append char to EXACTISH node;
                }
                get next input byte;
            }
        loopdone:
   }
   return the generated node;

   Specifically there are two separate switches for handling
   escape sequences, with the one for handling literal escapes requiring
   a dummy entry for all of the special escapes that are actually handled
   by the other.

*/

STATIC regnode_offset
S_regatom(pTHX_ RExC_state_t *pRExC_state, I32 *flagp, U32 depth)
{
    regnode_offset ret = 0;
    I32 flags = 0;
    char *atom_parse_start;
    U8 op;
    int invert = 0;

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    *flagp = 0;		/* Initialize. */

    DEBUG_PARSE("atom");

    PERL_ARGS_ASSERT_REGATOM;

  tryagain:
    atom_parse_start = RExC_parse;
    assert(RExC_parse < RExC_end);
    switch ((U8)*RExC_parse) {
    case '^':
        RExC_seen_zerolen++;
        nextchar(pRExC_state);
        if (RExC_flags & RXf_PMf_MULTILINE)
            ret = reg_node(pRExC_state, MBOL);
        else
            ret = reg_node(pRExC_state, SBOL);
        break;
    case '$':
        nextchar(pRExC_state);
        if (*RExC_parse)
            RExC_seen_zerolen++;
        if (RExC_flags & RXf_PMf_MULTILINE)
            ret = reg_node(pRExC_state, MEOL);
        else
            ret = reg_node(pRExC_state, SEOL);
        break;
    case '.':
        nextchar(pRExC_state);
        if (RExC_flags & RXf_PMf_SINGLELINE)
            ret = reg_node(pRExC_state, SANY);
        else
            ret = reg_node(pRExC_state, REG_ANY);
        *flagp |= HASWIDTH|SIMPLE;
        MARK_NAUGHTY(1);
        break;
    case '[':
    {
        char * const cc_parse_start = ++RExC_parse;
        ret = regclass(pRExC_state, flagp, depth+1,
                       FALSE, /* means parse the whole char class */
                       TRUE, /* allow multi-char folds */
                       FALSE, /* don't silence non-portable warnings. */
                       (bool) RExC_strict,
                       TRUE, /* Allow an optimized regnode result */
                       NULL);
        if (ret == 0) {
            RETURN_FAIL_ON_RESTART_FLAGP(flagp);
            FAIL2("panic: regclass returned failure to regatom, flags=%#" UVxf,
                  (UV) *flagp);
        }
        if (*RExC_parse != ']') {
            RExC_parse_set(cc_parse_start);
            vFAIL("Unmatched [");
        }
        nextchar(pRExC_state);
        break;
    }
    case '(':
        nextchar(pRExC_state);
        ret = reg(pRExC_state, 2, &flags, depth+1);
        if (ret == 0) {
                if (flags & TRYAGAIN) {
                    if (RExC_parse >= RExC_end) {
                         /* Make parent create an empty node if needed. */
                        *flagp |= TRYAGAIN;
                        return(0);
                    }
                    goto tryagain;
                }
                RETURN_FAIL_ON_RESTART(flags, flagp);
                FAIL2("panic: reg returned failure to regatom, flags=%#" UVxf,
                                                                 (UV) flags);
        }
        *flagp |= flags&(HASWIDTH|SIMPLE|POSTPONED);
        break;
    case '|':
    case ')':
        if (flags & TRYAGAIN) {
            *flagp |= TRYAGAIN;
            return 0;
        }
        vFAIL("Internal urp");
                                /* Supposed to be caught earlier. */
        break;
    case '?':
    case '+':
    case '*':
        RExC_parse_inc_by(1);
        vFAIL("Quantifier follows nothing");
        break;
    case '\\':
        /* Special Escapes

           This switch handles escape sequences that resolve to some kind
           of special regop and not to literal text. Escape sequences that
           resolve to literal text are handled below in the switch marked
           "Literal Escapes".

           Every entry in this switch *must* have a corresponding entry
           in the literal escape switch. However, the opposite is not
           required, as the default for this switch is to jump to the
           literal text handling code.
        */
        RExC_parse_inc_by(1);
        switch ((U8)*RExC_parse) {
        /* Special Escapes */
        case 'A':
            RExC_seen_zerolen++;
            /* Under wildcards, this is changed to match \n; should be
             * invisible to the user, as they have to compile under /m */
            if (RExC_pm_flags & PMf_WILDCARD) {
                ret = reg_node(pRExC_state, MBOL);
            }
            else {
                ret = reg_node(pRExC_state, SBOL);
                /* SBOL is shared with /^/ so we set the flags so we can tell
                 * /\A/ from /^/ in split. */
                FLAGS(REGNODE_p(ret)) = 1;
            }
            goto finish_meta_pat;
        case 'G':
            if (RExC_pm_flags & PMf_WILDCARD) {
                RExC_parse_inc_by(1);
                /* diag_listed_as: Use of %s is not allowed in Unicode property
                   wildcard subpatterns in regex; marked by <-- HERE in m/%s/
                 */
                vFAIL("Use of '\\G' is not allowed in Unicode property"
                      " wildcard subpatterns");
            }
            ret = reg_node(pRExC_state, GPOS);
            RExC_seen |= REG_GPOS_SEEN;
            goto finish_meta_pat;
        case 'K':
            if (!RExC_in_lookaround) {
                RExC_seen_zerolen++;
                ret = reg_node(pRExC_state, KEEPS);
                /* XXX:dmq : disabling in-place substitution seems to
                 * be necessary here to avoid cases of memory corruption, as
                 * with: C<$_="x" x 80; s/x\K/y/> -- rgs
                 */
                RExC_seen |= REG_LOOKBEHIND_SEEN;
                goto finish_meta_pat;
            }
            else {
                ++RExC_parse; /* advance past the 'K' */
                vFAIL("\\K not permitted in lookahead/lookbehind");
            }
        case 'Z':
            if (RExC_pm_flags & PMf_WILDCARD) {
                /* See comment under \A above */
                ret = reg_node(pRExC_state, MEOL);
            }
            else {
                ret = reg_node(pRExC_state, SEOL);
            }
            RExC_seen_zerolen++;		/* Do not optimize RE away */
            goto finish_meta_pat;
        case 'z':
            if (RExC_pm_flags & PMf_WILDCARD) {
                /* See comment under \A above */
                ret = reg_node(pRExC_state, MEOL);
            }
            else {
                ret = reg_node(pRExC_state, EOS);
            }
            RExC_seen_zerolen++;		/* Do not optimize RE away */
            goto finish_meta_pat;
        case 'C':
            vFAIL("\\C no longer supported");
        case 'X':
            ret = reg_node(pRExC_state, CLUMP);
            *flagp |= HASWIDTH;
            goto finish_meta_pat;

        case 'B':
            invert = 1;
            /* FALLTHROUGH */
        case 'b':
          {
            U8 flags = 0;
            regex_charset charset = get_regex_charset(RExC_flags);

            RExC_seen_zerolen++;
            RExC_seen |= REG_LOOKBEHIND_SEEN;
            op = BOUND + charset;

            if (RExC_parse >= RExC_end || *(RExC_parse + 1) != '{') {
                flags = TRADITIONAL_BOUND;
                if (op > BOUNDA) {  /* /aa is same as /a */
                    op = BOUNDA;
                }
            }
            else {
                STRLEN length;
                char name = *RExC_parse;
                char * endbrace =  (char *) memchr(RExC_parse, '}',
                                                   RExC_end - RExC_parse);
                char * e = endbrace;

                RExC_parse_inc_by(2);

                if (! endbrace) {
                    vFAIL2("Missing right brace on \\%c{}", name);
                }

                while (isBLANK(*RExC_parse)) {
                    RExC_parse_inc_by(1);
                }

                while (RExC_parse < e && isBLANK(*(e - 1))) {
                    e--;
                }

                if (e == RExC_parse) {
                    RExC_parse_set(endbrace + 1);  /* After the '}' */
                    vFAIL2("Empty \\%c{}", name);
                }

                length = e - RExC_parse;

                switch (*RExC_parse) {
                    case 'g':
                        if (    length != 1
                            && (memNEs(RExC_parse + 1, length - 1, "cb")))
                        {
                            goto bad_bound_type;
                        }
                        flags = GCB_BOUND;
                        break;
                    case 'l':
                        if (length != 2 || *(RExC_parse + 1) != 'b') {
                            goto bad_bound_type;
                        }
                        flags = LB_BOUND;
                        break;
                    case 's':
                        if (length != 2 || *(RExC_parse + 1) != 'b') {
                            goto bad_bound_type;
                        }
                        flags = SB_BOUND;
                        break;
                    case 'w':
                        if (length != 2 || *(RExC_parse + 1) != 'b') {
                            goto bad_bound_type;
                        }
                        flags = WB_BOUND;
                        break;
                    default:
                      bad_bound_type:
                        RExC_parse_set(e);
                        vFAIL2utf8f(
                            "'%" UTF8f "' is an unknown bound type",
                            UTF8fARG(UTF, length, e - length));
                        NOT_REACHED; /*NOTREACHED*/
                }
                RExC_parse_set(endbrace);
                REQUIRE_UNI_RULES(flagp, 0);

                if (op == BOUND) {
                    op = BOUNDU;
                }
                else if (op >= BOUNDA) {  /* /aa is same as /a */
                    op = BOUNDU;
                    length += 4;

                    /* Don't have to worry about UTF-8, in this message because
                     * to get here the contents of the \b must be ASCII */
                    ckWARN4reg(RExC_parse + 1,  /* Include the '}' in msg */
                              "Using /u for '%.*s' instead of /%s",
                              (unsigned) length,
                              endbrace - length + 1,
                              (charset == REGEX_ASCII_RESTRICTED_CHARSET)
                              ? ASCII_RESTRICT_PAT_MODS
                              : ASCII_MORE_RESTRICT_PAT_MODS);
                }
            }

            if (op == BOUND) {
                RExC_seen_d_op = TRUE;
            }
            else if (op == BOUNDL) {
                RExC_contains_locale = 1;
            }

            if (invert) {
                op += NBOUND - BOUND;
            }

            ret = reg_node(pRExC_state, op);
            FLAGS(REGNODE_p(ret)) = flags;

            goto finish_meta_pat;
          }

        case 'R':
            ret = reg_node(pRExC_state, LNBREAK);
            *flagp |= HASWIDTH|SIMPLE;
            goto finish_meta_pat;

        case 'd':
        case 'D':
        case 'h':
        case 'H':
        case 'p':
        case 'P':
        case 's':
        case 'S':
        case 'v':
        case 'V':
        case 'w':
        case 'W':
            /* These all have the same meaning inside [brackets], and it knows
             * how to do the best optimizations for them.  So, pretend we found
             * these within brackets, and let it do the work */
            RExC_parse--;

            ret = regclass(pRExC_state, flagp, depth+1,
                           TRUE, /* means just parse this element */
                           FALSE, /* don't allow multi-char folds */
                           FALSE, /* don't silence non-portable warnings.  It
                                     would be a bug if these returned
                                     non-portables */
                           (bool) RExC_strict,
                           TRUE, /* Allow an optimized regnode result */
                           NULL);
            RETURN_FAIL_ON_RESTART_FLAGP(flagp);
            /* regclass() can only return RESTART_PARSE and NEED_UTF8 if
             * multi-char folds are allowed.  */
            if (!ret)
                FAIL2("panic: regclass returned failure to regatom, flags=%#" UVxf,
                      (UV) *flagp);

            RExC_parse--;   /* regclass() leaves this one too far ahead */

          finish_meta_pat:
                   /* The escapes above that don't take a parameter can't be
                    * followed by a '{'.  But 'pX', 'p{foo}' and
                    * correspondingly 'P' can be */
            if (   RExC_parse - atom_parse_start == 1
                && UCHARAT(RExC_parse + 1) == '{'
                && UNLIKELY(! regcurly(RExC_parse + 1, RExC_end, NULL)))
            {
                RExC_parse_inc_by(2);
                vFAIL("Unescaped left brace in regex is illegal here");
            }
            nextchar(pRExC_state);
            break;
        case 'N':
            /* Handle \N, \N{} and \N{NAMED SEQUENCE} (the latter meaning the
             * \N{...} evaluates to a sequence of more than one code points).
             * The function call below returns a regnode, which is our result.
             * The parameters cause it to fail if the \N{} evaluates to a
             * single code point; we handle those like any other literal.  The
             * reason that the multicharacter case is handled here and not as
             * part of the EXACtish code is because of quantifiers.  In
             * /\N{BLAH}+/, the '+' applies to the whole thing, and doing it
             * this way makes that Just Happen. dmq.
             * join_exact() will join this up with adjacent EXACTish nodes
             * later on, if appropriate. */
            ++RExC_parse;
            if (grok_bslash_N(pRExC_state,
                              &ret,     /* Want a regnode returned */
                              NULL,     /* Fail if evaluates to a single code
                                           point */
                              NULL,     /* Don't need a count of how many code
                                           points */
                              flagp,
                              RExC_strict,
                              depth)
            ) {
                break;
            }

            RETURN_FAIL_ON_RESTART_FLAGP(flagp);

            /* Here, evaluates to a single code point.  Go get that */
            RExC_parse_set(atom_parse_start);
            goto defchar;

        case 'k':    /* Handle \k<NAME> and \k'NAME' and \k{NAME} */
      parse_named_seq:  /* Also handle non-numeric \g{...} */
        {
            char ch;
            if (   RExC_parse >= RExC_end - 1
                || ((   ch = RExC_parse[1]) != '<'
                                      && ch != '\''
                                      && ch != '{'))
            {
                RExC_parse_inc_by(1);
                /* diag_listed_as: Sequence \%s... not terminated in regex; marked by <-- HERE in m/%s/ */
                vFAIL2("Sequence %.2s... not terminated", atom_parse_start);
            } else {
                RExC_parse_inc_by(2);
                if (ch == '{') {
                    while (isBLANK(*RExC_parse)) {
                        RExC_parse_inc_by(1);
                    }
                }
                ret = handle_named_backref(pRExC_state,
                                           flagp,
                                           atom_parse_start,
                                           (ch == '<')
                                           ? '>'
                                           : (ch == '{')
                                             ? '}'
                                             : '\'');
            }
            break;
        }
        case 'g':
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            {
                I32 num;
                char * endbrace = NULL;
                char * s = RExC_parse;
                char * e = RExC_end;

                if (*s == 'g') {
                    bool isrel = 0;

                    s++;
                    if (*s == '{') {
                        endbrace = (char *) memchr(s, '}', RExC_end - s);
                        if (! endbrace ) {

                            /* Missing '}'.  Position after the number to give
                             * a better indication to the user of where the
                             * problem is. */
                            s++;
                            if (*s == '-') {
                                s++;
                            }

                            /* If it looks to be a name and not a number, go
                             * handle it there */
                            if (! isDIGIT(*s)) {
                                goto parse_named_seq;
                            }

                            do {
                                s++;
                            } while isDIGIT(*s);

                            RExC_parse_set(s);
                            vFAIL("Unterminated \\g{...} pattern");
                        }

                        s++;    /* Past the '{' */

                        while (isBLANK(*s)) {
                            s++;
                        }

                        /* Ignore trailing blanks */
                        e = endbrace;
                        while (s < e && isBLANK(*(e - 1))) {
                            e--;
                        }
                    }

                    /* Here, have isolated the meat of the construct from any
                     * surrounding braces */

                    if (*s == '-') {
                        isrel = 1;
                        s++;
                    }

                    if (endbrace && !isDIGIT(*s)) {
                        goto parse_named_seq;
                    }

                    RExC_parse_set(s);
                    num = S_backref_value(RExC_parse, RExC_end);
                    if (num == 0)
                        vFAIL("Reference to invalid group 0");
                    else if (num == I32_MAX) {
                         if (isDIGIT(*RExC_parse))
                            vFAIL("Reference to nonexistent group");
                        else
                            vFAIL("Unterminated \\g... pattern");
                    }

                    if (isrel) {
                        num = RExC_npar - num;
                        if (num < 1)
                            vFAIL("Reference to nonexistent or unclosed group");
                    }
                    else
                    if (num < RExC_logical_npar) {
                        num = RExC_logical_to_parno[num];
                    }
                    else
                    if (ALL_PARENS_COUNTED)  {
                        if (num < RExC_logical_total_parens)
                            num = RExC_logical_to_parno[num];
                        else {
                            num = -1;
                        }
                    }
                    else{
                        REQUIRE_PARENS_PASS;
                    }
                }
                else {
                    num = S_backref_value(RExC_parse, RExC_end);
                    /* bare \NNN might be backref or octal - if it is larger
                     * than or equal RExC_npar then it is assumed to be an
                     * octal escape. Note RExC_npar is +1 from the actual
                     * number of parens. */
                    /* Note we do NOT check if num == I32_MAX here, as that is
                     * handled by the RExC_npar check */

                    if (    /* any numeric escape < 10 is always a backref */
                           num > 9
                            /* any numeric escape < RExC_npar is a backref */
                        && num >= RExC_logical_npar
                            /* cannot be an octal escape if it starts with [89]
                             * */
                        && ! inRANGE(*RExC_parse, '8', '9')
                    ) {
                        /* Probably not meant to be a backref, instead likely
                         * to be an octal character escape, e.g. \35 or \777.
                         * The above logic should make it obvious why using
                         * octal escapes in patterns is problematic. - Yves */
                        RExC_parse_set(atom_parse_start);
                        goto defchar;
                    }
                    if (num < RExC_logical_npar) {
                        num = RExC_logical_to_parno[num];
                    }
                    else
                    if (ALL_PARENS_COUNTED) {
                        if (num < RExC_logical_total_parens) {
                            num = RExC_logical_to_parno[num];
                        } else {
                            num = -1;
                        }
                    } else {
                        REQUIRE_PARENS_PASS;
                    }
                }

                /* At this point RExC_parse points at a numeric escape like
                 * \12 or \88 or the digits in \g{34} or \g34 or something
                 * similar, which we should NOT treat as an octal escape. It
                 * may or may not be a valid backref escape. For instance
                 * \88888888 is unlikely to be a valid backref.
                 *
                 * We've already figured out what value the digits represent.
                 * Now, move the parse to beyond them. */
                if (endbrace) {
                    RExC_parse_set(endbrace + 1);
                }
                else while (isDIGIT(*RExC_parse)) {
                    RExC_parse_inc_by(1);
                }
                if (num < 0)
                    vFAIL("Reference to nonexistent group");

                if (num >= (I32)RExC_npar) {
                    /* It might be a forward reference; we can't fail until we
                     * know, by completing the parse to get all the groups, and
                     * then reparsing */
                    if (ALL_PARENS_COUNTED)  {
                        if (num >= RExC_total_parens)  {
                            vFAIL("Reference to nonexistent group");
                        }
                    }
                    else {
                        REQUIRE_PARENS_PASS;
                    }
                }
                RExC_sawback = 1;
                ret = reg2node(pRExC_state,
                               ((! FOLD)
                                 ? REF
                                 : (ASCII_FOLD_RESTRICTED)
                                   ? REFFA
                                   : (AT_LEAST_UNI_SEMANTICS)
                                     ? REFFU
                                     : (LOC)
                                       ? REFFL
                                       : REFF),
                                num, RExC_nestroot);
                if (RExC_nestroot && num >= RExC_nestroot)
                    FLAGS(REGNODE_p(ret)) = VOLATILE_REF;
                if (OP(REGNODE_p(ret)) == REFF) {
                    RExC_seen_d_op = TRUE;
                }
                *flagp |= HASWIDTH;

                skip_to_be_ignored_text(pRExC_state, &RExC_parse,
                                        FALSE /* Don't force to /x */ );
            }
            break;
        case '\0':
            if (RExC_parse >= RExC_end)
                FAIL("Trailing \\");
            /* FALLTHROUGH */
        default:
            /* Do not generate "unrecognized" warnings here, we fall
               back into the quick-grab loop below */
            RExC_parse_set(atom_parse_start);
            goto defchar;
        } /* end of switch on a \foo sequence */
        break;

    case '#':

        /* '#' comments should have been spaced over before this function was
         * called */
        assert((RExC_flags & RXf_PMf_EXTENDED) == 0);
        /*
        if (RExC_flags & RXf_PMf_EXTENDED) {
            RExC_parse_set( reg_skipcomment( pRExC_state, RExC_parse ) );
            if (RExC_parse < RExC_end)
                goto tryagain;
        }
        */

        /* FALLTHROUGH */

    default:
          defchar: {

            /* Here, we have determined that the next thing is probably a
             * literal character.  RExC_parse points to the first byte of its
             * definition.  (It still may be an escape sequence that evaluates
             * to a single character) */

            STRLEN len = 0;
            UV ender = 0;
            char *p;
            char *s, *old_s = NULL, *old_old_s = NULL;
            char *s0;
            U32 max_string_len = 255;

            /* We may have to reparse the node, artificially stopping filling
             * it early, based on info gleaned in the first parse.  This
             * variable gives where we stop.  Make it above the normal stopping
             * place first time through; otherwise it would stop too early */
            U32 upper_fill = max_string_len + 1;

            /* We start out as an EXACT node, even if under /i, until we find a
             * character which is in a fold.  The algorithm now segregates into
             * separate nodes, characters that fold from those that don't under
             * /i.  (This hopefully will create nodes that are fixed strings
             * even under /i, giving the optimizer something to grab on to.)
             * So, if a node has something in it and the next character is in
             * the opposite category, that node is closed up, and the function
             * returns.  Then regatom is called again, and a new node is
             * created for the new category. */
            U8 node_type = EXACT;

            /* Assume the node will be fully used; the excess is given back at
             * the end.  Under /i, we may need to temporarily add the fold of
             * an extra character or two at the end to check for splitting
             * multi-char folds, so allocate extra space for that.   We can't
             * make any other length assumptions, as a byte input sequence
             * could shrink down. */
            Ptrdiff_t current_string_nodes = STR_SZ(max_string_len
                                                 + ((! FOLD)
                                                    ? 0
                                                    : 2 * ((UTF)
                                                           ? UTF8_MAXBYTES_CASE
                        /* Max non-UTF-8 expansion is 2 */ : 2)));

            bool next_is_quantifier;
            char * oldp = NULL;

            /* We can convert EXACTF nodes to EXACTFU if they contain only
             * characters that match identically regardless of the target
             * string's UTF8ness.  The reason to do this is that EXACTF is not
             * trie-able, EXACTFU is, and EXACTFU requires fewer operations at
             * runtime.
             *
             * Similarly, we can convert EXACTFL nodes to EXACTFLU8 if they
             * contain only above-Latin1 characters (hence must be in UTF8),
             * which don't participate in folds with Latin1-range characters,
             * as the latter's folds aren't known until runtime. */
            bool maybe_exactfu = FOLD && (DEPENDS_SEMANTICS || LOC);

            /* Single-character EXACTish nodes are almost always SIMPLE.  This
             * allows us to override this as encountered */
            U8 maybe_SIMPLE = SIMPLE;

            /* Does this node contain something that can't match unless the
             * target string is (also) in UTF-8 */
            bool requires_utf8_target = FALSE;

            /* The sequence 'ss' is problematic in non-UTF-8 patterns. */
            bool has_ss = FALSE;

            /* So is the MICRO SIGN */
            bool has_micro_sign = FALSE;

            /* Set when we fill up the current node and there is still more
             * text to process */
            bool overflowed;

            /* Allocate an EXACT node.  The node_type may change below to
             * another EXACTish node, but since the size of the node doesn't
             * change, it works */
            ret = REGNODE_GUTS(pRExC_state, node_type, current_string_nodes);
            FILL_NODE(ret, node_type);
            RExC_emit += NODE_STEP_REGNODE;

            s = STRING(REGNODE_p(ret));

            s0 = s;

          reparse:

            p = RExC_parse;
            len = 0;
            s = s0;
            node_type = EXACT;
            oldp = NULL;
            maybe_exactfu = FOLD && (DEPENDS_SEMANTICS || LOC);
            maybe_SIMPLE = SIMPLE;
            requires_utf8_target = FALSE;
            has_ss = FALSE;
            has_micro_sign = FALSE;

          continue_parse:

            /* This breaks under rare circumstances.  If folding, we do not
             * want to split a node at a character that is a non-final in a
             * multi-char fold, as an input string could just happen to want to
             * match across the node boundary.  The code at the end of the loop
             * looks for this, and backs off until it finds not such a
             * character, but it is possible (though extremely, extremely
             * unlikely) for all characters in the node to be non-final fold
             * ones, in which case we just leave the node fully filled, and
             * hope that it doesn't match the string in just the wrong place */

            assert( ! UTF     /* Is at the beginning of a character */
                   || UTF8_IS_INVARIANT(UCHARAT(RExC_parse))
                   || UTF8_IS_START(UCHARAT(RExC_parse)));

            overflowed = FALSE;

            /* Here, we have a literal character.  Find the maximal string of
             * them in the input that we can fit into a single EXACTish node.
             * We quit at the first non-literal or when the node gets full, or
             * under /i the categorization of folding/non-folding character
             * changes */
            while (p < RExC_end && len < upper_fill) {

                /* In most cases each iteration adds one byte to the output.
                 * The exceptions override this */
                Size_t added_len = 1;

                oldp = p;
                old_old_s = old_s;
                old_s = s;

                /* White space has already been ignored */
                assert(   (RExC_flags & RXf_PMf_EXTENDED) == 0
                       || ! is_PATWS_safe((p), RExC_end, UTF));

                switch ((U8)*p) {
                  const char* message;
                  U32 packed_warn;
                  U8 grok_c_char;

                case '^':
                case '$':
                case '.':
                case '[':
                case '(':
                case ')':
                case '|':
                    goto loopdone;
                case '\\':
                    /* Literal Escapes Switch

                       This switch is meant to handle escape sequences that
                       resolve to a literal character.

                       Every escape sequence that represents something
                       else, like an assertion or a char class, is handled
                       in the switch marked 'Special Escapes' above in this
                       routine, but also has an entry here as anything that
                       isn't explicitly mentioned here will be treated as
                       an unescaped equivalent literal.
                    */

                    switch ((U8)*++p) {

                    /* These are all the special escapes. */
                    case 'A':             /* Start assertion */
                    case 'b': case 'B':   /* Word-boundary assertion*/
                    case 'C':             /* Single char !DANGEROUS! */
                    case 'd': case 'D':   /* digit class */
                    case 'g': case 'G':   /* generic-backref, pos assertion */
                    case 'h': case 'H':   /* HORIZWS */
                    case 'k': case 'K':   /* named backref, keep marker */
                    case 'p': case 'P':   /* Unicode property */
                              case 'R':   /* LNBREAK */
                    case 's': case 'S':   /* space class */
                    case 'v': case 'V':   /* VERTWS */
                    case 'w': case 'W':   /* word class */
                    case 'X':             /* eXtended Unicode "combining
                                             character sequence" */
                    case 'z': case 'Z':   /* End of line/string assertion */
                        --p;
                        goto loopdone;

                    /* Anything after here is an escape that resolves to a
                       literal. (Except digits, which may or may not)
                     */
                    case 'n':
                        ender = '\n';
                        p++;
                        break;
                    case 'N': /* Handle a single-code point named character. */
                        RExC_parse_set( p + 1 );
                        if (! grok_bslash_N(pRExC_state,
                                            NULL,   /* Fail if evaluates to
                                                       anything other than a
                                                       single code point */
                                            &ender, /* The returned single code
                                                       point */
                                            NULL,   /* Don't need a count of
                                                       how many code points */
                                            flagp,
                                            RExC_strict,
                                            depth)
                        ) {
                            if (*flagp & NEED_UTF8)
                                FAIL("panic: grok_bslash_N set NEED_UTF8");
                            RETURN_FAIL_ON_RESTART_FLAGP(flagp);

                            /* Here, it wasn't a single code point.  Go close
                             * up this EXACTish node.  The switch() prior to
                             * this switch handles the other cases */
                            p = oldp;
                            RExC_parse_set(p);
                            goto loopdone;
                        }
                        p = RExC_parse;
                        RExC_parse_set(atom_parse_start);

                        /* The \N{} means the pattern, if previously /d,
                         * becomes /u.  That means it can't be an EXACTF node,
                         * but an EXACTFU */
                        if (node_type == EXACTF) {
                            node_type = EXACTFU;

                            /* If the node already contains something that
                             * differs between EXACTF and EXACTFU, reparse it
                             * as EXACTFU */
                            if (! maybe_exactfu) {
                                len = 0;
                                s = s0;
                                goto reparse;
                            }
                        }

                        break;
                    case 'r':
                        ender = '\r';
                        p++;
                        break;
                    case 't':
                        ender = '\t';
                        p++;
                        break;
                    case 'f':
                        ender = '\f';
                        p++;
                        break;
                    case 'e':
                        ender = ESC_NATIVE;
                        p++;
                        break;
                    case 'a':
                        ender = '\a';
                        p++;
                        break;
                    case 'o':
                        if (! grok_bslash_o(&p,
                                            RExC_end,
                                            &ender,
                                            &message,
                                            &packed_warn,
                                            (bool) RExC_strict,
                                            FALSE, /* No illegal cp's */
                                            UTF))
                        {
                            RExC_parse_set(p); /* going to die anyway; point to
                                               exact spot of failure */
                            vFAIL(message);
                        }

                        if (message && TO_OUTPUT_WARNINGS(p)) {
                            warn_non_literal_string(p, packed_warn, message);
                        }
                        break;
                    case 'x':
                        if (! grok_bslash_x(&p,
                                            RExC_end,
                                            &ender,
                                            &message,
                                            &packed_warn,
                                            (bool) RExC_strict,
                                            FALSE, /* No illegal cp's */
                                            UTF))
                        {
                            RExC_parse_set(p);        /* going to die anyway; point
                                                   to exact spot of failure */
                            vFAIL(message);
                        }

                        if (message && TO_OUTPUT_WARNINGS(p)) {
                            warn_non_literal_string(p, packed_warn, message);
                        }

#ifdef EBCDIC
                        if (ender < 0x100) {
                            if (RExC_recode_x_to_native) {
                                ender = LATIN1_TO_NATIVE(ender);
                            }
                        }
#endif
                        break;
                    case 'c':
                        p++;
                        if (! grok_bslash_c(*p, &grok_c_char,
                                            &message, &packed_warn))
                        {
                            /* going to die anyway; point to exact spot of
                             * failure */
                            char *new_p= p + ((UTF)
                                              ? UTF8_SAFE_SKIP(p, RExC_end)
                                              : 1);
                            RExC_parse_set(new_p);
                            vFAIL(message);
                        }

                        ender = grok_c_char;
                        p++;
                        if (message && TO_OUTPUT_WARNINGS(p)) {
                            warn_non_literal_string(p, packed_warn, message);
                        }

                        break;
                    case '8': case '9': /* must be a backreference */
                        --p;
                        /* we have an escape like \8 which cannot be an octal escape
                         * so we exit the loop, and let the outer loop handle this
                         * escape which may or may not be a legitimate backref. */
                        goto loopdone;
                    case '1': case '2': case '3':case '4':
                    case '5': case '6': case '7':

                        /* When we parse backslash escapes there is ambiguity
                         * between backreferences and octal escapes. Any escape
                         * from \1 - \9 is a backreference, any multi-digit
                         * escape which does not start with 0 and which when
                         * evaluated as decimal could refer to an already
                         * parsed capture buffer is a back reference. Anything
                         * else is octal.
                         *
                         * Note this implies that \118 could be interpreted as
                         * 118 OR as "\11" . "8" depending on whether there
                         * were 118 capture buffers defined already in the
                         * pattern.  */

                        /* NOTE, RExC_npar is 1 more than the actual number of
                         * parens we have seen so far, hence the "<" as opposed
                         * to "<=" */
                        if ( !isDIGIT(p[1]) || S_backref_value(p, RExC_end) < RExC_npar)
                        {  /* Not to be treated as an octal constant, go
                                   find backref */
                            p = oldp;
                            goto loopdone;
                        }
                        /* FALLTHROUGH */
                    case '0':
                        {
                            I32 flags = PERL_SCAN_SILENT_ILLDIGIT
                                      | PERL_SCAN_NOTIFY_ILLDIGIT;
                            STRLEN numlen = 3;
                            ender = grok_oct(p, &numlen, &flags, NULL);
                            p += numlen;
                            if (  (flags & PERL_SCAN_NOTIFY_ILLDIGIT)
                                && isDIGIT(*p)  /* like \08, \178 */
                                && ckWARN(WARN_REGEXP))
                            {
                                reg_warn_non_literal_string(
                                     p + 1,
                                     form_alien_digit_msg(8, numlen, p,
                                                        RExC_end, UTF, FALSE));
                            }
                        }
                        break;
                    case '\0':
                        if (p >= RExC_end)
                            FAIL("Trailing \\");
                        /* FALLTHROUGH */
                    default:
                        if (isALPHANUMERIC(*p)) {
                            /* An alpha followed by '{' is going to fail next
                             * iteration, so don't output this warning in that
                             * case */
                            if (! isALPHA(*p) || *(p + 1) != '{') {
                                ckWARN2reg(p + 1, "Unrecognized escape \\%.1s"
                                                  " passed through", p);
                            }
                        }
                        goto normal_default;
                    } /* End of switch on '\' */
                    break;
                case '{':
                    /* Trying to gain new uses for '{' without breaking too
                     * much existing code is hard.  The solution currently
                     * adopted is:
                     *  1)  If there is no ambiguity that a '{' should always
                     *      be taken literally, at the start of a construct, we
                     *      just do so.
                     *  2)  If the literal '{' conflicts with our desired use
                     *      of it as a metacharacter, we die.  The deprecation
                     *      cycles for this have come and gone.
                     *  3)  If there is ambiguity, we raise a simple warning.
                     *      This could happen, for example, if the user
                     *      intended it to introduce a quantifier, but slightly
                     *      misspelled the quantifier.  Without this warning,
                     *      the quantifier would silently be taken as a literal
                     *      string of characters instead of a meta construct */
                    if (len || (p > RExC_start && isALPHA_A(*(p - 1)))) {
                        if (      RExC_strict
                            || (  p > atom_parse_start + 1
                                && isALPHA_A(*(p - 1))
                                && *(p - 2) == '\\'))
                        {
                            RExC_parse_set(p + 1);
                            vFAIL("Unescaped left brace in regex is "
                                  "illegal here");
                        }
                        ckWARNreg(p + 1, "Unescaped left brace in regex is"
                                         " passed through");
                    }
                    goto normal_default;
                case '}':
                case ']':
                    if (p > RExC_parse && RExC_strict) {
                        ckWARN2reg(p + 1, "Unescaped literal '%c'", *p);
                    }
                    /*FALLTHROUGH*/
                default:    /* A literal character */
                  normal_default:
                    if (! UTF8_IS_INVARIANT(*p) && UTF) {
                        STRLEN numlen;
                        ender = utf8n_to_uvchr((U8*)p, RExC_end - p,
                                               &numlen, UTF8_ALLOW_DEFAULT);
                        p += numlen;
                    }
                    else
                        ender = (U8) *p++;
                    break;
                } /* End of switch on the literal */

                /* Here, have looked at the literal character, and <ender>
                 * contains its ordinal; <p> points to the character after it.
                 * */

                if (ender > 255) {
                    REQUIRE_UTF8(flagp);
                    if (   UNICODE_IS_PERL_EXTENDED(ender)
                        && TO_OUTPUT_WARNINGS(p))
                    {
                        ckWARN2_non_literal_string(p,
                                                   packWARN(WARN_PORTABLE),
                                                   PL_extended_cp_format,
                                                   ender);
                    }
                }

                /* We need to check if the next non-ignored thing is a
                 * quantifier.  Move <p> to after anything that should be
                 * ignored, which, as a side effect, positions <p> for the next
                 * loop iteration */
                skip_to_be_ignored_text(pRExC_state, &p,
                                        FALSE /* Don't force to /x */ );

                /* If the next thing is a quantifier, it applies to this
                 * character only, which means that this character has to be in
                 * its own node and can't just be appended to the string in an
                 * existing node, so if there are already other characters in
                 * the node, close the node with just them, and set up to do
                 * this character again next time through, when it will be the
                 * only thing in its new node */

                next_is_quantifier =    LIKELY(p < RExC_end)
                                     && UNLIKELY(isQUANTIFIER(p, RExC_end));

                if (next_is_quantifier && LIKELY(len)) {
                    p = oldp;
                    goto loopdone;
                }

                /* Ready to add 'ender' to the node */

                if (! FOLD) {  /* The simple case, just append the literal */
                  not_fold_common:

                    /* Don't output if it would overflow */
                    if (UNLIKELY(len > max_string_len - ((UTF)
                                                      ? UVCHR_SKIP(ender)
                                                      : 1)))
                    {
                        overflowed = TRUE;
                        break;
                    }

                    if (UVCHR_IS_INVARIANT(ender) || ! UTF) {
                        *(s++) = (char) ender;
                    }
                    else {
                        U8 * new_s = uvchr_to_utf8((U8*)s, ender);
                        added_len = (char *) new_s - s;
                        s = (char *) new_s;

                        if (ender > 255)  {
                            requires_utf8_target = TRUE;
                        }
                    }
                }
                else if (LOC && is_PROBLEMATIC_LOCALE_FOLD_cp(ender)) {

                    /* Here are folding under /l, and the code point is
                     * problematic.  If this is the first character in the
                     * node, change the node type to folding.   Otherwise, if
                     * this is the first problematic character, close up the
                     * existing node, so can start a new node with this one */
                    if (! len) {
                        node_type = EXACTFL;
                        RExC_contains_locale = 1;
                    }
                    else if (node_type == EXACT) {
                        p = oldp;
                        goto loopdone;
                    }

                    /* This problematic code point means we can't simplify
                     * things */
                    maybe_exactfu = FALSE;

                    /* Although these two characters have folds that are
                     * locale-problematic, they also have folds to above Latin1
                     * that aren't a problem.  Doing these now helps at
                     * runtime. */
                    if (UNLIKELY(   ender == GREEK_CAPITAL_LETTER_MU
                                 || ender == LATIN_CAPITAL_LETTER_SHARP_S))
                    {
                        goto fold_anyway;
                    }

                    /* Here, we are adding a problematic fold character.
                     * "Problematic" in this context means that its fold isn't
                     * known until runtime.  (The non-problematic code points
                     * are the above-Latin1 ones that fold to also all
                     * above-Latin1.  Their folds don't vary no matter what the
                     * locale is.) But here we have characters whose fold
                     * depends on the locale.  We just add in the unfolded
                     * character, and wait until runtime to fold it */
                    goto not_fold_common;
                }
                else /* regular fold; see if actually is in a fold */
                     if (   (ender < 256 && ! IS_IN_SOME_FOLD_L1(ender))
                         || (ender > 255
                            && ! _invlist_contains_cp(PL_in_some_fold, ender)))
                {
                    /* Here, folding, but the character isn't in a fold.
                     *
                     * Start a new node if previous characters in the node were
                     * folded */
                    if (len && node_type != EXACT) {
                        p = oldp;
                        goto loopdone;
                    }

                    /* Here, continuing a node with non-folded characters.  Add
                     * this one */
                    goto not_fold_common;
                }
                else {  /* Here, does participate in some fold */

                    /* If this is the first character in the node, change its
                     * type to folding.  Otherwise, if this is the first
                     * folding character in the node, close up the existing
                     * node, so can start a new node with this one.  */
                    if (! len) {
                        node_type = compute_EXACTish(pRExC_state);
                    }
                    else if (node_type == EXACT) {
                        p = oldp;
                        goto loopdone;
                    }

                    if (UTF) {  /* Alway use the folded value for UTF-8
                                   patterns */
                        if (UVCHR_IS_INVARIANT(ender)) {
                            if (UNLIKELY(len + 1 > max_string_len)) {
                                overflowed = TRUE;
                                break;
                            }

                            *(s)++ = (U8) toFOLD(ender);
                        }
                        else {
                            UV folded;

                          fold_anyway:
                            folded = _to_uni_fold_flags(
                                    ender,
                                    (U8 *) s,  /* We have allocated extra space
                                                  in 's' so can't run off the
                                                  end */
                                    &added_len,
                                    FOLD_FLAGS_FULL
                                  | ((   ASCII_FOLD_RESTRICTED
                                      || node_type == EXACTFL)
                                    ? FOLD_FLAGS_NOMIX_ASCII
                                    : 0));
                            if (UNLIKELY(len + added_len > max_string_len)) {
                                overflowed = TRUE;
                                break;
                            }

                            s += added_len;

                            if (   folded > 255
                                && LIKELY(folded != GREEK_SMALL_LETTER_MU))
                            {
                                /* U+B5 folds to the MU, so its possible for a
                                 * non-UTF-8 target to match it */
                                requires_utf8_target = TRUE;
                            }
                        }
                    }
                    else { /* Here is non-UTF8. */

                        /* The fold will be one or (rarely) two characters.
                         * Check that there's room for at least a single one
                         * before setting any flags, etc.  Because otherwise an
                         * overflowing character could cause a flag to be set
                         * even though it doesn't end up in this node.  (For
                         * the two character fold, we check again, before
                         * setting any flags) */
                        if (UNLIKELY(len + 1 > max_string_len)) {
                            overflowed = TRUE;
                            break;
                        }

#if    UNICODE_MAJOR_VERSION > 3 /* no multifolds in early Unicode */   \
   || (UNICODE_MAJOR_VERSION == 3 && (   UNICODE_DOT_VERSION > 0)       \
                                      || UNICODE_DOT_DOT_VERSION > 0)

                        /* On non-ancient Unicodes, check for the only possible
                         * multi-char fold  */
                        if (UNLIKELY(ender == LATIN_SMALL_LETTER_SHARP_S)) {

                            /* This potential multi-char fold means the node
                             * can't be simple (because it could match more
                             * than a single char).  And in some cases it will
                             * match 'ss', so set that flag */
                            maybe_SIMPLE = 0;
                            has_ss = TRUE;

                            /* It can't change to be an EXACTFU (unless already
                             * is one).  We fold it iff under /u rules. */
                            if (node_type != EXACTFU) {
                                maybe_exactfu = FALSE;
                            }
                            else {
                                if (UNLIKELY(len + 2 > max_string_len)) {
                                    overflowed = TRUE;
                                    break;
                                }

                                *(s++) = 's';
                                *(s++) = 's';
                                added_len = 2;

                                goto done_with_this_char;
                            }
                        }
                        else if (   UNLIKELY(isALPHA_FOLD_EQ(ender, 's'))
                                 && LIKELY(len > 0)
                                 && UNLIKELY(isALPHA_FOLD_EQ(*(s-1), 's')))
                        {
                            /* Also, the sequence 'ss' is special when not
                             * under /u.  If the target string is UTF-8, it
                             * should match SHARP S; otherwise it won't.  So,
                             * here we have to exclude the possibility of this
                             * node moving to /u.*/
                            has_ss = TRUE;
                            maybe_exactfu = FALSE;
                        }
#endif
                        /* Here, the fold will be a single character */

                        if (UNLIKELY(ender == MICRO_SIGN)) {
                            has_micro_sign = TRUE;
                        }
                        else if (PL_fold[ender] != PL_fold_latin1[ender]) {

                            /* If the character's fold differs between /d and
                             * /u, this can't change to be an EXACTFU node */
                            maybe_exactfu = FALSE;
                        }

                        *(s++) = (DEPENDS_SEMANTICS)
                                 ? (char) toFOLD(ender)

                                   /* Under /u, the fold of any character in
                                    * the 0-255 range happens to be its
                                    * lowercase equivalent, except for LATIN
                                    * SMALL LETTER SHARP S, which was handled
                                    * above, and the MICRO SIGN, whose fold
                                    * requires UTF-8 to represent.  */
                                 : (char) toLOWER_L1(ender);
                    }
                } /* End of adding current character to the node */

              done_with_this_char:

                len += added_len;

                if (next_is_quantifier) {

                    /* Here, the next input is a quantifier, and to get here,
                     * the current character is the only one in the node. */
                    goto loopdone;
                }

            } /* End of loop through literal characters */

            /* Here we have either exhausted the input or run out of room in
             * the node.  If the former, we are done.  (If we encountered a
             * character that can't be in the node, transfer is made directly
             * to <loopdone>, and so we wouldn't have fallen off the end of the
             * loop.)  */
            if (LIKELY(! overflowed)) {
                goto loopdone;
            }

            /* Here we have run out of room.  We can grow plain EXACT and
             * LEXACT nodes.  If the pattern is gigantic enough, though,
             * eventually we'll have to artificially chunk the pattern into
             * multiple nodes. */
            if (! LOC && (node_type == EXACT || node_type == LEXACT)) {
                Size_t overhead = 1 + REGNODE_ARG_LEN(OP(REGNODE_p(ret)));
                Size_t overhead_expansion = 0;
                char temp[256];
                Size_t max_nodes_for_string;
                Size_t achievable;
                SSize_t delta;

                /* Here we couldn't fit the final character in the current
                 * node, so it will have to be reparsed, no matter what else we
                 * do */
                p = oldp;

                /* If would have overflowed a regular EXACT node, switch
                 * instead to an LEXACT.  The code below is structured so that
                 * the actual growing code is common to changing from an EXACT
                 * or just increasing the LEXACT size.  This means that we have
                 * to save the string in the EXACT case before growing, and
                 * then copy it afterwards to its new location */
                if (node_type == EXACT) {
                    overhead_expansion = REGNODE_ARG_LEN(LEXACT) - REGNODE_ARG_LEN(EXACT);
                    RExC_emit += overhead_expansion;
                    Copy(s0, temp, len, char);
                }

                /* Ready to grow.  If it was a plain EXACT, the string was
                 * saved, and the first few bytes of it overwritten by adding
                 * an argument field.  We assume, as we do elsewhere in this
                 * file, that one byte of remaining input will translate into
                 * one byte of output, and if that's too small, we grow again,
                 * if too large the excess memory is freed at the end */

                max_nodes_for_string = U16_MAX - overhead - overhead_expansion;
                achievable = MIN(max_nodes_for_string,
                                 current_string_nodes + STR_SZ(RExC_end - p));
                delta = achievable - current_string_nodes;

                /* If there is just no more room, go finish up this chunk of
                 * the pattern. */
                if (delta <= 0) {
                    goto loopdone;
                }

                change_engine_size(pRExC_state, delta + overhead_expansion);
                current_string_nodes += delta;
                max_string_len
                           = sizeof(struct regnode) * current_string_nodes;
                upper_fill = max_string_len + 1;

                /* If the length was small, we know this was originally an
                 * EXACT node now converted to LEXACT, and the string has to be
                 * restored.  Otherwise the string was untouched.  260 is just
                 * a number safely above 255 so don't have to worry about
                 * getting it precise */
                if (len < 260) {
                    node_type = LEXACT;
                    FILL_NODE(ret, node_type);
                    s0 = STRING(REGNODE_p(ret));
                    Copy(temp, s0, len, char);
                    s = s0 + len;
                }

                goto continue_parse;
            }
            else if (FOLD) {
                bool splittable = FALSE;
                bool backed_up = FALSE;
                char * e;       /* should this be U8? */
                char * s_start; /* should this be U8? */

                /* Here is /i.  Running out of room creates a problem if we are
                 * folding, and the split happens in the middle of a
                 * multi-character fold, as a match that should have occurred,
                 * won't, due to the way nodes are matched, and our artificial
                 * boundary.  So back off until we aren't splitting such a
                 * fold.  If there is no such place to back off to, we end up
                 * taking the entire node as-is.  This can happen if the node
                 * consists entirely of 'f' or entirely of 's' characters (or
                 * things that fold to them) as 'ff' and 'ss' are
                 * multi-character folds.
                 *
                 * The Unicode standard says that multi character folds consist
                 * of either two or three characters.  That means we would be
                 * splitting one if the final character in the node is at the
                 * beginning of either type, or is the second of a three
                 * character fold.
                 *
                 * At this point:
                 *  ender     is the code point of the character that won't fit
                 *            in the node
                 *  s         points to just beyond the final byte in the node.
                 *            It's where we would place ender if there were
                 *            room, and where in fact we do place ender's fold
                 *            in the code below, as we've over-allocated space
                 *            for s0 (hence s) to allow for this
                 *  e         starts at 's' and advances as we append things.
                 *  old_s     is the same as 's'.  (If ender had fit, 's' would
                 *            have been advanced to beyond it).
                 *  old_old_s points to the beginning byte of the final
                 *            character in the node
                 *  p         points to the beginning byte in the input of the
                 *            character beyond 'ender'.
                 *  oldp      points to the beginning byte in the input of
                 *            'ender'.
                 *
                 * In the case of /il, we haven't folded anything that could be
                 * affected by the locale.  That means only above-Latin1
                 * characters that fold to other above-latin1 characters get
                 * folded at compile time.  To check where a good place to
                 * split nodes is, everything in it will have to be folded.
                 * The boolean 'maybe_exactfu' keeps track in /il if there are
                 * any unfolded characters in the node. */
                bool need_to_fold_loc = LOC && ! maybe_exactfu;

                /* If we do need to fold the node, we need a place to store the
                 * folded copy, and a way to map back to the unfolded original
                 * */
                char * locfold_buf = NULL;
                Size_t * loc_correspondence = NULL;

                if (! need_to_fold_loc) {   /* The normal case.  Just
                                               initialize to the actual node */
                    e = s;
                    s_start = s0;
                    s = old_old_s;  /* Point to the beginning of the final char
                                       that fits in the node */
                }
                else {

                    /* Here, we have filled a /il node, and there are unfolded
                     * characters in it.  If the runtime locale turns out to be
                     * UTF-8, there are possible multi-character folds, just
                     * like when not under /l.  The node hence can't terminate
                     * in the middle of such a fold.  To determine this, we
                     * have to create a folded copy of this node.  That means
                     * reparsing the node, folding everything assuming a UTF-8
                     * locale.  (If at runtime it isn't such a locale, the
                     * actions here wouldn't have been necessary, but we have
                     * to assume the worst case.)  If we find we need to back
                     * off the folded string, we do so, and then map that
                     * position back to the original unfolded node, which then
                     * gets output, truncated at that spot */

                    char * redo_p = RExC_parse;
                    char * redo_e;
                    char * old_redo_e;

                    /* Allow enough space assuming a single byte input folds to
                     * a single byte output, plus assume that the two unparsed
                     * characters (that we may need) fold to the largest number
                     * of bytes possible, plus extra for one more worst case
                     * scenario.  In the loop below, if we start eating into
                     * that final spare space, we enlarge this initial space */
                    Size_t size = max_string_len + (3 * UTF8_MAXBYTES_CASE) + 1;

                    Newxz(locfold_buf, size, char);
                    Newxz(loc_correspondence, size, Size_t);

                    /* Redo this node's parse, folding into 'locfold_buf' */
                    redo_p = RExC_parse;
                    old_redo_e = redo_e = locfold_buf;
                    while (redo_p <= oldp) {

                        old_redo_e = redo_e;
                        loc_correspondence[redo_e - locfold_buf]
                                                        = redo_p - RExC_parse;

                        if (UTF) {
                            Size_t added_len;

                            (void) _to_utf8_fold_flags((U8 *) redo_p,
                                                       (U8 *) RExC_end,
                                                       (U8 *) redo_e,
                                                       &added_len,
                                                       FOLD_FLAGS_FULL);
                            redo_e += added_len;
                            redo_p += UTF8SKIP(redo_p);
                        }
                        else {

                            /* Note that if this code is run on some ancient
                             * Unicode versions, SHARP S doesn't fold to 'ss',
                             * but rather than clutter the code with #ifdef's,
                             * as is done above, we ignore that possibility.
                             * This is ok because this code doesn't affect what
                             * gets matched, but merely where the node gets
                             * split */
                            if (UCHARAT(redo_p) != LATIN_SMALL_LETTER_SHARP_S) {
                                *redo_e++ = toLOWER_L1(UCHARAT(redo_p));
                            }
                            else {
                                *redo_e++ = 's';
                                *redo_e++ = 's';
                            }
                            redo_p++;
                        }


                        /* If we're getting so close to the end that a
                         * worst-case fold in the next character would cause us
                         * to overflow, increase, assuming one byte output byte
                         * per one byte input one, plus room for another worst
                         * case fold */
                        if (   redo_p <= oldp
                            && redo_e > locfold_buf + size
                                                    - (UTF8_MAXBYTES_CASE + 1))
                        {
                            Size_t new_size = size
                                            + (oldp - redo_p)
                                            + UTF8_MAXBYTES_CASE + 1;
                            Ptrdiff_t e_offset = redo_e - locfold_buf;

                            Renew(locfold_buf, new_size, char);
                            Renew(loc_correspondence, new_size, Size_t);
                            size = new_size;

                            redo_e = locfold_buf + e_offset;
                        }
                    }

                    /* Set so that things are in terms of the folded, temporary
                     * string */
                    s = old_redo_e;
                    s_start = locfold_buf;
                    e = redo_e;

                }

                /* Here, we have 's', 's_start' and 'e' set up to point to the
                 * input that goes into the node, folded.
                 *
                 * If the final character of the node and the fold of ender
                 * form the first two characters of a three character fold, we
                 * need to peek ahead at the next (unparsed) character in the
                 * input to determine if the three actually do form such a
                 * fold.  Just looking at that character is not generally
                 * sufficient, as it could be, for example, an escape sequence
                 * that evaluates to something else, and it needs to be folded.
                 *
                 * khw originally thought to just go through the parse loop one
                 * extra time, but that doesn't work easily as that iteration
                 * could cause things to think that the parse is over and to
                 * goto loopdone.  The character could be a '$' for example, or
                 * the character beyond could be a quantifier, and other
                 * glitches as well.
                 *
                 * The solution used here for peeking ahead is to look at that
                 * next character.  If it isn't ASCII punctuation, then it will
                 * be something that would continue on in an EXACTish node if
                 * there were space.  We append the fold of it to s, having
                 * reserved enough room in s0 for the purpose.  If we can't
                 * reasonably peek ahead, we instead assume the worst case:
                 * that it is something that would form the completion of a
                 * multi-char fold.
                 *
                 * If we can't split between s and ender, we work backwards
                 * character-by-character down to s0.  At each current point
                 * see if we are at the beginning of a multi-char fold.  If so,
                 * that means we would be splitting the fold across nodes, and
                 * so we back up one and try again.
                 *
                 * If we're not at the beginning, we still could be at the
                 * final two characters of a (rare) three character fold.  We
                 * check if the sequence starting at the character before the
                 * current position (and including the current and next
                 * characters) is a three character fold.  If not, the node can
                 * be split here.  If it is, we have to backup two characters
                 * and try again.
                 *
                 * Otherwise, the node can be split at the current position.
                 *
                 * The same logic is used for UTF-8 patterns and not */
                if (UTF) {
                    Size_t added_len;

                    /* Append the fold of ender */
                    (void) _to_uni_fold_flags(
                        ender,
                        (U8 *) e,
                        &added_len,
                        FOLD_FLAGS_FULL | ((ASCII_FOLD_RESTRICTED)
                                        ? FOLD_FLAGS_NOMIX_ASCII
                                        : 0));
                    e += added_len;

                    /* 's' and the character folded to by ender may be the
                     * first two of a three-character fold, in which case the
                     * node should not be split here.  That may mean examining
                     * the so-far unparsed character starting at 'p'.  But if
                     * ender folded to more than one character, we already have
                     * three characters to look at.  Also, we first check if
                     * the sequence consisting of s and the next character form
                     * the first two of some three character fold.  If not,
                     * there's no need to peek ahead. */
                    if (   added_len <= UTF8SKIP(e - added_len)
                        && UNLIKELY(is_THREE_CHAR_FOLD_HEAD_utf8_safe(s, e)))
                    {
                        /* Here, the two do form the beginning of a potential
                         * three character fold.  The unexamined character may
                         * or may not complete it.  Peek at it.  It might be
                         * something that ends the node or an escape sequence,
                         * in which case we don't know without a lot of work
                         * what it evaluates to, so we have to assume the worst
                         * case: that it does complete the fold, and so we
                         * can't split here.  All such instances  will have
                         * that character be an ASCII punctuation character,
                         * like a backslash.  So, for that case, backup one and
                         * drop down to try at that position */
                        if (isPUNCT(*p)) {
                            s = (char *) utf8_hop_back((U8 *) s, -1,
                                       (U8 *) s_start);
                            backed_up = TRUE;
                        }
                        else {
                            /* Here, since it's not punctuation, it must be a
                             * real character, and we can append its fold to
                             * 'e' (having deliberately reserved enough space
                             * for this eventuality) and drop down to check if
                             * the three actually do form a folded sequence */
                            (void) _to_utf8_fold_flags(
                                (U8 *) p, (U8 *) RExC_end,
                                (U8 *) e,
                                &added_len,
                                FOLD_FLAGS_FULL | ((ASCII_FOLD_RESTRICTED)
                                                ? FOLD_FLAGS_NOMIX_ASCII
                                                : 0));
                            e += added_len;
                        }
                    }

                    /* Here, we either have three characters available in
                     * sequence starting at 's', or we have two characters and
                     * know that the following one can't possibly be part of a
                     * three character fold.  We go through the node backwards
                     * until we find a place where we can split it without
                     * breaking apart a multi-character fold.  At any given
                     * point we have to worry about if such a fold begins at
                     * the current 's', and also if a three-character fold
                     * begins at s-1, (containing s and s+1).  Splitting in
                     * either case would break apart a fold */
                    do {
                        char *prev_s = (char *) utf8_hop_back((U8 *) s, -1,
                                                            (U8 *) s_start);

                        /* If is a multi-char fold, can't split here.  Backup
                         * one char and try again */
                        if (UNLIKELY(is_MULTI_CHAR_FOLD_utf8_safe(s, e))) {
                            s = prev_s;
                            backed_up = TRUE;
                            continue;
                        }

                        /* If the two characters beginning at 's' are part of a
                         * three character fold starting at the character
                         * before s, we can't split either before or after s.
                         * Backup two chars and try again */
                        if (   LIKELY(s > s_start)
                            && UNLIKELY(is_THREE_CHAR_FOLD_utf8_safe(prev_s, e)))
                        {
                            s = prev_s;
                            s = (char *) utf8_hop_back((U8 *) s, -1, (U8 *) s_start);
                            backed_up = TRUE;
                            continue;
                        }

                        /* Here there's no multi-char fold between s and the
                         * next character following it.  We can split */
                        splittable = TRUE;
                        break;

                    } while (s > s_start); /* End of loops backing up through the node */

                    /* Here we either couldn't find a place to split the node,
                     * or else we broke out of the loop setting 'splittable' to
                     * true.  In the latter case, the place to split is between
                     * the first and second characters in the sequence starting
                     * at 's' */
                    if (splittable) {
                        s += UTF8SKIP(s);
                    }
                }
                else {  /* Pattern not UTF-8 */
                    if (   ender != LATIN_SMALL_LETTER_SHARP_S
                        || ASCII_FOLD_RESTRICTED)
                    {
                        assert( toLOWER_L1(ender) < 256 );
                        *e++ = (char)(toLOWER_L1(ender)); /* should e and the cast be U8? */
                    }
                    else {
                        *e++ = 's';
                        *e++ = 's';
                    }

                    if (   e - s  <= 1
                        && UNLIKELY(is_THREE_CHAR_FOLD_HEAD_latin1_safe(s, e)))
                    {
                        if (isPUNCT(*p)) {
                            s--;
                            backed_up = TRUE;
                        }
                        else {
                            if (   UCHARAT(p) != LATIN_SMALL_LETTER_SHARP_S
                                || ASCII_FOLD_RESTRICTED)
                            {
                                assert( toLOWER_L1(ender) < 256 );
                                *e++ = (char)(toLOWER_L1(ender)); /* should e and the cast be U8? */
                            }
                            else {
                                *e++ = 's';
                                *e++ = 's';
                            }
                        }
                    }

                    do {
                        if (UNLIKELY(is_MULTI_CHAR_FOLD_latin1_safe(s, e))) {
                            s--;
                            backed_up = TRUE;
                            continue;
                        }

                        if (   LIKELY(s > s_start)
                            && UNLIKELY(is_THREE_CHAR_FOLD_latin1_safe(s - 1, e)))
                        {
                            s -= 2;
                            backed_up = TRUE;
                            continue;
                        }

                        splittable = TRUE;
                        break;

                    } while (s > s_start);

                    if (splittable) {
                        s++;
                    }
                }

                /* Here, we are done backing up.  If we didn't backup at all
                 * (the likely case), just proceed */
                if (backed_up) {

                   /* If we did find a place to split, reparse the entire node
                    * stopping where we have calculated. */
                    if (splittable) {

                       /* If we created a temporary folded string under /l, we
                        * have to map that back to the original */
                        if (need_to_fold_loc) {
                            upper_fill = loc_correspondence[s - s_start];
                            if (upper_fill == 0) {
                                FAIL2("panic: loc_correspondence[%d] is 0",
                                      (int) (s - s_start));
                            }
                            Safefree(locfold_buf);
                            Safefree(loc_correspondence);
                        }
                        else {
                            upper_fill = s - s0;
                        }
                        goto reparse;
                    }

                    /* Here the node consists entirely of non-final multi-char
                     * folds.  (Likely it is all 'f's or all 's's.)  There's no
                     * decent place to split it, so give up and just take the
                     * whole thing */
                    len = old_s - s0;
                }

                if (need_to_fold_loc) {
                    Safefree(locfold_buf);
                    Safefree(loc_correspondence);
                }
            }   /* End of verifying node ends with an appropriate char */

            /* We need to start the next node at the character that didn't fit
             * in this one */
            p = oldp;

          loopdone:   /* Jumped to when encounters something that shouldn't be
                         in the node */

            /* Free up any over-allocated space; cast is to silence bogus
             * warning in MS VC */
            change_engine_size(pRExC_state,
                        - (Ptrdiff_t) (current_string_nodes - STR_SZ(len)));

            /* I (khw) don't know if you can get here with zero length, but the
             * old code handled this situation by creating a zero-length EXACT
             * node.  Might as well be NOTHING instead */
            if (len == 0) {
                OP(REGNODE_p(ret)) = NOTHING;
            }
            else {

                /* If the node type is EXACT here, check to see if it
                 * should be EXACTL, or EXACT_REQ8. */
                if (node_type == EXACT) {
                    if (LOC) {
                        node_type = EXACTL;
                    }
                    else if (requires_utf8_target) {
                        node_type = EXACT_REQ8;
                    }
                }
                else if (node_type == LEXACT) {
                    if (requires_utf8_target) {
                        node_type = LEXACT_REQ8;
                    }
                }
                else if (FOLD) {
                    if (    UNLIKELY(has_micro_sign || has_ss)
                        && (node_type == EXACTFU || (   node_type == EXACTF
                                                     && maybe_exactfu)))
                    {   /* These two conditions are problematic in non-UTF-8
                           EXACTFU nodes. */
                        assert(! UTF);
                        node_type = EXACTFUP;
                    }
                    else if (node_type == EXACTFL) {

                        /* 'maybe_exactfu' is deliberately set above to
                         * indicate this node type, where all code points in it
                         * are above 255 */
                        if (maybe_exactfu) {
                            node_type = EXACTFLU8;
                        }
                        else if (UNLIKELY(
                             _invlist_contains_cp(PL_HasMultiCharFold, ender)))
                        {
                            /* A character that folds to more than one will
                             * match multiple characters, so can't be SIMPLE.
                             * We don't have to worry about this with EXACTFLU8
                             * nodes just above, as they have already been
                             * folded (since the fold doesn't vary at run
                             * time).  Here, if the final character in the node
                             * folds to multiple, it can't be simple.  (This
                             * only has an effect if the node has only a single
                             * character, hence the final one, as elsewhere we
                             * turn off simple for nodes whose length > 1 */
                            maybe_SIMPLE = 0;
                        }
                    }
                    else if (node_type == EXACTF) {  /* Means is /di */

                        /* This intermediate variable is needed solely because
                         * the asserts in the macro where used exceed Win32's
                         * literal string capacity */
                        char first_char = * STRING(REGNODE_p(ret));

                        /* If 'maybe_exactfu' is clear, then we need to stay
                         * /di.  If it is set, it means there are no code
                         * points that match differently depending on UTF8ness
                         * of the target string, so it can become an EXACTFU
                         * node */
                        if (! maybe_exactfu) {
                            RExC_seen_d_op = TRUE;
                        }
                        else if (   isALPHA_FOLD_EQ(first_char, 's')
                                 || isALPHA_FOLD_EQ(ender, 's'))
                        {
                            /* But, if the node begins or ends in an 's' we
                             * have to defer changing it into an EXACTFU, as
                             * the node could later get joined with another one
                             * that ends or begins with 's' creating an 'ss'
                             * sequence which would then wrongly match the
                             * sharp s without the target being UTF-8.  We
                             * create a special node that we resolve later when
                             * we join nodes together */

                            node_type = EXACTFU_S_EDGE;
                        }
                        else {
                            node_type = EXACTFU;
                        }
                    }

                    if (requires_utf8_target && node_type == EXACTFU) {
                        node_type = EXACTFU_REQ8;
                    }
                }

                OP(REGNODE_p(ret)) = node_type;
                setSTR_LEN(REGNODE_p(ret), len);
                RExC_emit += STR_SZ(len);

                /* If the node isn't a single character, it can't be SIMPLE */
                if (len > (Size_t) ((UTF) ? UTF8SKIP(STRING(REGNODE_p(ret))) : 1)) {
                    maybe_SIMPLE = 0;
                }

                *flagp |= HASWIDTH | maybe_SIMPLE;
            }

            RExC_parse_set(p);

            {
                /* len is STRLEN which is unsigned, need to copy to signed */
                IV iv = len;
                if (iv < 0)
                    vFAIL("Internal disaster");
            }

        } /* End of label 'defchar:' */
        break;
    } /* End of giant switch on input character */

    /* Position parse to next real character */
    skip_to_be_ignored_text(pRExC_state, &RExC_parse,
                                            FALSE /* Don't force to /x */ );
    if (   *RExC_parse == '{'
        && OP(REGNODE_p(ret)) != SBOL && ! regcurly(RExC_parse, RExC_end, NULL))
    {
        if (RExC_strict) {
            RExC_parse_inc_by(1);
            vFAIL("Unescaped left brace in regex is illegal here");
        }
        ckWARNreg(RExC_parse + 1, "Unescaped left brace in regex is"
                                  " passed through");
    }

    return(ret);
}


void
Perl_populate_anyof_bitmap_from_invlist(pTHX_ regnode *node, SV** invlist_ptr)
{
    /* Uses the inversion list '*invlist_ptr' to populate the ANYOF 'node'.  It
     * sets up the bitmap and any flags, removing those code points from the
     * inversion list, setting it to NULL should it become completely empty */


    PERL_ARGS_ASSERT_POPULATE_ANYOF_BITMAP_FROM_INVLIST;

    /* There is no bitmap for this node type */
    if (REGNODE_TYPE(OP(node))  != ANYOF) {
        return;
    }

    ANYOF_BITMAP_ZERO(node);
    if (*invlist_ptr) {

        /* This gets set if we actually need to modify things */
        bool change_invlist = FALSE;

        UV start, end;

        /* Start looking through *invlist_ptr */
        invlist_iterinit(*invlist_ptr);
        while (invlist_iternext(*invlist_ptr, &start, &end)) {
            UV high;
            int i;

            /* Quit if are above what we should change */
            if (start >= NUM_ANYOF_CODE_POINTS) {
                break;
            }

            change_invlist = TRUE;

            /* Set all the bits in the range, up to the max that we are doing */
            high = (end < NUM_ANYOF_CODE_POINTS - 1)
                   ? end
                   : NUM_ANYOF_CODE_POINTS - 1;
            for (i = start; i <= (int) high; i++) {
                ANYOF_BITMAP_SET(node, i);
            }
        }
        invlist_iterfinish(*invlist_ptr);

        /* Done with loop; remove any code points that are in the bitmap from
         * *invlist_ptr */
        if (change_invlist) {
            _invlist_subtract(*invlist_ptr, PL_InBitmap, invlist_ptr);
        }

        /* If have completely emptied it, remove it completely */
        if (_invlist_len(*invlist_ptr) == 0) {
            SvREFCNT_dec_NN(*invlist_ptr);
            *invlist_ptr = NULL;
        }
    }
}

/* Parse POSIX character classes: [[:foo:]], [[=foo=]], [[.foo.]].
   Character classes ([:foo:]) can also be negated ([:^foo:]).
   Returns a named class id (ANYOF_XXX) if successful, -1 otherwise.
   Equivalence classes ([=foo=]) and composites ([.foo.]) are parsed,
   but trigger failures because they are currently unimplemented. */

#define POSIXCC_DONE(c)   ((c) == ':')
#define POSIXCC_NOTYET(c) ((c) == '=' || (c) == '.')
#define POSIXCC(c) (POSIXCC_DONE(c) || POSIXCC_NOTYET(c))
#define MAYBE_POSIXCC(c) (POSIXCC(c) || (c) == '^' || (c) == ';')

#define WARNING_PREFIX              "Assuming NOT a POSIX class since "
#define NO_BLANKS_POSIX_WARNING     "no blanks are allowed in one"
#define SEMI_COLON_POSIX_WARNING    "a semi-colon was found instead of a colon"

#define NOT_MEANT_TO_BE_A_POSIX_CLASS (OOB_NAMEDCLASS - 1)

/* 'posix_warnings' and 'warn_text' are names of variables in the following
 * routine. q.v. */
#define ADD_POSIX_WARNING(p, text)  STMT_START {                            \
        if (posix_warnings) {                                               \
            if (! RExC_warn_text ) RExC_warn_text =                         \
                                         (AV *) sv_2mortal((SV *) newAV()); \
            av_push_simple(RExC_warn_text, Perl_newSVpvf(aTHX_                     \
                                             WARNING_PREFIX                 \
                                             text                           \
                                             REPORT_LOCATION,               \
                                             REPORT_LOCATION_ARGS(p)));     \
        }                                                                   \
    } STMT_END
#define CLEAR_POSIX_WARNINGS()                                              \
    STMT_START {                                                            \
        if (posix_warnings && RExC_warn_text)                               \
            av_clear(RExC_warn_text);                                       \
    } STMT_END

#define CLEAR_POSIX_WARNINGS_AND_RETURN(ret)                                \
    STMT_START {                                                            \
        CLEAR_POSIX_WARNINGS();                                             \
        return ret;                                                         \
    } STMT_END

STATIC int
S_handle_possible_posix(pTHX_ RExC_state_t *pRExC_state,

    const char * const s,      /* Where the putative posix class begins.
                                  Normally, this is one past the '['.  This
                                  parameter exists so it can be somewhere
                                  besides RExC_parse. */
    char ** updated_parse_ptr, /* Where to set the updated parse pointer, or
                                  NULL */
    AV ** posix_warnings,      /* Where to place any generated warnings, or
                                  NULL */
    const bool check_only      /* Don't die if error */
)
{
    /* This parses what the caller thinks may be one of the three POSIX
     * constructs:
     *  1) a character class, like [:blank:]
     *  2) a collating symbol, like [. .]
     *  3) an equivalence class, like [= =]
     * In the latter two cases, it croaks if it finds a syntactically legal
     * one, as these are not handled by Perl.
     *
     * The main purpose is to look for a POSIX character class.  It returns:
     *  a) the class number
     *      if it is a completely syntactically and semantically legal class.
     *      'updated_parse_ptr', if not NULL, is set to point to just after the
     *      closing ']' of the class
     *  b) OOB_NAMEDCLASS
     *      if it appears that one of the three POSIX constructs was meant, but
     *      its specification was somehow defective.  'updated_parse_ptr', if
     *      not NULL, is set to point to the character just after the end
     *      character of the class.  See below for handling of warnings.
     *  c) NOT_MEANT_TO_BE_A_POSIX_CLASS
     *      if it  doesn't appear that a POSIX construct was intended.
     *      'updated_parse_ptr' is not changed.  No warnings nor errors are
     *      raised.
     *
     * In b) there may be errors or warnings generated.  If 'check_only' is
     * TRUE, then any errors are discarded.  Warnings are returned to the
     * caller via an AV* created into '*posix_warnings' if it is not NULL.  If
     * instead it is NULL, warnings are suppressed.
     *
     * The reason for this function, and its complexity is that a bracketed
     * character class can contain just about anything.  But it's easy to
     * mistype the very specific posix class syntax but yielding a valid
     * regular bracketed class, so it silently gets compiled into something
     * quite unintended.
     *
     * The solution adopted here maintains backward compatibility except that
     * it adds a warning if it looks like a posix class was intended but
     * improperly specified.  The warning is not raised unless what is input
     * very closely resembles one of the 14 legal posix classes.  To do this,
     * it uses fuzzy parsing.  It calculates how many single-character edits it
     * would take to transform what was input into a legal posix class.  Only
     * if that number is quite small does it think that the intention was a
     * posix class.  Obviously these are heuristics, and there will be cases
     * where it errs on one side or another, and they can be tweaked as
     * experience informs.
     *
     * The syntax for a legal posix class is:
     *
     * qr/(?xa: \[ : \^? [[:lower:]]{4,6} : \] )/
     *
     * What this routine considers syntactically to be an intended posix class
     * is this (the comments indicate some restrictions that the pattern
     * doesn't show):
     *
     *  qr/(?x: \[?                         # The left bracket, possibly
     *                                      # omitted
     *          \h*                         # possibly followed by blanks
     *          (?: \^ \h* )?               # possibly a misplaced caret
     *          [:;]?                       # The opening class character,
     *                                      # possibly omitted.  A typo
     *                                      # semi-colon can also be used.
     *          \h*
     *          \^?                         # possibly a correctly placed
     *                                      # caret, but not if there was also
     *                                      # a misplaced one
     *          \h*
     *          .{3,15}                     # The class name.  If there are
     *                                      # deviations from the legal syntax,
     *                                      # its edit distance must be close
     *                                      # to a real class name in order
     *                                      # for it to be considered to be
     *                                      # an intended posix class.
     *          \h*
     *          [[:punct:]]?                # The closing class character,
     *                                      # possibly omitted.  If not a colon
     *                                      # nor semi colon, the class name
     *                                      # must be even closer to a valid
     *                                      # one
     *          \h*
     *          \]?                         # The right bracket, possibly
     *                                      # omitted.
     *     )/
     *
     * In the above, \h must be ASCII-only.
     *
     * These are heuristics, and can be tweaked as field experience dictates.
     * There will be cases when someone didn't intend to specify a posix class
     * that this warns as being so.  The goal is to minimize these, while
     * maximizing the catching of things intended to be a posix class that
     * aren't parsed as such.
     */

    const char* p             = s;
    const char * const e      = RExC_end;
    unsigned complement       = 0;      /* If to complement the class */
    bool found_problem        = FALSE;  /* Assume OK until proven otherwise */
    bool has_opening_bracket  = FALSE;
    bool has_opening_colon    = FALSE;
    int class_number          = OOB_NAMEDCLASS; /* Out-of-bounds until find
                                                   valid class */
    const char * possible_end = NULL;   /* used for a 2nd parse pass */
    const char* name_start;             /* ptr to class name first char */

    /* If the number of single-character typos the input name is away from a
     * legal name is no more than this number, it is considered to have meant
     * the legal name */
    int max_distance          = 2;

    /* to store the name.  The size determines the maximum length before we
     * decide that no posix class was intended.  Should be at least
     * sizeof("alphanumeric") */
    UV input_text[15];
    STATIC_ASSERT_DECL(C_ARRAY_LENGTH(input_text) >= sizeof "alphanumeric");

    PERL_ARGS_ASSERT_HANDLE_POSSIBLE_POSIX;

    CLEAR_POSIX_WARNINGS();

    if (p >= e) {
        return NOT_MEANT_TO_BE_A_POSIX_CLASS;
    }

    if (*(p - 1) != '[') {
        ADD_POSIX_WARNING(p, "it doesn't start with a '['");
        found_problem = TRUE;
    }
    else {
        has_opening_bracket = TRUE;
    }

    /* They could be confused and think you can put spaces between the
     * components */
    if (isBLANK(*p)) {
        found_problem = TRUE;

        do {
            p++;
        } while (p < e && isBLANK(*p));

        ADD_POSIX_WARNING(p, NO_BLANKS_POSIX_WARNING);
    }

    /* For [. .] and [= =].  These are quite different internally from [: :],
     * so they are handled separately.  */
    if (POSIXCC_NOTYET(*p) && p < e - 3) /* 1 for the close, and 1 for the ']'
                                            and 1 for at least one char in it
                                          */
    {
        const char open_char  = *p;
        const char * temp_ptr = p + 1;

        /* These two constructs are not handled by perl, and if we find a
         * syntactically valid one, we croak.  khw, who wrote this code, finds
         * this explanation of them very unclear:
         * http://pubs.opengroup.org/onlinepubs/009696899/basedefs/xbd_chap09.html
         * And searching the rest of the internet wasn't very helpful either.
         * It looks like just about any byte can be in these constructs,
         * depending on the locale.  But unless the pattern is being compiled
         * under /l, which is very rare, Perl runs under the C or POSIX locale.
         * In that case, it looks like [= =] isn't allowed at all, and that
         * [. .] could be any single code point, but for longer strings the
         * constituent characters would have to be the ASCII alphabetics plus
         * the minus-hyphen.  Any sensible locale definition would limit itself
         * to these.  And any portable one definitely should.  Trying to parse
         * the general case is a nightmare (see [perl #127604]).  So, this code
         * looks only for interiors of these constructs that match:
         *      qr/.|[-\w]{2,}/
         * Using \w relaxes the apparent rules a little, without adding much
         * danger of mistaking something else for one of these constructs.
         *
         * [. .] in some implementations described on the internet is usable to
         * escape a character that otherwise is special in bracketed character
         * classes.  For example [.].] means a literal right bracket instead of
         * the ending of the class
         *
         * [= =] can legitimately contain a [. .] construct, but we don't
         * handle this case, as that [. .] construct will later get parsed
         * itself and croak then.  And [= =] is checked for even when not under
         * /l, as Perl has long done so.
         *
         * The code below relies on there being a trailing NUL, so it doesn't
         * have to keep checking if the parse ptr < e.
         */
        if (temp_ptr[1] == open_char) {
            temp_ptr++;
        }
        else while (    temp_ptr < e
                    && (isWORDCHAR(*temp_ptr) || *temp_ptr == '-'))
        {
            temp_ptr++;
        }

        if (*temp_ptr == open_char) {
            temp_ptr++;
            if (*temp_ptr == ']') {
                temp_ptr++;
                if (! found_problem && ! check_only) {
                    RExC_parse_set((char *) temp_ptr);
                    vFAIL3("POSIX syntax [%c %c] is reserved for future "
                            "extensions", open_char, open_char);
                }

                /* Here, the syntax wasn't completely valid, or else the call
                 * is to check-only */
                if (updated_parse_ptr) {
                    *updated_parse_ptr = (char *) temp_ptr;
                }

                CLEAR_POSIX_WARNINGS_AND_RETURN(OOB_NAMEDCLASS);
            }
        }

        /* If we find something that started out to look like one of these
         * constructs, but isn't, we continue below so that it can be checked
         * for being a class name with a typo of '.' or '=' instead of a colon.
         * */
    }

    /* Here, we think there is a possibility that a [: :] class was meant, and
     * we have the first real character.  It could be they think the '^' comes
     * first */
    if (*p == '^') {
        found_problem = TRUE;
        ADD_POSIX_WARNING(p + 1, "the '^' must come after the colon");
        complement = 1;
        p++;

        if (isBLANK(*p)) {
            found_problem = TRUE;

            do {
                p++;
            } while (p < e && isBLANK(*p));

            ADD_POSIX_WARNING(p, NO_BLANKS_POSIX_WARNING);
        }
    }

    /* But the first character should be a colon, which they could have easily
     * mistyped on a qwerty keyboard as a semi-colon (and which may be hard to
     * distinguish from a colon, so treat that as a colon).  */
    if (*p == ':') {
        p++;
        has_opening_colon = TRUE;
    }
    else if (*p == ';') {
        found_problem = TRUE;
        p++;
        ADD_POSIX_WARNING(p, SEMI_COLON_POSIX_WARNING);
        has_opening_colon = TRUE;
    }
    else {
        found_problem = TRUE;
        ADD_POSIX_WARNING(p, "there must be a starting ':'");

        /* Consider an initial punctuation (not one of the recognized ones) to
         * be a left terminator */
        if (*p != '^' && *p != ']' && isPUNCT(*p)) {
            p++;
        }
    }

    /* They may think that you can put spaces between the components */
    if (isBLANK(*p)) {
        found_problem = TRUE;

        do {
            p++;
        } while (p < e && isBLANK(*p));

        ADD_POSIX_WARNING(p, NO_BLANKS_POSIX_WARNING);
    }

    if (*p == '^') {

        /* We consider something like [^:^alnum:]] to not have been intended to
         * be a posix class, but XXX maybe we should */
        if (complement) {
            CLEAR_POSIX_WARNINGS_AND_RETURN(NOT_MEANT_TO_BE_A_POSIX_CLASS);
        }

        complement = 1;
        p++;
    }

    /* Again, they may think that you can put spaces between the components */
    if (isBLANK(*p)) {
        found_problem = TRUE;

        do {
            p++;
        } while (p < e && isBLANK(*p));

        ADD_POSIX_WARNING(p, NO_BLANKS_POSIX_WARNING);
    }

    if (*p == ']') {

        /* XXX This ']' may be a typo, and something else was meant.  But
         * treating it as such creates enough complications, that that
         * possibility isn't currently considered here.  So we assume that the
         * ']' is what is intended, and if we've already found an initial '[',
         * this leaves this construct looking like [:] or [:^], which almost
         * certainly weren't intended to be posix classes */
        if (has_opening_bracket) {
            CLEAR_POSIX_WARNINGS_AND_RETURN(NOT_MEANT_TO_BE_A_POSIX_CLASS);
        }

        /* But this function can be called when we parse the colon for
         * something like qr/[alpha:]]/, so we back up to look for the
         * beginning */
        p--;

        if (*p == ';') {
            found_problem = TRUE;
            ADD_POSIX_WARNING(p, SEMI_COLON_POSIX_WARNING);
        }
        else if (*p != ':') {

            /* XXX We are currently very restrictive here, so this code doesn't
             * consider the possibility that, say, /[alpha.]]/ was intended to
             * be a posix class. */
            CLEAR_POSIX_WARNINGS_AND_RETURN(NOT_MEANT_TO_BE_A_POSIX_CLASS);
        }

        /* Here we have something like 'foo:]'.  There was no initial colon,
         * and we back up over 'foo.  XXX Unlike the going forward case, we
         * don't handle typos of non-word chars in the middle */
        has_opening_colon = FALSE;
        p--;

        while (p > RExC_start && isWORDCHAR(*p)) {
            p--;
        }
        p++;

        /* Here, we have positioned ourselves to where we think the first
         * character in the potential class is */
    }

    /* Now the interior really starts.  There are certain key characters that
     * can end the interior, or these could just be typos.  To catch both
     * cases, we may have to do two passes.  In the first pass, we keep on
     * going unless we come to a sequence that matches
     *      qr/ [[:punct:]] [[:blank:]]* \] /xa
     * This means it takes a sequence to end the pass, so two typos in a row if
     * that wasn't what was intended.  If the class is perfectly formed, just
     * this one pass is needed.  We also stop if there are too many characters
     * being accumulated, but this number is deliberately set higher than any
     * real class.  It is set high enough so that someone who thinks that
     * 'alphanumeric' is a correct name would get warned that it wasn't.
     * While doing the pass, we keep track of where the key characters were in
     * it.  If we don't find an end to the class, and one of the key characters
     * was found, we redo the pass, but stop when we get to that character.
     * Thus the key character was considered a typo in the first pass, but a
     * terminator in the second.  If two key characters are found, we stop at
     * the second one in the first pass.  Again this can miss two typos, but
     * catches a single one
     *
     * In the first pass, 'possible_end' starts as NULL, and then gets set to
     * point to the first key character.  For the second pass, it starts as -1.
     * */

    name_start = p;
  parse_name:
    {
        bool has_blank               = FALSE;
        bool has_upper               = FALSE;
        bool has_terminating_colon   = FALSE;
        bool has_terminating_bracket = FALSE;
        bool has_semi_colon          = FALSE;
        unsigned int name_len        = 0;
        int punct_count              = 0;

        while (p < e) {

            /* Squeeze out blanks when looking up the class name below */
            if (isBLANK(*p) ) {
                has_blank = TRUE;
                found_problem = TRUE;
                p++;
                continue;
            }

            /* The name will end with a punctuation */
            if (isPUNCT(*p)) {
                const char * peek = p + 1;

                /* Treat any non-']' punctuation followed by a ']' (possibly
                 * with intervening blanks) as trying to terminate the class.
                 * ']]' is very likely to mean a class was intended (but
                 * missing the colon), but the warning message that gets
                 * generated shows the error position better if we exit the
                 * loop at the bottom (eventually), so skip it here. */
                if (*p != ']') {
                    if (peek < e && isBLANK(*peek)) {
                        has_blank = TRUE;
                        found_problem = TRUE;
                        do {
                            peek++;
                        } while (peek < e && isBLANK(*peek));
                    }

                    if (peek < e && *peek == ']') {
                        has_terminating_bracket = TRUE;
                        if (*p == ':') {
                            has_terminating_colon = TRUE;
                        }
                        else if (*p == ';') {
                            has_semi_colon = TRUE;
                            has_terminating_colon = TRUE;
                        }
                        else {
                            found_problem = TRUE;
                        }
                        p = peek + 1;
                        goto try_posix;
                    }
                }

                /* Here we have punctuation we thought didn't end the class.
                 * Keep track of the position of the key characters that are
                 * more likely to have been class-enders */
                if (*p == ']' || *p == '[' || *p == ':' || *p == ';') {

                    /* Allow just one such possible class-ender not actually
                     * ending the class. */
                    if (possible_end) {
                        break;
                    }
                    possible_end = p;
                }

                /* If we have too many punctuation characters, no use in
                 * keeping going */
                if (++punct_count > max_distance) {
                    break;
                }

                /* Treat the punctuation as a typo. */
                input_text[name_len++] = *p;
                p++;
            }
            else if (isUPPER(*p)) { /* Use lowercase for lookup */
                input_text[name_len++] = toLOWER(*p);
                has_upper = TRUE;
                found_problem = TRUE;
                p++;
            } else if (! UTF || UTF8_IS_INVARIANT(*p)) {
                input_text[name_len++] = *p;
                p++;
            }
            else {
                input_text[name_len++] = utf8_to_uvchr_buf((U8 *) p, e, NULL);
                p+= UTF8SKIP(p);
            }

            /* The declaration of 'input_text' is how long we allow a potential
             * class name to be, before saying they didn't mean a class name at
             * all */
            if (name_len >= C_ARRAY_LENGTH(input_text)) {
                break;
            }
        }

        /* We get to here when the possible class name hasn't been properly
         * terminated before:
         *   1) we ran off the end of the pattern; or
         *   2) found two characters, each of which might have been intended to
         *      be the name's terminator
         *   3) found so many punctuation characters in the purported name,
         *      that the edit distance to a valid one is exceeded
         *   4) we decided it was more characters than anyone could have
         *      intended to be one. */

        found_problem = TRUE;

        /* In the final two cases, we know that looking up what we've
         * accumulated won't lead to a match, even a fuzzy one. */
        if (   name_len >= C_ARRAY_LENGTH(input_text)
            || punct_count > max_distance)
        {
            /* If there was an intermediate key character that could have been
             * an intended end, redo the parse, but stop there */
            if (possible_end && possible_end != (char *) -1) {
                possible_end = (char *) -1; /* Special signal value to say
                                               we've done a first pass */
                p = name_start;
                goto parse_name;
            }

            /* Otherwise, it can't have meant to have been a class */
            CLEAR_POSIX_WARNINGS_AND_RETURN(NOT_MEANT_TO_BE_A_POSIX_CLASS);
        }

        /* If we ran off the end, and the final character was a punctuation
         * one, back up one, to look at that final one just below.  Later, we
         * will restore the parse pointer if appropriate */
        if (name_len && p == e && isPUNCT(*(p-1))) {
            p--;
            name_len--;
        }

        if (p < e && isPUNCT(*p)) {
            if (*p == ']') {
                has_terminating_bracket = TRUE;

                /* If this is a 2nd ']', and the first one is just below this
                 * one, consider that to be the real terminator.  This gives a
                 * uniform and better positioning for the warning message  */
                if (   possible_end
                    && possible_end != (char *) -1
                    && *possible_end == ']'
                    && name_len && input_text[name_len - 1] == ']')
                {
                    name_len--;
                    p = possible_end;

                    /* And this is actually equivalent to having done the 2nd
                     * pass now, so set it to not try again */
                    possible_end = (char *) -1;
                }
            }
            else {
                if (*p == ':') {
                    has_terminating_colon = TRUE;
                }
                else if (*p == ';') {
                    has_semi_colon = TRUE;
                    has_terminating_colon = TRUE;
                }
                p++;
            }
        }

    try_posix:

        /* Here, we have a class name to look up.  We can short circuit the
         * stuff below for short names that can't possibly be meant to be a
         * class name.  (We can do this on the first pass, as any second pass
         * will yield an even shorter name) */
        if (name_len < 3) {
            CLEAR_POSIX_WARNINGS_AND_RETURN(NOT_MEANT_TO_BE_A_POSIX_CLASS);
        }

        /* Find which class it is.  Initially switch on the length of the name.
         * */
        switch (name_len) {
            case 4:
                if (memEQs(name_start, 4, "word")) {
                    /* this is not POSIX, this is the Perl \w */
                    class_number = ANYOF_WORDCHAR;
                }
                break;
            case 5:
                /* Names all of length 5: alnum alpha ascii blank cntrl digit
                 *                        graph lower print punct space upper
                 * Offset 4 gives the best switch position.  */
                switch (name_start[4]) {
                    case 'a':
                        if (memBEGINs(name_start, 5, "alph")) /* alpha */
                            class_number = ANYOF_ALPHA;
                        break;
                    case 'e':
                        if (memBEGINs(name_start, 5, "spac")) /* space */
                            class_number = ANYOF_SPACE;
                        break;
                    case 'h':
                        if (memBEGINs(name_start, 5, "grap")) /* graph */
                            class_number = ANYOF_GRAPH;
                        break;
                    case 'i':
                        if (memBEGINs(name_start, 5, "asci")) /* ascii */
                            class_number = ANYOF_ASCII;
                        break;
                    case 'k':
                        if (memBEGINs(name_start, 5, "blan")) /* blank */
                            class_number = ANYOF_BLANK;
                        break;
                    case 'l':
                        if (memBEGINs(name_start, 5, "cntr")) /* cntrl */
                            class_number = ANYOF_CNTRL;
                        break;
                    case 'm':
                        if (memBEGINs(name_start, 5, "alnu")) /* alnum */
                            class_number = ANYOF_ALPHANUMERIC;
                        break;
                    case 'r':
                        if (memBEGINs(name_start, 5, "lowe")) /* lower */
                            class_number = (FOLD) ? ANYOF_CASED : ANYOF_LOWER;
                        else if (memBEGINs(name_start, 5, "uppe")) /* upper */
                            class_number = (FOLD) ? ANYOF_CASED : ANYOF_UPPER;
                        break;
                    case 't':
                        if (memBEGINs(name_start, 5, "digi")) /* digit */
                            class_number = ANYOF_DIGIT;
                        else if (memBEGINs(name_start, 5, "prin")) /* print */
                            class_number = ANYOF_PRINT;
                        else if (memBEGINs(name_start, 5, "punc")) /* punct */
                            class_number = ANYOF_PUNCT;
                        break;
                }
                break;
            case 6:
                if (memEQs(name_start, 6, "xdigit"))
                    class_number = ANYOF_XDIGIT;
                break;
        }

        /* If the name exactly matches a posix class name the class number will
         * here be set to it, and the input almost certainly was meant to be a
         * posix class, so we can skip further checking.  If instead the syntax
         * is exactly correct, but the name isn't one of the legal ones, we
         * will return that as an error below.  But if neither of these apply,
         * it could be that no posix class was intended at all, or that one
         * was, but there was a typo.  We tease these apart by doing fuzzy
         * matching on the name */
        if (class_number == OOB_NAMEDCLASS && found_problem) {
            const UV posix_names[][6] = {
                                                { 'a', 'l', 'n', 'u', 'm' },
                                                { 'a', 'l', 'p', 'h', 'a' },
                                                { 'a', 's', 'c', 'i', 'i' },
                                                { 'b', 'l', 'a', 'n', 'k' },
                                                { 'c', 'n', 't', 'r', 'l' },
                                                { 'd', 'i', 'g', 'i', 't' },
                                                { 'g', 'r', 'a', 'p', 'h' },
                                                { 'l', 'o', 'w', 'e', 'r' },
                                                { 'p', 'r', 'i', 'n', 't' },
                                                { 'p', 'u', 'n', 'c', 't' },
                                                { 's', 'p', 'a', 'c', 'e' },
                                                { 'u', 'p', 'p', 'e', 'r' },
                                                { 'w', 'o', 'r', 'd' },
                                                { 'x', 'd', 'i', 'g', 'i', 't' }
                                            };
            /* The names of the above all have added NULs to make them the same
             * size, so we need to also have the real lengths */
            const UV posix_name_lengths[] = {
                                                sizeof("alnum") - 1,
                                                sizeof("alpha") - 1,
                                                sizeof("ascii") - 1,
                                                sizeof("blank") - 1,
                                                sizeof("cntrl") - 1,
                                                sizeof("digit") - 1,
                                                sizeof("graph") - 1,
                                                sizeof("lower") - 1,
                                                sizeof("print") - 1,
                                                sizeof("punct") - 1,
                                                sizeof("space") - 1,
                                                sizeof("upper") - 1,
                                                sizeof("word")  - 1,
                                                sizeof("xdigit")- 1
                                            };
            unsigned int i;
            int temp_max = max_distance;    /* Use a temporary, so if we
                                               reparse, we haven't changed the
                                               outer one */

            /* Use a smaller max edit distance if we are missing one of the
             * delimiters */
            if (   has_opening_bracket + has_opening_colon < 2
                || has_terminating_bracket + has_terminating_colon < 2)
            {
                temp_max--;
            }

            /* See if the input name is close to a legal one */
            for (i = 0; i < C_ARRAY_LENGTH(posix_names); i++) {

                /* Short circuit call if the lengths are too far apart to be
                 * able to match */
                if (abs( (int) (name_len - posix_name_lengths[i]))
                    > temp_max)
                {
                    continue;
                }

                if (edit_distance(input_text,
                                  posix_names[i],
                                  name_len,
                                  posix_name_lengths[i],
                                  temp_max
                                 )
                    > -1)
                { /* If it is close, it probably was intended to be a class */
                    goto probably_meant_to_be;
                }
            }

            /* Here the input name is not close enough to a valid class name
             * for us to consider it to be intended to be a posix class.  If
             * we haven't already done so, and the parse found a character that
             * could have been terminators for the name, but which we absorbed
             * as typos during the first pass, repeat the parse, signalling it
             * to stop at that character */
            if (possible_end && possible_end != (char *) -1) {
                possible_end = (char *) -1;
                p = name_start;
                goto parse_name;
            }

            /* Here neither pass found a close-enough class name */
            CLEAR_POSIX_WARNINGS_AND_RETURN(NOT_MEANT_TO_BE_A_POSIX_CLASS);
        }

    probably_meant_to_be:

        /* Here we think that a posix specification was intended.  Update any
         * parse pointer */
        if (updated_parse_ptr) {
            *updated_parse_ptr = (char *) p;
        }

        /* If a posix class name was intended but incorrectly specified, we
         * output or return the warnings */
        if (found_problem) {

            /* We set flags for these issues in the parse loop above instead of
             * adding them to the list of warnings, because we can parse it
             * twice, and we only want one warning instance */
            if (has_upper) {
                ADD_POSIX_WARNING(p, "the name must be all lowercase letters");
            }
            if (has_blank) {
                ADD_POSIX_WARNING(p, NO_BLANKS_POSIX_WARNING);
            }
            if (has_semi_colon) {
                ADD_POSIX_WARNING(p, SEMI_COLON_POSIX_WARNING);
            }
            else if (! has_terminating_colon) {
                ADD_POSIX_WARNING(p, "there is no terminating ':'");
            }
            if (! has_terminating_bracket) {
                ADD_POSIX_WARNING(p, "there is no terminating ']'");
            }

            if (   posix_warnings
                && RExC_warn_text
                && av_count(RExC_warn_text) > 0)
            {
                *posix_warnings = RExC_warn_text;
            }
        }
        else if (class_number != OOB_NAMEDCLASS) {
            /* If it is a known class, return the class.  The class number
             * #defines are structured so each complement is +1 to the normal
             * one */
            CLEAR_POSIX_WARNINGS_AND_RETURN(class_number + complement);
        }
        else if (! check_only) {

            /* Here, it is an unrecognized class.  This is an error (unless the
            * call is to check only, which we've already handled above) */
            const char * const complement_string = (complement)
                                                   ? "^"
                                                   : "";
            RExC_parse_set((char *) p);
            vFAIL3utf8f("POSIX class [:%s%" UTF8f ":] unknown",
                        complement_string,
                        UTF8fARG(UTF, RExC_parse - name_start - 2, name_start));
        }
    }

    return OOB_NAMEDCLASS;
}
#undef ADD_POSIX_WARNING

STATIC unsigned  int
S_regex_set_precedence(const U8 my_operator) {

    /* Returns the precedence in the (?[...]) construct of the input operator,
     * specified by its character representation.  The precedence follows
     * general Perl rules, but it extends this so that ')' and ']' have (low)
     * precedence even though they aren't really operators */

    switch (my_operator) {
        case '!':
            return 5;
        case '&':
            return 4;
        case '^':
        case '|':
        case '+':
        case '-':
            return 3;
        case ')':
            return 2;
        case ']':
            return 1;
    }

    NOT_REACHED; /* NOTREACHED */
    return 0;   /* Silence compiler warning */
}

STATIC regnode_offset
S_handle_regex_sets(pTHX_ RExC_state_t *pRExC_state, SV** return_invlist,
                    I32 *flagp, U32 depth)
{
    /* Handle the (?[...]) construct to do set operations */

    U8 curchar;                     /* Current character being parsed */
    UV start, end;	            /* End points of code point ranges */
    SV* final = NULL;               /* The end result inversion list */
    SV* result_string;              /* 'final' stringified */
    AV* stack;                      /* stack of operators and operands not yet
                                       resolved */
    AV* fence_stack = NULL;         /* A stack containing the positions in
                                       'stack' of where the undealt-with left
                                       parens would be if they were actually
                                       put there */
    /* The 'volatile' is a workaround for an optimiser bug
     * in Solaris Studio 12.3. See RT #127455 */
    volatile IV fence = 0;          /* Position of where most recent undealt-
                                       with left paren in stack is; -1 if none.
                                     */
    STRLEN len;                     /* Temporary */
    regnode_offset node;            /* Temporary, and final regnode returned by
                                       this function */
    const bool save_fold = FOLD;    /* Temporary */
    char *save_end, *save_parse;    /* Temporaries */
    const bool in_locale = LOC;     /* we turn off /l during processing */

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_HANDLE_REGEX_SETS;

    DEBUG_PARSE("xcls");

    if (in_locale) {
        set_regex_charset(&RExC_flags, REGEX_UNICODE_CHARSET);
    }

    /* The use of this operator implies /u.  This is required so that the
     * compile time values are valid in all runtime cases */
    REQUIRE_UNI_RULES(flagp, 0);

    /* Everything in this construct is a metacharacter.  Operands begin with
     * either a '\' (for an escape sequence), or a '[' for a bracketed
     * character class.  Any other character should be an operator, or
     * parenthesis for grouping.  Both types of operands are handled by calling
     * regclass() to parse them.  It is called with a parameter to indicate to
     * return the computed inversion list.  The parsing here is implemented via
     * a stack.  Each entry on the stack is a single character representing one
     * of the operators; or else a pointer to an operand inversion list. */

#define IS_OPERATOR(a) SvIOK(a)
#define IS_OPERAND(a)  (! IS_OPERATOR(a))

    /* The stack is kept in Łukasiewicz order.  (That's pronounced similar
     * to luke-a-shave-itch (or -itz), but people who didn't want to bother
     * with pronouncing it called it Reverse Polish instead, but now that YOU
     * know how to pronounce it you can use the correct term, thus giving due
     * credit to the person who invented it, and impressing your geek friends.
     * Wikipedia says that the pronunciation of "Ł" has been changing so that
     * it is now more like an English initial W (as in wonk) than an L.)
     *
     * This means that, for example, 'a | b & c' is stored on the stack as
     *
     * c  [4]
     * b  [3]
     * &  [2]
     * a  [1]
     * |  [0]
     *
     * where the numbers in brackets give the stack [array] element number.
     * In this implementation, parentheses are not stored on the stack.
     * Instead a '(' creates a "fence" so that the part of the stack below the
     * fence is invisible except to the corresponding ')' (this allows us to
     * replace testing for parens, by using instead subtraction of the fence
     * position).  As new operands are processed they are pushed onto the stack
     * (except as noted in the next paragraph).  New operators of higher
     * precedence than the current final one are inserted on the stack before
     * the lhs operand (so that when the rhs is pushed next, everything will be
     * in the correct positions shown above.  When an operator of equal or
     * lower precedence is encountered in parsing, all the stacked operations
     * of equal or higher precedence are evaluated, leaving the result as the
     * top entry on the stack.  This makes higher precedence operations
     * evaluate before lower precedence ones, and causes operations of equal
     * precedence to left associate.
     *
     * The only unary operator '!' is immediately pushed onto the stack when
     * encountered.  When an operand is encountered, if the top of the stack is
     * a '!", the complement is immediately performed, and the '!' popped.  The
     * resulting value is treated as a new operand, and the logic in the
     * previous paragraph is executed.  Thus in the expression
     *      [a] + ! [b]
     * the stack looks like
     *
     * !
     * a
     * +
     *
     * as 'b' gets parsed, the latter gets evaluated to '!b', and the stack
     * becomes
     *
     * !b
     * a
     * +
     *
     * A ')' is treated as an operator with lower precedence than all the
     * aforementioned ones, which causes all operations on the stack above the
     * corresponding '(' to be evaluated down to a single resultant operand.
     * Then the fence for the '(' is removed, and the operand goes through the
     * algorithm above, without the fence.
     *
     * A separate stack is kept of the fence positions, so that the position of
     * the latest so-far unbalanced '(' is at the top of it.
     *
     * The ']' ending the construct is treated as the lowest operator of all,
     * so that everything gets evaluated down to a single operand, which is the
     * result */

    stack = (AV*)newSV_type_mortal(SVt_PVAV);
    fence_stack = (AV*)newSV_type_mortal(SVt_PVAV);

    while (RExC_parse < RExC_end) {
        I32 top_index;              /* Index of top-most element in 'stack' */
        SV** top_ptr;               /* Pointer to top 'stack' element */
        SV* current = NULL;         /* To contain the current inversion list
                                       operand */
        SV* only_to_avoid_leaks;

        skip_to_be_ignored_text(pRExC_state, &RExC_parse,
                                TRUE /* Force /x */ );
        if (RExC_parse >= RExC_end) {   /* Fail */
            break;
        }

        curchar = UCHARAT(RExC_parse);

redo_curchar:

#ifdef ENABLE_REGEX_SETS_DEBUGGING
                    /* Enable with -Accflags=-DENABLE_REGEX_SETS_DEBUGGING */
        DEBUG_U(dump_regex_sets_structures(pRExC_state,
                                           stack, fence, fence_stack));
#endif

        top_index = av_tindex_skip_len_mg(stack);

        switch (curchar) {
            SV** stacked_ptr;       /* Ptr to something already on 'stack' */
            char stacked_operator;  /* The topmost operator on the 'stack'. */
            SV* lhs;                /* Operand to the left of the operator */
            SV* rhs;                /* Operand to the right of the operator */
            SV* fence_ptr;          /* Pointer to top element of the fence
                                       stack */
            case '(':

                if (   RExC_parse < RExC_end - 2
                    && UCHARAT(RExC_parse + 1) == '?'
                    && strchr("^" STD_PAT_MODS, *(RExC_parse + 2)))
                {
                    const regnode_offset orig_emit = RExC_emit;
                    SV * resultant_invlist;

                    /* Here it could be an embedded '(?flags:(?[...])'.
                     * This happens when we have some thing like
                     *
                     *   my $thai_or_lao = qr/(?[ \p{Thai} + \p{Lao} ])/;
                     *   ...
                     *   qr/(?[ \p{Digit} & $thai_or_lao ])/;
                     *
                     * Here we would be handling the interpolated
                     * '$thai_or_lao'.  We handle this by a recursive call to
                     * reg which returns the inversion list the
                     * interpolated expression evaluates to.  Actually, the
                     * return is a special regnode containing a pointer to that
                     * inversion list.  If the return isn't that regnode alone,
                     * we know that this wasn't such an interpolation, which is
                     * an error: we need to get a single inversion list back
                     * from the recursion */

                    RExC_parse_inc_by(1);
                    RExC_sets_depth++;

                    node = reg(pRExC_state, 2, flagp, depth+1);
                    RETURN_FAIL_ON_RESTART(*flagp, flagp);

                    if (   OP(REGNODE_p(node)) != REGEX_SET
                           /* If more than a single node returned, the nested
                            * parens evaluated to more than just a (?[...]),
                            * which isn't legal */
                        || RExC_emit != orig_emit
                                      + NODE_STEP_REGNODE
                                      + REGNODE_ARG_LEN(REGEX_SET))
                    {
                        vFAIL("Expecting interpolated extended charclass");
                    }
                    resultant_invlist = (SV *) ARGp(REGNODE_p(node));
                    current = invlist_clone(resultant_invlist, NULL);
                    SvREFCNT_dec(resultant_invlist);

                    RExC_sets_depth--;
                    RExC_emit = orig_emit;
                    goto handle_operand;
                }

                /* A regular '('.  Look behind for illegal syntax */
                if (top_index - fence >= 0) {
                    /* If the top entry on the stack is an operator, it had
                     * better be a '!', otherwise the entry below the top
                     * operand should be an operator */
                    if (   ! (top_ptr = av_fetch(stack, top_index, FALSE))
                        || (IS_OPERATOR(*top_ptr) && SvUV(*top_ptr) != '!')
                        || (   IS_OPERAND(*top_ptr)
                            && (   top_index - fence < 1
                                || ! (stacked_ptr = av_fetch(stack,
                                                             top_index - 1,
                                                             FALSE))
                                || ! IS_OPERATOR(*stacked_ptr))))
                    {
                        RExC_parse_inc_by(1);
                        vFAIL("Unexpected '(' with no preceding operator");
                    }
                }

                /* Stack the position of this undealt-with left paren */
                av_push_simple(fence_stack, newSViv(fence));
                fence = top_index + 1;
                break;

            case '\\':
                /* regclass() can only return RESTART_PARSE and NEED_UTF8 if
                 * multi-char folds are allowed.  */
                if (!regclass(pRExC_state, flagp, depth+1,
                              TRUE, /* means parse just the next thing */
                              FALSE, /* don't allow multi-char folds */
                              FALSE, /* don't silence non-portable warnings.  */
                              TRUE,  /* strict */
                              FALSE, /* Require return to be an ANYOF */
                              &current))
                {
                    RETURN_FAIL_ON_RESTART(*flagp, flagp);
                    goto regclass_failed;
                }

                assert(current);

                /* regclass() will return with parsing just the \ sequence,
                 * leaving the parse pointer at the next thing to parse */
                RExC_parse--;
                goto handle_operand;

            case '[':   /* Is a bracketed character class */
            {
                /* See if this is a [:posix:] class. */
                bool is_posix_class = (OOB_NAMEDCLASS
                            < handle_possible_posix(pRExC_state,
                                                RExC_parse + 1,
                                                NULL,
                                                NULL,
                                                TRUE /* checking only */));
                /* If it is a posix class, leave the parse pointer at the '['
                 * to fool regclass() into thinking it is part of a
                 * '[[:posix:]]'. */
                if (! is_posix_class) {
                    RExC_parse_inc_by(1);
                }

                /* regclass() can only return RESTART_PARSE and NEED_UTF8 if
                 * multi-char folds are allowed.  */
                if (!regclass(pRExC_state, flagp, depth+1,
                                is_posix_class, /* parse the whole char
                                                    class only if not a
                                                    posix class */
                                FALSE, /* don't allow multi-char folds */
                                TRUE, /* silence non-portable warnings. */
                                TRUE, /* strict */
                                FALSE, /* Require return to be an ANYOF */
                                &current))
                {
                    RETURN_FAIL_ON_RESTART(*flagp, flagp);
                    goto regclass_failed;
                }

                assert(current);

                /* function call leaves parse pointing to the ']', except if we
                 * faked it */
                if (is_posix_class) {
                    RExC_parse--;
                }

                goto handle_operand;
            }

            case ']':
                if (top_index >= 1) {
                    goto join_operators;
                }

                /* Only a single operand on the stack: are done */
                goto done;

            case ')':
                if (av_tindex_skip_len_mg(fence_stack) < 0) {
                    if (UCHARAT(RExC_parse - 1) == ']')  {
                        break;
                    }
                    RExC_parse_inc_by(1);
                    vFAIL("Unexpected ')'");
                }

                /* If nothing after the fence, is missing an operand */
                if (top_index - fence < 0) {
                    RExC_parse_inc_by(1);
                    goto bad_syntax;
                }
                /* If at least two things on the stack, treat this as an
                  * operator */
                if (top_index - fence >= 1) {
                    goto join_operators;
                }

                /* Here only a single thing on the fenced stack, and there is a
                 * fence.  Get rid of it */
                fence_ptr = av_pop(fence_stack);
                assert(fence_ptr);
                fence = SvIV(fence_ptr);
                SvREFCNT_dec_NN(fence_ptr);
                fence_ptr = NULL;

                if (fence < 0) {
                    fence = 0;
                }

                /* Having gotten rid of the fence, we pop the operand at the
                 * stack top and process it as a newly encountered operand */
                current = av_pop(stack);
                if (IS_OPERAND(current)) {
                    goto handle_operand;
                }

                RExC_parse_inc_by(1);
                goto bad_syntax;

            case '&':
            case '|':
            case '+':
            case '-':
            case '^':

                /* These binary operators should have a left operand already
                 * parsed */
                if (   top_index - fence < 0
                    || top_index - fence == 1
                    || ( ! (top_ptr = av_fetch(stack, top_index, FALSE)))
                    || ! IS_OPERAND(*top_ptr))
                {
                    goto unexpected_binary;
                }

                /* If only the one operand is on the part of the stack visible
                 * to us, we just place this operator in the proper position */
                if (top_index - fence < 2) {

                    /* Place the operator before the operand */

                    SV* lhs = av_pop(stack);
                    av_push_simple(stack, newSVuv(curchar));
                    av_push_simple(stack, lhs);
                    break;
                }

                /* But if there is something else on the stack, we need to
                 * process it before this new operator if and only if the
                 * stacked operation has equal or higher precedence than the
                 * new one */

             join_operators:

                /* The operator on the stack is supposed to be below both its
                 * operands */
                if (   ! (stacked_ptr = av_fetch(stack, top_index - 2, FALSE))
                    || IS_OPERAND(*stacked_ptr))
                {
                    /* But if not, it's legal and indicates we are completely
                     * done if and only if we're currently processing a ']',
                     * which should be the final thing in the expression */
                    if (curchar == ']') {
                        goto done;
                    }

                  unexpected_binary:
                    RExC_parse_inc_by(1);
                    vFAIL2("Unexpected binary operator '%c' with no "
                           "preceding operand", curchar);
                }
                stacked_operator = (char) SvUV(*stacked_ptr);

                if (regex_set_precedence(curchar)
                    > regex_set_precedence(stacked_operator))
                {
                    /* Here, the new operator has higher precedence than the
                     * stacked one.  This means we need to add the new one to
                     * the stack to await its rhs operand (and maybe more
                     * stuff).  We put it before the lhs operand, leaving
                     * untouched the stacked operator and everything below it
                     * */
                    lhs = av_pop(stack);
                    assert(IS_OPERAND(lhs));
                    av_push_simple(stack, newSVuv(curchar));
                    av_push_simple(stack, lhs);
                    break;
                }

                /* Here, the new operator has equal or lower precedence than
                 * what's already there.  This means the operation already
                 * there should be performed now, before the new one. */

                rhs = av_pop(stack);
                if (! IS_OPERAND(rhs)) {

                    /* This can happen when a ! is not followed by an operand,
                     * like in /(?[\t &!])/ */
                    goto bad_syntax;
                }

                lhs = av_pop(stack);

                if (! IS_OPERAND(lhs)) {

                    /* This can happen when there is an empty (), like in
                     * /(?[[0]+()+])/ */
                    goto bad_syntax;
                }

                switch (stacked_operator) {
                    case '&':
                        _invlist_intersection(lhs, rhs, &rhs);
                        break;

                    case '|':
                    case '+':
                        _invlist_union(lhs, rhs, &rhs);
                        break;

                    case '-':
                        _invlist_subtract(lhs, rhs, &rhs);
                        break;

                    case '^':   /* The union minus the intersection */
                    {
                        SV* i = NULL;
                        SV* u = NULL;

                        _invlist_union(lhs, rhs, &u);
                        _invlist_intersection(lhs, rhs, &i);
                        _invlist_subtract(u, i, &rhs);
                        SvREFCNT_dec_NN(i);
                        SvREFCNT_dec_NN(u);
                        break;
                    }
                }
                SvREFCNT_dec(lhs);

                /* Here, the higher precedence operation has been done, and the
                 * result is in 'rhs'.  We overwrite the stacked operator with
                 * the result.  Then we redo this code to either push the new
                 * operator onto the stack or perform any higher precedence
                 * stacked operation */
                only_to_avoid_leaks = av_pop(stack);
                SvREFCNT_dec(only_to_avoid_leaks);
                av_push_simple(stack, rhs);
                goto redo_curchar;

            case '!':   /* Highest priority, right associative */

                /* If what's already at the top of the stack is another '!",
                 * they just cancel each other out */
                if (   (top_ptr = av_fetch(stack, top_index, FALSE))
                    && (IS_OPERATOR(*top_ptr) && SvUV(*top_ptr) == '!'))
                {
                    only_to_avoid_leaks = av_pop(stack);
                    SvREFCNT_dec(only_to_avoid_leaks);
                }
                else { /* Otherwise, since it's right associative, just push
                          onto the stack */
                    av_push_simple(stack, newSVuv(curchar));
                }
                break;

            default:
                RExC_parse_inc();
                if (RExC_parse >= RExC_end) {
                    break;
                }
                vFAIL("Unexpected character");

          handle_operand:

            /* Here 'current' is the operand.  If something is already on the
             * stack, we have to check if it is a !.  But first, the code above
             * may have altered the stack in the time since we earlier set
             * 'top_index'.  */

            top_index = av_tindex_skip_len_mg(stack);
            if (top_index - fence >= 0) {
                /* If the top entry on the stack is an operator, it had better
                 * be a '!', otherwise the entry below the top operand should
                 * be an operator */
                top_ptr = av_fetch(stack, top_index, FALSE);
                assert(top_ptr);
                if (IS_OPERATOR(*top_ptr)) {

                    /* The only permissible operator at the top of the stack is
                     * '!', which is applied immediately to this operand. */
                    curchar = (char) SvUV(*top_ptr);
                    if (curchar != '!') {
                        SvREFCNT_dec(current);
                        vFAIL2("Unexpected binary operator '%c' with no "
                                "preceding operand", curchar);
                    }

                    _invlist_invert(current);

                    only_to_avoid_leaks = av_pop(stack);
                    SvREFCNT_dec(only_to_avoid_leaks);

                    /* And we redo with the inverted operand.  This allows
                     * handling multiple ! in a row */
                    goto handle_operand;
                }
                          /* Single operand is ok only for the non-binary ')'
                           * operator */
                else if ((top_index - fence == 0 && curchar != ')')
                         || (top_index - fence > 0
                             && (! (stacked_ptr = av_fetch(stack,
                                                           top_index - 1,
                                                           FALSE))
                                 || IS_OPERAND(*stacked_ptr))))
                {
                    SvREFCNT_dec(current);
                    vFAIL("Operand with no preceding operator");
                }
            }

            /* Here there was nothing on the stack or the top element was
             * another operand.  Just add this new one */
            av_push_simple(stack, current);

        } /* End of switch on next parse token */

        RExC_parse_inc();
    } /* End of loop parsing through the construct */

    vFAIL("Syntax error in (?[...])");

  done:

    if (RExC_parse >= RExC_end || RExC_parse[1] != ')') {
        if (RExC_parse < RExC_end) {
            RExC_parse_inc_by(1);
        }

        vFAIL("Unexpected ']' with no following ')' in (?[...");
    }

    if (av_tindex_skip_len_mg(fence_stack) >= 0) {
        vFAIL("Unmatched (");
    }

    if (av_tindex_skip_len_mg(stack) < 0   /* Was empty */
        || ((final = av_pop(stack)) == NULL)
        || ! IS_OPERAND(final)
        || ! is_invlist(final)
        || av_tindex_skip_len_mg(stack) >= 0)  /* More left on stack */
    {
      bad_syntax:
        SvREFCNT_dec(final);
        vFAIL("Incomplete expression within '(?[ ])'");
    }

    /* Here, 'final' is the resultant inversion list from evaluating the
     * expression.  Return it if so requested */
    if (return_invlist) {
        *return_invlist = final;
        return END;
    }

    if (RExC_sets_depth) {  /* If within a recursive call, return in a special
                               regnode */
        RExC_parse_inc_by(1);
        node = regpnode(pRExC_state, REGEX_SET, final);
    }
    else {

        /* Otherwise generate a resultant node, based on 'final'.  regclass()
         * is expecting a string of ranges and individual code points */
        invlist_iterinit(final);
        result_string = newSVpvs("");
        while (invlist_iternext(final, &start, &end)) {
            if (start == end) {
                Perl_sv_catpvf(aTHX_ result_string, "\\x{%" UVXf "}", start);
            }
            else {
                Perl_sv_catpvf(aTHX_ result_string, "\\x{%" UVXf "}-\\x{%"
                                                        UVXf "}", start, end);
            }
        }

        /* About to generate an ANYOF (or similar) node from the inversion list
         * we have calculated */
        save_parse = RExC_parse;
        RExC_parse_set(SvPV(result_string, len));
        save_end = RExC_end;
        RExC_end = RExC_parse + len;
        TURN_OFF_WARNINGS_IN_SUBSTITUTE_PARSE;

        /* We turn off folding around the call, as the class we have
         * constructed already has all folding taken into consideration, and we
         * don't want regclass() to add to that */
        RExC_flags &= ~RXf_PMf_FOLD;
        /* regclass() can only return RESTART_PARSE and NEED_UTF8 if multi-char
         * folds are allowed.  */
        node = regclass(pRExC_state, flagp, depth+1,
                        FALSE, /* means parse the whole char class */
                        FALSE, /* don't allow multi-char folds */
                        TRUE, /* silence non-portable warnings.  The above may
                                 very well have generated non-portable code
                                 points, but they're valid on this machine */
                        FALSE, /* similarly, no need for strict */

                        /* We can optimize into something besides an ANYOF,
                         * except under /l, which needs to be ANYOF because of
                         * runtime checks for locale sanity, etc */
                    ! in_locale,
                        NULL
                    );

        RESTORE_WARNINGS;
        RExC_parse_set(save_parse + 1);
        RExC_end = save_end;
        SvREFCNT_dec_NN(final);
        SvREFCNT_dec_NN(result_string);

        if (save_fold) {
            RExC_flags |= RXf_PMf_FOLD;
        }

        if (!node) {
            RETURN_FAIL_ON_RESTART(*flagp, flagp);
            goto regclass_failed;
        }

        /* Fix up the node type if we are in locale.  (We have pretended we are
         * under /u for the purposes of regclass(), as this construct will only
         * work under UTF-8 locales.  But now we change the opcode to be ANYOFL
         * (so as to cause any warnings about bad locales to be output in
         * regexec.c), and add the flag that indicates to check if not in a
         * UTF-8 locale.  The reason we above forbid optimization into
         * something other than an ANYOF node is simply to minimize the number
         * of code changes in regexec.c.  Otherwise we would have to create new
         * EXACTish node types and deal with them.  This decision could be
         * revisited should this construct become popular.
         *
         * (One might think we could look at the resulting ANYOF node and
         * suppress the flag if everything is above 255, as those would be
         * UTF-8 only, but this isn't true, as the components that led to that
         * result could have been locale-affected, and just happen to cancel
         * each other out under UTF-8 locales.) */
        if (in_locale) {
            set_regex_charset(&RExC_flags, REGEX_LOCALE_CHARSET);

            assert(OP(REGNODE_p(node)) == ANYOF);

            OP(REGNODE_p(node)) = ANYOFL;
            ANYOF_FLAGS(REGNODE_p(node)) |= ANYOFL_UTF8_LOCALE_REQD;
        }
    }

    nextchar(pRExC_state);
    return node;

  regclass_failed:
    FAIL2("panic: regclass returned failure to handle_sets, " "flags=%#" UVxf,
                                                                (UV) *flagp);
}

#ifdef ENABLE_REGEX_SETS_DEBUGGING

STATIC void
S_dump_regex_sets_structures(pTHX_ RExC_state_t *pRExC_state,
                             AV * stack, const IV fence, AV * fence_stack)
{   /* Dumps the stacks in handle_regex_sets() */

    const SSize_t stack_top = av_tindex_skip_len_mg(stack);
    const SSize_t fence_stack_top = av_tindex_skip_len_mg(fence_stack);
    SSize_t i;

    PERL_ARGS_ASSERT_DUMP_REGEX_SETS_STRUCTURES;

    PerlIO_printf(Perl_debug_log, "\nParse position is:%s\n", RExC_parse);

    if (stack_top < 0) {
        PerlIO_printf(Perl_debug_log, "Nothing on stack\n");
    }
    else {
        PerlIO_printf(Perl_debug_log, "Stack: (fence=%d)\n", (int) fence);
        for (i = stack_top; i >= 0; i--) {
            SV ** element_ptr = av_fetch(stack, i, FALSE);
            if (! element_ptr) {
            }

            if (IS_OPERATOR(*element_ptr)) {
                PerlIO_printf(Perl_debug_log, "[%d]: %c\n",
                                            (int) i, (int) SvIV(*element_ptr));
            }
            else {
                PerlIO_printf(Perl_debug_log, "[%d] ", (int) i);
                sv_dump(*element_ptr);
            }
        }
    }

    if (fence_stack_top < 0) {
        PerlIO_printf(Perl_debug_log, "Nothing on fence_stack\n");
    }
    else {
        PerlIO_printf(Perl_debug_log, "Fence_stack: \n");
        for (i = fence_stack_top; i >= 0; i--) {
            SV ** element_ptr = av_fetch_simple(fence_stack, i, FALSE);
            if (! element_ptr) {
            }

            PerlIO_printf(Perl_debug_log, "[%d]: %d\n",
                                            (int) i, (int) SvIV(*element_ptr));
        }
    }
}

#endif

#undef IS_OPERATOR
#undef IS_OPERAND

void
Perl_add_above_Latin1_folds(pTHX_ RExC_state_t *pRExC_state, const U8 cp, SV** invlist)
{
    /* This adds the Latin1/above-Latin1 folding rules.
     *
     * This should be called only for a Latin1-range code points, cp, which is
     * known to be involved in a simple fold with other code points above
     * Latin1.  It would give false results if /aa has been specified.
     * Multi-char folds are outside the scope of this, and must be handled
     * specially. */

    PERL_ARGS_ASSERT_ADD_ABOVE_LATIN1_FOLDS;

    assert(HAS_NONLATIN1_SIMPLE_FOLD_CLOSURE(cp));

    /* The rules that are valid for all Unicode versions are hard-coded in */
    switch (cp) {
        case 'k':
        case 'K':
          *invlist =
             add_cp_to_invlist(*invlist, KELVIN_SIGN);
            break;
        case 's':
        case 'S':
          *invlist = add_cp_to_invlist(*invlist, LATIN_SMALL_LETTER_LONG_S);
            break;
        case MICRO_SIGN:
          *invlist = add_cp_to_invlist(*invlist, GREEK_CAPITAL_LETTER_MU);
          *invlist = add_cp_to_invlist(*invlist, GREEK_SMALL_LETTER_MU);
            break;
        case LATIN_CAPITAL_LETTER_A_WITH_RING_ABOVE:
        case LATIN_SMALL_LETTER_A_WITH_RING_ABOVE:
          *invlist = add_cp_to_invlist(*invlist, ANGSTROM_SIGN);
            break;
        case LATIN_SMALL_LETTER_Y_WITH_DIAERESIS:
          *invlist = add_cp_to_invlist(*invlist,
                                        LATIN_CAPITAL_LETTER_Y_WITH_DIAERESIS);
            break;

        default:    /* Other code points are checked against the data for the
                       current Unicode version */
          {
            Size_t folds_count;
            U32 first_fold;
            const U32 * remaining_folds;
            UV folded_cp;

            if (isASCII(cp)) {
                folded_cp = toFOLD(cp);
            }
            else {
                U8 dummy_fold[UTF8_MAXBYTES_CASE+1];
                Size_t dummy_len;
                folded_cp = _to_fold_latin1(cp, dummy_fold, &dummy_len, 0);
            }

            if (folded_cp > 255) {
                *invlist = add_cp_to_invlist(*invlist, folded_cp);
            }

            folds_count = _inverse_folds(folded_cp, &first_fold,
                                                    &remaining_folds);
            if (folds_count == 0) {

                /* Use deprecated warning to increase the chances of this being
                 * output */
                ckWARN2reg_d(RExC_parse,
                        "Perl folding rules are not up-to-date for 0x%02X;"
                        " please use the perlbug utility to report;", cp);
            }
            else {
                unsigned int i;

                if (first_fold > 255) {
                    *invlist = add_cp_to_invlist(*invlist, first_fold);
                }
                for (i = 0; i < folds_count - 1; i++) {
                    if (remaining_folds[i] > 255) {
                        *invlist = add_cp_to_invlist(*invlist,
                                                    remaining_folds[i]);
                    }
                }
            }
            break;
         }
    }
}

STATIC void
S_output_posix_warnings(pTHX_ RExC_state_t *pRExC_state, AV* posix_warnings)
{
    /* Output the elements of the array given by '*posix_warnings' as REGEXP
     * warnings. */

    SV * msg;
    const bool first_is_fatal = ckDEAD(packWARN(WARN_REGEXP));

    PERL_ARGS_ASSERT_OUTPUT_POSIX_WARNINGS;

    if (! TO_OUTPUT_WARNINGS(RExC_parse)) {
        CLEAR_POSIX_WARNINGS();
        return;
    }

    while ((msg = av_shift(posix_warnings)) != &PL_sv_undef) {
        if (first_is_fatal) {           /* Avoid leaking this */
            av_undef(posix_warnings);   /* This isn't necessary if the
                                            array is mortal, but is a
                                            fail-safe */
            (void) sv_2mortal(msg);
            PREPARE_TO_DIE;
        }
        Perl_warner(aTHX_ packWARN(WARN_REGEXP), "%s", SvPVX(msg));
        SvREFCNT_dec_NN(msg);
    }

    UPDATE_WARNINGS_LOC(RExC_parse);
}

PERL_STATIC_INLINE Size_t
S_find_first_differing_byte_pos(const U8 * s1, const U8 * s2, const Size_t max)
{
    const U8 * const start = s1;
    const U8 * const send = start + max;

    PERL_ARGS_ASSERT_FIND_FIRST_DIFFERING_BYTE_POS;

    while (s1 < send && *s1  == *s2) {
        s1++; s2++;
    }

    return s1 - start;
}

STATIC AV *
S_add_multi_match(pTHX_ AV* multi_char_matches, SV* multi_string, const STRLEN cp_count)
{
    /* This adds the string scalar <multi_string> to the array
     * <multi_char_matches>.  <multi_string> is known to have exactly
     * <cp_count> code points in it.  This is used when constructing a
     * bracketed character class and we find something that needs to match more
     * than a single character.
     *
     * <multi_char_matches> is actually an array of arrays.  Each top-level
     * element is an array that contains all the strings known so far that are
     * the same length.  And that length (in number of code points) is the same
     * as the index of the top-level array.  Hence, the [2] element is an
     * array, each element thereof is a string containing TWO code points;
     * while element [3] is for strings of THREE characters, and so on.  Since
     * this is for multi-char strings there can never be a [0] nor [1] element.
     *
     * When we rewrite the character class below, we will do so such that the
     * longest strings are written first, so that it prefers the longest
     * matching strings first.  This is done even if it turns out that any
     * quantifier is non-greedy, out of this programmer's (khw) laziness.  Tom
     * Christiansen has agreed that this is ok.  This makes the test for the
     * ligature 'ffi' come before the test for 'ff', for example */

    AV* this_array;
    AV** this_array_ptr;

    PERL_ARGS_ASSERT_ADD_MULTI_MATCH;

    if (! multi_char_matches) {
        multi_char_matches = newAV();
    }

    if (av_exists(multi_char_matches, cp_count)) {
        this_array_ptr = (AV**) av_fetch_simple(multi_char_matches, cp_count, FALSE);
        this_array = *this_array_ptr;
    }
    else {
        this_array = newAV();
        av_store_simple(multi_char_matches, cp_count,
                 (SV*) this_array);
    }
    av_push_simple(this_array, multi_string);

    return multi_char_matches;
}

/* The names of properties whose definitions are not known at compile time are
 * stored in this SV, after a constant heading.  So if the length has been
 * changed since initialization, then there is a run-time definition. */
#define HAS_NONLOCALE_RUNTIME_PROPERTY_DEFINITION                            \
                                        (SvCUR(listsv) != initial_listsv_len)

/* There is a restricted set of white space characters that are legal when
 * ignoring white space in a bracketed character class.  This generates the
 * code to skip them.
 *
 * There is a line below that uses the same white space criteria but is outside
 * this macro.  Both here and there must use the same definition */
#define SKIP_BRACKETED_WHITE_SPACE(do_skip, p, stop_p)                  \
    STMT_START {                                                        \
        if (do_skip) {                                                  \
            while (p < stop_p && isBLANK_A(UCHARAT(p)))                 \
            {                                                           \
                p++;                                                    \
            }                                                           \
        }                                                               \
    } STMT_END

STATIC regnode_offset
S_regclass(pTHX_ RExC_state_t *pRExC_state, I32 *flagp, U32 depth,
                 const bool stop_at_1,  /* Just parse the next thing, don't
                                           look for a full character class */
                 bool allow_mutiple_chars,
                 const bool silence_non_portable,   /* Don't output warnings
                                                       about too large
                                                       characters */
                 const bool strict,
                 bool optimizable,                  /* ? Allow a non-ANYOF return
                                                       node */
                 SV** ret_invlist  /* Return an inversion list, not a node */
          )
{
    /* parse a bracketed class specification.  Most of these will produce an
     * ANYOF node; but something like [a] will produce an EXACT node; [aA], an
     * EXACTFish node; [[:ascii:]], a POSIXA node; etc.  It is more complex
     * under /i with multi-character folds: it will be rewritten following the
     * paradigm of this example, where the <multi-fold>s are characters which
     * fold to multiple character sequences:
     *      /[abc\x{multi-fold1}def\x{multi-fold2}ghi]/i
     * gets effectively rewritten as:
     *      /(?:\x{multi-fold1}|\x{multi-fold2}|[abcdefghi]/i
     * reg() gets called (recursively) on the rewritten version, and this
     * function will return what it constructs.  (Actually the <multi-fold>s
     * aren't physically removed from the [abcdefghi], it's just that they are
     * ignored in the recursion by means of a flag:
     * <RExC_in_multi_char_class>.)
     *
     * ANYOF nodes contain a bit map for the first NUM_ANYOF_CODE_POINTS
     * characters, with the corresponding bit set if that character is in the
     * list.  For characters above this, an inversion list is used.  There
     * are extra bits for \w, etc. in locale ANYOFs, as what these match is not
     * determinable at compile time
     *
     * On success, returns the offset at which any next node should be placed
     * into the regex engine program being compiled.
     *
     * Returns 0 otherwise, setting flagp to RESTART_PARSE if the parse needs
     * to be restarted, or'd with NEED_UTF8 if the pattern needs to be upgraded to
     * UTF-8
     */

    UV prevvalue = OOB_UNICODE, save_prevvalue = OOB_UNICODE;
    IV range = 0;
    UV value = OOB_UNICODE, save_value = OOB_UNICODE;
    regnode_offset ret = -1;    /* Initialized to an illegal value */
    STRLEN numlen;
    int namedclass = OOB_NAMEDCLASS;
    char *rangebegin = NULL;
    SV *listsv = NULL;      /* List of \p{user-defined} whose definitions
                               aren't available at the time this was called */
    STRLEN initial_listsv_len = 0; /* Kind of a kludge to see if it is more
                                      than just initialized.  */
    SV* properties = NULL;    /* Code points that match \p{} \P{} */
    SV* posixes = NULL;     /* Code points that match classes like [:word:],
                               extended beyond the Latin1 range.  These have to
                               be kept separate from other code points for much
                               of this function because their handling  is
                               different under /i, and for most classes under
                               /d as well */
    SV* nposixes = NULL;    /* Similarly for [:^word:].  These are kept
                               separate for a while from the non-complemented
                               versions because of complications with /d
                               matching */
    SV* simple_posixes = NULL; /* But under some conditions, the classes can be
                                  treated more simply than the general case,
                                  leading to less compilation and execution
                                  work */
    UV element_count = 0;   /* Number of distinct elements in the class.
                               Optimizations may be possible if this is tiny */
    AV * multi_char_matches = NULL; /* Code points that fold to more than one
                                       character; used under /i */
    UV n;
    char * stop_ptr = RExC_end;    /* where to stop parsing */

    /* ignore unescaped whitespace? */
    const bool skip_white = cBOOL(   ret_invlist
                                  || (RExC_flags & RXf_PMf_EXTENDED_MORE));

    /* inversion list of code points this node matches only when the target
     * string is in UTF-8.  These are all non-ASCII, < 256.  (Because is under
     * /d) */
    SV* upper_latin1_only_utf8_matches = NULL;

    /* Inversion list of code points this node matches regardless of things
     * like locale, folding, utf8ness of the target string */
    SV* cp_list = NULL;

    /* Like cp_list, but code points on this list need to be checked for things
     * that fold to/from them under /i */
    SV* cp_foldable_list = NULL;

    /* Like cp_list, but code points on this list are valid only when the
     * runtime locale is UTF-8 */
    SV* only_utf8_locale_list = NULL;

    /* In a range, if one of the endpoints is non-character-set portable,
     * meaning that it hard-codes a code point that may mean a different
     * character in ASCII vs. EBCDIC, as opposed to, say, a literal 'A' or a
     * mnemonic '\t' which each mean the same character no matter which
     * character set the platform is on. */
    unsigned int non_portable_endpoint = 0;

    /* Is the range unicode? which means on a platform that isn't 1-1 native
     * to Unicode (i.e. non-ASCII), each code point in it should be considered
     * to be a Unicode value.  */
    bool unicode_range = FALSE;
    bool invert = FALSE;    /* Is this class to be complemented */

    bool warn_super = ALWAYS_WARN_SUPER;

    const char * orig_parse = RExC_parse;

    /* This variable is used to mark where the end in the input is of something
     * that looks like a POSIX construct but isn't.  During the parse, when
     * something looks like it could be such a construct is encountered, it is
     * checked for being one, but not if we've already checked this area of the
     * input.  Only after this position is reached do we check again */
    char *not_posix_region_end = RExC_parse - 1;

    AV* posix_warnings = NULL;
    const bool do_posix_warnings = ckWARN(WARN_REGEXP);
    U8 op = ANYOF;    /* The returned node-type, initialized to the expected
                         type. */
    U8 anyof_flags = 0;   /* flag bits if the node is an ANYOF-type */
    U32 posixl = 0;       /* bit field of posix classes matched under /l */


/* Flags as to what things aren't knowable until runtime.  (Note that these are
 * mutually exclusive.) */
#define HAS_USER_DEFINED_PROPERTY 0x01   /* /u any user-defined properties that
                                            haven't been defined as of yet */
#define HAS_D_RUNTIME_DEPENDENCY  0x02   /* /d if the target being matched is
                                            UTF-8 or not */
#define HAS_L_RUNTIME_DEPENDENCY   0x04 /* /l what the posix classes match and
                                            what gets folded */
    U32 has_runtime_dependency = 0;     /* OR of the above flags */

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGCLASS;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    assert(! (ret_invlist && allow_mutiple_chars));

    /* If wants an inversion list returned, we can't optimize to something
     * else. */
    if (ret_invlist) {
        optimizable = FALSE;
    }

    DEBUG_PARSE("clas");

#if UNICODE_MAJOR_VERSION < 3 /* no multifolds in early Unicode */      \
    || (UNICODE_MAJOR_VERSION == 3 && UNICODE_DOT_VERSION == 0          \
                                   && UNICODE_DOT_DOT_VERSION == 0)
    allow_mutiple_chars = FALSE;
#endif

    /* We include the /i status at the beginning of this so that we can
     * know it at runtime */
    listsv = sv_2mortal(Perl_newSVpvf(aTHX_ "#%d\n", cBOOL(FOLD)));
    initial_listsv_len = SvCUR(listsv);
    SvTEMP_off(listsv); /* Grr, TEMPs and mortals are conflated.  */

    SKIP_BRACKETED_WHITE_SPACE(skip_white, RExC_parse, RExC_end);

    assert(RExC_parse <= RExC_end);

    if (UCHARAT(RExC_parse) == '^') {	/* Complement the class */
        RExC_parse_inc_by(1);
        invert = TRUE;
        allow_mutiple_chars = FALSE;
        MARK_NAUGHTY(1);
        SKIP_BRACKETED_WHITE_SPACE(skip_white, RExC_parse, RExC_end);
    }

    /* Check that they didn't say [:posix:] instead of [[:posix:]] */
    if (! ret_invlist && MAYBE_POSIXCC(UCHARAT(RExC_parse))) {
        int maybe_class = handle_possible_posix(pRExC_state,
                                                RExC_parse,
                                                &not_posix_region_end,
                                                NULL,
                                                TRUE /* checking only */);
        if (maybe_class >= OOB_NAMEDCLASS && do_posix_warnings) {
            ckWARN4reg(not_posix_region_end,
                    "POSIX syntax [%c %c] belongs inside character classes%s",
                    *RExC_parse, *RExC_parse,
                    (maybe_class == OOB_NAMEDCLASS)
                    ? ((POSIXCC_NOTYET(*RExC_parse))
                        ? " (but this one isn't implemented)"
                        : " (but this one isn't fully valid)")
                    : ""
                    );
        }
    }

    /* If the caller wants us to just parse a single element, accomplish this
     * by faking the loop ending condition */
    if (stop_at_1 && RExC_end > RExC_parse) {
        stop_ptr = RExC_parse + 1;
    }

    /* allow 1st char to be ']' (allowing it to be '-' is dealt with later) */
    if (UCHARAT(RExC_parse) == ']')
        goto charclassloop;

    while (1) {

        if (   posix_warnings
            && av_tindex_skip_len_mg(posix_warnings) >= 0
            && RExC_parse > not_posix_region_end)
        {
            /* Warnings about posix class issues are considered tentative until
             * we are far enough along in the parse that we can no longer
             * change our mind, at which point we output them.  This is done
             * each time through the loop so that a later class won't zap them
             * before they have been dealt with. */
            output_posix_warnings(pRExC_state, posix_warnings);
        }

        SKIP_BRACKETED_WHITE_SPACE(skip_white, RExC_parse, RExC_end);

        if  (RExC_parse >= stop_ptr) {
            break;
        }

        if  (UCHARAT(RExC_parse) == ']') {
            break;
        }

      charclassloop:

        namedclass = OOB_NAMEDCLASS; /* initialize as illegal */
        save_value = value;
        save_prevvalue = prevvalue;

        if (!range) {
            rangebegin = RExC_parse;
            element_count++;
            non_portable_endpoint = 0;
        }
        if (UTF && ! UTF8_IS_INVARIANT(* RExC_parse)) {
            value = utf8n_to_uvchr((U8*)RExC_parse,
                                   RExC_end - RExC_parse,
                                   &numlen, UTF8_ALLOW_DEFAULT);
            RExC_parse_inc_by(numlen);
        }
        else {
            value = UCHARAT(RExC_parse);
            RExC_parse_inc_by(1);
        }

        if (value == '[') {
            char * posix_class_end;
            namedclass = handle_possible_posix(pRExC_state,
                                               RExC_parse,
                                               &posix_class_end,
                                               do_posix_warnings ? &posix_warnings : NULL,
                                               FALSE    /* die if error */);
            if (namedclass > OOB_NAMEDCLASS) {

                /* If there was an earlier attempt to parse this particular
                 * posix class, and it failed, it was a false alarm, as this
                 * successful one proves */
                if (   posix_warnings
                    && av_tindex_skip_len_mg(posix_warnings) >= 0
                    && not_posix_region_end >= RExC_parse
                    && not_posix_region_end <= posix_class_end)
                {
                    av_undef(posix_warnings);
                }

                RExC_parse_set(posix_class_end);
            }
            else if (namedclass == OOB_NAMEDCLASS) {
                not_posix_region_end = posix_class_end;
            }
            else {
                namedclass = OOB_NAMEDCLASS;
            }
        }
        else if (   RExC_parse - 1 > not_posix_region_end
                 && MAYBE_POSIXCC(value))
        {
            (void) handle_possible_posix(
                        pRExC_state,
                        RExC_parse - 1,  /* -1 because parse has already been
                                            advanced */
                        &not_posix_region_end,
                        do_posix_warnings ? &posix_warnings : NULL,
                        TRUE /* checking only */);
        }
        else if (  strict && ! skip_white
                 && (   generic_isCC_(value, CC_VERTSPACE_)
                     || is_VERTWS_cp_high(value)))
        {
            vFAIL("Literal vertical space in [] is illegal except under /x");
        }
        else if (value == '\\') {
            /* Is a backslash; get the code point of the char after it */

            if (RExC_parse >= RExC_end) {
                vFAIL("Unmatched [");
            }

            if (UTF && ! UTF8_IS_INVARIANT(UCHARAT(RExC_parse))) {
                value = utf8n_to_uvchr((U8*)RExC_parse,
                                   RExC_end - RExC_parse,
                                   &numlen, UTF8_ALLOW_DEFAULT);
                RExC_parse_inc_by(numlen);
            }
            else {
                value = UCHARAT(RExC_parse);
                RExC_parse_inc_by(1);
            }

            /* Some compilers cannot handle switching on 64-bit integer
             * values, therefore value cannot be an UV.  Yes, this will
             * be a problem later if we want switch on Unicode.
             * A similar issue a little bit later when switching on
             * namedclass. --jhi */

            /* If the \ is escaping white space when white space is being
             * skipped, it means that that white space is wanted literally, and
             * is already in 'value'.  Otherwise, need to translate the escape
             * into what it signifies. */
            if (! skip_white || ! isBLANK_A(value)) switch ((I32)value) {
                const char * message;
                U32 packed_warn;
                U8 grok_c_char;

            case 'w':	namedclass = ANYOF_WORDCHAR;	break;
            case 'W':	namedclass = ANYOF_NWORDCHAR;	break;
            case 's':	namedclass = ANYOF_SPACE;	break;
            case 'S':	namedclass = ANYOF_NSPACE;	break;
            case 'd':	namedclass = ANYOF_DIGIT;	break;
            case 'D':	namedclass = ANYOF_NDIGIT;	break;
            case 'v':	namedclass = ANYOF_VERTWS;	break;
            case 'V':	namedclass = ANYOF_NVERTWS;	break;
            case 'h':	namedclass = ANYOF_HORIZWS;	break;
            case 'H':	namedclass = ANYOF_NHORIZWS;	break;
            case 'N':  /* Handle \N{NAME} in class */
                {
                    const char * const backslash_N_beg = RExC_parse - 2;
                    int cp_count;

                    if (! grok_bslash_N(pRExC_state,
                                        NULL,      /* No regnode */
                                        &value,    /* Yes single value */
                                        &cp_count, /* Multiple code pt count */
                                        flagp,
                                        strict,
                                        depth)
                    ) {

                        if (*flagp & NEED_UTF8)
                            FAIL("panic: grok_bslash_N set NEED_UTF8");

                        RETURN_FAIL_ON_RESTART_FLAGP(flagp);

                        if (cp_count < 0) {
                            vFAIL("\\N in a character class must be a named character: \\N{...}");
                        }
                        else if (cp_count == 0) {
                            ckWARNreg(RExC_parse,
                              "Ignoring zero length \\N{} in character class");
                        }
                        else { /* cp_count > 1 */
                            assert(cp_count > 1);
                            if (! RExC_in_multi_char_class) {
                                if ( ! allow_mutiple_chars
                                    || invert
                                    || range
                                    || *RExC_parse == '-')
                                {
                                    if (strict) {
                                        RExC_parse--;
                                        vFAIL("\\N{} here is restricted to one character");
                                    }
                                    ckWARNreg(RExC_parse, "Using just the first character returned by \\N{} in character class");
                                    break; /* <value> contains the first code
                                              point. Drop out of the switch to
                                              process it */
                                }
                                else {
                                    SV * multi_char_N = newSVpvn(backslash_N_beg,
                                                 RExC_parse - backslash_N_beg);
                                    multi_char_matches
                                        = add_multi_match(multi_char_matches,
                                                          multi_char_N,
                                                          cp_count);
                                }
                            }
                        } /* End of cp_count != 1 */

                        /* This element should not be processed further in this
                         * class */
                        element_count--;
                        value = save_value;
                        prevvalue = save_prevvalue;
                        continue;   /* Back to top of loop to get next char */
                    }

                    /* Here, is a single code point, and <value> contains it */
                    unicode_range = TRUE;   /* \N{} are Unicode */
                }
                break;
            case 'p':
            case 'P':
                {
                char *e;

                if (RExC_pm_flags & PMf_WILDCARD) {
                    RExC_parse_inc_by(1);
                    /* diag_listed_as: Use of %s is not allowed in Unicode
                       property wildcard subpatterns in regex; marked by <--
                       HERE in m/%s/ */
                    vFAIL3("Use of '\\%c%c' is not allowed in Unicode property"
                           " wildcard subpatterns", (char) value, *(RExC_parse - 1));
                }

                /* \p means they want Unicode semantics */
                REQUIRE_UNI_RULES(flagp, 0);

                if (RExC_parse >= RExC_end)
                    vFAIL2("Empty \\%c", (U8)value);
                if (*RExC_parse == '{') {
                    const U8 c = (U8)value;
                    e = (char *) memchr(RExC_parse, '}', RExC_end - RExC_parse);
                    if (!e) {
                        RExC_parse_inc_by(1);
                        vFAIL2("Missing right brace on \\%c{}", c);
                    }

                    RExC_parse_inc_by(1);

                    /* White space is allowed adjacent to the braces and after
                     * any '^', even when not under /x */
                    while (isSPACE(*RExC_parse)) {
                         RExC_parse_inc_by(1);
                    }

                    if (UCHARAT(RExC_parse) == '^') {

                        /* toggle.  (The rhs xor gets the single bit that
                         * differs between P and p; the other xor inverts just
                         * that bit) */
                        value ^= 'P' ^ 'p';

                        RExC_parse_inc_by(1);
                        while (isSPACE(*RExC_parse)) {
                            RExC_parse_inc_by(1);
                        }
                    }

                    if (e == RExC_parse)
                        vFAIL2("Empty \\%c{}", c);

                    n = e - RExC_parse;
                    while (isSPACE(*(RExC_parse + n - 1)))
                        n--;

                }   /* The \p isn't immediately followed by a '{' */
                else if (! isALPHA(*RExC_parse)) {
                    RExC_parse_inc_safe();
                    vFAIL2("Character following \\%c must be '{' or a "
                           "single-character Unicode property name",
                           (U8) value);
                }
                else {
                    e = RExC_parse;
                    n = 1;
                }
                {
                    char* name = RExC_parse;

                    /* Any message returned about expanding the definition */
                    SV* msg = newSVpvs_flags("", SVs_TEMP);

                    /* If set TRUE, the property is user-defined as opposed to
                     * official Unicode */
                    bool user_defined = FALSE;
                    AV * strings = NULL;

                    SV * prop_definition = parse_uniprop_string(
                                            name, n, UTF, FOLD,
                                            FALSE, /* This is compile-time */

                                            /* We can't defer this defn when
                                             * the full result is required in
                                             * this call */
                                            ! cBOOL(ret_invlist),

                                            &strings,
                                            &user_defined,
                                            msg,
                                            0 /* Base level */
                                           );
                    if (SvCUR(msg)) {   /* Assumes any error causes a msg */
                        assert(prop_definition == NULL);
                        RExC_parse_set(e + 1);
                        if (SvUTF8(msg)) {  /* msg being UTF-8 makes the whole
                                               thing so, or else the display is
                                               mojibake */
                            RExC_utf8 = TRUE;
                        }
                        /* diag_listed_as: Can't find Unicode property definition "%s" in regex; marked by <-- HERE in m/%s/ */
                        vFAIL2utf8f("%" UTF8f, UTF8fARG(SvUTF8(msg),
                                    SvCUR(msg), SvPVX(msg)));
                    }

                    assert(prop_definition || strings);

                    if (strings) {
                        if (ret_invlist) {
                            if (! prop_definition) {
                                RExC_parse_set(e + 1);
                                vFAIL("Unicode string properties are not implemented in (?[...])");
                            }
                            else {
                                ckWARNreg(e + 1,
                                    "Using just the single character results"
                                    " returned by \\p{} in (?[...])");
                            }
                        }
                        else if (! RExC_in_multi_char_class) {
                            if (invert ^ (value == 'P')) {
                                RExC_parse_set(e + 1);
                                vFAIL("Inverting a character class which contains"
                                    " a multi-character sequence is illegal");
                            }

                            /* For each multi-character string ... */
                            while (av_count(strings) > 0) {
                                /* ... Each entry is itself an array of code
                                * points. */
                                AV * this_string = (AV *) av_shift( strings);
                                STRLEN cp_count = av_count(this_string);
                                SV * final = newSV(cp_count ? cp_count * 4 : 1);
                                SvPVCLEAR_FRESH(final);

                                /* Create another string of sequences of \x{...} */
                                while (av_count(this_string) > 0) {
                                    SV * character = av_shift(this_string);
                                    UV cp = SvUV(character);

                                    if (cp > 255) {
                                        REQUIRE_UTF8(flagp);
                                    }
                                    Perl_sv_catpvf(aTHX_ final, "\\x{%" UVXf "}",
                                                                        cp);
                                    SvREFCNT_dec_NN(character);
                                }
                                SvREFCNT_dec_NN(this_string);

                                /* And add that to the list of such things */
                                multi_char_matches
                                            = add_multi_match(multi_char_matches,
                                                            final,
                                                            cp_count);
                            }
                        }
                        SvREFCNT_dec_NN(strings);
                    }

                    if (! prop_definition) {    /* If we got only a string,
                                                   this iteration didn't really
                                                   find a character */
                        element_count--;
                    }
                    else if (! is_invlist(prop_definition)) {

                        /* Here, the definition isn't known, so we have gotten
                         * returned a string that will be evaluated if and when
                         * encountered at runtime.  We add it to the list of
                         * such properties, along with whether it should be
                         * complemented or not */
                        if (value == 'P') {
                            sv_catpvs(listsv, "!");
                        }
                        else {
                            sv_catpvs(listsv, "+");
                        }
                        sv_catsv(listsv, prop_definition);

                        has_runtime_dependency |= HAS_USER_DEFINED_PROPERTY;

                        /* We don't know yet what this matches, so have to flag
                         * it */
                        anyof_flags |= ANYOF_HAS_EXTRA_RUNTIME_MATCHES;
                    }
                    else {
                        assert (prop_definition && is_invlist(prop_definition));

                        /* Here we do have the complete property definition
                         *
                         * Temporary workaround for [GH #16520].  For this
                         * precise input that is in the .t that is failing,
                         * load utf8.pm, which is what the test wants, so that
                         * that .t passes */
                        if (     memEQs(RExC_start, e + 1 - RExC_start,
                                        "foo\\p{Alnum}")
                            && ! hv_common(GvHVn(PL_incgv),
                                           NULL,
                                           "utf8.pm", sizeof("utf8.pm") - 1,
                                           0, HV_FETCH_ISEXISTS, NULL, 0))
                        {
                            require_pv("utf8.pm");
                        }

                        if (! user_defined &&
                            /* We warn on matching an above-Unicode code point
                             * if the match would return true, except don't
                             * warn for \p{All}, which has exactly one element
                             * = 0 */
                            (_invlist_contains_cp(prop_definition, 0x110000)
                                && (! (_invlist_len(prop_definition) == 1
                                       && *invlist_array(prop_definition) == 0))))
                        {
                            warn_super = TRUE;
                        }

                        /* Invert if asking for the complement */
                        if (value == 'P') {
                            _invlist_union_complement_2nd(properties,
                                                          prop_definition,
                                                          &properties);
                        }
                        else {
                            _invlist_union(properties, prop_definition, &properties);
                        }
                    }
                }

                RExC_parse_set(e + 1);
                namedclass = ANYOF_UNIPROP;  /* no official name, but it's
                                                named */
                }
                break;
            case 'n':	value = '\n';			break;
            case 'r':	value = '\r';			break;
            case 't':	value = '\t';			break;
            case 'f':	value = '\f';			break;
            case 'b':	value = '\b';			break;
            case 'e':	value = ESC_NATIVE;             break;
            case 'a':	value = '\a';                   break;
            case 'o':
                RExC_parse--;	/* function expects to be pointed at the 'o' */
                if (! grok_bslash_o(&RExC_parse,
                                            RExC_end,
                                            &value,
                                            &message,
                                            &packed_warn,
                                            strict,
                                            cBOOL(range), /* MAX_UV allowed for range
                                                      upper limit */
                                            UTF))
                {
                    vFAIL(message);
                }
                else if (message && TO_OUTPUT_WARNINGS(RExC_parse)) {
                    warn_non_literal_string(RExC_parse, packed_warn, message);
                }

                if (value < 256) {
                    non_portable_endpoint++;
                }
                break;
            case 'x':
                RExC_parse--;	/* function expects to be pointed at the 'x' */
                if (!  grok_bslash_x(&RExC_parse,
                                            RExC_end,
                                            &value,
                                            &message,
                                            &packed_warn,
                                            strict,
                                            cBOOL(range), /* MAX_UV allowed for range
                                                      upper limit */
                                            UTF))
                {
                    vFAIL(message);
                }
                else if (message && TO_OUTPUT_WARNINGS(RExC_parse)) {
                    warn_non_literal_string(RExC_parse, packed_warn, message);
                }

                if (value < 256) {
                    non_portable_endpoint++;
                }
                break;
            case 'c':
                if (! grok_bslash_c(*RExC_parse, &grok_c_char, &message,
                                                                &packed_warn))
                {
                    /* going to die anyway; point to exact spot of
                        * failure */
                    RExC_parse_inc_safe();
                    vFAIL(message);
                }

                value = grok_c_char;
                RExC_parse_inc_by(1);
                if (message && TO_OUTPUT_WARNINGS(RExC_parse)) {
                    warn_non_literal_string(RExC_parse, packed_warn, message);
                }

                non_portable_endpoint++;
                break;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7':
                {
                    /* Take 1-3 octal digits */
                    I32 flags = PERL_SCAN_SILENT_ILLDIGIT
                              | PERL_SCAN_NOTIFY_ILLDIGIT;
                    numlen = (strict) ? 4 : 3;
                    value = grok_oct(--RExC_parse, &numlen, &flags, NULL);
                    RExC_parse_inc_by(numlen);
                    if (numlen != 3) {
                        if (strict) {
                            RExC_parse_inc_safe();
                            vFAIL("Need exactly 3 octal digits");
                        }
                        else if (  (flags & PERL_SCAN_NOTIFY_ILLDIGIT)
                                 && RExC_parse < RExC_end
                                 && isDIGIT(*RExC_parse)
                                 && ckWARN(WARN_REGEXP))
                        {
                            reg_warn_non_literal_string(
                                 RExC_parse + 1,
                                 form_alien_digit_msg(8, numlen, RExC_parse,
                                                        RExC_end, UTF, FALSE));
                        }
                    }
                    if (value < 256) {
                        non_portable_endpoint++;
                    }
                    break;
                }
            default:
                /* Allow \_ to not give an error */
                if (isWORDCHAR(value) && value != '_') {
                    if (strict) {
                        vFAIL2("Unrecognized escape \\%c in character class",
                               (int)value);
                    }
                    else {
                        ckWARN2reg(RExC_parse,
                            "Unrecognized escape \\%c in character class passed through",
                            (int)value);
                    }
                }
                break;
            }   /* End of switch on char following backslash */
        } /* end of handling backslash escape sequences */

        /* Here, we have the current token in 'value' */

        if (namedclass > OOB_NAMEDCLASS) { /* this is a named class \blah */
            U8 classnum;

            /* a bad range like a-\d, a-[:digit:].  The '-' is taken as a
             * literal, as is the character that began the false range, i.e.
             * the 'a' in the examples */
            if (range) {
                const int w = (RExC_parse >= rangebegin)
                                ? RExC_parse - rangebegin
                                : 0;
                if (strict) {
                    vFAIL2utf8f(
                        "False [] range \"%" UTF8f "\"",
                        UTF8fARG(UTF, w, rangebegin));
                }
                else {
                    ckWARN2reg(RExC_parse,
                        "False [] range \"%" UTF8f "\"",
                        UTF8fARG(UTF, w, rangebegin));
                    cp_list = add_cp_to_invlist(cp_list, '-');
                    cp_foldable_list = add_cp_to_invlist(cp_foldable_list,
                                                            prevvalue);
                }

                range = 0; /* this was not a true range */
                element_count += 2; /* So counts for three values */
            }

            classnum = namedclass_to_classnum(namedclass);

            if (LOC && namedclass < ANYOF_POSIXL_MAX
#ifndef HAS_ISASCII
                && classnum != CC_ASCII_
#endif
            ) {
                SV* scratch_list = NULL;

                /* What the Posix classes (like \w, [:space:]) match isn't
                 * generally knowable under locale until actual match time.  A
                 * special node is used for these which has extra space for a
                 * bitmap, with a bit reserved for each named class that is to
                 * be matched against.  (This isn't needed for \p{} and
                 * pseudo-classes, as they are not affected by locale, and
                 * hence are dealt with separately.)  However, if a named class
                 * and its complement are both present, then it matches
                 * everything, and there is no runtime dependency.  Odd numbers
                 * are the complements of the next lower number, so xor works.
                 * (Note that something like [\w\D] should match everything,
                 * because \d should be a proper subset of \w.  But rather than
                 * trust that the locale is well behaved, we leave this to
                 * runtime to sort out) */
                if (POSIXL_TEST(posixl, namedclass ^ 1)) {
                    cp_list = _add_range_to_invlist(cp_list, 0, UV_MAX);
                    POSIXL_ZERO(posixl);
                    has_runtime_dependency &= ~HAS_L_RUNTIME_DEPENDENCY;
                    anyof_flags &= ~ANYOF_MATCHES_POSIXL;
                    continue;   /* We could ignore the rest of the class, but
                                   best to parse it for any errors */
                }
                else { /* Here, isn't the complement of any already parsed
                          class */
                    POSIXL_SET(posixl, namedclass);
                    has_runtime_dependency |= HAS_L_RUNTIME_DEPENDENCY;
                    anyof_flags |= ANYOF_MATCHES_POSIXL;

                    /* The above-Latin1 characters are not subject to locale
                     * rules.  Just add them to the unconditionally-matched
                     * list */

                    /* Get the list of the above-Latin1 code points this
                     * matches */
                    _invlist_intersection_maybe_complement_2nd(PL_AboveLatin1,
                                            PL_XPosix_ptrs[classnum],

                                            /* Odd numbers are complements,
                                             * like NDIGIT, NASCII, ... */
                                            namedclass % 2 != 0,
                                            &scratch_list);
                    /* Checking if 'cp_list' is NULL first saves an extra
                     * clone.  Its reference count will be decremented at the
                     * next union, etc, or if this is the only instance, at the
                     * end of the routine */
                    if (! cp_list) {
                        cp_list = scratch_list;
                    }
                    else {
                        _invlist_union(cp_list, scratch_list, &cp_list);
                        SvREFCNT_dec_NN(scratch_list);
                    }
                    continue;   /* Go get next character */
                }
            }
            else {

                /* Here, is not /l, or is a POSIX class for which /l doesn't
                 * matter (or is a Unicode property, which is skipped here). */
                if (namedclass >= ANYOF_POSIXL_MAX) {  /* If a special class */
                    if (namedclass != ANYOF_UNIPROP) { /* UNIPROP = \p and \P */

                        /* Here, should be \h, \H, \v, or \V.  None of /d, /i
                         * nor /l make a difference in what these match,
                         * therefore we just add what they match to cp_list. */
                        if (classnum != CC_VERTSPACE_) {
                            assert(   namedclass == ANYOF_HORIZWS
                                   || namedclass == ANYOF_NHORIZWS);

                            /* It turns out that \h is just a synonym for
                             * XPosixBlank */
                            classnum = CC_BLANK_;
                        }

                        _invlist_union_maybe_complement_2nd(
                                cp_list,
                                PL_XPosix_ptrs[classnum],
                                namedclass % 2 != 0,    /* Complement if odd
                                                          (NHORIZWS, NVERTWS)
                                                        */
                                &cp_list);
                    }
                }
                else if (   AT_LEAST_UNI_SEMANTICS
                         || classnum == CC_ASCII_
                         || (DEPENDS_SEMANTICS && (   classnum == CC_DIGIT_
                                                   || classnum == CC_XDIGIT_)))
                {
                    /* We usually have to worry about /d affecting what POSIX
                     * classes match, with special code needed because we won't
                     * know until runtime what all matches.  But there is no
                     * extra work needed under /u and /a; and [:ascii:] is
                     * unaffected by /d; and :digit: and :xdigit: don't have
                     * runtime differences under /d.  So we can special case
                     * these, and avoid some extra work below, and at runtime.
                     * */
                    _invlist_union_maybe_complement_2nd(
                                                     simple_posixes,
                                                      ((AT_LEAST_ASCII_RESTRICTED)
                                                       ? PL_Posix_ptrs[classnum]
                                                       : PL_XPosix_ptrs[classnum]),
                                                     namedclass % 2 != 0,
                                                     &simple_posixes);
                }
                else {  /* Garden variety class.  If is NUPPER, NALPHA, ...
                           complement and use nposixes */
                    SV** posixes_ptr = namedclass % 2 == 0
                                       ? &posixes
                                       : &nposixes;
                    _invlist_union_maybe_complement_2nd(
                                                     *posixes_ptr,
                                                     PL_XPosix_ptrs[classnum],
                                                     namedclass % 2 != 0,
                                                     posixes_ptr);
                }
            }
        } /* end of namedclass \blah */

        SKIP_BRACKETED_WHITE_SPACE(skip_white, RExC_parse, RExC_end);

        /* If 'range' is set, 'value' is the ending of a range--check its
         * validity.  (If value isn't a single code point in the case of a
         * range, we should have figured that out above in the code that
         * catches false ranges).  Later, we will handle each individual code
         * point in the range.  If 'range' isn't set, this could be the
         * beginning of a range, so check for that by looking ahead to see if
         * the next real character to be processed is the range indicator--the
         * minus sign */

        if (range) {
#ifdef EBCDIC
            /* For unicode ranges, we have to test that the Unicode as opposed
             * to the native values are not decreasing.  (Above 255, there is
             * no difference between native and Unicode) */
            if (unicode_range && prevvalue < 255 && value < 255) {
                if (NATIVE_TO_LATIN1(prevvalue) > NATIVE_TO_LATIN1(value)) {
                    goto backwards_range;
                }
            }
            else
#endif
            if (prevvalue > value) /* b-a */ {
                int w;
#ifdef EBCDIC
              backwards_range:
#endif
                w = RExC_parse - rangebegin;
                vFAIL2utf8f(
                    "Invalid [] range \"%" UTF8f "\"",
                    UTF8fARG(UTF, w, rangebegin));
                NOT_REACHED; /* NOTREACHED */
            }
        }
        else {
            prevvalue = value; /* save the beginning of the potential range */
            if (! stop_at_1     /* Can't be a range if parsing just one thing */
                && *RExC_parse == '-')
            {
                char* next_char_ptr = RExC_parse + 1;

                /* Get the next real char after the '-' */
                SKIP_BRACKETED_WHITE_SPACE(skip_white, next_char_ptr, RExC_end);

                /* If the '-' is at the end of the class (just before the ']',
                 * it is a literal minus; otherwise it is a range */
                if (next_char_ptr < RExC_end && *next_char_ptr != ']') {
                    RExC_parse_set(next_char_ptr);

                    /* a bad range like \w-, [:word:]- ? */
                    if (namedclass > OOB_NAMEDCLASS) {
                        if (strict || ckWARN(WARN_REGEXP)) {
                            const int w = RExC_parse >= rangebegin
                                          ?  RExC_parse - rangebegin
                                          : 0;
                            if (strict) {
                                vFAIL4("False [] range \"%*.*s\"",
                                    w, w, rangebegin);
                            }
                            else {
                                vWARN4(RExC_parse,
                                    "False [] range \"%*.*s\"",
                                    w, w, rangebegin);
                            }
                        }
                        cp_list = add_cp_to_invlist(cp_list, '-');
                        element_count++;
                    } else
                        range = 1;	/* yeah, it's a range! */
                    continue;	/* but do it the next time */
                }
            }
        }

        if (namedclass > OOB_NAMEDCLASS) {
            continue;
        }

        /* Here, we have a single value this time through the loop, and
         * <prevvalue> is the beginning of the range, if any; or <value> if
         * not. */

        /* non-Latin1 code point implies unicode semantics. */
        if (value > 255) {
            if (value > MAX_LEGAL_CP && (   value != UV_MAX
                                         || prevvalue > MAX_LEGAL_CP))
            {
                vFAIL(form_cp_too_large_msg(16, NULL, 0, value));
            }
            REQUIRE_UNI_RULES(flagp, 0);
            if (  ! silence_non_portable
                &&  UNICODE_IS_PERL_EXTENDED(value)
                &&  TO_OUTPUT_WARNINGS(RExC_parse))
            {
                ckWARN2_non_literal_string(RExC_parse,
                                           packWARN(WARN_PORTABLE),
                                           PL_extended_cp_format,
                                           value);
            }
        }

        /* Ready to process either the single value, or the completed range.
         * For single-valued non-inverted ranges, we consider the possibility
         * of multi-char folds.  (We made a conscious decision to not do this
         * for the other cases because it can often lead to non-intuitive
         * results.  For example, you have the peculiar case that:
         *  "s s" =~ /^[^\xDF]+$/i => Y
         *  "ss"  =~ /^[^\xDF]+$/i => N
         *
         * See [perl #89750] */
        if (FOLD && allow_mutiple_chars && value == prevvalue) {
            if (    value == LATIN_SMALL_LETTER_SHARP_S
                || (value > 255 && _invlist_contains_cp(PL_HasMultiCharFold,
                                                        value)))
            {
                /* Here <value> is indeed a multi-char fold.  Get what it is */

                U8 foldbuf[UTF8_MAXBYTES_CASE+1];
                STRLEN foldlen;

                UV folded = _to_uni_fold_flags(
                                value,
                                foldbuf,
                                &foldlen,
                                FOLD_FLAGS_FULL | (ASCII_FOLD_RESTRICTED
                                                   ? FOLD_FLAGS_NOMIX_ASCII
                                                   : 0)
                                );

                /* Here, <folded> should be the first character of the
                 * multi-char fold of <value>, with <foldbuf> containing the
                 * whole thing.  But, if this fold is not allowed (because of
                 * the flags), <fold> will be the same as <value>, and should
                 * be processed like any other character, so skip the special
                 * handling */
                if (folded != value) {

                    /* Skip if we are recursed, currently parsing the class
                     * again.  Otherwise add this character to the list of
                     * multi-char folds. */
                    if (! RExC_in_multi_char_class) {
                        STRLEN cp_count = utf8_length(foldbuf,
                                                      foldbuf + foldlen);
                        SV* multi_fold = newSVpvs_flags("", SVs_TEMP);

                        Perl_sv_catpvf(aTHX_ multi_fold, "\\x{%" UVXf "}", value);

                        multi_char_matches
                                        = add_multi_match(multi_char_matches,
                                                          multi_fold,
                                                          cp_count);

                    }

                    /* This element should not be processed further in this
                     * class */
                    element_count--;
                    value = save_value;
                    prevvalue = save_prevvalue;
                    continue;
                }
            }
        }

        if (strict && ckWARN(WARN_REGEXP)) {
            if (range) {

                /* If the range starts above 255, everything is portable and
                 * likely to be so for any forseeable character set, so don't
                 * warn. */
                if (unicode_range && non_portable_endpoint && prevvalue < 256) {
                    vWARN(RExC_parse, "Both or neither range ends should be Unicode");
                }
                else if (prevvalue != value) {

                    /* Under strict, ranges that stop and/or end in an ASCII
                     * printable should have each end point be a portable value
                     * for it (preferably like 'A', but we don't warn if it is
                     * a (portable) Unicode name or code point), and the range
                     * must be all digits or all letters of the same case.
                     * Otherwise, the range is non-portable and unclear as to
                     * what it contains */
                    if (             (isPRINT_A(prevvalue) || isPRINT_A(value))
                        && (          non_portable_endpoint
                            || ! (   (isDIGIT_A(prevvalue) && isDIGIT_A(value))
                                  || (isLOWER_A(prevvalue) && isLOWER_A(value))
                                  || (isUPPER_A(prevvalue) && isUPPER_A(value))
                    ))) {
                        vWARN(RExC_parse, "Ranges of ASCII printables should"
                                          " be some subset of \"0-9\","
                                          " \"A-Z\", or \"a-z\"");
                    }
                    else if (prevvalue >= FIRST_NON_ASCII_DECIMAL_DIGIT) {
                        SSize_t index_start;
                        SSize_t index_final;

                        /* But the nature of Unicode and languages mean we
                         * can't do the same checks for above-ASCII ranges,
                         * except in the case of digit ones.  These should
                         * contain only digits from the same group of 10.  The
                         * ASCII case is handled just above.  Hence here, the
                         * range could be a range of digits.  First some
                         * unlikely special cases.  Grandfather in that a range
                         * ending in 19DA (NEW TAI LUE THAM DIGIT ONE) is bad
                         * if its starting value is one of the 10 digits prior
                         * to it.  This is because it is an alternate way of
                         * writing 19D1, and some people may expect it to be in
                         * that group.  But it is bad, because it won't give
                         * the expected results.  In Unicode 5.2 it was
                         * considered to be in that group (of 11, hence), but
                         * this was fixed in the next version */

                        if (UNLIKELY(value == 0x19DA && prevvalue >= 0x19D0)) {
                            goto warn_bad_digit_range;
                        }
                        else if (UNLIKELY(   prevvalue >= 0x1D7CE
                                          &&     value <= 0x1D7FF))
                        {
                            /* This is the only other case currently in Unicode
                             * where the algorithm below fails.  The code
                             * points just above are the end points of a single
                             * range containing only decimal digits.  It is 5
                             * different series of 0-9.  All other ranges of
                             * digits currently in Unicode are just a single
                             * series.  (And mktables will notify us if a later
                             * Unicode version breaks this.)
                             *
                             * If the range being checked is at most 9 long,
                             * and the digit values represented are in
                             * numerical order, they are from the same series.
                             * */
                            if (         value - prevvalue > 9
                                ||    (((    value - 0x1D7CE) % 10)
                                     <= (prevvalue - 0x1D7CE) % 10))
                            {
                                goto warn_bad_digit_range;
                            }
                        }
                        else {

                            /* For all other ranges of digits in Unicode, the
                             * algorithm is just to check if both end points
                             * are in the same series, which is the same range.
                             * */
                            index_start = _invlist_search(
                                                    PL_XPosix_ptrs[CC_DIGIT_],
                                                    prevvalue);

                            /* Warn if the range starts and ends with a digit,
                             * and they are not in the same group of 10. */
                            if (   index_start >= 0
                                && ELEMENT_RANGE_MATCHES_INVLIST(index_start)
                                && (index_final =
                                    _invlist_search(PL_XPosix_ptrs[CC_DIGIT_],
                                                    value)) != index_start
                                && index_final >= 0
                                && ELEMENT_RANGE_MATCHES_INVLIST(index_final))
                            {
                              warn_bad_digit_range:
                                vWARN(RExC_parse, "Ranges of digits should be"
                                                  " from the same group of"
                                                  " 10");
                            }
                        }
                    }
                }
            }
            if ((! range || prevvalue == value) && non_portable_endpoint) {
                if (isPRINT_A(value)) {
                    char literal[3];
                    unsigned d = 0;
                    if (isBACKSLASHED_PUNCT(value)) {
                        literal[d++] = '\\';
                    }
                    literal[d++] = (char) value;
                    literal[d++] = '\0';

                    vWARN4(RExC_parse,
                           "\"%.*s\" is more clearly written simply as \"%s\"",
                           (int) (RExC_parse - rangebegin),
                           rangebegin,
                           literal
                        );
                }
                else if (isMNEMONIC_CNTRL(value)) {
                    vWARN4(RExC_parse,
                           "\"%.*s\" is more clearly written simply as \"%s\"",
                           (int) (RExC_parse - rangebegin),
                           rangebegin,
                           cntrl_to_mnemonic((U8) value)
                        );
                }
            }
        }

        /* Deal with this element of the class */

#ifndef EBCDIC
        cp_foldable_list = _add_range_to_invlist(cp_foldable_list,
                                                    prevvalue, value);
#else
        /* On non-ASCII platforms, for ranges that span all of 0..255, and ones
         * that don't require special handling, we can just add the range like
         * we do for ASCII platforms */
        if ((UNLIKELY(prevvalue == 0) && value >= 255)
            || ! (prevvalue < 256
                    && (unicode_range
                        || (! non_portable_endpoint
                            && ((isLOWER_A(prevvalue) && isLOWER_A(value))
                                || (isUPPER_A(prevvalue)
                                    && isUPPER_A(value)))))))
        {
            cp_foldable_list = _add_range_to_invlist(cp_foldable_list,
                                                        prevvalue, value);
        }
        else {
            /* Here, requires special handling.  This can be because it is a
             * range whose code points are considered to be Unicode, and so
             * must be individually translated into native, or because its a
             * subrange of 'A-Z' or 'a-z' which each aren't contiguous in
             * EBCDIC, but we have defined them to include only the "expected"
             * upper or lower case ASCII alphabetics.  Subranges above 255 are
             * the same in native and Unicode, so can be added as a range */
            U8 start = NATIVE_TO_LATIN1(prevvalue);
            unsigned j;
            U8 end = (value < 256) ? NATIVE_TO_LATIN1(value) : 255;
            for (j = start; j <= end; j++) {
                cp_foldable_list = add_cp_to_invlist(cp_foldable_list, LATIN1_TO_NATIVE(j));
            }
            if (value > 255) {
                cp_foldable_list = _add_range_to_invlist(cp_foldable_list,
                                                            256, value);
            }
        }
#endif

        range = 0; /* this range (if it was one) is done now */
    } /* End of loop through all the text within the brackets */

    if (   posix_warnings && av_tindex_skip_len_mg(posix_warnings) >= 0) {
        output_posix_warnings(pRExC_state, posix_warnings);
    }

    /* If anything in the class expands to more than one character, we have to
     * deal with them by building up a substitute parse string, and recursively
     * calling reg() on it, instead of proceeding */
    if (multi_char_matches) {
        SV * substitute_parse = newSVpvn_flags("?:", 2, SVs_TEMP);
        I32 cp_count;
        STRLEN len;
        char *save_end = RExC_end;
        char *save_parse = RExC_parse;
        char *save_start = RExC_start;
        Size_t constructed_prefix_len = 0; /* This gives the length of the
                                              constructed portion of the
                                              substitute parse. */
        bool first_time = TRUE;     /* First multi-char occurrence doesn't get
                                       a "|" */
        I32 reg_flags;

        assert(! invert);
        /* Only one level of recursion allowed */
        assert(RExC_copy_start_in_constructed == RExC_precomp);

#if 0   /* Have decided not to deal with multi-char folds in inverted classes,
           because too confusing */
        if (invert) {
            sv_catpvs(substitute_parse, "(?:");
        }
#endif

        /* Look at the longest strings first */
        for (cp_count = av_tindex_skip_len_mg(multi_char_matches);
                        cp_count > 0;
                        cp_count--)
        {

            if (av_exists(multi_char_matches, cp_count)) {
                AV** this_array_ptr;
                SV* this_sequence;

                this_array_ptr = (AV**) av_fetch_simple(multi_char_matches,
                                                 cp_count, FALSE);
                while ((this_sequence = av_pop(*this_array_ptr)) !=
                                                                &PL_sv_undef)
                {
                    if (! first_time) {
                        sv_catpvs(substitute_parse, "|");
                    }
                    first_time = FALSE;

                    sv_catpv(substitute_parse, SvPVX(this_sequence));
                }
            }
        }

        /* If the character class contains anything else besides these
         * multi-character strings, have to include it in recursive parsing */
        if (element_count) {
            bool has_l_bracket = orig_parse > RExC_start && *(orig_parse - 1) == '[';

            sv_catpvs(substitute_parse, "|");
            if (has_l_bracket) {    /* Add an [ if the original had one */
                sv_catpvs(substitute_parse, "[");
            }
            constructed_prefix_len = SvCUR(substitute_parse);
            sv_catpvn(substitute_parse, orig_parse, RExC_parse - orig_parse);

            /* Put in a closing ']' to match any opening one, but not if going
             * off the end, as otherwise we are adding something that really
             * isn't there */
            if (has_l_bracket && RExC_parse < RExC_end) {
                sv_catpvs(substitute_parse, "]");
            }
        }

        sv_catpvs(substitute_parse, ")");
#if 0
        if (invert) {
            /* This is a way to get the parse to skip forward a whole named
             * sequence instead of matching the 2nd character when it fails the
             * first */
            sv_catpvs(substitute_parse, "(*THEN)(*SKIP)(*FAIL)|.)");
        }
#endif

        /* Set up the data structure so that any errors will be properly
         * reported.  See the comments at the definition of
         * REPORT_LOCATION_ARGS for details */
        RExC_copy_start_in_input = (char *) orig_parse;
        RExC_start = SvPV(substitute_parse, len);
        RExC_parse_set( RExC_start );
        RExC_copy_start_in_constructed = RExC_start + constructed_prefix_len;
        RExC_end = RExC_parse + len;
        RExC_in_multi_char_class = 1;

        ret = reg(pRExC_state, 1, &reg_flags, depth+1);

        *flagp |= reg_flags & (HASWIDTH|SIMPLE|POSTPONED|RESTART_PARSE|NEED_UTF8);

        /* And restore so can parse the rest of the pattern */
        RExC_parse_set(save_parse);
        RExC_start = RExC_copy_start_in_constructed = RExC_copy_start_in_input = save_start;
        RExC_end = save_end;
        RExC_in_multi_char_class = 0;
        SvREFCNT_dec_NN(multi_char_matches);
        SvREFCNT_dec(properties);
        SvREFCNT_dec(cp_list);
        SvREFCNT_dec(simple_posixes);
        SvREFCNT_dec(posixes);
        SvREFCNT_dec(nposixes);
        SvREFCNT_dec(cp_foldable_list);
        return ret;
    }

    /* If folding, we calculate all characters that could fold to or from the
     * ones already on the list */
    if (cp_foldable_list) {
        if (FOLD) {
            UV start, end;	/* End points of code point ranges */

            SV* fold_intersection = NULL;
            SV** use_list;

            /* Our calculated list will be for Unicode rules.  For locale
             * matching, we have to keep a separate list that is consulted at
             * runtime only when the locale indicates Unicode rules (and we
             * don't include potential matches in the ASCII/Latin1 range, as
             * any code point could fold to any other, based on the run-time
             * locale).   For non-locale, we just use the general list */
            if (LOC) {
                use_list = &only_utf8_locale_list;
            }
            else {
                use_list = &cp_list;
            }

            /* Only the characters in this class that participate in folds need
             * be checked.  Get the intersection of this class and all the
             * possible characters that are foldable.  This can quickly narrow
             * down a large class */
            _invlist_intersection(PL_in_some_fold, cp_foldable_list,
                                  &fold_intersection);

            /* Now look at the foldable characters in this class individually */
            invlist_iterinit(fold_intersection);
            while (invlist_iternext(fold_intersection, &start, &end)) {
                UV j;
                UV folded;

                /* Look at every character in the range */
                for (j = start; j <= end; j++) {
                    U8 foldbuf[UTF8_MAXBYTES_CASE+1];
                    STRLEN foldlen;
                    unsigned int k;
                    Size_t folds_count;
                    U32 first_fold;
                    const U32 * remaining_folds;

                    if (j < 256) {

                        /* Under /l, we don't know what code points below 256
                         * fold to, except we do know the MICRO SIGN folds to
                         * an above-255 character if the locale is UTF-8, so we
                         * add it to the special list (in *use_list)  Otherwise
                         * we know now what things can match, though some folds
                         * are valid under /d only if the target is UTF-8.
                         * Those go in a separate list */
                        if (      IS_IN_SOME_FOLD_L1(j)
                            && ! (LOC && j != MICRO_SIGN))
                        {

                            /* ASCII is always matched; non-ASCII is matched
                             * only under Unicode rules (which could happen
                             * under /l if the locale is a UTF-8 one */
                            if (isASCII(j) || ! DEPENDS_SEMANTICS) {
                                *use_list = add_cp_to_invlist(*use_list,
                                                            PL_fold_latin1[j]);
                            }
                            else if (j != PL_fold_latin1[j]) {
                                upper_latin1_only_utf8_matches
                                        = add_cp_to_invlist(
                                                upper_latin1_only_utf8_matches,
                                                PL_fold_latin1[j]);
                            }
                        }

                        if (HAS_NONLATIN1_SIMPLE_FOLD_CLOSURE(j)
                            && (! isASCII(j) || ! ASCII_FOLD_RESTRICTED))
                        {
                            add_above_Latin1_folds(pRExC_state,
                                                   (U8) j,
                                                   use_list);
                        }
                        continue;
                    }

                    /* Here is an above Latin1 character.  We don't have the
                     * rules hard-coded for it.  First, get its fold.  This is
                     * the simple fold, as the multi-character folds have been
                     * handled earlier and separated out */
                    folded = _to_uni_fold_flags(j, foldbuf, &foldlen,
                                                        (ASCII_FOLD_RESTRICTED)
                                                        ? FOLD_FLAGS_NOMIX_ASCII
                                                        : 0);

                    /* Single character fold of above Latin1.  Add everything
                     * in its fold closure to the list that this node should
                     * match. */
                    folds_count = _inverse_folds(folded, &first_fold,
                                                    &remaining_folds);
                    for (k = 0; k <= folds_count; k++) {
                        UV c = (k == 0)     /* First time through use itself */
                                ? folded
                                : (k == 1)  /* 2nd time use, the first fold */
                                   ? first_fold

                                     /* Then the remaining ones */
                                   : remaining_folds[k-2];

                        /* /aa doesn't allow folds between ASCII and non- */
                        if ((   ASCII_FOLD_RESTRICTED
                            && (isASCII(c) != isASCII(j))))
                        {
                            continue;
                        }

                        /* Folds under /l which cross the 255/256 boundary are
                         * added to a separate list.  (These are valid only
                         * when the locale is UTF-8.) */
                        if (c < 256 && LOC) {
                            *use_list = add_cp_to_invlist(*use_list, c);
                            continue;
                        }

                        if (isASCII(c) || c > 255 || AT_LEAST_UNI_SEMANTICS)
                        {
                            cp_list = add_cp_to_invlist(cp_list, c);
                        }
                        else {
                            /* Similarly folds involving non-ascii Latin1
                             * characters under /d are added to their list */
                            upper_latin1_only_utf8_matches
                                    = add_cp_to_invlist(
                                                upper_latin1_only_utf8_matches,
                                                c);
                        }
                    }
                }
            }
            SvREFCNT_dec_NN(fold_intersection);
        }

        /* Now that we have finished adding all the folds, there is no reason
         * to keep the foldable list separate */
        _invlist_union(cp_list, cp_foldable_list, &cp_list);
        SvREFCNT_dec_NN(cp_foldable_list);
    }

    /* And combine the result (if any) with any inversion lists from posix
     * classes.  The lists are kept separate up to now because we don't want to
     * fold the classes */
    if (simple_posixes) {   /* These are the classes known to be unaffected by
                               /a, /aa, and /d */
        if (cp_list) {
            _invlist_union(cp_list, simple_posixes, &cp_list);
            SvREFCNT_dec_NN(simple_posixes);
        }
        else {
            cp_list = simple_posixes;
        }
    }
    if (posixes || nposixes) {
        if (! DEPENDS_SEMANTICS) {

            /* For everything but /d, we can just add the current 'posixes' and
             * 'nposixes' to the main list */
            if (posixes) {
                if (cp_list) {
                    _invlist_union(cp_list, posixes, &cp_list);
                    SvREFCNT_dec_NN(posixes);
                }
                else {
                    cp_list = posixes;
                }
            }
            if (nposixes) {
                if (cp_list) {
                    _invlist_union(cp_list, nposixes, &cp_list);
                    SvREFCNT_dec_NN(nposixes);
                }
                else {
                    cp_list = nposixes;
                }
            }
        }
        else {
            /* Under /d, things like \w match upper Latin1 characters only if
             * the target string is in UTF-8.  But things like \W match all the
             * upper Latin1 characters if the target string is not in UTF-8.
             *
             * Handle the case with something like \W separately */
            if (nposixes) {
                SV* only_non_utf8_list = invlist_clone(PL_UpperLatin1, NULL);

                /* A complemented posix class matches all upper Latin1
                 * characters if not in UTF-8.  And it matches just certain
                 * ones when in UTF-8.  That means those certain ones are
                 * matched regardless, so can just be added to the
                 * unconditional list */
                if (cp_list) {
                    _invlist_union(cp_list, nposixes, &cp_list);
                    SvREFCNT_dec_NN(nposixes);
                    nposixes = NULL;
                }
                else {
                    cp_list = nposixes;
                }

                /* Likewise for 'posixes' */
                _invlist_union(posixes, cp_list, &cp_list);
                SvREFCNT_dec(posixes);

                /* Likewise for anything else in the range that matched only
                 * under UTF-8 */
                if (upper_latin1_only_utf8_matches) {
                    _invlist_union(cp_list,
                                   upper_latin1_only_utf8_matches,
                                   &cp_list);
                    SvREFCNT_dec_NN(upper_latin1_only_utf8_matches);
                    upper_latin1_only_utf8_matches = NULL;
                }

                /* If we don't match all the upper Latin1 characters regardless
                 * of UTF-8ness, we have to set a flag to match the rest when
                 * not in UTF-8 */
                _invlist_subtract(only_non_utf8_list, cp_list,
                                  &only_non_utf8_list);
                if (_invlist_len(only_non_utf8_list) != 0) {
                    anyof_flags |= ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared;
                }
                SvREFCNT_dec_NN(only_non_utf8_list);
            }
            else {
                /* Here there were no complemented posix classes.  That means
                 * the upper Latin1 characters in 'posixes' match only when the
                 * target string is in UTF-8.  So we have to add them to the
                 * list of those types of code points, while adding the
                 * remainder to the unconditional list.
                 *
                 * First calculate what they are */
                SV* nonascii_but_latin1_properties = NULL;
                _invlist_intersection(posixes, PL_UpperLatin1,
                                      &nonascii_but_latin1_properties);

                /* And add them to the final list of such characters. */
                _invlist_union(upper_latin1_only_utf8_matches,
                               nonascii_but_latin1_properties,
                               &upper_latin1_only_utf8_matches);

                /* Remove them from what now becomes the unconditional list */
                _invlist_subtract(posixes, nonascii_but_latin1_properties,
                                  &posixes);

                /* And add those unconditional ones to the final list */
                if (cp_list) {
                    _invlist_union(cp_list, posixes, &cp_list);
                    SvREFCNT_dec_NN(posixes);
                    posixes = NULL;
                }
                else {
                    cp_list = posixes;
                }

                SvREFCNT_dec(nonascii_but_latin1_properties);

                /* Get rid of any characters from the conditional list that we
                 * now know are matched unconditionally, which may make that
                 * list empty */
                _invlist_subtract(upper_latin1_only_utf8_matches,
                                  cp_list,
                                  &upper_latin1_only_utf8_matches);
                if (_invlist_len(upper_latin1_only_utf8_matches) == 0) {
                    SvREFCNT_dec_NN(upper_latin1_only_utf8_matches);
                    upper_latin1_only_utf8_matches = NULL;
                }
            }
        }
    }

    /* And combine the result (if any) with any inversion list from properties.
     * The lists are kept separate up to now so that we can distinguish the two
     * in regards to matching above-Unicode.  A run-time warning is generated
     * if a Unicode property is matched against a non-Unicode code point. But,
     * we allow user-defined properties to match anything, without any warning,
     * and we also suppress the warning if there is a portion of the character
     * class that isn't a Unicode property, and which matches above Unicode, \W
     * or [\x{110000}] for example.
     * (Note that in this case, unlike the Posix one above, there is no
     * <upper_latin1_only_utf8_matches>, because having a Unicode property
     * forces Unicode semantics */
    if (properties) {
        if (cp_list) {

            /* If it matters to the final outcome, see if a non-property
             * component of the class matches above Unicode.  If so, the
             * warning gets suppressed.  This is true even if just a single
             * such code point is specified, as, though not strictly correct if
             * another such code point is matched against, the fact that they
             * are using above-Unicode code points indicates they should know
             * the issues involved */
            if (warn_super) {
                warn_super = ! (invert
                               ^ (UNICODE_IS_SUPER(invlist_highest(cp_list))));
            }

            _invlist_union(properties, cp_list, &cp_list);
            SvREFCNT_dec_NN(properties);
        }
        else {
            cp_list = properties;
        }

        if (warn_super) {
            anyof_flags |= ANYOF_WARN_SUPER__shared;

            /* Because an ANYOF node is the only one that warns, this node
             * can't be optimized into something else */
            optimizable = FALSE;
        }
    }

    /* Here, we have calculated what code points should be in the character
     * class.
     *
     * Now we can see about various optimizations.  Fold calculation (which we
     * did above) needs to take place before inversion.  Otherwise /[^k]/i
     * would invert to include K, which under /i would match k, which it
     * shouldn't.  Therefore we can't invert folded locale now, as it won't be
     * folded until runtime */

    /* If we didn't do folding, it's because some information isn't available
     * until runtime; set the run-time fold flag for these  We know to set the
     * flag if we have a non-NULL list for UTF-8 locales, or the class matches
     * at least one 0-255 range code point */
    if (LOC && FOLD) {

        /* Some things on the list might be unconditionally included because of
         * other components.  Remove them, and clean up the list if it goes to
         * 0 elements */
        if (only_utf8_locale_list && cp_list) {
            _invlist_subtract(only_utf8_locale_list, cp_list,
                              &only_utf8_locale_list);

            if (_invlist_len(only_utf8_locale_list) == 0) {
                SvREFCNT_dec_NN(only_utf8_locale_list);
                only_utf8_locale_list = NULL;
            }
        }
        if (    only_utf8_locale_list
            || (    cp_list
                && (   _invlist_contains_cp(cp_list,
                                        LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE)
                    || _invlist_contains_cp(cp_list,
                                            LATIN_SMALL_LETTER_DOTLESS_I))))
        {
            has_runtime_dependency |= HAS_L_RUNTIME_DEPENDENCY;
            anyof_flags |= ANYOFL_FOLD|ANYOF_HAS_EXTRA_RUNTIME_MATCHES;
        }
        else if (cp_list && invlist_lowest(cp_list) < 256) {
            /* If nothing is below 256, has no locale dependency; otherwise it
             * does */
            anyof_flags |= ANYOFL_FOLD;
            has_runtime_dependency |= HAS_L_RUNTIME_DEPENDENCY;

            /* In a Turkish locale these could match, notify the run-time code
             * to check for that */
            if (   _invlist_contains_cp(cp_list, 'I')
                || _invlist_contains_cp(cp_list, 'i'))
            {
                anyof_flags |= ANYOFL_FOLD|ANYOF_HAS_EXTRA_RUNTIME_MATCHES;
            }
        }
    }
    else if (   DEPENDS_SEMANTICS
             && (    upper_latin1_only_utf8_matches
                 || (  anyof_flags
                     & ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared)))
    {
        RExC_seen_d_op = TRUE;
        has_runtime_dependency |= HAS_D_RUNTIME_DEPENDENCY;
    }

    /* Optimize inverted patterns (e.g. [^a-z]) when everything is known at
     * compile time. */
    if (     cp_list
        &&   invert
        && ! has_runtime_dependency)
    {
        _invlist_invert(cp_list);

        /* Clear the invert flag since have just done it here */
        invert = FALSE;
    }

    /* All possible optimizations below still have these characteristics.
     * (Multi-char folds aren't SIMPLE, but they don't get this far in this
     * routine) */
    *flagp |= HASWIDTH|SIMPLE;

    if (ret_invlist) {
        *ret_invlist = cp_list;

        return (cp_list) ? RExC_emit : 0;
    }

    if (anyof_flags & ANYOF_LOCALE_FLAGS) {
        RExC_contains_locale = 1;
    }

    if (optimizable) {

        /* Some character classes are equivalent to other nodes.  Such nodes
         * take up less room, and some nodes require fewer operations to
         * execute, than ANYOF nodes.  EXACTish nodes may be joinable with
         * adjacent nodes to improve efficiency. */
        op = optimize_regclass(pRExC_state, cp_list,
                                            only_utf8_locale_list,
                                            upper_latin1_only_utf8_matches,
                                            has_runtime_dependency,
                                            posixl,
                                            &anyof_flags, &invert, &ret, flagp);
        RETURN_FAIL_ON_RESTART_FLAGP(flagp);

        /* If optimized to something else and emitted, clean up and return */
        if (ret >= 0) {
            SvREFCNT_dec(cp_list);;
            SvREFCNT_dec(only_utf8_locale_list);
            SvREFCNT_dec(upper_latin1_only_utf8_matches);
            return ret;
        }

        /* If no optimization was found, an END was returned and we will now
         * emit an ANYOF */
        if (op == END) {
            op = ANYOF;
        }
    }

    /* Here are going to emit an ANYOF; set the particular type */
    if (op == ANYOF) {
        if (has_runtime_dependency & HAS_D_RUNTIME_DEPENDENCY) {
            op = ANYOFD;
        }
        else if (posixl) {
            op = ANYOFPOSIXL;
        }
        else if (LOC) {
            op = ANYOFL;
        }
    }

    ret = REGNODE_GUTS(pRExC_state, op, REGNODE_ARG_LEN(op));
    FILL_NODE(ret, op);        /* We set the argument later */
    RExC_emit += NODE_STEP_REGNODE + REGNODE_ARG_LEN(op);
    ANYOF_FLAGS(REGNODE_p(ret)) = anyof_flags;

    /* Here, <cp_list> contains all the code points we can determine at
     * compile time that match under all conditions.  Go through it, and
     * for things that belong in the bitmap, put them there, and delete from
     * <cp_list>.  While we are at it, see if everything above 255 is in the
     * list, and if so, set a flag to speed up execution */

    populate_anyof_bitmap_from_invlist(REGNODE_p(ret), &cp_list);

    if (posixl) {
        ANYOF_POSIXL_SET_TO_BITMAP(REGNODE_p(ret), posixl);
    }

    if (invert) {
        ANYOF_FLAGS(REGNODE_p(ret)) |= ANYOF_INVERT;
    }

    /* Here, the bitmap has been populated with all the Latin1 code points that
     * always match.  Can now add to the overall list those that match only
     * when the target string is UTF-8 (<upper_latin1_only_utf8_matches>).
     * */
    if (upper_latin1_only_utf8_matches) {
        if (cp_list) {
            _invlist_union(cp_list,
                           upper_latin1_only_utf8_matches,
                           &cp_list);
            SvREFCNT_dec_NN(upper_latin1_only_utf8_matches);
        }
        else {
            cp_list = upper_latin1_only_utf8_matches;
        }
        ANYOF_FLAGS(REGNODE_p(ret)) |= ANYOF_HAS_EXTRA_RUNTIME_MATCHES;
    }

    set_ANYOF_arg(pRExC_state, REGNODE_p(ret), cp_list,
                  (HAS_NONLOCALE_RUNTIME_PROPERTY_DEFINITION)
                   ? listsv
                   : NULL,
                  only_utf8_locale_list);

    SvREFCNT_dec(cp_list);;
    SvREFCNT_dec(only_utf8_locale_list);
    return ret;
}

STATIC U8
S_optimize_regclass(pTHX_
                    RExC_state_t *pRExC_state,
                    SV * cp_list,
                    SV* only_utf8_locale_list,
                    SV* upper_latin1_only_utf8_matches,
                    const U32 has_runtime_dependency,
                    const U32 posixl,
                    U8  * anyof_flags,
                    bool * invert,
                    regnode_offset * ret,
                    I32 *flagp
                  )
{
    /* This function exists just to make S_regclass() smaller.  It extracts out
     * the code that looks for potential optimizations away from a full generic
     * ANYOF node.  The parameter names are the same as the corresponding
     * variables in S_regclass.
     *
     * It returns the new op (the impossible END one if no optimization found)
     * and sets *ret to any created regnode.  If the new op is sufficiently
     * like plain ANYOF, it leaves *ret unchanged for allocation in S_regclass.
     *
     * Certain of the parameters may be updated as a result of the changes
     * herein */

    U8 op = END;    /* The returned node-type, initialized to an impossible
                      one. */
    UV value = 0;
    PERL_UINT_FAST8_T i;
    UV partial_cp_count = 0;
    UV start[MAX_FOLD_FROMS+1] = { 0 }; /* +1 for the folded-to char */
    UV   end[MAX_FOLD_FROMS+1] = { 0 };
    bool single_range = FALSE;
    UV lowest_cp = 0, highest_cp = 0;

    PERL_ARGS_ASSERT_OPTIMIZE_REGCLASS;

    if (cp_list) { /* Count the code points in enough ranges that we would see
                      all the ones possible in any fold in this version of
                      Unicode */

        invlist_iterinit(cp_list);
        for (i = 0; i <= MAX_FOLD_FROMS; i++) {
            if (! invlist_iternext(cp_list, &start[i], &end[i])) {
                break;
            }
            partial_cp_count += end[i] - start[i] + 1;
        }

        if (i == 1) {
            single_range = TRUE;
        }
        invlist_iterfinish(cp_list);

        /* If we know at compile time that this matches every possible code
         * point, any run-time dependencies don't matter */
        if (start[0] == 0 && end[0] == UV_MAX) {
            if (*invert) {
                goto return_OPFAIL;
            }
            else {
                goto return_SANY;
            }
        }

        /* Use a clearer mnemonic for below */
        lowest_cp = start[0];

        highest_cp = invlist_highest(cp_list);
    }

    /* Similarly, for /l posix classes, if both a class and its complement
     * match, any run-time dependencies don't matter */
    if (posixl) {
        int namedclass;
        for (namedclass = 0; namedclass < ANYOF_POSIXL_MAX; namedclass += 2) {
            if (   POSIXL_TEST(posixl, namedclass)      /* class */
                && POSIXL_TEST(posixl, namedclass + 1)) /* its complement */
            {
                if (*invert) {
                    goto return_OPFAIL;
                }
                goto return_SANY;
            }
        }

        /* For well-behaved locales, some classes are subsets of others, so
         * complementing the subset and including the non-complemented superset
         * should match everything, like [\D[:alnum:]], and
         * [[:^alpha:][:alnum:]], but some implementations of locales are
         * buggy, and khw thinks its a bad idea to have optimization change
         * behavior, even if it avoids an OS bug in a given case */

#define isSINGLE_BIT_SET(n) isPOWER_OF_2(n)

        /* If is a single posix /l class, can optimize to just that op.  Such a
         * node will not match anything in the Latin1 range, as that is not
         * determinable until runtime, but will match whatever the class does
         * outside that range.  (Note that some classes won't match anything
         * outside the range, like [:ascii:]) */
        if (   isSINGLE_BIT_SET(posixl)
            && (partial_cp_count == 0 || lowest_cp > 255))
        {
            U8 classnum;
            SV * class_above_latin1 = NULL;
            bool already_inverted;
            bool are_equivalent;


            namedclass = single_1bit_pos32(posixl);
            classnum = namedclass_to_classnum(namedclass);

            /* The named classes are such that the inverted number is one
             * larger than the non-inverted one */
            already_inverted = namedclass - classnum_to_namedclass(classnum);

            /* Create an inversion list of the official property, inverted if
             * the constructed node list is inverted, and restricted to only
             * the above latin1 code points, which are the only ones known at
             * compile time */
            _invlist_intersection_maybe_complement_2nd(
                                                PL_AboveLatin1,
                                                PL_XPosix_ptrs[classnum],
                                                already_inverted,
                                                &class_above_latin1);
            are_equivalent = _invlistEQ(class_above_latin1, cp_list, FALSE);
            SvREFCNT_dec_NN(class_above_latin1);

            if (are_equivalent) {

                /* Resolve the run-time inversion flag with this possibly
                 * inverted class */
                *invert = *invert ^ already_inverted;

                op = POSIXL + *invert * (NPOSIXL - POSIXL);
                *ret = reg_node(pRExC_state, op);
                FLAGS(REGNODE_p(*ret)) = classnum;
                return op;
            }
        }
    }

    /* khw can't think of any other possible transformation involving these. */
    if (has_runtime_dependency & HAS_USER_DEFINED_PROPERTY) {
        return END;
    }

    if (! has_runtime_dependency) {

        /* If the list is empty, nothing matches.  This happens, for example,
         * when a Unicode property that doesn't match anything is the only
         * element in the character class (perluniprops.pod notes such
         * properties). */
        if (partial_cp_count == 0) {
            if (*invert) {
                goto return_SANY;
            }
            else {
                goto return_OPFAIL;
            }
        }

        /* If matches everything but \n */
        if (   start[0] == 0 && end[0] == '\n' - 1
            && start[1] == '\n' + 1 && end[1] == UV_MAX)
        {
            assert (! *invert);
            op = REG_ANY;
            *ret = reg_node(pRExC_state, op);
            MARK_NAUGHTY(1);
            return op;
        }
    }

    /* Next see if can optimize classes that contain just a few code points
     * into an EXACTish node.  The reason to do this is to let the optimizer
     * join this node with adjacent EXACTish ones, and ANYOF nodes require
     * runtime conversion to code point from UTF-8, which we'd like to avoid.
     *
     * An EXACTFish node can be generated even if not under /i, and vice versa.
     * But care must be taken.  An EXACTFish node has to be such that it only
     * matches precisely the code points in the class, but we want to generate
     * the least restrictive one that does that, to increase the odds of being
     * able to join with an adjacent node.  For example, if the class contains
     * [kK], we have to make it an EXACTFAA node to prevent the KELVIN SIGN
     * from matching.  Whether we are under /i or not is irrelevant in this
     * case.  Less obvious is the pattern qr/[\x{02BC}]n/i.  U+02BC is MODIFIER
     * LETTER APOSTROPHE. That is supposed to match the single character U+0149
     * LATIN SMALL LETTER N PRECEDED BY APOSTROPHE.  And so even though there
     * is no simple fold that includes \X{02BC}, there is a multi-char fold
     * that does, and so the node generated for it must be an EXACTFish one.
     * On the other hand qr/:/i should generate a plain EXACT node since the
     * colon participates in no fold whatsoever, and having it be EXACT tells
     * the optimizer the target string cannot match unless it has a colon in
     * it. */
    if (   ! posixl
        && ! *invert

            /* Only try if there are no more code points in the class than in
             * the max possible fold */
        &&   inRANGE(partial_cp_count, 1, MAX_FOLD_FROMS + 1))
    {
        /* We can always make a single code point class into an EXACTish node.
         * */
        if (partial_cp_count == 1 && ! upper_latin1_only_utf8_matches) {
            if (LOC) {

                /* Here is /l:  Use EXACTL, except if there is a fold not known
                 * until runtime so shows as only a single code point here.
                 * For code points above 255, we know which can cause problems
                 * by having a potential fold to the Latin1 range. */
                if (  ! FOLD
                    || (     lowest_cp > 255
                        && ! is_PROBLEMATIC_LOCALE_FOLD_cp(lowest_cp)))
                {
                    op = EXACTL;
                }
                else {
                    op = EXACTFL;
                }
            }
            else if (! FOLD) { /* Not /l and not /i */
                op = (lowest_cp < 256) ? EXACT : EXACT_REQ8;
            }
            else if (lowest_cp < 256) { /* /i, not /l, and the code point is
                                          small */

                /* Under /i, it gets a little tricky.  A code point that
                 * doesn't participate in a fold should be an EXACT node.  We
                 * know this one isn't the result of a simple fold, or there'd
                 * be more than one code point in the list, but it could be
                 * part of a multi-character fold.  In that case we better not
                 * create an EXACT node, as we would wrongly be telling the
                 * optimizer that this code point must be in the target string,
                 * and that is wrong.  This is because if the sequence around
                 * this code point forms a multi-char fold, what needs to be in
                 * the string could be the code point that folds to the
                 * sequence.
                 *
                 * This handles the case of below-255 code points, as we have
                 * an easy look up for those.  The next clause handles the
                 * above-256 one */
                op = IS_IN_SOME_FOLD_L1(lowest_cp)
                     ? EXACTFU
                     : EXACT;
            }
            else {  /* /i, larger code point.  Since we are under /i, and have
                       just this code point, we know that it can't fold to
                       something else, so PL_InMultiCharFold applies to it */
                op = (_invlist_contains_cp(PL_InMultiCharFold, lowest_cp))
                         ? EXACTFU_REQ8
                         : EXACT_REQ8;
                }

                value = lowest_cp;
        }
        else if (  ! (has_runtime_dependency & ~HAS_D_RUNTIME_DEPENDENCY)
                 && _invlist_contains_cp(PL_in_some_fold, lowest_cp))
        {
            /* Here, the only runtime dependency, if any, is from /d, and the
             * class matches more than one code point, and the lowest code
             * point participates in some fold.  It might be that the other
             * code points are /i equivalent to this one, and hence they would
             * be representable by an EXACTFish node.  Above, we eliminated
             * classes that contain too many code points to be EXACTFish, with
             * the test for MAX_FOLD_FROMS
             *
             * First, special case the ASCII fold pairs, like 'B' and 'b'.  We
             * do this because we have EXACTFAA at our disposal for the ASCII
             * range */
            if (partial_cp_count == 2 && isASCII(lowest_cp)) {

                /* The only ASCII characters that participate in folds are
                 * alphabetics */
                assert(isALPHA(lowest_cp));
                if (   end[0] == start[0]   /* First range is a single
                                               character, so 2nd exists */
                    && isALPHA_FOLD_EQ(start[0], start[1]))
                {
                    /* Here, is part of an ASCII fold pair */

                    if (   ASCII_FOLD_RESTRICTED
                        || HAS_NONLATIN1_SIMPLE_FOLD_CLOSURE(lowest_cp))
                    {
                        /* If the second clause just above was true, it means
                         * we can't be under /i, or else the list would have
                         * included more than this fold pair.  Therefore we
                         * have to exclude the possibility of whatever else it
                         * is that folds to these, by using EXACTFAA */
                        op = EXACTFAA;
                    }
                    else if (HAS_NONLATIN1_FOLD_CLOSURE(lowest_cp)) {

                        /* Here, there's no simple fold that lowest_cp is part
                         * of, but there is a multi-character one.  If we are
                         * not under /i, we want to exclude that possibility;
                         * if under /i, we want to include it */
                        op = (FOLD) ? EXACTFU : EXACTFAA;
                    }
                    else {

                        /* Here, the only possible fold lowest_cp participates in
                         * is with start[1].  /i or not isn't relevant */
                        op = EXACTFU;
                    }

                    value = toFOLD(lowest_cp);
                }
            }
            else if (  ! upper_latin1_only_utf8_matches
                     || (   _invlist_len(upper_latin1_only_utf8_matches) == 2
                         && PL_fold_latin1[
                           invlist_highest(upper_latin1_only_utf8_matches)]
                         == lowest_cp))
            {
                /* Here, the smallest character is non-ascii or there are more
                 * than 2 code points matched by this node.  Also, we either
                 * don't have /d UTF-8 dependent matches, or if we do, they
                 * look like they could be a single character that is the fold
                 * of the lowest one is in the always-match list.  This test
                 * quickly excludes most of the false positives when there are
                 * /d UTF-8 depdendent matches.  These are like LATIN CAPITAL
                 * LETTER A WITH GRAVE matching LATIN SMALL LETTER A WITH GRAVE
                 * iff the target string is UTF-8.  (We don't have to worry
                 * above about exceeding the array bounds of PL_fold_latin1[]
                 * because any code point in 'upper_latin1_only_utf8_matches'
                 * is below 256.)
                 *
                 * EXACTFAA would apply only to pairs (hence exactly 2 code
                 * points) in the ASCII range, so we can't use it here to
                 * artificially restrict the fold domain, so we check if the
                 * class does or does not match some EXACTFish node.  Further,
                 * if we aren't under /i, and and the folded-to character is
                 * part of a multi-character fold, we can't do this
                 * optimization, as the sequence around it could be that
                 * multi-character fold, and we don't here know the context, so
                 * we have to assume it is that multi-char fold, to prevent
                 * potential bugs.
                 *
                 * To do the general case, we first find the fold of the lowest
                 * code point (which may be higher than that lowest unfolded
                 * one), then find everything that folds to it.  (The data
                 * structure we have only maps from the folded code points, so
                 * we have to do the earlier step.) */

                Size_t foldlen;
                U8 foldbuf[UTF8_MAXBYTES_CASE];
                UV folded = _to_uni_fold_flags(lowest_cp, foldbuf, &foldlen, 0);
                U32 first_fold;
                const U32 * remaining_folds;
                Size_t folds_to_this_cp_count = _inverse_folds(
                                                            folded,
                                                            &first_fold,
                                                            &remaining_folds);
                Size_t folds_count = folds_to_this_cp_count + 1;
                SV * fold_list = _new_invlist(folds_count);
                unsigned int i;

                /* If there are UTF-8 dependent matches, create a temporary
                 * list of what this node matches, including them. */
                SV * all_cp_list = NULL;
                SV ** use_this_list = &cp_list;

                if (upper_latin1_only_utf8_matches) {
                    all_cp_list = _new_invlist(0);
                    use_this_list = &all_cp_list;
                    _invlist_union(cp_list,
                                   upper_latin1_only_utf8_matches,
                                   use_this_list);
                }

                /* Having gotten everything that participates in the fold
                 * containing the lowest code point, we turn that into an
                 * inversion list, making sure everything is included. */
                fold_list = add_cp_to_invlist(fold_list, lowest_cp);
                fold_list = add_cp_to_invlist(fold_list, folded);
                if (folds_to_this_cp_count > 0) {
                    fold_list = add_cp_to_invlist(fold_list, first_fold);
                    for (i = 0; i + 1 < folds_to_this_cp_count; i++) {
                        fold_list = add_cp_to_invlist(fold_list,
                                                    remaining_folds[i]);
                    }
                }

                /* If the fold list is identical to what's in this ANYOF node,
                 * the node can be represented by an EXACTFish one instead */
                if (_invlistEQ(*use_this_list, fold_list,
                               0 /* Don't complement */ )
                ) {

                    /* But, we have to be careful, as mentioned above.  Just
                     * the right sequence of characters could match this if it
                     * is part of a multi-character fold.  That IS what we want
                     * if we are under /i.  But it ISN'T what we want if not
                     * under /i, as it could match when it shouldn't.  So, when
                     * we aren't under /i and this character participates in a
                     * multi-char fold, we don't optimize into an EXACTFish
                     * node.  So, for each case below we have to check if we
                     * are folding, and if not, if it is not part of a
                     * multi-char fold.  */
                    if (lowest_cp > 255) {    /* Highish code point */
                        if (FOLD || ! _invlist_contains_cp(
                                                   PL_InMultiCharFold, folded))
                        {
                            op = (LOC)
                                 ? EXACTFLU8
                                 : (ASCII_FOLD_RESTRICTED)
                                   ? EXACTFAA
                                   : EXACTFU_REQ8;
                            value = folded;
                        }
                    }   /* Below, the lowest code point < 256 */
                    else if (    FOLD
                             &&  folded == 's'
                             &&  DEPENDS_SEMANTICS)
                    {   /* An EXACTF node containing a single character 's',
                           can be an EXACTFU if it doesn't get joined with an
                           adjacent 's' */
                        op = EXACTFU_S_EDGE;
                        value = folded;
                    }
                    else if (     FOLD
                             || ! HAS_NONLATIN1_FOLD_CLOSURE(lowest_cp))
                    {
                        if (upper_latin1_only_utf8_matches) {
                            op = EXACTF;

                            /* We can't use the fold, as that only matches
                             * under UTF-8 */
                            value = lowest_cp;
                        }
                        else if (     UNLIKELY(lowest_cp == MICRO_SIGN)
                                 && ! UTF)
                        {   /* EXACTFUP is a special node for this character */
                            op = (ASCII_FOLD_RESTRICTED)
                                 ? EXACTFAA
                                 : EXACTFUP;
                            value = MICRO_SIGN;
                        }
                        else if (     ASCII_FOLD_RESTRICTED
                                 && ! isASCII(lowest_cp))
                        {   /* For ASCII under /iaa, we can use EXACTFU below
                             */
                            op = EXACTFAA;
                            value = folded;
                        }
                        else {
                            op = EXACTFU;
                            value = folded;
                        }
                    }
                }

                SvREFCNT_dec_NN(fold_list);
                SvREFCNT_dec(all_cp_list);
            }
        }

        if (op != END) {
            U8 len;

            /* Here, we have calculated what EXACTish node to use.  Have to
             * convert to UTF-8 if not already there */
            if (value > 255) {
                if (! UTF) {
                    SvREFCNT_dec(cp_list);;
                    REQUIRE_UTF8(flagp);
                }

                /* This is a kludge to the special casing issues with this
                 * ligature under /aa.  FB05 should fold to FB06, but the call
                 * above to _to_uni_fold_flags() didn't find this, as it didn't
                 * use the /aa restriction in order to not miss other folds
                 * that would be affected.  This is the only instance likely to
                 * ever be a problem in all of Unicode.  So special case it. */
                if (   value == LATIN_SMALL_LIGATURE_LONG_S_T
                    && ASCII_FOLD_RESTRICTED)
                {
                    value = LATIN_SMALL_LIGATURE_ST;
                }
            }

            len = (UTF) ? UVCHR_SKIP(value) : 1;

            *ret = REGNODE_GUTS(pRExC_state, op, len);
            FILL_NODE(*ret, op);
            RExC_emit += NODE_STEP_REGNODE + STR_SZ(len);
            setSTR_LEN(REGNODE_p(*ret), len);
            if (len == 1) {
                *STRINGs(REGNODE_p(*ret)) = (U8) value;
            }
            else {
                uvchr_to_utf8((U8 *) STRINGs(REGNODE_p(*ret)), value);
            }

            return op;
        }
    }

    if (! has_runtime_dependency) {

        /* See if this can be turned into an ANYOFM node.  Think about the bit
         * patterns in two different bytes.  In some positions, the bits in
         * each will be 1; and in other positions both will be 0; and in some
         * positions the bit will be 1 in one byte, and 0 in the other.  Let
         * 'n' be the number of positions where the bits differ.  We create a
         * mask which has exactly 'n' 0 bits, each in a position where the two
         * bytes differ.  Now take the set of all bytes that when ANDed with
         * the mask yield the same result.  That set has 2**n elements, and is
         * representable by just two 8 bit numbers: the result and the mask.
         * Importantly, matching the set can be vectorized by creating a word
         * full of the result bytes, and a word full of the mask bytes,
         * yielding a significant speed up.  Here, see if this node matches
         * such a set.  As a concrete example consider [01], and the byte
         * representing '0' which is 0x30 on ASCII machines.  It has the bits
         * 0011 0000.  Take the mask 1111 1110.  If we AND 0x31 and 0x30 with
         * that mask we get 0x30.  Any other bytes ANDed yield something else.
         * So [01], which is a common usage, is optimizable into ANYOFM, and
         * can benefit from the speed up.  We can only do this on UTF-8
         * invariant bytes, because they have the same bit patterns under UTF-8
         * as not. */
        PERL_UINT_FAST8_T inverted = 0;

        /* Highest possible UTF-8 invariant is 7F on ASCII platforms; FF on
         * EBCDIC */
        const PERL_UINT_FAST8_T max_permissible
                                    = nBIT_UMAX(7 + ONE_IF_EBCDIC_ZERO_IF_NOT);

        /* If doesn't fit the criteria for ANYOFM, invert and try again.  If
         * that works we will instead later generate an NANYOFM, and invert
         * back when through */
        if (highest_cp > max_permissible) {
            _invlist_invert(cp_list);
            inverted = 1;
        }

        if (invlist_highest(cp_list) <= max_permissible) {
            UV this_start, this_end;
            UV lowest_cp = UV_MAX;  /* init'ed to suppress compiler warn */
            U8 bits_differing = 0;
            Size_t full_cp_count = 0;
            bool first_time = TRUE;

            /* Go through the bytes and find the bit positions that differ */
            invlist_iterinit(cp_list);
            while (invlist_iternext(cp_list, &this_start, &this_end)) {
                unsigned int i = this_start;

                if (first_time) {
                    if (! UVCHR_IS_INVARIANT(i)) {
                        goto done_anyofm;
                    }

                    first_time = FALSE;
                    lowest_cp = this_start;

                    /* We have set up the code point to compare with.  Don't
                     * compare it with itself */
                    i++;
                }

                /* Find the bit positions that differ from the lowest code
                 * point in the node.  Keep track of all such positions by
                 * OR'ing */
                for (; i <= this_end; i++) {
                    if (! UVCHR_IS_INVARIANT(i)) {
                        goto done_anyofm;
                    }

                    bits_differing  |= i ^ lowest_cp;
                }

                full_cp_count += this_end - this_start + 1;
            }

            /* At the end of the loop, we count how many bits differ from the
             * bits in lowest code point, call the count 'd'.  If the set we
             * found contains 2**d elements, it is the closure of all code
             * points that differ only in those bit positions.  To convince
             * yourself of that, first note that the number in the closure must
             * be a power of 2, which we test for.  The only way we could have
             * that count and it be some differing set, is if we got some code
             * points that don't differ from the lowest code point in any
             * position, but do differ from each other in some other position.
             * That means one code point has a 1 in that position, and another
             * has a 0.  But that would mean that one of them differs from the
             * lowest code point in that position, which possibility we've
             * already excluded.  */
            if (  (inverted || full_cp_count > 1)
                && full_cp_count == 1U << PL_bitcount[bits_differing])
            {
                U8 ANYOFM_mask;

                op = ANYOFM + inverted;;

                /* We need to make the bits that differ be 0's */
                ANYOFM_mask = ~ bits_differing; /* This goes into FLAGS */

                /* The argument is the lowest code point */
                *ret = reg1node(pRExC_state, op, lowest_cp);
                FLAGS(REGNODE_p(*ret)) = ANYOFM_mask;
            }

          done_anyofm:
            invlist_iterfinish(cp_list);
        }

        if (inverted) {
            _invlist_invert(cp_list);
        }

        if (op != END) {
            return op;
        }

        /* XXX We could create an ANYOFR_LOW node here if we saved above if all
         * were invariants, it wasn't inverted, and there is a single range.
         * This would be faster than some of the posix nodes we create below
         * like /\d/a, but would be twice the size.  Without having actually
         * measured the gain, khw doesn't think the tradeoff is really worth it
         * */
    }

    if (! (*anyof_flags & ANYOF_LOCALE_FLAGS)) {
        PERL_UINT_FAST8_T type;
        SV * intersection = NULL;
        SV* d_invlist = NULL;

        /* See if this matches any of the POSIX classes.  The POSIXA and POSIXD
         * ones are about the same speed as ANYOF ops, but take less room; the
         * ones that have above-Latin1 code point matches are somewhat faster
         * than ANYOF. */

        for (type = POSIXA; type >= POSIXD; type--) {
            int posix_class;

            if (type == POSIXL) {   /* But not /l posix classes */
                continue;
            }

            for (posix_class = 0;
                 posix_class <= HIGHEST_REGCOMP_DOT_H_SYNC_;
                 posix_class++)
            {
                SV** our_code_points = &cp_list;
                SV** official_code_points;
                int try_inverted;

                if (type == POSIXA) {
                    official_code_points = &PL_Posix_ptrs[posix_class];
                }
                else {
                    official_code_points = &PL_XPosix_ptrs[posix_class];
                }

                /* Skip non-existent classes of this type.  e.g. \v only has an
                 * entry in PL_XPosix_ptrs */
                if (! *official_code_points) {
                    continue;
                }

                /* Try both the regular class, and its inversion */
                for (try_inverted = 0; try_inverted < 2; try_inverted++) {
                    bool this_inverted = *invert ^ try_inverted;

                    if (type != POSIXD) {

                        /* This class that isn't /d can't match if we have /d
                         * dependencies */
                        if (has_runtime_dependency
                                                & HAS_D_RUNTIME_DEPENDENCY)
                        {
                            continue;
                        }
                    }
                    else /* is /d */ if (! this_inverted) {

                        /* /d classes don't match anything non-ASCII below 256
                         * unconditionally (which cp_list contains) */
                        _invlist_intersection(cp_list, PL_UpperLatin1,
                                                       &intersection);
                        if (_invlist_len(intersection) != 0) {
                            continue;
                        }

                        SvREFCNT_dec(d_invlist);
                        d_invlist = invlist_clone(cp_list, NULL);

                        /* But under UTF-8 it turns into using /u rules.  Add
                         * the things it matches under these conditions so that
                         * we check below that these are identical to what the
                         * tested class should match */
                        if (upper_latin1_only_utf8_matches) {
                            _invlist_union(
                                        d_invlist,
                                        upper_latin1_only_utf8_matches,
                                        &d_invlist);
                        }
                        our_code_points = &d_invlist;
                    }
                    else {  /* POSIXD, inverted.  If this doesn't have this
                               flag set, it isn't /d. */
                        if (! ( *anyof_flags
                               & ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared))
                        {
                            continue;
                        }

                        our_code_points = &cp_list;
                    }

                    /* Here, have weeded out some things.  We want to see if
                     * the list of characters this node contains
                     * ('*our_code_points') precisely matches those of the
                     * class we are currently checking against
                     * ('*official_code_points'). */
                    if (_invlistEQ(*our_code_points,
                                   *official_code_points,
                                   try_inverted))
                    {
                        /* Here, they precisely match.  Optimize this ANYOF
                         * node into its equivalent POSIX one of the correct
                         * type, possibly inverted.
                         *
                         * Some of these nodes match a single range of
                         * characters (or [:alpha:] matches two parallel ranges
                         * on ASCII platforms).  The array lookup at execution
                         * time could be replaced by a range check for such
                         * nodes.  But regnodes are a finite resource, and the
                         * possible performance boost isn't large, so this
                         * hasn't been done.  An attempt to use just one node
                         * (and its inverse) to encompass all such cases was
                         * made in d62feba66bf43f35d092bb026694f927e9f94d38.
                         * But the shifting/masking it used ended up being
                         * slower than the array look up, so it was reverted */
                        op = (try_inverted)
                            ? type + NPOSIXA - POSIXA
                            : type;
                        *ret = reg_node(pRExC_state, op);
                        FLAGS(REGNODE_p(*ret)) = posix_class;
                        SvREFCNT_dec(d_invlist);
                        SvREFCNT_dec(intersection);
                        return op;
                    }
                }
            }
        }
        SvREFCNT_dec(d_invlist);
        SvREFCNT_dec(intersection);
    }

    /* If it is a single contiguous range, ANYOFR is an efficient regnode, both
     * in size and speed.  Currently, a 20 bit range base (smallest code point
     * in the range), and a 12 bit maximum delta are packed into a 32 bit word.
     * This allows for using it on all of the Unicode code points except for
     * the highest plane, which is only for private use code points.  khw
     * doubts that a bigger delta is likely in real world applications */
    if (     single_range
        && ! has_runtime_dependency
        &&   *anyof_flags == 0
        &&   start[0] < (1 << ANYOFR_BASE_BITS)
        &&   end[0] - start[0]
                < ((1U << (sizeof(ARG1u_LOC(NULL))
                               * CHARBITS - ANYOFR_BASE_BITS))))

    {
        U8 low_utf8[UTF8_MAXBYTES+1];
        U8 high_utf8[UTF8_MAXBYTES+1];

        op = ANYOFR;
        *ret = reg1node(pRExC_state, op,
                        (start[0] | (end[0] - start[0]) << ANYOFR_BASE_BITS));

        /* Place the lowest UTF-8 start byte in the flags field, so as to allow
         * efficient ruling out at run time of many possible inputs.  */
        (void) uvchr_to_utf8(low_utf8, start[0]);
        (void) uvchr_to_utf8(high_utf8, end[0]);

        /* If all code points share the same first byte, this can be an
         * ANYOFRb.  Otherwise store the lowest UTF-8 start byte which can
         * quickly rule out many inputs at run-time without having to compute
         * the code point from UTF-8.  For EBCDIC, we use I8, as not doing that
         * transformation would not rule out nearly so many things */
        if (low_utf8[0] == high_utf8[0]) {
            op = ANYOFRb;
            OP(REGNODE_p(*ret)) = op;
            ANYOF_FLAGS(REGNODE_p(*ret)) = low_utf8[0];
        }
        else {
            ANYOF_FLAGS(REGNODE_p(*ret)) = NATIVE_UTF8_TO_I8(low_utf8[0]);
        }

        return op;
    }

    /* If didn't find an optimization and there is no need for a bitmap,
     * of the lowest code points, optimize to indicate that */
    if (     lowest_cp >= NUM_ANYOF_CODE_POINTS
        && ! LOC
        && ! upper_latin1_only_utf8_matches
        &&   *anyof_flags == 0)
    {
        U8 low_utf8[UTF8_MAXBYTES+1];
        UV highest_cp = invlist_highest(cp_list);

        /* Currently the maximum allowed code point by the system is IV_MAX.
         * Higher ones are reserved for future internal use.  This particular
         * regnode can be used for higher ones, but we can't calculate the code
         * point of those.  IV_MAX suffices though, as it will be a large first
         * byte */
        Size_t low_len = uvchr_to_utf8(low_utf8, MIN(lowest_cp, IV_MAX))
                       - low_utf8;

        /* We store the lowest possible first byte of the UTF-8 representation,
         * using the flags field.  This allows for quick ruling out of some
         * inputs without having to convert from UTF-8 to code point.  For
         * EBCDIC, we use I8, as not doing that transformation would not rule
         * out nearly so many things */
        *anyof_flags = NATIVE_UTF8_TO_I8(low_utf8[0]);

        op = ANYOFH;

        /* If the first UTF-8 start byte for the highest code point in the
         * range is suitably small, we may be able to get an upper bound as
         * well */
        if (highest_cp <= IV_MAX) {
            U8 high_utf8[UTF8_MAXBYTES+1];
            Size_t high_len = uvchr_to_utf8(high_utf8, highest_cp) - high_utf8;

            /* If the lowest and highest are the same, we can get an exact
             * first byte instead of a just minimum or even a sequence of exact
             * leading bytes.  We signal these with different regnodes */
            if (low_utf8[0] == high_utf8[0]) {
                Size_t len = find_first_differing_byte_pos(low_utf8,
                                                           high_utf8,
                                                   MIN(low_len, high_len));
                if (len == 1) {

                    /* No need to convert to I8 for EBCDIC as this is an exact
                     * match */
                    *anyof_flags = low_utf8[0];

                    if (high_len == 2) {
                        /* If the elements matched all have a 2-byte UTF-8
                         * representation, with the first byte being the same,
                         * we can use a compact, fast regnode. capable of
                         * matching any combination of continuation byte
                         * patterns.
                         *
                         * (A similar regnode could be created for the Latin1
                         * range; the complication being that it could match
                         * non-UTF8 targets.  The internal bitmap would serve
                         * both cases; with some extra code in regexec.c) */
                        op = ANYOFHbbm;
                        *ret = REGNODE_GUTS(pRExC_state, op, REGNODE_ARG_LEN(op));
                        FILL_NODE(*ret, op);
                        FIRST_BYTE((struct regnode_bbm *) REGNODE_p(*ret)) = low_utf8[0],

                        /* The 64 bit (or 32 on EBCCDIC) map can be looked up
                         * directly based on the continuation byte, without
                         * needing to convert to code point */
                        populate_bitmap_from_invlist(
                            cp_list,

                            /* The base code point is from the start byte */
                            TWO_BYTE_UTF8_TO_NATIVE(low_utf8[0],
                                                    UTF_CONTINUATION_MARK | 0),

                            ((struct regnode_bbm *) REGNODE_p(*ret))->bitmap,
                            REGNODE_BBM_BITMAP_LEN);
                        RExC_emit += NODE_STEP_REGNODE + REGNODE_ARG_LEN(op);
                        return op;
                    }
                    else {
                        op = ANYOFHb;
                    }
                }
                else {
                    op = ANYOFHs;
                    *ret = REGNODE_GUTS(pRExC_state, op,
                                       REGNODE_ARG_LEN(op) + STR_SZ(len));
                    FILL_NODE(*ret, op);
                    STR_LEN_U8((struct regnode_anyofhs *) REGNODE_p(*ret))
                                                                    = len;
                    Copy(low_utf8,  /* Add the common bytes */
                    ((struct regnode_anyofhs *) REGNODE_p(*ret))->string,
                       len, U8);
                    RExC_emit = REGNODE_OFFSET(REGNODE_AFTER_varies(REGNODE_p(*ret)));
                    set_ANYOF_arg(pRExC_state, REGNODE_p(*ret), cp_list,
                                              NULL, only_utf8_locale_list);
                    return op;
                }
            }
            else if (NATIVE_UTF8_TO_I8(high_utf8[0]) <= MAX_ANYOF_HRx_BYTE) {

                /* Here, the high byte is not the same as the low, but is small
                 * enough that its reasonable to have a loose upper bound,
                 * which is packed in with the strict lower bound.  See
                 * comments at the definition of MAX_ANYOF_HRx_BYTE.  On EBCDIC
                 * platforms, I8 is used.  On ASCII platforms I8 is the same
                 * thing as UTF-8 */

                U8 bits = 0;
                U8 max_range_diff = MAX_ANYOF_HRx_BYTE - *anyof_flags;
                U8 range_diff = NATIVE_UTF8_TO_I8(high_utf8[0])
                            - *anyof_flags;

                if (range_diff <= max_range_diff / 8) {
                    bits = 3;
                }
                else if (range_diff <= max_range_diff / 4) {
                    bits = 2;
                }
                else if (range_diff <= max_range_diff / 2) {
                    bits = 1;
                }
                *anyof_flags = (*anyof_flags - 0xC0) << 2 | bits;
                op = ANYOFHr;
            }
        }
    }

    return op;

  return_OPFAIL:
    op = OPFAIL;
    *ret = reg1node(pRExC_state, op, 0);
    return op;

  return_SANY:
    op = SANY;
    *ret = reg_node(pRExC_state, op);
    MARK_NAUGHTY(1);
    return op;
}

#undef HAS_NONLOCALE_RUNTIME_PROPERTY_DEFINITION

void
Perl_set_ANYOF_arg(pTHX_ RExC_state_t* const pRExC_state,
                regnode* const node,
                SV* const cp_list,
                SV* const runtime_defns,
                SV* const only_utf8_locale_list)
{
    /* Sets the arg field of an ANYOF-type node 'node', using information about
     * the node passed-in.  If only the bitmap is needed to determine what
     * matches, the arg is set appropriately to either
     *      1) ANYOF_MATCHES_NONE_OUTSIDE_BITMAP_VALUE
     *      2) ANYOF_MATCHES_ALL_OUTSIDE_BITMAP_VALUE
     *
     * Otherwise, it sets the argument to the count returned by reg_add_data(),
     * having allocated and stored an array, av, as follows:
     *  av[0] stores the inversion list defining this class as far as known at
     *        this time, or PL_sv_undef if nothing definite is now known.
     *  av[1] stores the inversion list of code points that match only if the
     *        current locale is UTF-8, or if none, PL_sv_undef if there is an
     *        av[2], or no entry otherwise.
     *  av[2] stores the list of user-defined properties whose subroutine
     *        definitions aren't known at this time, or no entry if none. */

    UV n;

    PERL_ARGS_ASSERT_SET_ANYOF_ARG;

    /* If this is set, the final disposition won't be known until runtime, so
     * we can't do any of the compile time optimizations */
    if (! runtime_defns) {

        /* On plain ANYOF nodes without the possibility of a runtime locale
         * making a difference, maybe there's no information to be gleaned
         * except for what's in the bitmap */
        if (REGNODE_TYPE(OP(node)) == ANYOF && ! only_utf8_locale_list) {

            /* There are two such cases:
             *  1)  there is no list of code points matched outside the bitmap
             */
            if (! cp_list) {
                ARG1u_SET(node, ANYOF_MATCHES_NONE_OUTSIDE_BITMAP_VALUE);
                return;
            }

            /*  2)  the list indicates everything outside the bitmap matches */
            if (   invlist_highest(cp_list) == UV_MAX
                && invlist_highest_range_start(cp_list)
                                                       <= NUM_ANYOF_CODE_POINTS)
            {
                ARG1u_SET(node, ANYOF_MATCHES_ALL_OUTSIDE_BITMAP_VALUE);
                return;
            }

            /* In all other cases there are things outside the bitmap that we
             * may need to check at runtime. */
        }

        /* Here, we have resolved all the possible run-time matches, and they
         * are stored in one or both of two possible lists.  (While some match
         * only under certain runtime circumstances, we know all the possible
         * ones for each such circumstance.)
         *
         * It may very well be that the pattern being compiled contains an
         * identical class, already encountered.  Reusing that class here saves
         * space.  Look through all classes so far encountered. */
        U32 existing_items = RExC_rxi->data ? RExC_rxi->data->count : 0;
        for (unsigned int i = 0; i < existing_items; i++) {

            /* Only look at auxiliary data of this type */
            if (RExC_rxi->data->what[i] != 's') {
                continue;
            }

            SV * const rv = MUTABLE_SV(RExC_rxi->data->data[i]);
            AV * const av = MUTABLE_AV(SvRV(rv));

            /* If the already encountered class has data that won't be known
             * until runtime (stored in the final element of the array), we
             * can't share */
            if (av_top_index(av) > ONLY_LOCALE_MATCHES_INDEX) {
                continue;
            }

            SV ** stored_cp_list_ptr = av_fetch(av, INVLIST_INDEX,
                                                false /* no lvalue */);

            /* The new and the existing one both have to have or both not
             * have this element, for this one to duplicate that one */
            if (cBOOL(cp_list) != cBOOL(stored_cp_list_ptr)) {
                continue;
            }

            /* If the inversion lists aren't equivalent, can't share */
            if (cp_list && ! _invlistEQ(cp_list,
                                        *stored_cp_list_ptr,
                                        FALSE /* don't complement */))
            {
                continue;
            }

            /* Similarly for the other list */
            SV ** stored_only_utf8_locale_list_ptr = av_fetch(
                                                av,
                                                ONLY_LOCALE_MATCHES_INDEX,
                                                false /* no lvalue */);
            if (   cBOOL(only_utf8_locale_list)
                != cBOOL(stored_only_utf8_locale_list_ptr))
            {
                continue;
            }

            if (only_utf8_locale_list && ! _invlistEQ(
                                         only_utf8_locale_list,
                                         *stored_only_utf8_locale_list_ptr,
                                         FALSE /* don't complement */))
            {
                continue;
            }

            /* Here, the existence and contents of both compile-time lists
             * are identical between the new and existing data.  Re-use the
             * existing one */
            ARG1u_SET(node, i);
            return;
        } /* end of loop through existing classes */
    }

    /* Here, we need to create a new auxiliary data element; either because
     * this doesn't duplicate an existing one, or we can't tell at this time if
     * it eventually will */

    AV * const av = newAV();
    SV *rv;

    if (cp_list) {
        av_store_simple(av, INVLIST_INDEX, SvREFCNT_inc_NN(cp_list));
    }

    /* (Note that if any of this changes, the size calculations in
     * S_optimize_regclass() might need to be updated.) */

    if (only_utf8_locale_list) {
        av_store_simple(av, ONLY_LOCALE_MATCHES_INDEX,
                                       SvREFCNT_inc_NN(only_utf8_locale_list));
    }

    if (runtime_defns) {
        av_store_simple(av, DEFERRED_USER_DEFINED_INDEX,
                     SvREFCNT_inc_NN(runtime_defns));
    }

    rv = newRV_noinc(MUTABLE_SV(av));
    n = reg_add_data(pRExC_state, STR_WITH_LEN("s"));
    RExC_rxi->data->data[n] = (void*)rv;
    ARG1u_SET(node, n);
}

SV *

#if !defined(PERL_IN_XSUB_RE) || defined(PLUGGABLE_RE_EXTENSION)
Perl_get_regclass_aux_data(pTHX_ const regexp *prog, const regnode* node, bool doinit, SV** listsvp, SV** only_utf8_locale_ptr, SV** output_invlist)
#else
Perl_get_re_gclass_aux_data(pTHX_ const regexp *prog, const regnode* node, bool doinit, SV** listsvp, SV** only_utf8_locale_ptr, SV** output_invlist)
#endif

{
    /* For internal core use only.
     * Returns the inversion list for the input 'node' in the regex 'prog'.
     * If <doinit> is 'true', will attempt to create the inversion list if not
     *    already done.  If it is created, it will add to the normal inversion
     *    list any that comes from user-defined properties.  It croaks if this
     *    is called before such a list is ready to be generated, that is when a
     *    user-defined property has been declared, buyt still not yet defined.
     * If <listsvp> is non-null, will return the printable contents of the
     *    property definition.  This can be used to get debugging information
     *    even before the inversion list exists, by calling this function with
     *    'doinit' set to false, in which case the components that will be used
     *    to eventually create the inversion list are returned  (in a printable
     *    form).
     * If <only_utf8_locale_ptr> is not NULL, it is where this routine is to
     *    store an inversion list of code points that should match only if the
     *    execution-time locale is a UTF-8 one.
     * If <output_invlist> is not NULL, it is where this routine is to store an
     *    inversion list of the code points that would be instead returned in
     *    <listsvp> if this were NULL.  Thus, what gets output in <listsvp>
     *    when this parameter is used, is just the non-code point data that
     *    will go into creating the inversion list.  This currently should be just
     *    user-defined properties whose definitions were not known at compile
     *    time.  Using this parameter allows for easier manipulation of the
     *    inversion list's data by the caller.  It is illegal to call this
     *    function with this parameter set, but not <listsvp>
     *
     * Tied intimately to how S_set_ANYOF_arg sets up the data structure.  Note
     * that, in spite of this function's name, the inversion list it returns
     * may include the bitmap data as well */

    SV *si  = NULL;         /* Input initialization string */
    SV* invlist = NULL;

    RXi_GET_DECL_NULL(prog, progi);
    const struct reg_data * const data = prog ? progi->data : NULL;

#if !defined(PERL_IN_XSUB_RE) || defined(PLUGGABLE_RE_EXTENSION)
    PERL_ARGS_ASSERT_GET_REGCLASS_AUX_DATA;
#else
    PERL_ARGS_ASSERT_GET_RE_GCLASS_AUX_DATA;
#endif
    assert(! output_invlist || listsvp);

    if (data && data->count) {
        const U32 n = ARG1u(node);

        if (data->what[n] == 's') {
            SV * const rv = MUTABLE_SV(data->data[n]);
            AV * const av = MUTABLE_AV(SvRV(rv));
            SV **const ary = AvARRAY(av);

            invlist = ary[INVLIST_INDEX];

            if (av_tindex_skip_len_mg(av) >= ONLY_LOCALE_MATCHES_INDEX) {
                *only_utf8_locale_ptr = ary[ONLY_LOCALE_MATCHES_INDEX];
            }

            if (av_tindex_skip_len_mg(av) >= DEFERRED_USER_DEFINED_INDEX) {
                si = ary[DEFERRED_USER_DEFINED_INDEX];
            }

            if (doinit && (si || invlist)) {
                if (si) {
                    bool user_defined;
                    SV * msg = newSVpvs_flags("", SVs_TEMP);

                    SV * prop_definition = handle_user_defined_property(
                            "", 0, FALSE,   /* There is no \p{}, \P{} */
                            SvPVX_const(si)[1] - '0',   /* /i or not has been
                                                           stored here for just
                                                           this occasion */
                            TRUE,           /* run time */
                            FALSE,          /* This call must find the defn */
                            si,             /* The property definition  */
                            &user_defined,
                            msg,
                            0               /* base level call */
                           );

                    if (SvCUR(msg)) {
                        assert(prop_definition == NULL);

                        Perl_croak(aTHX_ "%" UTF8f,
                                UTF8fARG(SvUTF8(msg), SvCUR(msg), SvPVX(msg)));
                    }

                    if (invlist) {
                        _invlist_union(invlist, prop_definition, &invlist);
                        SvREFCNT_dec_NN(prop_definition);
                    }
                    else {
                        invlist = prop_definition;
                    }

                    STATIC_ASSERT_STMT(ONLY_LOCALE_MATCHES_INDEX == 1 + INVLIST_INDEX);
                    STATIC_ASSERT_STMT(DEFERRED_USER_DEFINED_INDEX == 1 + ONLY_LOCALE_MATCHES_INDEX);

                    ary[INVLIST_INDEX] = invlist;
                    av_fill(av, (ary[ONLY_LOCALE_MATCHES_INDEX])
                                 ? ONLY_LOCALE_MATCHES_INDEX
                                 : INVLIST_INDEX);
                    si = NULL;
                }
            }
        }
    }

    /* If requested, return a printable version of what this ANYOF node matches
     * */
    if (listsvp) {
        SV* matches_string = NULL;

        /* This function can be called at compile-time, before everything gets
         * resolved, in which case we return the currently best available
         * information, which is the string that will eventually be used to do
         * that resolving, 'si' */
        if (si) {
            /* Here, we only have 'si' (and possibly some passed-in data in
             * 'invlist', which is handled below)  If the caller only wants
             * 'si', use that.  */
            if (! output_invlist) {
                matches_string = newSVsv(si);
            }
            else {
                /* But if the caller wants an inversion list of the node, we
                 * need to parse 'si' and place as much as possible in the
                 * desired output inversion list, making 'matches_string' only
                 * contain the currently unresolvable things */
                const char *si_string = SvPVX(si);
                STRLEN remaining = SvCUR(si);
                UV prev_cp = 0;
                U8 count = 0;

                /* Ignore everything before and including the first new-line */
                si_string = (const char *) memchr(si_string, '\n', SvCUR(si));
                assert (si_string != NULL);
                si_string++;
                remaining = SvPVX(si) + SvCUR(si) - si_string;

                while (remaining > 0) {

                    /* The data consists of just strings defining user-defined
                     * property names, but in prior incarnations, and perhaps
                     * somehow from pluggable regex engines, it could still
                     * hold hex code point definitions, all of which should be
                     * legal (or it wouldn't have gotten this far).  Each
                     * component of a range would be separated by a tab, and
                     * each range by a new-line.  If these are found, instead
                     * add them to the inversion list */
                    I32 grok_flags =  PERL_SCAN_SILENT_ILLDIGIT
                                     |PERL_SCAN_SILENT_NON_PORTABLE;
                    STRLEN len = remaining;
                    UV cp = grok_hex(si_string, &len, &grok_flags, NULL);

                    /* If the hex decode routine found something, it should go
                     * up to the next \n */
                    if (   *(si_string + len) == '\n') {
                        if (count) {    /* 2nd code point on line */
                            *output_invlist = _add_range_to_invlist(*output_invlist, prev_cp, cp);
                        }
                        else {
                            *output_invlist = add_cp_to_invlist(*output_invlist, cp);
                        }
                        count = 0;
                        goto prepare_for_next_iteration;
                    }

                    /* If the hex decode was instead for the lower range limit,
                     * save it, and go parse the upper range limit */
                    if (*(si_string + len) == '\t') {
                        assert(count == 0);

                        prev_cp = cp;
                        count = 1;
                      prepare_for_next_iteration:
                        si_string += len + 1;
                        remaining -= len + 1;
                        continue;
                    }

                    /* Here, didn't find a legal hex number.  Just add the text
                     * from here up to the next \n, omitting any trailing
                     * markers. */

                    remaining -= len;
                    len = strcspn(si_string,
                                        DEFERRED_COULD_BE_OFFICIAL_MARKERs "\n");
                    remaining -= len;
                    if (matches_string) {
                        sv_catpvn(matches_string, si_string, len);
                    }
                    else {
                        matches_string = newSVpvn(si_string, len);
                    }
                    sv_catpvs(matches_string, " ");

                    si_string += len;
                    if (   remaining
                        && UCHARAT(si_string)
                                            == DEFERRED_COULD_BE_OFFICIAL_MARKERc)
                    {
                        si_string++;
                        remaining--;
                    }
                    if (remaining && UCHARAT(si_string) == '\n') {
                        si_string++;
                        remaining--;
                    }
                } /* end of loop through the text */

                assert(matches_string);
                if (SvCUR(matches_string)) {  /* Get rid of trailing blank */
                    SvCUR_set(matches_string, SvCUR(matches_string) - 1);
                }
            } /* end of has an 'si' */
        }

        /* Add the stuff that's already known */
        if (invlist) {

            /* Again, if the caller doesn't want the output inversion list, put
             * everything in 'matches-string' */
            if (! output_invlist) {
                if ( ! matches_string) {
                    matches_string = newSVpvs("\n");
                }
                sv_catsv(matches_string, invlist_contents(invlist,
                                                  TRUE /* traditional style */
                                                  ));
            }
            else if (! *output_invlist) {
                *output_invlist = invlist_clone(invlist, NULL);
            }
            else {
                _invlist_union(*output_invlist, invlist, output_invlist);
            }
        }

        *listsvp = matches_string;
    }

    return invlist;
}

/* reg_skipcomment()

   Absorbs an /x style # comment from the input stream,
   returning a pointer to the first character beyond the comment, or if the
   comment terminates the pattern without anything following it, this returns
   one past the final character of the pattern (in other words, RExC_end) and
   sets the REG_RUN_ON_COMMENT_SEEN flag.

   Note it's the callers responsibility to ensure that we are
   actually in /x mode

*/

PERL_STATIC_INLINE char*
S_reg_skipcomment(RExC_state_t *pRExC_state, char* p)
{
    PERL_ARGS_ASSERT_REG_SKIPCOMMENT;

    assert(*p == '#');

    while (p < RExC_end) {
        if (*(++p) == '\n') {
            return p+1;
        }
    }

    /* we ran off the end of the pattern without ending the comment, so we have
     * to add an \n when wrapping */
    RExC_seen |= REG_RUN_ON_COMMENT_SEEN;
    return p;
}

STATIC void
S_skip_to_be_ignored_text(pTHX_ RExC_state_t *pRExC_state,
                                char ** p,
                                const bool force_to_xmod
                         )
{
    /* If the text at the current parse position '*p' is a '(?#...)' comment,
     * or if we are under /x or 'force_to_xmod' is TRUE, and the text at '*p'
     * is /x whitespace, advance '*p' so that on exit it points to the first
     * byte past all such white space and comments */

    const bool use_xmod = force_to_xmod || (RExC_flags & RXf_PMf_EXTENDED);

    PERL_ARGS_ASSERT_SKIP_TO_BE_IGNORED_TEXT;

    assert( ! UTF || UTF8_IS_INVARIANT(**p) || UTF8_IS_START(**p));

    for (;;) {
        if (RExC_end - (*p) >= 3
            && *(*p)     == '('
            && *(*p + 1) == '?'
            && *(*p + 2) == '#')
        {
            while (*(*p) != ')') {
                if ((*p) == RExC_end)
                    FAIL("Sequence (?#... not terminated");
                (*p)++;
            }
            (*p)++;
            continue;
        }

        if (use_xmod) {
            const char * save_p = *p;
            while ((*p) < RExC_end) {
                STRLEN len;
                if ((len = is_PATWS_safe((*p), RExC_end, UTF))) {
                    (*p) += len;
                }
                else if (*(*p) == '#') {
                    (*p) = reg_skipcomment(pRExC_state, (*p));
                }
                else {
                    break;
                }
            }
            if (*p != save_p) {
                continue;
            }
        }

        break;
    }

    return;
}

/* nextchar()

   Advances the parse position by one byte, unless that byte is the beginning
   of a '(?#...)' style comment, or is /x whitespace and /x is in effect.  In
   those two cases, the parse position is advanced beyond all such comments and
   white space.

   This is the UTF, (?#...), and /x friendly way of saying RExC_parse_inc_by(1).
*/

STATIC void
S_nextchar(pTHX_ RExC_state_t *pRExC_state)
{
    PERL_ARGS_ASSERT_NEXTCHAR;

    if (RExC_parse < RExC_end) {
        assert(   ! UTF
               || UTF8_IS_INVARIANT(*RExC_parse)
               || UTF8_IS_START(*RExC_parse));

        RExC_parse_inc_safe();

        skip_to_be_ignored_text(pRExC_state, &RExC_parse,
                                FALSE /* Don't force /x */ );
    }
}

STATIC void
S_change_engine_size(pTHX_ RExC_state_t *pRExC_state, const Ptrdiff_t size)
{
    /* 'size' is the delta number of smallest regnode equivalents to add or
     * subtract from the current memory allocated to the regex engine being
     * constructed. */

    PERL_ARGS_ASSERT_CHANGE_ENGINE_SIZE;

    RExC_size += size;

    Renewc(RExC_rxi,
           sizeof(regexp_internal) + (RExC_size + 1) * sizeof(regnode),
                                                /* +1 for REG_MAGIC */
           char,
           regexp_internal);
    if ( RExC_rxi == NULL )
        FAIL("Regexp out of space");
    RXi_SET(RExC_rx, RExC_rxi);

    RExC_emit_start = RExC_rxi->program;
    if (size > 0) {
        Zero(REGNODE_p(RExC_emit), size, regnode);
    }
}

STATIC regnode_offset
S_regnode_guts(pTHX_ RExC_state_t *pRExC_state, const STRLEN extra_size)
{
    /* Allocate a regnode that is (1 + extra_size) times as big as the
     * smallest regnode worth of space, and also aligns and increments
     * RExC_size appropriately.
     *
     * It returns the regnode's offset into the regex engine program */

    const regnode_offset ret = RExC_emit;

    PERL_ARGS_ASSERT_REGNODE_GUTS;

    SIZE_ALIGN(RExC_size);
    change_engine_size(pRExC_state, (Ptrdiff_t) 1 + extra_size);
    NODE_ALIGN_FILL(REGNODE_p(ret));
    return(ret);
}

#ifdef DEBUGGING

STATIC regnode_offset
S_regnode_guts_debug(pTHX_ RExC_state_t *pRExC_state, const U8 op, const STRLEN extra_size) {
    PERL_ARGS_ASSERT_REGNODE_GUTS_DEBUG;
    assert(extra_size >= REGNODE_ARG_LEN(op) || REGNODE_TYPE(op) == ANYOF);
    return S_regnode_guts(aTHX_ pRExC_state, extra_size);
}

#endif



/*
- reg_node - emit a node
*/
STATIC regnode_offset /* Location. */
S_reg_node(pTHX_ RExC_state_t *pRExC_state, U8 op)
{
    const regnode_offset ret = REGNODE_GUTS(pRExC_state, op, REGNODE_ARG_LEN(op));
    regnode_offset ptr = ret;

    PERL_ARGS_ASSERT_REG_NODE;

    assert(REGNODE_ARG_LEN(op) == 0);

    FILL_ADVANCE_NODE(ptr, op);
    RExC_emit = ptr;
    return(ret);
}

/*
- reg1node - emit a node with an argument
*/
STATIC regnode_offset /* Location. */
S_reg1node(pTHX_ RExC_state_t *pRExC_state, U8 op, U32 arg)
{
    const regnode_offset ret = REGNODE_GUTS(pRExC_state, op, REGNODE_ARG_LEN(op));
    regnode_offset ptr = ret;

    PERL_ARGS_ASSERT_REG1NODE;

    /* ANYOF are special cased to allow non-length 1 args */
    assert(REGNODE_ARG_LEN(op) == 1);

    FILL_ADVANCE_NODE_ARG1u(ptr, op, arg);
    RExC_emit = ptr;
    return(ret);
}

/*
- regpnode - emit a temporary node with a SV* argument
*/
STATIC regnode_offset /* Location. */
S_regpnode(pTHX_ RExC_state_t *pRExC_state, U8 op, SV * arg)
{
    const regnode_offset ret = REGNODE_GUTS(pRExC_state, op, REGNODE_ARG_LEN(op));
    regnode_offset ptr = ret;

    PERL_ARGS_ASSERT_REGPNODE;

    FILL_ADVANCE_NODE_ARGp(ptr, op, arg);
    RExC_emit = ptr;
    return(ret);
}

STATIC regnode_offset
S_reg2node(pTHX_ RExC_state_t *pRExC_state, const U8 op, const U32 arg1, const I32 arg2)
{
    /* emit a node with U32 and I32 arguments */

    const regnode_offset ret = REGNODE_GUTS(pRExC_state, op, REGNODE_ARG_LEN(op));
    regnode_offset ptr = ret;

    PERL_ARGS_ASSERT_REG2NODE;

    assert(REGNODE_ARG_LEN(op) == 2);

    FILL_ADVANCE_NODE_2ui_ARG(ptr, op, arg1, arg2);
    RExC_emit = ptr;
    return(ret);
}

/*
- reginsert - insert an operator in front of already-emitted operand
*
* That means that on exit 'operand' is the offset of the newly inserted
* operator, and the original operand has been relocated.
*
* IMPORTANT NOTE - it is the *callers* responsibility to correctly
* set up NEXT_OFF() of the inserted node if needed. Something like this:
*
*   reginsert(pRExC, OPFAIL, orig_emit, depth+1);
*   NEXT_OFF(REGNODE_p(orig_emit)) = REGNODE_ARG_LEN(OPFAIL) + NODE_STEP_REGNODE;
*
* ALSO NOTE - FLAGS(newly-inserted-operator) will be set to 0 as well.
*/
STATIC void
S_reginsert(pTHX_ RExC_state_t *pRExC_state, const U8 op,
                  const regnode_offset operand, const U32 depth)
{
    regnode *src;
    regnode *dst;
    regnode *place;
    const int offset = REGNODE_ARG_LEN((U8)op);
    const int size = NODE_STEP_REGNODE + offset;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGINSERT;
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(depth);
    DEBUG_PARSE_FMT("inst"," - %s", REGNODE_NAME(op));
    assert(!RExC_study_started); /* I believe we should never use reginsert once we have started
                                    studying. If this is wrong then we need to adjust RExC_recurse
                                    below like we do with RExC_open_parens/RExC_close_parens. */
    change_engine_size(pRExC_state, (Ptrdiff_t) size);
    src = REGNODE_p(RExC_emit);
    RExC_emit += size;
    dst = REGNODE_p(RExC_emit);

    /* If we are in a "count the parentheses" pass, the numbers are unreliable,
     * and [perl #133871] shows this can lead to problems, so skip this
     * realignment of parens until a later pass when they are reliable */
    if (! IN_PARENS_PASS && RExC_open_parens) {
        int paren;
        /*DEBUG_PARSE_FMT("inst"," - %" IVdf, (IV)RExC_npar);*/
        /* remember that RExC_npar is rex->nparens + 1,
         * iow it is 1 more than the number of parens seen in
         * the pattern so far. */
        for ( paren=0 ; paren < RExC_npar ; paren++ ) {
            /* note, RExC_open_parens[0] is the start of the
             * regex, it can't move. RExC_close_parens[0] is the end
             * of the regex, it *can* move. */
            if ( paren && RExC_open_parens[paren] >= operand ) {
                /*DEBUG_PARSE_FMT("open"," - %d", size);*/
                RExC_open_parens[paren] += size;
            } else {
                /*DEBUG_PARSE_FMT("open"," - %s","ok");*/
            }
            if ( RExC_close_parens[paren] >= operand ) {
                /*DEBUG_PARSE_FMT("close"," - %d", size);*/
                RExC_close_parens[paren] += size;
            } else {
                /*DEBUG_PARSE_FMT("close"," - %s","ok");*/
            }
        }
    }
    if (RExC_end_op)
        RExC_end_op += size;

    while (src > REGNODE_p(operand)) {
        StructCopy(--src, --dst, regnode);
    }

    place = REGNODE_p(operand);	/* Op node, where operand used to be. */
    src = place + 1; /* NOT REGNODE_AFTER! */
    FLAGS(place) = 0;
    FILL_NODE(operand, op);

    /* Zero out any arguments in the new node */
    Zero(src, offset, regnode);
}

/*
- regtail - set the next-pointer at the end of a node chain of p to val.  If
            that value won't fit in the space available, instead returns FALSE.
            (Except asserts if we can't fit in the largest space the regex
            engine is designed for.)
- SEE ALSO: regtail_study
*/
STATIC bool
S_regtail(pTHX_ RExC_state_t * pRExC_state,
                const regnode_offset p,
                const regnode_offset val,
                const U32 depth)
{
    regnode_offset scan;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGTAIL;
#ifndef DEBUGGING
    PERL_UNUSED_ARG(depth);
#endif

    /* The final node in the chain is the first one with a nonzero next pointer
     * */
    scan = (regnode_offset) p;
    for (;;) {
        regnode * const temp = regnext(REGNODE_p(scan));
        DEBUG_PARSE_r({
            DEBUG_PARSE_MSG((scan==p ? "tail" : ""));
            regprop(RExC_rx, RExC_mysv, REGNODE_p(scan), NULL, pRExC_state);
            Perl_re_printf( aTHX_  "~ %s (%zu) %s %s\n",
                SvPV_nolen_const(RExC_mysv), scan,
                    (temp == NULL ? "->" : ""),
                    (temp == NULL ? REGNODE_NAME(OP(REGNODE_p(val))) : "")
            );
        });
        if (temp == NULL)
            break;
        scan = REGNODE_OFFSET(temp);
    }

    /* Populate this node's next pointer */
    assert(val >= scan);
    if (REGNODE_OFF_BY_ARG(OP(REGNODE_p(scan)))) {
        assert((UV) (val - scan) <= U32_MAX);
        ARG1u_SET(REGNODE_p(scan), val - scan);
    }
    else {
        if (val - scan > U16_MAX) {
            /* Populate this with something that won't loop and will likely
             * lead to a crash if the caller ignores the failure return, and
             * execution continues */
            NEXT_OFF(REGNODE_p(scan)) = U16_MAX;
            return FALSE;
        }
        NEXT_OFF(REGNODE_p(scan)) = val - scan;
    }

    return TRUE;
}

#ifdef DEBUGGING
/*
- regtail_study - set the next-pointer at the end of a node chain of p to val.
- Look for optimizable sequences at the same time.
- currently only looks for EXACT chains.

This is experimental code. The idea is to use this routine to perform
in place optimizations on branches and groups as they are constructed,
with the long term intention of removing optimization from study_chunk so
that it is purely analytical.

Currently only used when in DEBUG mode. The macro REGTAIL_STUDY() is used
to control which is which.

This used to return a value that was ignored.  It was a problem that it is
#ifdef'd to be another function that didn't return a value.  khw has changed it
so both currently return a pass/fail return.

*/
/* TODO: All four parms should be const */

STATIC bool
S_regtail_study(pTHX_ RExC_state_t *pRExC_state, regnode_offset p,
                      const regnode_offset val, U32 depth)
{
    regnode_offset scan;
    U8 exact = PSEUDO;
#ifdef EXPERIMENTAL_INPLACESCAN
    I32 min = 0;
#endif
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGTAIL_STUDY;


    /* Find last node. */

    scan = p;
    for (;;) {
        regnode * const temp = regnext(REGNODE_p(scan));
#ifdef EXPERIMENTAL_INPLACESCAN
        if (REGNODE_TYPE(OP(REGNODE_p(scan))) == EXACT) {
            bool unfolded_multi_char;	/* Unexamined in this routine */
            if (join_exact(pRExC_state, scan, &min,
                           &unfolded_multi_char, 1, REGNODE_p(val), depth+1))
                return TRUE; /* Was return EXACT */
        }
#endif
        if ( exact ) {
            if (REGNODE_TYPE(OP(REGNODE_p(scan))) == EXACT) {
                if (exact == PSEUDO )
                    exact= OP(REGNODE_p(scan));
                else if (exact != OP(REGNODE_p(scan)) )
                    exact= 0;
            }
            else if (OP(REGNODE_p(scan)) != NOTHING) {
                exact= 0;
            }
        }
        DEBUG_PARSE_r({
            DEBUG_PARSE_MSG((scan==p ? "tsdy" : ""));
            regprop(RExC_rx, RExC_mysv, REGNODE_p(scan), NULL, pRExC_state);
            Perl_re_printf( aTHX_  "~ %s (%zu) -> %s\n",
                SvPV_nolen_const(RExC_mysv),
                scan,
                REGNODE_NAME(exact));
        });
        if (temp == NULL)
            break;
        scan = REGNODE_OFFSET(temp);
    }
    DEBUG_PARSE_r({
        DEBUG_PARSE_MSG("");
        regprop(RExC_rx, RExC_mysv, REGNODE_p(val), NULL, pRExC_state);
        Perl_re_printf( aTHX_
                      "~ attach to %s (%" IVdf ") offset to %" IVdf "\n",
                      SvPV_nolen_const(RExC_mysv),
                      (IV)val,
                      (IV)(val - scan)
        );
    });
    if (REGNODE_OFF_BY_ARG(OP(REGNODE_p(scan)))) {
        assert((UV) (val - scan) <= U32_MAX);
        ARG1u_SET(REGNODE_p(scan), val - scan);
    }
    else {
        if (val - scan > U16_MAX) {
            /* Populate this with something that won't loop and will likely
             * lead to a crash if the caller ignores the failure return, and
             * execution continues */
            NEXT_OFF(REGNODE_p(scan)) = U16_MAX;
            return FALSE;
        }
        NEXT_OFF(REGNODE_p(scan)) = val - scan;
    }

    return TRUE; /* Was 'return exact' */
}
#endif

SV*
Perl_get_ANYOFM_contents(pTHX_ const regnode * n) {

    /* Returns an inversion list of all the code points matched by the
     * ANYOFM/NANYOFM node 'n' */

    SV * cp_list = _new_invlist(-1);
    const U8 lowest = (U8) ARG1u(n);
    unsigned int i;
    U8 count = 0;
    U8 needed = 1U << PL_bitcount[ (U8) ~ FLAGS(n)];

    PERL_ARGS_ASSERT_GET_ANYOFM_CONTENTS;

    /* Starting with the lowest code point, any code point that ANDed with the
     * mask yields the lowest code point is in the set */
    for (i = lowest; i <= 0xFF; i++) {
        if ((i & FLAGS(n)) == ARG1u(n)) {
            cp_list = add_cp_to_invlist(cp_list, i);
            count++;

            /* We know how many code points (a power of two) that are in the
             * set.  No use looking once we've got that number */
            if (count >= needed) break;
        }
    }

    if (OP(n) == NANYOFM) {
        _invlist_invert(cp_list);
    }
    return cp_list;
}

SV *
Perl_get_ANYOFHbbm_contents(pTHX_ const regnode * n) {
    PERL_ARGS_ASSERT_GET_ANYOFHBBM_CONTENTS;

    SV * cp_list = NULL;
    populate_invlist_from_bitmap(
              ((struct regnode_bbm *) n)->bitmap,
              REGNODE_BBM_BITMAP_LEN * CHARBITS,
              &cp_list,

              /* The base cp is from the start byte plus a zero continuation */
              TWO_BYTE_UTF8_TO_NATIVE(FIRST_BYTE((struct regnode_bbm *) n),
                                      UTF_CONTINUATION_MARK | 0));
    return cp_list;
}



SV *
Perl_re_intuit_string(pTHX_ REGEXP * const r)
{				/* Assume that RE_INTUIT is set */
    /* Returns an SV containing a string that must appear in the target for it
     * to match, or NULL if nothing is known that must match.
     *
     * CAUTION: the SV can be freed during execution of the regex engine */

    struct regexp *const prog = ReANY(r);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_RE_INTUIT_STRING;
    PERL_UNUSED_CONTEXT;

    DEBUG_COMPILE_r(
        {
            if (prog->maxlen > 0 && (prog->check_utf8 || prog->check_substr)) {
                const char * const s = SvPV_nolen_const(RX_UTF8(r)
                      ? prog->check_utf8 : prog->check_substr);

                if (!PL_colorset) reginitcolors();
                Perl_re_printf( aTHX_
                      "%sUsing REx %ssubstr:%s \"%s%.60s%s%s\"\n",
                      PL_colors[4],
                      RX_UTF8(r) ? "utf8 " : "",
                      PL_colors[5], PL_colors[0],
                      s,
                      PL_colors[1],
                      (strlen(s) > PL_dump_re_max_len ? "..." : ""));
            }
        } );

    /* use UTF8 check substring if regexp pattern itself is in UTF8 */
    return RX_UTF8(r) ? prog->check_utf8 : prog->check_substr;
}

/*
   pregfree()

   handles refcounting and freeing the perl core regexp structure. When
   it is necessary to actually free the structure the first thing it
   does is call the 'free' method of the regexp_engine associated to
   the regexp, allowing the handling of the void *pprivate; member
   first. (This routine is not overridable by extensions, which is why
   the extensions free is called first.)

   See regdupe and regdupe_internal if you change anything here.
*/
#ifndef PERL_IN_XSUB_RE
void
Perl_pregfree(pTHX_ REGEXP *r)
{
    SvREFCNT_dec(r);
}

void
Perl_pregfree2(pTHX_ REGEXP *rx)
{
    struct regexp *const r = ReANY(rx);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_PREGFREE2;

    if (! r)
        return;

    if (r->mother_re) {
        ReREFCNT_dec(r->mother_re);
    } else {
        CALLREGFREE_PVT(rx); /* free the private data */
        SvREFCNT_dec(RXp_PAREN_NAMES(r));
    }
    if (r->substrs) {
        int i;
        for (i = 0; i < 2; i++) {
            SvREFCNT_dec(r->substrs->data[i].substr);
            SvREFCNT_dec(r->substrs->data[i].utf8_substr);
        }
        Safefree(r->substrs);
    }
    RX_MATCH_COPY_FREE(rx);
#ifdef PERL_ANY_COW
    SvREFCNT_dec(r->saved_copy);
#endif
    Safefree(RXp_OFFSp(r));
    if (r->logical_to_parno) {
        Safefree(r->logical_to_parno);
        Safefree(r->parno_to_logical);
        Safefree(r->parno_to_logical_next);
    }

    SvREFCNT_dec(r->qr_anoncv);
    if (r->recurse_locinput)
        Safefree(r->recurse_locinput);
}


/*  reg_temp_copy()

    Copy ssv to dsv, both of which should of type SVt_REGEXP or SVt_PVLV,
    except that dsv will be created if NULL.

    This function is used in two main ways. First to implement
        $r = qr/....; $s = $$r;

    Secondly, it is used as a hacky workaround to the structural issue of
    match results
    being stored in the regexp structure which is in turn stored in
    PL_curpm/PL_reg_curpm. The problem is that due to qr// the pattern
    could be PL_curpm in multiple contexts, and could require multiple
    result sets being associated with the pattern simultaneously, such
    as when doing a recursive match with (??{$qr})

    The solution is to make a lightweight copy of the regexp structure
    when a qr// is returned from the code executed by (??{$qr}) this
    lightweight copy doesn't actually own any of its data except for
    the starp/end and the actual regexp structure itself.

*/


REGEXP *
Perl_reg_temp_copy(pTHX_ REGEXP *dsv, REGEXP *ssv)
{
    struct regexp *drx;
    struct regexp *const srx = ReANY(ssv);
    const bool islv = dsv && SvTYPE(dsv) == SVt_PVLV;

    PERL_ARGS_ASSERT_REG_TEMP_COPY;

    if (!dsv)
        dsv = (REGEXP*) newSV_type(SVt_REGEXP);
    else {
        assert(SvTYPE(dsv) == SVt_REGEXP || (SvTYPE(dsv) == SVt_PVLV));

        /* our only valid caller, sv_setsv_flags(), should have done
         * a SV_CHECK_THINKFIRST_COW_DROP() by now */
        assert(!SvOOK(dsv));
        assert(!SvIsCOW(dsv));
        assert(!SvROK(dsv));

        if (SvPVX_const(dsv)) {
            if (SvLEN(dsv))
                Safefree(SvPVX(dsv));
            SvPVX(dsv) = NULL;
        }
        SvLEN_set(dsv, 0);
        SvCUR_set(dsv, 0);
        SvOK_off((SV *)dsv);

        if (islv) {
            /* For PVLVs, the head (sv_any) points to an XPVLV, while
             * the LV's xpvlenu_rx will point to a regexp body, which
             * we allocate here */
            REGEXP *temp = (REGEXP *)newSV_type(SVt_REGEXP);
            assert(!SvPVX(dsv));
            /* We "steal" the body from the newly allocated SV temp, changing
             * the pointer in its HEAD to NULL. We then change its type to
             * SVt_NULL so that when we immediately release its only reference,
             * no memory deallocation happens.
             *
             * The body will eventually be freed (from the PVLV) either in
             * Perl_sv_force_normal_flags() (if the PVLV is "downgraded" and
             * the regexp body needs to be removed)
             * or in Perl_sv_clear() (if the PVLV still holds the pointer until
             * the PVLV itself is deallocated). */
            ((XPV*)SvANY(dsv))->xpv_len_u.xpvlenu_rx = temp->sv_any;
            temp->sv_any = NULL;
            SvFLAGS(temp) = (SvFLAGS(temp) & ~SVTYPEMASK) | SVt_NULL;
            SvREFCNT_dec_NN(temp);
            /* SvCUR still resides in the xpvlv struct, so the regexp copy-
               ing below will not set it. */
            SvCUR_set(dsv, SvCUR(ssv));
        }
    }
    /* This ensures that SvTHINKFIRST(sv) is true, and hence that
       sv_force_normal(sv) is called.  */
    SvFAKE_on(dsv);
    drx = ReANY(dsv);

    SvFLAGS(dsv) |= SvFLAGS(ssv) & (SVf_POK|SVp_POK|SVf_UTF8);
    SvPV_set(dsv, RX_WRAPPED(ssv));
    /* We share the same string buffer as the original regexp, on which we
       hold a reference count, incremented when mother_re is set below.
       The string pointer is copied here, being part of the regexp struct.
     */
    memcpy(&(drx->xpv_cur), &(srx->xpv_cur),
           sizeof(regexp) - STRUCT_OFFSET(regexp, xpv_cur));

    if (!islv)
        SvLEN_set(dsv, 0);
    if (RXp_OFFSp(srx)) {
        const I32 npar = srx->nparens+1;
        NewCopy(RXp_OFFSp(srx), RXp_OFFSp(drx), npar, regexp_paren_pair);
    }
    if (srx->substrs) {
        int i;
        Newx(drx->substrs, 1, struct reg_substr_data);
        StructCopy(srx->substrs, drx->substrs, struct reg_substr_data);

        for (i = 0; i < 2; i++) {
            SvREFCNT_inc_void(drx->substrs->data[i].substr);
            SvREFCNT_inc_void(drx->substrs->data[i].utf8_substr);
        }

        /* check_substr and check_utf8, if non-NULL, point to either their
           anchored or float namesakes, and don't hold a second reference.  */
    }
    if (srx->logical_to_parno) {
        NewCopy(srx->logical_to_parno,
                drx->logical_to_parno,
                srx->nparens+1, I32);
        NewCopy(srx->parno_to_logical,
                drx->parno_to_logical,
                srx->nparens+1, I32);
        NewCopy(srx->parno_to_logical_next,
                drx->parno_to_logical_next,
                srx->nparens+1, I32);
    } else {
        drx->logical_to_parno = NULL;
        drx->parno_to_logical = NULL;
        drx->parno_to_logical_next = NULL;
    }
    drx->logical_nparens = srx->logical_nparens;

    RX_MATCH_COPIED_off(dsv);
#ifdef PERL_ANY_COW
    RXp_SAVED_COPY(drx) = NULL;
#endif
    drx->mother_re = ReREFCNT_inc(srx->mother_re ? srx->mother_re : ssv);
    SvREFCNT_inc_void(drx->qr_anoncv);
    if (srx->recurse_locinput)
        Newx(drx->recurse_locinput, srx->nparens + 1, char *);

    return dsv;
}
#endif


/* regfree_internal()

   Free the private data in a regexp. This is overloadable by
   extensions. Perl takes care of the regexp structure in pregfree(),
   this covers the *pprivate pointer which technically perl doesn't
   know about, however of course we have to handle the
   regexp_internal structure when no extension is in use.

   Note this is called before freeing anything in the regexp
   structure.
 */

void
Perl_regfree_internal(pTHX_ REGEXP * const rx)
{
    struct regexp *const r = ReANY(rx);
    RXi_GET_DECL(r, ri);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_REGFREE_INTERNAL;

    if (! ri) {
        return;
    }

    DEBUG_COMPILE_r({
        if (!PL_colorset)
            reginitcolors();
        {
            SV *dsv= sv_newmortal();
            RE_PV_QUOTED_DECL(s, RX_UTF8(rx),
                dsv, RX_PRECOMP(rx), RX_PRELEN(rx), PL_dump_re_max_len);
            Perl_re_printf( aTHX_ "%sFreeing REx:%s %s\n",
                PL_colors[4], PL_colors[5], s);
        }
    });

    if (ri->code_blocks)
        S_free_codeblocks(aTHX_ ri->code_blocks);

    if (ri->data) {
        int n = ri->data->count;

        while (--n >= 0) {
          /* If you add a ->what type here, update the comment in regcomp.h */
            switch (ri->data->what[n]) {
            case 'a':
            case 'r':
            case 's':
            case 'S':
            case 'u':
                SvREFCNT_dec(MUTABLE_SV(ri->data->data[n]));
                break;
            case 'f':
                Safefree(ri->data->data[n]);
                break;
            case 'l':
            case 'L':
                break;
            case 'T':
                { /* Aho Corasick add-on structure for a trie node.
                     Used in stclass optimization only */
                    U32 refcount;
                    reg_ac_data *aho=(reg_ac_data*)ri->data->data[n];
                    OP_REFCNT_LOCK;
                    refcount = --aho->refcount;
                    OP_REFCNT_UNLOCK;
                    if ( !refcount ) {
                        PerlMemShared_free(aho->states);
                        PerlMemShared_free(aho->fail);
                         /* do this last!!!! */
                        PerlMemShared_free(ri->data->data[n]);
                        /* we should only ever get called once, so
                         * assert as much, and also guard the free
                         * which /might/ happen twice. At the least
                         * it will make code anlyzers happy and it
                         * doesn't cost much. - Yves */
                        assert(ri->regstclass);
                        if (ri->regstclass) {
                            PerlMemShared_free(ri->regstclass);
                            ri->regstclass = 0;
                        }
                    }
                }
                break;
            case 't':
                {
                    /* trie structure. */
                    U32 refcount;
                    reg_trie_data *trie=(reg_trie_data*)ri->data->data[n];
                    OP_REFCNT_LOCK;
                    refcount = --trie->refcount;
                    OP_REFCNT_UNLOCK;
                    if ( !refcount ) {
                        PerlMemShared_free(trie->charmap);
                        PerlMemShared_free(trie->states);
                        PerlMemShared_free(trie->trans);
                        if (trie->bitmap)
                            PerlMemShared_free(trie->bitmap);
                        if (trie->jump)
                            PerlMemShared_free(trie->jump);
                        if (trie->j_before_paren)
                            PerlMemShared_free(trie->j_before_paren);
                        if (trie->j_after_paren)
                            PerlMemShared_free(trie->j_after_paren);
                        PerlMemShared_free(trie->wordinfo);
                        /* do this last!!!! */
                        PerlMemShared_free(ri->data->data[n]);
                    }
                }
                break;
            case '%':
                /* NO-OP a '%' data contains a null pointer, so that reg_add_data
                 * always returns non-zero, this should only ever happen in the
                 * 0 index */
                assert(n==0);
                break;
            default:
                Perl_croak(aTHX_ "panic: regfree data code '%c'",
                                                    ri->data->what[n]);
            }
        }
        Safefree(ri->data->what);
        Safefree(ri->data);
    }

    Safefree(ri);
}

#define SAVEPVN(p, n)	((p) ? savepvn(p, n) : NULL)

/*
=for apidoc re_dup_guts
Duplicate a regexp.

This routine is expected to clone a given regexp structure. It is only
compiled under USE_ITHREADS.

After all of the core data stored in struct regexp is duplicated
the C<regexp_engine.dupe> method is used to copy any private data
stored in the *pprivate pointer. This allows extensions to handle
any duplication they need to do.

=cut

   See pregfree() and regfree_internal() if you change anything here.
*/
#if defined(USE_ITHREADS)
#ifndef PERL_IN_XSUB_RE
void
Perl_re_dup_guts(pTHX_ const REGEXP *sstr, REGEXP *dstr, CLONE_PARAMS *param)
{
    I32 npar;
    const struct regexp *r = ReANY(sstr);
    struct regexp *ret = ReANY(dstr);

    PERL_ARGS_ASSERT_RE_DUP_GUTS;

    npar = r->nparens+1;
    NewCopy(RXp_OFFSp(r), RXp_OFFSp(ret), npar, regexp_paren_pair);

    if (ret->substrs) {
        /* Do it this way to avoid reading from *r after the StructCopy().
           That way, if any of the sv_dup_inc()s dislodge *r from the L1
           cache, it doesn't matter.  */
        int i;
        const bool anchored = r->check_substr
            ? r->check_substr == r->substrs->data[0].substr
            : r->check_utf8   == r->substrs->data[0].utf8_substr;
        Newx(ret->substrs, 1, struct reg_substr_data);
        StructCopy(r->substrs, ret->substrs, struct reg_substr_data);

        for (i = 0; i < 2; i++) {
            ret->substrs->data[i].substr =
                        sv_dup_inc(ret->substrs->data[i].substr, param);
            ret->substrs->data[i].utf8_substr =
                        sv_dup_inc(ret->substrs->data[i].utf8_substr, param);
        }

        /* check_substr and check_utf8, if non-NULL, point to either their
           anchored or float namesakes, and don't hold a second reference.  */

        if (ret->check_substr) {
            if (anchored) {
                assert(r->check_utf8 == r->substrs->data[0].utf8_substr);

                ret->check_substr = ret->substrs->data[0].substr;
                ret->check_utf8   = ret->substrs->data[0].utf8_substr;
            } else {
                assert(r->check_substr == r->substrs->data[1].substr);
                assert(r->check_utf8   == r->substrs->data[1].utf8_substr);

                ret->check_substr = ret->substrs->data[1].substr;
                ret->check_utf8   = ret->substrs->data[1].utf8_substr;
            }
        } else if (ret->check_utf8) {
            if (anchored) {
                ret->check_utf8 = ret->substrs->data[0].utf8_substr;
            } else {
                ret->check_utf8 = ret->substrs->data[1].utf8_substr;
            }
        }
    }

    RXp_PAREN_NAMES(ret) = hv_dup_inc(RXp_PAREN_NAMES(ret), param);
    ret->qr_anoncv = MUTABLE_CV(sv_dup_inc((const SV *)ret->qr_anoncv, param));
    if (r->recurse_locinput)
        Newx(ret->recurse_locinput, r->nparens + 1, char *);

    if (ret->pprivate)
        RXi_SET(ret, CALLREGDUPE_PVT(dstr, param));

    if (RX_MATCH_COPIED(dstr))
        RXp_SUBBEG(ret)  = SAVEPVN(RXp_SUBBEG(ret), RXp_SUBLEN(ret));
    else
        RXp_SUBBEG(ret) = NULL;
#ifdef PERL_ANY_COW
    RXp_SAVED_COPY(ret) = NULL;
#endif

    if (r->logical_to_parno) {
        /* we use total_parens for all three just for symmetry */
        ret->logical_to_parno = (I32*)SAVEPVN((char*)(r->logical_to_parno), (1+r->nparens) * sizeof(I32));
        ret->parno_to_logical = (I32*)SAVEPVN((char*)(r->parno_to_logical), (1+r->nparens) * sizeof(I32));
        ret->parno_to_logical_next = (I32*)SAVEPVN((char*)(r->parno_to_logical_next), (1+r->nparens) * sizeof(I32));
    } else {
        ret->logical_to_parno = NULL;
        ret->parno_to_logical = NULL;
        ret->parno_to_logical_next = NULL;
    }

    ret->logical_nparens = r->logical_nparens;

    /* Whether mother_re be set or no, we need to copy the string.  We
       cannot refrain from copying it when the storage points directly to
       our mother regexp, because that's
               1: a buffer in a different thread
               2: something we no longer hold a reference on
               so we need to copy it locally.  */
    RX_WRAPPED(dstr) = SAVEPVN(RX_WRAPPED_const(sstr), SvCUR(sstr)+1);
    /* set malloced length to a non-zero value so it will be freed
     * (otherwise in combination with SVf_FAKE it looks like an alien
     * buffer). It doesn't have to be the actual malloced size, since it
     * should never be grown */
    SvLEN_set(dstr, SvCUR(sstr)+1);
    ret->mother_re   = NULL;
}
#endif /* PERL_IN_XSUB_RE */

/*
   regdupe_internal()

   This is the internal complement to regdupe() which is used to copy
   the structure pointed to by the *pprivate pointer in the regexp.
   This is the core version of the extension overridable cloning hook.
   The regexp structure being duplicated will be copied by perl prior
   to this and will be provided as the regexp *r argument, however
   with the /old/ structures pprivate pointer value. Thus this routine
   may override any copying normally done by perl.

   It returns a pointer to the new regexp_internal structure.
*/

void *
Perl_regdupe_internal(pTHX_ REGEXP * const rx, CLONE_PARAMS *param)
{
    struct regexp *const r = ReANY(rx);
    regexp_internal *reti;
    int len;
    RXi_GET_DECL(r, ri);

    PERL_ARGS_ASSERT_REGDUPE_INTERNAL;

    len = ProgLen(ri);

    Newxc(reti, sizeof(regexp_internal) + len*sizeof(regnode),
          char, regexp_internal);
    Copy(ri->program, reti->program, len+1, regnode);


    if (ri->code_blocks) {
        int n;
        Newx(reti->code_blocks, 1, struct reg_code_blocks);
        Newx(reti->code_blocks->cb, ri->code_blocks->count,
                    struct reg_code_block);
        Copy(ri->code_blocks->cb, reti->code_blocks->cb,
             ri->code_blocks->count, struct reg_code_block);
        for (n = 0; n < ri->code_blocks->count; n++)
             reti->code_blocks->cb[n].src_regex = (REGEXP*)
                    sv_dup_inc((SV*)(ri->code_blocks->cb[n].src_regex), param);
        reti->code_blocks->count = ri->code_blocks->count;
        reti->code_blocks->refcnt = 1;
    }
    else
        reti->code_blocks = NULL;

    reti->regstclass = NULL;

    if (ri->data) {
        struct reg_data *d;
        const int count = ri->data->count;
        int i;

        Newxc(d, sizeof(struct reg_data) + count*sizeof(void *),
                char, struct reg_data);
        Newx(d->what, count, U8);

        d->count = count;
        for (i = 0; i < count; i++) {
            d->what[i] = ri->data->what[i];
            switch (d->what[i]) {
                /* see also regcomp.h and regfree_internal() */
            case 'a': /* actually an AV, but the dup function is identical.
                         values seem to be "plain sv's" generally. */
            case 'r': /* a compiled regex (but still just another SV) */
            case 's': /* an RV (currently only used for an RV to an AV by the ANYOF code)
                         this use case should go away, the code could have used
                         'a' instead - see S_set_ANYOF_arg() for array contents. */
            case 'S': /* actually an SV, but the dup function is identical.  */
            case 'u': /* actually an HV, but the dup function is identical.
                         values are "plain sv's" */
                d->data[i] = sv_dup_inc((const SV *)ri->data->data[i], param);
                break;
            case 'f':
                /* Synthetic Start Class - "Fake" charclass we generate to optimize
                 * patterns which could start with several different things. Pre-TRIE
                 * this was more important than it is now, however this still helps
                 * in some places, for instance /x?a+/ might produce a SSC equivalent
                 * to [xa]. This is used by Perl_re_intuit_start() and S_find_byclass()
                 * in regexec.c
                 */
                /* This is cheating. */
                Newx(d->data[i], 1, regnode_ssc);
                StructCopy(ri->data->data[i], d->data[i], regnode_ssc);
                reti->regstclass = (regnode*)d->data[i];
                break;
            case 'T':
                /* AHO-CORASICK fail table */
                /* Trie stclasses are readonly and can thus be shared
                 * without duplication. We free the stclass in pregfree
                 * when the corresponding reg_ac_data struct is freed.
                 */
                reti->regstclass= ri->regstclass;
                /* FALLTHROUGH */
            case 't':
                /* TRIE transition table */
                OP_REFCNT_LOCK;
                ((reg_trie_data*)ri->data->data[i])->refcount++;
                OP_REFCNT_UNLOCK;
                /* FALLTHROUGH */
            case 'l': /* (?{...}) or (??{ ... }) code (cb->block) */
            case 'L': /* same when RExC_pm_flags & PMf_HAS_CV and code
                         is not from another regexp */
                d->data[i] = ri->data->data[i];
                break;
            case '%':
                /* this is a placeholder type, it exists purely so that
                 * reg_add_data always returns a non-zero value, this type of
                 * entry should ONLY be present in the 0 slot of the array */
                assert(i == 0);
                d->data[i]= ri->data->data[i];
                break;
            default:
                Perl_croak(aTHX_ "panic: re_dup_guts unknown data code '%c'",
                                                           ri->data->what[i]);
            }
        }

        reti->data = d;
    }
    else
        reti->data = NULL;

    if (ri->regstclass && !reti->regstclass) {
        /* Assume that the regstclass is a regnode which is inside of the
         * program which we have to copy over */
        regnode *node= ri->regstclass;
        assert(node >= ri->program && (node - ri->program) < len);
        reti->regstclass = reti->program + (node - ri->program);
    }


    reti->name_list_idx = ri->name_list_idx;

    SetProgLen(reti, len);

    return (void*)reti;
}

#endif    /* USE_ITHREADS */

STATIC void
S_re_croak(pTHX_ bool utf8, const char* pat,...)
{
    va_list args;
    STRLEN len = strlen(pat);
    char buf[512];
    SV *msv;
    const char *message;

    PERL_ARGS_ASSERT_RE_CROAK;

    if (len > 510)
        len = 510;
    Copy(pat, buf, len , char);
    buf[len] = '\n';
    buf[len + 1] = '\0';
    va_start(args, pat);
    msv = vmess(buf, &args);
    va_end(args);
    message = SvPV_const(msv, len);
    if (len > 512)
        len = 512;
    Copy(message, buf, len , char);
    /* len-1 to avoid \n */
    Perl_croak(aTHX_ "%" UTF8f, UTF8fARG(utf8, len-1, buf));
}

/* XXX Here's a total kludge.  But we need to re-enter for swash routines. */

#ifndef PERL_IN_XSUB_RE
void
Perl_save_re_context(pTHX)
{
    I32 nparens = -1;
    I32 i;

    /* Save $1..$n (#18107: UTF-8 s/(\w+)/uc($1)/e); AMS 20021106. */

    if (PL_curpm) {
        const REGEXP * const rx = PM_GETRE(PL_curpm);
        if (rx)
            nparens = RX_NPARENS(rx);
    }

    /* RT #124109. This is a complete hack; in the SWASHNEW case we know
     * that PL_curpm will be null, but that utf8.pm and the modules it
     * loads will only use $1..$3.
     * The t/porting/re_context.t test file checks this assumption.
     */
    if (nparens == -1)
        nparens = 3;

    for (i = 1; i <= nparens; i++) {
        char digits[TYPE_CHARS(long)];
        const STRLEN len = my_snprintf(digits, sizeof(digits),
                                       "%lu", (long)i);
        GV *const *const gvp
            = (GV**)hv_fetch(PL_defstash, digits, len, 0);

        if (gvp) {
            GV * const gv = *gvp;
            if (SvTYPE(gv) == SVt_PVGV && GvSV(gv))
                save_scalar(gv);
        }
    }
}
#endif

#ifndef PERL_IN_XSUB_RE

#  include "uni_keywords.h"

void
Perl_init_uniprops(pTHX)
{

#  ifdef DEBUGGING
    char * dump_len_string;

    dump_len_string = PerlEnv_getenv("PERL_DUMP_RE_MAX_LEN");
    if (   ! dump_len_string
        || ! grok_atoUV(dump_len_string, (UV *)&PL_dump_re_max_len, NULL))
    {
        PL_dump_re_max_len = 60;    /* A reasonable default */
    }
#  endif

    PL_user_def_props = newHV();

#  ifdef USE_ITHREADS

    HvSHAREKEYS_off(PL_user_def_props);
    PL_user_def_props_aTHX = aTHX;

#  endif

    /* Set up the inversion list interpreter-level variables */

    PL_XPosix_ptrs[CC_ASCII_] = _new_invlist_C_array(uni_prop_ptrs[UNI_ASCII]);
    PL_XPosix_ptrs[CC_ALPHANUMERIC_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXALNUM]);
    PL_XPosix_ptrs[CC_ALPHA_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXALPHA]);
    PL_XPosix_ptrs[CC_BLANK_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXBLANK]);
    PL_XPosix_ptrs[CC_CASED_] =  _new_invlist_C_array(uni_prop_ptrs[UNI_CASED]);
    PL_XPosix_ptrs[CC_CNTRL_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXCNTRL]);
    PL_XPosix_ptrs[CC_DIGIT_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXDIGIT]);
    PL_XPosix_ptrs[CC_GRAPH_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXGRAPH]);
    PL_XPosix_ptrs[CC_LOWER_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXLOWER]);
    PL_XPosix_ptrs[CC_PRINT_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXPRINT]);
    PL_XPosix_ptrs[CC_PUNCT_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXPUNCT]);
    PL_XPosix_ptrs[CC_SPACE_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXSPACE]);
    PL_XPosix_ptrs[CC_UPPER_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXUPPER]);
    PL_XPosix_ptrs[CC_VERTSPACE_] = _new_invlist_C_array(uni_prop_ptrs[UNI_VERTSPACE]);
    PL_XPosix_ptrs[CC_WORDCHAR_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXWORD]);
    PL_XPosix_ptrs[CC_XDIGIT_] = _new_invlist_C_array(uni_prop_ptrs[UNI_XPOSIXXDIGIT]);

    PL_Posix_ptrs[CC_ASCII_] = _new_invlist_C_array(uni_prop_ptrs[UNI_ASCII]);
    PL_Posix_ptrs[CC_ALPHANUMERIC_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXALNUM]);
    PL_Posix_ptrs[CC_ALPHA_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXALPHA]);
    PL_Posix_ptrs[CC_BLANK_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXBLANK]);
    PL_Posix_ptrs[CC_CASED_] = PL_Posix_ptrs[CC_ALPHA_];
    PL_Posix_ptrs[CC_CNTRL_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXCNTRL]);
    PL_Posix_ptrs[CC_DIGIT_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXDIGIT]);
    PL_Posix_ptrs[CC_GRAPH_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXGRAPH]);
    PL_Posix_ptrs[CC_LOWER_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXLOWER]);
    PL_Posix_ptrs[CC_PRINT_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXPRINT]);
    PL_Posix_ptrs[CC_PUNCT_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXPUNCT]);
    PL_Posix_ptrs[CC_SPACE_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXSPACE]);
    PL_Posix_ptrs[CC_UPPER_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXUPPER]);
    PL_Posix_ptrs[CC_VERTSPACE_] = NULL;
    PL_Posix_ptrs[CC_WORDCHAR_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXWORD]);
    PL_Posix_ptrs[CC_XDIGIT_] = _new_invlist_C_array(uni_prop_ptrs[UNI_POSIXXDIGIT]);

    PL_GCB_invlist = _new_invlist_C_array(_Perl_GCB_invlist);
    PL_SB_invlist = _new_invlist_C_array(_Perl_SB_invlist);
    PL_WB_invlist = _new_invlist_C_array(_Perl_WB_invlist);
    PL_LB_invlist = _new_invlist_C_array(_Perl_LB_invlist);
    PL_SCX_invlist = _new_invlist_C_array(_Perl_SCX_invlist);

    PL_InBitmap = _new_invlist_C_array(InBitmap_invlist);
    PL_AboveLatin1 = _new_invlist_C_array(AboveLatin1_invlist);
    PL_Latin1 = _new_invlist_C_array(Latin1_invlist);
    PL_UpperLatin1 = _new_invlist_C_array(UpperLatin1_invlist);

    PL_Assigned_invlist = _new_invlist_C_array(uni_prop_ptrs[UNI_ASSIGNED]);

    PL_utf8_perl_idstart = _new_invlist_C_array(uni_prop_ptrs[UNI__PERL_IDSTART]);
    PL_utf8_perl_idcont = _new_invlist_C_array(uni_prop_ptrs[UNI__PERL_IDCONT]);

    PL_utf8_charname_begin = _new_invlist_C_array(uni_prop_ptrs[UNI__PERL_CHARNAME_BEGIN]);
    PL_utf8_charname_continue = _new_invlist_C_array(uni_prop_ptrs[UNI__PERL_CHARNAME_CONTINUE]);

    PL_in_some_fold = _new_invlist_C_array(uni_prop_ptrs[UNI__PERL_ANY_FOLDS]);
    PL_HasMultiCharFold = _new_invlist_C_array(uni_prop_ptrs[
                                            UNI__PERL_FOLDS_TO_MULTI_CHAR]);
    PL_InMultiCharFold = _new_invlist_C_array(uni_prop_ptrs[
                                            UNI__PERL_IS_IN_MULTI_CHAR_FOLD]);
    PL_utf8_toupper = _new_invlist_C_array(Uppercase_Mapping_invlist);
    PL_utf8_tolower = _new_invlist_C_array(Lowercase_Mapping_invlist);
    PL_utf8_totitle = _new_invlist_C_array(Titlecase_Mapping_invlist);
    PL_utf8_tofold = _new_invlist_C_array(Case_Folding_invlist);
    PL_utf8_tosimplefold = _new_invlist_C_array(Simple_Case_Folding_invlist);
    PL_utf8_foldclosures = _new_invlist_C_array(_Perl_IVCF_invlist);
    PL_utf8_mark = _new_invlist_C_array(uni_prop_ptrs[UNI_M]);
    PL_CCC_non0_non230 = _new_invlist_C_array(_Perl_CCC_non0_non230_invlist);
    PL_Private_Use = _new_invlist_C_array(uni_prop_ptrs[UNI_CO]);

#  ifdef UNI_XIDC
    /* The below are used only by deprecated functions.  They could be removed */
    PL_utf8_xidcont  = _new_invlist_C_array(uni_prop_ptrs[UNI_XIDC]);
    PL_utf8_idcont   = _new_invlist_C_array(uni_prop_ptrs[UNI_IDC]);
    PL_utf8_xidstart = _new_invlist_C_array(uni_prop_ptrs[UNI_XIDS]);
#  endif
}

/* These four functions are compiled only in regcomp.c, where they have access
 * to the data they return.  They are a way for re_comp.c to get access to that
 * data without having to compile the whole data structures. */

I16
Perl_do_uniprop_match(const char * const key, const U16 key_len)
{
    PERL_ARGS_ASSERT_DO_UNIPROP_MATCH;

    return match_uniprop((U8 *) key, key_len);
}

SV *
Perl_get_prop_definition(pTHX_ const int table_index)
{
    PERL_ARGS_ASSERT_GET_PROP_DEFINITION;

    /* Create and return the inversion list */
    return _new_invlist_C_array(uni_prop_ptrs[table_index]);
}

const char * const *
Perl_get_prop_values(const int table_index)
{
    PERL_ARGS_ASSERT_GET_PROP_VALUES;

    return UNI_prop_value_ptrs[table_index];
}

const char *
Perl_get_deprecated_property_msg(const Size_t warning_offset)
{
    PERL_ARGS_ASSERT_GET_DEPRECATED_PROPERTY_MSG;

    return deprecated_property_msgs[warning_offset];
}

#  if 0

This code was mainly added for backcompat to give a warning for non-portable
code points in user-defined properties.  But experiments showed that the
warning in earlier perls were only omitted on overflow, which should be an
error, so there really isnt a backcompat issue, and actually adding the
warning when none was present before might cause breakage, for little gain.  So
khw left this code in, but not enabled.  Tests were never added.

embed.fnc entry:
Ei	|const char *|get_extended_utf8_msg|const UV cp

PERL_STATIC_INLINE const char *
S_get_extended_utf8_msg(pTHX_ const UV cp)
{
    U8 dummy[UTF8_MAXBYTES + 1];
    HV *msgs;
    SV **msg;

    uvchr_to_utf8_flags_msgs(dummy, cp, UNICODE_WARN_PERL_EXTENDED,
                             &msgs);

    msg = hv_fetchs(msgs, "text", 0);
    assert(msg);

    (void) sv_2mortal((SV *) msgs);

    return SvPVX(*msg);
}

#  endif
#endif /* end of ! PERL_IN_XSUB_RE */

STATIC REGEXP *
S_compile_wildcard(pTHX_ const char * subpattern, const STRLEN len,
                         const bool ignore_case)
{
    /* Pretends that the input subpattern is qr/subpattern/aam, compiling it
     * possibly with /i if the 'ignore_case' parameter is true.  Use /aa
     * because nothing outside of ASCII will match.  Use /m because the input
     * string may be a bunch of lines strung together.
     *
     * Also sets up the debugging info */

    U32 flags = PMf_MULTILINE|PMf_WILDCARD;
    U32 rx_flags;
    SV * subpattern_sv = newSVpvn_flags(subpattern, len, SVs_TEMP);
    REGEXP * subpattern_re;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_COMPILE_WILDCARD;

    if (ignore_case) {
        flags |= PMf_FOLD;
    }
    set_regex_charset(&flags, REGEX_ASCII_MORE_RESTRICTED_CHARSET);

    /* Like in op.c, we copy the compile time pm flags to the rx ones */
    rx_flags = flags & RXf_PMf_COMPILETIME;

#ifndef PERL_IN_XSUB_RE
    /* Use the core engine if this file is regcomp.c.  That means no
     * 'use re "Debug ..." is in effect, so the core engine is sufficient */
    subpattern_re = Perl_re_op_compile(aTHX_ &subpattern_sv, 1, NULL,
                                             &PL_core_reg_engine,
                                             NULL, NULL,
                                             rx_flags, flags);
#else
    if (isDEBUG_WILDCARD) {
        /* Use the special debugging engine if this file is re_comp.c and wants
         * to output the wildcard matching.  This uses whatever
         * 'use re "Debug ..." is in effect */
        subpattern_re = Perl_re_op_compile(aTHX_ &subpattern_sv, 1, NULL,
                                                 &my_reg_engine,
                                                 NULL, NULL,
                                                 rx_flags, flags);
    }
    else {
        /* Use the special wildcard engine if this file is re_comp.c and
         * doesn't want to output the wildcard matching.  This uses whatever
         * 'use re "Debug ..." is in effect for compilation, but this engine
         * structure has been set up so that it uses the core engine for
         * execution, so no execution debugging as a result of re.pm will be
         * displayed. */
        subpattern_re = Perl_re_op_compile(aTHX_ &subpattern_sv, 1, NULL,
                                                 &wild_reg_engine,
                                                 NULL, NULL,
                                                 rx_flags, flags);
        /* XXX The above has the effect that any user-supplied regex engine
         * won't be called for matching wildcards.  That might be good, or bad.
         * It could be changed in several ways.  The reason it is done the
         * current way is to avoid having to save and restore
         * ^{^RE_DEBUG_FLAGS} around the execution.  save_scalar() perhaps
         * could be used.  Another suggestion is to keep the authoritative
         * value of the debug flags in a thread-local variable and add set/get
         * magic to ${^RE_DEBUG_FLAGS} to keep the C level variable up to date.
         * Still another is to pass a flag, say in the engine's intflags that
         * would be checked each time before doing the debug output */
    }
#endif

    assert(subpattern_re);  /* Should have died if didn't compile successfully */
    return subpattern_re;
}

STATIC I32
S_execute_wildcard(pTHX_ REGEXP * const prog, char* stringarg, char *strend,
         char *strbeg, SSize_t minend, SV *screamer, U32 nosave)
{
    I32 result;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_EXECUTE_WILDCARD;

    ENTER;

    /* The compilation has set things up so that if the program doesn't want to
     * see the wildcard matching procedure, it will get the core execution
     * engine, which is subject only to -Dr.  So we have to turn that off
     * around this procedure */
    if (! isDEBUG_WILDCARD) {
        /* Note! Casts away 'volatile' */
        SAVEI32(PL_debug);
        PL_debug &= ~ DEBUG_r_FLAG;
    }

    result = CALLREGEXEC(prog, stringarg, strend, strbeg, minend, screamer,
                         NULL, nosave);
    LEAVE;

    return result;
}

SV *
S_handle_user_defined_property(pTHX_

    /* Parses the contents of a user-defined property definition; returning the
     * expanded definition if possible.  If so, the return is an inversion
     * list.
     *
     * If there are subroutines that are part of the expansion and which aren't
     * known at the time of the call to this function, this returns what
     * parse_uniprop_string() returned for the first one encountered.
     *
     * If an error was found, NULL is returned, and 'msg' gets a suitable
     * message appended to it.  (Appending allows the back trace of how we got
     * to the faulty definition to be displayed through nested calls of
     * user-defined subs.)
     *
     * The caller IS responsible for freeing any returned SV.
     *
     * The syntax of the contents is pretty much described in perlunicode.pod,
     * but we also allow comments on each line */

    const char * name,          /* Name of property */
    const STRLEN name_len,      /* The name's length in bytes */
    const bool is_utf8,         /* ? Is 'name' encoded in UTF-8 */
    const bool to_fold,         /* ? Is this under /i */
    const bool runtime,         /* ? Are we in compile- or run-time */
    const bool deferrable,      /* Is it ok for this property's full definition
                                   to be deferred until later? */
    SV* contents,               /* The property's definition */
    bool *user_defined_ptr,     /* This will be set TRUE as we wouldn't be
                                   getting called unless this is thought to be
                                   a user-defined property */
    SV * msg,                   /* Any error or warning msg(s) are appended to
                                   this */
    const STRLEN level)         /* Recursion level of this call */
{
    STRLEN len;
    const char * string         = SvPV_const(contents, len);
    const char * const e        = string + len;
    const bool is_contents_utf8 = cBOOL(SvUTF8(contents));
    const STRLEN msgs_length_on_entry = SvCUR(msg);

    const char * s0 = string;   /* Points to first byte in the current line
                                   being parsed in 'string' */
    const char overflow_msg[] = "Code point too large in \"";
    SV* running_definition = NULL;

    PERL_ARGS_ASSERT_HANDLE_USER_DEFINED_PROPERTY;

    *user_defined_ptr = TRUE;

    /* Look at each line */
    while (s0 < e) {
        const char * s;     /* Current byte */
        char op = '+';      /* Default operation is 'union' */
        IV   min = 0;       /* range begin code point */
        IV   max = -1;      /* and range end */
        SV* this_definition;

        /* Skip comment lines */
        if (*s0 == '#') {
            s0 = strchr(s0, '\n');
            if (s0 == NULL) {
                break;
            }
            s0++;
            continue;
        }

        /* For backcompat, allow an empty first line */
        if (*s0 == '\n') {
            s0++;
            continue;
        }

        /* First character in the line may optionally be the operation */
        if (   *s0 == '+'
            || *s0 == '!'
            || *s0 == '-'
            || *s0 == '&')
        {
            op = *s0++;
        }

        /* If the line is one or two hex digits separated by blank space, its
         * a range; otherwise it is either another user-defined property or an
         * error */

        s = s0;

        if (! isXDIGIT(*s)) {
            goto check_if_property;
        }

        do { /* Each new hex digit will add 4 bits. */
            if (min > ( (IV) MAX_LEGAL_CP >> 4)) {
                s = strchr(s, '\n');
                if (s == NULL) {
                    s = e;
                }
                if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
                sv_catpv(msg, overflow_msg);
                Perl_sv_catpvf(aTHX_ msg, "%" UTF8f,
                                     UTF8fARG(is_contents_utf8, s - s0, s0));
                sv_catpvs(msg, "\"");
                goto return_failure;
            }

            /* Accumulate this digit into the value */
            min = (min << 4) + READ_XDIGIT(s);
        } while (isXDIGIT(*s));

        while (isBLANK(*s)) { s++; }

        /* We allow comments at the end of the line */
        if (*s == '#') {
            s = strchr(s, '\n');
            if (s == NULL) {
                s = e;
            }
            s++;
        }
        else if (s < e && *s != '\n') {
            if (! isXDIGIT(*s)) {
                goto check_if_property;
            }

            /* Look for the high point of the range */
            max = 0;
            do {
                if (max > ( (IV) MAX_LEGAL_CP >> 4)) {
                    s = strchr(s, '\n');
                    if (s == NULL) {
                        s = e;
                    }
                    if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
                    sv_catpv(msg, overflow_msg);
                    Perl_sv_catpvf(aTHX_ msg, "%" UTF8f,
                                      UTF8fARG(is_contents_utf8, s - s0, s0));
                    sv_catpvs(msg, "\"");
                    goto return_failure;
                }

                max = (max << 4) + READ_XDIGIT(s);
            } while (isXDIGIT(*s));

            while (isBLANK(*s)) { s++; }

            if (*s == '#') {
                s = strchr(s, '\n');
                if (s == NULL) {
                    s = e;
                }
            }
            else if (s < e && *s != '\n') {
                goto check_if_property;
            }
        }

        if (max == -1) {    /* The line only had one entry */
            max = min;
        }
        else if (max < min) {
            if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
            sv_catpvs(msg, "Illegal range in \"");
            Perl_sv_catpvf(aTHX_ msg, "%" UTF8f,
                                UTF8fARG(is_contents_utf8, s - s0, s0));
            sv_catpvs(msg, "\"");
            goto return_failure;
        }

#  if 0   /* See explanation at definition above of get_extended_utf8_msg() */

        if (   UNICODE_IS_PERL_EXTENDED(min)
            || UNICODE_IS_PERL_EXTENDED(max))
        {
            if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");

            /* If both code points are non-portable, warn only on the lower
             * one. */
            sv_catpv(msg, get_extended_utf8_msg(
                                            (UNICODE_IS_PERL_EXTENDED(min))
                                            ? min : max));
            sv_catpvs(msg, " in \"");
            Perl_sv_catpvf(aTHX_ msg, "%" UTF8f,
                                 UTF8fARG(is_contents_utf8, s - s0, s0));
            sv_catpvs(msg, "\"");
        }

#  endif

        /* Here, this line contains a legal range */
        this_definition = sv_2mortal(_new_invlist(2));
        this_definition = _add_range_to_invlist(this_definition, min, max);
        goto calculate;

      check_if_property:

        /* Here it isn't a legal range line.  See if it is a legal property
         * line.  First find the end of the meat of the line */
        s = strpbrk(s, "#\n");
        if (s == NULL) {
            s = e;
        }

        /* Ignore trailing blanks in keeping with the requirements of
         * parse_uniprop_string() */
        s--;
        while (s > s0 && isBLANK_A(*s)) {
            s--;
        }
        s++;

        this_definition = parse_uniprop_string(s0, s - s0,
                                               is_utf8, to_fold, runtime,
                                               deferrable,
                                               NULL,
                                               user_defined_ptr, msg,
                                               (name_len == 0)
                                                ? level /* Don't increase level
                                                           if input is empty */
                                                : level + 1
                                              );
        if (this_definition == NULL) {
            goto return_failure;    /* 'msg' should have had the reason
                                       appended to it by the above call */
        }

        if (! is_invlist(this_definition)) {    /* Unknown at this time */
            return newSVsv(this_definition);
        }

        if (*s != '\n') {
            s = strchr(s, '\n');
            if (s == NULL) {
                s = e;
            }
        }

      calculate:

        switch (op) {
            case '+':
                _invlist_union(running_definition, this_definition,
                                                        &running_definition);
                break;
            case '-':
                _invlist_subtract(running_definition, this_definition,
                                                        &running_definition);
                break;
            case '&':
                _invlist_intersection(running_definition, this_definition,
                                                        &running_definition);
                break;
            case '!':
                _invlist_union_complement_2nd(running_definition,
                                        this_definition, &running_definition);
                break;
            default:
                Perl_croak(aTHX_ "panic: %s: %d: Unexpected operation %d",
                                 __FILE__, __LINE__, op);
                break;
        }

        /* Position past the '\n' */
        s0 = s + 1;
    }   /* End of loop through the lines of 'contents' */

    /* Here, we processed all the lines in 'contents' without error.  If we
     * didn't add any warnings, simply return success */
    if (msgs_length_on_entry == SvCUR(msg)) {

        /* If the expansion was empty, the answer isn't nothing: its an empty
         * inversion list */
        if (running_definition == NULL) {
            running_definition = _new_invlist(1);
        }

        return running_definition;
    }

    /* Otherwise, add some explanatory text, but we will return success */
    goto return_msg;

  return_failure:
    running_definition = NULL;

  return_msg:

    if (name_len > 0) {
        sv_catpvs(msg, " in expansion of ");
        Perl_sv_catpvf(aTHX_ msg, "%" UTF8f, UTF8fARG(is_utf8, name_len, name));
    }

    return running_definition;
}

/* As explained below, certain operations need to take place in the first
 * thread created.  These macros switch contexts */
#  ifdef USE_ITHREADS
#    define DECLARATION_FOR_GLOBAL_CONTEXT                                  \
                                        PerlInterpreter * save_aTHX = aTHX;
#    define SWITCH_TO_GLOBAL_CONTEXT                                        \
                           PERL_SET_CONTEXT((aTHX = PL_user_def_props_aTHX))
#    define RESTORE_CONTEXT  PERL_SET_CONTEXT((aTHX = save_aTHX));
#    define CUR_CONTEXT      aTHX
#    define ORIGINAL_CONTEXT save_aTHX
#  else
#    define DECLARATION_FOR_GLOBAL_CONTEXT    dNOOP
#    define SWITCH_TO_GLOBAL_CONTEXT          NOOP
#    define RESTORE_CONTEXT                   NOOP
#    define CUR_CONTEXT                       NULL
#    define ORIGINAL_CONTEXT                  NULL
#  endif

STATIC void
S_delete_recursion_entry(pTHX_ void *key)
{
    /* Deletes the entry used to detect recursion when expanding user-defined
     * properties.  This is a function so it can be set up to be called even if
     * the program unexpectedly quits */

    SV ** current_entry;
    const STRLEN key_len = strlen((const char *) key);
    DECLARATION_FOR_GLOBAL_CONTEXT;

    SWITCH_TO_GLOBAL_CONTEXT;

    /* If the entry is one of these types, it is a permanent entry, and not the
     * one used to detect recursions.  This function should delete only the
     * recursion entry */
    current_entry = hv_fetch(PL_user_def_props, (const char *) key, key_len, 0);
    if (     current_entry
        && ! is_invlist(*current_entry)
        && ! SvPOK(*current_entry))
    {
        (void) hv_delete(PL_user_def_props, (const char *) key, key_len,
                                                                    G_DISCARD);
    }

    RESTORE_CONTEXT;
}

STATIC SV *
S_get_fq_name(pTHX_
              const char * const name,    /* The first non-blank in the \p{}, \P{} */
              const Size_t name_len,      /* Its length in bytes, not including any trailing space */
              const bool is_utf8,         /* ? Is 'name' encoded in UTF-8 */
              const bool has_colon_colon
             )
{
    /* Returns a mortal SV containing the fully qualified version of the input
     * name */

    SV * fq_name;

    fq_name = newSVpvs_flags("", SVs_TEMP);

    /* Use the current package if it wasn't included in our input */
    if (! has_colon_colon) {
        const HV * pkg = (IN_PERL_COMPILETIME)
                         ? PL_curstash
                         : CopSTASH(PL_curcop);
        const char* pkgname = HvNAME(pkg);

        Perl_sv_catpvf(aTHX_ fq_name, "%" UTF8f,
                      UTF8fARG(is_utf8, strlen(pkgname), pkgname));
        sv_catpvs(fq_name, "::");
    }

    Perl_sv_catpvf(aTHX_ fq_name, "%" UTF8f,
                         UTF8fARG(is_utf8, name_len, name));
    return fq_name;
}

STATIC SV *
S_parse_uniprop_string(pTHX_

    /* Parse the interior of a \p{}, \P{}.  Returns its definition if knowable
     * now.  If so, the return is an inversion list.
     *
     * If the property is user-defined, it is a subroutine, which in turn
     * may call other subroutines.  This function will call the whole nest of
     * them to get the definition they return; if some aren't known at the time
     * of the call to this function, the fully qualified name of the highest
     * level sub is returned.  It is an error to call this function at runtime
     * without every sub defined.
     *
     * If an error was found, NULL is returned, and 'msg' gets a suitable
     * message appended to it.  (Appending allows the back trace of how we got
     * to the faulty definition to be displayed through nested calls of
     * user-defined subs.)
     *
     * The caller should NOT try to free any returned inversion list.
     *
     * Other parameters will be set on return as described below */

    const char * const name,    /* The first non-blank in the \p{}, \P{} */
    Size_t name_len,            /* Its length in bytes, not including any
                                   trailing space */
    const bool is_utf8,         /* ? Is 'name' encoded in UTF-8 */
    const bool to_fold,         /* ? Is this under /i */
    const bool runtime,         /* TRUE if this is being called at run time */
    const bool deferrable,      /* TRUE if it's ok for the definition to not be
                                   known at this call */
    AV ** strings,              /* To return string property values, like named
                                   sequences */
    bool *user_defined_ptr,     /* Upon return from this function it will be
                                   set to TRUE if any component is a
                                   user-defined property */
    SV * msg,                   /* Any error or warning msg(s) are appended to
                                   this */
    const STRLEN level)         /* Recursion level of this call */
{
    char* lookup_name;          /* normalized name for lookup in our tables */
    unsigned lookup_len;        /* Its length */
    enum { Not_Strict = 0,      /* Some properties have stricter name */
           Strict,              /* normalization rules, which we decide */
           As_Is                /* upon based on parsing */
         } stricter = Not_Strict;

    /* nv= or numeric_value=, or possibly one of the cjk numeric properties
     * (though it requires extra effort to download them from Unicode and
     * compile perl to know about them) */
    bool is_nv_type = FALSE;

    unsigned int i = 0, i_zero = 0, j = 0;
    int equals_pos = -1;    /* Where the '=' is found, or negative if none */
    int slash_pos  = -1;    /* Where the '/' is found, or negative if none */
    int table_index = 0;    /* The entry number for this property in the table
                               of all Unicode property names */
    bool starts_with_Is = FALSE;  /* ? Does the name start with 'Is' */
    Size_t lookup_offset = 0;   /* Used to ignore the first few characters of
                                   the normalized name in certain situations */
    Size_t non_pkg_begin = 0;   /* Offset of first byte in 'name' that isn't
                                   part of a package name */
    Size_t lun_non_pkg_begin = 0;   /* Similarly for 'lookup_name' */
    bool could_be_user_defined = TRUE;  /* ? Could this be a user-defined
                                             property rather than a Unicode
                                             one. */
    SV * prop_definition = NULL;  /* The returned definition of 'name' or NULL
                                     if an error.  If it is an inversion list,
                                     it is the definition.  Otherwise it is a
                                     string containing the fully qualified sub
                                     name of 'name' */
    SV * fq_name = NULL;        /* For user-defined properties, the fully
                                   qualified name */
    bool invert_return = FALSE; /* ? Do we need to complement the result before
                                     returning it */
    bool stripped_utf8_pkg = FALSE; /* Set TRUE if the input includes an
                                       explicit utf8:: package that we strip
                                       off  */
    /* The expansion of properties that could be either user-defined or
     * official unicode ones is deferred until runtime, including a marker for
     * those that might be in the latter category.  This boolean indicates if
     * we've seen that marker.  If not, what we're parsing can't be such an
     * official Unicode property whose expansion was deferred */
    bool could_be_deferred_official = FALSE;

    PERL_ARGS_ASSERT_PARSE_UNIPROP_STRING;

    /* The input will be normalized into 'lookup_name' */
    Newx(lookup_name, name_len, char);
    SAVEFREEPV(lookup_name);

    /* Parse the input. */
    for (i = 0; i < name_len; i++) {
        char cur = name[i];

        /* Most of the characters in the input will be of this ilk, being parts
         * of a name */
        if (isIDCONT_A(cur)) {

            /* Case differences are ignored.  Our lookup routine assumes
             * everything is lowercase, so normalize to that */
            if (isUPPER_A(cur)) {
                lookup_name[j++] = toLOWER_A(cur);
                continue;
            }

            if (cur == '_') { /* Don't include these in the normalized name */
                continue;
            }

            lookup_name[j++] = cur;

            /* The first character in a user-defined name must be of this type.
             * */
            if (i - non_pkg_begin == 0 && ! isIDFIRST_A(cur)) {
                could_be_user_defined = FALSE;
            }

            continue;
        }

        /* Here, the character is not something typically in a name,  But these
         * two types of characters (and the '_' above) can be freely ignored in
         * most situations.  Later it may turn out we shouldn't have ignored
         * them, and we have to reparse, but we don't have enough information
         * yet to make that decision */
        if (cur == '-' || isSPACE_A(cur)) {
            could_be_user_defined = FALSE;
            continue;
        }

        /* An equals sign or single colon mark the end of the first part of
         * the property name */
        if (    cur == '='
            || (cur == ':' && (i >= name_len - 1 || name[i+1] != ':')))
        {
            lookup_name[j++] = '='; /* Treat the colon as an '=' */
            equals_pos = j; /* Note where it occurred in the input */
            could_be_user_defined = FALSE;
            break;
        }

        /* If this looks like it is a marker we inserted at compile time,
         * set a flag and otherwise ignore it.  If it isn't in the final
         * position, keep it as it would have been user input. */
        if (     UNLIKELY(cur == DEFERRED_COULD_BE_OFFICIAL_MARKERc)
            && ! deferrable
            &&   could_be_user_defined
            &&   i == name_len - 1)
        {
            name_len--;
            could_be_deferred_official = TRUE;
            continue;
        }

        /* Otherwise, this character is part of the name. */
        lookup_name[j++] = cur;

        /* Here it isn't a single colon, so if it is a colon, it must be a
         * double colon */
        if (cur == ':') {

            /* A double colon should be a package qualifier.  We note its
             * position and continue.  Note that one could have
             *      pkg1::pkg2::...::foo
             * so that the position at the end of the loop will be just after
             * the final qualifier */

            i++;
            non_pkg_begin = i + 1;
            lookup_name[j++] = ':';
            lun_non_pkg_begin = j;
        }
        else { /* Only word chars (and '::') can be in a user-defined name */
            could_be_user_defined = FALSE;
        }
    } /* End of parsing through the lhs of the property name (or all of it if
         no rhs) */

    /* If there is a single package name 'utf8::', it is ambiguous.  It could
     * be for a user-defined property, or it could be a Unicode property, as
     * all of them are considered to be for that package.  For the purposes of
     * parsing the rest of the property, strip it off */
    if (non_pkg_begin == STRLENs("utf8::") && memBEGINPs(name, name_len, "utf8::")) {
        lookup_name += STRLENs("utf8::");
        j           -= STRLENs("utf8::");
        equals_pos  -= STRLENs("utf8::");
        i_zero       = STRLENs("utf8::");   /* When resetting 'i' to reparse
                                               from the beginning, it has to be
                                               set past what we're stripping
                                               off */
        stripped_utf8_pkg = TRUE;
    }

    /* Here, we are either done with the whole property name, if it was simple;
     * or are positioned just after the '=' if it is compound. */

    if (equals_pos >= 0) {
        assert(stricter == Not_Strict); /* We shouldn't have set this yet */

        /* Space immediately after the '=' is ignored */
        i++;
        for (; i < name_len; i++) {
            if (! isSPACE_A(name[i])) {
                break;
            }
        }

        /* Most punctuation after the equals indicates a subpattern, like
         * \p{foo=/bar/} */
        if (   isPUNCT_A(name[i])
            &&  name[i] != '-'
            &&  name[i] != '+'
            &&  name[i] != '_'
            &&  name[i] != '{'
                /* A backslash means the real delimiter is the next character,
                 * but it must be punctuation */
            && (name[i] != '\\' || (i < name_len && isPUNCT_A(name[i+1]))))
        {
            bool special_property = memEQs(lookup_name, j - 1, "name")
                                 || memEQs(lookup_name, j - 1, "na");
            if (! special_property) {
                /* Find the property.  The table includes the equals sign, so
                 * we use 'j' as-is */
                table_index = do_uniprop_match(lookup_name, j);
            }
            if (special_property || table_index) {
                REGEXP * subpattern_re;
                char open = name[i++];
                char close;
                const char * pos_in_brackets;
                const char * const * prop_values;
                bool escaped = 0;

                /* Backslash => delimiter is the character following.  We
                 * already checked that it is punctuation */
                if (open == '\\') {
                    open = name[i++];
                    escaped = 1;
                }

                /* This data structure is constructed so that the matching
                 * closing bracket is 3 past its matching opening.  The second
                 * set of closing is so that if the opening is something like
                 * ']', the closing will be that as well.  Something similar is
                 * done in toke.c */
                pos_in_brackets = memCHRs("([<)]>)]>", open);
                close = (pos_in_brackets) ? pos_in_brackets[3] : open;

                if (    i >= name_len
                    ||  name[name_len-1] != close
                    || (escaped && name[name_len-2] != '\\')
                        /* Also make sure that there are enough characters.
                         * e.g., '\\\' would show up incorrectly as legal even
                         * though it is too short */
                    || (SSize_t) (name_len - i - 1 - escaped) < 0)
                {
                    sv_catpvs(msg, "Unicode property wildcard not terminated");
                    goto append_name_to_msg;
                }

                Perl_ck_warner_d(aTHX_
                    packWARN(WARN_EXPERIMENTAL__UNIPROP_WILDCARDS),
                    "The Unicode property wildcards feature is experimental");

                if (special_property) {
                    const char * error_msg;
                    const char * revised_name = name + i;
                    Size_t revised_name_len = name_len - (i + 1 + escaped);

                    /* Currently, the only 'special_property' is name, which we
                     * lookup in _charnames.pm */

                    if (! load_charnames(newSVpvs("placeholder"),
                                         revised_name, revised_name_len,
                                         &error_msg))
                    {
                        sv_catpv(msg, error_msg);
                        goto append_name_to_msg;
                    }

                    /* Farm this out to a function just to make the current
                     * function less unwieldy */
                    if (handle_names_wildcard(revised_name, revised_name_len,
                                              &prop_definition,
                                              strings))
                    {
                        return prop_definition;
                    }

                    goto failed;
                }

                prop_values = get_prop_values(table_index);

                /* Now create and compile the wildcard subpattern.  Use /i
                 * because the property values are supposed to match with case
                 * ignored. */
                subpattern_re = compile_wildcard(name + i,
                                                 name_len - i - 1 - escaped,
                                                 TRUE /* /i */
                                                );

                /* For each legal property value, see if the supplied pattern
                 * matches it. */
                while (*prop_values) {
                    const char * const entry = *prop_values;
                    const Size_t len = strlen(entry);
                    SV* entry_sv = newSVpvn_flags(entry, len, SVs_TEMP);

                    if (execute_wildcard(subpattern_re,
                                 (char *) entry,
                                 (char *) entry + len,
                                 (char *) entry, 0,
                                 entry_sv,
                                 0))
                    { /* Here, matched.  Add to the returned list */
                        Size_t total_len = j + len;
                        SV * sub_invlist = NULL;
                        char * this_string;

                        /* We know this is a legal \p{property=value}.  Call
                         * the function to return the list of code points that
                         * match it */
                        Newxz(this_string, total_len + 1, char);
                        Copy(lookup_name, this_string, j, char);
                        my_strlcat(this_string, entry, total_len + 1);
                        SAVEFREEPV(this_string);
                        sub_invlist = parse_uniprop_string(this_string,
                                                           total_len,
                                                           is_utf8,
                                                           to_fold,
                                                           runtime,
                                                           deferrable,
                                                           NULL,
                                                           user_defined_ptr,
                                                           msg,
                                                           level + 1);
                        _invlist_union(prop_definition, sub_invlist,
                                       &prop_definition);
                    }

                    prop_values++;  /* Next iteration, look at next propvalue */
                } /* End of looking through property values; (the data
                     structure is terminated by a NULL ptr) */

                SvREFCNT_dec_NN(subpattern_re);

                if (prop_definition) {
                    return prop_definition;
                }

                sv_catpvs(msg, "No Unicode property value wildcard matches:");
                goto append_name_to_msg;
            }

            /* Here's how khw thinks we should proceed to handle the properties
             * not yet done:    Bidi Mirroring Glyph        can map to ""
                                Bidi Paired Bracket         can map to ""
                                Case Folding  (both full and simple)
                                            Shouldn't /i be good enough for Full
                                Decomposition Mapping
                                Equivalent Unified Ideograph    can map to ""
                                Lowercase Mapping  (both full and simple)
                                NFKC Case Fold                  can map to ""
                                Titlecase Mapping  (both full and simple)
                                Uppercase Mapping  (both full and simple)
             * Handle these the same way Name is done, using say, _wild.pm, but
             * having both loose and full, like in charclass_invlists.h.
             * Perhaps move block and script to that as they are somewhat large
             * in charclass_invlists.h.
             * For properties where the default is the code point itself, such
             * as any of the case changing mappings, the string would otherwise
             * consist of all Unicode code points in UTF-8 strung together.
             * This would be impractical.  So instead, examine their compiled
             * pattern, looking at the ssc.  If none, reject the pattern as an
             * error.  Otherwise run the pattern against every code point in
             * the ssc.  The ssc is kind of like tr18's 3.9 Possible Match Sets
             * And it might be good to create an API to return the ssc.
             * Or handle them like the algorithmic names are done
             */
        } /* End of is a wildcard subppattern */

        /* \p{name=...} is handled specially.  Instead of using the normal
         * mechanism involving charclass_invlists.h, it uses _charnames.pm
         * which has the necessary (huge) data accessible to it, and which
         * doesn't get loaded unless necessary.  The legal syntax for names is
         * somewhat different than other properties due both to the vagaries of
         * a few outlier official names, and the fact that only a few ASCII
         * characters are permitted in them */
        if (   memEQs(lookup_name, j - 1, "name")
            || memEQs(lookup_name, j - 1, "na"))
        {
            dSP;
            HV * table;
            SV * character;
            const char * error_msg;
            CV* lookup_loose;
            SV * character_name;
            STRLEN character_len;
            UV cp;

            stricter = As_Is;

            /* Since the RHS (after skipping initial space) is passed unchanged
             * to charnames, and there are different criteria for what are
             * legal characters in the name, just parse it here.  A character
             * name must begin with an ASCII alphabetic */
            if (! isALPHA(name[i])) {
                goto failed;
            }
            lookup_name[j++] = name[i];

            for (++i; i < name_len; i++) {
                /* Official names can only be in the ASCII range, and only
                 * certain characters */
                if (! isASCII(name[i]) || ! isCHARNAME_CONT(name[i])) {
                    goto failed;
                }
                lookup_name[j++] = name[i];
            }

            /* Finished parsing, save the name into an SV */
            character_name = newSVpvn(lookup_name + equals_pos, j - equals_pos);

            /* Make sure _charnames is loaded.  (The parameters give context
             * for any errors generated */
            table = load_charnames(character_name, name, name_len, &error_msg);
            if (table == NULL) {
                sv_catpv(msg, error_msg);
                goto append_name_to_msg;
            }

            lookup_loose = get_cvs("_charnames::_loose_regcomp_lookup", 0);
            if (! lookup_loose) {
                Perl_croak(aTHX_
                       "panic: Can't find '_charnames::_loose_regcomp_lookup");
            }

            PUSHSTACKi(PERLSI_REGCOMP);
            ENTER ;
            SAVETMPS;
            save_re_context();

            PUSHMARK(SP) ;
            XPUSHs(character_name);
            PUTBACK;
            call_sv(MUTABLE_SV(lookup_loose), G_SCALAR);

            SPAGAIN ;

            character = POPs;
            SvREFCNT_inc_simple_void_NN(character);

            PUTBACK ;
            FREETMPS ;
            LEAVE ;
            POPSTACK;

            if (! SvOK(character)) {
                goto failed;
            }

            cp = valid_utf8_to_uvchr((U8 *) SvPVX(character), &character_len);
            if (character_len == SvCUR(character)) {
                prop_definition = add_cp_to_invlist(NULL, cp);
            }
            else {
                AV * this_string;

                /* First of the remaining characters in the string. */
                char * remaining = SvPVX(character) + character_len;

                if (strings == NULL) {
                    goto failed;    /* XXX Perhaps a specific msg instead, like
                                       'not available here' */
                }

                if (*strings == NULL) {
                    *strings = newAV();
                }

                this_string = newAV();
                av_push_simple(this_string, newSVuv(cp));

                do {
                    cp = valid_utf8_to_uvchr((U8 *) remaining, &character_len);
                    av_push_simple(this_string, newSVuv(cp));
                    remaining += character_len;
                } while (remaining < SvEND(character));

                av_push_simple(*strings, (SV *) this_string);
            }

            return prop_definition;
        }

        /* Certain properties whose values are numeric need special handling.
         * They may optionally be prefixed by 'is'.  Ignore that prefix for the
         * purposes of checking if this is one of those properties */
        if (memBEGINPs(lookup_name, j, "is")) {
            lookup_offset = 2;
        }

        /* Then check if it is one of these specially-handled properties.  The
         * possibilities are hard-coded because easier this way, and the list
         * is unlikely to change.
         *
         * All numeric value type properties are of this ilk, and are also
         * special in a different way later on.  So find those first.  There
         * are several numeric value type properties in the Unihan DB (which is
         * unlikely to be compiled with perl, but we handle it here in case it
         * does get compiled).  They all end with 'numeric'.  The interiors
         * aren't checked for the precise property.  This would stop working if
         * a cjk property were to be created that ended with 'numeric' and
         * wasn't a numeric type */
        is_nv_type = memEQs(lookup_name + lookup_offset,
                       j - 1 - lookup_offset, "numericvalue")
                  || memEQs(lookup_name + lookup_offset,
                      j - 1 - lookup_offset, "nv")
                  || (   memENDPs(lookup_name + lookup_offset,
                            j - 1 - lookup_offset, "numeric")
                      && (   memBEGINPs(lookup_name + lookup_offset,
                                      j - 1 - lookup_offset, "cjk")
                          || memBEGINPs(lookup_name + lookup_offset,
                                      j - 1 - lookup_offset, "k")));
        if (   is_nv_type
            || memEQs(lookup_name + lookup_offset,
                      j - 1 - lookup_offset, "canonicalcombiningclass")
            || memEQs(lookup_name + lookup_offset,
                      j - 1 - lookup_offset, "ccc")
            || memEQs(lookup_name + lookup_offset,
                      j - 1 - lookup_offset, "age")
            || memEQs(lookup_name + lookup_offset,
                      j - 1 - lookup_offset, "in")
            || memEQs(lookup_name + lookup_offset,
                      j - 1 - lookup_offset, "presentin"))
        {
            unsigned int k;

            /* Since the stuff after the '=' is a number, we can't throw away
             * '-' willy-nilly, as those could be a minus sign.  Other stricter
             * rules also apply.  However, these properties all can have the
             * rhs not be a number, in which case they contain at least one
             * alphabetic.  In those cases, the stricter rules don't apply.
             * But the numeric type properties can have the alphas [Ee] to
             * signify an exponent, and it is still a number with stricter
             * rules.  So look for an alpha that signifies not-strict */
            stricter = Strict;
            for (k = i; k < name_len; k++) {
                if (   isALPHA_A(name[k])
                    && (! is_nv_type || ! isALPHA_FOLD_EQ(name[k], 'E')))
                {
                    stricter = Not_Strict;
                    break;
                }
            }
        }

        if (stricter) {

            /* A number may have a leading '+' or '-'.  The latter is retained
             * */
            if (name[i] == '+') {
                i++;
            }
            else if (name[i] == '-') {
                lookup_name[j++] = '-';
                i++;
            }

            /* Skip leading zeros including single underscores separating the
             * zeros, or between the final leading zero and the first other
             * digit */
            for (; i < name_len - 1; i++) {
                if (    name[i] != '0'
                    && (name[i] != '_' || ! isDIGIT_A(name[i+1])))
                {
                    break;
                }
            }

            /* Turn nv=-0 into nv=0.  These should be equivalent, but vary by
             * underling libc implementation. */
            if (   i == name_len - 1
                && name[name_len-1] == '0'
                && lookup_name[j-1] == '-')
            {
                j--;
            }
        }
    }
    else {  /* No '=' */

       /* Only a few properties without an '=' should be parsed with stricter
        * rules.  The list is unlikely to change. */
        if (   memBEGINPs(lookup_name, j, "perl")
            && memNEs(lookup_name + 4, j - 4, "space")
            && memNEs(lookup_name + 4, j - 4, "word"))
        {
            stricter = Strict;

            /* We set the inputs back to 0 and the code below will reparse,
             * using strict */
            i = i_zero;
            j = 0;
        }
    }

    /* Here, we have either finished the property, or are positioned to parse
     * the remainder, and we know if stricter rules apply.  Finish out, if not
     * already done */
    for (; i < name_len; i++) {
        char cur = name[i];

        /* In all instances, case differences are ignored, and we normalize to
         * lowercase */
        if (isUPPER_A(cur)) {
            lookup_name[j++] = toLOWER(cur);
            continue;
        }

        /* An underscore is skipped, but not under strict rules unless it
         * separates two digits */
        if (cur == '_') {
            if (    stricter
                && (   i == i_zero || (int) i == equals_pos || i == name_len- 1
                    || ! isDIGIT_A(name[i-1]) || ! isDIGIT_A(name[i+1])))
            {
                lookup_name[j++] = '_';
            }
            continue;
        }

        /* Hyphens are skipped except under strict */
        if (cur == '-' && ! stricter) {
            continue;
        }

        /* XXX Bug in documentation.  It says white space skipped adjacent to
         * non-word char.  Maybe we should, but shouldn't skip it next to a dot
         * in a number */
        if (isSPACE_A(cur) && ! stricter) {
            continue;
        }

        lookup_name[j++] = cur;

        /* Unless this is a non-trailing slash, we are done with it */
        if (i >= name_len - 1 || cur != '/') {
            continue;
        }

        slash_pos = j;

        /* A slash in the 'numeric value' property indicates that what follows
         * is a denominator.  It can have a leading '+' and '0's that should be
         * skipped.  But we have never allowed a negative denominator, so treat
         * a minus like every other character.  (No need to rule out a second
         * '/', as that won't match anything anyway */
        if (is_nv_type) {
            i++;
            if (i < name_len && name[i] == '+') {
                i++;
            }

            /* Skip leading zeros including underscores separating digits */
            for (; i < name_len - 1; i++) {
                if (   name[i] != '0'
                    && (name[i] != '_' || ! isDIGIT_A(name[i+1])))
                {
                    break;
                }
            }

            /* Store the first real character in the denominator */
            if (i < name_len) {
                lookup_name[j++] = name[i];
            }
        }
    }

    /* Here are completely done parsing the input 'name', and 'lookup_name'
     * contains a copy, normalized.
     *
     * This special case is grandfathered in: 'L_' and 'GC=L_' are accepted and
     * different from without the underscores.  */
    if (  (   UNLIKELY(memEQs(lookup_name, j, "l"))
           || UNLIKELY(memEQs(lookup_name, j, "gc=l")))
        && UNLIKELY(name[name_len-1] == '_'))
    {
        lookup_name[j++] = '&';
    }

    /* If the original input began with 'In' or 'Is', it could be a subroutine
     * call to a user-defined property instead of a Unicode property name. */
    if (    name_len - non_pkg_begin > 2
        &&  name[non_pkg_begin+0] == 'I'
        && (name[non_pkg_begin+1] == 'n' || name[non_pkg_begin+1] == 's'))
    {
        /* Names that start with In have different characteristics than those
         * that start with Is */
        if (name[non_pkg_begin+1] == 's') {
            starts_with_Is = TRUE;
        }
    }
    else {
        could_be_user_defined = FALSE;
    }

    if (could_be_user_defined) {
        CV* user_sub;

        /* If the user defined property returns the empty string, it could
         * easily be because the pattern is being compiled before the data it
         * actually needs to compile is available.  This could be argued to be
         * a bug in the perl code, but this is a change of behavior for Perl,
         * so we handle it.  This means that intentionally returning nothing
         * will not be resolved until runtime */
        bool empty_return = FALSE;

        /* Here, the name could be for a user defined property, which are
         * implemented as subs. */
        user_sub = get_cvn_flags(name, name_len, 0);
        if (! user_sub) {

            /* Here, the property name could be a user-defined one, but there
             * is no subroutine to handle it (as of now).   Defer handling it
             * until runtime.  Otherwise, a block defined by Unicode in a later
             * release would get the synonym InFoo added for it, and existing
             * code that used that name would suddenly break if it referred to
             * the property before the sub was declared.  See [perl #134146] */
            if (deferrable) {
                goto definition_deferred;
            }

            /* Here, we are at runtime, and didn't find the user property.  It
             * could be an official property, but only if no package was
             * specified, or just the utf8:: package. */
            if (could_be_deferred_official) {
                lookup_name += lun_non_pkg_begin;
                j -= lun_non_pkg_begin;
            }
            else if (! stripped_utf8_pkg) {
                goto unknown_user_defined;
            }

            /* Drop down to look up in the official properties */
        }
        else {
            const char insecure[] = "Insecure user-defined property";

            /* Here, there is a sub by the correct name.  Normally we call it
             * to get the property definition */
            dSP;
            SV * user_sub_sv = MUTABLE_SV(user_sub);
            SV * error;     /* Any error returned by calling 'user_sub' */
            SV * key;       /* The key into the hash of user defined sub names
                             */
            SV * placeholder;
            SV ** saved_user_prop_ptr;      /* Hash entry for this property */

            /* How many times to retry when another thread is in the middle of
             * expanding the same definition we want */
            PERL_INT_FAST8_T retry_countdown = 10;

            DECLARATION_FOR_GLOBAL_CONTEXT;

            /* If we get here, we know this property is user-defined */
            *user_defined_ptr = TRUE;

            /* We refuse to call a potentially tainted subroutine; returning an
             * error instead */
            if (TAINT_get) {
                if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
                sv_catpvn(msg, insecure, sizeof(insecure) - 1);
                goto append_name_to_msg;
            }

            /* In principal, we only call each subroutine property definition
             * once during the life of the program.  This guarantees that the
             * property definition never changes.  The results of the single
             * sub call are stored in a hash, which is used instead for future
             * references to this property.  The property definition is thus
             * immutable.  But, to allow the user to have a /i-dependent
             * definition, we call the sub once for non-/i, and once for /i,
             * should the need arise, passing the /i status as a parameter.
             *
             * We start by constructing the hash key name, consisting of the
             * fully qualified subroutine name, preceded by the /i status, so
             * that there is a key for /i and a different key for non-/i */
            key = newSVpvn_flags(((to_fold) ? "1" : "0"), 1, SVs_TEMP);
            fq_name = S_get_fq_name(aTHX_ name, name_len, is_utf8,
                                          non_pkg_begin != 0);
            sv_catsv(key, fq_name);

            /* We only call the sub once throughout the life of the program
             * (with the /i, non-/i exception noted above).  That means the
             * hash must be global and accessible to all threads.  It is
             * created at program start-up, before any threads are created, so
             * is accessible to all children.  But this creates some
             * complications.
             *
             * 1) The keys can't be shared, or else problems arise; sharing is
             *    turned off at hash creation time
             * 2) All SVs in it are there for the remainder of the life of the
             *    program, and must be created in the same interpreter context
             *    as the hash, or else they will be freed from the wrong pool
             *    at global destruction time.  This is handled by switching to
             *    the hash's context to create each SV going into it, and then
             *    immediately switching back
             * 3) All accesses to the hash must be controlled by a mutex, to
             *    prevent two threads from getting an unstable state should
             *    they simultaneously be accessing it.  The code below is
             *    crafted so that the mutex is locked whenever there is an
             *    access and unlocked only when the next stable state is
             *    achieved.
             *
             * The hash stores either the definition of the property if it was
             * valid, or, if invalid, the error message that was raised.  We
             * use the type of SV to distinguish.
             *
             * There's also the need to guard against the definition expansion
             * from infinitely recursing.  This is handled by storing the aTHX
             * of the expanding thread during the expansion.  Again the SV type
             * is used to distinguish this from the other two cases.  If we
             * come to here and the hash entry for this property is our aTHX,
             * it means we have recursed, and the code assumes that we would
             * infinitely recurse, so instead stops and raises an error.
             * (Any recursion has always been treated as infinite recursion in
             * this feature.)
             *
             * If instead, the entry is for a different aTHX, it means that
             * that thread has gotten here first, and hasn't finished expanding
             * the definition yet.  We just have to wait until it is done.  We
             * sleep and retry a few times, returning an error if the other
             * thread doesn't complete. */

          re_fetch:
            USER_PROP_MUTEX_LOCK;

            /* If we have an entry for this key, the subroutine has already
             * been called once with this /i status. */
            saved_user_prop_ptr = hv_fetch(PL_user_def_props,
                                                   SvPVX(key), SvCUR(key), 0);
            if (saved_user_prop_ptr) {

                /* If the saved result is an inversion list, it is the valid
                 * definition of this property */
                if (is_invlist(*saved_user_prop_ptr)) {
                    prop_definition = *saved_user_prop_ptr;

                    /* The SV in the hash won't be removed until global
                     * destruction, so it is stable and we can unlock */
                    USER_PROP_MUTEX_UNLOCK;

                    /* The caller shouldn't try to free this SV */
                    return prop_definition;
                }

                /* Otherwise, if it is a string, it is the error message
                 * that was returned when we first tried to evaluate this
                 * property.  Fail, and append the message */
                if (SvPOK(*saved_user_prop_ptr)) {
                    if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
                    sv_catsv(msg, *saved_user_prop_ptr);

                    /* The SV in the hash won't be removed until global
                     * destruction, so it is stable and we can unlock */
                    USER_PROP_MUTEX_UNLOCK;

                    return NULL;
                }

                assert(SvIOK(*saved_user_prop_ptr));

                /* Here, we have an unstable entry in the hash.  Either another
                 * thread is in the middle of expanding the property's
                 * definition, or we are ourselves recursing.  We use the aTHX
                 * in it to distinguish */
                if (SvIV(*saved_user_prop_ptr) != PTR2IV(CUR_CONTEXT)) {

                    /* Here, it's another thread doing the expanding.  We've
                     * looked as much as we are going to at the contents of the
                     * hash entry.  It's safe to unlock. */
                    USER_PROP_MUTEX_UNLOCK;

                    /* Retry a few times */
                    if (retry_countdown-- > 0) {
                        PerlProc_sleep(1);
                        goto re_fetch;
                    }

                    if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
                    sv_catpvs(msg, "Timeout waiting for another thread to "
                                   "define");
                    goto append_name_to_msg;
                }

                /* Here, we are recursing; don't dig any deeper */
                USER_PROP_MUTEX_UNLOCK;

                if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
                sv_catpvs(msg,
                          "Infinite recursion in user-defined property");
                goto append_name_to_msg;
            }

            /* Here, this thread has exclusive control, and there is no entry
             * for this property in the hash.  So we have the go ahead to
             * expand the definition ourselves. */

            PUSHSTACKi(PERLSI_REGCOMP);
            ENTER;

            /* Create a temporary placeholder in the hash to detect recursion
             * */
            SWITCH_TO_GLOBAL_CONTEXT;
            placeholder= newSVuv(PTR2IV(ORIGINAL_CONTEXT));
            (void) hv_store_ent(PL_user_def_props, key, placeholder, 0);
            RESTORE_CONTEXT;

            /* Now that we have a placeholder, we can let other threads
             * continue */
            USER_PROP_MUTEX_UNLOCK;

            /* Make sure the placeholder always gets destroyed */
            SAVEDESTRUCTOR_X(S_delete_recursion_entry, SvPVX(key));

            PUSHMARK(SP);
            SAVETMPS;

            /* Call the user's function, with the /i status as a parameter.
             * Note that we have gone to a lot of trouble to keep this call
             * from being within the locked mutex region. */
            XPUSHs(boolSV(to_fold));
            PUTBACK;

            /* The following block was taken from swash_init().  Presumably
             * they apply to here as well, though we no longer use a swash --
             * khw */
            SAVEHINTS();
            save_re_context();
            /* We might get here via a subroutine signature which uses a utf8
             * parameter name, at which point PL_subname will have been set
             * but not yet used. */
            save_item(PL_subname);

            /* G_SCALAR guarantees a single return value */
            (void) call_sv(user_sub_sv, G_EVAL|G_SCALAR);

            SPAGAIN;

            error = ERRSV;
            if (TAINT_get || SvTRUE(error)) {
                if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
                if (SvTRUE(error)) {
                    sv_catpvs(msg, "Error \"");
                    sv_catsv(msg, error);
                    sv_catpvs(msg, "\"");
                }
                if (TAINT_get) {
                    if (SvTRUE(error)) sv_catpvs(msg, "; ");
                    sv_catpvn(msg, insecure, sizeof(insecure) - 1);
                }

                if (name_len > 0) {
                    sv_catpvs(msg, " in expansion of ");
                    Perl_sv_catpvf(aTHX_ msg, "%" UTF8f, UTF8fARG(is_utf8,
                                                                  name_len,
                                                                  name));
                }

                (void) POPs;
                prop_definition = NULL;
            }
            else {
                SV * contents = POPs;

                /* The contents is supposed to be the expansion of the property
                 * definition.  If the definition is deferrable, and we got an
                 * empty string back, set a flag to later defer it (after clean
                 * up below). */
                if (      deferrable
                    && (! SvPOK(contents) || SvCUR(contents) == 0))
                {
                        empty_return = TRUE;
                }
                else { /* Otherwise, call a function to check for valid syntax,
                          and handle it */

                    prop_definition = handle_user_defined_property(
                                                    name, name_len,
                                                    is_utf8, to_fold, runtime,
                                                    deferrable,
                                                    contents, user_defined_ptr,
                                                    msg,
                                                    level);
                }
            }

            /* Here, we have the results of the expansion.  Delete the
             * placeholder, and if the definition is now known, replace it with
             * that definition.  We need exclusive access to the hash, and we
             * can't let anyone else in, between when we delete the placeholder
             * and add the permanent entry */
            USER_PROP_MUTEX_LOCK;

            S_delete_recursion_entry(aTHX_ SvPVX(key));

            if (    ! empty_return
                && (! prop_definition || is_invlist(prop_definition)))
            {
                /* If we got success we use the inversion list defining the
                 * property; otherwise use the error message */
                SWITCH_TO_GLOBAL_CONTEXT;
                (void) hv_store_ent(PL_user_def_props,
                                    key,
                                    ((prop_definition)
                                     ? newSVsv(prop_definition)
                                     : newSVsv(msg)),
                                    0);
                RESTORE_CONTEXT;
            }

            /* All done, and the hash now has a permanent entry for this
             * property.  Give up exclusive control */
            USER_PROP_MUTEX_UNLOCK;

            FREETMPS;
            LEAVE;
            POPSTACK;

            if (empty_return) {
                goto definition_deferred;
            }

            if (prop_definition) {

                /* If the definition is for something not known at this time,
                 * we toss it, and go return the main property name, as that's
                 * the one the user will be aware of */
                if (! is_invlist(prop_definition)) {
                    SvREFCNT_dec_NN(prop_definition);
                    goto definition_deferred;
                }

                sv_2mortal(prop_definition);
            }

            /* And return */
            return prop_definition;

        }   /* End of calling the subroutine for the user-defined property */
    }       /* End of it could be a user-defined property */

    /* Here it wasn't a user-defined property that is known at this time.  See
     * if it is a Unicode property */

    lookup_len = j;     /* This is a more mnemonic name than 'j' */

    /* Get the index into our pointer table of the inversion list corresponding
     * to the property */
    table_index = do_uniprop_match(lookup_name, lookup_len);

    /* If it didn't find the property ... */
    if (table_index == 0) {

        /* Try again stripping off any initial 'Is'.  This is because we
         * promise that an initial Is is optional.  The same isn't true of
         * names that start with 'In'.  Those can match only blocks, and the
         * lookup table already has those accounted for.  The lookup table also
         * has already accounted for Perl extensions (without and = sign)
         * starting with 'i's'. */
        if (starts_with_Is && equals_pos >= 0) {
            lookup_name += 2;
            lookup_len -= 2;
            equals_pos -= 2;
            slash_pos -= 2;

            table_index = do_uniprop_match(lookup_name, lookup_len);
        }

        if (table_index == 0) {
            char * canonical;

            /* Here, we didn't find it.  If not a numeric type property, and
             * can't be a user-defined one, it isn't a legal property */
            if (! is_nv_type) {
                if (! could_be_user_defined) {
                    goto failed;
                }

                /* Here, the property name is legal as a user-defined one.   At
                 * compile time, it might just be that the subroutine for that
                 * property hasn't been encountered yet, but at runtime, it's
                 * an error to try to use an undefined one */
                if (! deferrable) {
                    goto unknown_user_defined;;
                }

                goto definition_deferred;
            } /* End of isn't a numeric type property */

            /* The numeric type properties need more work to decide.  What we
             * do is make sure we have the number in canonical form and look
             * that up. */

            if (slash_pos < 0) {    /* No slash */

                /* When it isn't a rational, take the input, convert it to a
                 * NV, then create a canonical string representation of that
                 * NV. */

                NV value;
                SSize_t value_len = lookup_len - equals_pos;

                /* Get the value */
                if (   value_len <= 0
                    || my_atof3(lookup_name + equals_pos, &value,
                                value_len)
                          != lookup_name + lookup_len)
                {
                    goto failed;
                }

                /* If the value is an integer, the canonical value is integral
                 * */
                if (Perl_ceil(value) == value) {
                    canonical = Perl_form(aTHX_ "%.*s%.0" NVff,
                                            equals_pos, lookup_name, value);
                }
                else {  /* Otherwise, it is %e with a known precision */
                    char * exp_ptr;

                    canonical = Perl_form(aTHX_ "%.*s%.*" NVef,
                                                equals_pos, lookup_name,
                                                PL_E_FORMAT_PRECISION, value);

                    /* The exponent generated is expecting two digits, whereas
                     * %e on some systems will generate three.  Remove leading
                     * zeros in excess of 2 from the exponent.  We start
                     * looking for them after the '=' */
                    exp_ptr = strchr(canonical + equals_pos, 'e');
                    if (exp_ptr) {
                        char * cur_ptr = exp_ptr + 2; /* past the 'e[+-]' */
                        SSize_t excess_exponent_len = strlen(cur_ptr) - 2;

                        assert(*(cur_ptr - 1) == '-' || *(cur_ptr - 1) == '+');

                        if (excess_exponent_len > 0) {
                            SSize_t leading_zeros = strspn(cur_ptr, "0");
                            SSize_t excess_leading_zeros
                                    = MIN(leading_zeros, excess_exponent_len);
                            if (excess_leading_zeros > 0) {
                                Move(cur_ptr + excess_leading_zeros,
                                     cur_ptr,
                                     strlen(cur_ptr) - excess_leading_zeros
                                       + 1,  /* Copy the NUL as well */
                                     char);
                            }
                        }
                    }
                }
            }
            else {  /* Has a slash.  Create a rational in canonical form  */
                UV numerator, denominator, gcd, trial;
                const char * end_ptr;
                const char * sign = "";

                /* We can't just find the numerator, denominator, and do the
                 * division, then use the method above, because that is
                 * inexact.  And the input could be a rational that is within
                 * epsilon (given our precision) of a valid rational, and would
                 * then incorrectly compare valid.
                 *
                 * We're only interested in the part after the '=' */
                const char * this_lookup_name = lookup_name + equals_pos;
                lookup_len -= equals_pos;
                slash_pos -= equals_pos;

                /* Handle any leading minus */
                if (this_lookup_name[0] == '-') {
                    sign = "-";
                    this_lookup_name++;
                    lookup_len--;
                    slash_pos--;
                }

                /* Convert the numerator to numeric */
                end_ptr = this_lookup_name + slash_pos;
                if (! grok_atoUV(this_lookup_name, &numerator, &end_ptr)) {
                    goto failed;
                }

                /* It better have included all characters before the slash */
                if (*end_ptr != '/') {
                    goto failed;
                }

                /* Set to look at just the denominator */
                this_lookup_name += slash_pos;
                lookup_len -= slash_pos;
                end_ptr = this_lookup_name + lookup_len;

                /* Convert the denominator to numeric */
                if (! grok_atoUV(this_lookup_name, &denominator, &end_ptr)) {
                    goto failed;
                }

                /* It better be the rest of the characters, and don't divide by
                 * 0 */
                if (   end_ptr != this_lookup_name + lookup_len
                    || denominator == 0)
                {
                    goto failed;
                }

                /* Get the greatest common denominator using
                   http://en.wikipedia.org/wiki/Euclidean_algorithm */
                gcd = numerator;
                trial = denominator;
                while (trial != 0) {
                    UV temp = trial;
                    trial = gcd % trial;
                    gcd = temp;
                }

                /* If already in lowest possible terms, we have already tried
                 * looking this up */
                if (gcd == 1) {
                    goto failed;
                }

                /* Reduce the rational, which should put it in canonical form
                 * */
                numerator /= gcd;
                denominator /= gcd;

                canonical = Perl_form(aTHX_ "%.*s%s%" UVuf "/%" UVuf,
                        equals_pos, lookup_name, sign, numerator, denominator);
            }

            /* Here, we have the number in canonical form.  Try that */
            table_index = do_uniprop_match(canonical, strlen(canonical));
            if (table_index == 0) {
                goto failed;
            }
        }   /* End of still didn't find the property in our table */
    }       /* End of       didn't find the property in our table */

    /* Here, we have a non-zero return, which is an index into a table of ptrs.
     * A negative return signifies that the real index is the absolute value,
     * but the result needs to be inverted */
    if (table_index < 0) {
        invert_return = TRUE;
        table_index = -table_index;
    }

    /* Out-of band indices indicate a deprecated property.  The proper index is
     * modulo it with the table size.  And dividing by the table size yields
     * an offset into a table constructed by regen/mk_invlists.pl to contain
     * the corresponding warning message */
    if (table_index > MAX_UNI_KEYWORD_INDEX) {
        Size_t warning_offset = table_index / MAX_UNI_KEYWORD_INDEX;
        table_index %= MAX_UNI_KEYWORD_INDEX;
        Perl_ck_warner_d(aTHX_ packWARN(WARN_DEPRECATED__UNICODE_PROPERTY_NAME),
                "Use of '%.*s' in \\p{} or \\P{} is deprecated because: %s",
                (int) name_len, name,
                get_deprecated_property_msg(warning_offset));
    }

    /* In a few properties, a different property is used under /i.  These are
     * unlikely to change, so are hard-coded here. */
    if (to_fold) {
        if (   table_index == UNI_XPOSIXUPPER
            || table_index == UNI_XPOSIXLOWER
            || table_index == UNI_TITLE)
        {
            table_index = UNI_CASED;
        }
        else if (   table_index == UNI_UPPERCASELETTER
                 || table_index == UNI_LOWERCASELETTER
#  ifdef UNI_TITLECASELETTER   /* Missing from early Unicodes */
                 || table_index == UNI_TITLECASELETTER
#  endif
        ) {
            table_index = UNI_CASEDLETTER;
        }
        else if (  table_index == UNI_POSIXUPPER
                || table_index == UNI_POSIXLOWER)
        {
            table_index = UNI_POSIXALPHA;
        }
    }

    /* Create and return the inversion list */
    prop_definition = get_prop_definition(table_index);
    sv_2mortal(prop_definition);

    /* See if there is a private use override to add to this definition */
    {
        COPHH * hinthash = (IN_PERL_COMPILETIME)
                           ? CopHINTHASH_get(&PL_compiling)
                           : CopHINTHASH_get(PL_curcop);
        SV * pu_overrides = cophh_fetch_pv(hinthash, "private_use", 0, 0);

        if (UNLIKELY(pu_overrides && SvPOK(pu_overrides))) {

            /* See if there is an element in the hints hash for this table */
            SV * pu_lookup = Perl_newSVpvf(aTHX_ "%d=", table_index);
            const char * pos = strstr(SvPVX(pu_overrides), SvPVX(pu_lookup));

            if (pos) {
                bool dummy;
                SV * pu_definition;
                SV * pu_invlist;
                SV * expanded_prop_definition =
                            sv_2mortal(invlist_clone(prop_definition, NULL));

                /* If so, it's definition is the string from here to the next
                 * \a character.  And its format is the same as a user-defined
                 * property */
                pos += SvCUR(pu_lookup);
                pu_definition = newSVpvn(pos, strchr(pos, '\a') - pos);
                pu_invlist = handle_user_defined_property(lookup_name,
                                                          lookup_len,
                                                          0, /* Not UTF-8 */
                                                          0, /* Not folded */
                                                          runtime,
                                                          deferrable,
                                                          pu_definition,
                                                          &dummy,
                                                          msg,
                                                          level);
                if (TAINT_get) {
                    if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
                    sv_catpvs(msg, "Insecure private-use override");
                    goto append_name_to_msg;
                }

                /* For now, as a safety measure, make sure that it doesn't
                 * override non-private use code points */
                _invlist_intersection(pu_invlist, PL_Private_Use, &pu_invlist);

                /* Add it to the list to be returned */
                _invlist_union(prop_definition, pu_invlist,
                               &expanded_prop_definition);
                prop_definition = expanded_prop_definition;
                Perl_ck_warner_d(aTHX_ packWARN(WARN_EXPERIMENTAL__PRIVATE_USE), "The private_use feature is experimental");
            }
        }
    }

    if (invert_return) {
        _invlist_invert(prop_definition);
    }
    return prop_definition;

  unknown_user_defined:
    if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
    sv_catpvs(msg, "Unknown user-defined property name");
    goto append_name_to_msg;

  failed:
    if (non_pkg_begin != 0) {
        if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
        sv_catpvs(msg, "Illegal user-defined property name");
    }
    else {
        if (SvCUR(msg) > 0) sv_catpvs(msg, "; ");
        sv_catpvs(msg, "Can't find Unicode property definition");
    }
    /* FALLTHROUGH */

  append_name_to_msg:
    {
        const char * prefix = (runtime && level == 0) ?  " \\p{" : " \"";
        const char * suffix = (runtime && level == 0) ?  "}" : "\"";

        sv_catpv(msg, prefix);
        Perl_sv_catpvf(aTHX_ msg, "%" UTF8f, UTF8fARG(is_utf8, name_len, name));
        sv_catpv(msg, suffix);
    }

    return NULL;

  definition_deferred:

    {
        bool is_qualified = non_pkg_begin != 0;  /* If has "::" */

        /* Here it could yet to be defined, so defer evaluation of this until
         * its needed at runtime.  We need the fully qualified property name to
         * avoid ambiguity */
        if (! fq_name) {
            fq_name = S_get_fq_name(aTHX_ name, name_len, is_utf8,
                                                                is_qualified);
        }

        /* If it didn't come with a package, or the package is utf8::, this
         * actually could be an official Unicode property whose inclusion we
         * are deferring until runtime to make sure that it isn't overridden by
         * a user-defined property of the same name (which we haven't
         * encountered yet).  Add a marker to indicate this possibility, for
         * use at such time when we first need the definition during pattern
         * matching execution */
        if (! is_qualified || memBEGINPs(name, non_pkg_begin, "utf8::")) {
            sv_catpvs(fq_name, DEFERRED_COULD_BE_OFFICIAL_MARKERs);
        }

        /* We also need a trailing newline */
        sv_catpvs(fq_name, "\n");

        *user_defined_ptr = TRUE;
        return fq_name;
    }
}

STATIC bool
S_handle_names_wildcard(pTHX_ const char * wname, /* wildcard name to match */
                              const STRLEN wname_len, /* Its length */
                              SV ** prop_definition,
                              AV ** strings)
{
    /* Deal with Name property wildcard subpatterns; returns TRUE if there were
     * any matches, adding them to prop_definition */

    dSP;

    CV * get_names_info;        /* entry to charnames.pm to get info we need */
    SV * names_string;          /* Contains all character names, except algo */
    SV * algorithmic_names;     /* Contains info about algorithmically
                                   generated character names */
    REGEXP * subpattern_re;     /* The user's pattern to match with */
    struct regexp * prog;       /* The compiled pattern */
    char * all_names_start;     /* lib/unicore/Name.pl string of every
                                   (non-algorithmic) character name */
    char * cur_pos;             /* We match, effectively using /gc; this is
                                   where we are now */
    bool found_matches = FALSE; /* Did any name match so far? */
    SV * empty;                 /* For matching zero length names */
    SV * must_sv;               /* Contains the substring, if any, that must be
                                   in a name for the subpattern to match */
    const char * must;          /* The PV of 'must' */
    STRLEN must_len;            /* And its length */
    SV * syllable_name = NULL;  /* For Hangul syllables */
    const char hangul_prefix[] = "HANGUL SYLLABLE ";
    const STRLEN hangul_prefix_len = sizeof(hangul_prefix) - 1;

    /* By inspection, there are a maximum of 7 bytes in the suffix of a hangul
     * syllable name, and these are immutable and guaranteed by the Unicode
     * standard to never be extended */
    const STRLEN syl_max_len = hangul_prefix_len + 7;

    IV i;

    PERL_ARGS_ASSERT_HANDLE_NAMES_WILDCARD;

    /* Make sure _charnames is loaded.  (The parameters give context
     * for any errors generated */
    get_names_info = get_cv("_charnames::_get_names_info", 0);
    if (! get_names_info) {
        Perl_croak(aTHX_ "panic: Can't find '_charnames::_get_names_info");
    }

    /* Get the charnames data */
    PUSHSTACKi(PERLSI_REGCOMP);
    ENTER ;
    SAVETMPS;
    save_re_context();

    PUSHMARK(SP) ;
    PUTBACK;

    /* Special _charnames entry point that returns the info this routine
     * requires */
    call_sv(MUTABLE_SV(get_names_info), G_LIST);

    SPAGAIN ;

    /* Data structure for names which end in their very own code points */
    algorithmic_names = POPs;
    SvREFCNT_inc_simple_void_NN(algorithmic_names);

    /* The lib/unicore/Name.pl string */
    names_string = POPs;
    SvREFCNT_inc_simple_void_NN(names_string);

    PUTBACK ;
    FREETMPS ;
    LEAVE ;
    POPSTACK;

    if (   ! SvROK(names_string)
        || ! SvROK(algorithmic_names))
    {   /* Perhaps should panic instead XXX */
        SvREFCNT_dec(names_string);
        SvREFCNT_dec(algorithmic_names);
        return FALSE;
    }

    names_string = sv_2mortal(SvRV(names_string));
    all_names_start = SvPVX(names_string);
    cur_pos = all_names_start;

    algorithmic_names= sv_2mortal(SvRV(algorithmic_names));

    /* Compile the subpattern consisting of the name being looked for */
    subpattern_re = compile_wildcard(wname, wname_len, FALSE /* /-i */ );

    must_sv = re_intuit_string(subpattern_re);
    if (must_sv) {
        /* regexec.c can free the re_intuit_string() return. GH #17734 */
        must_sv = sv_2mortal(newSVsv(must_sv));
        must = SvPV(must_sv, must_len);
    }
    else {
        must = "";
        must_len = 0;
    }

    /* (Note: 'must' could contain a NUL.  And yet we use strspn() below on it.
     * This works because the NUL causes the function to return early, thus
     * showing that there are characters in it other than the acceptable ones,
     * which is our desired result.) */

    prog = ReANY(subpattern_re);

    /* If only nothing is matched, skip to where empty names are looked for */
    if (prog->maxlen == 0) {
        goto check_empty;
    }

    /* And match against the string of all names /gc.  Don't even try if it
     * must match a character not found in any name. */
    if (strspn(must, "\n -0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ()") == must_len)
    {
        while (execute_wildcard(subpattern_re,
                                cur_pos,
                                SvEND(names_string),
                                all_names_start, 0,
                                names_string,
                                0))
        { /* Here, matched. */

            /* Note the string entries look like
             *      00001\nSTART OF HEADING\n\n
             * so we could match anywhere in that string.  We have to rule out
             * matching a code point line */
            char * this_name_start = all_names_start
                                                + RX_OFFS_START(subpattern_re,0);
            char * this_name_end   = all_names_start
                                                + RX_OFFS_END(subpattern_re,0);
            char * cp_start;
            char * cp_end;
            UV cp = 0;      /* Silences some compilers */
            AV * this_string = NULL;
            bool is_multi = FALSE;

            /* If matched nothing, advance to next possible match */
            if (this_name_start == this_name_end) {
                cur_pos = (char *) memchr(this_name_end + 1, '\n',
                                          SvEND(names_string) - this_name_end);
                if (cur_pos == NULL) {
                    break;
                }
            }
            else {
                /* Position the next match to start beyond the current returned
                 * entry */
                cur_pos = (char *) memchr(this_name_end, '\n',
                                          SvEND(names_string) - this_name_end);
            }

            /* Back up to the \n just before the beginning of the character. */
            cp_end = (char *) my_memrchr(all_names_start,
                                         '\n',
                                         this_name_start - all_names_start);

            /* If we didn't find a \n, it means it matched somewhere in the
             * initial '00000' in the string, so isn't a real match */
            if (cp_end == NULL) {
                continue;
            }

            this_name_start = cp_end + 1;   /* The name starts just after */
            cp_end--;                       /* the \n, and the code point */
                                            /* ends just before it */

            /* All code points are 5 digits long */
            cp_start = cp_end - 4;

            /* This shouldn't happen, as we found a \n, and the first \n is
             * further along than what we subtracted */
            assert(cp_start >= all_names_start);

            if (cp_start == all_names_start) {
                *prop_definition = add_cp_to_invlist(*prop_definition, 0);
                continue;
            }

            /* If the character is a blank, we either have a named sequence, or
             * something is wrong */
            if (*(cp_start - 1) == ' ') {
                cp_start = (char *) my_memrchr(all_names_start,
                                               '\n',
                                               cp_start - all_names_start);
                cp_start++;
            }

            assert(cp_start != NULL && cp_start >= all_names_start + 2);

            /* Except for the first line in the string, the sequence before the
             * code point is \n\n.  If that isn't the case here, we didn't
             * match the name of a character.  (We could have matched a named
             * sequence, not currently handled */
            if (*(cp_start - 1) != '\n' || *(cp_start - 2) != '\n') {
                continue;
            }

            /* We matched!  Add this to the list */
            found_matches = TRUE;

            /* Loop through all the code points in the sequence */
            while (cp_start < cp_end) {

                /* Calculate this code point from its 5 digits */
                cp = (XDIGIT_VALUE(cp_start[0]) << 16)
                   + (XDIGIT_VALUE(cp_start[1]) << 12)
                   + (XDIGIT_VALUE(cp_start[2]) << 8)
                   + (XDIGIT_VALUE(cp_start[3]) << 4)
                   +  XDIGIT_VALUE(cp_start[4]);

                cp_start += 6;  /* Go past any blank */

                if (cp_start < cp_end || is_multi) {
                    if (this_string == NULL) {
                        this_string = newAV();
                    }

                    is_multi = TRUE;
                    av_push_simple(this_string, newSVuv(cp));
                }
            }

            if (is_multi) { /* Was more than one code point */
                if (*strings == NULL) {
                    *strings = newAV();
                }

                av_push_simple(*strings, (SV *) this_string);
            }
            else {  /* Only a single code point */
                *prop_definition = add_cp_to_invlist(*prop_definition, cp);
            }
        } /* End of loop through the non-algorithmic names string */
    }

    /* There are also character names not in 'names_string'.  These are
     * algorithmically generatable.  Try this pattern on each possible one.
     * (khw originally planned to leave this out given the large number of
     * matches attempted; but the speed turned out to be quite acceptable
     *
     * There are plenty of opportunities to optimize to skip many of the tests.
     * beyond the rudimentary ones already here */

    /* First see if the subpattern matches any of the algorithmic generatable
     * Hangul syllable names.
     *
     * We know none of these syllable names will match if the input pattern
     * requires more bytes than any syllable has, or if the input pattern only
     * matches an empty name, or if the pattern has something it must match and
     * one of the characters in that isn't in any Hangul syllable. */
    if (    prog->minlen <= (SSize_t) syl_max_len
        &&  prog->maxlen > 0
        && (strspn(must, "\n ABCDEGHIJKLMNOPRSTUWY") == must_len))
    {
        /* These constants, names, values, and algorithm are adapted from the
         * Unicode standard, version 5.1, section 3.12, and should never
         * change. */
        const char * JamoL[] = {
            "G", "GG", "N", "D", "DD", "R", "M", "B", "BB",
            "S", "SS", "", "J", "JJ", "C", "K", "T", "P", "H"
        };
        const int LCount = C_ARRAY_LENGTH(JamoL);

        const char * JamoV[] = {
            "A", "AE", "YA", "YAE", "EO", "E", "YEO", "YE", "O", "WA",
            "WAE", "OE", "YO", "U", "WEO", "WE", "WI", "YU", "EU", "YI",
            "I"
        };
        const int VCount = C_ARRAY_LENGTH(JamoV);

        const char * JamoT[] = {
            "", "G", "GG", "GS", "N", "NJ", "NH", "D", "L",
            "LG", "LM", "LB", "LS", "LT", "LP", "LH", "M", "B",
            "BS", "S", "SS", "NG", "J", "C", "K", "T", "P", "H"
        };
        const int TCount = C_ARRAY_LENGTH(JamoT);

        int L, V, T;

        /* This is the initial Hangul syllable code point; each time through the
         * inner loop, it maps to the next higher code point.  For more info,
         * see the Hangul syllable section of the Unicode standard. */
        int cp = 0xAC00;

        syllable_name = sv_2mortal(newSV(syl_max_len));
        sv_setpvn(syllable_name, hangul_prefix, hangul_prefix_len);

        for (L = 0; L < LCount; L++) {
            for (V = 0; V < VCount; V++) {
                for (T = 0; T < TCount; T++) {

                    /* Truncate back to the prefix, which is unvarying */
                    SvCUR_set(syllable_name, hangul_prefix_len);

                    sv_catpv(syllable_name, JamoL[L]);
                    sv_catpv(syllable_name, JamoV[V]);
                    sv_catpv(syllable_name, JamoT[T]);

                    if (execute_wildcard(subpattern_re,
                                SvPVX(syllable_name),
                                SvEND(syllable_name),
                                SvPVX(syllable_name), 0,
                                syllable_name,
                                0))
                    {
                        *prop_definition = add_cp_to_invlist(*prop_definition,
                                                             cp);
                        found_matches = TRUE;
                    }

                    cp++;
                }
            }
        }
    }

    /* The rest of the algorithmically generatable names are of the form
     * "PREFIX-code_point".  The prefixes and the code point limits of each
     * were returned to us in the array 'algorithmic_names' from data in
     * lib/unicore/Name.pm.  'code_point' in the name is expressed in hex. */
    for (i = 0; i <= av_top_index((AV *) algorithmic_names); i++) {
        IV j;

        /* Each element of the array is a hash, giving the details for the
         * series of names it covers.  There is the base name of the characters
         * in the series, and the low and high code points in the series.  And,
         * for optimization purposes a string containing all the legal
         * characters that could possibly be in a name in this series. */
        HV * this_series = (HV *) SvRV(* av_fetch((AV *) algorithmic_names, i, 0));
        SV * prefix = * hv_fetchs(this_series, "name", 0);
        IV low = SvIV(* hv_fetchs(this_series, "low", 0));
        IV high = SvIV(* hv_fetchs(this_series, "high", 0));
        char * legal = SvPVX(* hv_fetchs(this_series, "legal", 0));

        /* Pre-allocate an SV with enough space */
        SV * algo_name = sv_2mortal(Perl_newSVpvf(aTHX_ "%s-0000",
                                                        SvPVX(prefix)));
        if (high >= 0x10000) {
            sv_catpvs(algo_name, "0");
        }

        /* This series can be skipped entirely if the pattern requires
         * something longer than any name in the series, or can only match an
         * empty name, or contains a character not found in any name in the
         * series */
        if (    prog->minlen <= (SSize_t) SvCUR(algo_name)
            &&  prog->maxlen > 0
            && (strspn(must, legal) == must_len))
        {
            for (j = low; j <= high; j++) { /* For each code point in the series */

                /* Get its name, and see if it matches the subpattern */
                Perl_sv_setpvf(aTHX_ algo_name, "%s-%X", SvPVX(prefix),
                                     (unsigned) j);

                if (execute_wildcard(subpattern_re,
                                    SvPVX(algo_name),
                                    SvEND(algo_name),
                                    SvPVX(algo_name), 0,
                                    algo_name,
                                    0))
                {
                    *prop_definition = add_cp_to_invlist(*prop_definition, j);
                    found_matches = TRUE;
                }
            }
        }
    }

  check_empty:
    /* Finally, see if the subpattern matches an empty string */
    empty = newSVpvs("");
    if (execute_wildcard(subpattern_re,
                         SvPVX(empty),
                         SvEND(empty),
                         SvPVX(empty), 0,
                         empty,
                         0))
    {
        /* Many code points have empty names.  Currently these are the \p{GC=C}
         * ones, minus CC and CF */

        SV * empty_names_ref = get_prop_definition(UNI_C);
        SV * empty_names = invlist_clone(empty_names_ref, NULL);

        SV * subtract = get_prop_definition(UNI_CC);

        _invlist_subtract(empty_names, subtract, &empty_names);
        SvREFCNT_dec_NN(empty_names_ref);
        SvREFCNT_dec_NN(subtract);

        subtract = get_prop_definition(UNI_CF);
        _invlist_subtract(empty_names, subtract, &empty_names);
        SvREFCNT_dec_NN(subtract);

        _invlist_union(*prop_definition, empty_names, prop_definition);
        found_matches = TRUE;
        SvREFCNT_dec_NN(empty_names);
    }
    SvREFCNT_dec_NN(empty);

#if 0
    /* If we ever were to accept aliases for, say private use names, we would
     * need to do something fancier to find empty names.  The code below works
     * (at the time it was written), and is slower than the above */
    const char empties_pat[] = "^.";
    if (strNE(name, empties_pat)) {
        SV * empty = newSVpvs("");
        if (execute_wildcard(subpattern_re,
                    SvPVX(empty),
                    SvEND(empty),
                    SvPVX(empty), 0,
                    empty,
                    0))
        {
            SV * empties = NULL;

            (void) handle_names_wildcard(empties_pat, strlen(empties_pat), &empties);

            _invlist_union_complement_2nd(*prop_definition, empties, prop_definition);
            SvREFCNT_dec_NN(empties);

            found_matches = TRUE;
        }
        SvREFCNT_dec_NN(empty);
    }
#endif

    SvREFCNT_dec_NN(subpattern_re);
    return found_matches;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
