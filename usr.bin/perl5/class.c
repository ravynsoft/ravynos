/*    class.c
 *
 *    Copyright (C) 2022 by Paul Evans and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* This file contains the code that implements perl's new `use feature 'class'`
 * object model
 */

#include "EXTERN.h"
#define PERL_IN_CLASS_C
#include "perl.h"

#include "XSUB.h"

enum {
    PADIX_SELF   = 1,
    PADIX_PARAMS = 2,
};

void
Perl_croak_kw_unless_class(pTHX_ const char *kw)
{
    PERL_ARGS_ASSERT_CROAK_KW_UNLESS_CLASS;

    if(!HvSTASH_IS_CLASS(PL_curstash))
        croak("Cannot '%s' outside of a 'class'", kw);
}

#define newSVobject(fieldcount)  Perl_newSVobject(aTHX_ fieldcount)
SV *
Perl_newSVobject(pTHX_ Size_t fieldcount)
{
    SV *sv = newSV_type(SVt_PVOBJ);

    Newx(ObjectFIELDS(sv), fieldcount, SV *);
    ObjectMAXFIELD(sv) = fieldcount - 1;

    Zero(ObjectFIELDS(sv), fieldcount, SV *);

    return sv;
}

PP(pp_initfield)
{
    dSP;
    UNOP_AUX_item *aux = cUNOP_AUX->op_aux;

    SV *self = PAD_SVl(PADIX_SELF);
    assert(SvTYPE(SvRV(self)) == SVt_PVOBJ);
    SV *instance = SvRV(self);

    SV **fields = ObjectFIELDS(instance);

    PADOFFSET fieldix = aux[0].uv;

    SV *val = NULL;

    switch(PL_op->op_private & (OPpINITFIELD_AV|OPpINITFIELD_HV)) {
        case 0:
            if(PL_op->op_flags & OPf_STACKED)
                val = newSVsv(POPs);
            else
                val = newSV(0);
            break;

        case OPpINITFIELD_AV:
        {
            AV *av;
            if(PL_op->op_flags & OPf_STACKED) {
                SV **svp = PL_stack_base + POPMARK + 1;
                STRLEN count = SP - svp + 1;

                av = newAV_alloc_x(count);

                av_extend(av, count);
                while(svp <= SP) {
                    av_push_simple(av, newSVsv(*svp));
                    svp++;
                }
            }
            else
                av = newAV();
            val = (SV *)av;
            break;
        }

        case OPpINITFIELD_HV:
        {
            HV *hv = newHV();
            if(PL_op->op_flags & OPf_STACKED) {
                SV **svp = PL_stack_base + POPMARK + 1;
                STRLEN svcount = SP - svp + 1;

                if(svcount % 2)
                    Perl_warner(aTHX_
                            packWARN(WARN_MISC), "Odd number of elements in hash field initialization");

                while(svp <= SP) {
                    SV *key = *svp; svp++;
                    SV *val = svp <= SP ? *svp : &PL_sv_undef; svp++;

                    (void)hv_store_ent(hv, key, newSVsv(val), 0);
                }
            }
            val = (SV *)hv;
            break;
        }
    }

    fields[fieldix] = val;

    PADOFFSET padix = PL_op->op_targ;
    if(padix) {
        SAVESPTR(PAD_SVl(padix));
        SV *sv = PAD_SVl(padix) = SvREFCNT_inc(val);
        save_freesv(sv);
    }

    RETURN;
}

XS(injected_constructor);
XS(injected_constructor)
{
    dXSARGS;

    HV *stash = (HV *)XSANY.any_sv;
    assert(HvSTASH_IS_CLASS(stash));

    struct xpvhv_aux *aux = HvAUX(stash);

    if((items - 1) % 2)
        Perl_warn(aTHX_ "Odd number of arguments passed to %" HvNAMEf_QUOTEDPREFIX " constructor",
                HvNAMEfARG(stash));

    HV *params = NULL;
    {
        /* Set up params HV */
        params = newHV();
        SAVEFREESV((SV *)params);

        for(I32 i = 1; i < items; i += 2) {
            SV *name = ST(i);
            SV *val  = (i+1 < items) ? ST(i+1) : &PL_sv_undef;

            /* TODO: think about sanity-checking name for being 
             *   defined
             *   not ref (but overloaded objects?? boo)
             *   not duplicate
             * But then,  %params = @_;  wouldn't do that
             */

            (void)hv_store_ent(params, name, SvREFCNT_inc(val), 0);
        }
    }

    SV *instance = newSVobject(aux->xhv_class_next_fieldix);
    SvOBJECT_on(instance);
    SvSTASH_set(instance, MUTABLE_HV(SvREFCNT_inc_simple(stash)));

    SV *self = sv_2mortal(newRV_noinc(instance));

    assert(aux->xhv_class_initfields_cv);
    {
        ENTER;
        SAVETMPS;

        EXTEND(SP, 2);
        PUSHMARK(SP);
        PUSHs(self);
        if(params)
            PUSHs((SV *)params); // yes a raw HV
        else
            PUSHs(&PL_sv_undef);
        PUTBACK;

        call_sv((SV *)aux->xhv_class_initfields_cv, G_VOID);

        SPAGAIN;

        FREETMPS;
        LEAVE;
    }

    if(aux->xhv_class_adjust_blocks) {
        CV **cvp = (CV **)AvARRAY(aux->xhv_class_adjust_blocks);
        U32 nblocks = av_count(aux->xhv_class_adjust_blocks);

        for(U32 i = 0; i < nblocks; i++) {
            ENTER;
            SAVETMPS;
            SPAGAIN;

            EXTEND(SP, 2);

            PUSHMARK(SP);
            PUSHs(self);  /* I don't believe this needs to be an sv_mortalcopy() */
            PUTBACK;

            call_sv((SV *)cvp[i], G_VOID);

            SPAGAIN;

            FREETMPS;
            LEAVE;
        }
    }

    if(params && hv_iterinit(params) > 0) {
        /* TODO: consider sorting these into a canonical order, but that's awkward */
        HE *he = hv_iternext(params);

        SV *paramnames = newSVsv(HeSVKEY_force(he));
        SAVEFREESV(paramnames);

        while((he = hv_iternext(params)))
            Perl_sv_catpvf(aTHX_ paramnames, ", %" SVf, SVfARG(HeSVKEY_force(he)));

        croak("Unrecognised parameters for %" HvNAMEf_QUOTEDPREFIX " constructor: %" SVf,
                HvNAMEfARG(stash), SVfARG(paramnames));
    }

    EXTEND(SP, 1);
    ST(0) = self;
    XSRETURN(1);
}

/* OP_METHSTART is an UNOP_AUX whose AUX list contains
 *   [0].uv = count of fieldbinding pairs
 *   [1].uv = maximum fieldidx found in the binding list
 *   [...] = pairs of (padix, fieldix) to bind in .uv fields
 */

/* TODO: People would probably expect to find this in pp.c  ;) */
PP(pp_methstart)
{
    SV *self = av_shift(GvAV(PL_defgv));
    SV *rv = NULL;

    /* pp_methstart happens before the first OP_NEXTSTATE of the method body,
     * meaning PL_curcop still points at the callsite. This is useful for
     * croak() messages. However, it means we have to find our current stash
     * via a different technique.
     */
    CV *curcv;
    if(LIKELY(CxTYPE(CX_CUR()) == CXt_SUB))
        curcv = CX_CUR()->blk_sub.cv;
    else
        curcv = find_runcv(NULL);

    if(!SvROK(self) ||
        !SvOBJECT((rv = SvRV(self))) ||
        SvTYPE(rv) != SVt_PVOBJ) {
        HEK *namehek = CvGvNAME_HEK(curcv);
        croak(
            namehek ? "Cannot invoke method %" HEKf_QUOTEDPREFIX " on a non-instance" :
                      "Cannot invoke method on a non-instance",
            namehek);
    }

    if(CvSTASH(curcv) != SvSTASH(rv) &&
        !sv_derived_from_hv(self, CvSTASH(curcv)))
        croak("Cannot invoke a method of %" HvNAMEf_QUOTEDPREFIX " on an instance of %" HvNAMEf_QUOTEDPREFIX,
            HvNAMEfARG(CvSTASH(curcv)), HvNAMEfARG(SvSTASH(rv)));

    save_clearsv(&PAD_SVl(PADIX_SELF));
    sv_setsv(PAD_SVl(PADIX_SELF), self);

    UNOP_AUX_item *aux = cUNOP_AUX->op_aux;
    if(aux) {
        assert(SvTYPE(SvRV(self)) == SVt_PVOBJ);
        SV *instance = SvRV(self);
        SV **fieldp = ObjectFIELDS(instance);

        U32 fieldcount = (aux++)->uv;
        U32 max_fieldix = (aux++)->uv;

        assert((U32)(ObjectMAXFIELD(instance)+1) > max_fieldix);
        PERL_UNUSED_VAR(max_fieldix);

        for(Size_t i = 0; i < fieldcount; i++) {
            PADOFFSET padix   = (aux++)->uv;
            U32       fieldix = (aux++)->uv;

            assert(fieldp[fieldix]);

            /* TODO: There isn't a convenient SAVE macro for doing both these
             * steps in one go. Add one. */
            SAVESPTR(PAD_SVl(padix));
            SV *sv = PAD_SVl(padix) = SvREFCNT_inc(fieldp[fieldix]);
            save_freesv(sv);
        }
    }

    if(PL_op->op_private & OPpINITFIELDS) {
        SV *params = *av_fetch(GvAV(PL_defgv), 0, 0);
        if(params && SvTYPE(params) == SVt_PVHV) {
            SAVESPTR(PAD_SVl(PADIX_PARAMS));
            PAD_SVl(PADIX_PARAMS) = SvREFCNT_inc(params);
            save_freesv(params);
        }
    }

    return NORMAL;
}

static void
invoke_class_seal(pTHX_ void *_arg)
{
    class_seal_stash((HV *)_arg);
}

void
Perl_class_setup_stash(pTHX_ HV *stash)
{
    PERL_ARGS_ASSERT_CLASS_SETUP_STASH;

    assert(HvHasAUX(stash));

    if(HvSTASH_IS_CLASS(stash)) {
        croak("Cannot reopen existing class %" HvNAMEf_QUOTEDPREFIX,
            HvNAMEfARG(stash));
    }

    {
        SV *isaname = newSVpvf("%" HEKf "::ISA", HvNAME_HEK(stash));
        sv_2mortal(isaname);

        AV *isa = get_av(SvPV_nolen(isaname), (SvFLAGS(isaname) & SVf_UTF8));

        if(isa && av_count(isa) > 0)
            croak("Cannot create class %" HEKf " as it already has a non-empty @ISA",
                HvNAME_HEK(stash));
    }

    char *classname = HvNAME(stash);
    U32 nameflags = HvNAMEUTF8(stash) ? SVf_UTF8 : 0;

    /* TODO:
     *   Set some kind of flag on the stash to point out it's a class
     *   Allocate storage for all the extra things a class needs
     *     See https://github.com/leonerd/perl5/discussions/1
     */

    /* Inject the constructor */
    {
        SV *newname = Perl_newSVpvf(aTHX_ "%s::new", classname);
        SAVEFREESV(newname);

        CV *newcv = newXS_flags(SvPV_nolen(newname), injected_constructor, __FILE__, NULL, nameflags);
        CvXSUBANY(newcv).any_sv = (SV *)stash;
        CvREFCOUNTED_ANYSV_on(newcv);
    }

    /* TODO:
     *   DOES method
     */

    struct xpvhv_aux *aux = HvAUX(stash);
    aux->xhv_class_superclass    = NULL;
    aux->xhv_class_initfields_cv = NULL;
    aux->xhv_class_adjust_blocks = NULL;
    aux->xhv_class_fields        = NULL;
    aux->xhv_class_next_fieldix  = 0;
    aux->xhv_class_param_map     = NULL;

    aux->xhv_aux_flags |= HvAUXf_IS_CLASS;

    SAVEDESTRUCTOR_X(invoke_class_seal, stash);

    /* Prepare a suspended compcv for parsing field init expressions */
    {
        I32 floor_ix = start_subparse(FALSE, 0);

        CvIsMETHOD_on(PL_compcv);

        /* We don't want to make `$self` visible during the expression but we
         * still need to give it a name. Make it unusable from pure perl
         */
        PADOFFSET padix = pad_add_name_pvs("$(self)", 0, NULL, NULL);
        assert(padix == PADIX_SELF);

        padix = pad_add_name_pvs("%(params)", 0, NULL, NULL);
        assert(padix == PADIX_PARAMS);

        PERL_UNUSED_VAR(padix);

        Newx(aux->xhv_class_suspended_initfields_compcv, 1, struct suspended_compcv);
        suspend_compcv(aux->xhv_class_suspended_initfields_compcv);

        LEAVE_SCOPE(floor_ix);
    }
}

#define split_package_ver(value, pkgname, pkgversion)  S_split_package_ver(aTHX_ value, pkgname, pkgversion)
static const char *S_split_package_ver(pTHX_ SV *value, SV *pkgname, SV *pkgversion)
{
    const char *start = SvPVX(value),
               *p     = start,
               *end   = start + SvCUR(value);

    while(*p && !isSPACE_utf8_safe(p, end))
        p += UTF8SKIP(p);

    sv_setpvn(pkgname, start, p - start);
    if(SvUTF8(value))
        SvUTF8_on(pkgname);

    while(*p && isSPACE_utf8_safe(p, end))
        p += UTF8SKIP(p);

    if(*p) {
        /* scan_version() gets upset about trailing content. We need to extract
         * exactly what it wants
         */
        start = p;
        if(*p == 'v')
            p++;
        while(*p && strchr("0123456789._", *p))
            p++;
        SV *tmpsv = newSVpvn(start, p - start);
        SAVEFREESV(tmpsv);

        scan_version(SvPVX(tmpsv), pkgversion, FALSE);
    }

    while(*p && isSPACE_utf8_safe(p, end))
        p += UTF8SKIP(p);

    return p;
}

#define ensure_module_version(module, version)  S_ensure_module_version(aTHX_ module, version)
static void S_ensure_module_version(pTHX_ SV *module, SV *version)
{
    dSP;

    ENTER;

    PUSHMARK(SP);
    PUSHs(module);
    PUSHs(version);
    PUTBACK;

    call_method("VERSION", G_VOID);

    LEAVE;
}

#define split_attr_nameval(sv, namp, valp)  S_split_attr_nameval(aTHX_ sv, namp, valp)
static void S_split_attr_nameval(pTHX_ SV *sv, SV **namp, SV **valp)
{
    STRLEN svlen = SvCUR(sv);
    bool do_utf8 = SvUTF8(sv);

    const char *paren_at = (const char *)memchr(SvPVX(sv), '(', svlen);
    if(paren_at) {
        STRLEN namelen = paren_at - SvPVX(sv);

        if(SvPVX(sv)[svlen-1] != ')')
            /* Should be impossible to reach this by parsing regular perl code
             * by as class_apply_attributes() is XS-visible API it might still
             * be reachable. As it's likely unreachable by normal perl code,
             * don't bother listing it in perldiag.
             */
            /* diag_listed_as: SKIPME */
            croak("Malformed attribute string");
        *namp = sv_2mortal(newSVpvn_utf8(SvPVX(sv), namelen, do_utf8));

        const char *value_at = paren_at + 1;
        const char *value_max = SvPVX(sv) + svlen - 2;

        /* TODO: We're only obeying ASCII whitespace here */

        /* Trim whitespace at the start */
        while(value_at < value_max && isSPACE(*value_at))
            value_at += 1;
        while(value_max > value_at && isSPACE(*value_max))
            value_max -= 1;

        if(value_max >= value_at)
            *valp = sv_2mortal(newSVpvn_utf8(value_at, value_max - value_at + 1, do_utf8));
    }
    else {
        *namp = sv;
        *valp = NULL;
    }
}

static void
apply_class_attribute_isa(pTHX_ HV *stash, SV *value)
{
    assert(HvSTASH_IS_CLASS(stash));
    struct xpvhv_aux *aux = HvAUX(stash);

    /* Parse `value` into name + version */
    SV *superclassname = sv_newmortal(), *superclassver = sv_newmortal();
    const char *end = split_package_ver(value, superclassname, superclassver);
    if(*end)
        croak("Unexpected characters while parsing class :isa attribute: %s", end);

    if(aux->xhv_class_superclass)
        croak("Class already has a superclass, cannot add another");

    HV *superstash = gv_stashsv(superclassname, 0);
    if(!superstash) {
        /* Try to `require` the module then attempt a second time */
        load_module(PERL_LOADMOD_NOIMPORT, newSVsv(superclassname), NULL, NULL);
        superstash = gv_stashsv(superclassname, 0);
    }
    if(!superstash || !HvSTASH_IS_CLASS(superstash))
        /* TODO: This would be a useful feature addition */
        croak("Class :isa attribute requires a class but %" HvNAMEf_QUOTEDPREFIX " is not one",
            HvNAMEfARG(superstash));

    if(superclassver && SvOK(superclassver))
        ensure_module_version(superclassname, superclassver);

    /* TODO: Suuuurely there's a way to fetch this neatly with stash + "ISA"
     * You'd think that GvAV() of hv_fetchs() would do it, but no, because it
     * won't lazily create a proper (magical) GV if one didn't already exist.
     */
    {
        SV *isaname = newSVpvf("%" HEKf "::ISA", HvNAME_HEK(stash));
        sv_2mortal(isaname);

        AV *isa = get_av(SvPV_nolen(isaname), GV_ADD | (SvFLAGS(isaname) & SVf_UTF8));

        ENTER;

        /* Temporarily remove the SVf_READONLY flag */
        SAVESETSVFLAGS((SV *)isa, SVf_READONLY|SVf_PROTECT, SVf_READONLY|SVf_PROTECT);
        SvREADONLY_off((SV *)isa);

        av_push(isa, newSVsv(value));

        LEAVE;
    }

    aux->xhv_class_superclass = (HV *)SvREFCNT_inc(superstash);

    struct xpvhv_aux *superaux = HvAUX(superstash);

    aux->xhv_class_next_fieldix = superaux->xhv_class_next_fieldix;

    if(superaux->xhv_class_adjust_blocks) {
        if(!aux->xhv_class_adjust_blocks)
            aux->xhv_class_adjust_blocks = newAV();

        for(SSize_t i = 0; i <= AvFILL(superaux->xhv_class_adjust_blocks); i++)
            av_push(aux->xhv_class_adjust_blocks, AvARRAY(superaux->xhv_class_adjust_blocks)[i]);
    }

    if(superaux->xhv_class_param_map) {
        aux->xhv_class_param_map = newHVhv(superaux->xhv_class_param_map);
    }
}

static struct {
    const char *name;
    bool requires_value;
    void (*apply)(pTHX_ HV *stash, SV *value);
} const class_attributes[] = {
    { .name           = "isa",
      .requires_value = true,
      .apply          = &apply_class_attribute_isa,
    },
    {0}
};

static void
S_class_apply_attribute(pTHX_ HV *stash, OP *attr)
{
    assert(attr->op_type == OP_CONST);

    SV *name, *value;
    split_attr_nameval(cSVOPx_sv(attr), &name, &value);

    for(int i = 0; class_attributes[i].name; i++) {
        /* TODO: These attribute names are not UTF-8 aware */
        if(!strEQ(SvPVX(name), class_attributes[i].name))
            continue;

        if(class_attributes[i].requires_value && !(value && SvOK(value)))
            croak("Class attribute %" SVf " requires a value", SVfARG(name));

        (*class_attributes[i].apply)(aTHX_ stash, value);
        return;
    }

    croak("Unrecognized class attribute %" SVf, SVfARG(name));
}

void
Perl_class_apply_attributes(pTHX_ HV *stash, OP *attrlist)
{
    PERL_ARGS_ASSERT_CLASS_APPLY_ATTRIBUTES;

    if(!attrlist)
        return;
    if(attrlist->op_type == OP_NULL) {
        op_free(attrlist);
        return;
    }

    if(attrlist->op_type == OP_LIST) {
        OP *o = cLISTOPx(attrlist)->op_first;
        assert(o->op_type == OP_PUSHMARK);
        o = OpSIBLING(o);

        for(; o; o = OpSIBLING(o))
            S_class_apply_attribute(aTHX_ stash, o);
    }
    else
        S_class_apply_attribute(aTHX_ stash, attrlist);

    op_free(attrlist);
}

static OP *
S_newCROAKOP(pTHX_ SV *message)
{
    OP *o = newLISTOP(OP_LIST, 0,
            newOP(OP_PUSHMARK, 0),
            newSVOP(OP_CONST, 0, message));
    return op_convert_list(OP_DIE, 0, o);
}
#define newCROAKOP(message)  S_newCROAKOP(aTHX_ message)

void
Perl_class_seal_stash(pTHX_ HV *stash)
{
    PERL_ARGS_ASSERT_CLASS_SEAL_STASH;

    assert(HvSTASH_IS_CLASS(stash));
    struct xpvhv_aux *aux = HvAUX(stash);

    /* generate initfields CV */
    {
        I32 floor_ix = PL_savestack_ix;
        SAVEI32(PL_subline);
        save_item(PL_subname);

        resume_compcv_final(aux->xhv_class_suspended_initfields_compcv);

        /* Some OP_INITFIELD ops will need to populate the pad with their
         * result because later ops will rely on it. There's no need to do
         * this for every op though. Store a mapping to work out which ones
         * we'll need.
         */
        PADNAMELIST *pnl = PadlistNAMES(CvPADLIST(PL_compcv));
        HV *fieldix_to_padix = newHV();
        SAVEFREESV((SV *)fieldix_to_padix);

        /* padix 0 == @_; padix 1 == $self. Start at 2 */
        for(PADOFFSET padix = 2; padix <= PadnamelistMAX(pnl); padix++) {
            PADNAME *pn = PadnamelistARRAY(pnl)[padix];
            if(!pn || !PadnameIsFIELD(pn))
                continue;

            U32 fieldix = PadnameFIELDINFO(pn)->fieldix;
            (void)hv_store_ent(fieldix_to_padix, sv_2mortal(newSVuv(fieldix)), newSVuv(padix), 0);
        }

        OP *ops = NULL;

        ops = op_append_list(OP_LINESEQ, ops,
                newUNOP_AUX(OP_METHSTART, OPpINITFIELDS << 8, NULL, NULL));

        if(aux->xhv_class_superclass) {
            HV *superstash = aux->xhv_class_superclass;
            assert(HvSTASH_IS_CLASS(superstash));
            struct xpvhv_aux *superaux = HvAUX(superstash);

            /* Build an OP_ENTERSUB */
            OP *o = NULL;
            o = op_append_list(OP_LIST, o,
                newPADxVOP(OP_PADSV, 0, PADIX_SELF));
            o = op_append_list(OP_LIST, o,
                newPADxVOP(OP_PADHV, OPf_REF, PADIX_PARAMS));
            /* TODO: This won't work at all well under `use threads` because
             * it embeds the CV * to the superclass initfields CV right into
             * the optree. Maybe we'll have to pop it in the pad or something
             */
            o = op_append_list(OP_LIST, o,
                newSVOP(OP_CONST, 0, (SV *)superaux->xhv_class_initfields_cv));

            ops = op_append_list(OP_LINESEQ, ops,
                op_convert_list(OP_ENTERSUB, OPf_WANT_VOID|OPf_STACKED, o));
        }

        PADNAMELIST *fieldnames = aux->xhv_class_fields;

        for(SSize_t i = 0; fieldnames && i <= PadnamelistMAX(fieldnames); i++) {
            PADNAME *pn = PadnamelistARRAY(fieldnames)[i];
            char sigil = PadnamePV(pn)[0];
            PADOFFSET fieldix = PadnameFIELDINFO(pn)->fieldix;

            /* Extract the OP_{NEXT,DB}STATE op from the defop so we can
             * splice it in
             */
            OP *valop = PadnameFIELDINFO(pn)->defop;
            if(valop && valop->op_type == OP_LINESEQ) {
                OP *o = cLISTOPx(valop)->op_first;
                cLISTOPx(valop)->op_first = NULL;
                cLISTOPx(valop)->op_last = NULL;
                /* have to clear the OPf_KIDS flag or op_free() will get upset */
                valop->op_flags &= ~OPf_KIDS;
                op_free(valop);
                assert(valop->op_type == OP_FREED);

                OP *fieldcop = o;
                assert(fieldcop->op_type == OP_NEXTSTATE || fieldcop->op_type == OP_DBSTATE);
                o = OpSIBLING(o);
                OpLASTSIB_set(fieldcop, NULL);

                valop = o;
                OpLASTSIB_set(valop, NULL);

                ops = op_append_list(OP_LINESEQ, ops, fieldcop);
            }

            SV *paramname = PadnameFIELDINFO(pn)->paramname;

            U8 op_priv = 0;
            switch(sigil) {
                case '$':
                    if(paramname) {
                        if(!valop)
                            valop = newCROAKOP(
                                newSVpvf("Required parameter '%" SVf "' is missing for %" HvNAMEf_QUOTEDPREFIX " constructor",
                                    SVfARG(paramname), HvNAMEfARG(stash))
                            );

                        OP *helemop =
                            newBINOP(OP_HELEM, 0,
                                newPADxVOP(OP_PADHV, OPf_REF, PADIX_PARAMS),
                                newSVOP(OP_CONST, 0, SvREFCNT_inc(paramname)));

                        if(PadnameFIELDINFO(pn)->def_if_undef) {
                            /* delete $params{$paramname} // DEFOP */
                            valop = newLOGOP(OP_DOR, 0,
                                    newUNOP(OP_DELETE, 0, helemop), valop);
                        }
                        else if(PadnameFIELDINFO(pn)->def_if_false) {
                            /* delete $params{$paramname} || DEFOP */
                            valop = newLOGOP(OP_OR, 0,
                                newUNOP(OP_DELETE, 0, helemop), valop);
                        }
                        else {
                            /* exists $params{$paramname} ? delete $params{$paramname} : DEFOP */
                            /* more efficient with the new OP_HELEMEXISTSOR */
                            valop = newLOGOP(OP_HELEMEXISTSOR, OPpHELEMEXISTSOR_DELETE << 8,
                                helemop, valop);
                        }

                        valop = op_contextualize(valop, G_SCALAR);
                    }
                    break;

                case '@':
                    op_priv = OPpINITFIELD_AV;
                    break;

                case '%':
                    op_priv = OPpINITFIELD_HV;
                    break;

                default:
                    NOT_REACHED;
            }

            UNOP_AUX_item *aux;
            Newx(aux, 2, UNOP_AUX_item);

            aux[0].uv = fieldix;

            OP *fieldop = newUNOP_AUX(OP_INITFIELD, valop ? OPf_STACKED : 0, valop, aux);
            fieldop->op_private = op_priv;

            HE *he;
            if((he = hv_fetch_ent(fieldix_to_padix, sv_2mortal(newSVuv(fieldix)), 0, 0)) &&
                SvOK(HeVAL(he))) {
                fieldop->op_targ = SvUV(HeVAL(he));
            }

            ops = op_append_list(OP_LINESEQ, ops, fieldop);
        }

        /* initfields CV should not get class_wrap_method_body() called on its
         * body. pretend it isn't a method for now */
        CvIsMETHOD_off(PL_compcv);
        CV *initfields = newATTRSUB(floor_ix, NULL, NULL, NULL, ops);
        CvIsMETHOD_on(initfields);

        aux->xhv_class_initfields_cv = initfields;
    }
}

void
Perl_class_prepare_initfield_parse(pTHX)
{
    PERL_ARGS_ASSERT_CLASS_PREPARE_INITFIELD_PARSE;

    assert(HvSTASH_IS_CLASS(PL_curstash));
    struct xpvhv_aux *aux = HvAUX(PL_curstash);

    resume_compcv_and_save(aux->xhv_class_suspended_initfields_compcv);
    CvOUTSIDE_SEQ(PL_compcv) = PL_cop_seqmax;
}

void
Perl_class_prepare_method_parse(pTHX_ CV *cv)
{
    PERL_ARGS_ASSERT_CLASS_PREPARE_METHOD_PARSE;

    assert(cv == PL_compcv);
    assert(HvSTASH_IS_CLASS(PL_curstash));

    /* We expect this to be at the start of sub parsing, so there won't be
     * anything in the pad yet
     */
    assert(PL_comppad_name_fill == 0);

    PADOFFSET padix;

    padix = pad_add_name_pvs("$self", 0, NULL, NULL);
    assert(padix == PADIX_SELF);
    PERL_UNUSED_VAR(padix);

    intro_my();

    CvNOWARN_AMBIGUOUS_on(cv);
    CvIsMETHOD_on(cv);
}

OP *
Perl_class_wrap_method_body(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CLASS_WRAP_METHOD_BODY;

    if(!o)
        return o;

    PADNAMELIST *pnl = PadlistNAMES(CvPADLIST(PL_compcv));

    AV *fieldmap = newAV();
    UV max_fieldix = 0;
    SAVEFREESV((SV *)fieldmap);

    /* padix 0 == @_; padix 1 == $self. Start at 2 */
    for(PADOFFSET padix = 2; padix <= PadnamelistMAX(pnl); padix++) {
        PADNAME *pn = PadnamelistARRAY(pnl)[padix];
        if(!pn || !PadnameIsFIELD(pn))
            continue;

        U32 fieldix = PadnameFIELDINFO(pn)->fieldix;
        if(fieldix > max_fieldix)
            max_fieldix = fieldix;

        av_push(fieldmap, newSVuv(padix));
        av_push(fieldmap, newSVuv(fieldix));
    }

    UNOP_AUX_item *aux = NULL;

    if(av_count(fieldmap)) {
        Newx(aux, 2 + av_count(fieldmap), UNOP_AUX_item);

        UNOP_AUX_item *ap = aux;

        (ap++)->uv = av_count(fieldmap) / 2;
        (ap++)->uv = max_fieldix;

        for(Size_t i = 0; i < av_count(fieldmap); i++)
            (ap++)->uv = SvUV(AvARRAY(fieldmap)[i]);
    }

    /* If this is an empty method body then o will be an OP_STUB and not a
     * list. This will confuse op_sibling_splice() */
    if(o->op_type != OP_LINESEQ)
        o = newLISTOP(OP_LINESEQ, 0, o, NULL);

    op_sibling_splice(o, NULL, 0, newUNOP_AUX(OP_METHSTART, 0, NULL, aux));

    return o;
}

void
Perl_class_add_field(pTHX_ HV *stash, PADNAME *pn)
{
    PERL_ARGS_ASSERT_CLASS_ADD_FIELD;

    assert(HvSTASH_IS_CLASS(stash));
    struct xpvhv_aux *aux = HvAUX(stash);

    PADOFFSET fieldix = aux->xhv_class_next_fieldix;
    aux->xhv_class_next_fieldix++;

    Newxz(PadnameFIELDINFO(pn), 1, struct padname_fieldinfo);
    PadnameFLAGS(pn) |= PADNAMEf_FIELD;

    PadnameFIELDINFO(pn)->refcount = 1;
    PadnameFIELDINFO(pn)->fieldix = fieldix;
    PadnameFIELDINFO(pn)->fieldstash = (HV *)SvREFCNT_inc(stash);

    if(!aux->xhv_class_fields)
        aux->xhv_class_fields = newPADNAMELIST(0);

    padnamelist_store(aux->xhv_class_fields, PadnamelistMAX(aux->xhv_class_fields)+1, pn);
    PadnameREFCNT_inc(pn);
}

static void
apply_field_attribute_param(pTHX_ PADNAME *pn, SV *value)
{
    if(!value)
        /* Default to name minus the sigil */
        value = newSVpvn_utf8(PadnamePV(pn) + 1, PadnameLEN(pn) - 1, PadnameUTF8(pn));

    if(PadnamePV(pn)[0] != '$')
        croak("Only scalar fields can take a :param attribute");

    if(PadnameFIELDINFO(pn)->paramname)
        croak("Field already has a parameter name, cannot add another");

    HV *stash = PadnameFIELDINFO(pn)->fieldstash;
    assert(HvSTASH_IS_CLASS(stash));
    struct xpvhv_aux *aux = HvAUX(stash);

    if(aux->xhv_class_param_map &&
            hv_exists_ent(aux->xhv_class_param_map, value, 0))
        croak("Cannot assign :param(%" SVf ") to field %" SVf " because that name is already in use",
                SVfARG(value), SVfARG(PadnameSV(pn)));

    PadnameFIELDINFO(pn)->paramname = SvREFCNT_inc(value);

    if(!aux->xhv_class_param_map)
        aux->xhv_class_param_map = newHV();

    (void)hv_store_ent(aux->xhv_class_param_map, value, newSVuv(PadnameFIELDINFO(pn)->fieldix), 0);
}

static struct {
    const char *name;
    bool requires_value;
    void (*apply)(pTHX_ PADNAME *pn, SV *value);
} const field_attributes[] = {
    { .name           = "param",
      .requires_value = false,
      .apply          = &apply_field_attribute_param,
    },
    {0}
};

static void
S_class_apply_field_attribute(pTHX_ PADNAME *pn, OP *attr)
{
    assert(attr->op_type == OP_CONST);

    SV *name, *value;
    split_attr_nameval(cSVOPx_sv(attr), &name, &value);

    for(int i = 0; field_attributes[i].name; i++) {
        /* TODO: These attribute names are not UTF-8 aware */
        if(!strEQ(SvPVX(name), field_attributes[i].name))
            continue;

        if(field_attributes[i].requires_value && !(value && SvOK(value)))
            croak("Field attribute %" SVf " requires a value", SVfARG(name));

        (*field_attributes[i].apply)(aTHX_ pn, value);
        return;
    }

    croak("Unrecognized field attribute %" SVf, SVfARG(name));
}

void
Perl_class_apply_field_attributes(pTHX_ PADNAME *pn, OP *attrlist)
{
    PERL_ARGS_ASSERT_CLASS_APPLY_FIELD_ATTRIBUTES;

    if(!attrlist)
        return;
    if(attrlist->op_type == OP_NULL) {
        op_free(attrlist);
        return;
    }

    if(attrlist->op_type == OP_LIST) {
        OP *o = cLISTOPx(attrlist)->op_first;
        assert(o->op_type == OP_PUSHMARK);
        o = OpSIBLING(o);

        for(; o; o = OpSIBLING(o))
            S_class_apply_field_attribute(aTHX_ pn, o);
    }
    else
        S_class_apply_field_attribute(aTHX_ pn, attrlist);

    op_free(attrlist);
}

void
Perl_class_set_field_defop(pTHX_ PADNAME *pn, OPCODE defmode, OP *defop)
{
    PERL_ARGS_ASSERT_CLASS_SET_FIELD_DEFOP;

    assert(defmode == 0 || defmode == OP_ORASSIGN || defmode == OP_DORASSIGN);

    assert(HvSTASH_IS_CLASS(PL_curstash));

    forbid_outofblock_ops(defop, "field initialiser expression");

    if(PadnameFIELDINFO(pn)->defop)
        op_free(PadnameFIELDINFO(pn)->defop);

    char sigil = PadnamePV(pn)[0];
    switch(sigil) {
        case '$':
            defop = op_contextualize(defop, G_SCALAR);
            break;

        case '@':
        case '%':
            defop = op_contextualize(op_force_list(defop), G_LIST);
            break;
    }

    PadnameFIELDINFO(pn)->defop = newLISTOP(OP_LINESEQ, 0,
        newSTATEOP(0, NULL, NULL), defop);
    switch(defmode) {
        case OP_DORASSIGN:
            PadnameFIELDINFO(pn)->def_if_undef = true;
            break;
        case OP_ORASSIGN:
            PadnameFIELDINFO(pn)->def_if_false = true;
            break;
    }
}

void
Perl_class_add_ADJUST(pTHX_ HV *stash, CV *cv)
{
    PERL_ARGS_ASSERT_CLASS_ADD_ADJUST;

    assert(HvSTASH_IS_CLASS(stash));
    struct xpvhv_aux *aux = HvAUX(stash);

    if(!aux->xhv_class_adjust_blocks)
        aux->xhv_class_adjust_blocks = newAV();

    av_push(aux->xhv_class_adjust_blocks, (SV *)cv);
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
