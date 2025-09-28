#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

static bool
_runops_debug(int flag)
{
    dTHX;
    const bool d = PL_runops == Perl_runops_debug;

    if (flag >= 0)
	PL_runops = flag ? Perl_runops_debug : Perl_runops_standard;
    return d;
}

static SV *
DeadCode(pTHX)
{
#ifdef PURIFY
    return Nullsv;
#else
    SV* sva;
    SV* sv;
    SV* ret = newRV_noinc((SV*)newAV());
    SV* svend;
    int tm = 0, tref = 0, ts = 0, ta = 0, tas = 0;

    for (sva = PL_sv_arenaroot; sva; sva = (SV*)SvANY(sva)) {
	svend = &sva[SvREFCNT(sva)];
	for (sv = sva + 1; sv < svend; ++sv) {
	    if (SvTYPE(sv) == SVt_PVCV) {
		CV *cv = (CV*)sv;
		PADLIST* padlist;
                AV *argav;
		SV** svp;
		SV** pad;
		int i = 0, j, levelm, totm = 0, levelref, totref = 0;
		int levels, tots = 0, levela, tota = 0, levelas, totas = 0;
		int dumpit = 0;

		if (CvISXSUB(sv)) {
		    continue;		/* XSUB */
		}
		if (!CvGV(sv)) {
		    continue;		/* file-level scope. */
		}
		if (!CvROOT(cv)) {
		    /* PerlIO_printf(Perl_debug_log, "  no root?!\n"); */
		    continue;		/* autoloading stub. */
		}
		do_gvgv_dump(0, Perl_debug_log, "GVGV::GV", CvGV(sv));
		if (CvDEPTH(cv)) {
		    PerlIO_printf(Perl_debug_log, "  busy\n");
		    continue;
		}
		padlist = CvPADLIST(cv);
		svp = (SV**) PadlistARRAY(padlist);
		while (++i <= PadlistMAX(padlist)) { /* Depth. */
		    SV **args;
		    
		    if (!svp[i]) continue;
		    pad = AvARRAY((AV*)svp[i]);
		    argav = (AV*)pad[0];
		    if (!argav || (SV*)argav == &PL_sv_undef) {
			PerlIO_printf(Perl_debug_log, "    closure-template\n");
			continue;
		    }
		    args = AvARRAY(argav);
		    levelm = levels = levelref = levelas = 0;
		    levela = sizeof(SV*) * (AvMAX(argav) + 1);
		    if (AvREAL(argav)) {
			for (j = 0; j < AvFILL(argav); j++) {
			    if (SvROK(args[j])) {
				PerlIO_printf(Perl_debug_log, "     ref in args!\n");
				levelref++;
			    }
			    /* else if (SvPOK(args[j]) && SvPVX(args[j])) { */
			    else if (SvTYPE(args[j]) >= SVt_PV && SvLEN(args[j])) {
				levelas += SvLEN(args[j])/SvREFCNT(args[j]);
			    }
			}
		    }
		    for (j = 1; j < AvFILL((AV*)svp[1]); j++) {	/* Vars. */
			if (!pad[j]) continue;
			if (SvROK(pad[j])) {
			    levelref++;
			    do_sv_dump(0, Perl_debug_log, pad[j], 0, 4, 0, 0);
			    dumpit = 1;
			}
			/* else if (SvPOK(pad[j]) && SvPVX(pad[j])) { */
			else if (SvTYPE(pad[j]) >= SVt_PVAV) {
			    if (!SvPADMY(pad[j])) {
				levelref++;
				do_sv_dump(0, Perl_debug_log, pad[j], 0, 4, 0, 0);
				dumpit = 1;
			    }
			}
			else if (SvTYPE(pad[j]) >= SVt_PV && SvLEN(pad[j])) {
			    levels++;
			    levelm += SvLEN(pad[j])/SvREFCNT(pad[j]);
				/* Dump(pad[j],4); */
			}
		    }
		    PerlIO_printf(Perl_debug_log, "    level %i: refs: %i, strings: %i in %i,\targsarray: %i, argsstrings: %i\n", 
			    i, levelref, levelm, levels, levela, levelas);
		    totm += levelm;
		    tota += levela;
		    totas += levelas;
		    tots += levels;
		    totref += levelref;
		    if (dumpit)
			do_sv_dump(0, Perl_debug_log, (SV*)cv, 0, 2, 0, 0);
		}
		if (PadlistMAX(padlist) > 1) {
		    PerlIO_printf(Perl_debug_log, "  total: refs: %i, strings: %i in %i,\targsarrays: %i, argsstrings: %i\n", 
			    totref, totm, tots, tota, totas);
		}
		tref += totref;
		tm += totm;
		ts += tots;
		ta += tota;
		tas += totas;
	    }
	}
    }
    PerlIO_printf(Perl_debug_log, "total: refs: %i, strings: %i in %i\targsarray: %i, argsstrings: %i\n", tref, tm, ts, ta, tas);

    return ret;
#endif /* !PURIFY */
}

#if defined(MYMALLOC)
#   define mstat(str) dump_mstats(str)
#else
#   define mstat(str) \
	PerlIO_printf(Perl_debug_log, "%s: perl not compiled with MYMALLOC\n",str);
#endif

#if defined(MYMALLOC)

/* Very coarse overestimate, 2-per-power-of-2, one more to determine NBUCKETS. */
#  define _NBUCKETS (2*8*IVSIZE+1)

struct mstats_buffer 
{
    perl_mstats_t buffer;
    UV buf[_NBUCKETS*4];
};

static void
_fill_mstats(struct mstats_buffer *b, int level)
{
    dTHX;
    b->buffer.nfree  = b->buf;
    b->buffer.ntotal = b->buf + _NBUCKETS;
    b->buffer.bucket_mem_size = b->buf + 2*_NBUCKETS;
    b->buffer.bucket_available_size = b->buf + 3*_NBUCKETS;
    Zero(b->buf, (level ? 4*_NBUCKETS: 2*_NBUCKETS), unsigned long);
    get_mstats(&(b->buffer), _NBUCKETS, level);
}

static void
fill_mstats(SV *sv, int level)
{
    dTHX;

    if (SvREADONLY(sv))
	croak("Cannot modify a readonly value");
    sv_grow(sv, sizeof(struct mstats_buffer)+1);
    _fill_mstats((struct mstats_buffer*)SvPVX(sv),level);
    SvCUR_set(sv, sizeof(struct mstats_buffer));
    *SvEND(sv) = '\0';
    SvPOK_only(sv);
}

static void
_mstats_to_hv(HV *hv, const struct mstats_buffer *b, int level)
{
    dTHX;
    SV **svp;
    int type;

    svp = hv_fetchs(hv, "topbucket", 1);
    sv_setiv(*svp, b->buffer.topbucket);

    svp = hv_fetchs(hv, "topbucket_ev", 1);
    sv_setiv(*svp, b->buffer.topbucket_ev);

    svp = hv_fetchs(hv, "topbucket_odd", 1);
    sv_setiv(*svp, b->buffer.topbucket_odd);

    svp = hv_fetchs(hv, "totfree", 1);
    sv_setiv(*svp, b->buffer.totfree);

    svp = hv_fetchs(hv, "total", 1);
    sv_setiv(*svp, b->buffer.total);

    svp = hv_fetchs(hv, "total_chain", 1);
    sv_setiv(*svp, b->buffer.total_chain);

    svp = hv_fetchs(hv, "total_sbrk", 1);
    sv_setiv(*svp, b->buffer.total_sbrk);

    svp = hv_fetchs(hv, "sbrks", 1);
    sv_setiv(*svp, b->buffer.sbrks);

    svp = hv_fetchs(hv, "sbrk_good", 1);
    sv_setiv(*svp, b->buffer.sbrk_good);

    svp = hv_fetchs(hv, "sbrk_slack", 1);
    sv_setiv(*svp, b->buffer.sbrk_slack);

    svp = hv_fetchs(hv, "start_slack", 1);
    sv_setiv(*svp, b->buffer.start_slack);

    svp = hv_fetchs(hv, "sbrked_remains", 1);
    sv_setiv(*svp, b->buffer.sbrked_remains);
    
    svp = hv_fetchs(hv, "minbucket", 1);
    sv_setiv(*svp, b->buffer.minbucket);
    
    svp = hv_fetchs(hv, "nbuckets", 1);
    sv_setiv(*svp, b->buffer.nbuckets);

    if (_NBUCKETS < b->buffer.nbuckets) 
	warn("FIXME: internal mstats buffer too short");
    
    for (type = 0; type < (level ? 4 : 2); type++) {
	UV *p = 0, *p1 = 0, i;
	AV *av;
	static const char *types[4] = { 
	    "free", "used", "mem_size", "available_size"    
	};

	svp = hv_fetch(hv, types[type], strlen(types[type]), 1);

	if (SvOK(*svp) && !(SvROK(*svp) && SvTYPE(SvRV(*svp)) == SVt_PVAV))
	    croak("Unexpected value for the key '%s' in the mstats hash", types[type]);
	if (!SvOK(*svp)) {
	    av = newAV();
	    sv_setrv_noinc(*svp, (SV*)av);
	} else
	    av = (AV*)SvRV(*svp);

	av_extend(av, b->buffer.nbuckets - 1);
	/* XXXX What is the official way to reduce the size of the array? */
	switch (type) {
	case 0:
	    p = b->buffer.nfree;
	    break;
	case 1:
	    p = b->buffer.ntotal;
	    p1 = b->buffer.nfree;
	    break;
	case 2:
	    p = b->buffer.bucket_mem_size;
	    break;
	case 3:
	    p = b->buffer.bucket_available_size;
	    break;
	}
	for (i = 0; i < b->buffer.nbuckets; i++) {
	    svp = av_fetch(av, i, 1);
	    if (type == 1)
		sv_setiv(*svp, p[i]-p1[i]);
	    else
		sv_setuv(*svp, p[i]);
	}
    }
}

static void
mstats_fillhash(SV *sv, int level)
{
    struct mstats_buffer buf;

    if (!(SvROK(sv) && SvTYPE(SvRV(sv)) == SVt_PVHV))
	croak("Not a hash reference");
    _fill_mstats(&buf, level);
    _mstats_to_hv((HV *)SvRV(sv), &buf, level);
}

static void
mstats2hash(SV *sv, SV *rv, int level)
{
    if (!(SvROK(rv) && SvTYPE(SvRV(rv)) == SVt_PVHV))
	croak("Not a hash reference");
    if (!SvPOK(sv))
	croak("Undefined value when expecting mstats buffer");
    if (SvCUR(sv) != sizeof(struct mstats_buffer))
	croak("Wrong size for a value with a mstats buffer");
    _mstats_to_hv((HV *)SvRV(rv), (struct mstats_buffer*)SvPVX(sv), level);
}
#else	/* defined(MYMALLOC) */ 
static void
fill_mstats(SV *sv, int level)
{
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(level);
    croak("Cannot report mstats without Perl malloc");
}

static void
mstats_fillhash(SV *sv, int level)
{
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(level);
    croak("Cannot report mstats without Perl malloc");
}

static void
mstats2hash(SV *sv, SV *rv, int level)
{
    PERL_UNUSED_ARG(sv);
    PERL_UNUSED_ARG(rv);
    PERL_UNUSED_ARG(level);
    croak("Cannot report mstats without Perl malloc");
}
#endif	/* defined(MYMALLOC) */ 

#define _CvGV(cv)					\
	(SvROK(cv) && (SvTYPE(SvRV(cv))==SVt_PVCV)	\
	 ? SvREFCNT_inc(CvGV((CV*)SvRV(cv))) : &PL_sv_undef)

static void
S_do_dump(pTHX_ SV *const sv, I32 lim)
{
    SV *pv_lim_sv = perl_get_sv("Devel::Peek::pv_limit", 0);
    const STRLEN pv_lim = pv_lim_sv ? SvIV(pv_lim_sv) : 0;
    SV *dumpop = perl_get_sv("Devel::Peek::dump_ops", 0);
    const U16 save_dumpindent = PL_dumpindent;
    PL_dumpindent = 2;
    do_sv_dump(0, Perl_debug_log, sv, 0, lim,
	       (bool)(dumpop && SvTRUE(dumpop)), pv_lim);
    PL_dumpindent = save_dumpindent;
}

static OP *
S_pp_dump(pTHX)
{
    dSP;
    const I32 lim = PL_op->op_private == 2 ? (I32)POPi : 4;
    dPOPss;
    S_do_dump(aTHX_ sv, lim);
    RETPUSHUNDEF;
}

static OP *
S_ck_dump(pTHX_ OP *entersubop, GV *namegv, SV *cv)
{
    OP *parent, *pm, *first, *second;
    BINOP *newop;

    PERL_UNUSED_ARG(cv);

    ck_entersub_args_proto(entersubop, namegv,
			   newSVpvn_flags("$;$", 3, SVs_TEMP));

    parent = entersubop;
    pm = cUNOPx(entersubop)->op_first;
    if (!OpHAS_SIBLING(pm)) {
        parent = pm;
	pm = cUNOPx(pm)->op_first;
    }
    first = OpSIBLING(pm);
    second = OpSIBLING(first);
    if (!second) {
	/* It doesn’t really matter what we return here, as this only
	   occurs after yyerror.  */
	return entersubop;
    }
    /* we either have Dump($x):   [pushmark]->[first]->[ex-cvop]
     * or             Dump($x,1); [pushmark]->[first]->[second]->[ex-cvop]
     */
    if (!OpHAS_SIBLING(second))
        second = NULL;

    if (first->op_type == OP_RV2AV ||
	first->op_type == OP_PADAV ||
	first->op_type == OP_RV2HV ||
	first->op_type == OP_PADHV
    )
	first->op_flags |= OPf_REF;
    else
	first->op_flags &= ~OPf_MOD;

    /* splice out first (and optionally second) ops, then discard the rest
     * of the op tree */

    op_sibling_splice(parent, pm, second ? 2 : 1, NULL);
    op_free(entersubop);

    /* then attach first (and second) to a new binop */

    NewOp(1234, newop, 1, BINOP);
    newop->op_type   = OP_CUSTOM;
    newop->op_ppaddr = S_pp_dump;
    newop->op_private= second ? 2 : 1;
    newop->op_flags  = OPf_KIDS|OPf_WANT_SCALAR;
    op_sibling_splice((OP*)newop, NULL, 0, first);

    return (OP *)newop;
}

static const XOP my_xop = {
    XOPf_xop_name|XOPf_xop_desc|XOPf_xop_class,		/* xop_flags */
    "Devel_Peek_Dump",					/* xop_name */
    "Dump",						/* xop_desc */
    OA_BINOP,						/* xop_class */
    NULL						/* xop_peep */
};

MODULE = Devel::Peek		PACKAGE = Devel::Peek

void
mstat(str="Devel::Peek::mstat: ")
const char *str

void
fill_mstats(SV *sv, int level = 0)

void
mstats_fillhash(SV *sv, int level = 0)
    PROTOTYPE: \%;$

void
mstats2hash(SV *sv, SV *rv, int level = 0)
    PROTOTYPE: $\%;$

void
Dump(sv,lim=4)
SV *	sv
I32	lim
PPCODE:
{
    S_do_dump(aTHX_ sv, lim);
}

BOOT:
{
    CV * const cv = get_cvn_flags("Devel::Peek::Dump", 17, 0);
    assert(cv);
    cv_set_call_checker_flags(cv, S_ck_dump, (SV *)cv, 0);
    Perl_custom_op_register(aTHX_ S_pp_dump, &my_xop);
}

void
DumpArray(lim,...)
I32	lim
PPCODE:
{
    long i;
    SV *pv_lim_sv = perl_get_sv("Devel::Peek::pv_limit", 0);
    const STRLEN pv_lim = pv_lim_sv ? SvIV(pv_lim_sv) : 0;
    SV *dumpop = perl_get_sv("Devel::Peek::dump_ops", 0);
    const U16 save_dumpindent = PL_dumpindent;
    PL_dumpindent = 2;

    for (i=1; i<items; i++) {
	PerlIO_printf(Perl_debug_log, "Elt No. %ld  0x%" UVxf "\n", i - 1, PTR2UV(ST(i)));
	do_sv_dump(0, Perl_debug_log, ST(i), 0, lim,
		   (bool)(dumpop && SvTRUE(dumpop)), pv_lim);
    }
    PL_dumpindent = save_dumpindent;
}

void
DumpProg()
PPCODE:
{
    warn("dumpindent is %d", (int)PL_dumpindent);
    if (PL_main_root)
	op_dump(PL_main_root);
}

U32
SvREFCNT(sv)
SV *	sv
PROTOTYPE: \[$@%&*]
CODE:
    SvGETMAGIC(sv);
    if (!SvROK(sv))
        croak_xs_usage(cv, "SCALAR");
    RETVAL = SvREFCNT(SvRV(sv)) - 1; /* -1 because our ref doesn't count */
OUTPUT:
    RETVAL

SV *
DeadCode()
CODE:
    RETVAL = DeadCode(aTHX);
OUTPUT:
    RETVAL

MODULE = Devel::Peek		PACKAGE = Devel::Peek	PREFIX = _

SV *
_CvGV(cv)
    SV *cv

bool
_runops_debug(int flag = -1)
