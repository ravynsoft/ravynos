#ifdef PERL_EXT_RE_BUILD
#include "re_top.h"
#endif

#include "EXTERN.h"
#define PERL_IN_REGEX_ENGINE
#define PERL_IN_REGCOMP_ANY
#define PERL_IN_REGCOMP_STUDY_C
#include "perl.h"

#ifdef PERL_IN_XSUB_RE
#  include "re_comp.h"
#else
#  include "regcomp.h"
#endif

#include "invlist_inline.h"
#include "unicode_constants.h"
#include "regcomp_internal.h"

#define INIT_AND_WITHP \
    assert(!and_withp); \
    Newx(and_withp, 1, regnode_ssc); \
    SAVEFREEPV(and_withp)


STATIC void
S_unwind_scan_frames(pTHX_ const void *p)
{
    PERL_ARGS_ASSERT_UNWIND_SCAN_FRAMES;
    scan_frame *f= (scan_frame *)p;
    do {
        scan_frame *n= f->next_frame;
        Safefree(f);
        f= n;
    } while (f);
}

/* Follow the next-chain of the current node and optimize away
   all the NOTHINGs from it.
 */
STATIC void
S_rck_elide_nothing(pTHX_ regnode *node)
{
    PERL_ARGS_ASSERT_RCK_ELIDE_NOTHING;

    if (OP(node) != CURLYX) {
        const int max = (REGNODE_OFF_BY_ARG(OP(node))
                        ? I32_MAX
                          /* I32 may be smaller than U16 on CRAYs! */
                        : (I32_MAX < U16_MAX ? I32_MAX : U16_MAX));
        int off = (REGNODE_OFF_BY_ARG(OP(node)) ? ARG1u(node) : NEXT_OFF(node));
        int noff;
        regnode *n = node;

        /* Skip NOTHING and LONGJMP. */
        while (
            (n = regnext(n))
            && (
                (REGNODE_TYPE(OP(n)) == NOTHING && (noff = NEXT_OFF(n)))
                || ((OP(n) == LONGJMP) && (noff = ARG1u(n)))
            )
            && off + noff < max
        ) {
            off += noff;
        }
        if (REGNODE_OFF_BY_ARG(OP(node)))
            ARG1u(node) = off;
        else
            NEXT_OFF(node) = off;
    }
    return;
}


/*
 * As best we can, determine the characters that can match the start of
 * the given EXACTF-ish node.  This is for use in creating ssc nodes, so there
 * can be false positive matches
 *
 * Returns the invlist as a new SV*; it is the caller's responsibility to
 * call SvREFCNT_dec() when done with it.
 */
STATIC SV*
S_make_exactf_invlist(pTHX_ RExC_state_t *pRExC_state, regnode *node)
{
    const U8 * s = (U8*)STRING(node);
    SSize_t bytelen = STR_LEN(node);
    UV uc;
    /* Start out big enough for 2 separate code points */
    SV* invlist = _new_invlist(4);

    PERL_ARGS_ASSERT_MAKE_EXACTF_INVLIST;

    if (! UTF) {
        uc = *s;

        /* We punt and assume can match anything if the node begins
         * with a multi-character fold.  Things are complicated.  For
         * example, /ffi/i could match any of:
         *  "\N{LATIN SMALL LIGATURE FFI}"
         *  "\N{LATIN SMALL LIGATURE FF}I"
         *  "F\N{LATIN SMALL LIGATURE FI}"
         *  plus several other things; and making sure we have all the
         *  possibilities is hard. */
        if (is_MULTI_CHAR_FOLD_latin1_safe(s, s + bytelen)) {
            invlist = _add_range_to_invlist(invlist, 0, UV_MAX);
        }
        else {
            /* Any Latin1 range character can potentially match any
             * other depending on the locale, and in Turkic locales, 'I' and
             * 'i' can match U+130 and U+131 */
            if (OP(node) == EXACTFL) {
                _invlist_union(invlist, PL_Latin1, &invlist);
                if (isALPHA_FOLD_EQ(uc, 'I')) {
                    invlist = add_cp_to_invlist(invlist,
                                                LATIN_SMALL_LETTER_DOTLESS_I);
                    invlist = add_cp_to_invlist(invlist,
                                        LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE);
                }
            }
            else {
                /* But otherwise, it matches at least itself.  We can
                 * quickly tell if it has a distinct fold, and if so,
                 * it matches that as well */
                invlist = add_cp_to_invlist(invlist, uc);
                if (IS_IN_SOME_FOLD_L1(uc))
                    invlist = add_cp_to_invlist(invlist, PL_fold_latin1[uc]);
            }

            /* Some characters match above-Latin1 ones under /i.  This
             * is true of EXACTFL ones when the locale is UTF-8 */
            if (HAS_NONLATIN1_SIMPLE_FOLD_CLOSURE(uc)
                && (! isASCII(uc) || ! inRANGE(OP(node), EXACTFAA,
                                                         EXACTFAA_NO_TRIE)))
            {
                add_above_Latin1_folds(pRExC_state, (U8) uc, &invlist);
            }
        }
    }
    else {  /* Pattern is UTF-8 */
        U8 folded[UTF8_MAX_FOLD_CHAR_EXPAND * UTF8_MAXBYTES_CASE + 1] = { '\0' };
        const U8* e = s + bytelen;
        IV fc;

        fc = uc = utf8_to_uvchr_buf(s, s + bytelen, NULL);

        /* The only code points that aren't folded in a UTF EXACTFish
         * node are the problematic ones in EXACTFL nodes */
        if (OP(node) == EXACTFL && is_PROBLEMATIC_LOCALE_FOLDEDS_START_cp(uc)) {
            /* We need to check for the possibility that this EXACTFL
             * node begins with a multi-char fold.  Therefore we fold
             * the first few characters of it so that we can make that
             * check */
            U8 *d = folded;
            int i;

            fc = -1;
            for (i = 0; i < UTF8_MAX_FOLD_CHAR_EXPAND && s < e; i++) {
                if (isASCII(*s)) {
                    *(d++) = (U8) toFOLD(*s);
                    if (fc < 0) {       /* Save the first fold */
                        fc = *(d-1);
                    }
                    s++;
                }
                else {
                    STRLEN len;
                    UV fold = toFOLD_utf8_safe(s, e, d, &len);
                    if (fc < 0) {       /* Save the first fold */
                        fc = fold;
                    }
                    d += len;
                    s += UTF8SKIP(s);
                }
            }

            /* And set up so the code below that looks in this folded
             * buffer instead of the node's string */
            e = d;
            s = folded;
        }

        /* When we reach here 's' points to the fold of the first
         * character(s) of the node; and 'e' points to far enough along
         * the folded string to be just past any possible multi-char
         * fold.
         *
         * Like the non-UTF case above, we punt if the node begins with a
         * multi-char fold  */

        if (is_MULTI_CHAR_FOLD_utf8_safe(s, e)) {
            invlist = _add_range_to_invlist(invlist, 0, UV_MAX);
        }
        else {  /* Single char fold */
            unsigned int k;
            U32 first_fold;
            const U32 * remaining_folds;
            Size_t folds_count;

            /* It matches itself */
            invlist = add_cp_to_invlist(invlist, fc);

            /* ... plus all the things that fold to it, which are found in
             * PL_utf8_foldclosures */
            folds_count = _inverse_folds(fc, &first_fold,
                                                &remaining_folds);
            for (k = 0; k < folds_count; k++) {
                UV c = (k == 0) ? first_fold : remaining_folds[k-1];

                /* /aa doesn't allow folds between ASCII and non- */
                if (   inRANGE(OP(node), EXACTFAA, EXACTFAA_NO_TRIE)
                    && isASCII(c) != isASCII(fc))
                {
                    continue;
                }

                invlist = add_cp_to_invlist(invlist, c);
            }

            if (OP(node) == EXACTFL) {

                /* If either [iI] are present in an EXACTFL node the above code
                 * should have added its normal case pair, but under a Turkish
                 * locale they could match instead the case pairs from it.  Add
                 * those as potential matches as well */
                if (isALPHA_FOLD_EQ(fc, 'I')) {
                    invlist = add_cp_to_invlist(invlist,
                                                LATIN_SMALL_LETTER_DOTLESS_I);
                    invlist = add_cp_to_invlist(invlist,
                                        LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE);
                }
                else if (fc == LATIN_SMALL_LETTER_DOTLESS_I) {
                    invlist = add_cp_to_invlist(invlist, 'I');
                }
                else if (fc == LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE) {
                    invlist = add_cp_to_invlist(invlist, 'i');
                }
            }
        }
    }

    return invlist;
}


/* Mark that we cannot extend a found fixed substring at this point.
   Update the longest found anchored substring or the longest found
   floating substrings if needed. */

void
Perl_scan_commit(pTHX_ const RExC_state_t *pRExC_state, scan_data_t *data,
                    SSize_t *minlenp, int is_inf)
{
    const STRLEN l = CHR_SVLEN(data->last_found);
    SV * const longest_sv = data->substrs[data->cur_is_floating].str;
    const STRLEN old_l = CHR_SVLEN(longest_sv);
    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_SCAN_COMMIT;

    if ((l >= old_l) && ((l > old_l) || (data->flags & SF_BEFORE_EOL))) {
        const U8 i = data->cur_is_floating;
        SvSetMagicSV(longest_sv, data->last_found);
        data->substrs[i].min_offset = l ? data->last_start_min : data->pos_min;

        if (!i) /* fixed */
            data->substrs[0].max_offset = data->substrs[0].min_offset;
        else { /* float */
            data->substrs[1].max_offset =
                      (is_inf)
                       ? OPTIMIZE_INFTY
                       : (l
                          ? data->last_start_max
                          : (data->pos_delta > OPTIMIZE_INFTY - data->pos_min
                                         ? OPTIMIZE_INFTY
                                         : data->pos_min + data->pos_delta));
        }

        data->substrs[i].flags &= ~SF_BEFORE_EOL;
        data->substrs[i].flags |= data->flags & SF_BEFORE_EOL;
        data->substrs[i].minlenp = minlenp;
        data->substrs[i].lookbehind = 0;
    }

    SvCUR_set(data->last_found, 0);
    {
        SV * const sv = data->last_found;
        if (SvUTF8(sv) && SvMAGICAL(sv)) {
            MAGIC * const mg = mg_find(sv, PERL_MAGIC_utf8);
            if (mg)
                mg->mg_len = 0;
        }
    }
    data->last_end = -1;
    data->flags &= ~SF_BEFORE_EOL;
    DEBUG_STUDYDATA("commit", data, 0, is_inf, -1, -1, -1);
}

/* An SSC is just a regnode_charclass_posix with an extra field: the inversion
 * list that describes which code points it matches */

STATIC void
S_ssc_anything(pTHX_ regnode_ssc *ssc)
{
    /* Set the SSC 'ssc' to match an empty string or any code point */

    PERL_ARGS_ASSERT_SSC_ANYTHING;

    assert(is_ANYOF_SYNTHETIC(ssc));

    /* mortalize so won't leak */
    ssc->invlist = sv_2mortal(_add_range_to_invlist(NULL, 0, UV_MAX));
    ANYOF_FLAGS(ssc) |= SSC_MATCHES_EMPTY_STRING;  /* Plus matches empty */
}

STATIC int
S_ssc_is_anything(const regnode_ssc *ssc)
{
    /* Returns TRUE if the SSC 'ssc' can match the empty string and any code
     * point; FALSE otherwise.  Thus, this is used to see if using 'ssc' buys
     * us anything: if the function returns TRUE, 'ssc' hasn't been restricted
     * in any way, so there's no point in using it */

    UV start = 0, end = 0;  /* Initialize due to messages from dumb compiler */
    bool ret;

    PERL_ARGS_ASSERT_SSC_IS_ANYTHING;

    assert(is_ANYOF_SYNTHETIC(ssc));

    if (! (ANYOF_FLAGS(ssc) & SSC_MATCHES_EMPTY_STRING)) {
        return FALSE;
    }

    /* See if the list consists solely of the range 0 - Infinity */
    invlist_iterinit(ssc->invlist);
    ret = invlist_iternext(ssc->invlist, &start, &end)
          && start == 0
          && end == UV_MAX;

    invlist_iterfinish(ssc->invlist);

    if (ret) {
        return TRUE;
    }

    /* If e.g., both \w and \W are set, matches everything */
    if (ANYOF_POSIXL_SSC_TEST_ANY_SET(ssc)) {
        int i;
        for (i = 0; i < ANYOF_POSIXL_MAX; i += 2) {
            if (ANYOF_POSIXL_TEST(ssc, i) && ANYOF_POSIXL_TEST(ssc, i+1)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

void
Perl_ssc_init(pTHX_ const RExC_state_t *pRExC_state, regnode_ssc *ssc)
{
    /* Initializes the SSC 'ssc'.  This includes setting it to match an empty
     * string, any code point, or any posix class under locale */

    PERL_ARGS_ASSERT_SSC_INIT;

    Zero(ssc, 1, regnode_ssc);
    set_ANYOF_SYNTHETIC(ssc);
    ARG1u_SET(ssc, ANYOF_MATCHES_ALL_OUTSIDE_BITMAP_VALUE);
    ssc_anything(ssc);

    /* If any portion of the regex is to operate under locale rules that aren't
     * fully known at compile time, initialization includes it.  The reason
     * this isn't done for all regexes is that the optimizer was written under
     * the assumption that locale was all-or-nothing.  Given the complexity and
     * lack of documentation in the optimizer, and that there are inadequate
     * test cases for locale, many parts of it may not work properly, it is
     * safest to avoid locale unless necessary. */
    if (RExC_contains_locale) {
        ANYOF_POSIXL_SETALL(ssc);
    }
    else {
        ANYOF_POSIXL_ZERO(ssc);
    }
}

STATIC int
S_ssc_is_cp_posixl_init(const RExC_state_t *pRExC_state,
                        const regnode_ssc *ssc)
{
    /* Returns TRUE if the SSC 'ssc' is in its initial state with regard only
     * to the list of code points matched, and locale posix classes; hence does
     * not check its flags) */

    UV start = 0, end = 0;  /* Initialize due to messages from dumb compiler */
    bool ret;

    PERL_ARGS_ASSERT_SSC_IS_CP_POSIXL_INIT;

    assert(is_ANYOF_SYNTHETIC(ssc));

    invlist_iterinit(ssc->invlist);
    ret = invlist_iternext(ssc->invlist, &start, &end)
          && start == 0
          && end == UV_MAX;

    invlist_iterfinish(ssc->invlist);

    if (! ret) {
        return FALSE;
    }

    if (RExC_contains_locale && ! ANYOF_POSIXL_SSC_TEST_ALL_SET(ssc)) {
        return FALSE;
    }

    return TRUE;
}


STATIC SV*
S_get_ANYOF_cp_list_for_ssc(pTHX_ const RExC_state_t *pRExC_state,
                               const regnode_charclass* const node)
{
    /* Returns a mortal inversion list defining which code points are matched
     * by 'node', which is of ANYOF-ish type .  Handles complementing the
     * result if appropriate.  If some code points aren't knowable at this
     * time, the returned list must, and will, contain every code point that is
     * a possibility. */

    SV* invlist = NULL;
    SV* only_utf8_locale_invlist = NULL;
    bool new_node_has_latin1 = FALSE;
    const U8 flags = (REGNODE_TYPE(OP(node)) == ANYOF)
                      ? ANYOF_FLAGS(node)
                      : 0;

    PERL_ARGS_ASSERT_GET_ANYOF_CP_LIST_FOR_SSC;

    /* Look at the data structure created by S_set_ANYOF_arg() */
    if (ANYOF_MATCHES_ALL_OUTSIDE_BITMAP(node)) {
        invlist = sv_2mortal(_new_invlist(1));
        invlist = _add_range_to_invlist(invlist, NUM_ANYOF_CODE_POINTS, UV_MAX);
    }
    else if (ANYOF_HAS_AUX(node)) {
        const U32 n = ARG1u(node);
        SV * const rv = MUTABLE_SV(RExC_rxi->data->data[n]);
        AV * const av = MUTABLE_AV(SvRV(rv));
        SV **const ary = AvARRAY(av);

        if (av_tindex_skip_len_mg(av) >= DEFERRED_USER_DEFINED_INDEX) {

            /* Here there are things that won't be known until runtime -- we
             * have to assume it could be anything */
            invlist = sv_2mortal(_new_invlist(1));
            return _add_range_to_invlist(invlist, 0, UV_MAX);
        }
        else if (ary[INVLIST_INDEX]) {

            /* Use the node's inversion list */
            invlist = sv_2mortal(invlist_clone(ary[INVLIST_INDEX], NULL));
        }

        /* Get the code points valid only under UTF-8 locales */
        if (   (flags & ANYOFL_FOLD)
            &&  av_tindex_skip_len_mg(av) >= ONLY_LOCALE_MATCHES_INDEX)
        {
            only_utf8_locale_invlist = ary[ONLY_LOCALE_MATCHES_INDEX];
        }
    }

    if (! invlist) {
        invlist = sv_2mortal(_new_invlist(0));
    }

    /* An ANYOF node contains a bitmap for the first NUM_ANYOF_CODE_POINTS
     * code points, and an inversion list for the others, but if there are code
     * points that should match only conditionally on the target string being
     * UTF-8, those are placed in the inversion list, and not the bitmap.
     * Since there are circumstances under which they could match, they are
     * included in the SSC.  But if the ANYOF node is to be inverted, we have
     * to exclude them here, so that when we invert below, the end result
     * actually does include them.  (Think about "\xe0" =~ /[^\xc0]/di;).  We
     * have to do this here before we add the unconditionally matched code
     * points */
    if (flags & ANYOF_INVERT) {
        _invlist_intersection_complement_2nd(invlist,
                                             PL_UpperLatin1,
                                             &invlist);
    }

    /* Add in the points from the bit map */
    if (REGNODE_TYPE(OP(node)) == ANYOF){
        for (unsigned i = 0; i < NUM_ANYOF_CODE_POINTS; i++) {
            if (ANYOF_BITMAP_TEST(node, i)) {
                unsigned int start = i++;

                for (;    i < NUM_ANYOF_CODE_POINTS
                       && ANYOF_BITMAP_TEST(node, i); ++i)
                {
                    /* empty */
                }
                invlist = _add_range_to_invlist(invlist, start, i-1);
                new_node_has_latin1 = TRUE;
            }
        }
    }

    /* If this can match all upper Latin1 code points, have to add them
     * as well.  But don't add them if inverting, as when that gets done below,
     * it would exclude all these characters, including the ones it shouldn't
     * that were added just above */
    if ( ! (flags & ANYOF_INVERT)
        &&  OP(node) == ANYOFD
        && (flags & ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared))
    {
        _invlist_union(invlist, PL_UpperLatin1, &invlist);
    }

    /* Similarly for these */
    if (ANYOF_MATCHES_ALL_OUTSIDE_BITMAP(node)) {
        _invlist_union_complement_2nd(invlist, PL_InBitmap, &invlist);
    }

    if (flags & ANYOF_INVERT) {
        _invlist_invert(invlist);
    }
    else if (flags & ANYOFL_FOLD) {
        if (new_node_has_latin1) {

            /* These folds are potential in Turkic locales */
            if (_invlist_contains_cp(invlist, 'i')) {
                invlist = add_cp_to_invlist(invlist,
                                        LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE);
            }
            if (_invlist_contains_cp(invlist, 'I')) {
                invlist = add_cp_to_invlist(invlist,
                                                LATIN_SMALL_LETTER_DOTLESS_I);
            }

            /* Under /li, any 0-255 could fold to any other 0-255, depending on
             * the locale.  We can skip this if there are no 0-255 at all. */
            _invlist_union(invlist, PL_Latin1, &invlist);
        }
        else {
            if (_invlist_contains_cp(invlist, LATIN_SMALL_LETTER_DOTLESS_I)) {
                invlist = add_cp_to_invlist(invlist, 'I');
            }
            if (_invlist_contains_cp(invlist,
                                        LATIN_CAPITAL_LETTER_I_WITH_DOT_ABOVE))
            {
                invlist = add_cp_to_invlist(invlist, 'i');
            }
        }
    }

    /* Similarly add the UTF-8 locale possible matches.  These have to be
     * deferred until after the non-UTF-8 locale ones are taken care of just
     * above, or it leads to wrong results under ANYOF_INVERT */
    if (only_utf8_locale_invlist) {
        _invlist_union_maybe_complement_2nd(invlist,
                                            only_utf8_locale_invlist,
                                            flags & ANYOF_INVERT,
                                            &invlist);
    }

    return invlist;
}

/* 'AND' a given class with another one.  Can create false positives.  'ssc'
 * should not be inverted. */

STATIC void
S_ssc_and(pTHX_ const RExC_state_t *pRExC_state, regnode_ssc *ssc,
                const regnode_charclass *and_with)
{
    /* Accumulate into SSC 'ssc' its 'AND' with 'and_with', which is either
     * another SSC or a regular ANYOF class.  Can create false positives. */

    SV* anded_cp_list;
    U8  and_with_flags = (REGNODE_TYPE(OP(and_with)) == ANYOF)
                          ? ANYOF_FLAGS(and_with)
                          : 0;
    U8  anded_flags;

    PERL_ARGS_ASSERT_SSC_AND;

    assert(is_ANYOF_SYNTHETIC(ssc));

    /* 'and_with' is used as-is if it too is an SSC; otherwise have to extract
     * the code point inversion list and just the relevant flags */
    if (is_ANYOF_SYNTHETIC(and_with)) {
        anded_cp_list = ((regnode_ssc *)and_with)->invlist;
        anded_flags = and_with_flags;

        /* XXX This is a kludge around what appears to be deficiencies in the
         * optimizer.  If we make S_ssc_anything() add in the WARN_SUPER flag,
         * there are paths through the optimizer where it doesn't get weeded
         * out when it should.  And if we don't make some extra provision for
         * it like the code just below, it doesn't get added when it should.
         * This solution is to add it only when AND'ing, which is here, and
         * only when what is being AND'ed is the pristine, original node
         * matching anything.  Thus it is like adding it to ssc_anything() but
         * only when the result is to be AND'ed.  Probably the same solution
         * could be adopted for the same problem we have with /l matching,
         * which is solved differently in S_ssc_init(), and that would lead to
         * fewer false positives than that solution has.  But if this solution
         * creates bugs, the consequences are only that a warning isn't raised
         * that should be; while the consequences for having /l bugs is
         * incorrect matches */
        if (ssc_is_anything((regnode_ssc *)and_with)) {
            anded_flags |= ANYOF_WARN_SUPER__shared;
        }
    }
    else {
        anded_cp_list = get_ANYOF_cp_list_for_ssc(pRExC_state, and_with);
        if (OP(and_with) == ANYOFD) {
            anded_flags = and_with_flags & ANYOF_COMMON_FLAGS;
        }
        else {
            anded_flags = and_with_flags
                            & ( ANYOF_COMMON_FLAGS
                               |ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared
                               |ANYOF_HAS_EXTRA_RUNTIME_MATCHES);
            if (and_with_flags & ANYOFL_UTF8_LOCALE_REQD) {
                anded_flags &= ANYOF_HAS_EXTRA_RUNTIME_MATCHES;
            }
        }
    }

    ANYOF_FLAGS(ssc) &= anded_flags;

    /* Below, C1 is the list of code points in 'ssc'; P1, its posix classes.
     * C2 is the list of code points in 'and-with'; P2, its posix classes.
     * 'and_with' may be inverted.  When not inverted, we have the situation of
     * computing:
     *  (C1 | P1) & (C2 | P2)
     *                     =  (C1 & (C2 | P2)) | (P1 & (C2 | P2))
     *                     =  ((C1 & C2) | (C1 & P2)) | ((P1 & C2) | (P1 & P2))
     *                    <=  ((C1 & C2) |       P2)) | ( P1       | (P1 & P2))
     *                    <=  ((C1 & C2) | P1 | P2)
     * Alternatively, the last few steps could be:
     *                     =  ((C1 & C2) | (C1 & P2)) | ((P1 & C2) | (P1 & P2))
     *                    <=  ((C1 & C2) |  C1      ) | (      C2  | (P1 & P2))
     *                    <=  (C1 | C2 | (P1 & P2))
     * We favor the second approach if either P1 or P2 is non-empty.  This is
     * because these components are a barrier to doing optimizations, as what
     * they match cannot be known until the moment of matching as they are
     * dependent on the current locale, 'AND"ing them likely will reduce or
     * eliminate them.
     * But we can do better if we know that C1,P1 are in their initial state (a
     * frequent occurrence), each matching everything:
     *  (<everything>) & (C2 | P2) =  C2 | P2
     * Similarly, if C2,P2 are in their initial state (again a frequent
     * occurrence), the result is a no-op
     *  (C1 | P1) & (<everything>) =  C1 | P1
     *
     * Inverted, we have
     *  (C1 | P1) & ~(C2 | P2)  =  (C1 | P1) & (~C2 & ~P2)
     *                          =  (C1 & (~C2 & ~P2)) | (P1 & (~C2 & ~P2))
     *                         <=  (C1 & ~C2) | (P1 & ~P2)
     * */

    if ((and_with_flags & ANYOF_INVERT)
        && ! is_ANYOF_SYNTHETIC(and_with))
    {
        unsigned int i;

        ssc_intersection(ssc,
                         anded_cp_list,
                         FALSE /* Has already been inverted */
                         );

        /* If either P1 or P2 is empty, the intersection will be also; can skip
         * the loop */
        if (! (and_with_flags & ANYOF_MATCHES_POSIXL)) {
            ANYOF_POSIXL_ZERO(ssc);
        }
        else if (ANYOF_POSIXL_SSC_TEST_ANY_SET(ssc)) {

            /* Note that the Posix class component P from 'and_with' actually
             * looks like:
             *      P = Pa | Pb | ... | Pn
             * where each component is one posix class, such as in [\w\s].
             * Thus
             *      ~P = ~(Pa | Pb | ... | Pn)
             *         = ~Pa & ~Pb & ... & ~Pn
             *        <= ~Pa | ~Pb | ... | ~Pn
             * The last is something we can easily calculate, but unfortunately
             * is likely to have many false positives.  We could do better
             * in some (but certainly not all) instances if two classes in
             * P have known relationships.  For example
             *      :lower: <= :alpha: <= :alnum: <= \w <= :graph: <= :print:
             * So
             *      :lower: & :print: = :lower:
             * And similarly for classes that must be disjoint.  For example,
             * since \s and \w can have no elements in common based on rules in
             * the POSIX standard,
             *      \w & ^\S = nothing
             * Unfortunately, some vendor locales do not meet the Posix
             * standard, in particular almost everything by Microsoft.
             * The loop below just changes e.g., \w into \W and vice versa */

            regnode_charclass_posixl temp;
            int add = 1;    /* To calculate the index of the complement */

            Zero(&temp, 1, regnode_charclass_posixl);
            ANYOF_POSIXL_ZERO(&temp);
            for (i = 0; i < ANYOF_MAX; i++) {
                assert(i % 2 != 0
                       || ! ANYOF_POSIXL_TEST((regnode_charclass_posixl*) and_with, i)
                       || ! ANYOF_POSIXL_TEST((regnode_charclass_posixl*) and_with, i + 1));

                if (ANYOF_POSIXL_TEST((regnode_charclass_posixl*) and_with, i)) {
                    ANYOF_POSIXL_SET(&temp, i + add);
                }
                add = 0 - add; /* 1 goes to -1; -1 goes to 1 */
            }
            ANYOF_POSIXL_AND(&temp, ssc);

        } /* else ssc already has no posixes */
    } /* else: Not inverted.  This routine is a no-op if 'and_with' is an SSC
         in its initial state */
    else if (! is_ANYOF_SYNTHETIC(and_with)
             || ! ssc_is_cp_posixl_init(pRExC_state, (regnode_ssc *)and_with))
    {
        /* But if 'ssc' is in its initial state, the result is just 'and_with';
         * copy it over 'ssc' */
        if (ssc_is_cp_posixl_init(pRExC_state, ssc)) {
            if (is_ANYOF_SYNTHETIC(and_with)) {
                StructCopy(and_with, ssc, regnode_ssc);
            }
            else {
                ssc->invlist = anded_cp_list;
                ANYOF_POSIXL_ZERO(ssc);
                if (and_with_flags & ANYOF_MATCHES_POSIXL) {
                    ANYOF_POSIXL_OR((regnode_charclass_posixl*) and_with, ssc);
                }
            }
        }
        else if (ANYOF_POSIXL_SSC_TEST_ANY_SET(ssc)
                 || (and_with_flags & ANYOF_MATCHES_POSIXL))
        {
            /* One or the other of P1, P2 is non-empty. */
            if (and_with_flags & ANYOF_MATCHES_POSIXL) {
                ANYOF_POSIXL_AND((regnode_charclass_posixl*) and_with, ssc);
            }
            ssc_union(ssc, anded_cp_list, FALSE);
        }
        else { /* P1 = P2 = empty */
            ssc_intersection(ssc, anded_cp_list, FALSE);
        }
    }
}

STATIC void
S_ssc_or(pTHX_ const RExC_state_t *pRExC_state, regnode_ssc *ssc,
               const regnode_charclass *or_with)
{
    /* Accumulate into SSC 'ssc' its 'OR' with 'or_with', which is either
     * another SSC or a regular ANYOF class.  Can create false positives if
     * 'or_with' is to be inverted. */

    SV* ored_cp_list;
    U8 ored_flags;
    U8  or_with_flags = (REGNODE_TYPE(OP(or_with)) == ANYOF)
                         ? ANYOF_FLAGS(or_with)
                         : 0;

    PERL_ARGS_ASSERT_SSC_OR;

    assert(is_ANYOF_SYNTHETIC(ssc));

    /* 'or_with' is used as-is if it too is an SSC; otherwise have to extract
     * the code point inversion list and just the relevant flags */
    if (is_ANYOF_SYNTHETIC(or_with)) {
        ored_cp_list = ((regnode_ssc*) or_with)->invlist;
        ored_flags = or_with_flags;
    }
    else {
        ored_cp_list = get_ANYOF_cp_list_for_ssc(pRExC_state, or_with);
        ored_flags = or_with_flags & ANYOF_COMMON_FLAGS;
        if (OP(or_with) != ANYOFD) {
            ored_flags |=
                or_with_flags & ( ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared
                                 |ANYOF_HAS_EXTRA_RUNTIME_MATCHES);
            if (or_with_flags & ANYOFL_UTF8_LOCALE_REQD) {
                ored_flags |= ANYOF_HAS_EXTRA_RUNTIME_MATCHES;
            }
        }
    }

    ANYOF_FLAGS(ssc) |= ored_flags;

    /* Below, C1 is the list of code points in 'ssc'; P1, its posix classes.
     * C2 is the list of code points in 'or-with'; P2, its posix classes.
     * 'or_with' may be inverted.  When not inverted, we have the simple
     * situation of computing:
     *  (C1 | P1) | (C2 | P2)  =  (C1 | C2) | (P1 | P2)
     * If P1|P2 yields a situation with both a class and its complement are
     * set, like having both \w and \W, this matches all code points, and we
     * can delete these from the P component of the ssc going forward.  XXX We
     * might be able to delete all the P components, but I (khw) am not certain
     * about this, and it is better to be safe.
     *
     * Inverted, we have
     *  (C1 | P1) | ~(C2 | P2)  =  (C1 | P1) | (~C2 & ~P2)
     *                         <=  (C1 | P1) | ~C2
     *                         <=  (C1 | ~C2) | P1
     * (which results in actually simpler code than the non-inverted case)
     * */

    if ((or_with_flags & ANYOF_INVERT)
        && ! is_ANYOF_SYNTHETIC(or_with))
    {
        /* We ignore P2, leaving P1 going forward */
    }   /* else  Not inverted */
    else if (or_with_flags & ANYOF_MATCHES_POSIXL) {
        ANYOF_POSIXL_OR((regnode_charclass_posixl*)or_with, ssc);
        if (ANYOF_POSIXL_SSC_TEST_ANY_SET(ssc)) {
            unsigned int i;
            for (i = 0; i < ANYOF_MAX; i += 2) {
                if (ANYOF_POSIXL_TEST(ssc, i) && ANYOF_POSIXL_TEST(ssc, i + 1))
                {
                    ssc_match_all_cp(ssc);
                    ANYOF_POSIXL_CLEAR(ssc, i);
                    ANYOF_POSIXL_CLEAR(ssc, i+1);
                }
            }
        }
    }

    ssc_union(ssc,
              ored_cp_list,
              FALSE /* Already has been inverted */
              );
}

STATIC void
S_ssc_union(pTHX_ regnode_ssc *ssc, SV* const invlist, const bool invert2nd)
{
    PERL_ARGS_ASSERT_SSC_UNION;

    assert(is_ANYOF_SYNTHETIC(ssc));

    _invlist_union_maybe_complement_2nd(ssc->invlist,
                                        invlist,
                                        invert2nd,
                                        &ssc->invlist);
}

STATIC void
S_ssc_intersection(pTHX_ regnode_ssc *ssc,
                         SV* const invlist,
                         const bool invert2nd)
{
    PERL_ARGS_ASSERT_SSC_INTERSECTION;

    assert(is_ANYOF_SYNTHETIC(ssc));

    _invlist_intersection_maybe_complement_2nd(ssc->invlist,
                                               invlist,
                                               invert2nd,
                                               &ssc->invlist);
}

STATIC void
S_ssc_add_range(pTHX_ regnode_ssc *ssc, const UV start, const UV end)
{
    PERL_ARGS_ASSERT_SSC_ADD_RANGE;

    assert(is_ANYOF_SYNTHETIC(ssc));

    ssc->invlist = _add_range_to_invlist(ssc->invlist, start, end);
}

STATIC void
S_ssc_cp_and(pTHX_ regnode_ssc *ssc, const UV cp)
{
    /* AND just the single code point 'cp' into the SSC 'ssc' */

    SV* cp_list = _new_invlist(2);

    PERL_ARGS_ASSERT_SSC_CP_AND;

    assert(is_ANYOF_SYNTHETIC(ssc));

    cp_list = add_cp_to_invlist(cp_list, cp);
    ssc_intersection(ssc, cp_list,
                     FALSE /* Not inverted */
                     );
    SvREFCNT_dec_NN(cp_list);
}

STATIC void
S_ssc_clear_locale(regnode_ssc *ssc)
{
    /* Set the SSC 'ssc' to not match any locale things */
    PERL_ARGS_ASSERT_SSC_CLEAR_LOCALE;

    assert(is_ANYOF_SYNTHETIC(ssc));

    ANYOF_POSIXL_ZERO(ssc);
    ANYOF_FLAGS(ssc) &= ~ANYOF_LOCALE_FLAGS;
}

bool
Perl_is_ssc_worth_it(const RExC_state_t * pRExC_state, const regnode_ssc * ssc)
{
    /* The synthetic start class is used to hopefully quickly winnow down
     * places where a pattern could start a match in the target string.  If it
     * doesn't really narrow things down that much, there isn't much point to
     * having the overhead of using it.  This function uses some very crude
     * heuristics to decide if to use the ssc or not.
     *
     * It returns TRUE if 'ssc' rules out more than half what it considers to
     * be the "likely" possible matches, but of course it doesn't know what the
     * actual things being matched are going to be; these are only guesses
     *
     * For /l matches, it assumes that the only likely matches are going to be
     *      in the 0-255 range, uniformly distributed, so half of that is 127
     * For /a and /d matches, it assumes that the likely matches will be just
     *      the ASCII range, so half of that is 63
     * For /u and there isn't anything matching above the Latin1 range, it
     *      assumes that that is the only range likely to be matched, and uses
     *      half that as the cut-off: 127.  If anything matches above Latin1,
     *      it assumes that all of Unicode could match (uniformly), except for
     *      non-Unicode code points and things in the General Category "Other"
     *      (unassigned, private use, surrogates, controls and formats).  This
     *      is a much large number. */

    U32 count = 0;      /* Running total of number of code points matched by
                           'ssc' */
    UV start, end;      /* Start and end points of current range in inversion
                           XXX outdated.  UTF-8 locales are common, what about invert? list */
    const U32 max_code_points = (LOC)
                                ?  256
                                : ((  ! UNI_SEMANTICS
                                    ||  invlist_highest(ssc->invlist) < 256)
                                  ? 128
                                  : NON_OTHER_COUNT);
    const U32 max_match = max_code_points / 2;

    PERL_ARGS_ASSERT_IS_SSC_WORTH_IT;

    invlist_iterinit(ssc->invlist);
    while (invlist_iternext(ssc->invlist, &start, &end)) {
        if (start >= max_code_points) {
            break;
        }
        end = MIN(end, max_code_points - 1);
        count += end - start + 1;
        if (count >= max_match) {
            invlist_iterfinish(ssc->invlist);
            return FALSE;
        }
    }

    return TRUE;
}


void
Perl_ssc_finalize(pTHX_ RExC_state_t *pRExC_state, regnode_ssc *ssc)
{
    /* The inversion list in the SSC is marked mortal; now we need a more
     * permanent copy, which is stored the same way that is done in a regular
     * ANYOF node, with the first NUM_ANYOF_CODE_POINTS code points in a bit
     * map */

    SV* invlist = invlist_clone(ssc->invlist, NULL);

    PERL_ARGS_ASSERT_SSC_FINALIZE;

    assert(is_ANYOF_SYNTHETIC(ssc));

    /* The code in this file assumes that all but these flags aren't relevant
     * to the SSC, except SSC_MATCHES_EMPTY_STRING, which should be cleared
     * by the time we reach here */
    assert(! (ANYOF_FLAGS(ssc)
        & ~( ANYOF_COMMON_FLAGS
            |ANYOFD_NON_UTF8_MATCHES_ALL_NON_ASCII__shared
            |ANYOF_HAS_EXTRA_RUNTIME_MATCHES)));

    populate_anyof_bitmap_from_invlist( (regnode *) ssc, &invlist);

    set_ANYOF_arg(pRExC_state, (regnode *) ssc, invlist, NULL, NULL);
    SvREFCNT_dec(invlist);

    /* Make sure is clone-safe */
    ssc->invlist = NULL;

    if (ANYOF_POSIXL_SSC_TEST_ANY_SET(ssc)) {
        ANYOF_FLAGS(ssc) |= ANYOF_MATCHES_POSIXL;
        OP(ssc) = ANYOFPOSIXL;
    }
    else if (RExC_contains_locale) {
        OP(ssc) = ANYOFL;
    }

    assert(! (ANYOF_FLAGS(ssc) & ANYOF_LOCALE_FLAGS) || RExC_contains_locale);
}

/* The below joins as many adjacent EXACTish nodes as possible into a single
 * one.  The regop may be changed if the node(s) contain certain sequences that
 * require special handling.  The joining is only done if:
 * 1) there is room in the current conglomerated node to entirely contain the
 *    next one.
 * 2) they are compatible node types
 *
 * The adjacent nodes actually may be separated by NOTHING-kind nodes, and
 * these get optimized out
 *
 * XXX khw thinks this should be enhanced to fill EXACT (at least) nodes as full
 * as possible, even if that means splitting an existing node so that its first
 * part is moved to the preceding node.  This would maximise the efficiency of
 * memEQ during matching.
 *
 * If a node is to match under /i (folded), the number of characters it matches
 * can be different than its character length if it contains a multi-character
 * fold.  *min_subtract is set to the total delta number of characters of the
 * input nodes.
 *
 * And *unfolded_multi_char is set to indicate whether or not the node contains
 * an unfolded multi-char fold.  This happens when it won't be known until
 * runtime whether the fold is valid or not; namely
 *  1) for EXACTF nodes that contain LATIN SMALL LETTER SHARP S, as only if the
 *      target string being matched against turns out to be UTF-8 is that fold
 *      valid; or
 *  2) for EXACTFL nodes whose folding rules depend on the locale in force at
 *      runtime.
 * (Multi-char folds whose components are all above the Latin1 range are not
 * run-time locale dependent, and have already been folded by the time this
 * function is called.)
 *
 * This is as good a place as any to discuss the design of handling these
 * multi-character fold sequences.  It's been wrong in Perl for a very long
 * time.  There are three code points in Unicode whose multi-character folds
 * were long ago discovered to mess things up.  The previous designs for
 * dealing with these involved assigning a special node for them.  This
 * approach doesn't always work, as evidenced by this example:
 *      "\xDFs" =~ /s\xDF/ui    # Used to fail before these patches
 * Both sides fold to "sss", but if the pattern is parsed to create a node that
 * would match just the \xDF, it won't be able to handle the case where a
 * successful match would have to cross the node's boundary.  The new approach
 * that hopefully generally solves the problem generates an EXACTFUP node
 * that is "sss" in this case.
 *
 * It turns out that there are problems with all multi-character folds, and not
 * just these three.  Now the code is general, for all such cases.  The
 * approach taken is:
 * 1)   This routine examines each EXACTFish node that could contain multi-
 *      character folded sequences.  Since a single character can fold into
 *      such a sequence, the minimum match length for this node is less than
 *      the number of characters in the node.  This routine returns in
 *      *min_subtract how many characters to subtract from the actual
 *      length of the string to get a real minimum match length; it is 0 if
 *      there are no multi-char foldeds.  This delta is used by the caller to
 *      adjust the min length of the match, and the delta between min and max,
 *      so that the optimizer doesn't reject these possibilities based on size
 *      constraints.
 *
 * 2)   For the sequence involving the LATIN SMALL LETTER SHARP S (U+00DF)
 *      under /u, we fold it to 'ss' in regatom(), and in this routine, after
 *      joining, we scan for occurrences of the sequence 'ss' in non-UTF-8
 *      EXACTFU nodes.  The node type of such nodes is then changed to
 *      EXACTFUP, indicating it is problematic, and needs careful handling.
 *      (The procedures in step 1) above are sufficient to handle this case in
 *      UTF-8 encoded nodes.)  The reason this is problematic is that this is
 *      the only case where there is a possible fold length change in non-UTF-8
 *      patterns.  By reserving a special node type for problematic cases, the
 *      far more common regular EXACTFU nodes can be processed faster.
 *      regexec.c takes advantage of this.
 *
 *      EXACTFUP has been created as a grab-bag for (hopefully uncommon)
 *      problematic cases.   These all only occur when the pattern is not
 *      UTF-8.  In addition to the 'ss' sequence where there is a possible fold
 *      length change, it handles the situation where the string cannot be
 *      entirely folded.  The strings in an EXACTFish node are folded as much
 *      as possible during compilation in regcomp.c.  This saves effort in
 *      regex matching.  By using an EXACTFUP node when it is not possible to
 *      fully fold at compile time, regexec.c can know that everything in an
 *      EXACTFU node is folded, so folding can be skipped at runtime.  The only
 *      case where folding in EXACTFU nodes can't be done at compile time is
 *      the presumably uncommon MICRO SIGN, when the pattern isn't UTF-8.  This
 *      is because its fold requires UTF-8 to represent.  Thus EXACTFUP nodes
 *      handle two very different cases.  Alternatively, there could have been
 *      a node type where there are length changes, one for unfolded, and one
 *      for both.  If yet another special case needed to be created, the number
 *      of required node types would have to go to 7.  khw figures that even
 *      though there are plenty of node types to spare, that the maintenance
 *      cost wasn't worth the small speedup of doing it that way, especially
 *      since he thinks the MICRO SIGN is rarely encountered in practice.
 *
 *      There are other cases where folding isn't done at compile time, but
 *      none of them are under /u, and hence not for EXACTFU nodes.  The folds
 *      in EXACTFL nodes aren't known until runtime, and vary as the locale
 *      changes.  Some folds in EXACTF depend on if the runtime target string
 *      is UTF-8 or not.  (regatom() will create an EXACTFU node even under /di
 *      when no fold in it depends on the UTF-8ness of the target string.)
 *
 * 3)   A problem remains for unfolded multi-char folds. (These occur when the
 *      validity of the fold won't be known until runtime, and so must remain
 *      unfolded for now.  This happens for the sharp s in EXACTF and EXACTFAA
 *      nodes when the pattern isn't in UTF-8.  (Note, BTW, that there cannot
 *      be an EXACTF node with a UTF-8 pattern.)  They also occur for various
 *      folds in EXACTFL nodes, regardless of the UTF-ness of the pattern.)
 *      The reason this is a problem is that the optimizer part of regexec.c
 *      (probably unwittingly, in Perl_regexec_flags()) makes an assumption
 *      that a character in the pattern corresponds to at most a single
 *      character in the target string.  (And I do mean character, and not byte
 *      here, unlike other parts of the documentation that have never been
 *      updated to account for multibyte Unicode.)  Sharp s in EXACTF and
 *      EXACTFL nodes can match the two character string 'ss'; in EXACTFAA
 *      nodes it can match "\x{17F}\x{17F}".  These, along with other ones in
 *      EXACTFL nodes, violate the assumption, and they are the only instances
 *      where it is violated.  I'm reluctant to try to change the assumption,
 *      as the code involved is impenetrable to me (khw), so instead the code
 *      here punts.  This routine examines EXACTFL nodes, and (when the pattern
 *      isn't UTF-8) EXACTF and EXACTFAA for such unfolded folds, and returns a
 *      boolean indicating whether or not the node contains such a fold.  When
 *      it is true, the caller sets a flag that later causes the optimizer in
 *      this file to not set values for the floating and fixed string lengths,
 *      and thus avoids the optimizer code in regexec.c that makes the invalid
 *      assumption.  Thus, there is no optimization based on string lengths for
 *      EXACTFL nodes that contain these few folds, nor for non-UTF8-pattern
 *      EXACTF and EXACTFAA nodes that contain the sharp s.  (The reason the
 *      assumption is wrong only in these cases is that all other non-UTF-8
 *      folds are 1-1; and, for UTF-8 patterns, we pre-fold all other folds to
 *      their expanded versions.  (Again, we can't prefold sharp s to 'ss' in
 *      EXACTF nodes because we don't know at compile time if it actually
 *      matches 'ss' or not.  For EXACTF nodes it will match iff the target
 *      string is in UTF-8.  This is in contrast to EXACTFU nodes, where it
 *      always matches; and EXACTFAA where it never does.  In an EXACTFAA node
 *      in a UTF-8 pattern, sharp s is folded to "\x{17F}\x{17F}, avoiding the
 *      problem; but in a non-UTF8 pattern, folding it to that above-Latin1
 *      string would require the pattern to be forced into UTF-8, the overhead
 *      of which we want to avoid.  Similarly the unfolded multi-char folds in
 *      EXACTFL nodes will match iff the locale at the time of match is a UTF-8
 *      locale.)
 *
 *      Similarly, the code that generates tries doesn't currently handle
 *      not-already-folded multi-char folds, and it looks like a pain to change
 *      that.  Therefore, trie generation of EXACTFAA nodes with the sharp s
 *      doesn't work.  Instead, such an EXACTFAA is turned into a new regnode,
 *      EXACTFAA_NO_TRIE, which the trie code knows not to handle.  Most people
 *      using /iaa matching will be doing so almost entirely with ASCII
 *      strings, so this should rarely be encountered in practice */

U32
Perl_join_exact(pTHX_ RExC_state_t *pRExC_state, regnode *scan,
                   UV *min_subtract, bool *unfolded_multi_char,
                   U32 flags, regnode *val, U32 depth)
{
    /* Merge several consecutive EXACTish nodes into one. */

    regnode *n = regnext(scan);
    U32 stringok = 1;
    regnode *next = REGNODE_AFTER_varies(scan);
    U32 merged = 0;
    U32 stopnow = 0;
#ifdef DEBUGGING
    regnode *stop = scan;
    DECLARE_AND_GET_RE_DEBUG_FLAGS;
#else
    PERL_UNUSED_ARG(depth);
#endif

    PERL_ARGS_ASSERT_JOIN_EXACT;
#ifndef EXPERIMENTAL_INPLACESCAN
    PERL_UNUSED_ARG(flags);
    PERL_UNUSED_ARG(val);
#endif
    DEBUG_PEEP("join", scan, depth, 0);

    assert(REGNODE_TYPE(OP(scan)) == EXACT);

    /* Look through the subsequent nodes in the chain.  Skip NOTHING, merge
     * EXACT ones that are mergeable to the current one. */
    while (    n
           && (    REGNODE_TYPE(OP(n)) == NOTHING
               || (stringok && REGNODE_TYPE(OP(n)) == EXACT))
           && NEXT_OFF(n)
           && NEXT_OFF(scan) + NEXT_OFF(n) < I16_MAX)
    {

        if (OP(n) == TAIL || n > next)
            stringok = 0;
        if (REGNODE_TYPE(OP(n)) == NOTHING) {
            DEBUG_PEEP("skip:", n, depth, 0);
            NEXT_OFF(scan) += NEXT_OFF(n);
            next = n + NODE_STEP_REGNODE;
#ifdef DEBUGGING
            if (stringok)
                stop = n;
#endif
            n = regnext(n);
        }
        else if (stringok) {
            const unsigned int oldl = STR_LEN(scan);
            regnode * const nnext = regnext(n);

            /* XXX I (khw) kind of doubt that this works on platforms (should
             * Perl ever run on one) where U8_MAX is above 255 because of lots
             * of other assumptions */
            /* Don't join if the sum can't fit into a single node */
            if (oldl + STR_LEN(n) > U8_MAX)
                break;

            /* Joining something that requires UTF-8 with something that
             * doesn't, means the result requires UTF-8. */
            if (OP(scan) == EXACT && (OP(n) == EXACT_REQ8)) {
                OP(scan) = EXACT_REQ8;
            }
            else if (OP(scan) == EXACT_REQ8 && (OP(n) == EXACT)) {
                ;   /* join is compatible, no need to change OP */
            }
            else if ((OP(scan) == EXACTFU) && (OP(n) == EXACTFU_REQ8)) {
                OP(scan) = EXACTFU_REQ8;
            }
            else if ((OP(scan) == EXACTFU_REQ8) && (OP(n) == EXACTFU)) {
                ;   /* join is compatible, no need to change OP */
            }
            else if (OP(scan) == EXACTFU && OP(n) == EXACTFU) {
                ;   /* join is compatible, no need to change OP */
            }
            else if (OP(scan) == EXACTFU && OP(n) == EXACTFU_S_EDGE) {

                 /* Under /di, temporary EXACTFU_S_EDGE nodes are generated,
                  * which can join with EXACTFU ones.  We check for this case
                  * here.  These need to be resolved to either EXACTFU or
                  * EXACTF at joining time.  They have nothing in them that
                  * would forbid them from being the more desirable EXACTFU
                  * nodes except that they begin and/or end with a single [Ss].
                  * The reason this is problematic is because they could be
                  * joined in this loop with an adjacent node that ends and/or
                  * begins with [Ss] which would then form the sequence 'ss',
                  * which matches differently under /di than /ui, in which case
                  * EXACTFU can't be used.  If the 'ss' sequence doesn't get
                  * formed, the nodes get absorbed into any adjacent EXACTFU
                  * node.  And if the only adjacent node is EXACTF, they get
                  * absorbed into that, under the theory that a longer node is
                  * better than two shorter ones, even if one is EXACTFU.  Note
                  * that EXACTFU_REQ8 is generated only for UTF-8 patterns,
                  * and the EXACTFU_S_EDGE ones only for non-UTF-8.  */

                if (STRING(n)[STR_LEN(n)-1] == 's') {

                    /* Here the joined node would end with 's'.  If the node
                     * following the combination is an EXACTF one, it's better to
                     * join this trailing edge 's' node with that one, leaving the
                     * current one in 'scan' be the more desirable EXACTFU */
                    if (OP(nnext) == EXACTF) {
                        break;
                    }

                    OP(scan) = EXACTFU_S_EDGE;

                }   /* Otherwise, the beginning 's' of the 2nd node just
                       becomes an interior 's' in 'scan' */
            }
            else if (OP(scan) == EXACTF && OP(n) == EXACTF) {
                ;   /* join is compatible, no need to change OP */
            }
            else if (OP(scan) == EXACTF && OP(n) == EXACTFU_S_EDGE) {

                /* EXACTF nodes are compatible for joining with EXACTFU_S_EDGE
                 * nodes.  But the latter nodes can be also joined with EXACTFU
                 * ones, and that is a better outcome, so if the node following
                 * 'n' is EXACTFU, quit now so that those two can be joined
                 * later */
                if (OP(nnext) == EXACTFU) {
                    break;
                }

                /* The join is compatible, and the combined node will be
                 * EXACTF.  (These don't care if they begin or end with 's' */
            }
            else if (OP(scan) == EXACTFU_S_EDGE && OP(n) == EXACTFU_S_EDGE) {
                if (   STRING(scan)[STR_LEN(scan)-1] == 's'
                    && STRING(n)[0] == 's')
                {
                    /* When combined, we have the sequence 'ss', which means we
                     * have to remain /di */
                    OP(scan) = EXACTF;
                }
            }
            else if (OP(scan) == EXACTFU_S_EDGE && OP(n) == EXACTFU) {
                if (STRING(n)[0] == 's') {
                    ;   /* Here the join is compatible and the combined node
                           starts with 's', no need to change OP */
                }
                else {  /* Now the trailing 's' is in the interior */
                    OP(scan) = EXACTFU;
                }
            }
            else if (OP(scan) == EXACTFU_S_EDGE && OP(n) == EXACTF) {

                /* The join is compatible, and the combined node will be
                 * EXACTF.  (These don't care if they begin or end with 's' */
                OP(scan) = EXACTF;
            }
            else if (OP(scan) != OP(n)) {

                /* The only other compatible joinings are the same node type */
                break;
            }

            DEBUG_PEEP("merg", n, depth, 0);
            merged++;

            next = REGNODE_AFTER_varies(n);
            NEXT_OFF(scan) += NEXT_OFF(n);
            assert( ( STR_LEN(scan) + STR_LEN(n) ) < 256 );
            setSTR_LEN(scan, (U8)(STR_LEN(scan) + STR_LEN(n)));
            /* Now we can overwrite *n : */
            Move(STRING(n), STRING(scan) + oldl, STR_LEN(n), char);
#ifdef DEBUGGING
            stop = next - 1;
#endif
            n = nnext;
            if (stopnow) break;
        }

#ifdef EXPERIMENTAL_INPLACESCAN
        if (flags && !NEXT_OFF(n)) {
            DEBUG_PEEP("atch", val, depth, 0);
            if (REGNODE_OFF_BY_ARG(OP(n))) {
                ARG1u_SET(n, val - n);
            }
            else {
                NEXT_OFF(n) = val - n;
            }
            stopnow = 1;
        }
#endif
    }

    /* This temporary node can now be turned into EXACTFU, and must, as
     * regexec.c doesn't handle it */
    if (OP(scan) == EXACTFU_S_EDGE) {
        OP(scan) = EXACTFU;
    }

    *min_subtract = 0;
    *unfolded_multi_char = FALSE;

    /* Here, all the adjacent mergeable EXACTish nodes have been merged.  We
     * can now analyze for sequences of problematic code points.  (Prior to
     * this final joining, sequences could have been split over boundaries, and
     * hence missed).  The sequences only happen in folding, hence for any
     * non-EXACT EXACTish node */
    if (OP(scan) != EXACT && OP(scan) != EXACT_REQ8 && OP(scan) != EXACTL) {
        U8* s0 = (U8*) STRING(scan);
        U8* s = s0;
        U8* s_end = s0 + STR_LEN(scan);

        int total_count_delta = 0;  /* Total delta number of characters that
                                       multi-char folds expand to */

        /* One pass is made over the node's string looking for all the
         * possibilities.  To avoid some tests in the loop, there are two main
         * cases, for UTF-8 patterns (which can't have EXACTF nodes) and
         * non-UTF-8 */
        if (UTF) {
            U8* folded = NULL;

            if (OP(scan) == EXACTFL) {
                U8 *d;

                /* An EXACTFL node would already have been changed to another
                 * node type unless there is at least one character in it that
                 * is problematic; likely a character whose fold definition
                 * won't be known until runtime, and so has yet to be folded.
                 * For all but the UTF-8 locale, folds are 1-1 in length, but
                 * to handle the UTF-8 case, we need to create a temporary
                 * folded copy using UTF-8 locale rules in order to analyze it.
                 * This is because our macros that look to see if a sequence is
                 * a multi-char fold assume everything is folded (otherwise the
                 * tests in those macros would be too complicated and slow).
                 * Note that here, the non-problematic folds will have already
                 * been done, so we can just copy such characters.  We actually
                 * don't completely fold the EXACTFL string.  We skip the
                 * unfolded multi-char folds, as that would just create work
                 * below to figure out the size they already are */

                Newx(folded, UTF8_MAX_FOLD_CHAR_EXPAND * STR_LEN(scan) + 1, U8);
                d = folded;
                while (s < s_end) {
                    STRLEN s_len = UTF8SKIP(s);
                    if (! is_PROBLEMATIC_LOCALE_FOLD_utf8(s)) {
                        Copy(s, d, s_len, U8);
                        d += s_len;
                    }
                    else if (is_FOLDS_TO_MULTI_utf8(s)) {
                        *unfolded_multi_char = TRUE;
                        Copy(s, d, s_len, U8);
                        d += s_len;
                    }
                    else if (isASCII(*s)) {
                        *(d++) = toFOLD(*s);
                    }
                    else {
                        STRLEN len;
                        _toFOLD_utf8_flags(s, s_end, d, &len, FOLD_FLAGS_FULL);
                        d += len;
                    }
                    s += s_len;
                }

                /* Point the remainder of the routine to look at our temporary
                 * folded copy */
                s = folded;
                s_end = d;
            } /* End of creating folded copy of EXACTFL string */

            /* Examine the string for a multi-character fold sequence.  UTF-8
             * patterns have all characters pre-folded by the time this code is
             * executed */
            while (s < s_end - 1) /* Can stop 1 before the end, as minimum
                                     length sequence we are looking for is 2 */
            {
                int count = 0;  /* How many characters in a multi-char fold */
                int len = is_MULTI_CHAR_FOLD_utf8_safe(s, s_end);
                if (! len) {    /* Not a multi-char fold: get next char */
                    s += UTF8SKIP(s);
                    continue;
                }

                { /* Here is a generic multi-char fold. */
                    U8* multi_end  = s + len;

                    /* Count how many characters are in it.  In the case of
                     * /aa, no folds which contain ASCII code points are
                     * allowed, so check for those, and skip if found. */
                    if (OP(scan) != EXACTFAA && OP(scan) != EXACTFAA_NO_TRIE) {
                        count = utf8_length(s, multi_end);
                        s = multi_end;
                    }
                    else {
                        while (s < multi_end) {
                            if (isASCII(*s)) {
                                s++;
                                goto next_iteration;
                            }
                            else {
                                s += UTF8SKIP(s);
                            }
                            count++;
                        }
                    }
                }

                /* The delta is how long the sequence is minus 1 (1 is how long
                 * the character that folds to the sequence is) */
                total_count_delta += count - 1;
              next_iteration: ;
            }

            /* We created a temporary folded copy of the string in EXACTFL
             * nodes.  Therefore we need to be sure it doesn't go below zero,
             * as the real string could be shorter */
            if (OP(scan) == EXACTFL) {
                int total_chars = utf8_length((U8*) STRING(scan),
                                           (U8*) STRING(scan) + STR_LEN(scan));
                if (total_count_delta > total_chars) {
                    total_count_delta = total_chars;
                }
            }

            *min_subtract += total_count_delta;
            Safefree(folded);
        }
        else if (OP(scan) == EXACTFAA) {

            /* Non-UTF-8 pattern, EXACTFAA node.  There can't be a multi-char
             * fold to the ASCII range (and there are no existing ones in the
             * upper latin1 range).  But, as outlined in the comments preceding
             * this function, we need to flag any occurrences of the sharp s.
             * This character forbids trie formation (because of added
             * complexity) */
#if    UNICODE_MAJOR_VERSION > 3 /* no multifolds in early Unicode */   \
   || (UNICODE_MAJOR_VERSION == 3 && (   UNICODE_DOT_VERSION > 0)       \
                                      || UNICODE_DOT_DOT_VERSION > 0)
            while (s < s_end) {
                if (*s == LATIN_SMALL_LETTER_SHARP_S) {
                    OP(scan) = EXACTFAA_NO_TRIE;
                    *unfolded_multi_char = TRUE;
                    break;
                }
                s++;
            }
        }
        else if (OP(scan) != EXACTFAA_NO_TRIE) {

            /* Non-UTF-8 pattern, not EXACTFAA node.  Look for the multi-char
             * folds that are all Latin1.  As explained in the comments
             * preceding this function, we look also for the sharp s in EXACTF
             * and EXACTFL nodes; it can be in the final position.  Otherwise
             * we can stop looking 1 byte earlier because have to find at least
             * two characters for a multi-fold */
            const U8* upper = (OP(scan) == EXACTF || OP(scan) == EXACTFL)
                              ? s_end
                              : s_end -1;

            while (s < upper) {
                int len = is_MULTI_CHAR_FOLD_latin1_safe(s, s_end);
                if (! len) {    /* Not a multi-char fold. */
                    if (*s == LATIN_SMALL_LETTER_SHARP_S
                        && (OP(scan) == EXACTF || OP(scan) == EXACTFL))
                    {
                        *unfolded_multi_char = TRUE;
                    }
                    s++;
                    continue;
                }

                if (len == 2
                    && isALPHA_FOLD_EQ(*s, 's')
                    && isALPHA_FOLD_EQ(*(s+1), 's'))
                {

                    /* EXACTF nodes need to know that the minimum length
                     * changed so that a sharp s in the string can match this
                     * ss in the pattern, but they remain EXACTF nodes, as they
                     * won't match this unless the target string is in UTF-8,
                     * which we don't know until runtime.  EXACTFL nodes can't
                     * transform into EXACTFU nodes */
                    if (OP(scan) != EXACTF && OP(scan) != EXACTFL) {
                        OP(scan) = EXACTFUP;
                    }
                }

                *min_subtract += len - 1;
                s += len;
            }
#endif
        }
    }

#ifdef DEBUGGING
    /* Allow dumping but overwriting the collection of skipped
     * ops and/or strings with fake optimized ops */
    n = REGNODE_AFTER_varies(scan);
    while (n <= stop) {
        OP(n) = OPTIMIZED;
        FLAGS(n) = 0;
        NEXT_OFF(n) = 0;
        n++;
    }
#endif
    DEBUG_OPTIMISE_r(if (merged){DEBUG_PEEP("finl", scan, depth, 0);});
    return stopnow;
}

/* REx optimizer.  Converts nodes into quicker variants "in place".
   Finds fixed substrings.  */


/* Stops at toplevel WHILEM as well as at "last". At end *scanp is set
   to the position after last scanned or to NULL. */

/* the return from this sub is the minimum length that could possibly match */
SSize_t
Perl_study_chunk(pTHX_
    RExC_state_t *pRExC_state,
    regnode **scanp,        /* Start here (read-write). */
    SSize_t *minlenp,       /* used for the minlen of substrings? */
    SSize_t *deltap,        /* Write maxlen-minlen here. */
    regnode *last,          /* Stop before this one. */
    scan_data_t *data,      /* string data about the pattern */
    I32 stopparen,          /* treat CLOSE-N as END, see GOSUB */
    U32 recursed_depth,     /* how deep have we recursed via GOSUB */
    regnode_ssc *and_withp, /* Valid if flags & SCF_DO_STCLASS_OR */
    U32 flags,              /* flags controlling this call, see SCF_ flags */
    U32 depth,              /* how deep have we recursed period */
    bool was_mutate_ok      /* TRUE if in-place optimizations are allowed.
                               FALSE only if the caller (recursively) was
                               prohibited from modifying the regops, because
                               a higher caller is holding a ptr to them. */
)
{
    /* vars about the regnodes we are working with */
    regnode *scan = *scanp; /* the current opcode we are inspecting */
    regnode *next = NULL;   /* the next opcode beyond scan, tmp var */
    regnode *first_non_open = scan; /* FIXME: should this init to NULL?
                                       the first non open regop, if the init
                                       val IS an OPEN then we will skip past
                                       it just after the var decls section */
    I32 code = 0;           /* temp var used to hold the optype of a regop */

    /* vars about the min and max length of the pattern */
    SSize_t min = 0;    /* min length of this part of the pattern */
    SSize_t stopmin = OPTIMIZE_INFTY; /* min length accounting for ACCEPT
                                         this is adjusted down if we find
                                         an ACCEPT */
    SSize_t delta = 0;  /* difference between min and max length
                           (not accounting for stopmin) */

    /* vars about capture buffers in the pattern */
    I32 pars = 0;       /* count of OPEN opcodes */
    I32 is_par = OP(scan) == OPEN ? PARNO(scan) : 0; /* is this op an OPEN? */

    /* vars about whether this pattern contains something that can match
     * infinitely long strings, eg, X* or X+ */
    int is_inf = (flags & SCF_DO_SUBSTR) && (data->flags & SF_IS_INF);
    int is_inf_internal = 0;            /* The studied chunk is infinite */

    /* scan_data_t (struct) is used to hold information about the substrings
     * and start class we have extracted from the string */
    scan_data_t data_fake; /* temp var used for recursing in some cases */

    SV *re_trie_maxbuff = NULL; /* temp var used to hold whether we can do
                                   trie optimizations */

    scan_frame *frame = NULL;  /* used as part of fake recursion */

    DECLARE_AND_GET_RE_DEBUG_FLAGS;

    PERL_ARGS_ASSERT_STUDY_CHUNK;
    RExC_study_started= 1;

    Zero(&data_fake, 1, scan_data_t);

    if ( depth == 0 ) {
        while (first_non_open && OP(first_non_open) == OPEN)
            first_non_open=regnext(first_non_open);
    }

  fake_study_recurse:
    DEBUG_r(
        RExC_study_chunk_recursed_count++;
    );
    DEBUG_OPTIMISE_MORE_r(
    {
        Perl_re_indentf( aTHX_  "study_chunk stopparen=%ld recursed_count=%lu depth=%lu recursed_depth=%lu scan=%p last=%p",
            depth, (long)stopparen,
            (unsigned long)RExC_study_chunk_recursed_count,
            (unsigned long)depth, (unsigned long)recursed_depth,
            scan,
            last);
        if (recursed_depth) {
            U32 i;
            U32 j;
            for ( j = 0 ; j < recursed_depth ; j++ ) {
                for ( i = 0 ; i < (U32)RExC_total_parens ; i++ ) {
                    if (PAREN_TEST(j, i) && (!j || !PAREN_TEST(j - 1, i))) {
                        Perl_re_printf( aTHX_ " %d",(int)i);
                        break;
                    }
                }
                if ( j + 1 < recursed_depth ) {
                    Perl_re_printf( aTHX_  ",");
                }
            }
        }
        Perl_re_printf( aTHX_ "\n");
    }
    );
    while ( scan && OP(scan) != END && scan < last ){
        UV min_subtract = 0;    /* How mmany chars to subtract from the minimum
                                   node length to get a real minimum (because
                                   the folded version may be shorter) */
        bool unfolded_multi_char = FALSE;
        /* avoid mutating ops if we are anywhere within the recursed or
         * enframed handling for a GOSUB: the outermost level will handle it.
         */
        bool mutate_ok = was_mutate_ok && !(frame && frame->in_gosub);
        /* Peephole optimizer: */
        DEBUG_STUDYDATA("Peep", data, depth, is_inf, min, stopmin, delta);
        DEBUG_PEEP("Peep", scan, depth, flags);


        /* The reason we do this here is that we need to deal with things like
         * /(?:f)(?:o)(?:o)/ which cant be dealt with by the normal EXACT
         * parsing code, as each (?:..) is handled by a different invocation of
         * reg() -- Yves
         */
        if (REGNODE_TYPE(OP(scan)) == EXACT
            && OP(scan) != LEXACT
            && OP(scan) != LEXACT_REQ8
            && mutate_ok
        ) {
            join_exact(pRExC_state, scan, &min_subtract, &unfolded_multi_char,
                    0, NULL, depth + 1);
        }

        /* Follow the next-chain of the current node and optimize
           away all the NOTHINGs from it.
         */
        rck_elide_nothing(scan);

        /* The principal pseudo-switch.  Cannot be a switch, since we look into
         * several different things.  */
        if ( OP(scan) == DEFINEP ) {
            SSize_t minlen = 0;
            SSize_t deltanext = 0;
            SSize_t fake_last_close = 0;
            regnode *fake_last_close_op = NULL;
            U32 f = SCF_IN_DEFINE | (flags & SCF_TRIE_DOING_RESTUDY);

            StructCopy(&zero_scan_data, &data_fake, scan_data_t);
            scan = regnext(scan);
            assert( OP(scan) == IFTHEN );
            DEBUG_PEEP("expect IFTHEN", scan, depth, flags);

            data_fake.last_closep= &fake_last_close;
            data_fake.last_close_opp= &fake_last_close_op;
            minlen = *minlenp;
            next = regnext(scan);
            scan = REGNODE_AFTER_type(scan,tregnode_IFTHEN);
            DEBUG_PEEP("scan", scan, depth, flags);
            DEBUG_PEEP("next", next, depth, flags);

            /* we suppose the run is continuous, last=next...
             * NOTE we dont use the return here! */
            /* DEFINEP study_chunk() recursion */
            (void)study_chunk(pRExC_state, &scan, &minlen,
                              &deltanext, next, &data_fake, stopparen,
                              recursed_depth, NULL, f, depth+1, mutate_ok);

            scan = next;
        } else
        if (
            OP(scan) == BRANCH  ||
            OP(scan) == BRANCHJ ||
            OP(scan) == IFTHEN
        ) {
            next = regnext(scan);
            code = OP(scan);

            /* The op(next)==code check below is to see if we
             * have "BRANCH-BRANCH", "BRANCHJ-BRANCHJ", "IFTHEN-IFTHEN"
             * IFTHEN is special as it might not appear in pairs.
             * Not sure whether BRANCH-BRANCHJ is possible, regardless
             * we dont handle it cleanly. */
            if (OP(next) == code || code == IFTHEN) {
                /* NOTE - There is similar code to this block below for
                 * handling TRIE nodes on a re-study.  If you change stuff here
                 * check there too. */
                SSize_t max1 = 0, min1 = OPTIMIZE_INFTY, num = 0;
                regnode_ssc accum;
                regnode * const startbranch=scan;

                if (flags & SCF_DO_SUBSTR) {
                    /* Cannot merge strings after this. */
                    scan_commit(pRExC_state, data, minlenp, is_inf);
                }

                if (flags & SCF_DO_STCLASS)
                    ssc_init_zero(pRExC_state, &accum);

                while (OP(scan) == code) {
                    SSize_t deltanext, minnext, fake_last_close = 0;
                    regnode *fake_last_close_op = NULL;
                    U32 f = (flags & SCF_TRIE_DOING_RESTUDY);
                    regnode_ssc this_class;

                    DEBUG_PEEP("Branch", scan, depth, flags);

                    num++;
                    StructCopy(&zero_scan_data, &data_fake, scan_data_t);
                    if (data) {
                        data_fake.whilem_c = data->whilem_c;
                        data_fake.last_closep = data->last_closep;
                        data_fake.last_close_opp = data->last_close_opp;
                    }
                    else {
                        data_fake.last_closep = &fake_last_close;
                        data_fake.last_close_opp = &fake_last_close_op;
                    }

                    data_fake.pos_delta = delta;
                    next = regnext(scan);

                    scan = REGNODE_AFTER_opcode(scan, code);

                    if (flags & SCF_DO_STCLASS) {
                        ssc_init(pRExC_state, &this_class);
                        data_fake.start_class = &this_class;
                        f |= SCF_DO_STCLASS_AND;
                    }
                    if (flags & SCF_WHILEM_VISITED_POS)
                        f |= SCF_WHILEM_VISITED_POS;

                    /* we suppose the run is continuous, last=next...*/
                    /* recurse study_chunk() for each BRANCH in an alternation */
                    minnext = study_chunk(pRExC_state, &scan, minlenp,
                                      &deltanext, next, &data_fake, stopparen,
                                      recursed_depth, NULL, f, depth+1,
                                      mutate_ok);

                    if (min1 > minnext)
                        min1 = minnext;
                    if (deltanext == OPTIMIZE_INFTY) {
                        is_inf = is_inf_internal = 1;
                        max1 = OPTIMIZE_INFTY;
                    } else if (max1 < minnext + deltanext)
                        max1 = minnext + deltanext;
                    scan = next;
                    if (data_fake.flags & (SF_HAS_PAR|SF_IN_PAR))
                        pars++;
                    if (data_fake.flags & SCF_SEEN_ACCEPT) {
                        if ( stopmin > minnext)
                            stopmin = min + min1;
                        flags &= ~SCF_DO_SUBSTR;
                        if (data)
                            data->flags |= SCF_SEEN_ACCEPT;
                    }
                    if (data) {
                        if (data_fake.flags & SF_HAS_EVAL)
                            data->flags |= SF_HAS_EVAL;
                        data->whilem_c = data_fake.whilem_c;
                    }
                    if (flags & SCF_DO_STCLASS)
                        ssc_or(pRExC_state, &accum, (regnode_charclass*)&this_class);
                    DEBUG_STUDYDATA("end BRANCH", data, depth, is_inf, min, stopmin, delta);
                }
                if (code == IFTHEN && num < 2) /* Empty ELSE branch */
                    min1 = 0;
                if (flags & SCF_DO_SUBSTR) {
                    data->pos_min += min1;
                    if (data->pos_delta >= OPTIMIZE_INFTY - (max1 - min1))
                        data->pos_delta = OPTIMIZE_INFTY;
                    else
                        data->pos_delta += max1 - min1;
                    if (max1 != min1 || is_inf)
                        data->cur_is_floating = 1;
                }
                min += min1;
                if (delta == OPTIMIZE_INFTY
                 || OPTIMIZE_INFTY - delta - (max1 - min1) < 0)
                    delta = OPTIMIZE_INFTY;
                else
                    delta += max1 - min1;
                if (flags & SCF_DO_STCLASS_OR) {
                    ssc_or(pRExC_state, data->start_class, (regnode_charclass*) &accum);
                    if (min1) {
                        ssc_and(pRExC_state, data->start_class, (regnode_charclass *) and_withp);
                        flags &= ~SCF_DO_STCLASS;
                    }
                }
                else if (flags & SCF_DO_STCLASS_AND) {
                    if (min1) {
                        ssc_and(pRExC_state, data->start_class, (regnode_charclass *) &accum);
                        flags &= ~SCF_DO_STCLASS;
                    }
                    else {
                        /* Switch to OR mode: cache the old value of
                         * data->start_class */
                        INIT_AND_WITHP;
                        StructCopy(data->start_class, and_withp, regnode_ssc);
                        flags &= ~SCF_DO_STCLASS_AND;
                        StructCopy(&accum, data->start_class, regnode_ssc);
                        flags |= SCF_DO_STCLASS_OR;
                    }
                }
                DEBUG_STUDYDATA("pre TRIE", data, depth, is_inf, min, stopmin, delta);

                if (PERL_ENABLE_TRIE_OPTIMISATION
                    && OP(startbranch) == BRANCH
                    && mutate_ok
                ) {
                /* demq.

                   Assuming this was/is a branch we are dealing with: 'scan'
                   now points at the item that follows the branch sequence,
                   whatever it is. We now start at the beginning of the
                   sequence and look for subsequences of

                   BRANCH->EXACT=>x1
                   BRANCH->EXACT=>x2
                   tail

                   which would be constructed from a pattern like
                   /A|LIST|OF|WORDS/

                   If we can find such a subsequence we need to turn the first
                   element into a trie and then add the subsequent branch exact
                   strings to the trie.

                   We have two cases

                     1. patterns where the whole set of branches can be
                        converted.

                     2. patterns where only a subset can be converted.

                   In case 1 we can replace the whole set with a single regop
                   for the trie. In case 2 we need to keep the start and end
                   branches so

                     'BRANCH EXACT; BRANCH EXACT; BRANCH X'
                     becomes BRANCH TRIE; BRANCH X;

                  There is an additional case, that being where there is a
                  common prefix, which gets split out into an EXACT like node
                  preceding the TRIE node.

                  If X(1..n)==tail then we can do a simple trie, if not we make
                  a "jump" trie, such that when we match the appropriate word
                  we "jump" to the appropriate tail node. Essentially we turn
                  a nested if into a case structure of sorts.

                */

                    int made=0;
                    if (!re_trie_maxbuff) {
                        re_trie_maxbuff = get_sv(RE_TRIE_MAXBUF_NAME, 1);
                        if (!SvIOK(re_trie_maxbuff))
                            sv_setiv(re_trie_maxbuff, RE_TRIE_MAXBUF_INIT);
                    }
                    if ( SvIV(re_trie_maxbuff)>=0  ) {
                        regnode *cur;
                        regnode *first = (regnode *)NULL;
                        regnode *prev = (regnode *)NULL;
                        regnode *tail = scan;
                        U8 trietype = 0;
                        U32 count=0;

                        /* var tail is used because there may be a TAIL
                           regop in the way. Ie, the exacts will point to the
                           thing following the TAIL, but the last branch will
                           point at the TAIL. So we advance tail. If we
                           have nested (?:) we may have to move through several
                           tails.
                         */

                        while ( OP( tail ) == TAIL ) {
                            /* this is the TAIL generated by (?:) */
                            tail = regnext( tail );
                        }


                        DEBUG_TRIE_COMPILE_r({
                            regprop(RExC_rx, RExC_mysv, tail, NULL, pRExC_state);
                            Perl_re_indentf( aTHX_  "%s %" UVuf ":%s\n",
                              depth+1,
                              "Looking for TRIE'able sequences. Tail node is ",
                              (UV) REGNODE_OFFSET(tail),
                              SvPV_nolen_const( RExC_mysv )
                            );
                        });

                        /*

                            Step through the branches
                                cur represents each branch,
                                noper is the first thing to be matched as part
                                      of that branch
                                noper_next is the regnext() of that node.

                            We normally handle a case like this
                            /FOO[xyz]|BAR[pqr]/ via a "jump trie" but we also
                            support building with NOJUMPTRIE, which restricts
                            the trie logic to structures like /FOO|BAR/.

                            If noper is a trieable nodetype then the branch is
                            a possible optimization target. If we are building
                            under NOJUMPTRIE then we require that noper_next is
                            the same as scan (our current position in the regex
                            program).

                            Once we have two or more consecutive such branches
                            we can create a trie of the EXACT's contents and
                            stitch it in place into the program.

                            If the sequence represents all of the branches in
                            the alternation we replace the entire thing with a
                            single TRIE node.

                            Otherwise when it is a subsequence we need to
                            stitch it in place and replace only the relevant
                            branches. This means the first branch has to remain
                            as it is used by the alternation logic, and its
                            next pointer, and needs to be repointed at the item
                            on the branch chain following the last branch we
                            have optimized away.

                            This could be either a BRANCH, in which case the
                            subsequence is internal, or it could be the item
                            following the branch sequence in which case the
                            subsequence is at the end (which does not
                            necessarily mean the first node is the start of the
                            alternation).

                            TRIE_TYPE(X) is a define which maps the optype to a
                            trietype.

                                optype          |  trietype
                                ----------------+-----------
                                NOTHING         | NOTHING
                                EXACT           | EXACT
                                EXACT_REQ8      | EXACT
                                EXACTFU         | EXACTFU
                                EXACTFU_REQ8    | EXACTFU
                                EXACTFUP        | EXACTFU
                                EXACTFAA        | EXACTFAA
                                EXACTL          | EXACTL
                                EXACTFLU8       | EXACTFLU8


                        */
#define TRIE_TYPE(X) ( ( NOTHING == (X) )                                   \
                       ? NOTHING                                            \
                       : ( EXACT == (X) || EXACT_REQ8 == (X) )             \
                         ? EXACT                                            \
                         : (     EXACTFU == (X)                             \
                              || EXACTFU_REQ8 == (X)                       \
                              || EXACTFUP == (X) )                          \
                           ? EXACTFU                                        \
                           : ( EXACTFAA == (X) )                            \
                             ? EXACTFAA                                     \
                             : ( EXACTL == (X) )                            \
                               ? EXACTL                                     \
                               : ( EXACTFLU8 == (X) )                       \
                                 ? EXACTFLU8                                \
                                 : 0 )

                        /* dont use tail as the end marker for this traverse */
                        for ( cur = startbranch ; cur != scan ; cur = regnext( cur ) ) {
                            regnode * const noper = REGNODE_AFTER( cur );
                            U8 noper_type = OP( noper );
                            U8 noper_trietype = TRIE_TYPE( noper_type );
#if defined(DEBUGGING) || defined(NOJUMPTRIE)
                            regnode * const noper_next = regnext( noper );
                            U8 noper_next_type = (noper_next && noper_next < tail) ? OP(noper_next) : 0;
                            U8 noper_next_trietype = (noper_next && noper_next < tail) ? TRIE_TYPE( noper_next_type ) :0;
#endif

                            DEBUG_TRIE_COMPILE_r({
                                regprop(RExC_rx, RExC_mysv, cur, NULL, pRExC_state);
                                Perl_re_indentf( aTHX_  "- %d:%s (%d)",
                                   depth+1,
                                   REG_NODE_NUM(cur), SvPV_nolen_const( RExC_mysv ), REG_NODE_NUM(cur) );

                                regprop(RExC_rx, RExC_mysv, noper, NULL, pRExC_state);
                                Perl_re_printf( aTHX_  " -> %d:%s",
                                    REG_NODE_NUM(noper), SvPV_nolen_const(RExC_mysv));

                                if ( noper_next ) {
                                  regprop(RExC_rx, RExC_mysv, noper_next, NULL, pRExC_state);
                                  Perl_re_printf( aTHX_ "\t=> %d:%s\t",
                                    REG_NODE_NUM(noper_next), SvPV_nolen_const(RExC_mysv));
                                }
                                Perl_re_printf( aTHX_  "(First==%d,Last==%d,Cur==%d,tt==%s,ntt==%s,nntt==%s)\n",
                                   REG_NODE_NUM(first), REG_NODE_NUM(prev), REG_NODE_NUM(cur),
                                   REGNODE_NAME(trietype), REGNODE_NAME(noper_trietype), REGNODE_NAME(noper_next_trietype)
                                );
                            });

                            /* Is noper a trieable nodetype that can be merged
                             * with the current trie (if there is one)? */
                            if ( noper_trietype
                                  &&
                                  (
                                        ( noper_trietype == NOTHING )
                                        || ( trietype == NOTHING )
                                        || ( trietype == noper_trietype )
                                  )
#ifdef NOJUMPTRIE
                                  && noper_next >= tail
#endif
                                  && count < U16_MAX)
                            {
                                /* Handle mergable triable node Either we are
                                 * the first node in a new trieable sequence,
                                 * in which case we do some bookkeeping,
                                 * otherwise we update the end pointer. */
                                if ( !first ) {
                                    first = cur;
                                    if ( noper_trietype == NOTHING ) {
#if !defined(DEBUGGING) && !defined(NOJUMPTRIE)
                                        regnode * const noper_next = regnext( noper );
                                        U8 noper_next_type = (noper_next && noper_next < tail) ? OP(noper_next) : 0;
                                        U8 noper_next_trietype = noper_next_type ? TRIE_TYPE( noper_next_type ) :0;
#endif

                                        if ( noper_next_trietype ) {
                                            trietype = noper_next_trietype;
                                        } else if (noper_next_type)  {
                                            /* a NOTHING regop is 1 regop wide.
                                             * We need at least two for a trie
                                             * so we can't merge this in */
                                            first = NULL;
                                        }
                                    } else {
                                        trietype = noper_trietype;
                                    }
                                } else {
                                    if ( trietype == NOTHING )
                                        trietype = noper_trietype;
                                    prev = cur;
                                }
                                if (first)
                                    count++;
                            } /* end handle mergable triable node */
                            else {
                                /* handle unmergable node -
                                 * noper may either be a triable node which can
                                 * not be tried together with the current trie,
                                 * or a non triable node */
                                if ( prev ) {
                                    /* If last is set and trietype is not
                                     * NOTHING then we have found at least two
                                     * triable branch sequences in a row of a
                                     * similar trietype so we can turn them
                                     * into a trie. If/when we allow NOTHING to
                                     * start a trie sequence this condition
                                     * will be required, and it isn't expensive
                                     * so we leave it in for now. */
                                    if ( trietype && trietype != NOTHING )
                                        make_trie( pRExC_state,
                                                startbranch, first, cur, tail,
                                                count, trietype, depth+1 );
                                    prev = NULL; /* note: we clear/update
                                                    first, trietype etc below,
                                                    so we dont do it here */
                                }
                                if ( noper_trietype
#ifdef NOJUMPTRIE
                                     && noper_next >= tail
#endif
                                ){
                                    /* noper is triable, so we can start a new
                                     * trie sequence */
                                    count = 1;
                                    first = cur;
                                    trietype = noper_trietype;
                                } else if (first) {
                                    /* if we already saw a first but the
                                     * current node is not triable then we have
                                     * to reset the first information. */
                                    count = 0;
                                    first = NULL;
                                    trietype = 0;
                                }
                            } /* end handle unmergable node */
                        } /* loop over branches */
                        DEBUG_TRIE_COMPILE_r({
                            regprop(RExC_rx, RExC_mysv, cur, NULL, pRExC_state);
                            Perl_re_indentf( aTHX_  "- %s (%d) <SCAN FINISHED> ",
                              depth+1, SvPV_nolen_const( RExC_mysv ), REG_NODE_NUM(cur));
                            Perl_re_printf( aTHX_  "(First==%d, Last==%d, Cur==%d, tt==%s)\n",
                               REG_NODE_NUM(first), REG_NODE_NUM(prev), REG_NODE_NUM(cur),
                               REGNODE_NAME(trietype)
                            );

                        });
                        if ( prev && trietype ) {
                            if ( trietype != NOTHING ) {
                                /* the last branch of the sequence was part of
                                 * a trie, so we have to construct it here
                                 * outside of the loop */
                                made= make_trie( pRExC_state, startbranch,
                                                 first, scan, tail, count,
                                                 trietype, depth+1 );
#ifdef TRIE_STUDY_OPT
                                if ( ((made == MADE_EXACT_TRIE &&
                                     startbranch == first)
                                     || ( first_non_open == first )) &&
                                     depth==0 ) {
                                    flags |= SCF_TRIE_RESTUDY;
                                    if ( startbranch == first
                                         && scan >= tail )
                                    {
                                        RExC_seen &=~REG_TOP_LEVEL_BRANCHES_SEEN;
                                    }
                                }
#endif
                            } else {
                                /* at this point we know whatever we have is a
                                 * NOTHING sequence/branch AND if 'startbranch'
                                 * is 'first' then we can turn the whole thing
                                 * into a NOTHING
                                 */
                                if ( startbranch == first ) {
                                    regnode *opt;
                                    /* the entire thing is a NOTHING sequence,
                                     * something like this: (?:|) So we can
                                     * turn it into a plain NOTHING op. */
                                    DEBUG_TRIE_COMPILE_r({
                                        regprop(RExC_rx, RExC_mysv, cur, NULL, pRExC_state);
                                        Perl_re_indentf( aTHX_  "- %s (%d) <NOTHING BRANCH SEQUENCE>\n",
                                          depth+1,
                                          SvPV_nolen_const( RExC_mysv ), REG_NODE_NUM(cur));

                                    });
                                    OP(startbranch)= NOTHING;
                                    NEXT_OFF(startbranch)= tail - startbranch;
                                    for ( opt= startbranch + 1; opt < tail ; opt++ )
                                        OP(opt)= OPTIMIZED;
                                }
                            }
                        } /* end if ( prev) */
                    } /* TRIE_MAXBUF is non zero */
                } /* do trie */
                DEBUG_STUDYDATA("after TRIE", data, depth, is_inf, min, stopmin, delta);
            }
            else
                scan = REGNODE_AFTER_opcode(scan,code);
            continue;
        } else if (OP(scan) == SUSPEND || OP(scan) == GOSUB) {
            I32 paren = 0;
            regnode *start = NULL;
            regnode *end = NULL;
            U32 my_recursed_depth= recursed_depth;

            if (OP(scan) != SUSPEND) { /* GOSUB */
                /* Do setup, note this code has side effects beyond
                 * the rest of this block. Specifically setting
                 * RExC_recurse[] must happen at least once during
                 * study_chunk(). */
                paren = ARG1u(scan);
                RExC_recurse[ARG2i(scan)] = scan;
                start = REGNODE_p(RExC_open_parens[paren]);
                end   = REGNODE_p(RExC_close_parens[paren]);

                /* NOTE we MUST always execute the above code, even
                 * if we do nothing with a GOSUB */
                if (
                    ( flags & SCF_IN_DEFINE )
                    ||
                    (
                        (is_inf_internal || is_inf || (data && data->flags & SF_IS_INF))
                        &&
                        ( (flags & (SCF_DO_STCLASS | SCF_DO_SUBSTR)) == 0 )
                    )
                ) {
                    /* no need to do anything here if we are in a define. */
                    /* or we are after some kind of infinite construct
                     * so we can skip recursing into this item.
                     * Since it is infinite we will not change the maxlen
                     * or delta, and if we miss something that might raise
                     * the minlen it will merely pessimise a little.
                     *
                     * Iow /(?(DEFINE)(?<foo>foo|food))a+(?&foo)/
                     * might result in a minlen of 1 and not of 4,
                     * but this doesn't make us mismatch, just try a bit
                     * harder than we should.
                     *
                     * However we must assume this GOSUB is infinite, to
                     * avoid wrongly applying other optimizations in the
                     * enclosing scope - see GH 18096, for example.
                     */
                    is_inf = is_inf_internal = 1;
                    scan= regnext(scan);
                    continue;
                }

                if (
                    !recursed_depth
                    || !PAREN_TEST(recursed_depth - 1, paren)
                ) {
                    /* it is quite possible that there are more efficient ways
                     * to do this. We maintain a bitmap per level of recursion
                     * of which patterns we have entered so we can detect if a
                     * pattern creates a possible infinite loop. When we
                     * recurse down a level we copy the previous levels bitmap
                     * down. When we are at recursion level 0 we zero the top
                     * level bitmap. It would be nice to implement a different
                     * more efficient way of doing this. In particular the top
                     * level bitmap may be unnecessary.
                     */
                    if (!recursed_depth) {
                        Zero(RExC_study_chunk_recursed, RExC_study_chunk_recursed_bytes, U8);
                    } else {
                        Copy(PAREN_OFFSET(recursed_depth - 1),
                             PAREN_OFFSET(recursed_depth),
                             RExC_study_chunk_recursed_bytes, U8);
                    }
                    /* we havent recursed into this paren yet, so recurse into it */
                    DEBUG_STUDYDATA("gosub-set", data, depth, is_inf, min, stopmin, delta);
                    PAREN_SET(recursed_depth, paren);
                    my_recursed_depth= recursed_depth + 1;
                } else {
                    DEBUG_STUDYDATA("gosub-inf", data, depth, is_inf, min, stopmin, delta);
                    /* some form of infinite recursion, assume infinite length
                     * */
                    if (flags & SCF_DO_SUBSTR) {
                        scan_commit(pRExC_state, data, minlenp, is_inf);
                        data->cur_is_floating = 1;
                    }
                    is_inf = is_inf_internal = 1;
                    if (flags & SCF_DO_STCLASS_OR) /* Allow everything */
                        ssc_anything(data->start_class);
                    flags &= ~SCF_DO_STCLASS;

                    start= NULL; /* reset start so we dont recurse later on. */
                }
            } else {
                paren = stopparen;
                start = scan + 2;
                end = regnext(scan);
            }
            if (start) {
                scan_frame *newframe;
                assert(end);
                if (!RExC_frame_last) {
                    Newxz(newframe, 1, scan_frame);
                    SAVEDESTRUCTOR_X(S_unwind_scan_frames, newframe);
                    RExC_frame_head= newframe;
                    RExC_frame_count++;
                } else if (!RExC_frame_last->next_frame) {
                    Newxz(newframe, 1, scan_frame);
                    RExC_frame_last->next_frame= newframe;
                    newframe->prev_frame= RExC_frame_last;
                    RExC_frame_count++;
                } else {
                    newframe= RExC_frame_last->next_frame;
                }
                RExC_frame_last= newframe;

                newframe->next_regnode = regnext(scan);
                newframe->last_regnode = last;
                newframe->stopparen = stopparen;
                newframe->prev_recursed_depth = recursed_depth;
                newframe->this_prev_frame= frame;
                newframe->in_gosub = (
                    (frame && frame->in_gosub) || OP(scan) == GOSUB
                );

                DEBUG_STUDYDATA("frame-new", data, depth, is_inf, min, stopmin, delta);
                DEBUG_PEEP("fnew", scan, depth, flags);

                frame = newframe;
                scan =  start;
                stopparen = paren;
                last = end;
                depth = depth + 1;
                recursed_depth= my_recursed_depth;

                continue;
            }
        }
        else if (REGNODE_TYPE(OP(scan)) == EXACT && ! isEXACTFish(OP(scan))) {
            SSize_t bytelen = STR_LEN(scan), charlen;
            UV uc;
            assert(bytelen);
            if (UTF) {
                const U8 * const s = (U8*)STRING(scan);
                uc = utf8_to_uvchr_buf(s, s + bytelen, NULL);
                charlen = utf8_length(s, s + bytelen);
            } else {
                uc = *((U8*)STRING(scan));
                charlen = bytelen;
            }
            min += charlen;
            if (flags & SCF_DO_SUBSTR) { /* Update longest substr. */
                /* The code below prefers earlier match for fixed
                   offset, later match for variable offset.  */
                if (data->last_end == -1) { /* Update the start info. */
                    data->last_start_min = data->pos_min;
                    data->last_start_max =
                        is_inf ? OPTIMIZE_INFTY
                        : (data->pos_delta > OPTIMIZE_INFTY - data->pos_min)
                            ? OPTIMIZE_INFTY : data->pos_min + data->pos_delta;
                }
                sv_catpvn(data->last_found, STRING(scan), bytelen);
                if (UTF)
                    SvUTF8_on(data->last_found);
                {
                    SV * const sv = data->last_found;
                    MAGIC * const mg = SvUTF8(sv) && SvMAGICAL(sv) ?
                        mg_find(sv, PERL_MAGIC_utf8) : NULL;
                    if (mg && mg->mg_len >= 0)
                        mg->mg_len += charlen;
                }
                data->last_end = data->pos_min + charlen;
                data->pos_min += charlen; /* As in the first entry. */
                data->flags &= ~SF_BEFORE_EOL;
            }

            /* ANDing the code point leaves at most it, and not in locale, and
             * can't match null string */
            if (flags & SCF_DO_STCLASS_AND) {
                ssc_cp_and(data->start_class, uc);
                ANYOF_FLAGS(data->start_class) &= ~SSC_MATCHES_EMPTY_STRING;
                ssc_clear_locale(data->start_class);
            }
            else if (flags & SCF_DO_STCLASS_OR) {
                ssc_add_cp(data->start_class, uc);
                ssc_and(pRExC_state, data->start_class, (regnode_charclass *) and_withp);

                /* See commit msg 749e076fceedeb708a624933726e7989f2302f6a */
                ANYOF_FLAGS(data->start_class) &= ~SSC_MATCHES_EMPTY_STRING;
            }
            flags &= ~SCF_DO_STCLASS;
            DEBUG_STUDYDATA("end EXACT", data, depth, is_inf, min, stopmin, delta);
        }
        else if (REGNODE_TYPE(OP(scan)) == EXACT) {
            /* But OP != EXACT!, so is EXACTFish */
            SSize_t bytelen = STR_LEN(scan), charlen;
            const U8 * s = (U8*)STRING(scan);

            /* Replace a length 1 ASCII fold pair node with an ANYOFM node,
             * with the mask set to the complement of the bit that differs
             * between upper and lower case, and the lowest code point of the
             * pair (which the '&' forces) */
            if (     bytelen == 1
                &&   isALPHA_A(*s)
                &&  (         OP(scan) == EXACTFAA
                     || (     OP(scan) == EXACTFU
                         && ! HAS_NONLATIN1_SIMPLE_FOLD_CLOSURE(*s)))
                &&   mutate_ok
            ) {
                U8 mask = ~ ('A' ^ 'a'); /* These differ in just one bit */

                OP(scan) = ANYOFM;
                ARG1u_SET(scan, *s & mask);
                FLAGS(scan) = mask;
                /* We're not EXACTFish any more, so restudy.
                 * Search for "restudy" in this file to find
                 * a comment with details. */
                continue;
            }

            /* Search for fixed substrings supports EXACT only. */
            if (flags & SCF_DO_SUBSTR) {
                assert(data);
                scan_commit(pRExC_state, data, minlenp, is_inf);
            }
            charlen = UTF ? (SSize_t) utf8_length(s, s + bytelen) : bytelen;
            if (unfolded_multi_char) {
                RExC_seen |= REG_UNFOLDED_MULTI_SEEN;
            }
            min += charlen - min_subtract;
            assert (min >= 0);
            if ((SSize_t)min_subtract < OPTIMIZE_INFTY
                && delta < OPTIMIZE_INFTY - (SSize_t)min_subtract
            ) {
                delta += min_subtract;
            } else {
                delta = OPTIMIZE_INFTY;
            }
            if (flags & SCF_DO_SUBSTR) {
                data->pos_min += charlen - min_subtract;
                if (data->pos_min < 0) {
                    data->pos_min = 0;
                }
                if ((SSize_t)min_subtract < OPTIMIZE_INFTY
                    && data->pos_delta < OPTIMIZE_INFTY - (SSize_t)min_subtract
                ) {
                    data->pos_delta += min_subtract;
                } else {
                    data->pos_delta = OPTIMIZE_INFTY;
                }
                if (min_subtract) {
                    data->cur_is_floating = 1; /* float */
                }
            }

            if (flags & SCF_DO_STCLASS) {
                SV* EXACTF_invlist = make_exactf_invlist(pRExC_state, scan);

                assert(EXACTF_invlist);
                if (flags & SCF_DO_STCLASS_AND) {
                    if (OP(scan) != EXACTFL)
                        ssc_clear_locale(data->start_class);
                    ANYOF_FLAGS(data->start_class) &= ~SSC_MATCHES_EMPTY_STRING;
                    ANYOF_POSIXL_ZERO(data->start_class);
                    ssc_intersection(data->start_class, EXACTF_invlist, FALSE);
                }
                else {  /* SCF_DO_STCLASS_OR */
                    ssc_union(data->start_class, EXACTF_invlist, FALSE);
                    ssc_and(pRExC_state, data->start_class, (regnode_charclass *) and_withp);

                    /* See commit msg 749e076fceedeb708a624933726e7989f2302f6a */
                    ANYOF_FLAGS(data->start_class) &= ~SSC_MATCHES_EMPTY_STRING;
                }
                flags &= ~SCF_DO_STCLASS;
                SvREFCNT_dec(EXACTF_invlist);
            }
            DEBUG_STUDYDATA("end EXACTish", data, depth, is_inf, min, stopmin, delta);
        }
        else if (REGNODE_VARIES(OP(scan))) {
            SSize_t mincount, maxcount, minnext, deltanext, pos_before = 0;
            I32 fl = 0;
            U32 f = flags;
            regnode * const oscan = scan;
            regnode_ssc this_class;
            regnode_ssc *oclass = NULL;
            I32 next_is_eval = 0;

            switch (REGNODE_TYPE(OP(scan))) {
            case WHILEM:                /* End of (?:...)* . */
                scan = REGNODE_AFTER(scan);
                goto finish;
            case PLUS:
                if (flags & (SCF_DO_SUBSTR | SCF_DO_STCLASS)) {
                    next = REGNODE_AFTER(scan);
                    if (   (     REGNODE_TYPE(OP(next)) == EXACT
                            && ! isEXACTFish(OP(next)))
                        || (flags & SCF_DO_STCLASS))
                    {
                        mincount = 1;
                        maxcount = REG_INFTY;
                        next = regnext(scan);
                        scan = REGNODE_AFTER(scan);
                        goto do_curly;
                    }
                }
                if (flags & SCF_DO_SUBSTR)
                    data->pos_min++;
                /* This will bypass the formal 'min += minnext * mincount'
                 * calculation in the do_curly path, so assumes min width
                 * of the PLUS payload is exactly one. */
                min++;
                /* FALLTHROUGH */
            case STAR:
                next = REGNODE_AFTER(scan);

                /* This temporary node can now be turned into EXACTFU, and
                 * must, as regexec.c doesn't handle it */
                if (OP(next) == EXACTFU_S_EDGE && mutate_ok) {
                    OP(next) = EXACTFU;
                }

                if (     STR_LEN(next) == 1
                    &&   isALPHA_A(* STRING(next))
                    && (         OP(next) == EXACTFAA
                        || (     OP(next) == EXACTFU
                            && ! HAS_NONLATIN1_SIMPLE_FOLD_CLOSURE(* STRING(next))))
                    &&   mutate_ok
                ) {
                    /* These differ in just one bit */
                    U8 mask = ~ ('A' ^ 'a');

                    assert(isALPHA_A(* STRING(next)));

                    /* Then replace it by an ANYOFM node, with
                    * the mask set to the complement of the
                    * bit that differs between upper and lower
                    * case, and the lowest code point of the
                    * pair (which the '&' forces) */
                    OP(next) = ANYOFM;
                    ARG1u_SET(next, *STRING(next) & mask);
                    FLAGS(next) = mask;
                }

                if (flags & SCF_DO_STCLASS) {
                    mincount = 0;
                    maxcount = REG_INFTY;
                    next = regnext(scan);
                    scan = REGNODE_AFTER(scan);
                    goto do_curly;
                }
                if (flags & SCF_DO_SUBSTR) {
                    scan_commit(pRExC_state, data, minlenp, is_inf);
                    /* Cannot extend fixed substrings */
                    data->cur_is_floating = 1; /* float */
                }
                is_inf = is_inf_internal = 1;
                scan = regnext(scan);
                goto optimize_curly_tail;
            case CURLY:
                if (stopparen>0 && (OP(scan)==CURLYN || OP(scan)==CURLYM)
                    && (FLAGS(scan) == stopparen))
                {
                    mincount = 1;
                    maxcount = 1;
                } else {
                    mincount = ARG1i(scan);
                    maxcount = ARG2i(scan);
                }
                next = regnext(scan);
                if (OP(scan) == CURLYX) {
                    I32 lp = (data ? *(data->last_closep) : 0);
                    FLAGS(scan) = ((lp <= (I32)U8_MAX) ? (U8)lp : U8_MAX);
                }
                scan = REGNODE_AFTER(scan);
                next_is_eval = (OP(scan) == EVAL);
              do_curly:
                if (flags & SCF_DO_SUBSTR) {
                    if (mincount == 0)
                        scan_commit(pRExC_state, data, minlenp, is_inf);
                    /* Cannot extend fixed substrings */
                    pos_before = data->pos_min;
                }
                if (data) {
                    fl = data->flags;
                    data->flags &= ~(SF_HAS_PAR|SF_IN_PAR|SF_HAS_EVAL);
                    if (is_inf)
                        data->flags |= SF_IS_INF;
                }
                if (flags & SCF_DO_STCLASS) {
                    ssc_init(pRExC_state, &this_class);
                    oclass = data->start_class;
                    data->start_class = &this_class;
                    f |= SCF_DO_STCLASS_AND;
                    f &= ~SCF_DO_STCLASS_OR;
                }
                /* Exclude from super-linear cache processing any {n,m}
                   regops for which the combination of input pos and regex
                   pos is not enough information to determine if a match
                   will be possible.

                   For example, in the regex /foo(bar\s*){4,8}baz/ with the
                   regex pos at the \s*, the prospects for a match depend not
                   only on the input position but also on how many (bar\s*)
                   repeats into the {4,8} we are. */
               if ((mincount > 1) || (maxcount > 1 && maxcount != REG_INFTY))
                    f &= ~SCF_WHILEM_VISITED_POS;

                /* This will finish on WHILEM, setting scan, or on NULL: */
                /* recurse study_chunk() on loop bodies */
                minnext = study_chunk(pRExC_state, &scan, minlenp, &deltanext,
                                  last, data, stopparen, recursed_depth, NULL,
                                  (mincount == 0
                                   ? (f & ~SCF_DO_SUBSTR)
                                   : f)
                                  , depth+1, mutate_ok);

                if (data && data->flags & SCF_SEEN_ACCEPT) {
                    if (mincount > 1)
                        mincount = 1;
                }

                if (flags & SCF_DO_STCLASS)
                    data->start_class = oclass;
                if (mincount == 0 || minnext == 0) {
                    if (flags & SCF_DO_STCLASS_OR) {
                        ssc_or(pRExC_state, data->start_class, (regnode_charclass *) &this_class);
                    }
                    else if (flags & SCF_DO_STCLASS_AND) {
                        /* Switch to OR mode: cache the old value of
                         * data->start_class */
                        INIT_AND_WITHP;
                        StructCopy(data->start_class, and_withp, regnode_ssc);
                        flags &= ~SCF_DO_STCLASS_AND;
                        StructCopy(&this_class, data->start_class, regnode_ssc);
                        flags |= SCF_DO_STCLASS_OR;
                        ANYOF_FLAGS(data->start_class)
                                                |= SSC_MATCHES_EMPTY_STRING;
                    }
                } else {                /* Non-zero len */
                    if (flags & SCF_DO_STCLASS_OR) {
                        ssc_or(pRExC_state, data->start_class, (regnode_charclass *) &this_class);
                        ssc_and(pRExC_state, data->start_class, (regnode_charclass *) and_withp);
                    }
                    else if (flags & SCF_DO_STCLASS_AND)
                        ssc_and(pRExC_state, data->start_class, (regnode_charclass *) &this_class);
                    flags &= ~SCF_DO_STCLASS;
                }
                if (!scan)              /* It was not CURLYX, but CURLY. */
                    scan = next;
                if (((flags & (SCF_TRIE_DOING_RESTUDY|SCF_DO_SUBSTR))==SCF_DO_SUBSTR)
                    /* ? quantifier ok, except for (?{ ... }) */
                    && (next_is_eval || !(mincount == 0 && maxcount == 1))
                    && (minnext == 0) && (deltanext == 0)
                    && data && !(data->flags & (SF_HAS_PAR|SF_IN_PAR))
                    && maxcount <= REG_INFTY/3) /* Complement check for big
                                                   count */
                {
                    _WARN_HELPER(RExC_precomp_end, packWARN(WARN_REGEXP),
                        Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP),
                            "Quantifier unexpected on zero-length expression "
                            "in regex m/%" UTF8f "/",
                             UTF8fARG(UTF, RExC_precomp_end - RExC_precomp,
                                  RExC_precomp)));
                }

                if ( ( minnext > 0 && mincount >= SSize_t_MAX / minnext )
                    || min >= SSize_t_MAX - minnext * mincount )
                {
                    FAIL("Regexp out of space");
                }

                min += minnext * mincount;
                is_inf_internal |= deltanext == OPTIMIZE_INFTY
                         || (maxcount == REG_INFTY && minnext + deltanext > 0);
                is_inf |= is_inf_internal;
                if (is_inf) {
                    delta = OPTIMIZE_INFTY;
                } else {
                    delta += (minnext + deltanext) * maxcount
                             - minnext * mincount;
                }

                if (data && data->flags & SCF_SEEN_ACCEPT) {
                    if (flags & SCF_DO_SUBSTR) {
                        scan_commit(pRExC_state, data, minlenp, is_inf);
                        flags &= ~SCF_DO_SUBSTR;
                    }
                    if (stopmin > min)
                        stopmin = min;
                    DEBUG_STUDYDATA("after-whilem accept", data, depth, is_inf, min, stopmin, delta);
                }
                DEBUG_STUDYDATA("PRE CURLYX_TO_CURLYN", data, depth, is_inf, min, stopmin, delta);
                /* Try powerful optimization CURLYX => CURLYN. */
                if ( RE_OPTIMIZE_CURLYX_TO_CURLYN
                     && OP(oscan) == CURLYX
                     && data
                     && !(RExC_seen & REG_PESSIMIZE_SEEN) /* XXX: for now disable whenever a
                                                            non optimistic eval is seen
                                                            anywhere.*/
                     && ( data->flags & SF_IN_PAR ) /* has parens */
                     && !deltanext
                     && minnext == 1
                     && mutate_ok
                ) {
                    DEBUG_STUDYDATA("CURLYX_TO_CURLYN", data, depth, is_inf, min, stopmin, delta);
                    /* Try to optimize to CURLYN.  */
                    regnode *nxt = REGNODE_AFTER_type(oscan, tregnode_CURLYX);
                    regnode * const nxt1 = nxt;
#ifdef DEBUGGING
                    regnode *nxt2;
#endif
                    /* Skip open. */
                    nxt = regnext(nxt);
                    if (!REGNODE_SIMPLE(OP(nxt))
                        && !(REGNODE_TYPE(OP(nxt)) == EXACT
                             && STR_LEN(nxt) == 1))
                        goto nogo;
#ifdef DEBUGGING
                    nxt2 = nxt;
#endif
                    nxt = regnext(nxt);
                    if (OP(nxt) != CLOSE)
                        goto nogo;
                    if (RExC_open_parens) {

                        /*open->CURLYM*/
                        RExC_open_parens[PARNO(nxt1)] = REGNODE_OFFSET(oscan);

                        /*close->while*/
                        RExC_close_parens[PARNO(nxt1)] = REGNODE_OFFSET(nxt) + 2;
                    }
                    /* Now we know that nxt2 is the only contents: */
                    FLAGS(oscan) = (U8)PARNO(nxt);
                    OP(oscan) = CURLYN;
                    OP(nxt1) = NOTHING; /* was OPEN. */

#ifdef DEBUGGING
                    OP(nxt1 + 1) = OPTIMIZED; /* was count. */
                    NEXT_OFF(nxt1+ 1) = 0; /* just for consistency. */
                    NEXT_OFF(nxt2) = 0; /* just for consistency with CURLY. */
                    OP(nxt) = OPTIMIZED;        /* was CLOSE. */
                    OP(nxt + 1) = OPTIMIZED; /* was count. */
                    NEXT_OFF(nxt+ 1) = 0; /* just for consistency. */
#endif
                }
              nogo:

                DEBUG_STUDYDATA("PRE CURLYX_TO_CURLYM", data, depth, is_inf, min, stopmin, delta);

                /* Try optimization CURLYX => CURLYM. */
                if ( RE_OPTIMIZE_CURLYX_TO_CURLYM
                     && OP(oscan) == CURLYX
                     && data
                     && !(RExC_seen & REG_PESSIMIZE_SEEN) /* XXX: for now disable whenever a
                                                            non optimistic eval is seen
                                                            anywhere.*/
                     && !(data->flags & SF_HAS_PAR) /* no parens! */
                     && !deltanext     /* atom is fixed width */
                     && minnext != 0  /* CURLYM can't handle zero width */
                         /* Nor characters whose fold at run-time may be
                          * multi-character */
                     && !(RExC_seen & REG_UNFOLDED_MULTI_SEEN)
                     && mutate_ok
                ) {
                    DEBUG_STUDYDATA("CURLYX_TO_CURLYM", data, depth, is_inf, min, stopmin, delta);
                    /* XXXX How to optimize if data == 0? */
                    /* Optimize to a simpler form.  */
                    regnode *nxt = REGNODE_AFTER_type(oscan, tregnode_CURLYX); /* OPEN */
                    regnode *nxt2;

                    OP(oscan) = CURLYM;
                    while ( (nxt2 = regnext(nxt)) /* skip over embedded stuff*/
                            && (OP(nxt2) != WHILEM))
                        nxt = nxt2;
                    OP(nxt2)  = SUCCEED; /* Whas WHILEM */
                    /* Need to optimize away parenths. */
                    if ((data->flags & SF_IN_PAR) && OP(nxt) == CLOSE) {
                        /* Set the parenth number.  */
                        /* note that we have changed the type of oscan to CURLYM here */
                        regnode *nxt1 = REGNODE_AFTER_type(oscan, tregnode_CURLYM); /* OPEN*/

                        FLAGS(oscan) = (U8)PARNO(nxt);
                        if (RExC_open_parens) {
                             /*open->CURLYM*/
                            RExC_open_parens[PARNO(nxt1)] = REGNODE_OFFSET(oscan);

                            /*close->NOTHING*/
                            RExC_close_parens[PARNO(nxt1)] = REGNODE_OFFSET(nxt2)
                                                         + 1;
                        }
                        OP(nxt1) = OPTIMIZED;   /* was OPEN. */
                        OP(nxt) = OPTIMIZED;    /* was CLOSE. */

#ifdef DEBUGGING
                        OP(nxt1 + 1) = OPTIMIZED; /* was count. */
                        OP(nxt + 1) = OPTIMIZED; /* was count. */
                        NEXT_OFF(nxt1 + 1) = 0; /* just for consistency. */
                        NEXT_OFF(nxt + 1) = 0; /* just for consistency. */
#endif
#if 0
                        while ( nxt1 && (OP(nxt1) != WHILEM)) {
                            regnode *nnxt = regnext(nxt1);
                            if (nnxt == nxt) {
                                if (REGNODE_OFF_BY_ARG(OP(nxt1)))
                                    ARG1u_SET(nxt1, nxt2 - nxt1);
                                else if (nxt2 - nxt1 < U16_MAX)
                                    NEXT_OFF(nxt1) = nxt2 - nxt1;
                                else
                                    OP(nxt) = NOTHING;  /* Cannot beautify */
                            }
                            nxt1 = nnxt;
                        }
#endif
                        /* Optimize again: */
                        /* recurse study_chunk() on optimised CURLYX => CURLYM */
                        study_chunk(pRExC_state, &nxt1, minlenp, &deltanext, nxt,
                                    NULL, stopparen, recursed_depth, NULL, 0,
                                    depth+1, mutate_ok);
                    }
                    else
                        FLAGS(oscan) = 0;
                }
                else if ((OP(oscan) == CURLYX)
                         && (flags & SCF_WHILEM_VISITED_POS)
                         /* See the comment on a similar expression above.
                            However, this time it's not a subexpression
                            we care about, but the expression itself. */
                         && (maxcount == REG_INFTY)
                         && data) {
                    /* This stays as CURLYX, we can put the count/of pair. */
                    /* Find WHILEM (as in regexec.c) */
                    regnode *nxt = oscan + NEXT_OFF(oscan);

                    if (OP(REGNODE_BEFORE(nxt)) == NOTHING) /* LONGJMP */
                        nxt += ARG1u(nxt);
                    nxt = REGNODE_BEFORE(nxt);
                    if (FLAGS(nxt) & 0xf) {
                        /* we've already set whilem count on this node */
                    } else if (++data->whilem_c < 16) {
                        assert(data->whilem_c <= RExC_whilem_seen);
                        FLAGS(nxt) = (U8)(data->whilem_c
                            | (RExC_whilem_seen << 4)); /* On WHILEM */
                    }
                }
                if (data && fl & (SF_HAS_PAR|SF_IN_PAR))
                    pars++;
                if (flags & SCF_DO_SUBSTR) {
                    SV *last_str = NULL;
                    STRLEN last_chrs = 0;
                    int counted = mincount != 0;

                    if (data->last_end > 0 && mincount != 0) { /* Ends with a
                                                                  string. */
                        SSize_t b = pos_before >= data->last_start_min
                            ? pos_before : data->last_start_min;
                        STRLEN l;
                        const char * const s = SvPV_const(data->last_found, l);
                        SSize_t old = b - data->last_start_min;
                        assert(old >= 0);

                        if (UTF)
                            old = utf8_hop_forward((U8*)s, old,
                                               (U8 *) SvEND(data->last_found))
                                - (U8*)s;
                        l -= old;
                        /* Get the added string: */
                        last_str = newSVpvn_utf8(s  + old, l, UTF);
                        last_chrs = UTF ? utf8_length((U8*)(s + old),
                                            (U8*)(s + old + l)) : l;
                        if (deltanext == 0 && pos_before == b) {
                            /* What was added is a constant string */
                            if (mincount > 1) {

                                SvGROW(last_str, (mincount * l) + 1);
                                repeatcpy(SvPVX(last_str) + l,
                                          SvPVX_const(last_str), l,
                                          mincount - 1);
                                SvCUR_set(last_str, SvCUR(last_str) * mincount);
                                /* Add additional parts. */
                                SvCUR_set(data->last_found,
                                          SvCUR(data->last_found) - l);
                                sv_catsv(data->last_found, last_str);
                                {
                                    SV * sv = data->last_found;
                                    MAGIC *mg =
                                        SvUTF8(sv) && SvMAGICAL(sv) ?
                                        mg_find(sv, PERL_MAGIC_utf8) : NULL;
                                    if (mg && mg->mg_len >= 0)
                                        mg->mg_len += last_chrs * (mincount-1);
                                }
                                last_chrs *= mincount;
                                data->last_end += l * (mincount - 1);
                            }
                        } else {
                            /* start offset must point into the last copy */
                            data->last_start_min += minnext * (mincount - 1);
                            data->last_start_max =
                              is_inf
                               ? OPTIMIZE_INFTY
                               : data->last_start_max +
                                 (maxcount - 1) * (minnext + data->pos_delta);
                        }
                    }
                    /* It is counted once already... */
                    data->pos_min += minnext * (mincount - counted);
#if 0
    Perl_re_printf( aTHX_  "counted=%" UVuf " deltanext=%" UVuf
                              " OPTIMIZE_INFTY=%" UVuf " minnext=%" UVuf
                              " maxcount=%" UVuf " mincount=%" UVuf
                              " data->pos_delta=%" UVuf "\n",
        (UV)counted, (UV)deltanext, (UV)OPTIMIZE_INFTY, (UV)minnext,
        (UV)maxcount, (UV)mincount, (UV)data->pos_delta);
    if (deltanext != OPTIMIZE_INFTY)
        Perl_re_printf( aTHX_  "LHS=%" UVuf " RHS=%" UVuf "\n",
            (UV)(-counted * deltanext + (minnext + deltanext) * maxcount
            - minnext * mincount), (UV)(OPTIMIZE_INFTY - data->pos_delta));
#endif
                    if (deltanext == OPTIMIZE_INFTY
                        || data->pos_delta == OPTIMIZE_INFTY
                        || -counted * deltanext + (minnext + deltanext) * maxcount - minnext * mincount >= OPTIMIZE_INFTY - data->pos_delta)
                        data->pos_delta = OPTIMIZE_INFTY;
                    else
                        data->pos_delta += - counted * deltanext +
                        (minnext + deltanext) * maxcount - minnext * mincount;
                    if (mincount != maxcount) {
                         /* Cannot extend fixed substrings found inside
                            the group.  */
                        scan_commit(pRExC_state, data, minlenp, is_inf);
                        if (mincount && last_str) {
                            SV * const sv = data->last_found;
                            MAGIC * const mg = SvUTF8(sv) && SvMAGICAL(sv) ?
                                mg_find(sv, PERL_MAGIC_utf8) : NULL;

                            if (mg)
                                mg->mg_len = -1;
                            sv_setsv(sv, last_str);
                            data->last_end = data->pos_min;
                            data->last_start_min = data->pos_min - last_chrs;
                            data->last_start_max = is_inf
                                ? OPTIMIZE_INFTY
                                : data->pos_min + data->pos_delta - last_chrs;
                        }
                        data->cur_is_floating = 1; /* float */
                    }
                    SvREFCNT_dec(last_str);
                }
                if (data && (fl & SF_HAS_EVAL))
                    data->flags |= SF_HAS_EVAL;
              optimize_curly_tail:
                rck_elide_nothing(oscan);
                continue;

            default:
                Perl_croak(aTHX_ "panic: unexpected varying REx opcode %d",
                                                                    OP(scan));
            case REF:
            case CLUMP:
                if (flags & SCF_DO_SUBSTR) {
                    /* Cannot expect anything... */
                    scan_commit(pRExC_state, data, minlenp, is_inf);
                    data->cur_is_floating = 1; /* float */
                }
                is_inf = is_inf_internal = 1;
                if (flags & SCF_DO_STCLASS_OR) {
                    if (OP(scan) == CLUMP) {
                        /* Actually is any start char, but very few code points
                         * aren't start characters */
                        ssc_match_all_cp(data->start_class);
                    }
                    else {
                        ssc_anything(data->start_class);
                    }
                }
                flags &= ~SCF_DO_STCLASS;
                break;
            }
        }
        else if (OP(scan) == LNBREAK) {
            if (flags & SCF_DO_STCLASS) {
                if (flags & SCF_DO_STCLASS_AND) {
                    ssc_intersection(data->start_class,
                                    PL_XPosix_ptrs[CC_VERTSPACE_], FALSE);
                    ssc_clear_locale(data->start_class);
                    ANYOF_FLAGS(data->start_class)
                                                &= ~SSC_MATCHES_EMPTY_STRING;
                }
                else if (flags & SCF_DO_STCLASS_OR) {
                    ssc_union(data->start_class,
                              PL_XPosix_ptrs[CC_VERTSPACE_],
                              FALSE);
                    ssc_and(pRExC_state, data->start_class, (regnode_charclass *) and_withp);

                    /* See commit msg for
                     * 749e076fceedeb708a624933726e7989f2302f6a */
                    ANYOF_FLAGS(data->start_class)
                                                &= ~SSC_MATCHES_EMPTY_STRING;
                }
                flags &= ~SCF_DO_STCLASS;
            }
            min++;
            if (delta != OPTIMIZE_INFTY)
                delta++;    /* Because of the 2 char string cr-lf */
            if (flags & SCF_DO_SUBSTR) {
                /* Cannot expect anything... */
                scan_commit(pRExC_state, data, minlenp, is_inf);
                data->pos_min += 1;
                if (data->pos_delta != OPTIMIZE_INFTY) {
                    data->pos_delta += 1;
                }
                data->cur_is_floating = 1; /* float */
            }
        }
        else if (REGNODE_SIMPLE(OP(scan))) {

            if (flags & SCF_DO_SUBSTR) {
                scan_commit(pRExC_state, data, minlenp, is_inf);
                data->pos_min++;
            }
            min++;
            if (flags & SCF_DO_STCLASS) {
                bool invert = 0;
                SV* my_invlist = NULL;
                U8 namedclass;

                /* See commit msg 749e076fceedeb708a624933726e7989f2302f6a */
                ANYOF_FLAGS(data->start_class) &= ~SSC_MATCHES_EMPTY_STRING;

                /* Some of the logic below assumes that switching
                   locale on will only add false positives. */
                switch (OP(scan)) {

                default:
#ifdef DEBUGGING
                   Perl_croak(aTHX_ "panic: unexpected simple REx opcode %d",
                                                                     OP(scan));
#endif
                case SANY:
                    if (flags & SCF_DO_STCLASS_OR) /* Allow everything */
                        ssc_match_all_cp(data->start_class);
                    break;

                case REG_ANY:
                    {
                        SV* REG_ANY_invlist = _new_invlist(2);
                        REG_ANY_invlist = add_cp_to_invlist(REG_ANY_invlist,
                                                            '\n');
                        if (flags & SCF_DO_STCLASS_OR) {
                            ssc_union(data->start_class,
                                      REG_ANY_invlist,
                                      TRUE /* TRUE => invert, hence all but \n
                                            */
                                      );
                        }
                        else if (flags & SCF_DO_STCLASS_AND) {
                            ssc_intersection(data->start_class,
                                             REG_ANY_invlist,
                                             TRUE  /* TRUE => invert */
                                             );
                            ssc_clear_locale(data->start_class);
                        }
                        SvREFCNT_dec_NN(REG_ANY_invlist);
                    }
                    break;

                case ANYOFD:
                case ANYOFL:
                case ANYOFPOSIXL:
                case ANYOFH:
                case ANYOFHb:
                case ANYOFHr:
                case ANYOFHs:
                case ANYOF:
                    if (flags & SCF_DO_STCLASS_AND)
                        ssc_and(pRExC_state, data->start_class,
                                (regnode_charclass *) scan);
                    else
                        ssc_or(pRExC_state, data->start_class,
                                                          (regnode_charclass *) scan);
                    break;

                case ANYOFHbbm:
                  {
                    SV* cp_list = get_ANYOFHbbm_contents(scan);

                    if (flags & SCF_DO_STCLASS_OR) {
                        ssc_union(data->start_class, cp_list, invert);
                    }
                    else if (flags & SCF_DO_STCLASS_AND) {
                        ssc_intersection(data->start_class, cp_list, invert);
                    }

                    SvREFCNT_dec_NN(cp_list);
                    break;
                  }

                case NANYOFM: /* NANYOFM already contains the inversion of the
                                 input ANYOF data, so, unlike things like
                                 NPOSIXA, don't change 'invert' to TRUE */
                    /* FALLTHROUGH */
                case ANYOFM:
                  {
                    SV* cp_list = get_ANYOFM_contents(scan);

                    if (flags & SCF_DO_STCLASS_OR) {
                        ssc_union(data->start_class, cp_list, invert);
                    }
                    else if (flags & SCF_DO_STCLASS_AND) {
                        ssc_intersection(data->start_class, cp_list, invert);
                    }

                    SvREFCNT_dec_NN(cp_list);
                    break;
                  }

                case ANYOFR:
                case ANYOFRb:
                  {
                    SV* cp_list = NULL;

                    cp_list = _add_range_to_invlist(cp_list,
                                        ANYOFRbase(scan),
                                        ANYOFRbase(scan) + ANYOFRdelta(scan));

                    if (flags & SCF_DO_STCLASS_OR) {
                        ssc_union(data->start_class, cp_list, invert);
                    }
                    else if (flags & SCF_DO_STCLASS_AND) {
                        ssc_intersection(data->start_class, cp_list, invert);
                    }

                    SvREFCNT_dec_NN(cp_list);
                    break;
                  }

                case NPOSIXL:
                    invert = 1;
                    /* FALLTHROUGH */

                case POSIXL:
                    namedclass = classnum_to_namedclass(FLAGS(scan)) + invert;
                    if (flags & SCF_DO_STCLASS_AND) {
                        bool was_there = cBOOL(
                                          ANYOF_POSIXL_TEST(data->start_class,
                                                                 namedclass));
                        ANYOF_POSIXL_ZERO(data->start_class);
                        if (was_there) {    /* Do an AND */
                            ANYOF_POSIXL_SET(data->start_class, namedclass);
                        }
                        /* No individual code points can now match */
                        data->start_class->invlist
                                                = sv_2mortal(_new_invlist(0));
                    }
                    else {
                        int complement = namedclass + ((invert) ? -1 : 1);

                        assert(flags & SCF_DO_STCLASS_OR);

                        /* If the complement of this class was already there,
                         * the result is that they match all code points,
                         * (\d + \D == everything).  Remove the classes from
                         * future consideration.  Locale is not relevant in
                         * this case */
                        if (ANYOF_POSIXL_TEST(data->start_class, complement)) {
                            ssc_match_all_cp(data->start_class);
                            ANYOF_POSIXL_CLEAR(data->start_class, namedclass);
                            ANYOF_POSIXL_CLEAR(data->start_class, complement);
                        }
                        else {  /* The usual case; just add this class to the
                                   existing set */
                            ANYOF_POSIXL_SET(data->start_class, namedclass);
                        }
                    }
                    break;

                case NPOSIXA:   /* For these, we always know the exact set of
                                   what's matched */
                    invert = 1;
                    /* FALLTHROUGH */
                case POSIXA:
                    my_invlist = invlist_clone(PL_Posix_ptrs[FLAGS(scan)], NULL);
                    goto join_posix_and_ascii;

                case NPOSIXD:
                case NPOSIXU:
                    invert = 1;
                    /* FALLTHROUGH */
                case POSIXD:
                case POSIXU:
                    my_invlist = invlist_clone(PL_XPosix_ptrs[FLAGS(scan)], NULL);

                    /* NPOSIXD matches all upper Latin1 code points unless the
                     * target string being matched is UTF-8, which is
                     * unknowable until match time.  Since we are going to
                     * invert, we want to get rid of all of them so that the
                     * inversion will match all */
                    if (OP(scan) == NPOSIXD) {
                        _invlist_subtract(my_invlist, PL_UpperLatin1,
                                          &my_invlist);
                    }

                  join_posix_and_ascii:

                    if (flags & SCF_DO_STCLASS_AND) {
                        ssc_intersection(data->start_class, my_invlist, invert);
                        ssc_clear_locale(data->start_class);
                    }
                    else {
                        assert(flags & SCF_DO_STCLASS_OR);
                        ssc_union(data->start_class, my_invlist, invert);
                    }
                    SvREFCNT_dec(my_invlist);
                }
                if (flags & SCF_DO_STCLASS_OR)
                    ssc_and(pRExC_state, data->start_class, (regnode_charclass *) and_withp);
                flags &= ~SCF_DO_STCLASS;
            }
        }
        else if (REGNODE_TYPE(OP(scan)) == EOL && flags & SCF_DO_SUBSTR) {
            data->flags |= (OP(scan) == MEOL
                            ? SF_BEFORE_MEOL
                            : SF_BEFORE_SEOL);
            scan_commit(pRExC_state, data, minlenp, is_inf);

        }
        else if (  REGNODE_TYPE(OP(scan)) == BRANCHJ
                 /* Lookbehind, or need to calculate parens/evals/stclass: */
                   && (FLAGS(scan) || data || (flags & SCF_DO_STCLASS))
                   && (OP(scan) == IFMATCH || OP(scan) == UNLESSM))
        {
            if ( !PERL_ENABLE_POSITIVE_ASSERTION_STUDY
                || OP(scan) == UNLESSM )
            {
                /* Negative Lookahead/lookbehind
                   In this case we can't do fixed string optimisation.
                */

                bool is_positive = OP(scan) == IFMATCH ? 1 : 0;
                SSize_t deltanext, minnext;
                SSize_t fake_last_close = 0;
                regnode *fake_last_close_op = NULL;
                regnode *cur_last_close_op;
                regnode *nscan;
                regnode_ssc intrnl;
                U32 f = (flags & SCF_TRIE_DOING_RESTUDY);

                StructCopy(&zero_scan_data, &data_fake, scan_data_t);
                if (data) {
                    data_fake.whilem_c = data->whilem_c;
                    data_fake.last_closep = data->last_closep;
                    data_fake.last_close_opp = data->last_close_opp;
                }
                else {
                    data_fake.last_closep = &fake_last_close;
                    data_fake.last_close_opp = &fake_last_close_op;
                }

                /* remember the last_close_op we saw so we can see if
                 * we are dealing with variable length lookbehind that
                 * contains capturing buffers, which are considered
                 * experimental */
                cur_last_close_op= *(data_fake.last_close_opp);

                data_fake.pos_delta = delta;
                if ( flags & SCF_DO_STCLASS && !FLAGS(scan)
                     && OP(scan) == IFMATCH ) { /* Lookahead */
                    ssc_init(pRExC_state, &intrnl);
                    data_fake.start_class = &intrnl;
                    f |= SCF_DO_STCLASS_AND;
                }
                if (flags & SCF_WHILEM_VISITED_POS)
                    f |= SCF_WHILEM_VISITED_POS;
                next = regnext(scan);
                nscan = REGNODE_AFTER(scan);

                /* recurse study_chunk() for lookahead body */
                minnext = study_chunk(pRExC_state, &nscan, minlenp, &deltanext,
                                      last, &data_fake, stopparen,
                                      recursed_depth, NULL, f, depth+1,
                                      mutate_ok);

                if (FLAGS(scan)) {
                    if (   deltanext < 0
                        || deltanext > (I32) U8_MAX
                        || minnext > (I32)U8_MAX
                        || minnext + deltanext > (I32)U8_MAX)
                    {
                        FAIL2("Lookbehind longer than %" UVuf " not implemented",
                              (UV)U8_MAX);
                    }

                    /* The 'next_off' field has been repurposed to count the
                     * additional starting positions to try beyond the initial
                     * one.  (This leaves it at 0 for non-variable length
                     * matches to avoid breakage for those not using this
                     * extension) */
                    if (deltanext)  {
                        NEXT_OFF(scan) = deltanext;
                        if (
                            /* See a CLOSE op inside this lookbehind? */
                            cur_last_close_op != *(data_fake.last_close_opp)
                            /* and not doing restudy. see: restudied */
                            && !(flags & SCF_TRIE_DOING_RESTUDY)
                        ) {
                            /* this is positive variable length lookbehind with
                             * capture buffers inside of it */
                            ckWARNexperimental_with_arg(RExC_parse,
                                WARN_EXPERIMENTAL__VLB,
                                "Variable length %s lookbehind with capturing is experimental",
                                is_positive ? "positive" : "negative");
                        }
                    }
                    FLAGS(scan) = (U8)minnext + deltanext;
                }
                if (data) {
                    if (data_fake.flags & (SF_HAS_PAR|SF_IN_PAR))
                        pars++;
                    if (data_fake.flags & SF_HAS_EVAL)
                        data->flags |= SF_HAS_EVAL;
                    data->whilem_c = data_fake.whilem_c;
                }
                if (f & SCF_DO_STCLASS_AND) {
                    if (flags & SCF_DO_STCLASS_OR) {
                        /* OR before, AND after: ideally we would recurse with
                         * data_fake to get the AND applied by study of the
                         * remainder of the pattern, and then derecurse;
                         * *** HACK *** for now just treat as "no information".
                         * See [perl #56690].
                         */
                        ssc_init(pRExC_state, data->start_class);
                    }  else {
                        /* AND before and after: combine and continue.  These
                         * assertions are zero-length, so can match an EMPTY
                         * string */
                        ssc_and(pRExC_state, data->start_class, (regnode_charclass *) &intrnl);
                        ANYOF_FLAGS(data->start_class)
                                                   |= SSC_MATCHES_EMPTY_STRING;
                    }
                }
                DEBUG_STUDYDATA("end LOOKAROUND", data, depth, is_inf, min, stopmin, delta);
            }
#if PERL_ENABLE_POSITIVE_ASSERTION_STUDY
            else {
                /* Positive Lookahead/lookbehind
                   In this case we can do fixed string optimisation,
                   but we must be careful about it. Note in the case of
                   lookbehind the positions will be offset by the minimum
                   length of the pattern, something we won't know about
                   until after the recurse.
                */
                SSize_t deltanext, fake_last_close = 0;
                regnode *last_close_op = NULL;
                regnode *nscan;
                regnode_ssc intrnl;
                U32 f = (flags & SCF_TRIE_DOING_RESTUDY);
                /* We use SAVEFREEPV so that when the full compile
                    is finished perl will clean up the allocated
                    minlens when it's all done. This way we don't
                    have to worry about freeing them when we know
                    they wont be used, which would be a pain.
                 */
                SSize_t *minnextp;
                Newx( minnextp, 1, SSize_t );
                SAVEFREEPV(minnextp);

                if (data) {
                    StructCopy(data, &data_fake, scan_data_t);
                    if ((flags & SCF_DO_SUBSTR) && data->last_found) {
                        f |= SCF_DO_SUBSTR;
                        if (FLAGS(scan))
                            scan_commit(pRExC_state, &data_fake, minlenp, is_inf);
                        data_fake.last_found=newSVsv(data->last_found);
                    }
                }
                else {
                    data_fake.last_closep = &fake_last_close;
                    data_fake.last_close_opp = &fake_last_close_opp;
                }
                data_fake.flags = 0;
                data_fake.substrs[0].flags = 0;
                data_fake.substrs[1].flags = 0;
                data_fake.pos_delta = delta;
                if (is_inf)
                    data_fake.flags |= SF_IS_INF;
                if ( flags & SCF_DO_STCLASS && !FLAGS(scan)
                     && OP(scan) == IFMATCH ) { /* Lookahead */
                    ssc_init(pRExC_state, &intrnl);
                    data_fake.start_class = &intrnl;
                    f |= SCF_DO_STCLASS_AND;
                }
                if (flags & SCF_WHILEM_VISITED_POS)
                    f |= SCF_WHILEM_VISITED_POS;
                next = regnext(scan);
                nscan = REGNODE_AFTER(scan);

                /* positive lookahead study_chunk() recursion */
                *minnextp = study_chunk(pRExC_state, &nscan, minnextp,
                                        &deltanext, last, &data_fake,
                                        stopparen, recursed_depth, NULL,
                                        f, depth+1, mutate_ok);
                if (FLAGS(scan)) {
                    assert(0);  /* This code has never been tested since this
                                   is normally not compiled */
                    if (   deltanext < 0
                        || deltanext > (I32) U8_MAX
                        || *minnextp > (I32)U8_MAX
                        || *minnextp + deltanext > (I32)U8_MAX)
                    {
                        FAIL2("Lookbehind longer than %" UVuf " not implemented",
                              (UV)U8_MAX);
                    }

                    if (deltanext) {
                        NEXT_OFF(scan) = deltanext;
                    }
                    FLAGS(scan) = (U8)*minnextp + deltanext;
                }

                *minnextp += min;

                if (f & SCF_DO_STCLASS_AND) {
                    ssc_and(pRExC_state, data->start_class, (regnode_charclass *) &intrnl);
                    ANYOF_FLAGS(data->start_class) |= SSC_MATCHES_EMPTY_STRING;
                }
                if (data) {
                    if (data_fake.flags & (SF_HAS_PAR|SF_IN_PAR))
                        pars++;
                    if (data_fake.flags & SF_HAS_EVAL)
                        data->flags |= SF_HAS_EVAL;
                    data->whilem_c = data_fake.whilem_c;
                    if ((flags & SCF_DO_SUBSTR) && data_fake.last_found) {
                        int i;
                        if (RExC_rx->minlen < *minnextp)
                            RExC_rx->minlen = *minnextp;
                        scan_commit(pRExC_state, &data_fake, minnextp, is_inf);
                        SvREFCNT_dec_NN(data_fake.last_found);

                        for (i = 0; i < 2; i++) {
                            if (data_fake.substrs[i].minlenp != minlenp) {
                                data->substrs[i].min_offset =
                                            data_fake.substrs[i].min_offset;
                                data->substrs[i].max_offset =
                                            data_fake.substrs[i].max_offset;
                                data->substrs[i].minlenp =
                                            data_fake.substrs[i].minlenp;
                                data->substrs[i].lookbehind += FLAGS(scan);
                            }
                        }
                    }
                }
            }
#endif
        }
        else if (OP(scan) == OPEN) {
            if (stopparen != (I32)PARNO(scan))
                pars++;
        }
        else if (OP(scan) == CLOSE) {
            if (stopparen == (I32)PARNO(scan)) {
                break;
            }
            if ((I32)PARNO(scan) == is_par) {
                next = regnext(scan);

                if ( next && (OP(next) != WHILEM) && next < last)
                    is_par = 0;         /* Disable optimization */
            }
            if (data) {
                *(data->last_closep) = PARNO(scan);
                *(data->last_close_opp) = scan;
            }
        }
        else if (OP(scan) == EVAL) {
            if (data && !(FLAGS(scan) & EVAL_OPTIMISTIC_FLAG) )
                data->flags |= SF_HAS_EVAL;
        }
        else if ( REGNODE_TYPE(OP(scan)) == ENDLIKE ) {
            if (flags & SCF_DO_SUBSTR) {
                scan_commit(pRExC_state, data, minlenp, is_inf);
                flags &= ~SCF_DO_SUBSTR;
            }
            if (OP(scan)==ACCEPT) {
                /* m{(*ACCEPT)x} does not have to start with 'x' */
                flags &= ~SCF_DO_STCLASS;
                if (data)
                    data->flags |= SCF_SEEN_ACCEPT;
                if (stopmin > min)
                    stopmin = min;
            }
        }
        else if (OP(scan) == COMMIT) {
            /* gh18770: m{abc(*COMMIT)xyz} must fail on "abc abcxyz", so we
             * must not end up with "abcxyz" as a fixed substring else we'll
             * skip straight to attempting to match at offset 4.
             */
            if (flags & SCF_DO_SUBSTR) {
                scan_commit(pRExC_state, data, minlenp, is_inf);
                flags &= ~SCF_DO_SUBSTR;
            }
        }
        else if (OP(scan) == LOGICAL && FLAGS(scan) == 2) /* Embedded follows */
        {
                if (flags & SCF_DO_SUBSTR) {
                    scan_commit(pRExC_state, data, minlenp, is_inf);
                    data->cur_is_floating = 1; /* float */
                }
                is_inf = is_inf_internal = 1;
                if (flags & SCF_DO_STCLASS_OR) /* Allow everything */
                    ssc_anything(data->start_class);
                flags &= ~SCF_DO_STCLASS;
        }
        else if (OP(scan) == GPOS) {
            if (!(RExC_rx->intflags & PREGf_GPOS_FLOAT) &&
                !(delta || is_inf || (data && data->pos_delta)))
            {
                if (!(RExC_rx->intflags & PREGf_ANCH) && (flags & SCF_DO_SUBSTR))
                    RExC_rx->intflags |= PREGf_ANCH_GPOS;
                if (RExC_rx->gofs < (STRLEN)min)
                    RExC_rx->gofs = min;
            } else {
                RExC_rx->intflags |= PREGf_GPOS_FLOAT;
                RExC_rx->gofs = 0;
            }
        }
#ifdef TRIE_STUDY_OPT
#ifdef FULL_TRIE_STUDY
        else if (REGNODE_TYPE(OP(scan)) == TRIE) {
            /* NOTE - There is similar code to this block above for handling
               BRANCH nodes on the initial study.  If you change stuff here
               check there too. */
            regnode *trie_node= scan;
            regnode *tail= regnext(scan);
            reg_trie_data *trie = (reg_trie_data*)RExC_rxi->data->data[ ARG1u(scan) ];
            SSize_t max1 = 0, min1 = OPTIMIZE_INFTY;
            regnode_ssc accum;

            if (flags & SCF_DO_SUBSTR) { /* XXXX Add !SUSPEND? */
                /* Cannot merge strings after this. */
                scan_commit(pRExC_state, data, minlenp, is_inf);
            }
            if (flags & SCF_DO_STCLASS)
                ssc_init_zero(pRExC_state, &accum);

            if (!trie->jump) {
                min1= trie->minlen;
                max1= trie->maxlen;
            } else {
                const regnode *nextbranch= NULL;
                U32 word;

                for ( word=1 ; word <= trie->wordcount ; word++)
                {
                    SSize_t deltanext = 0, minnext = 0;
                    U32 f = (flags & SCF_TRIE_DOING_RESTUDY);
                    SSize_t fake_last_close = 0;
                    regnode *fake_last_close_op = NULL;
                    regnode_ssc this_class;

                    StructCopy(&zero_scan_data, &data_fake, scan_data_t);
                    if (data) {
                        data_fake.whilem_c = data->whilem_c;
                        data_fake.last_closep = data->last_closep;
                        data_fake.last_close_opp = data->last_close_opp;
                    }
                    else {
                        data_fake.last_closep = &fake_last_close;
                        data_fake.last_close_opp = &fake_last_close_op;
                    }
                    data_fake.pos_delta = delta;
                    if (flags & SCF_DO_STCLASS) {
                        ssc_init(pRExC_state, &this_class);
                        data_fake.start_class = &this_class;
                        f |= SCF_DO_STCLASS_AND;
                    }
                    if (flags & SCF_WHILEM_VISITED_POS)
                        f |= SCF_WHILEM_VISITED_POS;

                    if (trie->jump[word]) {
                        if (!nextbranch)
                            nextbranch = trie_node + trie->jump[0];
                        scan= trie_node + trie->jump[word];
                        /* We go from the jump point to the branch that follows
                           it. Note this means we need the vestigal unused
                           branches even though they arent otherwise used. */
                        /* optimise study_chunk() for TRIE */
                        minnext = study_chunk(pRExC_state, &scan, minlenp,
                            &deltanext, (regnode *)nextbranch, &data_fake,
                            stopparen, recursed_depth, NULL, f, depth+1,
                            mutate_ok);
                    }
                    if (nextbranch && REGNODE_TYPE(OP(nextbranch))==BRANCH)
                        nextbranch= regnext((regnode*)nextbranch);

                    if (min1 > (SSize_t)(minnext + trie->minlen))
                        min1 = minnext + trie->minlen;
                    if (deltanext == OPTIMIZE_INFTY) {
                        is_inf = is_inf_internal = 1;
                        max1 = OPTIMIZE_INFTY;
                    } else if (max1 < (SSize_t)(minnext + deltanext + trie->maxlen))
                        max1 = minnext + deltanext + trie->maxlen;

                    if (data_fake.flags & (SF_HAS_PAR|SF_IN_PAR))
                        pars++;
                    if (data_fake.flags & SCF_SEEN_ACCEPT) {
                        if ( stopmin > min + min1)
                            stopmin = min + min1;
                        flags &= ~SCF_DO_SUBSTR;
                        if (data)
                            data->flags |= SCF_SEEN_ACCEPT;
                    }
                    if (data) {
                        if (data_fake.flags & SF_HAS_EVAL)
                            data->flags |= SF_HAS_EVAL;
                        data->whilem_c = data_fake.whilem_c;
                    }
                    if (flags & SCF_DO_STCLASS)
                        ssc_or(pRExC_state, &accum, (regnode_charclass *) &this_class);
                }
                DEBUG_STUDYDATA("after JUMPTRIE", data, depth, is_inf, min, stopmin, delta);
            }
            if (flags & SCF_DO_SUBSTR) {
                data->pos_min += min1;
                data->pos_delta += max1 - min1;
                if (max1 != min1 || is_inf)
                    data->cur_is_floating = 1; /* float */
            }
            min += min1;
            if (delta != OPTIMIZE_INFTY) {
                if (OPTIMIZE_INFTY - (max1 - min1) >= delta)
                    delta += max1 - min1;
                else
                    delta = OPTIMIZE_INFTY;
            }
            if (flags & SCF_DO_STCLASS_OR) {
                ssc_or(pRExC_state, data->start_class, (regnode_charclass *) &accum);
                if (min1) {
                    ssc_and(pRExC_state, data->start_class, (regnode_charclass *) and_withp);
                    flags &= ~SCF_DO_STCLASS;
                }
            }
            else if (flags & SCF_DO_STCLASS_AND) {
                if (min1) {
                    ssc_and(pRExC_state, data->start_class, (regnode_charclass *) &accum);
                    flags &= ~SCF_DO_STCLASS;
                }
                else {
                    /* Switch to OR mode: cache the old value of
                     * data->start_class */
                    INIT_AND_WITHP;
                    StructCopy(data->start_class, and_withp, regnode_ssc);
                    flags &= ~SCF_DO_STCLASS_AND;
                    StructCopy(&accum, data->start_class, regnode_ssc);
                    flags |= SCF_DO_STCLASS_OR;
                }
            }
            scan= tail;
            DEBUG_STUDYDATA("after TRIE study", data, depth, is_inf, min, stopmin, delta);
            continue;
        }
#else
        else if (REGNODE_TYPE(OP(scan)) == TRIE) {
            reg_trie_data *trie = (reg_trie_data*)RExC_rxi->data->data[ ARG1u(scan) ];
            U8*bang=NULL;

            min += trie->minlen;
            delta += (trie->maxlen - trie->minlen);
            flags &= ~SCF_DO_STCLASS; /* xxx */
            if (flags & SCF_DO_SUBSTR) {
                /* Cannot expect anything... */
                scan_commit(pRExC_state, data, minlenp, is_inf);
                data->pos_min += trie->minlen;
                data->pos_delta += (trie->maxlen - trie->minlen);
                if (trie->maxlen != trie->minlen)
                    data->cur_is_floating = 1; /* float */
            }
            if (trie->jump) /* no more substrings -- for now /grr*/
               flags &= ~SCF_DO_SUBSTR;
        }

#endif /* old or new */
#endif /* TRIE_STUDY_OPT */

        else if (OP(scan) == REGEX_SET) {
            Perl_croak(aTHX_ "panic: %s regnode should be resolved"
                             " before optimization", REGNODE_NAME(REGEX_SET));
        }

        /* Else: zero-length, ignore. */
        scan = regnext(scan);
    }

  finish:
    if (frame) {
        /* we need to unwind recursion. */
        depth = depth - 1;

        DEBUG_STUDYDATA("frame-end", data, depth, is_inf, min, stopmin, delta);
        DEBUG_PEEP("fend", scan, depth, flags);

        /* restore previous context */
        last = frame->last_regnode;
        scan = frame->next_regnode;
        stopparen = frame->stopparen;
        recursed_depth = frame->prev_recursed_depth;

        RExC_frame_last = frame->prev_frame;
        frame = frame->this_prev_frame;
        goto fake_study_recurse;
    }

    assert(!frame);
    DEBUG_STUDYDATA("pre-fin", data, depth, is_inf, min, stopmin, delta);

    /* is this pattern infinite? Eg, consider /(a|b+)/ */
    if (is_inf_internal)
        delta = OPTIMIZE_INFTY;

    /* deal with (*ACCEPT), Eg, consider /(foo(*ACCEPT)|bop)bar/ */
    if (min > stopmin) {
        /*
        At this point 'min' represents the minimum length string we can
        match while *ignoring* the implication of ACCEPT, and 'delta'
        represents the difference between the minimum length and maximum
        length, and if the pattern matches an infinitely long string
        (consider the + and * quantifiers) then we use the special delta
        value of OPTIMIZE_INFTY to represent it. 'stopmin' is the
        minimum length that can be matched *and* accepted.

        A pattern is accepted when matching was successful *and*
        complete, and thus there is no further matching needing to be
        done, no backtracking to occur, etc. Prior to the introduction
        of ACCEPT the only opcode that signaled acceptance was the END
        opcode, which is always the very last opcode in a regex program.
        ACCEPT is thus conceptually an early successful return out of
        the matching process. stopmin starts out as OPTIMIZE_INFTY to
        represent "the entire pattern", and is ratched down to the
        "current min" if necessary when an ACCEPT opcode is encountered.

        Thus stopmin might be smaller than min if we saw an (*ACCEPT),
        and we now need to account for it in both min and delta.
        Consider that in a pattern /AB/ normally the min length it can
        match can be computed as min(A)+min(B). But (*ACCEPT) means
        that it might be something else, not even neccesarily min(A) at
        all. Consider

             A  = /(foo(*ACCEPT)|x+)/
             B  = /whop/
             AB = /(foo(*ACCEPT)|x+)whop/

        The min for A is 1 for "x" and the delta for A is OPTIMIZE_INFTY
        for "xxxxx...", its stopmin is 3 for "foo". The min for B is 4 for
        "whop", and the delta of 0 as the pattern is of fixed length, the
        stopmin would be OPTIMIZE_INFTY as it does not contain an ACCEPT.
        When handling AB we expect to see a min of 5 for "xwhop", and a
        delta of OPTIMIZE_INFTY for "xxxxx...whop", and a stopmin of 3
        for "foo". This should result in a final min of 3 for "foo", and
        a final delta of OPTIMIZE_INFTY for "xxxxx...whop".

        In something like /(dude(*ACCEPT)|irk)x{3,7}/ we would have a
        min of 6 for "irkxxx" and a delta of 4 for "irkxxxxxxx", and the
        stop min would be 4 for "dude". This should result in a final
        min of 4 for "dude", and a final delta of 6, for "irkxxxxxxx".

        When min is smaller than stopmin then we can ignore it. In the
        fragment /(x{10,20}(*ACCEPT)|a)b+/, we would have a min of 2,
        and a delta of OPTIMIZE_INFTY, and a stopmin of 10. Obviously
        the ACCEPT doesn't reduce the minimum length of the string that
        might be matched, nor affect the maximum length.

        In something like /foo(*ACCEPT)ba?r/ we would have a min of 5
        for "foobr", a delta of 1 for "foobar", and a stopmin of 3 for
        "foo". We currently turn this into a min of 3 for "foo" and a
        delta of 3 for "foobar" even though technically "foobar" isn't
        possible. ACCEPT affects some aspects of the optimizer, like
        length computations and mandatory substring optimizations, but
        there are other optimzations this routine perfoms that are not
        affected and this compromise simplifies implementation.

        It might be helpful to consider that this C function is called
        recursively on the pattern in a bottom up fashion, and that the
        min returned by a nested call may be marked as coming from an
        ACCEPT, causing its callers to treat the returned min as a
        stopmin as the recursion unwinds. Thus a single ACCEPT can affect
        multiple calls into this function in different ways.
        */

        if (OPTIMIZE_INFTY - delta >= min - stopmin)
            delta += min - stopmin;
        else
            delta = OPTIMIZE_INFTY;
        min = stopmin;
    }

    *scanp = scan;
    *deltap = delta;

    if (flags & SCF_DO_SUBSTR && is_inf)
        data->pos_delta = OPTIMIZE_INFTY - data->pos_min;
    if (is_par > (I32)U8_MAX)
        is_par = 0;
    if (is_par && pars==1 && data) {
        data->flags |= SF_IN_PAR;
        data->flags &= ~SF_HAS_PAR;
    }
    else if (pars && data) {
        data->flags |= SF_HAS_PAR;
        data->flags &= ~SF_IN_PAR;
    }
    if (flags & SCF_DO_STCLASS_OR)
        ssc_and(pRExC_state, data->start_class, (regnode_charclass *) and_withp);
    if (flags & SCF_TRIE_RESTUDY)
        data->flags |=  SCF_TRIE_RESTUDY;


    if (!(RExC_seen & REG_UNBOUNDED_QUANTIFIER_SEEN)) {
        if (min > OPTIMIZE_INFTY - delta)
            RExC_maxlen = OPTIMIZE_INFTY;
        else if (RExC_maxlen < min + delta)
            RExC_maxlen = min + delta;
    }
    DEBUG_STUDYDATA("post-fin", data, depth, is_inf, min, stopmin, delta);
    return min;
}
