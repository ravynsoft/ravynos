/*    hv.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 *      I sit beside the fire and think
 *          of all that I have seen.
 *                         --Bilbo
 *
 *     [p.278 of _The Lord of the Rings_, II/iii: "The Ring Goes South"]
 */

/* 
=head1 HV Handling
A HV structure represents a Perl hash.  It consists mainly of an array
of pointers, each of which points to a linked list of HE structures.  The
array is indexed by the hash function of the key, so each linked list
represents all the hash entries with the same hash value.  Each HE contains
a pointer to the actual value, plus a pointer to a HEK structure which
holds the key and hash value.

=cut

*/

#include "EXTERN.h"
#define PERL_IN_HV_C
#define PERL_HASH_INTERNAL_ACCESS
#include "perl.h"

/* we split when we collide and we have a load factor over 0.667.
 * NOTE if you change this formula so we split earlier than previously
 * you MUST change the logic in hv_ksplit()
 */

/*  MAX_BUCKET_MAX is the maximum max bucket index, at which point we stop growing the
 *  number of buckets,
 */
#define MAX_BUCKET_MAX ((1<<26)-1)
#define DO_HSPLIT(xhv) ( ( ((xhv)->xhv_keys + ((xhv)->xhv_keys >> 1)) > (xhv)->xhv_max ) && \
                           ((xhv)->xhv_max < MAX_BUCKET_MAX) )

static const char S_strtab_error[]
    = "Cannot modify shared string table in hv_%s";

#define DEBUG_HASH_RAND_BITS (DEBUG_h_TEST)

/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
 * See also https://en.wikipedia.org/wiki/Xorshift
 */
#if IVSIZE == 8
/* 64 bit version */
#define XORSHIFT_RAND_BITS(x)   PERL_XORSHIFT64_A(x)
#else
/* 32 bit version */
#define XORSHIFT_RAND_BITS(x)   PERL_XORSHIFT32_A(x)
#endif

#define UPDATE_HASH_RAND_BITS_KEY(key,klen)                             \
STMT_START {                                                            \
    XORSHIFT_RAND_BITS(PL_hash_rand_bits);                              \
    if (DEBUG_HASH_RAND_BITS) {                                         \
        PerlIO_printf( Perl_debug_log,                                  \
            "PL_hash_rand_bits=%016" UVxf" @ %s:%-4d",                   \
            (UV)PL_hash_rand_bits, __FILE__, __LINE__                   \
        );                                                              \
        if (DEBUG_v_TEST && key) {                                      \
            PerlIO_printf( Perl_debug_log, " key:'%.*s' %" UVuf"\n",     \
                    (int)klen,                                          \
                    key ? key : "", /* silence warning */               \
                    (UV)klen                                            \
            );                                                          \
        } else {                                                        \
            PerlIO_printf( Perl_debug_log, "\n");                       \
        }                                                               \
    }                                                                   \
} STMT_END

#define MAYBE_UPDATE_HASH_RAND_BITS_KEY(key,klen)                       \
STMT_START {                                                            \
    if (PL_HASH_RAND_BITS_ENABLED)                                      \
        UPDATE_HASH_RAND_BITS_KEY(key,klen);                            \
} STMT_END


#define UPDATE_HASH_RAND_BITS()                                         \
    UPDATE_HASH_RAND_BITS_KEY(NULL,0)

#define MAYBE_UPDATE_HASH_RAND_BITS()                                   \
    MAYBE_UPDATE_HASH_RAND_BITS_KEY(NULL,0)

/* HeKFLAGS(entry) is a single U8, so only provides 8 flags bits.
   We currently use 3. All 3 we have behave differently, so if we find a use for
   more flags it's hard to predict which they group with.

   Hash keys are stored as flat octet sequences, not SVs. Hence we need a flag
   bit to say whether those octet sequences represent ISO-8859-1 or UTF-8 -
   HVhek_UTF8. The value of this flag bit matters for (regular) hash key
   lookups.

   To speed up comparisons, keys are normalised to octets. But we (also)
   preserve whether the key was supplied, so we need another flag bit to say
   whether to reverse the normalisation when iterating the keys (converting them
   back to SVs) - HVhek_WASUTF8. The value of this flag bit must be ignored for
   (regular) hash key lookups.

   But for the shared string table (the private "hash" that manages shared hash
   keys and their reference counts), we need to be able to store both variants
   (HVhek_WASUTF8 set and clear), so the code performing lookups in this hash
   must be different and consider both keys.

   However, regular hashes (now) can have a mix of shared and unshared keys.
   (This avoids the need to reallocate all the keys into unshared storage at
   the point where hash passes the "large" hash threshold, and no longer uses
   the shared string table - existing keys remain shared, to avoid makework.)

   Meaning that HVhek_NOTSHARED *may* be set in regular hashes (but should be
   ignored for hash lookups) but must always be clear in the keys in the shared
   string table (because the pointers to these keys are directly copied into
   regular hashes - this is how shared keys work.)

   Hence all 3 are different, and it's hard to predict the best way to future
   proof what is needed next.

   We also have HVhek_ENABLEHVKFLAGS, which is used as a mask within the code
   below to determine whether to set HvHASKFLAGS() true on the hash as a whole.
   This is a public "optimisation" flag provided to serealisers, to indicate
   (up front) that a hash contains non-8-bit keys, if they want to use different
   storage formats for hashes where all keys are simple octet sequences
   (avoiding needing to store an extra byte per hash key), and they need to know
   that this holds *before* iterating the hash keys. Only Storable seems to use
   this. (For this use case, HVhek_NOTSHARED doesn't matter)

   For now, we assume that any future flag bits will need to be distinguished
   in the shared string table, hence we create this mask for the shared string
   table code. It happens to be the same as HVhek_ENABLEHVKFLAGS, but that might
   change if we add a flag bit that matters to the shared string table but not
   to Storable (or similar). */

#define HVhek_STORAGE_MASK (0xFF & ~HVhek_NOTSHARED)

#ifdef PURIFY

#define new_HE() (HE*)safemalloc(sizeof(HE))
#define del_HE(p) safefree((char*)p)

#else

STATIC HE*
S_new_he(pTHX)
{
    HE* he;
    void ** const root = &PL_body_roots[HE_ARENA_ROOT_IX];

    if (!*root)
        Perl_more_bodies(aTHX_ HE_ARENA_ROOT_IX, sizeof(HE), PERL_ARENA_SIZE);
    he = (HE*) *root;
    assert(he);
    *root = HeNEXT(he);
    return he;
}

#define new_HE() new_he()
#define del_HE(p) \
    STMT_START { \
        HeNEXT(p) = (HE*)(PL_body_roots[HE_ARENA_ROOT_IX]);	\
        PL_body_roots[HE_ARENA_ROOT_IX] = p; \
    } STMT_END



#endif

STATIC HEK *
S_save_hek_flags(const char *str, I32 len, U32 hash, int flags)
{
    char *k;
    HEK *hek;

    PERL_ARGS_ASSERT_SAVE_HEK_FLAGS;

    Newx(k, HEK_BASESIZE + len + 2, char);
    hek = (HEK*)k;
    Copy(str, HEK_KEY(hek), len, char);
    HEK_KEY(hek)[len] = 0;
    HEK_LEN(hek) = len;
    HEK_HASH(hek) = hash;
    HEK_FLAGS(hek) = HVhek_NOTSHARED | (flags & HVhek_STORAGE_MASK);

    if (flags & HVhek_FREEKEY)
        Safefree(str);
    return hek;
}

/* free the pool of temporary HE/HEK pairs returned by hv_fetch_ent
 * for tied hashes */

void
Perl_free_tied_hv_pool(pTHX)
{
    HE *he = PL_hv_fetch_ent_mh;
    while (he) {
        HE * const ohe = he;
        Safefree(HeKEY_hek(he));
        he = HeNEXT(he);
        del_HE(ohe);
    }
    PL_hv_fetch_ent_mh = NULL;
}

#if defined(USE_ITHREADS)
HEK *
Perl_hek_dup(pTHX_ HEK *source, CLONE_PARAMS* param)
{
    HEK *shared;

    PERL_ARGS_ASSERT_HEK_DUP;
    PERL_UNUSED_ARG(param);

    if (!source)
        return NULL;

    shared = (HEK*)ptr_table_fetch(PL_ptr_table, source);
    if (shared) {
        /* We already shared this hash key.  */
        (void)share_hek_hek(shared);
    }
    else {
        shared
            = share_hek_flags(HEK_KEY(source), HEK_LEN(source),
                              HEK_HASH(source), HEK_FLAGS(source));
        ptr_table_store(PL_ptr_table, source, shared);
    }
    return shared;
}

HE *
Perl_he_dup(pTHX_ const HE *e, bool shared, CLONE_PARAMS* param)
{
    HE *ret;

    PERL_ARGS_ASSERT_HE_DUP;

    /* All the *_dup functions are deemed to be API, despite most being deeply
       tied to the internals. Hence we can't simply remove the parameter
       "shared" from this function. */
    /* sv_dup and sv_dup_inc seem to be the only two that are used by XS code.
       Probably the others should be dropped from the API. See #19409 */
    PERL_UNUSED_ARG(shared);

    if (!e)
        return NULL;
    /* look for it in the table first */
    ret = (HE*)ptr_table_fetch(PL_ptr_table, e);
    if (ret)
        return ret;

    /* create anew and remember what it is */
    ret = new_HE();
    ptr_table_store(PL_ptr_table, e, ret);

    if (HeKLEN(e) == HEf_SVKEY) {
        char *k;
        Newx(k, HEK_BASESIZE + sizeof(const SV *), char);
        HeKEY_hek(ret) = (HEK*)k;
        HeKEY_sv(ret) = sv_dup_inc(HeKEY_sv(e), param);
    }
    else if (!(HeKFLAGS(e) & HVhek_NOTSHARED)) {
        /* This is hek_dup inlined, which seems to be important for speed
           reasons.  */
        HEK * const source = HeKEY_hek(e);
        HEK *shared = (HEK*)ptr_table_fetch(PL_ptr_table, source);

        if (shared) {
            /* We already shared this hash key.  */
            (void)share_hek_hek(shared);
        }
        else {
            shared
                = share_hek_flags(HEK_KEY(source), HEK_LEN(source),
                                  HEK_HASH(source), HEK_FLAGS(source));
            ptr_table_store(PL_ptr_table, source, shared);
        }
        HeKEY_hek(ret) = shared;
    }
    else
        HeKEY_hek(ret) = save_hek_flags(HeKEY(e), HeKLEN(e), HeHASH(e),
                                        HeKFLAGS(e));
    HeVAL(ret) = sv_dup_inc(HeVAL(e), param);

    HeNEXT(ret) = he_dup(HeNEXT(e), FALSE, param);
    return ret;
}
#endif	/* USE_ITHREADS */

static void
S_hv_notallowed(pTHX_ int flags, const char *key, I32 klen,
                const char *msg)
{
   /* Straight to SVt_PVN here, as needed by sv_setpvn_fresh and
    * sv_usepvn would otherwise call it */
    SV * const sv = newSV_type_mortal(SVt_PV);

    PERL_ARGS_ASSERT_HV_NOTALLOWED;

    if (!(flags & HVhek_FREEKEY)) {
        sv_setpvn_fresh(sv, key, klen);
    }
    else {
        /* Need to free saved eventually assign to mortal SV */
        /* XXX is this line an error ???:  SV *sv = sv_newmortal(); */
        sv_usepvn(sv, (char *) key, klen);
    }
    if (flags & HVhek_UTF8) {
        SvUTF8_on(sv);
    }
    Perl_croak(aTHX_ msg, SVfARG(sv));
}

/* (klen == HEf_SVKEY) is special for MAGICAL hv entries, meaning key slot
 * contains an SV* */

/*
=for apidoc      hv_store
=for apidoc_item hv_stores

These each store SV C<val> with the specified key in hash C<hv>, returning NULL
if the operation failed or if the value did not need to be actually stored
within the hash (as in the case of tied hashes).  Otherwise it can be
dereferenced to get the original C<SV*>.

They differ only in how the hash key is specified.

In C<hv_stores>, the key is a C language string literal, enclosed in double
quotes.  It is never treated as being in UTF-8.

In C<hv_store>, C<key> is either NULL or points to the first byte of the string
specifying the key, and its length in bytes is given by the absolute value of
an additional parameter, C<klen>.  A NULL key indicates the key is to be
treated as C<undef>, and C<klen> is ignored; otherwise the key string may
contain embedded-NUL bytes.  If C<klen> is negative, the string is treated as
being encoded in UTF-8; otherwise not.

C<hv_store> has another extra parameter, C<hash>, a precomputed hash of the key
string, or zero if it has not been precomputed.  This parameter is omitted from
C<hv_stores>, as it is computed automatically at compile time.

If <hv> is NULL, NULL is returned and no action is taken.

If C<val> is NULL, it is treated as being C<undef>; otherwise the caller is
responsible for suitably incrementing the reference count of C<val> before
the call, and decrementing it if the function returned C<NULL>.  Effectively
a successful C<hv_store> takes ownership of one reference to C<val>.  This is
usually what you want; a newly created SV has a reference count of one, so
if all your code does is create SVs then store them in a hash, C<hv_store>
will own the only reference to the new SV, and your code doesn't need to do
anything further to tidy up.

C<hv_store> is not implemented as a call to L</C<hv_store_ent>>, and does not
create a temporary SV for the key, so if your key data is not already in SV
form then use C<hv_store> in preference to C<hv_store_ent>.

See L<perlguts/"Understanding the Magic of Tied Hashes and Arrays"> for more
information on how to use this function on tied hashes.

=for apidoc hv_store_ent

Stores C<val> in a hash.  The hash key is specified as C<key>.  The C<hash>
parameter is the precomputed hash value; if it is zero then Perl will
compute it.  The return value is the new hash entry so created.  It will be
C<NULL> if the operation failed or if the value did not need to be actually
stored within the hash (as in the case of tied hashes).  Otherwise the
contents of the return value can be accessed using the C<He?> macros
described here.  Note that the caller is responsible for suitably
incrementing the reference count of C<val> before the call, and
decrementing it if the function returned NULL.  Effectively a successful
C<hv_store_ent> takes ownership of one reference to C<val>.  This is
usually what you want; a newly created SV has a reference count of one, so
if all your code does is create SVs then store them in a hash, C<hv_store>
will own the only reference to the new SV, and your code doesn't need to do
anything further to tidy up.  Note that C<hv_store_ent> only reads the C<key>;
unlike C<val> it does not take ownership of it, so maintaining the correct
reference count on C<key> is entirely the caller's responsibility.  The reason
it does not take ownership, is that C<key> is not used after this function
returns, and so can be freed immediately.  C<hv_store>
is not implemented as a call to C<hv_store_ent>, and does not create a temporary
SV for the key, so if your key data is not already in SV form then use
C<hv_store> in preference to C<hv_store_ent>.

See L<perlguts/"Understanding the Magic of Tied Hashes and Arrays"> for more
information on how to use this function on tied hashes.

=for apidoc hv_exists

Returns a boolean indicating whether the specified hash key exists.  The
absolute value of C<klen> is the length of the key.  If C<klen> is
negative the key is assumed to be in UTF-8-encoded Unicode.

=for apidoc hv_fetch

Returns the SV which corresponds to the specified key in the hash.
The absolute value of C<klen> is the length of the key.  If C<klen> is
negative the key is assumed to be in UTF-8-encoded Unicode.  If
C<lval> is set then the fetch will be part of a store.  This means that if
there is no value in the hash associated with the given key, then one is
created and a pointer to it is returned.  The C<SV*> it points to can be
assigned to.  But always check that the
return value is non-null before dereferencing it to an C<SV*>.

See L<perlguts/"Understanding the Magic of Tied Hashes and Arrays"> for more
information on how to use this function on tied hashes.

=for apidoc hv_exists_ent

Returns a boolean indicating whether
the specified hash key exists.  C<hash>
can be a valid precomputed hash value, or 0 to ask for it to be
computed.

=cut
*/

/* returns an HE * structure with the all fields set */
/* note that hent_val will be a mortal sv for MAGICAL hashes */
/*
=for apidoc hv_fetch_ent

Returns the hash entry which corresponds to the specified key in the hash.
C<hash> must be a valid precomputed hash number for the given C<key>, or 0
if you want the function to compute it.  IF C<lval> is set then the fetch
will be part of a store.  Make sure the return value is non-null before
accessing it.  The return value when C<hv> is a tied hash is a pointer to a
static location, so be sure to make a copy of the structure if you need to
store it somewhere.

See L<perlguts/"Understanding the Magic of Tied Hashes and Arrays"> for more
information on how to use this function on tied hashes.

=cut
*/

/* Common code for hv_delete()/hv_exists()/hv_fetch()/hv_store()  */
void *
Perl_hv_common_key_len(pTHX_ HV *hv, const char *key, I32 klen_i32,
                       const int action, SV *val, const U32 hash)
{
    STRLEN klen;
    int flags;

    PERL_ARGS_ASSERT_HV_COMMON_KEY_LEN;

    if (klen_i32 < 0) {
        klen = -klen_i32;
        flags = HVhek_UTF8;
    } else {
        klen = klen_i32;
        flags = 0;
    }
    return hv_common(hv, NULL, key, klen, flags, action, val, hash);
}

void *
Perl_hv_common(pTHX_ HV *hv, SV *keysv, const char *key, STRLEN klen,
               int flags, int action, SV *val, U32 hash)
{
    XPVHV* xhv;
    HE *entry;
    HE **oentry;
    SV *sv;
    bool is_utf8;
    bool in_collision;
    const int return_svp = action & HV_FETCH_JUST_SV;
    HEK *keysv_hek = NULL;

    if (!hv)
        return NULL;
    if (SvIS_FREED(hv))
        return NULL;

    assert(SvTYPE(hv) == SVt_PVHV);

    if (SvSMAGICAL(hv) && SvGMAGICAL(hv) && !(action & HV_DISABLE_UVAR_XKEY)) {
        MAGIC* mg;
        if ((mg = mg_find((const SV *)hv, PERL_MAGIC_uvar))) {
            struct ufuncs * const uf = (struct ufuncs *)mg->mg_ptr;
            if (uf->uf_set == NULL) {
                SV* obj = mg->mg_obj;

                if (!keysv) {
                    keysv = newSVpvn_flags(key, klen, SVs_TEMP |
                                           ((flags & HVhek_UTF8)
                                            ? SVf_UTF8 : 0));
                }
                
                mg->mg_obj = keysv;         /* pass key */
                uf->uf_index = action;      /* pass action */
                magic_getuvar(MUTABLE_SV(hv), mg);
                keysv = mg->mg_obj;         /* may have changed */
                mg->mg_obj = obj;

                /* If the key may have changed, then we need to invalidate
                   any passed-in computed hash value.  */
                hash = 0;
            }
        }
    }

    /* flags might have HVhek_NOTSHARED set. If so, we need to ignore that.
       Some callers to hv_common() pass the flags value from an existing HEK,
       and if that HEK is not shared, then it has the relevant flag bit set,
       which must not be passed into share_hek_flags().

       It would be "purer" to insist that all callers clear it, but we'll end up
       with subtle bugs if we leave it to them, or runtime assertion failures if
       we try to enforce our documentation with landmines.

       If keysv is true, all code paths assign a new value to flags with that
       bit clear, so we're always "good". Hence we only need to explicitly clear
       this bit in the else block. */
    if (keysv) {
        if (flags & HVhek_FREEKEY)
            Safefree(key);
        key = SvPV_const(keysv, klen);
        is_utf8 = (SvUTF8(keysv) != 0);
        if (SvIsCOW_shared_hash(keysv)) {
            flags = HVhek_KEYCANONICAL | (is_utf8 ? HVhek_UTF8 : 0);
        } else {
            flags = 0;
        }
    } else {
        is_utf8 = cBOOL(flags & HVhek_UTF8);
        flags &= ~HVhek_NOTSHARED;
    }

    if (action & HV_DELETE) {
        return (void *) hv_delete_common(hv, keysv, key, klen,
                                         flags | (is_utf8 ? HVhek_UTF8 : 0),
                                         action, hash);
    }

    xhv = (XPVHV*)SvANY(hv);
    if (SvMAGICAL(hv)) {
        if (SvRMAGICAL(hv) && !(action & (HV_FETCH_ISSTORE|HV_FETCH_ISEXISTS))) {
            if (mg_find((const SV *)hv, PERL_MAGIC_tied)
                || SvGMAGICAL((const SV *)hv))
            {
                /* FIXME should be able to skimp on the HE/HEK here when
                   HV_FETCH_JUST_SV is true.  */
                if (!keysv) {
                    keysv = newSVpvn_utf8(key, klen, is_utf8);
                } else {
                    keysv = newSVsv(keysv);
                }
                sv = sv_newmortal();
                mg_copy(MUTABLE_SV(hv), sv, (char *)keysv, HEf_SVKEY);

                /* grab a fake HE/HEK pair from the pool or make a new one */
                entry = PL_hv_fetch_ent_mh;
                if (entry)
                    PL_hv_fetch_ent_mh = HeNEXT(entry);
                else {
                    char *k;
                    entry = new_HE();
                    Newx(k, HEK_BASESIZE + sizeof(const SV *), char);
                    HeKEY_hek(entry) = (HEK*)k;
                }
                HeNEXT(entry) = NULL;
                HeSVKEY_set(entry, keysv);
                HeVAL(entry) = sv;
                sv_upgrade(sv, SVt_PVLV);
                LvTYPE(sv) = 'T';
                 /* so we can free entry when freeing sv */
                LvTARG(sv) = MUTABLE_SV(entry);

                /* XXX remove at some point? */
                if (flags & HVhek_FREEKEY)
                    Safefree(key);

                if (return_svp) {
                    return entry ? (void *) &HeVAL(entry) : NULL;
                }
                return (void *) entry;
            }
#ifdef ENV_IS_CASELESS
            else if (mg_find((const SV *)hv, PERL_MAGIC_env)) {
                U32 i;
                for (i = 0; i < klen; ++i)
                    if (isLOWER(key[i])) {
                        /* Would be nice if we had a routine to do the
                           copy and uppercase in a single pass through.  */
                        const char * const nkey = strupr(savepvn(key,klen));
                        /* Note that this fetch is for nkey (the uppercased
                           key) whereas the store is for key (the original)  */
                        void *result = hv_common(hv, NULL, nkey, klen,
                                                 HVhek_FREEKEY, /* free nkey */
                                                 0 /* non-LVAL fetch */
                                                 | HV_DISABLE_UVAR_XKEY
                                                 | return_svp,
                                                 NULL /* no value */,
                                                 0 /* compute hash */);
                        if (!result && (action & HV_FETCH_LVALUE)) {
                            /* This call will free key if necessary.
                               Do it this way to encourage compiler to tail
                               call optimise.  */
                            result = hv_common(hv, keysv, key, klen, flags,
                                               HV_FETCH_ISSTORE
                                               | HV_DISABLE_UVAR_XKEY
                                               | return_svp,
                                               newSV_type(SVt_NULL), hash);
                        } else {
                            if (flags & HVhek_FREEKEY)
                                Safefree(key);
                        }
                        return result;
                    }
            }
#endif
        } /* ISFETCH */
        else if (SvRMAGICAL(hv) && (action & HV_FETCH_ISEXISTS)) {
            if (mg_find((const SV *)hv, PERL_MAGIC_tied)
                || SvGMAGICAL((const SV *)hv)) {
                /* I don't understand why hv_exists_ent has svret and sv,
                   whereas hv_exists only had one.  */
                SV * const svret = sv_newmortal();
                sv = sv_newmortal();

                if (keysv || is_utf8) {
                    if (!keysv) {
                        keysv = newSVpvn_utf8(key, klen, TRUE);
                    } else {
                        keysv = newSVsv(keysv);
                    }
                    mg_copy(MUTABLE_SV(hv), sv, (char *)sv_2mortal(keysv), HEf_SVKEY);
                } else {
                    mg_copy(MUTABLE_SV(hv), sv, key, klen);
                }
                if (flags & HVhek_FREEKEY)
                    Safefree(key);
                {
                  MAGIC * const mg = mg_find(sv, PERL_MAGIC_tiedelem);
                  if (mg)
                    magic_existspack(svret, mg);
                }
                /* This cast somewhat evil, but I'm merely using NULL/
                   not NULL to return the boolean exists.
                   And I know hv is not NULL.  */
                return SvTRUE_NN(svret) ? (void *)hv : NULL;
                }
#ifdef ENV_IS_CASELESS
            else if (mg_find((const SV *)hv, PERL_MAGIC_env)) {
                /* XXX This code isn't UTF8 clean.  */
                char * const keysave = (char * const)key;
                /* Will need to free this, so set FREEKEY flag.  */
                key = savepvn(key,klen);
                key = (const char*)strupr((char*)key);
                is_utf8 = FALSE;
                hash = 0;
                keysv = 0;

                if (flags & HVhek_FREEKEY) {
                    Safefree(keysave);
                }
                flags |= HVhek_FREEKEY;
            }
#endif
        } /* ISEXISTS */
        else if (action & HV_FETCH_ISSTORE) {
            bool needs_copy;
            bool needs_store;
            hv_magic_check (hv, &needs_copy, &needs_store);
            if (needs_copy) {
                const bool save_taint = TAINT_get;
                if (keysv || is_utf8) {
                    if (!keysv) {
                        keysv = newSVpvn_utf8(key, klen, TRUE);
                    }
                    if (TAINTING_get)
                        TAINT_set(SvTAINTED(keysv));
                    keysv = sv_2mortal(newSVsv(keysv));
                    mg_copy(MUTABLE_SV(hv), val, (char*)keysv, HEf_SVKEY);
                } else {
                    mg_copy(MUTABLE_SV(hv), val, key, klen);
                }

                TAINT_IF(save_taint);
#ifdef NO_TAINT_SUPPORT
                PERL_UNUSED_VAR(save_taint);
#endif
                if (!needs_store) {
                    if (flags & HVhek_FREEKEY)
                        Safefree(key);
                    return NULL;
                }
#ifdef ENV_IS_CASELESS
                else if (mg_find((const SV *)hv, PERL_MAGIC_env)) {
                    /* XXX This code isn't UTF8 clean.  */
                    const char *keysave = key;
                    /* Will need to free this, so set FREEKEY flag.  */
                    key = savepvn(key,klen);
                    key = (const char*)strupr((char*)key);
                    is_utf8 = FALSE;
                    hash = 0;
                    keysv = 0;

                    if (flags & HVhek_FREEKEY) {
                        Safefree(keysave);
                    }
                    flags |= HVhek_FREEKEY;
                }
#endif
            }
        } /* ISSTORE */
    } /* SvMAGICAL */

    if (!HvARRAY(hv)) {
        if ((action & (HV_FETCH_LVALUE | HV_FETCH_ISSTORE))
#ifdef DYNAMIC_ENV_FETCH  /* if it's an %ENV lookup, we may get it on the fly */
                 || (SvRMAGICAL((const SV *)hv)
                     && mg_find((const SV *)hv, PERL_MAGIC_env))
#endif
                                                                  ) {
            char *array;
            Newxz(array,
                 PERL_HV_ARRAY_ALLOC_BYTES(xhv->xhv_max+1 /* HvMAX(hv)+1 */),
                 char);
            HvARRAY(hv) = (HE**)array;
        }
#ifdef DYNAMIC_ENV_FETCH
        else if (action & HV_FETCH_ISEXISTS) {
            /* for an %ENV exists, if we do an insert it's by a recursive
               store call, so avoid creating HvARRAY(hv) right now.  */
        }
#endif
        else {
            /* XXX remove at some point? */
            if (flags & HVhek_FREEKEY)
                Safefree(key);

            return NULL;
        }
    }

    if (is_utf8 && !(flags & HVhek_KEYCANONICAL)) {
        char * const keysave = (char *)key;
        key = (char*)bytes_from_utf8((U8*)key, &klen, &is_utf8);
        if (is_utf8)
            flags |= HVhek_UTF8;
        else
            flags &= ~HVhek_UTF8;
        if (key != keysave) {
            if (flags & HVhek_FREEKEY)
                Safefree(keysave);
            flags |= HVhek_WASUTF8 | HVhek_FREEKEY;
            /* If the caller calculated a hash, it was on the sequence of
               octets that are the UTF-8 form. We've now changed the sequence
               of octets stored to that of the equivalent byte representation,
               so the hash we need is different.  */
            hash = 0;
        }
    }

    if (keysv && (SvIsCOW_shared_hash(keysv))) {
        if (HvSHAREKEYS(hv))
            keysv_hek  = SvSHARED_HEK_FROM_PV(SvPVX_const(keysv));
        hash = SvSHARED_HASH(keysv);
    }
    else if (!hash)
        PERL_HASH(hash, key, klen);

#ifdef DYNAMIC_ENV_FETCH
    if (!HvARRAY(hv)) entry = NULL;
    else
#endif
    {
        entry = (HvARRAY(hv))[hash & (I32) HvMAX(hv)];
    }

    if (!entry)
        goto not_found;

    if (keysv_hek) {
        /* keysv is actually a HEK in disguise, so we can match just by
         * comparing the HEK pointers in the HE chain. There is a slight
         * caveat: on something like "\x80", which has both plain and utf8
         * representations, perl's hashes do encoding-insensitive lookups,
         * but preserve the encoding of the stored key. Thus a particular
         * key could map to two different HEKs in PL_strtab. We only
         * conclude 'not found' if all the flags are the same; otherwise
         * we fall back to a full search (this should only happen in rare
         * cases).
         */
        int keysv_flags = HEK_FLAGS(keysv_hek);
        HE  *orig_entry = entry;

        for (; entry; entry = HeNEXT(entry)) {
            HEK *hek = HeKEY_hek(entry);
            if (hek == keysv_hek)
                goto found;
            if (HEK_FLAGS(hek) != keysv_flags)
                break; /* need to do full match */
        }
        if (!entry)
            goto not_found;
        /* failed on shortcut - do full search loop */
        entry = orig_entry;
    }

    for (; entry; entry = HeNEXT(entry)) {
        if (HeHASH(entry) != hash)		/* strings can't be equal */
            continue;
        if (HeKLEN(entry) != (I32)klen)
            continue;
        if (memNE(HeKEY(entry),key,klen))	/* is this it? */
            continue;
        if ((HeKFLAGS(entry) ^ flags) & HVhek_UTF8)
            continue;

      found:
        if (action & (HV_FETCH_LVALUE|HV_FETCH_ISSTORE)) {
            if ((HeKFLAGS(entry) ^ flags) & HVhek_WASUTF8) {
                /* We match if HVhek_UTF8 bit in our flags and hash key's
                   match.  But if entry was set previously with HVhek_WASUTF8
                   and key now doesn't (or vice versa) then we should change
                   the key's flag, as this is assignment.  */
                if ((HeKFLAGS(entry) & HVhek_NOTSHARED) == 0) {
                    /* Need to swap the key we have for a key with the flags we
                       need. As keys are shared we can't just write to the
                       flag, so we share the new one, unshare the old one.  */
                    HEK * const new_hek
                        = share_hek_flags(key, klen, hash, flags & ~HVhek_FREEKEY);
                    unshare_hek (HeKEY_hek(entry));
                    HeKEY_hek(entry) = new_hek;
                }
                else if (hv == PL_strtab) {
                    /* PL_strtab is usually the only hash without HvSHAREKEYS,
                       so putting this test here is cheap  */
                    if (flags & HVhek_FREEKEY)
                        Safefree(key);
                    Perl_croak(aTHX_ S_strtab_error,
                               action & HV_FETCH_LVALUE ? "fetch" : "store");
                }
                else {
                    /* Effectively this is save_hek_flags() for a new version
                       of the HEK and Safefree() of the old rolled together. */
                    HeKFLAGS(entry) ^= HVhek_WASUTF8;
                }
                if (flags & HVhek_ENABLEHVKFLAGS)
                    HvHASKFLAGS_on(hv);
            }
            if (HeVAL(entry) == &PL_sv_placeholder) {
                /* yes, can store into placeholder slot */
                if (action & HV_FETCH_LVALUE) {
                    if (SvMAGICAL(hv)) {
                        /* This preserves behaviour with the old hv_fetch
                           implementation which at this point would bail out
                           with a break; (at "if we find a placeholder, we
                           pretend we haven't found anything")

                           That break mean that if a placeholder were found, it
                           caused a call into hv_store, which in turn would
                           check magic, and if there is no magic end up pretty
                           much back at this point (in hv_store's code).  */
                        break;
                    }
                    /* LVAL fetch which actually needs a store.  */
                    val = newSV_type(SVt_NULL);
                    HvPLACEHOLDERS(hv)--;
                } else {
                    /* store */
                    if (val != &PL_sv_placeholder)
                        HvPLACEHOLDERS(hv)--;
                }
                HeVAL(entry) = val;
            } else if (action & HV_FETCH_ISSTORE) {
                SvREFCNT_dec(HeVAL(entry));
                HeVAL(entry) = val;
            }
        } else if (HeVAL(entry) == &PL_sv_placeholder) {
            /* if we find a placeholder, we pretend we haven't found
               anything */
            break;
        }
        if (flags & HVhek_FREEKEY)
            Safefree(key);
        if (return_svp) {
            return (void *) &HeVAL(entry);
        }
        return entry;
    }

  not_found:
#ifdef DYNAMIC_ENV_FETCH  /* %ENV lookup?  If so, try to fetch the value now */
    if (!(action & HV_FETCH_ISSTORE) 
        && SvRMAGICAL((const SV *)hv)
        && mg_find((const SV *)hv, PERL_MAGIC_env)) {
        unsigned long len;
        const char * const env = PerlEnv_ENVgetenv_len(key,&len);
        if (env) {
            sv = newSVpvn(env,len);
            SvTAINTED_on(sv);
            return hv_common(hv, keysv, key, klen, flags,
                             HV_FETCH_ISSTORE|HV_DISABLE_UVAR_XKEY|return_svp,
                             sv, hash);
        }
    }
#endif

    if (!entry && SvREADONLY(hv) && !(action & HV_FETCH_ISEXISTS)) {
        hv_notallowed(flags, key, klen,
                        "Attempt to access disallowed key '%" SVf "' in"
                        " a restricted hash");
    }
    if (!(action & (HV_FETCH_LVALUE|HV_FETCH_ISSTORE))) {
        /* Not doing some form of store, so return failure.  */
        if (flags & HVhek_FREEKEY)
            Safefree(key);
        return NULL;
    }
    if (action & HV_FETCH_LVALUE) {
        val = action & HV_FETCH_EMPTY_HE ? NULL : newSV_type(SVt_NULL);
        if (SvMAGICAL(hv)) {
            /* At this point the old hv_fetch code would call to hv_store,
               which in turn might do some tied magic. So we need to make that
               magic check happen.  */
            /* gonna assign to this, so it better be there */
            /* If a fetch-as-store fails on the fetch, then the action is to
               recurse once into "hv_store". If we didn't do this, then that
               recursive call would call the key conversion routine again.
               However, as we replace the original key with the converted
               key, this would result in a double conversion, which would show
               up as a bug if the conversion routine is not idempotent.
               Hence the use of HV_DISABLE_UVAR_XKEY.  */
            return hv_common(hv, keysv, key, klen, flags,
                             HV_FETCH_ISSTORE|HV_DISABLE_UVAR_XKEY|return_svp,
                             val, hash);
            /* XXX Surely that could leak if the fetch-was-store fails?
               Just like the hv_fetch.  */
        }
    }

    /* Welcome to hv_store...  */

    if (!HvARRAY(hv)) {
        /* Not sure if we can get here.  I think the only case of oentry being
           NULL is for %ENV with dynamic env fetch.  But that should disappear
           with magic in the previous code.  */
        char *array;
        Newxz(array,
             PERL_HV_ARRAY_ALLOC_BYTES(xhv->xhv_max+1 /* HvMAX(hv)+1 */),
             char);
        HvARRAY(hv) = (HE**)array;
    }

    oentry = &(HvARRAY(hv))[hash & (I32) xhv->xhv_max];

    /* share_hek_flags will do the free for us.  This might be considered
       bad API design.  */
    if (LIKELY(HvSHAREKEYS(hv))) {
        entry = new_HE();
        HeKEY_hek(entry) = share_hek_flags(key, klen, hash, flags);
    }
    else if (UNLIKELY(hv == PL_strtab)) {
        /* PL_strtab is usually the only hash without HvSHAREKEYS, so putting
           this test here is cheap  */
        if (flags & HVhek_FREEKEY)
            Safefree(key);
        Perl_croak(aTHX_ S_strtab_error,
                   action & HV_FETCH_LVALUE ? "fetch" : "store");
    }
    else {
        /* gotta do the real thing */
        entry = new_HE();
        HeKEY_hek(entry) = save_hek_flags(key, klen, hash, flags);
    }
    HeVAL(entry) = val;
    in_collision = cBOOL(*oentry != NULL);


#ifdef PERL_HASH_RANDOMIZE_KEYS
    /* This logic semi-randomizes the insert order in a bucket.
     * Either we insert into the top, or the slot below the top,
     * making it harder to see if there is a collision. We also
     * reset the iterator randomizer if there is one.
     */


    if ( *oentry && PL_HASH_RAND_BITS_ENABLED) {
        UPDATE_HASH_RAND_BITS_KEY(key,klen);
        if ( PL_hash_rand_bits & 1 ) {
            HeNEXT(entry) = HeNEXT(*oentry);
            HeNEXT(*oentry) = entry;
        } else {
            HeNEXT(entry) = *oentry;
            *oentry = entry;
        }
    } else
#endif
    {
        HeNEXT(entry) = *oentry;
        *oentry = entry;
    }
#ifdef PERL_HASH_RANDOMIZE_KEYS
    if (HvHasAUX(hv)) {
        /* Currently this makes various tests warn in annoying ways.
         * So Silenced for now. - Yves | bogus end of comment =>* /
        if (HvAUX(hv)->xhv_riter != -1) {
            Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                             "[TESTING] Inserting into a hash during each() traversal results in undefined behavior"
                             pTHX__FORMAT
                             pTHX__VALUE);
        }
        */
        MAYBE_UPDATE_HASH_RAND_BITS_KEY(key,klen);
        HvAUX(hv)->xhv_rand= (U32)PL_hash_rand_bits;
    }
#endif

    if (val == &PL_sv_placeholder)
        HvPLACEHOLDERS(hv)++;
    if (flags & HVhek_ENABLEHVKFLAGS)
        HvHASKFLAGS_on(hv);

    xhv->xhv_keys++; /* HvTOTALKEYS(hv)++ */
    if ( in_collision && DO_HSPLIT(xhv) ) {
        const STRLEN oldsize = xhv->xhv_max + 1;
        const U32 items = (U32)HvPLACEHOLDERS_get(hv);

        if (items /* hash has placeholders  */
            && !SvREADONLY(hv) /* but is not a restricted hash */) {
            /* If this hash previously was a "restricted hash" and had
               placeholders, but the "restricted" flag has been turned off,
               then the placeholders no longer serve any useful purpose.
               However, they have the downsides of taking up RAM, and adding
               extra steps when finding used values. It's safe to clear them
               at this point, even though Storable rebuilds restricted hashes by
               putting in all the placeholders (first) before turning on the
               readonly flag, because Storable always pre-splits the hash.
               If we're lucky, then we may clear sufficient placeholders to
               avoid needing to split the hash at all.  */
            clear_placeholders(hv, items);
            if (DO_HSPLIT(xhv))
                hsplit(hv, oldsize, oldsize * 2);
        } else
            hsplit(hv, oldsize, oldsize * 2);
    }

    if (return_svp) {
        return entry ? (void *) &HeVAL(entry) : NULL;
    }
    return (void *) entry;
}

STATIC void
S_hv_magic_check(HV *hv, bool *needs_copy, bool *needs_store)
{
    const MAGIC *mg = SvMAGIC(hv);

    PERL_ARGS_ASSERT_HV_MAGIC_CHECK;

    *needs_copy = FALSE;
    *needs_store = TRUE;
    while (mg) {
        if (isUPPER(mg->mg_type)) {
            *needs_copy = TRUE;
            if (mg->mg_type == PERL_MAGIC_tied) {
                *needs_store = FALSE;
                return; /* We've set all there is to set. */
            }
        }
        mg = mg->mg_moremagic;
    }
}

/*
=for apidoc hv_scalar

Evaluates the hash in scalar context and returns the result.

When the hash is tied dispatches through to the SCALAR method,
otherwise returns a mortal SV containing the number of keys
in the hash.

Note, prior to 5.25 this function returned what is now
returned by the hv_bucket_ratio() function.

=cut
*/

SV *
Perl_hv_scalar(pTHX_ HV *hv)
{
    SV *sv;
    UV u;

    PERL_ARGS_ASSERT_HV_SCALAR;

    if (SvRMAGICAL(hv)) {
        MAGIC * const mg = mg_find((const SV *)hv, PERL_MAGIC_tied);
        if (mg)
            return magic_scalarpack(hv, mg);
    }

    sv = newSV_type_mortal(SVt_IV);

    /* Inlined sv_setuv(sv, HvUSEDKEYS(hv)) follows:*/
    u = HvUSEDKEYS(hv);

    if (u <= (UV)IV_MAX) {
        SvIV_set(sv, (IV)u);
        (void)SvIOK_only(sv);
        SvTAINT(sv);
    } else {
        SvIV_set(sv, 0);
        SvUV_set(sv, u);
        (void)SvIOK_only_UV(sv);
        SvTAINT(sv);
    }

    return sv;
}


/*
hv_pushkv(): push all the keys and/or values of a hash onto the stack.
The rough Perl equivalents:
    () = %hash;
    () = keys %hash;
    () = values %hash;

Resets the hash's iterator.

flags : 1   = push keys
        2   = push values
        1|2 = push keys and values
        XXX use symbolic flag constants at some point?
I might unroll the non-tied hv_iternext() in here at some point - DAPM
*/

void
Perl_hv_pushkv(pTHX_ HV *hv, U32 flags)
{
    HE *entry;
    bool tied = SvRMAGICAL(hv) && (mg_find(MUTABLE_SV(hv), PERL_MAGIC_tied)
#ifdef DYNAMIC_ENV_FETCH  /* might not know number of keys yet */
                                   || mg_find(MUTABLE_SV(hv), PERL_MAGIC_env)
#endif
                                  );
    dSP;

    PERL_ARGS_ASSERT_HV_PUSHKV;
    assert(flags); /* must be pushing at least one of keys and values */

    (void)hv_iterinit(hv);

    if (tied) {
        SSize_t ext = (flags == 3) ? 2 : 1;
        while ((entry = hv_iternext(hv))) {
            EXTEND(SP, ext);
            if (flags & 1)
                PUSHs(hv_iterkeysv(entry));
            if (flags & 2)
                PUSHs(hv_iterval(hv, entry));
        }
    }
    else {
        Size_t nkeys = HvUSEDKEYS(hv);
        SSize_t ext;

        if (!nkeys)
            return;

        /* 2*nkeys() should never be big enough to truncate or wrap */
        assert(nkeys <= (SSize_t_MAX >> 1));
        ext = nkeys * ((flags == 3) ? 2 : 1);

        EXTEND_MORTAL(nkeys);
        EXTEND(SP, ext);

        while ((entry = hv_iternext(hv))) {
            if (flags & 1) {
                SV *keysv = newSVhek(HeKEY_hek(entry));
                SvTEMP_on(keysv);
                PL_tmps_stack[++PL_tmps_ix] = keysv;
                PUSHs(keysv);
            }
            if (flags & 2)
                PUSHs(HeVAL(entry));
        }
    }

    PUTBACK;
}


/*
=for apidoc hv_bucket_ratio

If the hash is tied dispatches through to the SCALAR tied method,
otherwise if the hash contains no keys returns 0, otherwise returns
a mortal sv containing a string specifying the number of used buckets,
followed by a slash, followed by the number of available buckets.

This function is expensive, it must scan all of the buckets
to determine which are used, and the count is NOT cached.
In a large hash this could be a lot of buckets.

=cut
*/

SV *
Perl_hv_bucket_ratio(pTHX_ HV *hv)
{
    SV *sv;

    PERL_ARGS_ASSERT_HV_BUCKET_RATIO;

    if (SvRMAGICAL(hv)) {
        MAGIC * const mg = mg_find((const SV *)hv, PERL_MAGIC_tied);
        if (mg)
            return magic_scalarpack(hv, mg);
    }

    if (HvUSEDKEYS((HV *)hv)) {
        sv = sv_newmortal();
        Perl_sv_setpvf(aTHX_ sv, "%ld/%ld",
                (long)HvFILL(hv), (long)HvMAX(hv) + 1);
    }
    else
        sv = &PL_sv_zero;
    
    return sv;
}

/*
=for apidoc hv_delete

Deletes a key/value pair in the hash.  The value's SV is removed from
the hash, made mortal, and returned to the caller.  The absolute
value of C<klen> is the length of the key.  If C<klen> is negative the
key is assumed to be in UTF-8-encoded Unicode.  The C<flags> value
will normally be zero; if set to C<G_DISCARD> then C<NULL> will be returned.
C<NULL> will also be returned if the key is not found.

=for apidoc hv_delete_ent

Deletes a key/value pair in the hash.  The value SV is removed from the hash,
made mortal, and returned to the caller.  The C<flags> value will normally be
zero; if set to C<G_DISCARD> then C<NULL> will be returned.  C<NULL> will also
be returned if the key is not found.  C<hash> can be a valid precomputed hash
value, or 0 to ask for it to be computed.

=cut
*/

STATIC SV *
S_hv_delete_common(pTHX_ HV *hv, SV *keysv, const char *key, STRLEN klen,
                   int k_flags, I32 d_flags, U32 hash)
{
    XPVHV* xhv;
    HE *entry;
    HE **oentry;
    HE **first_entry;
    bool is_utf8 = cBOOL(k_flags & HVhek_UTF8);
    HEK *keysv_hek = NULL;
    U8 mro_changes = 0; /* 1 = isa; 2 = package moved */
    SV *sv;
    GV *gv = NULL;
    HV *stash = NULL;

    if (SvMAGICAL(hv)) {
        bool needs_copy;
        bool needs_store;
        hv_magic_check (hv, &needs_copy, &needs_store);

        if (needs_copy) {
            SV *sv;
            entry = (HE *) hv_common(hv, keysv, key, klen,
                                     k_flags & ~HVhek_FREEKEY,
                                     HV_FETCH_LVALUE|HV_DISABLE_UVAR_XKEY,
                                     NULL, hash);
            sv = entry ? HeVAL(entry) : NULL;
            if (sv) {
                if (SvMAGICAL(sv)) {
                    mg_clear(sv);
                }
                if (!needs_store) {
                    if (mg_find(sv, PERL_MAGIC_tiedelem)) {
                        /* No longer an element */
                        sv_unmagic(sv, PERL_MAGIC_tiedelem);
                        return sv;
                    }		
                    return NULL;		/* element cannot be deleted */
                }
#ifdef ENV_IS_CASELESS
                else if (mg_find((const SV *)hv, PERL_MAGIC_env)) {
                    /* XXX This code isn't UTF8 clean.  */
                    keysv = newSVpvn_flags(key, klen, SVs_TEMP);
                    if (k_flags & HVhek_FREEKEY) {
                        Safefree(key);
                    }
                    key = strupr(SvPVX(keysv));
                    is_utf8 = 0;
                    k_flags = 0;
                    hash = 0;
                }
#endif
            }
        }
    }
    xhv = (XPVHV*)SvANY(hv);
    if (!HvTOTALKEYS(hv))
        return NULL;

    if (is_utf8 && !(k_flags & HVhek_KEYCANONICAL)) {
        const char * const keysave = key;
        key = (char*)bytes_from_utf8((U8*)key, &klen, &is_utf8);

        if (is_utf8)
            k_flags |= HVhek_UTF8;
        else
            k_flags &= ~HVhek_UTF8;
        if (key != keysave) {
            if (k_flags & HVhek_FREEKEY) {
                /* This shouldn't happen if our caller does what we expect,
                   but strictly the API allows it.  */
                Safefree(keysave);
            }
            k_flags |= HVhek_WASUTF8 | HVhek_FREEKEY;
        }
    }

    if (keysv && (SvIsCOW_shared_hash(keysv))) {
        if (HvSHAREKEYS(hv))
            keysv_hek  = SvSHARED_HEK_FROM_PV(SvPVX_const(keysv));
        hash = SvSHARED_HASH(keysv);
    }
    else if (!hash)
        PERL_HASH(hash, key, klen);

    first_entry = oentry = &(HvARRAY(hv))[hash & (I32) HvMAX(hv)];
    entry = *oentry;

    if (!entry)
        goto not_found;

    if (keysv_hek) {
        /* keysv is actually a HEK in disguise, so we can match just by
         * comparing the HEK pointers in the HE chain. There is a slight
         * caveat: on something like "\x80", which has both plain and utf8
         * representations, perl's hashes do encoding-insensitive lookups,
         * but preserve the encoding of the stored key. Thus a particular
         * key could map to two different HEKs in PL_strtab. We only
         * conclude 'not found' if all the flags are the same; otherwise
         * we fall back to a full search (this should only happen in rare
         * cases).
         */
        int keysv_flags = HEK_FLAGS(keysv_hek);

        for (; entry; oentry = &HeNEXT(entry), entry = *oentry) {
            HEK *hek = HeKEY_hek(entry);
            if (hek == keysv_hek)
                goto found;
            if (HEK_FLAGS(hek) != keysv_flags)
                break; /* need to do full match */
        }
        if (!entry)
            goto not_found;
        /* failed on shortcut - do full search loop */
        oentry = first_entry;
        entry = *oentry;
    }

    for (; entry; oentry = &HeNEXT(entry), entry = *oentry) {
        if (HeHASH(entry) != hash)		/* strings can't be equal */
            continue;
        if (HeKLEN(entry) != (I32)klen)
            continue;
        if (memNE(HeKEY(entry),key,klen))	/* is this it? */
            continue;
        if ((HeKFLAGS(entry) ^ k_flags) & HVhek_UTF8)
            continue;

      found:
        if (hv == PL_strtab) {
            if (k_flags & HVhek_FREEKEY)
                Safefree(key);
            Perl_croak(aTHX_ S_strtab_error, "delete");
        }

        sv = HeVAL(entry);

        /* if placeholder is here, it's already been deleted.... */
        if (sv == &PL_sv_placeholder) {
            if (k_flags & HVhek_FREEKEY)
                Safefree(key);
            return NULL;
        }
        if (SvREADONLY(hv) && sv && SvREADONLY(sv)) {
            hv_notallowed(k_flags, key, klen,
                            "Attempt to delete readonly key '%" SVf "' from"
                            " a restricted hash");
        }

        /*
         * If a restricted hash, rather than really deleting the entry, put
         * a placeholder there. This marks the key as being "approved", so
         * we can still access via not-really-existing key without raising
         * an error.
         */
        if (SvREADONLY(hv)) {
            /* We'll be saving this slot, so the number of allocated keys
             * doesn't go down, but the number placeholders goes up */
            HeVAL(entry) = &PL_sv_placeholder;
            HvPLACEHOLDERS(hv)++;
        }
        else {
            HeVAL(entry) = NULL;
            *oentry = HeNEXT(entry);
            if (HvHasAUX(hv) && entry == HvAUX(hv)->xhv_eiter /* HvEITER(hv) */) {
                HvLAZYDEL_on(hv);
            }
            else {
                if (HvHasAUX(hv) && HvLAZYDEL(hv) &&
                    entry == HeNEXT(HvAUX(hv)->xhv_eiter))
                    HeNEXT(HvAUX(hv)->xhv_eiter) = HeNEXT(entry);
                hv_free_ent(NULL, entry);
            }
            xhv->xhv_keys--; /* HvTOTALKEYS(hv)-- */
            if (xhv->xhv_keys == 0)
                HvHASKFLAGS_off(hv);
        }

        /* If this is a stash and the key ends with ::, then someone is 
         * deleting a package.
         */
        if (sv && SvTYPE(sv) == SVt_PVGV && HvHasENAME(hv)) {
                gv = (GV *)sv;
                if ((
                     (klen > 1 && key[klen-2] == ':' && key[klen-1] == ':')
                      ||
                     (klen == 1 && key[0] == ':')
                    )
                 && (klen != 6 || hv!=PL_defstash || memNE(key,"main::",6))
                 && (stash = GvHV((GV *)gv))
                 && HvHasENAME(stash)) {
                        /* A previous version of this code checked that the
                         * GV was still in the symbol table by fetching the
                         * GV with its name. That is not necessary (and
                         * sometimes incorrect), as HvENAME cannot be set
                         * on hv if it is not in the symtab. */
                        mro_changes = 2;
                        /* Hang on to it for a bit. */
                        SvREFCNT_inc_simple_void_NN(
                         sv_2mortal((SV *)gv)
                        );
                }
                else if (memEQs(key, klen, "ISA") && GvAV(gv)) {
                    AV *isa = GvAV(gv);
                    MAGIC *mg = mg_find((SV*)isa, PERL_MAGIC_isa);

                    mro_changes = 1;
                    if (mg) {
                        if (mg->mg_obj == (SV*)gv) {
                            /* This is the only stash this ISA was used for.
                             * The isaelem magic asserts if there's no
                             * isa magic on the array, so explicitly
                             * remove the magic on both the array and its
                             * elements.  @ISA shouldn't be /too/ large.
                             */
                            SV **svp, **end;
                        strip_magic:
                            svp = AvARRAY(isa);
                            if (svp) {
                                end = svp + (AvFILLp(isa)+1);
                                while (svp < end) {
                                    if (*svp)
                                        mg_free_type(*svp, PERL_MAGIC_isaelem);
                                    ++svp;
                                }
                            }
                            mg_free_type((SV*)GvAV(gv), PERL_MAGIC_isa);
                        }
                        else {
                            /* mg_obj is an array of stashes
                               Note that the array doesn't keep a reference
                               count on the stashes.
                             */
                            AV *av = (AV*)mg->mg_obj;
                            SV **svp, **arrayp;
                            SSize_t index;
                            SSize_t items;

                            assert(SvTYPE(mg->mg_obj) == SVt_PVAV);

                            /* remove the stash from the magic array */
                            arrayp = svp = AvARRAY(av);
                            items = AvFILLp(av) + 1;
                            if (items == 1) {
                                assert(*arrayp == (SV *)gv);
                                mg->mg_obj = NULL;
                                /* avoid a double free on the last stash */
                                AvFILLp(av) = -1;
                                /* The magic isn't MGf_REFCOUNTED, so release
                                 * the array manually.
                                 */
                                SvREFCNT_dec_NN(av);
                                goto strip_magic;
                            }
                            else {
                                while (items--) {
                                    if (*svp == (SV*)gv)
                                        break;
                                    ++svp;
                                }
                                index = svp - arrayp;
                                assert(index >= 0 && index <= AvFILLp(av));
                                if (index < AvFILLp(av)) {
                                    arrayp[index] = arrayp[AvFILLp(av)];
                                }
                                arrayp[AvFILLp(av)] = NULL;
                                --AvFILLp(av);
                            }
                        }
                    }
                }
        }

        if (k_flags & HVhek_FREEKEY)
            Safefree(key);

        if (sv) {
            /* deletion of method from stash */
            if (isGV(sv) && isGV_with_GP(sv) && GvCVu(sv)
             && HvHasENAME(hv))
                mro_method_changed_in(hv);

            if (d_flags & G_DISCARD) {
                SvREFCNT_dec(sv);
                sv = NULL;
            }
            else {
                sv_2mortal(sv);
            }
        }

        if (mro_changes == 1) mro_isa_changed_in(hv);
        else if (mro_changes == 2)
            mro_package_moved(NULL, stash, gv, 1);

        return sv;
    }

  not_found:
    if (SvREADONLY(hv)) {
        hv_notallowed(k_flags, key, klen,
                        "Attempt to delete disallowed key '%" SVf "' from"
                        " a restricted hash");
    }

    if (k_flags & HVhek_FREEKEY)
        Safefree(key);
    return NULL;
}

/* HVs are used for (at least) three things
   1) objects
   2) symbol tables
   3) associative arrays

   shared hash keys benefit the first two greatly, because keys are likely
   to be re-used between objects, or for constants in the optree

   However, for large associative arrays (lookup tables, "seen" hashes) keys are
   unlikely to be re-used. Hence having those keys in the shared string table as
   well as the hash is a memory hit, if they are never actually shared with a
   second hash. Hence we turn off shared hash keys if a (regular) hash gets
   large.

   This is a heuristic. There might be a better answer than 42, but for now
   we'll use it.

   NOTE: Configure with -Accflags='-DPERL_USE_UNSHARED_KEYS_IN_LARGE_HASHES'
   to enable this new functionality.
*/

#ifdef PERL_USE_UNSHARED_KEYS_IN_LARGE_HASHES
static bool
S_large_hash_heuristic(pTHX_ HV *hv, STRLEN size) {
    if (size > 42
        && !SvOBJECT(hv)
        && !(HvHasAUX(hv) && HvENAME_get(hv))) {
        /* This hash appears to be growing quite large.
           We gamble that it is not sharing keys with other hashes. */
        return TRUE;
    }
    return FALSE;
}
#endif

STATIC void
S_hsplit(pTHX_ HV *hv, STRLEN const oldsize, STRLEN newsize)
{
    STRLEN i = 0;
    char *a = (char*) HvARRAY(hv);
    HE **aep;

    PERL_ARGS_ASSERT_HSPLIT;
    if (newsize > MAX_BUCKET_MAX+1)
            return;

    PL_nomemok = TRUE;
    Renew(a, PERL_HV_ARRAY_ALLOC_BYTES(newsize), char);
    PL_nomemok = FALSE;
    if (!a) {
      return;
    }

#ifdef PERL_HASH_RANDOMIZE_KEYS
    /* the idea of this is that we create a "random" value by hashing the address of
     * the array, we then use the low bit to decide if we insert at the top, or insert
     * second from top. After each such insert we rotate the hashed value. So we can
     * use the same hashed value over and over, and in normal build environments use
     * very few ops to do so. ROTL32() should produce a single machine operation. */
    MAYBE_UPDATE_HASH_RAND_BITS();
#endif
    HvARRAY(hv) = (HE**) a;
    HvMAX(hv) = newsize - 1;
    /* now we can safely clear the second half */
    Zero(&a[oldsize * sizeof(HE*)], (newsize-oldsize) * sizeof(HE*), char);	/* zero 2nd half*/

    if (!HvTOTALKEYS(hv))       /* skip rest if no entries */
        return;

    /* don't share keys in large simple hashes */
    if (LARGE_HASH_HEURISTIC(hv, HvTOTALKEYS(hv)))
        HvSHAREKEYS_off(hv);


    newsize--;
    aep = (HE**)a;
    do {
        HE **oentry = aep + i;
        HE *entry = aep[i];

        if (!entry)				/* non-existent */
            continue;
        do {
            U32 j = (HeHASH(entry) & newsize);
            if (j != (U32)i) {
                *oentry = HeNEXT(entry);
#ifdef PERL_HASH_RANDOMIZE_KEYS
                /* if the target cell is empty or PL_HASH_RAND_BITS_ENABLED is false
                 * insert to top, otherwise rotate the bucket rand 1 bit,
                 * and use the new low bit to decide if we insert at top,
                 * or next from top. IOW, we only rotate on a collision.*/
                if (aep[j] && PL_HASH_RAND_BITS_ENABLED) {
                    UPDATE_HASH_RAND_BITS();
                    if (PL_hash_rand_bits & 1) {
                        HeNEXT(entry)= HeNEXT(aep[j]);
                        HeNEXT(aep[j])= entry;
                    } else {
                        /* Note, this is structured in such a way as the optimizer
                        * should eliminate the duplicated code here and below without
                        * us needing to explicitly use a goto. */
                        HeNEXT(entry) = aep[j];
                        aep[j] = entry;
                    }
                } else
#endif
                {
                    /* see comment above about duplicated code */
                    HeNEXT(entry) = aep[j];
                    aep[j] = entry;
                }
            }
            else {
                oentry = &HeNEXT(entry);
            }
            entry = *oentry;
        } while (entry);
    } while (i++ < oldsize);
}

/*
=for apidoc hv_ksplit

Attempt to grow the hash C<hv> so it has at least C<newmax> buckets available.
Perl chooses the actual number for its convenience.

This is the same as doing the following in Perl code:

 keys %hv = newmax;

=cut
*/

void
Perl_hv_ksplit(pTHX_ HV *hv, IV newmax)
{
    XPVHV* xhv = (XPVHV*)SvANY(hv);
    const I32 oldsize = (I32) xhv->xhv_max+1;       /* HvMAX(hv)+1 */
    I32 newsize;
    I32 wantsize;
    I32 trysize;
    char *a;

    PERL_ARGS_ASSERT_HV_KSPLIT;

    wantsize = (I32) newmax;                            /* possible truncation here */
    if (wantsize != newmax)
        return;

    wantsize= wantsize + (wantsize >> 1);           /* wantsize *= 1.5 */
    if (wantsize < newmax)                          /* overflow detection */
        return;

    newsize = oldsize;
    while (wantsize > newsize) {
        trysize = newsize << 1;
        if (trysize > newsize) {
            newsize = trysize;
        } else {
            /* we overflowed */
            return;
        }
    }

    if (newsize <= oldsize)
        return;                                            /* overflow detection */

    a = (char *) HvARRAY(hv);
    if (a) {
#ifdef PERL_HASH_RANDOMIZE_KEYS
        U32 was_ook = HvHasAUX(hv);
#endif
        hsplit(hv, oldsize, newsize);
#ifdef PERL_HASH_RANDOMIZE_KEYS
        if (was_ook && HvHasAUX(hv) && HvTOTALKEYS(hv)) {
            MAYBE_UPDATE_HASH_RAND_BITS();
            HvAUX(hv)->xhv_rand = (U32)PL_hash_rand_bits;
        }
#endif
    } else {
        if (LARGE_HASH_HEURISTIC(hv, newmax))
            HvSHAREKEYS_off(hv);
        Newxz(a, PERL_HV_ARRAY_ALLOC_BYTES(newsize), char);
        xhv->xhv_max = newsize - 1;
        HvARRAY(hv) = (HE **) a;
    }
}

/* IMO this should also handle cases where hv_max is smaller than hv_keys
 * as tied hashes could play silly buggers and mess us around. We will
 * do the right thing during hv_store() afterwards, but still - Yves */
#define HV_SET_MAX_ADJUSTED_FOR_KEYS(hv,hv_max,hv_keys) STMT_START {\
    /* Can we use fewer buckets? (hv_max is always 2^n-1) */        \
    if (hv_max < PERL_HASH_DEFAULT_HvMAX) {                         \
        hv_max = PERL_HASH_DEFAULT_HvMAX;                           \
    } else {                                                        \
        while (hv_max > PERL_HASH_DEFAULT_HvMAX && hv_max + 1 >= hv_keys * 2) \
            hv_max = hv_max / 2;                                    \
    }                                                               \
    HvMAX(hv) = hv_max;                                             \
} STMT_END


/*
=for apidoc newHVhv

The content of C<ohv> is copied to a new hash.  A pointer to the new hash is
returned.

=cut
*/

HV *
Perl_newHVhv(pTHX_ HV *ohv)
{
    HV * const hv = newHV();
    STRLEN hv_max;

    if (!ohv || (!HvTOTALKEYS(ohv) && !SvMAGICAL((const SV *)ohv)))
        return hv;
    hv_max = HvMAX(ohv);

    if (!SvMAGICAL((const SV *)ohv)) {
        /* It's an ordinary hash, so copy it fast. AMS 20010804 */
        STRLEN i;
        HE **ents, ** const oents = (HE **)HvARRAY(ohv);
        char *a;
        Newx(a, PERL_HV_ARRAY_ALLOC_BYTES(hv_max+1), char);
        ents = (HE**)a;

        if (HvSHAREKEYS(ohv)) {
#ifdef NODEFAULT_SHAREKEYS
            HvSHAREKEYS_on(hv);
#else
            /* Shared is the default - it should have been set by newHV(). */
            assert(HvSHAREKEYS(hv));
#endif
        }
        else {
            HvSHAREKEYS_off(hv);
        }

        /* In each bucket... */
        for (i = 0; i <= hv_max; i++) {
            HE *prev = NULL;
            HE *oent = oents[i];

            if (!oent) {
                ents[i] = NULL;
                continue;
            }

            /* Copy the linked list of entries. */
            for (; oent; oent = HeNEXT(oent)) {
                HE * const ent   = new_HE();
                SV *const val    = HeVAL(oent);
                const int flags  = HeKFLAGS(oent);

                HeVAL(ent) = SvIMMORTAL(val) ? val : newSVsv(val);
                if ((flags & HVhek_NOTSHARED) == 0) {
                    HeKEY_hek(ent) = share_hek_hek(HeKEY_hek(oent));
                }
                else {
                    const U32 hash   = HeHASH(oent);
                    const char * const key = HeKEY(oent);
                    const STRLEN len = HeKLEN(oent);
                    HeKEY_hek(ent) = save_hek_flags(key, len, hash, flags);
                }
                if (prev)
                    HeNEXT(prev) = ent;
                else
                    ents[i] = ent;
                prev = ent;
                HeNEXT(ent) = NULL;
            }
        }

        HvMAX(hv)   = hv_max;
        HvTOTALKEYS(hv)  = HvTOTALKEYS(ohv);
        HvARRAY(hv) = ents;
    } /* not magical */
    else {
        /* Iterate over ohv, copying keys and values one at a time. */
        HE *entry;
        const I32 riter = HvRITER_get(ohv);
        HE * const eiter = HvEITER_get(ohv);
        STRLEN hv_keys = HvTOTALKEYS(ohv);

        HV_SET_MAX_ADJUSTED_FOR_KEYS(hv,hv_max,hv_keys);

        hv_iterinit(ohv);
        while ((entry = hv_iternext_flags(ohv, 0))) {
            SV *val = hv_iterval(ohv,entry);
            SV * const keysv = HeSVKEY(entry);
            val = SvIMMORTAL(val) ? val : newSVsv(val);
            if (keysv)
                (void)hv_store_ent(hv, keysv, val, 0);
            else
                (void)hv_store_flags(hv, HeKEY(entry), HeKLEN(entry), val,
                                 HeHASH(entry), HeKFLAGS(entry));
        }
        HvRITER_set(ohv, riter);
        HvEITER_set(ohv, eiter);
    }

    return hv;
}

/*
=for apidoc hv_copy_hints_hv

A specialised version of L</newHVhv> for copying C<%^H>.  C<ohv> must be
a pointer to a hash (which may have C<%^H> magic, but should be generally
non-magical), or C<NULL> (interpreted as an empty hash).  The content
of C<ohv> is copied to a new hash, which has the C<%^H>-specific magic
added to it.  A pointer to the new hash is returned.

=cut
*/

HV *
Perl_hv_copy_hints_hv(pTHX_ HV *const ohv)
{
    HV * const hv = newHV();

    if (ohv) {
        STRLEN hv_max = HvMAX(ohv);
        STRLEN hv_keys = HvTOTALKEYS(ohv);
        HE *entry;
        const I32 riter = HvRITER_get(ohv);
        HE * const eiter = HvEITER_get(ohv);

        ENTER;
        SAVEFREESV(hv);

        HV_SET_MAX_ADJUSTED_FOR_KEYS(hv,hv_max,hv_keys);

        hv_iterinit(ohv);
        while ((entry = hv_iternext_flags(ohv, 0))) {
            SV *const sv = newSVsv(hv_iterval(ohv,entry));
            SV *heksv = HeSVKEY(entry);
            if (!heksv && sv) heksv = newSVhek(HeKEY_hek(entry));
            if (sv) sv_magic(sv, NULL, PERL_MAGIC_hintselem,
                     (char *)heksv, HEf_SVKEY);
            if (heksv == HeSVKEY(entry))
                (void)hv_store_ent(hv, heksv, sv, 0);
            else {
                (void)hv_common(hv, heksv, HeKEY(entry), HeKLEN(entry),
                                 HeKFLAGS(entry), HV_FETCH_ISSTORE|HV_FETCH_JUST_SV, sv, HeHASH(entry));
                SvREFCNT_dec_NN(heksv);
            }
        }
        HvRITER_set(ohv, riter);
        HvEITER_set(ohv, eiter);

        SvREFCNT_inc_simple_void_NN(hv);
        LEAVE;
    }
    hv_magic(hv, NULL, PERL_MAGIC_hints);
    return hv;
}
#undef HV_SET_MAX_ADJUSTED_FOR_KEYS

/* like hv_free_ent, but returns the SV rather than freeing it */
STATIC SV*
S_hv_free_ent_ret(pTHX_ HE *entry)
{
    PERL_ARGS_ASSERT_HV_FREE_ENT_RET;

    SV *val = HeVAL(entry);
    if (HeKLEN(entry) == HEf_SVKEY) {
        SvREFCNT_dec(HeKEY_sv(entry));
        Safefree(HeKEY_hek(entry));
    }
    else if ((HeKFLAGS(entry) & HVhek_NOTSHARED) == 0) {
        unshare_hek(HeKEY_hek(entry));
    }
    else {
        Safefree(HeKEY_hek(entry));
    }
    del_HE(entry);
    return val;
}


void
Perl_hv_free_ent(pTHX_ HV *notused, HE *entry)
{
    PERL_UNUSED_ARG(notused);

    if (!entry)
        return;

    SV *val = hv_free_ent_ret(entry);
    SvREFCNT_dec(val);
}


void
Perl_hv_delayfree_ent(pTHX_ HV *notused, HE *entry)
{
    PERL_UNUSED_ARG(notused);

    if (!entry)
        return;
    /* SvREFCNT_inc to counter the SvREFCNT_dec in hv_free_ent  */
    sv_2mortal(SvREFCNT_inc(HeVAL(entry)));	/* free between statements */
    if (HeKLEN(entry) == HEf_SVKEY) {
        sv_2mortal(SvREFCNT_inc(HeKEY_sv(entry)));
    }
    hv_free_ent(NULL, entry);
}

/*
=for apidoc hv_clear

Frees all the elements of a hash, leaving it empty.
The XS equivalent of C<%hash = ()>.  See also L</hv_undef>.

See L</av_clear> for a note about the hash possibly being invalid on
return.

=cut
*/

void
Perl_hv_clear(pTHX_ HV *hv)
{
    SSize_t orig_ix;

    if (!hv)
        return;

    DEBUG_A(Perl_hv_assert(aTHX_ hv));

    /* avoid hv being freed when calling destructors below */
    EXTEND_MORTAL(1);
    PL_tmps_stack[++PL_tmps_ix] = SvREFCNT_inc_simple_NN(hv);
    orig_ix = PL_tmps_ix;
    if (SvREADONLY(hv) && HvTOTALKEYS(hv)) {
        /* restricted hash: convert all keys to placeholders */
        STRLEN max = HvMAX(hv);
        STRLEN i;
        for (i = 0; i <= max; i++) {
            HE *entry = (HvARRAY(hv))[i];
            for (; entry; entry = HeNEXT(entry)) {
                /* not already placeholder */
                if (HeVAL(entry) != &PL_sv_placeholder) {
                    if (HeVAL(entry)) {
                        if (SvREADONLY(HeVAL(entry))) {
                            SV* const keysv = hv_iterkeysv(entry);
                            Perl_croak_nocontext(
                                "Attempt to delete readonly key '%" SVf "' from a restricted hash",
                                (void*)keysv);
                        }
                        SvREFCNT_dec_NN(HeVAL(entry));
                    }
                    HeVAL(entry) = &PL_sv_placeholder;
                    HvPLACEHOLDERS(hv)++;
                }
            }
        }
    }
    else {
        hv_free_entries(hv);
        HvPLACEHOLDERS_set(hv, 0);

        if (SvRMAGICAL(hv))
            mg_clear(MUTABLE_SV(hv));

        HvHASKFLAGS_off(hv);
    }
    if (HvHasAUX(hv)) {
        if(HvENAME_get(hv))
            mro_isa_changed_in(hv);
        HvEITER_set(hv, NULL);
    }
    /* disarm hv's premature free guard */
    if (LIKELY(PL_tmps_ix == orig_ix))
        PL_tmps_ix--;
    else
        PL_tmps_stack[orig_ix] = &PL_sv_undef;
    SvREFCNT_dec_NN(hv);
}

/*
=for apidoc hv_clear_placeholders

Clears any placeholders from a hash.  If a restricted hash has any of its keys
marked as readonly and the key is subsequently deleted, the key is not actually
deleted but is marked by assigning it a value of C<&PL_sv_placeholder>.  This tags
it so it will be ignored by future operations such as iterating over the hash,
but will still allow the hash to have a value reassigned to the key at some
future point.  This function clears any such placeholder keys from the hash.
See C<L<Hash::Util::lock_keys()|Hash::Util/lock_keys>> for an example of its
use.

=cut
*/

void
Perl_hv_clear_placeholders(pTHX_ HV *hv)
{
    const U32 items = (U32)HvPLACEHOLDERS_get(hv);

    PERL_ARGS_ASSERT_HV_CLEAR_PLACEHOLDERS;

    if (items)
        clear_placeholders(hv, items);
}

static void
S_clear_placeholders(pTHX_ HV *hv, const U32 placeholders)
{
    I32 i;
    U32 to_find = placeholders;

    PERL_ARGS_ASSERT_CLEAR_PLACEHOLDERS;

    assert(to_find);

    i = HvMAX(hv);
    do {
        /* Loop down the linked list heads  */
        HE **oentry = &(HvARRAY(hv))[i];
        HE *entry;

        while ((entry = *oentry)) {
            if (HeVAL(entry) == &PL_sv_placeholder) {
                *oentry = HeNEXT(entry);
                if (entry == HvEITER_get(hv))
                    HvLAZYDEL_on(hv);
                else {
                    if (HvHasAUX(hv) && HvLAZYDEL(hv) &&
                        entry == HeNEXT(HvAUX(hv)->xhv_eiter))
                        HeNEXT(HvAUX(hv)->xhv_eiter) = HeNEXT(entry);
                    hv_free_ent(NULL, entry);
                }

                if (--to_find == 0) {
                    /* Finished.  */
                    HvTOTALKEYS(hv) -= (IV)placeholders;
                    if (HvTOTALKEYS(hv) == 0)
                        HvHASKFLAGS_off(hv);
                    HvPLACEHOLDERS_set(hv, 0);
                    return;
                }
            } else {
                oentry = &HeNEXT(entry);
            }
        }
    } while (--i >= 0);
    /* You can't get here, hence assertion should always fail.  */
    assert (to_find == 0);
    NOT_REACHED; /* NOTREACHED */
}

STATIC void
S_hv_free_entries(pTHX_ HV *hv)
{
    STRLEN index = 0;
    SV *sv;

    PERL_ARGS_ASSERT_HV_FREE_ENTRIES;

    while ((sv = Perl_hfree_next_entry(aTHX_ hv, &index)) || HvTOTALKEYS(hv)) {
        SvREFCNT_dec(sv);
    }
}


/* hfree_next_entry()
 * For use only by S_hv_free_entries() and sv_clear().
 * Delete the next available HE from hv and return the associated SV.
 * Returns null on empty hash. Nevertheless null is not a reliable
 * indicator that the hash is empty, as the deleted entry may have a
 * null value.
 * indexp is a pointer to the current index into HvARRAY. The index should
 * initially be set to 0. hfree_next_entry() may update it.  */

SV*
Perl_hfree_next_entry(pTHX_ HV *hv, STRLEN *indexp)
{
    struct xpvhv_aux *iter;
    HE *entry;
    HE ** array;
#ifdef DEBUGGING
    STRLEN orig_index = *indexp;
#endif

    PERL_ARGS_ASSERT_HFREE_NEXT_ENTRY;

    if (HvHasAUX(hv) && ((iter = HvAUX(hv)))) {
        if ((entry = iter->xhv_eiter)) {
            /* the iterator may get resurrected after each
             * destructor call, so check each time */
            if (entry && HvLAZYDEL(hv)) {	/* was deleted earlier? */
                HvLAZYDEL_off(hv);
                hv_free_ent(NULL, entry);
                /* warning: at this point HvARRAY may have been
                 * re-allocated, HvMAX changed etc */
            }
            iter->xhv_riter = -1; 	/* HvRITER(hv) = -1 */
            iter->xhv_eiter = NULL;	/* HvEITER(hv) = NULL */
#ifdef PERL_HASH_RANDOMIZE_KEYS
            iter->xhv_last_rand = iter->xhv_rand;
#endif
        }
    }

    if (!((XPVHV*)SvANY(hv))->xhv_keys)
        return NULL;

    array = HvARRAY(hv);
    assert(array);
    while ( ! ((entry = array[*indexp])) ) {
        if ((*indexp)++ >= HvMAX(hv))
            *indexp = 0;
        assert(*indexp != orig_index);
    }
    array[*indexp] = HeNEXT(entry);
    ((XPVHV*) SvANY(hv))->xhv_keys--;

    if (   PL_phase != PERL_PHASE_DESTRUCT && HvHasENAME(hv)
        && HeVAL(entry) && isGV(HeVAL(entry))
        && GvHV(HeVAL(entry)) && HvHasENAME(GvHV(HeVAL(entry)))
    ) {
        STRLEN klen;
        const char * const key = HePV(entry,klen);
        if ((klen > 1 && key[klen-1]==':' && key[klen-2]==':')
         || (klen == 1 && key[0] == ':')) {
            mro_package_moved(
             NULL, GvHV(HeVAL(entry)),
             (GV *)HeVAL(entry), 0
            );
        }
    }
    return hv_free_ent_ret(entry);
}


/*
=for apidoc hv_undef

Undefines the hash.  The XS equivalent of C<undef(%hash)>.

As well as freeing all the elements of the hash (like C<hv_clear()>), this
also frees any auxiliary data and storage associated with the hash.

See L</av_clear> for a note about the hash possibly being invalid on
return.

=cut
*/

void
Perl_hv_undef_flags(pTHX_ HV *hv, U32 flags)
{
    bool save;
    SSize_t orig_ix = PL_tmps_ix; /* silence compiler warning about uninitialized vars */

    if (!hv)
        return;
    save = cBOOL(SvREFCNT(hv));
    DEBUG_A(Perl_hv_assert(aTHX_ hv));

    /* The name must be deleted before the call to hv_free_entries so that
       CVs are anonymised properly. But the effective name must be pre-
       served until after that call (and only deleted afterwards if the
       call originated from sv_clear). For stashes with one name that is
       both the canonical name and the effective name, hv_name_set has to
       allocate an array for storing the effective name. We can skip that
       during global destruction, as it does not matter where the CVs point
       if they will be freed anyway. */
    /* note that the code following prior to hv_free_entries is duplicated
     * in sv_clear(), and changes here should be done there too */
    if (PL_phase != PERL_PHASE_DESTRUCT && HvHasNAME(hv)) {
        if (PL_stashcache) {
            DEBUG_o(Perl_deb(aTHX_ "hv_undef_flags clearing PL_stashcache for '%"
                             HEKf "'\n", HEKfARG(HvNAME_HEK(hv))));
            (void)hv_deletehek(PL_stashcache, HvNAME_HEK(hv), G_DISCARD);
        }
        hv_name_set(hv, NULL, 0, 0);
    }
    if (save) {
        /* avoid hv being freed when calling destructors below */
        EXTEND_MORTAL(1);
        PL_tmps_stack[++PL_tmps_ix] = SvREFCNT_inc_simple_NN(hv);
        orig_ix = PL_tmps_ix;
    }

    /* As well as any/all HE*s in HvARRAY(), this call also ensures that
       xhv_eiter is NULL, including handling the case of a tied hash partway
       through iteration where HvLAZYDEL() is true and xhv_eiter points to an
       HE* that needs to be explicitly freed. */
    hv_free_entries(hv);

    /* HvHasAUX() is true for a hash if it has struct xpvhv_aux allocated. That
       structure has several other pieces of allocated memory - hence those must
       be freed before the structure itself can be freed. Some can be freed when
       a hash is "undefined" (this function), but some must persist until it is
       destroyed (which might be this function's immediate caller).

       Hence the code in this block frees what it is logical to free (and NULLs
       out anything freed) so that the structure is left in a logically
       consistent state - pointers are NULL or point to valid memory, and
       non-pointer values are correct for an empty hash. The structure state
       must remain consistent, because this code can no longer clear SVf_OOK,
       meaning that this structure might be read again at any point in the
       future without further checks or reinitialisation. */
    if (HvHasAUX(hv)) {
      struct xpvhv_aux *aux = HvAUX(hv);
      struct mro_meta *meta;
      const char *name;

      if (HvHasENAME(hv)) {
        if (PL_phase != PERL_PHASE_DESTRUCT)
            mro_isa_changed_in(hv);
        if (PL_stashcache) {
            DEBUG_o(Perl_deb(aTHX_ "hv_undef_flags clearing PL_stashcache for effective name '%"
                             HEKf "'\n", HEKfARG(HvENAME_HEK_NN(hv))));
            (void)hv_deletehek(PL_stashcache, HvENAME_HEK_NN(hv), G_DISCARD);
        }
      }

      /* If this call originated from sv_clear, then we must check for
       * effective names that need freeing, as well as the usual name. */
      name = HvNAME(hv);
      if (flags & HV_NAME_SETALL
          ? cBOOL(aux->xhv_name_u.xhvnameu_name)
          : cBOOL(name))
      {
        if (name && PL_stashcache) {
            DEBUG_o(Perl_deb(aTHX_ "hv_undef_flags clearing PL_stashcache for name '%"
                             HEKf "'\n", HEKfARG(HvNAME_HEK_NN(hv))));
            (void)hv_deletehek(PL_stashcache, HvNAME_HEK_NN(hv), G_DISCARD);
        }
        hv_name_set(hv, NULL, 0, flags);
      }
      if((meta = aux->xhv_mro_meta)) {
        if (meta->mro_linear_all) {
            SvREFCNT_dec_NN(meta->mro_linear_all);
            /* mro_linear_current is just acting as a shortcut pointer,
               hence the else.  */
        }
        else
            /* Only the current MRO is stored, so this owns the data.
             */
            SvREFCNT_dec(meta->mro_linear_current);
        SvREFCNT_dec(meta->mro_nextmethod);
        SvREFCNT_dec(meta->isa);
        SvREFCNT_dec(meta->super);
        Safefree(meta);
        aux->xhv_mro_meta = NULL;
      }

      if(HvSTASH_IS_CLASS(hv)) {
          SvREFCNT_dec(aux->xhv_class_superclass);
          SvREFCNT_dec(aux->xhv_class_initfields_cv);
          SvREFCNT_dec(aux->xhv_class_adjust_blocks);
          if(aux->xhv_class_fields)
            PadnamelistREFCNT_dec(aux->xhv_class_fields);
          SvREFCNT_dec(aux->xhv_class_param_map);
          Safefree(aux->xhv_class_suspended_initfields_compcv);
          aux->xhv_class_suspended_initfields_compcv = NULL;

          aux->xhv_aux_flags &= ~HvAUXf_IS_CLASS;
      }
    }

    Safefree(HvARRAY(hv));
    HvMAX(hv) = PERL_HASH_DEFAULT_HvMAX;        /* 7 (it's a normal hash) */
    HvARRAY(hv) = 0;

    /* if we're freeing the HV, the SvMAGIC field has been reused for
     * other purposes, and so there can't be any placeholder magic */
    if (SvREFCNT(hv))
        HvPLACEHOLDERS_set(hv, 0);

    if (SvRMAGICAL(hv))
        mg_clear(MUTABLE_SV(hv));

    if (save) {
        /* disarm hv's premature free guard */
        if (LIKELY(PL_tmps_ix == orig_ix))
            PL_tmps_ix--;
        else
            PL_tmps_stack[orig_ix] = &PL_sv_undef;
        SvREFCNT_dec_NN(hv);
    }
}

/*
=for apidoc hv_fill

Returns the number of hash buckets that happen to be in use.

This function implements the L<C<HvFILL> macro|perlapi/HvFILL> which you should
use instead.

As of perl 5.25 this function is used only for debugging
purposes, and the number of used hash buckets is not
in any way cached, thus this function can be costly
to execute as it must iterate over all the buckets in the
hash.

=cut
*/

STRLEN
Perl_hv_fill(pTHX_ HV *const hv)
{
    STRLEN count = 0;
    HE **ents = HvARRAY(hv);

    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_HV_FILL;

    /* No keys implies no buckets used.
       One key can only possibly mean one bucket used.  */
    if (HvTOTALKEYS(hv) < 2)
        return HvTOTALKEYS(hv);

    if (ents) {
        /* I wonder why we count down here...
         * Is it some micro-optimisation?
         * I would have thought counting up was better.
         * - Yves
         */
        HE *const *const last = ents + HvMAX(hv);
        count = last + 1 - ents;

        do {
            if (!*ents)
                --count;
        } while (++ents <= last);
    }
    return count;
}

static struct xpvhv_aux*
S_hv_auxinit(pTHX_ HV *hv) {
    struct xpvhv_aux *iter;

    PERL_ARGS_ASSERT_HV_AUXINIT;

    if (!HvHasAUX(hv)) {
        char *array = (char *) HvARRAY(hv);
        if (!array) {
            Newxz(array, PERL_HV_ARRAY_ALLOC_BYTES(HvMAX(hv) + 1), char);
            HvARRAY(hv) = (HE**)array;
        }
        iter = Perl_hv_auxalloc(aTHX_ hv);
#ifdef PERL_HASH_RANDOMIZE_KEYS
        MAYBE_UPDATE_HASH_RAND_BITS();
        iter->xhv_rand = (U32)PL_hash_rand_bits;
#endif
    } else {
        iter = HvAUX(hv);
    }

    iter->xhv_riter = -1;       /* HvRITER(hv) = -1 */
    iter->xhv_eiter = NULL;     /* HvEITER(hv) = NULL */
#ifdef PERL_HASH_RANDOMIZE_KEYS
    iter->xhv_last_rand = iter->xhv_rand;
#endif
    iter->xhv_name_u.xhvnameu_name = 0;
    iter->xhv_name_count = 0;
    iter->xhv_backreferences = 0;
    iter->xhv_mro_meta = NULL;
    iter->xhv_aux_flags = 0;
    return iter;
}

/*
=for apidoc hv_iterinit

Prepares a starting point to traverse a hash table.  Returns the number of
keys in the hash, including placeholders (i.e. the same as C<HvTOTALKEYS(hv)>).
The return value is currently only meaningful for hashes without tie magic.

NOTE: Before version 5.004_65, C<hv_iterinit> used to return the number of
hash buckets that happen to be in use.  If you still need that esoteric
value, you can get it through the macro C<HvFILL(hv)>.


=cut
*/

I32
Perl_hv_iterinit(pTHX_ HV *hv)
{
    PERL_ARGS_ASSERT_HV_ITERINIT;

    if (HvHasAUX(hv)) {
        struct xpvhv_aux * iter = HvAUX(hv);
        HE * const entry = iter->xhv_eiter; /* HvEITER(hv) */
        if (entry && HvLAZYDEL(hv)) {	/* was deleted earlier? */
            HvLAZYDEL_off(hv);
            hv_free_ent(NULL, entry);
        }
        iter->xhv_riter = -1; 	/* HvRITER(hv) = -1 */
        iter->xhv_eiter = NULL; /* HvEITER(hv) = NULL */
#ifdef PERL_HASH_RANDOMIZE_KEYS
        iter->xhv_last_rand = iter->xhv_rand;
#endif
    } else {
        hv_auxinit(hv);
    }

    /* note this includes placeholders! */
    return HvTOTALKEYS(hv);
}

/*
=for apidoc hv_riter_p

Implements C<HvRITER> which you should use instead.

=cut
*/

I32 *
Perl_hv_riter_p(pTHX_ HV *hv) {
    struct xpvhv_aux *iter;

    PERL_ARGS_ASSERT_HV_RITER_P;

    iter = HvHasAUX(hv) ? HvAUX(hv) : hv_auxinit(hv);
    return &(iter->xhv_riter);
}

/*
=for apidoc hv_eiter_p

Implements C<HvEITER> which you should use instead.

=cut
*/

HE **
Perl_hv_eiter_p(pTHX_ HV *hv) {
    struct xpvhv_aux *iter;

    PERL_ARGS_ASSERT_HV_EITER_P;

    iter = HvHasAUX(hv) ? HvAUX(hv) : hv_auxinit(hv);
    return &(iter->xhv_eiter);
}

/*
=for apidoc hv_riter_set

Implements C<HvRITER_set> which you should use instead.

=cut
*/

void
Perl_hv_riter_set(pTHX_ HV *hv, I32 riter) {
    struct xpvhv_aux *iter;

    PERL_ARGS_ASSERT_HV_RITER_SET;

    if (HvHasAUX(hv)) {
        iter = HvAUX(hv);
    } else {
        if (riter == -1)
            return;

        iter = hv_auxinit(hv);
    }
    iter->xhv_riter = riter;
}

void
Perl_hv_rand_set(pTHX_ HV *hv, U32 new_xhv_rand) {
    struct xpvhv_aux *iter;

    PERL_ARGS_ASSERT_HV_RAND_SET;

#ifdef PERL_HASH_RANDOMIZE_KEYS
    if (HvHasAUX(hv)) {
        iter = HvAUX(hv);
    } else {
        iter = hv_auxinit(hv);
    }
    iter->xhv_rand = new_xhv_rand;
#else
    Perl_croak(aTHX_ "This Perl has not been built with support for randomized hash key traversal but something called Perl_hv_rand_set().");
#endif
}

/*
=for apidoc hv_eiter_set

Implements C<HvEITER_set> which you should use instead.

=cut
*/

void
Perl_hv_eiter_set(pTHX_ HV *hv, HE *eiter) {
    struct xpvhv_aux *iter;

    PERL_ARGS_ASSERT_HV_EITER_SET;

    if (HvHasAUX(hv)) {
        iter = HvAUX(hv);
    } else {
        /* 0 is the default so don't go malloc()ing a new structure just to
           hold 0.  */
        if (!eiter)
            return;

        iter = hv_auxinit(hv);
    }
    iter->xhv_eiter = eiter;
}

/*
=for apidoc        hv_name_set
=for apidoc_item ||hv_name_sets|HV *hv|"name"|U32 flags

These each set the name of stash C<hv> to the specified name.

They differ only in how the name is specified.

In C<hv_name_sets>, the name is a literal C string, enclosed in double quotes.

In C<hv_name_set>, C<name> points to the first byte of the name, and an
additional parameter, C<len>, specifies its length in bytes.  Hence, the name
may contain embedded-NUL characters.

If C<SVf_UTF8> is set in C<flags>, the name is treated as being in UTF-8;
otherwise not.

If C<HV_NAME_SETALL> is set in C<flags>, both the name and the effective name
are set.

=for apidoc Amnh||HV_NAME_SETALL

=cut
*/

void
Perl_hv_name_set(pTHX_ HV *hv, const char *name, U32 len, U32 flags)
{
    struct xpvhv_aux *iter;
    U32 hash;
    HEK **spot;

    PERL_ARGS_ASSERT_HV_NAME_SET;

    if (len > I32_MAX)
        Perl_croak(aTHX_ "panic: hv name too long (%" UVuf ")", (UV) len);

    if (HvHasAUX(hv)) {
        iter = HvAUX(hv);
        if (iter->xhv_name_u.xhvnameu_name) {
            if(iter->xhv_name_count) {
              if(flags & HV_NAME_SETALL) {
                HEK ** const this_name = HvAUX(hv)->xhv_name_u.xhvnameu_names;
                HEK **hekp = this_name + (
                    iter->xhv_name_count < 0
                     ? -iter->xhv_name_count
                     :  iter->xhv_name_count
                   );
                while(hekp-- > this_name+1)
                    unshare_hek_or_pvn(*hekp, 0, 0, 0);
                /* The first elem may be null. */
                if(*this_name) unshare_hek_or_pvn(*this_name, 0, 0, 0);
                Safefree(this_name);
                spot = &iter->xhv_name_u.xhvnameu_name;
                iter->xhv_name_count = 0;
              }
              else {
                if(iter->xhv_name_count > 0) {
                    /* shift some things over */
                    Renew(
                     iter->xhv_name_u.xhvnameu_names, iter->xhv_name_count + 1, HEK *
                    );
                    spot = iter->xhv_name_u.xhvnameu_names;
                    spot[iter->xhv_name_count] = spot[1];
                    spot[1] = spot[0];
                    iter->xhv_name_count = -(iter->xhv_name_count + 1);
                }
                else if(*(spot = iter->xhv_name_u.xhvnameu_names)) {
                    unshare_hek_or_pvn(*spot, 0, 0, 0);
                }
              }
            }
            else if (flags & HV_NAME_SETALL) {
                unshare_hek_or_pvn(iter->xhv_name_u.xhvnameu_name, 0, 0, 0);
                spot = &iter->xhv_name_u.xhvnameu_name;
            }
            else {
                HEK * const existing_name = iter->xhv_name_u.xhvnameu_name;
                Newx(iter->xhv_name_u.xhvnameu_names, 2, HEK *);
                iter->xhv_name_count = -2;
                spot = iter->xhv_name_u.xhvnameu_names;
                spot[1] = existing_name;
            }
        }
        else { spot = &iter->xhv_name_u.xhvnameu_name; iter->xhv_name_count = 0; }
    } else {
        if (name == 0)
            return;

        iter = hv_auxinit(hv);
        spot = &iter->xhv_name_u.xhvnameu_name;
    }
    PERL_HASH(hash, name, len);
    *spot = name ? share_hek(name, flags & SVf_UTF8 ? -(I32)len : (I32)len, hash) : NULL;
}

/*
This is basically sv_eq_flags() in sv.c, but we avoid the magic
and bytes checking.
*/

STATIC I32
hek_eq_pvn_flags(pTHX_ const HEK *hek, const char* pv, const I32 pvlen, const U32 flags) {
    if ( (HEK_UTF8(hek) ? 1 : 0) != (flags & SVf_UTF8 ? 1 : 0) ) {
        if (flags & SVf_UTF8)
            return (bytes_cmp_utf8(
                        (const U8*)HEK_KEY(hek), HEK_LEN(hek),
                        (const U8*)pv, pvlen) == 0);
        else
            return (bytes_cmp_utf8(
                        (const U8*)pv, pvlen,
                        (const U8*)HEK_KEY(hek), HEK_LEN(hek)) == 0);
    }
    else
        return HEK_LEN(hek) == pvlen && ((HEK_KEY(hek) == pv)
                    || memEQ(HEK_KEY(hek), pv, pvlen));
}

/*
=for apidoc hv_ename_add

Adds a name to a stash's internal list of effective names.  See
C<L</hv_ename_delete>>.

This is called when a stash is assigned to a new location in the symbol
table.

=cut
*/

void
Perl_hv_ename_add(pTHX_ HV *hv, const char *name, U32 len, U32 flags)
{
    struct xpvhv_aux *aux = HvHasAUX(hv) ? HvAUX(hv) : hv_auxinit(hv);
    U32 hash;

    PERL_ARGS_ASSERT_HV_ENAME_ADD;

    if (len > I32_MAX)
        Perl_croak(aTHX_ "panic: hv name too long (%" UVuf ")", (UV) len);

    PERL_HASH(hash, name, len);

    if (aux->xhv_name_count) {
        I32 count = aux->xhv_name_count;
        HEK ** const xhv_name = aux->xhv_name_u.xhvnameu_names + (count<0);
        HEK **hekp = xhv_name + (count < 0 ? -count - 1 : count);
        while (hekp-- > xhv_name)
        {
            assert(*hekp);
            if (
                 (HEK_UTF8(*hekp) || (flags & SVf_UTF8)) 
                    ? hek_eq_pvn_flags(aTHX_ *hekp, name, (I32)len, flags)
                    : (HEK_LEN(*hekp) == (I32)len && memEQ(HEK_KEY(*hekp), name, len))
               ) {
                if (hekp == xhv_name && count < 0)
                    aux->xhv_name_count = -count;
                return;
            }
        }
        if (count < 0) aux->xhv_name_count--, count = -count;
        else aux->xhv_name_count++;
        Renew(aux->xhv_name_u.xhvnameu_names, count + 1, HEK *);
        (aux->xhv_name_u.xhvnameu_names)[count] = share_hek(name, (flags & SVf_UTF8 ? -(I32)len : (I32)len), hash);
    }
    else {
        HEK *existing_name = aux->xhv_name_u.xhvnameu_name;
        if (
            existing_name && (
             (HEK_UTF8(existing_name) || (flags & SVf_UTF8))
                ? hek_eq_pvn_flags(aTHX_ existing_name, name, (I32)len, flags)
                : (HEK_LEN(existing_name) == (I32)len && memEQ(HEK_KEY(existing_name), name, len))
            )
        ) return;
        Newx(aux->xhv_name_u.xhvnameu_names, 2, HEK *);
        aux->xhv_name_count = existing_name ? 2 : -2;
        *aux->xhv_name_u.xhvnameu_names = existing_name;
        (aux->xhv_name_u.xhvnameu_names)[1] = share_hek(name, (flags & SVf_UTF8 ? -(I32)len : (I32)len), hash);
    }
}

/*
=for apidoc hv_ename_delete

Removes a name from a stash's internal list of effective names.  If this is
the name returned by C<HvENAME>, then another name in the list will take
its place (C<HvENAME> will use it).

This is called when a stash is deleted from the symbol table.

=cut
*/

void
Perl_hv_ename_delete(pTHX_ HV *hv, const char *name, U32 len, U32 flags)
{
    struct xpvhv_aux *aux;

    PERL_ARGS_ASSERT_HV_ENAME_DELETE;

    if (len > I32_MAX)
        Perl_croak(aTHX_ "panic: hv name too long (%" UVuf ")", (UV) len);

    if (!HvHasAUX(hv)) return;

    aux = HvAUX(hv);
    if (!aux->xhv_name_u.xhvnameu_name) return;

    if (aux->xhv_name_count) {
        HEK ** const namep = aux->xhv_name_u.xhvnameu_names;
        I32 const count = aux->xhv_name_count;
        HEK **victim = namep + (count < 0 ? -count : count);
        while (victim-- > namep + 1)
            if (
             (HEK_UTF8(*victim) || (flags & SVf_UTF8)) 
                ? hek_eq_pvn_flags(aTHX_ *victim, name, (I32)len, flags)
                : (HEK_LEN(*victim) == (I32)len && memEQ(HEK_KEY(*victim), name, len))
            ) {
                unshare_hek_or_pvn(*victim, 0, 0, 0);
                if (count < 0) ++aux->xhv_name_count;
                else --aux->xhv_name_count;
                if (
                    (aux->xhv_name_count == 1 || aux->xhv_name_count == -1)
                 && !*namep
                ) {  /* if there are none left */
                    Safefree(namep);
                    aux->xhv_name_u.xhvnameu_names = NULL;
                    aux->xhv_name_count = 0;
                }
                else {
                    /* Move the last one back to fill the empty slot. It
                       does not matter what order they are in. */
                    *victim = *(namep + (count < 0 ? -count : count) - 1);
                }
                return;
            }
        if (
            count > 0 && ((HEK_UTF8(*namep) || (flags & SVf_UTF8)) 
                ? hek_eq_pvn_flags(aTHX_ *namep, name, (I32)len, flags)
                : (HEK_LEN(*namep) == (I32)len && memEQ(HEK_KEY(*namep), name, len))
            )
        ) {
            aux->xhv_name_count = -count;
        }
    }
    else if(
        (HEK_UTF8(aux->xhv_name_u.xhvnameu_name) || (flags & SVf_UTF8)) 
                ? hek_eq_pvn_flags(aTHX_ aux->xhv_name_u.xhvnameu_name, name, (I32)len, flags)
                : (HEK_LEN(aux->xhv_name_u.xhvnameu_name) == (I32)len &&
                            memEQ(HEK_KEY(aux->xhv_name_u.xhvnameu_name), name, len))
    ) {
        HEK * const namehek = aux->xhv_name_u.xhvnameu_name;
        Newx(aux->xhv_name_u.xhvnameu_names, 1, HEK *);
        *aux->xhv_name_u.xhvnameu_names = namehek;
        aux->xhv_name_count = -1;
    }
}

AV **
Perl_hv_backreferences_p(pTHX_ HV *hv) {
    PERL_ARGS_ASSERT_HV_BACKREFERENCES_P;
    /* See also Perl_sv_get_backrefs in sv.c where this logic is unrolled */
    {
        struct xpvhv_aux * const iter = HvHasAUX(hv) ? HvAUX(hv) : hv_auxinit(hv);
        return &(iter->xhv_backreferences);
    }
}

void
Perl_hv_kill_backrefs(pTHX_ HV *hv) {
    AV *av;

    PERL_ARGS_ASSERT_HV_KILL_BACKREFS;

    if (!HvHasAUX(hv))
        return;

    av = HvAUX(hv)->xhv_backreferences;

    if (av) {
        HvAUX(hv)->xhv_backreferences = 0;
        Perl_sv_kill_backrefs(aTHX_ MUTABLE_SV(hv), av);
        if (SvTYPE(av) == SVt_PVAV)
            SvREFCNT_dec_NN(av);
    }
}

/*
hv_iternext is implemented as a macro in hv.h

=for apidoc hv_iternext

Returns entries from a hash iterator.  See C<L</hv_iterinit>>.

You may call C<hv_delete> or C<hv_delete_ent> on the hash entry that the
iterator currently points to, without losing your place or invalidating your
iterator.  Note that in this case the current entry is deleted from the hash
with your iterator holding the last reference to it.  Your iterator is flagged
to free the entry on the next call to C<hv_iternext>, so you must not discard
your iterator immediately else the entry will leak - call C<hv_iternext> to
trigger the resource deallocation.

=for apidoc hv_iternext_flags

Returns entries from a hash iterator.  See C<L</hv_iterinit>> and
C<L</hv_iternext>>.
The C<flags> value will normally be zero; if C<HV_ITERNEXT_WANTPLACEHOLDERS> is
set the placeholders keys (for restricted hashes) will be returned in addition
to normal keys.  By default placeholders are automatically skipped over.
Currently a placeholder is implemented with a value that is
C<&PL_sv_placeholder>.  Note that the implementation of placeholders and
restricted hashes may change, and the implementation currently is
insufficiently abstracted for any change to be tidy.

=for apidoc Amnh||HV_ITERNEXT_WANTPLACEHOLDERS

=cut
*/

HE *
Perl_hv_iternext_flags(pTHX_ HV *hv, I32 flags)
{
    HE *entry;
    HE *oldentry;
    MAGIC* mg;
    struct xpvhv_aux *iter;

    PERL_ARGS_ASSERT_HV_ITERNEXT_FLAGS;

    if (!HvHasAUX(hv)) {
        /* Too many things (well, pp_each at least) merrily assume that you can
           call hv_iternext without calling hv_iterinit, so we'll have to deal
           with it.  */
        hv_iterinit(hv);
    }
    else if (!HvARRAY(hv)) {
        /* Since 5.002 calling hv_iternext() has ensured that HvARRAY() is
           non-NULL. There was explicit code for this added as part of commit
           4633a7c4bad06b47, without any explicit comment as to why, but from
           code inspection it seems to be a fix to ensure that the later line
               entry = ((HE**)xhv->xhv_array)[xhv->xhv_riter];
           was accessing a valid address, because that lookup in the loop was
           always reached even if the hash had no keys.

           That explicit code was removed in 2005 as part of b79f7545f218479c:
               Store the xhv_aux structure after the main array.
               This reduces the size of HV bodies from 24 to 20 bytes on a 32 bit
               build. It has the side effect of defined %symbol_table:: now always
               being true. defined %hash is already deprecated.

           with a comment and assertion added to note that after the call to
           hv_iterinit() HvARRAY() will now always be non-NULL.

           In turn, that potential NULL-pointer access within the loop was made
           unreachable in 2009 by commit 9eb4ebd1619c0362
               In Perl_hv_iternext_flags(), clarify and generalise the empty hash bailout code.

           which skipped the entire while loop if the hash had no keys.
           (If the hash has any keys, HvARRAY() cannot be NULL.)
           Hence the code in hv_iternext_flags() has long been able to handle
           HvARRAY() being NULL because no keys are allocated.

           Now that we have decoupled the aux structure from HvARRAY(),
           HvARRAY() can now be NULL even when SVf_OOK is true (and the aux
           struct is allocated and correction initialised).

           Is this actually a guarantee that we need to make? We should check
           whether anything is actually relying on this, or if we are simply
           making work for ourselves.

           For now, keep the behaviour as-was - after calling hv_iternext_flags
           ensure that HvARRAY() is non-NULL. Many (other) things are changing -
           no need to add risk by changing this too. But in the future we should
           consider changing hv_iternext_flags() to avoid allocating HvARRAY()
           here, and potentially also we avoid allocating HvARRAY()
           automatically in hv_auxinit() */

        char *array;
        Newxz(array, PERL_HV_ARRAY_ALLOC_BYTES(HvMAX(hv) + 1), char);
        HvARRAY(hv) = (HE**)array;
    }

    iter = HvAUX(hv);

    oldentry = entry = iter->xhv_eiter; /* HvEITER(hv) */
    if (SvMAGICAL(hv) && SvRMAGICAL(hv)) {
        if ( ( mg = mg_find((const SV *)hv, PERL_MAGIC_tied) ) ) {
            SV * const key = sv_newmortal();
            if (entry) {
                sv_setsv(key, HeSVKEY_force(entry));
                SvREFCNT_dec(HeSVKEY(entry));       /* get rid of previous key */
                HeSVKEY_set(entry, NULL);
            }
            else {
                char *k;
                HEK *hek;

                /* one HE per MAGICAL hash */
                iter->xhv_eiter = entry = new_HE(); /* HvEITER(hv) = new_HE() */
                HvLAZYDEL_on(hv); /* make sure entry gets freed */
                Zero(entry, 1, HE);
                Newxz(k, HEK_BASESIZE + sizeof(const SV *), char);
                hek = (HEK*)k;
                HeKEY_hek(entry) = hek;
                HeKLEN(entry) = HEf_SVKEY;
            }
            magic_nextpack(MUTABLE_SV(hv),mg,key);
            if (SvOK(key)) {
                /* force key to stay around until next time */
                HeSVKEY_set(entry, SvREFCNT_inc_simple_NN(key));
                return entry;               /* beware, hent_val is not set */
            }
            SvREFCNT_dec(HeVAL(entry));
            Safefree(HeKEY_hek(entry));
            del_HE(entry);
            iter->xhv_eiter = NULL; /* HvEITER(hv) = NULL */
            HvLAZYDEL_off(hv);
            return NULL;
        }
    }
#if defined(DYNAMIC_ENV_FETCH) && defined(VMS)  /* set up %ENV for iteration */
    if (!entry && SvRMAGICAL((const SV *)hv)
        && mg_find((const SV *)hv, PERL_MAGIC_env)) {
        prime_env_iter();
    }
#endif

    /* hv_iterinit now ensures this.  */
    assert (HvARRAY(hv));

    /* At start of hash, entry is NULL.  */
    if (entry)
    {
        entry = HeNEXT(entry);
        if (!(flags & HV_ITERNEXT_WANTPLACEHOLDERS)) {
            /*
             * Skip past any placeholders -- don't want to include them in
             * any iteration.
             */
            while (entry && HeVAL(entry) == &PL_sv_placeholder) {
                entry = HeNEXT(entry);
            }
        }
    }

#ifdef PERL_HASH_RANDOMIZE_KEYS
    if (iter->xhv_last_rand != iter->xhv_rand) {
        if (iter->xhv_riter != -1) {
            Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                             "Use of each() on hash after insertion without resetting hash iterator results in undefined behavior"
                             pTHX__FORMAT
                             pTHX__VALUE);
        }
        iter->xhv_last_rand = iter->xhv_rand;
    }
#endif

    /* Skip the entire loop if the hash is empty.   */
    if ((flags & HV_ITERNEXT_WANTPLACEHOLDERS)
        ? HvTOTALKEYS(hv) : HvUSEDKEYS(hv)) {
        STRLEN max = HvMAX(hv);
        while (!entry) {
            /* OK. Come to the end of the current list.  Grab the next one.  */

            iter->xhv_riter++; /* HvRITER(hv)++ */
            if (iter->xhv_riter > (I32)max /* HvRITER(hv) > HvMAX(hv) */) {
                /* There is no next one.  End of the hash.  */
                iter->xhv_riter = -1; /* HvRITER(hv) = -1 */
#ifdef PERL_HASH_RANDOMIZE_KEYS
                iter->xhv_last_rand = iter->xhv_rand; /* reset xhv_last_rand so we can detect inserts during traversal */
#endif
                break;
            }
            entry = (HvARRAY(hv))[ PERL_HASH_ITER_BUCKET(iter) & max ];

            if (!(flags & HV_ITERNEXT_WANTPLACEHOLDERS)) {
                /* If we have an entry, but it's a placeholder, don't count it.
                   Try the next.  */
                while (entry && HeVAL(entry) == &PL_sv_placeholder)
                    entry = HeNEXT(entry);
            }
            /* Will loop again if this linked list starts NULL
               (for HV_ITERNEXT_WANTPLACEHOLDERS)
               or if we run through it and find only placeholders.  */
        }
    }
    else {
        iter->xhv_riter = -1;
#ifdef PERL_HASH_RANDOMIZE_KEYS
        iter->xhv_last_rand = iter->xhv_rand;
#endif
    }

    if (oldentry && HvLAZYDEL(hv)) {		/* was deleted earlier? */
        HvLAZYDEL_off(hv);
        hv_free_ent(NULL, oldentry);
    }

    iter->xhv_eiter = entry; /* HvEITER(hv) = entry */
    return entry;
}

/*
=for apidoc hv_iterkey

Returns the key from the current position of the hash iterator.  See
C<L</hv_iterinit>>.

=cut
*/

char *
Perl_hv_iterkey(pTHX_ HE *entry, I32 *retlen)
{
    PERL_ARGS_ASSERT_HV_ITERKEY;

    if (HeKLEN(entry) == HEf_SVKEY) {
        STRLEN len;
        char * const p = SvPV(HeKEY_sv(entry), len);
        *retlen = len;
        return p;
    }
    else {
        *retlen = HeKLEN(entry);
        return HeKEY(entry);
    }
}

/* unlike hv_iterval(), this always returns a mortal copy of the key */
/*
=for apidoc hv_iterkeysv

Returns the key as an C<SV*> from the current position of the hash
iterator.  The return value will always be a mortal copy of the key.  Also
see C<L</hv_iterinit>>.

=cut
*/

SV *
Perl_hv_iterkeysv(pTHX_ HE *entry)
{
    PERL_ARGS_ASSERT_HV_ITERKEYSV;

    return newSVhek_mortal(HeKEY_hek(entry));
}

/*
=for apidoc hv_iterval

Returns the value from the current position of the hash iterator.  See
C<L</hv_iterkey>>.

=cut
*/

SV *
Perl_hv_iterval(pTHX_ HV *hv, HE *entry)
{
    PERL_ARGS_ASSERT_HV_ITERVAL;

    if (SvRMAGICAL(hv)) {
        if (mg_find((const SV *)hv, PERL_MAGIC_tied)) {
            SV* const sv = sv_newmortal();
            if (HeKLEN(entry) == HEf_SVKEY)
                mg_copy(MUTABLE_SV(hv), sv, (char*)HeKEY_sv(entry), HEf_SVKEY);
            else
                mg_copy(MUTABLE_SV(hv), sv, HeKEY(entry), HeKLEN(entry));
            return sv;
        }
    }
    return HeVAL(entry);
}

/*
=for apidoc hv_iternextsv

Performs an C<hv_iternext>, C<hv_iterkey>, and C<hv_iterval> in one
operation.

=cut
*/

SV *
Perl_hv_iternextsv(pTHX_ HV *hv, char **key, I32 *retlen)
{
    HE * const he = hv_iternext_flags(hv, 0);

    PERL_ARGS_ASSERT_HV_ITERNEXTSV;

    if (!he)
        return NULL;
    *key = hv_iterkey(he, retlen);
    return hv_iterval(hv, he);
}

/*

Now a macro in hv.h

=for apidoc hv_magic

Adds magic to a hash.  See C<L</sv_magic>>.

=for apidoc unsharepvn

If no one has access to shared string C<str> with length C<len>, free it.

C<len> and C<hash> must both be valid for C<str>.

=cut
*/

void
Perl_unsharepvn(pTHX_ const char *str, I32 len, U32 hash)
{
    unshare_hek_or_pvn (NULL, str, len, hash);
}


void
Perl_unshare_hek(pTHX_ HEK *hek)
{
    assert(hek);
    unshare_hek_or_pvn(hek, NULL, 0, 0);
}

/* possibly free a shared string if no one has access to it
   hek if non-NULL takes priority over the other 3, else str, len and hash
   are used.  If so, len and hash must both be valid for str.
 */
STATIC void
S_unshare_hek_or_pvn(pTHX_ const HEK *hek, const char *str, I32 len, U32 hash)
{
    HE *entry;
    HE **oentry;
    bool is_utf8 = FALSE;
    int k_flags = 0;
    const char * const save = str;
    struct shared_he *he = NULL;

    if (hek) {
        assert((HEK_FLAGS(hek) & HVhek_NOTSHARED) == 0);
        /* Find the shared he which is just before us in memory.  */
        he = (struct shared_he *)(((char *)hek)
                                  - STRUCT_OFFSET(struct shared_he,
                                                  shared_he_hek));

        /* Assert that the caller passed us a genuine (or at least consistent)
           shared hek  */
        assert (he->shared_he_he.hent_hek == hek);

        if (he->shared_he_he.he_valu.hent_refcount - 1) {
            --he->shared_he_he.he_valu.hent_refcount;
            return;
        }

        hash = HEK_HASH(hek);
    } else if (len < 0) {
        STRLEN tmplen = -len;
        is_utf8 = TRUE;
        /* See the note in hv_fetch(). --jhi */
        str = (char*)bytes_from_utf8((U8*)str, &tmplen, &is_utf8);
        len = tmplen;
        if (is_utf8)
            k_flags = HVhek_UTF8;
        if (str != save)
            k_flags |= HVhek_WASUTF8 | HVhek_FREEKEY;
    }

    /* what follows was the moral equivalent of:
    if ((Svp = hv_fetch(PL_strtab, tmpsv, FALSE, hash))) {
        if (--*Svp == NULL)
            hv_delete(PL_strtab, str, len, G_DISCARD, hash);
    } */

    /* assert(xhv_array != 0) */
    oentry = &(HvARRAY(PL_strtab))[hash & (I32) HvMAX(PL_strtab)];
    if (he) {
        const HE *const he_he = &(he->shared_he_he);
        for (entry = *oentry; entry; oentry = &HeNEXT(entry), entry = *oentry) {
            if (entry == he_he)
                break;
        }
    } else {
        const U8 flags_masked = k_flags & HVhek_STORAGE_MASK;
        for (entry = *oentry; entry; oentry = &HeNEXT(entry), entry = *oentry) {
            if (HeHASH(entry) != hash)		/* strings can't be equal */
                continue;
            if (HeKLEN(entry) != len)
                continue;
            if (HeKEY(entry) != str && memNE(HeKEY(entry),str,len))	/* is this it? */
                continue;
            if (HeKFLAGS(entry) != flags_masked)
                continue;
            break;
        }
    }

    if (entry) {
        if (--entry->he_valu.hent_refcount == 0) {
            *oentry = HeNEXT(entry);
            Safefree(entry);
            HvTOTALKEYS(PL_strtab)--;
        }
    }

    if (!entry)
        Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                         "Attempt to free nonexistent shared string '%s'%s"
                         pTHX__FORMAT,
                         hek ? HEK_KEY(hek) : str,
                         ((k_flags & HVhek_UTF8) ? " (utf8)" : "") pTHX__VALUE);
    if (k_flags & HVhek_FREEKEY)
        Safefree(str);
}

/* get a (constant) string ptr from the global string table
 * string will get added if it is not already there.
 * len and hash must both be valid for str.
 */
HEK *
Perl_share_hek(pTHX_ const char *str, SSize_t len, U32 hash)
{
    bool is_utf8 = FALSE;
    int flags = 0;
    const char * const save = str;

    PERL_ARGS_ASSERT_SHARE_HEK;

    if (len < 0) {
      STRLEN tmplen = -len;
      is_utf8 = TRUE;
      /* See the note in hv_fetch(). --jhi */
      str = (char*)bytes_from_utf8((U8*)str, &tmplen, &is_utf8);
      len = tmplen;
      /* If we were able to downgrade here, then than means that we were passed
         in a key which only had chars 0-255, but was utf8 encoded.  */
      if (is_utf8)
          flags = HVhek_UTF8;
      /* If we found we were able to downgrade the string to bytes, then
         we should flag that it needs upgrading on keys or each.  Also flag
         that we need share_hek_flags to free the string.  */
      if (str != save) {
          PERL_HASH(hash, str, len);
          flags |= HVhek_WASUTF8 | HVhek_FREEKEY;
      }
    }

    return share_hek_flags (str, len, hash, flags);
}

STATIC HEK *
S_share_hek_flags(pTHX_ const char *str, STRLEN len, U32 hash, int flags)
{
    HE *entry;
    const U8 flags_masked = flags & HVhek_STORAGE_MASK;
    const U32 hindex = hash & (I32) HvMAX(PL_strtab);

    PERL_ARGS_ASSERT_SHARE_HEK_FLAGS;
    assert(!(flags & HVhek_NOTSHARED));

    if (UNLIKELY(len > (STRLEN) I32_MAX)) {
        Perl_croak_nocontext("Sorry, hash keys must be smaller than 2**31 bytes");
    }

    /* what follows is the moral equivalent of:

    if (!(Svp = hv_fetch(PL_strtab, str, len, FALSE)))
        hv_store(PL_strtab, str, len, NULL, hash);

        Can't rehash the shared string table, so not sure if it's worth
        counting the number of entries in the linked list
    */

    /* assert(xhv_array != 0) */
    entry = (HvARRAY(PL_strtab))[hindex];
    for (;entry; entry = HeNEXT(entry)) {
        if (HeHASH(entry) != hash)		/* strings can't be equal */
            continue;
        if (HeKLEN(entry) != (SSize_t) len)
            continue;
        if (HeKEY(entry) != str && memNE(HeKEY(entry),str,len))	/* is this it? */
            continue;
        if (HeKFLAGS(entry) != flags_masked)
            continue;
        break;
    }

    if (!entry) {
        /* What used to be head of the list.
           If this is NULL, then we're the first entry for this slot, which
           means we need to increase fill.  */
        struct shared_he *new_entry;
        HEK *hek;
        char *k;
        HE **const head = &HvARRAY(PL_strtab)[hindex];
        HE *const next = *head;
        XPVHV * const xhv = (XPVHV*)SvANY(PL_strtab);

        /* We don't actually store a HE from the arena and a regular HEK.
           Instead we allocate one chunk of memory big enough for both,
           and put the HEK straight after the HE. This way we can find the
           HE directly from the HEK.
        */

        Newx(k, STRUCT_OFFSET(struct shared_he,
                                shared_he_hek.hek_key[0]) + len + 2, char);
        new_entry = (struct shared_he *)k;
        entry = &(new_entry->shared_he_he);
        hek = &(new_entry->shared_he_hek);

        Copy(str, HEK_KEY(hek), len, char);
        HEK_KEY(hek)[len] = 0;
        HEK_LEN(hek) = len;
        HEK_HASH(hek) = hash;
        HEK_FLAGS(hek) = (unsigned char)flags_masked;

        /* Still "point" to the HEK, so that other code need not know what
           we're up to.  */
        HeKEY_hek(entry) = hek;
        entry->he_valu.hent_refcount = 0;
        HeNEXT(entry) = next;
        *head = entry;

        xhv->xhv_keys++; /* HvTOTALKEYS(hv)++ */
        if (!next) {			/* initial entry? */
        } else if ( DO_HSPLIT(xhv) ) {
            const STRLEN oldsize = xhv->xhv_max + 1;
            hsplit(PL_strtab, oldsize, oldsize * 2);
        }
    }

    ++entry->he_valu.hent_refcount;

    if (flags & HVhek_FREEKEY)
        Safefree(str);

    return HeKEY_hek(entry);
}

SSize_t *
Perl_hv_placeholders_p(pTHX_ HV *hv)
{
    MAGIC *mg = mg_find((const SV *)hv, PERL_MAGIC_rhash);

    PERL_ARGS_ASSERT_HV_PLACEHOLDERS_P;

    if (!mg) {
        mg = sv_magicext(MUTABLE_SV(hv), 0, PERL_MAGIC_rhash, 0, 0, 0);

        if (!mg) {
            Perl_die(aTHX_ "panic: hv_placeholders_p");
        }
    }
    return &(mg->mg_len);
}

/*
=for apidoc hv_placeholders_get

Implements C<HvPLACEHOLDERS_get>, which you should use instead.

=cut
*/

I32
Perl_hv_placeholders_get(pTHX_ const HV *hv)
{
    MAGIC * const mg = mg_find((const SV *)hv, PERL_MAGIC_rhash);

    PERL_ARGS_ASSERT_HV_PLACEHOLDERS_GET;
    PERL_UNUSED_CONTEXT;

    return mg ? mg->mg_len : 0;
}

/*
=for apidoc hv_placeholders_set

Implements C<HvPLACEHOLDERS_set>, which you should use instead.

=cut
*/

void
Perl_hv_placeholders_set(pTHX_ HV *hv, I32 ph)
{
    MAGIC * const mg = mg_find((const SV *)hv, PERL_MAGIC_rhash);

    PERL_ARGS_ASSERT_HV_PLACEHOLDERS_SET;

    if (mg) {
        mg->mg_len = ph;
    } else if (ph) {
        if (!sv_magicext(MUTABLE_SV(hv), 0, PERL_MAGIC_rhash, 0, 0, ph))
            Perl_die(aTHX_ "panic: hv_placeholders_set");
    }
    /* else we don't need to add magic to record 0 placeholders.  */
}

STATIC SV *
S_refcounted_he_value(pTHX_ const struct refcounted_he *he)
{
    SV *value;

    PERL_ARGS_ASSERT_REFCOUNTED_HE_VALUE;

    switch(he->refcounted_he_data[0] & HVrhek_typemask) {
    case HVrhek_undef:
        value = newSV_type(SVt_NULL);
        break;
    case HVrhek_delete:
        value = &PL_sv_placeholder;
        break;
    case HVrhek_IV:
        value = newSViv(he->refcounted_he_val.refcounted_he_u_iv);
        break;
    case HVrhek_UV:
        value = newSVuv(he->refcounted_he_val.refcounted_he_u_uv);
        break;
    case HVrhek_PV:
    case HVrhek_PV_UTF8:
        /* Create a string SV that directly points to the bytes in our
           structure.  */
        value = newSV_type(SVt_PV);
        SvPV_set(value, (char *) he->refcounted_he_data + 1);
        SvCUR_set(value, he->refcounted_he_val.refcounted_he_u_len);
        /* This stops anything trying to free it  */
        SvLEN_set(value, 0);
        SvPOK_on(value);
        SvREADONLY_on(value);
        if ((he->refcounted_he_data[0] & HVrhek_typemask) == HVrhek_PV_UTF8)
            SvUTF8_on(value);
        break;
    default:
        Perl_croak(aTHX_ "panic: refcounted_he_value bad flags %" UVxf,
                   (UV)he->refcounted_he_data[0]);
    }
    return value;
}

/*
=for apidoc refcounted_he_chain_2hv

Generates and returns a C<HV *> representing the content of a
C<refcounted_he> chain.
C<flags> is currently unused and must be zero.

=cut
*/
HV *
Perl_refcounted_he_chain_2hv(pTHX_ const struct refcounted_he *chain, U32 flags)
{
    HV *hv;
    U32 placeholders, max;

    if (flags)
        Perl_croak(aTHX_ "panic: refcounted_he_chain_2hv bad flags %" UVxf,
            (UV)flags);

    /* We could chase the chain once to get an idea of the number of keys,
       and call ksplit.  But for now we'll make a potentially inefficient
       hash with only 8 entries in its array.  */
    hv = newHV();
#ifdef NODEFAULT_SHAREKEYS
    /* We share keys in the COP, so it's much easier to keep sharing keys in
       the hash we build from it. */
    HvSHAREKEYS_on(hv);
#endif
    max = HvMAX(hv);
    if (!HvARRAY(hv)) {
        char *array;
        Newxz(array, PERL_HV_ARRAY_ALLOC_BYTES(max + 1), char);
        HvARRAY(hv) = (HE**)array;
    }

    placeholders = 0;
    while (chain) {
#ifdef USE_ITHREADS
        U32 hash = chain->refcounted_he_hash;
#else
        U32 hash = HEK_HASH(chain->refcounted_he_hek);
#endif
        HE **oentry = &((HvARRAY(hv))[hash & max]);
        HE *entry = *oentry;
        SV *value;

        for (; entry; entry = HeNEXT(entry)) {
            if (HeHASH(entry) == hash) {
                /* We might have a duplicate key here.  If so, entry is older
                   than the key we've already put in the hash, so if they are
                   the same, skip adding entry.  */
#ifdef USE_ITHREADS
                const STRLEN klen = HeKLEN(entry);
                const char *const key = HeKEY(entry);
                if (klen == chain->refcounted_he_keylen
                    && (cBOOL(HeKUTF8(entry))
                        == cBOOL((chain->refcounted_he_data[0] & HVhek_UTF8)))
                    && memEQ(key, REF_HE_KEY(chain), klen))
                    goto next_please;
#else
                if (HeKEY_hek(entry) == chain->refcounted_he_hek)
                    goto next_please;
                if (HeKLEN(entry) == HEK_LEN(chain->refcounted_he_hek)
                    && HeKUTF8(entry) == HEK_UTF8(chain->refcounted_he_hek)
                    && memEQ(HeKEY(entry), HEK_KEY(chain->refcounted_he_hek),
                             HeKLEN(entry)))
                    goto next_please;
#endif
            }
        }
        assert (!entry);
        entry = new_HE();

#ifdef USE_ITHREADS
        HeKEY_hek(entry)
            = share_hek_flags(REF_HE_KEY(chain),
                              chain->refcounted_he_keylen,
                              chain->refcounted_he_hash,
                              (chain->refcounted_he_data[0]
                               & (HVhek_UTF8|HVhek_WASUTF8)));
#else
        HeKEY_hek(entry) = share_hek_hek(chain->refcounted_he_hek);
#endif
        value = refcounted_he_value(chain);
        if (value == &PL_sv_placeholder)
            placeholders++;
        HeVAL(entry) = value;

        /* Link it into the chain.  */
        HeNEXT(entry) = *oentry;
        *oentry = entry;

        HvTOTALKEYS(hv)++;

    next_please:
        chain = chain->refcounted_he_next;
    }

    if (placeholders) {
        clear_placeholders(hv, placeholders);
    }

    /* We could check in the loop to see if we encounter any keys with key
       flags, but it's probably not worth it, as this per-hash flag is only
       really meant as an optimisation for things like Storable.  */
    HvHASKFLAGS_on(hv);
    DEBUG_A(Perl_hv_assert(aTHX_ hv));

    return hv;
}

/*
=for apidoc refcounted_he_fetch_pvn

Search along a C<refcounted_he> chain for an entry with the key specified
by C<keypv> and C<keylen>.  If C<flags> has the C<REFCOUNTED_HE_KEY_UTF8>
bit set, the key octets are interpreted as UTF-8, otherwise they
are interpreted as Latin-1.  C<hash> is a precomputed hash of the key
string, or zero if it has not been precomputed.  Returns a mortal scalar
representing the value associated with the key, or C<&PL_sv_placeholder>
if there is no value associated with the key.

=cut
*/

SV *
Perl_refcounted_he_fetch_pvn(pTHX_ const struct refcounted_he *chain,
                         const char *keypv, STRLEN keylen, U32 hash, U32 flags)
{
    U8 utf8_flag;
    PERL_ARGS_ASSERT_REFCOUNTED_HE_FETCH_PVN;

    if (flags & ~(REFCOUNTED_HE_KEY_UTF8|REFCOUNTED_HE_EXISTS))
        Perl_croak(aTHX_ "panic: refcounted_he_fetch_pvn bad flags %" UVxf,
            (UV)flags);
    if (!chain)
        goto ret;
    if (flags & REFCOUNTED_HE_KEY_UTF8) {
        /* For searching purposes, canonicalise to Latin-1 where possible. */
        const char *keyend = keypv + keylen, *p;
        STRLEN nonascii_count = 0;
        for (p = keypv; p != keyend; p++) {
            if (! UTF8_IS_INVARIANT(*p)) {
                if (! UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(p, keyend)) {
                    goto canonicalised_key;
                }
                nonascii_count++;
                p++;
            }
        }
        if (nonascii_count) {
            char *q;
            const char *p = keypv, *keyend = keypv + keylen;
            keylen -= nonascii_count;
            Newx(q, keylen, char);
            SAVEFREEPV(q);
            keypv = q;
            for (; p != keyend; p++, q++) {
                U8 c = (U8)*p;
                if (UTF8_IS_INVARIANT(c)) {
                    *q = (char) c;
                }
                else {
                    p++;
                    *q = (char) EIGHT_BIT_UTF8_TO_NATIVE(c, *p);
                }
            }
        }
        flags &= ~REFCOUNTED_HE_KEY_UTF8;
        canonicalised_key: ;
    }
    utf8_flag = (flags & REFCOUNTED_HE_KEY_UTF8) ? HVhek_UTF8 : 0;
    if (!hash)
        PERL_HASH(hash, keypv, keylen);

    for (; chain; chain = chain->refcounted_he_next) {
        if (
#ifdef USE_ITHREADS
            hash == chain->refcounted_he_hash &&
            keylen == chain->refcounted_he_keylen &&
            memEQ(REF_HE_KEY(chain), keypv, keylen) &&
            utf8_flag == (chain->refcounted_he_data[0] & HVhek_UTF8)
#else
            hash == HEK_HASH(chain->refcounted_he_hek) &&
            keylen == (STRLEN)HEK_LEN(chain->refcounted_he_hek) &&
            memEQ(HEK_KEY(chain->refcounted_he_hek), keypv, keylen) &&
            utf8_flag == (HEK_FLAGS(chain->refcounted_he_hek) & HVhek_UTF8)
#endif
        ) {
            if (flags & REFCOUNTED_HE_EXISTS)
                return (chain->refcounted_he_data[0] & HVrhek_typemask)
                    == HVrhek_delete
                    ? NULL : &PL_sv_yes;
            return sv_2mortal(refcounted_he_value(chain));
        }
    }
  ret:
    return flags & REFCOUNTED_HE_EXISTS ? NULL : &PL_sv_placeholder;
}

/*
=for apidoc refcounted_he_fetch_pv

Like L</refcounted_he_fetch_pvn>, but takes a nul-terminated string
instead of a string/length pair.

=cut
*/

SV *
Perl_refcounted_he_fetch_pv(pTHX_ const struct refcounted_he *chain,
                         const char *key, U32 hash, U32 flags)
{
    PERL_ARGS_ASSERT_REFCOUNTED_HE_FETCH_PV;
    return refcounted_he_fetch_pvn(chain, key, strlen(key), hash, flags);
}

/*
=for apidoc refcounted_he_fetch_sv

Like L</refcounted_he_fetch_pvn>, but takes a Perl scalar instead of a
string/length pair.

=cut
*/

SV *
Perl_refcounted_he_fetch_sv(pTHX_ const struct refcounted_he *chain,
                         SV *key, U32 hash, U32 flags)
{
    const char *keypv;
    STRLEN keylen;
    PERL_ARGS_ASSERT_REFCOUNTED_HE_FETCH_SV;
    if (flags & REFCOUNTED_HE_KEY_UTF8)
        Perl_croak(aTHX_ "panic: refcounted_he_fetch_sv bad flags %" UVxf,
            (UV)flags);
    keypv = SvPV_const(key, keylen);
    if (SvUTF8(key))
        flags |= REFCOUNTED_HE_KEY_UTF8;
    if (!hash && SvIsCOW_shared_hash(key))
        hash = SvSHARED_HASH(key);
    return refcounted_he_fetch_pvn(chain, keypv, keylen, hash, flags);
}

/*
=for apidoc refcounted_he_new_pvn

Creates a new C<refcounted_he>.  This consists of a single key/value
pair and a reference to an existing C<refcounted_he> chain (which may
be empty), and thus forms a longer chain.  When using the longer chain,
the new key/value pair takes precedence over any entry for the same key
further along the chain.

The new key is specified by C<keypv> and C<keylen>.  If C<flags> has
the C<REFCOUNTED_HE_KEY_UTF8> bit set, the key octets are interpreted
as UTF-8, otherwise they are interpreted as Latin-1.  C<hash> is
a precomputed hash of the key string, or zero if it has not been
precomputed.

C<value> is the scalar value to store for this key.  C<value> is copied
by this function, which thus does not take ownership of any reference
to it, and later changes to the scalar will not be reflected in the
value visible in the C<refcounted_he>.  Complex types of scalar will not
be stored with referential integrity, but will be coerced to strings.
C<value> may be either null or C<&PL_sv_placeholder> to indicate that no
value is to be associated with the key; this, as with any non-null value,
takes precedence over the existence of a value for the key further along
the chain.

C<parent> points to the rest of the C<refcounted_he> chain to be
attached to the new C<refcounted_he>.  This function takes ownership
of one reference to C<parent>, and returns one reference to the new
C<refcounted_he>.

=cut
*/

struct refcounted_he *
Perl_refcounted_he_new_pvn(pTHX_ struct refcounted_he *parent,
        const char *keypv, STRLEN keylen, U32 hash, SV *value, U32 flags)
{
    STRLEN value_len = 0;
    const char *value_p = NULL;
    bool is_pv;
    char value_type;
    char hekflags;
    STRLEN key_offset = 1;
    struct refcounted_he *he;
    PERL_ARGS_ASSERT_REFCOUNTED_HE_NEW_PVN;

    if (!value || value == &PL_sv_placeholder) {
        value_type = HVrhek_delete;
    } else if (SvPOK(value)) {
        value_type = HVrhek_PV;
    } else if (SvIOK(value)) {
        value_type = SvUOK((const SV *)value) ? HVrhek_UV : HVrhek_IV;
    } else if (!SvOK(value)) {
        value_type = HVrhek_undef;
    } else {
        value_type = HVrhek_PV;
    }
    is_pv = value_type == HVrhek_PV;
    if (is_pv) {
        /* Do it this way so that the SvUTF8() test is after the SvPV, in case
           the value is overloaded, and doesn't yet have the UTF-8flag set.  */
        value_p = SvPV_const(value, value_len);
        if (SvUTF8(value))
            value_type = HVrhek_PV_UTF8;
        key_offset = value_len + 2;
    }
    hekflags = value_type;

    if (flags & REFCOUNTED_HE_KEY_UTF8) {
        /* Canonicalise to Latin-1 where possible. */
        const char *keyend = keypv + keylen, *p;
        STRLEN nonascii_count = 0;
        for (p = keypv; p != keyend; p++) {
            if (! UTF8_IS_INVARIANT(*p)) {
                if (! UTF8_IS_NEXT_CHAR_DOWNGRADEABLE(p, keyend)) {
                    goto canonicalised_key;
                }
                nonascii_count++;
                p++;
            }
        }
        if (nonascii_count) {
            char *q;
            const char *p = keypv, *keyend = keypv + keylen;
            keylen -= nonascii_count;
            Newx(q, keylen, char);
            SAVEFREEPV(q);
            keypv = q;
            for (; p != keyend; p++, q++) {
                U8 c = (U8)*p;
                if (UTF8_IS_INVARIANT(c)) {
                    *q = (char) c;
                }
                else {
                    p++;
                    *q = (char) EIGHT_BIT_UTF8_TO_NATIVE(c, *p);
                }
            }
        }
        flags &= ~REFCOUNTED_HE_KEY_UTF8;
        canonicalised_key: ;
    }
    if (flags & REFCOUNTED_HE_KEY_UTF8)
        hekflags |= HVhek_UTF8;
    if (!hash)
        PERL_HASH(hash, keypv, keylen);

#ifdef USE_ITHREADS
    he = (struct refcounted_he*)
        PerlMemShared_malloc(sizeof(struct refcounted_he) - 1
                             + keylen
                             + key_offset);
#else
    he = (struct refcounted_he*)
        PerlMemShared_malloc(sizeof(struct refcounted_he) - 1
                             + key_offset);
#endif

    he->refcounted_he_next = parent;

    if (is_pv) {
        Copy(value_p, he->refcounted_he_data + 1, value_len + 1, char);
        he->refcounted_he_val.refcounted_he_u_len = value_len;
    } else if (value_type == HVrhek_IV) {
        he->refcounted_he_val.refcounted_he_u_iv = SvIVX(value);
    } else if (value_type == HVrhek_UV) {
        he->refcounted_he_val.refcounted_he_u_uv = SvUVX(value);
    }

#ifdef USE_ITHREADS
    he->refcounted_he_hash = hash;
    he->refcounted_he_keylen = keylen;
    Copy(keypv, he->refcounted_he_data + key_offset, keylen, char);
#else
    he->refcounted_he_hek = share_hek_flags(keypv, keylen, hash, hekflags);
#endif

    he->refcounted_he_data[0] = hekflags;
    he->refcounted_he_refcnt = 1;

    return he;
}

/*
=for apidoc refcounted_he_new_pv

Like L</refcounted_he_new_pvn>, but takes a nul-terminated string instead
of a string/length pair.

=cut
*/

struct refcounted_he *
Perl_refcounted_he_new_pv(pTHX_ struct refcounted_he *parent,
        const char *key, U32 hash, SV *value, U32 flags)
{
    PERL_ARGS_ASSERT_REFCOUNTED_HE_NEW_PV;
    return refcounted_he_new_pvn(parent, key, strlen(key), hash, value, flags);
}

/*
=for apidoc refcounted_he_new_sv

Like L</refcounted_he_new_pvn>, but takes a Perl scalar instead of a
string/length pair.

=cut
*/

struct refcounted_he *
Perl_refcounted_he_new_sv(pTHX_ struct refcounted_he *parent,
        SV *key, U32 hash, SV *value, U32 flags)
{
    const char *keypv;
    STRLEN keylen;
    PERL_ARGS_ASSERT_REFCOUNTED_HE_NEW_SV;
    if (flags & REFCOUNTED_HE_KEY_UTF8)
        Perl_croak(aTHX_ "panic: refcounted_he_new_sv bad flags %" UVxf,
            (UV)flags);
    keypv = SvPV_const(key, keylen);
    if (SvUTF8(key))
        flags |= REFCOUNTED_HE_KEY_UTF8;
    if (!hash && SvIsCOW_shared_hash(key))
        hash = SvSHARED_HASH(key);
    return refcounted_he_new_pvn(parent, keypv, keylen, hash, value, flags);
}

/*
=for apidoc refcounted_he_free

Decrements the reference count of a C<refcounted_he> by one.  If the
reference count reaches zero the structure's memory is freed, which
(recursively) causes a reduction of its parent C<refcounted_he>'s
reference count.  It is safe to pass a null pointer to this function:
no action occurs in this case.

=cut
*/

void
Perl_refcounted_he_free(pTHX_ struct refcounted_he *he) {
    PERL_UNUSED_CONTEXT;

    while (he) {
        struct refcounted_he *copy;
        U32 new_count;

        HINTS_REFCNT_LOCK;
        new_count = --he->refcounted_he_refcnt;
        HINTS_REFCNT_UNLOCK;
        
        if (new_count) {
            return;
        }

#ifndef USE_ITHREADS
        unshare_hek_or_pvn (he->refcounted_he_hek, 0, 0, 0);
#endif
        copy = he;
        he = he->refcounted_he_next;
        PerlMemShared_free(copy);
    }
}

/*
=for apidoc refcounted_he_inc

Increment the reference count of a C<refcounted_he>.  The pointer to the
C<refcounted_he> is also returned.  It is safe to pass a null pointer
to this function: no action occurs and a null pointer is returned.

=cut
*/

struct refcounted_he *
Perl_refcounted_he_inc(pTHX_ struct refcounted_he *he)
{
    PERL_UNUSED_CONTEXT;
    if (he) {
        HINTS_REFCNT_LOCK;
        he->refcounted_he_refcnt++;
        HINTS_REFCNT_UNLOCK;
    }
    return he;
}

/*
=for apidoc_section $COP
=for apidoc cop_fetch_label

Returns the label attached to a cop, and stores its length in bytes into
C<*len>.
Upon return, C<*flags> will be set to either C<SVf_UTF8> or 0.

Alternatively, use the macro C<L</CopLABEL_len_flags>>;
or if you don't need to know if the label is UTF-8 or not, the macro
C<L</CopLABEL_len>>;
or if you additionally don't need to know the length, C<L</CopLABEL>>.

=cut
*/

/* pp_entereval is aware that labels are stored with a key ':' at the top of
   the linked list.  */
const char *
Perl_cop_fetch_label(pTHX_ COP *const cop, STRLEN *len, U32 *flags) {
    struct refcounted_he *const chain = cop->cop_hints_hash;

    PERL_ARGS_ASSERT_COP_FETCH_LABEL;
    PERL_UNUSED_CONTEXT;

    if (!chain)
        return NULL;
#ifdef USE_ITHREADS
    if (chain->refcounted_he_keylen != 1)
        return NULL;
    if (*REF_HE_KEY(chain) != ':')
        return NULL;
#else
    if ((STRLEN)HEK_LEN(chain->refcounted_he_hek) != 1)
        return NULL;
    if (*HEK_KEY(chain->refcounted_he_hek) != ':')
        return NULL;
#endif
    /* Stop anyone trying to really mess us up by adding their own value for
       ':' into %^H  */
    if ((chain->refcounted_he_data[0] & HVrhek_typemask) != HVrhek_PV
        && (chain->refcounted_he_data[0] & HVrhek_typemask) != HVrhek_PV_UTF8)
        return NULL;

    if (len)
        *len = chain->refcounted_he_val.refcounted_he_u_len;
    if (flags) {
        *flags = ((chain->refcounted_he_data[0] & HVrhek_typemask)
                  == HVrhek_PV_UTF8) ? SVf_UTF8 : 0;
    }
    return chain->refcounted_he_data + 1;
}

/*
=for apidoc cop_store_label

Save a label into a C<cop_hints_hash>.
You need to set flags to C<SVf_UTF8>
for a UTF-8 label.  Any other flag is ignored.

=cut
*/

void
Perl_cop_store_label(pTHX_ COP *const cop, const char *label, STRLEN len,
                     U32 flags)
{
    SV *labelsv;
    PERL_ARGS_ASSERT_COP_STORE_LABEL;

    if (flags & ~(SVf_UTF8))
        Perl_croak(aTHX_ "panic: cop_store_label illegal flag bits 0x%" UVxf,
                   (UV)flags);
    labelsv = newSVpvn_flags(label, len, SVs_TEMP);
    if (flags & SVf_UTF8)
        SvUTF8_on(labelsv);
    cop->cop_hints_hash
        = refcounted_he_new_pvs(cop->cop_hints_hash, ":", labelsv, 0);
}

/*
=for apidoc_section $HV
=for apidoc hv_assert

Check that a hash is in an internally consistent state.

=cut
*/

#ifdef DEBUGGING

void
Perl_hv_assert(pTHX_ HV *hv)
{
    HE* entry;
    int withflags = 0;
    int placeholders = 0;
    int real = 0;
    int bad = 0;
    const I32 riter = HvRITER_get(hv);
    HE *eiter = HvEITER_get(hv);

    PERL_ARGS_ASSERT_HV_ASSERT;

    (void)hv_iterinit(hv);

    while ((entry = hv_iternext_flags(hv, HV_ITERNEXT_WANTPLACEHOLDERS))) {
        /* sanity check the values */
        if (HeVAL(entry) == &PL_sv_placeholder)
            placeholders++;
        else
            real++;
        /* sanity check the keys */
        if (HeSVKEY(entry)) {
            NOOP;   /* Don't know what to check on SV keys.  */
        } else if (HeKUTF8(entry)) {
            withflags++;
            if (HeKWASUTF8(entry)) {
                PerlIO_printf(Perl_debug_log,
                            "hash key has both WASUTF8 and UTF8: '%.*s'\n",
                            (int) HeKLEN(entry),  HeKEY(entry));
                bad = 1;
            }
        } else if (HeKWASUTF8(entry))
            withflags++;
    }
    if (!SvTIED_mg((const SV *)hv, PERL_MAGIC_tied)) {
        static const char bad_count[] = "Count %d %s(s), but hash reports %d\n";
        const int nhashkeys = HvUSEDKEYS(hv);
        const int nhashplaceholders = HvPLACEHOLDERS_get(hv);

        if (nhashkeys != real) {
            PerlIO_printf(Perl_debug_log, bad_count, real, "keys", nhashkeys );
            bad = 1;
        }
        if (nhashplaceholders != placeholders) {
            PerlIO_printf(Perl_debug_log, bad_count, placeholders, "placeholder", nhashplaceholders );
            bad = 1;
        }
    }
    if (withflags && ! HvHASKFLAGS(hv)) {
        PerlIO_printf(Perl_debug_log,
                    "Hash has HASKFLAGS off but I count %d key(s) with flags\n",
                    withflags);
        bad = 1;
    }
    if (bad) {
        sv_dump(MUTABLE_SV(hv));
    }
    HvRITER_set(hv, riter);		/* Restore hash iterator state */
    HvEITER_set(hv, eiter);
}

#endif

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
