#define PERL_NO_GET_CONTEXT

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

MODULE = Hash::Util		PACKAGE = Hash::Util

void
_clear_placeholders(hashref)
        HV *hashref
    PROTOTYPE: \%
    PREINIT:
        HV *hv;
    CODE:
        hv = MUTABLE_HV(hashref);
        hv_clear_placeholders(hv);

void
all_keys(hash,keys,placeholder)
	HV *hash
	AV *keys
	AV *placeholder
    PROTOTYPE: \%\@\@
    PREINIT:
        SV *key;
        HE *he;
    PPCODE:
        av_clear(keys);
        av_clear(placeholder);

        (void)hv_iterinit(hash);
	while((he = hv_iternext_flags(hash, HV_ITERNEXT_WANTPLACEHOLDERS))!= NULL) {
	    key=hv_iterkeysv(he);
	    av_push(HeVAL(he) == &PL_sv_placeholder ? placeholder : keys,
		    SvREFCNT_inc(key));
        }
	XSRETURN(1);

void
hidden_ref_keys(hash)
	HV *hash
    ALIAS:
	Hash::Util::legal_ref_keys = 1
    PREINIT:
        SV *key;
        HE *he;
    PPCODE:
        (void)hv_iterinit(hash);
	while((he = hv_iternext_flags(hash, HV_ITERNEXT_WANTPLACEHOLDERS))!= NULL) {
	    key=hv_iterkeysv(he);
            if (ix || HeVAL(he) == &PL_sv_placeholder) {
                XPUSHs( key );
            }
        }

void
hv_store(hash, key, val)
	HV *hash
	SV* key
	SV* val
    PROTOTYPE: \%$$
    CODE:
    {
        SvREFCNT_inc(val);
	if (!hv_store_ent(hash, key, val, 0)) {
	    SvREFCNT_dec(val);
	    XSRETURN_NO;
	} else {
	    XSRETURN_YES;
	}
    }

void
hash_seed()
    PROTOTYPE:
    PPCODE:
    mXPUSHs(newSVpvn((char *)PERL_HASH_SEED,PERL_HASH_SEED_BYTES));
    XSRETURN(1);


void
hash_value(string,...)
        SV* string
    PROTOTYPE: $;$
    PPCODE:
{
    UV uv;
    STRLEN len;
    char *pv= SvPV(string,len);
    if (items<2) {
        PERL_HASH(uv, pv, len);
    } else {
        STRLEN seedlen;
        U8 *seedbuf= (U8 *)SvPV(ST(1),seedlen);
        if ( seedlen < PERL_HASH_SEED_BYTES ) {
            sv_dump(ST(1));
            Perl_croak(aTHX_ "seed len must be at least %" UVuf " long only got %"
                             UVuf " bytes", (UV)PERL_HASH_SEED_BYTES, (UV)seedlen);
        }

        PERL_HASH_WITH_SEED(seedbuf, uv, pv, len);
    }
    XSRETURN_UV(uv);
}

void
hash_traversal_mask(rhv, ...)
        SV* rhv
    PPCODE:
{
#ifdef PERL_HASH_RANDOMIZE_KEYS
    if (SvROK(rhv) && SvTYPE(SvRV(rhv))==SVt_PVHV && !SvMAGICAL(SvRV(rhv))) {
        HV *hv = (HV *)SvRV(rhv);
        if (items>1) {
            hv_rand_set(hv, SvUV(ST(1)));
        }
        if (HvHasAUX(hv)) {
            XSRETURN_UV(HvRAND_get(hv));
        } else {
            XSRETURN_UNDEF;
        }
    }
#else
    Perl_croak(aTHX_ "Perl has not been compiled with support for randomized hash key traversal");
#endif
}

void
bucket_info(rhv)
        SV* rhv
    PPCODE:
{
    /*

    Takes a non-magical hash ref as an argument and returns a list of
    statistics about the hash. The number and keys and the size of the
    array will always be reported as the first two values. If the array is
    actually allocated (they are lazily allocated), then additionally
    will return a list of counts of bucket lengths. In other words in

        ($keys, $buckets, $used, @length_count)= hash::bucket_info(\%hash);

    $length_count[0] is the number of empty buckets, and $length_count[1]
    is the number of buckets with only one key in it, $buckets - $length_count[0]
    gives the number of used buckets, and @length_count-1 is the maximum
    bucket depth.

    If the argument is not a hash ref, or if it is magical, then returns
    nothing (the empty list).

    */
    const HV * hv = NULL;
    if (SvROK(rhv) && SvTYPE(SvRV(rhv))==SVt_PVHV && !SvMAGICAL(SvRV(rhv))) {
        hv = (const HV *) SvRV(rhv);
    } else if (!SvOK(rhv)) {
        hv = PL_strtab;
    }
    if (hv) {
        U32 max_bucket_index= HvMAX(hv);
        U32 total_keys= HvUSEDKEYS(hv);
        HE **bucket_array= HvARRAY(hv);
        mXPUSHi(total_keys);
        mXPUSHi(max_bucket_index+1);
        mXPUSHi(0); /* for the number of used buckets */
#define BUCKET_INFO_ITEMS_ON_STACK 3
        if (!bucket_array) {
            XSRETURN(BUCKET_INFO_ITEMS_ON_STACK);
        } else {
            /* we use chain_length to index the stack - we eliminate an add
             * by initializing things with the number of items already on the stack.
             * If we have 2 items then ST(2+0) (the third stack item) will be the counter
             * for empty chains, ST(2+1) will be for chains with one element,  etc.
             */
            I32 max_chain_length= BUCKET_INFO_ITEMS_ON_STACK - 1; /* so we do not have to do an extra push for the 0 index */
            HE *he;
            U32 bucket_index;
            for ( bucket_index= 0; bucket_index <= max_bucket_index; bucket_index++ ) {
                I32 chain_length= BUCKET_INFO_ITEMS_ON_STACK;
                for (he= bucket_array[bucket_index]; he; he= HeNEXT(he) ) {
                    chain_length++;
                }
                while ( max_chain_length < chain_length ) {
                    mXPUSHi(0);
                    max_chain_length++;
                }
                SvIVX( ST( chain_length ) )++;
            }
            /* now set the number of used buckets */
            SvIVX( ST( BUCKET_INFO_ITEMS_ON_STACK - 1 ) ) = max_bucket_index - SvIVX( ST( BUCKET_INFO_ITEMS_ON_STACK ) ) + 1;
            XSRETURN( max_chain_length + 1 ); /* max_chain_length is the index of the last item on the stack, so we add 1 */
        }
#undef BUCKET_INFO_ITEMS_ON_STACK
    }
    XSRETURN(0);
}

void
bucket_array(rhv)
        SV* rhv
    PPCODE:
{
    /* Returns an array of arrays representing key/bucket mappings.
     * Each element of the array contains either an integer or a reference
     * to an array of keys. A plain integer represents K empty buckets. An
     * array ref represents a single bucket, with each element being a key in
     * the hash. (Note this treats a placeholder as a normal key.)
     *
     * This allows one to "see" the keyorder. Note the "insert first" nature
     * of the hash store, combined with regular remappings means that relative
     * order of keys changes each remap.
     */
    const HV * hv = NULL;
    if (SvROK(rhv) && SvTYPE(SvRV(rhv))==SVt_PVHV && !SvMAGICAL(SvRV(rhv))) {
        hv = (const HV *) SvRV(rhv);
    } else if (!SvOK(rhv)) {
        hv = PL_strtab;
    }
    if (hv) {
        HE **he_ptr= HvARRAY(hv);
        if (!he_ptr) {
            XSRETURN(0);
        } else {
            U32 i, max;
            AV *info_av;
            HE *he;
            I32 empty_count=0;
            if (SvMAGICAL(hv)) {
                Perl_croak(aTHX_ "hash::bucket_array only works on 'normal' hashes");
            }
            info_av= newAV();
            max= HvMAX(hv);
            mXPUSHs(newRV_noinc((SV*)info_av));
            for ( i= 0; i <= max; i++ ) {
                AV *key_av= NULL;
                for (he= he_ptr[i]; he; he= HeNEXT(he) ) {
                    SV *key_sv;
                    char *str;
                    STRLEN len;
                    char mode;
                    if (!key_av) {
                        key_av= newAV();
                        if (empty_count) {
                            av_push(info_av, newSViv(empty_count));
                            empty_count= 0;
                        }
                        av_push(info_av, (SV *)newRV_noinc((SV *)key_av));
                    }
                    if (HeKLEN(he) == HEf_SVKEY) {
                        SV *sv= HeSVKEY(he);
                        SvGETMAGIC(sv);
                        str= SvPV(sv, len);
                        mode= SvUTF8(sv) ? 1 : 0;
                    } else {
                        str= HeKEY(he);
                        len= HeKLEN(he);
                        mode= HeKUTF8(he) ? 1 : 0;
                    }
                    key_sv= newSVpvn(str,len);
                    av_push(key_av,key_sv);
                    if (mode) {
                        SvUTF8_on(key_sv);
                    }
                }
                if (!key_av)
                    empty_count++;
            }
            if (empty_count) {
                av_push(info_av, newSViv(empty_count));
                empty_count++;
            }
        }
        XSRETURN(1);
    }
    XSRETURN(0);
}

void
bucket_ratio(rhv)
        SV* rhv
    PROTOTYPE: \%
    PPCODE:
{
    if (SvROK(rhv)) {
        rhv= SvRV(rhv);
        if ( SvTYPE(rhv)==SVt_PVHV ) {
#if PERL_VERSION_LT(5,25,0)
            SV *ret= Perl_hv_scalar(aTHX_ (HV*)rhv);
#else
            SV *ret= Perl_hv_bucket_ratio(aTHX_ (HV*)rhv);
#endif
            ST(0)= ret;
            XSRETURN(1);
        }
    }
    XSRETURN_UNDEF;
}

void
num_buckets(rhv)
        SV* rhv
    PROTOTYPE: \%
    PPCODE:
{
    if (SvROK(rhv)) {
        rhv= SvRV(rhv);
        if ( SvTYPE(rhv)==SVt_PVHV ) {
            XSRETURN_UV(HvMAX((HV*)rhv)+1);
        }
    }
    XSRETURN_UNDEF;
}

void
used_buckets(rhv)
        SV* rhv
    PROTOTYPE: \%
    PPCODE:
{
    if (SvROK(rhv)) {
        rhv= SvRV(rhv);
        if ( SvTYPE(rhv)==SVt_PVHV ) {
            XSRETURN_UV(HvFILL((HV*)rhv));
        }
    }
    XSRETURN_UNDEF;
}

