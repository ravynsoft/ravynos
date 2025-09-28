/*    sv_inline.h
 *
 *    Copyright (C) 2022 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* This file contains the newSV_type and newSV_type_mortal functions, as well as
 * the various struct and macro definitions they require. In the main, these
 * definitions were moved from sv.c, where many of them continue to also be used.
 * (In Perl_more_bodies, Perl_sv_upgrade and Perl_sv_clear, for example.) Code
 * comments associated with definitions and functions were also copied across
 * verbatim.
 *
 * The rationale for having these as inline functions, rather than in sv.c, is
 * that the target type is very often known at compile time, and therefore
 * optimum code can be emitted by the compiler, rather than having all calls
 * traverse the many branches of Perl_sv_upgrade at runtime.
 */

/* This definition came from perl.h*/

/* The old value was hard coded at 1008. (4096-16) seems to be a bit faster,
   at least on FreeBSD.  YMMV, so experiment.  */
#ifndef PERL_ARENA_SIZE
#define PERL_ARENA_SIZE 4080
#endif

/* All other pre-existing definitions and functions that were moved into this
 * file originally came from sv.c. */

#ifdef PERL_POISON
#  define SvARENA_CHAIN(sv)     ((sv)->sv_u.svu_rv)
#  define SvARENA_CHAIN_SET(sv,val)     (sv)->sv_u.svu_rv = MUTABLE_SV((val))
/* Whilst I'd love to do this, it seems that things like to check on
   unreferenced scalars
#  define POISON_SV_HEAD(sv)    PoisonNew(sv, 1, struct STRUCT_SV)
*/
#  define POISON_SV_HEAD(sv)    PoisonNew(&SvANY(sv), 1, void *), \
                                PoisonNew(&SvREFCNT(sv), 1, U32)
#else
#  define SvARENA_CHAIN(sv)     SvANY(sv)
#  define SvARENA_CHAIN_SET(sv,val)     SvANY(sv) = (void *)(val)
#  define POISON_SV_HEAD(sv)
#endif

#ifdef PERL_MEM_LOG
#  define MEM_LOG_NEW_SV(sv, file, line, func)  \
            Perl_mem_log_new_sv(sv, file, line, func)
#  define MEM_LOG_DEL_SV(sv, file, line, func)  \
            Perl_mem_log_del_sv(sv, file, line, func)
#else
#  define MEM_LOG_NEW_SV(sv, file, line, func)  NOOP
#  define MEM_LOG_DEL_SV(sv, file, line, func)  NOOP
#endif

#define uproot_SV(p) \
    STMT_START {                                        \
        (p) = PL_sv_root;                               \
        PL_sv_root = MUTABLE_SV(SvARENA_CHAIN(p));              \
        ++PL_sv_count;                                  \
    } STMT_END

/* Perl_more_sv lives in sv.c, we don't want to inline it.
 * but the function declaration seems to be needed. */
SV* Perl_more_sv(pTHX);

/* new_SV(): return a new, empty SV head */

#ifdef DEBUG_LEAKING_SCALARS
/* provide a real function for a debugger to play with */
STATIC SV*
S_new_SV(pTHX_ const char *file, int line, const char *func)
{
    SV* sv;

    if (PL_sv_root)
        uproot_SV(sv);
    else
        sv = Perl_more_sv(aTHX);
    SvANY(sv) = 0;
    SvREFCNT(sv) = 1;
    SvFLAGS(sv) = 0;
    sv->sv_debug_optype = PL_op ? PL_op->op_type : 0;
    sv->sv_debug_line = (U16) (PL_parser && PL_parser->copline != NOLINE
                ? PL_parser->copline
                :  PL_curcop
                    ? CopLINE(PL_curcop)
                    : 0
            );
    sv->sv_debug_inpad = 0;
    sv->sv_debug_parent = NULL;
    sv->sv_debug_file = PL_curcop ? savesharedpv(CopFILE(PL_curcop)): NULL;

    sv->sv_debug_serial = PL_sv_serial++;

    MEM_LOG_NEW_SV(sv, file, line, func);
    DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%" UVxf ": (%05ld) new_SV (from %s:%d [%s])\n",
            PTR2UV(sv), (long)sv->sv_debug_serial, file, line, func));

    return sv;
}
#  define new_SV(p) (p)=S_new_SV(aTHX_ __FILE__, __LINE__, FUNCTION__)

#else
#  define new_SV(p) \
    STMT_START {                                       \
        if (PL_sv_root)                                        \
            uproot_SV(p);                              \
        else                                           \
            (p) = Perl_more_sv(aTHX);                     \
        SvANY(p) = 0;                                  \
        SvREFCNT(p) = 1;                               \
        SvFLAGS(p) = 0;                                        \
        MEM_LOG_NEW_SV(p, __FILE__, __LINE__, FUNCTION__);  \
    } STMT_END
#endif


typedef struct xpvhv_with_aux XPVHV_WITH_AUX;

struct body_details {
    U8 body_size;      /* Size to allocate  */
    U8 copy;           /* Size of structure to copy (may be shorter)  */
    U8 offset;         /* Size of unalloced ghost fields to first alloced field*/
    PERL_BITFIELD8 type : 5;        /* We have space for a sanity check. */
    PERL_BITFIELD8 cant_upgrade : 1;/* Cannot upgrade this type */
    PERL_BITFIELD8 zero_nv : 1;     /* zero the NV when upgrading from this */
    PERL_BITFIELD8 arena : 1;       /* Allocated from an arena */
    U32 arena_size;                 /* Size of arena to allocate */
};

#define ALIGNED_TYPE_NAME(name) name##_aligned
#define ALIGNED_TYPE(name)             \
    typedef union {    \
        name align_me;                         \
        NV nv;                         \
        IV iv;                         \
    } ALIGNED_TYPE_NAME(name)

ALIGNED_TYPE(regexp);
ALIGNED_TYPE(XPVGV);
ALIGNED_TYPE(XPVLV);
ALIGNED_TYPE(XPVAV);
ALIGNED_TYPE(XPVHV);
ALIGNED_TYPE(XPVHV_WITH_AUX);
ALIGNED_TYPE(XPVCV);
ALIGNED_TYPE(XPVFM);
ALIGNED_TYPE(XPVIO);
ALIGNED_TYPE(XPVOBJ);

#define HADNV FALSE
#define NONV TRUE


#ifdef PURIFY
/* With -DPURFIY we allocate everything directly, and don't use arenas.
   This seems a rather elegant way to simplify some of the code below.  */
#define HASARENA FALSE
#else
#define HASARENA TRUE
#endif
#define NOARENA FALSE

/* Size the arenas to exactly fit a given number of bodies.  A count
   of 0 fits the max number bodies into a PERL_ARENA_SIZE.block,
   simplifying the default.  If count > 0, the arena is sized to fit
   only that many bodies, allowing arenas to be used for large, rare
   bodies (XPVFM, XPVIO) without undue waste.  The arena size is
   limited by PERL_ARENA_SIZE, so we can safely oversize the
   declarations.
 */
#define FIT_ARENA0(body_size)                          \
    ((size_t)(PERL_ARENA_SIZE / body_size) * body_size)
#define FIT_ARENAn(count,body_size)                    \
    ( count * body_size <= PERL_ARENA_SIZE)            \
    ? count * body_size                                        \
    : FIT_ARENA0 (body_size)
#define FIT_ARENA(count,body_size)                     \
   (U32)(count                                                 \
    ? FIT_ARENAn (count, body_size)                    \
    : FIT_ARENA0 (body_size))

/* Calculate the length to copy. Specifically work out the length less any
   final padding the compiler needed to add.  See the comment in sv_upgrade
   for why copying the padding proved to be a bug.  */

#define copy_length(type, last_member) \
        STRUCT_OFFSET(type, last_member) \
        + sizeof (((type*)SvANY((const SV *)0))->last_member)

static const struct body_details bodies_by_type[] = {
    /* HEs use this offset for their arena.  */
    { 0, 0, 0, SVt_NULL, FALSE, NONV, NOARENA, 0 },

    /* IVs are in the head, so the allocation size is 0.  */
    { 0,
      sizeof(IV), /* This is used to copy out the IV body.  */
      STRUCT_OFFSET(XPVIV, xiv_iv), SVt_IV, FALSE, NONV,
      NOARENA /* IVS don't need an arena  */, 0
    },

#if NVSIZE <= IVSIZE
    { 0, sizeof(NV),
      STRUCT_OFFSET(XPVNV, xnv_u),
      SVt_NV, FALSE, HADNV, NOARENA, 0 },
#else
    { sizeof(NV), sizeof(NV),
      STRUCT_OFFSET(XPVNV, xnv_u),
      SVt_NV, FALSE, HADNV, HASARENA, FIT_ARENA(0, sizeof(NV)) },
#endif

    { sizeof(XPV) - STRUCT_OFFSET(XPV, xpv_cur),
      copy_length(XPV, xpv_len) - STRUCT_OFFSET(XPV, xpv_cur),
      + STRUCT_OFFSET(XPV, xpv_cur),
      SVt_PV, FALSE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(XPV) - STRUCT_OFFSET(XPV, xpv_cur)) },

    { sizeof(XINVLIST) - STRUCT_OFFSET(XPV, xpv_cur),
      copy_length(XINVLIST, is_offset) - STRUCT_OFFSET(XPV, xpv_cur),
      + STRUCT_OFFSET(XPV, xpv_cur),
      SVt_INVLIST, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(XINVLIST) - STRUCT_OFFSET(XPV, xpv_cur)) },

    { sizeof(XPVIV) - STRUCT_OFFSET(XPV, xpv_cur),
      copy_length(XPVIV, xiv_u) - STRUCT_OFFSET(XPV, xpv_cur),
      + STRUCT_OFFSET(XPV, xpv_cur),
      SVt_PVIV, FALSE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(XPVIV) - STRUCT_OFFSET(XPV, xpv_cur)) },

    { sizeof(XPVNV) - STRUCT_OFFSET(XPV, xpv_cur),
      copy_length(XPVNV, xnv_u) - STRUCT_OFFSET(XPV, xpv_cur),
      + STRUCT_OFFSET(XPV, xpv_cur),
      SVt_PVNV, FALSE, HADNV, HASARENA,
      FIT_ARENA(0, sizeof(XPVNV) - STRUCT_OFFSET(XPV, xpv_cur)) },

    { sizeof(XPVMG), copy_length(XPVMG, xnv_u), 0, SVt_PVMG, FALSE, HADNV,
      HASARENA, FIT_ARENA(0, sizeof(XPVMG)) },

    { sizeof(ALIGNED_TYPE_NAME(regexp)),
      sizeof(regexp),
      0,
      SVt_REGEXP, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(ALIGNED_TYPE_NAME(regexp)))
    },

    { sizeof(ALIGNED_TYPE_NAME(XPVGV)), sizeof(XPVGV), 0, SVt_PVGV, TRUE, HADNV,
      HASARENA, FIT_ARENA(0, sizeof(ALIGNED_TYPE_NAME(XPVGV))) },

    { sizeof(ALIGNED_TYPE_NAME(XPVLV)), sizeof(XPVLV), 0, SVt_PVLV, TRUE, HADNV,
      HASARENA, FIT_ARENA(0, sizeof(ALIGNED_TYPE_NAME(XPVLV))) },

    { sizeof(ALIGNED_TYPE_NAME(XPVAV)),
      copy_length(XPVAV, xav_alloc),
      0,
      SVt_PVAV, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(ALIGNED_TYPE_NAME(XPVAV))) },

    { sizeof(ALIGNED_TYPE_NAME(XPVHV)),
      copy_length(XPVHV, xhv_max),
      0,
      SVt_PVHV, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(ALIGNED_TYPE_NAME(XPVHV))) },

    { sizeof(ALIGNED_TYPE_NAME(XPVCV)),
      sizeof(XPVCV),
      0,
      SVt_PVCV, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(ALIGNED_TYPE_NAME(XPVCV))) },

    { sizeof(ALIGNED_TYPE_NAME(XPVFM)),
      sizeof(XPVFM),
      0,
      SVt_PVFM, TRUE, NONV, NOARENA,
      FIT_ARENA(20, sizeof(ALIGNED_TYPE_NAME(XPVFM))) },

    { sizeof(ALIGNED_TYPE_NAME(XPVIO)),
      sizeof(XPVIO),
      0,
      SVt_PVIO, TRUE, NONV, HASARENA,
      FIT_ARENA(24, sizeof(ALIGNED_TYPE_NAME(XPVIO))) },

    { sizeof(ALIGNED_TYPE_NAME(XPVOBJ)),
      copy_length(XPVOBJ, xobject_fields),
      0,
      SVt_PVOBJ, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(ALIGNED_TYPE_NAME(XPVOBJ))) },
};

#define new_body_allocated(sv_type)            \
    (void *)((char *)S_new_body(aTHX_ sv_type) \
             - bodies_by_type[sv_type].offset)

#ifdef PURIFY
#if !(NVSIZE <= IVSIZE)
#  define new_XNV()    safemalloc(sizeof(XPVNV))
#endif
#define new_XPVNV()    safemalloc(sizeof(XPVNV))
#define new_XPVMG()    safemalloc(sizeof(XPVMG))

#define del_body_by_type(p, type)       safefree(p)

#else /* !PURIFY */

#if !(NVSIZE <= IVSIZE)
#  define new_XNV()    new_body_allocated(SVt_NV)
#endif
#define new_XPVNV()    new_body_allocated(SVt_PVNV)
#define new_XPVMG()    new_body_allocated(SVt_PVMG)

#define del_body_by_type(p, type)                               \
    del_body(p + bodies_by_type[(type)].offset,                 \
             &PL_body_roots[(type)])

#endif /* PURIFY */

/* no arena for you! */

#define new_NOARENA(details) \
        safemalloc((details)->body_size + (details)->offset)
#define new_NOARENAZ(details) \
        safecalloc((details)->body_size + (details)->offset, 1)

#ifndef PURIFY

/* grab a new thing from the arena's free list, allocating more if necessary. */
#define new_body_from_arena(xpv, root_index, type_meta) \
    STMT_START { \
        void ** const r3wt = &PL_body_roots[root_index]; \
        xpv = (PTR_TBL_ENT_t*) (*((void **)(r3wt))      \
          ? *((void **)(r3wt)) : Perl_more_bodies(aTHX_ root_index, \
                                             type_meta.body_size,\
                                             type_meta.arena_size)); \
        *(r3wt) = *(void**)(xpv); \
    } STMT_END

PERL_STATIC_INLINE void *
S_new_body(pTHX_ const svtype sv_type)
{
    void *xpv;
    new_body_from_arena(xpv, sv_type, bodies_by_type[sv_type]);
    return xpv;
}

#endif

static const struct body_details fake_rv =
    { 0, 0, 0, SVt_IV, FALSE, NONV, NOARENA, 0 };

static const struct body_details fake_hv_with_aux =
    /* The SVt_IV arena is used for (larger) PVHV bodies.  */
    { sizeof(ALIGNED_TYPE_NAME(XPVHV_WITH_AUX)),
      copy_length(XPVHV, xhv_max),
      0,
      SVt_PVHV, TRUE, NONV, HASARENA,
      FIT_ARENA(0, sizeof(ALIGNED_TYPE_NAME(XPVHV_WITH_AUX))) };

/*
=for apidoc newSV_type

Creates a new SV, of the type specified.  The reference count for the new SV
is set to 1.

=cut
*/

PERL_STATIC_INLINE SV *
Perl_newSV_type(pTHX_ const svtype type)
{
    SV *sv;
    void*      new_body;
    const struct body_details *type_details;

    new_SV(sv);

    type_details = bodies_by_type + type;

    SvFLAGS(sv) &= ~SVTYPEMASK;
    SvFLAGS(sv) |= type;

    switch (type) {
    case SVt_NULL:
        break;
    case SVt_IV:
        SET_SVANY_FOR_BODYLESS_IV(sv);
        SvIV_set(sv, 0);
        break;
    case SVt_NV:
#if NVSIZE <= IVSIZE
        SET_SVANY_FOR_BODYLESS_NV(sv);
#else
        SvANY(sv) = new_XNV();
#endif
        SvNV_set(sv, 0);
        break;
    case SVt_PVHV:
    case SVt_PVAV:
    case SVt_PVOBJ:
        assert(type_details->body_size);

#ifndef PURIFY
        assert(type_details->arena);
        assert(type_details->arena_size);
        /* This points to the start of the allocated area.  */
        new_body = S_new_body(aTHX_ type);
        /* xpvav and xpvhv have no offset, so no need to adjust new_body */
        assert(!(type_details->offset));
#else
        /* We always allocated the full length item with PURIFY. To do this
           we fake things so that arena is false for all 16 types..  */
        new_body = new_NOARENAZ(type_details);
#endif
        SvANY(sv) = new_body;

        SvSTASH_set(sv, NULL);
        SvMAGIC_set(sv, NULL);

        switch(type) {
        case SVt_PVAV:
            AvFILLp(sv) = -1;
            AvMAX(sv) = -1;
            AvALLOC(sv) = NULL;

            AvREAL_only(sv);
            break;
        case SVt_PVHV:
            HvTOTALKEYS(sv) = 0;
            /* start with PERL_HASH_DEFAULT_HvMAX+1 buckets: */
            HvMAX(sv) = PERL_HASH_DEFAULT_HvMAX;

            assert(!SvOK(sv));
            SvOK_off(sv);
#ifndef NODEFAULT_SHAREKEYS
            HvSHAREKEYS_on(sv);         /* key-sharing on by default */
#endif
            /* start with PERL_HASH_DEFAULT_HvMAX+1 buckets: */
            HvMAX(sv) = PERL_HASH_DEFAULT_HvMAX;
            break;
        case SVt_PVOBJ:
            ObjectMAXFIELD(sv) = -1;
            ObjectFIELDS(sv) = NULL;
            break;
        default:
            NOT_REACHED;
        }

        sv->sv_u.svu_array = NULL; /* or svu_hash  */
        break;

    case SVt_PVIV:
    case SVt_PVIO:
    case SVt_PVGV:
    case SVt_PVCV:
    case SVt_PVLV:
    case SVt_INVLIST:
    case SVt_REGEXP:
    case SVt_PVMG:
    case SVt_PVNV:
    case SVt_PV:
        /* For a type known at compile time, it should be possible for the
         * compiler to deduce the value of (type_details->arena), resolve
         * that branch below, and inline the relevant values from
         * bodies_by_type. Except, at least for gcc, it seems not to do that.
         * We help it out here with two deviations from sv_upgrade:
         * (1) Minor rearrangement here, so that PVFM - the only type at this
         *     point not to be allocated from an array appears last, not PV.
         * (2) The ASSUME() statement here for everything that isn't PVFM.
         * Obviously this all only holds as long as it's a true reflection of
         * the bodies_by_type lookup table. */
#ifndef PURIFY
         ASSUME(type_details->arena);
#endif
         /* FALLTHROUGH */
    case SVt_PVFM:

        assert(type_details->body_size);
        /* We always allocated the full length item with PURIFY. To do this
           we fake things so that arena is false for all 16 types..  */
#ifndef PURIFY
        if(type_details->arena) {
            /* This points to the start of the allocated area.  */
            new_body = S_new_body(aTHX_ type);
            Zero(new_body, type_details->body_size, char);
            new_body = ((char *)new_body) - type_details->offset;
        } else
#endif
        {
            new_body = new_NOARENAZ(type_details);
        }
        SvANY(sv) = new_body;

        if (UNLIKELY(type == SVt_PVIO)) {
            IO * const io = MUTABLE_IO(sv);
            GV *iogv = gv_fetchpvs("IO::File::", GV_ADD, SVt_PVHV);

            SvOBJECT_on(io);
            /* Clear the stashcache because a new IO could overrule a package
               name */
            DEBUG_o(Perl_deb(aTHX_ "sv_upgrade clearing PL_stashcache\n"));
            hv_clear(PL_stashcache);

            SvSTASH_set(io, MUTABLE_HV(SvREFCNT_inc(GvHV(iogv))));
            IoPAGE_LEN(sv) = 60;
        }

        sv->sv_u.svu_rv = NULL;
        break;
    default:
        Perl_croak(aTHX_ "panic: sv_upgrade to unknown type %lu",
                   (unsigned long)type);
    }

    return sv;
}

/*
=for apidoc newSV_type_mortal

Creates a new mortal SV, of the type specified.  The reference count for the
new SV is set to 1.

This is equivalent to
    SV* sv = sv_2mortal(newSV_type(<some type>))
and
    SV* sv = sv_newmortal();
    sv_upgrade(sv, <some_type>)
but should be more efficient than both of them. (Unless sv_2mortal is inlined
at some point in the future.)

=cut
*/

PERL_STATIC_INLINE SV *
Perl_newSV_type_mortal(pTHX_ const svtype type)
{
    SV *sv = newSV_type(type);
    SSize_t ix = ++PL_tmps_ix;
    if (UNLIKELY(ix >= PL_tmps_max))
        ix = Perl_tmps_grow_p(aTHX_ ix);
    PL_tmps_stack[ix] = (sv);
    SvTEMP_on(sv);
    return sv;
}

/* The following functions started out in sv.h and then moved to inline.h. They
 * moved again into this file during the 5.37.x development cycle. */

/*
=for apidoc_section $SV
=for apidoc SvPVXtrue

Returns a boolean as to whether or not C<sv> contains a PV that is considered
TRUE.  FALSE is returned if C<sv> doesn't contain a PV, or if the PV it does
contain is zero length, or consists of just the single character '0'.  Every
other PV value is considered TRUE.

As of Perl v5.37.1, C<sv> is evaluated exactly once; in earlier releases, it
could be evaluated more than once.

=cut
*/

PERL_STATIC_INLINE bool
Perl_SvPVXtrue(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SVPVXTRUE;

    if (! (XPV *) SvANY(sv)) {
        return false;
    }

    if ( ((XPV *) SvANY(sv))->xpv_cur > 1) { /* length > 1 */
        return true;
    }

    if (( (XPV *) SvANY(sv))->xpv_cur == 0) {
        return false;
    }

    return *sv->sv_u.svu_pv != '0';
}

/*
=for apidoc SvGETMAGIC
Invokes C<L</mg_get>> on an SV if it has 'get' magic.  For example, this
will call C<FETCH> on a tied variable.  As of 5.37.1, this function is
guaranteed to evaluate its argument exactly once.

=cut
*/

PERL_STATIC_INLINE void
Perl_SvGETMAGIC(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SVGETMAGIC;

    if (UNLIKELY(SvGMAGICAL(sv))) {
        mg_get(sv);
    }
}

PERL_STATIC_INLINE bool
Perl_SvTRUE(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SVTRUE;

    if (UNLIKELY(sv == NULL))
        return FALSE;
    SvGETMAGIC(sv);
    return SvTRUE_nomg_NN(sv);
}

PERL_STATIC_INLINE bool
Perl_SvTRUE_nomg(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SVTRUE_NOMG;

    if (UNLIKELY(sv == NULL))
        return FALSE;
    return SvTRUE_nomg_NN(sv);
}

PERL_STATIC_INLINE bool
Perl_SvTRUE_NN(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SVTRUE_NN;

    SvGETMAGIC(sv);
    return SvTRUE_nomg_NN(sv);
}

PERL_STATIC_INLINE bool
Perl_SvTRUE_common(pTHX_ SV * sv, const bool sv_2bool_is_fallback)
{
    PERL_ARGS_ASSERT_SVTRUE_COMMON;

    if (UNLIKELY(SvIMMORTAL_INTERP(sv)))
        return SvIMMORTAL_TRUE(sv);

    if (! SvOK(sv))
        return FALSE;

    if (SvPOK(sv))
        return SvPVXtrue(sv);

    if (SvIOK(sv))
        return SvIVX(sv) != 0; /* casts to bool */

    if (SvROK(sv) && !(SvOBJECT(SvRV(sv)) && HvAMAGIC(SvSTASH(SvRV(sv)))))
        return TRUE;

    if (sv_2bool_is_fallback)
        return sv_2bool_nomg(sv);

    return isGV_with_GP(sv);
}

PERL_STATIC_INLINE SV *
Perl_SvREFCNT_inc(SV *sv)
{
    if (LIKELY(sv != NULL))
        SvREFCNT(sv)++;
    return sv;
}

PERL_STATIC_INLINE SV *
Perl_SvREFCNT_inc_NN(SV *sv)
{
    PERL_ARGS_ASSERT_SVREFCNT_INC_NN;

    SvREFCNT(sv)++;
    return sv;
}

PERL_STATIC_INLINE void
Perl_SvREFCNT_inc_void(SV *sv)
{
    if (LIKELY(sv != NULL))
        SvREFCNT(sv)++;
}

PERL_STATIC_INLINE void
Perl_SvREFCNT_dec(pTHX_ SV *sv)
{
    if (LIKELY(sv != NULL)) {
        U32 rc = SvREFCNT(sv);
        if (LIKELY(rc > 1))
            SvREFCNT(sv) = rc - 1;
        else
            Perl_sv_free2(aTHX_ sv, rc);
    }
}

PERL_STATIC_INLINE SV *
Perl_SvREFCNT_dec_ret_NULL(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SVREFCNT_DEC_RET_NULL;
    Perl_SvREFCNT_dec(aTHX_ sv);
    return NULL;
}


PERL_STATIC_INLINE void
Perl_SvREFCNT_dec_NN(pTHX_ SV *sv)
{
    U32 rc = SvREFCNT(sv);

    PERL_ARGS_ASSERT_SVREFCNT_DEC_NN;

    if (LIKELY(rc > 1))
        SvREFCNT(sv) = rc - 1;
    else
        Perl_sv_free2(aTHX_ sv, rc);
}

/*
=for apidoc SvAMAGIC_on

Indicate that C<sv> has overloading (active magic) enabled.

=cut
*/

PERL_STATIC_INLINE void
Perl_SvAMAGIC_on(SV *sv)
{
    PERL_ARGS_ASSERT_SVAMAGIC_ON;
    assert(SvROK(sv));

    if (SvOBJECT(SvRV(sv))) HvAMAGIC_on(SvSTASH(SvRV(sv)));
}

/*
=for apidoc SvAMAGIC_off

Indicate that C<sv> has overloading (active magic) disabled.

=cut
*/

PERL_STATIC_INLINE void
Perl_SvAMAGIC_off(SV *sv)
{
    PERL_ARGS_ASSERT_SVAMAGIC_OFF;

    if (SvROK(sv) && SvOBJECT(SvRV(sv)))
        HvAMAGIC_off(SvSTASH(SvRV(sv)));
}

PERL_STATIC_INLINE U32
Perl_SvPADSTALE_on(SV *sv)
{
    assert(!(SvFLAGS(sv) & SVs_PADTMP));
    return SvFLAGS(sv) |= SVs_PADSTALE;
}
PERL_STATIC_INLINE U32
Perl_SvPADSTALE_off(SV *sv)
{
    assert(!(SvFLAGS(sv) & SVs_PADTMP));
    return SvFLAGS(sv) &= ~SVs_PADSTALE;
}

/*
=for apidoc_section $SV
=for apidoc      SvIV
=for apidoc_item SvIV_nomg
=for apidoc_item SvIVx

These each coerce the given SV to IV and return it.  The returned value in many
circumstances will get stored in C<sv>'s IV slot, but not in all cases.  (Use
C<L</sv_setiv>> to make sure it does).

As of 5.37.1, all are guaranteed to evaluate C<sv> only once.

C<SvIVx> is now identical to C<SvIV>, but prior to 5.37.1, it was the only form
guaranteed to evaluate C<sv> only once.

C<SvIV_nomg> is the same as C<SvIV>, but does not perform 'get' magic.

=for apidoc      SvNV
=for apidoc_item SvNV_nomg
=for apidoc_item SvNVx

These each coerce the given SV to NV and return it.  The returned value in many
circumstances will get stored in C<sv>'s NV slot, but not in all cases.  (Use
C<L</sv_setnv>> to make sure it does).

As of 5.37.1, all are guaranteed to evaluate C<sv> only once.

C<SvNVx> is now identical to C<SvNV>, but prior to 5.37.1, it was the only form
guaranteed to evaluate C<sv> only once.

C<SvNV_nomg> is the same as C<SvNV>, but does not perform 'get' magic.

=for apidoc      SvUV
=for apidoc_item SvUV_nomg
=for apidoc_item SvUVx

These each coerce the given SV to UV and return it.  The returned value in many
circumstances will get stored in C<sv>'s UV slot, but not in all cases.  (Use
C<L</sv_setuv>> to make sure it does).

As of 5.37.1, all are guaranteed to evaluate C<sv> only once.

C<SvUVx> is now identical to C<SvUV>, but prior to 5.37.1, it was the only form
guaranteed to evaluate C<sv> only once.

=cut
*/

PERL_STATIC_INLINE IV
Perl_SvIV(pTHX_ SV *sv) {
    PERL_ARGS_ASSERT_SVIV;

    if (SvIOK_nog(sv))
        return SvIVX(sv);
    return sv_2iv(sv);
}

PERL_STATIC_INLINE UV
Perl_SvUV(pTHX_ SV *sv) {
    PERL_ARGS_ASSERT_SVUV;

    if (SvUOK_nog(sv))
        return SvUVX(sv);
    return sv_2uv(sv);
}

PERL_STATIC_INLINE NV
Perl_SvNV(pTHX_ SV *sv) {
    PERL_ARGS_ASSERT_SVNV;

    if (SvNOK_nog(sv))
        return SvNVX(sv);
    return sv_2nv(sv);
}

PERL_STATIC_INLINE IV
Perl_SvIV_nomg(pTHX_ SV *sv) {
    PERL_ARGS_ASSERT_SVIV_NOMG;

    if (SvIOK(sv))
        return SvIVX(sv);
    return sv_2iv_flags(sv, 0);
}

PERL_STATIC_INLINE UV
Perl_SvUV_nomg(pTHX_ SV *sv) {
    PERL_ARGS_ASSERT_SVUV_NOMG;

    if (SvIOK_nog(sv))
        return SvUVX(sv);
    return sv_2uv_flags(sv, 0);
}

PERL_STATIC_INLINE NV
Perl_SvNV_nomg(pTHX_ SV *sv) {
    PERL_ARGS_ASSERT_SVNV_NOMG;

    if (SvNOK_nog(sv))
        return SvNVX(sv);
    return sv_2nv_flags(sv, 0);
}

#if defined(PERL_CORE) || defined (PERL_EXT)
PERL_STATIC_INLINE STRLEN
S_sv_or_pv_pos_u2b(pTHX_ SV *sv, const char *pv, STRLEN pos, STRLEN *lenp)
{
    PERL_ARGS_ASSERT_SV_OR_PV_POS_U2B;
    if (SvGAMAGIC(sv)) {
        U8 *hopped = utf8_hop((U8 *)pv, pos);
        if (lenp) *lenp = (STRLEN)(utf8_hop(hopped, *lenp) - hopped);
        return (STRLEN)(hopped - (U8 *)pv);
    }
    return sv_pos_u2b_flags(sv,pos,lenp,SV_CONST_RETURN);
}
#endif

PERL_STATIC_INLINE char *
Perl_sv_pvutf8n_force_wrapper(pTHX_ SV * const sv, STRLEN * const lp, const U32 dummy)
{
    /* This is just so can be passed to Perl_SvPV_helper() as a function
     * pointer with the same signature as all the other such pointers, and
     * having hence an unused parameter */
    PERL_ARGS_ASSERT_SV_PVUTF8N_FORCE_WRAPPER;
    PERL_UNUSED_ARG(dummy);

    return sv_pvutf8n_force(sv, lp);
}

PERL_STATIC_INLINE char *
Perl_sv_pvbyten_force_wrapper(pTHX_ SV * const sv, STRLEN * const lp, const U32 dummy)
{
    /* This is just so can be passed to Perl_SvPV_helper() as a function
     * pointer with the same signature as all the other such pointers, and
     * having hence an unused parameter */
    PERL_ARGS_ASSERT_SV_PVBYTEN_FORCE_WRAPPER;
    PERL_UNUSED_ARG(dummy);

    return sv_pvbyten_force(sv, lp);
}

PERL_STATIC_INLINE char *
Perl_SvPV_helper(pTHX_
                 SV * const sv,
                 STRLEN * const lp,
                 const U32 flags,
                 const PL_SvPVtype type,
                 char * (*non_trivial)(pTHX_ SV *, STRLEN * const, const U32),
                 const bool or_null,
                 const U32 return_flags
                )
{
    /* 'type' should be known at compile time, so this is reduced to a single
     * conditional at runtime */
    if (   (type == SvPVbyte_type_      && SvPOK_byte_nog(sv))
        || (type == SvPVforce_type_     && SvPOK_pure_nogthink(sv))
        || (type == SvPVutf8_type_      && SvPOK_utf8_nog(sv))
        || (type == SvPVnormal_type_    && SvPOK_nog(sv))
        || (type == SvPVutf8_pure_type_ && SvPOK_utf8_pure_nogthink(sv))
        || (type == SvPVbyte_pure_type_ && SvPOK_byte_pure_nogthink(sv))
   ) {
        if (lp) {
            *lp = SvCUR(sv);
        }

        /* Similarly 'return_flags is known at compile time, so this becomes
         * branchless */
        if (return_flags & SV_MUTABLE_RETURN) {
            return SvPVX_mutable(sv);
        }
        else if(return_flags & SV_CONST_RETURN) {
            return (char *) SvPVX_const(sv);
        }
        else {
            return SvPVX(sv);
        }
    }

    if (or_null) {  /* This is also known at compile time */
        if (flags & SV_GMAGIC) {    /* As is this */
            SvGETMAGIC(sv);
        }

        if (! SvOK(sv)) {
            if (lp) {   /* As is this */
                *lp = 0;
            }

            return NULL;
        }
    }

    /* Can't trivially handle this, call the function */
    return non_trivial(aTHX_ sv, lp, (flags|return_flags));
}

/*
=for apidoc newRV_noinc

Creates an RV wrapper for an SV.  The reference count for the original
SV is B<not> incremented.

=cut
*/

PERL_STATIC_INLINE SV *
Perl_newRV_noinc(pTHX_ SV *const tmpRef)
{
    SV *sv = newSV_type(SVt_IV);

    PERL_ARGS_ASSERT_NEWRV_NOINC;

    SvTEMP_off(tmpRef);

    /* inlined, simplified sv_setrv_noinc(sv, tmpRef); */
    SvRV_set(sv, tmpRef);
    SvROK_on(sv);

    return sv;
}

PERL_STATIC_INLINE char *
Perl_sv_setpv_freshbuf(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_SETPV_FRESHBUF;
    assert(SvTYPE(sv) >= SVt_PV);
    assert(SvTYPE(sv) <= SVt_PVMG);
    assert(!SvTHINKFIRST(sv));
    assert(SvPVX(sv));
    SvCUR_set(sv, 0);
    *(SvEND(sv))= '\0';
    (void)SvPOK_only_UTF8(sv);
    SvTAINT(sv);
    return SvPVX(sv);
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
