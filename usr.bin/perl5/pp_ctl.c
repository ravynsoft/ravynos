/*    pp_ctl.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *      Now far ahead the Road has gone,
 *          And I must follow, if I can,
 *      Pursuing it with eager feet,
 *          Until it joins some larger way
 *      Where many paths and errands meet.
 *          And whither then?  I cannot say.
 *
 *     [Bilbo on p.35 of _The Lord of the Rings_, I/i: "A Long-Expected Party"]
 */

/* This file contains control-oriented pp ("push/pop") functions that
 * execute the opcodes that make up a perl program. A typical pp function
 * expects to find its arguments on the stack, and usually pushes its
 * results onto the stack, hence the 'pp' terminology. Each OP structure
 * contains a pointer to the relevant pp_foo() function.
 *
 * Control-oriented means things like pp_enteriter() and pp_next(), which
 * alter the flow of control of the program.
 */


#include "EXTERN.h"
#define PERL_IN_PP_CTL_C
#include "perl.h"
#include "feature.h"

#define dopopto_cursub() \
    (PL_curstackinfo->si_cxsubix >= 0        \
        ? PL_curstackinfo->si_cxsubix        \
        : dopoptosub_at(cxstack, cxstack_ix))

#define dopoptosub(plop)	dopoptosub_at(cxstack, (plop))

PP(pp_wantarray)
{
    dSP;
    I32 cxix;
    const PERL_CONTEXT *cx;
    EXTEND(SP, 1);

    if (PL_op->op_private & OPpOFFBYONE) {
        if (!(cx = caller_cx(1,NULL))) RETPUSHUNDEF;
    }
    else {
      cxix = dopopto_cursub();
      if (cxix < 0)
        RETPUSHUNDEF;
      cx = &cxstack[cxix];
    }

    switch (cx->blk_gimme) {
    case G_LIST:
        RETPUSHYES;
    case G_SCALAR:
        RETPUSHNO;
    default:
        RETPUSHUNDEF;
    }
}

PP(pp_regcreset)
{
    TAINT_NOT;
    return NORMAL;
}

PP(pp_regcomp)
{
    dSP;
    PMOP *pm = cPMOPx(cLOGOP->op_other);
    SV **args;
    int nargs;
    REGEXP *re = NULL;
    REGEXP *new_re;
    const regexp_engine *eng;
    bool is_bare_re= FALSE;

    if (PL_op->op_flags & OPf_STACKED) {
        dMARK;
        nargs = SP - MARK;
        args  = ++MARK;
    }
    else {
        nargs = 1;
        args  = SP;
    }

    /* prevent recompiling under /o and ithreads. */
#if defined(USE_ITHREADS)
    if (pm->op_pmflags & PMf_KEEP && PM_GETRE(pm)) {
        SP = args-1;
        RETURN;
    }
#endif

    re = PM_GETRE(pm);
    assert (re != (REGEXP*) &PL_sv_undef);
    eng = re ? RX_ENGINE(re) : current_re_engine();

    new_re = (eng->op_comp
                    ? eng->op_comp
                    : &Perl_re_op_compile
            )(aTHX_ args, nargs, pm->op_code_list, eng, re,
                &is_bare_re,
                (pm->op_pmflags & RXf_PMf_FLAGCOPYMASK),
                pm->op_pmflags |
                    (PL_op->op_flags & OPf_SPECIAL ? PMf_USE_RE_EVAL : 0));

    if (pm->op_pmflags & PMf_HAS_CV)
        ReANY(new_re)->qr_anoncv
                        = (CV*) SvREFCNT_inc(PAD_SV(PL_op->op_targ));

    if (is_bare_re) {
        REGEXP *tmp;
        /* The match's LHS's get-magic might need to access this op's regexp
           (e.g. $' =~ /$re/ while foo; see bug 70764).  So we must call
           get-magic now before we replace the regexp. Hopefully this hack can
           be replaced with the approach described at
           http://www.nntp.perl.org/group/perl.perl5.porters/2007/03/msg122415.html
           some day. */
        if (pm->op_type == OP_MATCH) {
            SV *lhs;
            const bool was_tainted = TAINT_get;
            if (pm->op_flags & OPf_STACKED)
                lhs = args[-1];
            else if (pm->op_targ)
                lhs = PAD_SV(pm->op_targ);
            else lhs = DEFSV;
            SvGETMAGIC(lhs);
            /* Restore the previous value of PL_tainted (which may have been
               modified by get-magic), to avoid incorrectly setting the
               RXf_TAINTED flag with RX_TAINT_on further down. */
            TAINT_set(was_tainted);
#ifdef NO_TAINT_SUPPORT
            PERL_UNUSED_VAR(was_tainted);
#endif
        }
        tmp = reg_temp_copy(NULL, new_re);
        ReREFCNT_dec(new_re);
        new_re = tmp;
    }

    if (re != new_re) {
        ReREFCNT_dec(re);
        PM_SETRE(pm, new_re);
    }


    assert(TAINTING_get || !TAINT_get);
    if (TAINT_get) {
        SvTAINTED_on((SV*)new_re);
        RX_TAINT_on(new_re);
    }

    /* handle the empty pattern */
    if (!RX_PRELEN(PM_GETRE(pm)) && PL_curpm) {
        if (PL_curpm == PL_reg_curpm) {
            if (PL_curpm_under && PL_curpm_under == PL_reg_curpm) {
                Perl_croak(aTHX_ "Infinite recursion via empty pattern");
            }
        }
    }

#if !defined(USE_ITHREADS)
    /* can't change the optree at runtime either */
    /* PMf_KEEP is handled differently under threads to avoid these problems */
    if (pm->op_pmflags & PMf_KEEP) {
        cLOGOP->op_first->op_next = PL_op->op_next;
    }
#endif

    SP = args-1;
    RETURN;
}


PP(pp_substcont)
{
    dSP;
    PERL_CONTEXT *cx = CX_CUR();
    PMOP * const pm = cPMOPx(cLOGOP->op_other);
    SV * const dstr = cx->sb_dstr;
    char *s = cx->sb_s;
    char *m = cx->sb_m;
    char *orig = cx->sb_orig;
    REGEXP * const rx = cx->sb_rx;
    SV *nsv = NULL;
    REGEXP *old = PM_GETRE(pm);

    PERL_ASYNC_CHECK();

    if(old != rx) {
        if(old)
            ReREFCNT_dec(old);
        PM_SETRE(pm,ReREFCNT_inc(rx));
    }

    rxres_restore(&cx->sb_rxres, rx);

    if (cx->sb_iters++) {
        const SSize_t saviters = cx->sb_iters;
        if (cx->sb_iters > cx->sb_maxiters)
            DIE(aTHX_ "Substitution loop");

        SvGETMAGIC(TOPs); /* possibly clear taint on $1 etc: #67962 */

        /* See "how taint works": pp_subst() in pp_hot.c */
        sv_catsv_nomg(dstr, POPs);
        if (UNLIKELY(TAINT_get))
            cx->sb_rxtainted |= SUBST_TAINT_REPL;
        if (CxONCE(cx) || s < orig ||
                !CALLREGEXEC(rx, s, cx->sb_strend, orig,
                             (s == m), cx->sb_targ, NULL,
                    (REXEC_IGNOREPOS|REXEC_NOT_FIRST|REXEC_FAIL_ON_UNDERFLOW)))
        {
            SV *targ = cx->sb_targ;

            assert(cx->sb_strend >= s);
            if(cx->sb_strend > s) {
                 if (DO_UTF8(dstr) && !SvUTF8(targ))
                      sv_catpvn_nomg_utf8_upgrade(dstr, s, cx->sb_strend - s, nsv);
                 else
                      sv_catpvn_nomg(dstr, s, cx->sb_strend - s);
            }
            if (RX_MATCH_TAINTED(rx)) /* run time pattern taint, eg locale */
                cx->sb_rxtainted |= SUBST_TAINT_PAT;

            if (pm->op_pmflags & PMf_NONDESTRUCT) {
                PUSHs(dstr);
                /* From here on down we're using the copy, and leaving the
                   original untouched.  */
                targ = dstr;
            }
            else {
                SV_CHECK_THINKFIRST_COW_DROP(targ);
                if (isGV(targ)) Perl_croak_no_modify();
                SvPV_free(targ);
                SvPV_set(targ, SvPVX(dstr));
                SvCUR_set(targ, SvCUR(dstr));
                SvLEN_set(targ, SvLEN(dstr));
                if (DO_UTF8(dstr))
                    SvUTF8_on(targ);
                SvPV_set(dstr, NULL);

                PL_tainted = 0;
                mPUSHi(saviters - 1);

                (void)SvPOK_only_UTF8(targ);
            }

            /* update the taint state of various variables in
             * preparation for final exit.
             * See "how taint works": pp_subst() in pp_hot.c */
            if (TAINTING_get) {
                if ((cx->sb_rxtainted & SUBST_TAINT_PAT) ||
                    ((cx->sb_rxtainted & (SUBST_TAINT_STR|SUBST_TAINT_RETAINT))
                                    == (SUBST_TAINT_STR|SUBST_TAINT_RETAINT))
                )
                    (RX_MATCH_TAINTED_on(rx)); /* taint $1 et al */

                if (!(cx->sb_rxtainted & SUBST_TAINT_BOOLRET)
                    && (cx->sb_rxtainted & (SUBST_TAINT_STR|SUBST_TAINT_PAT))
                )
                    SvTAINTED_on(TOPs);  /* taint return value */
                /* needed for mg_set below */
                TAINT_set(
                    cBOOL(cx->sb_rxtainted &
                          (SUBST_TAINT_STR|SUBST_TAINT_PAT|SUBST_TAINT_REPL))
                );

                /* sv_magic(), when adding magic (e.g.taint magic), also
                 * recalculates any pos() magic, converting any byte offset
                 * to utf8 offset. Make sure pos() is reset before this
                 * happens rather than using the now invalid value (since
                 * we've just replaced targ's pvx buffer with the
                 * potentially shorter dstr buffer). Normally (i.e. in
                 * non-taint cases), pos() gets removed a few lines later
                 * with the SvSETMAGIC().
                 */
                {
                    MAGIC *mg;
                    mg = mg_find_mglob(targ);
                    if (mg) {
                        MgBYTEPOS_set(mg, targ, SvPVX(targ), -1);
                    }
                }

                SvTAINT(TARG);
            }
            /* PL_tainted must be correctly set for this mg_set */
            SvSETMAGIC(TARG);
            TAINT_NOT;

            CX_LEAVE_SCOPE(cx);
            CX_POPSUBST(cx);
            CX_POP(cx);

            PERL_ASYNC_CHECK();
            RETURNOP(pm->op_next);
            NOT_REACHED; /* NOTREACHED */
        }
        cx->sb_iters = saviters;
    }
    if (RX_MATCH_COPIED(rx) && RX_SUBBEG(rx) != orig) {
        m = s;
        s = orig;
        assert(!RX_SUBOFFSET(rx));
        cx->sb_orig = orig = RX_SUBBEG(rx);
        s = orig + (m - s);
        cx->sb_strend = s + (cx->sb_strend - m);
    }
    cx->sb_m = m = RX_OFFS_START(rx,0) + orig;
    if (m > s) {
        if (DO_UTF8(dstr) && !SvUTF8(cx->sb_targ))
            sv_catpvn_nomg_utf8_upgrade(dstr, s, m - s, nsv);
        else
            sv_catpvn_nomg(dstr, s, m-s);
    }
    cx->sb_s = RX_OFFS_END(rx,0) + orig;
    { /* Update the pos() information. */
        SV * const sv
            = (pm->op_pmflags & PMf_NONDESTRUCT) ? cx->sb_dstr : cx->sb_targ;
        MAGIC *mg;

        /* the string being matched against may no longer be a string,
         * e.g. $_=0; s/.../$_++/ge */

        if (!SvPOK(sv))
            SvPV_force_nomg_nolen(sv);

        if (!(mg = mg_find_mglob(sv))) {
            mg = sv_magicext_mglob(sv);
        }
        MgBYTEPOS_set(mg, sv, SvPVX(sv), m - orig);
    }
    if (old != rx)
        (void)ReREFCNT_inc(rx);
    /* update the taint state of various variables in preparation
     * for calling the code block.
     * See "how taint works": pp_subst() in pp_hot.c */
    if (TAINTING_get) {
        if (RX_MATCH_TAINTED(rx)) /* run time pattern taint, eg locale */
            cx->sb_rxtainted |= SUBST_TAINT_PAT;

        if ((cx->sb_rxtainted & SUBST_TAINT_PAT) ||
            ((cx->sb_rxtainted & (SUBST_TAINT_STR|SUBST_TAINT_RETAINT))
                            == (SUBST_TAINT_STR|SUBST_TAINT_RETAINT))
        )
            (RX_MATCH_TAINTED_on(rx)); /* taint $1 et al */

        if (cx->sb_iters > 1 && (cx->sb_rxtainted & 
                        (SUBST_TAINT_STR|SUBST_TAINT_PAT|SUBST_TAINT_REPL)))
            SvTAINTED_on((pm->op_pmflags & PMf_NONDESTRUCT)
                         ? cx->sb_dstr : cx->sb_targ);
        TAINT_NOT;
    }
    rxres_save(&cx->sb_rxres, rx);
    PL_curpm = pm;
    RETURNOP(pm->op_pmstashstartu.op_pmreplstart);
}

void
Perl_rxres_save(pTHX_ void **rsp, REGEXP *rx)
{
    UV *p = (UV*)*rsp;
    U32 i;

    PERL_ARGS_ASSERT_RXRES_SAVE;
    PERL_UNUSED_CONTEXT;

    /* deal with regexp_paren_pair items */
    if (!p || p[1] < RX_NPARENS(rx)) {
#ifdef PERL_ANY_COW
        i = 7 + (RX_NPARENS(rx)+1) * 2;
#else
        i = 6 + (RX_NPARENS(rx)+1) * 2;
#endif
        if (!p)
            Newx(p, i, UV);
        else
            Renew(p, i, UV);
        *rsp = (void*)p;
    }

    /* what (if anything) to free on croak */
    *p++ = PTR2UV(RX_MATCH_COPIED(rx) ? RX_SUBBEG(rx) : NULL);
    RX_MATCH_COPIED_off(rx);
    *p++ = RX_NPARENS(rx);

#ifdef PERL_ANY_COW
    *p++ = PTR2UV(RX_SAVED_COPY(rx));
    RX_SAVED_COPY(rx) = NULL;
#endif

    *p++ = PTR2UV(RX_SUBBEG(rx));
    *p++ = (UV)RX_SUBLEN(rx);
    *p++ = (UV)RX_SUBOFFSET(rx);
    *p++ = (UV)RX_SUBCOFFSET(rx);
    for (i = 0; i <= RX_NPARENS(rx); ++i) {
        *p++ = (UV)RX_OFFSp(rx)[i].start;
        *p++ = (UV)RX_OFFSp(rx)[i].end;
    }
}

static void
S_rxres_restore(pTHX_ void **rsp, REGEXP *rx)
{
    UV *p = (UV*)*rsp;
    U32 i;

    PERL_ARGS_ASSERT_RXRES_RESTORE;
    PERL_UNUSED_CONTEXT;

    RX_MATCH_COPY_FREE(rx);
    RX_MATCH_COPIED_set(rx, *p);
    *p++ = 0;
    RX_NPARENS(rx) = *p++;

#ifdef PERL_ANY_COW
    if (RX_SAVED_COPY(rx))
        SvREFCNT_dec (RX_SAVED_COPY(rx));
    RX_SAVED_COPY(rx) = INT2PTR(SV*,*p);
    *p++ = 0;
#endif

    RX_SUBBEG(rx) = INT2PTR(char*,*p++);
    RX_SUBLEN(rx) = (I32)(*p++);
    RX_SUBOFFSET(rx) = (I32)*p++;
    RX_SUBCOFFSET(rx) = (I32)*p++;
    for (i = 0; i <= RX_NPARENS(rx); ++i) {
        RX_OFFSp(rx)[i].start = (I32)(*p++);
        RX_OFFSp(rx)[i].end = (I32)(*p++);
    }
}

static void
S_rxres_free(pTHX_ void **rsp)
{
    UV * const p = (UV*)*rsp;

    PERL_ARGS_ASSERT_RXRES_FREE;
    PERL_UNUSED_CONTEXT;

    if (p) {
        void *tmp = INT2PTR(char*,*p);
#ifdef PERL_POISON
#ifdef PERL_ANY_COW
        U32 i = 9 + p[1] * 2;
#else
        U32 i = 8 + p[1] * 2;
#endif
#endif

#ifdef PERL_ANY_COW
        SvREFCNT_dec (INT2PTR(SV*,p[2]));
#endif
#ifdef PERL_POISON
        PoisonFree(p, i, sizeof(UV));
#endif

        Safefree(tmp);
        Safefree(p);
        *rsp = NULL;
    }
}

#define FORM_NUM_BLANK (1<<30)
#define FORM_NUM_POINT (1<<29)

PP(pp_formline)
{
    dSP; dMARK; dORIGMARK;
    SV * const tmpForm = *++MARK;
    SV *formsv;		    /* contains text of original format */
    U32 *fpc;	    /* format ops program counter */
    char *t;	    /* current append position in target string */
    const char *f;	    /* current position in format string */
    I32 arg;
    SV *sv = NULL; /* current item */
    const char *item = NULL;/* string value of current item */
    I32 itemsize  = 0;	    /* length (chars) of item, possibly truncated */
    I32 itembytes = 0;	    /* as itemsize, but length in bytes */
    I32 fieldsize = 0;	    /* width of current field */
    I32 lines = 0;	    /* number of lines that have been output */
    bool chopspace = (strchr(PL_chopset, ' ') != NULL); /* does $: have space */
    const char *chophere = NULL; /* where to chop current item */
    STRLEN linemark = 0;    /* pos of start of line in output */
    NV value;
    bool gotsome = FALSE;   /* seen at least one non-blank item on this line */
    STRLEN len;             /* length of current sv */
    STRLEN linemax;	    /* estimate of output size in bytes */
    bool item_is_utf8 = FALSE;
    bool targ_is_utf8 = FALSE;
    const char *fmt;
    MAGIC *mg = NULL;
    U8 *source;		    /* source of bytes to append */
    STRLEN to_copy;	    /* how may bytes to append */
    char trans;		    /* what chars to translate */
    bool copied_form = FALSE; /* have we duplicated the form? */

    mg = doparseform(tmpForm);

    fpc = (U32*)mg->mg_ptr;
    /* the actual string the format was compiled from.
     * with overload etc, this may not match tmpForm */
    formsv = mg->mg_obj;


    SvPV_force(PL_formtarget, len);
    if (SvTAINTED(tmpForm) || SvTAINTED(formsv))
        SvTAINTED_on(PL_formtarget);
    if (DO_UTF8(PL_formtarget))
        targ_is_utf8 = TRUE;
    /* this is an initial estimate of how much output buffer space
     * to allocate. It may be exceeded later */
    linemax = (SvCUR(formsv) * (IN_BYTES ? 1 : 3) + 1);
    t = SvGROW(PL_formtarget, len + linemax + 1);
    /* XXX from now onwards, SvCUR(PL_formtarget) is invalid */
    t += len;
    f = SvPV_const(formsv, len);

    for (;;) {
        DEBUG_f( {
            const char *name = "???";
            arg = -1;
            switch (*fpc) {
            case FF_LITERAL:	arg = fpc[1]; name = "LITERAL";	break;
            case FF_BLANK:	arg = fpc[1]; name = "BLANK";	break;
            case FF_SKIP:	arg = fpc[1]; name = "SKIP";	break;
            case FF_FETCH:	arg = fpc[1]; name = "FETCH";	break;
            case FF_DECIMAL:	arg = fpc[1]; name = "DECIMAL";	break;

            case FF_CHECKNL:	name = "CHECKNL";	break;
            case FF_CHECKCHOP:	name = "CHECKCHOP";	break;
            case FF_SPACE:	name = "SPACE";		break;
            case FF_HALFSPACE:	name = "HALFSPACE";	break;
            case FF_ITEM:	name = "ITEM";		break;
            case FF_CHOP:	name = "CHOP";		break;
            case FF_LINEGLOB:	name = "LINEGLOB";	break;
            case FF_NEWLINE:	name = "NEWLINE";	break;
            case FF_MORE:	name = "MORE";		break;
            case FF_LINEMARK:	name = "LINEMARK";	break;
            case FF_END:	name = "END";		break;
            case FF_0DECIMAL:	name = "0DECIMAL";	break;
            case FF_LINESNGL:	name = "LINESNGL";	break;
            }
            if (arg >= 0)
                PerlIO_printf(Perl_debug_log, "%-16s%ld\n", name, (long) arg);
            else
                PerlIO_printf(Perl_debug_log, "%-16s\n", name);
        } );
        switch (*fpc++) {
        case FF_LINEMARK: /* start (or end) of a line */
            linemark = t - SvPVX(PL_formtarget);
            lines++;
            gotsome = FALSE;
            break;

        case FF_LITERAL: /* append <arg> literal chars */
            to_copy = *fpc++;
            source = (U8 *)f;
            f += to_copy;
            trans = '~';
            item_is_utf8 = (targ_is_utf8)
                           ? cBOOL(DO_UTF8(formsv))
                           : cBOOL(SvUTF8(formsv));
            goto append;

        case FF_SKIP: /* skip <arg> chars in format */
            f += *fpc++;
            break;

        case FF_FETCH: /* get next item and set field size to <arg> */
            arg = *fpc++;
            f += arg;
            fieldsize = arg;

            if (MARK < SP)
                sv = *++MARK;
            else {
                sv = &PL_sv_no;
                Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX), "Not enough format arguments");
            }
            if (SvTAINTED(sv))
                SvTAINTED_on(PL_formtarget);
            break;

        case FF_CHECKNL: /* find max len of item (up to \n) that fits field */
            {
                const char *s = item = SvPV_const(sv, len);
                const char *send = s + len;

                itemsize = 0;
                item_is_utf8 = DO_UTF8(sv);
                while (s < send) {
                    if (!isCNTRL(*s))
                        gotsome = TRUE;
                    else if (*s == '\n')
                        break;

                    if (item_is_utf8)
                        s += UTF8SKIP(s);
                    else
                        s++;
                    itemsize++;
                    if (itemsize == fieldsize)
                        break;
                }
                itembytes = s - item;
                chophere = s;
                break;
            }

        case FF_CHECKCHOP: /* like CHECKNL, but up to highest split point */
            {
                const char *s = item = SvPV_const(sv, len);
                const char *send = s + len;
                I32 size = 0;

                chophere = NULL;
                item_is_utf8 = DO_UTF8(sv);
                while (s < send) {
                    /* look for a legal split position */
                    if (isSPACE(*s)) {
                        if (*s == '\r') {
                            chophere = s;
                            itemsize = size;
                            break;
                        }
                        if (chopspace) {
                            /* provisional split point */
                            chophere = s;
                            itemsize = size;
                        }
                        /* we delay testing fieldsize until after we've
                         * processed the possible split char directly
                         * following the last field char; so if fieldsize=3
                         * and item="a b cdef", we consume "a b", not "a".
                         * Ditto further down.
                         */
                        if (size == fieldsize)
                            break;
                    }
                    else {
                        if (size == fieldsize)
                            break;
                        if (strchr(PL_chopset, *s)) {
                            /* provisional split point */
                            /* for a non-space split char, we include
                             * the split char; hence the '+1' */
                            chophere = s + 1;
                            itemsize = size + 1;
                        }
                        if (!isCNTRL(*s))
                            gotsome = TRUE;
                    }

                    if (item_is_utf8)
                        s += UTF8SKIP(s);
                    else
                        s++;
                    size++;
                }
                if (!chophere || s == send) {
                    chophere = s;
                    itemsize = size;
                }
                itembytes = chophere - item;

                break;
            }

        case FF_SPACE: /* append padding space (diff of field, item size) */
            arg = fieldsize - itemsize;
            if (arg) {
                fieldsize -= arg;
                while (arg-- > 0)
                    *t++ = ' ';
            }
            break;

        case FF_HALFSPACE: /* like FF_SPACE, but only append half as many */
            arg = fieldsize - itemsize;
            if (arg) {
                arg /= 2;
                fieldsize -= arg;
                while (arg-- > 0)
                    *t++ = ' ';
            }
            break;

        case FF_ITEM: /* append a text item, while blanking ctrl chars */
            to_copy = itembytes;
            source = (U8 *)item;
            trans = 1;
            goto append;

        case FF_CHOP: /* (for ^*) chop the current item */
            if (sv != &PL_sv_no) {
                const char *s = chophere;
                if (!copied_form &&
                    ((sv == tmpForm || SvSMAGICAL(sv))
                     || (SvGMAGICAL(tmpForm) && !sv_only_taint_gmagic(tmpForm))) ) {
                    /* sv and tmpForm are either the same SV, or magic might allow modification
                       of tmpForm when sv is modified, so copy */
                    SV *newformsv = sv_mortalcopy(formsv);
                    U32 *new_compiled;

                    f = SvPV_nolen(newformsv) + (f - SvPV_nolen(formsv));
                    Newx(new_compiled, mg->mg_len / sizeof(U32), U32);
                    memcpy(new_compiled, mg->mg_ptr, mg->mg_len);
                    SAVEFREEPV(new_compiled);
                    fpc = new_compiled + (fpc - (U32*)mg->mg_ptr);
                    formsv = newformsv;

                    copied_form = TRUE;
                }
                if (chopspace) {
                    while (isSPACE(*s))
                        s++;
                }
                if (SvPOKp(sv))
                    sv_chop(sv,s);
                else
                    /* tied, overloaded or similar strangeness.
                     * Do it the hard way */
                    sv_setpvn(sv, s, len - (s-item));
                SvSETMAGIC(sv);
                break;
            }
            /* FALLTHROUGH */

        case FF_LINESNGL: /* process ^*  */
            chopspace = 0;
            /* FALLTHROUGH */

        case FF_LINEGLOB: /* process @*  */
            {
                const bool oneline = fpc[-1] == FF_LINESNGL;
                const char *s = item = SvPV_const(sv, len);
                const char *const send = s + len;

                item_is_utf8 = DO_UTF8(sv);
                chophere = s + len;
                if (!len)
                    break;
                trans = 0;
                gotsome = TRUE;
                source = (U8 *) s;
                to_copy = len;
                while (s < send) {
                    if (*s++ == '\n') {
                        if (oneline) {
                            to_copy = s - item - 1;
                            chophere = s;
                            break;
                        } else {
                            if (s == send) {
                                to_copy--;
                            } else
                                lines++;
                        }
                    }
                }
            }

        append:
            /* append to_copy bytes from source to PL_formstring.
             * item_is_utf8 implies source is utf8.
             * if trans, translate certain characters during the copy */
            {
                U8 *tmp = NULL;
                STRLEN grow = 0;

                SvCUR_set(PL_formtarget,
                          t - SvPVX_const(PL_formtarget));

                if (targ_is_utf8 && !item_is_utf8) {
                    source = tmp = bytes_to_utf8(source, &to_copy);
                    grow = to_copy;
                } else {
                    if (item_is_utf8 && !targ_is_utf8) {
                        U8 *s;
                        /* Upgrade targ to UTF8, and then we reduce it to
                           a problem we have a simple solution for.
                           Don't need get magic.  */
                        sv_utf8_upgrade_nomg(PL_formtarget);
                        targ_is_utf8 = TRUE;
                        /* re-calculate linemark */
                        s = (U8*)SvPVX(PL_formtarget);
                        /* the bytes we initially allocated to append the
                         * whole line may have been gobbled up during the
                         * upgrade, so allocate a whole new line's worth
                         * for safety */
                        grow = linemax;
                        while (linemark--)
                            s += UTF8_SAFE_SKIP(s,
                                            (U8 *) SvEND(PL_formtarget));
                        linemark = s - (U8*)SvPVX(PL_formtarget);
                    }
                    /* Easy. They agree.  */
                    assert (item_is_utf8 == targ_is_utf8);
                }
                if (!trans)
                    /* @* and ^* are the only things that can exceed
                     * the linemax, so grow by the output size, plus
                     * a whole new form's worth in case of any further
                     * output */
                    grow = linemax + to_copy;
                if (grow)
                    SvGROW(PL_formtarget, SvCUR(PL_formtarget) + grow + 1);
                t = SvPVX(PL_formtarget) + SvCUR(PL_formtarget);

                Copy(source, t, to_copy, char);
                if (trans) {
                    /* blank out ~ or control chars, depending on trans.
                     * works on bytes not chars, so relies on not
                     * matching utf8 continuation bytes */
                    U8 *s = (U8*)t;
                    U8 *send = s + to_copy;
                    while (s < send) {
                        const int ch = *s;
                        if (trans == '~' ? (ch == '~') : isCNTRL(ch))
                            *s = ' ';
                        s++;
                    }
                }

                t += to_copy;
                SvCUR_set(PL_formtarget, SvCUR(PL_formtarget) + to_copy);
                if (tmp)
                    Safefree(tmp);
                break;
            }

        case FF_0DECIMAL: /* like FF_DECIMAL but for 0### */
            arg = *fpc++;
            fmt = (const char *)
                ((arg & FORM_NUM_POINT) ? "%#0*.*" NVff : "%0*.*" NVff);
            goto ff_dec;

        case FF_DECIMAL: /* do @##, ^##, where <arg>=(precision|flags) */
            arg = *fpc++;
            fmt = (const char *)
                ((arg & FORM_NUM_POINT) ? "%#*.*" NVff : "%*.*" NVff);
        ff_dec:
            /* If the field is marked with ^ and the value is undefined,
               blank it out. */
            if ((arg & FORM_NUM_BLANK) && !SvOK(sv)) {
                arg = fieldsize;
                while (arg--)
                    *t++ = ' ';
                break;
            }
            gotsome = TRUE;
            value = SvNV(sv);
            /* overflow evidence */
            if (num_overflow(value, fieldsize, arg)) {
                arg = fieldsize;
                while (arg--)
                    *t++ = '#';
                break;
            }
            /* Formats aren't yet marked for locales, so assume "yes". */
            {
                Size_t max = SvLEN(PL_formtarget) - (t - SvPVX(PL_formtarget));
                int len;
                arg &= ~(FORM_NUM_POINT|FORM_NUM_BLANK);
#ifdef USE_QUADMATH
                {
                    int len;
                    if (!quadmath_format_valid(fmt))
                        Perl_croak_nocontext("panic: quadmath invalid format \"%s\"", fmt);
                    WITH_LC_NUMERIC_SET_TO_NEEDED(
                        len = quadmath_snprintf(t, max, fmt, (int) fieldsize,
                                               (int) arg, value);
                    );
                    if (len == -1)
                        Perl_croak_nocontext("panic: quadmath_snprintf failed, format \"%s\"", fmt);
                }
#else
                /* we generate fmt ourselves so it is safe */
                GCC_DIAG_IGNORE_STMT(-Wformat-nonliteral);
                len = my_snprintf(t, max, fmt, (int) fieldsize, (int) arg, value);
                GCC_DIAG_RESTORE_STMT;
#endif
                PERL_MY_SNPRINTF_POST_GUARD(len, max);
            }
            t += fieldsize;
            break;

        case FF_NEWLINE: /* delete trailing spaces, then append \n */
            f++;
            while (t-- > (SvPVX(PL_formtarget) + linemark) && *t == ' ') ;
            t++;
            *t++ = '\n';
            break;

        case FF_BLANK: /* for arg==0: do '~'; for arg>0 : do '~~' */
            arg = *fpc++;
            if (gotsome) {
                if (arg) {		/* repeat until fields exhausted? */
                    fpc--;
                    goto end;
                }
            }
            else {
                t = SvPVX(PL_formtarget) + linemark;
                lines--;
            }
            break;

        case FF_MORE: /* replace long end of string with '...' */
            {
                const char *s = chophere;
                const char *send = item + len;
                if (chopspace) {
                    while (isSPACE(*s) && (s < send))
                        s++;
                }
                if (s < send) {
                    char *s1;
                    arg = fieldsize - itemsize;
                    if (arg) {
                        fieldsize -= arg;
                        while (arg-- > 0)
                            *t++ = ' ';
                    }
                    s1 = t - 3;
                    if (strBEGINs(s1,"   ")) {
                        while (s1 > SvPVX_const(PL_formtarget) && isSPACE(s1[-1]))
                            s1--;
                    }
                    *s1++ = '.';
                    *s1++ = '.';
                    *s1++ = '.';
                }
                break;
            }

        case FF_END: /* tidy up, then return */
        end:
            assert(t < SvPVX_const(PL_formtarget) + SvLEN(PL_formtarget));
            *t = '\0';
            SvCUR_set(PL_formtarget, t - SvPVX_const(PL_formtarget));
            if (targ_is_utf8)
                SvUTF8_on(PL_formtarget);
            FmLINES(PL_formtarget) += lines;
            SP = ORIGMARK;
            if (fpc[-1] == FF_BLANK)
                RETURNOP(cLISTOP->op_first);
            else
                RETPUSHYES;
        }
    }
}

/* also used for: pp_mapstart() */
PP(pp_grepstart)
{
    /* See the code comments at the start of pp_grepwhile() and
     * pp_mapwhile() for an explanation of how the stack is used
     * during a grep or map.
     */

    dSP;
    SV *src;

    if (PL_stack_base + TOPMARK == SP) {
        (void)POPMARK;
        if (GIMME_V == G_SCALAR)
            XPUSHs(&PL_sv_zero);
        RETURNOP(PL_op->op_next->op_next);
    }
    PL_stack_sp = PL_stack_base + TOPMARK + 1;
    PUSHMARK(PL_stack_sp);				/* push dst */
    PUSHMARK(PL_stack_sp);				/* push src */
    ENTER_with_name("grep");					/* enter outer scope */

    SAVETMPS;
    SAVE_DEFSV;
    ENTER_with_name("grep_item");					/* enter inner scope */
    SAVEVPTR(PL_curpm);

    src = PL_stack_base[TOPMARK];
    if (SvPADTMP(src)) {
        src = PL_stack_base[TOPMARK] = sv_mortalcopy(src);
        PL_tmps_floor++;
    }
    SvTEMP_off(src);
    DEFSV_set(src);

    PUTBACK;
    if (PL_op->op_type == OP_MAPSTART)
        PUSHMARK(PL_stack_sp);			/* push top */
    return cLOGOPx(PL_op->op_next)->op_other;
}

/* pp_grepwhile() lives in pp_hot.c */

PP(pp_mapwhile)
{
    /* Understanding the stack during a map.
     *
     * 'map expr, args' is implemented in the form of
     *
     *     grepstart; // which handles map too
     *     do {
     *          expr;
     *          mapwhile;
     *     } while (args);
     *
     * The stack examples below are in the form of 'perl -Ds' output,
     * where any stack element indexed by PL_markstack_ptr[i] has a star
     * just to the right of it.  In addition, the corresponding i value
     * is displayed under the indexed stack element.
     *
     * On entry to mapwhile, the stack looks like this:
     *
     *      =>   *  A1..An  X1  *  X2..Xn  C  *  R1..Rn  *  E1..En
     *      [-3]           [-2]          [-1]        [0]
     *
     * where:
     *   A1..An   Accumulated results from all previous iterations of expr
     *   X1..Xn   Random garbage
     *   C        The current (just processed) arg, still aliased to $_.
     *   R1..Rn   The args remaining to be processed.
     *   E1..En   the (list) result of the just-executed map expression.
     *
     * Note that it is easiest to think of stack marks [-1] and [-2] as both
     * being one too high, and so it would make more sense to have had the
     * marks like this:
     *
     *      =>   *  A1..An  *  X1..Xn  *  C  R1..Rn  *  E1..En
     *      [-3]       [-2]       [-1]           [0]
     *
     * where the stack is divided neatly into 4 groups:
     *   - accumulated results
     *   - discards and/or holes proactively created for later result storage
     *   - being, or yet to be, processed,
     *   - results of last expr
     * But off-by-one is the way it is currently, and it works as long as
     * we keep it consistent and bear it in mind.
     *
     * pp_mapwhile() does the following:
     *
     * - If there isn't enough space in the X1..Xn zone to insert the
     *   expression results, grow the stack and shift up everything above C.
     * - move E1..En to just above An
     * - at the same time, manipulate the tmps stack so that temporaries
     *   from executing expr can be freed without prematurely freeing
     *   E1..En.
     * - if on last iteration, pop all the marks, reset the stack pointer
     *   and update the return args based on caller context.
     * - else alias $_ to the next arg.
     *
     */

    dSP;
    const U8 gimme = GIMME_V;
    I32 items = (SP - PL_stack_base) - TOPMARK; /* how many new items */
    I32 count;
    I32 shift;
    SV** src;
    SV** dst;

    /* first, move source pointer to the next item in the source list */
    ++PL_markstack_ptr[-1];

    /* if there are new items, push them into the destination list */
    if (items && gimme != G_VOID) {
        /* might need to make room back there first */
        if (items > PL_markstack_ptr[-1] - PL_markstack_ptr[-2]) {
            /* XXX this implementation is very pessimal because the stack
             * is repeatedly extended for every set of items.  Is possible
             * to do this without any stack extension or copying at all
             * by maintaining a separate list over which the map iterates
             * (like foreach does). --gsar */

            /* everything in the stack after the destination list moves
             * towards the end the stack by the amount of room needed */
            shift = items - (PL_markstack_ptr[-1] - PL_markstack_ptr[-2]);

            /* items to shift up (accounting for the moved source pointer) */
            count = (SP - PL_stack_base) - (PL_markstack_ptr[-1] - 1);

            /* This optimization is by Ben Tilly and it does
             * things differently from what Sarathy (gsar)
             * is describing.  The downside of this optimization is
             * that leaves "holes" (uninitialized and hopefully unused areas)
             * to the Perl stack, but on the other hand this
             * shouldn't be a problem.  If Sarathy's idea gets
             * implemented, this optimization should become
             * irrelevant.  --jhi */
            if (shift < count)
                shift = count; /* Avoid shifting too often --Ben Tilly */

            EXTEND(SP,shift);
            src = SP;
            dst = (SP += shift);
            PL_markstack_ptr[-1] += shift;
            *PL_markstack_ptr += shift;
            while (count--)
                *dst-- = *src--;
        }
        /* copy the new items down to the destination list */
        dst = PL_stack_base + (PL_markstack_ptr[-2] += items) - 1;
        if (gimme == G_LIST) {
            /* add returned items to the collection (making mortal copies
             * if necessary), then clear the current temps stack frame
             * *except* for those items. We do this splicing the items
             * into the start of the tmps frame (so some items may be on
             * the tmps stack twice), then moving PL_tmps_floor above
             * them, then freeing the frame. That way, the only tmps that
             * accumulate over iterations are the return values for map.
             * We have to do to this way so that everything gets correctly
             * freed if we die during the map.
             */
            I32 tmpsbase;
            I32 i = items;
            /* make space for the slice */
            EXTEND_MORTAL(items);
            tmpsbase = PL_tmps_floor + 1;
            Move(PL_tmps_stack + tmpsbase,
                 PL_tmps_stack + tmpsbase + items,
                 PL_tmps_ix - PL_tmps_floor,
                 SV*);
            PL_tmps_ix += items;

            while (i-- > 0) {
                SV *sv = POPs;
                if (!SvTEMP(sv))
                    sv = sv_mortalcopy(sv);
                *dst-- = sv;
                PL_tmps_stack[tmpsbase++] = SvREFCNT_inc_simple(sv);
            }
            /* clear the stack frame except for the items */
            PL_tmps_floor += items;
            FREETMPS;
            /* FREETMPS may have cleared the TEMP flag on some of the items */
            i = items;
            while (i-- > 0)
                SvTEMP_on(PL_tmps_stack[--tmpsbase]);
        }
        else {
            /* scalar context: we don't care about which values map returns
             * (we use undef here). And so we certainly don't want to do mortal
             * copies of meaningless values. */
            while (items-- > 0) {
                (void)POPs;
                *dst-- = &PL_sv_undef;
            }
            FREETMPS;
        }
    }
    else {
        FREETMPS;
    }
    LEAVE_with_name("grep_item");					/* exit inner scope */

    /* All done yet? */
    if (PL_markstack_ptr[-1] > TOPMARK) {

        (void)POPMARK;				/* pop top */
        LEAVE_with_name("grep");					/* exit outer scope */
        (void)POPMARK;				/* pop src */
        items = --*PL_markstack_ptr - PL_markstack_ptr[-1];
        (void)POPMARK;				/* pop dst */
        SP = PL_stack_base + POPMARK;		/* pop original mark */
        if (gimme == G_SCALAR) {
                dTARGET;
                XPUSHi(items);
        }
        else if (gimme == G_LIST)
            SP += items;
        RETURN;
    }
    else {
        SV *src;

        ENTER_with_name("grep_item");					/* enter inner scope */
        SAVEVPTR(PL_curpm);

        /* set $_ to the new source item */
        src = PL_stack_base[PL_markstack_ptr[-1]];
        if (SvPADTMP(src)) {
            src = sv_mortalcopy(src);
        }
        SvTEMP_off(src);
        DEFSV_set(src);

        RETURNOP(cLOGOP->op_other);
    }
}

/* Range stuff. */

PP(pp_range)
{
    dTARG;
    if (GIMME_V == G_LIST)
        return NORMAL;
    GETTARGET;
    if (SvTRUE_NN(targ))
        return cLOGOP->op_other;
    else
        return NORMAL;
}

PP(pp_flip)
{
    dSP;

    if (GIMME_V == G_LIST) {
        RETURNOP(cLOGOPx(cUNOP->op_first)->op_other);
    }
    else {
        dTOPss;
        SV * const targ = PAD_SV(PL_op->op_targ);
        int flip = 0;

        if (PL_op->op_private & OPpFLIP_LINENUM) {
            if (GvIO(PL_last_in_gv)) {
                flip = SvIV(sv) == (IV)IoLINES(GvIOp(PL_last_in_gv));
            }
            else {
                GV * const gv = gv_fetchpvs(".", GV_ADD|GV_NOTQUAL, SVt_PV);
                if (gv && GvSV(gv))
                    flip = SvIV(sv) == SvIV(GvSV(gv));
            }
        } else {
            flip = SvTRUE_NN(sv);
        }
        if (flip) {
            sv_setiv(PAD_SV(cUNOP->op_first->op_targ), 1);
            if (PL_op->op_flags & OPf_SPECIAL) {
                sv_setiv(targ, 1);
                SETs(targ);
                RETURN;
            }
            else {
                sv_setiv(targ, 0);
                SP--;
                RETURNOP(cLOGOPx(cUNOP->op_first)->op_other);
            }
        }
        SvPVCLEAR(TARG);
        SETs(targ);
        RETURN;
    }
}

/* This code tries to decide if "$left .. $right" should use the
   magical string increment, or if the range is numeric. Initially,
   an exception was made for *any* string beginning with "0" (see
   [#18165], AMS 20021031), but now that is only applied when the
   string's length is also >1 - see the rules now documented in
   perlop [#133695] */

#define RANGE_IS_NUMERIC(left,right) ( \
        SvNIOKp(left)  || (SvOK(left)  && !SvPOKp(left))  || \
        SvNIOKp(right) || (SvOK(right) && !SvPOKp(right)) || \
        (((!SvOK(left) && SvOK(right)) || ((!SvOK(left) || \
          looks_like_number(left)) && SvPOKp(left) \
          && !(*SvPVX_const(left) == '0' && SvCUR(left)>1 ) )) \
         && (!SvOK(right) || looks_like_number(right))))

PP(pp_flop)
{
    dSP;

    if (GIMME_V == G_LIST) {
        dPOPPOPssrl;

        SvGETMAGIC(left);
        SvGETMAGIC(right);

        if (RANGE_IS_NUMERIC(left,right)) {
            IV i, j, n;
            if ((SvOK(left) && !SvIOK(left) && SvNV_nomg(left) < IV_MIN) ||
                (SvOK(right) && (SvIOK(right)
                                 ? SvIsUV(right) && SvUV(right) > IV_MAX
                                 : SvNV_nomg(right) > (NV) IV_MAX)))
                DIE(aTHX_ "Range iterator outside integer range");
            i = SvIV_nomg(left);
            j = SvIV_nomg(right);
            if (j >= i) {
                /* Dance carefully around signed max. */
                bool overflow = (i <= 0 && j > SSize_t_MAX + i - 1);
                if (!overflow) {
                    n = j - i + 1;
                    /* The wraparound of signed integers is undefined
                     * behavior, but here we aim for count >=1, and
                     * negative count is just wrong. */
                    if (n < 1
#if IVSIZE > Size_t_size
                        || n > SSize_t_MAX
#endif
                        )
                        overflow = TRUE;
                }
                if (overflow)
                    Perl_croak(aTHX_ "Out of memory during list extend");
                EXTEND_MORTAL(n);
                EXTEND(SP, n);
            }
            else
                n = 0;
            while (n--) {
                SV * const sv = sv_2mortal(newSViv(i));
                PUSHs(sv);
                if (n) /* avoid incrementing above IV_MAX */
                    i++;
            }
        }
        else {
            STRLEN len, llen;
            const char * const lpv = SvPV_nomg_const(left, llen);
            const char * const tmps = SvPV_nomg_const(right, len);

            SV *sv = newSVpvn_flags(lpv, llen, SvUTF8(left)|SVs_TEMP);
            if (DO_UTF8(right) && IN_UNI_8_BIT)
                len = sv_len_utf8_nomg(right);
            while (!SvNIOKp(sv) && SvCUR(sv) <= len) {
                XPUSHs(sv);
                if (strEQ(SvPVX_const(sv),tmps))
                    break;
                sv = sv_2mortal(newSVsv(sv));
                sv_inc(sv);
            }
        }
    }
    else {
        dTOPss;
        SV * const targ = PAD_SV(cUNOP->op_first->op_targ);
        int flop = 0;
        sv_inc(targ);

        if (PL_op->op_private & OPpFLIP_LINENUM) {
            if (GvIO(PL_last_in_gv)) {
                flop = SvIV(sv) == (IV)IoLINES(GvIOp(PL_last_in_gv));
            }
            else {
                GV * const gv = gv_fetchpvs(".", GV_ADD|GV_NOTQUAL, SVt_PV);
                if (gv && GvSV(gv)) flop = SvIV(sv) == SvIV(GvSV(gv));
            }
        }
        else {
            flop = SvTRUE_NN(sv);
        }

        if (flop) {
            sv_setiv(PAD_SV(cUNOPx(cUNOP->op_first)->op_first->op_targ), 0);
            sv_catpvs(targ, "E0");
        }
        SETs(targ);
    }

    RETURN;
}

/* Control. */

static const char * const context_name[] = {
    "pseudo-block",
    NULL, /* CXt_WHEN never actually needs "block" */
    NULL, /* CXt_BLOCK never actually needs "block" */
    NULL, /* CXt_GIVEN never actually needs "block" */
    NULL, /* CXt_LOOP_PLAIN never actually needs "loop" */
    NULL, /* CXt_LOOP_LAZYIV never actually needs "loop" */
    NULL, /* CXt_LOOP_LAZYSV never actually needs "loop" */
    NULL, /* CXt_LOOP_LIST never actually needs "loop" */
    NULL, /* CXt_LOOP_ARY never actually needs "loop" */
    "subroutine",
    "format",
    "eval",
    "substitution",
    "defer block",
};

STATIC I32
S_dopoptolabel(pTHX_ const char *label, STRLEN len, U32 flags)
{
    I32 i;

    PERL_ARGS_ASSERT_DOPOPTOLABEL;

    for (i = cxstack_ix; i >= 0; i--) {
        const PERL_CONTEXT * const cx = &cxstack[i];
        switch (CxTYPE(cx)) {
        case CXt_EVAL:
            if(CxTRY(cx))
                continue;
            /* FALLTHROUGH */
        case CXt_SUBST:
        case CXt_SUB:
        case CXt_FORMAT:
        case CXt_NULL:
            /* diag_listed_as: Exiting subroutine via %s */
            Perl_ck_warner(aTHX_ packWARN(WARN_EXITING), "Exiting %s via %s",
                           context_name[CxTYPE(cx)], OP_NAME(PL_op));
            if (CxTYPE(cx) == CXt_NULL) /* sort BLOCK */
                return -1;
            break;
        case CXt_LOOP_PLAIN:
        case CXt_LOOP_LAZYIV:
        case CXt_LOOP_LAZYSV:
        case CXt_LOOP_LIST:
        case CXt_LOOP_ARY:
          {
            STRLEN cx_label_len = 0;
            U32 cx_label_flags = 0;
            const char *cx_label = CxLABEL_len_flags(cx, &cx_label_len, &cx_label_flags);
            if (!cx_label || !(
                    ( (cx_label_flags & SVf_UTF8) != (flags & SVf_UTF8) ) ?
                        (flags & SVf_UTF8)
                            ? (bytes_cmp_utf8(
                                        (const U8*)cx_label, cx_label_len,
                                        (const U8*)label, len) == 0)
                            : (bytes_cmp_utf8(
                                        (const U8*)label, len,
                                        (const U8*)cx_label, cx_label_len) == 0)
                    : (len == cx_label_len && ((cx_label == label)
                                    || memEQ(cx_label, label, len))) )) {
                DEBUG_l(Perl_deb(aTHX_ "(poptolabel(): skipping label at cx=%ld %s)\n",
                        (long)i, cx_label));
                continue;
            }
            DEBUG_l( Perl_deb(aTHX_ "(poptolabel(): found label at cx=%ld %s)\n", (long)i, label));
            return i;
          }
        }
    }
    return i;
}

/*
=for apidoc_section $callback
=for apidoc dowantarray

Implements the deprecated L<perlapi/C<GIMME>>.

=cut
*/

U8
Perl_dowantarray(pTHX)
{
    const U8 gimme = block_gimme();
    return (gimme == G_VOID) ? G_SCALAR : gimme;
}

/* note that this function has mostly been superseded by Perl_gimme_V */

U8
Perl_block_gimme(pTHX)
{
    const I32 cxix = dopopto_cursub();
    U8 gimme;
    if (cxix < 0)
        return G_VOID;

    gimme = (cxstack[cxix].blk_gimme & G_WANT);
    if (!gimme)
        Perl_croak(aTHX_ "panic: bad gimme: %d\n", gimme);
    return gimme;
}

/*
=for apidoc is_lvalue_sub

Returns non-zero if the sub calling this function is being called in an lvalue
context.  Returns 0 otherwise.

=cut
*/

I32
Perl_is_lvalue_sub(pTHX)
{
    const I32 cxix = dopopto_cursub();
    assert(cxix >= 0);  /* We should only be called from inside subs */

    if (CxLVAL(cxstack + cxix) && CvLVALUE(cxstack[cxix].blk_sub.cv))
        return CxLVAL(cxstack + cxix);
    else
        return 0;
}

/* only used by cx_pushsub() */
I32
Perl_was_lvalue_sub(pTHX)
{
    const I32 cxix = dopoptosub(cxstack_ix-1);
    assert(cxix >= 0);  /* We should only be called from inside subs */

    if (CxLVAL(cxstack + cxix) && CvLVALUE(cxstack[cxix].blk_sub.cv))
        return CxLVAL(cxstack + cxix);
    else
        return 0;
}

STATIC I32
S_dopoptosub_at(pTHX_ const PERL_CONTEXT *cxstk, I32 startingblock)
{
    I32 i;

    PERL_ARGS_ASSERT_DOPOPTOSUB_AT;
#ifndef DEBUGGING
    PERL_UNUSED_CONTEXT;
#endif

    for (i = startingblock; i >= 0; i--) {
        const PERL_CONTEXT * const cx = &cxstk[i];
        switch (CxTYPE(cx)) {
        default:
            continue;
        case CXt_SUB:
            /* in sub foo { /(?{...})/ }, foo ends up on the CX stack
             * twice; the first for the normal foo() call, and the second
             * for a faked up re-entry into the sub to execute the
             * code block. Hide this faked entry from the world. */
            if (cx->cx_type & CXp_SUB_RE_FAKE)
                continue;
            DEBUG_l( Perl_deb(aTHX_ "(dopoptosub_at(): found sub at cx=%ld)\n", (long)i));
            return i;

        case CXt_EVAL:
            if (CxTRY(cx))
                continue;
            DEBUG_l( Perl_deb(aTHX_ "(dopoptosub_at(): found sub at cx=%ld)\n", (long)i));
            return i;

        case CXt_FORMAT:
            DEBUG_l( Perl_deb(aTHX_ "(dopoptosub_at(): found sub at cx=%ld)\n", (long)i));
            return i;
        }
    }
    return i;
}

STATIC I32
S_dopoptoeval(pTHX_ I32 startingblock)
{
    I32 i;
    for (i = startingblock; i >= 0; i--) {
        const PERL_CONTEXT *cx = &cxstack[i];
        switch (CxTYPE(cx)) {
        default:
            continue;
        case CXt_EVAL:
            DEBUG_l( Perl_deb(aTHX_ "(dopoptoeval(): found eval at cx=%ld)\n", (long)i));
            return i;
        }
    }
    return i;
}

STATIC I32
S_dopoptoloop(pTHX_ I32 startingblock)
{
    I32 i;
    for (i = startingblock; i >= 0; i--) {
        const PERL_CONTEXT * const cx = &cxstack[i];
        switch (CxTYPE(cx)) {
        case CXt_EVAL:
            if(CxTRY(cx))
                continue;
            /* FALLTHROUGH */
        case CXt_SUBST:
        case CXt_SUB:
        case CXt_FORMAT:
        case CXt_NULL:
            /* diag_listed_as: Exiting subroutine via %s */
            Perl_ck_warner(aTHX_ packWARN(WARN_EXITING), "Exiting %s via %s",
                           context_name[CxTYPE(cx)], OP_NAME(PL_op));
            if ((CxTYPE(cx)) == CXt_NULL) /* sort BLOCK */
                return -1;
            break;
        case CXt_LOOP_PLAIN:
        case CXt_LOOP_LAZYIV:
        case CXt_LOOP_LAZYSV:
        case CXt_LOOP_LIST:
        case CXt_LOOP_ARY:
            DEBUG_l( Perl_deb(aTHX_ "(dopoptoloop(): found loop at cx=%ld)\n", (long)i));
            return i;
        }
    }
    return i;
}

/* find the next GIVEN or FOR (with implicit $_) loop context block */

STATIC I32
S_dopoptogivenfor(pTHX_ I32 startingblock)
{
    I32 i;
    for (i = startingblock; i >= 0; i--) {
        const PERL_CONTEXT *cx = &cxstack[i];
        switch (CxTYPE(cx)) {
        default:
            continue;
        case CXt_GIVEN:
            DEBUG_l( Perl_deb(aTHX_ "(dopoptogivenfor(): found given at cx=%ld)\n", (long)i));
            return i;
        case CXt_LOOP_PLAIN:
            assert(!(cx->cx_type & CXp_FOR_DEF));
            break;
        case CXt_LOOP_LAZYIV:
        case CXt_LOOP_LAZYSV:
        case CXt_LOOP_LIST:
        case CXt_LOOP_ARY:
            if (cx->cx_type & CXp_FOR_DEF) {
                DEBUG_l( Perl_deb(aTHX_ "(dopoptogivenfor(): found foreach at cx=%ld)\n", (long)i));
                return i;
            }
        }
    }
    return i;
}

STATIC I32
S_dopoptowhen(pTHX_ I32 startingblock)
{
    I32 i;
    for (i = startingblock; i >= 0; i--) {
        const PERL_CONTEXT *cx = &cxstack[i];
        switch (CxTYPE(cx)) {
        default:
            continue;
        case CXt_WHEN:
            DEBUG_l( Perl_deb(aTHX_ "(dopoptowhen(): found when at cx=%ld)\n", (long)i));
            return i;
        }
    }
    return i;
}

/* dounwind(): pop all contexts above (but not including) cxix.
 * Note that it clears the savestack frame associated with each popped
 * context entry, but doesn't free any temps.
 * It does a cx_popblock() of the last frame that it pops, and leaves
 * cxstack_ix equal to cxix.
 */

void
Perl_dounwind(pTHX_ I32 cxix)
{
    if (!PL_curstackinfo) /* can happen if die during thread cloning */
        return;

    while (cxstack_ix > cxix) {
        PERL_CONTEXT *cx = CX_CUR();

        CX_DEBUG(cx, "UNWIND");
        /* Note: we don't need to restore the base context info till the end. */

        CX_LEAVE_SCOPE(cx);

        switch (CxTYPE(cx)) {
        case CXt_SUBST:
            CX_POPSUBST(cx);
            /* CXt_SUBST is not a block context type, so skip the
             * cx_popblock(cx) below */
            if (cxstack_ix == cxix + 1) {
                cxstack_ix--;
                return;
            }
            break;
        case CXt_SUB:
            cx_popsub(cx);
            break;
        case CXt_EVAL:
            cx_popeval(cx);
            break;
        case CXt_LOOP_PLAIN:
        case CXt_LOOP_LAZYIV:
        case CXt_LOOP_LAZYSV:
        case CXt_LOOP_LIST:
        case CXt_LOOP_ARY:
            cx_poploop(cx);
            break;
        case CXt_WHEN:
            cx_popwhen(cx);
            break;
        case CXt_GIVEN:
            cx_popgiven(cx);
            break;
        case CXt_BLOCK:
        case CXt_NULL:
        case CXt_DEFER:
            /* these two don't have a POPFOO() */
            break;
        case CXt_FORMAT:
            cx_popformat(cx);
            break;
        }
        if (cxstack_ix == cxix + 1) {
            cx_popblock(cx);
        }
        cxstack_ix--;
    }

}

void
Perl_qerror(pTHX_ SV *err)
{
    PERL_ARGS_ASSERT_QERROR;
    if (err!=NULL) {
        if (PL_in_eval) {
            if (PL_in_eval & EVAL_KEEPERR) {
                    Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "\t(in cleanup) %" SVf,
                                                        SVfARG(err));
            }
            else {
                sv_catsv(ERRSV, err);
            }
        }
        else if (PL_errors)
            sv_catsv(PL_errors, err);
        else
            Perl_warn(aTHX_ "%" SVf, SVfARG(err));

        if (PL_parser) {
            ++PL_parser->error_count;
        }
    }

    if ( PL_parser && (err == NULL ||
         PL_parser->error_count >= PERL_STOP_PARSING_AFTER_N_ERRORS)
    ) {
        const char * const name = OutCopFILE(PL_curcop);
        SV * errsv = NULL;
        U8 raw_error_count = PERL_PARSE_ERROR_COUNT(PL_parser->error_count);

        if (PL_in_eval) {
            errsv = ERRSV;
        }

        if (err == NULL) {
            abort_execution(errsv, name);
        }
        else
        if (raw_error_count >= PERL_STOP_PARSING_AFTER_N_ERRORS) {
            if (errsv) {
                Perl_croak(aTHX_ "%" SVf "%s has too many errors.\n",
                    SVfARG(errsv), name);
            } else {
                Perl_croak(aTHX_ "%s has too many errors.\n", name);
            }
        }
    }
}


/* pop a CXt_EVAL context and in addition, if it was a require then
 * based on action:
 *     0: do nothing extra;
 *     1: undef  $INC{$name}; croak "$name did not return a true value";
 *     2: delete $INC{$name}; croak "$errsv: Compilation failed in require"
 */

static void
S_pop_eval_context_maybe_croak(pTHX_ PERL_CONTEXT *cx, SV *errsv, int action)
{
    SV  *namesv = NULL; /* init to avoid dumb compiler warning */
    bool do_croak;

    CX_LEAVE_SCOPE(cx);
    do_croak = action && (CxOLD_OP_TYPE(cx) == OP_REQUIRE);
    if (do_croak) {
        /* keep namesv alive after cx_popeval() */
        namesv = cx->blk_eval.old_namesv;
        cx->blk_eval.old_namesv = NULL;
        sv_2mortal(namesv);
    }
    cx_popeval(cx);
    cx_popblock(cx);
    CX_POP(cx);

    if (do_croak) {
        const char *fmt;
        HV *inc_hv = GvHVn(PL_incgv);

        if (action == 1) {
            (void)hv_delete_ent(inc_hv, namesv, G_DISCARD, 0);
            fmt = "%" SVf " did not return a true value";
            errsv = namesv;
        }
        else {
            (void)hv_store_ent(inc_hv, namesv, &PL_sv_undef, 0);
            fmt = "%" SVf "Compilation failed in require";
            if (!errsv)
                errsv = newSVpvs_flags("Unknown error\n", SVs_TEMP);
        }

        Perl_croak(aTHX_ fmt, SVfARG(errsv));
    }
}


/* die_unwind(): this is the final destination for the various croak()
 * functions. If we're in an eval, unwind the context and other stacks
 * back to the top-most CXt_EVAL and set $@ to msv; otherwise print msv
 * to STDERR and initiate an exit. Note that if the CXt_EVAL popped back
 * to is a require the exception will be rethrown, as requires don't
 * actually trap exceptions.
 */

void
Perl_die_unwind(pTHX_ SV *msv)
{
    SV *exceptsv = msv;
    U8 in_eval = PL_in_eval;
    PERL_ARGS_ASSERT_DIE_UNWIND;

    if (in_eval) {
        I32 cxix;

        /* We need to keep this SV alive through all the stack unwinding
         * and FREETMPSing below, while ensuing that it doesn't leak
         * if we call out to something which then dies (e.g. sub STORE{die}
         * when unlocalising a tied var). So we do a dance with
         * mortalising and SAVEFREEing.
         */
        if (PL_phase == PERL_PHASE_DESTRUCT) {
            exceptsv = sv_mortalcopy(exceptsv);
        } else {
            exceptsv = sv_2mortal(SvREFCNT_inc_simple_NN(exceptsv));
        }

        /*
         * Historically, perl used to set ERRSV ($@) early in the die
         * process and rely on it not getting clobbered during unwinding.
         * That sucked, because it was liable to get clobbered, so the
         * setting of ERRSV used to emit the exception from eval{} has
         * been moved to much later, after unwinding (see just before
         * JMPENV_JUMP below).	However, some modules were relying on the
         * early setting, by examining $@ during unwinding to use it as
         * a flag indicating whether the current unwinding was caused by
         * an exception.  It was never a reliable flag for that purpose,
         * being totally open to false positives even without actual
         * clobberage, but was useful enough for production code to
         * semantically rely on it.
         *
         * We'd like to have a proper introspective interface that
         * explicitly describes the reason for whatever unwinding
         * operations are currently in progress, so that those modules
         * work reliably and $@ isn't further overloaded.  But we don't
         * have one yet.  In its absence, as a stopgap measure, ERRSV is
         * now *additionally* set here, before unwinding, to serve as the
         * (unreliable) flag that it used to.
         *
         * This behaviour is temporary, and should be removed when a
         * proper way to detect exceptional unwinding has been developed.
         * As of 2010-12, the authors of modules relying on the hack
         * are aware of the issue, because the modules failed on
         * perls 5.13.{1..7} which had late setting of $@ without this
         * early-setting hack.
         */
        if (!(in_eval & EVAL_KEEPERR)) {
            /* remove any read-only/magic from the SV, so we don't
               get infinite recursion when setting ERRSV */
            SANE_ERRSV();
            sv_setsv_flags(ERRSV, exceptsv,
                        (SV_GMAGIC|SV_DO_COW_SVSETSV|SV_NOSTEAL));
        }

        if (in_eval & EVAL_KEEPERR) {
            Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "\t(in cleanup) %" SVf,
                           SVfARG(exceptsv));
        }

        while ((cxix = dopoptoeval(cxstack_ix)) < 0
               && PL_curstackinfo->si_prev)
        {
            dounwind(-1);
            POPSTACK;
        }

        if (cxix >= 0) {
            PERL_CONTEXT *cx;
            SV **oldsp;
            U8 gimme;
            JMPENV *restartjmpenv;
            OP *restartop;

            if (cxix < cxstack_ix)
                dounwind(cxix);

            cx = CX_CUR();
            assert(CxTYPE(cx) == CXt_EVAL);

            /* return false to the caller of eval */
            oldsp = PL_stack_base + cx->blk_oldsp;
            gimme = cx->blk_gimme;
            if (gimme == G_SCALAR)
                *++oldsp = &PL_sv_undef;
            PL_stack_sp = oldsp;

            restartjmpenv = cx->blk_eval.cur_top_env;
            restartop     = cx->blk_eval.retop;

            /* We need a FREETMPS here to avoid late-called destructors
             * clobbering $@ *after* we set it below, e.g.
             *    sub DESTROY { eval { die "X" } }
             *    eval { my $x = bless []; die $x = 0, "Y" };
             *    is($@, "Y")
             * Here the clearing of the $x ref mortalises the anon array,
             * which needs to be freed *before* $& is set to "Y",
             * otherwise it gets overwritten with "X".
             *
             * However, the FREETMPS will clobber exceptsv, so preserve it
             * on the savestack for now.
             */
            SAVEFREESV(SvREFCNT_inc_simple_NN(exceptsv));
            FREETMPS;
            /* now we're about to pop the savestack, so re-mortalise it */
            sv_2mortal(SvREFCNT_inc_simple_NN(exceptsv));

            /* Note that unlike pp_entereval, pp_require isn't supposed to
             * trap errors. So if we're a require, after we pop the
             * CXt_EVAL that pp_require pushed, rethrow the error with
             * croak(exceptsv). This is all handled by the call below when
             * action == 2.
             */
            S_pop_eval_context_maybe_croak(aTHX_ cx, exceptsv, 2);

            if (!(in_eval & EVAL_KEEPERR)) {
                SANE_ERRSV();
                sv_setsv(ERRSV, exceptsv);
            }
            PL_restartjmpenv = restartjmpenv;
            PL_restartop = restartop;
            JMPENV_JUMP(3);
            NOT_REACHED; /* NOTREACHED */
        }
    }

    write_to_stderr(exceptsv);
    my_failure_exit();
    NOT_REACHED; /* NOTREACHED */
}

PP(pp_xor)
{
    dSP; dPOPTOPssrl;
    if (SvTRUE_NN(left) != SvTRUE_NN(right))
        RETSETYES;
    else
        RETSETNO;
}

/*

=for apidoc_section $CV

=for apidoc caller_cx

The XSUB-writer's equivalent of L<caller()|perlfunc/caller>.  The
returned C<PERL_CONTEXT> structure can be interrogated to find all the
information returned to Perl by C<caller>.  Note that XSUBs don't get a
stack frame, so C<caller_cx(0, NULL)> will return information for the
immediately-surrounding Perl code.

This function skips over the automatic calls to C<&DB::sub> made on the
behalf of the debugger.  If the stack frame requested was a sub called by
C<DB::sub>, the return value will be the frame for the call to
C<DB::sub>, since that has the correct line number/etc. for the call
site.  If I<dbcxp> is non-C<NULL>, it will be set to a pointer to the
frame for the sub call itself.

=cut
*/

const PERL_CONTEXT *
Perl_caller_cx(pTHX_ I32 count, const PERL_CONTEXT **dbcxp)
{
    I32 cxix = dopopto_cursub();
    const PERL_CONTEXT *cx;
    const PERL_CONTEXT *ccstack = cxstack;
    const PERL_SI *top_si = PL_curstackinfo;

    for (;;) {
        /* we may be in a higher stacklevel, so dig down deeper */
        while (cxix < 0 && top_si->si_type != PERLSI_MAIN) {
            top_si = top_si->si_prev;
            ccstack = top_si->si_cxstack;
            cxix = dopoptosub_at(ccstack, top_si->si_cxix);
        }
        if (cxix < 0)
            return NULL;
        /* caller() should not report the automatic calls to &DB::sub */
        if (PL_DBsub && GvCV(PL_DBsub) && cxix >= 0 &&
                ccstack[cxix].blk_sub.cv == GvCV(PL_DBsub))
            count++;
        if (!count--)
            break;
        cxix = dopoptosub_at(ccstack, cxix - 1);
    }

    cx = &ccstack[cxix];
    if (dbcxp) *dbcxp = cx;

    if (CxTYPE(cx) == CXt_SUB || CxTYPE(cx) == CXt_FORMAT) {
        const I32 dbcxix = dopoptosub_at(ccstack, cxix - 1);
        /* We expect that ccstack[dbcxix] is CXt_SUB, anyway, the
           field below is defined for any cx. */
        /* caller() should not report the automatic calls to &DB::sub */
        if (PL_DBsub && GvCV(PL_DBsub) && dbcxix >= 0 && ccstack[dbcxix].blk_sub.cv == GvCV(PL_DBsub))
            cx = &ccstack[dbcxix];
    }

    return cx;
}

PP(pp_caller)
{
    dSP;
    const PERL_CONTEXT *cx;
    const PERL_CONTEXT *dbcx;
    U8 gimme = GIMME_V;
    const HEK *stash_hek;
    I32 count = 0;
    bool has_arg = MAXARG && TOPs;
    const COP *lcop;

    if (MAXARG) {
      if (has_arg)
        count = POPi;
      else (void)POPs;
    }

    cx = caller_cx(count + cBOOL(PL_op->op_private & OPpOFFBYONE), &dbcx);
    if (!cx) {
        if (gimme != G_LIST) {
            EXTEND(SP, 1);
            RETPUSHUNDEF;
        }
        RETURN;
    }

    CX_DEBUG(cx, "CALLER");
    assert(CopSTASH(cx->blk_oldcop));
    stash_hek = SvTYPE(CopSTASH(cx->blk_oldcop)) == SVt_PVHV
      ? HvNAME_HEK((HV*)CopSTASH(cx->blk_oldcop))
      : NULL;
    if (gimme != G_LIST) {
        EXTEND(SP, 1);
        if (!stash_hek)
            PUSHs(&PL_sv_undef);
        else {
            dTARGET;
            sv_sethek(TARG, stash_hek);
            PUSHs(TARG);
        }
        RETURN;
    }

    EXTEND(SP, 11);

    if (!stash_hek)
        PUSHs(&PL_sv_undef);
    else {
        dTARGET;
        sv_sethek(TARG, stash_hek);
        PUSHTARG;
    }
    mPUSHs(newSVpv(OutCopFILE(cx->blk_oldcop), 0));
    lcop = closest_cop(cx->blk_oldcop, OpSIBLING(cx->blk_oldcop),
                       cx->blk_sub.retop, TRUE);
    if (!lcop)
        lcop = cx->blk_oldcop;
    mPUSHu(CopLINE(lcop));
    if (!has_arg)
        RETURN;
    if (CxTYPE(cx) == CXt_SUB || CxTYPE(cx) == CXt_FORMAT) {
        /* So is ccstack[dbcxix]. */
        if (CvHASGV(dbcx->blk_sub.cv)) {
            PUSHs(cv_name(dbcx->blk_sub.cv, 0, 0));
            PUSHs(boolSV(CxHASARGS(cx)));
        }
        else {
            PUSHs(newSVpvs_flags("(unknown)", SVs_TEMP));
            PUSHs(boolSV(CxHASARGS(cx)));
        }
    }
    else {
        PUSHs(newSVpvs_flags("(eval)", SVs_TEMP));
        PUSHs(&PL_sv_zero);
    }
    gimme = cx->blk_gimme;
    if (gimme == G_VOID)
        PUSHs(&PL_sv_undef);
    else
        PUSHs(boolSV((gimme & G_WANT) == G_LIST));
    if (CxTYPE(cx) == CXt_EVAL) {
        /* eval STRING */
        if (CxOLD_OP_TYPE(cx) == OP_ENTEREVAL) {
            SV *cur_text = cx->blk_eval.cur_text;
            if (SvCUR(cur_text) >= 2) {
                PUSHs(newSVpvn_flags(SvPVX(cur_text), SvCUR(cur_text)-2,
                                     SvUTF8(cur_text)|SVs_TEMP));
            }
            else {
                /* I think this is will always be "", but be sure */
                PUSHs(sv_2mortal(newSVsv(cur_text)));
            }

            PUSHs(&PL_sv_no);
        }
        /* require */
        else if (cx->blk_eval.old_namesv) {
            mPUSHs(newSVsv(cx->blk_eval.old_namesv));
            PUSHs(&PL_sv_yes);
        }
        /* eval BLOCK (try blocks have old_namesv == 0) */
        else {
            PUSHs(&PL_sv_undef);
            PUSHs(&PL_sv_undef);
        }
    }
    else {
        PUSHs(&PL_sv_undef);
        PUSHs(&PL_sv_undef);
    }
    if (CxTYPE(cx) == CXt_SUB && CxHASARGS(cx)
        && CopSTASH_eq(PL_curcop, PL_debstash))
    {
        /* slot 0 of the pad contains the original @_ */
        AV * const ary = MUTABLE_AV(AvARRAY(MUTABLE_AV(
                            PadlistARRAY(CvPADLIST(cx->blk_sub.cv))[
                                cx->blk_sub.olddepth+1]))[0]);
        const SSize_t off = AvARRAY(ary) - AvALLOC(ary);

        Perl_init_dbargs(aTHX);

        if (AvMAX(PL_dbargs) < AvFILLp(ary) + off)
            av_extend(PL_dbargs, AvFILLp(ary) + off);
        if (AvFILLp(ary) + 1 + off)
            Copy(AvALLOC(ary), AvARRAY(PL_dbargs), AvFILLp(ary) + 1 + off, SV*);
        AvFILLp(PL_dbargs) = AvFILLp(ary) + off;
    }
    mPUSHi(CopHINTS_get(cx->blk_oldcop));
    {
        SV * mask ;
        char *old_warnings = cx->blk_oldcop->cop_warnings;

        if  (old_warnings == pWARN_NONE)
            mask = newSVpvn(WARN_NONEstring, WARNsize) ;
        else if (old_warnings == pWARN_STD && (PL_dowarn & G_WARN_ON) == 0)
            mask = &PL_sv_undef ;
        else if (old_warnings == pWARN_ALL ||
                  (old_warnings == pWARN_STD && PL_dowarn & G_WARN_ON)) {
            mask = newSVpvn(WARN_ALLstring, WARNsize) ;
        }
        else
            mask = newSVpvn(old_warnings, RCPV_LEN(old_warnings));
        mPUSHs(mask);
    }

    PUSHs(cx->blk_oldcop->cop_hints_hash ?
          sv_2mortal(newRV_noinc(MUTABLE_SV(cop_hints_2hv(cx->blk_oldcop, 0))))
          : &PL_sv_undef);
    RETURN;
}

PP(pp_reset)
{
    dSP;
    const char * tmps;
    STRLEN len = 0;
    if (MAXARG < 1 || (!TOPs && !POPs)) {
        EXTEND(SP, 1);
        tmps = NULL, len = 0;
    }
    else
        tmps = SvPVx_const(POPs, len);
    sv_resetpvn(tmps, len, CopSTASH(PL_curcop));
    PUSHs(&PL_sv_yes);
    RETURN;
}

/* like pp_nextstate, but used instead when the debugger is active */

PP(pp_dbstate)
{
    PL_curcop = (COP*)PL_op;
    TAINT_NOT;		/* Each statement is presumed innocent */
    PL_stack_sp = PL_stack_base + CX_CUR()->blk_oldsp;
    FREETMPS;

    PERL_ASYNC_CHECK();

    if (PL_op->op_flags & OPf_SPECIAL /* breakpoint */
            || PL_DBsingle_iv || PL_DBsignal_iv || PL_DBtrace_iv)
    {
        dSP;
        PERL_CONTEXT *cx;
        const U8 gimme = G_LIST;
        GV * const gv = PL_DBgv;
        CV * cv = NULL;

        if (gv && isGV_with_GP(gv))
            cv = GvCV(gv);

        if (!cv || (!CvROOT(cv) && !CvXSUB(cv)))
            DIE(aTHX_ "No DB::DB routine defined");

        if (CvDEPTH(cv) >= 1 && !(PL_debug & DEBUG_DB_RECURSE_FLAG))
            /* don't do recursive DB::DB call */
            return NORMAL;

        if (CvISXSUB(cv)) {
            ENTER;
            SAVEI32(PL_debug);
            PL_debug = 0;
            SAVESTACK_POS();
            SAVETMPS;
            PUSHMARK(SP);
            (void)(*CvXSUB(cv))(aTHX_ cv);
            FREETMPS;
            LEAVE;
            return NORMAL;
        }
        else {
            cx = cx_pushblock(CXt_SUB, gimme, SP, PL_savestack_ix);
            cx_pushsub(cx, cv, PL_op->op_next, 0);
            /* OP_DBSTATE's op_private holds hint bits rather than
             * the lvalue-ish flags seen in OP_ENTERSUB. So cancel
             * any CxLVAL() flags that have now been mis-calculated */
            cx->blk_u16 = 0;

            SAVEI32(PL_debug);
            PL_debug = 0;
            SAVESTACK_POS();
            CvDEPTH(cv)++;
            if (CvDEPTH(cv) >= 2)
                pad_push(CvPADLIST(cv), CvDEPTH(cv));
            PAD_SET_CUR_NOSAVE(CvPADLIST(cv), CvDEPTH(cv));
            RETURNOP(CvSTART(cv));
        }
    }
    else
        return NORMAL;
}


PP(pp_enter)
{
    U8 gimme = GIMME_V;

    (void)cx_pushblock(CXt_BLOCK, gimme, PL_stack_sp, PL_savestack_ix);
    return NORMAL;
}


PP(pp_leave)
{
    PERL_CONTEXT *cx;
    SV **oldsp;
    U8 gimme;

    cx = CX_CUR();
    assert(CxTYPE(cx) == CXt_BLOCK);

    if (PL_op->op_flags & OPf_SPECIAL)
        /* fake block should preserve $1 et al; e.g.  /(...)/ while ...; */
        cx->blk_oldpm = PL_curpm;

    oldsp = PL_stack_base + cx->blk_oldsp;
    gimme = cx->blk_gimme;

    if (gimme == G_VOID)
        PL_stack_sp = oldsp;
    else
        leave_adjust_stacks(oldsp, oldsp, gimme,
                                PL_op->op_private & OPpLVALUE ? 3 : 1);

    CX_LEAVE_SCOPE(cx);
    cx_popblock(cx);
    CX_POP(cx);

    return NORMAL;
}

static bool
S_outside_integer(pTHX_ SV *sv)
{
  if (SvOK(sv)) {
    const NV nv = SvNV_nomg(sv);
    if (Perl_isinfnan(nv))
      return TRUE;
#ifdef NV_PRESERVES_UV
    if (nv < (NV)IV_MIN || nv > (NV)IV_MAX)
      return TRUE;
#else
    if (nv <= (NV)IV_MIN)
      return TRUE;
    if ((nv > 0) &&
        ((nv > (NV)UV_MAX ||
          SvUV_nomg(sv) > (UV)IV_MAX)))
      return TRUE;
#endif
  }
  return FALSE;
}

PP(pp_enteriter)
{
    dSP; dMARK;
    PERL_CONTEXT *cx;
    const U8 gimme = GIMME_V;
    void *itervarp; /* GV or pad slot of the iteration variable */
    SV   *itersave; /* the old var in the iterator var slot */
    U8 cxflags = 0;

    if (PL_op->op_targ) {			 /* "my" variable */
        itervarp = &PAD_SVl(PL_op->op_targ);
        itersave = *(SV**)itervarp;
        assert(itersave);
        if (PL_op->op_private & OPpLVAL_INTRO) {        /* for my $x (...) */
            /* the SV currently in the pad slot is never live during
             * iteration (the slot is always aliased to one of the items)
             * so it's always stale */
            SvPADSTALE_on(itersave);
        }
        SvREFCNT_inc_simple_void_NN(itersave);
        cxflags = CXp_FOR_PAD;
    }
    else {
        SV * const sv = POPs;
        itervarp = (void *)sv;
        if (LIKELY(isGV(sv))) {		/* symbol table variable */
            SvREFCNT_inc_simple_void(sv);
            itersave = GvSV(sv);
            SvREFCNT_inc_simple_void(itersave);
            cxflags = CXp_FOR_GV;
            if (PL_op->op_private & OPpITER_DEF)
                cxflags |= CXp_FOR_DEF;
        }
        else {                          /* LV ref: for \$foo (...) */
            assert(SvTYPE(sv) == SVt_PVMG);
            assert(SvMAGIC(sv));
            assert(SvMAGIC(sv)->mg_type == PERL_MAGIC_lvref);
            itersave = NULL;
            cxflags = CXp_FOR_LVREF;
            SvREFCNT_inc_simple_void(sv);
        }
    }
    /* OPpITER_DEF (implicit $_) should only occur with a GV iter var */
    assert((cxflags & CXp_FOR_GV) || !(PL_op->op_private & OPpITER_DEF));

    /* Note that this context is initially set as CXt_NULL. Further on
     * down it's changed to one of the CXt_LOOP_*. Before it's changed,
     * there mustn't be anything in the blk_loop substruct that requires
     * freeing or undoing, in case we die in the meantime. And vice-versa.
     */
    cx = cx_pushblock(cxflags, gimme, MARK, PL_savestack_ix);
    cx_pushloop_for(cx, itervarp, itersave);

    if (PL_op->op_flags & OPf_STACKED) {
        /* OPf_STACKED implies either a single array: for(@), with a
         * single AV on the stack, or a range: for (1..5), with 1 and 5 on
         * the stack */
        SV *maybe_ary = POPs;
        if (SvTYPE(maybe_ary) != SVt_PVAV) {
            /* range */
            dPOPss;
            SV * const right = maybe_ary;
            if (UNLIKELY(cxflags & CXp_FOR_LVREF))
                DIE(aTHX_ "Assigned value is not a reference");
            SvGETMAGIC(sv);
            SvGETMAGIC(right);
            if (RANGE_IS_NUMERIC(sv,right)) {
                cx->cx_type |= CXt_LOOP_LAZYIV;
                if (S_outside_integer(aTHX_ sv) ||
                    S_outside_integer(aTHX_ right))
                    DIE(aTHX_ "Range iterator outside integer range");
                cx->blk_loop.state_u.lazyiv.cur = SvIV_nomg(sv);
                cx->blk_loop.state_u.lazyiv.end = SvIV_nomg(right);
            }
            else {
                cx->cx_type |= CXt_LOOP_LAZYSV;
                cx->blk_loop.state_u.lazysv.cur = newSVsv(sv);
                cx->blk_loop.state_u.lazysv.end = right;
                SvREFCNT_inc_simple_void_NN(right);
                (void) SvPV_force_nolen(cx->blk_loop.state_u.lazysv.cur);
                /* This will do the upgrade to SVt_PV, and warn if the value
                   is uninitialised.  */
                (void) SvPV_nolen_const(right);
                /* Doing this avoids a check every time in pp_iter in pp_hot.c
                   to replace !SvOK() with a pointer to "".  */
                if (!SvOK(right)) {
                    SvREFCNT_dec(right);
                    cx->blk_loop.state_u.lazysv.end = &PL_sv_no;
                }
            }
        }
        else /* SvTYPE(maybe_ary) == SVt_PVAV */ {
            /* for (@array) {} */
            cx->cx_type |= CXt_LOOP_ARY;
            cx->blk_loop.state_u.ary.ary = MUTABLE_AV(maybe_ary);
            SvREFCNT_inc_simple_void_NN(maybe_ary);
            cx->blk_loop.state_u.ary.ix =
                (PL_op->op_private & OPpITER_REVERSED) ?
                AvFILL(cx->blk_loop.state_u.ary.ary) + 1 :
                -1;
        }
        /* EXTEND(SP, 1) not needed in this branch because we just did POPs */
    }
    else { /* iterating over items on the stack */
        cx->cx_type |= CXt_LOOP_LIST;
        cx->blk_oldsp = SP - PL_stack_base;
        cx->blk_loop.state_u.stack.basesp = MARK - PL_stack_base;
        cx->blk_loop.state_u.stack.ix =
            (PL_op->op_private & OPpITER_REVERSED)
                ? cx->blk_oldsp + 1
                : cx->blk_loop.state_u.stack.basesp;
        /* pre-extend stack so pp_iter doesn't have to check every time
         * it pushes yes/no */
        EXTEND(SP, 1);
    }

    RETURN;
}

PP(pp_enterloop)
{
    PERL_CONTEXT *cx;
    const U8 gimme = GIMME_V;

    cx = cx_pushblock(CXt_LOOP_PLAIN, gimme, PL_stack_sp, PL_savestack_ix);
    cx_pushloop_plain(cx);
    return NORMAL;
}


PP(pp_leaveloop)
{
    PERL_CONTEXT *cx;
    U8 gimme;
    SV **base;
    SV **oldsp;

    cx = CX_CUR();
    assert(CxTYPE_is_LOOP(cx));
    oldsp = PL_stack_base + cx->blk_oldsp;
    base = CxTYPE(cx) == CXt_LOOP_LIST
                ? PL_stack_base + cx->blk_loop.state_u.stack.basesp
                : oldsp;
    gimme = cx->blk_gimme;

    if (gimme == G_VOID)
        PL_stack_sp = base;
    else
        leave_adjust_stacks(oldsp, base, gimme,
                                PL_op->op_private & OPpLVALUE ? 3 : 1);

    CX_LEAVE_SCOPE(cx);
    cx_poploop(cx);	/* Stack values are safe: release loop vars ... */
    cx_popblock(cx);
    CX_POP(cx);

    return NORMAL;
}


/* This duplicates most of pp_leavesub, but with additional code to handle
 * return args in lvalue context. It was forked from pp_leavesub to
 * avoid slowing down that function any further.
 *
 * Any changes made to this function may need to be copied to pp_leavesub
 * and vice-versa.
 *
 * also tail-called by pp_return
 */

PP(pp_leavesublv)
{
    U8 gimme;
    PERL_CONTEXT *cx;
    SV **oldsp;
    OP *retop;

    cx = CX_CUR();
    assert(CxTYPE(cx) == CXt_SUB);

    if (CxMULTICALL(cx)) {
        /* entry zero of a stack is always PL_sv_undef, which
         * simplifies converting a '()' return into undef in scalar context */
        assert(PL_stack_sp > PL_stack_base || *PL_stack_base == &PL_sv_undef);
        return 0;
    }

    gimme = cx->blk_gimme;
    oldsp = PL_stack_base + cx->blk_oldsp; /* last arg of previous frame */

    if (gimme == G_VOID)
        PL_stack_sp = oldsp;
    else {
        U8   lval    = CxLVAL(cx);
        bool is_lval = (lval && !(lval & OPpENTERSUB_INARGS));
        const char *what = NULL;

        if (gimme == G_SCALAR) {
            if (is_lval) {
                /* check for bad return arg */
                if (oldsp < PL_stack_sp) {
                    SV *sv = *PL_stack_sp;
                    if ((SvPADTMP(sv) || SvREADONLY(sv))) {
                        what =
                            SvREADONLY(sv) ? (sv == &PL_sv_undef) ? "undef"
                            : "a readonly value" : "a temporary";
                    }
                    else goto ok;
                }
                else {
                    /* sub:lvalue{} will take us here. */
                    what = "undef";
                }
              croak:
                Perl_croak(aTHX_
                          "Can't return %s from lvalue subroutine", what);
            }

          ok:
            leave_adjust_stacks(oldsp, oldsp, gimme, is_lval ? 3 : 2);

            if (lval & OPpDEREF) {
                /* lval_sub()->{...} and similar */
                dSP;
                SvGETMAGIC(TOPs);
                if (!SvOK(TOPs)) {
                    TOPs = vivify_ref(TOPs, CxLVAL(cx) & OPpDEREF);
                }
                PUTBACK;
            }
        }
        else {
            assert(gimme == G_LIST);
            assert (!(lval & OPpDEREF));

            if (is_lval) {
                /* scan for bad return args */
                SV **p;
                for (p = PL_stack_sp; p > oldsp; p--) {
                    SV *sv = *p;
                    /* the PL_sv_undef exception is to allow things like
                     * this to work, where PL_sv_undef acts as 'skip'
                     * placeholder on the LHS of list assigns:
                     *    sub foo :lvalue { undef }
                     *    ($a, undef, foo(), $b) = 1..4;
                     */
                    if (sv != &PL_sv_undef && (SvPADTMP(sv) || SvREADONLY(sv)))
                    {
                        /* Might be flattened array after $#array =  */
                        what = SvREADONLY(sv)
                                ? "a readonly value" : "a temporary";
                        goto croak;
                    }
                }
            }

            leave_adjust_stacks(oldsp, oldsp, gimme, is_lval ? 3 : 2);
        }
    }

    CX_LEAVE_SCOPE(cx);
    cx_popsub(cx);	/* Stack values are safe: release CV and @_ ... */
    cx_popblock(cx);
    retop =  cx->blk_sub.retop;
    CX_POP(cx);

    return retop;
}

static const char *S_defer_blockname(PERL_CONTEXT *cx)
{
    return (cx->cx_type & CXp_FINALLY) ? "finally" : "defer";
}


PP(pp_return)
{
    dSP; dMARK;
    PERL_CONTEXT *cx;
    I32 cxix = dopopto_cursub();

    assert(cxstack_ix >= 0);
    if (cxix < cxstack_ix) {
        I32 i;
        /* Check for  defer { return; } */
        for(i = cxstack_ix; i > cxix; i--) {
            if(CxTYPE(&cxstack[i]) == CXt_DEFER)
                /* diag_listed_as: Can't "%s" out of a "defer" block */
                /* diag_listed_as: Can't "%s" out of a "finally" block */
                Perl_croak(aTHX_ "Can't \"%s\" out of a \"%s\" block",
                        "return", S_defer_blockname(&cxstack[i]));
        }
        if (cxix < 0) {
            if (!(       PL_curstackinfo->si_type == PERLSI_SORT
                  || (   PL_curstackinfo->si_type == PERLSI_MULTICALL
                      && (cxstack[0].cx_type & CXp_SUB_RE_FAKE))
                 )
            )
                DIE(aTHX_ "Can't return outside a subroutine");
            /* We must be in:
             *  a sort block, which is a CXt_NULL not a CXt_SUB;
             *  or a /(?{...})/ block.
             * Handle specially. */
            assert(CxTYPE(&cxstack[0]) == CXt_NULL
                    || (   CxTYPE(&cxstack[0]) == CXt_SUB
                        && (cxstack[0].cx_type & CXp_SUB_RE_FAKE)));
            if (cxstack_ix > 0) {
                /* See comment below about context popping. Since we know
                 * we're scalar and not lvalue, we can preserve the return
                 * value in a simpler fashion than there. */
                SV *sv = *SP;
                assert(cxstack[0].blk_gimme == G_SCALAR);
                if (   (sp != PL_stack_base)
                    && !(SvFLAGS(sv) & (SVs_TEMP|SVs_PADTMP))
                )
                    *SP = sv_mortalcopy(sv);
                dounwind(0);
            }
            /* caller responsible for popping cxstack[0] */
            return 0;
        }

        /* There are contexts that need popping. Doing this may free the
         * return value(s), so preserve them first: e.g. popping the plain
         * loop here would free $x:
         *     sub f {  { my $x = 1; return $x } }
         * We may also need to shift the args down; for example,
         *    for (1,2) { return 3,4 }
         * leaves 1,2,3,4 on the stack. Both these actions will be done by
         * leave_adjust_stacks(), along with freeing any temps. Note that
         * whoever we tail-call (e.g. pp_leaveeval) will also call
         * leave_adjust_stacks(); however, the second call is likely to
         * just see a bunch of SvTEMPs with a ref count of 1, and so just
         * pass them through, rather than copying them again. So this
         * isn't as inefficient as it sounds.
         */
        cx = &cxstack[cxix];
        PUTBACK;
        if (cx->blk_gimme != G_VOID)
            leave_adjust_stacks(MARK, PL_stack_base + cx->blk_oldsp,
                    cx->blk_gimme,
                    CxTYPE(cx) == CXt_SUB && CvLVALUE(cx->blk_sub.cv)
                        ? 3 : 0);
        SPAGAIN;
        dounwind(cxix);
        cx = &cxstack[cxix]; /* CX stack may have been realloced */
    }
    else {
        /* Like in the branch above, we need to handle any extra junk on
         * the stack. But because we're not also popping extra contexts, we
         * don't have to worry about prematurely freeing args. So we just
         * need to do the bare minimum to handle junk, and leave the main
         * arg processing in the function we tail call, e.g. pp_leavesub.
         * In list context we have to splice out the junk; in scalar
         * context we can leave as-is (pp_leavesub will later return the
         * top stack element). But for an  empty arg list, e.g.
         *    for (1,2) { return }
         * we need to set sp = oldsp so that pp_leavesub knows to push
         * &PL_sv_undef onto the stack.
         */
        SV **oldsp;
        cx = &cxstack[cxix];
        oldsp = PL_stack_base + cx->blk_oldsp;
        if (oldsp != MARK) {
            SSize_t nargs = SP - MARK;
            if (nargs) {
                if (cx->blk_gimme == G_LIST) {
                    /* shift return args to base of call stack frame */
                    Move(MARK + 1, oldsp + 1, nargs, SV*);
                    PL_stack_sp  = oldsp + nargs;
                }
            }
            else
                PL_stack_sp  = oldsp;
        }
    }

    /* fall through to a normal exit */
    switch (CxTYPE(cx)) {
    case CXt_EVAL:
        return CxEVALBLOCK(cx)
            ? Perl_pp_leavetry(aTHX)
            : Perl_pp_leaveeval(aTHX);
    case CXt_SUB:
        return CvLVALUE(cx->blk_sub.cv)
            ? Perl_pp_leavesublv(aTHX)
            : Perl_pp_leavesub(aTHX);
    case CXt_FORMAT:
        return Perl_pp_leavewrite(aTHX);
    default:
        DIE(aTHX_ "panic: return, type=%u", (unsigned) CxTYPE(cx));
    }
}

/* find the enclosing loop or labelled loop and dounwind() back to it. */

static PERL_CONTEXT *
S_unwind_loop(pTHX)
{
    I32 cxix;
    if (PL_op->op_flags & OPf_SPECIAL) {
        cxix = dopoptoloop(cxstack_ix);
        if (cxix < 0)
            /* diag_listed_as: Can't "last" outside a loop block */
            Perl_croak(aTHX_ "Can't \"%s\" outside a loop block",
                OP_NAME(PL_op));
    }
    else {
        STRLEN label_len;
        const char * label;
        U32 label_flags;
        SV *sv;

        if (PL_op->op_flags & OPf_STACKED) {
            dSP;
            sv = POPs;
            PUTBACK;
            label       = SvPV(sv, label_len);
            label_flags = SvUTF8(sv);
        }
        else {
            sv          = NULL; /* not needed, but shuts up compiler warn */
            label       = cPVOP->op_pv;
            label_len   = strlen(label);
            label_flags = (cPVOP->op_private & OPpPV_IS_UTF8) ? SVf_UTF8 : 0;
        }

        cxix = dopoptolabel(label, label_len, label_flags);
        if (cxix < 0)
            /* diag_listed_as: Label not found for "last %s" */
            Perl_croak(aTHX_ "Label not found for \"%s %" SVf "\"",
                                       OP_NAME(PL_op),
                                       SVfARG(PL_op->op_flags & OPf_STACKED
                                              && !SvGMAGICAL(sv)
                                              ? sv
                                              : newSVpvn_flags(label,
                                                    label_len,
                                                    label_flags | SVs_TEMP)));
    }
    if (cxix < cxstack_ix) {
        I32 i;
        /* Check for  defer { last ... } etc */
        for(i = cxstack_ix; i > cxix; i--) {
            if(CxTYPE(&cxstack[i]) == CXt_DEFER)
                /* diag_listed_as: Can't "%s" out of a "defer" block */
                /* diag_listed_as: Can't "%s" out of a "finally" block */
                Perl_croak(aTHX_ "Can't \"%s\" out of a \"%s\" block",
                        OP_NAME(PL_op), S_defer_blockname(&cxstack[i]));
        }
        dounwind(cxix);
    }
    return &cxstack[cxix];
}


PP(pp_last)
{
    PERL_CONTEXT *cx;
    OP* nextop;

    cx = S_unwind_loop(aTHX);

    assert(CxTYPE_is_LOOP(cx));
    PL_stack_sp = PL_stack_base
                + (CxTYPE(cx) == CXt_LOOP_LIST
                    ?  cx->blk_loop.state_u.stack.basesp
                    : cx->blk_oldsp
                );

    TAINT_NOT;

    /* Stack values are safe: */
    CX_LEAVE_SCOPE(cx);
    cx_poploop(cx);	/* release loop vars ... */
    cx_popblock(cx);
    nextop = cx->blk_loop.my_op->op_lastop->op_next;
    CX_POP(cx);

    return nextop;
}

PP(pp_next)
{
    PERL_CONTEXT *cx;

    /* if not a bare 'next' in the main scope, search for it */
    cx = CX_CUR();
    if (!((PL_op->op_flags & OPf_SPECIAL) && CxTYPE_is_LOOP(cx)))
        cx = S_unwind_loop(aTHX);

    cx_topblock(cx);
    PL_curcop = cx->blk_oldcop;
    PERL_ASYNC_CHECK();
    return (cx)->blk_loop.my_op->op_nextop;
}

PP(pp_redo)
{
    PERL_CONTEXT *cx = S_unwind_loop(aTHX);
    OP* redo_op = cx->blk_loop.my_op->op_redoop;

    if (redo_op->op_type == OP_ENTER) {
        /* pop one less context to avoid $x being freed in while (my $x..) */
        cxstack_ix++;
        cx = CX_CUR();
        assert(CxTYPE(cx) == CXt_BLOCK);
        redo_op = redo_op->op_next;
    }

    FREETMPS;
    CX_LEAVE_SCOPE(cx);
    cx_topblock(cx);
    PL_curcop = cx->blk_oldcop;
    PERL_ASYNC_CHECK();
    return redo_op;
}

#define UNENTERABLE (OP *)1
#define GOTO_DEPTH 64

STATIC OP *
S_dofindlabel(pTHX_ OP *o, const char *label, STRLEN len, U32 flags, OP **opstack, OP **oplimit)
{
    OP **ops = opstack;
    static const char* const too_deep = "Target of goto is too deeply nested";

    PERL_ARGS_ASSERT_DOFINDLABEL;

    if (ops >= oplimit)
        Perl_croak(aTHX_ "%s", too_deep);
    if (o->op_type == OP_LEAVE ||
        o->op_type == OP_SCOPE ||
        o->op_type == OP_LEAVELOOP ||
        o->op_type == OP_LEAVESUB ||
        o->op_type == OP_LEAVETRY ||
        o->op_type == OP_LEAVEGIVEN)
    {
        *ops++ = cUNOPo->op_first;
    }
    else if (oplimit - opstack < GOTO_DEPTH) {
      if (o->op_flags & OPf_KIDS
          && cUNOPo->op_first->op_type == OP_PUSHMARK) {
        *ops++ = UNENTERABLE;
      }
      else if (o->op_flags & OPf_KIDS && PL_opargs[o->op_type]
          && OP_CLASS(o) != OA_LOGOP
          && o->op_type != OP_LINESEQ
          && o->op_type != OP_SREFGEN
          && o->op_type != OP_ENTEREVAL
          && o->op_type != OP_GLOB
          && o->op_type != OP_RV2CV) {
        OP * const kid = cUNOPo->op_first;
        if (OP_GIMME(kid, 0) != G_SCALAR || OpHAS_SIBLING(kid))
            *ops++ = UNENTERABLE;
      }
    }
    if (ops >= oplimit)
        Perl_croak(aTHX_ "%s", too_deep);
    *ops = 0;
    if (o->op_flags & OPf_KIDS) {
        OP *kid;
        OP * const kid1 = cUNOPo->op_first;
        /* First try all the kids at this level, since that's likeliest. */
        for (kid = cUNOPo->op_first; kid; kid = OpSIBLING(kid)) {
            if (kid->op_type == OP_NEXTSTATE || kid->op_type == OP_DBSTATE) {
                STRLEN kid_label_len;
                U32 kid_label_flags;
                const char *kid_label = CopLABEL_len_flags(kCOP,
                                                    &kid_label_len, &kid_label_flags);
                if (kid_label && (
                    ( (kid_label_flags & SVf_UTF8) != (flags & SVf_UTF8) ) ?
                        (flags & SVf_UTF8)
                            ? (bytes_cmp_utf8(
                                        (const U8*)kid_label, kid_label_len,
                                        (const U8*)label, len) == 0)
                            : (bytes_cmp_utf8(
                                        (const U8*)label, len,
                                        (const U8*)kid_label, kid_label_len) == 0)
                    : ( len == kid_label_len && ((kid_label == label)
                                    || memEQ(kid_label, label, len)))))
                    return kid;
            }
        }
        for (kid = cUNOPo->op_first; kid; kid = OpSIBLING(kid)) {
            bool first_kid_of_binary = FALSE;
            if (kid == PL_lastgotoprobe)
                continue;
            if (kid->op_type == OP_NEXTSTATE || kid->op_type == OP_DBSTATE) {
                if (ops == opstack)
                    *ops++ = kid;
                else if (ops[-1] != UNENTERABLE
                      && (ops[-1]->op_type == OP_NEXTSTATE ||
                          ops[-1]->op_type == OP_DBSTATE))
                    ops[-1] = kid;
                else
                    *ops++ = kid;
            }
            if (kid == kid1 && ops != opstack && ops[-1] == UNENTERABLE) {
                first_kid_of_binary = TRUE;
                ops--;
            }
            if ((o = dofindlabel(kid, label, len, flags, ops, oplimit))) {
                if (kid->op_type == OP_PUSHDEFER)
                    Perl_croak(aTHX_ "Can't \"goto\" into a \"defer\" block");
                return o;
            }
            if (first_kid_of_binary)
                *ops++ = UNENTERABLE;
        }
    }
    *ops = 0;
    return 0;
}


static void
S_check_op_type(pTHX_ OP * const o)
{
    /* Eventually we may want to stack the needed arguments
     * for each op.  For now, we punt on the hard ones. */
    /* XXX This comment seems to me like wishful thinking.  --sprout */
    if (o == UNENTERABLE)
        Perl_croak(aTHX_
                  "Can't \"goto\" into a binary or list expression");
    if (o->op_type == OP_ENTERITER)
        Perl_croak(aTHX_
                  "Can't \"goto\" into the middle of a foreach loop");
    if (o->op_type == OP_ENTERGIVEN)
        Perl_croak(aTHX_
                  "Can't \"goto\" into a \"given\" block");
}

/* also used for: pp_dump() */

PP(pp_goto)
{
    dSP;
    OP *retop = NULL;
    I32 ix;
    PERL_CONTEXT *cx;
    OP *enterops[GOTO_DEPTH];
    const char *label = NULL;
    STRLEN label_len = 0;
    U32 label_flags = 0;
    const bool do_dump = (PL_op->op_type == OP_DUMP);
    static const char* const must_have_label = "goto must have label";

    if (PL_op->op_flags & OPf_STACKED) {
        /* goto EXPR  or  goto &foo */

        SV * const sv = POPs;
        SvGETMAGIC(sv);

        if (SvROK(sv) && SvTYPE(SvRV(sv)) == SVt_PVCV) {
            /* This egregious kludge implements goto &subroutine */
            I32 cxix;
            PERL_CONTEXT *cx;
            CV *cv = MUTABLE_CV(SvRV(sv));
            AV *arg = GvAV(PL_defgv);
            CV *old_cv = NULL;

            while (!CvROOT(cv) && !CvXSUB(cv)) {
                const GV * const gv = CvGV(cv);
                if (gv) {
                    GV *autogv;
                    SV *tmpstr;
                    /* autoloaded stub? */
                    if (cv != GvCV(gv) && (cv = GvCV(gv)))
                        continue;
                    autogv = gv_autoload_pvn(GvSTASH(gv), GvNAME(gv),
                                          GvNAMELEN(gv),
                                          GvNAMEUTF8(gv) ? SVf_UTF8 : 0);
                    if (autogv && (cv = GvCV(autogv)))
                        continue;
                    tmpstr = sv_newmortal();
                    gv_efullname3(tmpstr, gv, NULL);
                    DIE(aTHX_ "Goto undefined subroutine &%" SVf, SVfARG(tmpstr));
                }
                DIE(aTHX_ "Goto undefined subroutine");
            }

            cxix = dopopto_cursub();
            if (cxix < 0) {
                DIE(aTHX_ "Can't goto subroutine outside a subroutine");
            }
            cx  = &cxstack[cxix];
            /* ban goto in eval: see <20050521150056.GC20213@iabyn.com> */
            if (CxTYPE(cx) == CXt_EVAL) {
                if (CxREALEVAL(cx))
                /* diag_listed_as: Can't goto subroutine from an eval-%s */
                    DIE(aTHX_ "Can't goto subroutine from an eval-string");
                else
                /* diag_listed_as: Can't goto subroutine from an eval-%s */
                    DIE(aTHX_ "Can't goto subroutine from an eval-block");
            }
            else if (CxMULTICALL(cx))
                DIE(aTHX_ "Can't goto subroutine from a sort sub (or similar callback)");

            /* Check for  defer { goto &...; } */
            for(ix = cxstack_ix; ix > cxix; ix--) {
                if(CxTYPE(&cxstack[ix]) == CXt_DEFER)
                    /* diag_listed_as: Can't "%s" out of a "defer" block */
                    Perl_croak(aTHX_ "Can't \"%s\" out of a \"%s\" block",
                            "goto", S_defer_blockname(&cxstack[ix]));
            }

            /* First do some returnish stuff. */

            SvREFCNT_inc_simple_void(cv); /* avoid premature free during unwind */
            FREETMPS;
            if (cxix < cxstack_ix) {
                dounwind(cxix);
            }
            cx = CX_CUR();
            cx_topblock(cx);
            SPAGAIN;

            /* protect @_ during save stack unwind. */
            if (arg)
                SvREFCNT_inc_NN(sv_2mortal(MUTABLE_SV(arg)));

            assert(PL_scopestack_ix == cx->blk_oldscopesp);
            CX_LEAVE_SCOPE(cx);

            if (CxTYPE(cx) == CXt_SUB && CxHASARGS(cx)) {
                /* this is part of cx_popsub_args() */
                AV* av = MUTABLE_AV(PAD_SVl(0));
                assert(AvARRAY(MUTABLE_AV(
                    PadlistARRAY(CvPADLIST(cx->blk_sub.cv))[
                            CvDEPTH(cx->blk_sub.cv)])) == PL_curpad);

                /* we are going to donate the current @_ from the old sub
                 * to the new sub. This first part of the donation puts a
                 * new empty AV in the pad[0] slot of the old sub,
                 * unless pad[0] and @_ differ (e.g. if the old sub did
                 * local *_ = []); in which case clear the old pad[0]
                 * array in the usual way */
                if (av == arg || AvREAL(av))
                    clear_defarray(av, av == arg);
                else CLEAR_ARGARRAY(av);
            }

            /* don't restore PL_comppad here. It won't be needed if the
             * sub we're going to is non-XS, but restoring it early then
             * croaking (e.g. the "Goto undefined subroutine" below)
             * means the CX block gets processed again in dounwind,
             * but this time with the wrong PL_comppad */

            /* A destructor called during LEAVE_SCOPE could have undefined
             * our precious cv.  See bug #99850. */
            if (!CvROOT(cv) && !CvXSUB(cv)) {
                const GV * const gv = CvGV(cv);
                if (gv) {
                    SV * const tmpstr = sv_newmortal();
                    gv_efullname3(tmpstr, gv, NULL);
                    DIE(aTHX_ "Goto undefined subroutine &%" SVf,
                               SVfARG(tmpstr));
                }
                DIE(aTHX_ "Goto undefined subroutine");
            }

            if (CxTYPE(cx) == CXt_SUB) {
                CvDEPTH(cx->blk_sub.cv) = cx->blk_sub.olddepth;
                /*on XS calls defer freeing the old CV as it could
                 * prematurely set PL_op to NULL, which could cause
                 * e..g XS subs using GIMME_V to SEGV */
                if (CvISXSUB(cv))
                    old_cv = cx->blk_sub.cv;
                else
                    SvREFCNT_dec_NN(cx->blk_sub.cv);
            }

            /* Now do some callish stuff. */
            if (CvISXSUB(cv)) {
                const SSize_t items = arg ? AvFILL(arg) + 1 : 0;
                const bool m = arg ? cBOOL(SvRMAGICAL(arg)) : 0;
                SV** mark;
                UNOP fake_goto_op;

                ENTER;
                SAVETMPS;
                SAVEFREESV(cv); /* later, undo the 'avoid premature free' hack */
                if (old_cv)
                    SAVEFREESV(old_cv); /* ditto, deferred freeing of old CV */

                /* put GvAV(defgv) back onto stack */
                if (items) {
                    EXTEND(SP, items+1); /* @_ could have been extended. */
                }
                mark = SP;
                if (items) {
                    SSize_t index;
                    bool r = cBOOL(AvREAL(arg));
                    for (index=0; index<items; index++)
                    {
                        SV *sv;
                        if (m) {
                            SV ** const svp = av_fetch(arg, index, 0);
                            sv = svp ? *svp : NULL;
                        }
                        else sv = AvARRAY(arg)[index];
                        SP[index+1] = sv
                            ? r ? SvREFCNT_inc_NN(sv_2mortal(sv)) : sv
                            : sv_2mortal(newSVavdefelem(arg, index, 1));
                    }
                }
                SP += items;
                if (CxTYPE(cx) == CXt_SUB && CxHASARGS(cx)) {
                    /* Restore old @_ */
                    CX_POP_SAVEARRAY(cx);
                }

                retop = cx->blk_sub.retop;
                PL_comppad = cx->blk_sub.prevcomppad;
                PL_curpad = LIKELY(PL_comppad) ? AvARRAY(PL_comppad) : NULL;

                /* Make a temporary a copy of the current GOTO op on the C
                 * stack, but with a modified gimme (we can't modify the
                 * real GOTO op as that's not thread-safe). This allows XS
                 * users of GIMME_V to get the correct calling context,
                 * even though there is no longer a CXt_SUB frame to
                 * provide that information.
                 */
                Copy(PL_op, &fake_goto_op, 1, UNOP);
                fake_goto_op.op_flags =
                                  (fake_goto_op.op_flags & ~OPf_WANT)
                                | (cx->blk_gimme & G_WANT);
                PL_op = (OP*)&fake_goto_op;

                /* XS subs don't have a CXt_SUB, so pop it;
                 * this is a cx_popblock(), less all the stuff we already did
                 * for cx_topblock() earlier */
                PL_curcop = cx->blk_oldcop;
                /* this is cx_popsub, less all the stuff we already did */
                PL_curstackinfo->si_cxsubix = cx->blk_sub.old_cxsubix;

                CX_POP(cx);

                /* Push a mark for the start of arglist */
                PUSHMARK(mark);
                PUTBACK;
                (void)(*CvXSUB(cv))(aTHX_ cv);
                LEAVE;
                goto _return;
            }
            else {
                PADLIST * const padlist = CvPADLIST(cv);

                SAVEFREESV(cv); /* later, undo the 'avoid premature free' hack */

                /* partial unrolled cx_pushsub(): */

                cx->blk_sub.cv = cv;
                cx->blk_sub.olddepth = CvDEPTH(cv);

                CvDEPTH(cv)++;
                SvREFCNT_inc_simple_void_NN(cv);
                if (CvDEPTH(cv) > 1) {
                    if (CvDEPTH(cv) == PERL_SUB_DEPTH_WARN && ckWARN(WARN_RECURSION))
                        sub_crush_depth(cv);
                    pad_push(padlist, CvDEPTH(cv));
                }
                PL_curcop = cx->blk_oldcop;
                PAD_SET_CUR_NOSAVE(padlist, CvDEPTH(cv));
                if (CxHASARGS(cx))
                {
                    /* second half of donating @_ from the old sub to the
                     * new sub: abandon the original pad[0] AV in the
                     * new sub, and replace it with the donated @_.
                     * pad[0] takes ownership of the extra refcount
                     * we gave arg earlier */
                    if (arg) {
                        SvREFCNT_dec(PAD_SVl(0));
                        PAD_SVl(0) = (SV *)arg;
                        SvREFCNT_inc_simple_void_NN(arg);
                    }

                    /* GvAV(PL_defgv) might have been modified on scope
                       exit, so point it at arg again. */
                    if (arg != GvAV(PL_defgv)) {
                        AV * const av = GvAV(PL_defgv);
                        GvAV(PL_defgv) = (AV *)SvREFCNT_inc_simple(arg);
                        SvREFCNT_dec(av);
                    }
                }

                if (PERLDB_SUB) {	/* Checking curstash breaks DProf. */
                    Perl_get_db_sub(aTHX_ NULL, cv);
                    if (PERLDB_GOTO) {
                        CV * const gotocv = get_cvs("DB::goto", 0);
                        if (gotocv) {
                            PUSHMARK( PL_stack_sp );
                            call_sv(MUTABLE_SV(gotocv), G_SCALAR | G_NODEBUG);
                            PL_stack_sp--;
                        }
                    }
                }
                retop = CvSTART(cv);
                goto putback_return;
            }
        }
        else {
            /* goto EXPR */
            label       = SvPV_nomg_const(sv, label_len);
            label_flags = SvUTF8(sv);
        }
    }
    else if (!(PL_op->op_flags & OPf_SPECIAL)) {
        /* goto LABEL  or  dump LABEL */
        label       = cPVOP->op_pv;
        label_flags = (cPVOP->op_private & OPpPV_IS_UTF8) ? SVf_UTF8 : 0;
        label_len   = strlen(label);
    }
    if (!(do_dump || label_len)) DIE(aTHX_ "%s", must_have_label);

    PERL_ASYNC_CHECK();

    if (label_len) {
        OP *gotoprobe = NULL;
        bool leaving_eval = FALSE;
        bool in_block = FALSE;
        bool pseudo_block = FALSE;
        PERL_CONTEXT *last_eval_cx = NULL;

        /* find label */

        PL_lastgotoprobe = NULL;
        *enterops = 0;
        for (ix = cxstack_ix; ix >= 0; ix--) {
            cx = &cxstack[ix];
            switch (CxTYPE(cx)) {
            case CXt_EVAL:
                leaving_eval = TRUE;
                if (!CxEVALBLOCK(cx)) {
                    gotoprobe = (last_eval_cx ?
                                last_eval_cx->blk_eval.old_eval_root :
                                PL_eval_root);
                    last_eval_cx = cx;
                    break;
                }
                /* else fall through */
            case CXt_LOOP_PLAIN:
            case CXt_LOOP_LAZYIV:
            case CXt_LOOP_LAZYSV:
            case CXt_LOOP_LIST:
            case CXt_LOOP_ARY:
            case CXt_GIVEN:
            case CXt_WHEN:
                gotoprobe = OpSIBLING(cx->blk_oldcop);
                break;
            case CXt_SUBST:
                continue;
            case CXt_BLOCK:
                if (ix) {
                    gotoprobe = OpSIBLING(cx->blk_oldcop);
                    in_block = TRUE;
                } else
                    gotoprobe = PL_main_root;
                break;
            case CXt_SUB:
                gotoprobe = CvROOT(cx->blk_sub.cv);
                pseudo_block = cBOOL(CxMULTICALL(cx));
                break;
            case CXt_FORMAT:
            case CXt_NULL:
                DIE(aTHX_ "Can't \"goto\" out of a pseudo block");
            case CXt_DEFER:
                /* diag_listed_as: Can't "%s" out of a "defer" block */
                DIE(aTHX_ "Can't \"%s\" out of a \"%s\" block", "goto", S_defer_blockname(cx));
            default:
                if (ix)
                    DIE(aTHX_ "panic: goto, type=%u, ix=%ld",
                        CxTYPE(cx), (long) ix);
                gotoprobe = PL_main_root;
                break;
            }
            if (gotoprobe) {
                OP *sibl1, *sibl2;

                retop = dofindlabel(gotoprobe, label, label_len, label_flags,
                                    enterops, enterops + GOTO_DEPTH);
                if (retop)
                    break;
                if ( (sibl1 = OpSIBLING(gotoprobe)) &&
                     sibl1->op_type == OP_UNSTACK &&
                     (sibl2 = OpSIBLING(sibl1)))
                {
                    retop = dofindlabel(sibl2,
                                        label, label_len, label_flags, enterops,
                                        enterops + GOTO_DEPTH);
                    if (retop)
                        break;
                }
            }
            if (pseudo_block)
                DIE(aTHX_ "Can't \"goto\" out of a pseudo block");
            PL_lastgotoprobe = gotoprobe;
        }
        if (!retop)
            DIE(aTHX_ "Can't find label %" UTF8f,
                       UTF8fARG(label_flags, label_len, label));

        /* if we're leaving an eval, check before we pop any frames
           that we're not going to punt, otherwise the error
           won't be caught */

        if (leaving_eval && *enterops && enterops[1]) {
            I32 i;
            for (i = 1; enterops[i]; i++)
                S_check_op_type(aTHX_ enterops[i]);
        }

        if (*enterops && enterops[1]) {
            I32 i = enterops[1] != UNENTERABLE
                 && enterops[1]->op_type == OP_ENTER && in_block
                    ? 2
                    : 1;
            if (enterops[i])
                deprecate(WARN_DEPRECATED__GOTO_CONSTRUCT, "Use of \"goto\" to jump into a construct");
        }

        /* pop unwanted frames */

        if (ix < cxstack_ix) {
            if (ix < 0)
                DIE(aTHX_ "panic: docatch: illegal ix=%ld", (long)ix);
            dounwind(ix);
            cx = CX_CUR();
            cx_topblock(cx);
        }

        /* push wanted frames */

        if (*enterops && enterops[1]) {
            OP * const oldop = PL_op;
            ix = enterops[1] != UNENTERABLE
              && enterops[1]->op_type == OP_ENTER && in_block
                   ? 2
                   : 1;
            for (; enterops[ix]; ix++) {
                PL_op = enterops[ix];
                S_check_op_type(aTHX_ PL_op);
                DEBUG_l( Perl_deb(aTHX_ "pp_goto: Entering %s\n",
                                         OP_NAME(PL_op)));
                PL_op->op_ppaddr(aTHX);
            }
            PL_op = oldop;
        }
    }

    if (do_dump) {
#ifdef VMS
        if (!retop) retop = PL_main_start;
#endif
        PL_restartop = retop;
        PL_do_undump = TRUE;

        my_unexec();

        PL_restartop = 0;		/* hmm, must be GNU unexec().. */
        PL_do_undump = FALSE;
    }

    putback_return:
    PL_stack_sp = sp;
    _return:
    PERL_ASYNC_CHECK();
    return retop;
}

PP(pp_exit)
{
    dSP;
    I32 anum;

    if (MAXARG < 1)
        anum = 0;
    else if (!TOPs) {
        anum = 0; (void)POPs;
    }
    else {
        anum = SvIVx(POPs);
#ifdef VMS
        if (anum == 1
         && SvTRUE(cop_hints_fetch_pvs(PL_curcop, "vmsish_exit", 0)))
            anum = 0;
        VMSISH_HUSHED  =
            VMSISH_HUSHED || (PL_curcop->op_private & OPpHUSH_VMSISH);
#endif
    }
    PL_exit_flags |= PERL_EXIT_EXPECTED;
    my_exit(anum);
    PUSHs(&PL_sv_undef);
    RETURN;
}

/* Eval. */

STATIC void
S_save_lines(pTHX_ AV *array, SV *sv)
{
    const char *s = SvPVX_const(sv);
    const char * const send = SvPVX_const(sv) + SvCUR(sv);
    I32 line = 1;

    PERL_ARGS_ASSERT_SAVE_LINES;

    while (s && s < send) {
        const char *t;
        SV * const tmpstr = newSV_type(SVt_PVMG);

        t = (const char *)memchr(s, '\n', send - s);
        if (t)
            t++;
        else
            t = send;

        sv_setpvn_fresh(tmpstr, s, t - s);
        av_store(array, line++, tmpstr);
        s = t;
    }
}

/*
=for apidoc docatch

Interpose, for the current op and RUNOPS loop,

    - a new JMPENV stack catch frame, and
    - an inner RUNOPS loop to run all the remaining ops following the
      current PL_op.

Then handle any exceptions raised while in that loop.
For a caught eval at this level, re-enter the loop with the specified
restart op (i.e. the op following the OP_LEAVETRY etc); otherwise re-throw
the exception.

docatch() is intended to be used like this:

    PP(pp_entertry)
    {
        if (CATCH_GET)
            return docatch(Perl_pp_entertry);

        ... rest of function ...
        return PL_op->op_next;
    }

If a new catch frame isn't needed, the op behaves normally. Otherwise it
calls docatch(), which recursively calls pp_entertry(), this time with
CATCH_GET() false, so the rest of the body of the entertry is run. Then
docatch() calls CALLRUNOPS() which executes all the ops following the
entertry. When the loop finally finishes, control returns to docatch(),
which pops the JMPENV and returns to the parent pp_entertry(), which
itself immediately returns. Note that *all* subsequent ops are run within
the inner RUNOPS loop, not just the body of the eval. For example, in

    sub TIEARRAY { eval {1}; my $x }
    tie @a, "main";

at the point the 'my' is executed, the C stack will look something like:

    #10 main()
    #9  perl_run()              # JMPENV_PUSH level 1 here
    #8  S_run_body()
    #7  Perl_runops_standard()  # main RUNOPS loop
    #6  Perl_pp_tie()
    #5  Perl_call_sv()
    #4  Perl_runops_standard()  # unguarded RUNOPS loop: no new JMPENV
    #3  Perl_pp_entertry()
    #2  S_docatch()             # JMPENV_PUSH level 2 here
    #1  Perl_runops_standard()  # docatch()'s RUNOPs loop
    #0  Perl_pp_padsv()

Basically, any section of the perl core which starts a RUNOPS loop may
make a promise that it will catch any exceptions and restart the loop if
necessary. If it's not prepared to do that (like call_sv() isn't), then
it sets CATCH_GET() to true, so that any later eval-like code knows to
set up a new handler and loop (via docatch()).

See L<perlinterp/"Exception handing"> for further details.

=cut
*/

STATIC OP *
S_docatch(pTHX_ Perl_ppaddr_t firstpp)
{
    int ret;
    OP * const oldop = PL_op;
    dJMPENV;

    assert(CATCH_GET);
    JMPENV_PUSH(ret);
    assert(!CATCH_GET);

    switch (ret) {
    case 0: /* normal flow-of-control return from JMPENV_PUSH */

        /* re-run the current op, this time executing the full body of the
         * pp function */
        PL_op = firstpp(aTHX);
 redo_body:
        if (PL_op) {
            CALLRUNOPS(aTHX);
        }
        break;

    case 3: /* an exception raised within an eval */
        if (PL_restartjmpenv == PL_top_env) {
            /* die caught by an inner eval - continue inner loop */

            if (!PL_restartop)
                break;
            PL_restartjmpenv = NULL;
            PL_op = PL_restartop;
            PL_restartop = 0;
            goto redo_body;
        }
        /* FALLTHROUGH */

    default:
        JMPENV_POP;
        PL_op = oldop;
        JMPENV_JUMP(ret); /* re-throw the exception */
        NOT_REACHED; /* NOTREACHED */
    }
    JMPENV_POP;
    PL_op = oldop;
    return NULL;
}


/*
=for apidoc find_runcv

Locate the CV corresponding to the currently executing sub or eval.
If C<db_seqp> is non_null, skip CVs that are in the DB package and populate
C<*db_seqp> with the cop sequence number at the point that the DB:: code was
entered.  (This allows debuggers to eval in the scope of the breakpoint
rather than in the scope of the debugger itself.)

=cut
*/

CV*
Perl_find_runcv(pTHX_ U32 *db_seqp)
{
    return Perl_find_runcv_where(aTHX_ 0, 0, db_seqp);
}

/* If this becomes part of the API, it might need a better name. */
CV *
Perl_find_runcv_where(pTHX_ U8 cond, IV arg, U32 *db_seqp)
{
    PERL_SI	 *si;
    int		 level = 0;

    if (db_seqp)
        *db_seqp =
            PL_curcop == &PL_compiling
                ? PL_cop_seqmax
                : PL_curcop->cop_seq;

    for (si = PL_curstackinfo; si; si = si->si_prev) {
        I32 ix;
        for (ix = si->si_cxix; ix >= 0; ix--) {
            const PERL_CONTEXT *cx = &(si->si_cxstack[ix]);
            CV *cv = NULL;
            if (CxTYPE(cx) == CXt_SUB || CxTYPE(cx) == CXt_FORMAT) {
                cv = cx->blk_sub.cv;
                /* skip DB:: code */
                if (db_seqp && PL_debstash && CvSTASH(cv) == PL_debstash) {
                    *db_seqp = cx->blk_oldcop->cop_seq;
                    continue;
                }
                if (cx->cx_type & CXp_SUB_RE)
                    continue;
            }
            else if (CxTYPE(cx) == CXt_EVAL && !CxEVALBLOCK(cx))
                cv = cx->blk_eval.cv;
            if (cv) {
                switch (cond) {
                case FIND_RUNCV_padid_eq:
                    if (!CvPADLIST(cv)
                     || CvPADLIST(cv)->xpadl_id != (U32)arg)
                        continue;
                    return cv;
                case FIND_RUNCV_level_eq:
                    if (level++ != arg) continue;
                    /* FALLTHROUGH */
                default:
                    return cv;
                }
            }
        }
    }
    return cond == FIND_RUNCV_padid_eq ? NULL : PL_main_cv;
}


/* S_try_yyparse():
 *
 * Run yyparse() in a setjmp wrapper. Returns:
 *   0: yyparse() successful
 *   1: yyparse() failed
 *   3: yyparse() died
 *
 * This is used to trap Perl_croak() calls that are executed
 * during the compilation process and before the code has been
 * completely compiled. It is expected to be called from
 * doeval_compile() only. The parameter 'caller_op' is
 * only used in DEBUGGING to validate the logic is working
 * correctly.
 *
 * See also try_run_unitcheck().
 *
 */
STATIC int
S_try_yyparse(pTHX_ int gramtype, OP *caller_op)
{
    /* if we die during compilation PL_restartop and PL_restartjmpenv
     * will be set by Perl_die_unwind(). We need to restore their values
     * if that happens as they are intended for the case where the code
     * compiles and dies during execution, not where it dies during
     * compilation. PL_restartop and caller_op->op_next should be the
     * same anyway, and when compilation fails then caller_op->op_next is
     * used as the next op after the compile.
     */
    JMPENV *restartjmpenv = PL_restartjmpenv;
    OP *restartop = PL_restartop;
    dJMPENV;
    int ret;
    PERL_UNUSED_ARG(caller_op); /* only used in debugging builds */

    assert(CxTYPE(CX_CUR()) == CXt_EVAL);
    JMPENV_PUSH(ret);
    switch (ret) {
    case 0:
        ret = yyparse(gramtype) ? 1 : 0;
        break;
    case 3:
        /* yyparse() died and we trapped the error. We need to restore
         * the old PL_restartjmpenv and PL_restartop values. */
        assert(PL_restartop == caller_op->op_next); /* we expect these to match */
        PL_restartjmpenv = restartjmpenv;
        PL_restartop = restartop;
        break;
    default:
        JMPENV_POP;
        JMPENV_JUMP(ret);
        NOT_REACHED; /* NOTREACHED */
    }
    JMPENV_POP;
    return ret;
}

/* S_try_run_unitcheck()
 *
 * Run PL_unitcheckav in a setjmp wrapper via call_list.
 * Returns:
 *   0: unitcheck blocks ran without error
 *   3: a unitcheck block died
 *
 * This is used to trap Perl_croak() calls that are executed
 * during UNITCHECK blocks executed after the compilation
 * process has completed but before the code itself has been
 * executed via the normal run loops. It is expected to be called
 * from doeval_compile() only. The parameter 'caller_op' is
 * only used in DEBUGGING to validate the logic is working
 * correctly.
 *
 * See also try_yyparse().
 */
STATIC int
S_try_run_unitcheck(pTHX_ OP* caller_op)
{
    /* if we die during compilation PL_restartop and PL_restartjmpenv
     * will be set by Perl_die_unwind(). We need to restore their values
     * if that happens as they are intended for the case where the code
     * compiles and dies during execution, not where it dies during
     * compilation. UNITCHECK runs after compilation completes, and
     * if it dies we will execute the PL_restartop anyway via the
     * failed compilation code path. PL_restartop and caller_op->op_next
     * should be the same anyway, and when compilation fails then
     * caller_op->op_next is  used as the next op after the compile.
     */
    JMPENV *restartjmpenv = PL_restartjmpenv;
    OP *restartop = PL_restartop;
    dJMPENV;
    int ret;
    PERL_UNUSED_ARG(caller_op); /* only used in debugging builds */

    assert(CxTYPE(CX_CUR()) == CXt_EVAL);
    JMPENV_PUSH(ret);
    switch (ret) {
    case 0:
        call_list(PL_scopestack_ix, PL_unitcheckav);
        break;
    case 3:
        /* call_list died */
        /* call_list() died and we trapped the error. We should restore
         * the old PL_restartjmpenv and PL_restartop values, as they are
         * used only in the case where the code was actually run.
         * The assert validates that we will still execute the PL_restartop.
         */
        assert(PL_restartop == caller_op->op_next); /* we expect these to match */
        PL_restartjmpenv = restartjmpenv;
        PL_restartop = restartop;
        break;
    default:
        JMPENV_POP;
        JMPENV_JUMP(ret);
        NOT_REACHED; /* NOTREACHED */
    }
    JMPENV_POP;
    return ret;
}

/* Compile a require/do or an eval ''.
 *
 * outside is the lexically enclosing CV (if any) that invoked us.
 * seq     is the current COP scope value.
 * hh      is the saved hints hash, if any.
 *
 * Returns a bool indicating whether the compile was successful; if so,
 * PL_eval_start contains the first op of the compiled code; otherwise,
 * pushes undef.
 *
 * This function is called from two places: pp_require and pp_entereval.
 * These can be distinguished by whether PL_op is entereval.
 */

STATIC bool
S_doeval_compile(pTHX_ U8 gimme, CV* outside, U32 seq, HV *hh)
{
    dSP;
    OP * const saveop = PL_op;
    bool clear_hints = saveop->op_type != OP_ENTEREVAL;
    COP * const oldcurcop = PL_curcop;
    bool in_require = (saveop->op_type == OP_REQUIRE);
    int yystatus;
    CV *evalcv;

    PL_in_eval = (in_require
                  ? (EVAL_INREQUIRE | (PL_in_eval & EVAL_INEVAL))
                  : (EVAL_INEVAL |
                        ((PL_op->op_private & OPpEVAL_RE_REPARSING)
                            ? EVAL_RE_REPARSING : 0)));

    PUSHMARK(SP);

    evalcv = MUTABLE_CV(newSV_type(SVt_PVCV));
    CvEVAL_on(evalcv);
    assert(CxTYPE(CX_CUR()) == CXt_EVAL);
    CX_CUR()->blk_eval.cv = evalcv;
    CX_CUR()->blk_gimme = gimme;

    CvOUTSIDE_SEQ(evalcv) = seq;
    CvOUTSIDE(evalcv) = MUTABLE_CV(SvREFCNT_inc_simple(outside));

    /* set up a scratch pad */

    CvPADLIST_set(evalcv, pad_new(padnew_SAVE));
    PL_op = NULL; /* avoid PL_op and PL_curpad referring to different CVs */


    SAVEMORTALIZESV(evalcv);	/* must remain until end of current statement */

    /* make sure we compile in the right package */

    if (CopSTASH_ne(PL_curcop, PL_curstash)) {
        SAVEGENERICSV(PL_curstash);
        PL_curstash = (HV *)CopSTASH(PL_curcop);
        if (SvTYPE(PL_curstash) != SVt_PVHV) PL_curstash = NULL;
        else {
            SvREFCNT_inc_simple_void(PL_curstash);
            save_item(PL_curstname);
            sv_sethek(PL_curstname, HvNAME_HEK(PL_curstash));
        }
    }
    /* XXX:ajgo do we really need to alloc an AV for begin/checkunit */
    SAVESPTR(PL_beginav);
    PL_beginav = newAV();
    SAVEFREESV(PL_beginav);
    SAVESPTR(PL_unitcheckav);
    PL_unitcheckav = newAV();
    SAVEFREESV(PL_unitcheckav);


    ENTER_with_name("evalcomp");
    SAVESPTR(PL_compcv);
    PL_compcv = evalcv;

    /* try to compile it */

    PL_eval_root = NULL;
    PL_curcop = &PL_compiling;
    if ((saveop->op_type != OP_REQUIRE) && (saveop->op_flags & OPf_SPECIAL))
        PL_in_eval |= EVAL_KEEPERR;
    else
        CLEAR_ERRSV();

    SAVEHINTS();
    if (clear_hints) {
        PL_hints = HINTS_DEFAULT;
        PL_prevailing_version = 0;
        hv_clear(GvHV(PL_hintgv));
        CLEARFEATUREBITS();
    }
    else {
        PL_hints = saveop->op_private & OPpEVAL_COPHH
                     ? oldcurcop->cop_hints : (U32)saveop->op_targ;

        /* making 'use re eval' not be in scope when compiling the
         * qr/mabye_has_runtime_code_block/ ensures that we don't get
         * infinite recursion when S_has_runtime_code() gives a false
         * positive: the second time round, HINT_RE_EVAL isn't set so we
         * don't bother calling S_has_runtime_code() */
        if (PL_in_eval & EVAL_RE_REPARSING)
            PL_hints &= ~HINT_RE_EVAL;

        if (hh) {
            /* SAVEHINTS created a new HV in PL_hintgv, which we need to GC */
            SvREFCNT_dec(GvHV(PL_hintgv));
            GvHV(PL_hintgv) = hh;
            FETCHFEATUREBITSHH(hh);
        }
    }
    SAVECOMPILEWARNINGS();
    if (clear_hints) {
        if (PL_dowarn & G_WARN_ALL_ON)
            PL_compiling.cop_warnings = pWARN_ALL ;
        else if (PL_dowarn & G_WARN_ALL_OFF)
            PL_compiling.cop_warnings = pWARN_NONE ;
        else
            PL_compiling.cop_warnings = pWARN_STD ;
    }
    else {
        PL_compiling.cop_warnings =
            DUP_WARNINGS(oldcurcop->cop_warnings);
        cophh_free(CopHINTHASH_get(&PL_compiling));
        if (Perl_cop_fetch_label(aTHX_ oldcurcop, NULL, NULL)) {
            /* The label, if present, is the first entry on the chain. So rather
               than writing a blank label in front of it (which involves an
               allocation), just use the next entry in the chain.  */
            PL_compiling.cop_hints_hash
                = cophh_copy(oldcurcop->cop_hints_hash->refcounted_he_next);
            /* Check the assumption that this removed the label.  */
            assert(Perl_cop_fetch_label(aTHX_ &PL_compiling, NULL, NULL) == NULL);
        }
        else
            PL_compiling.cop_hints_hash = cophh_copy(oldcurcop->cop_hints_hash);
    }

    CALL_BLOCK_HOOKS(bhk_eval, saveop);

    /* we should never be CATCH_GET true here, as our immediate callers should
     * always handle that case. */
    assert(!CATCH_GET);
    /* compile the code */


    yystatus = (!in_require)
               ? S_try_yyparse(aTHX_ GRAMPROG, saveop)
               : yyparse(GRAMPROG);

    if (yystatus || PL_parser->error_count || !PL_eval_root) {
        PERL_CONTEXT *cx;
        SV *errsv;

        PL_op = saveop;
        if (yystatus != 3) {
            /* note that if yystatus == 3, then the require/eval died during
             * compilation, so the EVAL CX block has already been popped, and
             * various vars restored. This block applies similar steps after
             * the other "failed to compile" cases in yyparse, eg, where
             * yystatus=1, "failed, but did not die". */

            if (!in_require)
                invoke_exception_hook(ERRSV,FALSE);
            if (PL_eval_root) {
                op_free(PL_eval_root);
                PL_eval_root = NULL;
            }
            SP = PL_stack_base + POPMARK;	/* pop original mark */
            cx = CX_CUR();
            assert(CxTYPE(cx) == CXt_EVAL);
            /* If we are in an eval we need to make sure that $SIG{__DIE__}
             * handler is invoked so we simulate that part of the
             * Perl_die_unwind() process. In a require we will croak
             * so it will happen there. */
            /* pop the CXt_EVAL, and if was a require, croak */
            S_pop_eval_context_maybe_croak(aTHX_ cx, ERRSV, 2);
        }

        /* die_unwind() re-croaks when in require, having popped the
         * require EVAL context. So we should never catch a require
         * exception here */
        assert(!in_require);

        errsv = ERRSV;
        if (!*(SvPV_nolen_const(errsv)))
            sv_setpvs(errsv, "Compilation error");


        if (gimme != G_LIST) PUSHs(&PL_sv_undef);
        PUTBACK;
        return FALSE;
    }

    /* Compilation successful. Now clean up */

    LEAVE_with_name("evalcomp");

    CopLINE_set(&PL_compiling, 0);
    SAVEFREEOP(PL_eval_root);
    cv_forget_slab(evalcv);

    DEBUG_x(dump_eval());

    /* Register with debugger: */
    if (PERLDB_INTER && saveop->op_type == OP_REQUIRE) {
        CV * const cv = get_cvs("DB::postponed", 0);
        if (cv) {
            dSP;
            PUSHMARK(SP);
            XPUSHs(MUTABLE_SV(CopFILEGV(&PL_compiling)));
            PUTBACK;
            call_sv(MUTABLE_SV(cv), G_DISCARD);
        }
    }

    if (PL_unitcheckav && av_count(PL_unitcheckav)>0) {
        OP *es = PL_eval_start;
        /* TODO: are we sure we shouldn't do S_try_run_unitcheck()
        * when `in_require` is true? */
        if (in_require) {
            call_list(PL_scopestack_ix, PL_unitcheckav);
        }
        else if (S_try_run_unitcheck(aTHX_ saveop)) {
            /* there was an error! */

            /* Restore PL_OP */
            PL_op = saveop;

            SV *errsv = ERRSV;
            if (!*(SvPV_nolen_const(errsv))) {
                /* This happens when using:
                 * eval qq# UNITCHECK { die "\x00"; } #;
                 */
                sv_setpvs(errsv, "Unit check error");
            }

            if (gimme != G_LIST) PUSHs(&PL_sv_undef);
            PUTBACK;
            return FALSE;
        }
        PL_eval_start = es;
    }

    CvDEPTH(evalcv) = 1;
    SP = PL_stack_base + POPMARK;		/* pop original mark */
    PL_op = saveop;			/* The caller may need it. */
    PL_parser->lex_state = LEX_NOTPARSING;	/* $^S needs this. */

    PUTBACK;
    return TRUE;
}

/* Return NULL if the file doesn't exist or isn't a file;
 * else return PerlIO_openn().
 */

STATIC PerlIO *
S_check_type_and_open(pTHX_ SV *name)
{
    Stat_t st;
    STRLEN len;
    PerlIO * retio;
    const char *p = SvPV_const(name, len);
    int st_rc;

    PERL_ARGS_ASSERT_CHECK_TYPE_AND_OPEN;

    /* checking here captures a reasonable error message when
     * PERL_DISABLE_PMC is true, but when PMC checks are enabled, the
     * user gets a confusing message about looking for the .pmc file
     * rather than for the .pm file so do the check in S_doopen_pm when
     * PMC is on instead of here. S_doopen_pm calls this func.
     * This check prevents a \0 in @INC causing problems.
     */
#ifdef PERL_DISABLE_PMC
    if (!IS_SAFE_PATHNAME(p, len, "require"))
        return NULL;
#endif

    /* on Win32 stat is expensive (it does an open() and close() twice and
       a couple other IO calls), the open will fail with a dir on its own with
       errno EACCES, so only do a stat to separate a dir from a real EACCES
       caused by user perms */
#ifndef WIN32
    st_rc = PerlLIO_stat(p, &st);

    if (st_rc < 0)
        return NULL;
    else {
        int eno;
        if(S_ISBLK(st.st_mode)) {
            eno = EINVAL;
            goto not_file;
        }
        else if(S_ISDIR(st.st_mode)) {
            eno = EISDIR;
            not_file:
            errno = eno;
            return NULL;
        }
    }
#endif

    retio = PerlIO_openn(aTHX_ ":", PERL_SCRIPT_MODE, -1, 0, 0, NULL, 1, &name);
#ifdef WIN32
    /* EACCES stops the INC search early in pp_require to implement
       feature RT #113422 */
    if(!retio && errno == EACCES) { /* exists but probably a directory */
        int eno;
        st_rc = PerlLIO_stat(p, &st);
        if (st_rc >= 0) {
            if(S_ISDIR(st.st_mode))
                eno = EISDIR;
            else if(S_ISBLK(st.st_mode))
                eno = EINVAL;
            else
                eno = EACCES;
            errno = eno;
        }
    }
#endif
    return retio;
}

/* doopen_pm(): return the equivalent of PerlIO_openn() on the given name,
 * but first check for bad names (\0) and non-files.
 * Also if the filename ends in .pm and unless PERL_DISABLE_PMC,
 * try loading Foo.pmc first.
 */
#ifndef PERL_DISABLE_PMC
STATIC PerlIO *
S_doopen_pm(pTHX_ SV *name)
{
    STRLEN namelen;
    const char *p = SvPV_const(name, namelen);

    PERL_ARGS_ASSERT_DOOPEN_PM;

    /* check the name before trying for the .pmc name to avoid the
     * warning referring to the .pmc which the user probably doesn't
     * know or care about
     */
    if (!IS_SAFE_PATHNAME(p, namelen, "require"))
        return NULL;

    if (memENDPs(p, namelen, ".pm")) {
        SV *const pmcsv = sv_newmortal();
        PerlIO * pmcio;

        SvSetSV_nosteal(pmcsv,name);
        sv_catpvs(pmcsv, "c");

        pmcio = check_type_and_open(pmcsv);
        if (pmcio)
            return pmcio;
    }
    return check_type_and_open(name);
}
#else
#  define doopen_pm(name) check_type_and_open(name)
#endif /* !PERL_DISABLE_PMC */

/* require doesn't search in @INC for absolute names, or when the name is
   explicitly relative the current directory: i.e. ./, ../ */
PERL_STATIC_INLINE bool
S_path_is_searchable(const char *name)
{
    PERL_ARGS_ASSERT_PATH_IS_SEARCHABLE;

    if (PERL_FILE_IS_ABSOLUTE(name)
#ifdef WIN32
        || (*name == '.' && ((name[1] == '/' ||
                             (name[1] == '.' && name[2] == '/'))
                         || (name[1] == '\\' ||
                             ( name[1] == '.' && name[2] == '\\')))
            )
#else
        || (*name == '.' && (name[1] == '/' ||
                             (name[1] == '.' && name[2] == '/')))
#endif
         )
    {
        return FALSE;
    }
    else
        return TRUE;
}


/* implement 'require 5.010001' */

static OP *
S_require_version(pTHX_ SV *sv)
{
    dSP;

    sv = sv_2mortal(new_version(sv));
    if (!Perl_sv_derived_from_pvn(aTHX_ PL_patchlevel, STR_WITH_LEN("version"), 0))
        upg_version(PL_patchlevel, TRUE);
    if (cUNOP->op_first->op_type == OP_CONST && cUNOP->op_first->op_private & OPpCONST_NOVER) {
        if ( vcmp(sv,PL_patchlevel) <= 0 )
            DIE(aTHX_ "Perls since %" SVf " too modern--this is %" SVf ", stopped",
                SVfARG(sv_2mortal(vnormal(sv))),
                SVfARG(sv_2mortal(vnormal(PL_patchlevel)))
            );
    }
    else {
        if ( vcmp(sv,PL_patchlevel) > 0 ) {
            I32 first = 0;
            AV *lav;
            SV * const req = SvRV(sv);
            SV * const pv = *hv_fetchs(MUTABLE_HV(req), "original", FALSE);

            /* get the left hand term */
            lav = MUTABLE_AV(SvRV(*hv_fetchs(MUTABLE_HV(req), "version", FALSE)));

            first  = SvIV(*av_fetch(lav,0,0));
            if (   first > (int)PERL_REVISION    /* probably 'use 6.0' */
                || hv_exists(MUTABLE_HV(req), "qv", 2 ) /* qv style */
                || av_count(lav) > 2             /* FP with > 3 digits */
                || strstr(SvPVX(pv),".0")        /* FP with leading 0 */
               ) {
                DIE(aTHX_ "Perl %" SVf " required--this is only "
                    "%" SVf ", stopped",
                    SVfARG(sv_2mortal(vnormal(req))),
                    SVfARG(sv_2mortal(vnormal(PL_patchlevel)))
                );
            }
            else { /* probably 'use 5.10' or 'use 5.8' */
                SV *hintsv;
                I32 second = 0;

                if (av_count(lav) > 1)
                    second = SvIV(*av_fetch(lav,1,0));

                second /= second >= 600  ? 100 : 10;
                hintsv = Perl_newSVpvf(aTHX_ "v%d.%d.0",
                                       (int)first, (int)second);
                upg_version(hintsv, TRUE);

                DIE(aTHX_ "Perl %" SVf " required (did you mean %" SVf "?)"
                    "--this is only %" SVf ", stopped",
                    SVfARG(sv_2mortal(vnormal(req))),
                    SVfARG(sv_2mortal(vnormal(sv_2mortal(hintsv)))),
                    SVfARG(sv_2mortal(vnormal(PL_patchlevel)))
                );
            }
        }
    }

    RETPUSHYES;
}

/* Handle C<require Foo::Bar>, C<require "Foo/Bar.pm"> and C<do "Foo.pm">.
 * The first form will have already been converted at compile time to
 * the second form */

static OP *
S_require_file(pTHX_ SV *sv)
{
    dSP;

    PERL_CONTEXT *cx;
    const char *name;
    STRLEN len;
    char * unixname;
    STRLEN unixlen;
#ifdef VMS
    int vms_unixname = 0;
    char *unixdir;
#endif
    /* tryname is the actual pathname (with @INC prefix) which was loaded.
     * It's stored as a value in %INC, and used for error messages */
    const char *tryname = NULL;
    SV *namesv = NULL; /* SV equivalent of tryname */
    const U8 gimme = GIMME_V;
    int filter_has_file = 0;
    PerlIO *tryrsfp = NULL;
    SV *filter_cache = NULL;
    SV *filter_state = NULL;
    SV *filter_sub = NULL;
    SV *hook_sv = NULL;
    OP *op;
    int saved_errno;
    bool path_searchable;
    I32 old_savestack_ix;
    const bool op_is_require = PL_op->op_type == OP_REQUIRE;
    const char *const op_name = op_is_require ? "require" : "do";
    SV ** svp_cached = NULL;

    assert(op_is_require || PL_op->op_type == OP_DOFILE);

    if (!SvOK(sv))
        DIE(aTHX_ "Missing or undefined argument to %s", op_name);
    name = SvPV_nomg_const(sv, len);
    if (!(name && len > 0 && *name))
        DIE(aTHX_ "Missing or undefined argument to %s", op_name);

    if (
        PL_hook__require__before
        && SvROK(PL_hook__require__before)
        && SvTYPE(SvRV(PL_hook__require__before)) == SVt_PVCV
    ) {
        SV* name_sv = sv_mortalcopy(sv);
        SV *post_hook__require__before_sv = NULL;

        ENTER_with_name("call_PRE_REQUIRE");
        SAVETMPS;
        EXTEND(SP, 1);
        PUSHMARK(SP);
        PUSHs(name_sv); /* always use the object for method calls */
        PUTBACK;
        int count = call_sv(PL_hook__require__before, G_SCALAR);
        SPAGAIN;
        if (count && SvOK(*SP) && SvROK(*SP) && SvTYPE(SvRV(*SP)) == SVt_PVCV)
            post_hook__require__before_sv = SvREFCNT_inc_simple_NN(*SP);
        if (!sv_streq(name_sv,sv)) {
            /* they modified the name argument, so do some sleight of hand */
            name = SvPV_nomg_const(name_sv, len);
            if (!(name && len > 0 && *name))
                DIE(aTHX_ "Missing or undefined argument to %s via %%{^HOOK}{require__before}",
                        op_name);
            sv = SvREFCNT_inc_simple_NN(name_sv);
        }
        FREETMPS;
        LEAVE_with_name("call_PRE_REQUIRE");
        if (post_hook__require__before_sv) {
            MORTALDESTRUCTOR_SV(post_hook__require__before_sv, newSVsv(sv));
        }
    }
    if (
        PL_hook__require__after
        && SvROK(PL_hook__require__after)
        && SvTYPE(SvRV(PL_hook__require__after)) == SVt_PVCV
    ) {
        MORTALDESTRUCTOR_SV(PL_hook__require__after, newSVsv(sv));
    }

#ifndef VMS
        /* try to return earlier (save the SAFE_PATHNAME check) if INC already got the name */
        if (op_is_require) {
                /* can optimize to only perform one single lookup */
                svp_cached = hv_fetch(GvHVn(PL_incgv), (char*) name, len, 0);
                if ( svp_cached && (SvGETMAGIC(*svp_cached), SvOK(*svp_cached)) ) RETPUSHYES;
        }
#endif

    if (!IS_SAFE_PATHNAME(name, len, op_name)) {
        if (!op_is_require) {
            CLEAR_ERRSV();
            RETPUSHUNDEF;
        }
        DIE(aTHX_ "Can't locate %s:   %s",
            pv_escape(newSVpvs_flags("",SVs_TEMP),name,len,len*2,
                      NULL, SvUTF8(sv)?PERL_PV_ESCAPE_UNI:0),
            Strerror(ENOENT));
    }
    TAINT_PROPER(op_name);

    path_searchable = path_is_searchable(name);

#ifdef VMS
    /* The key in the %ENV hash is in the syntax of file passed as the argument
     * usually this is in UNIX format, but sometimes in VMS format, which
     * can result in a module being pulled in more than once.
     * To prevent this, the key must be stored in UNIX format if the VMS
     * name can be translated to UNIX.
     */
    
    if ((unixname =
          tounixspec(name, SvPVX(sv_2mortal(newSVpv("", VMS_MAXRSS-1)))))
         != NULL) {
        unixlen = strlen(unixname);
        vms_unixname = 1;
    }
    else
#endif
    {
        /* if not VMS or VMS name can not be translated to UNIX, pass it
         * through.
         */
        unixname = (char *) name;
        unixlen = len;
    }
    if (op_is_require) {
        /* reuse the previous hv_fetch result if possible */
        SV * const * const svp = svp_cached ? svp_cached : hv_fetch(GvHVn(PL_incgv), unixname, unixlen, 0);
        if ( svp ) {
            /* we already did a get magic if this was cached */
            if (!svp_cached)
                SvGETMAGIC(*svp);
            if (SvOK(*svp))
                RETPUSHYES;
            else
                DIE(aTHX_ "Attempt to reload %s aborted.\n"
                            "Compilation failed in require", unixname);
        }

        /*XXX OPf_KIDS should always be true? -dapm 4/2017 */
        if (PL_op->op_flags & OPf_KIDS) {
            SVOP * const kid = cSVOPx(cUNOP->op_first);

            if (kid->op_type == OP_CONST && (kid->op_private & OPpCONST_BARE)) {
                /* Make sure that a bareword module name (e.g. ::Foo::Bar)
                 * doesn't map to a naughty pathname like /Foo/Bar.pm.
                 * Note that the parser will normally detect such errors
                 * at compile time before we reach here, but
                 * Perl_load_module() can fake up an identical optree
                 * without going near the parser, and being able to put
                 * anything as the bareword. So we include a duplicate set
                 * of checks here at runtime.
                 */
                const STRLEN package_len = len - 3;
                const char slashdot[2] = {'/', '.'};
#ifdef DOSISH
                const char backslashdot[2] = {'\\', '.'};
#endif

                /* Disallow *purported* barewords that map to absolute
                   filenames, filenames relative to the current or parent
                   directory, or (*nix) hidden filenames.  Also sanity check
                   that the generated filename ends .pm  */
                if (!path_searchable || len < 3 || name[0] == '.'
                    || !memEQs(name + package_len, len - package_len, ".pm"))
                    DIE(aTHX_ "Bareword in require maps to disallowed filename \"%" SVf "\"", sv);
                if (memchr(name, 0, package_len)) {
                    /* diag_listed_as: Bareword in require contains "%s" */
                    DIE(aTHX_ "Bareword in require contains \"\\0\"");
                }
                if (ninstr(name, name + package_len, slashdot,
                           slashdot + sizeof(slashdot))) {
                    /* diag_listed_as: Bareword in require contains "%s" */
                    DIE(aTHX_ "Bareword in require contains \"/.\"");
                }
#ifdef DOSISH
                if (ninstr(name, name + package_len, backslashdot,
                           backslashdot + sizeof(backslashdot))) {
                    /* diag_listed_as: Bareword in require contains "%s" */
                    DIE(aTHX_ "Bareword in require contains \"\\.\"");
                }
#endif
            }
        }
    }

    PERL_DTRACE_PROBE_FILE_LOADING(unixname);

    /* Try to locate and open a file, possibly using @INC  */

    /* with "/foo/bar.pm", "./foo.pm" and "../foo/bar.pm", try to load
     * the file directly rather than via @INC ... */
    if (!path_searchable) {
        /* At this point, name is SvPVX(sv)  */
        tryname = name;
        tryrsfp = doopen_pm(sv);
    }

    /* ... but if we fail, still search @INC for code references;
     * these are applied even on non-searchable paths (except
     * if we got EACESS).
     *
     * For searchable paths, just search @INC normally
     */
    AV *inc_checked = (AV*)sv_2mortal((SV*)newAV());
    if (!tryrsfp && !(errno == EACCES && !path_searchable)) {
        SSize_t inc_idx;
#ifdef VMS
        if (vms_unixname)
#endif
        {
            AV *incdir_av = (AV*)sv_2mortal((SV*)newAV());
            SV *nsv = sv; /* non const copy we can change if necessary */
            namesv = newSV_type(SVt_PV);
            AV *inc_ar = GvAVn(PL_incgv);
            SSize_t incdir_continue_inc_idx = -1;

            for (
                inc_idx = 0;
                (AvFILL(incdir_av)>=0 /* we have INCDIR items pending */
                    || inc_idx <= AvFILL(inc_ar));  /* @INC entries remain */
                inc_idx++
            ) {
                SV *dirsv;

                /* do we have any pending INCDIR items? */
                if (AvFILL(incdir_av)>=0) {
                    /* yep, shift it out */
                    dirsv = av_shift(incdir_av);
                    if (AvFILL(incdir_av)<0) {
                        /* incdir is now empty, continue from where
                         * we left off after we process this entry  */
                        inc_idx = incdir_continue_inc_idx;
                    }
                } else {
                    dirsv = *av_fetch(inc_ar, inc_idx, TRUE);
                }

                if (SvGMAGICAL(dirsv)) {
                    SvGETMAGIC(dirsv);
                    dirsv = newSVsv_nomg(dirsv);
                } else {
                    /* on the other hand, since we aren't copying we do need
                     * to increment */
                    SvREFCNT_inc(dirsv);
                }
                if (!SvOK(dirsv))
                    continue;

                av_push(inc_checked, dirsv);

                if (SvROK(dirsv)) {
                    int count;
                    SV **svp;
                    SV *loader = dirsv;
                    UV diruv = PTR2UV(SvRV(dirsv));

                    if (SvTYPE(SvRV(loader)) == SVt_PVAV
                        && !SvOBJECT(SvRV(loader)))
                    {
                        loader = *av_fetch(MUTABLE_AV(SvRV(loader)), 0, TRUE);
                        if (SvGMAGICAL(loader)) {
                            SvGETMAGIC(loader);
                            SV *l = sv_newmortal();
                            sv_setsv_nomg(l, loader);
                            loader = l;
                        }
                    }

                    if (SvPADTMP(nsv)) {
                        nsv = sv_newmortal();
                        SvSetSV_nosteal(nsv,sv);
                    }

                    const char *method = NULL;
                    bool is_incdir = FALSE;
                    SV * inc_idx_sv = save_scalar(PL_incgv);
                    sv_setiv(inc_idx_sv,inc_idx);
                    if (sv_isobject(loader)) {
                        /* if it is an object and it has an INC method, then
                         * call the method.
                         */
                        HV *pkg = SvSTASH(SvRV(loader));
                        GV * gv = gv_fetchmethod_pvn_flags(pkg, "INC", 3, GV_AUTOLOAD);
                        if (gv && isGV(gv)) {
                            method = "INC";
                        } else {
                            /* no point to autoload here, it would have been found above */
                            gv = gv_fetchmethod_pvn_flags(pkg, "INCDIR", 6, 0);
                            if (gv && isGV(gv)) {
                                method = "INCDIR";
                                is_incdir = TRUE;
                            }
                        }
                        /* But if we have no method, check if this is a
                         * coderef, if it is then we treat it as an
                         * unblessed coderef would be treated: we
                         * execute it. If it is some other and it is in
                         * an array ref wrapper, then really we don't
                         * know what to do with it, (why use the
                         * wrapper?) and we throw an exception to help
                         * debug. If it is not in a wrapper assume it
                         * has an overload and treat it as a string.
                         * Maybe in the future we can detect if it does
                         * have overloading and throw an error if not.
                         */
                        if (!method) {
                            if (SvTYPE(SvRV(loader)) != SVt_PVCV) {
                                if (amagic_applies(loader,string_amg,AMGf_unary))
                                    goto treat_as_string;
                                else {
                                    croak("Can't locate object method \"INC\", nor"
                                          " \"INCDIR\" nor string overload via"
                                          " package %" HvNAMEf_QUOTEDPREFIX " %s"
                                          " in @INC", pkg,
                                          dirsv == loader
                                          ? "in object hook"
                                          : "in object in ARRAY hook"
                                    );
                                }
                            }
                        }
                    }

                    Perl_sv_setpvf(aTHX_ namesv, "/loader/0x%" UVxf "/%s",
                                   diruv, name);
                    tryname = SvPVX_const(namesv);
                    tryrsfp = NULL;

                    ENTER_with_name("call_INC_hook");
                    SAVETMPS;
                    EXTEND(SP, 2 + ((method && (loader != dirsv)) ? 1 : 0));
                    PUSHMARK(SP);
                    PUSHs(method ? loader : dirsv); /* always use the object for method calls */
                    PUSHs(nsv);
                    if (method && (loader != dirsv)) /* add the args array for method calls */
                        PUSHs(dirsv);
                    PUTBACK;
                    if (method) {
                        count = call_method(method, G_LIST|G_EVAL);
                    } else {
                        count = call_sv(loader, G_LIST|G_EVAL);
                    }
                    SPAGAIN;

                    if (count > 0) {
                        int i = 0;
                        SV *arg;

                        SP -= count - 1;

                        if (is_incdir) {
                            /* push the stringified returned items into the
                             * incdir_av array for processing immediately
                             * afterwards. we deliberately stringify or copy
                             * "special" arguments, so that overload logic for
                             * instance applies, but so that the end result is
                             * stable. We speficially do *not* support returning
                             * coderefs from an INCDIR call. */
                            while (count-->0) {
                                arg = SP[i++];
                                SvGETMAGIC(arg);
                                if (!SvOK(arg))
                                    continue;
                                if (SvROK(arg)) {
                                    STRLEN l;
                                    char *pv = SvPV(arg,l);
                                    arg = newSVpvn(pv,l);
                                }
                                else if (SvGMAGICAL(arg)) {
                                    arg = newSVsv_nomg(arg);
                                }
                                else {
                                    SvREFCNT_inc(arg);
                                }
                                av_push(incdir_av, arg);
                            }
                            /* We copy $INC into incdir_continue_inc_idx
                             * so that when we finish processing the items
                             * we just inserted into incdir_av we can continue
                             * as though we had just finished executing the INCDIR
                             * hook. We honour $INC here just like we would for
                             * an INC hook, the hook might have rewritten @INC
                             * at the same time as returning something to us.
                             */
                            inc_idx_sv = GvSVn(PL_incgv);
                            incdir_continue_inc_idx = SvOK(inc_idx_sv)
                                                      ? SvIV(inc_idx_sv) : -1;

                            goto done_hook;
                        }

                        arg = SP[i++];

                        if (SvROK(arg) && (SvTYPE(SvRV(arg)) <= SVt_PVLV)
                            && !isGV_with_GP(SvRV(arg))) {
                            filter_cache = SvRV(arg);

                            if (i < count) {
                                arg = SP[i++];
                            }
                        }

                        if (SvROK(arg) && isGV_with_GP(SvRV(arg))) {
                            arg = SvRV(arg);
                        }

                        if (isGV_with_GP(arg)) {
                            IO * const io = GvIO((const GV *)arg);

                            ++filter_has_file;

                            if (io) {
                                tryrsfp = IoIFP(io);
                                if (IoOFP(io) && IoOFP(io) != IoIFP(io)) {
                                    PerlIO_close(IoOFP(io));
                                }
                                IoIFP(io) = NULL;
                                IoOFP(io) = NULL;
                            }

                            if (i < count) {
                                arg = SP[i++];
                            }
                        }

                        if (SvROK(arg) && SvTYPE(SvRV(arg)) == SVt_PVCV) {
                            filter_sub = arg;
                            SvREFCNT_inc_simple_void_NN(filter_sub);

                            if (i < count) {
                                filter_state = SP[i];
                                SvREFCNT_inc_simple_void(filter_state);
                            }
                        }

                        if (!tryrsfp && (filter_cache || filter_sub)) {
                            tryrsfp = PerlIO_open(BIT_BUCKET,
                                                  PERL_SCRIPT_MODE);
                        }
                        done_hook:
                        SP--;
                    } else {
                        SV *errsv= ERRSV;
                        if (SvTRUE(errsv) && !SvROK(errsv)) {
                            STRLEN l;
                            char *pv= SvPV(errsv,l);
                            /* Heuristic to tell if this error message
                             * includes the standard line number info:
                             * check if the line ends in digit dot newline.
                             * If it does then we add some extra info so
                             * its obvious this is coming from a hook.
                             * If it is a user generated error we try to
                             * leave it alone. l>12 is to ensure the
                             * other checks are in string, but also
                             * accounts for "at ... line 1.\n" to a
                             * certain extent. Really we should check
                             * further, but this is good enough for back
                             * compat I think.
                             */
                            if (l>=12 && pv[l-1] == '\n' && pv[l-2] == '.' && isDIGIT(pv[l-3]))
                                sv_catpvf(errsv, "%s %s hook died--halting @INC search",
                                          method ? method : "INC",
                                          method ? "method" : "sub");
                            croak_sv(errsv);
                        }
                    }

                    /* FREETMPS may free our filter_cache */
                    SvREFCNT_inc_simple_void(filter_cache);

                    /*
                     Let the hook override which @INC entry we visit
                     next by setting $INC to a different value than it
                     was before we called the hook. If they have
                     completely rewritten the array they might want us
                     to start traversing from the beginning, which is
                     represented by -1. We use undef as an equivalent of
                     -1. This can't be used as a way to call a hook
                     twice, as we still dedupe.
                     We have to do this before we LEAVE, as we localized
                     $INC before we called the hook.
                    */
                    inc_idx_sv = GvSVn(PL_incgv);
                    inc_idx = SvOK(inc_idx_sv) ? SvIV(inc_idx_sv) : -1;

                    PUTBACK;
                    FREETMPS;
                    LEAVE_with_name("call_INC_hook");

                    /*
                     It is possible that @INC has been replaced and that inc_ar
                     now points at a freed AV. So we have to refresh it from
                     the GV to be sure.
                    */
                    inc_ar = GvAVn(PL_incgv);

                    /* Now re-mortalize it. */
                    sv_2mortal(filter_cache);

                    /* Adjust file name if the hook has set an %INC entry.
                       This needs to happen after the FREETMPS above.  */
                    svp = hv_fetch(GvHVn(PL_incgv), name, len, 0);
                    /* we have to make sure that the value is not undef
                     * or the empty string, if it is then we should not
                     * set tryname to it as this will break error messages.
                     *
                     * This might happen if an @INC hook evals the module
                     * which was required in the first place and which
                     * triggered the @INC hook, and that eval dies.
                     * See https://github.com/Perl/perl5/issues/20535
                     */
                    if (svp && SvOK(*svp)) {
                        STRLEN len;
                        const char *tmp_pv = SvPV_const(*svp,len);
                        /* we also guard against the deliberate empty string.
                         * We do not guard against '0', if people want to set their
                         * file name to 0 that is up to them. */
                        if (len)
                            tryname = tmp_pv;
                    }

                    if (tryrsfp) {
                        hook_sv = dirsv;
                        break;
                    }

                    filter_has_file = 0;
                    filter_cache = NULL;
                    if (filter_state) {
                        SvREFCNT_dec_NN(filter_state);
                        filter_state = NULL;
                    }
                    if (filter_sub) {
                        SvREFCNT_dec_NN(filter_sub);
                        filter_sub = NULL;
                    }
                }
                else
                    treat_as_string:
                    if (path_searchable) {
                    /* match against a plain @INC element (non-searchable
                     * paths are only matched against refs in @INC) */
                    const char *dir;
                    STRLEN dirlen;
                    if (SvOK(dirsv)) {
                        dir = SvPV_nomg_const(dirsv, dirlen);
                    } else {
                        dir = "";
                        dirlen = 0;
                    }

                    if (!IS_SAFE_SYSCALL(dir, dirlen, "@INC entry", op_name))
                        continue;
#ifdef VMS
                    if ((unixdir =
                          tounixpath(dir, SvPVX(sv_2mortal(newSVpv("", VMS_MAXRSS-1)))))
                         == NULL)
                        continue;
                    sv_setpv(namesv, unixdir);
                    sv_catpv(namesv, unixname);
#else
                    /* The equivalent of		    
                       Perl_sv_setpvf(aTHX_ namesv, "%s/%s", dir, name);
                       but without the need to parse the format string, or
                       call strlen on either pointer, and with the correct
                       allocation up front.  */
                    {
                        char *tmp = SvGROW(namesv, dirlen + len + 2);

                        memcpy(tmp, dir, dirlen);
                        tmp +=dirlen;

                        /* Avoid '<dir>//<file>' */
                        if (!dirlen || *(tmp-1) != '/') {
                            *tmp++ = '/';
                        } else {
                            /* So SvCUR_set reports the correct length below */
                            dirlen--;
                        }

                        /* name came from an SV, so it will have a '\0' at the
                           end that we can copy as part of this memcpy().  */
                        memcpy(tmp, name, len + 1);

                        SvCUR_set(namesv, dirlen + len + 1);
                        SvPOK_on(namesv);
                    }
#endif
                    TAINT_PROPER(op_name);
                    tryname = SvPVX_const(namesv);
                    tryrsfp = doopen_pm(namesv);
                    if (tryrsfp) {
                        if (tryname[0] == '.' && tryname[1] == '/') {
                            ++tryname;
                            while (*++tryname == '/') {}
                        }
                        break;
                    }
                    else if (errno == EMFILE || errno == EACCES) {
                        /* no point in trying other paths if out of handles;
                         * on the other hand, if we couldn't open one of the
                         * files, then going on with the search could lead to
                         * unexpected results; see perl #113422
                         */
                        break;
                    }
                }
            }
        }
    }

    /* at this point we've ether opened a file (tryrsfp) or set errno */

    saved_errno = errno; /* sv_2mortal can realloc things */
    sv_2mortal(namesv);
    if (!tryrsfp) {
        /* we failed; croak if require() or return undef if do() */
        if (op_is_require) {
            if(saved_errno == EMFILE || saved_errno == EACCES) {
                /* diag_listed_as: Can't locate %s */
                DIE(aTHX_ "Can't locate %s:   %s: %s",
                    name, tryname, Strerror(saved_errno));
            } else {
                if (path_searchable) {          /* did we lookup @INC? */
                    SSize_t i;
                    SV *const msg = newSVpvs_flags("", SVs_TEMP);
                    SV *const inc = newSVpvs_flags("", SVs_TEMP);
                    for (i = 0; i <= AvFILL(inc_checked); i++) {
                        SV **svp= av_fetch(inc_checked, i, TRUE);
                        if (!svp || !*svp) continue;
                        sv_catpvs(inc, " ");
                        sv_catsv(inc, *svp);
                    }
                    if (memENDPs(name, len, ".pm")) {
                        const char *e = name + len - (sizeof(".pm") - 1);
                        const char *c;
                        bool utf8 = cBOOL(SvUTF8(sv));

                        /* if the filename, when converted from "Foo/Bar.pm"
                         * form back to Foo::Bar form, makes a valid
                         * package name (i.e. parseable by C<require
                         * Foo::Bar>), then emit a hint.
                         *
                         * this loop is modelled after the one in
                         S_parse_ident */
                        c = name;
                        while (c < e) {
                            if (utf8 && isIDFIRST_utf8_safe(c, e)) {
                                c += UTF8SKIP(c);
                                while (c < e && isIDCONT_utf8_safe(
                                            (const U8*) c, (const U8*) e))
                                    c += UTF8SKIP(c);
                            }
                            else if (isWORDCHAR_A(*c)) {
                                while (c < e && isWORDCHAR_A(*c))
                                    c++;
                            }
                            else if (*c == '/')
                                c++;
                            else
                                break;
                        }

                        if (c == e && isIDFIRST_lazy_if_safe(name, e, utf8)) {
                            sv_catpvs(msg, " (you may need to install the ");
                            for (c = name; c < e; c++) {
                                if (*c == '/') {
                                    sv_catpvs(msg, "::");
                                }
                                else {
                                    sv_catpvn(msg, c, 1);
                                }
                            }
                            sv_catpvs(msg, " module)");
                        }
                    }
                    else if (memENDs(name, len, ".h")) {
                        sv_catpvs(msg, " (change .h to .ph maybe?) (did you run h2ph?)");
                    }
                    else if (memENDs(name, len, ".ph")) {
                        sv_catpvs(msg, " (did you run h2ph?)");
                    }

                    /* diag_listed_as: Can't locate %s */
                    DIE(aTHX_
                        "Can't locate %s in @INC%" SVf " (@INC entries checked:%" SVf ")",
                        name, msg, inc);
                }
            }
            DIE(aTHX_ "Can't locate %s", name);
        }
        else {
#ifdef DEFAULT_INC_EXCLUDES_DOT
            Stat_t st;
            PerlIO *io = NULL;
            dSAVE_ERRNO;
            /* the complication is to match the logic from doopen_pm() so
             * we don't treat do "sda1" as a previously successful "do".
            */
            bool do_warn = namesv && ckWARN_d(WARN_DEPRECATED__DOT_IN_INC)
                && PerlLIO_stat(name, &st) == 0 && !S_ISDIR(st.st_mode) && !S_ISBLK(st.st_mode)
                && (io = PerlIO_openn(aTHX_ ":", PERL_SCRIPT_MODE, -1, 0, 0, NULL, 1, &sv)) != NULL;
            if (io)
                PerlIO_close(io);

            RESTORE_ERRNO;
            if (do_warn) {
                Perl_warner(aTHX_ packWARN(WARN_DEPRECATED__DOT_IN_INC),
                "do \"%s\" failed, '.' is no longer in @INC; "
                "did you mean do \"./%s\"?",
                name, name);
            }
#endif
            CLEAR_ERRSV();
            RETPUSHUNDEF;
        }
    }
    else
        SETERRNO(0, SS_NORMAL);

    /* Update %INC. Assume success here to prevent recursive requirement. */
    /* name is never assigned to again, so len is still strlen(name)  */
    /* Check whether a hook in @INC has already filled %INC */
    if (!hook_sv) {
        (void)hv_store(GvHVn(PL_incgv),
                       unixname, unixlen, newSVpv(tryname,0),0);
    } else {
        /* store the hook in the sv, note we have to *copy* hook_sv,
         * we don't want modifications to it to change @INC - see GH #20577
         */
        SV** const svp = hv_fetch(GvHVn(PL_incgv), unixname, unixlen, 0);
        if (!svp)
            (void)hv_store(GvHVn(PL_incgv),
                           unixname, unixlen, newSVsv(hook_sv), 0 );
    }

    /* Now parse the file */

    old_savestack_ix = PL_savestack_ix;
    SAVECOPFILE_FREE(&PL_compiling);
    CopFILE_set(&PL_compiling, tryname);
    lex_start(NULL, tryrsfp, 0);

    if (filter_sub || filter_cache) {
        /* We can use the SvPV of the filter PVIO itself as our cache, rather
           than hanging another SV from it. In turn, filter_add() optionally
           takes the SV to use as the filter (or creates a new SV if passed
           NULL), so simply pass in whatever value filter_cache has.  */
        SV * const fc = filter_cache ? newSV_type(SVt_NULL) : NULL;
        SV *datasv;
        if (fc) sv_copypv(fc, filter_cache);
        datasv = filter_add(S_run_user_filter, fc);
        IoLINES(datasv) = filter_has_file;
        IoTOP_GV(datasv) = MUTABLE_GV(filter_state);
        IoBOTTOM_GV(datasv) = MUTABLE_GV(filter_sub);
    }

    /* switch to eval mode */
    assert(!CATCH_GET);
    cx = cx_pushblock(CXt_EVAL, gimme, SP, old_savestack_ix);
    cx_pusheval(cx, PL_op->op_next, newSVpv(name, 0));

    SAVECOPLINE(&PL_compiling);
    CopLINE_set(&PL_compiling, 0);

    PUTBACK;

    if (doeval_compile(gimme, NULL, PL_curcop->cop_seq, NULL))
        op = PL_eval_start;
    else
        op = PL_op->op_next;

    PERL_DTRACE_PROBE_FILE_LOADED(unixname);

    return op;
}


/* also used for: pp_dofile() */

PP(pp_require)
{
    /* If a suitable JMPENV catch frame isn't present, call docatch(),
     * which will:
     *   - add such a frame, and
     *   - start a new RUNOPS loop, which will (as the first op to run),
     *     recursively call this pp function again.
     * The main body of this function is then executed by the inner call.
     */
    if (CATCH_GET)
        return docatch(Perl_pp_require);

    {
        dSP;
        SV *sv = POPs;
        SvGETMAGIC(sv);
        PUTBACK;
        return ((SvNIOKp(sv) || SvVOK(sv)) && PL_op->op_type != OP_DOFILE)
            ? S_require_version(aTHX_ sv)
            : S_require_file(aTHX_ sv);
    }
}


/* This is a op added to hold the hints hash for
   pp_entereval. The hash can be modified by the code
   being eval'ed, so we return a copy instead. */

PP(pp_hintseval)
{
    dSP;
    mXPUSHs(MUTABLE_SV(hv_copy_hints_hv(MUTABLE_HV(cSVOP_sv))));
    RETURN;
}


PP(pp_entereval)
{
    dSP;
    PERL_CONTEXT *cx;
    SV *sv;
    U8 gimme;
    U32 was;
    char tbuf[TYPE_DIGITS(long) + 12];
    bool saved_delete;
    char *tmpbuf;
    STRLEN len;
    CV* runcv;
    U32 seq, lex_flags;
    HV *saved_hh;
    bool bytes;
    I32 old_savestack_ix;

    /* If a suitable JMPENV catch frame isn't present, call docatch(),
     * which will:
     *   - add such a frame, and
     *   - start a new RUNOPS loop, which will (as the first op to run),
     *     recursively call this pp function again.
     * The main body of this function is then executed by the inner call.
     */
    if (CATCH_GET)
        return docatch(Perl_pp_entereval);

    assert(!CATCH_GET);

    gimme = GIMME_V;
    was = PL_breakable_sub_gen;
    saved_delete = FALSE;
    tmpbuf = tbuf;
    lex_flags = 0;
    saved_hh = NULL;
    bytes = PL_op->op_private & OPpEVAL_BYTES;

    if (PL_op->op_private & OPpEVAL_HAS_HH) {
        saved_hh = MUTABLE_HV(SvREFCNT_inc(POPs));
    }
    else if (PL_hints & HINT_LOCALIZE_HH || (
                PL_op->op_private & OPpEVAL_COPHH
             && PL_curcop->cop_hints & HINT_LOCALIZE_HH
            )) {
        saved_hh = cop_hints_2hv(PL_curcop, 0);
        hv_magic(saved_hh, NULL, PERL_MAGIC_hints);
    }
    sv = POPs;
    if (!SvPOK(sv)) {
        /* make sure we've got a plain PV (no overload etc) before testing
         * for taint. Making a copy here is probably overkill, but better
         * safe than sorry */
        STRLEN len;
        const char * const p = SvPV_const(sv, len);

        sv = newSVpvn_flags(p, len, SVs_TEMP | SvUTF8(sv));
        lex_flags |= LEX_START_COPIED;

        if (bytes && SvUTF8(sv))
            SvPVbyte_force(sv, len);
    }
    else if (bytes && SvUTF8(sv)) {
        /* Don't modify someone else's scalar */
        STRLEN len;
        sv = newSVsv(sv);
        (void)sv_2mortal(sv);
        SvPVbyte_force(sv,len);
        lex_flags |= LEX_START_COPIED;
    }

    TAINT_IF(SvTAINTED(sv));
    TAINT_PROPER("eval");

    old_savestack_ix = PL_savestack_ix;

    lex_start(sv, NULL, lex_flags | (PL_op->op_private & OPpEVAL_UNICODE
                           ? LEX_IGNORE_UTF8_HINTS
                           : bytes ? LEX_EVALBYTES : LEX_START_SAME_FILTER
                        )
             );

    /* switch to eval mode */

    if (PERLDB_NAMEEVAL && CopLINE(PL_curcop)) {
        SV * const temp_sv = sv_newmortal();
        Perl_sv_setpvf(aTHX_ temp_sv, "_<(eval %lu)[%s:%" LINE_Tf "]",
                       (unsigned long)++PL_evalseq,
                       CopFILE(PL_curcop), CopLINE(PL_curcop));
        tmpbuf = SvPVX(temp_sv);
        len = SvCUR(temp_sv);
    }
    else
        len = my_snprintf(tmpbuf, sizeof(tbuf), "_<(eval %lu)", (unsigned long)++PL_evalseq);
    SAVECOPFILE_FREE(&PL_compiling);
    CopFILE_set(&PL_compiling, tmpbuf+2);
    SAVECOPLINE(&PL_compiling);
    CopLINE_set(&PL_compiling, 1);
    /* special case: an eval '' executed within the DB package gets lexically
     * placed in the first non-DB CV rather than the current CV - this
     * allows the debugger to execute code, find lexicals etc, in the
     * scope of the code being debugged. Passing &seq gets find_runcv
     * to do the dirty work for us */
    runcv = find_runcv(&seq);

    assert(!CATCH_GET);
    cx = cx_pushblock((CXt_EVAL|CXp_REAL), gimme, SP, old_savestack_ix);
    cx_pusheval(cx, PL_op->op_next, NULL);

    /* prepare to compile string */

    if (PERLDB_LINE_OR_SAVESRC && PL_curstash != PL_debstash)
        save_lines(CopFILEAV(&PL_compiling), PL_parser->linestr);
    else {
        /* XXX For C<eval "...">s within BEGIN {} blocks, this ends up
           deleting the eval's FILEGV from the stash before gv_check() runs
           (i.e. before run-time proper). To work around the coredump that
           ensues, we always turn GvMULTI_on for any globals that were
           introduced within evals. See force_ident(). GSAR 96-10-12 */
        char *const safestr = savepvn(tmpbuf, len);
        SAVEDELETE(PL_defstash, safestr, len);
        saved_delete = TRUE;
    }
    
    PUTBACK;

    if (doeval_compile(gimme, runcv, seq, saved_hh)) {
        if (was != PL_breakable_sub_gen /* Some subs defined here. */
            ?  PERLDB_LINE_OR_SAVESRC
            :  PERLDB_SAVESRC_NOSUBS) {
            /* Retain the filegv we created.  */
        } else if (!saved_delete) {
            char *const safestr = savepvn(tmpbuf, len);
            SAVEDELETE(PL_defstash, safestr, len);
        }
        return PL_eval_start;
    } else {
        /* We have already left the scope set up earlier thanks to the LEAVE
           in doeval_compile().  */
        if (was != PL_breakable_sub_gen /* Some subs defined here. */
            ?  PERLDB_LINE_OR_SAVESRC
            :  PERLDB_SAVESRC_INVALID) {
            /* Retain the filegv we created.  */
        } else if (!saved_delete) {
            (void)hv_delete(PL_defstash, tmpbuf, len, G_DISCARD);
        }
        if (PL_op->op_private & OPpEVAL_EVALSV)
            /* signal compiletime failure to our eval_sv() caller */
            *++PL_stack_sp = NULL;
        return PL_op->op_next;
    }
}


/* also tail-called by pp_return */

PP(pp_leaveeval)
{
    SV **oldsp;
    U8 gimme;
    PERL_CONTEXT *cx;
    OP *retop;
    int failed;
    bool override_return = FALSE; /* is feature 'module_true' in effect? */
    CV *evalcv;
    bool keep;

    PERL_ASYNC_CHECK();

    cx = CX_CUR();
    assert(CxTYPE(cx) == CXt_EVAL);

    oldsp = PL_stack_base + cx->blk_oldsp;
    gimme = cx->blk_gimme;

    bool is_require= CxOLD_OP_TYPE(cx) == OP_REQUIRE;
    if (is_require) {
        /* We are in an require. Check if use feature 'module_true' is enabled,
         * and if so later on correct any returns from the require. */

        /* we might be called for an OP_LEAVEEVAL or OP_RETURN opcode
         * and the parse tree will look different for either case.
         * so find the right op to check later */
        if (OP_TYPE_IS_OR_WAS(PL_op, OP_RETURN)) {
            if (PL_op->op_flags & OPf_SPECIAL)
                override_return = true;
        }
        else if ((PL_op->op_flags & OPf_KIDS) && OP_TYPE_IS_OR_WAS(PL_op, OP_LEAVEEVAL)){
            COP *old_pl_curcop = PL_curcop;
            OP *check = cUNOPx(PL_op)->op_first;

            /* ok, we found something to check, we need to scan through
             * it and find the last OP_NEXTSTATE it contains and then read the
             * feature state out of the COP data it contains.
             */
            if (check) {
                if (!OP_TYPE_IS(check,OP_STUB)) {
                    const OP *kid = cLISTOPx(check)->op_first;
                    const OP *last_state = NULL;

                    for (; kid; kid = OpSIBLING(kid)) {
                        if (
                               OP_TYPE_IS_OR_WAS(kid, OP_NEXTSTATE)
                            || OP_TYPE_IS_OR_WAS(kid, OP_DBSTATE)
                        ){
                            last_state = kid;
                        }
                    }
                    if (last_state) {
                        PL_curcop = cCOPx(last_state);
                        if (FEATURE_MODULE_TRUE_IS_ENABLED) {
                            override_return = TRUE;
                        }
                    } else {
                        NOT_REACHED; /* NOTREACHED */
                    }
                }
            } else {
                NOT_REACHED; /* NOTREACHED */
            }
            PL_curcop = old_pl_curcop;
        }
    }

    /* we might override this later if 'module_true' is enabled */
    failed =    is_require
             && !(gimme == G_SCALAR
                    ? SvTRUE_NN(*PL_stack_sp)
                    : PL_stack_sp > oldsp);

    if (gimme == G_VOID) {
        PL_stack_sp = oldsp;
        /* free now to avoid late-called destructors clobbering $@ */
        FREETMPS;
    }
    else
        leave_adjust_stacks(oldsp, oldsp, gimme, 0);

    /* the cx_popeval does a leavescope, which frees the optree associated
     * with eval, which if it frees the nextstate associated with
     * PL_curcop, sets PL_curcop to NULL. Which can mess up freeing a
     * regex when running under 'use re Debug' because it needs PL_curcop
     * to get the current hints. So restore it early.
     */
    PL_curcop = cx->blk_oldcop;

    /* grab this value before cx_popeval restores the old PL_in_eval */
    keep = cBOOL(PL_in_eval & EVAL_KEEPERR);
    retop = cx->blk_eval.retop;
    evalcv = cx->blk_eval.cv;
#ifdef DEBUGGING
    assert(CvDEPTH(evalcv) == 1);
#endif
    CvDEPTH(evalcv) = 0;

    if (override_return) {
        /* make sure that we use a standard return when feature 'module_load'
         * is enabled. Returns from require are problematic (consider what happens
         * when it is called twice) */
        if (gimme == G_SCALAR) {
            /* this following is an optimization of POPs()/PUSHs().
             * and does the same thing with less bookkeeping */
            *PL_stack_sp = &PL_sv_yes;
        }
        assert(gimme == G_VOID || gimme == G_SCALAR);
        failed = 0;
    }

    /* pop the CXt_EVAL, and if a require failed, croak */
    S_pop_eval_context_maybe_croak(aTHX_ cx, NULL, failed);

    if (!keep)
        CLEAR_ERRSV();

    return retop;
}

/* Ops that implement try/catch syntax
 * Note the asymmetry here:
 *   pp_entertrycatch does two pushblocks
 *   pp_leavetrycatch pops only the outer one; the inner one is popped by
 *     pp_poptry or by stack-unwind of die within the try block
 */

PP(pp_entertrycatch)
{
    PERL_CONTEXT *cx;
    const U8 gimme = GIMME_V;

    /* If a suitable JMPENV catch frame isn't present, call docatch(),
     * which will:
     *   - add such a frame, and
     *   - start a new RUNOPS loop, which will (as the first op to run),
     *     recursively call this pp function again.
     * The main body of this function is then executed by the inner call.
     */
    if (CATCH_GET)
        return docatch(Perl_pp_entertrycatch);

    assert(!CATCH_GET);

    Perl_pp_enter(aTHX); /* performs cx_pushblock(CXt_BLOCK, ...) */

    save_scalar(PL_errgv);
    CLEAR_ERRSV();

    cx = cx_pushblock((CXt_EVAL|CXp_EVALBLOCK|CXp_TRY), gimme,
            PL_stack_sp, PL_savestack_ix);
    cx_pushtry(cx, cLOGOP->op_other);

    PL_in_eval = EVAL_INEVAL;

    return NORMAL;
}

PP(pp_leavetrycatch)
{
    /* leavetrycatch is leave */
    return Perl_pp_leave(aTHX);
}

PP(pp_poptry)
{
    /* poptry is leavetry */
    return Perl_pp_leavetry(aTHX);
}

PP(pp_catch)
{
    dTARGET;

    save_clearsv(&(PAD_SVl(PL_op->op_targ)));
    sv_setsv(TARG, ERRSV);
    CLEAR_ERRSV();

    return cLOGOP->op_other;
}

/* Common code for Perl_call_sv and Perl_fold_constants, put here to keep it
   close to the related Perl_create_eval_scope.  */
void
Perl_delete_eval_scope(pTHX)
{
    PERL_CONTEXT *cx;
        
    cx = CX_CUR();
    CX_LEAVE_SCOPE(cx);
    cx_popeval(cx);
    cx_popblock(cx);
    CX_POP(cx);
}

/* Common-ish code salvaged from Perl_call_sv and pp_entertry, because it was
   also needed by Perl_fold_constants.  */
void
Perl_create_eval_scope(pTHX_ OP *retop, U32 flags)
{
    PERL_CONTEXT *cx;
    const U8 gimme = GIMME_V;
        
    cx = cx_pushblock((CXt_EVAL|CXp_EVALBLOCK), gimme,
                    PL_stack_sp, PL_savestack_ix);
    cx_pusheval(cx, retop, NULL);

    PL_in_eval = EVAL_INEVAL;
    if (flags & G_KEEPERR)
        PL_in_eval |= EVAL_KEEPERR;
    else
        CLEAR_ERRSV();
    if (flags & G_FAKINGEVAL) {
        PL_eval_root = PL_op; /* Only needed so that goto works right. */
    }
}
    
PP(pp_entertry)
{
    OP *retop = cLOGOP->op_other->op_next;

    /* If a suitable JMPENV catch frame isn't present, call docatch(),
     * which will:
     *   - add such a frame, and
     *   - start a new RUNOPS loop, which will (as the first op to run),
     *     recursively call this pp function again.
     * The main body of this function is then executed by the inner call.
     */
    if (CATCH_GET)
        return docatch(Perl_pp_entertry);

    assert(!CATCH_GET);

    create_eval_scope(retop, 0);

    return PL_op->op_next;
}


/* also tail-called by pp_return */

PP(pp_leavetry)
{
    SV **oldsp;
    U8 gimme;
    PERL_CONTEXT *cx;
    OP *retop;

    PERL_ASYNC_CHECK();

    cx = CX_CUR();
    assert(CxTYPE(cx) == CXt_EVAL);
    oldsp = PL_stack_base + cx->blk_oldsp;
    gimme = cx->blk_gimme;

    if (gimme == G_VOID) {
        PL_stack_sp = oldsp;
        /* free now to avoid late-called destructors clobbering $@ */
        FREETMPS;
    }
    else
        leave_adjust_stacks(oldsp, oldsp, gimme, 1);
    CX_LEAVE_SCOPE(cx);
    cx_popeval(cx);
    cx_popblock(cx);
    retop = CxTRY(cx) ? PL_op->op_next : cx->blk_eval.retop;
    CX_POP(cx);

    CLEAR_ERRSV();
    return retop;
}

PP(pp_entergiven)
{
    dSP;
    PERL_CONTEXT *cx;
    const U8 gimme = GIMME_V;
    SV *origsv = DEFSV;
    SV *newsv = POPs;
    
    assert(!PL_op->op_targ); /* used to be set for lexical $_ */
    GvSV(PL_defgv) = SvREFCNT_inc(newsv);

    cx = cx_pushblock(CXt_GIVEN, gimme, SP, PL_savestack_ix);
    cx_pushgiven(cx, origsv);

    RETURN;
}

PP(pp_leavegiven)
{
    PERL_CONTEXT *cx;
    U8 gimme;
    SV **oldsp;
    PERL_UNUSED_CONTEXT;

    cx = CX_CUR();
    assert(CxTYPE(cx) == CXt_GIVEN);
    oldsp = PL_stack_base + cx->blk_oldsp;
    gimme = cx->blk_gimme;

    if (gimme == G_VOID)
        PL_stack_sp = oldsp;
    else
        leave_adjust_stacks(oldsp, oldsp, gimme, 1);

    CX_LEAVE_SCOPE(cx);
    cx_popgiven(cx);
    cx_popblock(cx);
    CX_POP(cx);

    return NORMAL;
}

/* Helper routines used by pp_smartmatch */
STATIC PMOP *
S_make_matcher(pTHX_ REGEXP *re)
{
    PMOP *matcher = cPMOPx(newPMOP(OP_MATCH, OPf_WANT_SCALAR | OPf_STACKED));

    PERL_ARGS_ASSERT_MAKE_MATCHER;

    PM_SETRE(matcher, ReREFCNT_inc(re));

    SAVEFREEOP((OP *) matcher);
    ENTER_with_name("matcher"); SAVETMPS;
    SAVEOP();
    return matcher;
}

STATIC bool
S_matcher_matches_sv(pTHX_ PMOP *matcher, SV *sv)
{
    dSP;
    bool result;

    PERL_ARGS_ASSERT_MATCHER_MATCHES_SV;
    
    PL_op = (OP *) matcher;
    XPUSHs(sv);
    PUTBACK;
    (void) Perl_pp_match(aTHX);
    SPAGAIN;
    result = SvTRUEx(POPs);
    PUTBACK;

    return result;
}

STATIC void
S_destroy_matcher(pTHX_ PMOP *matcher)
{
    PERL_ARGS_ASSERT_DESTROY_MATCHER;
    PERL_UNUSED_ARG(matcher);

    FREETMPS;
    LEAVE_with_name("matcher");
}

/* Do a smart match */
PP(pp_smartmatch)
{
    DEBUG_M(Perl_deb(aTHX_ "Starting smart match resolution\n"));
    return do_smartmatch(NULL, NULL, 0);
}

/* This version of do_smartmatch() implements the
 * table of smart matches that is found in perlsyn.
 */
STATIC OP *
S_do_smartmatch(pTHX_ HV *seen_this, HV *seen_other, const bool copied)
{
    dSP;
    
    bool object_on_left = FALSE;
    SV *e = TOPs;	/* e is for 'expression' */
    SV *d = TOPm1s;	/* d is for 'default', as in PL_defgv */

    /* Take care only to invoke mg_get() once for each argument.
     * Currently we do this by copying the SV if it's magical. */
    if (d) {
        if (!copied && SvGMAGICAL(d))
            d = sv_mortalcopy(d);
    }
    else
        d = &PL_sv_undef;

    assert(e);
    if (SvGMAGICAL(e))
        e = sv_mortalcopy(e);

    /* First of all, handle overload magic of the rightmost argument */
    if (SvAMAGIC(e)) {
        SV * tmpsv;
        DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Object\n"));
        DEBUG_M(Perl_deb(aTHX_ "        attempting overload\n"));

        tmpsv = amagic_call(d, e, smart_amg, AMGf_noleft);
        if (tmpsv) {
            SPAGAIN;
            (void)POPs;
            SETs(tmpsv);
            RETURN;
        }
        DEBUG_M(Perl_deb(aTHX_ "        failed to run overload method; continuing...\n"));
    }

    SP -= 2;	/* Pop the values */
    PUTBACK;

    /* ~~ undef */
    if (!SvOK(e)) {
        DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-undef\n"));
        if (SvOK(d))
            RETPUSHNO;
        else
            RETPUSHYES;
    }

    if (SvROK(e) && SvOBJECT(SvRV(e)) && (SvTYPE(SvRV(e)) != SVt_REGEXP)) {
        DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Object\n"));
        Perl_croak(aTHX_ "Smart matching a non-overloaded object breaks encapsulation");
    }
    if (SvROK(d) && SvOBJECT(SvRV(d)) && (SvTYPE(SvRV(d)) != SVt_REGEXP))
        object_on_left = TRUE;

    /* ~~ sub */
    if (SvROK(e) && SvTYPE(SvRV(e)) == SVt_PVCV) {
        I32 c;
        if (object_on_left) {
            goto sm_any_sub; /* Treat objects like scalars */
        }
        else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVHV) {
            /* Test sub truth for each key */
            HE *he;
            bool andedresults = TRUE;
            HV *hv = (HV*) SvRV(d);
            I32 numkeys = hv_iterinit(hv);
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Hash-CodeRef\n"));
            if (numkeys == 0)
                RETPUSHYES;
            while ( (he = hv_iternext(hv)) ) {
                DEBUG_M(Perl_deb(aTHX_ "        testing hash key...\n"));
                ENTER_with_name("smartmatch_hash_key_test");
                SAVETMPS;
                PUSHMARK(SP);
                PUSHs(hv_iterkeysv(he));
                PUTBACK;
                c = call_sv(e, G_SCALAR);
                SPAGAIN;
                if (c == 0)
                    andedresults = FALSE;
                else
                    andedresults = SvTRUEx(POPs) && andedresults;
                FREETMPS;
                LEAVE_with_name("smartmatch_hash_key_test");
            }
            if (andedresults)
                RETPUSHYES;
            else
                RETPUSHNO;
        }
        else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVAV) {
            /* Test sub truth for each element */
            Size_t i;
            bool andedresults = TRUE;
            AV *av = (AV*) SvRV(d);
            const Size_t len = av_count(av);
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Array-CodeRef\n"));
            if (len == 0)
                RETPUSHYES;
            for (i = 0; i < len; ++i) {
                SV * const * const svp = av_fetch(av, i, FALSE);
                DEBUG_M(Perl_deb(aTHX_ "        testing array element...\n"));
                ENTER_with_name("smartmatch_array_elem_test");
                SAVETMPS;
                PUSHMARK(SP);
                if (svp)
                    PUSHs(*svp);
                PUTBACK;
                c = call_sv(e, G_SCALAR);
                SPAGAIN;
                if (c == 0)
                    andedresults = FALSE;
                else
                    andedresults = SvTRUEx(POPs) && andedresults;
                FREETMPS;
                LEAVE_with_name("smartmatch_array_elem_test");
            }
            if (andedresults)
                RETPUSHYES;
            else
                RETPUSHNO;
        }
        else {
          sm_any_sub:
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-CodeRef\n"));
            ENTER_with_name("smartmatch_coderef");
            SAVETMPS;
            PUSHMARK(SP);
            PUSHs(d);
            PUTBACK;
            c = call_sv(e, G_SCALAR);
            SPAGAIN;
            if (c == 0)
                PUSHs(&PL_sv_no);
            else if (SvTEMP(TOPs))
                SvREFCNT_inc_void(TOPs);
            FREETMPS;
            LEAVE_with_name("smartmatch_coderef");
            RETURN;
        }
    }
    /* ~~ %hash */
    else if (SvROK(e) && SvTYPE(SvRV(e)) == SVt_PVHV) {
        if (object_on_left) {
            goto sm_any_hash; /* Treat objects like scalars */
        }
        else if (!SvOK(d)) {
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Hash ($a undef)\n"));
            RETPUSHNO;
        }
        else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVHV) {
            /* Check that the key-sets are identical */
            HE *he;
            HV *other_hv = MUTABLE_HV(SvRV(d));
            bool tied;
            bool other_tied;
            U32 this_key_count  = 0,
                other_key_count = 0;
            HV *hv = MUTABLE_HV(SvRV(e));

            DEBUG_M(Perl_deb(aTHX_ "    applying rule Hash-Hash\n"));
            /* Tied hashes don't know how many keys they have. */
            tied = cBOOL(SvTIED_mg((SV*)hv, PERL_MAGIC_tied));
            other_tied = cBOOL(SvTIED_mg((const SV *)other_hv, PERL_MAGIC_tied));
            if (!tied ) {
                if(other_tied) {
                    /* swap HV sides */
                    HV * const temp = other_hv;
                    other_hv = hv;
                    hv = temp;
                    tied = TRUE;
                    other_tied = FALSE;
                }
                else if(HvUSEDKEYS((const HV *) hv) != HvUSEDKEYS(other_hv))
                    RETPUSHNO;
            }

            /* The hashes have the same number of keys, so it suffices
               to check that one is a subset of the other. */
            (void) hv_iterinit(hv);
            while ( (he = hv_iternext(hv)) ) {
                SV *key = hv_iterkeysv(he);

                DEBUG_M(Perl_deb(aTHX_ "        comparing hash key...\n"));
                ++ this_key_count;
                
                if(!hv_exists_ent(other_hv, key, 0)) {
                    (void) hv_iterinit(hv);	/* reset iterator */
                    RETPUSHNO;
                }
            }
            
            if (other_tied) {
                (void) hv_iterinit(other_hv);
                while ( hv_iternext(other_hv) )
                    ++other_key_count;
            }
            else
                other_key_count = HvUSEDKEYS(other_hv);
            
            if (this_key_count != other_key_count)
                RETPUSHNO;
            else
                RETPUSHYES;
        }
        else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVAV) {
            AV * const other_av = MUTABLE_AV(SvRV(d));
            const Size_t other_len = av_count(other_av);
            Size_t i;
            HV *hv = MUTABLE_HV(SvRV(e));

            DEBUG_M(Perl_deb(aTHX_ "    applying rule Array-Hash\n"));
            for (i = 0; i < other_len; ++i) {
                SV ** const svp = av_fetch(other_av, i, FALSE);
                DEBUG_M(Perl_deb(aTHX_ "        checking for key existence...\n"));
                if (svp) {	/* ??? When can this not happen? */
                    if (hv_exists_ent(hv, *svp, 0))
                        RETPUSHYES;
                }
            }
            RETPUSHNO;
        }
        else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_REGEXP) {
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Regex-Hash\n"));
          sm_regex_hash:
            {
                PMOP * const matcher = make_matcher((REGEXP*) SvRV(d));
                HE *he;
                HV *hv = MUTABLE_HV(SvRV(e));

                (void) hv_iterinit(hv);
                while ( (he = hv_iternext(hv)) ) {
                    DEBUG_M(Perl_deb(aTHX_ "        testing key against pattern...\n"));
                    PUTBACK;
                    if (matcher_matches_sv(matcher, hv_iterkeysv(he))) {
                        SPAGAIN;
                        (void) hv_iterinit(hv);
                        destroy_matcher(matcher);
                        RETPUSHYES;
                    }
                    SPAGAIN;
                }
                destroy_matcher(matcher);
                RETPUSHNO;
            }
        }
        else {
          sm_any_hash:
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Hash\n"));
            if (hv_exists_ent(MUTABLE_HV(SvRV(e)), d, 0))
                RETPUSHYES;
            else
                RETPUSHNO;
        }
    }
    /* ~~ @array */
    else if (SvROK(e) && SvTYPE(SvRV(e)) == SVt_PVAV) {
        if (object_on_left) {
            goto sm_any_array; /* Treat objects like scalars */
        }
        else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVHV) {
            AV * const other_av = MUTABLE_AV(SvRV(e));
            const Size_t other_len = av_count(other_av);
            Size_t i;

            DEBUG_M(Perl_deb(aTHX_ "    applying rule Hash-Array\n"));
            for (i = 0; i < other_len; ++i) {
                SV ** const svp = av_fetch(other_av, i, FALSE);

                DEBUG_M(Perl_deb(aTHX_ "        testing for key existence...\n"));
                if (svp) {	/* ??? When can this not happen? */
                    if (hv_exists_ent(MUTABLE_HV(SvRV(d)), *svp, 0))
                        RETPUSHYES;
                }
            }
            RETPUSHNO;
        }
        if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVAV) {
            AV *other_av = MUTABLE_AV(SvRV(d));
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Array-Array\n"));
            if (av_count(MUTABLE_AV(SvRV(e))) != av_count(other_av))
                RETPUSHNO;
            else {
                Size_t i;
                const Size_t other_len = av_count(other_av);

                if (NULL == seen_this) {
                    seen_this = (HV*)newSV_type_mortal(SVt_PVHV);
                }
                if (NULL == seen_other) {
                    seen_other = (HV*)newSV_type_mortal(SVt_PVHV);
                }
                for(i = 0; i < other_len; ++i) {
                    SV * const * const this_elem = av_fetch(MUTABLE_AV(SvRV(e)), i, FALSE);
                    SV * const * const other_elem = av_fetch(other_av, i, FALSE);

                    if (!this_elem || !other_elem) {
                        if ((this_elem && SvOK(*this_elem))
                                || (other_elem && SvOK(*other_elem)))
                            RETPUSHNO;
                    }
                    else if (hv_exists_ent(seen_this,
                                sv_2mortal(newSViv(PTR2IV(*this_elem))), 0) ||
                            hv_exists_ent(seen_other,
                                sv_2mortal(newSViv(PTR2IV(*other_elem))), 0))
                    {
                        if (*this_elem != *other_elem)
                            RETPUSHNO;
                    }
                    else {
                        (void)hv_store_ent(seen_this,
                                sv_2mortal(newSViv(PTR2IV(*this_elem))),
                                &PL_sv_undef, 0);
                        (void)hv_store_ent(seen_other,
                                sv_2mortal(newSViv(PTR2IV(*other_elem))),
                                &PL_sv_undef, 0);
                        PUSHs(*other_elem);
                        PUSHs(*this_elem);
                        
                        PUTBACK;
                        DEBUG_M(Perl_deb(aTHX_ "        recursively comparing array element...\n"));
                        (void) do_smartmatch(seen_this, seen_other, 0);
                        SPAGAIN;
                        DEBUG_M(Perl_deb(aTHX_ "        recursion finished\n"));
                        
                        if (!SvTRUEx(POPs))
                            RETPUSHNO;
                    }
                }
                RETPUSHYES;
            }
        }
        else if (SvROK(d) && SvTYPE(SvRV(d)) == SVt_REGEXP) {
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Regex-Array\n"));
          sm_regex_array:
            {
                PMOP * const matcher = make_matcher((REGEXP*) SvRV(d));
                const Size_t this_len = av_count(MUTABLE_AV(SvRV(e)));
                Size_t i;

                for(i = 0; i < this_len; ++i) {
                    SV * const * const svp = av_fetch(MUTABLE_AV(SvRV(e)), i, FALSE);
                    DEBUG_M(Perl_deb(aTHX_ "        testing element against pattern...\n"));
                    PUTBACK;
                    if (svp && matcher_matches_sv(matcher, *svp)) {
                        SPAGAIN;
                        destroy_matcher(matcher);
                        RETPUSHYES;
                    }
                    SPAGAIN;
                }
                destroy_matcher(matcher);
                RETPUSHNO;
            }
        }
        else if (!SvOK(d)) {
            /* undef ~~ array */
            const Size_t this_len = av_count(MUTABLE_AV(SvRV(e)));
            Size_t i;

            DEBUG_M(Perl_deb(aTHX_ "    applying rule Undef-Array\n"));
            for (i = 0; i < this_len; ++i) {
                SV * const * const svp = av_fetch(MUTABLE_AV(SvRV(e)), i, FALSE);
                DEBUG_M(Perl_deb(aTHX_ "        testing for undef element...\n"));
                if (!svp || !SvOK(*svp))
                    RETPUSHYES;
            }
            RETPUSHNO;
        }
        else {
          sm_any_array:
            {
                Size_t i;
                const Size_t this_len = av_count(MUTABLE_AV(SvRV(e)));

                DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Array\n"));
                for (i = 0; i < this_len; ++i) {
                    SV * const * const svp = av_fetch(MUTABLE_AV(SvRV(e)), i, FALSE);
                    if (!svp)
                        continue;

                    PUSHs(d);
                    PUSHs(*svp);
                    PUTBACK;
                    /* infinite recursion isn't supposed to happen here */
                    DEBUG_M(Perl_deb(aTHX_ "        recursively testing array element...\n"));
                    (void) do_smartmatch(NULL, NULL, 1);
                    SPAGAIN;
                    DEBUG_M(Perl_deb(aTHX_ "        recursion finished\n"));
                    if (SvTRUEx(POPs))
                        RETPUSHYES;
                }
                RETPUSHNO;
            }
        }
    }
    /* ~~ qr// */
    else if (SvROK(e) && SvTYPE(SvRV(e)) == SVt_REGEXP) {
        if (!object_on_left && SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVHV) {
            SV *t = d; d = e; e = t;
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Hash-Regex\n"));
            goto sm_regex_hash;
        }
        else if (!object_on_left && SvROK(d) && SvTYPE(SvRV(d)) == SVt_PVAV) {
            SV *t = d; d = e; e = t;
            DEBUG_M(Perl_deb(aTHX_ "    applying rule Array-Regex\n"));
            goto sm_regex_array;
        }
        else {
            PMOP * const matcher = make_matcher((REGEXP*) SvRV(e));
            bool result;

            DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Regex\n"));
            PUTBACK;
            result = matcher_matches_sv(matcher, d);
            SPAGAIN;
            PUSHs(result ? &PL_sv_yes : &PL_sv_no);
            destroy_matcher(matcher);
            RETURN;
        }
    }
    /* ~~ scalar */
    /* See if there is overload magic on left */
    else if (object_on_left && SvAMAGIC(d)) {
        SV *tmpsv;
        DEBUG_M(Perl_deb(aTHX_ "    applying rule Object-Any\n"));
        DEBUG_M(Perl_deb(aTHX_ "        attempting overload\n"));
        PUSHs(d); PUSHs(e);
        PUTBACK;
        tmpsv = amagic_call(d, e, smart_amg, AMGf_noright);
        if (tmpsv) {
            SPAGAIN;
            (void)POPs;
            SETs(tmpsv);
            RETURN;
        }
        SP -= 2;
        DEBUG_M(Perl_deb(aTHX_ "        failed to run overload method; falling back...\n"));
        goto sm_any_scalar;
    }
    else if (!SvOK(d)) {
        /* undef ~~ scalar ; we already know that the scalar is SvOK */
        DEBUG_M(Perl_deb(aTHX_ "    applying rule undef-Any\n"));
        RETPUSHNO;
    }
    else
  sm_any_scalar:
    if (SvNIOK(e) || (SvPOK(e) && looks_like_number(e) && SvNIOK(d))) {
        DEBUG_M(if (SvNIOK(e))
                    Perl_deb(aTHX_ "    applying rule Any-Num\n");
                else
                    Perl_deb(aTHX_ "    applying rule Num-numish\n");
        );
        /* numeric comparison */
        PUSHs(d); PUSHs(e);
        PUTBACK;
        if (CopHINTS_get(PL_curcop) & HINT_INTEGER)
            (void) Perl_pp_i_eq(aTHX);
        else
            (void) Perl_pp_eq(aTHX);
        SPAGAIN;
        if (SvTRUEx(POPs))
            RETPUSHYES;
        else
            RETPUSHNO;
    }
    
    /* As a last resort, use string comparison */
    DEBUG_M(Perl_deb(aTHX_ "    applying rule Any-Any\n"));
    PUSHs(d); PUSHs(e);
    PUTBACK;
    return Perl_pp_seq(aTHX);
}

PP(pp_enterwhen)
{
    dSP;
    PERL_CONTEXT *cx;
    const U8 gimme = GIMME_V;

    /* This is essentially an optimization: if the match
       fails, we don't want to push a context and then
       pop it again right away, so we skip straight
       to the op that follows the leavewhen.
       RETURNOP calls PUTBACK which restores the stack pointer after the POPs.
    */
    if (!(PL_op->op_flags & OPf_SPECIAL) && !SvTRUEx(POPs)) {
        if (gimme == G_SCALAR)
            PUSHs(&PL_sv_undef);
        RETURNOP(cLOGOP->op_other->op_next);
    }

    cx = cx_pushblock(CXt_WHEN, gimme, SP, PL_savestack_ix);
    cx_pushwhen(cx);

    RETURN;
}

PP(pp_leavewhen)
{
    I32 cxix;
    PERL_CONTEXT *cx;
    U8 gimme;
    SV **oldsp;

    cx = CX_CUR();
    assert(CxTYPE(cx) == CXt_WHEN);
    gimme = cx->blk_gimme;

    cxix = dopoptogivenfor(cxstack_ix);
    if (cxix < 0)
        /* diag_listed_as: Can't "when" outside a topicalizer */
        DIE(aTHX_ "Can't \"%s\" outside a topicalizer",
                   PL_op->op_flags & OPf_SPECIAL ? "default" : "when");

    oldsp = PL_stack_base + cx->blk_oldsp;
    if (gimme == G_VOID)
        PL_stack_sp = oldsp;
    else
        leave_adjust_stacks(oldsp, oldsp, gimme, 1);

    /* pop the WHEN, BLOCK and anything else before the GIVEN/FOR */
    assert(cxix < cxstack_ix);
    dounwind(cxix);

    cx = &cxstack[cxix];

    if (CxFOREACH(cx)) {
        /* emulate pp_next. Note that any stack(s) cleanup will be
         * done by the pp_unstack which op_nextop should point to */
        cx = CX_CUR();
        cx_topblock(cx);
        PL_curcop = cx->blk_oldcop;
        return cx->blk_loop.my_op->op_nextop;
    }
    else {
        PERL_ASYNC_CHECK();
        assert(cx->blk_givwhen.leave_op->op_type == OP_LEAVEGIVEN);
        return cx->blk_givwhen.leave_op;
    }
}

PP(pp_continue)
{
    I32 cxix;
    PERL_CONTEXT *cx;
    OP *nextop;
    
    cxix = dopoptowhen(cxstack_ix); 
    if (cxix < 0)   
        DIE(aTHX_ "Can't \"continue\" outside a when block");

    if (cxix < cxstack_ix)
        dounwind(cxix);
    
    cx = CX_CUR();
    assert(CxTYPE(cx) == CXt_WHEN);
    PL_stack_sp = PL_stack_base + cx->blk_oldsp;
    CX_LEAVE_SCOPE(cx);
    cx_popwhen(cx);
    cx_popblock(cx);
    nextop = cx->blk_givwhen.leave_op->op_next;
    CX_POP(cx);

    return nextop;
}

PP(pp_break)
{
    I32 cxix;
    PERL_CONTEXT *cx;

    cxix = dopoptogivenfor(cxstack_ix);
    if (cxix < 0)
        DIE(aTHX_ "Can't \"break\" outside a given block");

    cx = &cxstack[cxix];
    if (CxFOREACH(cx))
        DIE(aTHX_ "Can't \"break\" in a loop topicalizer");

    if (cxix < cxstack_ix)
        dounwind(cxix);

    /* Restore the sp at the time we entered the given block */
    cx = CX_CUR();
    PL_stack_sp = PL_stack_base + cx->blk_oldsp;

    return cx->blk_givwhen.leave_op;
}

static void
_invoke_defer_block(pTHX_ U8 type, void *_arg)
{
    OP *start = (OP *)_arg;
#ifdef DEBUGGING
    I32 was_cxstack_ix = cxstack_ix;
#endif

    cx_pushblock(type, G_VOID, PL_stack_sp, PL_savestack_ix);
    ENTER;
    SAVETMPS;

    SAVEOP();
    PL_op = start;

    CALLRUNOPS(aTHX);

    FREETMPS;
    LEAVE;

    {
        PERL_CONTEXT *cx;

        cx = CX_CUR();
        assert(CxTYPE(cx) == CXt_DEFER);

        PL_stack_sp = PL_stack_base + cx->blk_oldsp;

        CX_LEAVE_SCOPE(cx);
        cx_popblock(cx);
        CX_POP(cx);
    }

    assert(cxstack_ix == was_cxstack_ix);
}

static void
invoke_defer_block(pTHX_ void *_arg)
{
    _invoke_defer_block(aTHX_ CXt_DEFER, _arg);
}

static void
invoke_finally_block(pTHX_ void *_arg)
{
    _invoke_defer_block(aTHX_ CXt_DEFER|CXp_FINALLY, _arg);
}

PP(pp_pushdefer)
{
    if(PL_op->op_private & OPpDEFER_FINALLY)
        SAVEDESTRUCTOR_X(invoke_finally_block, cLOGOP->op_other);
    else
        SAVEDESTRUCTOR_X(invoke_defer_block, cLOGOP->op_other);

    return NORMAL;
}

static MAGIC *
S_doparseform(pTHX_ SV *sv)
{
    STRLEN len;
    char *s = SvPV(sv, len);
    char *send;
    char *base = NULL; /* start of current field */
    I32 skipspaces = 0; /* number of contiguous spaces seen */
    bool noblank   = FALSE; /* ~ or ~~ seen on this line */
    bool repeat    = FALSE; /* ~~ seen on this line */
    bool postspace = FALSE; /* a text field may need right padding */
    U32 *fops;
    U32 *fpc;
    U32 *linepc = NULL;	    /* position of last FF_LINEMARK */
    I32 arg;
    bool ischop;	    /* it's a ^ rather than a @ */
    bool unchopnum = FALSE; /* at least one @ (i.e. non-chop) num field seen */
    int maxops = 12; /* FF_LINEMARK + FF_END + 10 (\0 without preceding \n) */
    MAGIC *mg = NULL;
    SV *sv_copy;

    PERL_ARGS_ASSERT_DOPARSEFORM;

    if (len == 0)
        Perl_croak(aTHX_ "Null picture in formline");

    if (SvTYPE(sv) >= SVt_PVMG) {
        /* This might, of course, still return NULL.  */
        mg = mg_find(sv, PERL_MAGIC_fm);
    } else {
        sv_upgrade(sv, SVt_PVMG);
    }

    if (mg) {
        /* still the same as previously-compiled string? */
        SV *old = mg->mg_obj;
        if ( ! (cBOOL(SvUTF8(old)) ^ cBOOL(SvUTF8(sv)))
            && len == SvCUR(old)
            && strnEQ(SvPVX(old), s, len)
        ) {
            DEBUG_f(PerlIO_printf(Perl_debug_log,"Re-using compiled format\n"));
            return mg;
        }

        DEBUG_f(PerlIO_printf(Perl_debug_log, "Re-compiling format\n"));
        Safefree(mg->mg_ptr);
        mg->mg_ptr = NULL;
        SvREFCNT_dec(old);
        mg->mg_obj = NULL;
    }
    else {
        DEBUG_f(PerlIO_printf(Perl_debug_log, "Compiling format\n"));
        mg = sv_magicext(sv, NULL, PERL_MAGIC_fm, &PL_vtbl_fm, NULL, 0);
    }

    sv_copy = newSVpvn_utf8(s, len, SvUTF8(sv));
    s = SvPV(sv_copy, len); /* work on the copy, not the original */
    send = s + len;


    /* estimate the buffer size needed */
    for (base = s; s <= send; s++) {
        if (*s == '\n' || *s == '@' || *s == '^')
            maxops += 10;
    }
    s = base;
    base = NULL;

    Newx(fops, maxops, U32);
    fpc = fops;

    if (s < send) {
        linepc = fpc;
        *fpc++ = FF_LINEMARK;
        noblank = repeat = FALSE;
        base = s;
    }

    while (s <= send) {
        switch (*s++) {
        default:
            skipspaces = 0;
            continue;

        case '~':
            if (*s == '~') {
                repeat = TRUE;
                skipspaces++;
                s++;
            }
            noblank = TRUE;
            /* FALLTHROUGH */
        case ' ': case '\t':
            skipspaces++;
            continue;
        case 0:
            if (s < send) {
                skipspaces = 0;
                continue;
            }
            /* FALLTHROUGH */
        case '\n':
            arg = s - base;
            skipspaces++;
            arg -= skipspaces;
            if (arg) {
                if (postspace)
                    *fpc++ = FF_SPACE;
                *fpc++ = FF_LITERAL;
                *fpc++ = (U32)arg;
            }
            postspace = FALSE;
            if (s <= send)
                skipspaces--;
            if (skipspaces) {
                *fpc++ = FF_SKIP;
                *fpc++ = (U32)skipspaces;
            }
            skipspaces = 0;
            if (s <= send)
                *fpc++ = FF_NEWLINE;
            if (noblank) {
                *fpc++ = FF_BLANK;
                if (repeat)
                    arg = fpc - linepc + 1;
                else
                    arg = 0;
                *fpc++ = (U32)arg;
            }
            if (s < send) {
                linepc = fpc;
                *fpc++ = FF_LINEMARK;
                noblank = repeat = FALSE;
                base = s;
            }
            else
                s++;
            continue;

        case '@':
        case '^':
            ischop = s[-1] == '^';

            if (postspace) {
                *fpc++ = FF_SPACE;
                postspace = FALSE;
            }
            arg = (s - base) - 1;
            if (arg) {
                *fpc++ = FF_LITERAL;
                *fpc++ = (U32)arg;
            }

            base = s - 1;
            *fpc++ = FF_FETCH;
            if (*s == '*') { /*  @* or ^*  */
                s++;
                *fpc++ = 2;  /* skip the @* or ^* */
                if (ischop) {
                    *fpc++ = FF_LINESNGL;
                    *fpc++ = FF_CHOP;
                } else
                    *fpc++ = FF_LINEGLOB;
            }
            else if (*s == '#' || (*s == '.' && s[1] == '#')) { /* @###, ^### */
                arg = ischop ? FORM_NUM_BLANK : 0;
                base = s - 1;
                while (*s == '#')
                    s++;
                if (*s == '.') {
                    const char * const f = ++s;
                    while (*s == '#')
                        s++;
                    arg |= FORM_NUM_POINT + (s - f);
                }
                *fpc++ = s - base;		/* fieldsize for FETCH */
                *fpc++ = FF_DECIMAL;
                *fpc++ = (U32)arg;
                unchopnum |= ! ischop;
            }
            else if (*s == '0' && s[1] == '#') {  /* Zero padded decimals */
                arg = ischop ? FORM_NUM_BLANK : 0;
                base = s - 1;
                s++;                                /* skip the '0' first */
                while (*s == '#')
                    s++;
                if (*s == '.') {
                    const char * const f = ++s;
                    while (*s == '#')
                        s++;
                    arg |= FORM_NUM_POINT + (s - f);
                }
                *fpc++ = s - base;                /* fieldsize for FETCH */
                *fpc++ = FF_0DECIMAL;
                *fpc++ = (U32)arg;
                unchopnum |= ! ischop;
            }
            else {				/* text field */
                I32 prespace = 0;
                bool ismore = FALSE;

                if (*s == '>') {
                    while (*++s == '>') ;
                    prespace = FF_SPACE;
                }
                else if (*s == '|') {
                    while (*++s == '|') ;
                    prespace = FF_HALFSPACE;
                    postspace = TRUE;
                }
                else {
                    if (*s == '<')
                        while (*++s == '<') ;
                    postspace = TRUE;
                }
                if (*s == '.' && s[1] == '.' && s[2] == '.') {
                    s += 3;
                    ismore = TRUE;
                }
                *fpc++ = s - base;		/* fieldsize for FETCH */

                *fpc++ = ischop ? FF_CHECKCHOP : FF_CHECKNL;

                if (prespace)
                    *fpc++ = (U32)prespace; /* add SPACE or HALFSPACE */
                *fpc++ = FF_ITEM;
                if (ismore)
                    *fpc++ = FF_MORE;
                if (ischop)
                    *fpc++ = FF_CHOP;
            }
            base = s;
            skipspaces = 0;
            continue;
        }
    }
    *fpc++ = FF_END;

    assert (fpc <= fops + maxops); /* ensure our buffer estimate was valid */
    arg = fpc - fops;

    mg->mg_ptr = (char *) fops;
    mg->mg_len = arg * sizeof(U32);
    mg->mg_obj = sv_copy;
    mg->mg_flags |= MGf_REFCOUNTED;

    if (unchopnum && repeat)
        Perl_die(aTHX_ "Repeated format line will never terminate (~~ and @#)");

    return mg;
}


STATIC bool
S_num_overflow(NV value, I32 fldsize, I32 frcsize)
{
    /* Can value be printed in fldsize chars, using %*.*f ? */
    NV pwr = 1;
    NV eps = 0.5;
    bool res = FALSE;
    int intsize = fldsize - (value < 0 ? 1 : 0);

    if (frcsize & FORM_NUM_POINT)
        intsize--;
    frcsize &= ~(FORM_NUM_POINT|FORM_NUM_BLANK);
    intsize -= frcsize;

    while (intsize--) pwr *= 10.0;
    while (frcsize--) eps /= 10.0;

    if( value >= 0 ){
        if (value + eps >= pwr)
            res = TRUE;
    } else {
        if (value - eps <= -pwr)
            res = TRUE;
    }
    return res;
}

static I32
S_run_user_filter(pTHX_ int idx, SV *buf_sv, int maxlen)
{
    SV * const datasv = FILTER_DATA(idx);
    const int filter_has_file = IoLINES(datasv);
    SV * const filter_state = MUTABLE_SV(IoTOP_GV(datasv));
    SV * const filter_sub = MUTABLE_SV(IoBOTTOM_GV(datasv));
    int status = 0;
    SV *upstream;
    STRLEN got_len;
    char *got_p = NULL;
    char *prune_from = NULL;
    bool read_from_cache = FALSE;
    STRLEN umaxlen;
    SV *err = NULL;

    PERL_ARGS_ASSERT_RUN_USER_FILTER;

    assert(maxlen >= 0);
    umaxlen = maxlen;

    /* I was having segfault trouble under Linux 2.2.5 after a
       parse error occurred.  (Had to hack around it with a test
       for PL_parser->error_count == 0.)  Solaris doesn't segfault --
       not sure where the trouble is yet.  XXX */

    {
        SV *const cache = datasv;
        if (SvOK(cache)) {
            STRLEN cache_len;
            const char *cache_p = SvPV(cache, cache_len);
            STRLEN take = 0;

            if (umaxlen) {
                /* Running in block mode and we have some cached data already.
                 */
                if (cache_len >= umaxlen) {
                    /* In fact, so much data we don't even need to call
                       filter_read.  */
                    take = umaxlen;
                }
            } else {
                const char *const first_nl =
                    (const char *)memchr(cache_p, '\n', cache_len);
                if (first_nl) {
                    take = first_nl + 1 - cache_p;
                }
            }
            if (take) {
                sv_catpvn(buf_sv, cache_p, take);
                sv_chop(cache, cache_p + take);
                /* Definitely not EOF  */
                return 1;
            }

            sv_catsv(buf_sv, cache);
            if (umaxlen) {
                umaxlen -= cache_len;
            }
            SvOK_off(cache);
            read_from_cache = TRUE;
        }
    }

    /* Filter API says that the filter appends to the contents of the buffer.
       Usually the buffer is "", so the details don't matter. But if it's not,
       then clearly what it contains is already filtered by this filter, so we
       don't want to pass it in a second time.
       I'm going to use a mortal in case the upstream filter croaks.  */
    upstream = ((SvOK(buf_sv) && sv_len(buf_sv)) || SvGMAGICAL(buf_sv))
        ? newSV_type_mortal(SVt_PV) : buf_sv;
    SvUPGRADE(upstream, SVt_PV);
        
    if (filter_has_file) {
        status = FILTER_READ(idx+1, upstream, 0);
    }

    if (filter_sub && status >= 0) {
        dSP;
        int count;

        ENTER_with_name("call_filter_sub");
        SAVE_DEFSV;
        SAVETMPS;
        EXTEND(SP, 2);

        DEFSV_set(upstream);
        PUSHMARK(SP);
        PUSHs(&PL_sv_zero);
        if (filter_state) {
            PUSHs(filter_state);
        }
        PUTBACK;
        count = call_sv(filter_sub, G_SCALAR|G_EVAL);
        SPAGAIN;

        if (count > 0) {
            SV *out = POPs;
            SvGETMAGIC(out);
            if (SvOK(out)) {
                status = SvIV(out);
            }
            else {
                SV * const errsv = ERRSV;
                if (SvTRUE_NN(errsv))
                    err = newSVsv(errsv);
            }
        }

        PUTBACK;
        FREETMPS;
        LEAVE_with_name("call_filter_sub");
    }

    if (SvGMAGICAL(upstream)) {
        mg_get(upstream);
        if (upstream == buf_sv) mg_free(buf_sv);
    }
    if (SvIsCOW(upstream)) sv_force_normal(upstream);
    if(!err && SvOK(upstream)) {
        got_p = SvPV_nomg(upstream, got_len);
        if (umaxlen) {
            if (got_len > umaxlen) {
                prune_from = got_p + umaxlen;
            }
        } else {
            char *const first_nl = (char *)memchr(got_p, '\n', got_len);
            if (first_nl && first_nl + 1 < got_p + got_len) {
                /* There's a second line here... */
                prune_from = first_nl + 1;
            }
        }
    }
    if (!err && prune_from) {
        /* Oh. Too long. Stuff some in our cache.  */
        STRLEN cached_len = got_p + got_len - prune_from;
        SV *const cache = datasv;

        if (SvOK(cache)) {
            /* Cache should be empty.  */
            assert(!SvCUR(cache));
        }

        sv_setpvn(cache, prune_from, cached_len);
        /* If you ask for block mode, you may well split UTF-8 characters.
           "If it breaks, you get to keep both parts"
           (Your code is broken if you  don't put them back together again
           before something notices.) */
        if (SvUTF8(upstream)) {
            SvUTF8_on(cache);
        }
        if (SvPOK(upstream)) SvCUR_set(upstream, got_len - cached_len);
        else
            /* Cannot just use sv_setpvn, as that could free the buffer
               before we have a chance to assign it. */
            sv_usepvn(upstream, savepvn(got_p, got_len - cached_len),
                      got_len - cached_len);
        *prune_from = 0;
        /* Can't yet be EOF  */
        if (status == 0)
            status = 1;
    }

    /* If they are at EOF but buf_sv has something in it, then they may never
       have touched the SV upstream, so it may be undefined.  If we naively
       concatenate it then we get a warning about use of uninitialised value.
    */
    if (!err && upstream != buf_sv &&
        SvOK(upstream)) {
        sv_catsv_nomg(buf_sv, upstream);
    }
    else if (SvOK(upstream)) (void)SvPV_force_nolen(buf_sv);

    if (status <= 0) {
        IoLINES(datasv) = 0;
        if (filter_state) {
            SvREFCNT_dec(filter_state);
            IoTOP_GV(datasv) = NULL;
        }
        if (filter_sub) {
            SvREFCNT_dec(filter_sub);
            IoBOTTOM_GV(datasv) = NULL;
        }
        filter_del(S_run_user_filter);
    }

    if (err)
        croak_sv(err);

    if (status == 0 && read_from_cache) {
        /* If we read some data from the cache (and by getting here it implies
           that we emptied the cache) then we aren't yet at EOF, and mustn't
           report that to our caller.  */
        return 1;
    }
    return status;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
