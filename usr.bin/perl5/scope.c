/*    scope.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * For the fashion of Minas Tirith was such that it was built on seven
 * levels...
 *
 *     [p.751 of _The Lord of the Rings_, V/i: "Minas Tirith"]
 */

/* This file contains functions to manipulate several of Perl's stacks;
 * in particular it contains code to push various types of things onto
 * the savestack, then to pop them off and perform the correct restorative
 * action for each one. This corresponds to the cleanup Perl does at
 * each scope exit.
 */

#include "EXTERN.h"
#define PERL_IN_SCOPE_C
#include "perl.h"
#include "feature.h"

SV**
Perl_stack_grow(pTHX_ SV **sp, SV **p, SSize_t n)
{
    SSize_t extra;
    SSize_t current = (p - PL_stack_base);

    PERL_ARGS_ASSERT_STACK_GROW;

    if (UNLIKELY(n < 0))
        Perl_croak(aTHX_
            "panic: stack_grow() negative count (%" IVdf ")", (IV)n);

    PL_stack_sp = sp;
    extra =
#ifdef STRESS_REALLOC
        1;
#else
        128;
#endif
    /* If the total might wrap, panic instead. This is really testing
     * that (current + n + extra < SSize_t_MAX), but done in a way that
     * can't wrap */
    if (UNLIKELY(   current         > SSize_t_MAX - extra
                 || current + extra > SSize_t_MAX - n
    ))
        /* diag_listed_as: Out of memory during %s extend */
        Perl_croak(aTHX_ "Out of memory during stack extend");

    av_extend(PL_curstack, current + n + extra);
#ifdef DEBUGGING
        PL_curstackinfo->si_stack_hwm = current + n + extra;
#endif

    return PL_stack_sp;
}

#ifdef STRESS_REALLOC
#define GROW(old) ((old) + 1)
#else
#define GROW(old) ((old) * 3 / 2)
#endif

PERL_SI *
Perl_new_stackinfo(pTHX_ I32 stitems, I32 cxitems)
{
    PERL_SI *si;
    Newx(si, 1, PERL_SI);
    si->si_stack = newAV();
    AvREAL_off(si->si_stack);
    av_extend(si->si_stack, stitems > 0 ? stitems-1 : 0);
    AvALLOC(si->si_stack)[0] = &PL_sv_undef;
    AvFILLp(si->si_stack) = 0;
    si->si_prev = 0;
    si->si_next = 0;
    si->si_cxmax = cxitems - 1;
    si->si_cxix = -1;
    si->si_cxsubix = -1;
    si->si_type = PERLSI_UNDEF;
    Newx(si->si_cxstack, cxitems, PERL_CONTEXT);
    /* Without any kind of initialising CX_PUSHSUBST()
     * in pp_subst() will read uninitialised heap. */
    PoisonNew(si->si_cxstack, cxitems, PERL_CONTEXT);
    return si;
}

I32
Perl_cxinc(pTHX)
{
    const IV old_max = cxstack_max;
    const IV new_max = GROW(cxstack_max);
    Renew(cxstack, new_max + 1, PERL_CONTEXT);
    cxstack_max = new_max;
    /* Without any kind of initialising deep enough recursion
     * will end up reading uninitialised PERL_CONTEXTs. */
    PoisonNew(cxstack + old_max + 1, new_max - old_max, PERL_CONTEXT);
    return cxstack_ix + 1;
}

/*
=for apidoc_section $callback
=for apidoc push_scope

Implements L<perlapi/C<ENTER>>

=cut
*/

void
Perl_push_scope(pTHX)
{
    if (UNLIKELY(PL_scopestack_ix == PL_scopestack_max)) {
        const IV new_max = GROW(PL_scopestack_max);
        Renew(PL_scopestack, new_max, I32);
#ifdef DEBUGGING
        Renew(PL_scopestack_name, new_max, const char*);
#endif
        PL_scopestack_max = new_max;
    }
#ifdef DEBUGGING
    PL_scopestack_name[PL_scopestack_ix] = "unknown";
#endif
    PL_scopestack[PL_scopestack_ix++] = PL_savestack_ix;

}

/*
=for apidoc_section $callback
=for apidoc pop_scope

Implements L<perlapi/C<LEAVE>>

=cut
*/

void
Perl_pop_scope(pTHX)
{
    const I32 oldsave = PL_scopestack[--PL_scopestack_ix];
    LEAVE_SCOPE(oldsave);
}

I32 *
Perl_markstack_grow(pTHX)
{
    const I32 oldmax = PL_markstack_max - PL_markstack;
    const I32 newmax = GROW(oldmax);

    Renew(PL_markstack, newmax, I32);
    PL_markstack_max = PL_markstack + newmax;
    PL_markstack_ptr = PL_markstack + oldmax;
    DEBUG_s(DEBUG_v(PerlIO_printf(Perl_debug_log,
            "MARK grow %p %" IVdf " by %" IVdf "\n",
            PL_markstack_ptr, (IV)*PL_markstack_ptr, (IV)oldmax)));
    return PL_markstack_ptr;
}

void
Perl_savestack_grow(pTHX)
{
    const I32 by = PL_savestack_max - PL_savestack_ix;
    Perl_savestack_grow_cnt(aTHX_ by);
}

void
Perl_savestack_grow_cnt(pTHX_ I32 need)
{
    /* NOTE: PL_savestack_max and PL_savestack_ix are I32.
     *
     * This makes sense when you consider that having I32_MAX items on
     * the stack would be quite large.
     *
     * However, we use IV here so that we can detect if the new requested
     * amount is larger than I32_MAX.
     */
    const IV new_floor = PL_savestack_max + need; /* what we need */
    /* the GROW() macro normally does scales by 1.5 but under
     * STRESS_REALLOC it simply adds 1 */
    IV new_max         = GROW(new_floor); /* and some extra */

    /* the new_max < PL_savestack_max is for cases where IV is I32
     * and we have rolled over from I32_MAX to a small value */
    if (new_max > I32_MAX || new_max < PL_savestack_max) {
        if (new_floor > I32_MAX || new_floor < PL_savestack_max) {
            Perl_croak(aTHX_ "panic: savestack overflows I32_MAX");
        }
        new_max = new_floor;
    }

    /* Note that we add an additional SS_MAXPUSH slots on top of
     * PL_savestack_max so that SS_ADD_END(), SSGROW() etc can do
     * a simper check and if necessary realloc *after* apparently
     * overwriting the current PL_savestack_max. See scope.h.
     *
     * The +1 is because new_max/PL_savestack_max is the highest
     * index, by Renew needs the number of items, which is one
     * larger than the highest index. */
    Renew(PL_savestack, new_max + SS_MAXPUSH + 1, ANY);
    PL_savestack_max = new_max;
}

#undef GROW

/*  The original function was called Perl_tmps_grow and was removed from public
    API, Perl_tmps_grow_p is the replacement and it used in public macros but
    isn't public itself.

    Perl_tmps_grow_p takes a proposed ix. A proposed ix is PL_tmps_ix + extend_by,
    where the result of (PL_tmps_ix + extend_by) is >= PL_tmps_max
    Upon return, PL_tmps_stack[ix] will be a valid address. For machine code
    optimization and register usage reasons, the proposed ix passed into
    tmps_grow is returned to the caller which the caller can then use to write
    an SV * to PL_tmps_stack[ix]. If the caller was using tmps_grow in
    pre-extend mode (EXTEND_MORTAL macro), then it ignores the return value of
    tmps_grow. Note, tmps_grow DOES NOT write ix to PL_tmps_ix, the caller
    must assign ix or ret val of tmps_grow to PL_temps_ix themselves if that is
    appropriate. The assignment to PL_temps_ix can happen before or after
    tmps_grow call since tmps_grow doesn't look at PL_tmps_ix.
 */

SSize_t
Perl_tmps_grow_p(pTHX_ SSize_t ix)
{
    SSize_t extend_to = ix;
#ifndef STRESS_REALLOC
    if (ix - PL_tmps_max < 128)
        extend_to += (PL_tmps_max < 512) ? 128 : 512;
#endif
    Renew(PL_tmps_stack, extend_to + 1, SV*);
    PL_tmps_max = extend_to + 1;
    return ix;
}


void
Perl_free_tmps(pTHX)
{
    /* XXX should tmps_floor live in cxstack? */
    const SSize_t myfloor = PL_tmps_floor;
    while (PL_tmps_ix > myfloor) {      /* clean up after last statement */
        SV* const sv = PL_tmps_stack[PL_tmps_ix--];
#ifdef PERL_POISON
        PoisonWith(PL_tmps_stack + PL_tmps_ix + 1, 1, SV *, 0xAB);
#endif
        if (LIKELY(sv)) {
            SvTEMP_off(sv);
            SvREFCNT_dec_NN(sv);		/* note, can modify tmps_ix!!! */
        }
    }
}

/*
=for apidoc save_scalar_at

A helper function for localizing the SV referenced by C<*sptr>.

If C<SAVEf_KEEPOLDELEM> is set in in C<flags>, the function returns the input
scalar untouched.

Otherwise it replaces C<*sptr> with a new C<undef> scalar, and returns that.
The new scalar will have the old one's magic (if any) copied to it.
If there is such magic, and C<SAVEf_SETMAGIC> is set in in C<flags>, 'set'
magic will be processed on the new scalar.  If unset, 'set' magic will be
skipped.  The latter typically means that assignment will soon follow (I<e.g.>,
S<C<'local $x = $y'>>), and that will handle the magic.

=for apidoc Amnh ||SAVEf_KEEPOLDELEM
=for apidoc Amnh ||SAVEf_SETMAGIC

=cut
*/

STATIC SV *
S_save_scalar_at(pTHX_ SV **sptr, const U32 flags)
{
    SV * osv;
    SV *sv;

    PERL_ARGS_ASSERT_SAVE_SCALAR_AT;

    osv = *sptr;
    if (flags & SAVEf_KEEPOLDELEM)
        sv = osv;
    else {
        sv  = (*sptr = newSV_type(SVt_NULL));
        if (SvTYPE(osv) >= SVt_PVMG && SvMAGIC(osv))
            mg_localize(osv, sv, cBOOL(flags & SAVEf_SETMAGIC));
    }

    return sv;
}

void
Perl_save_pushptrptr(pTHX_ void *const ptr1, void *const ptr2, const int type)
{
    dSS_ADD;
    SS_ADD_PTR(ptr1);
    SS_ADD_PTR(ptr2);
    SS_ADD_UV(type);
    SS_ADD_END(3);
}

SV *
Perl_save_scalar(pTHX_ GV *gv)
{
    SV ** const sptr = &GvSVn(gv);

    PERL_ARGS_ASSERT_SAVE_SCALAR;

    if (UNLIKELY(SvGMAGICAL(*sptr))) {
        PL_localizing = 1;
        (void)mg_get(*sptr);
        PL_localizing = 0;
    }
    save_pushptrptr(SvREFCNT_inc_simple(gv), SvREFCNT_inc(*sptr), SAVEt_SV);
    return save_scalar_at(sptr, SAVEf_SETMAGIC); /* XXX - FIXME - see #60360 */
}

/*
=for apidoc save_generic_svref

Implements C<SAVEGENERICSV>.

Like save_sptr(), but also SvREFCNT_dec()s the new value.  Can be used to
restore a global SV to its prior contents, freeing new value.

=cut
 */

void
Perl_save_generic_svref(pTHX_ SV **sptr)
{
    PERL_ARGS_ASSERT_SAVE_GENERIC_SVREF;

    save_pushptrptr(sptr, SvREFCNT_inc(*sptr), SAVEt_GENERIC_SVREF);
}


/*
=for apidoc save_rcpv

Implements C<SAVERCPV>.

Saves and restores a refcounted string, similar to what
save_generic_svref would do for a SV*. Can be used to restore
a refcounted string to its previous state. Performs the 
appropriate refcount counting so that nothing should leak
or be prematurely freed.

=cut
 */
void
Perl_save_rcpv(pTHX_ char **prcpv) {
    PERL_ARGS_ASSERT_SAVE_RCPV;
    save_pushptrptr(prcpv, rcpv_copy(*prcpv), SAVEt_RCPV);
}

/*
=for apidoc save_freercpv

Implements C<SAVEFREERCPV>.

Saves and frees a refcounted string. Calls rcpv_free()
on the argument when the current pseudo block is finished.

=cut
 */
void
Perl_save_freercpv(pTHX_ char *rcpv) {
    PERL_ARGS_ASSERT_SAVE_FREERCPV;
    save_pushptr(rcpv, SAVEt_FREERCPV);
}


/*
=for apidoc_section $callback
=for apidoc save_generic_pvref

Implements C<SAVEGENERICPV>.

Like save_pptr(), but also Safefree()s the new value if it is different
from the old one.  Can be used to restore a global char* to its prior
contents, freeing new value.

=cut
 */

void
Perl_save_generic_pvref(pTHX_ char **str)
{
    PERL_ARGS_ASSERT_SAVE_GENERIC_PVREF;

    save_pushptrptr(*str, str, SAVEt_GENERIC_PVREF);
}

/*
=for apidoc_section $callback
=for apidoc save_shared_pvref

Implements C<SAVESHAREDPV>.

Like save_generic_pvref(), but uses PerlMemShared_free() rather than Safefree().
Can be used to restore a shared global char* to its prior
contents, freeing new value.

=cut
 */

void
Perl_save_shared_pvref(pTHX_ char **str)
{
    PERL_ARGS_ASSERT_SAVE_SHARED_PVREF;

    save_pushptrptr(str, *str, SAVEt_SHARED_PVREF);
}


/*
=for apidoc_section $callback
=for apidoc save_set_svflags

Implements C<SAVESETSVFLAGS>.

Set the SvFLAGS specified by mask to the values in val

=cut
 */

void
Perl_save_set_svflags(pTHX_ SV* sv, U32 mask, U32 val)
{
    dSS_ADD;

    PERL_ARGS_ASSERT_SAVE_SET_SVFLAGS;

    SS_ADD_PTR(sv);
    SS_ADD_INT(mask);
    SS_ADD_INT(val);
    SS_ADD_UV(SAVEt_SET_SVFLAGS);
    SS_ADD_END(4);
}

/*

=for apidoc_section $GV

=for apidoc save_gp

Saves the current GP of gv on the save stack to be restored on scope exit.

If C<empty> is true, replace the GP with a new GP.

If C<empty> is false, mark C<gv> with C<GVf_INTRO> so the next reference
assigned is localized, which is how S<C< local *foo = $someref; >> works.

=cut
*/

void
Perl_save_gp(pTHX_ GV *gv, I32 empty)
{
    PERL_ARGS_ASSERT_SAVE_GP;

    /* XXX For now, we just upgrade any coderef in the stash to a full GV
           during localisation.  Maybe at some point we could make localis-
           ation work without needing the upgrade.  (In which case our
           callers should probably call a different function, not save_gp.)
     */
    if (!isGV(gv)) {
        assert(isGV_or_RVCV(gv));
        (void)CvGV(SvRV((SV *)gv)); /* CvGV does the upgrade */
        assert(isGV(gv));
    }

    save_pushptrptr(SvREFCNT_inc(gv), GvGP(gv), SAVEt_GP);

    if (empty) {
        GP *gp = Perl_newGP(aTHX_ gv);
        HV * const stash = GvSTASH(gv);
        bool isa_changed = 0;

        if (stash && HvHasENAME(stash)) {
            if (memEQs(GvNAME(gv), GvNAMELEN(gv), "ISA"))
                isa_changed = TRUE;
            else if (GvCVu(gv))
                /* taking a method out of circulation ("local")*/
                mro_method_changed_in(stash);
        }
        if (GvIOp(gv) && (IoFLAGS(GvIOp(gv)) & IOf_ARGV)) {
            gp->gp_io = newIO();
            IoFLAGS(gp->gp_io) |= IOf_ARGV|IOf_START;
        }
        GvGP_set(gv,gp);
        if (isa_changed) mro_isa_changed_in(stash);
    }
    else {
        gp_ref(GvGP(gv));
        GvINTRO_on(gv);
    }
}

AV *
Perl_save_ary(pTHX_ GV *gv)
{
    AV * const oav = GvAVn(gv);
    AV *av;

    PERL_ARGS_ASSERT_SAVE_ARY;

    if (UNLIKELY(!AvREAL(oav) && AvREIFY(oav)))
        av_reify(oav);
    save_pushptrptr(SvREFCNT_inc_simple_NN(gv), oav, SAVEt_AV);

    GvAV(gv) = NULL;
    av = GvAVn(gv);
    if (UNLIKELY(SvMAGIC(oav)))
        mg_localize(MUTABLE_SV(oav), MUTABLE_SV(av), TRUE);
    return av;
}

HV *
Perl_save_hash(pTHX_ GV *gv)
{
    HV *ohv, *hv;

    PERL_ARGS_ASSERT_SAVE_HASH;

    save_pushptrptr(
        SvREFCNT_inc_simple_NN(gv), (ohv = GvHVn(gv)), SAVEt_HV
    );

    GvHV(gv) = NULL;
    hv = GvHVn(gv);
    if (UNLIKELY(SvMAGIC(ohv)))
        mg_localize(MUTABLE_SV(ohv), MUTABLE_SV(hv), TRUE);
    return hv;
}

void
Perl_save_item(pTHX_ SV *item)
{
    SV * const sv = newSVsv(item);

    PERL_ARGS_ASSERT_SAVE_ITEM;

    save_pushptrptr(item, /* remember the pointer */
                    sv,   /* remember the value */
                    SAVEt_ITEM);
}

void
Perl_save_bool(pTHX_ bool *boolp)
{
    dSS_ADD;

    PERL_ARGS_ASSERT_SAVE_BOOL;

    SS_ADD_PTR(boolp);
    SS_ADD_UV(SAVEt_BOOL | (*boolp << 8));
    SS_ADD_END(2);
}

void
Perl_save_pushi32ptr(pTHX_ const I32 i, void *const ptr, const int type)
{
    dSS_ADD;

    SS_ADD_INT(i);
    SS_ADD_PTR(ptr);
    SS_ADD_UV(type);
    SS_ADD_END(3);
}

void
Perl_save_int(pTHX_ int *intp)
{
    const int i = *intp;
    UV type = ((UV)((UV)i << SAVE_TIGHT_SHIFT) | SAVEt_INT_SMALL);
    int size = 2;
    dSS_ADD;

    PERL_ARGS_ASSERT_SAVE_INT;

    if (UNLIKELY((int)(type >> SAVE_TIGHT_SHIFT) != i)) {
        SS_ADD_INT(i);
        type = SAVEt_INT;
        size++;
    }
    SS_ADD_PTR(intp);
    SS_ADD_UV(type);
    SS_ADD_END(size);
}

void
Perl_save_I8(pTHX_ I8 *bytep)
{
    dSS_ADD;

    PERL_ARGS_ASSERT_SAVE_I8;

    SS_ADD_PTR(bytep);
    SS_ADD_UV(SAVEt_I8 | ((UV)*bytep << 8));
    SS_ADD_END(2);
}

void
Perl_save_I16(pTHX_ I16 *intp)
{
    dSS_ADD;

    PERL_ARGS_ASSERT_SAVE_I16;

    SS_ADD_PTR(intp);
    SS_ADD_UV(SAVEt_I16 | ((UV)*intp << 8));
    SS_ADD_END(2);
}

void
Perl_save_I32(pTHX_ I32 *intp)
{
    const I32 i = *intp;
    UV type = ((I32)((U32)i << SAVE_TIGHT_SHIFT) | SAVEt_I32_SMALL);
    int size = 2;
    dSS_ADD;

    PERL_ARGS_ASSERT_SAVE_I32;

    if (UNLIKELY((I32)(type >> SAVE_TIGHT_SHIFT) != i)) {
        SS_ADD_INT(i);
        type = SAVEt_I32;
        size++;
    }
    SS_ADD_PTR(intp);
    SS_ADD_UV(type);
    SS_ADD_END(size);
}

void
Perl_save_strlen(pTHX_ STRLEN *ptr)
{
    const IV i = *ptr;
    UV type = ((I32)((U32)i << SAVE_TIGHT_SHIFT) | SAVEt_STRLEN_SMALL);
    int size = 2;
    dSS_ADD;

    PERL_ARGS_ASSERT_SAVE_STRLEN;

    if (UNLIKELY((I32)(type >> SAVE_TIGHT_SHIFT) != i)) {
        SS_ADD_IV(*ptr);
        type = SAVEt_STRLEN;
        size++;
    }

    SS_ADD_PTR(ptr);
    SS_ADD_UV(type);
    SS_ADD_END(size);
}

void
Perl_save_iv(pTHX_ IV *ivp)
{
    PERL_ARGS_ASSERT_SAVE_IV;

    SSGROW(3);
    SSPUSHIV(*ivp);
    SSPUSHPTR(ivp);
    SSPUSHUV(SAVEt_IV);
}

/* Cannot use save_sptr() to store a char* since the SV** cast will
 * force word-alignment and we'll miss the pointer.
 */
void
Perl_save_pptr(pTHX_ char **pptr)
{
    PERL_ARGS_ASSERT_SAVE_PPTR;

    save_pushptrptr(*pptr, pptr, SAVEt_PPTR);
}

/*
=for apidoc_section $callback
=for apidoc save_vptr

Implements C<SAVEVPTR>.

=cut
 */

void
Perl_save_vptr(pTHX_ void *ptr)
{
    PERL_ARGS_ASSERT_SAVE_VPTR;

    save_pushptrptr(*(char**)ptr, ptr, SAVEt_VPTR);
}

void
Perl_save_sptr(pTHX_ SV **sptr)
{
    PERL_ARGS_ASSERT_SAVE_SPTR;

    save_pushptrptr(*sptr, sptr, SAVEt_SPTR);
}

/*
=for apidoc_section $callback
=for apidoc save_padsv_and_mortalize

Implements C<SAVEPADSVANDMORTALIZE>.

=cut
 */

void
Perl_save_padsv_and_mortalize(pTHX_ PADOFFSET off)
{
    dSS_ADD;

    ASSERT_CURPAD_ACTIVE("save_padsv");
    SS_ADD_PTR(SvREFCNT_inc_simple_NN(PL_curpad[off]));
    SS_ADD_PTR(PL_comppad);
    SS_ADD_UV((UV)off);
    SS_ADD_UV(SAVEt_PADSV_AND_MORTALIZE);
    SS_ADD_END(4);
}

void
Perl_save_hptr(pTHX_ HV **hptr)
{
    PERL_ARGS_ASSERT_SAVE_HPTR;

    save_pushptrptr(*hptr, hptr, SAVEt_HPTR);
}

void
Perl_save_aptr(pTHX_ AV **aptr)
{
    PERL_ARGS_ASSERT_SAVE_APTR;

    save_pushptrptr(*aptr, aptr, SAVEt_APTR);
}

/*
=for apidoc_section $callback
=for apidoc save_pushptr

The refcnt of object C<ptr> will be decremented at the end of the current
I<pseudo-block>.  C<type> gives the type of C<ptr>, expressed as one of the
constants in F<scope.h> whose name begins with C<SAVEt_>.

This is the underlying implementation of several macros, like
C<SAVEFREESV>.

=cut
*/

void
Perl_save_pushptr(pTHX_ void *const ptr, const int type)
{
    dSS_ADD;
    SS_ADD_PTR(ptr);
    SS_ADD_UV(type);
    SS_ADD_END(2);
}

void
Perl_save_clearsv(pTHX_ SV **svp)
{
    const UV offset = svp - PL_curpad;
    const UV offset_shifted = offset << SAVE_TIGHT_SHIFT;

    PERL_ARGS_ASSERT_SAVE_CLEARSV;

    ASSERT_CURPAD_ACTIVE("save_clearsv");
    assert(*svp);
    SvPADSTALE_off(*svp); /* mark lexical as active */
    if (UNLIKELY((offset_shifted >> SAVE_TIGHT_SHIFT) != offset)) {
        Perl_croak(aTHX_ "panic: pad offset %" UVuf " out of range (%p-%p)",
                   offset, svp, PL_curpad);
    }

    {
        dSS_ADD;
        SS_ADD_UV(offset_shifted | SAVEt_CLEARSV);
        SS_ADD_END(1);
    }
}

void
Perl_save_delete(pTHX_ HV *hv, char *key, I32 klen)
{
    PERL_ARGS_ASSERT_SAVE_DELETE;

    save_pushptri32ptr(key, klen, SvREFCNT_inc_simple(hv), SAVEt_DELETE);
}

/*
=for apidoc_section $callback
=for apidoc save_hdelete

Implements C<SAVEHDELETE>.

=cut
*/

void
Perl_save_hdelete(pTHX_ HV *hv, SV *keysv)
{
    STRLEN len;
    I32 klen;
    const char *key;

    PERL_ARGS_ASSERT_SAVE_HDELETE;

    key  = SvPV_const(keysv, len);
    klen = SvUTF8(keysv) ? -(I32)len : (I32)len;
    SvREFCNT_inc_simple_void_NN(hv);
    save_pushptri32ptr(savepvn(key, len), klen, hv, SAVEt_DELETE);
}

/*
=for apidoc_section $callback
=for apidoc save_adelete

Implements C<SAVEADELETE>.

=cut
*/

void
Perl_save_adelete(pTHX_ AV *av, SSize_t key)
{
    dSS_ADD;

    PERL_ARGS_ASSERT_SAVE_ADELETE;

    SvREFCNT_inc_void(av);
    SS_ADD_UV(key);
    SS_ADD_PTR(av);
    SS_ADD_IV(SAVEt_ADELETE);
    SS_ADD_END(3);
}

void
Perl_save_destructor(pTHX_ DESTRUCTORFUNC_NOCONTEXT_t f, void* p)
{
    dSS_ADD;
    PERL_ARGS_ASSERT_SAVE_DESTRUCTOR;

    SS_ADD_DPTR(f);
    SS_ADD_PTR(p);
    SS_ADD_UV(SAVEt_DESTRUCTOR);
    SS_ADD_END(3);
}

void
Perl_save_destructor_x(pTHX_ DESTRUCTORFUNC_t f, void* p)
{
    dSS_ADD;

    SS_ADD_DXPTR(f);
    SS_ADD_PTR(p);
    SS_ADD_UV(SAVEt_DESTRUCTOR_X);
    SS_ADD_END(3);
}

/*
=for apidoc_section $callback
=for apidoc save_hints

Implements C<SAVEHINTS>.

=cut
 */

void
Perl_save_hints(pTHX)
{
    COPHH *save_cophh = cophh_copy(CopHINTHASH_get(&PL_compiling));
    if (PL_hints & HINT_LOCALIZE_HH) {
        HV *oldhh = GvHV(PL_hintgv);
        {
            dSS_ADD;
            SS_ADD_INT(PL_hints);
            SS_ADD_PTR(save_cophh);
            SS_ADD_PTR(oldhh);
            SS_ADD_UV(SAVEt_HINTS_HH | (PL_prevailing_version << 8));
            SS_ADD_END(4);
        }
        GvHV(PL_hintgv) = NULL; /* in case copying dies */
        GvHV(PL_hintgv) = hv_copy_hints_hv(oldhh);
        SAVEFEATUREBITS();
    } else {
        save_pushi32ptr(PL_hints, save_cophh, SAVEt_HINTS | (PL_prevailing_version << 8));
    }
}

static void
S_save_pushptri32ptr(pTHX_ void *const ptr1, const I32 i, void *const ptr2,
                        const int type)
{
    dSS_ADD;
    SS_ADD_PTR(ptr1);
    SS_ADD_INT(i);
    SS_ADD_PTR(ptr2);
    SS_ADD_UV(type);
    SS_ADD_END(4);
}

/*
=for apidoc_section $callback
=for apidoc      save_aelem
=for apidoc_item save_aelem_flags

These each arrange for the value of the array element C<av[idx]> to be restored
at the end of the enclosing I<pseudo-block>.

In C<save_aelem>, the SV at C**sptr> will be replaced by a new C<undef>
scalar.  That scalar will inherit any magic from the original C<**sptr>,
and any 'set' magic will be processed.

In C<save_aelem_flags>, C<SAVEf_KEEPOLDELEM> being set in C<flags> causes
the function to forgo all that:  the scalar at C<**sptr> is untouched.
If C<SAVEf_KEEPOLDELEM> is not set, the SV at C**sptr> will be replaced by a
new C<undef> scalar.  That scalar will inherit any magic from the original
C<**sptr>.  Any 'set' magic will be processed if and only if C<SAVEf_SETMAGIC>
is set in in C<flags>.

=cut
*/

void
Perl_save_aelem_flags(pTHX_ AV *av, SSize_t idx, SV **sptr,
                            const U32 flags)
{
    dSS_ADD;
    SV *sv;

    PERL_ARGS_ASSERT_SAVE_AELEM_FLAGS;

    SvGETMAGIC(*sptr);
    SS_ADD_PTR(SvREFCNT_inc_simple(av));
    SS_ADD_IV(idx);
    SS_ADD_PTR(SvREFCNT_inc(*sptr));
    SS_ADD_UV(SAVEt_AELEM);
    SS_ADD_END(4);
    /* The array needs to hold a reference count on its new element, so it
       must be AvREAL. */
    if (UNLIKELY(!AvREAL(av) && AvREIFY(av)))
        av_reify(av);
    save_scalar_at(sptr, flags); /* XXX - FIXME - see #60360 */
    if (flags & SAVEf_KEEPOLDELEM)
        return;
    sv = *sptr;
    /* If we're localizing a tied array element, this new sv
     * won't actually be stored in the array - so it won't get
     * reaped when the localize ends. Ensure it gets reaped by
     * mortifying it instead. DAPM */
    if (UNLIKELY(SvTIED_mg((const SV *)av, PERL_MAGIC_tied)))
        sv_2mortal(sv);
}

/*
=for apidoc_section $callback
=for apidoc      save_helem
=for apidoc_item save_helem_flags

These each arrange for the value of the hash element (in Perlish terms)
C<$hv{key}]> to be restored at the end of the enclosing I<pseudo-block>.

In C<save_helem>, the SV at C**sptr> will be replaced by a new C<undef>
scalar.  That scalar will inherit any magic from the original C<**sptr>,
and any 'set' magic will be processed.

In C<save_helem_flags>, C<SAVEf_KEEPOLDELEM> being set in C<flags> causes
the function to forgo all that:  the scalar at C<**sptr> is untouched.
If C<SAVEf_KEEPOLDELEM> is not set, the SV at C**sptr> will be replaced by a
new C<undef> scalar.  That scalar will inherit any magic from the original
C<**sptr>.  Any 'set' magic will be processed if and only if C<SAVEf_SETMAGIC>
is set in in C<flags>.

=cut
*/

void
Perl_save_helem_flags(pTHX_ HV *hv, SV *key, SV **sptr, const U32 flags)
{
    SV *sv;

    PERL_ARGS_ASSERT_SAVE_HELEM_FLAGS;

    SvGETMAGIC(*sptr);
    {
        dSS_ADD;
        SS_ADD_PTR(SvREFCNT_inc_simple(hv));
        SS_ADD_PTR(newSVsv(key));
        SS_ADD_PTR(SvREFCNT_inc(*sptr));
        SS_ADD_UV(SAVEt_HELEM);
        SS_ADD_END(4);
    }
    save_scalar_at(sptr, flags);
    if (flags & SAVEf_KEEPOLDELEM)
        return;
    sv = *sptr;
    /* If we're localizing a tied hash element, this new sv
     * won't actually be stored in the hash - so it won't get
     * reaped when the localize ends. Ensure it gets reaped by
     * mortifying it instead. DAPM */
    if (UNLIKELY(SvTIED_mg((const SV *)hv, PERL_MAGIC_tied)))
        sv_2mortal(sv);
}

SV*
Perl_save_svref(pTHX_ SV **sptr)
{
    PERL_ARGS_ASSERT_SAVE_SVREF;

    SvGETMAGIC(*sptr);
    save_pushptrptr(sptr, SvREFCNT_inc(*sptr), SAVEt_SVREF);
    return save_scalar_at(sptr, SAVEf_SETMAGIC); /* XXX - FIXME - see #60360 */
}


void
Perl_savetmps(pTHX)
{
    dSS_ADD;
    SS_ADD_IV(PL_tmps_floor);
    PL_tmps_floor = PL_tmps_ix;
    SS_ADD_UV(SAVEt_TMPSFLOOR);
    SS_ADD_END(2);
}

/*
=for apidoc_section $stack
=for apidoc save_alloc

Implements L<perlapi/C<SSNEW>> and kin, which should be used instead of this
function.

=cut
*/

SSize_t
Perl_save_alloc(pTHX_ SSize_t size, I32 pad)
{
    const SSize_t start = pad + ((char*)&PL_savestack[PL_savestack_ix]
                          - (char*)PL_savestack);
    const UV elems = 1 + ((size + pad - 1) / sizeof(*PL_savestack));
    const UV elems_shifted = elems << SAVE_TIGHT_SHIFT;

    if (UNLIKELY((elems_shifted >> SAVE_TIGHT_SHIFT) != elems))
        Perl_croak(aTHX_
            "panic: save_alloc elems %" UVuf " out of range (%" IVdf "-%" IVdf ")",
                   elems, (IV)size, (IV)pad);

    SSGROW(elems + 1);

    PL_savestack_ix += elems;
    SSPUSHUV(SAVEt_ALLOC | elems_shifted);
    return start;
}



/*
=for apidoc_section $callback
=for apidoc leave_scope

Implements C<LEAVE_SCOPE> which you should use instead.

=cut
 */

void
Perl_leave_scope(pTHX_ I32 base)
{
    /* Localise the effects of the TAINT_NOT inside the loop.  */
    bool was = TAINT_get;

    if (UNLIKELY(base < -1))
        Perl_croak(aTHX_ "panic: corrupt saved stack index %ld", (long) base);
    DEBUG_l(Perl_deb(aTHX_ "savestack: releasing items %ld -> %ld\n",
                        (long)PL_savestack_ix, (long)base));
    while (PL_savestack_ix > base) {
        UV uv;
        U8 type;
        ANY *ap; /* arg pointer */
        ANY a0, a1, a2; /* up to 3 args */

        TAINT_NOT;

        {
            U8  argcount;
            I32 ix = PL_savestack_ix - 1;

            ap = &PL_savestack[ix];
            uv = ap->any_uv;
            type = (U8)uv & SAVE_MASK;
            argcount = leave_scope_arg_counts[type];
            PL_savestack_ix = ix - argcount;
            ap -= argcount;
        }

        switch (type) {
        case SAVEt_ITEM:			/* normal string */
            a0 = ap[0]; a1 = ap[1];
            sv_replace(a0.any_sv, a1.any_sv);
            if (UNLIKELY(SvSMAGICAL(a0.any_sv))) {
                PL_localizing = 2;
                mg_set(a0.any_sv);
                PL_localizing = 0;
            }
            break;

            /* This would be a mathom, but Perl_save_svref() calls a static
               function, S_save_scalar_at(), so has to stay in this file.  */
        case SAVEt_SVREF:			/* scalar reference */
            a0 = ap[0]; a1 = ap[1];
            a2.any_svp = a0.any_svp;
            a0.any_sv = NULL; /* what to refcnt_dec */
            goto restore_sv;

        case SAVEt_SV:				/* scalar reference */
            a0 = ap[0]; a1 = ap[1];
            a2.any_svp = &GvSV(a0.any_gv);
        restore_sv:
        {
            /* do *a2.any_svp = a1 and free a0 */
            SV * const sv = *a2.any_svp;
            *a2.any_svp = a1.any_sv;
            SvREFCNT_dec(sv);
            if (UNLIKELY(SvSMAGICAL(a1.any_sv))) {
                /* mg_set could die, skipping the freeing of a0 and
                 * a1; Ensure that they're always freed in that case */
                dSS_ADD;
                SS_ADD_PTR(a1.any_sv);
                SS_ADD_UV(SAVEt_FREESV);
                SS_ADD_PTR(a0.any_sv);
                SS_ADD_UV(SAVEt_FREESV);
                SS_ADD_END(4);
                PL_localizing = 2;
                mg_set(a1.any_sv);
                PL_localizing = 0;
                break;
            }
            SvREFCNT_dec_NN(a1.any_sv);
            SvREFCNT_dec(a0.any_sv);
            break;
        }

        case SAVEt_GENERIC_PVREF:		/* generic pv */
            a0 = ap[0]; a1 = ap[1];
            if (*a1.any_pvp != a0.any_pv) {
                Safefree(*a1.any_pvp);
                *a1.any_pvp = a0.any_pv;
            }
            break;

        case SAVEt_SHARED_PVREF:		/* shared pv */
            a0 = ap[0]; a1 = ap[1];
            if (*a0.any_pvp != a1.any_pv) {
                PerlMemShared_free(*a0.any_pvp);
                *a0.any_pvp = a1.any_pv;
            }
            break;

        case SAVEt_GVSV:			/* scalar slot in GV */
            a0 = ap[0]; a1 = ap[1];
            a0.any_svp = &GvSV(a0.any_gv);
            goto restore_svp;


        case SAVEt_GENERIC_SVREF:		/* generic sv */
            a0 = ap[0]; a1 = ap[1];
        restore_svp:
        {
            /* do *a0.any_svp = a1 */
            SV * const sv = *a0.any_svp;
            *a0.any_svp = a1.any_sv;
            SvREFCNT_dec(sv);
            SvREFCNT_dec(a1.any_sv);
            break;
        }

        case SAVEt_RCPV:           /* like generic sv, but for struct rcpv */
        {
            a0 = ap[0]; a1 = ap[1];
            char *old = *a0.any_pvp;
            *a0.any_pvp = a1.any_pv;
            (void)rcpv_free(old);
            (void)rcpv_free(a1.any_pv);
            break;
        }

        case SAVEt_FREERCPV:           /* like SAVEt_FREEPV but for a RCPV */
        {
            a0 = ap[0];
            char *rcpv = a0.any_pv;
            (void)rcpv_free(rcpv);
            break;
        }

        case SAVEt_GVSLOT:			/* any slot in GV */
        {
            HV * hv;
            a0 = ap[0]; a1 = ap[1]; a2 = ap[2];
            hv = GvSTASH(a0.any_gv);
            if (hv && HvHasENAME(hv) && (
                    (a2.any_sv && SvTYPE(a2.any_sv) == SVt_PVCV)
                 || (*a1.any_svp && SvTYPE(*a1.any_svp) == SVt_PVCV)
               ))
            {
                if ((char *)a1.any_svp < (char *)GvGP(a0.any_gv)
                 || (char *)a1.any_svp > (char *)GvGP(a0.any_gv) + sizeof(struct gp)
                 || GvREFCNT(a0.any_gv) > 2) /* "> 2" to ignore savestack's ref */
                    PL_sub_generation++;
                else mro_method_changed_in(hv);
            }
            a0.any_svp = a1.any_svp;
            a1.any_sv  = a2.any_sv;
            goto restore_svp;
        }

        case SAVEt_AV:				/* array reference */
            a0 = ap[0]; a1 = ap[1];
            SvREFCNT_dec(GvAV(a0.any_gv));
            GvAV(a0.any_gv) = a1.any_av;
          avhv_common:
            if (UNLIKELY(SvSMAGICAL(a1.any_sv))) {
                /* mg_set might die, so make sure a0 isn't leaked */
                dSS_ADD;
                SS_ADD_PTR(a0.any_sv);
                SS_ADD_UV(SAVEt_FREESV);
                SS_ADD_END(2);
                PL_localizing = 2;
                mg_set(a1.any_sv);
                PL_localizing = 0;
                break;
            }
            SvREFCNT_dec_NN(a0.any_sv);
            break;

        case SAVEt_HV:				/* hash reference */
            a0 = ap[0]; a1 = ap[1];
            SvREFCNT_dec(GvHV(a0.any_gv));
            GvHV(a0.any_gv) = a1.any_hv;
            goto avhv_common;

        case SAVEt_INT_SMALL:
            a0 = ap[0];
            *(int*)a0.any_ptr = (int)(uv >> SAVE_TIGHT_SHIFT);
            break;

        case SAVEt_INT:				/* int reference */
            a0 = ap[0]; a1 = ap[1];
            *(int*)a1.any_ptr = (int)a0.any_i32;
            break;

        case SAVEt_STRLEN_SMALL:
            a0 = ap[0];
            *(STRLEN*)a0.any_ptr = (STRLEN)(uv >> SAVE_TIGHT_SHIFT);
            break;

        case SAVEt_STRLEN:			/* STRLEN/size_t ref */
            a0 = ap[0]; a1 = ap[1];
            *(STRLEN*)a1.any_ptr = (STRLEN)a0.any_iv;
            break;

        case SAVEt_TMPSFLOOR:			/* restore PL_tmps_floor */
            a0 = ap[0];
            PL_tmps_floor = (SSize_t)a0.any_iv;
            break;

        case SAVEt_BOOL:			/* bool reference */
            a0 = ap[0];
            *(bool*)a0.any_ptr = cBOOL(uv >> 8);
#ifdef NO_TAINT_SUPPORT
            PERL_UNUSED_VAR(was);
#else
            if (UNLIKELY(a0.any_ptr == &(PL_tainted))) {
                /* If we don't update <was>, to reflect what was saved on the
                 * stack for PL_tainted, then we will overwrite this attempt to
                 * restore it when we exit this routine.  Note that this won't
                 * work if this value was saved in a wider-than necessary type,
                 * such as I32 */
                was = *(bool*)a0.any_ptr;
            }
#endif
            break;

        case SAVEt_I32_SMALL:
            a0 = ap[0];
            *(I32*)a0.any_ptr = (I32)(uv >> SAVE_TIGHT_SHIFT);
            break;

        case SAVEt_I32:				/* I32 reference */
            a0 = ap[0]; a1 = ap[1];
#ifdef PERL_DEBUG_READONLY_OPS
            if (*(I32*)a1.any_ptr != a0.any_i32)
#endif
                *(I32*)a1.any_ptr = a0.any_i32;
            break;

        case SAVEt_SPTR:			/* SV* reference */
        case SAVEt_VPTR:			/* random* reference */
        case SAVEt_PPTR:			/* char* reference */
        case SAVEt_HPTR:			/* HV* reference */
        case SAVEt_APTR:			/* AV* reference */
            a0 = ap[0]; a1 = ap[1];
            *a1.any_svp= a0.any_sv;
            break;

        case SAVEt_GP:				/* scalar reference */
        {
            HV *hv;
            bool had_method;

            a0 = ap[0]; a1 = ap[1];
            /* possibly taking a method out of circulation */	
            had_method = cBOOL(GvCVu(a0.any_gv));
            gp_free(a0.any_gv);
            GvGP_set(a0.any_gv, (GP*)a1.any_ptr);
            if ((hv=GvSTASH(a0.any_gv)) && HvHasENAME(hv)) {
                if (memEQs(GvNAME(a0.any_gv), GvNAMELEN(a0.any_gv), "ISA"))
                    mro_isa_changed_in(hv);
                else if (had_method || GvCVu(a0.any_gv))
                    /* putting a method back into circulation ("local")*/	
                    gv_method_changed(a0.any_gv);
            }
            SvREFCNT_dec_NN(a0.any_gv);
            break;
        }

        case SAVEt_FREESV:
            a0 = ap[0];
            SvREFCNT_dec(a0.any_sv);
            break;

        case SAVEt_FREEPADNAME:
            a0 = ap[0];
            PadnameREFCNT_dec((PADNAME *)a0.any_ptr);
            break;

        case SAVEt_FREECOPHH:
            a0 = ap[0];
            cophh_free((COPHH *)a0.any_ptr);
            break;

        case SAVEt_MORTALIZESV:
            a0 = ap[0];
            sv_2mortal(a0.any_sv);
            break;

        case SAVEt_FREEOP:
            a0 = ap[0];
            ASSERT_CURPAD_LEGAL("SAVEt_FREEOP");
            op_free(a0.any_op);
            break;

        case SAVEt_FREEPV:
            a0 = ap[0];
            Safefree(a0.any_ptr);
            break;

        case SAVEt_CLEARPADRANGE:
        {
            I32 i;
            SV **svp;
            i = (I32)((uv >> SAVE_TIGHT_SHIFT) & OPpPADRANGE_COUNTMASK);
            svp = &PL_curpad[uv >>
                    (OPpPADRANGE_COUNTSHIFT + SAVE_TIGHT_SHIFT)] + i - 1;
            goto clearsv;
        case SAVEt_CLEARSV:
            svp = &PL_curpad[uv >> SAVE_TIGHT_SHIFT];
            i = 1;
          clearsv:
            for (; i; i--, svp--) {
                SV *sv = *svp;

                DEBUG_Xv(PerlIO_printf(Perl_debug_log,
             "Pad 0x%" UVxf "[0x%" UVxf "] clearsv: %ld sv=0x%" UVxf "<%" IVdf "> %s\n",
                    PTR2UV(PL_comppad), PTR2UV(PL_curpad),
                    (long)(svp-PL_curpad), PTR2UV(sv), (IV)SvREFCNT(sv),
                    (SvREFCNT(sv) <= 1 && !SvOBJECT(sv)) ? "clear" : "abandon"
                ));

                /* Can clear pad variable in place? */
                if (SvREFCNT(sv) == 1 && !SvOBJECT(sv)) {

                    /* these flags are the union of all the relevant flags
                     * in the individual conditions within */
                    if (UNLIKELY(SvFLAGS(sv) & (
                            SVf_READONLY|SVf_PROTECT /*for SvREADONLY_off*/
                          | (SVs_GMG|SVs_SMG|SVs_RMG) /* SvMAGICAL() */
                          | SVf_OOK
                          | SVf_THINKFIRST)))
                    {
                        /* if a my variable that was made readonly is
                         * going out of scope, we want to remove the
                         * readonlyness so that it can go out of scope
                         * quietly
                         */
                        if (SvREADONLY(sv))
                            SvREADONLY_off(sv);

                        if (SvTYPE(sv) == SVt_PVHV && HvHasAUX(sv))
                            Perl_hv_kill_backrefs(aTHX_ MUTABLE_HV(sv));
                        else if(SvOOK(sv))
                            sv_backoff(sv);

                        if (SvMAGICAL(sv)) {
                            /* note that backrefs (either in HvAUX or magic)
                             * must be removed before other magic */
                            sv_unmagic(sv, PERL_MAGIC_backref);
                            if (SvTYPE(sv) != SVt_PVCV)
                                mg_free(sv);
                        }
                        if (SvTHINKFIRST(sv))
                            sv_force_normal_flags(sv, SV_IMMEDIATE_UNREF
                                                     |SV_COW_DROP_PV);

                    }
                    switch (SvTYPE(sv)) {
                    case SVt_NULL:
                        break;
                    case SVt_PVAV:
                        av_clear(MUTABLE_AV(sv));
                        break;
                    case SVt_PVHV:
                        hv_clear(MUTABLE_HV(sv));
                        break;
                    case SVt_PVCV:
                    {
                        HEK *hek = CvGvNAME_HEK(sv);
                        assert(hek);
                        (void)share_hek_hek(hek);
                        cv_undef((CV *)sv);
                        CvNAME_HEK_set(sv, hek);
                        CvLEXICAL_on(sv);
                        break;
                    }
                    default:
                        /* This looks odd, but these two macros are for use in
                           expressions and finish with a trailing comma, so
                           adding a ; after them would be wrong. */
                        assert_not_ROK(sv)
                        assert_not_glob(sv)
                        SvFLAGS(sv) &=~ (SVf_OK|SVf_IVisUV|SVf_UTF8);
                        break;
                    }
                    SvPADTMP_off(sv);
                    SvPADSTALE_on(sv); /* mark as no longer live */
                }
                else {	/* Someone has a claim on this, so abandon it. */
                    switch (SvTYPE(sv)) {	/* Console ourselves with a new value */
                    case SVt_PVAV:	*svp = MUTABLE_SV(newAV());	break;
                    case SVt_PVHV:	*svp = MUTABLE_SV(newHV());	break;
                    case SVt_PVCV:
                    {
                        HEK * const hek = CvGvNAME_HEK(sv);

                        /* Create a stub */
                        *svp = newSV_type(SVt_PVCV);

                        /* Share name */
                        CvNAME_HEK_set(*svp,
                                       share_hek_hek(hek));
                        CvLEXICAL_on(*svp);
                        break;
                    }
                    default:	*svp = newSV_type(SVt_NULL);		break;
                    }
                    SvREFCNT_dec_NN(sv); /* Cast current value to the winds. */
                    /* preserve pad nature, but also mark as not live
                     * for any closure capturing */
                    SvFLAGS(*svp) |= SVs_PADSTALE;
                }
            }
            break;
        }

        case SAVEt_DELETE:
            a0 = ap[0]; a1 = ap[1]; a2 = ap[2];
            /* hv_delete could die, so free the key and SvREFCNT_dec the
             * hv by pushing new save actions
             */
            /* ap[0] is the key */
            ap[1].any_uv = SAVEt_FREEPV; /* was len */
            /* ap[2] is the hv */
            ap[3].any_uv = SAVEt_FREESV; /* was SAVEt_DELETE */
            PL_savestack_ix += 4;
            (void)hv_delete(a2.any_hv, a0.any_pv, a1.any_i32, G_DISCARD);
            break;

        case SAVEt_ADELETE:
            a0 = ap[0]; a1 = ap[1];
            /* av_delete could die, so SvREFCNT_dec the av by pushing a
             * new save action
             */
            ap[0].any_av = a1.any_av;
            ap[1].any_uv = SAVEt_FREESV;
            PL_savestack_ix += 2;
            (void)av_delete(a1.any_av, a0.any_iv, G_DISCARD);
            break;

        case SAVEt_DESTRUCTOR_X:
            a0 = ap[0]; a1 = ap[1];
            (*a0.any_dxptr)(aTHX_ a1.any_ptr);
            break;

        case SAVEt_REGCONTEXT:
            /* regexp must have croaked */
        case SAVEt_ALLOC:
            PL_savestack_ix -= uv >> SAVE_TIGHT_SHIFT;
            break;

        case SAVEt_STACK_POS:		/* Position on Perl stack */
            a0 = ap[0];
            PL_stack_sp = PL_stack_base + a0.any_i32;
            break;

        case SAVEt_AELEM:		/* array element */
        {
            SV **svp;
            a0 = ap[0]; a1 = ap[1]; a2 = ap[2];
            svp = av_fetch(a0.any_av, a1.any_iv, 1);
            if (UNLIKELY(!AvREAL(a0.any_av) && AvREIFY(a0.any_av))) /* undo reify guard */
                SvREFCNT_dec(a2.any_sv);
            if (LIKELY(svp)) {
                SV * const sv = *svp;
                if (LIKELY(sv && sv != &PL_sv_undef)) {
                    if (UNLIKELY(SvTIED_mg((const SV *)a0.any_av, PERL_MAGIC_tied)))
                        SvREFCNT_inc_void_NN(sv);
                    a1.any_sv  = a2.any_sv;
                    a2.any_svp = svp;
                    goto restore_sv;
                }
            }
            SvREFCNT_dec(a0.any_av);
            SvREFCNT_dec(a2.any_sv);
            break;
        }

        case SAVEt_HELEM:		/* hash element */
        {
            HE *he;

            a0 = ap[0]; a1 = ap[1]; a2 = ap[2];
            he = hv_fetch_ent(a0.any_hv, a1.any_sv, 1, 0);
            SvREFCNT_dec(a1.any_sv);
            if (LIKELY(he)) {
                const SV * const oval = HeVAL(he);
                if (LIKELY(oval && oval != &PL_sv_undef)) {
                    SV **svp = &HeVAL(he);
                    if (UNLIKELY(SvTIED_mg((const SV *)a0.any_hv, PERL_MAGIC_tied)))
                        SvREFCNT_inc_void(*svp);
                    a1.any_sv  = a2.any_sv;
                    a2.any_svp = svp;
                    goto restore_sv;
                }
            }
            SvREFCNT_dec(a0.any_hv);
            SvREFCNT_dec(a2.any_sv);
            break;
        }

        case SAVEt_OP:
            a0 = ap[0];
            PL_op = (OP*)a0.any_ptr;
            break;

        case SAVEt_HINTS_HH:
            a2 = ap[2];
            /* FALLTHROUGH */
        case SAVEt_HINTS:
            a0 = ap[0]; a1 = ap[1];
            if ((PL_hints & HINT_LOCALIZE_HH)) {
              while (GvHV(PL_hintgv)) {
                HV *hv = GvHV(PL_hintgv);
                GvHV(PL_hintgv) = NULL;
                SvREFCNT_dec(MUTABLE_SV(hv));
              }
            }
            cophh_free(CopHINTHASH_get(&PL_compiling));
            CopHINTHASH_set(&PL_compiling, (COPHH*)a1.any_ptr);
            *(I32*)&PL_hints = a0.any_i32;
            PL_prevailing_version = (U16)(uv >> 8);
            if (type == SAVEt_HINTS_HH) {
                SvREFCNT_dec(MUTABLE_SV(GvHV(PL_hintgv)));
                GvHV(PL_hintgv) = MUTABLE_HV(a2.any_ptr);
            }
            if (!GvHV(PL_hintgv)) {
                /* Need to add a new one manually, else rv2hv can
                   add one via GvHVn and it won't have the magic set.  */
                HV *const hv = newHV();
                hv_magic(hv, NULL, PERL_MAGIC_hints);
                GvHV(PL_hintgv) = hv;
            }
            assert(GvHV(PL_hintgv));
            break;

        case SAVEt_COMPPAD:
            a0 = ap[0];
            PL_comppad = (PAD*)a0.any_ptr;
            if (LIKELY(PL_comppad))
                PL_curpad = AvARRAY(PL_comppad);
            else
                PL_curpad = NULL;
            break;

        case SAVEt_PADSV_AND_MORTALIZE:
            {
                SV **svp;

                a0 = ap[0]; a1 = ap[1]; a2 = ap[2];
                assert (a1.any_ptr);
                svp = AvARRAY((PAD*)a1.any_ptr) + (PADOFFSET)a2.any_uv;
                /* This mortalizing used to be done by CX_POOPLOOP() via
                   itersave.  But as we have all the information here, we
                   can do it here, save even having to have itersave in
                   the struct.
                   */
                sv_2mortal(*svp);
                *svp = a0.any_sv;
            }
            break;

        case SAVEt_SAVESWITCHSTACK:
            {
                dSP;

                a0 = ap[0]; a1 = ap[1];
                SWITCHSTACK(a1.any_av, a0.any_av);
                PL_curstackinfo->si_stack = a0.any_av;
            }
            break;

        case SAVEt_SET_SVFLAGS:
            a0 = ap[0]; a1 = ap[1]; a2 = ap[2];
            SvFLAGS(a0.any_sv) &= ~(a1.any_u32);
            SvFLAGS(a0.any_sv) |= a2.any_u32;
            break;

            /* These are only saved in mathoms.c */
        case SAVEt_NSTAB:
            a0 = ap[0];
            (void)sv_clear(a0.any_sv);
            break;

        case SAVEt_LONG:			/* long reference */
            a0 = ap[0]; a1 = ap[1];
            *(long*)a1.any_ptr = a0.any_long;
            break;

        case SAVEt_IV:				/* IV reference */
            a0 = ap[0]; a1 = ap[1];
            *(IV*)a1.any_ptr = a0.any_iv;
            break;

        case SAVEt_I16:				/* I16 reference */
            a0 = ap[0];
            *(I16*)a0.any_ptr = (I16)(uv >> 8);
            break;

        case SAVEt_I8:				/* I8 reference */
            a0 = ap[0];
            *(I8*)a0.any_ptr = (I8)(uv >> 8);
            break;

        case SAVEt_DESTRUCTOR:
            a0 = ap[0]; a1 = ap[1];
            (*a0.any_dptr)(a1.any_ptr);
            break;

        case SAVEt_COMPILE_WARNINGS:
            /* NOTE: we can't put &PL_compiling or PL_curcop on the save
             *       stack directly, as we currently cannot translate
             *       them to the correct addresses after a thread start
             *       or win32 fork start. - Yves
             */
            a0 = ap[0];
            free_and_set_cop_warnings(&PL_compiling, a0.any_pv);
            break;

        case SAVEt_CURCOP_WARNINGS:
            /* NOTE: see comment above about SAVEt_COMPILE_WARNINGS */
            a0 = ap[0];
            free_and_set_cop_warnings(PL_curcop, a0.any_pv);
            break;

        case SAVEt_PARSER:
            a0 = ap[0];
            parser_free((yy_parser *)a0.any_ptr);
            break;

        case SAVEt_READONLY_OFF:
            a0 = ap[0];
            SvREADONLY_off(a0.any_sv);
            break;

        default:
            Perl_croak(aTHX_ "panic: leave_scope inconsistency %u",
                    (U8)uv & SAVE_MASK);
        }
    }

    TAINT_set(was);
}

void
Perl_cx_dump(pTHX_ PERL_CONTEXT *cx)
{
    PERL_ARGS_ASSERT_CX_DUMP;

#ifdef DEBUGGING
    PerlIO_printf(Perl_debug_log, "CX %ld = %s\n", (long)(cx - cxstack), PL_block_type[CxTYPE(cx)]);
    if (CxTYPE(cx) != CXt_SUBST) {
        const char *gimme_text;
        PerlIO_printf(Perl_debug_log, "BLK_OLDSP = %ld\n", (long)cx->blk_oldsp);
        PerlIO_printf(Perl_debug_log, "BLK_OLDCOP = 0x%" UVxf "\n",
                      PTR2UV(cx->blk_oldcop));
        PerlIO_printf(Perl_debug_log, "BLK_OLDMARKSP = %ld\n", (long)cx->blk_oldmarksp);
        PerlIO_printf(Perl_debug_log, "BLK_OLDSCOPESP = %ld\n", (long)cx->blk_oldscopesp);
        PerlIO_printf(Perl_debug_log, "BLK_OLDSAVEIX = %ld\n", (long)cx->blk_oldsaveix);
        PerlIO_printf(Perl_debug_log, "BLK_OLDPM = 0x%" UVxf "\n",
                      PTR2UV(cx->blk_oldpm));
        switch (cx->blk_gimme) {
            case G_VOID:
                gimme_text = "VOID";
                break;
            case G_SCALAR:
                gimme_text = "SCALAR";
                break;
            case G_LIST:
                gimme_text = "LIST";
                break;
            default:
                gimme_text = "UNKNOWN";
                break;
        }
        PerlIO_printf(Perl_debug_log, "BLK_GIMME = %s\n", gimme_text);
    }
    switch (CxTYPE(cx)) {
    case CXt_NULL:
    case CXt_BLOCK:
    case CXt_DEFER:
        break;
    case CXt_FORMAT:
        PerlIO_printf(Perl_debug_log, "BLK_FORMAT.CV = 0x%" UVxf "\n",
                PTR2UV(cx->blk_format.cv));
        PerlIO_printf(Perl_debug_log, "BLK_FORMAT.GV = 0x%" UVxf "\n",
                PTR2UV(cx->blk_format.gv));
        PerlIO_printf(Perl_debug_log, "BLK_FORMAT.DFOUTGV = 0x%" UVxf "\n",
                PTR2UV(cx->blk_format.dfoutgv));
        PerlIO_printf(Perl_debug_log, "BLK_FORMAT.HASARGS = %d\n",
                      (int)CxHASARGS(cx));
        PerlIO_printf(Perl_debug_log, "BLK_FORMAT.RETOP = 0x%" UVxf "\n",
                PTR2UV(cx->blk_format.retop));
        break;
    case CXt_SUB:
        PerlIO_printf(Perl_debug_log, "BLK_SUB.CV = 0x%" UVxf "\n",
                PTR2UV(cx->blk_sub.cv));
        PerlIO_printf(Perl_debug_log, "BLK_SUB.OLDDEPTH = %ld\n",
                (long)cx->blk_sub.olddepth);
        PerlIO_printf(Perl_debug_log, "BLK_SUB.HASARGS = %d\n",
                (int)CxHASARGS(cx));
        PerlIO_printf(Perl_debug_log, "BLK_SUB.LVAL = %d\n", (int)CxLVAL(cx));
        PerlIO_printf(Perl_debug_log, "BLK_SUB.RETOP = 0x%" UVxf "\n",
                PTR2UV(cx->blk_sub.retop));
        break;
    case CXt_EVAL:
        PerlIO_printf(Perl_debug_log, "BLK_EVAL.OLD_IN_EVAL = %ld\n",
                (long)CxOLD_IN_EVAL(cx));
        PerlIO_printf(Perl_debug_log, "BLK_EVAL.OLD_OP_TYPE = %s (%s)\n",
                PL_op_name[CxOLD_OP_TYPE(cx)],
                PL_op_desc[CxOLD_OP_TYPE(cx)]);
        if (cx->blk_eval.old_namesv)
            PerlIO_printf(Perl_debug_log, "BLK_EVAL.OLD_NAME = %s\n",
                          SvPVX_const(cx->blk_eval.old_namesv));
        PerlIO_printf(Perl_debug_log, "BLK_EVAL.OLD_EVAL_ROOT = 0x%" UVxf "\n",
                PTR2UV(cx->blk_eval.old_eval_root));
        PerlIO_printf(Perl_debug_log, "BLK_EVAL.RETOP = 0x%" UVxf "\n",
                PTR2UV(cx->blk_eval.retop));
        break;

    case CXt_LOOP_PLAIN:
    case CXt_LOOP_LAZYIV:
    case CXt_LOOP_LAZYSV:
    case CXt_LOOP_LIST:
    case CXt_LOOP_ARY:
        PerlIO_printf(Perl_debug_log, "BLK_LOOP.LABEL = %s\n", CxLABEL(cx));
        PerlIO_printf(Perl_debug_log, "BLK_LOOP.MY_OP = 0x%" UVxf "\n",
                PTR2UV(cx->blk_loop.my_op));
        if (CxTYPE(cx) != CXt_LOOP_PLAIN) {
            PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERVAR = 0x%" UVxf "\n",
                    PTR2UV(CxITERVAR(cx)));
            PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERSAVE = 0x%" UVxf "\n",
                    PTR2UV(cx->blk_loop.itersave));
        }
        if (CxTYPE(cx) == CXt_LOOP_ARY) {
            PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERARY = 0x%" UVxf "\n",
                    PTR2UV(cx->blk_loop.state_u.ary.ary));
            PerlIO_printf(Perl_debug_log, "BLK_LOOP.ITERIX = %ld\n",
                    (long)cx->blk_loop.state_u.ary.ix);
        }
        break;

    case CXt_SUBST:
        PerlIO_printf(Perl_debug_log, "SB_ITERS = %ld\n",
                (long)cx->sb_iters);
        PerlIO_printf(Perl_debug_log, "SB_MAXITERS = %ld\n",
                (long)cx->sb_maxiters);
        PerlIO_printf(Perl_debug_log, "SB_RFLAGS = %ld\n",
                (long)cx->sb_rflags);
        PerlIO_printf(Perl_debug_log, "SB_ONCE = %ld\n",
                (long)CxONCE(cx));
        PerlIO_printf(Perl_debug_log, "SB_ORIG = %s\n",
                cx->sb_orig);
        PerlIO_printf(Perl_debug_log, "SB_DSTR = 0x%" UVxf "\n",
                PTR2UV(cx->sb_dstr));
        PerlIO_printf(Perl_debug_log, "SB_TARG = 0x%" UVxf "\n",
                PTR2UV(cx->sb_targ));
        PerlIO_printf(Perl_debug_log, "SB_S = 0x%" UVxf "\n",
                PTR2UV(cx->sb_s));
        PerlIO_printf(Perl_debug_log, "SB_M = 0x%" UVxf "\n",
                PTR2UV(cx->sb_m));
        PerlIO_printf(Perl_debug_log, "SB_STREND = 0x%" UVxf "\n",
                PTR2UV(cx->sb_strend));
        PerlIO_printf(Perl_debug_log, "SB_RXRES = 0x%" UVxf "\n",
                PTR2UV(cx->sb_rxres));
        break;
    }
#else
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(cx);
#endif	/* DEBUGGING */
}

/*
=for apidoc_section $callback
=for apidoc mortal_destructor_sv

This function arranges for either a Perl code reference, or a C function
reference to be called at the B<end of the current statement>.

The C<coderef> argument determines the type of function that will be
called. If it is C<SvROK()> it is assumed to be a reference to a CV and
will arrange for the coderef to be called. If it is not SvROK() then it
is assumed to be a C<SvIV()> which is C<SvIOK()> whose value is a pointer
to a C function of type C<DESTRUCTORFUNC_t> created using C<PTR2INT()>.
Either way the C<args> parameter will be provided to the callback as a
parameter, although the rules for doing so differ between the Perl and
C mode. Normally this function is only used directly for the Perl case
and the wrapper C<mortal_destructor_x()> is used for the C function case.

When operating in Perl callback mode the C<args> parameter may be NULL
in which case the code reference is called with no arguments, otherwise
if it is an AV (SvTYPE(args) == SVt_PVAV) then the contents of the AV
will be used as the arguments to the code reference, and if it is any
other type then the C<args> SV will be provided as a single argument to
the code reference.

When operating in a C callback mode the C<args> parameter will be passed
directly to the C function as a C<void *> pointer. No additional
processing of the argument will be peformed, and it is the callers
responsibility to free the C<args> parameter if necessary.

Be aware that there is a signficant difference in timing between the
I<end of the current statement> and the I<end of the current pseudo
block>. If you are looking for a mechanism to trigger a function at the
end of the B<current pseudo block> you should look at
C<SAVEDESTRUCTORX()> instead of this function.

=for apidoc mortal_svfunc_x

This function arranges for a C function reference to be called at the
B<end of the current statement> with the arguments provided. It is a
wrapper around C<mortal_destructor_sv()> which ensures that the latter
function is called appropriately.

Be aware that there is a signficant difference in timing between the
I<end of the current statement> and the I<end of the current pseudo
block>. If you are looking for a mechanism to trigger a function at the
end of the B<current pseudo block> you should look at
C<SAVEDESTRUCTORX()> instead of this function.

=for apidoc magic_freedestruct

This function is called via magic to implement the
C<mortal_destructor_sv()> and C<mortal_destructor_x()> functions. It
should not be called directly and has no user servicable parts.

=cut
*/

void
Perl_mortal_destructor_sv(pTHX_ SV *coderef, SV *args) {
    PERL_ARGS_ASSERT_MORTAL_DESTRUCTOR_SV;
    assert(
        (SvROK(coderef) && SvTYPE(SvRV(coderef)) == SVt_PVCV) /* perl coderef */
         ||
        (SvIOK(coderef) && !SvROK(coderef))                  /* C function ref */
    );
    SV *variable = newSV_type_mortal(SVt_IV);
    (void)sv_magicext(variable, coderef, PERL_MAGIC_destruct,
                      &PL_vtbl_destruct, (char *)args, args ? HEf_SVKEY : 0);
}


void
Perl_mortal_svfunc_x(pTHX_ SVFUNC_t f, SV *sv) {
    PERL_ARGS_ASSERT_MORTAL_SVFUNC_X;
    SV *sviv = newSViv(PTR2IV(f));
    mortal_destructor_sv(sviv,sv);
}


int
Perl_magic_freedestruct(pTHX_ SV* sv, MAGIC* mg) {
    PERL_ARGS_ASSERT_MAGIC_FREEDESTRUCT;
    dSP;
    union {
        SV   *sv;
        AV   *av;
        char *pv;
    } args_any;
    SV *coderef;

    IV nargs = 0;
    if (PL_phase == PERL_PHASE_DESTRUCT) {
        Perl_warn(aTHX_ "Can't call destructor for 0x%p in global destruction\n", sv);
        return 1;
    }

    args_any.pv = mg->mg_ptr;
    coderef = mg->mg_obj;

    /* Deal with C function destructor */
    if (SvTYPE(coderef) == SVt_IV && !SvROK(coderef)) {
        SVFUNC_t f = INT2PTR(SVFUNC_t, SvIV(coderef));
        (f)(aTHX_ args_any.sv);
        return 0;
    }

    if (args_any.sv) {
        if (SvTYPE(args_any.sv) == SVt_PVAV) {
            nargs = av_len(args_any.av) + 1;
        } else {
            nargs = 1;
        }
    }
    PUSHSTACKi(PERLSI_MAGIC);
    ENTER_with_name("call_freedestruct");
    SAVETMPS;
    EXTEND(SP, nargs);
    PUSHMARK(SP);
    if (args_any.sv) {
        if (SvTYPE(args_any.sv) == SVt_PVAV) {
            IV n;
            for (n = 0 ; n < nargs ; n++ ) {
                SV **argp = av_fetch(args_any.av, n, 0);
                if (argp && *argp)
                    PUSHs(*argp);
            }
        } else {
            PUSHs(args_any.sv);
        }
    }
    PUTBACK;
    (void)call_sv(coderef, G_VOID | G_EVAL | G_KEEPERR);
    FREETMPS;
    LEAVE_with_name("call_freedestruct");
    POPSTACK;
    return 0;
}


/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
