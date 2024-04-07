/*    builtin.c
 *
 *    Copyright (C) 2021 by Paul Evans and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* This file contains the code that implements functions in perl's "builtin::"
 * namespace
 */

#include "EXTERN.h"
#include "perl.h"

#include "XSUB.h"

struct BuiltinFuncDescriptor {
    const char *name;
    XSUBADDR_t xsub;
    OP *(*checker)(pTHX_ OP *, GV *, SV *);
    IV ckval;
};

#define warn_experimental_builtin(name, prefix) S_warn_experimental_builtin(aTHX_ name, prefix)
static void S_warn_experimental_builtin(pTHX_ const char *name, bool prefix)
{
    /* diag_listed_as: Built-in function '%s' is experimental */
    Perl_ck_warner_d(aTHX_ packWARN(WARN_EXPERIMENTAL__BUILTIN),
                     "Built-in function '%s%s' is experimental",
                     prefix ? "builtin::" : "", name);
}

/* These three utilities might want to live elsewhere to be reused from other
 * code sometime
 */
#define prepare_export_lexical()  S_prepare_export_lexical(aTHX)
static void S_prepare_export_lexical(pTHX)
{
    assert(PL_compcv);

    /* We need to have PL_comppad / PL_curpad set correctly for lexical importing */
    ENTER;
    SAVESPTR(PL_comppad_name); PL_comppad_name = PadlistNAMES(CvPADLIST(PL_compcv));
    SAVESPTR(PL_comppad);      PL_comppad      = PadlistARRAY(CvPADLIST(PL_compcv))[1];
    SAVESPTR(PL_curpad);       PL_curpad       = PadARRAY(PL_comppad);
}

#define export_lexical(name, sv)  S_export_lexical(aTHX_ name, sv)
static void S_export_lexical(pTHX_ SV *name, SV *sv)
{
    PADOFFSET off = pad_add_name_sv(name, padadd_STATE, 0, 0);
    SvREFCNT_dec(PL_curpad[off]);
    PL_curpad[off] = SvREFCNT_inc(sv);
}

#define finish_export_lexical()  S_finish_export_lexical(aTHX)
static void S_finish_export_lexical(pTHX)
{
    intro_my();

    LEAVE;
}


XS(XS_builtin_true);
XS(XS_builtin_true)
{
    dXSARGS;
    warn_experimental_builtin("true", true);
    if(items)
        croak_xs_usage(cv, "");
    XSRETURN_YES;
}

XS(XS_builtin_false);
XS(XS_builtin_false)
{
    dXSARGS;
    warn_experimental_builtin("false", true);
    if(items)
        croak_xs_usage(cv, "");
    XSRETURN_NO;
}

enum {
    BUILTIN_CONST_FALSE,
    BUILTIN_CONST_TRUE,
};

static OP *ck_builtin_const(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    const struct BuiltinFuncDescriptor *builtin = NUM2PTR(const struct BuiltinFuncDescriptor *, SvUV(ckobj));

    warn_experimental_builtin(builtin->name, false);

    SV *prototype = newSVpvs("");
    SAVEFREESV(prototype);

    assert(entersubop->op_type == OP_ENTERSUB);

    entersubop = ck_entersub_args_proto(entersubop, namegv, prototype);

    SV *constval;
    switch(builtin->ckval) {
        case BUILTIN_CONST_FALSE: constval = &PL_sv_no; break;
        case BUILTIN_CONST_TRUE:  constval = &PL_sv_yes; break;
        default:
            DIE(aTHX_ "panic: unrecognised builtin_const value %" IVdf,
                      builtin->ckval);
            break;
    }

    op_free(entersubop);

    return newSVOP(OP_CONST, 0, constval);
}

XS(XS_builtin_func1_scalar);
XS(XS_builtin_func1_scalar)
{
    dXSARGS;
    dXSI32;

    warn_experimental_builtin(PL_op_name[ix], true);

    if(items != 1)
        croak_xs_usage(cv, "arg");

    switch(ix) {
        case OP_IS_BOOL:
            Perl_pp_is_bool(aTHX);
            break;

        case OP_IS_WEAK:
            Perl_pp_is_weak(aTHX);
            break;

        case OP_BLESSED:
            Perl_pp_blessed(aTHX);
            break;

        case OP_REFADDR:
            Perl_pp_refaddr(aTHX);
            break;

        case OP_REFTYPE:
            Perl_pp_reftype(aTHX);
            break;

        case OP_CEIL:
            Perl_pp_ceil(aTHX);
            break;

        case OP_FLOOR:
            Perl_pp_floor(aTHX);
            break;

        case OP_IS_TAINTED:
            Perl_pp_is_tainted(aTHX);
            break;

        default:
            Perl_die(aTHX_ "panic: unhandled opcode %" IVdf
                           " for xs_builtin_func1_scalar()", (IV) ix);
    }

    XSRETURN(1);
}

XS(XS_builtin_trim);
XS(XS_builtin_trim)
{
    dXSARGS;

    warn_experimental_builtin("trim", true);

    if (items != 1) {
        croak_xs_usage(cv, "arg");
    }

    dTARGET;
    SV *source = TOPs;
    STRLEN len;
    const U8 *start;
    SV *dest;

    SvGETMAGIC(source);

    if (SvOK(source))
        start = (const U8*)SvPV_nomg_const(source, len);
    else {
        if (ckWARN(WARN_UNINITIALIZED))
            report_uninit(source);
        start = (const U8*)"";
        len = 0;
    }

    if (DO_UTF8(source)) {
        const U8 *end = start + len;

        /* Find the first non-space */
        while(len) {
            STRLEN thislen;
            if (!isSPACE_utf8_safe(start, end))
                break;
            start += (thislen = UTF8SKIP(start));
            len -= thislen;
        }

        /* Find the final non-space */
        STRLEN thislen;
        const U8 *cur_end = end;
        while ((thislen = is_SPACE_utf8_safe_backwards(cur_end, start))) {
            cur_end -= thislen;
        }
        len -= (end - cur_end);
    }
    else if (len) {
        while(len) {
            if (!isSPACE_L1(*start))
                break;
            start++;
            len--;
        }

        while(len) {
            if (!isSPACE_L1(start[len-1]))
                break;
            len--;
        }
    }

    dest = TARG;

    if (SvPOK(dest) && (dest == source)) {
        sv_chop(dest, (const char *)start);
        SvCUR_set(dest, len);
    }
    else {
        SvUPGRADE(dest, SVt_PV);
        SvGROW(dest, len + 1);

        Copy(start, SvPVX(dest), len, U8);
        SvPVX(dest)[len] = '\0';
        SvPOK_on(dest);
        SvCUR_set(dest, len);

        if (DO_UTF8(source))
            SvUTF8_on(dest);
        else
            SvUTF8_off(dest);

        if (SvTAINTED(source))
            SvTAINT(dest);
    }

    SvSETMAGIC(dest);

    SETs(dest);

    XSRETURN(1);
}

XS(XS_builtin_export_lexically);
XS(XS_builtin_export_lexically)
{
    dXSARGS;

    warn_experimental_builtin("export_lexically", true);

    if(!PL_compcv)
        Perl_croak(aTHX_
                "export_lexically can only be called at compile time");

    if(items % 2)
        Perl_croak(aTHX_ "Odd number of elements in export_lexically");

    for(int i = 0; i < items; i += 2) {
        SV *name = ST(i);
        SV *ref  = ST(i+1);

        if(!SvROK(ref))
            /* diag_listed_as: Expected %s reference in export_lexically */
            Perl_croak(aTHX_ "Expected a reference in export_lexically");

        char sigil = SvPVX(name)[0];
        SV *rv = SvRV(ref);

        const char *bad = NULL;
        switch(sigil) {
            default:
                /* overwrites the pointer on the stack; but this is fine, the
                 * caller's value isn't modified */
                ST(i) = name = sv_2mortal(Perl_newSVpvf(aTHX_ "&%" SVf, SVfARG(name)));

                /* FALLTHROUGH */
            case '&':
                if(SvTYPE(rv) != SVt_PVCV)
                    bad = "a CODE";
                break;

            case '$':
                /* Permit any of SVt_NULL to SVt_PVMG. Technically this also
                 * includes SVt_INVLIST but it isn't thought possible for pureperl
                 * code to ever manage to see one of those. */
                if(SvTYPE(rv) > SVt_PVMG)
                    bad = "a SCALAR";
                break;

            case '@':
                if(SvTYPE(rv) != SVt_PVAV)
                    bad = "an ARRAY";
                break;

            case '%':
                if(SvTYPE(rv) != SVt_PVHV)
                    bad = "a HASH";
                break;
        }

        if(bad)
            Perl_croak(aTHX_ "Expected %s reference in export_lexically", bad);
    }

    prepare_export_lexical();

    for(int i = 0; i < items; i += 2) {
        SV *name = ST(i);
        SV *ref  = ST(i+1);

        export_lexical(name, SvRV(ref));
    }

    finish_export_lexical();
}

XS(XS_builtin_func1_void);
XS(XS_builtin_func1_void)
{
    dXSARGS;
    dXSI32;

    warn_experimental_builtin(PL_op_name[ix], true);

    if(items != 1)
        croak_xs_usage(cv, "arg");

    switch(ix) {
        case OP_WEAKEN:
            Perl_pp_weaken(aTHX);
            break;

        case OP_UNWEAKEN:
            Perl_pp_unweaken(aTHX);
            break;

        default:
            Perl_die(aTHX_ "panic: unhandled opcode %" IVdf
                           " for xs_builtin_func1_void()", (IV) ix);
    }

    XSRETURN(0);
}

XS(XS_builtin_created_as_string)
{
    dXSARGS;

    if(items != 1)
        croak_xs_usage(cv, "arg");

    SV *arg = ST(0);
    SvGETMAGIC(arg);

    /* SV was created as string if it has POK and isn't bool */
    ST(0) = boolSV(SvPOK(arg) && !SvIsBOOL(arg));
    XSRETURN(1);
}

XS(XS_builtin_created_as_number)
{
    dXSARGS;

    if(items != 1)
        croak_xs_usage(cv, "arg");

    SV *arg = ST(0);
    SvGETMAGIC(arg);

    /* SV was created as number if it has NOK or IOK but not POK and is not bool */
    ST(0) = boolSV(SvNIOK(arg) && !SvPOK(arg) && !SvIsBOOL(arg));
    XSRETURN(1);
}

static OP *ck_builtin_func1(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    const struct BuiltinFuncDescriptor *builtin = NUM2PTR(const struct BuiltinFuncDescriptor *, SvUV(ckobj));

    warn_experimental_builtin(builtin->name, false);

    SV *prototype = newSVpvs("$");
    SAVEFREESV(prototype);

    assert(entersubop->op_type == OP_ENTERSUB);

    entersubop = ck_entersub_args_proto(entersubop, namegv, prototype);

    OPCODE opcode = builtin->ckval;
    if(!opcode)
        return entersubop;

    OP *parent = entersubop, *pushop, *argop;

    pushop = cUNOPx(entersubop)->op_first;
    if (!OpHAS_SIBLING(pushop)) {
        pushop = cUNOPx(pushop)->op_first;
    }

    argop = OpSIBLING(pushop);

    if (!argop || !OpHAS_SIBLING(argop) || OpHAS_SIBLING(OpSIBLING(argop)))
        return entersubop;

    (void)op_sibling_splice(parent, pushop, 1, NULL);

    U8 wantflags = entersubop->op_flags & OPf_WANT;

    op_free(entersubop);

    return newUNOP(opcode, wantflags, argop);
}

XS(XS_builtin_indexed)
{
    dXSARGS;

    switch(GIMME_V) {
        case G_VOID:
            Perl_ck_warner(aTHX_ packWARN(WARN_VOID),
                "Useless use of %s in void context", "builtin::indexed");
            XSRETURN(0);

        case G_SCALAR:
            Perl_ck_warner(aTHX_ packWARN(WARN_SCALAR),
                "Useless use of %s in scalar context", "builtin::indexed");
            ST(0) = sv_2mortal(newSViv(items * 2));
            XSRETURN(1);

        case G_LIST:
            break;
    }

    SSize_t retcount = items * 2;
    EXTEND(SP, retcount);

    /* Copy from [items-1] down to [0] so we don't have to make
     * temporary copies */
    for(SSize_t index = items - 1; index >= 0; index--) {
        /* Copy, not alias */
        ST(index * 2 + 1) = sv_mortalcopy(ST(index));
        ST(index * 2)     = sv_2mortal(newSViv(index));
    }

    XSRETURN(retcount);
}

static OP *ck_builtin_funcN(pTHX_ OP *entersubop, GV *namegv, SV *ckobj)
{
    const struct BuiltinFuncDescriptor *builtin = NUM2PTR(const struct BuiltinFuncDescriptor *, SvUV(ckobj));

    warn_experimental_builtin(builtin->name, false);

    SV *prototype = newSVpvs("@");
    SAVEFREESV(prototype);

    assert(entersubop->op_type == OP_ENTERSUB);

    entersubop = ck_entersub_args_proto(entersubop, namegv, prototype);
    return entersubop;
}

static const char builtin_not_recognised[] = "'%" SVf "' is not recognised as a builtin function";

static const struct BuiltinFuncDescriptor builtins[] = {
    /* constants */
    { "builtin::true",   &XS_builtin_true,   &ck_builtin_const, BUILTIN_CONST_TRUE  },
    { "builtin::false",  &XS_builtin_false,  &ck_builtin_const, BUILTIN_CONST_FALSE },

    /* unary functions */
    { "builtin::is_bool",    &XS_builtin_func1_scalar, &ck_builtin_func1, OP_IS_BOOL    },
    { "builtin::weaken",     &XS_builtin_func1_void,   &ck_builtin_func1, OP_WEAKEN     },
    { "builtin::unweaken",   &XS_builtin_func1_void,   &ck_builtin_func1, OP_UNWEAKEN   },
    { "builtin::is_weak",    &XS_builtin_func1_scalar, &ck_builtin_func1, OP_IS_WEAK    },
    { "builtin::blessed",    &XS_builtin_func1_scalar, &ck_builtin_func1, OP_BLESSED    },
    { "builtin::refaddr",    &XS_builtin_func1_scalar, &ck_builtin_func1, OP_REFADDR    },
    { "builtin::reftype",    &XS_builtin_func1_scalar, &ck_builtin_func1, OP_REFTYPE    },
    { "builtin::ceil",       &XS_builtin_func1_scalar, &ck_builtin_func1, OP_CEIL       },
    { "builtin::floor",      &XS_builtin_func1_scalar, &ck_builtin_func1, OP_FLOOR      },
    { "builtin::is_tainted", &XS_builtin_func1_scalar, &ck_builtin_func1, OP_IS_TAINTED },
    { "builtin::trim",       &XS_builtin_trim,         &ck_builtin_func1, 0 },

    { "builtin::created_as_string", &XS_builtin_created_as_string, &ck_builtin_func1, 0 },
    { "builtin::created_as_number", &XS_builtin_created_as_number, &ck_builtin_func1, 0 },

    /* list functions */
    { "builtin::indexed", &XS_builtin_indexed, &ck_builtin_funcN, 0 },
    { "builtin::export_lexically", &XS_builtin_export_lexically, NULL, 0 },
    { 0 }
};

XS(XS_builtin_import);
XS(XS_builtin_import)
{
    dXSARGS;

    if(!PL_compcv)
        Perl_croak(aTHX_
                "builtin::import can only be called at compile time");

    prepare_export_lexical();

    for(int i = 1; i < items; i++) {
        SV *sym = ST(i);
        if(strEQ(SvPV_nolen(sym), "import"))
            Perl_croak(aTHX_ builtin_not_recognised, sym);

        SV *ampname = sv_2mortal(Perl_newSVpvf(aTHX_ "&%" SVf, SVfARG(sym)));
        SV *fqname = sv_2mortal(Perl_newSVpvf(aTHX_ "builtin::%" SVf, SVfARG(sym)));

        CV *cv = get_cv(SvPV_nolen(fqname), SvUTF8(fqname) ? SVf_UTF8 : 0);
        if(!cv)
            Perl_croak(aTHX_ builtin_not_recognised, sym);

        export_lexical(ampname, (SV *)cv);
    }

    finish_export_lexical();
}

void
Perl_boot_core_builtin(pTHX)
{
    I32 i;
    for(i = 0; builtins[i].name; i++) {
        const struct BuiltinFuncDescriptor *builtin = &builtins[i];

        const char *proto = NULL;
        if(builtin->checker == &ck_builtin_const)
            proto = "";
        else if(builtin->checker == &ck_builtin_func1)
            proto = "$";

        CV *cv = newXS_flags(builtin->name, builtin->xsub, __FILE__, proto, 0);
        XSANY.any_i32 = builtin->ckval;

        if(builtin->checker) {
            cv_set_call_checker_flags(cv, builtin->checker, newSVuv(PTR2UV(builtin)), 0);
        }
    }

    newXS_flags("builtin::import", &XS_builtin_import, __FILE__, NULL, 0);
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
