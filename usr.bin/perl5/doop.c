/*    doop.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *  'So that was the job I felt I had to do when I started,' thought Sam.
 *
 *     [p.934 of _The Lord of the Rings_, VI/iii: "Mount Doom"]
 */

/* This file contains some common functions needed to carry out certain
 * ops. For example, both pp_sprintf() and pp_prtf() call the function
 * do_sprintf() found in this file.
 */

#include "EXTERN.h"
#define PERL_IN_DOOP_C
#include "perl.h"
#include "invlist_inline.h"

#ifndef PERL_MICRO
#include <signal.h>
#endif


/* Helper function for do_trans().
 * Handles cases where the search and replacement charlists aren't UTF-8,
 * aren't identical, and neither the /d nor /s flag is present.
 *
 * sv may or may not be utf8.  Note that no code point above 255 can possibly
 * be in the to-translate set
 */

STATIC Size_t
S_do_trans_simple(pTHX_ SV * const sv, const OPtrans_map * const tbl)
{
    Size_t matches = 0;
    STRLEN len;
    U8 *s = (U8*)SvPV_nomg(sv,len);
    U8 * const send = s+len;

    PERL_ARGS_ASSERT_DO_TRANS_SIMPLE;
    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: entering do_trans_simple:"
                                          " input sv:\n",
                                          __FILE__, __LINE__));
    DEBUG_y(sv_dump(sv));

    /* First, take care of non-UTF-8 input strings, because they're easy */
    if (!SvUTF8(sv)) {
        while (s < send) {
            const short ch = tbl->map[*s];
            if (ch >= 0) {
                matches++;
                *s = (U8)ch;
            }
            s++;
        }
        SvSETMAGIC(sv);
    }
    else {
        const bool grows = cBOOL(PL_op->op_private & OPpTRANS_GROWS);
        U8 *d;
        U8 *dstart;

        /* Allow for worst-case expansion: Each input byte can become 2.  For a
         * given input character, this happens when it occupies a single byte
         * under UTF-8, but is to be translated to something that occupies two:
         * $_="a".chr(400); tr/a/\xFE/, FE needs encoding. */
        if (grows)
            Newx(d, len*2+1, U8);
        else
            d = s;
        dstart = d;
        while (s < send) {
            STRLEN ulen;
            short ch;

            /* Need to check this, otherwise 128..255 won't match */
            const UV c = utf8n_to_uvchr(s, send - s, &ulen, UTF8_ALLOW_DEFAULT);
            if (c < 0x100 && (ch = tbl->map[c]) >= 0) {
                matches++;
                d = uvchr_to_utf8(d, (UV)ch);
                s += ulen;
            }
            else { /* No match -> copy */
                Move(s, d, ulen, U8);
                d += ulen;
                s += ulen;
            }
        }
        if (grows) {
            sv_setpvn(sv, (char*)dstart, d - dstart);
            Safefree(dstart);
        }
        else {
            *d = '\0';
            SvCUR_set(sv, d - dstart);
        }
        SvUTF8_on(sv);
        SvSETMAGIC(sv);
    }
    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: returning %zu\n",
                                          __FILE__, __LINE__, matches));
    DEBUG_y(sv_dump(sv));
    return matches;
}


/* Helper function for do_trans().
 * Handles cases where the search and replacement charlists are identical and
 * non-utf8: so the string isn't modified, and only a count of modifiable
 * chars is needed.
 *
 * Note that it doesn't handle /d or /s, since these modify the string even if
 * the replacement list is empty.
 *
 * sv may or may not be utf8.  Note that no code point above 255 can possibly
 * be in the to-translate set
 */

STATIC Size_t
S_do_trans_count(pTHX_ SV * const sv, const OPtrans_map * const tbl)
{
    STRLEN len;
    const U8 *s = (const U8*)SvPV_nomg_const(sv, len);
    const U8 * const send = s + len;
    Size_t matches = 0;

    PERL_ARGS_ASSERT_DO_TRANS_COUNT;

    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: entering do_trans_count:"
                                          " input sv:\n",
                                          __FILE__, __LINE__));
    DEBUG_y(sv_dump(sv));

    if (!SvUTF8(sv)) {
        while (s < send) {
            if (tbl->map[*s++] >= 0)
                matches++;
        }
    }
    else {
        const bool complement = cBOOL(PL_op->op_private & OPpTRANS_COMPLEMENT);
        while (s < send) {
            STRLEN ulen;
            const UV c = utf8n_to_uvchr(s, send - s, &ulen, UTF8_ALLOW_DEFAULT);
            if (c < 0x100) {
                if (tbl->map[c] >= 0)
                    matches++;
            } else if (complement)
                matches++;
            s += ulen;
        }
    }

    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: count returning %zu\n",
                                          __FILE__, __LINE__, matches));
    return matches;
}


/* Helper function for do_trans().
 * Handles cases where the search and replacement charlists aren't identical
 * and both are non-utf8, and one or both of /d, /s is specified.
 *
 * sv may or may not be utf8.  Note that no code point above 255 can possibly
 * be in the to-translate set
 */

STATIC Size_t
S_do_trans_complex(pTHX_ SV * const sv, const OPtrans_map * const tbl)
{
    STRLEN len;
    U8 *s = (U8*)SvPV_nomg(sv, len);
    U8 * const send = s+len;
    Size_t matches = 0;
    const bool complement = cBOOL(PL_op->op_private & OPpTRANS_COMPLEMENT);

    PERL_ARGS_ASSERT_DO_TRANS_COMPLEX;

    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: entering do_trans_complex:"
                                          " input sv:\n",
                                          __FILE__, __LINE__));
    DEBUG_y(sv_dump(sv));

    if (!SvUTF8(sv)) {
        U8 *d = s;
        U8 * const dstart = d;

        if (PL_op->op_private & OPpTRANS_SQUASH) {

            /* What the mapping of the previous character was to.  If the new
             * character has the same mapping, it is squashed from the output
             * (but still is included in the count) */
            short previous_map = (short) TR_OOB;

            while (s < send) {
                const short this_map = tbl->map[*s];
                if (this_map >= 0) {
                    matches++;
                    if (this_map != previous_map) {
                        *d++ = (U8)this_map;
                        previous_map = this_map;
                    }
                }
                else {
                    if (this_map == (short) TR_UNMAPPED) {
                        *d++ = *s;
                        previous_map = (short) TR_OOB;
                    }
                    else {
                        assert(this_map == (short) TR_DELETE);
                        matches++;
                    }
                }

                s++;
            }
        }
        else {  /* Not to squash */
            while (s < send) {
                const short this_map = tbl->map[*s];
                if (this_map >= 0) {
                    matches++;
                    *d++ = (U8)this_map;
                }
                else if (this_map == (short) TR_UNMAPPED)
                    *d++ = *s;
                else if (this_map == (short) TR_DELETE)
                    matches++;
                s++;
            }
        }
        *d = '\0';
        SvCUR_set(sv, d - dstart);
    }
    else { /* is utf8 */
        const bool squash = cBOOL(PL_op->op_private & OPpTRANS_SQUASH);
        const bool grows  = cBOOL(PL_op->op_private & OPpTRANS_GROWS);
        U8 *d;
        U8 *dstart;
        Size_t size = tbl->size;

        /* What the mapping of the previous character was to.  If the new
         * character has the same mapping, it is squashed from the output (but
         * still is included in the count) */
        UV pch = TR_OOB;

        if (grows)
            /* Allow for worst-case expansion: Each input byte can become 2.
             * For a given input character, this happens when it occupies a
             * single byte under UTF-8, but is to be translated to something
             * that occupies two: */
            Newx(d, len*2+1, U8);
        else
            d = s;
        dstart = d;

        while (s < send) {
            STRLEN len;
            const UV comp = utf8n_to_uvchr(s, send - s, &len,
                                           UTF8_ALLOW_DEFAULT);
            UV     ch;
            short sch;

            sch = (comp < size)
                  ? tbl->map[comp]
                  : (! complement)
                    ? (short) TR_UNMAPPED
                    : tbl->map[size];

            if (sch >= 0) {
                ch = (UV)sch;
              replace:
                matches++;
                if (LIKELY(!squash || ch != pch)) {
                    d = uvchr_to_utf8(d, ch);
                    pch = ch;
                }
                s += len;
                continue;
            }
            else if (sch == (short) TR_UNMAPPED) {
                Move(s, d, len, U8);
                d += len;
                pch = TR_OOB;
            }
            else if (sch == (short) TR_DELETE)
                matches++;
            else {
                assert(sch == (short) TR_R_EMPTY);  /* empty replacement */
                ch = comp;
                goto replace;
            }

            s += len;
        }

        if (grows) {
            sv_setpvn(sv, (char*)dstart, d - dstart);
            Safefree(dstart);
        }
        else {
            *d = '\0';
            SvCUR_set(sv, d - dstart);
        }
        SvUTF8_on(sv);
    }
    SvSETMAGIC(sv);
    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: returning %zu\n",
                                          __FILE__, __LINE__, matches));
    DEBUG_y(sv_dump(sv));
    return matches;
}


/* Helper function for do_trans().
 * Handles cases where an inversion map implementation is to be used and the
 * search and replacement charlists are identical: so the string isn't
 * modified, and only a count of modifiable chars is needed.
 *
 * Note that it doesn't handle /d nor /s, since these modify the string
 * even if the replacement charlist is empty.
 *
 * sv may or may not be utf8.
 */

STATIC Size_t
S_do_trans_count_invmap(pTHX_ SV * const sv, AV * const invmap)
{
    U8 *s;
    U8 *send;
    Size_t matches = 0;
    STRLEN len;
    SV** const from_invlist_ptr = av_fetch(invmap, 0, TRUE);
    SV** const to_invmap_ptr = av_fetch(invmap, 1, TRUE);
    SV* from_invlist = *from_invlist_ptr;
    SV* to_invmap_sv = *to_invmap_ptr;
    UV* map = (UV *) SvPVX(to_invmap_sv);

    PERL_ARGS_ASSERT_DO_TRANS_COUNT_INVMAP;

    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d:"
                                          "entering do_trans_count_invmap:"
                                          " input sv:\n",
                                          __FILE__, __LINE__));
    DEBUG_y(sv_dump(sv));
    DEBUG_y(PerlIO_printf(Perl_debug_log, "mapping:\n"));
    DEBUG_y(invmap_dump(from_invlist, (UV *) SvPVX(to_invmap_sv)));

    s = (U8*)SvPV_nomg(sv, len);

    send = s + len;

    while (s < send) {
        UV from;
        SSize_t i;
        STRLEN s_len;

        /* Get the code point of the next character in the string */
        if (! SvUTF8(sv) || UTF8_IS_INVARIANT(*s)) {
            from = *s;
            s_len = 1;
        }
        else {
            from = utf8_to_uvchr_buf(s, send, &s_len);
            if (from == 0 && *s != '\0') {
                _force_out_malformed_utf8_message(s, send, 0, /*die*/TRUE);
            }
        }

        /* Look the code point up in the data structure for this tr/// to get
         * what it maps to */
        i = _invlist_search(from_invlist, from);
        assert(i >= 0);

        if (map[i] != (UV) TR_UNLISTED) {
            matches++;
        }

        s += s_len;
    }

    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: returning %zu\n",
                                          __FILE__, __LINE__, matches));
    return matches;
}

/* Helper function for do_trans().
 * Handles cases where an inversion map implementation is to be used and the
 * search and replacement charlists are either not identical or flags are
 * present.
 *
 * sv may or may not be utf8.
 */

STATIC Size_t
S_do_trans_invmap(pTHX_ SV * const sv, AV * const invmap)
{
    U8 *s;
    U8 *send;
    U8 *d;
    U8 *s0;
    U8 *d0;
    Size_t matches = 0;
    STRLEN len;
    SV** const from_invlist_ptr = av_fetch(invmap, 0, TRUE);
    SV** const to_invmap_ptr = av_fetch(invmap, 1, TRUE);
    SV** const to_expansion_ptr = av_fetch(invmap, 2, TRUE);
    NV max_expansion = SvNV(*to_expansion_ptr);
    SV* from_invlist = *from_invlist_ptr;
    SV* to_invmap_sv = *to_invmap_ptr;
    UV* map = (UV *) SvPVX(to_invmap_sv);
    UV previous_map = TR_OOB;
    const bool squash         = cBOOL(PL_op->op_private & OPpTRANS_SQUASH);
    const bool delete_unfound = cBOOL(PL_op->op_private & OPpTRANS_DELETE);
    bool inplace = ! cBOOL(PL_op->op_private & OPpTRANS_GROWS);
    const UV* from_array = invlist_array(from_invlist);
    UV final_map = TR_OOB;
    bool out_is_utf8 = cBOOL(SvUTF8(sv));
    STRLEN s_len;

    PERL_ARGS_ASSERT_DO_TRANS_INVMAP;

    /* A third element in the array indicates that the replacement list was
     * shorter than the search list, and this element contains the value to use
     * for the items that don't correspond */
    if (av_top_index(invmap) >= 3) {
        SV** const final_map_ptr = av_fetch(invmap, 3, TRUE);
        SV*  const final_map_sv = *final_map_ptr;
        final_map = SvUV(final_map_sv);
    }

    /* If there is something in the transliteration that could force the input
     * to be changed to UTF-8, we don't know if we can do it in place, so
     * assume cannot */
    if (! out_is_utf8 && (PL_op->op_private & OPpTRANS_CAN_FORCE_UTF8)) {
        inplace = FALSE;
    }

    s = (U8*)SvPV_nomg(sv, len);
    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: entering do_trans_invmap:"
                                          " input sv:\n",
                                          __FILE__, __LINE__));
    DEBUG_y(sv_dump(sv));
    DEBUG_y(PerlIO_printf(Perl_debug_log, "mapping:\n"));
    DEBUG_y(invmap_dump(from_invlist, map));

    send = s + len;
    s0 = s;

    /* We know by now if there are some possible input strings whose
     * transliterations are longer than the input.  If none can, we just edit
     * in place. */
    if (inplace) {
        d0 = d = s;
    }
    else {
        /* Here, we can't edit in place.  We have no idea how much, if any,
         * this particular input string will grow.  However, the compilation
         * calculated the maximum expansion possible.  Use that to allocate
         * based on the worst case scenario.  (First +1 is to round up; 2nd is
         * for \0) */
        Newx(d, (STRLEN) (len * max_expansion + 1 + 1), U8);
        d0 = d;
    }

  restart:

    /* Do the actual transliteration */
    while (s < send) {
        UV from;
        UV to;
        SSize_t i;
        STRLEN s_len;

        /* Get the code point of the next character in the string */
        if (! SvUTF8(sv) || UTF8_IS_INVARIANT(*s)) {
            from = *s;
            s_len = 1;
        }
        else {
            from = utf8_to_uvchr_buf(s, send, &s_len);
            if (from == 0 && *s != '\0') {
                _force_out_malformed_utf8_message(s, send, 0, /*die*/TRUE);
            }
        }

        /* Look the code point up in the data structure for this tr/// to get
         * what it maps to */
        i = _invlist_search(from_invlist, from);
        assert(i >= 0);

        to = map[i];

        if (to == (UV) TR_UNLISTED) { /* Just copy the unreplaced character */
            if (UVCHR_IS_INVARIANT(from) || ! out_is_utf8) {
                *d++ = (U8) from;
            }
            else if (SvUTF8(sv)) {
                Move(s, d, s_len, U8);
                d += s_len;
            }
            else {  /* Convert to UTF-8 */
                append_utf8_from_native_byte(*s, &d);
            }

            previous_map = to;
            s += s_len;
            continue;
        }

        /* Everything else is counted as a match */
        matches++;

        if (to == (UV) TR_SPECIAL_HANDLING) {
            if (delete_unfound) {
                s += s_len;
                continue;
            }

            /* Use the final character in the replacement list */
            to = final_map;
        }
        else { /* Here the input code point is to be remapped.  The actual
                  value is offset from the base of this entry */
            to += from - from_array[i];
        }

        /* If copying all occurrences, or this is the first occurrence, copy it
         * to the output */
        if (! squash || to != previous_map) {
            if (out_is_utf8) {
                d = uvchr_to_utf8(d, to);
            }
            else {
                if (to >= 256) {    /* If need to convert to UTF-8, restart */
                    out_is_utf8 = TRUE;
                    s = s0;
                    d = d0;
                    matches = 0;
                    goto restart;
                }
                *d++ = (U8) to;
            }
        }

        previous_map = to;
        s += s_len;
    }

    s_len = 0;
    s += s_len;
    if (! inplace) {
        sv_setpvn(sv, (char*)d0, d - d0);
        Safefree(d0);
    }
    else {
        *d = '\0';
        SvCUR_set(sv, d - d0);
    }

    if (! SvUTF8(sv) && out_is_utf8) {
        SvUTF8_on(sv);
    }
    SvSETMAGIC(sv);

    DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d: returning %zu\n",
                                          __FILE__, __LINE__, matches));
    DEBUG_y(sv_dump(sv));
    return matches;
}

/* Execute a tr//. sv is the value to be translated, while PL_op
 * should be an OP_TRANS or OP_TRANSR op, whose op_pv field contains a
 * translation table or whose op_sv field contains an inversion map.
 *
 * Returns a count of number of characters translated
 */

Size_t
Perl_do_trans(pTHX_ SV *sv)
{
    STRLEN len;
    const U8 flags = PL_op->op_private;
    bool use_utf8_fcns = cBOOL(flags & OPpTRANS_USE_SVOP);
    bool identical     = cBOOL(flags & OPpTRANS_IDENTICAL);

    PERL_ARGS_ASSERT_DO_TRANS;

    if (SvREADONLY(sv) && ! identical) {
        Perl_croak_no_modify();
    }
    (void)SvPV_const(sv, len);
    if (!len)
        return 0;
    if (! identical) {
        if (!SvPOKp(sv) || SvTHINKFIRST(sv))
            (void)SvPV_force_nomg(sv, len);
        (void)SvPOK_only_UTF8(sv);
    }

    if (use_utf8_fcns) {
        SV* const map =
#ifdef USE_ITHREADS
                        PAD_SVl(cPADOP->op_padix);
#else
                        MUTABLE_SV(cSVOP->op_sv);
#endif

        if (identical) {
            return do_trans_count_invmap(sv, (AV *) map);
        }
        else {
            return do_trans_invmap(sv, (AV *) map);
        }
    }
    else {
        const OPtrans_map * const map = (OPtrans_map*)cPVOP->op_pv;

        if (identical) {
            return do_trans_count(sv, map);
        }
        else if (flags & (OPpTRANS_SQUASH|OPpTRANS_DELETE|OPpTRANS_COMPLEMENT)) {
            return do_trans_complex(sv, map);
        }
        else
            return do_trans_simple(sv, map);
    }
}

/*
=for apidoc_section $string
=for apidoc do_join

This performs a Perl L<C<join>|perlfunc/join>, placing the joined output
into C<sv>.

The elements to join are in SVs, stored in a C array of pointers to SVs, from
C<**mark> to S<C<**sp - 1>>.  Hence C<*mark> is a reference to the first SV.
Each SV will be coerced into a PV if not one already.

C<delim> contains the string (or coerced into a string) that is to separate
each of the joined elements.

If any component is in UTF-8, the result will be as well, and all non-UTF-8
components will be converted to UTF-8 as necessary.

Magic and tainting are handled.

=cut
*/

void
Perl_do_join(pTHX_ SV *sv, SV *delim, SV **mark, SV **sp)
{
    SV ** const oldmark = mark;
    I32 items = sp - mark;
    STRLEN len;
    STRLEN delimlen;
    const char * const delims = SvPV_const(delim, delimlen);

    PERL_ARGS_ASSERT_DO_JOIN;

    mark++;
    len = (items > 0 ? (delimlen * (items - 1) ) : 0);
    SvUPGRADE(sv, SVt_PV);
    if (SvLEN(sv) < len + items) {	/* current length is way too short */
        while (items-- > 0) {
            if (*mark && !SvGAMAGIC(*mark) && SvOK(*mark)) {
                STRLEN tmplen;
                SvPV_const(*mark, tmplen);
                len += tmplen;
            }
            mark++;
        }
        SvGROW(sv, len + 1);		/* so try to pre-extend */

        mark = oldmark;
        items = sp - mark;
        ++mark;
    }

    SvPVCLEAR(sv);
    /* sv_setpv retains old UTF8ness [perl #24846] */
    SvUTF8_off(sv);

    if (TAINTING_get && SvMAGICAL(sv))
        SvTAINTED_off(sv);

    if (items-- > 0) {
        if (*mark)
            sv_catsv(sv, *mark);
        mark++;
    }

    if (delimlen) {
        const U32 delimflag = DO_UTF8(delim) ? SV_CATUTF8 : SV_CATBYTES;
        for (; items > 0; items--,mark++) {
            STRLEN len;
            const char *s;
            sv_catpvn_flags(sv,delims,delimlen,delimflag);
            s = SvPV_const(*mark,len);
            sv_catpvn_flags(sv,s,len,
                            DO_UTF8(*mark) ? SV_CATUTF8 : SV_CATBYTES);
        }
    }
    else {
        for (; items > 0; items--,mark++)
        {
            STRLEN len;
            const char *s = SvPV_const(*mark,len);
            sv_catpvn_flags(sv,s,len,
                            DO_UTF8(*mark) ? SV_CATUTF8 : SV_CATBYTES);
        }
    }
    SvSETMAGIC(sv);
}

/*
=for apidoc_section $string
=for apidoc do_sprintf

This performs a Perl L<C<sprintf>|perlfunc/sprintf> placing the string output
into C<sv>.

The elements to format are in SVs, stored in a C array of pointers to SVs of
length C<len>> and beginning at C<**sarg>.  The element referenced by C<*sarg>
is the format.

Magic and tainting are handled.

=cut
*/

void
Perl_do_sprintf(pTHX_ SV *sv, SSize_t len, SV **sarg)
{
    STRLEN patlen;
    const char * const pat = SvPV_const(*sarg, patlen);
    bool do_taint = FALSE;

    PERL_ARGS_ASSERT_DO_SPRINTF;
    assert(len >= 1);

    if (SvTAINTED(*sarg))
        TAINT_PROPER(
                (PL_op && PL_op->op_type < OP_max)
                    ? (PL_op->op_type == OP_PRTF)
                        ? "printf"
                        : PL_op_name[PL_op->op_type]
                    : "(unknown)"
        );
    SvUTF8_off(sv);
    if (DO_UTF8(*sarg))
        SvUTF8_on(sv);
    sv_vsetpvfn(sv, pat, patlen, NULL, sarg + 1, (Size_t)(len - 1), &do_taint);
    SvSETMAGIC(sv);
    if (do_taint)
        SvTAINTED_on(sv);
}

UV
Perl_do_vecget(pTHX_ SV *sv, STRLEN offset, int size)
{
    STRLEN srclen;
    const I32 svpv_flags = ((PL_op->op_flags & OPf_MOD || LVRET)
                                          ? SV_UNDEF_RETURNS_NULL : 0);
    unsigned char *s = (unsigned char *)
                            SvPV_flags(sv, srclen, (svpv_flags|SV_GMAGIC));
    UV retnum = 0;

    if (!s) {
      s = (unsigned char *)"";
    }

    PERL_ARGS_ASSERT_DO_VECGET;

    if (size < 1 || ! isPOWER_OF_2(size))
        Perl_croak(aTHX_ "Illegal number of bits in vec");

    if (SvUTF8(sv)) {
        if (Perl_sv_utf8_downgrade_flags(aTHX_ sv, TRUE, 0)) {
            /* PVX may have changed */
            s = (unsigned char *) SvPV_flags(sv, srclen, svpv_flags);
        }
        else {
            Perl_croak(aTHX_ "Use of strings with code points over 0xFF"
                             " as arguments to vec is forbidden");
        }
    }

    if (size <= 8) {
        STRLEN bitoffs = ((offset % 8) * size) % 8;
        STRLEN uoffset = offset / (8 / size);

        if (uoffset >= srclen)
            return 0;

        retnum = (s[uoffset] >> bitoffs) & nBIT_MASK(size);
    }
    else {
        int n = size / 8;            /* required number of bytes */
        SSize_t uoffset;

#ifdef UV_IS_QUAD

        if (size == 64) {
            Perl_ck_warner(aTHX_ packWARN(WARN_PORTABLE),
                           "Bit vector size > 32 non-portable");
        }
#endif
        if (offset > Size_t_MAX / n - 1) /* would overflow */
            return 0;

        uoffset = offset * n;

        /* Switch on the number of bytes available, but no more than the number
         * required */
        switch (MIN(n, (SSize_t) srclen - uoffset)) {

#ifdef UV_IS_QUAD

          case 8:
            retnum += ((UV) s[uoffset + 7]);
            /* FALLTHROUGH */
          case 7:
            retnum += ((UV) s[uoffset + 6] <<  8);  /* = size - 56 */
            /* FALLTHROUGH */
          case 6:
            retnum += ((UV) s[uoffset + 5] << 16);  /* = size - 48 */
            /* FALLTHROUGH */
          case 5:
            retnum += ((UV) s[uoffset + 4] << 24);  /* = size - 40 */
#endif
            /* FALLTHROUGH */
          case 4:
            retnum += ((UV) s[uoffset + 3] << (size - 32));
            /* FALLTHROUGH */
          case 3:
            retnum += ((UV) s[uoffset + 2] << (size - 24));
            /* FALLTHROUGH */
          case 2:
            retnum += ((UV) s[uoffset + 1] << (size - 16));
            /* FALLTHROUGH */
          case 1:
            retnum += ((UV) s[uoffset    ] << (size - 8));
            break;

          default:
            return 0;
        }
    }

    return retnum;
}

/* currently converts input to bytes if possible but doesn't sweat failures,
 * although it does ensure that the string it clobbers is not marked as
 * utf8-valid any more
 */
void
Perl_do_vecset(pTHX_ SV *sv)
{
    STRLEN offset, bitoffs = 0;
    int size;
    unsigned char *s;
    UV lval;
    I32 mask;
    STRLEN targlen;
    STRLEN len;
    SV * const targ = LvTARG(sv);
    char errflags = LvFLAGS(sv);

    PERL_ARGS_ASSERT_DO_VECSET;

    /* some out-of-range errors have been deferred if/until the LV is
     * actually written to: f(vec($s,-1,8)) is not always fatal */
    if (errflags) {
        assert(!(errflags & ~(LVf_NEG_OFF|LVf_OUT_OF_RANGE)));
        if (errflags & LVf_NEG_OFF)
            Perl_croak_nocontext("Negative offset to vec in lvalue context");
        Perl_croak_nocontext("Out of memory!");
    }

    if (!targ)
        return;
    s = (unsigned char*)SvPV_force_flags(targ, targlen,
                                         SV_GMAGIC | SV_UNDEF_RETURNS_NULL);
    if (SvUTF8(targ)) {
        /* This is handled by the SvPOK_only below...
        if (!Perl_sv_utf8_downgrade_flags(aTHX_ targ, TRUE, 0))
            SvUTF8_off(targ);
         */
        (void) Perl_sv_utf8_downgrade_flags(aTHX_ targ, TRUE, 0);
    }

    (void)SvPOK_only(targ);
    lval = SvUV(sv);
    offset = LvTARGOFF(sv);
    size = LvTARGLEN(sv);

    if (size < 1 || (size & (size-1))) /* size < 1 or not a power of two */
        Perl_croak(aTHX_ "Illegal number of bits in vec");

    if (size < 8) {
        bitoffs = ((offset%8)*size)%8;
        offset /= 8/size;
    }
    else if (size > 8) {
        int n = size/8;
        if (offset > Size_t_MAX / n - 1) /* would overflow */
            Perl_croak_nocontext("Out of memory!");
        offset *= n;
    }

    len = (bitoffs + size + 7)/8;	/* required number of bytes */
    if (targlen < offset || targlen - offset < len) {
        STRLEN newlen = offset > Size_t_MAX - len - 1 ? /* avoid overflow */
                                        Size_t_MAX : offset + len + 1;
        s = (unsigned char*)SvGROW(targ, newlen);
        (void)memzero((char *)(s + targlen), newlen - targlen);
        SvCUR_set(targ, newlen - 1);
    }

    if (size < 8) {
        mask = nBIT_MASK(size);
        lval &= mask;
        s[offset] &= ~(mask << bitoffs);
        s[offset] |= lval << bitoffs;
    }
    else switch (size) {

#ifdef UV_IS_QUAD

      case 64:
        Perl_ck_warner(aTHX_ packWARN(WARN_PORTABLE),
                       "Bit vector size > 32 non-portable");
        s[offset+7] = (U8)( lval      );    /* = size - 64 */
        s[offset+6] = (U8)( lval >>  8);    /* = size - 56 */
        s[offset+5] = (U8)( lval >> 16);    /* = size - 48 */
        s[offset+4] = (U8)( lval >> 24);    /* = size - 40 */
#endif
        /* FALLTHROUGH */
      case 32:
        s[offset+3] = (U8)( lval >> (size - 32));
        s[offset+2] = (U8)( lval >> (size - 24));
        /* FALLTHROUGH */
      case 16:
        s[offset+1] = (U8)( lval >> (size - 16));
        /* FALLTHROUGH */
      case 8:
        s[offset  ] = (U8)( lval >> (size - 8));
    }
    SvSETMAGIC(targ);
}

void
Perl_do_vop(pTHX_ I32 optype, SV *sv, SV *left, SV *right)
{
    long *dl;
    long *ll;
    long *rl;
    char *dc;
    STRLEN leftlen;
    STRLEN rightlen;
    const char *lc;
    const char *rc;
    STRLEN len = 0;
    STRLEN lensave;
    const char *lsave;
    const char *rsave;
    STRLEN needlen = 0;
    bool result_needs_to_be_utf8 = FALSE;
    bool left_utf8 = FALSE;
    bool right_utf8 = FALSE;
    U8 * left_non_downgraded = NULL;
    U8 * right_non_downgraded = NULL;
    Size_t left_non_downgraded_len = 0;
    Size_t right_non_downgraded_len = 0;
    char * non_downgraded = NULL;
    Size_t non_downgraded_len = 0;

    PERL_ARGS_ASSERT_DO_VOP;

    if (sv != left || (optype != OP_BIT_AND && !SvOK(sv)))
        SvPVCLEAR(sv);        /* avoid undef warning on |= and ^= */
    if (sv == left) {
        lc = SvPV_force_nomg(left, leftlen);
    }
    else {
        lc = SvPV_nomg_const(left, leftlen);
        SvPV_force_nomg_nolen(sv);
    }
    rc = SvPV_nomg_const(right, rightlen);

    /* This needs to come after SvPV to ensure that string overloading has
       fired off.  */

    /* Create downgraded temporaries of any UTF-8 encoded operands */
    if (DO_UTF8(left)) {
        const U8 * save_lc = (U8 *) lc;

        left_utf8 = TRUE;
        result_needs_to_be_utf8 = TRUE;

        left_non_downgraded_len = leftlen;
        lc = (char *) bytes_from_utf8_loc((const U8 *) lc, &leftlen,
                                          &left_utf8,
                                          (const U8 **) &left_non_downgraded);
        /* Calculate the number of trailing unconvertible bytes.  This quantity
         * is the original length minus the length of the converted portion. */
        left_non_downgraded_len -= left_non_downgraded - save_lc;
        SAVEFREEPV(lc);
    }
    if (DO_UTF8(right)) {
        const U8 * save_rc = (U8 *) rc;

        right_utf8 = TRUE;
        result_needs_to_be_utf8 = TRUE;

        right_non_downgraded_len = rightlen;
        rc = (char *) bytes_from_utf8_loc((const U8 *) rc, &rightlen,
                                          &right_utf8,
                                          (const U8 **) &right_non_downgraded);
        right_non_downgraded_len -= right_non_downgraded - save_rc;
        SAVEFREEPV(rc);
    }

    /* We set 'len' to the length that the operation actually operates on.  The
     * dangling part of the longer operand doesn't actually participate in the
     * operation.  What happens is that we pretend that the shorter operand has
     * been extended to the right by enough imaginary zeros to match the length
     * of the longer one.  But we know in advance the result of the operation
     * on zeros without having to do it.  In the case of '&', the result is
     * zero, and the dangling portion is simply discarded.  For '|' and '^', the
     * result is the same as the other operand, so the dangling part is just
     * appended to the final result, unchanged.  As of perl-5.32, we no longer
     * accept above-FF code points in the dangling portion.
     */
    if (left_utf8 || right_utf8) {
        Perl_croak(aTHX_ FATAL_ABOVE_FF_MSG, PL_op_desc[optype]);
    }
    else {  /* Neither is UTF-8 */
        len = MIN(leftlen, rightlen);
    }

    lensave = len;
    lsave = lc;
    rsave = rc;

    (void)SvPOK_only(sv);
    if (SvOK(sv) || SvTYPE(sv) > SVt_PVMG) {
        dc = SvPV_force_nomg_nolen(sv);
        if (SvLEN(sv) < len + 1) {
            dc = SvGROW(sv, len + 1);
            (void)memzero(dc + SvCUR(sv), len - SvCUR(sv) + 1);
        }
    }
    else {
        needlen = optype == OP_BIT_AND
                    ? len : (leftlen > rightlen ? leftlen : rightlen);
        Newxz(dc, needlen + 1, char);
        sv_usepvn_flags(sv, dc, needlen, SV_HAS_TRAILING_NUL);
        dc = SvPVX(sv);		/* sv_usepvn() calls Renew() */
    }
    SvCUR_set(sv, len);

    if (len >= sizeof(long)*4 &&
        !(PTR2nat(dc) % sizeof(long)) &&
        !(PTR2nat(lc) % sizeof(long)) &&
        !(PTR2nat(rc) % sizeof(long)))	/* It's almost always aligned... */
    {
        const STRLEN remainder = len % (sizeof(long)*4);
        len /= (sizeof(long)*4);

        dl = (long*)dc;
        ll = (long*)lc;
        rl = (long*)rc;

        switch (optype) {
        case OP_BIT_AND:
            while (len--) {
                *dl++ = *ll++ & *rl++;
                *dl++ = *ll++ & *rl++;
                *dl++ = *ll++ & *rl++;
                *dl++ = *ll++ & *rl++;
            }
            break;
        case OP_BIT_XOR:
            while (len--) {
                *dl++ = *ll++ ^ *rl++;
                *dl++ = *ll++ ^ *rl++;
                *dl++ = *ll++ ^ *rl++;
                *dl++ = *ll++ ^ *rl++;
            }
            break;
        case OP_BIT_OR:
            while (len--) {
                *dl++ = *ll++ | *rl++;
                *dl++ = *ll++ | *rl++;
                *dl++ = *ll++ | *rl++;
                *dl++ = *ll++ | *rl++;
            }
        }

        dc = (char*)dl;
        lc = (char*)ll;
        rc = (char*)rl;

        len = remainder;
    }

    switch (optype) {
    case OP_BIT_AND:
        while (len--)
            *dc++ = *lc++ & *rc++;
        *dc = '\0';
        break;
    case OP_BIT_XOR:
        while (len--)
            *dc++ = *lc++ ^ *rc++;
        goto mop_up;
    case OP_BIT_OR:
        while (len--)
            *dc++ = *lc++ | *rc++;
      mop_up:
        len = lensave;
        if (rightlen > len) {
            if (dc == rc)
                SvCUR_set(sv, rightlen);
            else
                sv_catpvn_nomg(sv, rsave + len, rightlen - len);
        }
        else if (leftlen > len) {
            if (dc == lc)
                SvCUR_set(sv, leftlen);
            else
                sv_catpvn_nomg(sv, lsave + len, leftlen - len);
        }
        *SvEND(sv) = '\0';

        /* If there is trailing stuff that couldn't be converted from UTF-8, it
         * is appended as-is for the ^ and | operators.  This preserves
         * backwards compatibility */
        if (right_non_downgraded) {
            non_downgraded = (char *) right_non_downgraded;
            non_downgraded_len = right_non_downgraded_len;
        }
        else if (left_non_downgraded) {
            non_downgraded = (char *) left_non_downgraded;
            non_downgraded_len = left_non_downgraded_len;
        }

        break;
    }

    if (result_needs_to_be_utf8) {
        sv_utf8_upgrade_nomg(sv);

        /* Append any trailing UTF-8 as-is. */
        if (non_downgraded) {
            sv_catpvn_nomg(sv, non_downgraded, non_downgraded_len);
        }
    }

    SvTAINT(sv);
}


/* Perl_do_kv() may be:
 *  * called directly as the pp function for pp_keys() and pp_values();
 *  * It may also be called directly when the op is OP_AVHVSWITCH, to
 *       implement CORE::keys(), CORE::values().
 *
 * In all cases it expects an HV on the stack and returns a list of keys,
 * values, or key-value pairs, depending on PL_op.
 */

PP(do_kv)
{
    dSP;
    HV * const keys = MUTABLE_HV(POPs);
    const U8 gimme = GIMME_V;

    const I32 dokeys   =     (PL_op->op_type == OP_KEYS)
                          || (    PL_op->op_type == OP_AVHVSWITCH
                              && (PL_op->op_private & OPpAVHVSWITCH_MASK)
                                    + OP_EACH == OP_KEYS);

    const I32 dovalues =     (PL_op->op_type == OP_VALUES)
                          || (    PL_op->op_type == OP_AVHVSWITCH
                              && (PL_op->op_private & OPpAVHVSWITCH_MASK)
                                     + OP_EACH == OP_VALUES);

    assert(   PL_op->op_type == OP_KEYS
           || PL_op->op_type == OP_VALUES
           || PL_op->op_type == OP_AVHVSWITCH);

    assert(!(    PL_op->op_type == OP_VALUES
             && (PL_op->op_private & OPpMAYBE_LVSUB)));

    (void)hv_iterinit(keys);	/* always reset iterator regardless */

    if (gimme == G_VOID)
        RETURN;

    if (gimme == G_SCALAR) {
        if (PL_op->op_flags & OPf_MOD || LVRET) {	/* lvalue */
            SV * const ret = newSV_type_mortal(SVt_PVLV);  /* Not TARG RT#67838 */
            sv_magic(ret, NULL, PERL_MAGIC_nkeys, NULL, 0);
            LvTYPE(ret) = 'k';
            LvTARG(ret) = SvREFCNT_inc_simple(keys);
            PUSHs(ret);
        }
        else {
            IV i;
            dTARGET;

            /* note that in 'scalar(keys %h)' the OP_KEYS is usually
             * optimised away and the action is performed directly by the
             * padhv or rv2hv op. We now only get here via OP_AVHVSWITCH
             * and \&CORE::keys
             */
            if (! SvTIED_mg((const SV *)keys, PERL_MAGIC_tied) ) {
                i = HvUSEDKEYS(keys);
            }
            else {
                i = 0;
                while (hv_iternext(keys)) i++;
            }
            PUSHi( i );
        }
        RETURN;
    }

    if (UNLIKELY(PL_op->op_private & OPpMAYBE_LVSUB)) {
        const I32 flags = is_lvalue_sub();
        if (flags && !(flags & OPpENTERSUB_INARGS))
            /* diag_listed_as: Can't modify %s in %s */
            Perl_croak(aTHX_ "Can't modify keys in list assignment");
    }

    PUTBACK;
    hv_pushkv(keys, (dokeys | (dovalues << 1)));
    return NORMAL;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
