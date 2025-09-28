/* Copyright (c) 1997-2000 Graham Barr <gbarr@pobox.com>. All rights reserved.
 * This program is free software; you can redistribute it and/or
 * modify it under the same terms as Perl itself.
 */

#define PERL_NO_GET_CONTEXT /* we want efficiency */
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#ifdef USE_PPPORT_H
#  define NEED_sv_2pv_flags 1
#  define NEED_newSVpvn_flags 1
#  define NEED_sv_catpvn_flags
#  include "ppport.h"
#endif

/* For uniqnum, define ACTUAL_NVSIZE to be the number *
 * of bytes that are actually used to store the NV    */

#if defined(USE_LONG_DOUBLE) && LDBL_MANT_DIG == 64
#  define ACTUAL_NVSIZE 10
#else
#  define ACTUAL_NVSIZE NVSIZE
#endif

/* Detect "DoubleDouble" nvtype */

#if defined(USE_LONG_DOUBLE) && LDBL_MANT_DIG == 106
#  define NV_IS_DOUBLEDOUBLE
#endif

#ifndef PERL_VERSION_DECIMAL
#  define PERL_VERSION_DECIMAL(r,v,s) (r*1000000 + v*1000 + s)
#endif
#ifndef PERL_DECIMAL_VERSION
#  define PERL_DECIMAL_VERSION \
        PERL_VERSION_DECIMAL(PERL_REVISION,PERL_VERSION,PERL_SUBVERSION)
#endif
#ifndef PERL_VERSION_GE
#  define PERL_VERSION_GE(r,v,s) \
        (PERL_DECIMAL_VERSION >= PERL_VERSION_DECIMAL(r,v,s))
#endif
#ifndef PERL_VERSION_LE
#  define PERL_VERSION_LE(r,v,s) \
        (PERL_DECIMAL_VERSION <= PERL_VERSION_DECIMAL(r,v,s))
#endif

#if PERL_VERSION_GE(5,6,0)
#  include "multicall.h"
#endif

#if !PERL_VERSION_GE(5,23,8)
#  define UNUSED_VAR_newsp PERL_UNUSED_VAR(newsp)
#else
#  define UNUSED_VAR_newsp NOOP
#endif

#ifndef CvISXSUB
#  define CvISXSUB(cv) CvXSUB(cv)
#endif

#ifndef HvNAMELEN_get
#define HvNAMELEN_get(stash) strlen(HvNAME(stash))
#endif

#ifndef HvNAMEUTF8
#define HvNAMEUTF8(stash) 0
#endif

#ifndef GvNAMEUTF8
#ifdef GvNAME_HEK
#define GvNAMEUTF8(gv) HEK_UTF8(GvNAME_HEK(gv))
#else
#define GvNAMEUTF8(gv) 0
#endif
#endif

#ifndef SV_CATUTF8
#define SV_CATUTF8 0
#endif

#ifndef SV_CATBYTES
#define SV_CATBYTES 0
#endif

#ifndef sv_catpvn_flags
#define sv_catpvn_flags(b,n,l,f) sv_catpvn(b,n,l)
#endif

#if !PERL_VERSION_GE(5,8,3)
static NV Perl_ceil(NV nv) {
    return -Perl_floor(-nv);
}
#endif

/* Some platforms have strict exports. And before 5.7.3 cxinc (or Perl_cxinc)
   was not exported. Therefore platforms like win32, VMS etc have problems
   so we redefine it here -- GMB
*/
#if !PERL_VERSION_GE(5,7,0)
/* Not in 5.6.1. */
#  ifdef cxinc
#    undef cxinc
#  endif
#  define cxinc() my_cxinc(aTHX)
static I32
my_cxinc(pTHX)
{
    cxstack_max = cxstack_max * 3 / 2;
    Renew(cxstack, cxstack_max + 1, struct context); /* fencepost bug in older CXINC macros requires +1 here */
    return cxstack_ix + 1;
}
#endif

#ifndef sv_copypv
#define sv_copypv(a, b) my_sv_copypv(aTHX_ a, b)
static void
my_sv_copypv(pTHX_ SV *const dsv, SV *const ssv)
{
    STRLEN len;
    const char * const s = SvPV_const(ssv,len);
    sv_setpvn(dsv,s,len);
    if(SvUTF8(ssv))
        SvUTF8_on(dsv);
    else
        SvUTF8_off(dsv);
}
#endif

#ifdef SVf_IVisUV
#  define slu_sv_value(sv) (SvIOK(sv)) ? (SvIOK_UV(sv)) ? (NV)(SvUVX(sv)) : (NV)(SvIVX(sv)) : (SvNV(sv))
#else
#  define slu_sv_value(sv) (SvIOK(sv)) ? (NV)(SvIVX(sv)) : (SvNV(sv))
#endif

#if PERL_VERSION < 13 || (PERL_VERSION == 13 && PERL_SUBVERSION < 9)
#  define PERL_HAS_BAD_MULTICALL_REFCOUNT
#endif

#ifndef SvNV_nomg
#  define SvNV_nomg SvNV
#endif

#if PERL_VERSION_GE(5,16,0)
#  define HAVE_UNICODE_PACKAGE_NAMES

#  ifndef sv_sethek
#    define sv_sethek(a, b)  Perl_sv_sethek(aTHX_ a, b)
#  endif

#  ifndef sv_ref
#  define sv_ref(dst, sv, ob) my_sv_ref(aTHX_ dst, sv, ob)
static SV *
my_sv_ref(pTHX_ SV *dst, const SV *sv, int ob)
{
  /* cargoculted from perl 5.22's sv.c */
  if(!dst)
    dst = sv_newmortal();

  if(ob && SvOBJECT(sv)) {
    if(HvNAME_get(SvSTASH(sv)))
      sv_sethek(dst, HvNAME_HEK(SvSTASH(sv)));
    else
      sv_setpvs(dst, "__ANON__");
  }
  else {
    const char *reftype = sv_reftype(sv, 0);
    sv_setpv(dst, reftype);
  }

  return dst;
}
#  endif
#endif /* HAVE_UNICODE_PACKAGE_NAMES */

enum slu_accum {
    ACC_IV,
    ACC_NV,
    ACC_SV,
};

static enum slu_accum accum_type(SV *sv) {
    if(SvAMAGIC(sv))
        return ACC_SV;

    if(SvIOK(sv) && !SvNOK(sv) && !SvUOK(sv))
        return ACC_IV;

    return ACC_NV;
}

/* Magic for set_subname */
static MGVTBL subname_vtbl;

static void MY_initrand(pTHX)
{
#if (PERL_VERSION < 9)
    struct op dmy_op;
    struct op *old_op = PL_op;

    /* We call pp_rand here so that Drand01 get initialized if rand()
       or srand() has not already been called
    */
    memzero((char*)(&dmy_op), sizeof(struct op));
    /* we let pp_rand() borrow the TARG allocated for this XS sub */
    dmy_op.op_targ = PL_op->op_targ;
    PL_op = &dmy_op;
    (void)*(PL_ppaddr[OP_RAND])(aTHX);
    PL_op = old_op;
#else
    /* Initialize Drand01 if rand() or srand() has
       not already been called
    */
    if(!PL_srand_called) {
        (void)seedDrand01((Rand_seed_t)Perl_seed(aTHX));
        PL_srand_called = TRUE;
    }
#endif
}

static double MY_callrand(pTHX_ CV *randcv)
{
    dSP;
    double ret, dummy;

    ENTER;
    PUSHMARK(SP);
    PUTBACK;

    call_sv((SV *)randcv, G_SCALAR);

    SPAGAIN;

    ret = modf(POPn, &dummy);      /* bound to < 1 */
    if(ret < 0) ret += 1.0; /* bound to 0 <= ret < 1 */

    LEAVE;

    return ret;
}

#define sv_to_cv(sv, subname) MY_sv_to_cv(aTHX_ sv, subname);
static CV* MY_sv_to_cv(pTHX_ SV* sv, const char * const subname)
{
    GV *gv;
    HV *stash;
    CV *cv = sv_2cv(sv, &stash, &gv, 0);

    if(cv == Nullcv)
        croak("Not a subroutine reference");

    if(!CvROOT(cv) && !CvXSUB(cv))
        croak("Undefined subroutine in %s", subname);

    return cv;
}

enum {
    ZIP_SHORTEST = 1,
    ZIP_LONGEST  = 2,

    ZIP_MESH          = 4,
    ZIP_MESH_LONGEST  = ZIP_MESH|ZIP_LONGEST,
    ZIP_MESH_SHORTEST = ZIP_MESH|ZIP_SHORTEST,
};

MODULE=List::Util       PACKAGE=List::Util

void
min(...)
PROTOTYPE: @
ALIAS:
    min = 0
    max = 1
CODE:
{
    int index;
    NV retval = 0.0; /* avoid 'uninit var' warning */
    SV *retsv;
    int magic;

    if(!items)
        XSRETURN_UNDEF;

    retsv = ST(0);
    SvGETMAGIC(retsv);
    magic = SvAMAGIC(retsv);
    if(!magic)
      retval = slu_sv_value(retsv);

    for(index = 1 ; index < items ; index++) {
        SV *stacksv = ST(index);
        SV *tmpsv;
        SvGETMAGIC(stacksv);
        if((magic || SvAMAGIC(stacksv)) && (tmpsv = amagic_call(retsv, stacksv, gt_amg, 0))) {
             if(SvTRUE(tmpsv) ? !ix : ix) {
                  retsv = stacksv;
                  magic = SvAMAGIC(retsv);
                  if(!magic) {
                      retval = slu_sv_value(retsv);
                  }
             }
        }
        else {
            NV val = slu_sv_value(stacksv);
            if(magic) {
                retval = slu_sv_value(retsv);
                magic = 0;
            }
            if(val < retval ? !ix : ix) {
                retsv = stacksv;
                retval = val;
            }
        }
    }
    ST(0) = retsv;
    XSRETURN(1);
}


void
sum(...)
PROTOTYPE: @
ALIAS:
    sum     = 0
    sum0    = 1
    product = 2
CODE:
{
    dXSTARG;
    SV *sv;
    IV retiv = 0;
    NV retnv = 0.0;
    SV *retsv = NULL;
    int index;
    enum slu_accum accum;
    int is_product = (ix == 2);
    SV *tmpsv;

    if(!items)
        switch(ix) {
            case 0: XSRETURN_UNDEF;
            case 1: ST(0) = sv_2mortal(newSViv(0)); XSRETURN(1);
            case 2: ST(0) = sv_2mortal(newSViv(1)); XSRETURN(1);
        }

    sv    = ST(0);
    SvGETMAGIC(sv);
    switch((accum = accum_type(sv))) {
    case ACC_SV:
        retsv = TARG;
        sv_setsv(retsv, sv);
        break;
    case ACC_IV:
        retiv = SvIV(sv);
        break;
    case ACC_NV:
        retnv = slu_sv_value(sv);
        break;
    }

    for(index = 1 ; index < items ; index++) {
        sv = ST(index);
        SvGETMAGIC(sv);
        if(accum < ACC_SV && SvAMAGIC(sv)){
            if(!retsv)
                retsv = TARG;
            sv_setnv(retsv, accum == ACC_NV ? retnv : retiv);
            accum = ACC_SV;
        }
        switch(accum) {
        case ACC_SV:
            tmpsv = amagic_call(retsv, sv,
                is_product ? mult_amg : add_amg,
                SvAMAGIC(retsv) ? AMGf_assign : 0);
            if(tmpsv) {
                switch((accum = accum_type(tmpsv))) {
                case ACC_SV:
                    retsv = tmpsv;
                    break;
                case ACC_IV:
                    retiv = SvIV(tmpsv);
                    break;
                case ACC_NV:
                    retnv = slu_sv_value(tmpsv);
                    break;
                }
            }
            else {
                /* fall back to default */
                accum = ACC_NV;
                is_product ? (retnv = SvNV(retsv) * SvNV(sv))
                           : (retnv = SvNV(retsv) + SvNV(sv));
            }
            break;
        case ACC_IV:
            if(is_product) {
                /* TODO: Consider if product() should shortcircuit the moment its
                 *   accumulator becomes zero
                 */
                /* XXX testing flags before running get_magic may
                 * cause some valid tied values to fallback to the NV path
                 * - DAPM */
                if(!SvNOK(sv) && SvIOK(sv)) {
                    IV i = SvIV(sv);
                    if (retiv == 0) /* avoid later division by zero */
                        break;
                    if (retiv < -1) { /* avoid -1 because that causes SIGFPE */
                        if (i < 0) {
                            if (i >= IV_MAX / retiv) {
                                retiv *= i;
                                break;
                            }
                        }
                        else {
                            if (i <= IV_MIN / retiv) {
                                retiv *= i;
                                break;
                            }
                        }
                    }
                    else if (retiv > 0) {
                        if (i < 0) {
                            if (i >= IV_MIN / retiv) {
                                retiv *= i;
                                break;
                            }
                        }
                        else {
                            if (i <= IV_MAX / retiv) {
                                retiv *= i;
                                break;
                            }
                        }
                    }
                }
                /* else fallthrough */
            }
            else {
                /* XXX testing flags before running get_magic may
                 * cause some valid tied values to fallback to the NV path
                 * - DAPM */
                if(!SvNOK(sv) && SvIOK(sv)) {
                    IV i = SvIV(sv);
                    if (retiv >= 0 && i >= 0) {
                        if (retiv <= IV_MAX - i) {
                            retiv += i;
                            break;
                        }
                        /* else fallthrough */
                    }
                    else if (retiv < 0 && i < 0) {
                        if (retiv >= IV_MIN - i) {
                            retiv += i;
                            break;
                        }
                        /* else fallthrough */
                    }
                    else {
                        /* mixed signs can't overflow */
                        retiv += i;
                        break;
                    }
                }
                /* else fallthrough */
            }

            retnv = retiv;
            accum = ACC_NV;
            /* FALLTHROUGH */
        case ACC_NV:
            is_product ? (retnv *= slu_sv_value(sv))
                       : (retnv += slu_sv_value(sv));
            break;
        }
    }

    if(!retsv)
        retsv = TARG;

    switch(accum) {
    case ACC_SV: /* nothing to do */
        break;
    case ACC_IV:
        sv_setiv(retsv, retiv);
        break;
    case ACC_NV:
        sv_setnv(retsv, retnv);
        break;
    }

    ST(0) = retsv;
    XSRETURN(1);
}

#define SLU_CMP_LARGER   1
#define SLU_CMP_SMALLER -1

void
minstr(...)
PROTOTYPE: @
ALIAS:
    minstr = SLU_CMP_LARGER
    maxstr = SLU_CMP_SMALLER
CODE:
{
    SV *left;
    int index;

    if(!items)
        XSRETURN_UNDEF;

    left = ST(0);
#ifdef OPpLOCALE
    if(MAXARG & OPpLOCALE) {
        for(index = 1 ; index < items ; index++) {
            SV *right = ST(index);
            if(sv_cmp_locale(left, right) == ix)
                left = right;
        }
    }
    else {
#endif
        for(index = 1 ; index < items ; index++) {
            SV *right = ST(index);
            if(sv_cmp(left, right) == ix)
                left = right;
        }
#ifdef OPpLOCALE
    }
#endif
    ST(0) = left;
    XSRETURN(1);
}




void
reduce(block,...)
    SV *block
PROTOTYPE: &@
ALIAS:
    reduce     = 0
    reductions = 1
CODE:
{
    SV *ret = sv_newmortal();
    int index;
    AV *retvals = NULL;
    GV *agv,*bgv;
    SV **args = &PL_stack_base[ax];
    CV *cv    = sv_to_cv(block, ix ? "reductions" : "reduce");

    if(items <= 1) {
        if(ix)
            XSRETURN(0);
        else
            XSRETURN_UNDEF;
    }

    agv = gv_fetchpv("a", GV_ADD, SVt_PV);
    bgv = gv_fetchpv("b", GV_ADD, SVt_PV);
    SAVESPTR(GvSV(agv));
    SAVESPTR(GvSV(bgv));
    GvSV(agv) = ret;
    SvSetMagicSV(ret, args[1]);

    if(ix) {
        /* Precreate an AV for return values; -1 for cv, -1 for top index */
        retvals = newAV();
        av_extend(retvals, items-1-1);

        /* so if throw an exception they can be reclaimed */
        SAVEFREESV(retvals);

        av_push(retvals, newSVsv(ret));
    }
#ifdef dMULTICALL
    assert(cv);
    if(!CvISXSUB(cv)) {
        dMULTICALL;
        I32 gimme = G_SCALAR;

        UNUSED_VAR_newsp;
        PUSH_MULTICALL(cv);
        for(index = 2 ; index < items ; index++) {
            GvSV(bgv) = args[index];
            MULTICALL;
            SvSetMagicSV(ret, *PL_stack_sp);
            if(ix)
                av_push(retvals, newSVsv(ret));
        }
#  ifdef PERL_HAS_BAD_MULTICALL_REFCOUNT
        if(CvDEPTH(multicall_cv) > 1)
            SvREFCNT_inc_simple_void_NN(multicall_cv);
#  endif
        POP_MULTICALL;
    }
    else
#endif
    {
        for(index = 2 ; index < items ; index++) {
            dSP;
            GvSV(bgv) = args[index];

            PUSHMARK(SP);
            call_sv((SV*)cv, G_SCALAR);

            SvSetMagicSV(ret, *PL_stack_sp);
            if(ix)
                av_push(retvals, newSVsv(ret));
        }
    }

    if(ix) {
        int i;
        SV **svs = AvARRAY(retvals);
        /* steal the SVs from retvals */
        for(i = 0; i < items-1; i++) {
            ST(i) = sv_2mortal(svs[i]);
            svs[i] = NULL;
        }

        XSRETURN(items-1);
    }
    else {
        ST(0) = ret;
        XSRETURN(1);
    }
}

void
first(block,...)
    SV *block
PROTOTYPE: &@
CODE:
{
    int index;
    SV **args = &PL_stack_base[ax];
    CV *cv    = sv_to_cv(block, "first");

    if(items <= 1)
        XSRETURN_UNDEF;

    SAVESPTR(GvSV(PL_defgv));
#ifdef dMULTICALL
    assert(cv);
    if(!CvISXSUB(cv)) {
        dMULTICALL;
        I32 gimme = G_SCALAR;

        UNUSED_VAR_newsp;
        PUSH_MULTICALL(cv);

        for(index = 1 ; index < items ; index++) {
            SV *def_sv = GvSV(PL_defgv) = args[index];
#  ifdef SvTEMP_off
            SvTEMP_off(def_sv);
#  endif
            MULTICALL;
            if(SvTRUEx(*PL_stack_sp)) {
#  ifdef PERL_HAS_BAD_MULTICALL_REFCOUNT
                if(CvDEPTH(multicall_cv) > 1)
                    SvREFCNT_inc_simple_void_NN(multicall_cv);
#  endif
                POP_MULTICALL;
                ST(0) = ST(index);
                XSRETURN(1);
            }
        }
#  ifdef PERL_HAS_BAD_MULTICALL_REFCOUNT
        if(CvDEPTH(multicall_cv) > 1)
            SvREFCNT_inc_simple_void_NN(multicall_cv);
#  endif
        POP_MULTICALL;
    }
    else
#endif
    {
        for(index = 1 ; index < items ; index++) {
            dSP;
            GvSV(PL_defgv) = args[index];

            PUSHMARK(SP);
            call_sv((SV*)cv, G_SCALAR);
            if(SvTRUEx(*PL_stack_sp)) {
                ST(0) = ST(index);
                XSRETURN(1);
            }
        }
    }
    XSRETURN_UNDEF;
}


void
any(block,...)
    SV *block
ALIAS:
    none   = 0
    all    = 1
    any    = 2
    notall = 3
PROTOTYPE: &@
PPCODE:
{
    int ret_true = !(ix & 2); /* return true at end of loop for none/all; false for any/notall */
    int invert   =  (ix & 1); /* invert block test for all/notall */
    SV **args = &PL_stack_base[ax];
    CV *cv    = sv_to_cv(block,
                         ix == 0 ? "none" :
                         ix == 1 ? "all" :
                         ix == 2 ? "any" :
                         ix == 3 ? "notall" :
                         "unknown 'any' alias");

    SAVESPTR(GvSV(PL_defgv));
#ifdef dMULTICALL
    assert(cv);
    if(!CvISXSUB(cv)) {
        dMULTICALL;
        I32 gimme = G_SCALAR;
        int index;

        UNUSED_VAR_newsp;
        PUSH_MULTICALL(cv);
        for(index = 1; index < items; index++) {
            SV *def_sv = GvSV(PL_defgv) = args[index];
#  ifdef SvTEMP_off
            SvTEMP_off(def_sv);
#  endif

            MULTICALL;
            if(SvTRUEx(*PL_stack_sp) ^ invert) {
                POP_MULTICALL;
                ST(0) = ret_true ? &PL_sv_no : &PL_sv_yes;
                XSRETURN(1);
            }
        }
        POP_MULTICALL;
    }
    else
#endif
    {
        int index;
        for(index = 1; index < items; index++) {
            dSP;
            GvSV(PL_defgv) = args[index];

            PUSHMARK(SP);
            call_sv((SV*)cv, G_SCALAR);
            if(SvTRUEx(*PL_stack_sp) ^ invert) {
                ST(0) = ret_true ? &PL_sv_no : &PL_sv_yes;
                XSRETURN(1);
            }
        }
    }

    ST(0) = ret_true ? &PL_sv_yes : &PL_sv_no;
    XSRETURN(1);
}

void
head(size,...)
PROTOTYPE: $@
ALIAS:
    head = 0
    tail = 1
PPCODE:
{
    int size = 0;
    int start = 0;
    int end = 0;
    int i = 0;

    size = SvIV( ST(0) );

    if ( ix == 0 ) {
        start = 1;
        end = start + size;
        if ( size < 0 ) {
            end += items - 1;
        }
        if ( end > items ) {
            end = items;
        }
    }
    else {
        end = items;
        if ( size < 0 ) {
            start = -size + 1;
        }
        else {
            start = end - size;
        }
        if ( start < 1 ) {
            start = 1;
        }
    }

    if ( end <= start ) {
        XSRETURN(0);
    }
    else {
        EXTEND( SP, end - start );
        for ( i = start; i < end; i++ ) {
            PUSHs( sv_2mortal( newSVsv( ST(i) ) ) );
        }
        XSRETURN( end - start );
    }
}

void
pairs(...)
PROTOTYPE: @
PPCODE:
{
    int argi = 0;
    int reti = 0;
    HV *pairstash = get_hv("List::Util::_Pair::", GV_ADD);

    if(items % 2 && ckWARN(WARN_MISC))
        warn("Odd number of elements in pairs");

    {
        for(; argi < items; argi += 2) {
            SV *a = ST(argi);
            SV *b = argi < items-1 ? ST(argi+1) : &PL_sv_undef;

            AV *av = newAV();
            av_push(av, newSVsv(a));
            av_push(av, newSVsv(b));

            ST(reti) = sv_2mortal(newRV_noinc((SV *)av));
            sv_bless(ST(reti), pairstash);
            reti++;
        }
    }

    XSRETURN(reti);
}

void
unpairs(...)
PROTOTYPE: @
PPCODE:
{
    /* Unlike pairs(), we're going to trash the input values on the stack
     * almost as soon as we start generating output. So clone them first
     */
    int i;
    SV **args_copy;
    Newx(args_copy, items, SV *);
    SAVEFREEPV(args_copy);

    Copy(&ST(0), args_copy, items, SV *);

    for(i = 0; i < items; i++) {
        SV *pair = args_copy[i];
        AV *pairav;

        SvGETMAGIC(pair);

        if(SvTYPE(pair) != SVt_RV)
            croak("Not a reference at List::Util::unpairs() argument %d", i);
        if(SvTYPE(SvRV(pair)) != SVt_PVAV)
            croak("Not an ARRAY reference at List::Util::unpairs() argument %d", i);

        /* TODO: assert pair is an ARRAY ref */
        pairav = (AV *)SvRV(pair);

        EXTEND(SP, 2);

        if(AvFILL(pairav) >= 0)
            mPUSHs(newSVsv(AvARRAY(pairav)[0]));
        else
            PUSHs(&PL_sv_undef);

        if(AvFILL(pairav) >= 1)
            mPUSHs(newSVsv(AvARRAY(pairav)[1]));
        else
            PUSHs(&PL_sv_undef);
    }

    XSRETURN(items * 2);
}

void
pairkeys(...)
PROTOTYPE: @
PPCODE:
{
    int argi = 0;
    int reti = 0;

    if(items % 2 && ckWARN(WARN_MISC))
        warn("Odd number of elements in pairkeys");

    {
        for(; argi < items; argi += 2) {
            SV *a = ST(argi);

            ST(reti++) = sv_2mortal(newSVsv(a));
        }
    }

    XSRETURN(reti);
}

void
pairvalues(...)
PROTOTYPE: @
PPCODE:
{
    int argi = 0;
    int reti = 0;

    if(items % 2 && ckWARN(WARN_MISC))
        warn("Odd number of elements in pairvalues");

    {
        for(; argi < items; argi += 2) {
            SV *b = argi < items-1 ? ST(argi+1) : &PL_sv_undef;

            ST(reti++) = sv_2mortal(newSVsv(b));
        }
    }

    XSRETURN(reti);
}

void
pairfirst(block,...)
    SV *block
PROTOTYPE: &@
PPCODE:
{
    GV *agv,*bgv;
    CV *cv = sv_to_cv(block, "pairfirst");
    I32 ret_gimme = GIMME_V;
    int argi = 1; /* "shift" the block */

    if(!(items % 2) && ckWARN(WARN_MISC))
        warn("Odd number of elements in pairfirst");

    agv = gv_fetchpv("a", GV_ADD, SVt_PV);
    bgv = gv_fetchpv("b", GV_ADD, SVt_PV);
    SAVESPTR(GvSV(agv));
    SAVESPTR(GvSV(bgv));
#ifdef dMULTICALL
    assert(cv);
    if(!CvISXSUB(cv)) {
        /* Since MULTICALL is about to move it */
        SV **stack = PL_stack_base + ax;

        dMULTICALL;
        I32 gimme = G_SCALAR;

        UNUSED_VAR_newsp;
        PUSH_MULTICALL(cv);
        for(; argi < items; argi += 2) {
            SV *a = GvSV(agv) = stack[argi];
            SV *b = GvSV(bgv) = argi < items-1 ? stack[argi+1] : &PL_sv_undef;

            MULTICALL;

            if(!SvTRUEx(*PL_stack_sp))
                continue;

            POP_MULTICALL;
            if(ret_gimme == G_LIST) {
                ST(0) = sv_mortalcopy(a);
                ST(1) = sv_mortalcopy(b);
                XSRETURN(2);
            }
            else
                XSRETURN_YES;
        }
        POP_MULTICALL;
        XSRETURN(0);
    }
    else
#endif
    {
        for(; argi < items; argi += 2) {
            dSP;
            SV *a = GvSV(agv) = ST(argi);
            SV *b = GvSV(bgv) = argi < items-1 ? ST(argi+1) : &PL_sv_undef;

            PUSHMARK(SP);
            call_sv((SV*)cv, G_SCALAR);

            SPAGAIN;

            if(!SvTRUEx(*PL_stack_sp))
                continue;

            if(ret_gimme == G_LIST) {
                ST(0) = sv_mortalcopy(a);
                ST(1) = sv_mortalcopy(b);
                XSRETURN(2);
            }
            else
                XSRETURN_YES;
        }
    }

    XSRETURN(0);
}

void
pairgrep(block,...)
    SV *block
PROTOTYPE: &@
PPCODE:
{
    GV *agv,*bgv;
    CV *cv = sv_to_cv(block, "pairgrep");
    I32 ret_gimme = GIMME_V;

    /* This function never returns more than it consumed in arguments. So we
     * can build the results "live", behind the arguments
     */
    int argi = 1; /* "shift" the block */
    int reti = 0;

    if(!(items % 2) && ckWARN(WARN_MISC))
        warn("Odd number of elements in pairgrep");

    agv = gv_fetchpv("a", GV_ADD, SVt_PV);
    bgv = gv_fetchpv("b", GV_ADD, SVt_PV);
    SAVESPTR(GvSV(agv));
    SAVESPTR(GvSV(bgv));
#ifdef dMULTICALL
    assert(cv);
    if(!CvISXSUB(cv)) {
        /* Since MULTICALL is about to move it */
        SV **stack = PL_stack_base + ax;
        int i;

        dMULTICALL;
        I32 gimme = G_SCALAR;

        UNUSED_VAR_newsp;
        PUSH_MULTICALL(cv);
        for(; argi < items; argi += 2) {
            SV *a = GvSV(agv) = stack[argi];
            SV *b = GvSV(bgv) = argi < items-1 ? stack[argi+1] : &PL_sv_undef;

            MULTICALL;

            if(SvTRUEx(*PL_stack_sp)) {
                if(ret_gimme == G_LIST) {
                    /* We can't mortalise yet or they'd be mortal too early */
                    stack[reti++] = newSVsv(a);
                    stack[reti++] = newSVsv(b);
                }
                else if(ret_gimme == G_SCALAR)
                    reti++;
            }
        }
        POP_MULTICALL;

        if(ret_gimme == G_LIST)
            for(i = 0; i < reti; i++)
                sv_2mortal(stack[i]);
    }
    else
#endif
    {
        for(; argi < items; argi += 2) {
            dSP;
            SV *a = GvSV(agv) = ST(argi);
            SV *b = GvSV(bgv) = argi < items-1 ? ST(argi+1) : &PL_sv_undef;

            PUSHMARK(SP);
            call_sv((SV*)cv, G_SCALAR);

            SPAGAIN;

            if(SvTRUEx(*PL_stack_sp)) {
                if(ret_gimme == G_LIST) {
                    ST(reti++) = sv_mortalcopy(a);
                    ST(reti++) = sv_mortalcopy(b);
                }
                else if(ret_gimme == G_SCALAR)
                    reti++;
            }
        }
    }

    if(ret_gimme == G_LIST)
        XSRETURN(reti);
    else if(ret_gimme == G_SCALAR) {
        ST(0) = newSViv(reti);
        XSRETURN(1);
    }
}

void
pairmap(block,...)
    SV *block
PROTOTYPE: &@
PPCODE:
{
    GV *agv,*bgv;
    CV *cv = sv_to_cv(block, "pairmap");
    SV **args_copy = NULL;
    I32 ret_gimme = GIMME_V;

    int argi = 1; /* "shift" the block */
    int reti = 0;

    if(!(items % 2) && ckWARN(WARN_MISC))
        warn("Odd number of elements in pairmap");

    agv = gv_fetchpv("a", GV_ADD, SVt_PV);
    bgv = gv_fetchpv("b", GV_ADD, SVt_PV);
    SAVESPTR(GvSV(agv));
    SAVESPTR(GvSV(bgv));
/* This MULTICALL-based code appears to fail on perl 5.10.0 and 5.8.9
 * Skip it on those versions (RT#87857)
 */
#if defined(dMULTICALL) && (PERL_VERSION_GE(5,10,1) || PERL_VERSION_LE(5,8,8))
    assert(cv);
    if(!CvISXSUB(cv)) {
        /* Since MULTICALL is about to move it */
        SV **stack = PL_stack_base + ax;
        I32 ret_gimme = GIMME_V;
        int i;
        AV *spill = NULL; /* accumulates results if too big for stack */

        dMULTICALL;
        I32 gimme = G_LIST;

        UNUSED_VAR_newsp;
        PUSH_MULTICALL(cv);
        for(; argi < items; argi += 2) {
            int count;

            GvSV(agv) = stack[argi];
            GvSV(bgv) = argi < items-1 ? stack[argi+1]: &PL_sv_undef;

            MULTICALL;
            count = PL_stack_sp - PL_stack_base;

            if (count > 2 || spill) {
                /* We can't return more than 2 results for a given input pair
                 * without trashing the remaining arguments on the stack still
                 * to be processed, or possibly overrunning the stack end.
                 * So, we'll accumulate the results in a temporary buffer
                 * instead.
                 * We didn't do this initially because in the common case, most
                 * code blocks will return only 1 or 2 items so it won't be
                 * necessary
                 */
                int fill;

                if (!spill) {
                    spill = newAV();
                    AvREAL_off(spill); /* don't ref count its contents */
                    /* can't mortalize here as every nextstate in the code
                     * block frees temps */
                    SAVEFREESV(spill);
                }

                fill = (int)AvFILL(spill);
                av_extend(spill, fill + count);
                for(i = 0; i < count; i++)
                    (void)av_store(spill, ++fill,
                                    newSVsv(PL_stack_base[i + 1]));
            }
            else
                for(i = 0; i < count; i++)
                    stack[reti++] = newSVsv(PL_stack_base[i + 1]);
        }

        if (spill) {
            /* the POP_MULTICALL will trigger the SAVEFREESV above;
             * keep it alive  it on the temps stack instead */
            SvREFCNT_inc_simple_void_NN(spill);
            sv_2mortal((SV*)spill);
        }

        POP_MULTICALL;

        if (spill) {
            int n = (int)AvFILL(spill) + 1;
            SP = &ST(reti - 1);
            EXTEND(SP, n);
            for (i = 0; i < n; i++)
                *++SP = *av_fetch(spill, i, FALSE);
            reti += n;
            av_clear(spill);
        }

        if(ret_gimme == G_LIST)
            for(i = 0; i < reti; i++)
                sv_2mortal(ST(i));
    }
    else
#endif
    {
        for(; argi < items; argi += 2) {
            dSP;
            int count;
            int i;

            GvSV(agv) = args_copy ? args_copy[argi] : ST(argi);
            GvSV(bgv) = argi < items-1 ?
                (args_copy ? args_copy[argi+1] : ST(argi+1)) :
                &PL_sv_undef;

            PUSHMARK(SP);
            count = call_sv((SV*)cv, G_LIST);

            SPAGAIN;

            if(count > 2 && !args_copy && ret_gimme == G_LIST) {
                int n_args = items - argi;
                Newx(args_copy, n_args, SV *);
                SAVEFREEPV(args_copy);

                Copy(&ST(argi), args_copy, n_args, SV *);

                argi = 0;
                items = n_args;
            }

            if(ret_gimme == G_LIST)
                for(i = 0; i < count; i++)
                    ST(reti++) = sv_mortalcopy(SP[i - count + 1]);
            else
                reti += count;

            PUTBACK;
        }
    }

    if(ret_gimme == G_LIST)
        XSRETURN(reti);

    ST(0) = sv_2mortal(newSViv(reti));
    XSRETURN(1);
}

void
shuffle(...)
PROTOTYPE: @
CODE:
{
    int index;
    SV *randsv = get_sv("List::Util::RAND", 0);
    CV * const randcv = randsv && SvROK(randsv) && SvTYPE(SvRV(randsv)) == SVt_PVCV ?
        (CV *)SvRV(randsv) : NULL;

    if(!randcv)
        MY_initrand(aTHX);

    for (index = items ; index > 1 ; ) {
        int swap = (int)(
            (randcv ? MY_callrand(aTHX_ randcv) : Drand01()) * (double)(index--)
        );
        SV *tmp = ST(swap);
        ST(swap) = ST(index);
        ST(index) = tmp;
    }

    XSRETURN(items);
}

void
sample(...)
PROTOTYPE: $@
CODE:
{
    IV count = items ? SvUV(ST(0)) : 0;
    IV reti = 0;
    SV *randsv = get_sv("List::Util::RAND", 0);
    CV * const randcv = randsv && SvROK(randsv) && SvTYPE(SvRV(randsv)) == SVt_PVCV ?
        (CV *)SvRV(randsv) : NULL;

    if(!count)
        XSRETURN(0);

    /* Now we've extracted count from ST(0) the rest of this logic will be a
     * lot neater if we move the topmost item into ST(0) so we can just work
     * within 0..items-1 */
    ST(0) = POPs;
    items--;

    if(count > items)
        count = items;

    if(!randcv)
        MY_initrand(aTHX);

    /* Partition the stack into ST(0)..ST(reti-1) containing the sampled results
     * and ST(reti)..ST(items-1) containing the remaining pending candidates
     */
    while(reti < count) {
        int index = (int)(
            (randcv ? MY_callrand(aTHX_ randcv) : Drand01()) * (double)(items - reti)
        );

        SV *selected = ST(reti + index);
        /* preserve the element we're about to stomp on by putting it back into
         * the pending partition */
        ST(reti + index) = ST(reti);

        ST(reti) = selected;
        reti++;
    }

    XSRETURN(reti);
}


void
uniq(...)
PROTOTYPE: @
ALIAS:
    uniqint = 0
    uniqstr = 1
    uniq    = 2
CODE:
{
    int retcount = 0;
    int index;
    SV **args = &PL_stack_base[ax];
    HV *seen;
    int seen_undef = 0;

    if(items == 0 || (items == 1 && !SvGAMAGIC(args[0]) && SvOK(args[0]))) {
        /* Optimise for the case of the empty list or a defined nonmagic
         * singleton. Leave a singleton magical||undef for the regular case */
        retcount = items;
        goto finish;
    }

    sv_2mortal((SV *)(seen = newHV()));

    for(index = 0 ; index < items ; index++) {
        SV *arg = args[index];
#ifdef HV_FETCH_EMPTY_HE
        HE *he;
#endif

        if(SvGAMAGIC(arg))
            /* clone the value so we don't invoke magic again */
            arg = sv_mortalcopy(arg);

        if(ix == 2 && !SvOK(arg)) {
            /* special handling of undef for uniq() */
            if(seen_undef)
                continue;

            seen_undef++;

            if(GIMME_V == G_LIST)
                ST(retcount) = arg;
            retcount++;
            continue;
        }
        if(ix == 0) {
            /* uniqint */
            /* coerce to integer */
#if PERL_VERSION >= 8
            /* int_amg only appeared in perl 5.8.0 */
            if(SvAMAGIC(arg) && (arg = AMG_CALLun(arg, int)))
                ; /* nothing to do */
            else
#endif
            if(!SvOK(arg) || SvNOK(arg) || SvPOK(arg))
            {
                /* Convert undef, NVs and PVs into a well-behaved int */
                NV nv = SvNV(arg);

                if(nv > (NV)UV_MAX)
                    /* Too positive for UV - use NV */
                    arg = newSVnv(Perl_floor(nv));
                else if(nv < (NV)IV_MIN)
                    /* Too negative for IV - use NV */
                    arg = newSVnv(Perl_ceil(nv));
                else if(nv > 0 && (UV)nv > (UV)IV_MAX)
                    /* Too positive for IV - use UV */
                    arg = newSVuv(nv);
                else
                    /* Must now fit into IV */
                    arg = newSViv(nv);

                sv_2mortal(arg);
            }
        }
#ifdef HV_FETCH_EMPTY_HE
        he = (HE*) hv_common(seen, arg, NULL, 0, 0, HV_FETCH_LVALUE | HV_FETCH_EMPTY_HE, NULL, 0);
        if (HeVAL(he))
            continue;

        HeVAL(he) = &PL_sv_undef;
#else
        if (hv_exists_ent(seen, arg, 0))
            continue;

        hv_store_ent(seen, arg, &PL_sv_yes, 0);
#endif

        if(GIMME_V == G_LIST)
            ST(retcount) = SvOK(arg) ? arg : sv_2mortal(newSVpvn("", 0));
        retcount++;
    }

  finish:
    if(GIMME_V == G_LIST)
        XSRETURN(retcount);
    else
        ST(0) = sv_2mortal(newSViv(retcount));
}

void
uniqnum(...)
PROTOTYPE: @
CODE:
{
    int retcount = 0;
    int index;
    SV **args = &PL_stack_base[ax];
    HV *seen;
    /* A temporary buffer for number stringification */
    SV *keysv = sv_newmortal();

    if(items == 0 || (items == 1 && !SvGAMAGIC(args[0]) && SvOK(args[0]))) {
        /* Optimise for the case of the empty list or a defined nonmagic
         * singleton. Leave a singleton magical||undef for the regular case */
        retcount = items;
        goto finish;
    }

    sv_2mortal((SV *)(seen = newHV()));

    for(index = 0 ; index < items ; index++) {
        SV *arg = args[index];
        NV nv_arg;
#ifdef HV_FETCH_EMPTY_HE
        HE* he;
#endif

        if(SvGAMAGIC(arg))
            /* clone the value so we don't invoke magic again */
            arg = sv_mortalcopy(arg);

        if(SvOK(arg) && !(SvUOK(arg) || SvIOK(arg) || SvNOK(arg))) {
#if PERL_VERSION >= 8
            SvIV(arg); /* sets SVf_IOK/SVf_IsUV if it's an integer */
#else
            SvNV(arg); /* SvIV() sets SVf_IOK even on floats on 5.6 */
#endif
        }
#if NVSIZE > IVSIZE                          /* $Config{nvsize} > $Config{ivsize} */
        /* Avoid altering arg's flags */
        if(SvUOK(arg))      nv_arg = (NV)SvUV(arg);
        else if(SvIOK(arg)) nv_arg = (NV)SvIV(arg);
        else                nv_arg = SvNV(arg);

        /* use 0 for all zeros */
        if(nv_arg == 0) sv_setpvs(keysv, "0");

        /* for NaN, use the platform's normal stringification */
        else if (nv_arg != nv_arg) sv_setpvf(keysv, "%" NVgf, nv_arg);
#ifdef NV_IS_DOUBLEDOUBLE
        /* If the least significant double is zero, it could be either 0.0     *
         * or -0.0. We therefore ignore the least significant double and       *
         * assign to keysv the bytes of the most significant double only.      */
        else if(nv_arg == (double)nv_arg) {
            double double_arg = (double)nv_arg;
            sv_setpvn(keysv, (char *) &double_arg, 8);
        }
#endif
        else {
            /* Use the byte structure of the NV.                               *
             * ACTUAL_NVSIZE == sizeof(NV) minus the number of bytes           *
             * that are allocated but never used. (It is only the 10-byte      *
             * extended precision long double that allocates bytes that are    *
             * never used. For all other NV types ACTUAL_NVSIZE == sizeof(NV). */
            sv_setpvn(keysv, (char *) &nv_arg, ACTUAL_NVSIZE);
        }
#else                                    /* $Config{nvsize} == $Config{ivsize} == 8 */
        if( SvIOK(arg) || !SvOK(arg) ) {

            /* It doesn't matter if SvUOK(arg) is TRUE */
            IV iv = SvIV(arg);

            /* use "0" for all zeros */
            if(iv == 0) sv_setpvs(keysv, "0");

            else {
                int uok = SvUOK(arg);
                int sign = ( iv > 0 || uok ) ? 1 : -1;

                /* Set keysv to the bytes of SvNV(arg) if and only if the integer value  *
                 * held by arg can be represented exactly as a double - ie if there are  *
                 * no more than 51 bits between its least significant set bit and its    *
                 * most significant set bit.                                             *
                 * The neatest approach I could find was provided by roboticus at:       *
                 *     https://www.perlmonks.org/?node_id=11113490                       *
                 * First, identify the lowest set bit and assign its value to an IV.     *
                 * Note that this value will always be > 0, and always a power of 2.     */
                IV lowest_set = iv & -iv;

                /* Second, shift it left 53 bits to get location of the first bit        *
                 * beyond arg's highest "allowed" set bit.                                                    *
                 * NOTE: If lowest set bit is initially far enough left, then this left  *
                 * shift operation will result in a value of 0, which is fine.           *
                 * Then subtract 1 so that all of the ("allowed") bits below the set bit *
                 * are 1 && all other ("disallowed") bits are set to 0.                  *
                 * (If the value prior to subtraction was 0, then subtracting 1 will set *
                 * all bits - which is also fine.)                                       */
                UV valid_bits = (lowest_set << 53) - 1;

                /* The value of arg can be exactly represented by a double unless one    *
                 * or more of its "disallowed" bits are set - ie if iv & (~valid_bits)   *
                 * is untrue. However, if (iv < 0 && !SvUOK(arg)) we need to multiply iv *
                 * by -1 prior to performing that '&' operation - so multiply iv by sign.*/
                if( !((iv * sign) & (~valid_bits)) ) {
                    /* Avoid altering arg's flags */
                    nv_arg = uok ? (NV)SvUV(arg) : (NV)SvIV(arg);
                    sv_setpvn(keysv, (char *) &nv_arg, 8);
                }
                else {
                    /* Read in the bytes, rather than the numeric value of the IV/UV as  *
                     * this is more efficient, despite having to sv_catpvn an extra byte.*/
                    sv_setpvn(keysv, (char *) &iv, 8);
                    /* We add an extra byte to distinguish between an IV/UV and an NV.   *
                     * We also use that byte to distinguish between a -ve IV and a UV.   */
                    if(uok) sv_catpvn(keysv, "U", 1);
                    else    sv_catpvn(keysv, "I", 1);
                }
            }
        }
        else {
            nv_arg = SvNV(arg);

            /* for NaN, use the platform's normal stringification */
            if (nv_arg != nv_arg) sv_setpvf(keysv, "%" NVgf, nv_arg);

            /* use "0" for all zeros */
            else if(nv_arg == 0) sv_setpvs(keysv, "0");
            else sv_setpvn(keysv, (char *) &nv_arg, 8);
        }
#endif
#ifdef HV_FETCH_EMPTY_HE
        he = (HE*) hv_common(seen, NULL, SvPVX(keysv), SvCUR(keysv), 0, HV_FETCH_LVALUE | HV_FETCH_EMPTY_HE, NULL, 0);
        if (HeVAL(he))
            continue;

        HeVAL(he) = &PL_sv_undef;
#else
        if(hv_exists(seen, SvPVX(keysv), SvCUR(keysv)))
            continue;

        hv_store(seen, SvPVX(keysv), SvCUR(keysv), &PL_sv_yes, 0);
#endif

        if(GIMME_V == G_LIST)
            ST(retcount) = SvOK(arg) ? arg : sv_2mortal(newSViv(0));
        retcount++;
    }

  finish:
    if(GIMME_V == G_LIST)
        XSRETURN(retcount);
    else
        ST(0) = sv_2mortal(newSViv(retcount));
}

void
zip(...)
ALIAS:
    zip_longest   = ZIP_LONGEST
    zip_shortest  = ZIP_SHORTEST
    mesh          = ZIP_MESH
    mesh_longest  = ZIP_MESH_LONGEST
    mesh_shortest = ZIP_MESH_SHORTEST
PPCODE:
    Size_t nlists = items; /* number of lists */
    AV **lists;         /* inbound lists */
    Size_t len = 0;        /* length of longest inbound list = length of result */
    Size_t i;
    bool is_mesh = (ix & ZIP_MESH);
    ix &= ~ZIP_MESH;

    if(!nlists)
        XSRETURN(0);

    Newx(lists, nlists, AV *);
    SAVEFREEPV(lists);

    /* TODO: This may or maynot work on objects with arrayification overload */
    /* Remember to unit test it */

    for(i = 0; i < nlists; i++) {
        SV *arg = ST(i);
        AV *av;

        if(!SvROK(arg) || SvTYPE(SvRV(arg)) != SVt_PVAV)
            croak("Expected an ARRAY reference to zip");
        av = lists[i] = (AV *)SvRV(arg);

        if(!i) {
            len = av_count(av);
            continue;
        }

        switch(ix) {
            case 0: /* zip is alias to zip_longest */
            case ZIP_LONGEST:
                if(av_count(av) > len)
                    len = av_count(av);
                break;

            case ZIP_SHORTEST:
                if(av_count(av) < len)
                    len = av_count(av);
                break;
        }
    }

    if(is_mesh) {
        SSize_t retcount = (SSize_t)(len * nlists);

        EXTEND(SP, retcount);

        for(i = 0; i < len; i++) {
            Size_t listi;

            for(listi = 0; listi < nlists; listi++) {
                SV *item = (i < av_count(lists[listi])) ?
                    AvARRAY(lists[listi])[i] :
                    &PL_sv_undef;

                mPUSHs(SvREFCNT_inc(item));
            }
        }

        XSRETURN(retcount);
    }
    else {
        EXTEND(SP, (SSize_t)len);

        for(i = 0; i < len; i++) {
            Size_t listi;
            AV *ret = newAV();
            av_extend(ret, nlists);

            for(listi = 0; listi < nlists; listi++) {
                SV *item = (i < av_count(lists[listi])) ?
                    AvARRAY(lists[listi])[i] :
                    &PL_sv_undef;

                av_push(ret, SvREFCNT_inc(item));
            }

            mPUSHs(newRV_noinc((SV *)ret));
        }

        XSRETURN(len);
    }

MODULE=List::Util       PACKAGE=Scalar::Util

void
dualvar(num,str)
    SV *num
    SV *str
PROTOTYPE: $$
CODE:
{
    dXSTARG;

    (void)SvUPGRADE(TARG, SVt_PVNV);

    sv_copypv(TARG,str);

    if(SvNOK(num) || SvPOK(num) || SvMAGICAL(num)) {
        SvNV_set(TARG, SvNV(num));
        SvNOK_on(TARG);
    }
#ifdef SVf_IVisUV
    else if(SvUOK(num)) {
        SvUV_set(TARG, SvUV(num));
        SvIOK_on(TARG);
        SvIsUV_on(TARG);
    }
#endif
    else {
        SvIV_set(TARG, SvIV(num));
        SvIOK_on(TARG);
    }

    if(PL_tainting && (SvTAINTED(num) || SvTAINTED(str)))
        SvTAINTED_on(TARG);

    ST(0) = TARG;
    XSRETURN(1);
}

void
isdual(sv)
    SV *sv
PROTOTYPE: $
CODE:
    if(SvMAGICAL(sv))
        mg_get(sv);

    ST(0) = boolSV((SvPOK(sv) || SvPOKp(sv)) && (SvNIOK(sv) || SvNIOKp(sv)));
    XSRETURN(1);

SV *
blessed(sv)
    SV *sv
PROTOTYPE: $
CODE:
{
    SvGETMAGIC(sv);

    if(!(SvROK(sv) && SvOBJECT(SvRV(sv))))
        XSRETURN_UNDEF;
#ifdef HAVE_UNICODE_PACKAGE_NAMES
    RETVAL = newSVsv(sv_ref(NULL, SvRV(sv), TRUE));
#else
    RETVAL = newSV(0);
    sv_setpv(RETVAL, sv_reftype(SvRV(sv), TRUE));
#endif
}
OUTPUT:
    RETVAL

char *
reftype(sv)
    SV *sv
PROTOTYPE: $
CODE:
{
    SvGETMAGIC(sv);
    if(!SvROK(sv))
        XSRETURN_UNDEF;

    RETVAL = (char*)sv_reftype(SvRV(sv),FALSE);
}
OUTPUT:
    RETVAL

UV
refaddr(sv)
    SV *sv
PROTOTYPE: $
CODE:
{
    SvGETMAGIC(sv);
    if(!SvROK(sv))
        XSRETURN_UNDEF;

    RETVAL = PTR2UV(SvRV(sv));
}
OUTPUT:
    RETVAL

void
weaken(sv)
    SV *sv
PROTOTYPE: $
CODE:
    sv_rvweaken(sv);

void
unweaken(sv)
    SV *sv
PROTOTYPE: $
INIT:
    SV *tsv;
CODE:
#if defined(sv_rvunweaken)
    PERL_UNUSED_VAR(tsv);
    sv_rvunweaken(sv);
#else
    /* This code stolen from core's sv_rvweaken() and modified */
    if (!SvOK(sv))
        return;
    if (!SvROK(sv))
        croak("Can't unweaken a nonreference");
    else if (!SvWEAKREF(sv)) {
        if(ckWARN(WARN_MISC))
            warn("Reference is not weak");
        return;
    }
    else if (SvREADONLY(sv)) croak_no_modify();

    tsv = SvRV(sv);
#if PERL_VERSION >= 14
    SvWEAKREF_off(sv); SvROK_on(sv);
    SvREFCNT_inc_NN(tsv);
    Perl_sv_del_backref(aTHX_ tsv, sv);
#else
    /* Lacking sv_del_backref() the best we can do is clear the old (weak) ref
     * then set a new strong one
     */
    sv_setsv(sv, &PL_sv_undef);
    SvRV_set(sv, SvREFCNT_inc_NN(tsv));
    SvROK_on(sv);
#endif
#endif

void
isweak(sv)
    SV *sv
PROTOTYPE: $
CODE:
    ST(0) = boolSV(SvROK(sv) && SvWEAKREF(sv));
    XSRETURN(1);

int
readonly(sv)
    SV *sv
PROTOTYPE: $
CODE:
    SvGETMAGIC(sv);
    RETVAL = SvREADONLY(sv);
OUTPUT:
    RETVAL

int
tainted(sv)
    SV *sv
PROTOTYPE: $
CODE:
    SvGETMAGIC(sv);
    RETVAL = SvTAINTED(sv);
OUTPUT:
    RETVAL

void
isvstring(sv)
    SV *sv
PROTOTYPE: $
CODE:
#ifdef SvVOK
    SvGETMAGIC(sv);
    ST(0) = boolSV(SvVOK(sv));
    XSRETURN(1);
#else
    croak("vstrings are not implemented in this release of perl");
#endif

SV *
looks_like_number(sv)
    SV *sv
PROTOTYPE: $
CODE:
    SV *tempsv;
    SvGETMAGIC(sv);
    if(SvAMAGIC(sv) && (tempsv = AMG_CALLun(sv, numer))) {
        sv = tempsv;
    }
#if !PERL_VERSION_GE(5,8,5)
    if(SvPOK(sv) || SvPOKp(sv)) {
        RETVAL = looks_like_number(sv) ? &PL_sv_yes : &PL_sv_no;
    }
    else {
        RETVAL = (SvFLAGS(sv) & (SVf_NOK|SVp_NOK|SVf_IOK|SVp_IOK)) ? &PL_sv_yes : &PL_sv_no;
    }
#else
    RETVAL = looks_like_number(sv) ? &PL_sv_yes : &PL_sv_no;
#endif
OUTPUT:
    RETVAL

void
openhandle(SV *sv)
PROTOTYPE: $
CODE:
{
    IO *io = NULL;
    SvGETMAGIC(sv);
    if(SvROK(sv)){
        /* deref first */
        sv = SvRV(sv);
    }

    /* must be GLOB or IO */
    if(isGV(sv)){
        io = GvIO((GV*)sv);
    }
    else if(SvTYPE(sv) == SVt_PVIO){
        io = (IO*)sv;
    }

    if(io){
        /* real or tied filehandle? */
        if(IoIFP(io) || SvTIED_mg((SV*)io, PERL_MAGIC_tiedscalar)){
            XSRETURN(1);
        }
    }
    XSRETURN_UNDEF;
}

MODULE=List::Util       PACKAGE=Sub::Util

void
set_prototype(proto, code)
    SV *proto
    SV *code
PREINIT:
    SV *cv; /* not CV * */
PPCODE:
    SvGETMAGIC(code);
    if(!SvROK(code))
        croak("set_prototype: not a reference");

    cv = SvRV(code);
    if(SvTYPE(cv) != SVt_PVCV)
        croak("set_prototype: not a subroutine reference");

    if(SvPOK(proto)) {
        /* set the prototype */
        sv_copypv(cv, proto);
    }
    else {
        /* delete the prototype */
        SvPOK_off(cv);
    }

    PUSHs(code);
    XSRETURN(1);

void
set_subname(name, sub)
    SV *name
    SV *sub
PREINIT:
    CV *cv = NULL;
    GV *gv;
    HV *stash = CopSTASH(PL_curcop);
    const char *s, *end = NULL, *begin = NULL;
    MAGIC *mg;
    STRLEN namelen;
    const char* nameptr = SvPV(name, namelen);
    int utf8flag = SvUTF8(name);
    int quotes_seen = 0;
    bool need_subst = FALSE;
PPCODE:
    if (!SvROK(sub) && SvGMAGICAL(sub))
        mg_get(sub);
    if (SvROK(sub))
        cv = (CV *) SvRV(sub);
    else if (SvTYPE(sub) == SVt_PVGV)
        cv = GvCVu(sub);
    else if (!SvOK(sub))
        croak(PL_no_usym, "a subroutine");
    else if (PL_op->op_private & HINT_STRICT_REFS)
        croak("Can't use string (\"%.32s\") as %s ref while \"strict refs\" in use",
              SvPV_nolen(sub), "a subroutine");
    else if ((gv = gv_fetchsv(sub, FALSE, SVt_PVCV)))
        cv = GvCVu(gv);
    if (!cv)
        croak("Undefined subroutine %s", SvPV_nolen(sub));
    if (SvTYPE(cv) != SVt_PVCV && SvTYPE(cv) != SVt_PVFM)
        croak("Not a subroutine reference");
    for (s = nameptr; s <= nameptr + namelen; s++) {
        if (s > nameptr && *s == ':' && s[-1] == ':') {
            end = s - 1;
            begin = ++s;
            if (quotes_seen)
                need_subst = TRUE;
        }
        else if (s > nameptr && *s != '\0' && s[-1] == '\'') {
            end = s - 1;
            begin = s;
            if (quotes_seen++)
                need_subst = TRUE;
        }
    }
    s--;
    if (end) {
        SV* tmp;
        if (need_subst) {
            STRLEN length = end - nameptr + quotes_seen - (*end == '\'' ? 1 : 0);
            char* left;
            int i, j;
            tmp = sv_2mortal(newSV(length));
            left = SvPVX(tmp);
            for (i = 0, j = 0; j < end - nameptr; ++i, ++j) {
                if (nameptr[j] == '\'') {
                    left[i] = ':';
                    left[++i] = ':';
                }
                else {
                    left[i] = nameptr[j];
                }
            }
            stash = gv_stashpvn(left, length, GV_ADD | utf8flag);
        }
        else
            stash = gv_stashpvn(nameptr, end - nameptr, GV_ADD | utf8flag);
        nameptr = begin;
        namelen -= begin - nameptr;
    }

    /* under debugger, provide information about sub location */
    if (PL_DBsub && CvGV(cv)) {
        HV* DBsub = GvHV(PL_DBsub);
        HE* old_data = NULL;

        GV* oldgv = CvGV(cv);
        HV* oldhv = GvSTASH(oldgv);

        if (oldhv) {
            SV* old_full_name = sv_2mortal(newSVpvn_flags(HvNAME(oldhv), HvNAMELEN_get(oldhv), HvNAMEUTF8(oldhv) ? SVf_UTF8 : 0));
            sv_catpvn(old_full_name, "::", 2);
            sv_catpvn_flags(old_full_name, GvNAME(oldgv), GvNAMELEN(oldgv), GvNAMEUTF8(oldgv) ? SV_CATUTF8 : SV_CATBYTES);

            old_data = hv_fetch_ent(DBsub, old_full_name, 0, 0);
        }

        if (old_data && HeVAL(old_data)) {
            SV* old_val = HeVAL(old_data);
            SV* new_full_name = sv_2mortal(newSVpvn_flags(HvNAME(stash), HvNAMELEN_get(stash), HvNAMEUTF8(stash) ? SVf_UTF8 : 0));
            sv_catpvn(new_full_name, "::", 2);
            sv_catpvn_flags(new_full_name, nameptr, s - nameptr, utf8flag ? SV_CATUTF8 : SV_CATBYTES);
            SvREFCNT_inc(old_val);
            if (!hv_store_ent(DBsub, new_full_name, old_val, 0))
                SvREFCNT_dec(old_val);
        }
    }

    gv = (GV *) newSV(0);
    gv_init_pvn(gv, stash, nameptr, s - nameptr, GV_ADDMULTI | utf8flag);

    /*
     * set_subname needs to create a GV to store the name. The CvGV field of a
     * CV is not refcounted, so perl wouldn't know to SvREFCNT_dec() this GV if
     * it destroys the containing CV. We use a MAGIC with an empty vtable
     * simply for the side-effect of using MGf_REFCOUNTED to store the
     * actually-counted reference to the GV.
     */
    mg = SvMAGIC(cv);
    while (mg && mg->mg_virtual != &subname_vtbl)
        mg = mg->mg_moremagic;
    if (!mg) {
        Newxz(mg, 1, MAGIC);
        mg->mg_moremagic = SvMAGIC(cv);
        mg->mg_type = PERL_MAGIC_ext;
        mg->mg_virtual = &subname_vtbl;
        SvMAGIC_set(cv, mg);
    }
    if (mg->mg_flags & MGf_REFCOUNTED)
        SvREFCNT_dec(mg->mg_obj);
    mg->mg_flags |= MGf_REFCOUNTED;
    mg->mg_obj = (SV *) gv;
    SvRMAGICAL_on(cv);
    CvANON_off(cv);
#ifndef CvGV_set
    CvGV(cv) = gv;
#else
    CvGV_set(cv, gv);
#endif
    PUSHs(sub);

void
subname(code)
    SV *code
PREINIT:
    CV *cv;
    GV *gv;
    const char *stashname;
PPCODE:
    if (!SvROK(code) && SvGMAGICAL(code))
        mg_get(code);

    if(!SvROK(code) || SvTYPE(cv = (CV *)SvRV(code)) != SVt_PVCV)
        croak("Not a subroutine reference");

    if(!(gv = CvGV(cv)))
        XSRETURN(0);

    if(GvSTASH(gv))
        stashname = HvNAME(GvSTASH(gv));
    else
        stashname = "__ANON__";

    mPUSHs(newSVpvf("%s::%s", stashname, GvNAME(gv)));
    XSRETURN(1);

BOOT:
{
    HV *lu_stash = gv_stashpvn("List::Util", 10, TRUE);
    GV *rmcgv = *(GV**)hv_fetch(lu_stash, "REAL_MULTICALL", 14, TRUE);
    SV *rmcsv;
#if !defined(SvVOK)
    HV *su_stash = gv_stashpvn("Scalar::Util", 12, TRUE);
    GV *vargv = *(GV**)hv_fetch(su_stash, "EXPORT_FAIL", 11, TRUE);
    AV *varav;
    if(SvTYPE(vargv) != SVt_PVGV)
        gv_init(vargv, su_stash, "Scalar::Util", 12, TRUE);
    varav = GvAVn(vargv);
#endif
    if(SvTYPE(rmcgv) != SVt_PVGV)
        gv_init(rmcgv, lu_stash, "List::Util", 10, TRUE);
    rmcsv = GvSVn(rmcgv);
#ifndef SvVOK
    av_push(varav, newSVpv("isvstring",9));
#endif
#ifdef REAL_MULTICALL
    sv_setsv(rmcsv, &PL_sv_yes);
#else
    sv_setsv(rmcsv, &PL_sv_no);
#endif
}
