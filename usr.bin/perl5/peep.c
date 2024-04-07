/*    peep.c
 *
 *    Copyright (C) 1991-2022 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * Aragorn sped on up the hill. Every now and again he bent to the ground.
 * Hobbits go light, and their footprints are not easy even for a Ranger to
 * read, but not far from the top a spring crossed the path, and in the wet
 * earth he saw what he was seeking.
 * 'I read the signs aright,' he said to himself. 'Frodo ran to the hill-top.
 * I wonder what he saw there? But he returned by the same way, and went down
 * the hill again.'
 */

/* This file contains functions for optimizing and finalizing the OP
 * structures that hold a compiled perl program
 */

#include "EXTERN.h"
#define PERL_IN_PEEP_C
#include "perl.h"


#define CALL_RPEEP(o) PL_rpeepp(aTHX_ o)


static void
S_scalar_slice_warning(pTHX_ const OP *o)
{
    OP *kid;
    const bool is_hash = o->op_type == OP_HSLICE
                || (o->op_type == OP_NULL && o->op_targ == OP_HSLICE);
    SV *name;

    if (!(o->op_private & OPpSLICEWARNING))
        return;
    if (PL_parser && PL_parser->error_count)
        /* This warning can be nonsensical when there is a syntax error. */
        return;

    kid = cLISTOPo->op_first;
    kid = OpSIBLING(kid); /* get past pushmark */
    /* weed out false positives: any ops that can return lists */
    switch (kid->op_type) {
    case OP_BACKTICK:
    case OP_GLOB:
    case OP_READLINE:
    case OP_MATCH:
    case OP_RV2AV:
    case OP_EACH:
    case OP_VALUES:
    case OP_KEYS:
    case OP_SPLIT:
    case OP_LIST:
    case OP_SORT:
    case OP_REVERSE:
    case OP_ENTERSUB:
    case OP_CALLER:
    case OP_LSTAT:
    case OP_STAT:
    case OP_READDIR:
    case OP_SYSTEM:
    case OP_TMS:
    case OP_LOCALTIME:
    case OP_GMTIME:
    case OP_ENTEREVAL:
        return;
    }

    /* Don't warn if we have a nulled list either. */
    if (kid->op_type == OP_NULL && kid->op_targ == OP_LIST)
        return;

    assert(OpSIBLING(kid));
    name = op_varname(OpSIBLING(kid));
    if (!name) /* XS module fiddling with the op tree */
        return;
    warn_elem_scalar_context(kid, name, is_hash, true);
}


/* info returned by S_sprintf_is_multiconcatable() */

struct sprintf_ismc_info {
    SSize_t nargs;    /* num of args to sprintf (not including the format) */
    char  *start;     /* start of raw format string */
    char  *end;       /* bytes after end of raw format string */
    STRLEN total_len; /* total length (in bytes) of format string, not
                         including '%s' and  half of '%%' */
    STRLEN variant;   /* number of bytes by which total_len_p would grow
                         if upgraded to utf8 */
    bool   utf8;      /* whether the format is utf8 */
};

/* is the OP_SPRINTF o suitable for converting into a multiconcat op?
 * i.e. its format argument is a const string with only '%s' and '%%'
 * formats, and the number of args is known, e.g.
 *    sprintf "a=%s f=%s", $a[0], scalar(f());
 * but not
 *    sprintf "i=%d a=%s f=%s", $i, @a, f();
 *
 * If successful, the sprintf_ismc_info struct pointed to by info will be
 * populated.
 */

STATIC bool
S_sprintf_is_multiconcatable(pTHX_ OP *o,struct sprintf_ismc_info *info)
{
    OP    *pm, *constop, *kid;
    SV    *sv;
    char  *s, *e, *p;
    SSize_t nargs, nformats;
    STRLEN cur, total_len, variant;
    bool   utf8;

    /* if sprintf's behaviour changes, die here so that someone
     * can decide whether to enhance this function or skip optimising
     * under those new circumstances */
    assert(!(o->op_flags & OPf_STACKED));
    assert(!(PL_opargs[OP_SPRINTF] & OA_TARGLEX));
    assert(!(o->op_private & ~OPpARG4_MASK));

    pm = cUNOPo->op_first;
    if (pm->op_type != OP_PUSHMARK) /* weird coreargs stuff */
        return FALSE;
    constop = OpSIBLING(pm);
    if (!constop || constop->op_type != OP_CONST)
        return FALSE;
    sv = cSVOPx_sv(constop);
    if (SvMAGICAL(sv) || !SvPOK(sv))
        return FALSE;

    s = SvPV(sv, cur);
    e = s + cur;

    /* Scan format for %% and %s and work out how many %s there are.
     * Abandon if other format types are found.
     */

    nformats  = 0;
    total_len = 0;
    variant   = 0;

    for (p = s; p < e; p++) {
        if (*p != '%') {
            total_len++;
            if (!UTF8_IS_INVARIANT(*p))
                variant++;
            continue;
        }
        p++;
        if (p >= e)
            return FALSE; /* lone % at end gives "Invalid conversion" */
        if (*p == '%')
            total_len++;
        else if (*p == 's')
            nformats++;
        else
            return FALSE;
    }

    if (!nformats || nformats > PERL_MULTICONCAT_MAXARG)
        return FALSE;

    utf8 = cBOOL(SvUTF8(sv));
    if (utf8)
        variant = 0;

    /* scan args; they must all be in scalar cxt */

    nargs = 0;
    kid = OpSIBLING(constop);

    while (kid) {
        if ((kid->op_flags & OPf_WANT) != OPf_WANT_SCALAR)
            return FALSE;
        nargs++;
        kid = OpSIBLING(kid);
    }

    if (nargs != nformats)
        return FALSE; /* e.g. sprintf("%s%s", $a); */


    info->nargs      = nargs;
    info->start      = s;
    info->end        = e;
    info->total_len  = total_len;
    info->variant    = variant;
    info->utf8       = utf8;

    return TRUE;
}

/* S_maybe_multiconcat():
 *
 * given an OP_STRINGIFY, OP_SASSIGN, OP_CONCAT or OP_SPRINTF op, possibly
 * convert it (and its children) into an OP_MULTICONCAT. See the code
 * comments just before pp_multiconcat() for the full details of what
 * OP_MULTICONCAT supports.
 *
 * Basically we're looking for an optree with a chain of OP_CONCATS down
 * the LHS (or an OP_SPRINTF), with possibly an OP_SASSIGN, and/or
 * OP_STRINGIFY, and/or OP_CONCAT acting as '.=' at its head, e.g.
 *
 *      $x = "$a$b-$c"
 *
 *  looks like
 *
 *      SASSIGN
 *         |
 *      STRINGIFY   -- PADSV[$x]
 *         |
 *         |
 *      ex-PUSHMARK -- CONCAT/S
 *                        |
 *                     CONCAT/S  -- PADSV[$d]
 *                        |
 *                     CONCAT    -- CONST["-"]
 *                        |
 *                     PADSV[$a] -- PADSV[$b]
 *
 * Note that at this stage the OP_SASSIGN may have already been optimised
 * away with OPpTARGET_MY set on the OP_STRINGIFY or OP_CONCAT.
 */

STATIC void
S_maybe_multiconcat(pTHX_ OP *o)
{
    OP *lastkidop;   /* the right-most of any kids unshifted onto o */
    OP *topop;       /* the top-most op in the concat tree (often equals o,
                        unless there are assign/stringify ops above it */
    OP *parentop;    /* the parent op of topop (or itself if no parent) */
    OP *targmyop;    /* the op (if any) with the OPpTARGET_MY flag */
    OP *targetop;    /* the op corresponding to target=... or target.=... */
    OP *stringop;    /* the OP_STRINGIFY op, if any */
    OP *nextop;      /* used for recreating the op_next chain without consts */
    OP *kid;         /* general-purpose op pointer */
    UNOP_AUX_item *aux;
    UNOP_AUX_item *lenp;
    char *const_str, *p;
    struct sprintf_ismc_info sprintf_info;

                     /* store info about each arg in args[];
                      * toparg is the highest used slot; argp is a general
                      * pointer to args[] slots */
    struct {
        void *p;      /* initially points to const sv (or null for op);
                         later, set to SvPV(constsv), with ... */
        STRLEN len;   /* ... len set to SvPV(..., len) */
    } *argp, *toparg, args[PERL_MULTICONCAT_MAXARG*2 + 1];

    SSize_t nargs  = 0;
    SSize_t nconst = 0;
    SSize_t nadjconst  = 0; /* adjacent consts - may be demoted to args */
    STRLEN variant;
    bool utf8 = FALSE;
    bool kid_is_last = FALSE; /* most args will be the RHS kid of a concat op;
                                 the last-processed arg will the LHS of one,
                                 as args are processed in reverse order */
    U8   stacked_last = 0;   /* whether the last seen concat op was STACKED */
    STRLEN total_len  = 0;   /* sum of the lengths of the const segments */
    U8 flags          = 0;   /* what will become the op_flags and ... */
    U8 private_flags  = 0;   /* ... op_private of the multiconcat op */
    bool is_sprintf = FALSE; /* we're optimising an sprintf */
    bool is_targable  = FALSE; /* targetop is an OPpTARGET_MY candidate */
    bool prev_was_const = FALSE; /* previous arg was a const */

    /* -----------------------------------------------------------------
     * Phase 1:
     *
     * Examine the optree non-destructively to determine whether it's
     * suitable to be converted into an OP_MULTICONCAT. Accumulate
     * information about the optree in args[].
     */

    argp     = args;
    targmyop = NULL;
    targetop = NULL;
    stringop = NULL;
    topop    = o;
    parentop = o;

    assert(   o->op_type == OP_SASSIGN
           || o->op_type == OP_CONCAT
           || o->op_type == OP_SPRINTF
           || o->op_type == OP_STRINGIFY);

    Zero(&sprintf_info, 1, struct sprintf_ismc_info);

    /* first see if, at the top of the tree, there is an assign,
     * append and/or stringify */

    if (topop->op_type == OP_SASSIGN) {
        /* expr = ..... */
        if (o->op_ppaddr != PL_ppaddr[OP_SASSIGN])
            return;
        if (o->op_private & (OPpASSIGN_BACKWARDS|OPpASSIGN_CV_TO_GV))
            return;
        assert(!(o->op_private & ~OPpARG2_MASK)); /* barf on unknown flags */

        parentop = topop;
        topop = cBINOPo->op_first;
        targetop = OpSIBLING(topop);
        if (!targetop) /* probably some sort of syntax error */
            return;

        /* don't optimise away assign in 'local $foo = ....' */
        if (   (targetop->op_private & OPpLVAL_INTRO)
            /* these are the common ops which do 'local', but
             * not all */
            && (   targetop->op_type == OP_GVSV
                || targetop->op_type == OP_RV2SV
                || targetop->op_type == OP_AELEM
                || targetop->op_type == OP_HELEM
                )
        )
            return;
    }
    else if (   topop->op_type == OP_CONCAT
             && (topop->op_flags & OPf_STACKED)
             && (!(topop->op_private & OPpCONCAT_NESTED))
            )
    {
        /* expr .= ..... */

        /* OPpTARGET_MY shouldn't be able to be set here. If it is,
         * decide what to do about it */
        assert(!(o->op_private & OPpTARGET_MY));

        /* barf on unknown flags */
        assert(!(o->op_private & ~(OPpARG2_MASK|OPpTARGET_MY)));
        private_flags |= OPpMULTICONCAT_APPEND;
        targetop = cBINOPo->op_first;
        parentop = topop;
        topop    = OpSIBLING(targetop);

        /* $x .= <FOO> gets optimised to rcatline instead */
        if (topop->op_type == OP_READLINE)
            return;
    }

    if (targetop) {
        /* Can targetop (the LHS) if it's a padsv, be optimised
         * away and use OPpTARGET_MY instead?
         */
        if (    (targetop->op_type == OP_PADSV)
            && !(targetop->op_private & OPpDEREF)
            && !(targetop->op_private & OPpPAD_STATE)
               /* we don't support 'my $x .= ...' */
            && (   o->op_type == OP_SASSIGN
                || !(targetop->op_private & OPpLVAL_INTRO))
        )
            is_targable = TRUE;
    }

    if (topop->op_type == OP_STRINGIFY) {
        if (topop->op_ppaddr != PL_ppaddr[OP_STRINGIFY])
            return;
        stringop = topop;

        /* barf on unknown flags */
        assert(!(o->op_private & ~(OPpARG4_MASK|OPpTARGET_MY)));

        if ((topop->op_private & OPpTARGET_MY)) {
            if (o->op_type == OP_SASSIGN)
                return; /* can't have two assigns */
            targmyop = topop;
        }

        private_flags |= OPpMULTICONCAT_STRINGIFY;
        parentop = topop;
        topop = cBINOPx(topop)->op_first;
        assert(OP_TYPE_IS_OR_WAS_NN(topop, OP_PUSHMARK));
        topop = OpSIBLING(topop);
    }

    if (topop->op_type == OP_SPRINTF) {
        if (topop->op_ppaddr != PL_ppaddr[OP_SPRINTF])
            return;
        if (S_sprintf_is_multiconcatable(aTHX_ topop, &sprintf_info)) {
            nargs     = sprintf_info.nargs;
            total_len = sprintf_info.total_len;
            variant   = sprintf_info.variant;
            utf8      = sprintf_info.utf8;
            is_sprintf = TRUE;
            private_flags |= OPpMULTICONCAT_FAKE;
            toparg = argp;
            /* we have an sprintf op rather than a concat optree.
             * Skip most of the code below which is associated with
             * processing that optree. We also skip phase 2, determining
             * whether its cost effective to optimise, since for sprintf,
             * multiconcat is *always* faster */
            goto create_aux;
        }
        /* note that even if the sprintf itself isn't multiconcatable,
         * the expression as a whole may be, e.g. in
         *    $x .= sprintf("%d",...)
         * the sprintf op will be left as-is, but the concat/S op may
         * be upgraded to multiconcat
         */
    }
    else if (topop->op_type == OP_CONCAT) {
        if (topop->op_ppaddr != PL_ppaddr[OP_CONCAT])
            return;

        if ((topop->op_private & OPpTARGET_MY)) {
            if (o->op_type == OP_SASSIGN || targmyop)
                return; /* can't have two assigns */
            targmyop = topop;
        }
    }

    /* Is it safe to convert a sassign/stringify/concat op into
     * a multiconcat? */
    assert((PL_opargs[OP_SASSIGN]   & OA_CLASS_MASK) == OA_BINOP);
    assert((PL_opargs[OP_CONCAT]    & OA_CLASS_MASK) == OA_BINOP);
    assert((PL_opargs[OP_STRINGIFY] & OA_CLASS_MASK) == OA_LISTOP);
    assert((PL_opargs[OP_SPRINTF]   & OA_CLASS_MASK) == OA_LISTOP);
    STATIC_ASSERT_STMT(   STRUCT_OFFSET(BINOP,    op_last)
                       == STRUCT_OFFSET(UNOP_AUX, op_aux));
    STATIC_ASSERT_STMT(   STRUCT_OFFSET(LISTOP,   op_last)
                       == STRUCT_OFFSET(UNOP_AUX, op_aux));

    /* Now scan the down the tree looking for a series of
     * CONCAT/OPf_STACKED ops on the LHS (with the last one not
     * stacked). For example this tree:
     *
     *     |
     *   CONCAT/STACKED
     *     |
     *   CONCAT/STACKED -- EXPR5
     *     |
     *   CONCAT/STACKED -- EXPR4
     *     |
     *   CONCAT -- EXPR3
     *     |
     *   EXPR1  -- EXPR2
     *
     * corresponds to an expression like
     *
     *   (EXPR1 . EXPR2 . EXPR3 . EXPR4 . EXPR5)
     *
     * Record info about each EXPR in args[]: in particular, whether it is
     * a stringifiable OP_CONST and if so what the const sv is.
     *
     * The reason why the last concat can't be STACKED is the difference
     * between
     *
     *    ((($a .= $a) .= $a) .= $a) .= $a
     *
     * and
     *    $a . $a . $a . $a . $a
     *
     * The main difference between the optrees for those two constructs
     * is the presence of the last STACKED. As well as modifying $a,
     * the former sees the changed $a between each concat, so if $s is
     * initially 'a', the first returns 'a' x 16, while the latter returns
     * 'a' x 5. And pp_multiconcat can't handle that kind of thing.
     */

    kid = topop;

    for (;;) {
        OP *argop;
        SV *sv;
        bool last = FALSE;

        if (    kid->op_type == OP_CONCAT
            && !kid_is_last
        ) {
            OP *k1, *k2;
            k1 = cUNOPx(kid)->op_first;
            k2 = OpSIBLING(k1);
            /* shouldn't happen except maybe after compile err? */
            if (!k2)
                return;

            /* avoid turning (A . B . ($lex = C) ...)  into  (A . B . C ...) */
            if (kid->op_private & OPpTARGET_MY)
                kid_is_last = TRUE;

            stacked_last = (kid->op_flags & OPf_STACKED);
            if (!stacked_last)
                kid_is_last = TRUE;

            kid   = k1;
            argop = k2;
        }
        else {
            argop = kid;
            last = TRUE;
        }

        if (   nargs + nadjconst  >  PERL_MULTICONCAT_MAXARG        - 2
            || (argp - args + 1)  > (PERL_MULTICONCAT_MAXARG*2 + 1) - 2)
        {
            /* At least two spare slots are needed to decompose both
             * concat args. If there are no slots left, continue to
             * examine the rest of the optree, but don't push new values
             * on args[]. If the optree as a whole is legal for conversion
             * (in particular that the last concat isn't STACKED), then
             * the first PERL_MULTICONCAT_MAXARG elements of the optree
             * can be converted into an OP_MULTICONCAT now, with the first
             * child of that op being the remainder of the optree -
             * which may itself later be converted to a multiconcat op
             * too.
             */
            if (last) {
                /* the last arg is the rest of the optree */
                argp++->p = NULL;
                nargs++;
            }
        }
        else if (   argop->op_type == OP_CONST
            && ((sv = cSVOPx_sv(argop)))
            /* defer stringification until runtime of 'constant'
             * things that might stringify variantly, e.g. the radix
             * point of NVs, or overloaded RVs */
            && (SvPOK(sv) || SvIOK(sv))
            && (!SvGMAGICAL(sv))
        ) {
            if (argop->op_private & OPpCONST_STRICT)
                no_bareword_allowed(argop);
            argp++->p = sv;
            utf8   |= cBOOL(SvUTF8(sv));
            nconst++;
            if (prev_was_const)
                /* this const may be demoted back to a plain arg later;
                 * make sure we have enough arg slots left */
                nadjconst++;
            prev_was_const = !prev_was_const;
        }
        else {
            argp++->p = NULL;
            nargs++;
            prev_was_const = FALSE;
        }

        if (last)
            break;
    }

    toparg = argp - 1;

    if (stacked_last)
        return; /* we don't support ((A.=B).=C)...) */

    /* look for two adjacent consts and don't fold them together:
     *     $o . "a" . "b"
     * should do
     *     $o->concat("a")->concat("b")
     * rather than
     *     $o->concat("ab")
     * (but $o .=  "a" . "b" should still fold)
     */
    {
        bool seen_nonconst = FALSE;
        for (argp = toparg; argp >= args; argp--) {
            if (argp->p == NULL) {
                seen_nonconst = TRUE;
                continue;
            }
            if (!seen_nonconst)
                continue;
            if (argp[1].p) {
                /* both previous and current arg were constants;
                 * leave the current OP_CONST as-is */
                argp->p = NULL;
                nconst--;
                nargs++;
            }
        }
    }

    /* -----------------------------------------------------------------
     * Phase 2:
     *
     * At this point we have determined that the optree *can* be converted
     * into a multiconcat. Having gathered all the evidence, we now decide
     * whether it *should*.
     */


    /* we need at least one concat action, e.g.:
     *
     *  Y . Z
     *  X = Y . Z
     *  X .= Y
     *
     * otherwise we could be doing something like $x = "foo", which
     * if treated as a concat, would fail to COW.
     */
    if (nargs + nconst + cBOOL(private_flags & OPpMULTICONCAT_APPEND) < 2)
        return;

    /* Benchmarking seems to indicate that we gain if:
     * * we optimise at least two actions into a single multiconcat
     *    (e.g concat+concat, sassign+concat);
     * * or if we can eliminate at least 1 OP_CONST;
     * * or if we can eliminate a padsv via OPpTARGET_MY
     */

    if (
           /* eliminated at least one OP_CONST */
           nconst >= 1
           /* eliminated an OP_SASSIGN */
        || o->op_type == OP_SASSIGN
           /* eliminated an OP_PADSV */
        || (!targmyop && is_targable)
    )
        /* definitely a net gain to optimise */
        goto optimise;

    /* ... if not, what else? */

    /* special-case '$lex1 = expr . $lex1' (where expr isn't lex1):
     * multiconcat is faster (due to not creating a temporary copy of
     * $lex1), whereas for a general $lex1 = $lex2 . $lex3, concat is
     * faster.
     */
    if (   nconst == 0
         && nargs == 2
         && targmyop
         && topop->op_type == OP_CONCAT
    ) {
        PADOFFSET t = targmyop->op_targ;
        OP *k1 = cBINOPx(topop)->op_first;
        OP *k2 = cBINOPx(topop)->op_last;
        if (   k2->op_type == OP_PADSV
            && k2->op_targ == t
            && (   k1->op_type != OP_PADSV
                || k1->op_targ != t)
        )
            goto optimise;
    }

    /* need at least two concats */
    if (nargs + nconst + cBOOL(private_flags & OPpMULTICONCAT_APPEND) < 3)
        return;



    /* -----------------------------------------------------------------
     * Phase 3:
     *
     * At this point the optree has been verified as ok to be optimised
     * into an OP_MULTICONCAT. Now start changing things.
     */

   optimise:

    /* stringify all const args and determine utf8ness */

    variant = 0;
    for (argp = args; argp <= toparg; argp++) {
        SV *sv = (SV*)argp->p;
        if (!sv)
            continue; /* not a const op */
        if (utf8 && !SvUTF8(sv))
            sv_utf8_upgrade_nomg(sv);
        argp->p = SvPV_nomg(sv, argp->len);
        total_len += argp->len;

        /* see if any strings would grow if converted to utf8 */
        if (!utf8) {
            variant += variant_under_utf8_count((U8 *) argp->p,
                                                (U8 *) argp->p + argp->len);
        }
    }

    /* create and populate aux struct */

  create_aux:

    aux = (UNOP_AUX_item*)PerlMemShared_malloc(
                    sizeof(UNOP_AUX_item)
                    *  (
                           PERL_MULTICONCAT_HEADER_SIZE
                         + ((nargs + 1) * (variant ? 2 : 1))
                        )
                    );
    const_str = (char *)PerlMemShared_malloc(total_len ? total_len : 1);

    /* Extract all the non-const expressions from the concat tree then
     * dispose of the old tree, e.g. convert the tree from this:
     *
     *  o => SASSIGN
     *         |
     *       STRINGIFY   -- TARGET
     *         |
     *       ex-PUSHMARK -- CONCAT
     *                        |
     *                      CONCAT -- EXPR5
     *                        |
     *                      CONCAT -- EXPR4
     *                        |
     *                      CONCAT -- EXPR3
     *                        |
     *                      EXPR1  -- EXPR2
     *
     *
     * to:
     *
     *  o => MULTICONCAT
     *         |
     *       ex-PUSHMARK -- EXPR1 -- EXPR2 -- EXPR3 -- EXPR4 -- EXPR5 -- TARGET
     *
     * except that if EXPRi is an OP_CONST, it's discarded.
     *
     * During the conversion process, EXPR ops are stripped from the tree
     * and unshifted onto o. Finally, any of o's remaining original
     * children are discarded and o is converted into an OP_MULTICONCAT.
     *
     * In this middle of this, o may contain both: unshifted args on the
     * left, and some remaining original args on the right. lastkidop
     * is set to point to the right-most unshifted arg to delineate
     * between the two sets.
     */


    if (is_sprintf) {
        /* create a copy of the format with the %'s removed, and record
         * the sizes of the const string segments in the aux struct */
        char *q, *oldq;
        lenp = aux + PERL_MULTICONCAT_IX_LENGTHS;

        p    = sprintf_info.start;
        q    = const_str;
        oldq = q;
        for (; p < sprintf_info.end; p++) {
            if (*p == '%') {
                p++;
                if (*p != '%') {
                    (lenp++)->ssize = q - oldq;
                    oldq = q;
                    continue;
                }
            }
            *q++ = *p;
        }
        lenp->ssize = q - oldq;
        assert((STRLEN)(q - const_str) == total_len);

        /* Attach all the args (i.e. the kids of the sprintf) to o (which
         * may or may not be topop) The pushmark and const ops need to be
         * kept in case they're an op_next entry point.
         */
        lastkidop = cLISTOPx(topop)->op_last;
        kid = cUNOPx(topop)->op_first; /* pushmark */
        op_null(kid);
        op_null(OpSIBLING(kid));       /* const */
        if (o != topop) {
            kid = op_sibling_splice(topop, NULL, -1, NULL); /* cut all args */
            op_sibling_splice(o, NULL, 0, kid); /* and attach to o */
            lastkidop->op_next = o;
        }
    }
    else {
        p = const_str;
        lenp = aux + PERL_MULTICONCAT_IX_LENGTHS;

        lenp->ssize = -1;

        /* Concatenate all const strings into const_str.
         * Note that args[] contains the RHS args in reverse order, so
         * we scan args[] from top to bottom to get constant strings
         * in L-R order
         */
        for (argp = toparg; argp >= args; argp--) {
            if (!argp->p)
                /* not a const op */
                (++lenp)->ssize = -1;
            else {
                STRLEN l = argp->len;
                Copy(argp->p, p, l, char);
                p += l;
                if (lenp->ssize == -1)
                    lenp->ssize = l;
                else
                    lenp->ssize += l;
            }
        }

        kid = topop;
        nextop = o;
        lastkidop = NULL;

        for (argp = args; argp <= toparg; argp++) {
            /* only keep non-const args, except keep the first-in-next-chain
             * arg no matter what it is (but nulled if OP_CONST), because it
             * may be the entry point to this subtree from the previous
             * op_next.
             */
            bool last = (argp == toparg);
            OP *prev;

            /* set prev to the sibling *before* the arg to be cut out,
             * e.g. when cutting EXPR:
             *
             *         |
             * kid=  CONCAT
             *         |
             * prev= CONCAT -- EXPR
             *         |
             */
            if (argp == args && kid->op_type != OP_CONCAT) {
                /* in e.g. '$x .= f(1)' there's no RHS concat tree
                 * so the expression to be cut isn't kid->op_last but
                 * kid itself */
                OP *o1, *o2;
                /* find the op before kid */
                o1 = NULL;
                o2 = cUNOPx(parentop)->op_first;
                while (o2 && o2 != kid) {
                    o1 = o2;
                    o2 = OpSIBLING(o2);
                }
                assert(o2 == kid);
                prev = o1;
                kid  = parentop;
            }
            else if (kid == o && lastkidop)
                prev = last ? lastkidop : OpSIBLING(lastkidop);
            else
                prev = last ? NULL : cUNOPx(kid)->op_first;

            if (!argp->p || last) {
                /* cut RH op */
                OP *aop = op_sibling_splice(kid, prev, 1, NULL);
                /* and unshift to front of o */
                op_sibling_splice(o, NULL, 0, aop);
                /* record the right-most op added to o: later we will
                 * free anything to the right of it */
                if (!lastkidop)
                    lastkidop = aop;
                aop->op_next = nextop;
                if (last) {
                    if (argp->p)
                        /* null the const at start of op_next chain */
                        op_null(aop);
                }
                else if (prev)
                    nextop = prev->op_next;
            }

            /* the last two arguments are both attached to the same concat op */
            if (argp < toparg - 1)
                kid = prev;
        }
    }

    /* Populate the aux struct */

    aux[PERL_MULTICONCAT_IX_NARGS].ssize     = nargs;
    aux[PERL_MULTICONCAT_IX_PLAIN_PV].pv    = utf8 ? NULL : const_str;
    aux[PERL_MULTICONCAT_IX_PLAIN_LEN].ssize = utf8 ?    0 : total_len;
    aux[PERL_MULTICONCAT_IX_UTF8_PV].pv     = const_str;
    aux[PERL_MULTICONCAT_IX_UTF8_LEN].ssize  = total_len;

    /* if variant > 0, calculate a variant const string and lengths where
     * the utf8 version of the string will take 'variant' more bytes than
     * the plain one. */

    if (variant) {
        char              *p = const_str;
        STRLEN          ulen = total_len + variant;
        UNOP_AUX_item  *lens = aux + PERL_MULTICONCAT_IX_LENGTHS;
        UNOP_AUX_item *ulens = lens + (nargs + 1);
        char             *up = (char*)PerlMemShared_malloc(ulen);
        SSize_t            n;

        aux[PERL_MULTICONCAT_IX_UTF8_PV].pv    = up;
        aux[PERL_MULTICONCAT_IX_UTF8_LEN].ssize = ulen;

        for (n = 0; n < (nargs + 1); n++) {
            SSize_t i;
            char * orig_up = up;
            for (i = (lens++)->ssize; i > 0; i--) {
                U8 c = *p++;
                append_utf8_from_native_byte(c, (U8**)&up);
            }
            (ulens++)->ssize = (i < 0) ? i : up - orig_up;
        }
    }

    if (stringop) {
        /* if there was a top(ish)-level OP_STRINGIFY, we need to keep
         * that op's first child - an ex-PUSHMARK - because the op_next of
         * the previous op may point to it (i.e. it's the entry point for
         * the o optree)
         */
        OP *pmop =
            (stringop == o)
                ? op_sibling_splice(o, lastkidop, 1, NULL)
                : op_sibling_splice(stringop, NULL, 1, NULL);
        assert(OP_TYPE_IS_OR_WAS_NN(pmop, OP_PUSHMARK));
        op_sibling_splice(o, NULL, 0, pmop);
        if (!lastkidop)
            lastkidop = pmop;
    }

    /* Optimise
     *    target  = A.B.C...
     *    target .= A.B.C...
     */

    if (targetop) {
        assert(!targmyop);

        if (o->op_type == OP_SASSIGN) {
            /* Move the target subtree from being the last of o's children
             * to being the last of o's preserved children.
             * Note the difference between 'target = ...' and 'target .= ...':
             * for the former, target is executed last; for the latter,
             * first.
             */
            kid = OpSIBLING(lastkidop);
            op_sibling_splice(o, kid, 1, NULL); /* cut target op */
            op_sibling_splice(o, lastkidop, 0, targetop); /* and paste */
            lastkidop->op_next = kid->op_next;
            lastkidop = targetop;
        }
        else {
            /* Move the target subtree from being the first of o's
             * original children to being the first of *all* o's children.
             */
            if (lastkidop) {
                op_sibling_splice(o, lastkidop, 1, NULL); /* cut target op */
                op_sibling_splice(o, NULL, 0, targetop);  /* and paste*/
            }
            else {
                /* if the RHS of .= doesn't contain a concat (e.g.
                 * $x .= "foo"), it gets missed by the "strip ops from the
                 * tree and add to o" loop earlier */
                assert(topop->op_type != OP_CONCAT);
                if (stringop) {
                    /* in e.g. $x .= "$y", move the $y expression
                     * from being a child of OP_STRINGIFY to being the
                     * second child of the OP_CONCAT
                     */
                    assert(cUNOPx(stringop)->op_first == topop);
                    op_sibling_splice(stringop, NULL, 1, NULL);
                    op_sibling_splice(o, cUNOPo->op_first, 0, topop);
                }
                assert(topop == OpSIBLING(cBINOPo->op_first));
                if (toparg->p)
                    op_null(topop);
                lastkidop = topop;
            }
        }

        if (is_targable) {
            /* optimise
             *  my $lex  = A.B.C...
             *     $lex  = A.B.C...
             *     $lex .= A.B.C...
             * The original padsv op is kept but nulled in case it's the
             * entry point for the optree (which it will be for
             * '$lex .=  ... '
             */
            private_flags |= OPpTARGET_MY;
            private_flags |= (targetop->op_private & OPpLVAL_INTRO);
            o->op_targ = targetop->op_targ;
            targetop->op_targ = 0;
            op_null(targetop);
        }
        else
            flags |= OPf_STACKED;
    }
    else if (targmyop) {
        private_flags |= OPpTARGET_MY;
        if (o != targmyop) {
            o->op_targ = targmyop->op_targ;
            targmyop->op_targ = 0;
        }
    }

    /* detach the emaciated husk of the sprintf/concat optree and free it */
    for (;;) {
        kid = op_sibling_splice(o, lastkidop, 1, NULL);
        if (!kid)
            break;
        op_free(kid);
    }

    /* and convert o into a multiconcat */

    o->op_flags        = (flags|OPf_KIDS|stacked_last
                         |(o->op_flags & (OPf_WANT|OPf_PARENS)));
    o->op_private      = private_flags;
    o->op_type         = OP_MULTICONCAT;
    o->op_ppaddr       = PL_ppaddr[OP_MULTICONCAT];
    cUNOP_AUXo->op_aux = aux;
}


/*
=for apidoc_section $optree_manipulation

=for apidoc optimize_optree

This function applies some optimisations to the optree in top-down order.
It is called before the peephole optimizer, which processes ops in
execution order. Note that finalize_optree() also does a top-down scan,
but is called *after* the peephole optimizer.

=cut
*/

void
Perl_optimize_optree(pTHX_ OP* o)
{
    PERL_ARGS_ASSERT_OPTIMIZE_OPTREE;

    ENTER;
    SAVEVPTR(PL_curcop);

    optimize_op(o);

    LEAVE;
}


#define warn_implicit_snail_cvsig(o)  S_warn_implicit_snail_cvsig(aTHX_ o)
static void
S_warn_implicit_snail_cvsig(pTHX_ OP *o)
{
    CV *cv = PL_compcv;
    while(cv && CvEVAL(cv))
        cv = CvOUTSIDE(cv);

    if(cv && CvSIGNATURE(cv))
        Perl_ck_warner_d(aTHX_ packWARN(WARN_EXPERIMENTAL__ARGS_ARRAY_WITH_SIGNATURES),
            "Implicit use of @_ in %s with signatured subroutine is experimental", OP_DESC(o));
}


#define OP_ZOOM(o)  (OP_TYPE_IS(o, OP_NULL) ? cUNOPx(o)->op_first : (o))

/* helper for optimize_optree() which optimises one op then recurses
 * to optimise any children.
 */

STATIC void
S_optimize_op(pTHX_ OP* o)
{
    OP *top_op = o;

    PERL_ARGS_ASSERT_OPTIMIZE_OP;

    while (1) {
        OP * next_kid = NULL;

        assert(o->op_type != OP_FREED);

        switch (o->op_type) {
        case OP_NEXTSTATE:
        case OP_DBSTATE:
            PL_curcop = ((COP*)o);		/* for warnings */
            break;


        case OP_CONCAT:
        case OP_SASSIGN:
        case OP_STRINGIFY:
        case OP_SPRINTF:
            S_maybe_multiconcat(aTHX_ o);
            break;

        case OP_SUBST:
            if (cPMOPo->op_pmreplrootu.op_pmreplroot) {
                /* we can't assume that op_pmreplroot->op_sibparent == o
                 * and that it is thus possible to walk back up the tree
                 * past op_pmreplroot. So, although we try to avoid
                 * recursing through op trees, do it here. After all,
                 * there are unlikely to be many nested s///e's within
                 * the replacement part of a s///e.
                 */
                optimize_op(cPMOPo->op_pmreplrootu.op_pmreplroot);
            }
            break;

        case OP_RV2AV:
        {
            OP *first = (o->op_flags & OPf_KIDS) ? cUNOPo->op_first : NULL;
            CV *cv = PL_compcv;
            while(cv && CvEVAL(cv))
                cv = CvOUTSIDE(cv);

            if(cv && CvSIGNATURE(cv) &&
                    OP_TYPE_IS(first, OP_GV) && cGVOPx_gv(first) == PL_defgv) {
                OP *parent = op_parent(o);
                while(OP_TYPE_IS(parent, OP_NULL))
                    parent = op_parent(parent);

                Perl_ck_warner_d(aTHX_ packWARN(WARN_EXPERIMENTAL__ARGS_ARRAY_WITH_SIGNATURES),
                    "Use of @_ in %s with signatured subroutine is experimental", OP_DESC(parent));
            }
            break;
        }

        case OP_SHIFT:
        case OP_POP:
            if(!CvUNIQUE(PL_compcv) && !(o->op_flags & OPf_KIDS))
                warn_implicit_snail_cvsig(o);
            break;

        case OP_ENTERSUB:
            if(!(o->op_flags & OPf_STACKED))
                warn_implicit_snail_cvsig(o);
            break;

        case OP_GOTO:
        {
            OP *first = (o->op_flags & OPf_KIDS) ? cUNOPo->op_first : NULL;
            OP *ffirst;
            if(OP_TYPE_IS(first, OP_SREFGEN) &&
                    (ffirst = OP_ZOOM(cUNOPx(first)->op_first)) &&
                    OP_TYPE_IS(ffirst, OP_RV2CV))
                warn_implicit_snail_cvsig(o);
            break;
        }

        default:
            break;
        }

        if (o->op_flags & OPf_KIDS)
            next_kid = cUNOPo->op_first;

        /* if a kid hasn't been nominated to process, continue with the
         * next sibling, or if no siblings left, go back to the parent's
         * siblings and so on
         */
        while (!next_kid) {
            if (o == top_op)
                return; /* at top; no parents/siblings to try */
            if (OpHAS_SIBLING(o))
                next_kid = o->op_sibparent;
            else
                o = o->op_sibparent; /*try parent's next sibling */
        }

      /* this label not yet used. Goto here if any code above sets
       * next-kid
       get_next_op:
       */
        o = next_kid;
    }
}

/*
=for apidoc finalize_optree

This function finalizes the optree.  Should be called directly after
the complete optree is built.  It does some additional
checking which can't be done in the normal C<ck_>xxx functions and makes
the tree thread-safe.

=cut
*/

void
Perl_finalize_optree(pTHX_ OP* o)
{
    PERL_ARGS_ASSERT_FINALIZE_OPTREE;

    ENTER;
    SAVEVPTR(PL_curcop);

    finalize_op(o);

    LEAVE;
}


/*
=for apidoc traverse_op_tree

Return the next op in a depth-first traversal of the op tree,
returning NULL when the traversal is complete.

The initial call must supply the root of the tree as both top and o.

For now it's static, but it may be exposed to the API in the future.

=cut
*/

STATIC OP*
S_traverse_op_tree(pTHX_ OP *top, OP *o) {
    OP *sib;

    PERL_ARGS_ASSERT_TRAVERSE_OP_TREE;

    if ((o->op_flags & OPf_KIDS) && cUNOPo->op_first) {
        return cUNOPo->op_first;
    }
    else if ((sib = OpSIBLING(o))) {
        return sib;
    }
    else {
        OP *parent = o->op_sibparent;
        assert(!(o->op_moresib));
        while (parent && parent != top) {
            OP *sib = OpSIBLING(parent);
            if (sib)
                return sib;
            parent = parent->op_sibparent;
        }

        return NULL;
    }
}

STATIC void
S_finalize_op(pTHX_ OP* o)
{
    OP * const top = o;
    PERL_ARGS_ASSERT_FINALIZE_OP;

    do {
        assert(o->op_type != OP_FREED);

        switch (o->op_type) {
        case OP_NEXTSTATE:
        case OP_DBSTATE:
            PL_curcop = ((COP*)o);		/* for warnings */
            break;
        case OP_EXEC:
            if (OpHAS_SIBLING(o)) {
                OP *sib = OpSIBLING(o);
                if ((  sib->op_type == OP_NEXTSTATE || sib->op_type == OP_DBSTATE)
                    && ckWARN(WARN_EXEC)
                    && OpHAS_SIBLING(sib))
                {
                    const OPCODE type = OpSIBLING(sib)->op_type;
                    if (type != OP_EXIT && type != OP_WARN && type != OP_DIE) {
                        const line_t oldline = CopLINE(PL_curcop);
                        CopLINE_set(PL_curcop, CopLINE((COP*)sib));
                        Perl_warner(aTHX_ packWARN(WARN_EXEC),
                            "Statement unlikely to be reached");
                        Perl_warner(aTHX_ packWARN(WARN_EXEC),
                            "\t(Maybe you meant system() when you said exec()?)\n");
                        CopLINE_set(PL_curcop, oldline);
                    }
                }
            }
            break;

        case OP_GV:
            if ((o->op_private & OPpEARLY_CV) && ckWARN(WARN_PROTOTYPE)) {
                GV * const gv = cGVOPo_gv;
                if (SvTYPE(gv) == SVt_PVGV && GvCV(gv) && SvPVX_const(GvCV(gv))) {
                    /* XXX could check prototype here instead of just carping */
                    SV * const sv = sv_newmortal();
                    gv_efullname3(sv, gv, NULL);
                    Perl_warner(aTHX_ packWARN(WARN_PROTOTYPE),
                                "%" SVf "() called too early to check prototype",
                                SVfARG(sv));
                }
            }
            break;

        case OP_CONST:
            if (cSVOPo->op_private & OPpCONST_STRICT)
                no_bareword_allowed(o);
#ifdef USE_ITHREADS
            /* FALLTHROUGH */
        case OP_HINTSEVAL:
            op_relocate_sv(&cSVOPo->op_sv, &o->op_targ);
#endif
            break;

#ifdef USE_ITHREADS
            /* Relocate all the METHOP's SVs to the pad for thread safety. */
        case OP_METHOD_NAMED:
        case OP_METHOD_SUPER:
        case OP_METHOD_REDIR:
        case OP_METHOD_REDIR_SUPER:
            op_relocate_sv(&cMETHOPo->op_u.op_meth_sv, &o->op_targ);
            break;
#endif

        case OP_HELEM: {
            UNOP *rop;
            SVOP *key_op;
            OP *kid;

            if ((key_op = cSVOPx(cBINOPo->op_last))->op_type != OP_CONST)
                break;

            rop = cUNOPx(cBINOPo->op_first);

            goto check_keys;

            case OP_HSLICE:
                S_scalar_slice_warning(aTHX_ o);
                /* FALLTHROUGH */

            case OP_KVHSLICE:
                kid = OpSIBLING(cLISTOPo->op_first);
            if (/* I bet there's always a pushmark... */
                OP_TYPE_ISNT_AND_WASNT_NN(kid, OP_LIST)
                && OP_TYPE_ISNT_NN(kid, OP_CONST))
            {
                break;
            }

            key_op = cSVOPx(kid->op_type == OP_CONST
                             ? kid
                             : OpSIBLING(kLISTOP->op_first));

            rop = cUNOPx(cLISTOPo->op_last);

        check_keys:
            if (o->op_private & OPpLVAL_INTRO || rop->op_type != OP_RV2HV)
                rop = NULL;
            check_hash_fields_and_hekify(rop, key_op, 1);
            break;
        }
        case OP_NULL:
            if (o->op_targ != OP_HSLICE && o->op_targ != OP_ASLICE)
                break;
            /* FALLTHROUGH */
        case OP_ASLICE:
            S_scalar_slice_warning(aTHX_ o);
            break;

        case OP_SUBST: {
            if (cPMOPo->op_pmreplrootu.op_pmreplroot)
                finalize_op(cPMOPo->op_pmreplrootu.op_pmreplroot);
            break;
        }
        default:
            break;
        }

#ifdef DEBUGGING
        if (o->op_flags & OPf_KIDS) {
            OP *kid;

            /* check that op_last points to the last sibling, and that
             * the last op_sibling/op_sibparent field points back to the
             * parent, and that the only ops with KIDS are those which are
             * entitled to them */
            U32 type = o->op_type;
            U32 family;
            bool has_last;

            if (type == OP_NULL) {
                type = o->op_targ;
                /* ck_glob creates a null UNOP with ex-type GLOB
                 * (which is a list op. So pretend it wasn't a listop */
                if (type == OP_GLOB)
                    type = OP_NULL;
            }
            family = PL_opargs[type] & OA_CLASS_MASK;

            has_last = (   family == OA_BINOP
                        || family == OA_LISTOP
                        || family == OA_PMOP
                        || family == OA_LOOP
                       );
            assert(  has_last /* has op_first and op_last, or ...
                  ... has (or may have) op_first: */
                  || family == OA_UNOP
                  || family == OA_UNOP_AUX
                  || family == OA_LOGOP
                  || family == OA_BASEOP_OR_UNOP
                  || family == OA_FILESTATOP
                  || family == OA_LOOPEXOP
                  || family == OA_METHOP
                  || type == OP_CUSTOM
                  || type == OP_NULL /* new_logop does this */
                  );

            for (kid = cUNOPo->op_first; kid; kid = OpSIBLING(kid)) {
                if (!OpHAS_SIBLING(kid)) {
                    if (has_last)
                        assert(kid == cLISTOPo->op_last);
                    assert(kid->op_sibparent == o);
                }
            }
        }
#endif
    } while (( o = traverse_op_tree(top, o)) != NULL);
}


/*
   ---------------------------------------------------------

   Common vars in list assignment

   There now follows some enums and static functions for detecting
   common variables in list assignments. Here is a little essay I wrote
   for myself when trying to get my head around this. DAPM.

   ----

   First some random observations:

   * If a lexical var is an alias of something else, e.g.
       for my $x ($lex, $pkg, $a[0]) {...}
     then the act of aliasing will increase the reference count of the SV

   * If a package var is an alias of something else, it may still have a
     reference count of 1, depending on how the alias was created, e.g.
     in *a = *b, $a may have a refcount of 1 since the GP is shared
     with a single GvSV pointer to the SV. So If it's an alias of another
     package var, then RC may be 1; if it's an alias of another scalar, e.g.
     a lexical var or an array element, then it will have RC > 1.

   * There are many ways to create a package alias; ultimately, XS code
     may quite legally do GvSV(gv) = SvREFCNT_inc(sv) for example, so
     run-time tracing mechanisms are unlikely to be able to catch all cases.

   * When the LHS is all my declarations, the same vars can't appear directly
     on the RHS, but they can indirectly via closures, aliasing and lvalue
     subs. But those techniques all involve an increase in the lexical
     scalar's ref count.

   * When the LHS is all lexical vars (but not necessarily my declarations),
     it is possible for the same lexicals to appear directly on the RHS, and
     without an increased ref count, since the stack isn't refcounted.
     This case can be detected at compile time by scanning for common lex
     vars with PL_generation.

   * lvalue subs defeat common var detection, but they do at least
     return vars with a temporary ref count increment. Also, you can't
     tell at compile time whether a sub call is lvalue.


   So...

   A: There are a few circumstances where there definitely can't be any
     commonality:

       LHS empty:  () = (...);
       RHS empty:  (....) = ();
       RHS contains only constants or other 'can't possibly be shared'
           elements (e.g. ops that return PADTMPs):  (...) = (1,2, length)
           i.e. they only contain ops not marked as dangerous, whose children
           are also not dangerous;
       LHS ditto;
       LHS contains a single scalar element: e.g. ($x) = (....); because
           after $x has been modified, it won't be used again on the RHS;
       RHS contains a single element with no aggregate on LHS: e.g.
           ($a,$b,$c)  = ($x); again, once $a has been modified, its value
           won't be used again.

   B: If LHS are all 'my' lexical var declarations (or safe ops, which
     we can ignore):

       my ($a, $b, @c) = ...;

       Due to closure and goto tricks, these vars may already have content.
       For the same reason, an element on the RHS may be a lexical or package
       alias of one of the vars on the left, or share common elements, for
       example:

           my ($x,$y) = f(); # $x and $y on both sides
           sub f : lvalue { ($x,$y) = (1,2); $y, $x }

       and

           my $ra = f();
           my @a = @$ra;  # elements of @a on both sides
           sub f { @a = 1..4; \@a }


       First, just consider scalar vars on LHS:

           RHS is safe only if (A), or in addition,
               * contains only lexical *scalar* vars, where neither side's
                 lexicals have been flagged as aliases

           If RHS is not safe, then it's always legal to check LHS vars for
           RC==1, since the only RHS aliases will always be associated
           with an RC bump.

           Note that in particular, RHS is not safe if:

               * it contains package scalar vars; e.g.:

                   f();
                   my ($x, $y) = (2, $x_alias);
                   sub f { $x = 1; *x_alias = \$x; }

               * It contains other general elements, such as flattened or
               * spliced or single array or hash elements, e.g.

                   f();
                   my ($x,$y) = @a; # or $a[0] or @a{@b} etc

                   sub f {
                       ($x, $y) = (1,2);
                       use feature 'refaliasing';
                       \($a[0], $a[1]) = \($y,$x);
                   }

                 It doesn't matter if the array/hash is lexical or package.

               * it contains a function call that happens to be an lvalue
                 sub which returns one or more of the above, e.g.

                   f();
                   my ($x,$y) = f();

                   sub f : lvalue {
                       ($x, $y) = (1,2);
                       *x1 = \$x;
                       $y, $x1;
                   }

                   (so a sub call on the RHS should be treated the same
                   as having a package var on the RHS).

               * any other "dangerous" thing, such an op or built-in that
                 returns one of the above, e.g. pp_preinc


           If RHS is not safe, what we can do however is at compile time flag
           that the LHS are all my declarations, and at run time check whether
           all the LHS have RC == 1, and if so skip the full scan.

       Now consider array and hash vars on LHS: e.g. my (...,@a) = ...;

           Here the issue is whether there can be elements of @a on the RHS
           which will get prematurely freed when @a is cleared prior to
           assignment. This is only a problem if the aliasing mechanism
           is one which doesn't increase the refcount - only if RC == 1
           will the RHS element be prematurely freed.

           Because the array/hash is being INTROed, it or its elements
           can't directly appear on the RHS:

               my (@a) = ($a[0], @a, etc) # NOT POSSIBLE

           but can indirectly, e.g.:

               my $r = f();
               my (@a) = @$r;
               sub f { @a = 1..3; \@a }

           So if the RHS isn't safe as defined by (A), we must always
           mortalise and bump the ref count of any remaining RHS elements
           when assigning to a non-empty LHS aggregate.

           Lexical scalars on the RHS aren't safe if they've been involved in
           aliasing, e.g.

               use feature 'refaliasing';

               f();
               \(my $lex) = \$pkg;
               my @a = ($lex,3); # equivalent to ($a[0],3)

               sub f {
                   @a = (1,2);
                   \$pkg = \$a[0];
               }

           Similarly with lexical arrays and hashes on the RHS:

               f();
               my @b;
               my @a = (@b);

               sub f {
                   @a = (1,2);
                   \$b[0] = \$a[1];
                   \$b[1] = \$a[0];
               }



   C: As (B), but in addition the LHS may contain non-intro lexicals, e.g.
       my $a; ($a, my $b) = (....);

       The difference between (B) and (C) is that it is now physically
       possible for the LHS vars to appear on the RHS too, where they
       are not reference counted; but in this case, the compile-time
       PL_generation sweep will detect such common vars.

       So the rules for (C) differ from (B) in that if common vars are
       detected, the runtime "test RC==1" optimisation can no longer be used,
       and a full mark and sweep is required

   D: As (C), but in addition the LHS may contain package vars.

       Since package vars can be aliased without a corresponding refcount
       increase, all bets are off. It's only safe if (A). E.g.

           my ($x, $y) = (1,2);

           for $x_alias ($x) {
               ($x_alias, $y) = (3, $x); # whoops
           }

       Ditto for LHS aggregate package vars.

   E: Any other dangerous ops on LHS, e.g.
           (f(), $a[0], @$r) = (...);

       this is similar to (E) in that all bets are off. In addition, it's
       impossible to determine at compile time whether the LHS
       contains a scalar or an aggregate, e.g.

           sub f : lvalue { @a }
           (f()) = 1..3;

* ---------------------------------------------------------
*/

/* A set of bit flags returned by S_aassign_scan(). Each flag indicates
 * that at least one of the things flagged was seen.
 */

enum {
    AAS_MY_SCALAR       = 0x001, /* my $scalar */
    AAS_MY_AGG          = 0x002, /* aggregate: my @array or my %hash */
    AAS_LEX_SCALAR      = 0x004, /* $lexical */
    AAS_LEX_AGG         = 0x008, /* @lexical or %lexical aggregate */
    AAS_LEX_SCALAR_COMM = 0x010, /* $lexical seen on both sides */
    AAS_PKG_SCALAR      = 0x020, /* $scalar (where $scalar is pkg var) */
    AAS_PKG_AGG         = 0x040, /* package @array or %hash aggregate */
    AAS_DANGEROUS       = 0x080, /* an op (other than the above)
                                         that's flagged OA_DANGEROUS */
    AAS_SAFE_SCALAR     = 0x100, /* produces at least one scalar SV that's
                                        not in any of the categories above */
    AAS_DEFAV           = 0x200  /* contains just a single '@_' on RHS */
};

/* helper function for S_aassign_scan().
 * check a PAD-related op for commonality and/or set its generation number.
 * Returns a boolean indicating whether its shared */

static bool
S_aassign_padcheck(pTHX_ OP* o, bool rhs)
{
    if (PAD_COMPNAME_GEN(o->op_targ) == PERL_INT_MAX)
        /* lexical used in aliasing */
        return TRUE;

    if (rhs)
        return cBOOL(PAD_COMPNAME_GEN(o->op_targ) == (STRLEN)PL_generation);
    else
        PAD_COMPNAME_GEN_set(o->op_targ, PL_generation);

    return FALSE;
}

/*
  Helper function for OPpASSIGN_COMMON* detection in rpeep().
  It scans the left or right hand subtree of the aassign op, and returns a
  set of flags indicating what sorts of things it found there.
  'rhs' indicates whether we're scanning the LHS or RHS. If the former, we
  set PL_generation on lexical vars; if the latter, we see if
  PL_generation matches.
  'scalars_p' is a pointer to a counter of the number of scalar SVs seen.
  This fn will increment it by the number seen. It's not intended to
  be an accurate count (especially as many ops can push a variable
  number of SVs onto the stack); rather it's used as to test whether there
  can be at most 1 SV pushed; so it's only meanings are "0, 1, many".
*/

static int
S_aassign_scan(pTHX_ OP* o, bool rhs, int *scalars_p)
{
    OP *top_op           = o;
    OP *effective_top_op = o;
    int all_flags = 0;

    while (1) {
        bool top = o == effective_top_op;
        int flags = 0;
        OP* next_kid = NULL;

        /* first, look for a solitary @_ on the RHS */
        if (   rhs
            && top
            && (o->op_flags & OPf_KIDS)
            && OP_TYPE_IS_OR_WAS(o, OP_LIST)
        ) {
            OP *kid = cUNOPo->op_first;
            if (   (   kid->op_type == OP_PUSHMARK
                    || kid->op_type == OP_PADRANGE) /* ex-pushmark */
                && ((kid = OpSIBLING(kid)))
                && !OpHAS_SIBLING(kid)
                && kid->op_type == OP_RV2AV
                && !(kid->op_flags & OPf_REF)
                && !(kid->op_private & (OPpLVAL_INTRO|OPpMAYBE_LVSUB))
                && ((kid->op_flags & OPf_WANT) == OPf_WANT_LIST)
                && ((kid = cUNOPx(kid)->op_first))
                && kid->op_type == OP_GV
                && cGVOPx_gv(kid) == PL_defgv
            )
                flags = AAS_DEFAV;
        }

        switch (o->op_type) {
        case OP_GVSV:
            (*scalars_p)++;
            all_flags |= AAS_PKG_SCALAR;
            goto do_next;

        case OP_PADAV:
        case OP_PADHV:
            (*scalars_p) += 2;
            /* if !top, could be e.g. @a[0,1] */
            all_flags |=  (top && (o->op_flags & OPf_REF))
                            ? ((o->op_private & OPpLVAL_INTRO)
                                ? AAS_MY_AGG : AAS_LEX_AGG)
                            : AAS_DANGEROUS;
            goto do_next;

        case OP_PADSV:
            {
                int comm = S_aassign_padcheck(aTHX_ o, rhs)
                            ?  AAS_LEX_SCALAR_COMM : 0;
                (*scalars_p)++;
                all_flags |= (o->op_private & OPpLVAL_INTRO)
                    ? (AAS_MY_SCALAR|comm) : (AAS_LEX_SCALAR|comm);
                goto do_next;

            }

        case OP_RV2AV:
        case OP_RV2HV:
            (*scalars_p) += 2;
            if (cUNOPx(o)->op_first->op_type != OP_GV)
                all_flags |= AAS_DANGEROUS; /* @{expr}, %{expr} */
            /* @pkg, %pkg */
            /* if !top, could be e.g. @a[0,1] */
            else if (top && (o->op_flags & OPf_REF))
                all_flags |= AAS_PKG_AGG;
            else
                all_flags |= AAS_DANGEROUS;
            goto do_next;

        case OP_RV2SV:
            (*scalars_p)++;
            if (cUNOPx(o)->op_first->op_type != OP_GV) {
                (*scalars_p) += 2;
                all_flags |= AAS_DANGEROUS; /* ${expr} */
            }
            else
                all_flags |= AAS_PKG_SCALAR; /* $pkg */
            goto do_next;

        case OP_SPLIT:
            if (o->op_private & OPpSPLIT_ASSIGN) {
                /* the assign in @a = split() has been optimised away
                 * and the @a attached directly to the split op
                 * Treat the array as appearing on the RHS, i.e.
                 *    ... = (@a = split)
                 * is treated like
                 *    ... = @a;
                 */

                if (o->op_flags & OPf_STACKED) {
                    /* @{expr} = split() - the array expression is tacked
                     * on as an extra child to split - process kid */
                    next_kid = cLISTOPo->op_last;
                    goto do_next;
                }

                /* ... else array is directly attached to split op */
                (*scalars_p) += 2;
                all_flags |= (PL_op->op_private & OPpSPLIT_LEX)
                                ? ((o->op_private & OPpLVAL_INTRO)
                                    ? AAS_MY_AGG : AAS_LEX_AGG)
                                : AAS_PKG_AGG;
                goto do_next;
            }
            (*scalars_p)++;
            /* other args of split can't be returned */
            all_flags |= AAS_SAFE_SCALAR;
            goto do_next;

        case OP_UNDEF:
            /* undef on LHS following a var is significant, e.g.
             *    my $x = 1;
             *    @a = (($x, undef) = (2 => $x));
             *    # @a shoul be (2,1) not (2,2)
             *
             * undef on RHS counts as a scalar:
             *   ($x, $y)    = (undef, $x); # 2 scalars on RHS: unsafe
             */
            if ((!rhs && *scalars_p) || rhs)
                (*scalars_p)++;
            flags = AAS_SAFE_SCALAR;
            break;

        case OP_PUSHMARK:
        case OP_STUB:
            /* these are all no-ops; they don't push a potentially common SV
             * onto the stack, so they are neither AAS_DANGEROUS nor
             * AAS_SAFE_SCALAR */
            goto do_next;

        case OP_PADRANGE: /* Ignore padrange; checking its siblings is enough */
            break;

        case OP_NULL:
        case OP_LIST:
            /* these do nothing, but may have children */
            break;

        default:
            if (PL_opargs[o->op_type] & OA_DANGEROUS) {
                (*scalars_p) += 2;
                flags = AAS_DANGEROUS;
                break;
            }

            if (   (PL_opargs[o->op_type] & OA_TARGLEX)
                && (o->op_private & OPpTARGET_MY))
            {
                (*scalars_p)++;
                all_flags |= S_aassign_padcheck(aTHX_ o, rhs)
                                ? AAS_LEX_SCALAR_COMM : AAS_LEX_SCALAR;
                goto do_next;
            }

            /* if its an unrecognised, non-dangerous op, assume that it
             * is the cause of at least one safe scalar */
            (*scalars_p)++;
            flags = AAS_SAFE_SCALAR;
            break;
        }

        all_flags |= flags;

        /* by default, process all kids next
         * XXX this assumes that all other ops are "transparent" - i.e. that
         * they can return some of their children. While this true for e.g.
         * sort and grep, it's not true for e.g. map. We really need a
         * 'transparent' flag added to regen/opcodes
         */
        if (o->op_flags & OPf_KIDS) {
            next_kid = cUNOPo->op_first;
            /* these ops do nothing but may have children; but their
             * children should also be treated as top-level */
            if (   o == effective_top_op
                && (o->op_type == OP_NULL || o->op_type == OP_LIST)
            )
                effective_top_op = next_kid;
        }


        /* If next_kid is set, someone in the code above wanted us to process
         * that kid and all its remaining siblings.  Otherwise, work our way
         * back up the tree */
      do_next:
        while (!next_kid) {
            if (o == top_op)
                return all_flags; /* at top; no parents/siblings to try */
            if (OpHAS_SIBLING(o)) {
                next_kid = o->op_sibparent;
                if (o == effective_top_op)
                    effective_top_op = next_kid;
            }
            else if (o == effective_top_op)
              effective_top_op = o->op_sibparent;
            o = o->op_sibparent; /* try parent's next sibling */
        }
        o = next_kid;
    } /* while */
}

/* S_maybe_multideref(): given an op_next chain of ops beginning at 'start'
 * that potentially represent a series of one or more aggregate derefs
 * (such as $a->[1]{$key}), examine the chain, and if appropriate, convert
 * the whole chain to a single OP_MULTIDEREF op (maybe with a few
 * additional ops left in too).
 *
 * The caller will have already verified that the first few ops in the
 * chain following 'start' indicate a multideref candidate, and will have
 * set 'orig_o' to the point further on in the chain where the first index
 * expression (if any) begins.  'orig_action' specifies what type of
 * beginning has already been determined by the ops between start..orig_o
 * (e.g.  $lex_ary[], $pkg_ary->{}, expr->[], etc).
 *
 * 'hints' contains any hints flags that need adding (currently just
 * OPpHINT_STRICT_REFS) as found in any rv2av/hv skipped by the caller.
 */

STATIC void
S_maybe_multideref(pTHX_ OP *start, OP *orig_o, UV orig_action, U8 hints)
{
    int pass;
    UNOP_AUX_item *arg_buf = NULL;
    bool reset_start_targ  = FALSE; /* start->op_targ needs zeroing */
    int index_skip         = -1;    /* don't output index arg on this action */

    /* similar to regex compiling, do two passes; the first pass
     * determines whether the op chain is convertible and calculates the
     * buffer size; the second pass populates the buffer and makes any
     * changes necessary to ops (such as moving consts to the pad on
     * threaded builds).
     *
     * NB: for things like Coverity, note that both passes take the same
     * path through the logic tree (except for 'if (pass)' bits), since
     * both passes are following the same op_next chain; and in
     * particular, if it would return early on the second pass, it would
     * already have returned early on the first pass.
     */
    for (pass = 0; pass < 2; pass++) {
        OP *o                = orig_o;
        UV action            = orig_action;
        OP *first_elem_op    = NULL;  /* first seen aelem/helem */
        OP *top_op           = NULL;  /* highest [ah]elem/exists/del/rv2[ah]v */
        int action_count     = 0;     /* number of actions seen so far */
        int action_ix        = 0;     /* action_count % (actions per IV) */
        bool next_is_hash    = FALSE; /* is the next lookup to be a hash? */
        bool is_last         = FALSE; /* no more derefs to follow */
        bool maybe_aelemfast = FALSE; /* we can replace with aelemfast? */
        UV action_word       = 0;     /* all actions so far */
        size_t argi          = 0;
        UNOP_AUX_item *action_ptr = arg_buf;

        argi++; /* reserve slot for first action word */

        switch (action) {
        case MDEREF_HV_gvsv_vivify_rv2hv_helem:
        case MDEREF_HV_gvhv_helem:
            next_is_hash = TRUE;
            /* FALLTHROUGH */
        case MDEREF_AV_gvsv_vivify_rv2av_aelem:
        case MDEREF_AV_gvav_aelem:
            if (pass) {
#ifdef USE_ITHREADS
                arg_buf[argi].pad_offset = cPADOPx(start)->op_padix;
                /* stop it being swiped when nulled */
                cPADOPx(start)->op_padix = 0;
#else
                arg_buf[argi].sv = cSVOPx(start)->op_sv;
                cSVOPx(start)->op_sv = NULL;
#endif
            }
            argi++;
            break;

        case MDEREF_HV_padhv_helem:
        case MDEREF_HV_padsv_vivify_rv2hv_helem:
            next_is_hash = TRUE;
            /* FALLTHROUGH */
        case MDEREF_AV_padav_aelem:
        case MDEREF_AV_padsv_vivify_rv2av_aelem:
            if (pass) {
                arg_buf[argi].pad_offset = start->op_targ;
                /* we skip setting op_targ = 0 for now, since the intact
                 * OP_PADXV is needed by check_hash_fields_and_hekify */
                reset_start_targ = TRUE;
            }
            argi++;
            break;

        case MDEREF_HV_pop_rv2hv_helem:
            next_is_hash = TRUE;
            /* FALLTHROUGH */
        case MDEREF_AV_pop_rv2av_aelem:
            break;

        default:
            NOT_REACHED; /* NOTREACHED */
            return;
        }

        while (!is_last) {
            /* look for another (rv2av/hv; get index;
             * aelem/helem/exists/delele) sequence */

            OP *kid;
            bool is_deref;
            bool ok;
            UV index_type = MDEREF_INDEX_none;

            if (action_count) {
                /* if this is not the first lookup, consume the rv2av/hv  */

                /* for N levels of aggregate lookup, we normally expect
                 * that the first N-1 [ah]elem ops will be flagged as
                 * /DEREF (so they autovivify if necessary), and the last
                 * lookup op not to be.
                 * For other things (like @{$h{k1}{k2}}) extra scope or
                 * leave ops can appear, so abandon the effort in that
                 * case */
                if (o->op_type != OP_RV2AV && o->op_type != OP_RV2HV)
                    return;

                /* rv2av or rv2hv sKR/1 */

                ASSUME(!(o->op_flags & ~(OPf_WANT|OPf_KIDS|OPf_PARENS
                                            |OPf_REF|OPf_MOD|OPf_SPECIAL)));
                if (o->op_flags != (OPf_WANT_SCALAR|OPf_KIDS|OPf_REF))
                    return;

                /* at this point, we wouldn't expect any of these
                 * possible private flags:
                 * OPpMAYBE_LVSUB, OPpOUR_INTRO, OPpLVAL_INTRO
                 * OPpTRUEBOOL, OPpMAYBE_TRUEBOOL (rv2hv only)
                 */
                ASSUME(!(o->op_private &
                    ~(OPpHINT_STRICT_REFS|OPpARG1_MASK|OPpSLICEWARNING)));

                hints = (o->op_private & OPpHINT_STRICT_REFS);

                /* make sure the type of the previous /DEREF matches the
                 * type of the next lookup */
                ASSUME(o->op_type == (next_is_hash ? OP_RV2HV : OP_RV2AV));
                top_op = o;

                action = next_is_hash
                            ? MDEREF_HV_vivify_rv2hv_helem
                            : MDEREF_AV_vivify_rv2av_aelem;
                o = o->op_next;
            }

            /* if this is the second pass, and we're at the depth where
             * previously we encountered a non-simple index expression,
             * stop processing the index at this point */
            if (action_count != index_skip) {

                /* look for one or more simple ops that return an array
                 * index or hash key */

                switch (o->op_type) {
                case OP_PADSV:
                    /* it may be a lexical var index */
                    ASSUME(!(o->op_flags & ~(OPf_WANT|OPf_PARENS
                                            |OPf_REF|OPf_MOD|OPf_SPECIAL)));
                    ASSUME(!(o->op_private &
                            ~(OPpPAD_STATE|OPpDEREF|OPpLVAL_INTRO)));

                    if (   OP_GIMME(o,0) == G_SCALAR
                        && !(o->op_flags & (OPf_REF|OPf_MOD))
                        && o->op_private == 0)
                    {
                        if (pass)
                            arg_buf[argi].pad_offset = o->op_targ;
                        argi++;
                        index_type = MDEREF_INDEX_padsv;
                        o = o->op_next;
                    }
                    break;

                case OP_CONST:
                    if (next_is_hash) {
                        /* it's a constant hash index */
                        if (!(SvFLAGS(cSVOPo_sv) & (SVf_IOK|SVf_NOK|SVf_POK)))
                            /* "use constant foo => FOO; $h{+foo}" for
                             * some weird FOO, can leave you with constants
                             * that aren't simple strings. It's not worth
                             * the extra hassle for those edge cases */
                            break;

                        {
                            UNOP *rop = NULL;
                            OP * helem_op = o->op_next;

                            ASSUME(   helem_op->op_type == OP_HELEM
                                   || helem_op->op_type == OP_NULL
                                   || pass == 0);
                            if (helem_op->op_type == OP_HELEM) {
                                rop = cUNOPx(cBINOPx(helem_op)->op_first);
                                if (   helem_op->op_private & OPpLVAL_INTRO
                                    || rop->op_type != OP_RV2HV
                                )
                                    rop = NULL;
                            }
                            /* on first pass just check; on second pass
                             * hekify */
                            check_hash_fields_and_hekify(rop, cSVOPo, pass);
                        }

                        if (pass) {
#ifdef USE_ITHREADS
                            /* Relocate sv to the pad for thread safety */
                            op_relocate_sv(&cSVOPo->op_sv, &o->op_targ);
                            arg_buf[argi].pad_offset = o->op_targ;
                            o->op_targ = 0;
#else
                            arg_buf[argi].sv = cSVOPx_sv(o);
#endif
                        }
                    }
                    else {
                        /* it's a constant array index */
                        IV iv;
                        SV *ix_sv = cSVOPo->op_sv;
                        if (!SvIOK(ix_sv))
                            break;
                        iv = SvIV(ix_sv);

                        if (   action_count == 0
                            && iv >= -128
                            && iv <= 127
                            && (   action == MDEREF_AV_padav_aelem
                                || action == MDEREF_AV_gvav_aelem)
                        )
                            maybe_aelemfast = TRUE;

                        if (pass) {
                            arg_buf[argi].iv = iv;
                            SvREFCNT_dec_NN(cSVOPo->op_sv);
                        }
                    }
                    if (pass)
                        /* we've taken ownership of the SV */
                        cSVOPo->op_sv = NULL;
                    argi++;
                    index_type = MDEREF_INDEX_const;
                    o = o->op_next;
                    break;

                case OP_GV:
                    /* it may be a package var index */

                    ASSUME(!(o->op_flags & ~(OPf_WANT|OPf_PARENS|OPf_SPECIAL)));
                    ASSUME(!(o->op_private & ~(OPpEARLY_CV)));
                    if (  (o->op_flags & ~(OPf_PARENS|OPf_SPECIAL)) != OPf_WANT_SCALAR
                        || o->op_private != 0
                    )
                        break;

                    kid = o->op_next;
                    if (kid->op_type != OP_RV2SV)
                        break;

                    ASSUME(!(kid->op_flags &
                            ~(OPf_WANT|OPf_KIDS|OPf_MOD|OPf_REF
                             |OPf_SPECIAL|OPf_PARENS)));
                    ASSUME(!(kid->op_private &
                                    ~(OPpARG1_MASK
                                     |OPpHINT_STRICT_REFS|OPpOUR_INTRO
                                     |OPpDEREF|OPpLVAL_INTRO)));
                    if(   (kid->op_flags &~ OPf_PARENS)
                            != (OPf_WANT_SCALAR|OPf_KIDS)
                       || (kid->op_private & ~(OPpARG1_MASK|HINT_STRICT_REFS))
                    )
                        break;

                    if (pass) {
#ifdef USE_ITHREADS
                        arg_buf[argi].pad_offset = cPADOPx(o)->op_padix;
                        /* stop it being swiped when nulled */
                        cPADOPx(o)->op_padix = 0;
#else
                        arg_buf[argi].sv = cSVOPx(o)->op_sv;
                        cSVOPo->op_sv = NULL;
#endif
                    }
                    argi++;
                    index_type = MDEREF_INDEX_gvsv;
                    o = kid->op_next;
                    break;

                } /* switch */
            } /* action_count != index_skip */

            action |= index_type;


            /* at this point we have either:
             *   * detected what looks like a simple index expression,
             *     and expect the next op to be an [ah]elem, or
             *     an nulled  [ah]elem followed by a delete or exists;
             *  * found a more complex expression, so something other
             *    than the above follows.
             */

            /* possibly an optimised away [ah]elem (where op_next is
             * exists or delete) */
            if (o->op_type == OP_NULL)
                o = o->op_next;

            /* at this point we're looking for an OP_AELEM, OP_HELEM,
             * OP_EXISTS or OP_DELETE */

            /* if a custom array/hash access checker is in scope,
             * abandon optimisation attempt */
            if (  (o->op_type == OP_AELEM || o->op_type == OP_HELEM)
               && PL_check[o->op_type] != Perl_ck_null)
                return;
            /* similarly for customised exists and delete */
            if (  (o->op_type == OP_EXISTS)
               && PL_check[o->op_type] != Perl_ck_exists)
                return;
            if (  (o->op_type == OP_DELETE)
               && PL_check[o->op_type] != Perl_ck_delete)
                return;

            if (   o->op_type != OP_AELEM
                || (o->op_private &
                      (OPpLVAL_INTRO|OPpLVAL_DEFER|OPpDEREF|OPpMAYBE_LVSUB))
                )
                maybe_aelemfast = FALSE;

            /* look for aelem/helem/exists/delete. If it's not the last elem
             * lookup, it *must* have OPpDEREF_AV/HV, but not many other
             * flags; if it's the last, then it mustn't have
             * OPpDEREF_AV/HV, but may have lots of other flags, like
             * OPpLVAL_INTRO etc
             */

            if (   index_type == MDEREF_INDEX_none
                || (   o->op_type != OP_AELEM  && o->op_type != OP_HELEM
                    && o->op_type != OP_EXISTS && o->op_type != OP_DELETE)
            )
                ok = FALSE;
            else {
                /* we have aelem/helem/exists/delete with valid simple index */

                is_deref =    (o->op_type == OP_AELEM || o->op_type == OP_HELEM)
                           && (   (o->op_private & OPpDEREF) == OPpDEREF_AV
                               || (o->op_private & OPpDEREF) == OPpDEREF_HV);

                /* This doesn't make much sense but is legal:
                 *    @{ local $x[0][0] } = 1
                 * Since scope exit will undo the autovivification,
                 * don't bother in the first place. The OP_LEAVE
                 * assertion is in case there are other cases of both
                 * OPpLVAL_INTRO and OPpDEREF which don't include a scope
                 * exit that would undo the local - in which case this
                 * block of code would need rethinking.
                 */
                if (is_deref && (o->op_private & OPpLVAL_INTRO)) {
#ifdef DEBUGGING
                    OP *n = o->op_next;
                    while (n && (  n->op_type == OP_NULL
                                || n->op_type == OP_LIST
                                || n->op_type == OP_SCALAR))
                        n = n->op_next;
                    assert(n && n->op_type == OP_LEAVE);
#endif
                    o->op_private &= ~OPpDEREF;
                    is_deref = FALSE;
                }

                if (is_deref) {
                    ASSUME(!(o->op_flags &
                                 ~(OPf_WANT|OPf_KIDS|OPf_MOD|OPf_PARENS)));
                    ASSUME(!(o->op_private & ~(OPpARG2_MASK|OPpDEREF)));

                    ok =    (o->op_flags &~ OPf_PARENS)
                               == (OPf_WANT_SCALAR|OPf_KIDS|OPf_MOD)
                         && !(o->op_private & ~(OPpDEREF|OPpARG2_MASK));
                }
                else if (o->op_type == OP_EXISTS) {
                    ASSUME(!(o->op_flags & ~(OPf_WANT|OPf_KIDS|OPf_PARENS
                                |OPf_REF|OPf_MOD|OPf_SPECIAL)));
                    ASSUME(!(o->op_private & ~(OPpARG1_MASK|OPpEXISTS_SUB)));
                    ok =  !(o->op_private & ~OPpARG1_MASK);
                }
                else if (o->op_type == OP_DELETE) {
                    ASSUME(!(o->op_flags & ~(OPf_WANT|OPf_KIDS|OPf_PARENS
                                |OPf_REF|OPf_MOD|OPf_SPECIAL)));
                    ASSUME(!(o->op_private &
                                    ~(OPpARG1_MASK|OPpSLICE|OPpLVAL_INTRO)));
                    /* don't handle slices or 'local delete'; the latter
                     * is fairly rare, and has a complex runtime */
                    ok =  !(o->op_private & ~OPpARG1_MASK);
                    if (OP_TYPE_IS_OR_WAS(cUNOPo->op_first, OP_AELEM))
                        /* skip handling run-tome error */
                        ok = (ok && cBOOL(o->op_flags & OPf_SPECIAL));
                }
                else {
                    ASSUME(o->op_type == OP_AELEM || o->op_type == OP_HELEM);
                    ASSUME(!(o->op_flags & ~(OPf_WANT|OPf_KIDS|OPf_MOD
                                            |OPf_PARENS|OPf_REF|OPf_SPECIAL)));
                    ASSUME(!(o->op_private & ~(OPpARG2_MASK|OPpMAYBE_LVSUB
                                    |OPpLVAL_DEFER|OPpDEREF|OPpLVAL_INTRO)));
                    ok = (o->op_private & OPpDEREF) != OPpDEREF_SV;
                }
            }

            if (ok) {
                if (!first_elem_op)
                    first_elem_op = o;
                top_op = o;
                if (is_deref) {
                    next_is_hash = cBOOL((o->op_private & OPpDEREF) == OPpDEREF_HV);
                    o = o->op_next;
                }
                else {
                    is_last = TRUE;
                    action |= MDEREF_FLAG_last;
                }
            }
            else {
                /* at this point we have something that started
                 * promisingly enough (with rv2av or whatever), but failed
                 * to find a simple index followed by an
                 * aelem/helem/exists/delete. If this is the first action,
                 * give up; but if we've already seen at least one
                 * aelem/helem, then keep them and add a new action with
                 * MDEREF_INDEX_none, which causes it to do the vivify
                 * from the end of the previous lookup, and do the deref,
                 * but stop at that point. So $a[0][expr] will do one
                 * av_fetch, vivify and deref, then continue executing at
                 * expr */
                if (!action_count)
                    return;
                is_last = TRUE;
                index_skip = action_count;
                action |= MDEREF_FLAG_last;
                if (index_type != MDEREF_INDEX_none)
                    argi--;
            }

            action_word |= (action << (action_ix * MDEREF_SHIFT));
            action_ix++;
            action_count++;
            /* if there's no space for the next action, reserve a new slot
             * for it *before* we start adding args for that action */
            if ((action_ix + 1) * MDEREF_SHIFT > UVSIZE*8) {
                if (pass) {
                    action_ptr->uv = action_word;
                    action_ptr = arg_buf + argi;
                }
                action_word = 0;
                argi++;
                action_ix = 0;
            }
        } /* while !is_last */

        /* success! */

        if (!action_ix)
            /* slot reserved for next action word not now needed */
            argi--;
        else if (pass)
            action_ptr->uv = action_word;

        if (pass) {
            OP *mderef;
            OP *p, *q;

            mderef = newUNOP_AUX(OP_MULTIDEREF, 0, NULL, arg_buf);
            if (index_skip == -1) {
                mderef->op_flags = o->op_flags
                        & (OPf_WANT|OPf_MOD|(next_is_hash ? OPf_SPECIAL : 0));
                if (o->op_type == OP_EXISTS)
                    mderef->op_private = OPpMULTIDEREF_EXISTS;
                else if (o->op_type == OP_DELETE)
                    mderef->op_private = OPpMULTIDEREF_DELETE;
                else
                    mderef->op_private = o->op_private
                        & (OPpMAYBE_LVSUB|OPpLVAL_DEFER|OPpLVAL_INTRO);
            }
            /* accumulate strictness from every level (although I don't think
             * they can actually vary) */
            mderef->op_private |= hints;

            /* integrate the new multideref op into the optree and the
             * op_next chain.
             *
             * In general an op like aelem or helem has two child
             * sub-trees: the aggregate expression (a_expr) and the
             * index expression (i_expr):
             *
             *     aelem
             *       |
             *     a_expr - i_expr
             *
             * The a_expr returns an AV or HV, while the i-expr returns an
             * index. In general a multideref replaces most or all of a
             * multi-level tree, e.g.
             *
             *     exists
             *       |
             *     ex-aelem
             *       |
             *     rv2av  - i_expr1
             *       |
             *     helem
             *       |
             *     rv2hv  - i_expr2
             *       |
             *     aelem
             *       |
             *     a_expr - i_expr3
             *
             * With multideref, all the i_exprs will be simple vars or
             * constants, except that i_expr1 may be arbitrary in the case
             * of MDEREF_INDEX_none.
             *
             * The bottom-most a_expr will be either:
             *   1) a simple var (so padXv or gv+rv2Xv);
             *   2) a simple scalar var dereferenced (e.g. $r->[0]):
             *      so a simple var with an extra rv2Xv;
             *   3) or an arbitrary expression.
             *
             * 'start', the first op in the execution chain, will point to
             *   1),2): the padXv or gv op;
             *   3):    the rv2Xv which forms the last op in the a_expr
             *          execution chain, and the top-most op in the a_expr
             *          subtree.
             *
             * For all cases, the 'start' node is no longer required,
             * but we can't free it since one or more external nodes
             * may point to it. E.g. consider
             *     $h{foo} = $a ? $b : $c
             * Here, both the op_next and op_other branches of the
             * cond_expr point to the gv[*h] of the hash expression, so
             * we can't free the 'start' op.
             *
             * For expr->[...], we need to save the subtree containing the
             * expression; for the other cases, we just need to save the
             * start node.
             * So in all cases, we null the start op and keep it around by
             * making it the child of the multideref op; for the expr->
             * case, the expr will be a subtree of the start node.
             *
             * So in the simple 1,2 case the  optree above changes to
             *
             *     ex-exists
             *       |
             *     multideref
             *       |
             *     ex-gv (or ex-padxv)
             *
             *  with the op_next chain being
             *
             *  -> ex-gv -> multideref -> op-following-ex-exists ->
             *
             *  In the 3 case, we have
             *
             *     ex-exists
             *       |
             *     multideref
             *       |
             *     ex-rv2xv
             *       |
             *    rest-of-a_expr
             *      subtree
             *
             *  and
             *
             *  -> rest-of-a_expr subtree ->
             *    ex-rv2xv -> multideref -> op-following-ex-exists ->
             *
             *
             * Where the last i_expr is non-simple (i.e. MDEREF_INDEX_none,
             * e.g. $a[0]{foo}[$x+1], the next rv2xv is nulled and the
             * multideref attached as the child, e.g.
             *
             *     exists
             *       |
             *     ex-aelem
             *       |
             *     ex-rv2av  - i_expr1
             *       |
             *     multideref
             *       |
             *     ex-whatever
             *
             */

            /* if we free this op, don't free the pad entry */
            if (reset_start_targ)
                start->op_targ = 0;


            /* Cut the bit we need to save out of the tree and attach to
             * the multideref op, then free the rest of the tree */

            /* find parent of node to be detached (for use by splice) */
            p = first_elem_op;
            if (   orig_action == MDEREF_AV_pop_rv2av_aelem
                || orig_action == MDEREF_HV_pop_rv2hv_helem)
            {
                /* there is an arbitrary expression preceding us, e.g.
                 * expr->[..]? so we need to save the 'expr' subtree */
                if (p->op_type == OP_EXISTS || p->op_type == OP_DELETE)
                    p = cUNOPx(p)->op_first;
                ASSUME(   start->op_type == OP_RV2AV
                       || start->op_type == OP_RV2HV);
            }
            else {
                /* either a padXv or rv2Xv+gv, maybe with an ex-Xelem
                 * above for exists/delete. */
                while (   (p->op_flags & OPf_KIDS)
                       && cUNOPx(p)->op_first != start
                )
                    p = cUNOPx(p)->op_first;
            }
            ASSUME(cUNOPx(p)->op_first == start);

            /* detach from main tree, and re-attach under the multideref */
            op_sibling_splice(mderef, NULL, 0,
                    op_sibling_splice(p, NULL, 1, NULL));
            op_null(start);

            start->op_next = mderef;

            mderef->op_next = index_skip == -1 ? o->op_next : o;

            /* excise and free the original tree, and replace with
             * the multideref op */
            p = op_sibling_splice(top_op, NULL, -1, mderef);
            while (p) {
                q = OpSIBLING(p);
                op_free(p);
                p = q;
            }
            op_null(top_op);
        }
        else {
            Size_t size = argi;

            if (maybe_aelemfast && action_count == 1)
                return;

            arg_buf = (UNOP_AUX_item*)PerlMemShared_malloc(
                                sizeof(UNOP_AUX_item) * (size + 1));
            /* for dumping etc: store the length in a hidden first slot;
             * we set the op_aux pointer to the second slot */
            arg_buf->uv = size;
            arg_buf++;
        }
    } /* for (pass = ...) */
}

/* See if the ops following o are such that o will always be executed in
 * boolean context: that is, the SV which o pushes onto the stack will
 * only ever be consumed by later ops via SvTRUE(sv) or similar.
 * If so, set a suitable private flag on o. Normally this will be
 * bool_flag; but see below why maybe_flag is needed too.
 *
 * Typically the two flags you pass will be the generic OPpTRUEBOOL and
 * OPpMAYBE_TRUEBOOL, buts it's possible that for some ops those bits may
 * already be taken, so you'll have to give that op two different flags.
 *
 * More explanation of 'maybe_flag' and 'safe_and' parameters.
 * The binary logical ops &&, ||, // (plus 'if' and 'unless' which use
 * those underlying ops) short-circuit, which means that rather than
 * necessarily returning a truth value, they may return the LH argument,
 * which may not be boolean. For example in $x = (keys %h || -1), keys
 * should return a key count rather than a boolean, even though its
 * sort-of being used in boolean context.
 *
 * So we only consider such logical ops to provide boolean context to
 * their LH argument if they themselves are in void or boolean context.
 * However, sometimes the context isn't known until run-time. In this
 * case the op is marked with the maybe_flag flag it.
 *
 * Consider the following.
 *
 *     sub f { ....;  if (%h) { .... } }
 *
 * This is actually compiled as
 *
 *     sub f { ....;  %h && do { .... } }
 *
 * Here we won't know until runtime whether the final statement (and hence
 * the &&) is in void context and so is safe to return a boolean value.
 * So mark o with maybe_flag rather than the bool_flag.
 * Note that there is cost associated with determining context at runtime
 * (e.g. a call to block_gimme()), so it may not be worth setting (at
 * compile time) and testing (at runtime) maybe_flag if the scalar verses
 * boolean costs savings are marginal.
 *
 * However, we can do slightly better with && (compared to || and //):
 * this op only returns its LH argument when that argument is false. In
 * this case, as long as the op promises to return a false value which is
 * valid in both boolean and scalar contexts, we can mark an op consumed
 * by && with bool_flag rather than maybe_flag.
 * For example as long as pp_padhv and pp_rv2hv return &PL_sv_zero rather
 * than &PL_sv_no for a false result in boolean context, then it's safe. An
 * op which promises to handle this case is indicated by setting safe_and
 * to true.
 */

static void
S_check_for_bool_cxt(OP*o, bool safe_and, U8 bool_flag, U8 maybe_flag)
{
    OP *lop;
    U8 flag = 0;

    assert((o->op_flags & OPf_WANT) == OPf_WANT_SCALAR);

    /* OPpTARGET_MY and boolean context probably don't mix well.
     * If someone finds a valid use case, maybe add an extra flag to this
     * function which indicates its safe to do so for this op? */
    assert(!(   (PL_opargs[o->op_type] & OA_TARGLEX)
             && (o->op_private & OPpTARGET_MY)));

    lop = o->op_next;

    while (lop) {
        switch (lop->op_type) {
        case OP_NULL:
        case OP_SCALAR:
            break;

        /* these two consume the stack argument in the scalar case,
         * and treat it as a boolean in the non linenumber case */
        case OP_FLIP:
        case OP_FLOP:
            if (   ((lop->op_flags & OPf_WANT) == OPf_WANT_LIST)
                || (lop->op_private & OPpFLIP_LINENUM))
            {
                lop = NULL;
                break;
            }
            /* FALLTHROUGH */
        /* these never leave the original value on the stack */
        case OP_NOT:
        case OP_XOR:
        case OP_COND_EXPR:
        case OP_GREPWHILE:
            flag = bool_flag;
            lop = NULL;
            break;

        /* OR DOR and AND evaluate their arg as a boolean, but then may
         * leave the original scalar value on the stack when following the
         * op_next route. If not in void context, we need to ensure
         * that whatever follows consumes the arg only in boolean context
         * too.
         */
        case OP_AND:
            if (safe_and) {
                flag = bool_flag;
                lop = NULL;
                break;
            }
            /* FALLTHROUGH */
        case OP_OR:
        case OP_DOR:
            if ((lop->op_flags & OPf_WANT) == OPf_WANT_VOID) {
                flag = bool_flag;
                lop = NULL;
            }
            else if (!(lop->op_flags & OPf_WANT)) {
                /* unknown context - decide at runtime */
                flag = maybe_flag;
                lop = NULL;
            }
            break;

        default:
            lop = NULL;
            break;
        }

        if (lop)
            lop = lop->op_next;
    }

    o->op_private |= flag;
}

/* mechanism for deferring recursion in rpeep() */

#define MAX_DEFERRED 4

#define DEFER(o) \
  STMT_START { \
    if (defer_ix == (MAX_DEFERRED-1)) { \
        OP **defer = defer_queue[defer_base]; \
        CALL_RPEEP(*defer); \
        op_prune_chain_head(defer); \
        defer_base = (defer_base + 1) % MAX_DEFERRED; \
        defer_ix--; \
    } \
    defer_queue[(defer_base + ++defer_ix) % MAX_DEFERRED] = &(o); \
  } STMT_END

#define IS_AND_OP(o)   (o->op_type == OP_AND)
#define IS_OR_OP(o)    (o->op_type == OP_OR)

/* A peephole optimizer.  We visit the ops in the order they're to execute.
 * See the comments at the top of this file for more details about when
 * peep() is called */

void
Perl_rpeep(pTHX_ OP *o)
{
    OP* oldop = NULL;
    OP* oldoldop = NULL;
    OP** defer_queue[MAX_DEFERRED] = { NULL }; /* small queue of deferred branches */
    int defer_base = 0;
    int defer_ix = -1;

    if (!o || o->op_opt)
        return;

    assert(o->op_type != OP_FREED);

    ENTER;
    SAVEOP();
    SAVEVPTR(PL_curcop);
    for (;; o = o->op_next) {
        if (o && o->op_opt)
            o = NULL;
        if (!o) {
            while (defer_ix >= 0) {
                OP **defer =
                        defer_queue[(defer_base + defer_ix--) % MAX_DEFERRED];
                CALL_RPEEP(*defer);
                op_prune_chain_head(defer);
            }
            break;
        }

      redo:

        /* oldoldop -> oldop -> o should be a chain of 3 adjacent ops */
        assert(!oldoldop || oldoldop->op_next == oldop);
        assert(!oldop    || oldop->op_next    == o);

        /* By default, this op has now been optimised. A couple of cases below
           clear this again.  */
        o->op_opt = 1;
        PL_op = o;

        /* look for a series of 1 or more aggregate derefs, e.g.
         *   $a[1]{foo}[$i]{$k}
         * and replace with a single OP_MULTIDEREF op.
         * Each index must be either a const, or a simple variable,
         *
         * First, look for likely combinations of starting ops,
         * corresponding to (global and lexical variants of)
         *     $a[...]   $h{...}
         *     $r->[...] $r->{...}
         *     (preceding expression)->[...]
         *     (preceding expression)->{...}
         * and if so, call maybe_multideref() to do a full inspection
         * of the op chain and if appropriate, replace with an
         * OP_MULTIDEREF
         */
        {
            UV action;
            OP *o2 = o;
            U8 hints = 0;

            switch (o2->op_type) {
            case OP_GV:
                /* $pkg[..]   :   gv[*pkg]
                 * $pkg->[...]:   gv[*pkg]; rv2sv sKM/DREFAV */

                /* Fail if there are new op flag combinations that we're
                 * not aware of, rather than:
                 *  * silently failing to optimise, or
                 *  * silently optimising the flag away.
                 * If this ASSUME starts failing, examine what new flag
                 * has been added to the op, and decide whether the
                 * optimisation should still occur with that flag, then
                 * update the code accordingly. This applies to all the
                 * other ASSUMEs in the block of code too.
                 */
                ASSUME(!(o2->op_flags &
                            ~(OPf_WANT|OPf_MOD|OPf_PARENS|OPf_SPECIAL)));
                ASSUME(!(o2->op_private & ~OPpEARLY_CV));

                o2 = o2->op_next;

                if (o2->op_type == OP_RV2AV) {
                    action = MDEREF_AV_gvav_aelem;
                    goto do_deref;
                }

                if (o2->op_type == OP_RV2HV) {
                    action = MDEREF_HV_gvhv_helem;
                    goto do_deref;
                }

                if (o2->op_type != OP_RV2SV)
                    break;

                /* at this point we've seen gv,rv2sv, so the only valid
                 * construct left is $pkg->[] or $pkg->{} */

                ASSUME(!(o2->op_flags & OPf_STACKED));
                if ((o2->op_flags & (OPf_WANT|OPf_REF|OPf_MOD|OPf_SPECIAL))
                            != (OPf_WANT_SCALAR|OPf_MOD))
                    break;

                ASSUME(!(o2->op_private & ~(OPpARG1_MASK|HINT_STRICT_REFS
                                    |OPpOUR_INTRO|OPpDEREF|OPpLVAL_INTRO)));
                if (o2->op_private & (OPpOUR_INTRO|OPpLVAL_INTRO))
                    break;
                if (   (o2->op_private & OPpDEREF) != OPpDEREF_AV
                    && (o2->op_private & OPpDEREF) != OPpDEREF_HV)
                    break;

                o2 = o2->op_next;
                if (o2->op_type == OP_RV2AV) {
                    action = MDEREF_AV_gvsv_vivify_rv2av_aelem;
                    goto do_deref;
                }
                if (o2->op_type == OP_RV2HV) {
                    action = MDEREF_HV_gvsv_vivify_rv2hv_helem;
                    goto do_deref;
                }
                break;

            case OP_PADSV:
                /* $lex->[...]: padsv[$lex] sM/DREFAV */

                ASSUME(!(o2->op_flags &
                    ~(OPf_WANT|OPf_PARENS|OPf_REF|OPf_MOD|OPf_SPECIAL)));
                if ((o2->op_flags &
                        (OPf_WANT|OPf_REF|OPf_MOD|OPf_SPECIAL))
                     != (OPf_WANT_SCALAR|OPf_MOD))
                    break;

                ASSUME(!(o2->op_private &
                                ~(OPpPAD_STATE|OPpDEREF|OPpLVAL_INTRO)));
                /* skip if state or intro, or not a deref */
                if (      o2->op_private != OPpDEREF_AV
                       && o2->op_private != OPpDEREF_HV)
                    break;

                o2 = o2->op_next;
                if (o2->op_type == OP_RV2AV) {
                    action = MDEREF_AV_padsv_vivify_rv2av_aelem;
                    goto do_deref;
                }
                if (o2->op_type == OP_RV2HV) {
                    action = MDEREF_HV_padsv_vivify_rv2hv_helem;
                    goto do_deref;
                }
                break;

            case OP_PADAV:
            case OP_PADHV:
                /*    $lex[..]:  padav[@lex:1,2] sR *
                 * or $lex{..}:  padhv[%lex:1,2] sR */
                ASSUME(!(o2->op_flags & ~(OPf_WANT|OPf_MOD|OPf_PARENS|
                                            OPf_REF|OPf_SPECIAL)));
                if ((o2->op_flags &
                        (OPf_WANT|OPf_REF|OPf_MOD|OPf_SPECIAL))
                     != (OPf_WANT_SCALAR|OPf_REF))
                    break;
                if (o2->op_flags != (OPf_WANT_SCALAR|OPf_REF))
                    break;
                /* OPf_PARENS isn't currently used in this case;
                 * if that changes, let us know! */
                ASSUME(!(o2->op_flags & OPf_PARENS));

                /* at this point, we wouldn't expect any of the remaining
                 * possible private flags:
                 * OPpPAD_STATE, OPpLVAL_INTRO, OPpTRUEBOOL,
                 * OPpMAYBE_TRUEBOOL, OPpMAYBE_LVSUB
                 *
                 * OPpSLICEWARNING shouldn't affect runtime
                 */
                ASSUME(!(o2->op_private & ~(OPpSLICEWARNING)));

                action = o2->op_type == OP_PADAV
                            ? MDEREF_AV_padav_aelem
                            : MDEREF_HV_padhv_helem;
                o2 = o2->op_next;
                S_maybe_multideref(aTHX_ o, o2, action, 0);
                break;


            case OP_RV2AV:
            case OP_RV2HV:
                action = o2->op_type == OP_RV2AV
                            ? MDEREF_AV_pop_rv2av_aelem
                            : MDEREF_HV_pop_rv2hv_helem;
                /* FALLTHROUGH */
            do_deref:
                /* (expr)->[...]:  rv2av sKR/1;
                 * (expr)->{...}:  rv2hv sKR/1; */

                ASSUME(o2->op_type == OP_RV2AV || o2->op_type == OP_RV2HV);

                ASSUME(!(o2->op_flags & ~(OPf_WANT|OPf_KIDS|OPf_PARENS
                                |OPf_REF|OPf_MOD|OPf_STACKED|OPf_SPECIAL)));
                if (o2->op_flags != (OPf_WANT_SCALAR|OPf_KIDS|OPf_REF))
                    break;

                /* at this point, we wouldn't expect any of these
                 * possible private flags:
                 * OPpMAYBE_LVSUB, OPpLVAL_INTRO
                 * OPpTRUEBOOL, OPpMAYBE_TRUEBOOL, (rv2hv only)
                 */
                ASSUME(!(o2->op_private &
                    ~(OPpHINT_STRICT_REFS|OPpARG1_MASK|OPpSLICEWARNING
                     |OPpOUR_INTRO)));
                hints |= (o2->op_private & OPpHINT_STRICT_REFS);

                o2 = o2->op_next;

                S_maybe_multideref(aTHX_ o, o2, action, hints);
                break;

            default:
                break;
            }
        }


        switch (o->op_type) {
        case OP_DBSTATE:
            PL_curcop = ((COP*)o);		/* for warnings */
            break;
        case OP_NEXTSTATE:
            PL_curcop = ((COP*)o);		/* for warnings */

            /* Optimise a "return ..." at the end of a sub to just be "...".
             * This saves 2 ops. Before:
             * 1  <;> nextstate(main 1 -e:1) v ->2
             * 4  <@> return K ->5
             * 2    <0> pushmark s ->3
             * -    <1> ex-rv2sv sK/1 ->4
             * 3      <#> gvsv[*cat] s ->4
             *
             * After:
             * -  <@> return K ->-
             * -    <0> pushmark s ->2
             * -    <1> ex-rv2sv sK/1 ->-
             * 2      <$> gvsv(*cat) s ->3
             */
            {
                OP *next = o->op_next;
                OP *sibling = OpSIBLING(o);
                if (   OP_TYPE_IS(next, OP_PUSHMARK)
                    && OP_TYPE_IS(sibling, OP_RETURN)
                    && OP_TYPE_IS(sibling->op_next, OP_LINESEQ)
                    && ( OP_TYPE_IS(sibling->op_next->op_next, OP_LEAVESUB)
                       ||OP_TYPE_IS(sibling->op_next->op_next,
                                    OP_LEAVESUBLV))
                    && cUNOPx(sibling)->op_first == next
                    && OpHAS_SIBLING(next) && OpSIBLING(next)->op_next
                    && next->op_next
                ) {
                    /* Look through the PUSHMARK's siblings for one that
                     * points to the RETURN */
                    OP *top = OpSIBLING(next);
                    while (top && top->op_next) {
                        if (top->op_next == sibling) {
                            top->op_next = sibling->op_next;
                            o->op_next = next->op_next;
                            break;
                        }
                        top = OpSIBLING(top);
                    }
                }
            }

            /* Optimise 'my $x; my $y;' into 'my ($x, $y);'
             *
             * This latter form is then suitable for conversion into padrange
             * later on. Convert:
             *
             *   nextstate1 -> padop1 -> nextstate2 -> padop2 -> nextstate3
             *
             * into:
             *
             *   nextstate1 ->     listop     -> nextstate3
             *                 /            \
             *         pushmark -> padop1 -> padop2
             */
            if (o->op_next && (
                    o->op_next->op_type == OP_PADSV
                 || o->op_next->op_type == OP_PADAV
                 || o->op_next->op_type == OP_PADHV
                )
                && !(o->op_next->op_private & ~OPpLVAL_INTRO)
                && o->op_next->op_next && o->op_next->op_next->op_type == OP_NEXTSTATE
                && o->op_next->op_next->op_next && (
                    o->op_next->op_next->op_next->op_type == OP_PADSV
                 || o->op_next->op_next->op_next->op_type == OP_PADAV
                 || o->op_next->op_next->op_next->op_type == OP_PADHV
                )
                && !(o->op_next->op_next->op_next->op_private & ~OPpLVAL_INTRO)
                && o->op_next->op_next->op_next->op_next && o->op_next->op_next->op_next->op_next->op_type == OP_NEXTSTATE
                && (!CopLABEL((COP*)o)) /* Don't mess with labels */
                && (!CopLABEL((COP*)o->op_next->op_next)) /* ... */
            ) {
                OP *pad1, *ns2, *pad2, *ns3, *newop, *newpm;

                pad1 =    o->op_next;
                ns2  = pad1->op_next;
                pad2 =  ns2->op_next;
                ns3  = pad2->op_next;

                /* we assume here that the op_next chain is the same as
                 * the op_sibling chain */
                assert(OpSIBLING(o)    == pad1);
                assert(OpSIBLING(pad1) == ns2);
                assert(OpSIBLING(ns2)  == pad2);
                assert(OpSIBLING(pad2) == ns3);

                /* excise and delete ns2 */
                op_sibling_splice(NULL, pad1, 1, NULL);
                op_free(ns2);

                /* excise pad1 and pad2 */
                op_sibling_splice(NULL, o, 2, NULL);

                /* create new listop, with children consisting of:
                 * a new pushmark, pad1, pad2. */
                newop = newLISTOP(OP_LIST, 0, pad1, pad2);
                newop->op_flags |= OPf_PARENS;
                newop->op_flags = (newop->op_flags & ~OPf_WANT) | OPf_WANT_VOID;

                /* insert newop between o and ns3 */
                op_sibling_splice(NULL, o, 0, newop);

                /*fixup op_next chain */
                newpm = cUNOPx(newop)->op_first; /* pushmark */
                o    ->op_next = newpm;
                newpm->op_next = pad1;
                pad1 ->op_next = pad2;
                pad2 ->op_next = newop; /* listop */
                newop->op_next = ns3;

                /* Ensure pushmark has this flag if padops do */
                if (pad1->op_flags & OPf_MOD && pad2->op_flags & OPf_MOD) {
                    newpm->op_flags |= OPf_MOD;
                }

                break;
            }

            /* Two NEXTSTATEs in a row serve no purpose. Except if they happen
               to carry two labels. For now, take the easier option, and skip
               this optimisation if the first NEXTSTATE has a label.
               Yves asked what about if they have different hints or features?
               Tony thinks that as we remove the first of the pair it should
               be fine.
            */
            if (!CopLABEL((COP*)o) && !PERLDB_NOOPT) {
                OP *nextop = o->op_next;
                while (nextop) {
                    switch (nextop->op_type) {
                        case OP_NULL:
                        case OP_SCALAR:
                        case OP_LINESEQ:
                        case OP_SCOPE:
                            nextop = nextop->op_next;
                            continue;
                    }
                    break;
                }

                if (nextop && (nextop->op_type == OP_NEXTSTATE)) {
                    op_null(o);
                    if (oldop)
                        oldop->op_next = nextop;
                    o = nextop;
                    /* Skip (old)oldop assignment since the current oldop's
                       op_next already points to the next op.  */
                    goto redo;
                }
            }
            break;

        case OP_CONCAT:
            if (o->op_next && o->op_next->op_type == OP_STRINGIFY) {
                if (o->op_next->op_private & OPpTARGET_MY) {
                    if (o->op_flags & OPf_STACKED) /* chained concats */
                        break; /* ignore_optimization */
                    else {
                        /* assert(PL_opargs[o->op_type] & OA_TARGLEX); */
                        o->op_targ = o->op_next->op_targ;
                        o->op_next->op_targ = 0;
                        o->op_private |= OPpTARGET_MY;
                    }
                }
                op_null(o->op_next);
            }
            break;
        case OP_STUB:
            if ((o->op_flags & OPf_WANT) != OPf_WANT_LIST) {
                break; /* Scalar stub must produce undef.  List stub is noop */
            }
            goto nothin;
        case OP_NULL:
            if (o->op_targ == OP_NEXTSTATE
                || o->op_targ == OP_DBSTATE)
            {
                PL_curcop = ((COP*)o);
            }
            /* XXX: We avoid setting op_seq here to prevent later calls
               to rpeep() from mistakenly concluding that optimisation
               has already occurred. This doesn't fix the real problem,
               though (See 20010220.007 (#5874)). AMS 20010719 */
            /* op_seq functionality is now replaced by op_opt */
            o->op_opt = 0;
            /* FALLTHROUGH */
        case OP_SCALAR:
        case OP_LINESEQ:
        case OP_SCOPE:
        nothin:
            if (oldop) {
                oldop->op_next = o->op_next;
                o->op_opt = 0;
                continue;
            }
            break;

        case OP_PUSHMARK:

            /* Given
                 5 repeat/DOLIST
                 3   ex-list
                 1     pushmark
                 2     scalar or const
                 4   const[0]
               convert repeat into a stub with no kids.
             */
            if (o->op_next->op_type == OP_CONST
             || (  o->op_next->op_type == OP_PADSV
                && !(o->op_next->op_private & OPpLVAL_INTRO))
             || (  o->op_next->op_type == OP_GV
                && o->op_next->op_next->op_type == OP_RV2SV
                && !(o->op_next->op_next->op_private
                        & (OPpLVAL_INTRO|OPpOUR_INTRO))))
            {
                const OP *kid = o->op_next->op_next;
                if (o->op_next->op_type == OP_GV)
                   kid = kid->op_next;
                /* kid is now the ex-list.  */
                if (kid->op_type == OP_NULL
                 && (kid = kid->op_next)->op_type == OP_CONST
                    /* kid is now the repeat count.  */
                 && kid->op_next->op_type == OP_REPEAT
                 && kid->op_next->op_private & OPpREPEAT_DOLIST
                 && (kid->op_next->op_flags & OPf_WANT) == OPf_WANT_LIST
                 && SvIOK(kSVOP_sv) && SvIVX(kSVOP_sv) == 0
                 && oldop)
                {
                    o = kid->op_next; /* repeat */
                    oldop->op_next = o;
                    op_free(cBINOPo->op_first);
                    op_free(cBINOPo->op_last );
                    o->op_flags &=~ OPf_KIDS;
                    /* stub is a baseop; repeat is a binop */
                    STATIC_ASSERT_STMT(sizeof(OP) <= sizeof(BINOP));
                    OpTYPE_set(o, OP_STUB);
                    o->op_private = 0;
                    break;
                }
            }

            /* If the pushmark is associated with an empty anonhash
             * or anonlist, null out the pushmark and swap in a
             * specialised op for the parent.
             *     4        <@> anonhash sK* ->5
             *     3           <0> pushmark s ->4
             * becomes:
             *     3        <@> emptyavhv sK* ->4
             *     -           <0> pushmark s ->3
             */
            if (!OpHAS_SIBLING(o) && (o->op_next == o->op_sibparent) && (
                (o->op_next->op_type == OP_ANONHASH) ||
                (o->op_next->op_type == OP_ANONLIST) ) &&
                (o->op_next->op_flags & OPf_SPECIAL) ) {

                OP* anon = o->op_next;
                /* These next two are _potentially_ a padsv and an sassign */
                OP* padsv = anon->op_next;
                OP* sassign = (padsv) ? padsv->op_next: NULL;

                anon->op_private = (anon->op_type == OP_ANONLIST) ?
                                                0 : OPpEMPTYAVHV_IS_HV;
                OpTYPE_set(anon, OP_EMPTYAVHV);
                op_null(o);
                o = anon;
                if (oldop) /* A previous optimization may have NULLED it */
                    oldop->op_next = anon;

                /* Further optimise scalar assignment of an empty anonhash
                 * or anonlist by subsuming the padsv & sassign OPs. */
                if ((padsv->op_type == OP_PADSV) &&
                    !(padsv->op_private & OPpDEREF) &&
                    sassign && (sassign->op_type == OP_SASSIGN) ){

                    /* Take some public flags from the sassign */
                    anon->op_flags = OPf_KIDS | OPf_SPECIAL |
                        (anon->op_flags & OPf_PARENS) |
                        (sassign->op_flags & (OPf_WANT|OPf_PARENS));

                    /* Take some private flags from the padsv */
                    anon->op_private |= OPpTARGET_MY |
                        (padsv->op_private & (OPpLVAL_INTRO|OPpPAD_STATE));

                    /* Take the targ slot from the padsv*/
                    anon->op_targ = padsv->op_targ;
                    padsv->op_targ = 0;

                    /* Clean up */
                    anon->op_next = sassign->op_next;
                    op_null(padsv);
                    op_null(sassign);
                }
                break;

            }


            /* Convert a series of PAD ops for my vars plus support into a
             * single padrange op. Basically
             *
             *    pushmark -> pad[ahs]v -> pad[ahs]?v -> ... -> (list) -> rest
             *
             * becomes, depending on circumstances, one of
             *
             *    padrange  ----------------------------------> (list) -> rest
             *    padrange  --------------------------------------------> rest
             *
             * where all the pad indexes are sequential and of the same type
             * (INTRO or not).
             * We convert the pushmark into a padrange op, then skip
             * any other pad ops, and possibly some trailing ops.
             * Note that we don't null() the skipped ops, to make it
             * easier for Deparse to undo this optimisation (and none of
             * the skipped ops are holding any resources). It also makes
             * it easier for find_uninit_var(), as it can just ignore
             * padrange, and examine the original pad ops.
             */
        {
            OP *p;
            OP *followop = NULL; /* the op that will follow the padrange op */
            U8 count = 0;
            U8 intro = 0;
            PADOFFSET base = 0; /* init only to stop compiler whining */
            bool gvoid = 0;     /* init only to stop compiler whining */
            bool defav = 0;  /* seen (...) = @_ */
            bool reuse = 0;  /* reuse an existing padrange op */

            /* look for a pushmark -> gv[_] -> rv2av */

            {
                OP *rv2av, *q;
                p = o->op_next;
                if (   p->op_type == OP_GV
                    && cGVOPx_gv(p) == PL_defgv
                    && (rv2av = p->op_next)
                    && rv2av->op_type == OP_RV2AV
                    && !(rv2av->op_flags & OPf_REF)
                    && !(rv2av->op_private & (OPpLVAL_INTRO|OPpMAYBE_LVSUB))
                    && ((rv2av->op_flags & OPf_WANT) == OPf_WANT_LIST)
                ) {
                    q = rv2av->op_next;
                    if (q->op_type == OP_NULL)
                        q = q->op_next;
                    if (q->op_type == OP_PUSHMARK) {
                        defav = 1;
                        p = q;
                    }
                }
            }
            if (!defav) {
                p = o;
            }

            /* scan for PAD ops */

            for (p = p->op_next; p; p = p->op_next) {
                if (p->op_type == OP_NULL)
                    continue;

                if ((     p->op_type != OP_PADSV
                       && p->op_type != OP_PADAV
                       && p->op_type != OP_PADHV
                    )
                      /* any private flag other than INTRO? e.g. STATE */
                   || (p->op_private & ~OPpLVAL_INTRO)
                )
                    break;

                /* let $a[N] potentially be optimised into AELEMFAST_LEX
                 * instead */
                if (   p->op_type == OP_PADAV
                    && p->op_next
                    && p->op_next->op_type == OP_CONST
                    && p->op_next->op_next
                    && p->op_next->op_next->op_type == OP_AELEM
                )
                    break;

                /* for 1st padop, note what type it is and the range
                 * start; for the others, check that it's the same type
                 * and that the targs are contiguous */
                if (count == 0) {
                    intro = (p->op_private & OPpLVAL_INTRO);
                    base = p->op_targ;
                    gvoid = OP_GIMME(p,0) == G_VOID;
                }
                else {
                    if ((p->op_private & OPpLVAL_INTRO) != intro)
                        break;
                    /* Note that you'd normally  expect targs to be
                     * contiguous in my($a,$b,$c), but that's not the case
                     * when external modules start doing things, e.g.
                     * Function::Parameters */
                    if (p->op_targ != base + count)
                        break;
                    assert(p->op_targ == base + count);
                    /* Either all the padops or none of the padops should
                       be in void context.  Since we only do the optimisa-
                       tion for av/hv when the aggregate itself is pushed
                       on to the stack (one item), there is no need to dis-
                       tinguish list from scalar context.  */
                    if (gvoid != (OP_GIMME(p,0) == G_VOID))
                        break;
                }

                /* for AV, HV, only when we're not flattening */
                if (   p->op_type != OP_PADSV
                    && !gvoid
                    && !(p->op_flags & OPf_REF)
                )
                    break;

                if (count >= OPpPADRANGE_COUNTMASK)
                    break;

                /* there's a biggest base we can fit into a
                 * SAVEt_CLEARPADRANGE in pp_padrange.
                 * (The sizeof() stuff will be constant-folded, and is
                 * intended to avoid getting "comparison is always false"
                 * compiler warnings. See the comments above
                 * MEM_WRAP_CHECK for more explanation on why we do this
                 * in a weird way to avoid compiler warnings.)
                 */
                if (   intro
                    && (8*sizeof(base) >
                        8*sizeof(UV)-OPpPADRANGE_COUNTSHIFT-SAVE_TIGHT_SHIFT
                        ? (Size_t)base
                        : (UV_MAX >> (OPpPADRANGE_COUNTSHIFT+SAVE_TIGHT_SHIFT))
                        ) >
                        (UV_MAX >> (OPpPADRANGE_COUNTSHIFT+SAVE_TIGHT_SHIFT))
                )
                    break;

                /* Success! We've got another valid pad op to optimise away */
                count++;
                followop = p->op_next;
            }

            if (count < 1 || (count == 1 && !defav))
                break;

            /* pp_padrange in specifically compile-time void context
             * skips pushing a mark and lexicals; in all other contexts
             * (including unknown till runtime) it pushes a mark and the
             * lexicals. We must be very careful then, that the ops we
             * optimise away would have exactly the same effect as the
             * padrange.
             * In particular in void context, we can only optimise to
             * a padrange if we see the complete sequence
             *     pushmark, pad*v, ...., list
             * which has the net effect of leaving the markstack as it
             * was.  Not pushing onto the stack (whereas padsv does touch
             * the stack) makes no difference in void context.
             */
            assert(followop);
            if (gvoid) {
                if (followop->op_type == OP_LIST
                        && OP_GIMME(followop,0) == G_VOID
                   )
                {
                    followop = followop->op_next; /* skip OP_LIST */

                    /* consolidate two successive my(...);'s */

                    if (   oldoldop
                        && oldoldop->op_type == OP_PADRANGE
                        && (oldoldop->op_flags & OPf_WANT) == OPf_WANT_VOID
                        && (oldoldop->op_private & OPpLVAL_INTRO) == intro
                        && !(oldoldop->op_flags & OPf_SPECIAL)
                    ) {
                        U8 old_count;
                        assert(oldoldop->op_next == oldop);
                        assert(   oldop->op_type == OP_NEXTSTATE
                               || oldop->op_type == OP_DBSTATE);
                        assert(oldop->op_next == o);

                        old_count
                            = (oldoldop->op_private & OPpPADRANGE_COUNTMASK);

                       /* Do not assume pad offsets for $c and $d are con-
                          tiguous in
                            my ($a,$b,$c);
                            my ($d,$e,$f);
                        */
                        if (  oldoldop->op_targ + old_count == base
                           && old_count < OPpPADRANGE_COUNTMASK - count) {
                            base = oldoldop->op_targ;
                            count += old_count;
                            reuse = 1;
                        }
                    }

                    /* if there's any immediately following singleton
                     * my var's; then swallow them and the associated
                     * nextstates; i.e.
                     *    my ($a,$b); my $c; my $d;
                     * is treated as
                     *    my ($a,$b,$c,$d);
                     */

                    while (    ((p = followop->op_next))
                            && (  p->op_type == OP_PADSV
                               || p->op_type == OP_PADAV
                               || p->op_type == OP_PADHV)
                            && (p->op_flags & OPf_WANT) == OPf_WANT_VOID
                            && (p->op_private & OPpLVAL_INTRO) == intro
                            && !(p->op_private & ~OPpLVAL_INTRO)
                            && p->op_next
                            && (   p->op_next->op_type == OP_NEXTSTATE
                                || p->op_next->op_type == OP_DBSTATE)
                            && count < OPpPADRANGE_COUNTMASK
                            && base + count == p->op_targ
                    ) {
                        count++;
                        followop = p->op_next;
                    }
                }
                else
                    break;
            }

            if (reuse) {
                assert(oldoldop->op_type == OP_PADRANGE);
                oldoldop->op_next = followop;
                oldoldop->op_private = (intro | count);
                o = oldoldop;
                oldop = NULL;
                oldoldop = NULL;
            }
            else {
                /* Convert the pushmark into a padrange.
                 * To make Deparse easier, we guarantee that a padrange was
                 * *always* formerly a pushmark */
                assert(o->op_type == OP_PUSHMARK);
                o->op_next = followop;
                OpTYPE_set(o, OP_PADRANGE);
                o->op_targ = base;
                /* bit 7: INTRO; bit 6..0: count */
                o->op_private = (intro | count);
                o->op_flags = ((o->op_flags & ~(OPf_WANT|OPf_SPECIAL))
                              | gvoid * OPf_WANT_VOID
                              | (defav ? OPf_SPECIAL : 0));
            }
            break;
        }

        case OP_RV2AV:
            if ((o->op_flags & OPf_WANT) == OPf_WANT_SCALAR)
                S_check_for_bool_cxt(o, 1, OPpTRUEBOOL, 0);
            break;

        case OP_RV2HV:
        case OP_PADHV:
            /*'keys %h' in void or scalar context: skip the OP_KEYS
             * and perform the functionality directly in the RV2HV/PADHV
             * op
             */
            if (o->op_flags & OPf_REF) {
                OP *k = o->op_next;
                U8 want = (k->op_flags & OPf_WANT);
                if (   k
                    && k->op_type == OP_KEYS
                    && (   want == OPf_WANT_VOID
                        || want == OPf_WANT_SCALAR)
                    && !(k->op_private & OPpMAYBE_LVSUB)
                    && !(k->op_flags & OPf_MOD)
                ) {
                    o->op_next     = k->op_next;
                    o->op_flags   &= ~(OPf_REF|OPf_WANT);
                    o->op_flags   |= want;
                    o->op_private |= (o->op_type == OP_PADHV ?
                                      OPpPADHV_ISKEYS : OPpRV2HV_ISKEYS);
                    /* for keys(%lex), hold onto the OP_KEYS's targ
                     * since padhv doesn't have its own targ to return
                     * an int with */
                    if (!(o->op_type ==OP_PADHV && want == OPf_WANT_SCALAR))
                        op_null(k);
                }
            }

            /* see if %h is used in boolean context */
            if ((o->op_flags & OPf_WANT) == OPf_WANT_SCALAR)
                S_check_for_bool_cxt(o, 1, OPpTRUEBOOL, OPpMAYBE_TRUEBOOL);


            if (o->op_type != OP_PADHV)
                break;
            /* FALLTHROUGH */
        case OP_PADAV:
            if (   o->op_type == OP_PADAV
                && (o->op_flags & OPf_WANT) == OPf_WANT_SCALAR
            )
                S_check_for_bool_cxt(o, 1, OPpTRUEBOOL, 0);
            /* FALLTHROUGH */
        case OP_PADSV:
            /* Skip over state($x) in void context.  */
            if (oldop && o->op_private == (OPpPAD_STATE|OPpLVAL_INTRO)
             && (o->op_flags & OPf_WANT) == OPf_WANT_VOID)
            {
                oldop->op_next = o->op_next;
                goto redo_nextstate;
            }
            if (o->op_type != OP_PADAV)
                break;
            /* FALLTHROUGH */
        case OP_GV:
            if (o->op_type == OP_PADAV || o->op_next->op_type == OP_RV2AV) {
                OP* const pop = (o->op_type == OP_PADAV) ?
                            o->op_next : o->op_next->op_next;
                IV i;
                if (pop && pop->op_type == OP_CONST &&
                    ((PL_op = pop->op_next)) &&
                    pop->op_next->op_type == OP_AELEM &&
                    !(pop->op_next->op_private &
                      (OPpLVAL_INTRO|OPpLVAL_DEFER|OPpDEREF|OPpMAYBE_LVSUB)) &&
                    (i = SvIV(cSVOPx(pop)->op_sv)) >= -128 && i <= 127)
                {
                    GV *gv;
                    if (cSVOPx(pop)->op_private & OPpCONST_STRICT)
                        no_bareword_allowed(pop);
                    if (o->op_type == OP_GV)
                        op_null(o->op_next);
                    op_null(pop->op_next);
                    op_null(pop);
                    o->op_flags |= pop->op_next->op_flags & OPf_MOD;
                    o->op_next = pop->op_next->op_next;
                    o->op_ppaddr = PL_ppaddr[OP_AELEMFAST];
                    o->op_private = (U8)i;
                    if (o->op_type == OP_GV) {
                        gv = cGVOPo_gv;
                        GvAVn(gv);
                        o->op_type = OP_AELEMFAST;
                    }
                    else
                        o->op_type = OP_AELEMFAST_LEX;
                }
                if (o->op_type != OP_GV)
                    break;
            }

            /* Remove $foo from the op_next chain in void context.  */
            if (oldop
             && (  o->op_next->op_type == OP_RV2SV
                || o->op_next->op_type == OP_RV2AV
                || o->op_next->op_type == OP_RV2HV  )
             && (o->op_next->op_flags & OPf_WANT) == OPf_WANT_VOID
             && !(o->op_next->op_private & OPpLVAL_INTRO))
            {
                oldop->op_next = o->op_next->op_next;
                /* Reprocess the previous op if it is a nextstate, to
                   allow double-nextstate optimisation.  */
              redo_nextstate:
                if (oldop->op_type == OP_NEXTSTATE) {
                    oldop->op_opt = 0;
                    o = oldop;
                    oldop = oldoldop;
                    oldoldop = NULL;
                    goto redo;
                }
                o = oldop->op_next;
                goto redo;
            }
            else if (o->op_next->op_type == OP_RV2SV) {
                if (!(o->op_next->op_private & OPpDEREF)) {
                    op_null(o->op_next);
                    o->op_private |= o->op_next->op_private & (OPpLVAL_INTRO
                                                               | OPpOUR_INTRO);
                    o->op_next = o->op_next->op_next;
                    OpTYPE_set(o, OP_GVSV);
                }
            }
            else if (o->op_next->op_type == OP_READLINE
                    && o->op_next->op_next->op_type == OP_CONCAT
                    && (o->op_next->op_next->op_flags & OPf_STACKED))
            {
                /* Turn "$a .= <FH>" into an OP_RCATLINE. AMS 20010917 */
                OpTYPE_set(o, OP_RCATLINE);
                o->op_flags |= OPf_STACKED;
                op_null(o->op_next->op_next);
                op_null(o->op_next);
            }

            break;

        case OP_NOT:
            break;

        case OP_AND:
        case OP_OR:
        case OP_DOR:
        case OP_CMPCHAIN_AND:
        case OP_PUSHDEFER:
            while (cLOGOP->op_other->op_type == OP_NULL)
                cLOGOP->op_other = cLOGOP->op_other->op_next;
            while (o->op_next && (   o->op_type == o->op_next->op_type
                                  || o->op_next->op_type == OP_NULL))
                o->op_next = o->op_next->op_next;

            /* If we're an OR and our next is an AND in void context, we'll
               follow its op_other on short circuit, same for reverse.
               We can't do this with OP_DOR since if it's true, its return
               value is the underlying value which must be evaluated
               by the next op. */
            if (o->op_next &&
                (
                    (IS_AND_OP(o) && IS_OR_OP(o->op_next))
                 || (IS_OR_OP(o) && IS_AND_OP(o->op_next))
                )
                && (o->op_next->op_flags & OPf_WANT) == OPf_WANT_VOID
            ) {
                o->op_next = cLOGOPx(o->op_next)->op_other;
            }
            DEFER(cLOGOP->op_other);
            o->op_opt = 1;
            break;

        case OP_GREPWHILE:
            if ((o->op_flags & OPf_WANT) == OPf_WANT_SCALAR)
                S_check_for_bool_cxt(o, 1, OPpTRUEBOOL, 0);
            /* FALLTHROUGH */
        case OP_COND_EXPR:
        case OP_MAPWHILE:
        case OP_ANDASSIGN:
        case OP_ORASSIGN:
        case OP_DORASSIGN:
        case OP_RANGE:
        case OP_ONCE:
        case OP_ARGDEFELEM:
            while (cLOGOP->op_other->op_type == OP_NULL)
                cLOGOP->op_other = cLOGOP->op_other->op_next;
            DEFER(cLOGOP->op_other);
            break;

        case OP_ENTERLOOP:
        case OP_ENTERITER:
            while (cLOOP->op_redoop->op_type == OP_NULL)
                cLOOP->op_redoop = cLOOP->op_redoop->op_next;
            while (cLOOP->op_nextop->op_type == OP_NULL)
                cLOOP->op_nextop = cLOOP->op_nextop->op_next;
            while (cLOOP->op_lastop->op_type == OP_NULL)
                cLOOP->op_lastop = cLOOP->op_lastop->op_next;
            /* a while(1) loop doesn't have an op_next that escapes the
             * loop, so we have to explicitly follow the op_lastop to
             * process the rest of the code */
            DEFER(cLOOP->op_lastop);
            break;

        case OP_ENTERTRY:
            assert(cLOGOPo->op_other->op_type == OP_LEAVETRY);
            DEFER(cLOGOPo->op_other);
            break;

        case OP_ENTERTRYCATCH:
            assert(cLOGOPo->op_other->op_type == OP_CATCH);
            /* catch body is the ->op_other of the OP_CATCH */
            DEFER(cLOGOPx(cLOGOPo->op_other)->op_other);
            break;

        case OP_SUBST:
            if ((o->op_flags & OPf_WANT) == OPf_WANT_SCALAR)
                S_check_for_bool_cxt(o, 1, OPpTRUEBOOL, 0);
            assert(!(cPMOP->op_pmflags & PMf_ONCE));
            while (cPMOP->op_pmstashstartu.op_pmreplstart &&
                   cPMOP->op_pmstashstartu.op_pmreplstart->op_type == OP_NULL)
                cPMOP->op_pmstashstartu.op_pmreplstart
                    = cPMOP->op_pmstashstartu.op_pmreplstart->op_next;
            DEFER(cPMOP->op_pmstashstartu.op_pmreplstart);
            break;

        case OP_SORT: {
            OP *oright;

            if (o->op_flags & OPf_SPECIAL) {
                /* first arg is a code block */
                OP * const nullop = OpSIBLING(cLISTOP->op_first);
                OP * kid          = cUNOPx(nullop)->op_first;

                assert(nullop->op_type == OP_NULL);
                assert(kid->op_type == OP_SCOPE
                 || (kid->op_type == OP_NULL && kid->op_targ == OP_LEAVE));
                /* since OP_SORT doesn't have a handy op_other-style
                 * field that can point directly to the start of the code
                 * block, store it in the otherwise-unused op_next field
                 * of the top-level OP_NULL. This will be quicker at
                 * run-time, and it will also allow us to remove leading
                 * OP_NULLs by just messing with op_nexts without
                 * altering the basic op_first/op_sibling layout. */
                kid = kLISTOP->op_first;
                assert(
                      (kid->op_type == OP_NULL
                      && (  kid->op_targ == OP_NEXTSTATE
                         || kid->op_targ == OP_DBSTATE  ))
                    || kid->op_type == OP_STUB
                    || kid->op_type == OP_ENTER
                    || (PL_parser && PL_parser->error_count));
                nullop->op_next = kid->op_next;
                DEFER(nullop->op_next);
            }

            /* check that RHS of sort is a single plain array */
            oright = cUNOPo->op_first;
            if (!oright || oright->op_type != OP_PUSHMARK)
                break;

            if (o->op_private & OPpSORT_INPLACE)
                break;

            /* reverse sort ... can be optimised.  */
            if (!OpHAS_SIBLING(cUNOPo)) {
                /* Nothing follows us on the list. */
                OP * const reverse = o->op_next;

                if (reverse->op_type == OP_REVERSE &&
                    (reverse->op_flags & OPf_WANT) == OPf_WANT_LIST) {
                    OP * const pushmark = cUNOPx(reverse)->op_first;
                    if (pushmark && (pushmark->op_type == OP_PUSHMARK)
                        && (OpSIBLING(cUNOPx(pushmark)) == o)) {
                        /* reverse -> pushmark -> sort */
                        o->op_private |= OPpSORT_REVERSE;
                        op_null(reverse);
                        pushmark->op_next = oright->op_next;
                        op_null(oright);
                    }
                }
            }

            break;
        }

        case OP_REVERSE: {
            OP *ourmark, *theirmark, *ourlast, *iter, *expushmark, *rv2av;
            OP *gvop = NULL;
            LISTOP *enter, *exlist;

            if (o->op_private & OPpSORT_INPLACE)
                break;

            enter = cLISTOPx(o->op_next);
            if (!enter)
                break;
            if (enter->op_type == OP_NULL) {
                enter = cLISTOPx(enter->op_next);
                if (!enter)
                    break;
            }
            /* for $a (...) will have OP_GV then OP_RV2GV here.
               for (...) just has an OP_GV.  */
            if (enter->op_type == OP_GV) {
                gvop = (OP *) enter;
                enter = cLISTOPx(enter->op_next);
                if (!enter)
                    break;
                if (enter->op_type == OP_RV2GV) {
                  enter = cLISTOPx(enter->op_next);
                  if (!enter)
                    break;
                }
            }

            if (enter->op_type != OP_ENTERITER)
                break;

            iter = enter->op_next;
            if (!iter || iter->op_type != OP_ITER)
                break;

            expushmark = enter->op_first;
            if (!expushmark || expushmark->op_type != OP_NULL
                || expushmark->op_targ != OP_PUSHMARK)
                break;

            exlist = cLISTOPx(OpSIBLING(expushmark));
            if (!exlist || exlist->op_type != OP_NULL
                || exlist->op_targ != OP_LIST)
                break;

            if (exlist->op_last != o) {
                /* Mmm. Was expecting to point back to this op.  */
                break;
            }
            theirmark = exlist->op_first;
            if (!theirmark || theirmark->op_type != OP_PUSHMARK)
                break;

            if (OpSIBLING(theirmark) != o) {
                /* There's something between the mark and the reverse, eg
                   for (1, reverse (...))
                   so no go.  */
                break;
            }

            ourmark = cLISTOPo->op_first;
            if (!ourmark || ourmark->op_type != OP_PUSHMARK)
                break;

            ourlast = cLISTOPo->op_last;
            if (!ourlast || ourlast->op_next != o)
                break;

            rv2av = OpSIBLING(ourmark);
            if (rv2av && rv2av->op_type == OP_RV2AV && !OpHAS_SIBLING(rv2av)
                && rv2av->op_flags == (OPf_WANT_LIST | OPf_KIDS)) {
                /* We're just reversing a single array.  */
                rv2av->op_flags = OPf_WANT_SCALAR | OPf_KIDS | OPf_REF;
                enter->op_flags |= OPf_STACKED;
            }

            /* We don't have control over who points to theirmark, so sacrifice
               ours.  */
            theirmark->op_next = ourmark->op_next;
            theirmark->op_flags = ourmark->op_flags;
            ourlast->op_next = gvop ? gvop : (OP *) enter;
            op_null(ourmark);
            op_null(o);
            enter->op_private |= OPpITER_REVERSED;
            iter->op_private |= OPpITER_REVERSED;

            oldoldop = NULL;
            oldop    = ourlast;
            o        = oldop->op_next;
            goto redo;
            NOT_REACHED; /* NOTREACHED */
            break;
        }

        case OP_UNDEF:
            if ((o->op_flags & OPf_KIDS) &&
                (cUNOPx(o)->op_first->op_type == OP_PADSV)) {

                /* Convert:
                 *     undef
                 *       padsv[$x]
                 * to:
                 *     undef[$x]
                 */

                OP * padsv = cUNOPx(o)->op_first;
                o->op_private = OPpTARGET_MY |
                        (padsv->op_private & (OPpLVAL_INTRO|OPpPAD_STATE));
                o->op_targ = padsv->op_targ; padsv->op_targ = 0;
                op_null(padsv);
                /* Optimizer does NOT seem to fix up the padsv op_next ptr */
                if (oldoldop)
                    oldoldop->op_next = o;
                oldop = oldoldop;
                oldoldop = NULL;

            } else if (o->op_next->op_type == OP_PADSV) {
                OP * padsv = o->op_next;
                OP * sassign = (padsv->op_next &&
                        padsv->op_next->op_type == OP_SASSIGN) ?
                        padsv->op_next : NULL;
                if (sassign && cBINOPx(sassign)->op_first == o) {
                    /* Convert:
                     *     sassign
                     *       undef
                     *       padsv[$x]
                     * to:
                     *     undef[$x]
                     * NOTE: undef does not have the "T" flag set in
                     *       regen/opcodes, as this would cause
                     *       S_maybe_targlex to do the optimization.
                     *       Seems easier to keep it all here, rather
                     *       than have an undef-specific branch in
                     *       S_maybe_targlex just to add the
                     *       OPpUNDEF_KEEP_PV flag.
                     */
                     o->op_private = OPpTARGET_MY | OPpUNDEF_KEEP_PV |
                         (padsv->op_private & (OPpLVAL_INTRO|OPpPAD_STATE));
                     o->op_targ = padsv->op_targ; padsv->op_targ = 0;
                     op_null(padsv);
                     op_null(sassign);
                     /* Optimizer DOES seems to fix up the op_next ptrs */
                }
            }
            break;

        case OP_QR:
        case OP_MATCH:
            if (!(cPMOP->op_pmflags & PMf_ONCE)) {
                assert (!cPMOP->op_pmstashstartu.op_pmreplstart);
            }
            break;

        case OP_RUNCV:
            if (!(o->op_private & OPpOFFBYONE) && !CvCLONE(PL_compcv)
             && (!CvANON(PL_compcv) || (!PL_cv_has_eval && !PL_perldb)))
            {
                SV *sv;
                if (CvEVAL(PL_compcv)) sv = &PL_sv_undef;
                else {
                    sv = newRV((SV *)PL_compcv);
                    sv_rvweaken(sv);
                    SvREADONLY_on(sv);
                }
                OpTYPE_set(o, OP_CONST);
                o->op_flags |= OPf_SPECIAL;
                cSVOPo->op_sv = sv;
            }
            break;

        case OP_SASSIGN: {
            if (OP_GIMME(o,0) == G_VOID
             || (  o->op_next->op_type == OP_LINESEQ
                && (  o->op_next->op_next->op_type == OP_LEAVESUB
                   || (  o->op_next->op_next->op_type == OP_RETURN
                      && !CvLVALUE(PL_compcv)))))
            {
                OP *right = cBINOP->op_first;
                if (right) {
                    /*   sassign
                    *      RIGHT
                    *      substr
                    *         pushmark
                    *         arg1
                    *         arg2
                    *         ...
                    * becomes
                    *
                    *  ex-sassign
                    *     substr
                    *        pushmark
                    *        RIGHT
                    *        arg1
                    *        arg2
                    *        ...
                    */
                    OP *left = OpSIBLING(right);
                    if (left->op_type == OP_SUBSTR
                         && (left->op_private & 7) < 4) {
                        op_null(o);
                        /* cut out right */
                        op_sibling_splice(o, NULL, 1, NULL);
                        /* and insert it as second child of OP_SUBSTR */
                        op_sibling_splice(left, cBINOPx(left)->op_first, 0,
                                    right);
                        left->op_private |= OPpSUBSTR_REPL_FIRST;
                        left->op_flags =
                            (o->op_flags & ~OPf_WANT) | OPf_WANT_VOID;
                    }
                }
            }
            OP* rhs = cBINOPx(o)->op_first;
            OP* lval = cBINOPx(o)->op_last;

            /* Combine a simple SASSIGN OP with a PADSV lvalue child OP
             * into a single OP. */

            /* This optimization covers arbitrarily complicated RHS OP
             * trees. Separate optimizations may exist for specific,
             * single RHS OPs, such as:
             * "my $foo = undef;" or "my $bar = $other_padsv;" */

            if (!(o->op_private & (OPpASSIGN_BACKWARDS|OPpASSIGN_CV_TO_GV))
                 && lval && (lval->op_type == OP_PADSV) &&
                !(lval->op_private & OPpDEREF)
                 /* skip if padrange has already gazumped the padsv */
                 && (lval == oldop)
                 /* Memoize::Once produces a non-standard SASSIGN that
                  * doesn't actually point to pp_sassign, has only one
                  * child (PADSV), and gets to it via op_other rather
                  * than op_next. Don't try to optimize this. */
                 && (lval != rhs)
               ) {
                /* SASSIGN's bitfield flags, such as op_moresib and
                 * op_slabbed, will be carried over unchanged. */
                OpTYPE_set(o, OP_PADSV_STORE);

                /* Explicitly craft the new OP's op_flags, carrying
                 * some bits over from the SASSIGN */
                o->op_flags = (
                    OPf_KIDS | OPf_STACKED |
                    (o->op_flags & (OPf_WANT|OPf_PARENS))
                );

                /* Reset op_private flags, taking relevant private flags
                 * from the PADSV */
                o->op_private = (lval->op_private &
                                (OPpLVAL_INTRO|OPpPAD_STATE|OPpDEREF));

                /* Steal the targ from the PADSV */
                o->op_targ = lval->op_targ; lval->op_targ = 0;

                /* Fixup op_next ptrs */
                assert(oldop->op_type == OP_PADSV);
                /* oldoldop can be arbitrarily deep in the RHS OP tree */
                oldoldop->op_next = o;

                /* Even when (rhs != oldoldop), rhs might still have a
                 * relevant op_next ptr to lval. This is definitely true
                 * when rhs is OP_NULL with a LOGOP kid (e.g. orassign).
                 * There may be other cases. */
                if (rhs->op_next == lval)
                    rhs->op_next = o;

                /* Now null-out the PADSV */
                op_null(lval);

                /* NULL the previous op ptrs, so rpeep can continue */
                oldoldop = NULL; oldop = NULL;
            }

            /* Combine a simple SASSIGN OP with an AELEMFAST_LEX lvalue
             * into a single OP. This optimization covers arbitrarily
             * complicated RHS OP trees. */

            if (!(o->op_private & (OPpASSIGN_BACKWARDS|OPpASSIGN_CV_TO_GV))
                && (lval->op_type == OP_NULL) && (lval->op_private == 2) &&
                (cBINOPx(lval)->op_first->op_type == OP_AELEMFAST_LEX)
            ) {
                OP * lex = cBINOPx(lval)->op_first;
                /* SASSIGN's bitfield flags, such as op_moresib and
                 * op_slabbed, will be carried over unchanged. */
                OpTYPE_set(o, OP_AELEMFASTLEX_STORE);

                /* Explicitly craft the new OP's op_flags, carrying
                 * some bits over from the SASSIGN */
                o->op_flags = (
                    OPf_KIDS | OPf_STACKED |
                    (o->op_flags & (OPf_WANT|OPf_PARENS))
                );

                /* Copy the AELEMFAST_LEX op->private, which contains
                 * the key index. */
                o->op_private = lex->op_private;

                /* Take the targ from the AELEMFAST_LEX */
                o->op_targ = lex->op_targ; lex->op_targ = 0;

                assert(oldop->op_type == OP_AELEMFAST_LEX);
                /* oldoldop can be arbitrarily deep in the RHS OP tree */
                oldoldop->op_next = o;

                /* Even when (rhs != oldoldop), rhs might still have a
                 * relevant op_next ptr to lex. (Updating it here can
                 * also cause other ops in the RHS to get the desired
                 * op_next pointer, presumably thanks to the finalizer.)
                 * This is definitely truewhen rhs is OP_NULL with a
                 * LOGOP kid (e.g. orassign). There may be other cases. */
                if (rhs->op_next == lex)
                    rhs->op_next = o;

                /* Now null-out the AELEMFAST_LEX */
                op_null(lex);

                /* NULL the previous op ptrs, so rpeep can continue */
                oldop = oldoldop; oldoldop = NULL;
            }

            break;
        }

        case OP_AASSIGN: {
            int l, r, lr, lscalars, rscalars;

            /* handle common vars detection, e.g. ($a,$b) = ($b,$a).
               Note that we do this now rather than in newASSIGNOP(),
               since only by now are aliased lexicals flagged as such

               See the essay "Common vars in list assignment" above for
               the full details of the rationale behind all the conditions
               below.

               PL_generation sorcery:
               To detect whether there are common vars, the global var
               PL_generation is incremented for each assign op we scan.
               Then we run through all the lexical variables on the LHS,
               of the assignment, setting a spare slot in each of them to
               PL_generation.  Then we scan the RHS, and if any lexicals
               already have that value, we know we've got commonality.
               Also, if the generation number is already set to
               PERL_INT_MAX, then the variable is involved in aliasing, so
               we also have potential commonality in that case.
             */

            PL_generation++;
            /* scan LHS */
            lscalars = 0;
            l = S_aassign_scan(aTHX_ cLISTOPo->op_last,  FALSE, &lscalars);
            /* scan RHS */
            rscalars = 0;
            r = S_aassign_scan(aTHX_ cLISTOPo->op_first, TRUE, &rscalars);
            lr = (l|r);


            /* After looking for things which are *always* safe, this main
             * if/else chain selects primarily based on the type of the
             * LHS, gradually working its way down from the more dangerous
             * to the more restrictive and thus safer cases */

            if (   !l                      /* () = ....; */
                || !r                      /* .... = (); */
                || !(l & ~AAS_SAFE_SCALAR) /* (undef, pos()) = ...; */
                || !(r & ~AAS_SAFE_SCALAR) /* ... = (1,2,length,undef); */
                || (lscalars < 2)          /* (undef, $x) = ... */
            ) {
                NOOP; /* always safe */
            }
            else if (l & AAS_DANGEROUS) {
                /* always dangerous */
                o->op_private |= OPpASSIGN_COMMON_SCALAR;
                o->op_private |= OPpASSIGN_COMMON_AGG;
            }
            else if (l & (AAS_PKG_SCALAR|AAS_PKG_AGG)) {
                /* package vars are always dangerous - too many
                 * aliasing possibilities */
                if (l & AAS_PKG_SCALAR)
                    o->op_private |= OPpASSIGN_COMMON_SCALAR;
                if (l & AAS_PKG_AGG)
                    o->op_private |= OPpASSIGN_COMMON_AGG;
            }
            else if (l & ( AAS_MY_SCALAR|AAS_MY_AGG
                          |AAS_LEX_SCALAR|AAS_LEX_AGG))
            {
                /* LHS contains only lexicals and safe ops */

                if (l & (AAS_MY_AGG|AAS_LEX_AGG))
                    o->op_private |= OPpASSIGN_COMMON_AGG;

                if (l & (AAS_MY_SCALAR|AAS_LEX_SCALAR)) {
                    if (lr & AAS_LEX_SCALAR_COMM)
                        o->op_private |= OPpASSIGN_COMMON_SCALAR;
                    else if (   !(l & AAS_LEX_SCALAR)
                             && (r & AAS_DEFAV))
                    {
                        /* falsely mark
                         *    my (...) = @_
                         * as scalar-safe for performance reasons.
                         * (it will still have been marked _AGG if necessary */
                        NOOP;
                    }
                    else if (r  & (AAS_PKG_SCALAR|AAS_PKG_AGG|AAS_DANGEROUS))
                        /* if there are only lexicals on the LHS and no
                         * common ones on the RHS, then we assume that the
                         * only way those lexicals could also get
                         * on the RHS is via some sort of dereffing or
                         * closure, e.g.
                         *    $r = \$lex;
                         *    ($lex, $x) = (1, $$r)
                         * and in this case we assume the var must have
                         *  a bumped ref count. So if its ref count is 1,
                         *  it must only be on the LHS.
                         */
                        o->op_private |= OPpASSIGN_COMMON_RC1;
                }
            }

            /* ... = ($x)
             * may have to handle aggregate on LHS, but we can't
             * have common scalars. */
            if (rscalars < 2)
                o->op_private &=
                        ~(OPpASSIGN_COMMON_SCALAR|OPpASSIGN_COMMON_RC1);

            if ((o->op_flags & OPf_WANT) == OPf_WANT_SCALAR)
                S_check_for_bool_cxt(o, 1, OPpASSIGN_TRUEBOOL, 0);
            break;
        }

        case OP_REF:
        case OP_BLESSED:
            /* if the op is used in boolean context, set the TRUEBOOL flag
             * which enables an optimisation at runtime which avoids creating
             * a stack temporary for known-true package names */
            if ((o->op_flags & OPf_WANT) == OPf_WANT_SCALAR)
                S_check_for_bool_cxt(o, 1, OPpTRUEBOOL, OPpMAYBE_TRUEBOOL);
            break;

        case OP_LENGTH:
            /* see if the op is used in known boolean context,
             * but not if OA_TARGLEX optimisation is enabled */
            if (   (o->op_flags & OPf_WANT) == OPf_WANT_SCALAR
                && !(o->op_private & OPpTARGET_MY)
            )
                S_check_for_bool_cxt(o, 1, OPpTRUEBOOL, 0);
            break;

        case OP_POS:
            /* see if the op is used in known boolean context */
            if ((o->op_flags & OPf_WANT) == OPf_WANT_SCALAR)
                S_check_for_bool_cxt(o, 1, OPpTRUEBOOL, 0);
            break;

        case OP_CUSTOM: {
            Perl_cpeep_t cpeep =
                XopENTRYCUSTOM(o, xop_peep);
            if (cpeep)
                cpeep(aTHX_ o, oldop);
            break;
        }

        }
        /* did we just null the current op? If so, re-process it to handle
         * eliding "empty" ops from the chain */
        if (o->op_type == OP_NULL && oldop && oldop->op_next == o) {
            o->op_opt = 0;
            o = oldop;
        }
        else {
            oldoldop = oldop;
            oldop = o;
        }
    }
    LEAVE;
}

void
Perl_peep(pTHX_ OP *o)
{
    CALL_RPEEP(o);
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
