/*    av.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * '...for the Entwives desired order, and plenty, and peace (by which they
 *  meant that things should remain where they had set them).' --Treebeard
 *
 *     [p.476 of _The Lord of the Rings_, III/iv: "Treebeard"]
 */

#include "EXTERN.h"
#define PERL_IN_AV_C
#include "perl.h"

void
Perl_av_reify(pTHX_ AV *av)
{
    SSize_t key;

    PERL_ARGS_ASSERT_AV_REIFY;
    assert(SvTYPE(av) == SVt_PVAV);

    if (AvREAL(av))
        return;
#ifdef DEBUGGING
    if (SvTIED_mg((const SV *)av, PERL_MAGIC_tied))
        Perl_ck_warner_d(aTHX_ packWARN(WARN_DEBUGGING), "av_reify called on tied array");
#endif
    key = AvMAX(av) + 1;
    while (key > AvFILLp(av) + 1)
        AvARRAY(av)[--key] = NULL;
    while (key) {
        SV * const sv = AvARRAY(av)[--key];
        if (sv != &PL_sv_undef)
            SvREFCNT_inc_simple_void(sv);
    }
    key = AvARRAY(av) - AvALLOC(av);
    while (key)
        AvALLOC(av)[--key] = NULL;
    AvREIFY_off(av);
    AvREAL_on(av);
}

/*
=for apidoc av_extend

Pre-extend an array so that it is capable of storing values at indexes
C<0..key>. Thus C<av_extend(av,99)> guarantees that the array can store 100
elements, i.e. that C<av_store(av, 0, sv)> through C<av_store(av, 99, sv)>
on a plain array will work without any further memory allocation.

If the av argument is a tied array then will call the C<EXTEND> tied
array method with an argument of C<(key+1)>.

=cut
*/

void
Perl_av_extend(pTHX_ AV *av, SSize_t key)
{
    MAGIC *mg;

    PERL_ARGS_ASSERT_AV_EXTEND;
    assert(SvTYPE(av) == SVt_PVAV);

    mg = SvTIED_mg((const SV *)av, PERL_MAGIC_tied);
    if (mg) {
        SV *arg1 = sv_newmortal();
        /* NOTE: the API for av_extend() is NOT the same as the tie method EXTEND.
         *
         * The C function takes an *index* (assumes 0 indexed arrays) and ensures
         * that the array is at least as large as the index provided.
         *
         * The tied array method EXTEND takes a *count* and ensures that the array
         * is at least that many elements large. Thus we have to +1 the key when
         * we call the tied method.
         */
        sv_setiv(arg1, (IV)(key + 1));
        Perl_magic_methcall(aTHX_ MUTABLE_SV(av), mg, SV_CONST(EXTEND), G_DISCARD, 1,
                            arg1);
        return;
    }
    av_extend_guts(av,key,&AvMAX(av),&AvALLOC(av),&AvARRAY(av));
}    

/* The guts of av_extend.  *Not* for general use! */
/* Also called directly from pp_assign, padlist_store, padnamelist_store */
void
Perl_av_extend_guts(pTHX_ AV *av, SSize_t key, SSize_t *maxp, SV ***allocp,
                      SV ***arrayp)
{
    PERL_ARGS_ASSERT_AV_EXTEND_GUTS;

    if (key < -1) /* -1 is legal */
        Perl_croak(aTHX_
            "panic: av_extend_guts() negative count (%" IVdf ")", (IV)key);

    if (key > *maxp) {
        SSize_t ary_offset = *maxp + 1;
        SSize_t to_null = 0;
        SSize_t newmax  = 0;

        if (av && *allocp != *arrayp) { /* a shifted SV* array exists */
            to_null = *arrayp - *allocp;
            *maxp += to_null;
            ary_offset = AvFILLp(av) + 1;

            Move(*arrayp, *allocp, AvFILLp(av)+1, SV*);

            if (key > *maxp - 10) {
                newmax = key + *maxp;
                goto resize;
            }
        } else if (*allocp) { /* a full SV* array exists */

#ifdef Perl_safesysmalloc_size
            /* Whilst it would be quite possible to move this logic around
               (as I did in the SV code), so as to set AvMAX(av) early,
               based on calling Perl_safesysmalloc_size() immediately after
               allocation, I'm not convinced that it is a great idea here.
               In an array we have to loop round setting everything to
               NULL, which means writing to memory, potentially lots
               of it, whereas for the SV buffer case we don't touch the
               "bonus" memory. So there there is no cost in telling the
               world about it, whereas here we have to do work before we can
               tell the world about it, and that work involves writing to
               memory that might never be read. So, I feel, better to keep
               the current lazy system of only writing to it if our caller
               has a need for more space. NWC  */
            newmax = Perl_safesysmalloc_size((void*)*allocp) /
                sizeof(const SV *) - 1;

            if (key <= newmax)
                goto resized;
#endif 
            /* overflow-safe version of newmax = key + *maxp/5 */
            newmax = *maxp / 5;
            newmax = (key > SSize_t_MAX - newmax)
                        ? SSize_t_MAX : key + newmax;
          resize:
        {
          /* it should really be newmax+1 here, but if newmax
           * happens to equal SSize_t_MAX, then newmax+1 is
           * undefined. This means technically we croak one
           * index lower than we should in theory; in practice
           * its unlikely the system has SSize_t_MAX/sizeof(SV*)
           * bytes to spare! */
          MEM_WRAP_CHECK_s(newmax, SV*, "Out of memory during array extend");
        }
#ifdef STRESS_REALLOC
            {
                SV ** const old_alloc = *allocp;
                Newx(*allocp, newmax+1, SV*);
                Copy(old_alloc, *allocp, *maxp + 1, SV*);
                Safefree(old_alloc);
            }
#else
            Renew(*allocp,newmax+1, SV*);
#endif
#ifdef Perl_safesysmalloc_size
          resized:
#endif
            to_null += newmax - *maxp;
            *maxp = newmax;

            /* See GH#18014 for discussion of when this might be needed: */
            if (av == PL_curstack) { /* Oops, grew stack (via av_store()?) */
                PL_stack_sp = *allocp + (PL_stack_sp - PL_stack_base);
                PL_stack_base = *allocp;
                PL_stack_max = PL_stack_base + newmax;
            }
        } else { /* there is no SV* array yet */
            *maxp = key < PERL_ARRAY_NEW_MIN_KEY ?
                          PERL_ARRAY_NEW_MIN_KEY : key;
            {
                /* see comment above about newmax+1*/
                MEM_WRAP_CHECK_s(*maxp, SV*,
                                 "Out of memory during array extend");
            }
            /* Newxz isn't used below because testing showed it to be slower
             * than Newx+Zero (also slower than Newx + the previous while
             * loop) for small arrays, which are very common in perl. */
            Newx(*allocp, *maxp+1, SV*);
            /* Stacks require only the first element to be &PL_sv_undef
             * (set elsewhere). However, since non-stack AVs are likely
             * to dominate in modern production applications, stacks
             * don't get any special treatment here.
             * See https://github.com/Perl/perl5/pull/18690 for more detail */
            ary_offset = 0;
            to_null = *maxp+1;
            goto zero;
        }

        if (av && AvREAL(av)) {
          zero:
            Zero(*allocp + ary_offset,to_null,SV*);
        }

        *arrayp = *allocp;
    }
}

/*
=for apidoc av_fetch

Returns the SV at the specified index in the array.  The C<key> is the
index.  If C<lval> is true, you are guaranteed to get a real SV back (in case
it wasn't real before), which you can then modify.  Check that the return
value is non-NULL before dereferencing it to a C<SV*>.

See L<perlguts/"Understanding the Magic of Tied Hashes and Arrays"> for
more information on how to use this function on tied arrays. 

The rough perl equivalent is C<$myarray[$key]>.

=cut
*/

static bool
S_adjust_index(pTHX_ AV *av, const MAGIC *mg, SSize_t *keyp)
{
    bool adjust_index = 1;
    if (mg) {
        /* Handle negative array indices 20020222 MJD */
        SV * const ref = SvTIED_obj(MUTABLE_SV(av), mg);
        SvGETMAGIC(ref);
        if (SvROK(ref) && SvOBJECT(SvRV(ref))) {
            SV * const * const negative_indices_glob =
                hv_fetchs(SvSTASH(SvRV(ref)), NEGATIVE_INDICES_VAR, 0);

            if (negative_indices_glob && isGV(*negative_indices_glob)
             && SvTRUE(GvSV(*negative_indices_glob)))
                adjust_index = 0;
        }
    }

    if (adjust_index) {
        *keyp += AvFILL(av) + 1;
        if (*keyp < 0)
            return FALSE;
    }
    return TRUE;
}

SV**
Perl_av_fetch(pTHX_ AV *av, SSize_t key, I32 lval)
{
    SSize_t neg;
    SSize_t size;

    PERL_ARGS_ASSERT_AV_FETCH;
    assert(SvTYPE(av) == SVt_PVAV);

    if (UNLIKELY(SvRMAGICAL(av))) {
        const MAGIC * const tied_magic
            = mg_find((const SV *)av, PERL_MAGIC_tied);
        if (tied_magic || mg_find((const SV *)av, PERL_MAGIC_regdata)) {
            SV *sv;
            if (key < 0) {
                if (!S_adjust_index(aTHX_ av, tied_magic, &key))
                        return NULL;
            }

            sv = newSV_type_mortal(SVt_PVLV);
            mg_copy(MUTABLE_SV(av), sv, 0, key);
            if (!tied_magic) /* for regdata, force leavesub to make copies */
                SvTEMP_off(sv);
            LvTYPE(sv) = 't';
            LvTARG(sv) = sv; /* fake (SV**) */
            return &(LvTARG(sv));
        }
    }

    neg  = (key < 0);
    size = AvFILLp(av) + 1;
    key += neg * size; /* handle negative index without using branch */

    /* the cast from SSize_t to Size_t allows both (key < 0) and (key >= size)
     * to be tested as a single condition */
    if ((Size_t)key >= (Size_t)size) {
        if (UNLIKELY(neg))
            return NULL;
        goto emptiness;
    }

    if (!AvARRAY(av)[key]) {
      emptiness:
        return lval ? av_store(av,key,newSV_type(SVt_NULL)) : NULL;
    }

    return &AvARRAY(av)[key];
}

/*
=for apidoc av_store

Stores an SV in an array.  The array index is specified as C<key>.  The
return value will be C<NULL> if the operation failed or if the value did not
need to be actually stored within the array (as in the case of tied
arrays).  Otherwise, it can be dereferenced
to get the C<SV*> that was stored
there (= C<val>)).

Note that the caller is responsible for suitably incrementing the reference
count of C<val> before the call, and decrementing it if the function
returned C<NULL>.

Approximate Perl equivalent: C<splice(@myarray, $key, 1, $val)>.

See L<perlguts/"Understanding the Magic of Tied Hashes and Arrays"> for
more information on how to use this function on tied arrays.

=cut
*/

SV**
Perl_av_store(pTHX_ AV *av, SSize_t key, SV *val)
{
    SV** ary;

    PERL_ARGS_ASSERT_AV_STORE;
    assert(SvTYPE(av) == SVt_PVAV);

    /* S_regclass relies on being able to pass in a NULL sv
       (unicode_alternate may be NULL).
    */

    if (SvRMAGICAL(av)) {
        const MAGIC * const tied_magic = mg_find((const SV *)av, PERL_MAGIC_tied);
        if (tied_magic) {
            if (key < 0) {
                if (!S_adjust_index(aTHX_ av, tied_magic, &key))
                        return 0;
            }
            if (val) {
                mg_copy(MUTABLE_SV(av), val, 0, key);
            }
            return NULL;
        }
    }


    if (key < 0) {
        key += AvFILL(av) + 1;
        if (key < 0)
            return NULL;
    }

    if (SvREADONLY(av) && key >= AvFILL(av))
        Perl_croak_no_modify();

    if (!AvREAL(av) && AvREIFY(av))
        av_reify(av);
    if (key > AvMAX(av))
        av_extend(av,key);
    ary = AvARRAY(av);
    if (AvFILLp(av) < key) {
        if (!AvREAL(av)) {
            if (av == PL_curstack && key > PL_stack_sp - PL_stack_base)
                PL_stack_sp = PL_stack_base + key;	/* XPUSH in disguise */
            do {
                ary[++AvFILLp(av)] = NULL;
            } while (AvFILLp(av) < key);
        }
        AvFILLp(av) = key;
    }
    else if (AvREAL(av))
        SvREFCNT_dec(ary[key]);

    /* store the val into the AV before we call magic so that the magic can
     * "see" the new value. Especially set magic on the AV itself. */
    ary[key] = val;

    if (SvSMAGICAL(av)) {
        const MAGIC *mg = SvMAGIC(av);
        bool set = TRUE;
        /* We have to increment the refcount on val before we call any magic,
         * as it is now stored in the AV (just before this block), we will
         * then call the magic handlers which might die/Perl_croak, and
         * longjmp up the stack to the most recent exception trap. Which means
         * the caller code that would be expected to handle the refcount
         * increment likely would never be executed, leading to a double free.
         * This can happen in a case like
         *
         * @ary = (1);
         *
         * or this:
         *
         * if (av_store(av,n,sv)) SvREFCNT_inc(sv);
         *
         * where @ary/av has set magic applied to it which can die. In the
         * first case the sv representing 1 would be mortalized, so when the
         * set magic threw an exception it would be freed as part of the
         * normal stack unwind. However this leaves the av structure still
         * holding a valid visible pointer to the now freed value. In practice
         * the next SV created will reuse the same reference, but without the
         * refcount to account for the previous ownership and we end up with
         * warnings about a totally different variable being double freed in
         * the form of "attempt to free unreferenced variable"
         * warnings/errors.
         *
         * https://github.com/Perl/perl5/issues/20675
         *
         * Arguably the API for av_store is broken in the face of magic. Instead
         * av_store should be responsible for the refcount increment, and only
         * not do it when specifically told to do so (eg, when storing an
         * otherwise unreferenced scalar into an AV).
         */
        SvREFCNT_inc(val);  /* see comment above */
        for (; mg; mg = mg->mg_moremagic) {
          if (!isUPPER(mg->mg_type)) continue;
          if (val) {
            sv_magic(val, MUTABLE_SV(av), toLOWER(mg->mg_type), 0, key);
          }
          if (PL_delaymagic && mg->mg_type == PERL_MAGIC_isa) {
            PL_delaymagic |= DM_ARRAY_ISA;
            set = FALSE;
          }
        }
        if (set)
           mg_set(MUTABLE_SV(av));
        /* And now we are done the magic, we have to decrement it back as the av_store() api
         * says the caller is responsible for the refcount increment, assuming
         * av_store returns true. */
        SvREFCNT_dec(val);
    }
    return &ary[key];
}

/*
=for apidoc av_make

Creates a new AV and populates it with a list (C<**strp>, length C<size>) of
SVs.  A copy is made of each SV, so their refcounts are not changed.  The new
AV will have a reference count of 1.

Perl equivalent: C<my @new_array = ($scalar1, $scalar2, $scalar3...);>

=cut
*/

AV *
Perl_av_make(pTHX_ SSize_t size, SV **strp)
{
    AV * const av = newAV();
    /* sv_upgrade does AvREAL_only()  */
    PERL_ARGS_ASSERT_AV_MAKE;
    assert(SvTYPE(av) == SVt_PVAV);

    if (size) {		/* "defined" was returning undef for size==0 anyway. */
        SV** ary;
        SSize_t i;
        SSize_t orig_ix;

        Newx(ary,size,SV*);
        AvALLOC(av) = ary;
        AvARRAY(av) = ary;
        AvMAX(av) = size - 1;
        /* avoid av being leaked if croak when calling magic below */
        EXTEND_MORTAL(1);
        PL_tmps_stack[++PL_tmps_ix] = (SV*)av;
        orig_ix = PL_tmps_ix;

        for (i = 0; i < size; i++) {
            assert (*strp);

            /* Don't let sv_setsv swipe, since our source array might
               have multiple references to the same temp scalar (e.g.
               from a list slice) */

            SvGETMAGIC(*strp); /* before newSV, in case it dies */
            AvFILLp(av)++;
            ary[i] = newSV_type(SVt_NULL);
            sv_setsv_flags(ary[i], *strp,
                           SV_DO_COW_SVSETSV|SV_NOSTEAL);
            strp++;
        }
        /* disarm av's leak guard */
        if (LIKELY(PL_tmps_ix == orig_ix))
            PL_tmps_ix--;
        else
            PL_tmps_stack[orig_ix] = &PL_sv_undef;
    }
    return av;
}

/*
=for apidoc newAVav

Creates a new AV and populates it with values copied from an existing AV.  The
new AV will have a reference count of 1, and will contain newly created SVs
copied from the original SV.  The original source will remain unchanged.

Perl equivalent: C<my @new_array = @existing_array;>

=cut
*/

AV *
Perl_newAVav(pTHX_ AV *oav)
{
    PERL_ARGS_ASSERT_NEWAVAV;

    Size_t count = av_count(oav);

    if(UNLIKELY(!oav) || count == 0)
        return newAV();

    AV *ret = newAV_alloc_x(count);

    /* avoid ret being leaked if croak when calling magic below */
    EXTEND_MORTAL(1);
    PL_tmps_stack[++PL_tmps_ix] = (SV *)ret;
    SSize_t ret_at_tmps_ix = PL_tmps_ix;

    Size_t i;
    if(LIKELY(!SvRMAGICAL(oav) && AvREAL(oav) && (SvTYPE(oav) == SVt_PVAV))) {
        for(i = 0; i < count; i++) {
            SV **svp = av_fetch_simple(oav, i, 0);
            av_push_simple(ret, svp ? newSVsv(*svp) : &PL_sv_undef);
        }
    } else {
        for(i = 0; i < count; i++) {
            SV **svp = av_fetch(oav, i, 0);
            av_push_simple(ret, svp ? newSVsv(*svp) : &PL_sv_undef);
        }
    }

    /* disarm leak guard */
    if(LIKELY(PL_tmps_ix == ret_at_tmps_ix))
        PL_tmps_ix--;
    else
        PL_tmps_stack[ret_at_tmps_ix] = &PL_sv_undef;

    return ret;
}

/*
=for apidoc newAVhv

Creates a new AV and populates it with keys and values copied from an existing
HV.  The new AV will have a reference count of 1, and will contain newly
created SVs copied from the original HV.  The original source will remain
unchanged.

Perl equivalent: C<my @new_array = %existing_hash;>

=cut
*/

AV *
Perl_newAVhv(pTHX_ HV *ohv)
{
    PERL_ARGS_ASSERT_NEWAVHV;

    if(UNLIKELY(!ohv))
        return newAV();

    bool tied = SvRMAGICAL(ohv) && mg_find(MUTABLE_SV(ohv), PERL_MAGIC_tied);

    Size_t nkeys = hv_iterinit(ohv);
    /* This number isn't perfect but it doesn't matter; it only has to be
     * close to make the initial allocation about the right size
     */
    AV *ret = newAV_alloc_xz(nkeys ? nkeys * 2 : 2);

    /* avoid ret being leaked if croak when calling magic below */
    EXTEND_MORTAL(1);
    PL_tmps_stack[++PL_tmps_ix] = (SV *)ret;
    SSize_t ret_at_tmps_ix = PL_tmps_ix;


    HE *he;
    while((he = hv_iternext(ohv))) {
        if(tied) {
            av_push_simple(ret, newSVsv(hv_iterkeysv(he)));
            av_push_simple(ret, newSVsv(hv_iterval(ohv, he)));
        }
        else {
            av_push_simple(ret, newSVhek(HeKEY_hek(he)));
            av_push_simple(ret, HeVAL(he) ? newSVsv(HeVAL(he)) : &PL_sv_undef);
        }
    }

    /* disarm leak guard */
    if(LIKELY(PL_tmps_ix == ret_at_tmps_ix))
        PL_tmps_ix--;
    else
        PL_tmps_stack[ret_at_tmps_ix] = &PL_sv_undef;

    return ret;
}

/*
=for apidoc av_clear

Frees all the elements of an array, leaving it empty.
The XS equivalent of C<@array = ()>.  See also L</av_undef>.

Note that it is possible that the actions of a destructor called directly
or indirectly by freeing an element of the array could cause the reference
count of the array itself to be reduced (e.g. by deleting an entry in the
symbol table). So it is a possibility that the AV could have been freed
(or even reallocated) on return from the call unless you hold a reference
to it.

=cut
*/

void
Perl_av_clear(pTHX_ AV *av)
{
    SSize_t extra;
    bool real;
    SSize_t orig_ix = 0;

    PERL_ARGS_ASSERT_AV_CLEAR;
    assert(SvTYPE(av) == SVt_PVAV);

#ifdef DEBUGGING
    if (SvREFCNT(av) == 0) {
        Perl_ck_warner_d(aTHX_ packWARN(WARN_DEBUGGING), "Attempt to clear deleted array");
    }
#endif

    if (SvREADONLY(av))
        Perl_croak_no_modify();

    /* Give any tie a chance to cleanup first */
    if (SvRMAGICAL(av)) {
        const MAGIC* const mg = SvMAGIC(av);
        if (PL_delaymagic && mg && mg->mg_type == PERL_MAGIC_isa)
            PL_delaymagic |= DM_ARRAY_ISA;
        else
            mg_clear(MUTABLE_SV(av)); 
    }

    if (AvMAX(av) < 0)
        return;

    if ((real = cBOOL(AvREAL(av)))) {
        SV** const ary = AvARRAY(av);
        SSize_t index = AvFILLp(av) + 1;

        /* avoid av being freed when calling destructors below */
        EXTEND_MORTAL(1);
        PL_tmps_stack[++PL_tmps_ix] = SvREFCNT_inc_simple_NN(av);
        orig_ix = PL_tmps_ix;

        while (index) {
            SV * const sv = ary[--index];
            /* undef the slot before freeing the value, because a
             * destructor might try to modify this array */
            ary[index] = NULL;
            SvREFCNT_dec(sv);
        }
    }
    extra = AvARRAY(av) - AvALLOC(av);
    if (extra) {
        AvMAX(av) += extra;
        AvARRAY(av) = AvALLOC(av);
    }
    AvFILLp(av) = -1;
    if (real) {
        /* disarm av's premature free guard */
        if (LIKELY(PL_tmps_ix == orig_ix))
            PL_tmps_ix--;
        else
            PL_tmps_stack[orig_ix] = &PL_sv_undef;
        SvREFCNT_dec_NN(av);
    }
}

/*
=for apidoc av_undef

Undefines the array. The XS equivalent of C<undef(@array)>.

As well as freeing all the elements of the array (like C<av_clear()>), this
also frees the memory used by the av to store its list of scalars.

See L</av_clear> for a note about the array possibly being invalid on
return.

=cut
*/

void
Perl_av_undef(pTHX_ AV *av)
{
    bool real;
    SSize_t orig_ix = PL_tmps_ix; /* silence bogus warning about possible uninitialized use */

    PERL_ARGS_ASSERT_AV_UNDEF;
    assert(SvTYPE(av) == SVt_PVAV);

    /* Give any tie a chance to cleanup first */
    if (SvTIED_mg((const SV *)av, PERL_MAGIC_tied)) 
        av_fill(av, -1);

    real = cBOOL(AvREAL(av));
    if (real) {
        SSize_t key = AvFILLp(av) + 1;

        /* avoid av being freed when calling destructors below */
        EXTEND_MORTAL(1);
        PL_tmps_stack[++PL_tmps_ix] = SvREFCNT_inc_simple_NN(av);
        orig_ix = PL_tmps_ix;

        while (key)
            SvREFCNT_dec(AvARRAY(av)[--key]);
    }

    Safefree(AvALLOC(av));
    AvALLOC(av) = NULL;
    AvARRAY(av) = NULL;
    AvMAX(av) = AvFILLp(av) = -1;

    if(SvRMAGICAL(av)) mg_clear(MUTABLE_SV(av));
    if (real) {
        /* disarm av's premature free guard */
        if (LIKELY(PL_tmps_ix == orig_ix))
            PL_tmps_ix--;
        else
            PL_tmps_stack[orig_ix] = &PL_sv_undef;
        SvREFCNT_dec_NN(av);
    }
}

/*

=for apidoc av_create_and_push

Push an SV onto the end of the array, creating the array if necessary.
A small internal helper function to remove a commonly duplicated idiom.

=cut
*/

void
Perl_av_create_and_push(pTHX_ AV **const avp, SV *const val)
{
    PERL_ARGS_ASSERT_AV_CREATE_AND_PUSH;

    if (!*avp)
        *avp = newAV();
    av_push(*avp, val);
}

/*
=for apidoc av_push

Pushes an SV (transferring control of one reference count) onto the end of the
array.  The array will grow automatically to accommodate the addition.

Perl equivalent: C<push @myarray, $val;>.

=cut
*/

void
Perl_av_push(pTHX_ AV *av, SV *val)
{             
    MAGIC *mg;

    PERL_ARGS_ASSERT_AV_PUSH;
    assert(SvTYPE(av) == SVt_PVAV);

    if (SvREADONLY(av))
        Perl_croak_no_modify();

    if ((mg = SvTIED_mg((const SV *)av, PERL_MAGIC_tied))) {
        Perl_magic_methcall(aTHX_ MUTABLE_SV(av), mg, SV_CONST(PUSH), G_DISCARD, 1,
                            val);
        return;
    }
    av_store(av,AvFILLp(av)+1,val);
}

/*
=for apidoc av_pop

Removes one SV from the end of the array, reducing its size by one and
returning the SV (transferring control of one reference count) to the
caller.  Returns C<&PL_sv_undef> if the array is empty.

Perl equivalent: C<pop(@myarray);>

=cut
*/

SV *
Perl_av_pop(pTHX_ AV *av)
{
    SV *retval;
    MAGIC* mg;

    PERL_ARGS_ASSERT_AV_POP;
    assert(SvTYPE(av) == SVt_PVAV);

    if (SvREADONLY(av))
        Perl_croak_no_modify();
    if ((mg = SvTIED_mg((const SV *)av, PERL_MAGIC_tied))) {
        retval = Perl_magic_methcall(aTHX_ MUTABLE_SV(av), mg, SV_CONST(POP), 0, 0);
        if (retval)
            retval = newSVsv(retval);
        return retval;
    }
    if (AvFILL(av) < 0)
        return &PL_sv_undef;
    retval = AvARRAY(av)[AvFILLp(av)];
    AvARRAY(av)[AvFILLp(av)--] = NULL;
    if (SvSMAGICAL(av))
        mg_set(MUTABLE_SV(av));
    return retval ? retval : &PL_sv_undef;
}

/*

=for apidoc av_create_and_unshift_one

Unshifts an SV onto the beginning of the array, creating the array if
necessary.
A small internal helper function to remove a commonly duplicated idiom.

=cut
*/

SV **
Perl_av_create_and_unshift_one(pTHX_ AV **const avp, SV *const val)
{
    PERL_ARGS_ASSERT_AV_CREATE_AND_UNSHIFT_ONE;

    if (!*avp)
        *avp = newAV();
    av_unshift(*avp, 1);
    return av_store(*avp, 0, val);
}

/*
=for apidoc av_unshift

Unshift the given number of C<undef> values onto the beginning of the
array.  The array will grow automatically to accommodate the addition.

Perl equivalent: S<C<unshift @myarray, ((undef) x $num);>>

=cut
*/

void
Perl_av_unshift(pTHX_ AV *av, SSize_t num)
{
    SSize_t i;
    MAGIC* mg;

    PERL_ARGS_ASSERT_AV_UNSHIFT;
    assert(SvTYPE(av) == SVt_PVAV);

    if (SvREADONLY(av))
        Perl_croak_no_modify();

    if ((mg = SvTIED_mg((const SV *)av, PERL_MAGIC_tied))) {
        Perl_magic_methcall(aTHX_ MUTABLE_SV(av), mg, SV_CONST(UNSHIFT),
                            G_DISCARD | G_UNDEF_FILL, num);
        return;
    }

    if (num <= 0)
      return;
    if (!AvREAL(av) && AvREIFY(av))
        av_reify(av);
    i = AvARRAY(av) - AvALLOC(av);
    if (i) {
        if (i > num)
            i = num;
        num -= i;
    
        AvMAX(av) += i;
        AvFILLp(av) += i;
        AvARRAY(av) = AvARRAY(av) - i;
    }
    if (num) {
        SV **ary;
        const SSize_t i = AvFILLp(av);
        /* Create extra elements */
        const SSize_t slide = i > 0 ? i : 0;
        num += slide;
        av_extend(av, i + num);
        AvFILLp(av) += num;
        ary = AvARRAY(av);
        Move(ary, ary + num, i + 1, SV*);
        do {
            ary[--num] = NULL;
        } while (num);
        /* Make extra elements into a buffer */
        AvMAX(av) -= slide;
        AvFILLp(av) -= slide;
        AvARRAY(av) = AvARRAY(av) + slide;
    }
}

/*
=for apidoc av_shift

Removes one SV from the start of the array, reducing its size by one and
returning the SV (transferring control of one reference count) to the
caller.  Returns C<&PL_sv_undef> if the array is empty.

Perl equivalent: C<shift(@myarray);>

=cut
*/

SV *
Perl_av_shift(pTHX_ AV *av)
{
    SV *retval;
    MAGIC* mg;

    PERL_ARGS_ASSERT_AV_SHIFT;
    assert(SvTYPE(av) == SVt_PVAV);

    if (SvREADONLY(av))
        Perl_croak_no_modify();
    if ((mg = SvTIED_mg((const SV *)av, PERL_MAGIC_tied))) {
        retval = Perl_magic_methcall(aTHX_ MUTABLE_SV(av), mg, SV_CONST(SHIFT), 0, 0);
        if (retval)
            retval = newSVsv(retval);
        return retval;
    }
    if (AvFILL(av) < 0)
      return &PL_sv_undef;
    retval = *AvARRAY(av);
    if (AvREAL(av))
        *AvARRAY(av) = NULL;
    AvARRAY(av) = AvARRAY(av) + 1;
    AvMAX(av)--;
    AvFILLp(av)--;
    if (SvSMAGICAL(av))
        mg_set(MUTABLE_SV(av));
    return retval ? retval : &PL_sv_undef;
}

/*
=for apidoc av_tindex
=for apidoc_item av_top_index

These behave identically.
If the array C<av> is empty, these return -1; otherwise they return the maximum
value of the indices of all the array elements which are currently defined in
C<av>.

They process 'get' magic.

The Perl equivalent for these is C<$#av>.

Use C<L</av_count>> to get the number of elements in an array.

=for apidoc av_len

Same as L</av_top_index>.  Note that, unlike what the name implies, it returns
the maximum index in the array.  This is unlike L</sv_len>, which returns what
you would expect.

B<To get the true number of elements in the array, instead use C<L</av_count>>>.

=cut
*/

SSize_t
Perl_av_len(pTHX_ AV *av)
{
    PERL_ARGS_ASSERT_AV_LEN;

    return av_top_index(av);
}

/*
=for apidoc av_fill

Set the highest index in the array to the given number, equivalent to
Perl's S<C<$#array = $fill;>>.

The number of elements in the array will be S<C<fill + 1>> after
C<av_fill()> returns.  If the array was previously shorter, then the
additional elements appended are set to NULL.  If the array
was longer, then the excess elements are freed.  S<C<av_fill(av, -1)>> is
the same as C<av_clear(av)>.

=cut
*/
void
Perl_av_fill(pTHX_ AV *av, SSize_t fill)
{
    MAGIC *mg;

    PERL_ARGS_ASSERT_AV_FILL;
    assert(SvTYPE(av) == SVt_PVAV);

    if (fill < 0)
        fill = -1;
    if ((mg = SvTIED_mg((const SV *)av, PERL_MAGIC_tied))) {
        SV *arg1 = sv_newmortal();
        sv_setiv(arg1, (IV)(fill + 1));
        Perl_magic_methcall(aTHX_ MUTABLE_SV(av), mg, SV_CONST(STORESIZE), G_DISCARD,
                            1, arg1);
        return;
    }
    if (fill <= AvMAX(av)) {
        SSize_t key = AvFILLp(av);
        SV** const ary = AvARRAY(av);

        if (AvREAL(av)) {
            while (key > fill) {
                SvREFCNT_dec(ary[key]);
                ary[key--] = NULL;
            }
        }
        else {
            while (key < fill)
                ary[++key] = NULL;
        }
            
        AvFILLp(av) = fill;
        if (SvSMAGICAL(av))
            mg_set(MUTABLE_SV(av));
    }
    else
        (void)av_store(av,fill,NULL);
}

/*
=for apidoc av_delete

Deletes the element indexed by C<key> from the array, makes the element
mortal, and returns it.  If C<flags> equals C<G_DISCARD>, the element is
freed and NULL is returned. NULL is also returned if C<key> is out of
range.

Perl equivalent: S<C<splice(@myarray, $key, 1, undef)>> (with the
C<splice> in void context if C<G_DISCARD> is present).

=cut
*/
SV *
Perl_av_delete(pTHX_ AV *av, SSize_t key, I32 flags)
{
    SV *sv;

    PERL_ARGS_ASSERT_AV_DELETE;
    assert(SvTYPE(av) == SVt_PVAV);

    if (SvREADONLY(av))
        Perl_croak_no_modify();

    if (SvRMAGICAL(av)) {
        const MAGIC * const tied_magic
            = mg_find((const SV *)av, PERL_MAGIC_tied);
        if ((tied_magic || mg_find((const SV *)av, PERL_MAGIC_regdata))) {
            SV **svp;
            if (key < 0) {
                if (!S_adjust_index(aTHX_ av, tied_magic, &key))
                        return NULL;
            }
            svp = av_fetch(av, key, TRUE);
            if (svp) {
                sv = *svp;
                mg_clear(sv);
                if (mg_find(sv, PERL_MAGIC_tiedelem)) {
                    sv_unmagic(sv, PERL_MAGIC_tiedelem); /* No longer an element */
                    return sv;
                }
                return NULL;
            }
        }
    }

    if (key < 0) {
        key += AvFILL(av) + 1;
        if (key < 0)
            return NULL;
    }

    if (key > AvFILLp(av))
        return NULL;
    else {
        if (!AvREAL(av) && AvREIFY(av))
            av_reify(av);
        sv = AvARRAY(av)[key];
        AvARRAY(av)[key] = NULL;
        if (key == AvFILLp(av)) {
            do {
                AvFILLp(av)--;
            } while (--key >= 0 && !AvARRAY(av)[key]);
        }
        if (SvSMAGICAL(av))
            mg_set(MUTABLE_SV(av));
    }
    if(sv != NULL) {
        if (flags & G_DISCARD) {
            SvREFCNT_dec_NN(sv);
            return NULL;
        }
        else if (AvREAL(av))
            sv_2mortal(sv);
    }
    return sv;
}

/*
=for apidoc av_exists

Returns true if the element indexed by C<key> has been initialized.

This relies on the fact that uninitialized array elements are set to
C<NULL>.

Perl equivalent: C<exists($myarray[$key])>.

=cut
*/
bool
Perl_av_exists(pTHX_ AV *av, SSize_t key)
{
    PERL_ARGS_ASSERT_AV_EXISTS;
    assert(SvTYPE(av) == SVt_PVAV);

    if (SvRMAGICAL(av)) {
        const MAGIC * const tied_magic
            = mg_find((const SV *)av, PERL_MAGIC_tied);
        const MAGIC * const regdata_magic
            = mg_find((const SV *)av, PERL_MAGIC_regdata);
        if (tied_magic || regdata_magic) {
            MAGIC *mg;
            /* Handle negative array indices 20020222 MJD */
            if (key < 0) {
                if (!S_adjust_index(aTHX_ av, tied_magic, &key))
                        return FALSE;
            }

            if(key >= 0 && regdata_magic) {
                if (key <= AvFILL(av))
                    return TRUE;
                else
                    return FALSE;
            }
            {
                SV * const sv = sv_newmortal();
                mg_copy(MUTABLE_SV(av), sv, 0, key);
                mg = mg_find(sv, PERL_MAGIC_tiedelem);
                if (mg) {
                    magic_existspack(sv, mg);
                    {
                        I32 retbool = SvTRUE_nomg_NN(sv);
                        return cBOOL(retbool);
                    }
                }
            }
        }
    }

    if (key < 0) {
        key += AvFILL(av) + 1;
        if (key < 0)
            return FALSE;
    }

    if (key <= AvFILLp(av) && AvARRAY(av)[key])
    {
        if (SvSMAGICAL(AvARRAY(av)[key])
         && mg_find(AvARRAY(av)[key], PERL_MAGIC_nonelem))
            return FALSE;
        return TRUE;
    }
    else
        return FALSE;
}

static MAGIC *
S_get_aux_mg(pTHX_ AV *av) {
    MAGIC *mg;

    PERL_ARGS_ASSERT_GET_AUX_MG;
    assert(SvTYPE(av) == SVt_PVAV);

    mg = mg_find((const SV *)av, PERL_MAGIC_arylen_p);

    if (!mg) {
        mg = sv_magicext(MUTABLE_SV(av), 0, PERL_MAGIC_arylen_p,
                         &PL_vtbl_arylen_p, 0, 0);
        assert(mg);
        /* sv_magicext won't set this for us because we pass in a NULL obj  */
        mg->mg_flags |= MGf_REFCOUNTED;
    }
    return mg;
}

SV **
Perl_av_arylen_p(pTHX_ AV *av) {
    MAGIC *const mg = get_aux_mg(av);

    PERL_ARGS_ASSERT_AV_ARYLEN_P;
    assert(SvTYPE(av) == SVt_PVAV);

    return &(mg->mg_obj);
}

IV *
Perl_av_iter_p(pTHX_ AV *av) {
    MAGIC *const mg = get_aux_mg(av);

    PERL_ARGS_ASSERT_AV_ITER_P;
    assert(SvTYPE(av) == SVt_PVAV);

    if (sizeof(IV) == sizeof(SSize_t)) {
        return (IV *)&(mg->mg_len);
    } else {
        if (!mg->mg_ptr) {
            IV *temp;
            mg->mg_len = IVSIZE;
            Newxz(temp, 1, IV);
            mg->mg_ptr = (char *) temp;
        }
        return (IV *)mg->mg_ptr;
    }
}

SV *
Perl_av_nonelem(pTHX_ AV *av, SSize_t ix) {
    SV * const sv = newSV_type(SVt_NULL);
    PERL_ARGS_ASSERT_AV_NONELEM;
    if (!av_store(av,ix,sv))
        return sv_2mortal(sv); /* has tie magic */
    sv_magic(sv, NULL, PERL_MAGIC_nonelem, NULL, 0);
    return sv;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
