#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

static AV*
S_mro_get_linear_isa_c3(pTHX_ HV* stash, U32 level);

static const struct mro_alg c3_alg =
    {S_mro_get_linear_isa_c3, "c3", 2, 0, 0};

/*
=for apidoc mro_get_linear_isa_c3

Returns the C3 linearization of C<@ISA>
the given stash.  The return value is a read-only AV*
whose values are string SVs giving class names.
C<level> should be 0 (it is used internally in this
function's recursion).

You are responsible for C<SvREFCNT_inc()> on the
return value if you plan to store it anywhere
semi-permanently (otherwise it might be deleted
out from under you the next time the cache is
invalidated).

=cut
*/

static AV*
S_mro_get_linear_isa_c3(pTHX_ HV* stash, U32 level)
{
    AV* retval;
    GV** gvp;
    GV* gv;
    AV* isa;
    const HEK* stashhek;
    struct mro_meta* meta;

    assert(HvAUX(stash));

    stashhek = HvENAME_HEK(stash);
    if (!stashhek) stashhek = HvNAME_HEK(stash);
    if (!stashhek)
      Perl_croak(aTHX_ "Can't linearize anonymous symbol table");

    if (level > 100)
        Perl_croak(aTHX_ "Recursive inheritance detected in package '%" HEKf
                         "'",
                          HEKfARG(stashhek));

    meta = HvMROMETA(stash);

    /* return cache if valid */
    if((retval = MUTABLE_AV(MRO_GET_PRIVATE_DATA(meta, &c3_alg)))) {
        return retval;
    }

    /* not in cache, make a new one */

    gvp = (GV**)hv_fetchs(stash, "ISA", FALSE);
    isa = (gvp && (gv = *gvp) && isGV_with_GP(gv)) ? GvAV(gv) : NULL;

    /* For a better idea how the rest of this works, see the much clearer
       pure perl version in Algorithm::C3 0.01:
       https://fastapi.metacpan.org/source/STEVAN/Algorithm-C3-0.01/lib/Algorithm/C3.pm
       (later versions of this module go about it differently than this code
       for speed reasons)
    */

    if(isa && AvFILLp(isa) >= 0) {
        SV** seqs_ptr;
        I32 seqs_items;
        HV *tails;
        AV *const seqs = MUTABLE_AV(sv_2mortal(MUTABLE_SV(newAV())));
        I32* heads;

        /* This builds @seqs, which is an array of arrays.
           The members of @seqs are the MROs of
           the members of @ISA, followed by @ISA itself.
        */
        SSize_t items = AvFILLp(isa) + 1;
        SV** isa_ptr = AvARRAY(isa);
        while(items--) {
            SV* const isa_item = *isa_ptr ? *isa_ptr : &PL_sv_undef;
            HV* const isa_item_stash = gv_stashsv(isa_item, 0);
            isa_ptr++;
            if(!isa_item_stash) {
                /* if no stash, make a temporary fake MRO
                   containing just itself */
                AV* const isa_lin = newAV();
                av_push(isa_lin, newSVsv(isa_item));
                av_push(seqs, MUTABLE_SV(isa_lin));
            }
            else {
                /* recursion */
                AV* const isa_lin
		  = S_mro_get_linear_isa_c3(aTHX_ isa_item_stash, level + 1);

		if(items == 0 && AvFILLp(seqs) == -1) {
		    /* Only one parent class. For this case, the C3
		       linearisation is this class followed by the parent's
		       linearisation, so don't bother with the expensive
		       calculation.  */
		    SV **svp;
		    I32 subrv_items = AvFILLp(isa_lin) + 1;
		    SV *const *subrv_p = AvARRAY(isa_lin);

		    /* Hijack the allocated but unused array seqs to be the
		       return value. It's currently mortalised.  */

		    retval = seqs;

		    av_extend(retval, subrv_items);
		    AvFILLp(retval) = subrv_items;
		    svp = AvARRAY(retval);

		    /* First entry is this class.  We happen to make a shared
		       hash key scalar because it's the cheapest and fastest
		       way to do it.  */
		    *svp++ = newSVhek(stashhek);

		    while(subrv_items--) {
			/* These values are unlikely to be shared hash key
			   scalars, so no point in adding code to optimising
			   for a case that is unlikely to be true.
			   (Or prove me wrong and do it.)  */

			SV *const val = *subrv_p++;
			*svp++ = newSVsv(val);
		    }

		    SvREFCNT_inc(retval);

		    goto done;
		}
                av_push(seqs, SvREFCNT_inc_simple_NN(MUTABLE_SV(isa_lin)));
            }
        }
        av_push(seqs, SvREFCNT_inc_simple_NN(MUTABLE_SV(isa)));
	tails = MUTABLE_HV(sv_2mortal(MUTABLE_SV(newHV())));

        /* This builds "heads", which as an array of integer array
           indices, one per seq, which point at the virtual "head"
           of the seq (initially zero) */
        Newxz(heads, AvFILLp(seqs)+1, I32);

        /* This builds %tails, which has one key for every class
           mentioned in the tail of any sequence in @seqs (tail meaning
           everything after the first class, the "head").  The value
           is how many times this key appears in the tails of @seqs.
        */
        seqs_ptr = AvARRAY(seqs);
        seqs_items = AvFILLp(seqs) + 1;
        while(seqs_items--) {
            AV *const seq = MUTABLE_AV(*seqs_ptr++);
            I32 seq_items = AvFILLp(seq);
            if(seq_items > 0) {
                SV** seq_ptr = AvARRAY(seq) + 1;
                while(seq_items--) {
                    SV* const seqitem = *seq_ptr++;
		    /* LVALUE fetch will create a new undefined SV if necessary
		     */
                    HE* const he = hv_fetch_ent(tails, seqitem, 1, 0);
                    if(he) {
                        sv_inc_nomg(HeVAL(he));
                    }
                }
            }
        }

        /* Initialize retval to build the return value in */
        retval = newAV();
        av_push(retval, newSVhek(stashhek)); /* us first */

        /* This loop won't terminate until we either finish building
           the MRO, or get an exception. */
        while(1) {
            SV* cand = NULL;
            SV* winner = NULL;
            int s;

            /* "foreach $seq (@seqs)" */
            SV** const avptr = AvARRAY(seqs);
            for(s = 0; s <= AvFILLp(seqs); s++) {
                SV** svp;
                AV * const seq = MUTABLE_AV(avptr[s]);
		SV* seqhead;
                if(!seq) continue; /* skip empty seqs */
                svp = av_fetch(seq, heads[s], 0);
                seqhead = *svp; /* seqhead = head of this seq */
                if(!winner) {
		    HE* tail_entry;
		    SV* val;
                    /* if we haven't found a winner for this round yet,
                       and this seqhead is not in tails (or the count
                       for it in tails has dropped to zero), then this
                       seqhead is our new winner, and is added to the
                       final MRO immediately */
                    cand = seqhead;
                    if((tail_entry = hv_fetch_ent(tails, cand, 0, 0))
                       && (val = HeVAL(tail_entry))
                       && (SvIVX(val) > 0))
                           continue;
                    winner = newSVsv(cand);
                    av_push(retval, winner);
                    /* note however that even when we find a winner,
                       we continue looping over @seqs to do housekeeping */
                }
                if(!sv_cmp(seqhead, winner)) {
                    /* Once we have a winner (including the iteration
                       where we first found him), inc the head ptr
                       for any seq which had the winner as a head,
                       NULL out any seq which is now empty,
                       and adjust tails for consistency */

                    const int new_head = ++heads[s];
                    if(new_head > AvFILLp(seq)) {
                        SvREFCNT_dec(avptr[s]);
                        avptr[s] = NULL;
                    }
                    else {
			HE* tail_entry;
			SV* val;
                        /* Because we know this new seqhead used to be
                           a tail, we can assume it is in tails and has
                           a positive value, which we need to dec */
                        svp = av_fetch(seq, new_head, 0);
                        seqhead = *svp;
                        tail_entry = hv_fetch_ent(tails, seqhead, 0, 0);
                        val = HeVAL(tail_entry);
                        sv_dec(val);
                    }
                }
            }

            /* if we found no candidates, we are done building the MRO.
               !cand means no seqs have any entries left to check */
            if(!cand) {
                Safefree(heads);
                break;
            }

            /* If we had candidates, but nobody won, then the @ISA
               hierarchy is not C3-incompatible */
            if(!winner) {
                SV *errmsg;
                Size_t i;

                errmsg = newSVpvf(
                           "Inconsistent hierarchy during C3 merge of class '%" HEKf "':\n\t"
                            "current merge results [\n",
                            HEKfARG(stashhek));
                for (i = 0; i < av_count(retval); i++) {
                    SV **elem = av_fetch(retval, i, 0);
                    sv_catpvf(errmsg, "\t\t%" SVf ",\n", SVfARG(*elem));
                }
                sv_catpvf(errmsg, "\t]\n\tmerging failed on '%" SVf "'", SVfARG(cand));

                /* we have to do some cleanup before we croak */

                SvREFCNT_dec(retval);
                Safefree(heads);

                Perl_croak(aTHX_ "%" SVf, SVfARG(errmsg));
            }
        }
    }
    else { /* @ISA was undefined or empty */
        /* build a retval containing only ourselves */
        retval = newAV();
        av_push(retval, newSVhek(stashhek));
    }

 done:
    /* we don't want anyone modifying the cache entry but us,
       and we do so by replacing it completely */
    SvREADONLY_on(retval);

    return MUTABLE_AV(Perl_mro_set_private_data(aTHX_ meta, &c3_alg,
						MUTABLE_SV(retval)));
}


/* These two are static helpers for next::method and friends,
   and re-implement a bunch of the code from pp_caller() in
   a more efficient manner for this particular usage.
*/

static I32
__dopoptosub_at(const PERL_CONTEXT *cxstk, I32 startingblock) {
    I32 i;
    for (i = startingblock; i >= 0; i--) {
        if(CxTYPE((PERL_CONTEXT*)(&cxstk[i])) == CXt_SUB) return i;
    }
    return i;
}

MODULE = mro		PACKAGE = mro		PREFIX = mro_

void
mro_get_linear_isa(...)
  PROTOTYPE: $;$
  PREINIT:
    AV* RETVAL;
    HV* class_stash;
    SV* classname;
  PPCODE:
    if(items < 1 || items > 2)
	croak_xs_usage(cv, "classname [, type ]");

    classname = ST(0);
    class_stash = gv_stashsv(classname, 0);

    if(!class_stash) {
        /* No stash exists yet, give them just the classname */
        AV* isalin = newAV();
        av_push(isalin, newSVsv(classname));
        ST(0) = sv_2mortal(newRV_noinc(MUTABLE_SV(isalin)));
        XSRETURN(1);
    }
    else if(items > 1) {
	const struct mro_alg *const algo = Perl_mro_get_from_name(aTHX_ ST(1));
	if (!algo)
	    Perl_croak(aTHX_ "Invalid mro name: '%" SVf "'", ST(1));
	RETVAL = algo->resolve(aTHX_ class_stash, 0);
    }
    else {
        RETVAL = mro_get_linear_isa(class_stash);
    }
    ST(0) = newRV_inc(MUTABLE_SV(RETVAL));
    sv_2mortal(ST(0));
    XSRETURN(1);

void
mro_set_mro(...)
  PROTOTYPE: $$
  PREINIT:
    SV* classname;
    HV* class_stash;
    struct mro_meta* meta;
  PPCODE:
    if (items != 2)
	croak_xs_usage(cv, "classname, type");

    classname = ST(0);
    class_stash = gv_stashsv(classname, GV_ADD);
    if(!class_stash) Perl_croak(aTHX_ "Cannot create class: '%" SVf "'!", SVfARG(classname));
    meta = HvMROMETA(class_stash);

    Perl_mro_set_mro(aTHX_ meta, ST(1));

    XSRETURN_EMPTY;

void
mro_get_mro(...)
  PROTOTYPE: $
  PREINIT:
    SV* classname;
    HV* class_stash;
  PPCODE:
    if (items != 1)
	croak_xs_usage(cv, "classname");

    classname = ST(0);
    class_stash = gv_stashsv(classname, 0);

    if (class_stash) {
        const struct mro_alg *const meta = HvMROMETA(class_stash)->mro_which;
 	ST(0) = newSVpvn_flags(meta->name, meta->length,
			       SVs_TEMP
			       | ((meta->kflags & HVhek_UTF8) ? SVf_UTF8 : 0));
    } else {
      ST(0) = newSVpvn_flags("dfs", 3, SVs_TEMP);
    }
    XSRETURN(1);

void
mro_get_isarev(...)
  PROTOTYPE: $
  PREINIT:
    SV* classname;
    HE* he;
    HV* isarev;
    AV* ret_array;
  PPCODE:
    if (items != 1)
	croak_xs_usage(cv, "classname");

    classname = ST(0);

    he = hv_fetch_ent(PL_isarev, classname, 0, 0);
    isarev = he ? MUTABLE_HV(HeVAL(he)) : NULL;

    ret_array = newAV();
    if(isarev) {
        HE* iter;
        hv_iterinit(isarev);
        while((iter = hv_iternext(isarev)))
            av_push(ret_array, newSVsv(hv_iterkeysv(iter)));
    }
    mXPUSHs(newRV_noinc(MUTABLE_SV(ret_array)));

    PUTBACK;

void
mro_is_universal(...)
  PROTOTYPE: $
  PREINIT:
    SV* classname;
    HV* isarev;
    char* classname_pv;
    STRLEN classname_len;
    HE* he;
  PPCODE:
    if (items != 1)
	croak_xs_usage(cv, "classname");

    classname = ST(0);

    classname_pv = SvPV(classname,classname_len);

    he = hv_fetch_ent(PL_isarev, classname, 0, 0);
    isarev = he ? MUTABLE_HV(HeVAL(he)) : NULL;

    if((memEQs(classname_pv, classname_len, "UNIVERSAL"))
        || (isarev && hv_existss(isarev, "UNIVERSAL")))
        XSRETURN_YES;
    else
        XSRETURN_NO;


void
mro_invalidate_all_method_caches(...)
  PROTOTYPE: 
  PPCODE:
    if (items != 0)
	croak_xs_usage(cv, "");

    PL_sub_generation++;

    XSRETURN_EMPTY;

void
mro_get_pkg_gen(...)
  PROTOTYPE: $
  PREINIT:
    SV* classname;
    HV* class_stash;
  PPCODE:
    if(items != 1)
	croak_xs_usage(cv, "classname");
    
    classname = ST(0);

    class_stash = gv_stashsv(classname, 0);

    mXPUSHi(class_stash ? HvMROMETA(class_stash)->pkg_gen : 0);
    
    PUTBACK;

void
mro__nextcan(...)
  PREINIT:
    SV* self = ST(0);
    const I32 throw_nomethod = SvIVX(ST(1));
    I32 cxix = cxstack_ix;
    const PERL_CONTEXT *ccstack = cxstack;
    const PERL_SI *top_si = PL_curstackinfo;
    HV* selfstash;
    SV *stashname;
    const char *fq_subname = NULL;
    const char *subname = NULL;
    bool subname_utf8 = 0;
    STRLEN stashname_len;
    STRLEN subname_len;
    SV* sv;
    GV** gvp;
    AV* linear_av;
    SV** linear_svp;
    const char *hvname;
    I32 entries;
    struct mro_meta* selfmeta;
    HV* nmcache;
    I32 i;
  PPCODE:
    PERL_UNUSED_ARG(cv);

    if(sv_isobject(self))
        selfstash = SvSTASH(SvRV(self));
    else
        selfstash = gv_stashsv(self, GV_ADD);

    assert(selfstash);

    hvname = HvNAME_get(selfstash);
    if (!hvname)
        Perl_croak(aTHX_ "Can't use anonymous symbol table for method lookup");

    /* This block finds the contextually-enclosing fully-qualified subname,
       much like looking at (caller($i))[3] until you find a real sub that
       isn't ANON, etc (also skips over pureperl next::method, etc) */
    for(i = 0; i < 2; i++) {
        cxix = __dopoptosub_at(ccstack, cxix);
        for (;;) {
	    GV* cvgv;

            /* we may be in a higher stacklevel, so dig down deeper */
            while (cxix < 0) {
                if(top_si->si_type == PERLSI_MAIN)
                    Perl_croak(aTHX_ "next::method/next::can/maybe::next::method must be used in method context");
                top_si = top_si->si_prev;
                ccstack = top_si->si_cxstack;
                cxix = __dopoptosub_at(ccstack, top_si->si_cxix);
            }

            if(CxTYPE((PERL_CONTEXT*)(&ccstack[cxix])) != CXt_SUB
              || (PL_DBsub && GvCV(PL_DBsub) && ccstack[cxix].blk_sub.cv == GvCV(PL_DBsub))) {
                cxix = __dopoptosub_at(ccstack, cxix - 1);
                continue;
            }

            {
                const I32 dbcxix = __dopoptosub_at(ccstack, cxix - 1);
                if (PL_DBsub && GvCV(PL_DBsub) && dbcxix >= 0 && ccstack[dbcxix].blk_sub.cv == GvCV(PL_DBsub)) {
                    if(CxTYPE((PERL_CONTEXT*)(&ccstack[dbcxix])) != CXt_SUB) {
                        cxix = dbcxix;
                        continue;
                    }
                }
            }

            cvgv = CvGV(ccstack[cxix].blk_sub.cv);

            if(!isGV(cvgv)) {
                cxix = __dopoptosub_at(ccstack, cxix - 1);
                continue;
            }

            /* we found a real sub here */
            sv = sv_newmortal();

            gv_efullname3(sv, cvgv, NULL);

	    if(SvPOK(sv)) {
		fq_subname = SvPVX(sv);
		subname = strrchr(fq_subname, ':');
            }
            if(!subname)
                Perl_croak(aTHX_ "next::method/next::can/maybe::next::method cannot find enclosing method");

            subname_utf8 = SvUTF8(sv) ? 1 : 0;
            subname++;
            subname_len = SvCUR(sv) - (subname - fq_subname);
            if(memEQs(subname, subname_len, "__ANON__")) {
                cxix = __dopoptosub_at(ccstack, cxix - 1);
                continue;
            }
            break;
        }
        cxix--;
    }

    /* If we made it to here, we found our context */

    /* Initialize the next::method cache for this stash
       if necessary */
    selfmeta = HvMROMETA(selfstash);
    if(!(nmcache = selfmeta->mro_nextmethod)) {
        nmcache = selfmeta->mro_nextmethod = newHV();
    }
    else { /* Use the cached coderef if it exists */
	HE* cache_entry = hv_fetch_ent(nmcache, sv, 0, 0);
	if (cache_entry) {
	    SV* const val = HeVAL(cache_entry);
	    if(val == &PL_sv_undef) {
		if(throw_nomethod)
		    Perl_croak(aTHX_
                       "No next::method '%" SVf "' found for %" HEKf,
                        SVfARG(newSVpvn_flags(subname, subname_len,
                                SVs_TEMP | ( subname_utf8 ? SVf_UTF8 : 0 ) )),
                        HEKfARG( HvNAME_HEK(selfstash) ));
                XSRETURN_EMPTY;
	    }
	    mXPUSHs(newRV_inc(val));
            XSRETURN(1);
	}
    }

    /* beyond here is just for cache misses, so perf isn't as critical */

    stashname_len = subname - fq_subname - 2;
    stashname = newSVpvn_flags(fq_subname, stashname_len,
                                SVs_TEMP | (subname_utf8 ? SVf_UTF8 : 0));

    /* has ourselves at the top of the list */
    linear_av = S_mro_get_linear_isa_c3(aTHX_ selfstash, 0);

    linear_svp = AvARRAY(linear_av);
    entries = AvFILLp(linear_av) + 1;

    /* Walk down our MRO, skipping everything up
       to the contextually enclosing class */
    while (entries--) {
        SV * const linear_sv = *linear_svp++;
        assert(linear_sv);
        if(sv_eq(linear_sv, stashname))
            break;
    }

    /* Now search the remainder of the MRO for the
       same method name as the contextually enclosing
       method */
    if(entries > 0) {
        while (entries--) {
            SV * const linear_sv = *linear_svp++;
	    HV* curstash;
	    GV* candidate;
	    CV* cand_cv;

            assert(linear_sv);
            curstash = gv_stashsv(linear_sv, FALSE);

            if (!curstash) {
                if (ckWARN(WARN_SYNTAX))
                    Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
                       "Can't locate package %" SVf " for @%" HEKf "::ISA",
                        (void*)linear_sv,
                        HEKfARG( HvNAME_HEK(selfstash) ));
                continue;
            }

            assert(curstash);

            gvp = (GV**)hv_fetch(curstash, subname,
                                    subname_utf8 ? -(I32)subname_len : (I32)subname_len, 0);
            if (!gvp) continue;

            candidate = *gvp;
            assert(candidate);

            if (SvTYPE(candidate) != SVt_PVGV)
                gv_init_pvn(candidate, curstash, subname, subname_len,
                                GV_ADDMULTI|(subname_utf8 ? SVf_UTF8 : 0));

            /* Notably, we only look for real entries, not method cache
               entries, because in C3 the method cache of a parent is not
               valid for the child */
            if (SvTYPE(candidate) == SVt_PVGV && (cand_cv = GvCV(candidate)) && !GvCVGEN(candidate)) {
                SvREFCNT_inc_simple_void_NN(MUTABLE_SV(cand_cv));
                (void)hv_store_ent(nmcache, sv, MUTABLE_SV(cand_cv), 0);
                mXPUSHs(newRV_inc(MUTABLE_SV(cand_cv)));
                XSRETURN(1);
            }
        }
    }

    (void)hv_store_ent(nmcache, sv, &PL_sv_undef, 0);
    if(throw_nomethod)
        Perl_croak(aTHX_ "No next::method '%" SVf "' found for %" HEKf,
                         SVfARG(newSVpvn_flags(subname, subname_len,
                                SVs_TEMP | ( subname_utf8 ? SVf_UTF8 : 0 ) )),
                        HEKfARG( HvNAME_HEK(selfstash) ));
    XSRETURN_EMPTY;

BOOT:
    Perl_mro_register(aTHX_ &c3_alg);
