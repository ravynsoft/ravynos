/*    sv.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 by Larry Wall
 *    and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'I wonder what the Entish is for "yes" and "no",' he thought.
 *                                                      --Pippin
 *
 *     [p.480 of _The Lord of the Rings_, III/iv: "Treebeard"]
 */

/*
 *
 *
 * This file contains the code that creates, manipulates and destroys
 * scalar values (SVs). The other types (AV, HV, GV, etc.) reuse the
 * structure of an SV, so their creation and destruction is handled
 * here; higher-level functions are in av.c, hv.c, and so on. Opcode
 * level functions (eg. substr, split, join) for each of the types are
 * in the pp*.c files.
 */

#include "EXTERN.h"
#define PERL_IN_SV_C
#include "perl.h"
#include "regcomp.h"
#ifdef __VMS
# include <rms.h>
#endif

#ifdef __Lynx__
/* Missing proto on LynxOS */
  char *gconvert(double, int, int,  char *);
#endif

#ifdef USE_QUADMATH
#  define SNPRINTF_G(nv, buffer, size, ndig) \
    quadmath_snprintf(buffer, size, "%.*Qg", (int)ndig, (NV)(nv))
#else
#  define SNPRINTF_G(nv, buffer, size, ndig) \
    PERL_UNUSED_RESULT(Gconvert((NV)(nv), (int)ndig, 0, buffer))
#endif

#ifndef SV_COW_THRESHOLD
#    define SV_COW_THRESHOLD                    0   /* COW iff len > K */
#endif
#ifndef SV_COWBUF_THRESHOLD
#    define SV_COWBUF_THRESHOLD                 1250 /* COW iff len > K */
#endif
#ifndef SV_COW_MAX_WASTE_THRESHOLD
#    define SV_COW_MAX_WASTE_THRESHOLD          80   /* COW iff (len - cur) < K */
#endif
#ifndef SV_COWBUF_WASTE_THRESHOLD
#    define SV_COWBUF_WASTE_THRESHOLD           80   /* COW iff (len - cur) < K */
#endif
#ifndef SV_COW_MAX_WASTE_FACTOR_THRESHOLD
#    define SV_COW_MAX_WASTE_FACTOR_THRESHOLD   2    /* COW iff len < (cur * K) */
#endif
#ifndef SV_COWBUF_WASTE_FACTOR_THRESHOLD
#    define SV_COWBUF_WASTE_FACTOR_THRESHOLD    2    /* COW iff len < (cur * K) */
#endif
/* Work around compiler warnings about unsigned >= THRESHOLD when thres-
   hold is 0. */
#if SV_COW_THRESHOLD
# define GE_COW_THRESHOLD(cur) ((cur) >= SV_COW_THRESHOLD)
#else
# define GE_COW_THRESHOLD(cur) 1
#endif
#if SV_COWBUF_THRESHOLD
# define GE_COWBUF_THRESHOLD(cur) ((cur) >= SV_COWBUF_THRESHOLD)
#else
# define GE_COWBUF_THRESHOLD(cur) 1
#endif
#if SV_COW_MAX_WASTE_THRESHOLD
# define GE_COW_MAX_WASTE_THRESHOLD(cur,len) (((len)-(cur)) < SV_COW_MAX_WASTE_THRESHOLD)
#else
# define GE_COW_MAX_WASTE_THRESHOLD(cur,len) 1
#endif
#if SV_COWBUF_WASTE_THRESHOLD
# define GE_COWBUF_WASTE_THRESHOLD(cur,len) (((len)-(cur)) < SV_COWBUF_WASTE_THRESHOLD)
#else
# define GE_COWBUF_WASTE_THRESHOLD(cur,len) 1
#endif
#if SV_COW_MAX_WASTE_FACTOR_THRESHOLD
# define GE_COW_MAX_WASTE_FACTOR_THRESHOLD(cur,len) ((len) < SV_COW_MAX_WASTE_FACTOR_THRESHOLD * (cur))
#else
# define GE_COW_MAX_WASTE_FACTOR_THRESHOLD(cur,len) 1
#endif
#if SV_COWBUF_WASTE_FACTOR_THRESHOLD
# define GE_COWBUF_WASTE_FACTOR_THRESHOLD(cur,len) ((len) < SV_COWBUF_WASTE_FACTOR_THRESHOLD * (cur))
#else
# define GE_COWBUF_WASTE_FACTOR_THRESHOLD(cur,len) 1
#endif

#define CHECK_COW_THRESHOLD(cur,len) (\
    GE_COW_THRESHOLD((cur)) && \
    GE_COW_MAX_WASTE_THRESHOLD((cur),(len)) && \
    GE_COW_MAX_WASTE_FACTOR_THRESHOLD((cur),(len)) \
)
#define CHECK_COWBUF_THRESHOLD(cur,len) (\
    GE_COWBUF_THRESHOLD((cur)) && \
    GE_COWBUF_WASTE_THRESHOLD((cur),(len)) && \
    GE_COWBUF_WASTE_FACTOR_THRESHOLD((cur),(len)) \
)

#ifdef PERL_UTF8_CACHE_ASSERT
/* if adding more checks watch out for the following tests:
 *   t/op/index.t t/op/length.t t/op/pat.t t/op/substr.t
 *   lib/utf8.t lib/Unicode/Collate/t/index.t
 * --jhi
 */
#   define ASSERT_UTF8_CACHE(cache) \
    STMT_START { if (cache) { assert((cache)[0] <= (cache)[1]); \
                              assert((cache)[2] <= (cache)[3]); \
                              assert((cache)[3] <= (cache)[1]);} \
                              } STMT_END
#else
#   define ASSERT_UTF8_CACHE(cache) NOOP
#endif

static const char S_destroy[] = "DESTROY";
#define S_destroy_len (sizeof(S_destroy)-1)

/* ============================================================================

An SV (or AV, HV, etc.) is allocated in two parts: the head (struct
sv, av, hv...) contains type and reference count information, and for
many types, a pointer to the body (struct xrv, xpv, xpviv...), which
contains fields specific to each type.  Some types store all they need
in the head, so don't have a body.

In all but the most memory-paranoid configurations (ex: PURIFY), heads
and bodies are allocated out of arenas, which by default are
approximately 4K chunks of memory parcelled up into N heads or bodies.
Sv-bodies are allocated by their sv-type, guaranteeing size
consistency needed to allocate safely from arrays.

For SV-heads, the first slot in each arena is reserved, and holds a
link to the next arena, some flags, and a note of the number of slots.
Snaked through each arena chain is a linked list of free items; when
this becomes empty, an extra arena is allocated and divided up into N
items which are threaded into the free list.

SV-bodies are similar, but they use arena-sets by default, which
separate the link and info from the arena itself, and reclaim the 1st
slot in the arena.  SV-bodies are further described later.

The following global variables are associated with arenas:

 PL_sv_arenaroot     pointer to list of SV arenas
 PL_sv_root          pointer to list of free SV structures

 PL_body_arenas      head of linked-list of body arenas
 PL_body_roots[]     array of pointers to list of free bodies of svtype
                     arrays are indexed by the svtype needed

A few special SV heads are not allocated from an arena, but are
instead directly created in the interpreter structure, eg PL_sv_undef.
The size of arenas can be changed from the default by setting
PERL_ARENA_SIZE appropriately at compile time.

The SV arena serves the secondary purpose of allowing still-live SVs
to be located and destroyed during final cleanup.

At the lowest level, the macros new_SV() and del_SV() grab and free
an SV head.  (If debugging with -DD, del_SV() calls the function S_del_sv()
to return the SV to the free list with error checking.) new_SV() calls
more_sv() / sv_add_arena() to add an extra arena if the free list is empty.
SVs in the free list have their SvTYPE field set to all ones.

At the time of very final cleanup, sv_free_arenas() is called from
perl_destruct() to physically free all the arenas allocated since the
start of the interpreter.

The internal function visit() scans the SV arenas list, and calls a specified
function for each SV it finds which is still live, I<i.e.> which has an SvTYPE
other than all 1's, and a non-zero SvREFCNT. visit() is used by the
following functions (specified as [function that calls visit()] / [function
called by visit() for each SV]):

    sv_report_used() / do_report_used()
                        dump all remaining SVs (debugging aid)

    sv_clean_objs() / do_clean_objs(),do_clean_named_objs(),
                      do_clean_named_io_objs(),do_curse()
                        Attempt to free all objects pointed to by RVs,
                        try to do the same for all objects indir-
                        ectly referenced by typeglobs too, and
                        then do a final sweep, cursing any
                        objects that remain.  Called once from
                        perl_destruct(), prior to calling sv_clean_all()
                        below.

    sv_clean_all() / do_clean_all()
                        SvREFCNT_dec(sv) each remaining SV, possibly
                        triggering an sv_free(). It also sets the
                        SVf_BREAK flag on the SV to indicate that the
                        refcnt has been artificially lowered, and thus
                        stopping sv_free() from giving spurious warnings
                        about SVs which unexpectedly have a refcnt
                        of zero.  called repeatedly from perl_destruct()
                        until there are no SVs left.

=head2 Arena allocator API Summary

Private API to rest of sv.c

    new_SV(),  del_SV(),

    new_XPVNV(), del_body()
    etc

Public API:

    sv_report_used(), sv_clean_objs(), sv_clean_all(), sv_free_arenas()

=cut

 * ========================================================================= */

/*
 * "A time to plant, and a time to uproot what was planted..."
 */

#ifdef DEBUG_LEAKING_SCALARS
#  define FREE_SV_DEBUG_FILE(sv) STMT_START { \
        if ((sv)->sv_debug_file) {                   \
            PerlMemShared_free((sv)->sv_debug_file); \
            sv->sv_debug_file = NULL;                \
        }                                            \
    } STMT_END
#  define DEBUG_SV_SERIAL(sv)						    \
    DEBUG_m(PerlIO_printf(Perl_debug_log, "0x%" UVxf ": (%05ld) del_SV\n",    \
            PTR2UV(sv), (long)(sv)->sv_debug_serial))
#else
#  define FREE_SV_DEBUG_FILE(sv)
#  define DEBUG_SV_SERIAL(sv)	NOOP
#endif

/* Mark an SV head as unused, and add to free list.
 *
 * If SVf_BREAK is set, skip adding it to the free list, as this SV had
 * its refcount artificially decremented during global destruction, so
 * there may be dangling pointers to it. The last thing we want in that
 * case is for it to be reused. */

#define plant_SV(p) \
    STMT_START {					\
        const U32 old_flags = SvFLAGS(p);			\
        MEM_LOG_DEL_SV(p, __FILE__, __LINE__, FUNCTION__);  \
        DEBUG_SV_SERIAL(p);				\
        FREE_SV_DEBUG_FILE(p);				\
        POISON_SV_HEAD(p);				\
        SvFLAGS(p) = SVTYPEMASK;			\
        if (!(old_flags & SVf_BREAK)) {		\
            SvARENA_CHAIN_SET(p, PL_sv_root);	\
            PL_sv_root = (p);				\
        }						\
        --PL_sv_count;					\
    } STMT_END


/* make some more SVs by adding another arena */

SV*
Perl_more_sv(pTHX)
{
    SV* sv;
    char *chunk;                /* must use New here to match call to */
    Newx(chunk,PERL_ARENA_SIZE,char);  /* Safefree() in sv_free_arenas() */
    sv_add_arena(chunk, PERL_ARENA_SIZE, 0);
    uproot_SV(sv);
    return sv;
}

/* del_SV(): return an empty SV head to the free list */

#ifdef DEBUGGING

#define del_SV(p) \
    STMT_START {					\
        if (DEBUG_D_TEST)				\
            del_sv(p);					\
        else						\
            plant_SV(p);				\
    } STMT_END

STATIC void
S_del_sv(pTHX_ SV *p)
{
    PERL_ARGS_ASSERT_DEL_SV;

    if (DEBUG_D_TEST) {
        SV* sva;
        bool ok = 0;
        for (sva = PL_sv_arenaroot; sva; sva = MUTABLE_SV(SvANY(sva))) {
            const SV * const sv = sva + 1;
            const SV * const svend = &sva[SvREFCNT(sva)];
            if (p >= sv && p < svend) {
                ok = 1;
                break;
            }
        }
        if (!ok) {
            Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                             "Attempt to free non-arena SV: 0x%" UVxf
                             pTHX__FORMAT, PTR2UV(p) pTHX__VALUE);
            return;
        }
    }
    plant_SV(p);
}

#else /* ! DEBUGGING */

#define del_SV(p)   plant_SV(p)

#endif /* DEBUGGING */


/*
=for apidoc_section $SV

=for apidoc sv_add_arena

Given a chunk of memory, link it to the head of the list of arenas,
and split it into a list of free SVs.

=cut
*/

static void
S_sv_add_arena(pTHX_ char *const ptr, const U32 size, const U32 flags)
{
    SV *const sva = MUTABLE_SV(ptr);
    SV* sv;
    SV* svend;

    PERL_ARGS_ASSERT_SV_ADD_ARENA;

    /* The first SV in an arena isn't an SV. */
    SvANY(sva) = (void *) PL_sv_arenaroot;		/* ptr to next arena */
    SvREFCNT(sva) = size / sizeof(SV);		/* number of SV slots */
    SvFLAGS(sva) = flags;			/* FAKE if not to be freed */

    PL_sv_arenaroot = sva;
    PL_sv_root = sva + 1;

    svend = &sva[SvREFCNT(sva) - 1];
    sv = sva + 1;
    while (sv < svend) {
        SvARENA_CHAIN_SET(sv, (sv + 1));
#ifdef DEBUGGING
        SvREFCNT(sv) = 0;
#endif
        /* Must always set typemask because it's always checked in on cleanup
           when the arenas are walked looking for objects.  */
        SvFLAGS(sv) = SVTYPEMASK;
        sv++;
    }
    SvARENA_CHAIN_SET(sv, 0);
#ifdef DEBUGGING
    SvREFCNT(sv) = 0;
#endif
    SvFLAGS(sv) = SVTYPEMASK;
}

/* visit(): call the named function for each non-free SV in the arenas
 * whose flags field matches the flags/mask args. */

STATIC I32
S_visit(pTHX_ SVFUNC_t f, const U32 flags, const U32 mask)
{
    SV* sva;
    I32 visited = 0;

    PERL_ARGS_ASSERT_VISIT;

    for (sva = PL_sv_arenaroot; sva; sva = MUTABLE_SV(SvANY(sva))) {
        const SV * const svend = &sva[SvREFCNT(sva)];
        SV* sv;
        for (sv = sva + 1; sv < svend; ++sv) {
            if (!SvIS_FREED(sv)
                    && (sv->sv_flags & mask) == flags
                    && SvREFCNT(sv))
            {
                (*f)(aTHX_ sv);
                ++visited;
            }
        }
    }
    return visited;
}

#ifdef DEBUGGING

/* called by sv_report_used() for each live SV */

static void
do_report_used(pTHX_ SV *const sv)
{
    if (!SvIS_FREED(sv)) {
        PerlIO_printf(Perl_debug_log, "****\n");
        sv_dump(sv);
    }
}
#endif

/*
=for apidoc sv_report_used

Dump the contents of all SVs not yet freed (debugging aid).

=cut
*/

void
Perl_sv_report_used(pTHX)
{
#ifdef DEBUGGING
    visit(do_report_used, 0, 0);
#else
    PERL_UNUSED_CONTEXT;
#endif
}

/* called by sv_clean_objs() for each live SV */

static void
do_clean_objs(pTHX_ SV *const ref)
{
    assert (SvROK(ref));
    {
        SV * const target = SvRV(ref);
        if (SvOBJECT(target)) {
            DEBUG_D((PerlIO_printf(Perl_debug_log, "Cleaning object ref:\n "), sv_dump(ref)));
            if (SvWEAKREF(ref)) {
                sv_del_backref(target, ref);
                SvWEAKREF_off(ref);
                SvRV_set(ref, NULL);
            } else {
                SvROK_off(ref);
                SvRV_set(ref, NULL);
                SvREFCNT_dec_NN(target);
            }
        }
    }
}


/* clear any slots in a GV which hold objects - except IO;
 * called by sv_clean_objs() for each live GV */

static void
do_clean_named_objs(pTHX_ SV *const sv)
{
    SV *obj;
    assert(SvTYPE(sv) == SVt_PVGV);
    assert(isGV_with_GP(sv));
    if (!GvGP(sv))
        return;

    /* freeing GP entries may indirectly free the current GV;
     * hold onto it while we mess with the GP slots */
    SvREFCNT_inc(sv);

    if ( ((obj = GvSV(sv) )) && SvOBJECT(obj)) {
        DEBUG_D((PerlIO_printf(Perl_debug_log,
                "Cleaning named glob SV object:\n "), sv_dump(obj)));
        GvSV(sv) = NULL;
        SvREFCNT_dec_NN(obj);
    }
    if ( ((obj = MUTABLE_SV(GvAV(sv)) )) && SvOBJECT(obj)) {
        DEBUG_D((PerlIO_printf(Perl_debug_log,
                "Cleaning named glob AV object:\n "), sv_dump(obj)));
        GvAV(sv) = NULL;
        SvREFCNT_dec_NN(obj);
    }
    if ( ((obj = MUTABLE_SV(GvHV(sv)) )) && SvOBJECT(obj)) {
        DEBUG_D((PerlIO_printf(Perl_debug_log,
                "Cleaning named glob HV object:\n "), sv_dump(obj)));
        GvHV(sv) = NULL;
        SvREFCNT_dec_NN(obj);
    }
    if ( ((obj = MUTABLE_SV(GvCV(sv)) )) && SvOBJECT(obj)) {
        DEBUG_D((PerlIO_printf(Perl_debug_log,
                "Cleaning named glob CV object:\n "), sv_dump(obj)));
        GvCV_set(sv, NULL);
        SvREFCNT_dec_NN(obj);
    }
    SvREFCNT_dec_NN(sv); /* undo the inc above */
}

/* clear any IO slots in a GV which hold objects (except stderr, defout);
 * called by sv_clean_objs() for each live GV */

static void
do_clean_named_io_objs(pTHX_ SV *const sv)
{
    SV *obj;
    assert(SvTYPE(sv) == SVt_PVGV);
    assert(isGV_with_GP(sv));
    if (!GvGP(sv) || sv == (SV*)PL_stderrgv || sv == (SV*)PL_defoutgv)
        return;

    SvREFCNT_inc(sv);
    if ( ((obj = MUTABLE_SV(GvIO(sv)) )) && SvOBJECT(obj)) {
        DEBUG_D((PerlIO_printf(Perl_debug_log,
                "Cleaning named glob IO object:\n "), sv_dump(obj)));
        GvIOp(sv) = NULL;
        SvREFCNT_dec_NN(obj);
    }
    SvREFCNT_dec_NN(sv); /* undo the inc above */
}

/* Void wrapper to pass to visit() */
static void
do_curse(pTHX_ SV * const sv) {
    if ((PL_stderrgv && GvGP(PL_stderrgv) && (SV*)GvIO(PL_stderrgv) == sv)
     || (PL_defoutgv && GvGP(PL_defoutgv) && (SV*)GvIO(PL_defoutgv) == sv))
        return;
    (void)curse(sv, 0);
}

/*
=for apidoc sv_clean_objs

Attempt to destroy all objects not yet freed.

=cut
*/

void
Perl_sv_clean_objs(pTHX)
{
    GV *olddef, *olderr;
    PL_in_clean_objs = TRUE;
    visit(do_clean_objs, SVf_ROK, SVf_ROK);
    /* Some barnacles may yet remain, clinging to typeglobs.
     * Run the non-IO destructors first: they may want to output
     * error messages, close files etc */
    visit(do_clean_named_objs, SVt_PVGV|SVpgv_GP, SVTYPEMASK|SVp_POK|SVpgv_GP);
    visit(do_clean_named_io_objs, SVt_PVGV|SVpgv_GP, SVTYPEMASK|SVp_POK|SVpgv_GP);
    /* And if there are some very tenacious barnacles clinging to arrays,
       closures, or what have you.... */
    visit(do_curse, SVs_OBJECT, SVs_OBJECT);
    olddef = PL_defoutgv;
    PL_defoutgv = NULL; /* disable skip of PL_defoutgv */
    if (olddef && isGV_with_GP(olddef))
        do_clean_named_io_objs(aTHX_ MUTABLE_SV(olddef));
    olderr = PL_stderrgv;
    PL_stderrgv = NULL; /* disable skip of PL_stderrgv */
    if (olderr && isGV_with_GP(olderr))
        do_clean_named_io_objs(aTHX_ MUTABLE_SV(olderr));
    SvREFCNT_dec(olddef);
    PL_in_clean_objs = FALSE;
}

/* called by sv_clean_all() for each live SV */

static void
do_clean_all(pTHX_ SV *const sv)
{
    if (sv == (const SV *) PL_fdpid || sv == (const SV *)PL_strtab) {
        /* don't clean pid table and strtab */
        return;
    }
    DEBUG_D((PerlIO_printf(Perl_debug_log, "Cleaning loops: SV at 0x%" UVxf "\n", PTR2UV(sv)) ));
    SvFLAGS(sv) |= SVf_BREAK;
    SvREFCNT_dec_NN(sv);
}

/*
=for apidoc sv_clean_all

Decrement the refcnt of each remaining SV, possibly triggering a
cleanup.  This function may have to be called multiple times to free
SVs which are in complex self-referential hierarchies.

=cut
*/

I32
Perl_sv_clean_all(pTHX)
{
    I32 cleaned;
    PL_in_clean_all = TRUE;
    cleaned = visit(do_clean_all, 0,0);
    return cleaned;
}

/*
  ARENASETS: a meta-arena implementation which separates arena-info
  into struct arena_set, which contains an array of struct
  arena_descs, each holding info for a single arena.  By separating
  the meta-info from the arena, we recover the 1st slot, formerly
  borrowed for list management.  The arena_set is about the size of an
  arena, avoiding the needless malloc overhead of a naive linked-list.

  The cost is 1 arena-set malloc per ~320 arena-mallocs, + the unused
  memory in the last arena-set (1/2 on average).  In trade, we get
  back the 1st slot in each arena (ie 1.7% of a CV-arena, less for
  smaller types).  The recovery of the wasted space allows use of
  small arenas for large, rare body types, by changing array* fields
  in body_details_by_type[] below.
*/
struct arena_desc {
    char       *arena;		/* the raw storage, allocated aligned */
    size_t      size;		/* its size ~4k typ */
    svtype	utype;		/* bodytype stored in arena */
};

struct arena_set;

/* Get the maximum number of elements in set[] such that struct arena_set
   will fit within PERL_ARENA_SIZE, which is probably just under 4K, and
   therefore likely to be 1 aligned memory page.  */

#define ARENAS_PER_SET  ((PERL_ARENA_SIZE - sizeof(struct arena_set*) \
                          - 2 * sizeof(int)) / sizeof (struct arena_desc))

struct arena_set {
    struct arena_set* next;
    unsigned int   set_size;	/* ie ARENAS_PER_SET */
    unsigned int   curr;	/* index of next available arena-desc */
    struct arena_desc set[ARENAS_PER_SET];
};

/*
=for apidoc sv_free_arenas

Deallocate the memory used by all arenas.  Note that all the individual SV
heads and bodies within the arenas must already have been freed.

=cut

*/
void
Perl_sv_free_arenas(pTHX)
{
    SV* sva;
    SV* svanext;
    unsigned int i;

    /* Free arenas here, but be careful about fake ones.  (We assume
       contiguity of the fake ones with the corresponding real ones.) */

    for (sva = PL_sv_arenaroot; sva; sva = svanext) {
        svanext = MUTABLE_SV(SvANY(sva));
        while (svanext && SvFAKE(svanext))
            svanext = MUTABLE_SV(SvANY(svanext));

        if (!SvFAKE(sva))
            Safefree(sva);
    }

    {
        struct arena_set *aroot = (struct arena_set*) PL_body_arenas;

        while (aroot) {
            struct arena_set *current = aroot;
            i = aroot->curr;
            while (i--) {
                assert(aroot->set[i].arena);
                Safefree(aroot->set[i].arena);
            }
            aroot = aroot->next;
            Safefree(current);
        }
    }
    PL_body_arenas = 0;

    i = PERL_ARENA_ROOTS_SIZE;
    while (i--)
        PL_body_roots[i] = 0;

    PL_sv_arenaroot = 0;
    PL_sv_root = 0;
}

/*
  Historically, here were mid-level routines that manage the
  allocation of bodies out of the various arenas. Some of these
  routines and related definitions remain here, but others were
  moved into sv_inline.h to facilitate inlining of newSV_type().

  There are 4 kinds of arenas:

  1. SV-head arenas, which are discussed and handled above
  2. regular body arenas
  3. arenas for reduced-size bodies
  4. Hash-Entry arenas

  Arena types 2 & 3 are chained by body-type off an array of
  arena-root pointers, which is indexed by svtype.  Some of the
  larger/less used body types are malloced singly, since a large
  unused block of them is wasteful.  Also, several svtypes don't have
  bodies; the data fits into the sv-head itself.  The arena-root
  pointer thus has a few unused root-pointers (which may be hijacked
  later for arena type 4)

  3 differs from 2 as an optimization; some body types have several
  unused fields in the front of the structure (which are kept in-place
  for consistency).  These bodies can be allocated in smaller chunks,
  because the leading fields arent accessed.  Pointers to such bodies
  are decremented to point at the unused 'ghost' memory, knowing that
  the pointers are used with offsets to the real memory.

Allocation of SV-bodies is similar to SV-heads, differing as follows;
the allocation mechanism is used for many body types, so is somewhat
more complicated, it uses arena-sets, and has no need for still-live
SV detection.

At the outermost level, (new|del)_X*V macros return bodies of the
appropriate type.  These macros call either (new|del)_body_type or
(new|del)_body_allocated macro pairs, depending on specifics of the
type.  Most body types use the former pair, the latter pair is used to
allocate body types with "ghost fields".

"ghost fields" are fields that are unused in certain types, and
consequently don't need to actually exist.  They are declared because
they're part of a "base type", which allows use of functions as
methods.  The simplest examples are AVs and HVs, 2 aggregate types
which don't use the fields which support SCALAR semantics.

For these types, the arenas are carved up into appropriately sized
chunks, we thus avoid wasted memory for those unaccessed members.
When bodies are allocated, we adjust the pointer back in memory by the
size of the part not allocated, so it's as if we allocated the full
structure.  (But things will all go boom if you write to the part that
is "not there", because you'll be overwriting the last members of the
preceding structure in memory.)

We calculate the correction using the STRUCT_OFFSET macro on the first
member present.  If the allocated structure is smaller (no initial NV
actually allocated) then the net effect is to subtract the size of the NV
from the pointer, to return a new pointer as if an initial NV were actually
allocated.  (We were using structures named *_allocated for this, but
this turned out to be a subtle bug, because a structure without an NV
could have a lower alignment constraint, but the compiler is allowed to
optimised accesses based on the alignment constraint of the actual pointer
to the full structure, for example, using a single 64 bit load instruction
because it "knows" that two adjacent 32 bit members will be 8-byte aligned.)

This is the same trick as was used for NV and IV bodies.  Ironically it
doesn't need to be used for NV bodies any more, because NV is now at
the start of the structure.  IV bodies, and also in some builds NV bodies,
don't need it either, because they are no longer allocated.

In turn, the new_body_* allocators call S_new_body(), which invokes
new_body_from_arena macro, which takes a lock, and takes a body off the
linked list at PL_body_roots[sv_type], calling Perl_more_bodies() if
necessary to refresh an empty list.  Then the lock is released, and
the body is returned.

Perl_more_bodies allocates a new arena, and carves it up into an array of N
bodies, which it strings into a linked list.  It looks up arena-size
and body-size from the body_details table described below, thus
supporting the multiple body-types.

If PURIFY is defined, or PERL_ARENA_SIZE=0, arenas are not used, and
the (new|del)_X*V macros are mapped directly to malloc/free.

For each sv-type, struct body_details bodies_by_type[] carries
parameters which control these aspects of SV handling:

Arena_size determines whether arenas are used for this body type, and if
so, how big they are.  PURIFY or PERL_ARENA_SIZE=0 set this field to
zero, forcing individual mallocs and frees.

Body_size determines how big a body is, and therefore how many fit into
each arena.  Offset carries the body-pointer adjustment needed for
"ghost fields", and is used in *_allocated macros.

But its main purpose is to parameterize info needed in
Perl_sv_upgrade().  The info here dramatically simplifies the function
vs the implementation in 5.8.8, making it table-driven.  All fields
are used for this, except for arena_size.

For the sv-types that have no bodies, arenas are not used, so those
PL_body_roots[sv_type] are unused, and can be overloaded.  In
something of a special case, SVt_NULL is borrowed for HE arenas;
PL_body_roots[HE_ARENA_ROOT_IX=SVt_NULL] is filled by S_more_he, but the
bodies_by_type[SVt_NULL] slot is not used, as the table is not
available in hv.c. Similarly SVt_IV is re-used for HVAUX_ARENA_ROOT_IX.

*/

/* return a thing to the free list */

#define del_body(thing, root)				\
    STMT_START {					\
        void ** const thing_copy = (void **)thing;	\
        *thing_copy = *root;				\
        *root = (void*)thing_copy;			\
    } STMT_END


void *
Perl_more_bodies (pTHX_ const svtype sv_type, const size_t body_size,
                  const size_t arena_size)
{
    void ** const root = &PL_body_roots[sv_type];
    struct arena_desc *adesc;
    struct arena_set *aroot = (struct arena_set *) PL_body_arenas;
    unsigned int curr;
    char *start;
    const char *end;
    const size_t good_arena_size = Perl_malloc_good_size(arena_size);
#if defined(DEBUGGING)
    static bool done_sanity_check;

    if (!done_sanity_check) {
        unsigned int i = SVt_LAST;

        done_sanity_check = TRUE;

        while (i--)
            assert (bodies_by_type[i].type == i);
    }
#endif

    assert(arena_size);

    /* may need new arena-set to hold new arena */
    if (!aroot || aroot->curr >= aroot->set_size) {
        struct arena_set *newroot;
        Newxz(newroot, 1, struct arena_set);
        newroot->set_size = ARENAS_PER_SET;
        newroot->next = aroot;
        aroot = newroot;
        PL_body_arenas = (void *) newroot;
        DEBUG_m(PerlIO_printf(Perl_debug_log, "new arenaset %p\n", (void*)aroot));
    }

    /* ok, now have arena-set with at least 1 empty/available arena-desc */
    curr = aroot->curr++;
    adesc = &(aroot->set[curr]);
    assert(!adesc->arena);

    Newx(adesc->arena, good_arena_size, char);
    adesc->size = good_arena_size;
    adesc->utype = sv_type;
    DEBUG_m(PerlIO_printf(Perl_debug_log, "arena %d added: %p size %" UVuf "\n",
                          curr, (void*)adesc->arena, (UV)good_arena_size));

    start = (char *) adesc->arena;

    /* Get the address of the byte after the end of the last body we can fit.
       Remember, this is integer division:  */
    end = start + good_arena_size / body_size * body_size;

    /* computed count doesn't reflect the 1st slot reservation */
#if defined(MYMALLOC) || defined(HAS_MALLOC_GOOD_SIZE)
    DEBUG_m(PerlIO_printf(Perl_debug_log,
                          "arena %p end %p arena-size %d (from %d) type %d "
                          "size %d ct %d\n",
                          (void*)start, (void*)end, (int)good_arena_size,
                          (int)arena_size, sv_type, (int)body_size,
                          (int)good_arena_size / (int)body_size));
#else
    DEBUG_m(PerlIO_printf(Perl_debug_log,
                          "arena %p end %p arena-size %d type %d size %d ct %d\n",
                          (void*)start, (void*)end,
                          (int)arena_size, sv_type, (int)body_size,
                          (int)good_arena_size / (int)body_size));
#endif
    *root = (void *)start;

    while (1) {
        /* Where the next body would start:  */
        char * const next = start + body_size;

        if (next >= end) {
            /* This is the last body:  */
            assert(next == end);

            *(void **)start = 0;
            return *root;
        }

        *(void**) start = (void *)next;
        start = next;
    }
}

/*
=for apidoc sv_upgrade

Upgrade an SV to a more complex form.  Generally adds a new body type to the
SV, then copies across as much information as possible from the old body.
It croaks if the SV is already in a more complex form than requested.  You
generally want to use the C<SvUPGRADE> macro wrapper, which checks the type
before calling C<sv_upgrade>, and hence does not croak.  See also
C<L</svtype>>.

=cut
*/

void
Perl_sv_upgrade(pTHX_ SV *const sv, svtype new_type)
{
    void*	old_body;
    void*	new_body;
    const svtype old_type = SvTYPE(sv);
    const struct body_details *new_type_details;
    const struct body_details *old_type_details
        = bodies_by_type + old_type;
    SV *referent = NULL;

    PERL_ARGS_ASSERT_SV_UPGRADE;

    if (old_type == new_type)
        return;

    /* This clause was purposefully added ahead of the early return above to
       the shared string hackery for (sort {$a <=> $b} keys %hash), with the
       inference by Nick I-S that it would fix other troublesome cases. See
       changes 7162, 7163 (f130fd4589cf5fbb24149cd4db4137c8326f49c1 and parent)

       Given that shared hash key scalars are no longer PVIV, but PV, there is
       no longer need to unshare so as to free up the IVX slot for its proper
       purpose. So it's safe to move the early return earlier.  */

    if (new_type > SVt_PVMG && SvIsCOW(sv)) {
        sv_force_normal_flags(sv, 0);
    }

    old_body = SvANY(sv);

    /* Copying structures onto other structures that have been neatly zeroed
       has a subtle gotcha. Consider XPVMG

       +------+------+------+------+------+-------+-------+
       |     NV      | CUR  | LEN  |  IV  | MAGIC | STASH |
       +------+------+------+------+------+-------+-------+
       0      4      8     12     16     20      24      28

       where NVs are aligned to 8 bytes, so that sizeof that structure is
       actually 32 bytes long, with 4 bytes of padding at the end:

       +------+------+------+------+------+-------+-------+------+
       |     NV      | CUR  | LEN  |  IV  | MAGIC | STASH | ???  |
       +------+------+------+------+------+-------+-------+------+
       0      4      8     12     16     20      24      28     32

       so what happens if you allocate memory for this structure:

       +------+------+------+------+------+-------+-------+------+------+...
       |     NV      | CUR  | LEN  |  IV  | MAGIC | STASH |  GP  | NAME |
       +------+------+------+------+------+-------+-------+------+------+...
       0      4      8     12     16     20      24      28     32     36

       zero it, then copy sizeof(XPVMG) bytes on top of it? Not quite what you
       expect, because you copy the area marked ??? onto GP. Now, ??? may have
       started out as zero once, but it's quite possible that it isn't. So now,
       rather than a nicely zeroed GP, you have it pointing somewhere random.
       Bugs ensue.

       (In fact, GP ends up pointing at a previous GP structure, because the
       principle cause of the padding in XPVMG getting garbage is a copy of
       sizeof(XPVMG) bytes from a XPVGV structure in sv_unglob. Right now
       this happens to be moot because XPVGV has been re-ordered, with GP
       no longer after STASH)

       So we are careful and work out the size of used parts of all the
       structures.  */

    switch (old_type) {
    case SVt_NULL:
        break;
    case SVt_IV:
        if (SvROK(sv)) {
            referent = SvRV(sv);
            old_type_details = &fake_rv;
            if (new_type == SVt_NV)
                new_type = SVt_PVNV;
        } else {
            if (new_type < SVt_PVIV) {
                new_type = (new_type == SVt_NV)
                    ? SVt_PVNV : SVt_PVIV;
            }
        }
        break;
    case SVt_NV:
        if (new_type < SVt_PVNV) {
            new_type = SVt_PVNV;
        }
        break;
    case SVt_PV:
        assert(new_type > SVt_PV);
        STATIC_ASSERT_STMT(SVt_IV < SVt_PV);
        STATIC_ASSERT_STMT(SVt_NV < SVt_PV);
        break;
    case SVt_PVIV:
        break;
    case SVt_PVNV:
        break;
    case SVt_PVMG:
        /* Because the XPVMG of PL_mess_sv isn't allocated from the arena,
           there's no way that it can be safely upgraded, because perl.c
           expects to Safefree(SvANY(PL_mess_sv))  */
        assert(sv != PL_mess_sv);
        break;
    default:
        if (UNLIKELY(old_type_details->cant_upgrade))
            Perl_croak(aTHX_ "Can't upgrade %s (%" UVuf ") to %" UVuf,
                       sv_reftype(sv, 0), (UV) old_type, (UV) new_type);
    }

    if (UNLIKELY(old_type > new_type))
        Perl_croak(aTHX_ "sv_upgrade from type %d down to type %d",
                (int)old_type, (int)new_type);

    new_type_details = bodies_by_type + new_type;

    SvFLAGS(sv) &= ~SVTYPEMASK;
    SvFLAGS(sv) |= new_type;

    /* This can't happen, as SVt_NULL is <= all values of new_type, so one of
       the return statements above will have triggered.  */
    assert (new_type != SVt_NULL);
    switch (new_type) {
    case SVt_IV:
        assert(old_type == SVt_NULL);
        SET_SVANY_FOR_BODYLESS_IV(sv);
        SvIV_set(sv, 0);
        return;
    case SVt_NV:
        assert(old_type == SVt_NULL);
#if NVSIZE <= IVSIZE
        SET_SVANY_FOR_BODYLESS_NV(sv);
#else
        SvANY(sv) = new_XNV();
#endif
        SvNV_set(sv, 0);
        return;
    case SVt_PVHV:
    case SVt_PVAV:
    case SVt_PVOBJ:
        assert(new_type_details->body_size);

#ifndef PURIFY
        assert(new_type_details->arena);
        assert(new_type_details->arena_size);
        /* This points to the start of the allocated area.  */
        new_body = S_new_body(aTHX_ new_type);
        /* xpvav and xpvhv have no offset, so no need to adjust new_body */
        assert(!(new_type_details->offset));
#else
        /* We always allocated the full length item with PURIFY. To do this
           we fake things so that arena is false for all 16 types..  */
        new_body = new_NOARENAZ(new_type_details);
#endif
        SvANY(sv) = new_body;
        switch(new_type) {
        case SVt_PVAV:
            {
                XPVAV pvav = {
                    .xmg_stash = NULL,
                    .xmg_u = {.xmg_magic = NULL},
                    .xav_fill = -1, .xav_max = -1, .xav_alloc = 0
                };
                *((XPVAV*) SvANY(sv)) = pvav;
            }

            AvREAL_only(sv);
            break;
        case SVt_PVHV:
            {
                XPVHV pvhv = {
                    .xmg_stash = NULL,
                    .xmg_u = {.xmg_magic = NULL},
                    .xhv_keys = 0,
                    /* start with PERL_HASH_DEFAULT_HvMAX+1 buckets: */
                    .xhv_max = PERL_HASH_DEFAULT_HvMAX
                };
                *((XPVHV*) SvANY(sv)) = pvhv;
            }

            assert(!SvOK(sv));
            SvOK_off(sv);
#ifndef NODEFAULT_SHAREKEYS
            HvSHAREKEYS_on(sv);         /* key-sharing on by default */
#endif
            break;
        case SVt_PVOBJ:
            {
                XPVOBJ pvo = {
                    .xmg_stash = NULL, .xmg_u = {.xmg_magic = NULL},
                    .xobject_maxfield = -1,
                    .xobject_iter_sv_at = 0,
                    .xobject_fields = NULL,
                };
                *((XPVOBJ*) SvANY(sv)) = pvo;
            }
            break;
        default:
            NOT_REACHED;
        }

        /* SVt_NULL isn't the only thing upgraded to AV or HV.
           The target created by newSVrv also is, and it can have magic.
           However, it never has SvPVX set.
        */
        if (old_type == SVt_IV) {
            assert(!SvROK(sv));
        } else if (old_type >= SVt_PV) {
            assert(SvPVX_const(sv) == 0);
        }

        if (old_type >= SVt_PVMG) {
            SvMAGIC_set(sv, ((XPVMG*)old_body)->xmg_u.xmg_magic);
            SvSTASH_set(sv, ((XPVMG*)old_body)->xmg_stash);
        } else {
            sv->sv_u.svu_array = NULL; /* or svu_hash  */
        }
        break;

    case SVt_PVIV:
        /* XXX Is this still needed?  Was it ever needed?   Surely as there is
           no route from NV to PVIV, NOK can never be true  */
        assert(!SvNOKp(sv));
        assert(!SvNOK(sv));
        /* FALLTHROUGH */
    case SVt_PVIO:
    case SVt_PVFM:
    case SVt_PVGV:
    case SVt_PVCV:
    case SVt_PVLV:
    case SVt_INVLIST:
    case SVt_REGEXP:
    case SVt_PVMG:
    case SVt_PVNV:
    case SVt_PV:

        assert(new_type_details->body_size);
        /* We always allocated the full length item with PURIFY. To do this
           we fake things so that arena is false for all 16 types..  */
#ifndef PURIFY
        if(new_type_details->arena) {
            /* This points to the start of the allocated area.  */
            new_body = S_new_body(aTHX_ new_type);
            Zero(new_body, new_type_details->body_size, char);
            new_body = ((char *)new_body) - new_type_details->offset;
        } else
#endif
        {
            new_body = new_NOARENAZ(new_type_details);
        }
        SvANY(sv) = new_body;

        if (old_type_details->copy) {
            /* There is now the potential for an upgrade from something without
               an offset (PVNV or PVMG) to something with one (PVCV, PVFM)  */
            int offset = old_type_details->offset;
            int length = old_type_details->copy;

            if (new_type_details->offset > old_type_details->offset) {
                const int difference
                    = new_type_details->offset - old_type_details->offset;
                offset += difference;
                length -= difference;
            }
            assert (length >= 0);

            Copy((char *)old_body + offset, (char *)new_body + offset, length,
                 char);
        }

#ifndef NV_ZERO_IS_ALLBITS_ZERO
        /* If NV 0.0 is stores as all bits 0 then Zero() already creates a
         * correct 0.0 for us.  Otherwise, if the old body didn't have an
         * NV slot, but the new one does, then we need to initialise the
         * freshly created NV slot with whatever the correct bit pattern is
         * for 0.0  */
        if (old_type_details->zero_nv && !new_type_details->zero_nv
            && !isGV_with_GP(sv))
            SvNV_set(sv, 0);
#endif

        if (UNLIKELY(new_type == SVt_PVIO)) {
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
        if (old_type < SVt_PV) {
            /* referent will be NULL unless the old type was SVt_IV emulating
               SVt_RV */
            sv->sv_u.svu_rv = referent;
        }
        break;
    default:
        Perl_croak(aTHX_ "panic: sv_upgrade to unknown type %lu",
                   (unsigned long)new_type);
    }

    /* if this is zero, this is a body-less SVt_NULL, SVt_IV/SVt_RV,
       and sometimes SVt_NV */
    if (old_type_details->body_size) {
#ifdef PURIFY
        safefree(old_body);
#else
        /* Note that there is an assumption that all bodies of types that
           can be upgraded came from arenas. Only the more complex non-
           upgradable types are allowed to be directly malloc()ed.  */
        assert(old_type_details->arena);
        del_body((void*)((char*)old_body + old_type_details->offset),
                 &PL_body_roots[old_type]);
#endif
    }
}

struct xpvhv_aux*
Perl_hv_auxalloc(pTHX_ HV *hv) {
    const struct body_details *old_type_details = bodies_by_type + SVt_PVHV;
    void *old_body;
    void *new_body;

    PERL_ARGS_ASSERT_HV_AUXALLOC;
    assert(SvTYPE(hv) == SVt_PVHV);
    assert(!HvHasAUX(hv));

#ifdef PURIFY
    new_body = new_NOARENAZ(&fake_hv_with_aux);
#else
    new_body_from_arena(new_body, HVAUX_ARENA_ROOT_IX, fake_hv_with_aux);
#endif

    old_body = SvANY(hv);

    Copy((char *)old_body + old_type_details->offset,
         (char *)new_body + fake_hv_with_aux.offset,
         old_type_details->copy,
         char);

#ifdef PURIFY
    safefree(old_body);
#else
    assert(old_type_details->arena);
    del_body((void*)((char*)old_body + old_type_details->offset),
             &PL_body_roots[SVt_PVHV]);
#endif

    SvANY(hv) = (XPVHV *) new_body;
    SvFLAGS(hv) |= SVphv_HasAUX;
    return HvAUX(hv);
}

/*
=for apidoc sv_backoff

Remove any string offset.  You should normally use the C<SvOOK_off> macro
wrapper instead.

=cut
*/

/* prior to 5.000 stable, this function returned the new OOK-less SvFLAGS
   prior to 5.23.4 this function always returned 0
*/

void
Perl_sv_backoff(SV *const sv)
{
    STRLEN delta;
    const char * const s = SvPVX_const(sv);

    PERL_ARGS_ASSERT_SV_BACKOFF;

    assert(SvOOK(sv));
    assert(SvTYPE(sv) != SVt_PVHV);
    assert(SvTYPE(sv) != SVt_PVAV);

    SvOOK_offset(sv, delta);

    SvLEN_set(sv, SvLEN(sv) + delta);
    SvPV_set(sv, SvPVX(sv) - delta);
    SvFLAGS(sv) &= ~SVf_OOK;
    Move(s, SvPVX(sv), SvCUR(sv)+1, char);
    return;
}


/* forward declaration */
static void S_sv_uncow(pTHX_ SV * const sv, const U32 flags);


/*
=for apidoc sv_grow

Expands the character buffer in the SV.  If necessary, uses C<sv_unref> and
upgrades the SV to C<SVt_PV>.  Returns a pointer to the character buffer.
Use the C<SvGROW> wrapper instead.

=cut
*/


char *
Perl_sv_grow(pTHX_ SV *const sv, STRLEN newlen)
{
    char *s;

    PERL_ARGS_ASSERT_SV_GROW;

    if (SvROK(sv))
        sv_unref(sv);
    if (SvTYPE(sv) < SVt_PV) {
        sv_upgrade(sv, SVt_PV);
        s = SvPVX_mutable(sv);
    }
    else if (SvOOK(sv)) {	/* pv is offset? */
        sv_backoff(sv);
        s = SvPVX_mutable(sv);
        if (newlen > SvLEN(sv))
            newlen += 10 * (newlen - SvCUR(sv)); /* avoid copy each time */
    }
    else
    {
        if (SvIsCOW(sv)) S_sv_uncow(aTHX_ sv, 0);
        s = SvPVX_mutable(sv);
    }

#ifdef PERL_COPY_ON_WRITE
    /* the new COW scheme uses SvPVX(sv)[SvLEN(sv)-1] (if spare)
     * to store the COW count. So in general, allocate one more byte than
     * asked for, to make it likely this byte is always spare: and thus
     * make more strings COW-able.
     *
     * Only increment if the allocation isn't MEM_SIZE_MAX,
     * otherwise it will wrap to 0.
     */
    if ( newlen != MEM_SIZE_MAX )
        newlen++;
#endif

#if defined(PERL_USE_MALLOC_SIZE) && defined(Perl_safesysmalloc_size)
#define PERL_UNWARANTED_CHUMMINESS_WITH_MALLOC
#endif

    if (newlen > SvLEN(sv)) {		/* need more room? */
        STRLEN minlen = SvCUR(sv);
        minlen += (minlen >> PERL_STRLEN_EXPAND_SHIFT) + PERL_STRLEN_NEW_MIN;
        if (newlen < minlen)
            newlen = minlen;
#ifndef PERL_UNWARANTED_CHUMMINESS_WITH_MALLOC

        /* Don't round up on the first allocation, as odds are pretty good that
         * the initial request is accurate as to what is really needed */
        if (SvLEN(sv)) {
            STRLEN rounded = PERL_STRLEN_ROUNDUP(newlen);
            if (rounded > newlen)
                newlen = rounded;
        }
#endif
        if (SvLEN(sv) && s) {
            s = (char*)saferealloc(s, newlen);
        }
        else {
            s = (char*)safemalloc(newlen);
            if (SvPVX_const(sv) && SvCUR(sv)) {
                Move(SvPVX_const(sv), s, SvCUR(sv), char);
            }
        }
        SvPV_set(sv, s);
#ifdef PERL_UNWARANTED_CHUMMINESS_WITH_MALLOC
        /* Do this here, do it once, do it right, and then we will never get
           called back into sv_grow() unless there really is some growing
           needed.  */
        SvLEN_set(sv, Perl_safesysmalloc_size(s));
#else
        SvLEN_set(sv, newlen);
#endif
    }
    return s;
}

/*
=for apidoc sv_grow_fresh

A cut-down version of sv_grow intended only for when sv is a freshly-minted
SVt_PV, SVt_PVIV, SVt_PVNV, or SVt_PVMG. i.e. sv has the default flags, has
never been any other type, and does not have an existing string. Basically,
just assigns a char buffer and returns a pointer to it.

=cut
*/


char *
Perl_sv_grow_fresh(pTHX_ SV *const sv, STRLEN newlen)
{
    char *s;

    PERL_ARGS_ASSERT_SV_GROW_FRESH;

    assert(SvTYPE(sv) >= SVt_PV && SvTYPE(sv) <= SVt_PVMG);
    assert(!SvROK(sv));
    assert(!SvOOK(sv));
    assert(!SvIsCOW(sv));
    assert(!SvLEN(sv));
    assert(!SvCUR(sv));

#ifdef PERL_COPY_ON_WRITE
    /* the new COW scheme uses SvPVX(sv)[SvLEN(sv)-1] (if spare)
     * to store the COW count. So in general, allocate one more byte than
     * asked for, to make it likely this byte is always spare: and thus
     * make more strings COW-able.
     *
     * Only increment if the allocation isn't MEM_SIZE_MAX,
     * otherwise it will wrap to 0.
     */
    if ( newlen != MEM_SIZE_MAX )
        newlen++;
#endif

    if (newlen < PERL_STRLEN_NEW_MIN)
        newlen = PERL_STRLEN_NEW_MIN;

    s = (char*)safemalloc(newlen);
    SvPV_set(sv, s);

    /* No PERL_UNWARANTED_CHUMMINESS_WITH_MALLOC here, since many strings */
    /* will never be grown once set. Let the real sv_grow worry about that. */
    SvLEN_set(sv, newlen);
    return s;
}

/*
=for apidoc sv_setiv
=for apidoc_item sv_setiv_mg

These copy an integer into the given SV, upgrading first if necessary.

They differ only in that C<sv_setiv_mg> handles 'set' magic; C<sv_setiv> does
not.

=cut
*/

void
Perl_sv_setiv(pTHX_ SV *const sv, const IV i)
{
    PERL_ARGS_ASSERT_SV_SETIV;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    switch (SvTYPE(sv)) {
#if NVSIZE <= IVSIZE
    case SVt_NULL:
    case SVt_NV:
        SET_SVANY_FOR_BODYLESS_IV(sv);
        SvFLAGS(sv) &= ~SVTYPEMASK;
        SvFLAGS(sv) |= SVt_IV;
        break;
#else
    case SVt_NULL:
        SET_SVANY_FOR_BODYLESS_IV(sv);
        SvFLAGS(sv) &= ~SVTYPEMASK;
        SvFLAGS(sv) |= SVt_IV;
        break;
    case SVt_NV:
        sv_upgrade(sv, SVt_IV);
        break;
#endif
    case SVt_PV:
        sv_upgrade(sv, SVt_PVIV);
        break;

    case SVt_PVGV:
        if (!isGV_with_GP(sv))
            break;
        /* FALLTHROUGH */
    case SVt_PVAV:
    case SVt_PVHV:
    case SVt_PVCV:
    case SVt_PVFM:
    case SVt_PVIO:
        /* diag_listed_as: Can't coerce %s to %s in %s */
        Perl_croak(aTHX_ "Can't coerce %s to integer in %s", sv_reftype(sv,0),
                   OP_DESC(PL_op));
        NOT_REACHED; /* NOTREACHED */
        break;
    default: NOOP;
    }
    (void)SvIOK_only(sv);			/* validate number */
    SvIV_set(sv, i);
    SvTAINT(sv);
}

void
Perl_sv_setiv_mg(pTHX_ SV *const sv, const IV i)
{
    PERL_ARGS_ASSERT_SV_SETIV_MG;

    sv_setiv(sv,i);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_setuv
=for apidoc_item sv_setuv_mg

These copy an unsigned integer into the given SV, upgrading first if necessary.


They differ only in that C<sv_setuv_mg> handles 'set' magic; C<sv_setuv> does
not.

=cut
*/

void
Perl_sv_setuv(pTHX_ SV *const sv, const UV u)
{
    PERL_ARGS_ASSERT_SV_SETUV;

    /* With the if statement to ensure that integers are stored as IVs whenever
       possible:
       u=1.49  s=0.52  cu=72.49  cs=10.64  scripts=270  tests=20865

       without
       u=1.35  s=0.47  cu=73.45  cs=11.43  scripts=270  tests=20865

       If you wish to remove the following if statement, so that this routine
       (and its callers) always return UVs, please benchmark to see what the
       effect is. Modern CPUs may be different. Or may not :-)
    */
    if (u <= (UV)IV_MAX) {
       sv_setiv(sv, (IV)u);
       return;
    }
    sv_setiv(sv, 0);
    SvIsUV_on(sv);
    SvUV_set(sv, u);
}

void
Perl_sv_setuv_mg(pTHX_ SV *const sv, const UV u)
{
    PERL_ARGS_ASSERT_SV_SETUV_MG;

    sv_setuv(sv,u);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_setnv
=for apidoc_item sv_setnv_mg

These copy a double into the given SV, upgrading first if necessary.

They differ only in that C<sv_setnv_mg> handles 'set' magic; C<sv_setnv> does
not.

=cut
*/

void
Perl_sv_setnv(pTHX_ SV *const sv, const NV num)
{
    PERL_ARGS_ASSERT_SV_SETNV;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    switch (SvTYPE(sv)) {
    case SVt_NULL:
    case SVt_IV:
#if NVSIZE <= IVSIZE
        SET_SVANY_FOR_BODYLESS_NV(sv);
        SvFLAGS(sv) &= ~SVTYPEMASK;
        SvFLAGS(sv) |= SVt_NV;
        break;
#else
        sv_upgrade(sv, SVt_NV);
        break;
#endif
    case SVt_PV:
    case SVt_PVIV:
        sv_upgrade(sv, SVt_PVNV);
        break;

    case SVt_PVGV:
        if (!isGV_with_GP(sv))
            break;
        /* FALLTHROUGH */
    case SVt_PVAV:
    case SVt_PVHV:
    case SVt_PVCV:
    case SVt_PVFM:
    case SVt_PVIO:
        /* diag_listed_as: Can't coerce %s to %s in %s */
        Perl_croak(aTHX_ "Can't coerce %s to number in %s", sv_reftype(sv,0),
                   OP_DESC(PL_op));
        NOT_REACHED; /* NOTREACHED */
        break;
    default: NOOP;
    }
    SvNV_set(sv, num);
    (void)SvNOK_only(sv);			/* validate number */
    SvTAINT(sv);
}

void
Perl_sv_setnv_mg(pTHX_ SV *const sv, const NV num)
{
    PERL_ARGS_ASSERT_SV_SETNV_MG;

    sv_setnv(sv,num);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_setrv_noinc
=for apidoc_item sv_setrv_noinc_mg

Copies an SV pointer into the given SV as an SV reference, upgrading it if
necessary. After this, C<SvRV(sv)> is equal to I<ref>. This does not adjust
the reference count of I<ref>. The reference I<ref> must not be NULL.

C<sv_setrv_noinc_mg> will invoke 'set' magic on the SV; C<sv_setrv_noinc> will
not.

=cut
*/

void
Perl_sv_setrv_noinc(pTHX_ SV *const sv, SV *const ref)
{
    PERL_ARGS_ASSERT_SV_SETRV_NOINC;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    prepare_SV_for_RV(sv);

    SvOK_off(sv);
    SvRV_set(sv, ref);
    SvROK_on(sv);
}

void
Perl_sv_setrv_noinc_mg(pTHX_ SV *const sv, SV *const ref)
{
    PERL_ARGS_ASSERT_SV_SETRV_NOINC_MG;

    sv_setrv_noinc(sv, ref);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_setrv_inc
=for apidoc_item sv_setrv_inc_mg

As C<sv_setrv_noinc> but increments the reference count of I<ref>.

C<sv_setrv_inc_mg> will invoke 'set' magic on the SV; C<sv_setrv_inc> will
not.

=cut
*/

void
Perl_sv_setrv_inc(pTHX_ SV *const sv, SV *const ref)
{
    PERL_ARGS_ASSERT_SV_SETRV_INC;

    sv_setrv_noinc(sv, SvREFCNT_inc_simple_NN(ref));
}

void
Perl_sv_setrv_inc_mg(pTHX_ SV *const sv, SV *const ref)
{
    PERL_ARGS_ASSERT_SV_SETRV_INC_MG;

    sv_setrv_noinc(sv, SvREFCNT_inc_simple_NN(ref));
    SvSETMAGIC(sv);
}

/* Return a cleaned-up, printable version of sv, for non-numeric, or
 * not incrementable warning display.
 * Originally part of S_not_a_number().
 * The return value may be != tmpbuf.
 */

STATIC const char *
S_sv_display(pTHX_ SV *const sv, char *tmpbuf, STRLEN tmpbuf_size) {
    const char *pv;

     PERL_ARGS_ASSERT_SV_DISPLAY;

     if (DO_UTF8(sv)) {
          SV *dsv = newSVpvs_flags("", SVs_TEMP);
          pv = sv_uni_display(dsv, sv, 32, UNI_DISPLAY_ISPRINT);
     } else {
          char *d = tmpbuf;
          const char * const limit = tmpbuf + tmpbuf_size - 8;
          /* each *s can expand to 4 chars + "...\0",
             i.e. need room for 8 chars */

          const char *s = SvPVX_const(sv);
          const char * const end = s + SvCUR(sv);
          for ( ; s < end && d < limit; s++ ) {
               int ch = (U8) *s;
               if (! isASCII(ch) && !isPRINT_LC(ch)) {
                    *d++ = 'M';
                    *d++ = '-';

                    /* Map to ASCII "equivalent" of Latin1 */
                    ch = LATIN1_TO_NATIVE(NATIVE_TO_LATIN1(ch) & 127);
               }
               if (ch == '\n') {
                    *d++ = '\\';
                    *d++ = 'n';
               }
               else if (ch == '\r') {
                    *d++ = '\\';
                    *d++ = 'r';
               }
               else if (ch == '\f') {
                    *d++ = '\\';
                    *d++ = 'f';
               }
               else if (ch == '\\') {
                    *d++ = '\\';
                    *d++ = '\\';
               }
               else if (ch == '\0') {
                    *d++ = '\\';
                    *d++ = '0';
               }
               else if (isPRINT_LC(ch))
                    *d++ = ch;
               else {
                    *d++ = '^';
                    *d++ = toCTRL(ch);
               }
          }
          if (s < end) {
               *d++ = '.';
               *d++ = '.';
               *d++ = '.';
          }
          *d = '\0';
          pv = tmpbuf;
    }

    return pv;
}

/* Print an "isn't numeric" warning, using a cleaned-up,
 * printable version of the offending string
 */

STATIC void
S_not_a_number(pTHX_ SV *const sv)
{
     char tmpbuf[64];
     const char *pv;

     PERL_ARGS_ASSERT_NOT_A_NUMBER;

     pv = sv_display(sv, tmpbuf, sizeof(tmpbuf));

    if (PL_op)
        Perl_warner(aTHX_ packWARN(WARN_NUMERIC),
                    /* diag_listed_as: Argument "%s" isn't numeric%s */
                    "Argument \"%s\" isn't numeric in %s", pv,
                    OP_DESC(PL_op));
    else
        Perl_warner(aTHX_ packWARN(WARN_NUMERIC),
                    /* diag_listed_as: Argument "%s" isn't numeric%s */
                    "Argument \"%s\" isn't numeric", pv);
}

STATIC void
S_not_incrementable(pTHX_ SV *const sv) {
     char tmpbuf[64];
     const char *pv;

     PERL_ARGS_ASSERT_NOT_INCREMENTABLE;

     pv = sv_display(sv, tmpbuf, sizeof(tmpbuf));

     Perl_warner(aTHX_ packWARN(WARN_NUMERIC),
                 "Argument \"%s\" treated as 0 in increment (++)", pv);
}

/*
=for apidoc looks_like_number

Test if the content of an SV looks like a number (or is a number).
C<Inf> and C<Infinity> are treated as numbers (so will not issue a
non-numeric warning), even if your C<atof()> doesn't grok them.  Get-magic is
ignored.

=cut
*/

I32
Perl_looks_like_number(pTHX_ SV *const sv)
{
    const char *sbegin;
    STRLEN len;
    int numtype;

    PERL_ARGS_ASSERT_LOOKS_LIKE_NUMBER;

    if (SvPOK(sv) || SvPOKp(sv)) {
        sbegin = SvPV_nomg_const(sv, len);
    }
    else
        return SvFLAGS(sv) & (SVf_NOK|SVp_NOK|SVf_IOK|SVp_IOK);
    numtype = grok_number(sbegin, len, NULL);
    return ((numtype & IS_NUMBER_TRAILING)) ? 0 : numtype;
}

STATIC bool
S_glob_2number(pTHX_ GV * const gv)
{
    PERL_ARGS_ASSERT_GLOB_2NUMBER;

    /* We know that all GVs stringify to something that is not-a-number,
        so no need to test that.  */
    if (ckWARN(WARN_NUMERIC))
    {
        SV *const buffer = sv_newmortal();
        gv_efullname3(buffer, gv, "*");
        not_a_number(buffer);
    }
    /* We just want something true to return, so that S_sv_2iuv_common
        can tail call us and return true.  */
    return TRUE;
}

/* Actually, ISO C leaves conversion of UV to IV undefined, but
   until proven guilty, assume that things are not that bad... */

/*
   NV_PRESERVES_UV:

   As 64 bit platforms often have an NV that doesn't preserve all bits of
   an IV (an assumption perl has been based on to date) it becomes necessary
   to remove the assumption that the NV always carries enough precision to
   recreate the IV whenever needed, and that the NV is the canonical form.
   Instead, IV/UV and NV need to be given equal rights. So as to not lose
   precision as a side effect of conversion (which would lead to insanity
   and the dragon(s) in t/op/numconvert.t getting very angry) the intent is
   1) to distinguish between IV/UV/NV slots that have a valid conversion cached
      where precision was lost, and IV/UV/NV slots that have a valid conversion
      which has lost no precision
   2) to ensure that if a numeric conversion to one form is requested that
      would lose precision, the precise conversion (or differently
      imprecise conversion) is also performed and cached, to prevent
      requests for different numeric formats on the same SV causing
      lossy conversion chains. (lossless conversion chains are perfectly
      acceptable (still))


   flags are used:
   SvIOKp is true if the IV slot contains a valid value
   SvIOK  is true only if the IV value is accurate (UV if SvIOK_UV true)
   SvNOKp is true if the NV slot contains a valid value
   SvNOK  is true only if the NV value is accurate

   so
   while converting from PV to NV, check to see if converting that NV to an
   IV(or UV) would lose accuracy over a direct conversion from PV to
   IV(or UV). If it would, cache both conversions, return NV, but mark
   SV as IOK NOKp (ie not NOK).

   While converting from PV to IV, check to see if converting that IV to an
   NV would lose accuracy over a direct conversion from PV to NV. If it
   would, cache both conversions, flag similarly.

   Before, the SV value "3.2" could become NV=3.2 IV=3 NOK, IOK quite
   correctly because if IV & NV were set NV *always* overruled.
   Now, "3.2" will become NV=3.2 IV=3 NOK, IOKp, because the flag's meaning
   changes - now IV and NV together means that the two are interchangeable:
   SvIVX == (IV) SvNVX && SvNVX == (NV) SvIVX;

   The benefit of this is that operations such as pp_add know that if
   SvIOK is true for both left and right operands, then integer addition
   can be used instead of floating point (for cases where the result won't
   overflow). Before, floating point was always used, which could lead to
   loss of precision compared with integer addition.

   * making IV and NV equal status should make maths accurate on 64 bit
     platforms
   * may speed up maths somewhat if pp_add and friends start to use
     integers when possible instead of fp. (Hopefully the overhead in
     looking for SvIOK and checking for overflow will not outweigh the
     fp to integer speedup)
   * will slow down integer operations (callers of SvIV) on "inaccurate"
     values, as the change from SvIOK to SvIOKp will cause a call into
     sv_2iv each time rather than a macro access direct to the IV slot
   * should speed up number->string conversion on integers as IV is
     favoured when IV and NV are equally accurate

   ####################################################################
   You had better be using SvIOK_notUV if you want an IV for arithmetic:
   SvIOK is true if (IV or UV), so you might be getting (IV)SvUV.
   On the other hand, SvUOK is true iff UV.
   ####################################################################

   Your mileage will vary depending your CPU's relative fp to integer
   performance ratio.
*/

#ifndef NV_PRESERVES_UV
#  define IS_NUMBER_UNDERFLOW_IV 1
#  define IS_NUMBER_UNDERFLOW_UV 2
#  define IS_NUMBER_IV_AND_UV    2
#  define IS_NUMBER_OVERFLOW_IV  4
#  define IS_NUMBER_OVERFLOW_UV  5

/* sv_2iuv_non_preserve(): private routine for use by sv_2iv() and sv_2uv() */

/* For sv_2nv these three cases are "SvNOK and don't bother casting"  */
STATIC int
S_sv_2iuv_non_preserve(pTHX_ SV *const sv
#  ifdef DEBUGGING
                       , I32 numtype
#  endif
                       )
{
    PERL_ARGS_ASSERT_SV_2IUV_NON_PRESERVE;
    PERL_UNUSED_CONTEXT;

    DEBUG_c(PerlIO_printf(Perl_debug_log,"sv_2iuv_non '%s', IV=0x%" UVxf " NV=%" NVgf " inttype=%" UVXf "\n", SvPVX_const(sv), SvIVX(sv), SvNVX(sv), (UV)numtype));
    if (SvNVX(sv) < (NV)IV_MIN) {
        (void)SvIOKp_on(sv);
        (void)SvNOK_on(sv);
        SvIV_set(sv, IV_MIN);
        return IS_NUMBER_UNDERFLOW_IV;
    }
    if (SvNVX(sv) > (NV)UV_MAX) {
        (void)SvIOKp_on(sv);
        (void)SvNOK_on(sv);
        SvIsUV_on(sv);
        SvUV_set(sv, UV_MAX);
        return IS_NUMBER_OVERFLOW_UV;
    }
    (void)SvIOKp_on(sv);
    (void)SvNOK_on(sv);
    /* Can't use strtol etc to convert this string.  (See truth table in
       sv_2iv  */
    if (SvNVX(sv) < IV_MAX_P1) {
        SvIV_set(sv, I_V(SvNVX(sv)));
        if ((NV)(SvIVX(sv)) == SvNVX(sv)) {
            SvIOK_on(sv); /* Integer is precise. NOK, IOK */
        } else {
            /* Integer is imprecise. NOK, IOKp */
        }
        return SvNVX(sv) < 0 ? IS_NUMBER_UNDERFLOW_UV : IS_NUMBER_IV_AND_UV;
    }
    SvIsUV_on(sv);
    SvUV_set(sv, U_V(SvNVX(sv)));
    if ((NV)(SvUVX(sv)) == SvNVX(sv)) {
        if (SvUVX(sv) == UV_MAX) {
            /* As we know that NVs don't preserve UVs, UV_MAX cannot
               possibly be preserved by NV. Hence, it must be overflow.
               NOK, IOKp */
            return IS_NUMBER_OVERFLOW_UV;
        }
        SvIOK_on(sv); /* Integer is precise. NOK, UOK */
    } else {
        /* Integer is imprecise. NOK, IOKp */
    }
    return IS_NUMBER_OVERFLOW_IV;
}
#endif /* !NV_PRESERVES_UV*/

/* If numtype is infnan, set the NV of the sv accordingly.
 * If numtype is anything else, try setting the NV using Atof(PV). */
static void
S_sv_setnv(pTHX_ SV* sv, int numtype)
{
    bool pok = cBOOL(SvPOK(sv));
    bool nok = FALSE;
#ifdef NV_INF
    if ((numtype & IS_NUMBER_INFINITY)) {
        SvNV_set(sv, (numtype & IS_NUMBER_NEG) ? -NV_INF : NV_INF);
        nok = TRUE;
    } else
#endif
#ifdef NV_NAN
    if ((numtype & IS_NUMBER_NAN)) {
        SvNV_set(sv, NV_NAN);
        nok = TRUE;
    } else
#endif
    if (pok) {
        SvNV_set(sv, Atof(SvPVX_const(sv)));
        /* Purposefully no true nok here, since we don't want to blow
         * away the possible IOK/UV of an existing sv. */
    }
    if (nok) {
        SvNOK_only(sv); /* No IV or UV please, this is pure infnan. */
        if (pok)
            SvPOK_on(sv); /* PV is okay, though. */
    }
}

STATIC bool
S_sv_2iuv_common(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_2IUV_COMMON;

    if (SvNOKp(sv)) {
        /* erm. not sure. *should* never get NOKp (without NOK) from sv_2nv
         * without also getting a cached IV/UV from it at the same time
         * (ie PV->NV conversion should detect loss of accuracy and cache
         * IV or UV at same time to avoid this. */
        /* IV-over-UV optimisation - choose to cache IV if possible */

        if (SvTYPE(sv) == SVt_NV)
            sv_upgrade(sv, SVt_PVNV);

    got_nv:
        (void)SvIOKp_on(sv);	/* Must do this first, to clear any SvOOK */
        /* < not <= as for NV doesn't preserve UV, ((NV)IV_MAX+1) will almost
           certainly cast into the IV range at IV_MAX, whereas the correct
           answer is the UV IV_MAX +1. Hence < ensures that dodgy boundary
           cases go to UV */
#if defined(NAN_COMPARE_BROKEN) && defined(Perl_isnan)
        if (Perl_isnan(SvNVX(sv))) {
            SvUV_set(sv, 0);
            SvIsUV_on(sv);
            return FALSE;
        }
#endif
        if (SvNVX(sv) < (NV)IV_MAX + 0.5) {
            SvIV_set(sv, I_V(SvNVX(sv)));
            if (SvNVX(sv) == (NV) SvIVX(sv)
#ifndef NV_PRESERVES_UV
                && SvIVX(sv) != IV_MIN /* avoid negating IV_MIN below */
                && (((UV)1 << NV_PRESERVES_UV_BITS) >
                    (UV)(SvIVX(sv) > 0 ? SvIVX(sv) : -SvIVX(sv)))
                /* Don't flag it as "accurately an integer" if the number
                   came from a (by definition imprecise) NV operation, and
                   we're outside the range of NV integer precision */
#endif
                ) {
                if (SvNOK(sv))
                    SvIOK_on(sv);  /* Can this go wrong with rounding? NWC */
                else {
                    /* scalar has trailing garbage, eg "42a" */
                }
                DEBUG_c(PerlIO_printf(Perl_debug_log,
                                      "0x%" UVxf " iv(%" NVgf " => %" IVdf ") (precise)\n",
                                      PTR2UV(sv),
                                      SvNVX(sv),
                                      SvIVX(sv)));

            } else {
                /* IV not precise.  No need to convert from PV, as NV
                   conversion would already have cached IV if it detected
                   that PV->IV would be better than PV->NV->IV
                   flags already correct - don't set public IOK.  */
                DEBUG_c(PerlIO_printf(Perl_debug_log,
                                      "0x%" UVxf " iv(%" NVgf " => %" IVdf ") (imprecise)\n",
                                      PTR2UV(sv),
                                      SvNVX(sv),
                                      SvIVX(sv)));
            }
            /* Can the above go wrong if SvIVX == IV_MIN and SvNVX < IV_MIN,
               but the cast (NV)IV_MIN rounds to a the value less (more
               negative) than IV_MIN which happens to be equal to SvNVX ??
               Analogous to 0xFFFFFFFFFFFFFFFF rounding up to NV (2**64) and
               NV rounding back to 0xFFFFFFFFFFFFFFFF, so UVX == UV(NVX) and
               (NV)UVX == NVX are both true, but the values differ. :-(
               Hopefully for 2s complement IV_MIN is something like
               0x8000000000000000 which will be exact. NWC */
        }
        else {
            SvUV_set(sv, U_V(SvNVX(sv)));
            if (
                (SvNVX(sv) == (NV) SvUVX(sv))
#ifndef  NV_PRESERVES_UV
                /* Make sure it's not 0xFFFFFFFFFFFFFFFF */
                /*&& (SvUVX(sv) != UV_MAX) irrelevant with code below */
                && (((UV)1 << NV_PRESERVES_UV_BITS) > SvUVX(sv))
                /* Don't flag it as "accurately an integer" if the number
                   came from a (by definition imprecise) NV operation, and
                   we're outside the range of NV integer precision */
#endif
                && SvNOK(sv)
                )
                SvIOK_on(sv);
            SvIsUV_on(sv);
            DEBUG_c(PerlIO_printf(Perl_debug_log,
                                  "0x%" UVxf " 2iv(%" UVuf " => %" IVdf ") (as unsigned)\n",
                                  PTR2UV(sv),
                                  SvUVX(sv),
                                  SvUVX(sv)));
        }
    }
    else if (SvPOKp(sv)) {
        UV value;
        int numtype;
        const char *s = SvPVX_const(sv);
        const STRLEN cur = SvCUR(sv);

        /* short-cut for a single digit string like "1" */

        if (cur == 1) {
            char c = *s;
            if (isDIGIT(c)) {
                if (SvTYPE(sv) < SVt_PVIV)
                    sv_upgrade(sv, SVt_PVIV);
                (void)SvIOK_on(sv);
                SvIV_set(sv, (IV)(c - '0'));
                return FALSE;
            }
        }

        numtype = grok_number(s, cur, &value);
        /* We want to avoid a possible problem when we cache an IV/ a UV which
           may be later translated to an NV, and the resulting NV is not
           the same as the direct translation of the initial string
           (eg 123.456 can shortcut to the IV 123 with atol(), but we must
           be careful to ensure that the value with the .456 is around if the
           NV value is requested in the future).

           This means that if we cache such an IV/a UV, we need to cache the
           NV as well.  Moreover, we trade speed for space, and do not
           cache the NV if we are sure it's not needed.
         */

        /* SVt_PVNV is one higher than SVt_PVIV, hence this order  */
        if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
             == IS_NUMBER_IN_UV) {
            /* It's definitely an integer, only upgrade to PVIV */
            if (SvTYPE(sv) < SVt_PVIV)
                sv_upgrade(sv, SVt_PVIV);
            (void)SvIOK_on(sv);
        } else if (SvTYPE(sv) < SVt_PVNV)
            sv_upgrade(sv, SVt_PVNV);

        if ((numtype & (IS_NUMBER_INFINITY | IS_NUMBER_NAN))) {
            if (ckWARN(WARN_NUMERIC) && ((numtype & IS_NUMBER_TRAILING)))
                not_a_number(sv);
            S_sv_setnv(aTHX_ sv, numtype);
            goto got_nv;        /* Fill IV/UV slot and set IOKp */
        }

        /* If NVs preserve UVs then we only use the UV value if we know that
           we aren't going to call atof() below. If NVs don't preserve UVs
           then the value returned may have more precision than atof() will
           return, even though value isn't perfectly accurate.  */
        if ((numtype & (IS_NUMBER_IN_UV
#ifdef NV_PRESERVES_UV
                        | IS_NUMBER_NOT_INT
#endif
            )) == IS_NUMBER_IN_UV) {
            /* This won't turn off the public IOK flag if it was set above  */
            (void)SvIOKp_on(sv);

            if (!(numtype & IS_NUMBER_NEG)) {
                /* positive */;
                if (value <= (UV)IV_MAX) {
                    SvIV_set(sv, (IV)value);
                } else {
                    /* it didn't overflow, and it was positive. */
                    SvUV_set(sv, value);
                    SvIsUV_on(sv);
                }
            } else {
                /* 2s complement assumption  */
                if (value <= (UV)IV_MIN) {
                    SvIV_set(sv, value == (UV)IV_MIN
                                    ? IV_MIN : -(IV)value);
                } else {
                    /* Too negative for an IV.  This is a double upgrade, but
                       I'm assuming it will be rare.  */
                    if (SvTYPE(sv) < SVt_PVNV)
                        sv_upgrade(sv, SVt_PVNV);
                    SvNOK_on(sv);
                    SvIOK_off(sv);
                    SvIOKp_on(sv);
                    SvNV_set(sv, -(NV)value);
                    SvIV_set(sv, IV_MIN);
                }
            }
        }
        /* For !NV_PRESERVES_UV and IS_NUMBER_IN_UV and IS_NUMBER_NOT_INT we
           will be in the previous block to set the IV slot, and the next
           block to set the NV slot.  So no else here.  */

        if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
            != IS_NUMBER_IN_UV) {
            /* It wasn't an (integer that doesn't overflow the UV). */
            S_sv_setnv(aTHX_ sv, numtype);

            if (! numtype && ckWARN(WARN_NUMERIC))
                not_a_number(sv);

            DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%" UVxf " 2iv(%" NVgf ")\n",
                                  PTR2UV(sv), SvNVX(sv)));

#ifdef NV_PRESERVES_UV
            SvNOKp_on(sv);
            if (numtype)
                SvNOK_on(sv);
            goto got_nv;        /* Fill IV/UV slot and set IOKp, maybe IOK */
#else /* NV_PRESERVES_UV */
            if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
                == (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT)) {
                /* The IV/UV slot will have been set from value returned by
                   grok_number above.  The NV slot has just been set using
                   Atof.  */
                SvNOK_on(sv);
                assert (SvIOKp(sv));
            } else {
                if (((UV)1 << NV_PRESERVES_UV_BITS) >
                    U_V(Perl_fabs(SvNVX(sv)))) {
                    /* Small enough to preserve all bits. */
                    (void)SvIOKp_on(sv);
                    SvNOK_on(sv);
                    SvIV_set(sv, I_V(SvNVX(sv)));
                    if ((NV)(SvIVX(sv)) == SvNVX(sv))
                        SvIOK_on(sv);
                    /* There had been runtime checking for
                       "U_V(Perl_fabs(SvNVX(sv))) < (UV)IV_MAX" here to ensure
                       that this NV is in the preserved range, but this should
                       be always true if the following assertion is true: */
                    STATIC_ASSERT_STMT(((UV)1 << NV_PRESERVES_UV_BITS) <=
                                       (UV)IV_MAX);
                } else {
                    /* IN_UV NOT_INT
                         0      0	already failed to read UV.
                         0      1       already failed to read UV.
                         1      0       you won't get here in this case. IV/UV
                                        slot set, public IOK, Atof() unneeded.
                         1      1       already read UV.
                       so there's no point in sv_2iuv_non_preserve() attempting
                       to use atol, strtol, strtoul etc.  */
#  ifdef DEBUGGING
                    sv_2iuv_non_preserve (sv, numtype);
#  else
                    sv_2iuv_non_preserve (sv);
#  endif
                }
            }
        /* It might be more code efficient to go through the entire logic above
           and conditionally set with SvIOKp_on() rather than SvIOK(), but it
           gets complex and potentially buggy, so more programmer efficient
           to do it this way, by turning off the public flags:  */
        if (!numtype)
            SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK);
#endif /* NV_PRESERVES_UV */
        }
    }
    else {
        if (isGV_with_GP(sv))
            return glob_2number(MUTABLE_GV(sv));

        if (!PL_localizing && ckWARN(WARN_UNINITIALIZED))
                report_uninit(sv);
        if (SvTYPE(sv) < SVt_IV)
            /* Typically the caller expects that sv_any is not NULL now.  */
            sv_upgrade(sv, SVt_IV);
        /* Return 0 from the caller.  */
        return TRUE;
    }
    return FALSE;
}

/*
=for apidoc sv_2iv_flags

Return the integer value of an SV, doing any necessary string
conversion.  If C<flags> has the C<SV_GMAGIC> bit set, does an C<mg_get()> first.
Normally used via the C<SvIV(sv)> and C<SvIVx(sv)> macros.

=cut
*/

IV
Perl_sv_2iv_flags(pTHX_ SV *const sv, const I32 flags)
{
    PERL_ARGS_ASSERT_SV_2IV_FLAGS;

    assert (SvTYPE(sv) != SVt_PVAV && SvTYPE(sv) != SVt_PVHV
         && SvTYPE(sv) != SVt_PVFM);

    if (SvGMAGICAL(sv) && (flags & SV_GMAGIC))
        mg_get(sv);

    if (SvROK(sv)) {
        if (SvAMAGIC(sv)) {
            SV * tmpstr;
            if (flags & SV_SKIP_OVERLOAD)
                return 0;
            tmpstr = AMG_CALLunary(sv, numer_amg);
            if (tmpstr && (!SvROK(tmpstr) || (SvRV(tmpstr) != SvRV(sv)))) {
                return SvIV(tmpstr);
            }
        }
        return PTR2IV(SvRV(sv));
    }

    if (SvVALID(sv) || isREGEXP(sv)) {
        /* FBMs use the space for SvIVX and SvNVX for other purposes, so
           must not let them cache IVs.
           In practice they are extremely unlikely to actually get anywhere
           accessible by user Perl code - the only way that I'm aware of is when
           a constant subroutine which is used as the second argument to index.

           Regexps have no SvIVX and SvNVX fields.
        */
        assert(SvPOKp(sv));
        {
            UV value;
            const char * const ptr =
                isREGEXP(sv) ? RX_WRAPPED((REGEXP*)sv) : SvPVX_const(sv);
            const int numtype
                = grok_number(ptr, SvCUR(sv), &value);

            if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
                == IS_NUMBER_IN_UV) {
                /* It's definitely an integer */
                if (numtype & IS_NUMBER_NEG) {
                    if (value < (UV)IV_MIN)
                        return -(IV)value;
                } else {
                    if (value < (UV)IV_MAX)
                        return (IV)value;
                }
            }

            /* Quite wrong but no good choices. */
            if ((numtype & IS_NUMBER_INFINITY)) {
                return (numtype & IS_NUMBER_NEG) ? IV_MIN : IV_MAX;
            } else if ((numtype & IS_NUMBER_NAN)) {
                return 0; /* So wrong. */
            }

            if (!numtype) {
                if (ckWARN(WARN_NUMERIC))
                    not_a_number(sv);
            }
            return I_V(Atof(ptr));
        }
    }

    if (SvTHINKFIRST(sv)) {
        if (SvREADONLY(sv) && !SvOK(sv)) {
            if (ckWARN(WARN_UNINITIALIZED))
                report_uninit(sv);
            return 0;
        }
    }

    if (!SvIOKp(sv)) {
        if (S_sv_2iuv_common(aTHX_ sv))
            return 0;
    }

    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%" UVxf " 2iv(%" IVdf ")\n",
        PTR2UV(sv),SvIVX(sv)));
    return SvIsUV(sv) ? (IV)SvUVX(sv) : SvIVX(sv);
}

/*
=for apidoc sv_2uv_flags

Return the unsigned integer value of an SV, doing any necessary string
conversion.  If C<flags> has the C<SV_GMAGIC> bit set, does an C<mg_get()> first.
Normally used via the C<SvUV(sv)> and C<SvUVx(sv)> macros.

=for apidoc Amnh||SV_GMAGIC

=cut
*/

UV
Perl_sv_2uv_flags(pTHX_ SV *const sv, const I32 flags)
{
    PERL_ARGS_ASSERT_SV_2UV_FLAGS;

    if (SvGMAGICAL(sv) && (flags & SV_GMAGIC))
        mg_get(sv);

    if (SvROK(sv)) {
        if (SvAMAGIC(sv)) {
            SV *tmpstr;
            if (flags & SV_SKIP_OVERLOAD)
                return 0;
            tmpstr = AMG_CALLunary(sv, numer_amg);
            if (tmpstr && (!SvROK(tmpstr) || (SvRV(tmpstr) != SvRV(sv)))) {
                return SvUV(tmpstr);
            }
        }
        return PTR2UV(SvRV(sv));
    }

    if (SvVALID(sv) || isREGEXP(sv)) {
        /* FBMs use the space for SvIVX and SvNVX for other purposes, and use
           the same flag bit as SVf_IVisUV, so must not let them cache IVs.
           Regexps have no SvIVX and SvNVX fields. */
        assert(SvPOKp(sv));
        {
            UV value;
            const char * const ptr =
                isREGEXP(sv) ? RX_WRAPPED((REGEXP*)sv) : SvPVX_const(sv);
            const int numtype
                = grok_number(ptr, SvCUR(sv), &value);

            if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
                == IS_NUMBER_IN_UV) {
                /* It's definitely an integer */
                if (!(numtype & IS_NUMBER_NEG))
                    return value;
            }

            /* Quite wrong but no good choices. */
            if ((numtype & IS_NUMBER_INFINITY)) {
                return UV_MAX; /* So wrong. */
            } else if ((numtype & IS_NUMBER_NAN)) {
                return 0; /* So wrong. */
            }

            if (!numtype) {
                if (ckWARN(WARN_NUMERIC))
                    not_a_number(sv);
            }
            return U_V(Atof(ptr));
        }
    }

    if (SvTHINKFIRST(sv)) {
        if (SvREADONLY(sv) && !SvOK(sv)) {
            if (ckWARN(WARN_UNINITIALIZED))
                report_uninit(sv);
            return 0;
        }
    }

    if (!SvIOKp(sv)) {
        if (S_sv_2iuv_common(aTHX_ sv))
            return 0;
    }

    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%" UVxf " 2uv(%" UVuf ")\n",
                          PTR2UV(sv),SvUVX(sv)));
    return SvIsUV(sv) ? SvUVX(sv) : (UV)SvIVX(sv);
}

/*
=for apidoc sv_2nv_flags

Return the num value of an SV, doing any necessary string or integer
conversion.  If C<flags> has the C<SV_GMAGIC> bit set, does an C<mg_get()> first.
Normally used via the C<SvNV(sv)> and C<SvNVx(sv)> macros.

=cut
*/

NV
Perl_sv_2nv_flags(pTHX_ SV *const sv, const I32 flags)
{
    PERL_ARGS_ASSERT_SV_2NV_FLAGS;

    assert (SvTYPE(sv) != SVt_PVAV && SvTYPE(sv) != SVt_PVHV
         && SvTYPE(sv) != SVt_PVFM);
    if (SvGMAGICAL(sv) || SvVALID(sv) || isREGEXP(sv)) {
        /* FBMs use the space for SvIVX and SvNVX for other purposes, and use
           the same flag bit as SVf_IVisUV, so must not let them cache NVs.
           Regexps have no SvIVX and SvNVX fields.  */
        const char *ptr;
        if (flags & SV_GMAGIC)
            mg_get(sv);
        if (SvNOKp(sv))
            return SvNVX(sv);
        if (SvPOKp(sv) && !SvIOKp(sv)) {
            ptr = SvPVX_const(sv);
            if (!SvIOKp(sv) && ckWARN(WARN_NUMERIC) &&
                !grok_number(ptr, SvCUR(sv), NULL))
                not_a_number(sv);
            return Atof(ptr);
        }
        if (SvIOKp(sv)) {
            if (SvIsUV(sv))
                return (NV)SvUVX(sv);
            else
                return (NV)SvIVX(sv);
        }
        if (SvROK(sv)) {
            goto return_rok;
        }
        assert(SvTYPE(sv) >= SVt_PVMG);
        /* This falls through to the report_uninit near the end of the
           function. */
    } else if (SvTHINKFIRST(sv)) {
        if (SvROK(sv)) {
        return_rok:
            if (SvAMAGIC(sv)) {
                SV *tmpstr;
                if (flags & SV_SKIP_OVERLOAD)
                    return 0;
                tmpstr = AMG_CALLunary(sv, numer_amg);
                if (tmpstr && (!SvROK(tmpstr) || (SvRV(tmpstr) != SvRV(sv)))) {
                    return SvNV(tmpstr);
                }
            }
            return PTR2NV(SvRV(sv));
        }
        if (SvREADONLY(sv) && !SvOK(sv)) {
            if (ckWARN(WARN_UNINITIALIZED))
                report_uninit(sv);
            return 0.0;
        }
    }
    if (SvTYPE(sv) < SVt_NV) {
        /* The logic to use SVt_PVNV if necessary is in sv_upgrade.  */
        sv_upgrade(sv, SVt_NV);
        CLANG_DIAG_IGNORE_STMT(-Wthread-safety);
        DEBUG_c({
            DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
            STORE_LC_NUMERIC_SET_STANDARD();
            PerlIO_printf(Perl_debug_log,
                          "0x%" UVxf " num(%" NVgf ")\n",
                          PTR2UV(sv), SvNVX(sv));
            RESTORE_LC_NUMERIC();
        });
        CLANG_DIAG_RESTORE_STMT;

    }
    else if (SvTYPE(sv) < SVt_PVNV)
        sv_upgrade(sv, SVt_PVNV);
    if (SvNOKp(sv)) {
        return SvNVX(sv);
    }
    if (SvIOKp(sv)) {
        SvNV_set(sv, SvIsUV(sv) ? (NV)SvUVX(sv) : (NV)SvIVX(sv));
#ifdef NV_PRESERVES_UV
        if (SvIOK(sv))
            SvNOK_on(sv);
        else
            SvNOKp_on(sv);
#else
        /* Only set the public NV OK flag if this NV preserves the IV  */
        /* Check it's not 0xFFFFFFFFFFFFFFFF */
        if (SvIOK(sv) &&
            SvIsUV(sv) ? ((SvUVX(sv) != UV_MAX)&&(SvUVX(sv) == U_V(SvNVX(sv))))
                       : (SvIVX(sv) == I_V(SvNVX(sv))))
            SvNOK_on(sv);
        else
            SvNOKp_on(sv);
#endif
    }
    else if (SvPOKp(sv)) {
        UV value;
        const int numtype = grok_number(SvPVX_const(sv), SvCUR(sv), &value);
        if (!SvIOKp(sv) && !numtype && ckWARN(WARN_NUMERIC))
            not_a_number(sv);
#ifdef NV_PRESERVES_UV
        if ((numtype & (IS_NUMBER_IN_UV | IS_NUMBER_NOT_INT))
            == IS_NUMBER_IN_UV) {
            /* It's definitely an integer */
            SvNV_set(sv, (numtype & IS_NUMBER_NEG) ? -(NV)value : (NV)value);
        } else {
            S_sv_setnv(aTHX_ sv, numtype);
        }
        if (numtype)
            SvNOK_on(sv);
        else
            SvNOKp_on(sv);
#else
        SvNV_set(sv, Atof(SvPVX_const(sv)));
        /* Only set the public NV OK flag if this NV preserves the value in
           the PV at least as well as an IV/UV would.
           Not sure how to do this 100% reliably. */
        /* if that shift count is out of range then Configure's test is
           wonky. We shouldn't be in here with NV_PRESERVES_UV_BITS ==
           UV_BITS */
        if (((UV)1 << NV_PRESERVES_UV_BITS) > U_V(Perl_fabs(SvNVX(sv)))) {
            SvNOK_on(sv); /* Definitely small enough to preserve all bits */
        } else if (!(numtype & IS_NUMBER_IN_UV)) {
            /* Can't use strtol etc to convert this string, so don't try.
               sv_2iv and sv_2uv will use the NV to convert, not the PV.  */
            SvNOK_on(sv);
        } else {
            /* value has been set.  It may not be precise.  */
            if ((numtype & IS_NUMBER_NEG) && (value >= (UV)IV_MIN)) {
                /* 2s complement assumption for (UV)IV_MIN  */
                SvNOK_on(sv); /* Integer is too negative.  */
            } else {
                SvNOKp_on(sv);
                SvIOKp_on(sv);

                if (numtype & IS_NUMBER_NEG) {
                    /* -IV_MIN is undefined, but we should never reach
                     * this point with both IS_NUMBER_NEG and value ==
                     * (UV)IV_MIN */
                    assert(value != (UV)IV_MIN);
                    SvIV_set(sv, -(IV)value);
                } else if (value <= (UV)IV_MAX) {
                    SvIV_set(sv, (IV)value);
                } else {
                    SvUV_set(sv, value);
                    SvIsUV_on(sv);
                }

                if (numtype & IS_NUMBER_NOT_INT) {
                    /* I believe that even if the original PV had decimals,
                       they are lost beyond the limit of the FP precision.
                       However, neither is canonical, so both only get p
                       flags.  NWC, 2000/11/25 */
                    /* Both already have p flags, so do nothing */
                } else {
                    const NV nv = SvNVX(sv);
                    /* XXX should this spot have NAN_COMPARE_BROKEN, too? */
                    if (SvNVX(sv) < (NV)IV_MAX + 0.5) {
                        if (SvIVX(sv) == I_V(nv)) {
                            SvNOK_on(sv);
                        } else {
                            /* It had no "." so it must be integer.  */
                        }
                        SvIOK_on(sv);
                    } else {
                        /* between IV_MAX and NV(UV_MAX).
                           Could be slightly > UV_MAX */

                        if (numtype & IS_NUMBER_NOT_INT) {
                            /* UV and NV both imprecise.  */
                        } else {
                            const UV nv_as_uv = U_V(nv);

                            if (value == nv_as_uv && SvUVX(sv) != UV_MAX) {
                                SvNOK_on(sv);
                            }
                            SvIOK_on(sv);
                        }
                    }
                }
            }
        }
        /* It might be more code efficient to go through the entire logic above
           and conditionally set with SvNOKp_on() rather than SvNOK(), but it
           gets complex and potentially buggy, so more programmer efficient
           to do it this way, by turning off the public flags:  */
        if (!numtype)
            SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK);
#endif /* NV_PRESERVES_UV */
    }
    else {
        if (isGV_with_GP(sv)) {
            glob_2number(MUTABLE_GV(sv));
            return 0.0;
        }

        if (!PL_localizing && ckWARN(WARN_UNINITIALIZED))
            report_uninit(sv);
        assert (SvTYPE(sv) >= SVt_NV);
        /* Typically the caller expects that sv_any is not NULL now.  */
        /* XXX Ilya implies that this is a bug in callers that assume this
           and ideally should be fixed.  */
        return 0.0;
    }
    CLANG_DIAG_IGNORE_STMT(-Wthread-safety);
    DEBUG_c({
        DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
        STORE_LC_NUMERIC_SET_STANDARD();
        PerlIO_printf(Perl_debug_log, "0x%" UVxf " 2nv(%" NVgf ")\n",
                      PTR2UV(sv), SvNVX(sv));
        RESTORE_LC_NUMERIC();
    });
    CLANG_DIAG_RESTORE_STMT;
    return SvNVX(sv);
}

/*
=for apidoc sv_2num

Return an SV with the numeric value of the source SV, doing any necessary
reference or overload conversion.  The caller is expected to have handled
get-magic already.

=cut
*/

SV *
Perl_sv_2num(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_2NUM;

    if (!SvROK(sv))
        return sv;
    if (SvAMAGIC(sv)) {
        SV * const tmpsv = AMG_CALLunary(sv, numer_amg);
        TAINT_IF(tmpsv && SvTAINTED(tmpsv));
        if (tmpsv && (!SvROK(tmpsv) || (SvRV(tmpsv) != SvRV(sv))))
            return sv_2num(tmpsv);
    }
    return sv_2mortal(newSVuv(PTR2UV(SvRV(sv))));
}

/* int2str_table: lookup table containing string representations of all
 * two digit numbers. For example, int2str_table.arr[0] is "00" and
 * int2str_table.arr[12*2] is "12".
 *
 * We are going to read two bytes at a time, so we have to ensure that
 * the array is aligned to a 2 byte boundary. That's why it was made a
 * union with a dummy U16 member. */
static const union {
    char arr[200];
    U16 dummy;
} int2str_table = {{
    '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6',
    '0', '7', '0', '8', '0', '9', '1', '0', '1', '1', '1', '2', '1', '3',
    '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9', '2', '0',
    '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7',
    '2', '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3', '4',
    '3', '5', '3', '6', '3', '7', '3', '8', '3', '9', '4', '0', '4', '1',
    '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8',
    '4', '9', '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5',
    '5', '6', '5', '7', '5', '8', '5', '9', '6', '0', '6', '1', '6', '2',
    '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
    '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6',
    '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8', '2', '8', '3',
    '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9', '9', '0',
    '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7',
    '9', '8', '9', '9'
}};

/* uiv_2buf(): private routine for use by sv_2pv_flags(): print an IV or
 * UV as a string towards the end of buf, and return pointers to start and
 * end of it.
 *
 * We assume that buf is at least TYPE_CHARS(UV) long.
 */

PERL_STATIC_INLINE char *
S_uiv_2buf(char *const buf, const IV iv, UV uv, const int is_uv, char **const peob)
{
    char *ptr = buf + TYPE_CHARS(UV);
    char * const ebuf = ptr;
    int sign;
    U16 *word_ptr, *word_table;

    PERL_ARGS_ASSERT_UIV_2BUF;

    /* ptr has to be properly aligned, because we will cast it to U16* */
    assert(PTR2nat(ptr) % 2 == 0);
    /* we are going to read/write two bytes at a time */
    word_ptr = (U16*)ptr;
    word_table = (U16*)int2str_table.arr;

    if (UNLIKELY(is_uv))
        sign = 0;
    else if (iv >= 0) {
        uv = iv;
        sign = 0;
    } else {
        /* Using 0- here to silence bogus warning from MS VC */
        uv = (UV) (0 - (UV) iv);
        sign = 1;
    }

    while (uv > 99) {
        *--word_ptr = word_table[uv % 100];
        uv /= 100;
    }
    ptr = (char*)word_ptr;

    if (uv < 10)
        *--ptr = (char)uv + '0';
    else {
        *--word_ptr = word_table[uv];
        ptr = (char*)word_ptr;
    }

    if (sign)
        *--ptr = '-';

    *peob = ebuf;
    return ptr;
}

/* Helper for sv_2pv_flags and sv_vcatpvfn_flags.  If the NV is an
 * infinity or a not-a-number, writes the appropriate strings to the
 * buffer, including a zero byte.  On success returns the written length,
 * excluding the zero byte, on failure (not an infinity, not a nan)
 * returns zero, assert-fails on maxlen being too short.
 *
 * XXX for "Inf", "-Inf", and "NaN", we could have three read-only
 * shared string constants we point to, instead of generating a new
 * string for each instance. */
STATIC size_t
S_infnan_2pv(NV nv, char* buffer, size_t maxlen, char plus) {
    char* s = buffer;
    assert(maxlen >= 4);
    if (Perl_isinf(nv)) {
        if (nv < 0) {
            if (maxlen < 5) /* "-Inf\0"  */
                return 0;
            *s++ = '-';
        } else if (plus) {
            *s++ = '+';
        }
        *s++ = 'I';
        *s++ = 'n';
        *s++ = 'f';
    }
    else if (Perl_isnan(nv)) {
        *s++ = 'N';
        *s++ = 'a';
        *s++ = 'N';
        /* XXX optionally output the payload mantissa bits as
         * "(unsigned)" (to match the nan("...") C99 function,
         * or maybe as "(0xhhh...)"  would make more sense...
         * provide a format string so that the user can decide?
         * NOTE: would affect the maxlen and assert() logic.*/
    }
    else {
      return 0;
    }
    assert((s == buffer + 3) || (s == buffer + 4));
    *s = 0;
    return s - buffer;
}

/*
=for apidoc      sv_2pv
=for apidoc_item sv_2pv_flags

These implement the various forms of the L<perlapi/C<SvPV>> macros.
The macros are the preferred interface.

These return a pointer to the string value of an SV (coercing it to a string if
necessary), and set C<*lp> to its length in bytes.

The forms differ in that plain C<sv_2pvbyte> always processes 'get' magic; and
C<sv_2pvbyte_flags> processes 'get' magic if and only if C<flags> contains
C<SV_GMAGIC>.

=for apidoc Amnh||SV_GMAGIC

=cut
*/

char *
Perl_sv_2pv_flags(pTHX_ SV *const sv, STRLEN *const lp, const U32 flags)
{
    char *s;
    bool done_gmagic = FALSE;

    PERL_ARGS_ASSERT_SV_2PV_FLAGS;

    assert (SvTYPE(sv) != SVt_PVAV && SvTYPE(sv) != SVt_PVHV
         && SvTYPE(sv) != SVt_PVFM);
    if (SvGMAGICAL(sv) && (flags & SV_GMAGIC)) {
        mg_get(sv);
        done_gmagic = TRUE;
    }

    if (SvROK(sv)) {
        if (SvAMAGIC(sv)) {
            SV *tmpstr;
            SV *nsv= (SV *)sv;
            if (flags & SV_SKIP_OVERLOAD)
                return NULL;
            if (done_gmagic)
                nsv = sv_mortalcopy_flags(sv,0);
            tmpstr = AMG_CALLunary(nsv, string_amg);
            TAINT_IF(tmpstr && SvTAINTED(tmpstr));
            if (tmpstr && (!SvROK(tmpstr) || (SvRV(tmpstr) != SvRV(nsv)))) {
                /* Unwrap this:  */
                /* char *pv = lp ? SvPV(tmpstr, *lp) : SvPV_nolen(tmpstr);
                 */

                char *pv;
                if ((SvFLAGS(tmpstr) & (SVf_POK)) == SVf_POK) {
                    if (flags & SV_CONST_RETURN) {
                        pv = (char *) SvPVX_const(tmpstr);
                    } else {
                        pv = (flags & SV_MUTABLE_RETURN)
                            ? SvPVX_mutable(tmpstr) : SvPVX(tmpstr);
                    }
                    if (lp)
                        *lp = SvCUR(tmpstr);
                } else {
                    pv = sv_2pv_flags(tmpstr, lp, flags);
                }
                if (SvUTF8(tmpstr))
                    SvUTF8_on(sv);
                else
                    SvUTF8_off(sv);
                return pv;
            }
        }
        {
            STRLEN len;
            char *retval;
            char *buffer;
            SV *const referent = SvRV(sv);

            if (!referent) {
                len = 7;
                retval = buffer = savepvn("NULLREF", len);
            } else if (SvTYPE(referent) == SVt_REGEXP &&
                       (!(PL_curcop->cop_hints & HINT_NO_AMAGIC) ||
                        amagic_is_enabled(string_amg))) {
                REGEXP * const re = (REGEXP *)MUTABLE_PTR(referent);

                assert(re);

                /* If the regex is UTF-8 we want the containing scalar to
                   have an UTF-8 flag too */
                if (RX_UTF8(re))
                    SvUTF8_on(sv);
                else
                    SvUTF8_off(sv);

                if (lp)
                    *lp = RX_WRAPLEN(re);

                return RX_WRAPPED(re);
            } else {
                const char *const typestring = sv_reftype(referent, 0);
                const STRLEN typelen = strlen(typestring);
                UV addr = PTR2UV(referent);
                const char *stashname = NULL;
                STRLEN stashnamelen = 0; /* hush, gcc */
                const char *buffer_end;

                if (SvOBJECT(referent)) {
                    const HEK *const name = HvNAME_HEK(SvSTASH(referent));

                    if (name) {
                        stashname = HEK_KEY(name);
                        stashnamelen = HEK_LEN(name);

                        if (HEK_UTF8(name)) {
                            SvUTF8_on(sv);
                        } else {
                            SvUTF8_off(sv);
                        }
                    } else {
                        stashname = "__ANON__";
                        stashnamelen = 8;
                    }
                    len = stashnamelen + 1 /* = */ + typelen + 3 /* (0x */
                        + 2 * sizeof(UV) + 2 /* )\0 */;
                } else {
                    len = typelen + 3 /* (0x */
                        + 2 * sizeof(UV) + 2 /* )\0 */;
                }

                Newx(buffer, len, char);
                buffer_end = retval = buffer + len;

                /* Working backwards  */
                *--retval = '\0';
                *--retval = ')';
                do {
                    *--retval = PL_hexdigit[addr & 15];
                } while (addr >>= 4);
                *--retval = 'x';
                *--retval = '0';
                *--retval = '(';

                retval -= typelen;
                memcpy(retval, typestring, typelen);

                if (stashname) {
                    *--retval = '=';
                    retval -= stashnamelen;
                    memcpy(retval, stashname, stashnamelen);
                }
                /* retval may not necessarily have reached the start of the
                   buffer here.  */
                assert (retval >= buffer);

                len = buffer_end - retval - 1; /* -1 for that \0  */
            }
            if (lp)
                *lp = len;
            SAVEFREEPV(buffer);
            return retval;
        }
    }

    if (SvPOKp(sv)) {
        if (lp)
            *lp = SvCUR(sv);
        if (flags & SV_MUTABLE_RETURN)
            return SvPVX_mutable(sv);
        if (flags & SV_CONST_RETURN)
            return (char *)SvPVX_const(sv);
        return SvPVX(sv);
    }

    if (SvIOK(sv)) {
        /* I'm assuming that if both IV and NV are equally valid then
           converting the IV is going to be more efficient */
        const U32 isUIOK = SvIsUV(sv);
        /* The purpose of this union is to ensure that arr is aligned on
           a 2 byte boundary, because that is what uiv_2buf() requires */
        union {
            char arr[TYPE_CHARS(UV)];
            U16 dummy;
        } buf;
        char *ebuf, *ptr;
        STRLEN len;

        if (SvTYPE(sv) < SVt_PVIV)
            sv_upgrade(sv, SVt_PVIV);
        ptr = uiv_2buf(buf.arr, SvIVX(sv), SvUVX(sv), isUIOK, &ebuf);
        len = ebuf - ptr;
        /* inlined from sv_setpvn */
        s = SvGROW_mutable(sv, len + 1);
        Move(ptr, s, len, char);
        s += len;
        *s = '\0';
        /* We used to call SvPOK_on(). Whilst this is fine for (most) Perl code,
           it means that after this stringification is cached, there is no way
           to distinguish between values originally assigned as $a = 42; and
           $a = "42"; (or results of string operators vs numeric operators)
           where the value has subsequently been used in the other sense
           and had a value cached.
           This (somewhat) hack means that we retain the cached stringification,
           but don't set SVf_POK. Hence if a value is SVf_IOK|SVf_POK then it
           originated as "42", whereas if it's SVf_IOK then it originated as 42.
           (ignore SVp_IOK and SVp_POK)
           The SvPV macros are now updated to recognise this specific case
           (and that there isn't overloading or magic that could alter the
           cached value) and so return the cached value immediately without
           re-entering this function, getting back here to this block of code,
           and repeating the same conversion. */
        SvPOKp_on(sv);
    }
    else if (SvNOK(sv)) {
        if (SvTYPE(sv) < SVt_PVNV)
            sv_upgrade(sv, SVt_PVNV);
        if (SvNVX(sv) == 0.0
#if defined(NAN_COMPARE_BROKEN) && defined(Perl_isnan)
            && !Perl_isnan(SvNVX(sv))
#endif
        ) {
            s = SvGROW_mutable(sv, 2);
            *s++ = '0';
            *s = '\0';
        } else {
            STRLEN len;
            STRLEN size = 5; /* "-Inf\0" */

            s = SvGROW_mutable(sv, size);
            len = S_infnan_2pv(SvNVX(sv), s, size, 0);
            if (len > 0) {
                s += len;
                SvPOKp_on(sv);
            }
            else {
                /* some Xenix systems wipe out errno here */
                dSAVE_ERRNO;

                size =
                    1 + /* sign */
                    1 + /* "." */
                    NV_DIG +
                    1 + /* "e" */
                    1 + /* sign */
                    5 + /* exponent digits */
                    1 + /* \0 */
                    2; /* paranoia */

                s = SvGROW_mutable(sv, size);
#ifndef USE_LOCALE_NUMERIC
                SNPRINTF_G(SvNVX(sv), s, SvLEN(sv), NV_DIG);

                SvPOKp_on(sv);
#else
                {
                    bool local_radix;
                    DECLARATION_FOR_LC_NUMERIC_MANIPULATION;
                    STORE_LC_NUMERIC_SET_TO_NEEDED();

                    local_radix = NOT_IN_NUMERIC_STANDARD_;
                    if (local_radix && SvCUR(PL_numeric_radix_sv) > 1) {
                        size += SvCUR(PL_numeric_radix_sv) - 1;
                        s = SvGROW_mutable(sv, size);
                    }

                    SNPRINTF_G(SvNVX(sv), s, SvLEN(sv), NV_DIG);

                    /* If the radix character is UTF-8, and actually is in the
                     * output, turn on the UTF-8 flag for the scalar */
                    if (   local_radix
                        && SvUTF8(PL_numeric_radix_sv)
                        && instr(s, SvPVX_const(PL_numeric_radix_sv)))
                    {
                        SvUTF8_on(sv);
                    }

                    RESTORE_LC_NUMERIC();
                }

                /* We don't call SvPOK_on(), because it may come to
                 * pass that the locale changes so that the
                 * stringification we just did is no longer correct.  We
                 * will have to re-stringify every time it is needed */
#endif
                RESTORE_ERRNO;
            }
            while (*s) s++;
        }
    }
    else if (isGV_with_GP(sv)) {
        GV *const gv = MUTABLE_GV(sv);
        SV *const buffer = sv_newmortal();

        gv_efullname3(buffer, gv, "*");

        assert(SvPOK(buffer));
        if (SvUTF8(buffer))
            SvUTF8_on(sv);
        else
            SvUTF8_off(sv);
        if (lp)
            *lp = SvCUR(buffer);
        return SvPVX(buffer);
    }
    else {
        if (lp)
            *lp = 0;
        if (flags & SV_UNDEF_RETURNS_NULL)
            return NULL;
        if (!PL_localizing && ckWARN(WARN_UNINITIALIZED))
            report_uninit(sv);
        /* Typically the caller expects that sv_any is not NULL now.  */
        if (!SvREADONLY(sv) && SvTYPE(sv) < SVt_PV)
            sv_upgrade(sv, SVt_PV);
        return (char *)"";
    }

    {
        const STRLEN len = s - SvPVX_const(sv);
        if (lp)
            *lp = len;
        SvCUR_set(sv, len);
    }
    DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%" UVxf " 2pv(%s)\n",
                          PTR2UV(sv),SvPVX_const(sv)));
    if (flags & SV_CONST_RETURN)
        return (char *)SvPVX_const(sv);
    if (flags & SV_MUTABLE_RETURN)
        return SvPVX_mutable(sv);
    return SvPVX(sv);
}

/*
=for apidoc sv_copypv
=for apidoc_item sv_copypv_flags
=for apidoc_item sv_copypv_nomg

These copy a stringified representation of the source SV into the
destination SV.  They automatically perform coercion of numeric values into
strings.  Guaranteed to preserve the C<UTF8> flag even from overloaded objects.
Similar in nature to C<sv_2pv[_flags]> but they operate directly on an SV
instead of just the string.  Mostly they use L</C<sv_2pv_flags>> to
do the work, except when that would lose the UTF-8'ness of the PV.

The three forms differ only in whether or not they perform 'get magic' on
C<sv>.  C<sv_copypv_nomg> skips 'get magic'; C<sv_copypv> performs it; and
C<sv_copypv_flags> either performs it (if the C<SV_GMAGIC> bit is set in
C<flags>) or doesn't (if that bit is cleared).

=cut
*/

void
Perl_sv_copypv_flags(pTHX_ SV *const dsv, SV *const ssv, const I32 flags)
{
    STRLEN len;
    const char *s;

    PERL_ARGS_ASSERT_SV_COPYPV_FLAGS;

    s = SvPV_flags_const(ssv,len,(flags & SV_GMAGIC));
    sv_setpvn(dsv,s,len);
    if (SvUTF8(ssv))
        SvUTF8_on(dsv);
    else
        SvUTF8_off(dsv);
}

/*
=for apidoc      sv_2pvbyte
=for apidoc_item sv_2pvbyte_flags

These implement the various forms of the L<perlapi/C<SvPVbyte>> macros.
The macros are the preferred interface.

These return a pointer to the byte-encoded representation of the SV, and set
C<*lp> to its length.  If the SV is marked as being encoded as UTF-8, it will
be downgraded, if possible, to a byte string.  If the SV cannot be downgraded,
they croak.

The forms differ in that plain C<sv_2pvbyte> always processes 'get' magic; and
C<sv_2pvbyte_flags> processes 'get' magic if and only if C<flags> contains
C<SV_GMAGIC>.

=for apidoc Amnh||SV_GMAGIC

=cut
*/

char *
Perl_sv_2pvbyte_flags(pTHX_ SV *sv, STRLEN *const lp, const U32 flags)
{
    PERL_ARGS_ASSERT_SV_2PVBYTE_FLAGS;

    if (SvGMAGICAL(sv) && (flags & SV_GMAGIC))
        mg_get(sv);
    if (((SvREADONLY(sv) || SvFAKE(sv)) && !SvIsCOW(sv))
     || isGV_with_GP(sv) || SvROK(sv)) {
        SV *sv2 = sv_newmortal();
        sv_copypv_nomg(sv2,sv);
        sv = sv2;
    }
    sv_utf8_downgrade_nomg(sv,0);
    return lp ? SvPV_nomg(sv,*lp) : SvPV_nomg_nolen(sv);
}

/*
=for apidoc      sv_2pvutf8
=for apidoc_item sv_2pvutf8_flags

These implement the various forms of the L<perlapi/C<SvPVutf8>> macros.
The macros are the preferred interface.

These return a pointer to the UTF-8-encoded representation of the SV, and set
C<*lp> to its length in bytes.  They may cause the SV to be upgraded to UTF-8
as a side-effect.

The forms differ in that plain C<sv_2pvutf8> always processes 'get' magic; and
C<sv_2pvutf8_flags> processes 'get' magic if and only if C<flags> contains
C<SV_GMAGIC>.

=cut
*/

char *
Perl_sv_2pvutf8_flags(pTHX_ SV *sv, STRLEN *const lp, const U32 flags)
{
    PERL_ARGS_ASSERT_SV_2PVUTF8_FLAGS;

    if (SvGMAGICAL(sv) && (flags & SV_GMAGIC))
        mg_get(sv);
    if (((SvREADONLY(sv) || SvFAKE(sv)) && !SvIsCOW(sv))
     || isGV_with_GP(sv) || SvROK(sv)) {
        SV *sv2 = sv_newmortal();
        sv_copypv_nomg(sv2,sv);
        sv = sv2;
    }
    sv_utf8_upgrade_nomg(sv);
    return lp ? SvPV_nomg(sv,*lp) : SvPV_nomg_nolen(sv);
}


/*
=for apidoc sv_2bool

This macro is only used by C<sv_true()> or its macro equivalent, and only if
the latter's argument is neither C<SvPOK>, C<SvIOK> nor C<SvNOK>.
It calls C<sv_2bool_flags> with the C<SV_GMAGIC> flag.

=for apidoc sv_2bool_flags

This function is only used by C<sv_true()> and friends,  and only if
the latter's argument is neither C<SvPOK>, C<SvIOK> nor C<SvNOK>.  If the flags
contain C<SV_GMAGIC>, then it does an C<mg_get()> first.


=cut
*/

bool
Perl_sv_2bool_flags(pTHX_ SV *sv, I32 flags)
{
    PERL_ARGS_ASSERT_SV_2BOOL_FLAGS;

    restart:
    if(flags & SV_GMAGIC) SvGETMAGIC(sv);

    if (!SvOK(sv))
        return 0;
    if (SvROK(sv)) {
        if (SvAMAGIC(sv)) {
            SV * const tmpsv = AMG_CALLunary(sv, bool__amg);
            if (tmpsv && (!SvROK(tmpsv) || (SvRV(tmpsv) != SvRV(sv)))) {
                bool svb;
                sv = tmpsv;
                if(SvGMAGICAL(sv)) {
                    flags = SV_GMAGIC;
                    goto restart; /* call sv_2bool */
                }
                /* expanded SvTRUE_common(sv, (flags = 0, goto restart)) */
                else if(!SvOK(sv)) {
                    svb = 0;
                }
                else if(SvPOK(sv)) {
                    svb = SvPVXtrue(sv);
                }
                else if((SvFLAGS(sv) & (SVf_IOK|SVf_NOK))) {
                    svb = (SvIOK(sv) && SvIVX(sv) != 0)
                        || (SvNOK(sv) && SvNVX(sv) != 0.0);
                }
                else {
                    flags = 0;
                    goto restart; /* call sv_2bool_nomg */
                }
                return cBOOL(svb);
            }
        }
        assert(SvRV(sv));
        return TRUE;
    }
    if (isREGEXP(sv))
        return
          RX_WRAPLEN(sv) > 1 || (RX_WRAPLEN(sv) && *RX_WRAPPED(sv) != '0');

    if (SvNOK(sv) && !SvPOK(sv))
        return SvNVX(sv) != 0.0;

    return SvTRUE_common(sv, 0);
}

/*
=for apidoc sv_utf8_upgrade
=for apidoc_item sv_utf8_upgrade_flags
=for apidoc_item sv_utf8_upgrade_flags_grow
=for apidoc_item sv_utf8_upgrade_nomg

These convert the PV of an SV to its UTF-8-encoded form.
The SV is forced to string form if it is not already.
They always set the C<SvUTF8> flag to avoid future validity checks even if the
whole string is the same in UTF-8 as not.
They return the number of bytes in the converted string

The forms differ in just two ways.  The main difference is whether or not they
perform 'get magic' on C<sv>.  C<sv_utf8_upgrade_nomg> skips 'get magic';
C<sv_utf8_upgrade> performs it; and C<sv_utf8_upgrade_flags> and
C<sv_utf8_upgrade_flags_grow> either perform it (if the C<SV_GMAGIC> bit is set
in C<flags>) or don't (if that bit is cleared).

The other difference is that C<sv_utf8_upgrade_flags_grow> has an additional
parameter, C<extra>, which allows the caller to specify an amount of space to
be reserved as spare beyond what is needed for the actual conversion.  This is
used when the caller knows it will soon be needing yet more space, and it is
more efficient to request space from the system in a single call.
This form is otherwise identical to C<sv_utf8_upgrade_flags>.

These are not a general purpose byte encoding to Unicode interface: use the
Encode extension for that.

The C<SV_FORCE_UTF8_UPGRADE> flag is now ignored.

=for apidoc Amnh||SV_GMAGIC|
=for apidoc Amnh||SV_FORCE_UTF8_UPGRADE|

=cut

If the routine itself changes the string, it adds a trailing C<NUL>.  Such a
C<NUL> isn't guaranteed due to having other routines do the work in some input
cases, or if the input is already flagged as being in utf8.

*/

STRLEN
Perl_sv_utf8_upgrade_flags_grow(pTHX_ SV *const sv, const I32 flags, STRLEN extra)
{
    PERL_ARGS_ASSERT_SV_UTF8_UPGRADE_FLAGS_GROW;

    if (sv == &PL_sv_undef)
        return 0;
    if (!SvPOK_nog(sv)) {
        STRLEN len = 0;
        if (SvREADONLY(sv) && (SvPOKp(sv) || SvIOKp(sv) || SvNOKp(sv))) {
            (void) sv_2pv_flags(sv,&len, flags);
            if (SvUTF8(sv)) {
                if (extra) SvGROW(sv, SvCUR(sv) + extra);
                return len;
            }
        } else {
            (void) SvPV_force_flags(sv,len,flags & SV_GMAGIC);
        }
    }

    /* SVt_REGEXP's shouldn't be upgraded to UTF8 - they're already
     * compiled and individual nodes will remain non-utf8 even if the
     * stringified version of the pattern gets upgraded. Whether the
     * PVX of a REGEXP should be grown or we should just croak, I don't
     * know - DAPM */
    if (SvUTF8(sv) || isREGEXP(sv)) {
        if (extra) SvGROW(sv, SvCUR(sv) + extra);
        return SvCUR(sv);
    }

    if (SvIsCOW(sv)) {
        S_sv_uncow(aTHX_ sv, 0);
    }

    if (SvCUR(sv) == 0) {
        if (extra) SvGROW(sv, extra + 1); /* Make sure is room for a trailing
                                             byte */
    } else { /* Assume Latin-1/EBCDIC */
        /* This function could be much more efficient if we
         * had a FLAG in SVs to signal if there are any variant
         * chars in the PV.  Given that there isn't such a flag
         * make the loop as fast as possible. */
        U8 * s = (U8 *) SvPVX_const(sv);
        U8 *t = s;

        if (is_utf8_invariant_string_loc(s, SvCUR(sv), (const U8 **) &t)) {

            /* utf8 conversion not needed because all are invariants.  Mark
             * as UTF-8 even if no variant - saves scanning loop */
            SvUTF8_on(sv);
            if (extra) SvGROW(sv, SvCUR(sv) + extra);
            return SvCUR(sv);
        }

        /* Here, there is at least one variant (t points to the first one), so
         * the string should be converted to utf8.  Everything from 's' to
         * 't - 1' will occupy only 1 byte each on output.
         *
         * Note that the incoming SV may not have a trailing '\0', as certain
         * code in pp_formline can send us partially built SVs.
         *
         * There are two main ways to convert.  One is to create a new string
         * and go through the input starting from the beginning, appending each
         * converted value onto the new string as we go along.  Going this
         * route, it's probably best to initially allocate enough space in the
         * string rather than possibly running out of space and having to
         * reallocate and then copy what we've done so far.  Since everything
         * from 's' to 't - 1' is invariant, the destination can be initialized
         * with these using a fast memory copy.  To be sure to allocate enough
         * space, one could use the worst case scenario, where every remaining
         * byte expands to two under UTF-8, or one could parse it and count
         * exactly how many do expand.
         *
         * The other way is to unconditionally parse the remainder of the
         * string to figure out exactly how big the expanded string will be,
         * growing if needed.  Then start at the end of the string and place
         * the character there at the end of the unfilled space in the expanded
         * one, working backwards until reaching 't'.
         *
         * The problem with assuming the worst case scenario is that for very
         * long strings, we could allocate much more memory than actually
         * needed, which can create performance problems.  If we have to parse
         * anyway, the second method is the winner as it may avoid an extra
         * copy.  The code used to use the first method under some
         * circumstances, but now that there is faster variant counting on
         * ASCII platforms, the second method is used exclusively, eliminating
         * some code that no longer has to be maintained. */

        {
            /* Count the total number of variants there are.  We can start
             * just beyond the first one, which is known to be at 't' */
            const Size_t invariant_length = t - s;
            U8 * e = (U8 *) SvEND(sv);

            /* The length of the left overs, plus 1. */
            const Size_t remaining_length_p1 = e - t;

            /* We expand by 1 for the variant at 't' and one for each remaining
             * variant (we start looking at 't+1') */
            Size_t expansion = 1 + variant_under_utf8_count(t + 1, e);

            /* +1 = trailing NUL */
            Size_t need = SvCUR(sv) + expansion + extra + 1;
            U8 * d;

            /* Grow if needed */
            if (SvLEN(sv) < need) {
                t = invariant_length + (U8*) SvGROW(sv, need);
                e = t + remaining_length_p1;
            }
            SvCUR_set(sv, invariant_length + remaining_length_p1 + expansion);

            /* Set the NUL at the end */
            d = (U8 *) SvEND(sv);
            *d-- = '\0';

            /* Having decremented d, it points to the position to put the
             * very last byte of the expanded string.  Go backwards through
             * the string, copying and expanding as we go, stopping when we
             * get to the part that is invariant the rest of the way down */

            e--;
            while (e >= t) {
                if (NATIVE_BYTE_IS_INVARIANT(*e)) {
                    *d-- = *e;
                } else {
                    *d-- = UTF8_EIGHT_BIT_LO(*e);
                    *d-- = UTF8_EIGHT_BIT_HI(*e);
                }
                e--;
            }

            if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
                /* Update pos. We do it at the end rather than during
                 * the upgrade, to avoid slowing down the common case
                 * (upgrade without pos).
                 * pos can be stored as either bytes or characters.  Since
                 * this was previously a byte string we can just turn off
                 * the bytes flag. */
                MAGIC * mg = mg_find(sv, PERL_MAGIC_regex_global);
                if (mg) {
                    mg->mg_flags &= ~MGf_BYTES;
                }
                if ((mg = mg_find(sv, PERL_MAGIC_utf8)))
                    magic_setutf8(sv,mg); /* clear UTF8 cache */
            }
        }
    }

    SvUTF8_on(sv);
    return SvCUR(sv);
}

/*
=for apidoc sv_utf8_downgrade
=for apidoc_item sv_utf8_downgrade_flags
=for apidoc_item sv_utf8_downgrade_nomg

These attempt to convert the PV of an SV from characters to bytes.  If the PV
contains a character that cannot fit in a byte, this conversion will fail; in
this case, C<FALSE> is returned if C<fail_ok> is true; otherwise they croak.

They are not a general purpose Unicode to byte encoding interface:
use the C<Encode> extension for that.

They differ only in that:

C<sv_utf8_downgrade> processes 'get' magic on C<sv>.

C<sv_utf8_downgrade_nomg> does not.

C<sv_utf8_downgrade_flags> has an additional C<flags> parameter in which you can specify
C<SV_GMAGIC> to process 'get' magic, or leave it cleared to not process 'get' magic.

=cut
*/

bool
Perl_sv_utf8_downgrade_flags(pTHX_ SV *const sv, const bool fail_ok, const U32 flags)
{
    PERL_ARGS_ASSERT_SV_UTF8_DOWNGRADE_FLAGS;

    if (SvPOKp(sv) && SvUTF8(sv)) {
        if (SvCUR(sv)) {
            U8 *s;
            STRLEN len;
            U32 mg_flags = flags & SV_GMAGIC;

            if (SvIsCOW(sv)) {
                S_sv_uncow(aTHX_ sv, 0);
            }
            if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
                /* update pos */
                MAGIC * mg = mg_find(sv, PERL_MAGIC_regex_global);
                if (mg && mg->mg_len > 0 && mg->mg_flags & MGf_BYTES) {
                        mg->mg_len = sv_pos_b2u_flags(sv, mg->mg_len,
                                                mg_flags|SV_CONST_RETURN);
                        mg_flags = 0; /* sv_pos_b2u does get magic */
                }
                if ((mg = mg_find(sv, PERL_MAGIC_utf8)))
                    magic_setutf8(sv,mg); /* clear UTF8 cache */

            }
            s = (U8 *) SvPV_flags(sv, len, mg_flags);

            if (!utf8_to_bytes(s, &len)) {
                if (fail_ok)
                    return FALSE;
                else {
                    if (PL_op)
                        Perl_croak(aTHX_ "Wide character in %s",
                                   OP_DESC(PL_op));
                    else
                        Perl_croak(aTHX_ "Wide character");
                }
            }
            SvCUR_set(sv, len);
        }
    }
    SvUTF8_off(sv);
    return TRUE;
}

/*
=for apidoc sv_utf8_encode

Converts the PV of an SV to UTF-8, but then turns the C<SvUTF8>
flag off so that it looks like octets again.

=cut
*/

void
Perl_sv_utf8_encode(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_UTF8_ENCODE;

    if (SvREADONLY(sv)) {
        sv_force_normal_flags(sv, 0);
    }
    (void) sv_utf8_upgrade(sv);
    SvUTF8_off(sv);
}

/*
=for apidoc sv_utf8_decode

If the PV of the SV is an octet sequence in Perl's extended UTF-8
and contains a multiple-byte character, the C<SvUTF8> flag is turned on
so that it looks like a character.  If the PV contains only single-byte
characters, the C<SvUTF8> flag stays off.
Scans PV for validity and returns FALSE if the PV is invalid UTF-8.

=cut
*/

bool
Perl_sv_utf8_decode(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_UTF8_DECODE;

    if (SvPOKp(sv)) {
        const U8 *start, *c, *first_variant;

        /* The octets may have got themselves encoded - get them back as
         * bytes
         */
        if (!sv_utf8_downgrade(sv, TRUE))
            return FALSE;

        /* it is actually just a matter of turning the utf8 flag on, but
         * we want to make sure everything inside is valid utf8 first.
         */
        c = start = (const U8 *) SvPVX_const(sv);
        if (! is_utf8_invariant_string_loc(c, SvCUR(sv), &first_variant)) {
            if (!is_utf8_string(first_variant, SvCUR(sv) - (first_variant -c)))
                return FALSE;
            SvUTF8_on(sv);
        }
        if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
            /* XXX Is this dead code?  XS_utf8_decode calls SvSETMAGIC
                   after this, clearing pos.  Does anything on CPAN
                   need this? */
            /* adjust pos to the start of a UTF8 char sequence */
            MAGIC * mg = mg_find(sv, PERL_MAGIC_regex_global);
            if (mg) {
                I32 pos = mg->mg_len;
                if (pos > 0) {
                    for (c = start + pos; c > start; c--) {
                        if (UTF8_IS_START(*c))
                            break;
                    }
                    mg->mg_len  = c - start;
                }
            }
            if ((mg = mg_find(sv, PERL_MAGIC_utf8)))
                magic_setutf8(sv,mg); /* clear UTF8 cache */
        }
    }
    return TRUE;
}

/*
=for apidoc sv_setsv
=for apidoc_item sv_setsv_flags
=for apidoc_item sv_setsv_mg
=for apidoc_item sv_setsv_nomg

These copy the contents of the source SV C<ssv> into the destination SV C<dsv>.
C<ssv> may be destroyed if it is mortal, so don't use these functions if
the source SV needs to be reused.
Loosely speaking, they perform a copy-by-value, obliterating any previous
content of the destination.

They differ only in that:

C<sv_setsv> calls 'get' magic on C<ssv>, but skips 'set' magic on C<dsv>.

C<sv_setsv_mg> calls both 'get' magic on C<ssv> and 'set' magic on C<dsv>.

C<sv_setsv_nomg> skips all magic.

C<sv_setsv_flags> has a C<flags> parameter which you can use to specify any
combination of magic handling, and also you can specify C<SV_NOSTEAL> so that
the buffers of temps will not be stolen.

You probably want to instead use one of the assortment of wrappers, such as
C<L</SvSetSV>>, C<L</SvSetSV_nosteal>>, C<L</SvSetMagicSV>> and
C<L</SvSetMagicSV_nosteal>>.

C<sv_setsv_flags> is the primary function for copying scalars, and most other
copy-ish functions and macros use it underneath.

=for apidoc Amnh||SV_NOSTEAL

=cut
*/

static void
S_glob_assign_glob(pTHX_ SV *const dsv, SV *const ssv, const int dtype)
{
    I32 mro_changes = 0; /* 1 = method, 2 = isa, 3 = recursive isa */
    HV *old_stash = NULL;

    PERL_ARGS_ASSERT_GLOB_ASSIGN_GLOB;

    if (dtype != SVt_PVGV && !isGV_with_GP(dsv)) {
        const char * const name = GvNAME(ssv);
        const STRLEN len = GvNAMELEN(ssv);
        {
            if (dtype >= SVt_PV) {
                SvPV_free(dsv);
                SvPV_set(dsv, 0);
                SvLEN_set(dsv, 0);
                SvCUR_set(dsv, 0);
            }
            SvUPGRADE(dsv, SVt_PVGV);
            (void)SvOK_off(dsv);
            isGV_with_GP_on(dsv);
        }
        GvSTASH(dsv) = GvSTASH(ssv);
        if (GvSTASH(dsv))
            Perl_sv_add_backref(aTHX_ MUTABLE_SV(GvSTASH(dsv)), dsv);
        gv_name_set(MUTABLE_GV(dsv), name, len,
                        GV_ADD | (GvNAMEUTF8(ssv) ? SVf_UTF8 : 0 ));
        SvFAKE_on(dsv);	/* can coerce to non-glob */
    }

    if(GvGP(MUTABLE_GV(ssv))) {
        /* If source has method cache entry, clear it */
        if(GvCVGEN(ssv)) {
            SvREFCNT_dec(GvCV(ssv));
            GvCV_set(ssv, NULL);
            GvCVGEN(ssv) = 0;
        }
        /* If source has a real method, then a method is
           going to change */
        else if(
         GvCV((const GV *)ssv) && GvSTASH(dsv) && HvHasENAME(GvSTASH(dsv))
        ) {
            mro_changes = 1;
        }
    }

    /* If dest already had a real method, that's a change as well */
    if(
        !mro_changes && GvGP(MUTABLE_GV(dsv)) && GvCVu((const GV *)dsv)
     && GvSTASH(dsv) && HvHasENAME(GvSTASH(dsv))
    ) {
        mro_changes = 1;
    }

    /* We don't need to check the name of the destination if it was not a
       glob to begin with. */
    if(dtype == SVt_PVGV) {
        const char * const name = GvNAME((const GV *)dsv);
        const STRLEN len = GvNAMELEN(dsv);
        if(memEQs(name, len, "ISA")
         /* The stash may have been detached from the symbol table, so
            check its name. */
         && GvSTASH(dsv) && HvHasENAME(GvSTASH(dsv))
        )
            mro_changes = 2;
        else {
            if ((len > 1 && name[len-2] == ':' && name[len-1] == ':')
             || (len == 1 && name[0] == ':')) {
                mro_changes = 3;

                /* Set aside the old stash, so we can reset isa caches on
                   its subclasses. */
                if((old_stash = GvHV(dsv)))
                    /* Make sure we do not lose it early. */
                    SvREFCNT_inc_simple_void_NN(
                     sv_2mortal((SV *)old_stash)
                    );
            }
        }

        SvREFCNT_inc_simple_void_NN(sv_2mortal(dsv));
    }

    /* freeing dsv's GP might free ssv (e.g. *x = $x),
     * so temporarily protect it */
    ENTER;
    SAVEFREESV(SvREFCNT_inc_simple_NN(ssv));
    gp_free(MUTABLE_GV(dsv));
    GvINTRO_off(dsv);		/* one-shot flag */
    GvGP_set(dsv, gp_ref(GvGP(ssv)));
    LEAVE;

    if (SvTAINTED(ssv))
        SvTAINT(dsv);
    if (GvIMPORTED(dsv) != GVf_IMPORTED
        && CopSTASH_ne(PL_curcop, GvSTASH(dsv)))
        {
            GvIMPORTED_on(dsv);
        }
    GvMULTI_on(dsv);
    if(mro_changes == 2) {
      if (GvAV((const GV *)ssv)) {
        MAGIC *mg;
        SV * const sref = (SV *)GvAV((const GV *)dsv);
        if (SvSMAGICAL(sref) && (mg = mg_find(sref, PERL_MAGIC_isa))) {
            if (SvTYPE(mg->mg_obj) != SVt_PVAV) {
                AV * const ary = newAV_alloc_x(2);
                av_push_simple(ary, mg->mg_obj); /* takes the refcount */
                av_push_simple(ary, SvREFCNT_inc_simple_NN(dsv));
                mg->mg_obj = (SV *)ary;
            } else {
                av_push((AV *)mg->mg_obj, SvREFCNT_inc_simple_NN(dsv));
            }
        }
        else sv_magic(sref, dsv, PERL_MAGIC_isa, NULL, 0);
      }
      mro_isa_changed_in(GvSTASH(dsv));
    }
    else if(mro_changes == 3) {
        HV * const stash = GvHV(dsv);
        if(old_stash ? HvHasENAME(old_stash) : cBOOL(stash))
            mro_package_moved(
                stash, old_stash,
                (GV *)dsv, 0
            );
    }
    else if(mro_changes) mro_method_changed_in(GvSTASH(dsv));
    if (GvIO(dsv) && dtype == SVt_PVGV) {
        DEBUG_o(Perl_deb(aTHX_
                        "glob_assign_glob clearing PL_stashcache\n"));
        /* It's a cache. It will rebuild itself quite happily.
           It's a lot of effort to work out exactly which key (or keys)
           might be invalidated by the creation of the this file handle.
         */
        hv_clear(PL_stashcache);
    }
    return;
}

void
Perl_gv_setref(pTHX_ SV *const dsv, SV *const ssv)
{
    SV * const sref = SvRV(ssv);
    SV *dref;
    const int intro = GvINTRO(dsv);
    SV **location;
    U8 import_flag = 0;
    const U32 stype = SvTYPE(sref);

    PERL_ARGS_ASSERT_GV_SETREF;

    if (intro) {
        GvINTRO_off(dsv);	/* one-shot flag */
        GvLINE(dsv) = CopLINE(PL_curcop);
        GvEGV(dsv) = MUTABLE_GV(dsv);
    }
    GvMULTI_on(dsv);
    switch (stype) {
    case SVt_PVCV:
        location = (SV **) &(GvGP(dsv)->gp_cv); /* XXX bypassing GvCV_set */
        import_flag = GVf_IMPORTED_CV;
        goto common;
    case SVt_PVHV:
        location = (SV **) &GvHV(dsv);
        import_flag = GVf_IMPORTED_HV;
        goto common;
    case SVt_PVAV:
        location = (SV **) &GvAV(dsv);
        import_flag = GVf_IMPORTED_AV;
        goto common;
    case SVt_PVIO:
        location = (SV **) &GvIOp(dsv);
        goto common;
    case SVt_PVFM:
        location = (SV **) &GvFORM(dsv);
        goto common;
    default:
        location = &GvSV(dsv);
        import_flag = GVf_IMPORTED_SV;
    common:
        if (intro) {
            if (stype == SVt_PVCV) {
                /*if (GvCVGEN(dsv) && (GvCV(dsv) != (const CV *)sref || GvCVGEN(dsv))) {*/
                if (GvCVGEN(dsv)) {
                    SvREFCNT_dec(GvCV(dsv));
                    GvCV_set(dsv, NULL);
                    GvCVGEN(dsv) = 0; /* Switch off cacheness. */
                }
            }
            /* SAVEt_GVSLOT takes more room on the savestack and has more
               overhead in leave_scope than SAVEt_GENERIC_SV.  But for CVs
               leave_scope needs access to the GV so it can reset method
               caches.  We must use SAVEt_GVSLOT whenever the type is
               SVt_PVCV, even if the stash is anonymous, as the stash may
               gain a name somehow before leave_scope. */
            if (stype == SVt_PVCV) {
                /* There is no save_pushptrptrptr.  Creating it for this
                   one call site would be overkill.  So inline the ss add
                   routines here. */
                dSS_ADD;
                SS_ADD_PTR(dsv);
                SS_ADD_PTR(location);
                SS_ADD_PTR(SvREFCNT_inc(*location));
                SS_ADD_UV(SAVEt_GVSLOT);
                SS_ADD_END(4);
            }
            else SAVEGENERICSV(*location);
        }
        dref = *location;
        if (stype == SVt_PVCV && (*location != sref || GvCVGEN(dsv))) {
            CV* const cv = MUTABLE_CV(*location);
            if (cv) {
                if (!GvCVGEN((const GV *)dsv) &&
                    (CvROOT(cv) || CvXSUB(cv)) &&
                    /* redundant check that avoids creating the extra SV
                       most of the time: */
                    (CvCONST(cv) || (ckWARN(WARN_REDEFINE) && !intro)))
                    {
                        SV * const new_const_sv =
                            CvCONST((const CV *)sref)
                                 ? cv_const_sv_or_av((const CV *)sref)
                                 : NULL;
                        HV * const stash = GvSTASH((const GV *)dsv);
                        report_redefined_cv(
                           sv_2mortal(
                             stash
                               ? Perl_newSVpvf(aTHX_
                                    "%" HEKf "::%" HEKf,
                                    HEKfARG(HvNAME_HEK(stash)),
                                    HEKfARG(GvENAME_HEK(MUTABLE_GV(dsv))))
                               : Perl_newSVpvf(aTHX_
                                    "%" HEKf,
                                    HEKfARG(GvENAME_HEK(MUTABLE_GV(dsv))))
                           ),
                           cv,
                           CvCONST((const CV *)sref) ? &new_const_sv : NULL
                        );
                    }
                if (!intro)
                    cv_ckproto_len_flags(cv, (const GV *)dsv,
                                   SvPOK(sref) ? CvPROTO(sref) : NULL,
                                   SvPOK(sref) ? CvPROTOLEN(sref) : 0,
                                   SvPOK(sref) ? SvUTF8(sref) : 0);
            }
            GvCVGEN(dsv) = 0; /* Switch off cacheness. */
            GvASSUMECV_on(dsv);
            if(GvSTASH(dsv)) { /* sub foo { 1 } sub bar { 2 } *bar = \&foo */
                if (intro && GvREFCNT(dsv) > 1) {
                    /* temporary remove extra savestack's ref */
                    --GvREFCNT(dsv);
                    gv_method_changed(dsv);
                    ++GvREFCNT(dsv);
                }
                else gv_method_changed(dsv);
            }
        }
        *location = SvREFCNT_inc_simple_NN(sref);
        if (import_flag && !(GvFLAGS(dsv) & import_flag)
            && CopSTASH_ne(PL_curcop, GvSTASH(dsv))) {
            GvFLAGS(dsv) |= import_flag;
        }

        if (stype == SVt_PVHV) {
            const char * const name = GvNAME((GV*)dsv);
            const STRLEN len = GvNAMELEN(dsv);
            if (
                (
                   (len > 1 && name[len-2] == ':' && name[len-1] == ':')
                || (len == 1 && name[0] == ':')
                )
             && (!dref || HvHasENAME(dref))
            ) {
                mro_package_moved(
                    (HV *)sref, (HV *)dref,
                    (GV *)dsv, 0
                );
            }
        }
        else if (
            stype == SVt_PVAV && sref != dref
         && memEQs(GvNAME((GV*)dsv), GvNAMELEN((GV*)dsv), "ISA")
         /* The stash may have been detached from the symbol table, so
            check its name before doing anything. */
         && GvSTASH(dsv) && HvHasENAME(GvSTASH(dsv))
        ) {
            MAGIC *mg;
            MAGIC * const omg = dref && SvSMAGICAL(dref)
                                 ? mg_find(dref, PERL_MAGIC_isa)
                                 : NULL;
            if (SvSMAGICAL(sref) && (mg = mg_find(sref, PERL_MAGIC_isa))) {
                if (SvTYPE(mg->mg_obj) != SVt_PVAV) {
                    AV * const ary = newAV_alloc_xz(4);
                    av_push_simple(ary, mg->mg_obj); /* takes the refcount */
                    mg->mg_obj = (SV *)ary;
                }
                if (omg) {
                    if (SvTYPE(omg->mg_obj) == SVt_PVAV) {
                        SV **svp = AvARRAY((AV *)omg->mg_obj);
                        I32 items = AvFILLp((AV *)omg->mg_obj) + 1;
                        while (items--)
                            av_push(
                             (AV *)mg->mg_obj,
                             SvREFCNT_inc_simple_NN(*svp++)
                            );
                    }
                    else
                        av_push(
                         (AV *)mg->mg_obj,
                         SvREFCNT_inc_simple_NN(omg->mg_obj)
                        );
                }
                else
                    av_push((AV *)mg->mg_obj,SvREFCNT_inc_simple_NN(dsv));
            }
            else
            {
                SSize_t i;
                sv_magic(
                 sref, omg ? omg->mg_obj : dsv, PERL_MAGIC_isa, NULL, 0
                );
                for (i = 0; i <= AvFILL(sref); ++i) {
                    SV **elem = av_fetch ((AV*)sref, i, 0);
                    if (elem) {
                        sv_magic(
                          *elem, sref, PERL_MAGIC_isaelem, NULL, i
                        );
                    }
                }
                mg = mg_find(sref, PERL_MAGIC_isa);
            }
            /* Since the *ISA assignment could have affected more than
               one stash, don't call mro_isa_changed_in directly, but let
               magic_clearisa do it for us, as it already has the logic for
               dealing with globs vs arrays of globs. */
            assert(mg);
            Perl_magic_clearisa(aTHX_ NULL, mg);
        }
        else if (stype == SVt_PVIO) {
            DEBUG_o(Perl_deb(aTHX_ "gv_setref clearing PL_stashcache\n"));
            /* It's a cache. It will rebuild itself quite happily.
               It's a lot of effort to work out exactly which key (or keys)
               might be invalidated by the creation of the this file handle.
            */
            hv_clear(PL_stashcache);
        }
        break;
    }
    if (!intro) SvREFCNT_dec(dref);
    if (SvTAINTED(ssv))
        SvTAINT(dsv);
    return;
}




#ifdef PERL_DEBUG_READONLY_COW
# include <sys/mman.h>

# ifndef PERL_MEMORY_DEBUG_HEADER_SIZE
#  define PERL_MEMORY_DEBUG_HEADER_SIZE 0
# endif

void
Perl_sv_buf_to_ro(pTHX_ SV *sv)
{
    struct perl_memory_debug_header * const header =
        (struct perl_memory_debug_header *)(SvPVX(sv)-PERL_MEMORY_DEBUG_HEADER_SIZE);
    const MEM_SIZE len = header->size;
    PERL_ARGS_ASSERT_SV_BUF_TO_RO;
# ifdef PERL_TRACK_MEMPOOL
    if (!header->readonly) header->readonly = 1;
# endif
    if (mprotect(header, len, PROT_READ))
        Perl_warn(aTHX_ "mprotect RW for COW string %p %lu failed with %d",
                         header, len, errno);
}

static void
S_sv_buf_to_rw(pTHX_ SV *sv)
{
    struct perl_memory_debug_header * const header =
        (struct perl_memory_debug_header *)(SvPVX(sv)-PERL_MEMORY_DEBUG_HEADER_SIZE);
    const MEM_SIZE len = header->size;
    PERL_ARGS_ASSERT_SV_BUF_TO_RW;
    if (mprotect(header, len, PROT_READ|PROT_WRITE))
        Perl_warn(aTHX_ "mprotect for COW string %p %lu failed with %d",
                         header, len, errno);
# ifdef PERL_TRACK_MEMPOOL
    header->readonly = 0;
# endif
}

#else
# define sv_buf_to_ro(sv)	NOOP
# define sv_buf_to_rw(sv)	NOOP
#endif

void
Perl_sv_setsv_flags(pTHX_ SV *dsv, SV* ssv, const I32 flags)
{
    U32 sflags;
    int dtype;
    svtype stype;
    unsigned int both_type;

    PERL_ARGS_ASSERT_SV_SETSV_FLAGS;

    if (UNLIKELY( ssv == dsv ))
        return;

    if (UNLIKELY( !ssv ))
        ssv = &PL_sv_undef;

    stype = SvTYPE(ssv);
    dtype = SvTYPE(dsv);
    both_type = (stype | dtype);

    /* with these values, we can check that both SVs are NULL/IV (and not
     * freed) just by testing the or'ed types */
    STATIC_ASSERT_STMT(SVt_NULL == 0);
    STATIC_ASSERT_STMT(SVt_IV   == 1);
    STATIC_ASSERT_STMT(SVt_NV   == 2);
#if NVSIZE <= IVSIZE
    if (both_type <= 2) {
#else
    if (both_type <= 1) {
#endif
        /* both src and dst are UNDEF/IV/RV - maybe NV depending on config,
         * so we can do a lot of special-casing */
        U32 sflags;
        U32 new_dflags;
        SV *old_rv = NULL;

        /* minimal subset of SV_CHECK_THINKFIRST_COW_DROP(dsv) */
        if (SvREADONLY(dsv))
            Perl_croak_no_modify();
        if (SvROK(dsv)) {
            if (SvWEAKREF(dsv))
                sv_unref_flags(dsv, 0);
            else
                old_rv = SvRV(dsv);
            SvROK_off(dsv);
        }

        assert(!SvGMAGICAL(ssv));
        assert(!SvGMAGICAL(dsv));

        sflags = SvFLAGS(ssv);
        if (sflags & (SVf_IOK|SVf_ROK)) {
            SET_SVANY_FOR_BODYLESS_IV(dsv);
            new_dflags = SVt_IV;

            if (sflags & SVf_ROK) {
                dsv->sv_u.svu_rv = SvREFCNT_inc(SvRV(ssv));
                new_dflags |= SVf_ROK;
            }
            else {
                /* both src and dst are <= SVt_IV, so sv_any points to the
                 * head; so access the head directly
                 */
                assert(    &(ssv->sv_u.svu_iv)
                        == &(((XPVIV*) SvANY(ssv))->xiv_iv));
                assert(    &(dsv->sv_u.svu_iv)
                        == &(((XPVIV*) SvANY(dsv))->xiv_iv));
                dsv->sv_u.svu_iv = ssv->sv_u.svu_iv;
                new_dflags |= (SVf_IOK|SVp_IOK|(sflags & SVf_IVisUV));
            }
        }
#if NVSIZE <= IVSIZE
        else if (sflags & SVf_NOK) {
            SET_SVANY_FOR_BODYLESS_NV(dsv);
            new_dflags = (SVt_NV|SVf_NOK|SVp_NOK);

            /* both src and dst are <= SVt_MV, so sv_any points to the
             * head; so access the head directly
             */
            assert(    &(ssv->sv_u.svu_nv)
                    == &(((XPVNV*) SvANY(ssv))->xnv_u.xnv_nv));
            assert(    &(dsv->sv_u.svu_nv)
                    == &(((XPVNV*) SvANY(dsv))->xnv_u.xnv_nv));
            dsv->sv_u.svu_nv = ssv->sv_u.svu_nv;
        }
#endif
        else {
            new_dflags = dtype; /* turn off everything except the type */
        }
        /* Should preserve some dsv flags - at least SVs_TEMP, */
        /* so cannot just set SvFLAGS(dsv) = new_dflags        */
        /* First clear the flags that we do want to clobber    */
        (void)SvOK_off(dsv);
        SvFLAGS(dsv) &= ~SVTYPEMASK;
        /* Now set the new flags */
        SvFLAGS(dsv) |= new_dflags;

        SvREFCNT_dec(old_rv);
        return;
    }

    if (UNLIKELY(both_type == SVTYPEMASK)) {
        if (SvIS_FREED(dsv)) {
            Perl_croak(aTHX_ "panic: attempt to copy value %" SVf
                       " to a freed scalar %p", SVfARG(ssv), (void *)dsv);
        }
        if (SvIS_FREED(ssv)) {
            Perl_croak(aTHX_ "panic: attempt to copy freed scalar %p to %p",
                       (void*)ssv, (void*)dsv);
        }
    }



    SV_CHECK_THINKFIRST_COW_DROP(dsv);
    dtype = SvTYPE(dsv); /* THINKFIRST may have changed type */

    /* There's a lot of redundancy below but we're going for speed here
     * Note: some of the cases below do return; rather than break; so the
     * if-elseif-else logic below this switch does not see all cases. */

    switch (stype) {
    case SVt_NULL:
      undef_sstr:
        if (LIKELY( dtype != SVt_PVGV && dtype != SVt_PVLV )) {
            (void)SvOK_off(dsv);
            return;
        }
        break;
    case SVt_IV:
        if (SvIOK(ssv)) {
            switch (dtype) {
            case SVt_NULL:
                /* For performance, we inline promoting to type SVt_IV. */
                /* We're starting from SVt_NULL, so provided that define is
                 * actual 0, we don't have to unset any SV type flags
                 * to promote to SVt_IV. */
                STATIC_ASSERT_STMT(SVt_NULL == 0);
                SET_SVANY_FOR_BODYLESS_IV(dsv);
                SvFLAGS(dsv) |= SVt_IV;
                break;
            case SVt_NV:
            case SVt_PV:
                sv_upgrade(dsv, SVt_PVIV);
                break;
            case SVt_PVGV:
            case SVt_PVLV:
                goto end_of_first_switch;
            }
            (void)SvIOK_only(dsv);
            SvIV_set(dsv,  SvIVX(ssv));
            if (SvIsUV(ssv))
                SvIsUV_on(dsv);
            /* SvTAINTED can only be true if the SV has taint magic, which in
               turn means that the SV type is PVMG (or greater). This is the
               case statement for SVt_IV, so this cannot be true (whatever gcov
               may say).  */
            assert(!SvTAINTED(ssv));
            return;
        }
        if (!SvROK(ssv))
            goto undef_sstr;
        if (dtype < SVt_PV && dtype != SVt_IV)
            sv_upgrade(dsv, SVt_IV);
        break;

    case SVt_NV:
        if (LIKELY( SvNOK(ssv) )) {
            switch (dtype) {
            case SVt_NULL:
            case SVt_IV:
                sv_upgrade(dsv, SVt_NV);
                break;
            case SVt_PV:
            case SVt_PVIV:
                sv_upgrade(dsv, SVt_PVNV);
                break;
            case SVt_PVGV:
            case SVt_PVLV:
                goto end_of_first_switch;
            }
            SvNV_set(dsv, SvNVX(ssv));
            (void)SvNOK_only(dsv);
            /* SvTAINTED can only be true if the SV has taint magic, which in
               turn means that the SV type is PVMG (or greater). This is the
               case statement for SVt_NV, so this cannot be true (whatever gcov
               may say).  */
            assert(!SvTAINTED(ssv));
            return;
        }
        goto undef_sstr;

    case SVt_PV:
        if (dtype < SVt_PV)
            sv_upgrade(dsv, SVt_PV);
        break;
    case SVt_PVIV:
        if (dtype < SVt_PVIV)
            sv_upgrade(dsv, SVt_PVIV);
        break;
    case SVt_PVNV:
        if (dtype < SVt_PVNV)
            sv_upgrade(dsv, SVt_PVNV);
        break;

    case SVt_INVLIST:
        invlist_clone(ssv, dsv);
        return;
    default:
        {
        const char * const type = sv_reftype(ssv,0);
        if (PL_op)
            /* diag_listed_as: Bizarre copy of %s */
            Perl_croak(aTHX_ "Bizarre copy of %s in %s", type, OP_DESC(PL_op));
        else
            Perl_croak(aTHX_ "Bizarre copy of %s", type);
        }
        NOT_REACHED; /* NOTREACHED */

    case SVt_REGEXP:
      upgregexp:
        if (dtype < SVt_REGEXP)
            sv_upgrade(dsv, SVt_REGEXP);
        break;

    case SVt_PVLV:
    case SVt_PVGV:
    case SVt_PVMG:
        if (SvGMAGICAL(ssv) && (flags & SV_GMAGIC)) {
            mg_get(ssv);
            if (SvTYPE(ssv) != stype)
                stype = SvTYPE(ssv);
        }
        if (isGV_with_GP(ssv) && dtype <= SVt_PVLV) {
                    glob_assign_glob(dsv, ssv, dtype);
                    return;
        }
        if (stype == SVt_PVLV)
        {
            if (isREGEXP(ssv)) goto upgregexp;
            SvUPGRADE(dsv, SVt_PVNV);
        }
        else
            SvUPGRADE(dsv, (svtype)stype);
    }
 end_of_first_switch:

    /* dsv may have been upgraded.  */
    dtype = SvTYPE(dsv);
    sflags = SvFLAGS(ssv);

    if (UNLIKELY( dtype == SVt_PVCV )) {
        /* Assigning to a subroutine sets the prototype.  */
        if (SvOK(ssv)) {
            STRLEN len;
            const char *const ptr = SvPV_const(ssv, len);

            SvGROW(dsv, len + 1);
            Copy(ptr, SvPVX(dsv), len + 1, char);
            SvCUR_set(dsv, len);
            SvPOK_only(dsv);
            SvFLAGS(dsv) |= sflags & SVf_UTF8;
            CvAUTOLOAD_off(dsv);
        } else {
            SvOK_off(dsv);
        }
    }
    else if (UNLIKELY(dtype == SVt_PVAV || dtype == SVt_PVHV
             || dtype == SVt_PVFM))
    {
        const char * const type = sv_reftype(dsv,0);
        if (PL_op)
            /* diag_listed_as: Cannot copy to %s */
            Perl_croak(aTHX_ "Cannot copy to %s in %s", type, OP_DESC(PL_op));
        else
            Perl_croak(aTHX_ "Cannot copy to %s", type);
    } else if (sflags & SVf_ROK) {
        if (isGV_with_GP(dsv)
            && SvTYPE(SvRV(ssv)) == SVt_PVGV && isGV_with_GP(SvRV(ssv))) {
            ssv = SvRV(ssv);
            if (ssv == dsv) {
                if (GvIMPORTED(dsv) != GVf_IMPORTED
                    && CopSTASH_ne(PL_curcop, GvSTASH(dsv)))
                {
                    GvIMPORTED_on(dsv);
                }
                GvMULTI_on(dsv);
                return;
            }
            glob_assign_glob(dsv, ssv, dtype);
            return;
        }

        if (dtype >= SVt_PV) {
            if (isGV_with_GP(dsv)) {
                gv_setref(dsv, ssv);
                return;
            }
            if (SvPVX_const(dsv)) {
                SvPV_free(dsv);
                SvLEN_set(dsv, 0);
                SvCUR_set(dsv, 0);
            }
        }
        (void)SvOK_off(dsv);
        SvRV_set(dsv, SvREFCNT_inc(SvRV(ssv)));
        SvFLAGS(dsv) |= sflags & SVf_ROK;
        assert(!(sflags & SVp_NOK));
        assert(!(sflags & SVp_IOK));
        assert(!(sflags & SVf_NOK));
        assert(!(sflags & SVf_IOK));
    }
    else if (isGV_with_GP(dsv)) {
        if (!(sflags & SVf_OK)) {
            Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
                           "Undefined value assigned to typeglob");
        }
        else {
            GV *gv = gv_fetchsv_nomg(ssv, GV_ADD, SVt_PVGV);
            if (dsv != (const SV *)gv) {
                const char * const name = GvNAME((const GV *)dsv);
                const STRLEN len = GvNAMELEN(dsv);
                HV *old_stash = NULL;
                bool reset_isa = FALSE;
                if ((len > 1 && name[len-2] == ':' && name[len-1] == ':')
                 || (len == 1 && name[0] == ':')) {
                    /* Set aside the old stash, so we can reset isa caches
                       on its subclasses. */
                    if((old_stash = GvHV(dsv))) {
                        /* Make sure we do not lose it early. */
                        SvREFCNT_inc_simple_void_NN(
                         sv_2mortal((SV *)old_stash)
                        );
                    }
                    reset_isa = TRUE;
                }

                if (GvGP(dsv)) {
                    SvREFCNT_inc_simple_void_NN(sv_2mortal(dsv));
                    gp_free(MUTABLE_GV(dsv));
                }
                GvGP_set(dsv, gp_ref(GvGP(gv)));

                if (reset_isa) {
                    HV * const stash = GvHV(dsv);
                    if(
                        old_stash ? HvHasENAME(old_stash) : cBOOL(stash)
                    )
                        mro_package_moved(
                         stash, old_stash,
                         (GV *)dsv, 0
                        );
                }
            }
        }
    }
    else if ((dtype == SVt_REGEXP || dtype == SVt_PVLV)
          && (stype == SVt_REGEXP || isREGEXP(ssv))) {
        reg_temp_copy((REGEXP*)dsv, (REGEXP*)ssv);
    }
    else if (sflags & SVp_POK) {
        const STRLEN cur = SvCUR(ssv);
        const STRLEN len = SvLEN(ssv);

        /*
         * We have three basic ways to copy the string:
         *
         *  1. Swipe
         *  2. Copy-on-write
         *  3. Actual copy
         *
         * Which we choose is based on various factors.  The following
         * things are listed in order of speed, fastest to slowest:
         *  - Swipe
         *  - Copying a short string
         *  - Copy-on-write bookkeeping
         *  - malloc
         *  - Copying a long string
         *
         * We swipe the string (steal the string buffer) if the SV on the
         * rhs is about to be freed anyway (TEMP and refcnt==1).  This is a
         * big win on long strings.  It should be a win on short strings if
         * SvPVX_const(dsv) has to be allocated.  If not, it should not
         * slow things down, as SvPVX_const(ssv) would have been freed
         * soon anyway.
         *
         * We also steal the buffer from a PADTMP (operator target) if it
         * is ‘long enough’.  For short strings, a swipe does not help
         * here, as it causes more malloc calls the next time the target
         * is used.  Benchmarks show that even if SvPVX_const(dsv) has to
         * be allocated it is still not worth swiping PADTMPs for short
         * strings, as the savings here are small.
         *
         * If swiping is not an option, then we see whether it is worth using
         * copy-on-write.  If the lhs already has a buffer big enough and the
         * string is short, we skip it and fall back to method 3, since memcpy
         * is faster for short strings than the later bookkeeping overhead that
         * copy-on-write entails.

         * If the rhs is not a copy-on-write string yet, then we also
         * consider whether the buffer is too large relative to the string
         * it holds.  Some operations such as readline allocate a large
         * buffer in the expectation of reusing it.  But turning such into
         * a COW buffer is counter-productive because it increases memory
         * usage by making readline allocate a new large buffer the sec-
         * ond time round.  So, if the buffer is too large, again, we use
         * method 3 (copy).
         *
         * Finally, if there is no buffer on the left, or the buffer is too
         * small, then we use copy-on-write and make both SVs share the
         * string buffer.
         *
         */

        /* Whichever path we take through the next code, we want this true,
           and doing it now facilitates the COW check.  */
        (void)SvPOK_only(dsv);

        if (
                 (              /* Either ... */
                                /* slated for free anyway (and not COW)? */
                    (sflags & (SVs_TEMP|SVf_IsCOW)) == SVs_TEMP
                                /* or a swipable TARG */
                 || ((sflags &
                           (SVs_PADTMP|SVf_READONLY|SVf_PROTECT|SVf_IsCOW))
                       == SVs_PADTMP
                                /* whose buffer is worth stealing */
                     && CHECK_COWBUF_THRESHOLD(cur,len)
                    )
                 ) &&
                 !(sflags & SVf_OOK) &&   /* and not involved in OOK hack? */
                 (!(flags & SV_NOSTEAL)) &&
                                        /* and we're allowed to steal temps */
                 SvREFCNT(ssv) == 1 &&   /* and no other references to it? */
                 len)             /* and really is a string */
        {	/* Passes the swipe test.  */
            if (SvPVX_const(dsv))	/* we know that dtype >= SVt_PV */
                SvPV_free(dsv);
            SvPV_set(dsv, SvPVX_mutable(ssv));
            SvLEN_set(dsv, SvLEN(ssv));
            SvCUR_set(dsv, SvCUR(ssv));

            SvTEMP_off(dsv);
            (void)SvOK_off(ssv);	/* NOTE: nukes most SvFLAGS on ssv */
            SvPV_set(ssv, NULL);
            SvLEN_set(ssv, 0);
            SvCUR_set(ssv, 0);
            SvTEMP_off(ssv);
        }
        /* We must check for SvIsCOW_static() even without
         * SV_COW_SHARED_HASH_KEYS being set or else we'll break SvIsBOOL()
         */
        else if (SvIsCOW_static(ssv)) {
            if (SvPVX_const(dsv)) {     /* we know that dtype >= SVt_PV */
                SvPV_free(dsv);
            }
            SvPV_set(dsv, SvPVX(ssv));
            SvLEN_set(dsv, 0);
            SvCUR_set(dsv, cur);
            SvFLAGS(dsv) |= (SVf_IsCOW|SVppv_STATIC);
        }
        else if (flags & SV_COW_SHARED_HASH_KEYS
              &&
#ifdef PERL_COPY_ON_WRITE
                 (sflags & SVf_IsCOW
                   ? (!len ||
                       (  (CHECK_COWBUF_THRESHOLD(cur,len) || SvLEN(dsv) < cur+1)
                          /* If this is a regular (non-hek) COW, only so
                             many COW "copies" are possible. */
                       && CowREFCNT(ssv) != SV_COW_REFCNT_MAX  ))
                   : (  (sflags & CAN_COW_MASK) == CAN_COW_FLAGS
                     && !(SvFLAGS(dsv) & SVf_BREAK)
                     && CHECK_COW_THRESHOLD(cur,len) && cur+1 < len
                     && (CHECK_COWBUF_THRESHOLD(cur,len) || SvLEN(dsv) < cur+1)
                    ))
#else
                 sflags & SVf_IsCOW
              && !(SvFLAGS(dsv) & SVf_BREAK)
#endif
            ) {
            /* Either it's a shared hash key, or it's suitable for
               copy-on-write.  */
#ifdef DEBUGGING
            if (DEBUG_C_TEST) {
                PerlIO_printf(Perl_debug_log, "Copy on write: ssv --> dsv\n");
                sv_dump(ssv);
                sv_dump(dsv);
            }
#endif
#ifdef PERL_ANY_COW
            if (!(sflags & SVf_IsCOW)) {
                    SvIsCOW_on(ssv);
                    CowREFCNT(ssv) = 0;
            }
#endif
            if (SvPVX_const(dsv)) {	/* we know that dtype >= SVt_PV */
                SvPV_free(dsv);
            }

#ifdef PERL_ANY_COW
            if (len) {
                    if (sflags & SVf_IsCOW) {
                        sv_buf_to_rw(ssv);
                    }
                    CowREFCNT(ssv)++;
                    SvPV_set(dsv, SvPVX_mutable(ssv));
                    sv_buf_to_ro(ssv);
            } else
#endif
            {
                    /* SvIsCOW_shared_hash */
                    DEBUG_C(PerlIO_printf(Perl_debug_log,
                                          "Copy on write: Sharing hash\n"));

                    assert (SvTYPE(dsv) >= SVt_PV);
                    SvPV_set(dsv,
                             HEK_KEY(share_hek_hek(SvSHARED_HEK_FROM_PV(SvPVX_const(ssv)))));
            }
            SvLEN_set(dsv, len);
            SvCUR_set(dsv, cur);
            SvIsCOW_on(dsv);
        } else {
            /* Failed the swipe test, and we cannot do copy-on-write either.
               Have to copy the string.  */
            SvGROW(dsv, cur + 1);	/* inlined from sv_setpvn */
            Move(SvPVX_const(ssv),SvPVX(dsv),cur,char);
            SvCUR_set(dsv, cur);
            *SvEND(dsv) = '\0';
        }
        if (sflags & SVp_NOK) {
            SvNV_set(dsv, SvNVX(ssv));
            if ((sflags & SVf_NOK) && !(sflags & SVf_POK)) {
                /* Source was SVf_NOK|SVp_NOK|SVp_POK but not SVf_POK, meaning
                   a value set as floating point and later stringified, where
                  the value happens to be one of the few that we know aren't
                  affected by the numeric locale, hence we can cache the
                  stringification. Currently that's  +Inf, -Inf and NaN, but
                  conceivably we might extend this to -9 .. +9 (excluding -0).
                  So mark destination the same: */
                SvFLAGS(dsv) &= ~SVf_POK;
            }
        }
        if (sflags & SVp_IOK) {
            SvIV_set(dsv, SvIVX(ssv));
            if (sflags & SVf_IVisUV)
                SvIsUV_on(dsv);
            if ((sflags & SVf_IOK) && !(sflags & SVf_POK)) {
                /* Source was SVf_IOK|SVp_IOK|SVp_POK but not SVf_POK, meaning
                   a value set as an integer and later stringified. So mark
                   destination the same: */
                SvFLAGS(dsv) &= ~SVf_POK;
            }
        }
        SvFLAGS(dsv) |= sflags & (SVf_IOK|SVp_IOK|SVf_NOK|SVp_NOK|SVf_UTF8);
        {
            const MAGIC * const smg = SvVSTRING_mg(ssv);
            if (smg) {
                sv_magic(dsv, NULL, PERL_MAGIC_vstring,
                         smg->mg_ptr, smg->mg_len);
                SvRMAGICAL_on(dsv);
            }
        }
    }
    else if (sflags & (SVp_IOK|SVp_NOK)) {
        (void)SvOK_off(dsv);
        SvFLAGS(dsv) |= sflags & (SVf_IOK|SVp_IOK|SVf_IVisUV|SVf_NOK|SVp_NOK);
        if (sflags & SVp_IOK) {
            /* XXXX Do we want to set IsUV for IV(ROK)?  Be extra safe... */
            SvIV_set(dsv, SvIVX(ssv));
        }
        if (sflags & SVp_NOK) {
            SvNV_set(dsv, SvNVX(ssv));
        }
    }
    else {
        if (isGV_with_GP(ssv)) {
            gv_efullname3(dsv, MUTABLE_GV(ssv), "*");
        }
        else
            (void)SvOK_off(dsv);
    }
    if (SvTAINTED(ssv))
        SvTAINT(dsv);
}


/*
=for apidoc sv_set_undef

Equivalent to C<sv_setsv(sv, &PL_sv_undef)>, but more efficient.
Doesn't handle set magic.

The perl equivalent is C<$sv = undef;>. Note that it doesn't free any string
buffer, unlike C<undef $sv>.

Introduced in perl 5.25.12.

=cut
*/

void
Perl_sv_set_undef(pTHX_ SV *sv)
{
    U32 type = SvTYPE(sv);

    PERL_ARGS_ASSERT_SV_SET_UNDEF;

    /* shortcut, NULL, IV, RV */

    if (type <= SVt_IV) {
        assert(!SvGMAGICAL(sv));
        if (SvREADONLY(sv)) {
            /* does undeffing PL_sv_undef count as modifying a read-only
             * variable? Some XS code does this */
            if (sv == &PL_sv_undef)
                return;
            Perl_croak_no_modify();
        }

        if (SvROK(sv)) {
            if (SvWEAKREF(sv))
                sv_unref_flags(sv, 0);
            else {
                SV *rv = SvRV(sv);
                SvFLAGS(sv) = type; /* quickly turn off all flags */
                SvREFCNT_dec_NN(rv);
                return;
            }
        }
        SvFLAGS(sv) = type; /* quickly turn off all flags */
        return;
    }

    if (SvIS_FREED(sv))
        Perl_croak(aTHX_ "panic: attempt to undefine a freed scalar %p",
            (void *)sv);

    SV_CHECK_THINKFIRST_COW_DROP(sv);

    if (isGV_with_GP(sv))
        Perl_ck_warner(aTHX_ packWARN(WARN_MISC),
                       "Undefined value assigned to typeglob");
    else
        SvOK_off(sv);
}

/*
=for apidoc sv_set_true

Equivalent to C<sv_setsv(sv, &PL_sv_yes)>, but may be made more
efficient in the future. Doesn't handle set magic.

The perl equivalent is C<$sv = !0;>.

Introduced in perl 5.35.11.

=cut
*/

void
Perl_sv_set_true(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_SET_TRUE;
    sv_setsv(sv, &PL_sv_yes);
}

/*
=for apidoc sv_set_false

Equivalent to C<sv_setsv(sv, &PL_sv_no)>, but may be made more
efficient in the future. Doesn't handle set magic.

The perl equivalent is C<$sv = !1;>.

Introduced in perl 5.35.11.

=cut
*/

void
Perl_sv_set_false(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_SET_FALSE;
    sv_setsv(sv, &PL_sv_no);
}

/*
=for apidoc sv_set_bool

Equivalent to C<sv_setsv(sv, bool_val ? &Pl_sv_yes : &PL_sv_no)>, but
may be made more efficient in the future. Doesn't handle set magic.

The perl equivalent is C<$sv = !!$expr;>.

Introduced in perl 5.35.11.

=cut
*/

void
Perl_sv_set_bool(pTHX_ SV *sv, const bool bool_val)
{
    PERL_ARGS_ASSERT_SV_SET_BOOL;
    sv_setsv(sv, bool_val ? &PL_sv_yes : &PL_sv_no);
}


void
Perl_sv_setsv_mg(pTHX_ SV *const dsv, SV *const ssv)
{
    PERL_ARGS_ASSERT_SV_SETSV_MG;

    sv_setsv(dsv,ssv);
    SvSETMAGIC(dsv);
}

#ifdef PERL_ANY_COW
#  define SVt_COW SVt_PV
SV *
Perl_sv_setsv_cow(pTHX_ SV *dsv, SV *ssv)
{
    STRLEN cur = SvCUR(ssv);
    STRLEN len = SvLEN(ssv);
    char *new_pv;
    U32 new_flags = (SVt_COW|SVf_POK|SVp_POK|SVf_IsCOW);
#if defined(PERL_DEBUG_READONLY_COW) && defined(PERL_COPY_ON_WRITE)
    const bool already = cBOOL(SvIsCOW(ssv));
#endif

    PERL_ARGS_ASSERT_SV_SETSV_COW;
#ifdef DEBUGGING
    if (DEBUG_C_TEST) {
        PerlIO_printf(Perl_debug_log, "Fast copy on write: %p -> %p\n",
                      (void*)ssv, (void*)dsv);
        sv_dump(ssv);
        if (dsv)
                    sv_dump(dsv);
    }
#endif
    if (dsv) {
        if (SvTHINKFIRST(dsv))
            sv_force_normal_flags(dsv, SV_COW_DROP_PV);
        else if (SvPVX_const(dsv))
            Safefree(SvPVX_mutable(dsv));
        SvUPGRADE(dsv, SVt_COW);
    }
    else
        dsv = newSV_type(SVt_COW);

    assert (SvPOK(ssv));
    assert (SvPOKp(ssv));

    if (SvIsCOW(ssv)) {
        if (SvIsCOW_shared_hash(ssv)) {
            /* source is a COW shared hash key.  */
            DEBUG_C(PerlIO_printf(Perl_debug_log,
                                  "Fast copy on write: Sharing hash\n"));
            new_pv = HEK_KEY(share_hek_hek(SvSHARED_HEK_FROM_PV(SvPVX_const(ssv))));
            goto common_exit;
        }
        else if (SvIsCOW_static(ssv)) {
            /* source is static constant; preserve this */
            new_pv = SvPVX(ssv);
            new_flags |= SVppv_STATIC;
            goto common_exit;
        }
        assert(SvCUR(ssv)+1 < SvLEN(ssv));
        assert(CowREFCNT(ssv) < SV_COW_REFCNT_MAX);
    } else {
        assert ((SvFLAGS(ssv) & CAN_COW_MASK) == CAN_COW_FLAGS);
        SvUPGRADE(ssv, SVt_COW);
        SvIsCOW_on(ssv);
        DEBUG_C(PerlIO_printf(Perl_debug_log,
                              "Fast copy on write: Converting ssv to COW\n"));
        CowREFCNT(ssv) = 0;
    }
#  ifdef PERL_DEBUG_READONLY_COW
    if (already) sv_buf_to_rw(ssv);
#  endif
    CowREFCNT(ssv)++;
    new_pv = SvPVX_mutable(ssv);
    sv_buf_to_ro(ssv);

  common_exit:
    SvPV_set(dsv, new_pv);
    SvFLAGS(dsv) = new_flags;
    if (SvUTF8(ssv))
        SvUTF8_on(dsv);
    SvLEN_set(dsv, len);
    SvCUR_set(dsv, cur);
#ifdef DEBUGGING
    if (DEBUG_C_TEST)
                sv_dump(dsv);
#endif
    return dsv;
}
#endif

/*
=for apidoc sv_setpv_bufsize

Sets the SV to be a string of cur bytes length, with at least
len bytes available. Ensures that there is a null byte at SvEND.
Returns a char * pointer to the SvPV buffer.

=cut
*/

char *
Perl_sv_setpv_bufsize(pTHX_ SV *const sv, const STRLEN cur, const STRLEN len)
{
    char *pv;

    PERL_ARGS_ASSERT_SV_SETPV_BUFSIZE;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    SvUPGRADE(sv, SVt_PV);
    pv = SvGROW(sv, len + 1);
    SvCUR_set(sv, cur);
    *(SvEND(sv))= '\0';
    (void)SvPOK_only_UTF8(sv);                /* validate pointer */

    SvTAINT(sv);
    if (SvTYPE(sv) == SVt_PVCV) CvAUTOLOAD_off(sv);
    return pv;
}

/*
=for apidoc            sv_setpv
=for apidoc_item       sv_setpv_mg
=for apidoc_item       sv_setpvn
=for apidoc_item       sv_setpvn_fresh
=for apidoc_item       sv_setpvn_mg
=for apidoc_item |void|sv_setpvs|SV* sv|"literal string"
=for apidoc_item |void|sv_setpvs_mg|SV* sv|"literal string"

These copy a string into the SV C<sv>, making sure it is C<L</SvPOK_only>>.

In the C<pvs> forms, the string must be a C literal string, enclosed in double
quotes.

In the C<pvn> forms, the first byte of the string is pointed to by C<ptr>, and
C<len> indicates the number of bytes to be copied, potentially including
embedded C<NUL> characters.

In the plain C<pv> forms, C<ptr> points to a NUL-terminated C string.  That is,
it points to the first byte of the string, and the copy proceeds up through the
first encountered C<NUL> byte.

In the forms that take a C<ptr> argument, if it is NULL, the SV will become
undefined.

The UTF-8 flag is not changed by these functions.  A terminating NUL byte is
guaranteed in the result.

The C<_mg> forms handle 'set' magic; the other forms skip all magic.

C<sv_setpvn_fresh> is a cut-down alternative to C<sv_setpvn>, intended ONLY
to be used with a fresh sv that has been upgraded to a SVt_PV, SVt_PVIV,
SVt_PVNV, or SVt_PVMG.

=cut
*/

void
Perl_sv_setpvn(pTHX_ SV *const sv, const char *const ptr, const STRLEN len)
{
    char *dptr;

    PERL_ARGS_ASSERT_SV_SETPVN;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    if (isGV_with_GP(sv))
        Perl_croak_no_modify();
    if (!ptr) {
        (void)SvOK_off(sv);
        return;
    }
    else {
        /* len is STRLEN which is unsigned, need to copy to signed */
        const IV iv = len;
        if (iv < 0)
            Perl_croak(aTHX_ "panic: sv_setpvn called with negative strlen %"
                       IVdf, iv);
    }
    SvUPGRADE(sv, SVt_PV);

    dptr = SvGROW(sv, len + 1);
    Move(ptr,dptr,len,char);
    dptr[len] = '\0';
    SvCUR_set(sv, len);
    (void)SvPOK_only_UTF8(sv);		/* validate pointer */
    SvTAINT(sv);
    if (SvTYPE(sv) == SVt_PVCV) CvAUTOLOAD_off(sv);
}

void
Perl_sv_setpvn_mg(pTHX_ SV *const sv, const char *const ptr, const STRLEN len)
{
    PERL_ARGS_ASSERT_SV_SETPVN_MG;

    sv_setpvn(sv,ptr,len);
    SvSETMAGIC(sv);
}

void
Perl_sv_setpvn_fresh(pTHX_ SV *const sv, const char *const ptr, const STRLEN len)
{
    char *dptr;

    PERL_ARGS_ASSERT_SV_SETPVN_FRESH;
    assert(SvTYPE(sv) >= SVt_PV && SvTYPE(sv) <= SVt_PVMG);
    assert(!SvTHINKFIRST(sv));
    assert(!isGV_with_GP(sv));

    if (ptr) {
        const IV iv = len;
        /* len is STRLEN which is unsigned, need to copy to signed */
        if (iv < 0)
            Perl_croak(aTHX_ "panic: sv_setpvn_fresh called with negative strlen %"
                       IVdf, iv);

        dptr = sv_grow_fresh(sv, len + 1);
        Move(ptr,dptr,len,char);
        dptr[len] = '\0';
        SvCUR_set(sv, len);
        SvPOK_on(sv);
        SvTAINT(sv);
    }
}

void
Perl_sv_setpv(pTHX_ SV *const sv, const char *const ptr)
{
    STRLEN len;

    PERL_ARGS_ASSERT_SV_SETPV;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    if (!ptr) {
        (void)SvOK_off(sv);
        return;
    }
    len = strlen(ptr);
    SvUPGRADE(sv, SVt_PV);

    SvGROW(sv, len + 1);
    Move(ptr,SvPVX(sv),len+1,char);
    SvCUR_set(sv, len);
    (void)SvPOK_only_UTF8(sv);		/* validate pointer */
    SvTAINT(sv);
    if (SvTYPE(sv) == SVt_PVCV) CvAUTOLOAD_off(sv);
}

void
Perl_sv_setpv_mg(pTHX_ SV *const sv, const char *const ptr)
{
    PERL_ARGS_ASSERT_SV_SETPV_MG;

    sv_setpv(sv,ptr);
    SvSETMAGIC(sv);
}

void
Perl_sv_sethek(pTHX_ SV *const sv, const HEK *const hek)
{
    PERL_ARGS_ASSERT_SV_SETHEK;

    if (!hek) {
        return;
    }

    if (HEK_LEN(hek) == HEf_SVKEY) {
        sv_setsv(sv, *(SV**)HEK_KEY(hek));
        return;
    } else {
        const int flags = HEK_FLAGS(hek);
        if (flags & HVhek_WASUTF8) {
            STRLEN utf8_len = HEK_LEN(hek);
            char *as_utf8 = (char *)bytes_to_utf8((U8*)HEK_KEY(hek), &utf8_len);
            sv_usepvn_flags(sv, as_utf8, utf8_len, SV_HAS_TRAILING_NUL);
            SvUTF8_on(sv);
            return;
        } else if (flags & HVhek_NOTSHARED) {
            sv_setpvn(sv, HEK_KEY(hek), HEK_LEN(hek));
            if (HEK_UTF8(hek))
                SvUTF8_on(sv);
            else SvUTF8_off(sv);
            return;
        }
        {
            SV_CHECK_THINKFIRST_COW_DROP(sv);
            SvUPGRADE(sv, SVt_PV);
            SvPV_free(sv);
            SvPV_set(sv,(char *)HEK_KEY(share_hek_hek(hek)));
            SvCUR_set(sv, HEK_LEN(hek));
            SvLEN_set(sv, 0);
            SvIsCOW_on(sv);
            SvPOK_on(sv);
            if (HEK_UTF8(hek))
                SvUTF8_on(sv);
            else SvUTF8_off(sv);
            return;
        }
    }
}


/*
=for apidoc      sv_usepvn
=for apidoc_item sv_usepvn_flags
=for apidoc_item sv_usepvn_mg

These tell an SV to use C<ptr> for its string value.  Normally SVs have
their string stored inside the SV, but these tell the SV to use an
external string instead.

C<ptr> should point to memory that was allocated
by L</C<Newx>>.  It must be
the start of a C<Newx>-ed block of memory, and not a pointer to the
middle of it (beware of L<C<OOK>|perlguts/Offsets> and copy-on-write),
and not be from a non-C<Newx> memory allocator like C<malloc>.  The
string length, C<len>, must be supplied.  By default this function
will L</C<Renew>> (i.e. realloc, move) the memory pointed to by C<ptr>,
so that the pointer should not be freed or used by the programmer after giving
it to C<sv_usepvn>, and neither should any pointers from "behind" that pointer
(I<e.g.>, S<C<ptr> + 1>) be used.

In the C<sv_usepvn_flags> form, if S<C<flags & SV_SMAGIC>> is true,
C<SvSETMAGIC> is called before returning.
And if S<C<flags & SV_HAS_TRAILING_NUL>> is true, then C<ptr[len]> must be
C<NUL>, and the realloc will be skipped (I<i.e.>, the buffer is actually at
least 1 byte longer than C<len>, and already meets the requirements for storing
in C<SvPVX>).

C<sv_usepvn> is merely C<sv_usepvn_flags> with C<flags> set to 0, so 'set'
magic is skipped.

C<sv_usepvn_mg> is merely C<sv_usepvn_flags> with C<flags> set to C<SV_SMAGIC>,
so 'set' magic is performed.

=for apidoc Amnh||SV_SMAGIC
=for apidoc Amnh||SV_HAS_TRAILING_NUL

=cut
*/

void
Perl_sv_usepvn_flags(pTHX_ SV *const sv, char *ptr, const STRLEN len, const U32 flags)
{
    STRLEN allocate;

    PERL_ARGS_ASSERT_SV_USEPVN_FLAGS;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    SvUPGRADE(sv, SVt_PV);
    if (!ptr) {
        (void)SvOK_off(sv);
        if (flags & SV_SMAGIC)
            SvSETMAGIC(sv);
        return;
    }
    if (SvPVX_const(sv))
        SvPV_free(sv);

#ifdef DEBUGGING
    if (flags & SV_HAS_TRAILING_NUL)
        assert(ptr[len] == '\0');
#endif

    allocate = (flags & SV_HAS_TRAILING_NUL)
        ? len + 1 :
#ifdef Perl_safesysmalloc_size
        len + 1;
#else
        PERL_STRLEN_ROUNDUP(len + 1);
#endif
    if (flags & SV_HAS_TRAILING_NUL) {
        /* It's long enough - do nothing.
           Specifically Perl_newCONSTSUB is relying on this.  */
    } else {
#ifdef DEBUGGING
        /* Force a move to shake out bugs in callers.  */
        char *new_ptr = (char*)safemalloc(allocate);
        Copy(ptr, new_ptr, len, char);
        PoisonFree(ptr,len,char);
        Safefree(ptr);
        ptr = new_ptr;
#else
        ptr = (char*) saferealloc (ptr, allocate);
#endif
    }
#ifdef Perl_safesysmalloc_size
    SvLEN_set(sv, Perl_safesysmalloc_size(ptr));
#else
    SvLEN_set(sv, allocate);
#endif
    SvCUR_set(sv, len);
    SvPV_set(sv, ptr);
    if (!(flags & SV_HAS_TRAILING_NUL)) {
        ptr[len] = '\0';
    }
    (void)SvPOK_only_UTF8(sv);		/* validate pointer */
    SvTAINT(sv);
    if (flags & SV_SMAGIC)
        SvSETMAGIC(sv);
}


static void
S_sv_uncow(pTHX_ SV * const sv, const U32 flags)
{
    assert(SvIsCOW(sv));
    {
#ifdef PERL_ANY_COW
        const char * const pvx = SvPVX_const(sv);
        const STRLEN len = SvLEN(sv);
        const STRLEN cur = SvCUR(sv);
        const bool was_shared_hek = SvIsCOW_shared_hash(sv);

#ifdef DEBUGGING
        if (DEBUG_C_TEST) {
                PerlIO_printf(Perl_debug_log,
                              "Copy on write: Force normal %ld\n",
                              (long) flags);
                sv_dump(sv);
        }
#endif
        SvIsCOW_off(sv);
# ifdef PERL_COPY_ON_WRITE
        if (len) {
            /* Must do this first, since the CowREFCNT uses SvPVX and
            we need to write to CowREFCNT, or de-RO the whole buffer if we are
            the only owner left of the buffer. */
            sv_buf_to_rw(sv); /* NOOP if RO-ing not supported */
            {
                U8 cowrefcnt = CowREFCNT(sv);
                if(cowrefcnt != 0) {
                    cowrefcnt--;
                    CowREFCNT(sv) = cowrefcnt;
                    sv_buf_to_ro(sv);
                    goto copy_over;
                }
            }
            /* Else we are the only owner of the buffer. */
        }
        else
# endif
        {
            /* This SV doesn't own the buffer, so need to Newx() a new one:  */
            copy_over:
            SvPV_set(sv, NULL);
            SvCUR_set(sv, 0);
            SvLEN_set(sv, 0);
            if (flags & SV_COW_DROP_PV) {
                /* OK, so we don't need to copy our buffer.  */
                SvPOK_off(sv);
            } else {
                SvGROW(sv, cur + 1);
                Move(pvx,SvPVX(sv),cur,char);
                SvCUR_set(sv, cur);
                *SvEND(sv) = '\0';
            }
            if (was_shared_hek) {
                        unshare_hek(SvSHARED_HEK_FROM_PV(pvx));
            }
#ifdef DEBUGGING
            if (DEBUG_C_TEST)
                sv_dump(sv);
#endif
        }
#else
            const char * const pvx = SvPVX_const(sv);
            const STRLEN len = SvCUR(sv);
            SvIsCOW_off(sv);
            SvPV_set(sv, NULL);
            SvLEN_set(sv, 0);
            if (flags & SV_COW_DROP_PV) {
                /* OK, so we don't need to copy our buffer.  */
                SvPOK_off(sv);
            } else {
                SvGROW(sv, len + 1);
                Move(pvx,SvPVX(sv),len,char);
                *SvEND(sv) = '\0';
            }
            unshare_hek(SvSHARED_HEK_FROM_PV(pvx));
#endif
    }
}


/*
=for apidoc sv_force_normal_flags

Undo various types of fakery on an SV, where fakery means
"more than" a string: if the PV is a shared string, make
a private copy; if we're a ref, stop refing; if we're a glob, downgrade to
an C<xpvmg>; if we're a copy-on-write scalar, this is the on-write time when
we do the copy, and is also used locally; if this is a
vstring, drop the vstring magic.  If C<SV_COW_DROP_PV> is set
then a copy-on-write scalar drops its PV buffer (if any) and becomes
C<SvPOK_off> rather than making a copy.  (Used where this
scalar is about to be set to some other value.)  In addition,
the C<flags> parameter gets passed to C<sv_unref_flags()>
when unreffing.  C<sv_force_normal> calls this function
with flags set to 0.

This function is expected to be used to signal to perl that this SV is
about to be written to, and any extra book-keeping needs to be taken care
of.  Hence, it croaks on read-only values.

=for apidoc Amnh||SV_COW_DROP_PV

=cut
*/

void
Perl_sv_force_normal_flags(pTHX_ SV *const sv, const U32 flags)
{
    PERL_ARGS_ASSERT_SV_FORCE_NORMAL_FLAGS;

    if (SvREADONLY(sv))
        Perl_croak_no_modify();
    else if (SvIsCOW(sv) && LIKELY(SvTYPE(sv) != SVt_PVHV))
        S_sv_uncow(aTHX_ sv, flags);
    if (SvROK(sv))
        sv_unref_flags(sv, flags);
    else if (SvFAKE(sv) && isGV_with_GP(sv))
        sv_unglob(sv, flags);
    else if (SvFAKE(sv) && isREGEXP(sv)) {
        /* Need to downgrade the REGEXP to a simple(r) scalar. This is analogous
           to sv_unglob. We only need it here, so inline it.  */
        const bool islv = SvTYPE(sv) == SVt_PVLV;
        const svtype new_type =
          islv ? SVt_NULL : SvMAGIC(sv) || SvSTASH(sv) ? SVt_PVMG : SVt_PV;
        SV *const temp = newSV_type(new_type);
        regexp *old_rx_body;

        if (new_type == SVt_PVMG) {
            SvMAGIC_set(temp, SvMAGIC(sv));
            SvMAGIC_set(sv, NULL);
            SvSTASH_set(temp, SvSTASH(sv));
            SvSTASH_set(sv, NULL);
        }
        if (!islv)
            SvCUR_set(temp, SvCUR(sv));
        /* Remember that SvPVX is in the head, not the body. */
        assert(ReANY((REGEXP *)sv)->mother_re);

        if (islv) {
            /* LV-as-regex has sv->sv_any pointing to an XPVLV body,
             * whose xpvlenu_rx field points to the regex body */
            XPV *xpv = (XPV*)(SvANY(sv));
            old_rx_body = xpv->xpv_len_u.xpvlenu_rx;
            xpv->xpv_len_u.xpvlenu_rx = NULL;
        }
        else
            old_rx_body = ReANY((REGEXP *)sv);

        /* Their buffer is already owned by someone else. */
        if (flags & SV_COW_DROP_PV) {
            /* SvLEN is already 0.  For SVt_REGEXP, we have a brand new
               zeroed body.  For SVt_PVLV, we zeroed it above (len field
               a union with xpvlenu_rx) */
            assert(!SvLEN(islv ? sv : temp));
            sv->sv_u.svu_pv = 0;
        }
        else {
            sv->sv_u.svu_pv = savepvn(RX_WRAPPED((REGEXP *)sv), SvCUR(sv));
            SvLEN_set(islv ? sv : temp, SvCUR(sv)+1);
            SvPOK_on(sv);
        }

        /* Now swap the rest of the bodies. */

        SvFAKE_off(sv);
        if (!islv) {
            SvFLAGS(sv) &= ~SVTYPEMASK;
            SvFLAGS(sv) |= new_type;
            SvANY(sv) = SvANY(temp);
        }

        SvFLAGS(temp) &= ~(SVTYPEMASK);
        SvFLAGS(temp) |= SVt_REGEXP|SVf_FAKE;
        SvANY(temp) = old_rx_body;

        /* temp is now rebuilt as a correctly structured SVt_REGEXP, so this
         * will trigger a call to sv_clear() which will correctly free the
         * body. */
        SvREFCNT_dec_NN(temp);
    }
    else if (SvVOK(sv)) sv_unmagic(sv, PERL_MAGIC_vstring);
}

/*
=for apidoc sv_chop

Efficient removal of characters from the beginning of the string buffer.
C<SvPOK(sv)>, or at least C<SvPOKp(sv)>, must be true and C<ptr> must be a
pointer to somewhere inside the string buffer.  C<ptr> becomes the first
character of the adjusted string.  Uses the C<OOK> hack.  On return, only
C<SvPOK(sv)> and C<SvPOKp(sv)> among the C<OK> flags will be true.

Beware: after this function returns, C<ptr> and SvPVX_const(sv) may no longer
refer to the same chunk of data.

The unfortunate similarity of this function's name to that of Perl's C<chop>
operator is strictly coincidental.  This function works from the left;
C<chop> works from the right.

=cut
*/

void
Perl_sv_chop(pTHX_ SV *const sv, const char *const ptr)
{
    STRLEN delta;
    STRLEN old_delta;
    U8 *p;
#ifdef DEBUGGING
    const U8 *evacp;
    STRLEN evacn;
#endif
    STRLEN max_delta;

    PERL_ARGS_ASSERT_SV_CHOP;

    if (!ptr || !SvPOKp(sv))
        return;
    delta = ptr - SvPVX_const(sv);
    if (!delta) {
        /* Nothing to do.  */
        return;
    }
    max_delta = SvLEN(sv) ? SvLEN(sv) : SvCUR(sv);
    if (delta > max_delta)
        Perl_croak(aTHX_ "panic: sv_chop ptr=%p, start=%p, end=%p",
                   ptr, SvPVX_const(sv), SvPVX_const(sv) + max_delta);
    /* SvPVX(sv) may move in SV_CHECK_THINKFIRST(sv), so don't use ptr any more */
    SV_CHECK_THINKFIRST(sv);
    SvPOK_only_UTF8(sv);

    if (!SvOOK(sv)) {
        if (!SvLEN(sv)) { /* make copy of shared string */
            const char *pvx = SvPVX_const(sv);
            const STRLEN len = SvCUR(sv);
            SvGROW(sv, len + 1);
            Move(pvx,SvPVX(sv),len,char);
            *SvEND(sv) = '\0';
        }
        SvOOK_on(sv);
        old_delta = 0;
    } else {
        SvOOK_offset(sv, old_delta);
    }
    SvLEN_set(sv, SvLEN(sv) - delta);
    SvCUR_set(sv, SvCUR(sv) - delta);
    SvPV_set(sv, SvPVX(sv) + delta);

    p = (U8 *)SvPVX_const(sv);

#ifdef DEBUGGING
    /* how many bytes were evacuated?  we will fill them with sentinel
       bytes, except for the part holding the new offset of course. */
    evacn = delta;
    if (old_delta)
        evacn += (old_delta < 0x100 ? 1 : 1 + sizeof(STRLEN));
    assert(evacn);
    assert(evacn <= delta + old_delta);
    evacp = p - evacn;
#endif

    /* This sets 'delta' to the accumulated value of all deltas so far */
    delta += old_delta;
    assert(delta);

    /* If 'delta' fits in a byte, store it just prior to the new beginning of
     * the string; otherwise store a 0 byte there and store 'delta' just prior
     * to that, using as many bytes as a STRLEN occupies.  Thus it overwrites a
     * portion of the chopped part of the string */
    if (delta < 0x100) {
        *--p = (U8) delta;
    } else {
        *--p = 0;
        p -= sizeof(STRLEN);
        Copy((U8*)&delta, p, sizeof(STRLEN), U8);
    }

#ifdef DEBUGGING
    /* Fill the preceding buffer with sentinals to verify that no-one is
       using it.  */
    while (p > evacp) {
        --p;
        *p = (U8)PTR2UV(p);
    }
#endif
}

/*
=for apidoc sv_catpvn
=for apidoc_item sv_catpvn_flags
=for apidoc_item sv_catpvn_mg
=for apidoc_item sv_catpvn_nomg

These concatenate the C<len> bytes of the string beginning at C<ptr> onto the
end of the string which is in C<dsv>.  The caller must make sure C<ptr>
contains at least C<len> bytes.

For all but C<sv_catpvn_flags>, the string appended is assumed to be valid
UTF-8 if the SV has the UTF-8 status set, and a string of bytes otherwise.

They differ in that:

C<sv_catpvn_mg> performs both 'get' and 'set' magic on C<dsv>.

C<sv_catpvn> performs only 'get' magic.

C<sv_catpvn_nomg> skips all magic.

C<sv_catpvn_flags> has an extra C<flags> parameter which allows you to specify
any combination of magic handling (using C<SV_GMAGIC> and/or C<SV_SMAGIC>) and
to also override the UTF-8 handling.  By supplying the C<SV_CATBYTES> flag, the
appended string is interpreted as plain bytes; by supplying instead the
C<SV_CATUTF8> flag, it will be interpreted as UTF-8, and the C<dsv> will be
upgraded to UTF-8 if necessary.

C<sv_catpvn>, C<sv_catpvn_mg>, and C<sv_catpvn_nomg> are implemented
in terms of C<sv_catpvn_flags>.

=for apidoc Amnh||SV_CATUTF8
=for apidoc Amnh||SV_CATBYTES

=cut
*/

void
Perl_sv_catpvn_flags(pTHX_ SV *const dsv, const char *sstr, const STRLEN slen, const I32 flags)
{
    STRLEN dlen;
    const char * const dstr = SvPV_force_flags(dsv, dlen, flags);

    PERL_ARGS_ASSERT_SV_CATPVN_FLAGS;
    assert((flags & (SV_CATBYTES|SV_CATUTF8)) != (SV_CATBYTES|SV_CATUTF8));

    if (!(flags & SV_CATBYTES) || !SvUTF8(dsv)) {
      if (flags & SV_CATUTF8 && !SvUTF8(dsv)) {
         sv_utf8_upgrade_flags_grow(dsv, 0, slen + 1);
         dlen = SvCUR(dsv);
      }
      else SvGROW(dsv, dlen + slen + 3);
      if (sstr == dstr)
        sstr = SvPVX_const(dsv);
      Move(sstr, SvPVX(dsv) + dlen, slen, char);
      SvCUR_set(dsv, SvCUR(dsv) + slen);
    }
    else {
        /* We inline bytes_to_utf8, to avoid an extra malloc. */
        const char * const send = sstr + slen;
        U8 *d;

        /* Something this code does not account for, which I think is
           impossible; it would require the same pv to be treated as
           bytes *and* utf8, which would indicate a bug elsewhere. */
        assert(sstr != dstr);

        SvGROW(dsv, dlen + slen * 2 + 3);
        d = (U8 *)SvPVX(dsv) + dlen;

        while (sstr < send) {
            append_utf8_from_native_byte(*sstr, &d);
            sstr++;
        }
        SvCUR_set(dsv, d-(const U8 *)SvPVX(dsv));
    }
    *SvEND(dsv) = '\0';
    (void)SvPOK_only_UTF8(dsv);		/* validate pointer */
    SvTAINT(dsv);
    if (flags & SV_SMAGIC)
        SvSETMAGIC(dsv);
}

/*
=for apidoc sv_catsv
=for apidoc_item sv_catsv_flags
=for apidoc_item sv_catsv_mg
=for apidoc_item sv_catsv_nomg

These concatenate the string from SV C<sstr> onto the end of the string in SV
C<dsv>.  If C<sstr> is null, these are no-ops; otherwise only C<dsv> is
modified.

They differ only in what magic they perform:

C<sv_catsv_mg> performs 'get' magic on both SVs before the copy, and 'set' magic
on C<dsv> afterwards.

C<sv_catsv> performs just 'get' magic, on both SVs.

C<sv_catsv_nomg> skips all magic.

C<sv_catsv_flags> has an extra C<flags> parameter which allows you to use
C<SV_GMAGIC> and/or C<SV_SMAGIC> to specify any combination of magic handling
(although either both or neither SV will have 'get' magic applied to it.)

C<sv_catsv>, C<sv_catsv_mg>, and C<sv_catsv_nomg> are implemented
in terms of C<sv_catsv_flags>.

=cut */

void
Perl_sv_catsv_flags(pTHX_ SV *const dsv, SV *const sstr, const I32 flags)
{
    PERL_ARGS_ASSERT_SV_CATSV_FLAGS;

    if (sstr) {
        STRLEN slen;
        const char *spv = SvPV_flags_const(sstr, slen, flags);
        if (flags & SV_GMAGIC)
                SvGETMAGIC(dsv);
        sv_catpvn_flags(dsv, spv, slen,
                            DO_UTF8(sstr) ? SV_CATUTF8 : SV_CATBYTES);
        if (flags & SV_SMAGIC)
                SvSETMAGIC(dsv);
    }
}

/*
=for apidoc sv_catpv
=for apidoc_item sv_catpv_flags
=for apidoc_item sv_catpv_mg
=for apidoc_item sv_catpv_nomg

These concatenate the C<NUL>-terminated string C<sstr> onto the end of the
string which is in the SV.
If the SV has the UTF-8 status set, then the bytes appended should be
valid UTF-8.

They differ only in how they handle magic:

C<sv_catpv_mg> performs both 'get' and 'set' magic.

C<sv_catpv> performs only 'get' magic.

C<sv_catpv_nomg> skips all magic.

C<sv_catpv_flags> has an extra C<flags> parameter which allows you to specify
any combination of magic handling (using C<SV_GMAGIC> and/or C<SV_SMAGIC>), and
to also override the UTF-8 handling.  By supplying the C<SV_CATUTF8> flag, the
appended string is forced to be interpreted as UTF-8; by supplying instead the
C<SV_CATBYTES> flag, it will be interpreted as just bytes.  Either the SV or
the string appended will be upgraded to UTF-8 if necessary.

=cut
*/

void
Perl_sv_catpv(pTHX_ SV *const dsv, const char *sstr)
{
    STRLEN len;
    STRLEN tlen;
    char *junk;

    PERL_ARGS_ASSERT_SV_CATPV;

    if (!sstr)
        return;
    junk = SvPV_force(dsv, tlen);
    len = strlen(sstr);
    SvGROW(dsv, tlen + len + 1);
    if (sstr == junk)
        sstr = SvPVX_const(dsv);
    Move(sstr,SvPVX(dsv)+tlen,len+1,char);
    SvCUR_set(dsv, SvCUR(dsv) + len);
    (void)SvPOK_only_UTF8(dsv);		/* validate pointer */
    SvTAINT(dsv);
}

void
Perl_sv_catpv_flags(pTHX_ SV *dsv, const char *sstr, const I32 flags)
{
    PERL_ARGS_ASSERT_SV_CATPV_FLAGS;
    sv_catpvn_flags(dsv, sstr, strlen(sstr), flags);
}

void
Perl_sv_catpv_mg(pTHX_ SV *const dsv, const char *const sstr)
{
    PERL_ARGS_ASSERT_SV_CATPV_MG;

    sv_catpv(dsv,sstr);
    SvSETMAGIC(dsv);
}

/*
=for apidoc newSV

Creates a new SV.  A non-zero C<len> parameter indicates the number of
bytes of preallocated string space the SV should have.  An extra byte for a
trailing C<NUL> is also reserved.  (C<SvPOK> is not set for the SV even if string
space is allocated.)  The reference count for the new SV is set to 1.

In 5.9.3, C<newSV()> replaces the older C<NEWSV()> API, and drops the first
parameter, I<x>, a debug aid which allowed callers to identify themselves.
This aid has been superseded by a new build option, C<PERL_MEM_LOG> (see
L<perlhacktips/PERL_MEM_LOG>).  The older API is still there for use in XS
modules supporting older perls.

=cut
*/

SV *
Perl_newSV(pTHX_ const STRLEN len)
{
    SV *sv;

    if (!len)
        new_SV(sv);
    else {
        sv = newSV_type(SVt_PV);
        sv_grow_fresh(sv, len + 1);
    }
    return sv;
}
/*
=for apidoc sv_magicext

Adds magic to an SV, upgrading it if necessary.  Applies the
supplied C<vtable> and returns a pointer to the magic added.

Note that C<sv_magicext> will allow things that C<sv_magic> will not.
In particular, you can add magic to C<SvREADONLY> SVs, and add more than
one instance of the same C<how>.

If C<namlen> is greater than zero then a C<savepvn> I<copy> of C<name> is
stored, if C<namlen> is zero then C<name> is stored as-is and - as another
special case - if C<(name && namlen == HEf_SVKEY)> then C<name> is assumed
to contain an SV* and is stored as-is with its C<REFCNT> incremented.

(This is now used as a subroutine by C<sv_magic>.)

=cut
*/
MAGIC *
Perl_sv_magicext(pTHX_ SV *const sv, SV *const obj, const int how,
                const MGVTBL *const vtable, const char *const name, const I32 namlen)
{
    MAGIC* mg;

    PERL_ARGS_ASSERT_SV_MAGICEXT;

    SvUPGRADE(sv, SVt_PVMG);
    Newxz(mg, 1, MAGIC);
    mg->mg_moremagic = SvMAGIC(sv);
    SvMAGIC_set(sv, mg);

    /* Sometimes a magic contains a reference loop, where the sv and
       object refer to each other.  To prevent a reference loop that
       would prevent such objects being freed, we look for such loops
       and if we find one we avoid incrementing the object refcount.

       Note we cannot do this to avoid self-tie loops as intervening RV must
       have its REFCNT incremented to keep it in existence.

    */
    if (!obj || obj == sv ||
        how == PERL_MAGIC_arylen ||
        how == PERL_MAGIC_regdata ||
        how == PERL_MAGIC_regdatum ||
        how == PERL_MAGIC_symtab ||
        (SvTYPE(obj) == SVt_PVGV &&
            (GvSV(obj) == sv || GvHV(obj) == (const HV *)sv
             || GvAV(obj) == (const AV *)sv || GvCV(obj) == (const CV *)sv
             || GvIOp(obj) == (const IO *)sv || GvFORM(obj) == (const CV *)sv)))
    {
        mg->mg_obj = obj;
    }
    else {
        mg->mg_obj = SvREFCNT_inc_simple(obj);
        mg->mg_flags |= MGf_REFCOUNTED;
    }

    /* Normal self-ties simply pass a null object, and instead of
       using mg_obj directly, use the SvTIED_obj macro to produce a
       new RV as needed.  For glob "self-ties", we are tieing the PVIO
       with an RV obj pointing to the glob containing the PVIO.  In
       this case, to avoid a reference loop, we need to weaken the
       reference.
    */

    if (how == PERL_MAGIC_tiedscalar && SvTYPE(sv) == SVt_PVIO &&
        obj && SvROK(obj) && GvIO(SvRV(obj)) == (const IO *)sv)
    {
      sv_rvweaken(obj);
    }

    mg->mg_type = how;
    mg->mg_len = namlen;
    if (name) {
        if (namlen > 0)
            mg->mg_ptr = savepvn(name, namlen);
        else if (namlen == HEf_SVKEY) {
            /* Yes, this is casting away const. This is only for the case of
               HEf_SVKEY. I think we need to document this aberration of the
               constness of the API, rather than making name non-const, as
               that change propagating outwards a long way.  */
            mg->mg_ptr = (char*)SvREFCNT_inc_simple_NN((SV *)name);
        } else
            mg->mg_ptr = (char *) name;
    }
    mg->mg_virtual = (MGVTBL *) vtable;

    mg_magical(sv);
    return mg;
}

MAGIC *
Perl_sv_magicext_mglob(pTHX_ SV *sv)
{
    PERL_ARGS_ASSERT_SV_MAGICEXT_MGLOB;
    if (SvTYPE(sv) == SVt_PVLV && LvTYPE(sv) == 'y') {
        /* This sv is only a delegate.  //g magic must be attached to
           its target. */
        vivify_defelem(sv);
        sv = LvTARG(sv);
    }
    return sv_magicext(sv, NULL, PERL_MAGIC_regex_global,
                       &PL_vtbl_mglob, 0, 0);
}

/*
=for apidoc sv_magic

Adds magic to an SV.  First upgrades C<sv> to type C<SVt_PVMG> if
necessary, then adds a new magic item of type C<how> to the head of the
magic list.

See C<L</sv_magicext>> (which C<sv_magic> now calls) for a description of the
handling of the C<name> and C<namlen> arguments.

You need to use C<sv_magicext> to add magic to C<SvREADONLY> SVs and also
to add more than one instance of the same C<how>.

=cut
*/

void
Perl_sv_magic(pTHX_ SV *const sv, SV *const obj, const int how,
             const char *const name, const I32 namlen)
{
    const MGVTBL *vtable;
    MAGIC* mg;
    unsigned int flags;
    unsigned int vtable_index;

    PERL_ARGS_ASSERT_SV_MAGIC;

    if (how < 0 || (unsigned)how >= C_ARRAY_LENGTH(PL_magic_data)
        || ((flags = PL_magic_data[how]),
            (vtable_index = flags & PERL_MAGIC_VTABLE_MASK)
            > magic_vtable_max))
        Perl_croak(aTHX_ "Don't know how to handle magic of type \\%o", how);

    /* PERL_MAGIC_ext is reserved for use by extensions not perl internals.
       Useful for attaching extension internal data to perl vars.
       Note that multiple extensions may clash if magical scalars
       etc holding private data from one are passed to another. */

    vtable = (vtable_index == magic_vtable_max)
        ? NULL : PL_magic_vtables + vtable_index;

    if (SvREADONLY(sv)) {
        if (
            !PERL_MAGIC_TYPE_READONLY_ACCEPTABLE(how)
           )
        {
            Perl_croak_no_modify();
        }
    }
    if (SvMAGICAL(sv) || (how == PERL_MAGIC_taint && SvTYPE(sv) >= SVt_PVMG)) {
        if (SvMAGIC(sv) && (mg = mg_find(sv, how))) {
            /* sv_magic() refuses to add a magic of the same 'how' as an
               existing one
             */
            if (how == PERL_MAGIC_taint)
                mg->mg_len |= 1;
            return;
        }
    }

    /* Rest of work is done else where */
    mg = sv_magicext(sv,obj,how,vtable,name,namlen);

    switch (how) {
    case PERL_MAGIC_taint:
        mg->mg_len = 1;
        break;
    case PERL_MAGIC_ext:
    case PERL_MAGIC_dbfile:
        SvRMAGICAL_on(sv);
        break;
    }
}

static int
S_sv_unmagicext_flags(pTHX_ SV *const sv, const int type, const MGVTBL *vtbl, const U32 flags)
{
    MAGIC* mg;
    MAGIC** mgp;

    assert(flags <= 1);

    if (SvTYPE(sv) < SVt_PVMG || !SvMAGIC(sv))
        return 0;
    mgp = &(((XPVMG*) SvANY(sv))->xmg_u.xmg_magic);
    for (mg = *mgp; mg; mg = *mgp) {
        const MGVTBL* const virt = mg->mg_virtual;
        if (mg->mg_type == type && (!flags || virt == vtbl)) {
            *mgp = mg->mg_moremagic;
            if (virt && virt->svt_free)
                virt->svt_free(aTHX_ sv, mg);
            if (mg->mg_ptr && mg->mg_type != PERL_MAGIC_regex_global) {
                if (mg->mg_len > 0)
                    Safefree(mg->mg_ptr);
                else if (mg->mg_len == HEf_SVKEY)
                    SvREFCNT_dec(MUTABLE_SV(mg->mg_ptr));
                else if (mg->mg_type == PERL_MAGIC_utf8)
                    Safefree(mg->mg_ptr);
            }
            if (mg->mg_flags & MGf_REFCOUNTED)
                SvREFCNT_dec(mg->mg_obj);
            Safefree(mg);
        }
        else
            mgp = &mg->mg_moremagic;
    }
    if (SvMAGIC(sv)) {
        if (SvMAGICAL(sv))	/* if we're under save_magic, wait for restore_magic; */
            mg_magical(sv);	/*    else fix the flags now */
    }
    else
        SvMAGICAL_off(sv);

    return 0;
}

/*
=for apidoc sv_unmagic

Removes all magic of type C<type> from an SV.

=cut
*/

int
Perl_sv_unmagic(pTHX_ SV *const sv, const int type)
{
    PERL_ARGS_ASSERT_SV_UNMAGIC;
    return S_sv_unmagicext_flags(aTHX_ sv, type, NULL, 0);
}

/*
=for apidoc sv_unmagicext

Removes all magic of type C<type> with the specified C<vtbl> from an SV.

=cut
*/

int
Perl_sv_unmagicext(pTHX_ SV *const sv, const int type, const MGVTBL *vtbl)
{
    PERL_ARGS_ASSERT_SV_UNMAGICEXT;
    return S_sv_unmagicext_flags(aTHX_ sv, type, vtbl, 1);
}

/*
=for apidoc sv_rvweaken

Weaken a reference: set the C<SvWEAKREF> flag on this RV; give the
referred-to SV C<PERL_MAGIC_backref> magic if it hasn't already; and
push a back-reference to this RV onto the array of backreferences
associated with that magic.  If the RV is magical, set magic will be
called after the RV is cleared.  Silently ignores C<undef> and warns
on already-weak references.

=cut
*/

SV *
Perl_sv_rvweaken(pTHX_ SV *const sv)
{
    SV *tsv;

    PERL_ARGS_ASSERT_SV_RVWEAKEN;

    if (!SvOK(sv))  /* let undefs pass */
        return sv;
    if (!SvROK(sv))
        Perl_croak(aTHX_ "Can't weaken a nonreference");
    else if (SvWEAKREF(sv)) {
        Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "Reference is already weak");
        return sv;
    }
    else if (SvREADONLY(sv)) croak_no_modify();
    tsv = SvRV(sv);
    Perl_sv_add_backref(aTHX_ tsv, sv);
    SvWEAKREF_on(sv);
    SvREFCNT_dec_NN(tsv);
    return sv;
}

/*
=for apidoc sv_rvunweaken

Unweaken a reference: Clear the C<SvWEAKREF> flag on this RV; remove
the backreference to this RV from the array of backreferences
associated with the target SV, increment the refcount of the target.
Silently ignores C<undef> and warns on non-weak references.

=cut
*/

SV *
Perl_sv_rvunweaken(pTHX_ SV *const sv)
{
    SV *tsv;

    PERL_ARGS_ASSERT_SV_RVUNWEAKEN;

    if (!SvOK(sv)) /* let undefs pass */
        return sv;
    if (!SvROK(sv))
        Perl_croak(aTHX_ "Can't unweaken a nonreference");
    else if (!SvWEAKREF(sv)) {
        Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "Reference is not weak");
        return sv;
    }
    else if (SvREADONLY(sv)) croak_no_modify();

    tsv = SvRV(sv);
    SvWEAKREF_off(sv);
    SvROK_on(sv);
    SvREFCNT_inc_NN(tsv);
    Perl_sv_del_backref(aTHX_ tsv, sv);
    return sv;
}

/*
=for apidoc sv_get_backrefs

If C<sv> is the target of a weak reference then it returns the back
references structure associated with the sv; otherwise return C<NULL>.

When returning a non-null result the type of the return is relevant. If it
is an AV then the elements of the AV are the weak reference RVs which
point at this item. If it is any other type then the item itself is the
weak reference.

See also C<Perl_sv_add_backref()>, C<Perl_sv_del_backref()>,
C<Perl_sv_kill_backrefs()>

=cut
*/

SV *
Perl_sv_get_backrefs(SV *const sv)
{
    SV *backrefs= NULL;

    PERL_ARGS_ASSERT_SV_GET_BACKREFS;

    /* find slot to store array or singleton backref */

    if (SvTYPE(sv) == SVt_PVHV) {
        if (HvHasAUX(sv)) {
            struct xpvhv_aux * const iter = HvAUX((HV *)sv);
            backrefs = (SV *)iter->xhv_backreferences;
        }
    } else if (SvMAGICAL(sv)) {
        MAGIC *mg = mg_find(sv, PERL_MAGIC_backref);
        if (mg)
            backrefs = mg->mg_obj;
    }
    return backrefs;
}

/* Give tsv backref magic if it hasn't already got it, then push a
 * back-reference to sv onto the array associated with the backref magic.
 *
 * As an optimisation, if there's only one backref and it's not an AV,
 * store it directly in the HvAUX or mg_obj slot, avoiding the need to
 * allocate an AV. (Whether the slot holds an AV tells us whether this is
 * active.)
 */

/* A discussion about the backreferences array and its refcount:
 *
 * The AV holding the backreferences is pointed to either as the mg_obj of
 * PERL_MAGIC_backref, or in the specific case of a HV, from the
 * xhv_backreferences field. The array is created with a refcount
 * of 2. This means that if during global destruction the array gets
 * picked on before its parent to have its refcount decremented by the
 * random zapper, it won't actually be freed, meaning it's still there for
 * when its parent gets freed.
 *
 * When the parent SV is freed, the extra ref is killed by
 * Perl_sv_kill_backrefs.  The other ref is killed, in the case of magic,
 * by mg_free() / MGf_REFCOUNTED, or for a hash, by Perl_hv_kill_backrefs.
 *
 * When a single backref SV is stored directly, it is not reference
 * counted.
 */

void
Perl_sv_add_backref(pTHX_ SV *const tsv, SV *const sv)
{
    SV **svp;
    AV *av = NULL;
    MAGIC *mg = NULL;

    PERL_ARGS_ASSERT_SV_ADD_BACKREF;

    /* find slot to store array or singleton backref */

    if (SvTYPE(tsv) == SVt_PVHV) {
        svp = (SV**)Perl_hv_backreferences_p(aTHX_ MUTABLE_HV(tsv));
    } else {
        if (SvMAGICAL(tsv))
            mg = mg_find(tsv, PERL_MAGIC_backref);
        if (!mg)
            mg = sv_magicext(tsv, NULL, PERL_MAGIC_backref, &PL_vtbl_backref, NULL, 0);
        svp = &(mg->mg_obj);
    }

    /* create or retrieve the array */

    if (   (!*svp && SvTYPE(sv) == SVt_PVAV)
        || (*svp && SvTYPE(*svp) != SVt_PVAV)
    ) {
        /* create array */
        if (mg)
            mg->mg_flags |= MGf_REFCOUNTED;
        av = newAV();
        AvREAL_off(av);
        SvREFCNT_inc_simple_void_NN(av);
        /* av now has a refcnt of 2; see discussion above */
        av_extend(av, *svp ? 2 : 1);
        if (*svp) {
            /* move single existing backref to the array */
            AvARRAY(av)[++AvFILLp(av)] = *svp; /* av_push() */
        }
        *svp = (SV*)av;
    }
    else {
        av = MUTABLE_AV(*svp);
        if (!av) {
            /* optimisation: store single backref directly in HvAUX or mg_obj */
            *svp = sv;
            return;
        }
        assert(SvTYPE(av) == SVt_PVAV);
        if (AvFILLp(av) >= AvMAX(av)) {
            av_extend(av, AvFILLp(av)+1);
        }
    }
    /* push new backref */
    AvARRAY(av)[++AvFILLp(av)] = sv; /* av_push() */
}

/* delete a back-reference to ourselves from the backref magic associated
 * with the SV we point to.
 */

void
Perl_sv_del_backref(pTHX_ SV *const tsv, SV *const sv)
{
    SV **svp = NULL;

    PERL_ARGS_ASSERT_SV_DEL_BACKREF;

    if (SvTYPE(tsv) == SVt_PVHV) {
        if (HvHasAUX(tsv))
            svp = (SV**)Perl_hv_backreferences_p(aTHX_ MUTABLE_HV(tsv));
    }
    else if (SvIS_FREED(tsv) && PL_phase == PERL_PHASE_DESTRUCT) {
        /* It's possible for the last (strong) reference to tsv to have
           become freed *before* the last thing holding a weak reference.
           If both survive longer than the backreferences array, then when
           the referent's reference count drops to 0 and it is freed, it's
           not able to chase the backreferences, so they aren't NULLed.

           For example, a CV holds a weak reference to its stash. If both the
           CV and the stash survive longer than the backreferences array,
           and the CV gets picked for the SvBREAK() treatment first,
           *and* it turns out that the stash is only being kept alive because
           of an our variable in the pad of the CV, then midway during CV
           destruction the stash gets freed, but CvSTASH() isn't set to NULL.
           It ends up pointing to the freed HV. Hence it's chased in here, and
           if this block wasn't here, it would hit the !svp panic just below.

           I don't believe that "better" destruction ordering is going to help
           here - during global destruction there's always going to be the
           chance that something goes out of order. We've tried to make it
           foolproof before, and it only resulted in evolutionary pressure on
           fools. Which made us look foolish for our hubris. :-(
        */
        return;
    }
    else {
        MAGIC *const mg
            = SvMAGICAL(tsv) ? mg_find(tsv, PERL_MAGIC_backref) : NULL;
        svp =  mg ? &(mg->mg_obj) : NULL;
    }

    if (!svp)
        Perl_croak(aTHX_ "panic: del_backref, svp=0");
    if (!*svp) {
        /* It's possible that sv is being freed recursively part way through the
           freeing of tsv. If this happens, the backreferences array of tsv has
           already been freed, and so svp will be NULL. If this is the case,
           we should not panic. Instead, nothing needs doing, so return.  */
        if (PL_phase == PERL_PHASE_DESTRUCT && SvREFCNT(tsv) == 0)
            return;
        Perl_croak(aTHX_ "panic: del_backref, *svp=%p phase=%s refcnt=%" UVuf,
                   (void*)*svp, PL_phase_names[PL_phase], (UV)SvREFCNT(tsv));
    }

    if (SvTYPE(*svp) == SVt_PVAV) {
#ifdef DEBUGGING
        int count = 1;
#endif
        AV * const av = (AV*)*svp;
        SSize_t fill;
        assert(!SvIS_FREED(av));
        fill = AvFILLp(av);
        assert(fill > -1);
        svp = AvARRAY(av);
        /* for an SV with N weak references to it, if all those
         * weak refs are deleted, then sv_del_backref will be called
         * N times and O(N^2) compares will be done within the backref
         * array. To ameliorate this potential slowness, we:
         * 1) make sure this code is as tight as possible;
         * 2) when looking for SV, look for it at both the head and tail of the
         *    array first before searching the rest, since some create/destroy
         *    patterns will cause the backrefs to be freed in order.
         */
        if (*svp == sv) {
            AvARRAY(av)++;
            AvMAX(av)--;
        }
        else {
            SV **p = &svp[fill];
            SV *const topsv = *p;
            if (topsv != sv) {
#ifdef DEBUGGING
                count = 0;
#endif
                while (--p > svp) {
                    if (*p == sv) {
                        /* We weren't the last entry.
                           An unordered list has this property that you
                           can take the last element off the end to fill
                           the hole, and it's still an unordered list :-)
                        */
                        *p = topsv;
#ifdef DEBUGGING
                        count++;
#else
                        break; /* should only be one */
#endif
                    }
                }
            }
        }
        assert(count ==1);
        AvFILLp(av) = fill-1;
    }
    else if (SvIS_FREED(*svp) && PL_phase == PERL_PHASE_DESTRUCT) {
        /* freed AV; skip */
    }
    else {
        /* optimisation: only a single backref, stored directly */
        if (*svp != sv)
            Perl_croak(aTHX_ "panic: del_backref, *svp=%p, sv=%p",
                       (void*)*svp, (void*)sv);
        *svp = NULL;
    }

}

void
Perl_sv_kill_backrefs(pTHX_ SV *const sv, AV *const av)
{
    SV **svp;
    SV **last;
    bool is_array;

    PERL_ARGS_ASSERT_SV_KILL_BACKREFS;

    if (!av)
        return;

    /* after multiple passes through Perl_sv_clean_all() for a thingy
     * that has badly leaked, the backref array may have gotten freed,
     * since we only protect it against 1 round of cleanup */
    if (SvIS_FREED(av)) {
        if (PL_in_clean_all) /* All is fair */
            return;
        Perl_croak(aTHX_
                   "panic: magic_killbackrefs (freed backref AV/SV)");
    }


    is_array = (SvTYPE(av) == SVt_PVAV);
    if (is_array) {
        assert(!SvIS_FREED(av));
        svp = AvARRAY(av);
        if (svp)
            last = svp + AvFILLp(av);
    }
    else {
        /* optimisation: only a single backref, stored directly */
        svp = (SV**)&av;
        last = svp;
    }

    if (svp) {
        while (svp <= last) {
            if (*svp) {
                SV *const referrer = *svp;
                if (SvWEAKREF(referrer)) {
                    /* XXX Should we check that it hasn't changed? */
                    assert(SvROK(referrer));
                    SvRV_set(referrer, 0);
                    SvOK_off(referrer);
                    SvWEAKREF_off(referrer);
                    SvSETMAGIC(referrer);
                } else if (SvTYPE(referrer) == SVt_PVGV ||
                           SvTYPE(referrer) == SVt_PVLV) {
                    assert(SvTYPE(sv) == SVt_PVHV); /* stash backref */
                    /* You lookin' at me?  */
                    assert(GvSTASH(referrer));
                    assert(GvSTASH(referrer) == (const HV *)sv);
                    GvSTASH(referrer) = 0;
                } else if (SvTYPE(referrer) == SVt_PVCV ||
                           SvTYPE(referrer) == SVt_PVFM) {
                    if (SvTYPE(sv) == SVt_PVHV) { /* stash backref */
                        /* You lookin' at me?  */
                        assert(CvSTASH(referrer));
                        assert(CvSTASH(referrer) == (const HV *)sv);
                        SvANY(MUTABLE_CV(referrer))->xcv_stash = 0;
                    }
                    else {
                        assert(SvTYPE(sv) == SVt_PVGV);
                        /* You lookin' at me?  */
                        assert(CvGV(referrer));
                        assert(CvGV(referrer) == (const GV *)sv);
                        anonymise_cv_maybe(MUTABLE_GV(sv),
                                                MUTABLE_CV(referrer));
                    }

                } else {
                    Perl_croak(aTHX_
                               "panic: magic_killbackrefs (flags=%" UVxf ")",
                               (UV)SvFLAGS(referrer));
                }

                if (is_array)
                    *svp = NULL;
            }
            svp++;
        }
    }
    if (is_array) {
        AvFILLp(av) = -1;
        SvREFCNT_dec_NN(av); /* remove extra count added by sv_add_backref() */
    }
    return;
}

/*
=for apidoc sv_insert

Inserts and/or replaces a string at the specified offset/length within the SV.
Similar to the Perl C<substr()> function, with C<littlelen> bytes starting at
C<little> replacing C<len> bytes of the string in C<bigstr> starting at
C<offset>.  Handles get magic.

=for apidoc sv_insert_flags

Same as C<sv_insert>, but the extra C<flags> are passed to the
C<SvPV_force_flags> that applies to C<bigstr>.

=cut
*/

void
Perl_sv_insert_flags(pTHX_ SV *const bigstr, const STRLEN offset, const STRLEN len, const char *little, const STRLEN littlelen, const U32 flags)
{
    char *big;
    char *mid;
    char *midend;
    char *bigend;
    SSize_t i;		/* better be sizeof(STRLEN) or bad things happen */
    STRLEN curlen;

    PERL_ARGS_ASSERT_SV_INSERT_FLAGS;

    SvPV_force_flags(bigstr, curlen, flags);
    (void)SvPOK_only_UTF8(bigstr);

    if (little >= SvPVX(bigstr) &&
        little < SvPVX(bigstr) + (SvLEN(bigstr) ? SvLEN(bigstr) : SvCUR(bigstr))) {
        /* little is a pointer to within bigstr, since we can reallocate bigstr,
           or little...little+littlelen might overlap offset...offset+len we make a copy
        */
        little = savepvn(little, littlelen);
        SAVEFREEPV(little);
    }

    if (offset + len > curlen) {
        SvGROW(bigstr, offset+len+1);
        Zero(SvPVX(bigstr)+curlen, offset+len-curlen, char);
        SvCUR_set(bigstr, offset+len);
    }

    SvTAINT(bigstr);
    i = littlelen - len;
    if (i > 0) {			/* string might grow */
        big = SvGROW(bigstr, SvCUR(bigstr) + i + 1);
        mid = big + offset + len;
        midend = bigend = big + SvCUR(bigstr);
        bigend += i;
        *bigend = '\0';
        while (midend > mid)		/* shove everything down */
            *--bigend = *--midend;
        Move(little,big+offset,littlelen,char);
        SvCUR_set(bigstr, SvCUR(bigstr) + i);
        SvSETMAGIC(bigstr);
        return;
    }
    else if (i == 0) {
        Move(little,SvPVX(bigstr)+offset,len,char);
        SvSETMAGIC(bigstr);
        return;
    }

    big = SvPVX(bigstr);
    mid = big + offset;
    midend = mid + len;
    bigend = big + SvCUR(bigstr);

    if (midend > bigend)
        Perl_croak(aTHX_ "panic: sv_insert, midend=%p, bigend=%p",
                   midend, bigend);

    if (mid - big > bigend - midend) {	/* faster to shorten from end */
        if (littlelen) {
            Move(little, mid, littlelen,char);
            mid += littlelen;
        }
        i = bigend - midend;
        if (i > 0) {
            Move(midend, mid, i,char);
            mid += i;
        }
        *mid = '\0';
        SvCUR_set(bigstr, mid - big);
    }
    else if ((i = mid - big)) {	/* faster from front */
        midend -= littlelen;
        mid = midend;
        Move(big, midend - i, i, char);
        sv_chop(bigstr,midend-i);
        if (littlelen)
            Move(little, mid, littlelen,char);
    }
    else if (littlelen) {
        midend -= littlelen;
        sv_chop(bigstr,midend);
        Move(little,midend,littlelen,char);
    }
    else {
        sv_chop(bigstr,midend);
    }
    SvSETMAGIC(bigstr);
}

/*
=for apidoc sv_replace

Make the first argument a copy of the second, then delete the original.
The target SV physically takes over ownership of the body of the source SV
and inherits its flags; however, the target keeps any magic it owns,
and any magic in the source is discarded.
Note that this is a rather specialist SV copying operation; most of the
time you'll want to use C<sv_setsv> or one of its many macro front-ends.

=cut
*/

void
Perl_sv_replace(pTHX_ SV *const sv, SV *const nsv)
{
    const U32 refcnt = SvREFCNT(sv);

    PERL_ARGS_ASSERT_SV_REPLACE;

    SV_CHECK_THINKFIRST_COW_DROP(sv);
    if (SvREFCNT(nsv) != 1) {
        Perl_croak(aTHX_ "panic: reference miscount on nsv in sv_replace()"
                   " (%" UVuf " != 1)", (UV) SvREFCNT(nsv));
    }
    if (SvMAGICAL(sv)) {
        if (SvMAGICAL(nsv))
            mg_free(nsv);
        else
            sv_upgrade(nsv, SVt_PVMG);
        SvMAGIC_set(nsv, SvMAGIC(sv));
        SvFLAGS(nsv) |= SvMAGICAL(sv);
        SvMAGICAL_off(sv);
        SvMAGIC_set(sv, NULL);
    }
    SvREFCNT(sv) = 0;
    sv_clear(sv);
    assert(!SvREFCNT(sv));
#ifdef DEBUG_LEAKING_SCALARS
    sv->sv_flags  = nsv->sv_flags;
    sv->sv_any    = nsv->sv_any;
    sv->sv_refcnt = nsv->sv_refcnt;
    sv->sv_u      = nsv->sv_u;
#else
    StructCopy(nsv,sv,SV);
#endif
    if(SvTYPE(sv) == SVt_IV) {
        SET_SVANY_FOR_BODYLESS_IV(sv);
    }


    SvREFCNT(sv) = refcnt;
    SvFLAGS(nsv) |= SVTYPEMASK;		/* Mark as freed */
    SvREFCNT(nsv) = 0;
    del_SV(nsv);
}

/* We're about to free a GV which has a CV that refers back to us.
 * If that CV will outlive us, make it anonymous (i.e. fix up its CvGV
 * field) */

STATIC void
S_anonymise_cv_maybe(pTHX_ GV *gv, CV* cv)
{
    SV *gvname;
    GV *anongv;

    PERL_ARGS_ASSERT_ANONYMISE_CV_MAYBE;

    /* be assertive! */
    assert(SvREFCNT(gv) == 0);
    assert(isGV(gv) && isGV_with_GP(gv));
    assert(GvGP(gv));
    assert(!CvANON(cv));
    assert(CvGV(cv) == gv);
    assert(!CvNAMED(cv));

    /* will the CV shortly be freed by gp_free() ? */
    if (GvCV(gv) == cv && GvGP(gv)->gp_refcnt < 2 && SvREFCNT(cv) < 2) {
        SvANY(cv)->xcv_gv_u.xcv_gv = NULL;
        return;
    }

    /* if not, anonymise: */
    gvname = (GvSTASH(gv) && HvHasNAME(GvSTASH(gv)) && HvHasENAME(GvSTASH(gv)))
                    ? newSVhek(HvENAME_HEK(GvSTASH(gv)))
                    : newSVpvn_flags( "__ANON__", 8, 0 );
    sv_catpvs(gvname, "::__ANON__");
    anongv = gv_fetchsv(gvname, GV_ADDMULTI, SVt_PVCV);
    SvREFCNT_dec_NN(gvname);

    CvANON_on(cv);
    CvCVGV_RC_on(cv);
    SvANY(cv)->xcv_gv_u.xcv_gv = MUTABLE_GV(SvREFCNT_inc(anongv));
}


/*
=for apidoc sv_clear

Clear an SV: call any destructors, free up any memory used by the body,
and free the body itself.  The SV's head is I<not> freed, although
its type is set to all 1's so that it won't inadvertently be assumed
to be live during global destruction etc.
This function should only be called when C<REFCNT> is zero.  Most of the time
you'll want to call C<SvREFCNT_dec> instead.

=cut
*/

void
Perl_sv_clear(pTHX_ SV *const orig_sv)
{
    SV* iter_sv = NULL;
    SV* next_sv = NULL;
    SV *sv = orig_sv;
    STRLEN hash_index = 0; /* initialise to make Coverity et al happy.
                              Not strictly necessary */

    PERL_ARGS_ASSERT_SV_CLEAR;

    /* within this loop, sv is the SV currently being freed, and
     * iter_sv is the most recent AV or whatever that's being iterated
     * over to provide more SVs */

    while (sv) {
        U32 type = SvTYPE(sv);
        HV *stash;

        assert(SvREFCNT(sv) == 0);
        assert(!SvIS_FREED(sv));
#if NVSIZE <= IVSIZE
        if (type <= SVt_NV) {
#else
        if (type <= SVt_IV) {
#endif
            /* Historically this check on type was needed so that the code to
             * free bodies wasn't reached for these types, because the arena
             * slots were re-used for HEs and pointer table entries. The
             * metadata table `bodies_by_type` had the information for the sizes
             * for HEs and PTEs, hence the code here had to have a special-case
             * check to ensure that the "regular" body freeing code wasn't
             * reached, and get confused by the "lies" in `bodies_by_type`.
             *
             * However, it hasn't actually been needed for that reason since
             * Aug 2010 (commit 829cd18aa7f45221), because `bodies_by_type` was
             * changed to always hold the accurate metadata for the SV types.
             * This was possible because PTEs were no longer allocated from the
             * "SVt_IV" arena, and the code to allocate HEs from the "SVt_NULL"
             * arena is entirely in hv.c, so doesn't access the table.
             *
             * Some sort of check is still needed to handle SVt_IVs - pure RVs
             * need to take one code path which is common with RVs stored in
             * SVt_PV (or larger), but pure IVs mustn't take the "PV but not RV"
             * path, as SvPVX() doesn't point to valid memory.
             *
             * Hence this code is still the most efficient way to handle this.
             *
             * Additionally, for bodyless NVs, riding this branch is more
             * efficient than stepping through the general logic.
             */

            if (SvROK(sv))
                goto free_rv;
            SvFLAGS(sv) &= SVf_BREAK;
            SvFLAGS(sv) |= SVTYPEMASK;
            goto free_head;
        }

        /* objs are always >= MG, but pad names use the SVs_OBJECT flag
           for another purpose  */
        assert(!SvOBJECT(sv) || type >= SVt_PVMG);

        if (type >= SVt_PVMG) {
            if (SvOBJECT(sv)) {
                if (!curse(sv, 1)) goto get_next_sv;
                type = SvTYPE(sv); /* destructor may have changed it */
            }
            /* Free back-references before magic, in case the magic calls
             * Perl code that has weak references to sv. */
            if (type == SVt_PVHV) {
                Perl_hv_kill_backrefs(aTHX_ MUTABLE_HV(sv));
                if (SvMAGIC(sv))
                    mg_free(sv);
            }
            else if (SvMAGIC(sv)) {
                /* Free back-references before other types of magic. */
                sv_unmagic(sv, PERL_MAGIC_backref);
                mg_free(sv);
            }
            SvMAGICAL_off(sv);
        }
        switch (type) {
            /* case SVt_INVLIST: */
        case SVt_PVIO:
            if (IoIFP(sv) &&
                IoIFP(sv) != PerlIO_stdin() &&
                IoIFP(sv) != PerlIO_stdout() &&
                IoIFP(sv) != PerlIO_stderr() &&
                !(IoFLAGS(sv) & IOf_FAKE_DIRP))
            {
                io_close(MUTABLE_IO(sv), NULL, FALSE,
                         (IoTYPE(sv) == IoTYPE_WRONLY ||
                          IoTYPE(sv) == IoTYPE_RDWR   ||
                          IoTYPE(sv) == IoTYPE_APPEND));
            }
            if (IoDIRP(sv) && !(IoFLAGS(sv) & IOf_FAKE_DIRP))
                PerlDir_close(IoDIRP(sv));
            IoDIRP(sv) = (DIR*)NULL;
            Safefree(IoTOP_NAME(sv));
            Safefree(IoFMT_NAME(sv));
            Safefree(IoBOTTOM_NAME(sv));
            if ((const GV *)sv == PL_statgv)
                PL_statgv = NULL;
            goto freescalar;
        case SVt_REGEXP:
            /* FIXME for plugins */
            pregfree2((REGEXP*) sv);
            goto freescalar;
        case SVt_PVCV:
        case SVt_PVFM:
            cv_undef(MUTABLE_CV(sv));
            /* If we're in a stash, we don't own a reference to it.
             * However it does have a back reference to us, which needs to
             * be cleared.  */
            if ((stash = CvSTASH(sv)))
                sv_del_backref(MUTABLE_SV(stash), sv);
            goto freescalar;
        case SVt_PVHV:
            if (HvTOTALKEYS((HV*)sv) > 0) {
                const HEK *hek;
                /* this statement should match the one at the beginning of
                 * hv_undef_flags() */
                if (   PL_phase != PERL_PHASE_DESTRUCT
                    && (hek = HvNAME_HEK((HV*)sv)))
                {
                    if (PL_stashcache) {
                        DEBUG_o(Perl_deb(aTHX_
                            "sv_clear clearing PL_stashcache for '%" HEKf
                            "'\n",
                             HEKfARG(hek)));
                        (void)hv_deletehek(PL_stashcache,
                                           hek, G_DISCARD);
                    }
                    hv_name_set((HV*)sv, NULL, 0, 0);
                }

                /* save old iter_sv in unused SvSTASH field */
                assert(!SvOBJECT(sv));
                SvSTASH(sv) = (HV*)iter_sv;
                iter_sv = sv;

                /* save old hash_index in unused SvMAGIC field */
                assert(!SvMAGICAL(sv));
                assert(!SvMAGIC(sv));
                ((XPVMG*) SvANY(sv))->xmg_u.xmg_hash_index = hash_index;
                hash_index = 0;

                next_sv = Perl_hfree_next_entry(aTHX_ (HV*)sv, &hash_index);
                goto get_next_sv; /* process this new sv */
            }
            /* free empty hash */
            Perl_hv_undef_flags(aTHX_ MUTABLE_HV(sv), HV_NAME_SETALL);
            assert(!HvARRAY((HV*)sv));
            break;
        case SVt_PVAV:
            {
                AV* av = MUTABLE_AV(sv);
                if (PL_comppad == av) {
                    PL_comppad = NULL;
                    PL_curpad = NULL;
                }
                if (AvREAL(av) && AvFILLp(av) > -1) {
                    next_sv = AvARRAY(av)[AvFILLp(av)--];
                    /* save old iter_sv in top-most slot of AV,
                     * and pray that it doesn't get wiped in the meantime */
                    AvARRAY(av)[AvMAX(av)] = iter_sv;
                    iter_sv = sv;
                    goto get_next_sv; /* process this new sv */
                }
                Safefree(AvALLOC(av));
            }

            break;
        case SVt_PVOBJ:
            if(ObjectMAXFIELD(sv) > -1) {
                next_sv = ObjectFIELDS(sv)[ObjectMAXFIELD(sv)--];
                /* save old iter_sv in top-most field, and pray that it
                 * doesn't get wiped in the meantime */
                ObjectFIELDS(sv)[(ObjectITERSVAT(sv) = ObjectMAXFIELD(sv) + 1)] = iter_sv;
                iter_sv = sv;
                goto get_next_sv;
            }
            Safefree(ObjectFIELDS(sv));
            break;
        case SVt_PVLV:
            if (LvTYPE(sv) == 'T') { /* for tie: return HE to pool */
                SvREFCNT_dec(HeKEY_sv((HE*)LvTARG(sv)));
                HeNEXT((HE*)LvTARG(sv)) = PL_hv_fetch_ent_mh;
                PL_hv_fetch_ent_mh = (HE*)LvTARG(sv);
            }
            else if (LvTYPE(sv) != 't') /* unless tie: unrefcnted fake SV**  */
                SvREFCNT_dec(LvTARG(sv));
            if (isREGEXP(sv)) {
                /* This PVLV has had a REGEXP assigned to it - the memory
                 * normally used to store SvLEN instead points to a regex body.
                 * Retrieving the pointer to the regex body from the correct
                 * location is normally abstracted by ReANY(), which handles
                 * both SVt_PVLV and SVt_REGEXP
                 *
                 * This code is unwinding the storage specific to SVt_PVLV.
                 * We get the body pointer directly from the union, free it,
                 * then set SvLEN to whatever value was in the now-freed regex
                 * body. The PVX buffer is shared by multiple re's and only
                 * freed once, by the re whose SvLEN is non-null.
                 *
                 * Perl_sv_force_normal_flags() also has code to free this
                 * hidden body - it swaps the body into a temporary SV it has
                 * just allocated, then frees that SV. That causes execution
                 * to reach the SVt_REGEXP: case about 60 lines earlier in this
                 * function.
                 *
                 * See Perl_reg_temp_copy() for the code that sets up this
                 * REGEXP body referenced by the PVLV. */
                struct regexp *r = ((XPV*)SvANY(sv))->xpv_len_u.xpvlenu_rx;
                STRLEN len = r->xpv_len;
                pregfree2((REGEXP*) sv);
                del_body_by_type(r, SVt_REGEXP);
                SvLEN_set((sv), len);
                goto freescalar;
            }
            /* FALLTHROUGH */
        case SVt_PVGV:
            if (isGV_with_GP(sv)) {
                if(GvCVu((const GV *)sv) && (stash = GvSTASH(MUTABLE_GV(sv)))
                   && HvHasENAME(stash))
                    mro_method_changed_in(stash);
                gp_free(MUTABLE_GV(sv));
                if (GvNAME_HEK(sv))
                    unshare_hek(GvNAME_HEK(sv));
                /* If we're in a stash, we don't own a reference to it.
                 * However it does have a back reference to us, which
                 * needs to be cleared.  */
                if ((stash = GvSTASH(sv)))
                        sv_del_backref(MUTABLE_SV(stash), sv);
            }
            /* FIXME. There are probably more unreferenced pointers to SVs
             * in the interpreter struct that we should check and tidy in
             * a similar fashion to this:  */
            /* See also S_sv_unglob, which does the same thing. */
            if ((const GV *)sv == PL_last_in_gv)
                PL_last_in_gv = NULL;
            else if ((const GV *)sv == PL_statgv)
                PL_statgv = NULL;
            else if ((const GV *)sv == PL_stderrgv)
                PL_stderrgv = NULL;
            /* FALLTHROUGH */
        case SVt_PVMG:
        case SVt_PVNV:
        case SVt_PVIV:
        case SVt_INVLIST:
        case SVt_PV:
          freescalar:
            /* Don't bother with SvOOK_off(sv); as we're only going to
             * free it.  */
            if (SvOOK(sv)) {
                STRLEN offset;
                SvOOK_offset(sv, offset);
                SvPV_set(sv, SvPVX_mutable(sv) - offset);
                /* Don't even bother with turning off the OOK flag.  */
            }
            if (SvROK(sv)) {
            free_rv:
                {
                    SV * const target = SvRV(sv);
                    if (SvWEAKREF(sv))
                        sv_del_backref(target, sv);
                    else
                        next_sv = target;
                }
            }
#ifdef PERL_ANY_COW
            else if (SvPVX_const(sv)
                     && !(SvTYPE(sv) == SVt_PVIO
                     && !(IoFLAGS(sv) & IOf_FAKE_DIRP)))
            {
                if (SvIsCOW(sv)) {
#ifdef DEBUGGING
                    if (DEBUG_C_TEST) {
                        PerlIO_printf(Perl_debug_log, "Copy on write: clear\n");
                        sv_dump(sv);
                    }
#endif
                    if (SvIsCOW_static(sv)) {
                        SvLEN_set(sv, 0);
                    }
                    else if (SvIsCOW_shared_hash(sv)) {
                        unshare_hek(SvSHARED_HEK_FROM_PV(SvPVX_const(sv)));
                    }
                    else {
                        if (CowREFCNT(sv)) {
                            sv_buf_to_rw(sv);
                            CowREFCNT(sv)--;
                            sv_buf_to_ro(sv);
                            SvLEN_set(sv, 0);
                        }
                    }
                }
                if (SvLEN(sv)) {
                    Safefree(SvPVX_mutable(sv));
                }
            }
#else
            else if (SvPVX_const(sv) && SvLEN(sv)
                     && !(SvTYPE(sv) == SVt_PVIO
                     && !(IoFLAGS(sv) & IOf_FAKE_DIRP)))
                Safefree(SvPVX_mutable(sv));
            else if (SvPVX_const(sv) && SvIsCOW(sv)) {
                unshare_hek(SvSHARED_HEK_FROM_PV(SvPVX_const(sv)));
            }
#endif
            break;
        case SVt_NV:
            break;
        }

      free_body:

        {
            U32 arena_index;
            const struct body_details *sv_type_details;

            if (type == SVt_PVHV && HvHasAUX(sv)) {
                arena_index = HVAUX_ARENA_ROOT_IX;
                sv_type_details = &fake_hv_with_aux;
            }
            else {
                arena_index = type;
                sv_type_details = bodies_by_type + arena_index;
            }

            SvFLAGS(sv) &= SVf_BREAK;
            SvFLAGS(sv) |= SVTYPEMASK;

            if (sv_type_details->arena) {
                del_body(((char *)SvANY(sv) + sv_type_details->offset),
                         &PL_body_roots[arena_index]);
            }
            else if (sv_type_details->body_size) {
                safefree(SvANY(sv));
            }
        }

      free_head:
        /* caller is responsible for freeing the head of the original sv */
        if (sv != orig_sv && !SvREFCNT(sv))
            del_SV(sv);

        /* grab and free next sv, if any */
      get_next_sv:
        while (1) {
            sv = NULL;
            if (next_sv) {
                sv = next_sv;
                next_sv = NULL;
            }
            else if (!iter_sv) {
                break;
            } else if (SvTYPE(iter_sv) == SVt_PVAV) {
                AV *const av = (AV*)iter_sv;
                if (AvFILLp(av) > -1) {
                    sv = AvARRAY(av)[AvFILLp(av)--];
                }
                else { /* no more elements of current AV to free */
                    sv = iter_sv;
                    type = SvTYPE(sv);
                    /* restore previous value, squirrelled away */
                    iter_sv = AvARRAY(av)[AvMAX(av)];
                    Safefree(AvALLOC(av));
                    goto free_body;
                }
            } else if (SvTYPE(iter_sv) == SVt_PVOBJ) {
                if (ObjectMAXFIELD(iter_sv) > -1) {
                    sv = ObjectFIELDS(iter_sv)[ObjectMAXFIELD(iter_sv)--];
                }
                else { /* no more fields in the current SV to free */
                    sv = iter_sv;
                    type = SvTYPE(sv);
                    iter_sv = ObjectFIELDS(sv)[ObjectITERSVAT(sv)];
                    Safefree(ObjectFIELDS(sv));
                    goto free_body;
                }
            } else if (SvTYPE(iter_sv) == SVt_PVHV) {
                sv = Perl_hfree_next_entry(aTHX_ (HV*)iter_sv, &hash_index);
                if (!sv && !HvTOTALKEYS((HV *)iter_sv)) {
                    /* no more elements of current HV to free */
                    sv = iter_sv;
                    type = SvTYPE(sv);
                    /* Restore previous values of iter_sv and hash_index,
                     * squirrelled away */
                    assert(!SvOBJECT(sv));
                    iter_sv = (SV*)SvSTASH(sv);
                    assert(!SvMAGICAL(sv));
                    hash_index = ((XPVMG*) SvANY(sv))->xmg_u.xmg_hash_index;
#ifdef DEBUGGING
                    /* perl -DA does not like rubbish in SvMAGIC. */
                    SvMAGIC_set(sv, 0);
#endif

                    /* free any remaining detritus from the hash struct */
                    Perl_hv_undef_flags(aTHX_ MUTABLE_HV(sv), HV_NAME_SETALL);
                    assert(!HvARRAY((HV*)sv));
                    goto free_body;
                }
            }

            /* unrolled SvREFCNT_dec and sv_free2 follows: */

            if (!sv)
                continue;
            if (!SvREFCNT(sv)) {
                sv_free(sv);
                continue;
            }
            if (--(SvREFCNT(sv)))
                continue;
#ifdef DEBUGGING
            if (SvTEMP(sv)) {
                Perl_ck_warner_d(aTHX_ packWARN(WARN_DEBUGGING),
                         "Attempt to free temp prematurely: SV 0x%" UVxf
                         pTHX__FORMAT, PTR2UV(sv) pTHX__VALUE);
                continue;
            }
#endif
            if (SvIMMORTAL(sv)) {
                /* make sure SvREFCNT(sv)==0 happens very seldom */
                SvREFCNT(sv) = SvREFCNT_IMMORTAL;
                continue;
            }
            break;
        } /* while 1 */

    } /* while sv */
}

/* This routine curses the sv itself, not the object referenced by sv. So
   sv does not have to be ROK. */

static bool
S_curse(pTHX_ SV * const sv, const bool check_refcnt) {
    PERL_ARGS_ASSERT_CURSE;
    assert(SvOBJECT(sv));

    if (PL_defstash &&	/* Still have a symbol table? */
        SvDESTROYABLE(sv))
    {
        dSP;
        HV* stash;
        do {
          stash = SvSTASH(sv);
          assert(SvTYPE(stash) == SVt_PVHV);
          if (HvNAME(stash)) {
            CV* destructor = NULL;
            struct mro_meta *meta;

            assert (HvHasAUX(stash));

            DEBUG_o( Perl_deb(aTHX_ "Looking for DESTROY method for %s\n",
                         HvNAME(stash)) );

            /* don't make this an initialization above the assert, since it needs
               an AUX structure */
            meta = HvMROMETA(stash);
            if (meta->destroy_gen && meta->destroy_gen == PL_sub_generation) {
                destructor = meta->destroy;
                DEBUG_o( Perl_deb(aTHX_ "Using cached DESTROY method %p for %s\n",
                             (void *)destructor, HvNAME(stash)) );
            }
            else {
                bool autoload = FALSE;
                GV *gv =
                    gv_fetchmeth_pvn(stash, S_destroy, S_destroy_len, -1, 0);
                if (gv)
                    destructor = GvCV(gv);
                if (!destructor) {
                    gv = gv_autoload_pvn(stash, S_destroy, S_destroy_len,
                                         GV_AUTOLOAD_ISMETHOD);
                    if (gv)
                        destructor = GvCV(gv);
                    if (destructor)
                        autoload = TRUE;
                }
                /* we don't cache AUTOLOAD for DESTROY, since this code
                   would then need to set $__PACKAGE__::AUTOLOAD, or the
                   equivalent for XS AUTOLOADs */
                if (!autoload) {
                    meta->destroy_gen = PL_sub_generation;
                    meta->destroy = destructor;

                    DEBUG_o( Perl_deb(aTHX_ "Set cached DESTROY method %p for %s\n",
                                      (void *)destructor, HvNAME(stash)) );
                }
                else {
                    DEBUG_o( Perl_deb(aTHX_ "Not caching AUTOLOAD for DESTROY method for %s\n",
                                      HvNAME(stash)) );
                }
            }
            assert(!destructor || SvTYPE(destructor) == SVt_PVCV);
            if (destructor
                /* A constant subroutine can have no side effects, so
                   don't bother calling it.  */
                && !CvCONST(destructor)
                /* Don't bother calling an empty destructor or one that
                   returns immediately. */
                && (CvISXSUB(destructor)
                || (CvSTART(destructor)
                    && (CvSTART(destructor)->op_next->op_type
                                        != OP_LEAVESUB)
                    && (CvSTART(destructor)->op_next->op_type
                                        != OP_PUSHMARK
                        || CvSTART(destructor)->op_next->op_next->op_type
                                        != OP_RETURN
                       )
                   ))
               )
            {
                SV* const tmpref = newRV(sv);
                SvREADONLY_on(tmpref); /* DESTROY() could be naughty */
                ENTER;
                PUSHSTACKi(PERLSI_DESTROY);
                EXTEND(SP, 2);
                PUSHMARK(SP);
                PUSHs(tmpref);
                PUTBACK;
                call_sv(MUTABLE_SV(destructor),
                            G_DISCARD|G_EVAL|G_KEEPERR|G_VOID);
                POPSTACK;
                SPAGAIN;
                LEAVE;
                if(SvREFCNT(tmpref) < 2) {
                    /* tmpref is not kept alive! */
                    SvREFCNT(sv)--;
                    SvRV_set(tmpref, NULL);
                    SvROK_off(tmpref);
                }
                SvREFCNT_dec_NN(tmpref);
            }
          }
        } while (SvOBJECT(sv) && SvSTASH(sv) != stash);


        if (check_refcnt && SvREFCNT(sv)) {
            if (PL_in_clean_objs)
                Perl_croak(aTHX_
                  "DESTROY created new reference to dead object '%" HEKf "'",
                   HEKfARG(HvNAME_HEK(stash)));
            /* DESTROY gave object new lease on life */
            return FALSE;
        }
    }

    if (SvOBJECT(sv)) {
        HV * const stash = SvSTASH(sv);
        /* Curse before freeing the stash, as freeing the stash could cause
           a recursive call into S_curse. */
        SvOBJECT_off(sv);	/* Curse the object. */
        SvSTASH_set(sv,0);	/* SvREFCNT_dec may try to read this */
        SvREFCNT_dec(stash); /* possibly of changed persuasion */
    }
    return TRUE;
}

/*
=for apidoc sv_newref

Increment an SV's reference count.  Use the C<SvREFCNT_inc()> wrapper
instead.

=cut
*/

SV *
Perl_sv_newref(pTHX_ SV *const sv)
{
    PERL_UNUSED_CONTEXT;
    if (sv)
        (SvREFCNT(sv))++;
    return sv;
}

/*
=for apidoc sv_free

Decrement an SV's reference count, and if it drops to zero, call
C<sv_clear> to invoke destructors and free up any memory used by
the body; finally, deallocating the SV's head itself.
Normally called via a wrapper macro C<SvREFCNT_dec>.

=cut
*/

void
Perl_sv_free(pTHX_ SV *const sv)
{
    SvREFCNT_dec(sv);
}


/* Private helper function for SvREFCNT_dec().
 * Called with rc set to original SvREFCNT(sv), where rc == 0 or 1 */

void
Perl_sv_free2(pTHX_ SV *const sv, const U32 rc)
{

    PERL_ARGS_ASSERT_SV_FREE2;

    if (LIKELY( rc == 1 )) {
        /* normal case */
        SvREFCNT(sv) = 0;

#ifdef DEBUGGING
        if (SvTEMP(sv)) {
            Perl_ck_warner_d(aTHX_ packWARN(WARN_DEBUGGING),
                             "Attempt to free temp prematurely: SV 0x%" UVxf
                             pTHX__FORMAT, PTR2UV(sv) pTHX__VALUE);
            return;
        }
#endif
        if (SvIMMORTAL(sv)) {
            /* make sure SvREFCNT(sv)==0 happens very seldom */
            SvREFCNT(sv) = SvREFCNT_IMMORTAL;
            return;
        }
        sv_clear(sv);
        if (! SvREFCNT(sv)) /* may have have been resurrected */
            del_SV(sv);
        return;
    }

    /* handle exceptional cases */

    assert(rc == 0);

    if (SvFLAGS(sv) & SVf_BREAK)
        /* this SV's refcnt has been artificially decremented to
         * trigger cleanup */
        return;
    if (PL_in_clean_all) /* All is fair */
        return;
    if (SvIMMORTAL(sv)) {
        /* make sure SvREFCNT(sv)==0 happens very seldom */
        SvREFCNT(sv) = SvREFCNT_IMMORTAL;
        return;
    }
    if (ckWARN_d(WARN_INTERNAL)) {
#ifdef DEBUG_LEAKING_SCALARS_FORK_DUMP
        Perl_dump_sv_child(aTHX_ sv);
#else
    #ifdef DEBUG_LEAKING_SCALARS
        sv_dump(sv);
    #endif
#ifdef DEBUG_LEAKING_SCALARS_ABORT
        if (PL_warnhook == PERL_WARNHOOK_FATAL
            || ckDEAD(packWARN(WARN_INTERNAL))) {
            /* Don't let Perl_warner cause us to escape our fate:  */
            abort();
        }
#endif
        /* This may not return:  */
        Perl_warner(aTHX_ packWARN(WARN_INTERNAL),
                    "Attempt to free unreferenced scalar: SV 0x%" UVxf
                    pTHX__FORMAT, PTR2UV(sv) pTHX__VALUE);
#endif
    }
#ifdef DEBUG_LEAKING_SCALARS_ABORT
    abort();
#endif

}


/*
=for apidoc sv_len

Returns the length of the string in the SV.  Handles magic and type
coercion and sets the UTF8 flag appropriately.  See also C<L</SvCUR>>, which
gives raw access to the C<xpv_cur> slot.

=cut
*/

STRLEN
Perl_sv_len(pTHX_ SV *const sv)
{
    STRLEN len;

    if (!sv)
        return 0;

    (void)SvPV_const(sv, len);
    return len;
}

/*
=for apidoc sv_len_utf8
=for apidoc_item sv_len_utf8_nomg

These return the number of characters in the string in an SV, counting wide
UTF-8 bytes as a single character.  Both handle type coercion.
They differ only in that C<sv_len_utf8> performs 'get' magic;
C<sv_len_utf8_nomg> skips any magic.

=cut
*/

/*
 * The length is cached in PERL_MAGIC_utf8, in the mg_len field.  Also the
 * mg_ptr is used, by sv_pos_u2b() and sv_pos_b2u() - see the comments below.
 * (Note that the mg_len is not the length of the mg_ptr field.
 * This allows the cache to store the character length of the string without
 * needing to malloc() extra storage to attach to the mg_ptr.)
 *
 */

STRLEN
Perl_sv_len_utf8(pTHX_ SV *const sv)
{
    if (!sv)
        return 0;

    SvGETMAGIC(sv);
    return sv_len_utf8_nomg(sv);
}

STRLEN
Perl_sv_len_utf8_nomg(pTHX_ SV * const sv)
{
    STRLEN len;
    const U8 *s = (U8*)SvPV_nomg_const(sv, len);

    PERL_ARGS_ASSERT_SV_LEN_UTF8_NOMG;

    if (PL_utf8cache && SvUTF8(sv)) {
            STRLEN ulen;
            MAGIC *mg = SvMAGICAL(sv) ? mg_find(sv, PERL_MAGIC_utf8) : NULL;

            if (mg && (mg->mg_len != -1 || mg->mg_ptr)) {
                if (mg->mg_len != -1)
                    ulen = mg->mg_len;
                else {
                    /* We can use the offset cache for a headstart.
                       The longer value is stored in the first pair.  */
                    STRLEN *cache = (STRLEN *) mg->mg_ptr;

                    ulen = cache[0] + Perl_utf8_length(aTHX_ s + cache[1],
                                                       s + len);
                }

                if (PL_utf8cache < 0) {
                    const STRLEN real = Perl_utf8_length(aTHX_ s, s + len);
                    assert_uft8_cache_coherent("sv_len_utf8", ulen, real, sv);
                }
            }
            else {
                ulen = Perl_utf8_length(aTHX_ s, s + len);
                utf8_mg_len_cache_update(sv, &mg, ulen);
            }
            return ulen;
    }
    return SvUTF8(sv) ? Perl_utf8_length(aTHX_ s, s + len) : len;
}

/* Walk forwards to find the byte corresponding to the passed in UTF-8
   offset.  */
static STRLEN
S_sv_pos_u2b_forwards(const U8 *const start, const U8 *const send,
                      STRLEN *const uoffset_p, bool *const at_end,
                      bool* canonical_position)
{
    const U8 *s = start;
    STRLEN uoffset = *uoffset_p;

    PERL_ARGS_ASSERT_SV_POS_U2B_FORWARDS;

    while (s < send && uoffset) {
        --uoffset;
        s += UTF8SKIP(s);
    }
    if (s == send) {
        *at_end = TRUE;
    }
    else if (s > send) {
        *at_end = TRUE;
        /* This is the existing behaviour. Possibly it should be a croak, as
           it's actually a bounds error  */
        s = send;
    }
    /* If the unicode position is beyond the end, we return the end but
       shouldn't cache that position */
    *canonical_position = (uoffset == 0);
    *uoffset_p -= uoffset;
    return s - start;
}

/* Given the length of the string in both bytes and UTF-8 characters, decide
   whether to walk forwards or backwards to find the byte corresponding to
   the passed in UTF-8 offset.  */
static STRLEN
S_sv_pos_u2b_midway(const U8 *const start, const U8 *send,
                    STRLEN uoffset, const STRLEN uend)
{
    STRLEN backw = uend - uoffset;

    PERL_ARGS_ASSERT_SV_POS_U2B_MIDWAY;

    if (uoffset < 2 * backw) {
        /* The assumption is that going forwards is twice the speed of going
           forward (that's where the 2 * backw comes from).
           (The real figure of course depends on the UTF-8 data.)  */
        const U8 *s = start;

        while (s < send && uoffset--)
            s += UTF8SKIP(s);
        assert (s <= send);
        if (s > send)
            s = send;
        return s - start;
    }

    while (backw--) {
        send--;
        while (UTF8_IS_CONTINUATION(*send))
            send--;
    }
    return send - start;
}

/* For the string representation of the given scalar, find the byte
   corresponding to the passed in UTF-8 offset.  uoffset0 and boffset0
   give another position in the string, *before* the sought offset, which
   (which is always true, as 0, 0 is a valid pair of positions), which should
   help reduce the amount of linear searching.
   If *mgp is non-NULL, it should point to the UTF-8 cache magic, which
   will be used to reduce the amount of linear searching. The cache will be
   created if necessary, and the found value offered to it for update.  */
static STRLEN
S_sv_pos_u2b_cached(pTHX_ SV *const sv, MAGIC **const mgp, const U8 *const start,
                    const U8 *const send, STRLEN uoffset,
                    STRLEN uoffset0, STRLEN boffset0)
{
    STRLEN boffset = 0; /* Actually always set, but let's keep gcc happy.  */
    bool found = FALSE;
    bool at_end = FALSE;
    bool canonical_position = FALSE;

    PERL_ARGS_ASSERT_SV_POS_U2B_CACHED;

    assert (uoffset >= uoffset0);

    if (!uoffset)
        return 0;

    if (!SvREADONLY(sv) && !SvGMAGICAL(sv) && SvPOK(sv)
        && PL_utf8cache
        && (*mgp || (SvTYPE(sv) >= SVt_PVMG &&
                     (*mgp = mg_find(sv, PERL_MAGIC_utf8))))) {
        if ((*mgp)->mg_ptr) {
            STRLEN *cache = (STRLEN *) (*mgp)->mg_ptr;
            if (cache[0] == uoffset) {
                /* An exact match. */
                return cache[1];
            }
            if (cache[2] == uoffset) {
                /* An exact match. */
                return cache[3];
            }

            if (cache[0] < uoffset) {
                /* The cache already knows part of the way.   */
                if (cache[0] > uoffset0) {
                    /* The cache knows more than the passed in pair  */
                    uoffset0 = cache[0];
                    boffset0 = cache[1];
                }
                if ((*mgp)->mg_len != -1) {
                    /* And we know the end too.  */
                    boffset = boffset0
                        + sv_pos_u2b_midway(start + boffset0, send,
                                              uoffset - uoffset0,
                                              (*mgp)->mg_len - uoffset0);
                } else {
                    uoffset -= uoffset0;
                    boffset = boffset0
                        + sv_pos_u2b_forwards(start + boffset0,
                                              send, &uoffset, &at_end,
                                              &canonical_position);
                    uoffset += uoffset0;
                }
            }
            else if (cache[2] < uoffset) {
                /* We're between the two cache entries.  */
                if (cache[2] > uoffset0) {
                    /* and the cache knows more than the passed in pair  */
                    uoffset0 = cache[2];
                    boffset0 = cache[3];
                }

                boffset = boffset0
                    + sv_pos_u2b_midway(start + boffset0,
                                          start + cache[1],
                                          uoffset - uoffset0,
                                          cache[0] - uoffset0);
            } else {
                boffset = boffset0
                    + sv_pos_u2b_midway(start + boffset0,
                                          start + cache[3],
                                          uoffset - uoffset0,
                                          cache[2] - uoffset0);
            }
            found = TRUE;
        }
        else if ((*mgp)->mg_len != -1) {
            /* If we can take advantage of a passed in offset, do so.  */
            /* In fact, offset0 is either 0, or less than offset, so don't
               need to worry about the other possibility.  */
            boffset = boffset0
                + sv_pos_u2b_midway(start + boffset0, send,
                                      uoffset - uoffset0,
                                      (*mgp)->mg_len - uoffset0);
            found = TRUE;
        }
    }

    if (!found || PL_utf8cache < 0) {
        STRLEN real_boffset;
        uoffset -= uoffset0;
        real_boffset = boffset0 + sv_pos_u2b_forwards(start + boffset0,
                                                      send, &uoffset, &at_end,
                                                      &canonical_position);
        uoffset += uoffset0;

        if (found && PL_utf8cache < 0)
            assert_uft8_cache_coherent("sv_pos_u2b_cache", boffset,
                                       real_boffset, sv);
        boffset = real_boffset;
    }

    if (PL_utf8cache && canonical_position && !SvGMAGICAL(sv) && SvPOK(sv)) {
        if (at_end)
            utf8_mg_len_cache_update(sv, mgp, uoffset);
        else
            utf8_mg_pos_cache_update(sv, mgp, boffset, uoffset, send - start);
    }
    return boffset;
}


/*
=for apidoc sv_pos_u2b_flags

Converts the offset from a count of UTF-8 chars from
the start of the string, to a count of the equivalent number of bytes; if
C<lenp> is non-zero, it does the same to C<lenp>, but this time starting from
C<offset>, rather than from the start
of the string.  Handles type coercion.
C<flags> is passed to C<SvPV_flags>, and usually should be
C<SV_GMAGIC|SV_CONST_RETURN> to handle magic.

=cut
*/

/*
 * sv_pos_u2b_flags() uses, like sv_pos_b2u(), the mg_ptr of the potential
 * PERL_MAGIC_utf8 of the sv to store the mapping between UTF-8 and
 * byte offsets.  See also the comments of S_utf8_mg_pos_cache_update().
 *
 */

STRLEN
Perl_sv_pos_u2b_flags(pTHX_ SV *const sv, STRLEN uoffset, STRLEN *const lenp,
                      U32 flags)
{
    const U8 *start;
    STRLEN len;
    STRLEN boffset;

    PERL_ARGS_ASSERT_SV_POS_U2B_FLAGS;

    start = (U8*)SvPV_flags(sv, len, flags);
    if (len) {
        const U8 * const send = start + len;
        MAGIC *mg = NULL;
        boffset = sv_pos_u2b_cached(sv, &mg, start, send, uoffset, 0, 0);

        if (lenp
            && *lenp /* don't bother doing work for 0, as its bytes equivalent
                        is 0, and *lenp is already set to that.  */) {
            /* Convert the relative offset to absolute.  */
            const STRLEN uoffset2 = uoffset + *lenp;
            const STRLEN boffset2
                = sv_pos_u2b_cached(sv, &mg, start, send, uoffset2,
                                      uoffset, boffset) - boffset;

            *lenp = boffset2;
        }
    } else {
        if (lenp)
            *lenp = 0;
        boffset = 0;
    }

    return boffset;
}

/*
=for apidoc sv_pos_u2b

Converts the value pointed to by C<offsetp> from a count of UTF-8 chars from
the start of the string, to a count of the equivalent number of bytes; if
C<lenp> is non-zero, it does the same to C<lenp>, but this time starting from
the offset, rather than from the start of the string.  Handles magic and
type coercion.

Use C<sv_pos_u2b_flags> in preference, which correctly handles strings longer
than 2Gb.

=cut
*/

/*
 * sv_pos_u2b() uses, like sv_pos_b2u(), the mg_ptr of the potential
 * PERL_MAGIC_utf8 of the sv to store the mapping between UTF-8 and
 * byte offsets.  See also the comments of S_utf8_mg_pos_cache_update().
 *
 */

/* This function is subject to size and sign problems */

void
Perl_sv_pos_u2b(pTHX_ SV *const sv, I32 *const offsetp, I32 *const lenp)
{
    PERL_ARGS_ASSERT_SV_POS_U2B;

    if (lenp) {
        STRLEN ulen = (STRLEN)*lenp;
        *offsetp = (I32)sv_pos_u2b_flags(sv, (STRLEN)*offsetp, &ulen,
                                         SV_GMAGIC|SV_CONST_RETURN);
        *lenp = (I32)ulen;
    } else {
        *offsetp = (I32)sv_pos_u2b_flags(sv, (STRLEN)*offsetp, NULL,
                                         SV_GMAGIC|SV_CONST_RETURN);
    }
}

static void
S_utf8_mg_len_cache_update(pTHX_ SV *const sv, MAGIC **const mgp,
                           const STRLEN ulen)
{
    PERL_ARGS_ASSERT_UTF8_MG_LEN_CACHE_UPDATE;
    if (SvREADONLY(sv) || SvGMAGICAL(sv) || !SvPOK(sv))
        return;

    if (!*mgp && (SvTYPE(sv) < SVt_PVMG ||
                  !(*mgp = mg_find(sv, PERL_MAGIC_utf8)))) {
        *mgp = sv_magicext(sv, 0, PERL_MAGIC_utf8, &PL_vtbl_utf8, 0, 0);
    }
    assert(*mgp);

    (*mgp)->mg_len = ulen;
}

/* Create and update the UTF8 magic offset cache, with the proffered utf8/
   byte length pairing. The (byte) length of the total SV is passed in too,
   as blen, because for some (more esoteric) SVs, the call to SvPV_const()
   may not have updated SvCUR, so we can't rely on reading it directly.

   The proffered utf8/byte length pairing isn't used if the cache already has
   two pairs, and swapping either for the proffered pair would increase the
   RMS of the intervals between known byte offsets.

   The cache itself consists of 4 STRLEN values
   0: larger UTF-8 offset
   1: corresponding byte offset
   2: smaller UTF-8 offset
   3: corresponding byte offset

   Unused cache pairs have the value 0, 0.
   Keeping the cache "backwards" means that the invariant of
   cache[0] >= cache[2] is maintained even with empty slots, which means that
   the code that uses it doesn't need to worry if only 1 entry has actually
   been set to non-zero.  It also makes the "position beyond the end of the
   cache" logic much simpler, as the first slot is always the one to start
   from.
*/
static void
S_utf8_mg_pos_cache_update(pTHX_ SV *const sv, MAGIC **const mgp, const STRLEN byte,
                           const STRLEN utf8, const STRLEN blen)
{
    STRLEN *cache;

    PERL_ARGS_ASSERT_UTF8_MG_POS_CACHE_UPDATE;

    if (SvREADONLY(sv))
        return;

    if (!*mgp && (SvTYPE(sv) < SVt_PVMG ||
                  !(*mgp = mg_find(sv, PERL_MAGIC_utf8)))) {
        *mgp = sv_magicext(sv, 0, PERL_MAGIC_utf8, (MGVTBL*)&PL_vtbl_utf8, 0,
                           0);
        (*mgp)->mg_len = -1;
    }
    assert(*mgp);

    if (!(cache = (STRLEN *)(*mgp)->mg_ptr)) {
        Newxz(cache, PERL_MAGIC_UTF8_CACHESIZE * 2, STRLEN);
        (*mgp)->mg_ptr = (char *) cache;
    }
    assert(cache);

    if (PL_utf8cache < 0 && SvPOKp(sv)) {
        /* SvPOKp() because, if sv is a reference, then SvPVX() is actually
           a pointer.  Note that we no longer cache utf8 offsets on refer-
           ences, but this check is still a good idea, for robustness.  */
        const U8 *start = (const U8 *) SvPVX_const(sv);
        const STRLEN realutf8 = utf8_length(start, start + byte);

        assert_uft8_cache_coherent("utf8_mg_pos_cache_update", utf8, realutf8,
                                   sv);
    }

    /* Cache is held with the later position first, to simplify the code
       that deals with unbounded ends.  */

    ASSERT_UTF8_CACHE(cache);
    if (cache[1] == 0) {
        /* Cache is totally empty  */
        cache[0] = utf8;
        cache[1] = byte;
    } else if (cache[3] == 0) {
        if (byte > cache[1]) {
            /* New one is larger, so goes first.  */
            cache[2] = cache[0];
            cache[3] = cache[1];
            cache[0] = utf8;
            cache[1] = byte;
        } else {
            cache[2] = utf8;
            cache[3] = byte;
        }
    } else {
/* float casts necessary? XXX */
#define THREEWAY_SQUARE(a,b,c,d) \
            ((float)((d) - (c))) * ((float)((d) - (c))) \
            + ((float)((c) - (b))) * ((float)((c) - (b))) \
               + ((float)((b) - (a))) * ((float)((b) - (a)))

        /* Cache has 2 slots in use, and we know three potential pairs.
           Keep the two that give the lowest RMS distance. Do the
           calculation in bytes simply because we always know the byte
           length.  squareroot has the same ordering as the positive value,
           so don't bother with the actual square root.  */
        if (byte > cache[1]) {
            /* New position is after the existing pair of pairs.  */
            const float keep_earlier
                = THREEWAY_SQUARE(0, cache[3], byte, blen);
            const float keep_later
                = THREEWAY_SQUARE(0, cache[1], byte, blen);

            if (keep_later < keep_earlier) {
                cache[2] = cache[0];
                cache[3] = cache[1];
            }
            cache[0] = utf8;
            cache[1] = byte;
        }
        else {
            const float keep_later = THREEWAY_SQUARE(0, byte, cache[1], blen);
            float b, c, keep_earlier;
            if (byte > cache[3]) {
                /* New position is between the existing pair of pairs.  */
                b = (float)cache[3];
                c = (float)byte;
            } else {
                /* New position is before the existing pair of pairs.  */
                b = (float)byte;
                c = (float)cache[3];
            }
            keep_earlier = THREEWAY_SQUARE(0, b, c, blen);
            if (byte > cache[3]) {
                if (keep_later < keep_earlier) {
                    cache[2] = utf8;
                    cache[3] = byte;
                }
                else {
                    cache[0] = utf8;
                    cache[1] = byte;
                }
            }
            else {
                if (! (keep_later < keep_earlier)) {
                    cache[0] = cache[2];
                    cache[1] = cache[3];
                }
                cache[2] = utf8;
                cache[3] = byte;
            }
        }
    }
    ASSERT_UTF8_CACHE(cache);
}

/* We already know all of the way, now we may be able to walk back.  The same
   assumption is made as in S_sv_pos_u2b_midway(), namely that walking
   backward is half the speed of walking forward. */
static STRLEN
S_sv_pos_b2u_midway(pTHX_ const U8 *const s, const U8 *const target,
                    const U8 *end, STRLEN endu)
{
    const STRLEN forw = target - s;
    STRLEN backw = end - target;

    PERL_ARGS_ASSERT_SV_POS_B2U_MIDWAY;

    if (forw < 2 * backw) {
        return utf8_length(s, target);
    }

    while (end > target) {
        end--;
        while (UTF8_IS_CONTINUATION(*end)) {
            end--;
        }
        endu--;
    }
    return endu;
}

/*
=for apidoc sv_pos_b2u_flags

Converts C<offset> from a count of bytes from the start of the string, to
a count of the equivalent number of UTF-8 chars.  Handles type coercion.
C<flags> is passed to C<SvPV_flags>, and usually should be
C<SV_GMAGIC|SV_CONST_RETURN> to handle magic.

=cut
*/

/*
 * sv_pos_b2u_flags() uses, like sv_pos_u2b_flags(), the mg_ptr of the
 * potential PERL_MAGIC_utf8 of the sv to store the mapping between UTF-8
 * and byte offsets.
 *
 */
STRLEN
Perl_sv_pos_b2u_flags(pTHX_ SV *const sv, STRLEN const offset, U32 flags)
{
    const U8* s;
    STRLEN len = 0; /* Actually always set, but let's keep gcc happy.  */
    STRLEN blen;
    MAGIC* mg = NULL;
    const U8* send;
    bool found = FALSE;

    PERL_ARGS_ASSERT_SV_POS_B2U_FLAGS;

    s = (const U8*)SvPV_flags(sv, blen, flags);

    if (blen < offset)
        Perl_croak(aTHX_ "panic: sv_pos_b2u: bad byte offset, blen=%" UVuf
                   ", byte=%" UVuf, (UV)blen, (UV)offset);

    send = s + offset;

    if (!SvREADONLY(sv)
        && PL_utf8cache
        && SvTYPE(sv) >= SVt_PVMG
        && (mg = mg_find(sv, PERL_MAGIC_utf8)))
    {
        if (mg->mg_ptr) {
            STRLEN * const cache = (STRLEN *) mg->mg_ptr;
            if (cache[1] == offset) {
                /* An exact match. */
                return cache[0];
            }
            if (cache[3] == offset) {
                /* An exact match. */
                return cache[2];
            }

            if (cache[1] < offset) {
                /* We already know part of the way. */
                if (mg->mg_len != -1) {
                    /* Actually, we know the end too.  */
                    len = cache[0]
                        + S_sv_pos_b2u_midway(aTHX_ s + cache[1], send,
                                              s + blen, mg->mg_len - cache[0]);
                } else {
                    len = cache[0] + utf8_length(s + cache[1], send);
                }
            }
            else if (cache[3] < offset) {
                /* We're between the two cached pairs, so we do the calculation
                   offset by the byte/utf-8 positions for the earlier pair,
                   then add the utf-8 characters from the string start to
                   there.  */
                len = S_sv_pos_b2u_midway(aTHX_ s + cache[3], send,
                                          s + cache[1], cache[0] - cache[2])
                    + cache[2];

            }
            else { /* cache[3] > offset */
                len = S_sv_pos_b2u_midway(aTHX_ s, send, s + cache[3],
                                          cache[2]);

            }
            ASSERT_UTF8_CACHE(cache);
            found = TRUE;
        } else if (mg->mg_len != -1) {
            len = S_sv_pos_b2u_midway(aTHX_ s, send, s + blen, mg->mg_len);
            found = TRUE;
        }
    }
    if (!found || PL_utf8cache < 0) {
        const STRLEN real_len = utf8_length(s, send);

        if (found && PL_utf8cache < 0)
            assert_uft8_cache_coherent("sv_pos_b2u", len, real_len, sv);
        len = real_len;
    }

    if (PL_utf8cache) {
        if (blen == offset)
            utf8_mg_len_cache_update(sv, &mg, len);
        else
            utf8_mg_pos_cache_update(sv, &mg, offset, len, blen);
    }

    return len;
}

/*
=for apidoc sv_pos_b2u

Converts the value pointed to by C<offsetp> from a count of bytes from the
start of the string, to a count of the equivalent number of UTF-8 chars.
Handles magic and type coercion.

Use C<sv_pos_b2u_flags> in preference, which correctly handles strings
longer than 2Gb.

=cut
*/

/*
 * sv_pos_b2u() uses, like sv_pos_u2b(), the mg_ptr of the potential
 * PERL_MAGIC_utf8 of the sv to store the mapping between UTF-8 and
 * byte offsets.
 *
 */
void
Perl_sv_pos_b2u(pTHX_ SV *const sv, I32 *const offsetp)
{
    PERL_ARGS_ASSERT_SV_POS_B2U;

    if (!sv)
        return;

    *offsetp = (I32)sv_pos_b2u_flags(sv, (STRLEN)*offsetp,
                                     SV_GMAGIC|SV_CONST_RETURN);
}

static void
S_assert_uft8_cache_coherent(pTHX_ const char *const func, STRLEN from_cache,
                             STRLEN real, SV *const sv)
{
    PERL_ARGS_ASSERT_ASSERT_UFT8_CACHE_COHERENT;

    /* As this is debugging only code, save space by keeping this test here,
       rather than inlining it in all the callers.  */
    if (from_cache == real)
        return;

    /* Need to turn the assertions off otherwise we may recurse infinitely
       while printing error messages.  */
    SAVEI8(PL_utf8cache);
    PL_utf8cache = 0;
    Perl_croak(aTHX_ "panic: %s cache %" UVuf " real %" UVuf " for %" SVf,
               func, (UV) from_cache, (UV) real, SVfARG(sv));
}

/*
=for apidoc sv_eq

Returns a boolean indicating whether the strings in the two SVs are
identical.  Is UTF-8 and S<C<'use bytes'>> aware, handles get magic, and will
coerce its args to strings if necessary.

This function does not handle operator overloading. For a version that does,
see instead C<sv_streq>.

=for apidoc sv_eq_flags

Returns a boolean indicating whether the strings in the two SVs are
identical.  Is UTF-8 and S<C<'use bytes'>> aware and coerces its args to strings
if necessary.  If the flags has the C<SV_GMAGIC> bit set, it handles get-magic, too.

This function does not handle operator overloading. For a version that does,
see instead C<sv_streq_flags>.

=cut
*/

I32
Perl_sv_eq_flags(pTHX_ SV *sv1, SV *sv2, const U32 flags)
{
    const char *pv1;
    STRLEN cur1;
    const char *pv2;
    STRLEN cur2;

    if (!sv1) {
        pv1 = "";
        cur1 = 0;
    }
    else {
        /* if pv1 and pv2 are the same, second SvPV_const call may
         * invalidate pv1 (if we are handling magic), so we may need to
         * make a copy */
        if (sv1 == sv2 && flags & SV_GMAGIC
         && (SvTHINKFIRST(sv1) || SvGMAGICAL(sv1))) {
            pv1 = SvPV_const(sv1, cur1);
            sv1 = newSVpvn_flags(pv1, cur1, SVs_TEMP | SvUTF8(sv2));
        }
        pv1 = SvPV_flags_const(sv1, cur1, flags);
    }

    if (!sv2){
        pv2 = "";
        cur2 = 0;
    }
    else
        pv2 = SvPV_flags_const(sv2, cur2, flags);

    if (cur1 && cur2 && SvUTF8(sv1) != SvUTF8(sv2) && !IN_BYTES) {
        /* Differing utf8ness.  */
        if (SvUTF8(sv1)) {
                  /* sv1 is the UTF-8 one  */
                  return bytes_cmp_utf8((const U8*)pv2, cur2,
                                        (const U8*)pv1, cur1) == 0;
        }
        else {
                  /* sv2 is the UTF-8 one  */
                  return bytes_cmp_utf8((const U8*)pv1, cur1,
                                        (const U8*)pv2, cur2) == 0;
        }
    }

    if (cur1 == cur2)
        return (pv1 == pv2) || memEQ(pv1, pv2, cur1);
    else
        return 0;
}

/*
=for apidoc sv_streq_flags

Returns a boolean indicating whether the strings in the two SVs are
identical. If the flags argument has the C<SV_GMAGIC> bit set, it handles
get-magic too. Will coerce its args to strings if necessary. Treats
C<NULL> as undef. Correctly handles the UTF8 flag.

If flags does not have the C<SV_SKIP_OVERLOAD> bit set, an attempt to use
C<eq> overloading will be made. If such overloading does not exist or the
flag is set, then regular string comparison will be used instead.

=for apidoc sv_streq

A convenient shortcut for calling C<sv_streq_flags> with the C<SV_GMAGIC>
flag. This function basically behaves like the Perl code C<$sv1 eq $sv2>.

=cut
*/

bool
Perl_sv_streq_flags(pTHX_ SV *sv1, SV *sv2, const U32 flags)
{
    PERL_ARGS_ASSERT_SV_STREQ_FLAGS;

    if(flags & SV_GMAGIC) {
        if(sv1)
            SvGETMAGIC(sv1);
        if(sv2)
            SvGETMAGIC(sv2);
    }

    /* Treat NULL as undef */
    if(!sv1)
        sv1 = &PL_sv_undef;
    if(!sv2)
        sv2 = &PL_sv_undef;

    if(!(flags & SV_SKIP_OVERLOAD) &&
            (SvAMAGIC(sv1) || SvAMAGIC(sv2))) {
        SV *ret = amagic_call(sv1, sv2, seq_amg, 0);
        if(ret)
            return SvTRUE(ret);
    }

    return sv_eq_flags(sv1, sv2, 0);
}

/*
=for apidoc sv_numeq_flags

Returns a boolean indicating whether the numbers in the two SVs are
identical. If the flags argument has the C<SV_GMAGIC> bit set, it handles
get-magic too. Will coerce its args to numbers if necessary. Treats
C<NULL> as undef.

If flags does not have the C<SV_SKIP_OVERLOAD> bit set, an attempt to use
C<==> overloading will be made. If such overloading does not exist or the
flag is set, then regular numerical comparison will be used instead.

=for apidoc sv_numeq

A convenient shortcut for calling C<sv_numeq_flags> with the C<SV_GMAGIC>
flag. This function basically behaves like the Perl code C<$sv1 == $sv2>.

=cut
*/

bool
Perl_sv_numeq_flags(pTHX_ SV *sv1, SV *sv2, const U32 flags)
{
    PERL_ARGS_ASSERT_SV_NUMEQ_FLAGS;

    if(flags & SV_GMAGIC) {
        if(sv1)
            SvGETMAGIC(sv1);
        if(sv2)
            SvGETMAGIC(sv2);
    }

    /* Treat NULL as undef */
    if(!sv1)
        sv1 = &PL_sv_undef;
    if(!sv2)
        sv2 = &PL_sv_undef;

    if(!(flags & SV_SKIP_OVERLOAD) &&
            (SvAMAGIC(sv1) || SvAMAGIC(sv2))) {
        SV *ret = amagic_call(sv1, sv2, eq_amg, 0);
        if(ret)
            return SvTRUE(ret);
    }

    return do_ncmp(sv1, sv2) == 0;
}

/*
=for apidoc sv_cmp

Compares the strings in two SVs.  Returns -1, 0, or 1 indicating whether the
string in C<sv1> is less than, equal to, or greater than the string in
C<sv2>.  Is UTF-8 and S<C<'use bytes'>> aware, handles get magic, and will
coerce its args to strings if necessary.  See also C<L</sv_cmp_locale>>.

=for apidoc sv_cmp_flags

Compares the strings in two SVs.  Returns -1, 0, or 1 indicating whether the
string in C<sv1> is less than, equal to, or greater than the string in
C<sv2>.  Is UTF-8 and S<C<'use bytes'>> aware and will coerce its args to strings
if necessary.  If the flags has the C<SV_GMAGIC> bit set, it handles get magic.  See
also C<L</sv_cmp_locale_flags>>.

=cut
*/

I32
Perl_sv_cmp(pTHX_ SV *const sv1, SV *const sv2)
{
    return sv_cmp_flags(sv1, sv2, SV_GMAGIC);
}

I32
Perl_sv_cmp_flags(pTHX_ SV *const sv1, SV *const sv2,
                  const U32 flags)
{
    STRLEN cur1, cur2;
    const char *pv1, *pv2;
    I32  cmp;
    SV *svrecode = NULL;

    if (!sv1) {
        pv1 = "";
        cur1 = 0;
    }
    else
        pv1 = SvPV_flags_const(sv1, cur1, flags);

    if (!sv2) {
        pv2 = "";
        cur2 = 0;
    }
    else
        pv2 = SvPV_flags_const(sv2, cur2, flags);

    if (cur1 && cur2 && SvUTF8(sv1) != SvUTF8(sv2) && !IN_BYTES) {
        /* Differing utf8ness.  */
        if (SvUTF8(sv1)) {
                const int retval = -bytes_cmp_utf8((const U8*)pv2, cur2,
                                                   (const U8*)pv1, cur1);
                return retval ? retval < 0 ? -1 : +1 : 0;
        }
        else {
                const int retval = bytes_cmp_utf8((const U8*)pv1, cur1,
                                                  (const U8*)pv2, cur2);
                return retval ? retval < 0 ? -1 : +1 : 0;
        }
    }

    /* Here, if both are non-NULL, then they have the same UTF8ness. */

    if (!cur1) {
        cmp = cur2 ? -1 : 0;
    } else if (!cur2) {
        cmp = 1;
    } else {
        STRLEN shortest_len = cur1 < cur2 ? cur1 : cur2;

#ifdef EBCDIC
        if (! DO_UTF8(sv1)) {
#endif
            const I32 retval = memcmp((const void*)pv1,
                                      (const void*)pv2,
                                      shortest_len);
            if (retval) {
                cmp = retval < 0 ? -1 : 1;
            } else if (cur1 == cur2) {
                cmp = 0;
            } else {
                cmp = cur1 < cur2 ? -1 : 1;
            }
#ifdef EBCDIC
        }
        else {  /* Both are to be treated as UTF-EBCDIC */

            /* EBCDIC UTF-8 is complicated by the fact that it is based on I8
             * which remaps code points 0-255.  We therefore generally have to
             * unmap back to the original values to get an accurate comparison.
             * But we don't have to do that for UTF-8 invariants, as by
             * definition, they aren't remapped, nor do we have to do it for
             * above-latin1 code points, as they also aren't remapped.  (This
             * code also works on ASCII platforms, but the memcmp() above is
             * much faster). */

            const char *e = pv1 + shortest_len;

            /* Find the first bytes that differ between the two strings */
            while (pv1 < e && *pv1 == *pv2) {
                pv1++;
                pv2++;
            }


            if (pv1 == e) { /* Are the same all the way to the end */
                if (cur1 == cur2) {
                    cmp = 0;
                } else {
                    cmp = cur1 < cur2 ? -1 : 1;
                }
            }
            else   /* Here *pv1 and *pv2 are not equal, but all bytes earlier
                    * in the strings were.  The current bytes may or may not be
                    * at the beginning of a character.  But neither or both are
                    * (or else earlier bytes would have been different).  And
                    * if we are in the middle of a character, the two
                    * characters are comprised of the same number of bytes
                    * (because in this case the start bytes are the same, and
                    * the start bytes encode the character's length). */
                 if (UTF8_IS_INVARIANT(*pv1))
            {
                /* If both are invariants; can just compare directly */
                if (UTF8_IS_INVARIANT(*pv2)) {
                    cmp = ((U8) *pv1 < (U8) *pv2) ? -1 : 1;
                }
                else   /* Since *pv1 is invariant, it is the whole character,
                          which means it is at the beginning of a character.
                          That means pv2 is also at the beginning of a
                          character (see earlier comment).  Since it isn't
                          invariant, it must be a start byte.  If it starts a
                          character whose code point is above 255, that
                          character is greater than any single-byte char, which
                          *pv1 is */
                      if (UTF8_IS_ABOVE_LATIN1_START(*pv2))
                {
                    cmp = -1;
                }
                else {
                    /* Here, pv2 points to a character composed of 2 bytes
                     * whose code point is < 256.  Get its code point and
                     * compare with *pv1 */
                    cmp = ((U8) *pv1 < EIGHT_BIT_UTF8_TO_NATIVE(*pv2, *(pv2 + 1)))
                           ?  -1
                           : 1;
                }
            }
            else   /* The code point starting at pv1 isn't a single byte */
                 if (UTF8_IS_INVARIANT(*pv2))
            {
                /* But here, the code point starting at *pv2 is a single byte,
                 * and so *pv1 must begin a character, hence is a start byte.
                 * If that character is above 255, it is larger than any
                 * single-byte char, which *pv2 is */
                if (UTF8_IS_ABOVE_LATIN1_START(*pv1)) {
                    cmp = 1;
                }
                else {
                    /* Here, pv1 points to a character composed of 2 bytes
                     * whose code point is < 256.  Get its code point and
                     * compare with the single byte character *pv2 */
                    cmp = (EIGHT_BIT_UTF8_TO_NATIVE(*pv1, *(pv1 + 1)) < (U8) *pv2)
                          ?  -1
                          : 1;
                }
            }
            else   /* Here, we've ruled out either *pv1 and *pv2 being
                      invariant.  That means both are part of variants, but not
                      necessarily at the start of a character */
                 if (   UTF8_IS_ABOVE_LATIN1_START(*pv1)
                     || UTF8_IS_ABOVE_LATIN1_START(*pv2))
            {
                /* Here, at least one is the start of a character, which means
                 * the other is also a start byte.  And the code point of at
                 * least one of the characters is above 255.  It is a
                 * characteristic of UTF-EBCDIC that all start bytes for
                 * above-latin1 code points are well behaved as far as code
                 * point comparisons go, and all are larger than all other
                 * start bytes, so the comparison with those is also well
                 * behaved */
                cmp = ((U8) *pv1 < (U8) *pv2) ? -1 : 1;
            }
            else {
                /* Here both *pv1 and *pv2 are part of variant characters.
                 * They could be both continuations, or both start characters.
                 * (One or both could even be an illegal start character (for
                 * an overlong) which for the purposes of sorting we treat as
                 * legal. */
                if (UTF8_IS_CONTINUATION(*pv1)) {

                    /* If they are continuations for code points above 255,
                     * then comparing the current byte is sufficient, as there
                     * is no remapping of these and so the comparison is
                     * well-behaved.   We determine if they are such
                     * continuations by looking at the preceding byte.  It
                     * could be a start byte, from which we can tell if it is
                     * for an above 255 code point.  Or it could be a
                     * continuation, which means the character occupies at
                     * least 3 bytes, so must be above 255.  */
                    if (   UTF8_IS_CONTINUATION(*(pv2 - 1))
                        || UTF8_IS_ABOVE_LATIN1_START(*(pv2 -1)))
                    {
                        cmp = ((U8) *pv1 < (U8) *pv2) ? -1 : 1;
                        goto cmp_done;
                    }

                    /* Here, the continuations are for code points below 256;
                     * back up one to get to the start byte */
                    pv1--;
                    pv2--;
                }

                /* We need to get the actual native code point of each of these
                 * variants in order to compare them */
                cmp =  (  EIGHT_BIT_UTF8_TO_NATIVE(*pv1, *(pv1 + 1))
                        < EIGHT_BIT_UTF8_TO_NATIVE(*pv2, *(pv2 + 1)))
                        ? -1
                        : 1;
            }
        }
      cmp_done: ;
#endif
    }

    SvREFCNT_dec(svrecode);

    return cmp;
}

/*
=for apidoc sv_cmp_locale

Compares the strings in two SVs in a locale-aware manner.  Is UTF-8 and
S<C<'use bytes'>> aware, handles get magic, and will coerce its args to strings
if necessary.  See also C<L</sv_cmp>>.

=for apidoc sv_cmp_locale_flags

Compares the strings in two SVs in a locale-aware manner.  Is UTF-8 and
S<C<'use bytes'>> aware and will coerce its args to strings if necessary.  If
the flags contain C<SV_GMAGIC>, it handles get magic.  See also
C<L</sv_cmp_flags>>.

=cut
*/

I32
Perl_sv_cmp_locale(pTHX_ SV *const sv1, SV *const sv2)
{
    return sv_cmp_locale_flags(sv1, sv2, SV_GMAGIC);
}

I32
Perl_sv_cmp_locale_flags(pTHX_ SV *const sv1, SV *const sv2,
                         const U32 flags)
{
#ifdef USE_LOCALE_COLLATE

    char *pv1, *pv2;
    STRLEN len1, len2;
    I32 retval;

    if (PL_collation_standard)
        goto raw_compare;

    len1 = len2 = 0;

    /* Revert to using raw compare if both operands exist, but either one
     * doesn't transform properly for collation */
    if (sv1 && sv2) {
        pv1 = sv_collxfrm_flags(sv1, &len1, flags);
        if (! pv1) {
            goto raw_compare;
        }
        pv2 = sv_collxfrm_flags(sv2, &len2, flags);
        if (! pv2) {
            goto raw_compare;
        }
    }
    else {
        pv1 = sv1 ? sv_collxfrm_flags(sv1, &len1, flags) : (char *) NULL;
        pv2 = sv2 ? sv_collxfrm_flags(sv2, &len2, flags) : (char *) NULL;
    }

    if (!pv1 || !len1) {
        if (pv2 && len2)
            return -1;
        else
            goto raw_compare;
    }
    else {
        if (!pv2 || !len2)
            return 1;
    }

    retval = memcmp((void*)pv1, (void*)pv2, len1 < len2 ? len1 : len2);

    if (retval)
        return retval < 0 ? -1 : 1;

    /*
     * When the result of collation is equality, that doesn't mean
     * that there are no differences -- some locales exclude some
     * characters from consideration.  So to avoid false equalities,
     * we use the raw string as a tiebreaker.
     */

  raw_compare:
    /* FALLTHROUGH */

#else
    PERL_UNUSED_ARG(flags);
#endif /* USE_LOCALE_COLLATE */

    return sv_cmp(sv1, sv2);
}


#ifdef USE_LOCALE_COLLATE

/*
=for apidoc sv_collxfrm

This calls C<sv_collxfrm_flags> with the SV_GMAGIC flag.  See
C<L</sv_collxfrm_flags>>.

=for apidoc sv_collxfrm_flags

Add Collate Transform magic to an SV if it doesn't already have it.  If the
flags contain C<SV_GMAGIC>, it handles get-magic.

Any scalar variable may carry C<PERL_MAGIC_collxfrm> magic that contains the
scalar data of the variable, but transformed to such a format that a normal
memory comparison can be used to compare the data according to the locale
settings.

=cut
*/

char *
Perl_sv_collxfrm_flags(pTHX_ SV *const sv, STRLEN *const nxp, const I32 flags)
{
    MAGIC *mg;

    PERL_ARGS_ASSERT_SV_COLLXFRM_FLAGS;

    mg = SvMAGICAL(sv) ? mg_find(sv, PERL_MAGIC_collxfrm) : (MAGIC *) NULL;

    /* If we don't have collation magic on 'sv', or the locale has changed
     * since the last time we calculated it, get it and save it now */
    if (!mg || !mg->mg_ptr || *(U32*)mg->mg_ptr != PL_collation_ix) {
        const char *s;
        char *xf;
        STRLEN len, xlen;

        /* Free the old space */
        if (mg)
            Safefree(mg->mg_ptr);

        s = SvPV_flags_const(sv, len, flags);
        if ((xf = mem_collxfrm_(s, len, &xlen, cBOOL(SvUTF8(sv))))) {
            if (! mg) {
                mg = sv_magicext(sv, 0, PERL_MAGIC_collxfrm, &PL_vtbl_collxfrm,
                                 0, 0);
                assert(mg);
            }
            mg->mg_ptr = xf;
            mg->mg_len = xlen;
        }
        else {
            if (mg) {
                mg->mg_ptr = NULL;
                mg->mg_len = -1;
            }
        }
    }

    if (mg && mg->mg_ptr) {
        *nxp = mg->mg_len;
        return mg->mg_ptr + sizeof(PL_collation_ix);
    }
    else {
        *nxp = 0;
        return NULL;
    }
}

#endif /* USE_LOCALE_COLLATE */

static char *
S_sv_gets_append_to_utf8(pTHX_ SV *const sv, PerlIO *const fp, I32 append)
{
    SV * const tsv = newSV_type(SVt_NULL);
    ENTER;
    SAVEFREESV(tsv);
    sv_gets(tsv, fp, 0);
    sv_utf8_upgrade_nomg(tsv);
    SvCUR_set(sv,append);
    sv_catsv(sv,tsv);
    LEAVE;
    return (SvCUR(sv) - append) ? SvPVX(sv) : NULL;
}

static char *
S_sv_gets_read_record(pTHX_ SV *const sv, PerlIO *const fp, I32 append)
{
    SSize_t bytesread;
    const STRLEN recsize = SvUV(SvRV(PL_rs)); /* RsRECORD() guarantees > 0. */
      /* Grab the size of the record we're getting */
    char *buffer = SvGROW(sv, (STRLEN)(recsize + append + 1)) + append;

    /* Go yank in */
#ifdef __VMS
    int fd;
    Stat_t st;

    /* With a true, record-oriented file on VMS, we need to use read directly
     * to ensure that we respect RMS record boundaries.  The user is responsible
     * for providing a PL_rs value that corresponds to the FAB$W_MRS (maximum
     * record size) field.  N.B. This is likely to produce invalid results on
     * varying-width character data when a record ends mid-character.
     */
    fd = PerlIO_fileno(fp);
    if (fd != -1
        && PerlLIO_fstat(fd, &st) == 0
        && (st.st_fab_rfm == FAB$C_VAR
            || st.st_fab_rfm == FAB$C_VFC
            || st.st_fab_rfm == FAB$C_FIX)) {

        bytesread = PerlLIO_read(fd, buffer, recsize);
    }
    else /* in-memory file from PerlIO::Scalar
          * or not a record-oriented file
          */
#endif
    {
        bytesread = PerlIO_read(fp, buffer, recsize);

        /* At this point, the logic in sv_get() means that sv will
           be treated as utf-8 if the handle is utf8.
        */
        if (PerlIO_isutf8(fp) && bytesread > 0) {
            char *bend = buffer + bytesread;
            char *bufp = buffer;
            size_t charcount = 0;
            bool charstart = TRUE;
            STRLEN skip = 0;

            while (charcount < recsize) {
                /* count accumulated characters */
                while (bufp < bend) {
                    if (charstart) {
                        skip = UTF8SKIP(bufp);
                    }
                    if (bufp + skip > bend) {
                        /* partial at the end */
                        charstart = FALSE;
                        break;
                    }
                    else {
                        ++charcount;
                        bufp += skip;
                        charstart = TRUE;
                    }
                }

                if (charcount < recsize) {
                    STRLEN readsize;
                    STRLEN bufp_offset = bufp - buffer;
                    SSize_t morebytesread;

                    /* originally I read enough to fill any incomplete
                       character and the first byte of the next
                       character if needed, but if there's many
                       multi-byte encoded characters we're going to be
                       making a read call for every character beyond
                       the original read size.

                       So instead, read the rest of the character if
                       any, and enough bytes to match at least the
                       start bytes for each character we're going to
                       read.
                    */
                    if (charstart)
                        readsize = recsize - charcount;
                    else
                        readsize = skip - (bend - bufp) + recsize - charcount - 1;
                    buffer = SvGROW(sv, append + bytesread + readsize + 1) + append;
                    bend = buffer + bytesread;
                    morebytesread = PerlIO_read(fp, bend, readsize);
                    if (morebytesread <= 0) {
                        /* we're done, if we still have incomplete
                           characters the check code in sv_gets() will
                           warn about them.

                           I'd originally considered doing
                           PerlIO_ungetc() on all but the lead
                           character of the incomplete character, but
                           read() doesn't do that, so I don't.
                        */
                        break;
                    }

                    /* prepare to scan some more */
                    bytesread += morebytesread;
                    bend = buffer + bytesread;
                    bufp = buffer + bufp_offset;
                }
            }
        }
    }

    if (bytesread < 0)
        bytesread = 0;
    SvCUR_set(sv, bytesread + append);
    buffer[bytesread] = '\0';
    return (SvCUR(sv) - append) ? SvPVX(sv) : NULL;
}

/*
=for apidoc sv_gets

Get a line from the filehandle and store it into the SV, optionally
appending to the currently-stored string.  If C<append> is not 0, the
line is appended to the SV instead of overwriting it.  C<append> should
be set to the byte offset that the appended string should start at
in the SV (typically, C<SvCUR(sv)> is a suitable choice).

=cut
*/

char *
Perl_sv_gets(pTHX_ SV *const sv, PerlIO *const fp, I32 append)
{
    const char *rsptr;
    STRLEN rslen;
    STDCHAR rslast;
    STDCHAR *bp;
    SSize_t cnt;
    int i = 0;
    int rspara = 0;

    PERL_ARGS_ASSERT_SV_GETS;

    if (SvTHINKFIRST(sv))
        sv_force_normal_flags(sv, append ? 0 : SV_COW_DROP_PV);
    /* XXX. If you make this PVIV, then copy on write can copy scalars read
       from <>.
       However, perlbench says it's slower, because the existing swipe code
       is faster than copy on write.
       Swings and roundabouts.  */
    SvUPGRADE(sv, SVt_PV);

    if (append) {
        /* line is going to be appended to the existing buffer in the sv */
        if (PerlIO_isutf8(fp)) {
            if (!SvUTF8(sv)) {
                sv_utf8_upgrade_nomg(sv);
                sv_pos_u2b(sv,&append,0);
            }
        } else if (SvUTF8(sv)) {
            return S_sv_gets_append_to_utf8(aTHX_ sv, fp, append);
        }
    }

    SvPOK_only(sv);
    if (!append) {
        /* not appending - "clear" the string by setting SvCUR to 0,
         * the pv is still available. */
        SvCUR_set(sv,0);
    }
    if (PerlIO_isutf8(fp))
        SvUTF8_on(sv);

    if (IN_PERL_COMPILETIME) {
        /* we always read code in line mode */
        rsptr = "\n";
        rslen = 1;
    }
    else if (RsSNARF(PL_rs)) {
        /* If it is a regular disk file use size from stat() as estimate
           of amount we are going to read -- may result in mallocing
           more memory than we really need if the layers below reduce
           the size we read (e.g. CRLF or a gzip layer).
         */
        Stat_t st;
        int fd = PerlIO_fileno(fp);
        if (fd >= 0 && (PerlLIO_fstat(fd, &st) == 0) && S_ISREG(st.st_mode))  {
            const Off_t offset = PerlIO_tell(fp);
            if (offset != (Off_t) -1 && st.st_size + append > offset) {
#ifdef PERL_COPY_ON_WRITE
                /* Add an extra byte for the sake of copy-on-write's
                 * buffer reference count. */
                (void) SvGROW(sv, (STRLEN)((st.st_size - offset) + append + 2));
#else
                (void) SvGROW(sv, (STRLEN)((st.st_size - offset) + append + 1));
#endif
            }
        }
        rsptr = NULL;
        rslen = 0;
    }
    else if (RsRECORD(PL_rs)) {
        return S_sv_gets_read_record(aTHX_ sv, fp, append);
    }
    else if (RsPARA(PL_rs)) {
        rsptr = "\n\n";
        rslen = 2;
        rspara = 1;
    }
    else {
        /* Get $/ i.e. PL_rs into same encoding as stream wants */
        if (PerlIO_isutf8(fp)) {
            rsptr = SvPVutf8(PL_rs, rslen);
        }
        else {
            if (SvUTF8(PL_rs)) {
                if (!sv_utf8_downgrade(PL_rs, TRUE)) {
                    Perl_croak(aTHX_ "Wide character in $/");
                }
            }
            /* extract the raw pointer to the record separator */
            rsptr = SvPV_const(PL_rs, rslen);
        }
    }

    /* rslast is the last character in the record separator
     * note we don't use rslast except when rslen is true, so the
     * null assign is a placeholder. */
    rslast = rslen ? rsptr[rslen - 1] : '\0';

    if (rspara) {        /* have to do this both before and after */
                         /* to make sure file boundaries work right */
        while (1) {
            if (PerlIO_eof(fp))
                return 0;
            i = PerlIO_getc(fp);
            if (i != '\n') {
                if (i == -1)
                    return 0;
                PerlIO_ungetc(fp,i);
                break;
            }
        }
    }

    /* See if we know enough about I/O mechanism to cheat it ! */

    /* This used to be #ifdef test - it is made run-time test for ease
       of abstracting out stdio interface. One call should be cheap
       enough here - and may even be a macro allowing compile
       time optimization.
     */

    if (PerlIO_fast_gets(fp)) {
    /*
     * We can do buffer based IO operations on this filehandle.
     *
     * This means we can bypass a lot of subcalls and process
     * the buffer directly, it also means we know the upper bound
     * on the amount of data we might read of the current buffer
     * into our sv. Knowing this allows us to preallocate the pv
     * to be able to hold that maximum, which allows us to simplify
     * a lot of logic. */

    /*
     * We're going to steal some values from the stdio struct
     * and put EVERYTHING in the innermost loop into registers.
     */
    STDCHAR *ptr;       /* pointer into fp's read-ahead buffer */
    STRLEN bpx;         /* length of the data in the target sv
                           used to fix pointers after a SvGROW */
    I32 shortbuffered;  /* If the pv buffer is shorter than the amount
                           of data left in the read-ahead buffer.
                           If 0 then the pv buffer can hold the full
                           amount left, otherwise this is the amount it
                           can hold. */

    /* Here is some breathtakingly efficient cheating */

    /* When you read the following logic resist the urge to think
     * of record separators that are 1 byte long. They are an
     * uninteresting special (simple) case.
     *
     * Instead think of record separators which are at least 2 bytes
     * long, and keep in mind that we need to deal with such
     * separators when they cross a read-ahead buffer boundary.
     *
     * Also consider that we need to gracefully deal with separators
     * that may be longer than a single read ahead buffer.
     *
     * Lastly do not forget we want to copy the delimiter as well. We
     * are copying all data in the file _up_to_and_including_ the separator
     * itself.
     *
     * Now that you have all that in mind here is what is happening below:
     *
     * 1. When we first enter the loop we do some memory book keeping to see
     * how much free space there is in the target SV. (This sub assumes that
     * it is operating on the same SV most of the time via $_ and that it is
     * going to be able to reuse the same pv buffer each call.) If there is
     * "enough" room then we set "shortbuffered" to how much space there is
     * and start reading forward.
     *
     * 2. When we scan forward we copy from the read-ahead buffer to the target
     * SV's pv buffer. While we go we watch for the end of the read-ahead buffer,
     * and the end of the of pv, as well as for the "rslast", which is the last
     * char of the separator.
     *
     * 3. When scanning forward if we see rslast then we jump backwards in *pv*
     * (which has a "complete" record up to the point we saw rslast) and check
     * it to see if it matches the separator. If it does we are done. If it doesn't
     * we continue on with the scan/copy.
     *
     * 4. If we run out of read-ahead buffer (cnt goes to 0) then we have to get
     * the IO system to read the next buffer. We do this by doing a getc(), which
     * returns a single char read (or EOF), and prefills the buffer, and also
     * allows us to find out how full the buffer is.  We use this information to
     * SvGROW() the sv to the size remaining in the buffer, after which we copy
     * the returned single char into the target sv, and then go back into scan
     * forward mode.
     *
     * 5. If we run out of write-buffer then we SvGROW() it by the size of the
     * remaining space in the read-buffer.
     *
     * Note that this code despite its twisty-turny nature is pretty darn slick.
     * It manages single byte separators, multi-byte cross boundary separators,
     * and cross-read-buffer separators cleanly and efficiently at the cost
     * of potentially greatly overallocating the target SV.
     *
     * Yves
     */


    /* get the number of bytes remaining in the read-ahead buffer
     * on first call on a given fp this will return 0.*/
    cnt = PerlIO_get_cnt(fp);

    /* make sure we have the room */
    if ((I32)(SvLEN(sv) - append) <= cnt + 1) {
        /* Not room for all of it
           if we are looking for a separator and room for some
         */
        if (rslen && cnt > 80 && (I32)SvLEN(sv) > append) {
            /* just process what we have room for */
            shortbuffered = cnt - SvLEN(sv) + append + 1;
            cnt -= shortbuffered;
        }
        else {
            /* ensure that the target sv has enough room to hold
             * the rest of the read-ahead buffer */
            shortbuffered = 0;
            /* remember that cnt can be negative */
            SvGROW(sv, (STRLEN)(append + (cnt <= 0 ? 2 : (cnt + 1))));
        }
    }
    else {
        /* we have enough room to hold the full buffer, lets scream */
        shortbuffered = 0;
    }

    /* extract the pointer to sv's string buffer, offset by append as necessary */
    bp = (STDCHAR*)SvPVX_const(sv) + append;  /* move these two too to registers */
    /* extract the point to the read-ahead buffer */
    ptr = (STDCHAR*)PerlIO_get_ptr(fp);

    /* some trace debug output */
    DEBUG_P(PerlIO_printf(Perl_debug_log,
        "Screamer: entering, ptr=%" UVuf ", cnt=%ld\n",PTR2UV(ptr),(long)cnt));
    DEBUG_P(PerlIO_printf(Perl_debug_log,
        "Screamer: entering: PerlIO * thinks ptr=%" UVuf ", cnt=%" IVdf ", base=%"
         UVuf "\n",
               PTR2UV(PerlIO_get_ptr(fp)), (IV)PerlIO_get_cnt(fp),
               PTR2UV(PerlIO_has_base(fp) ? PerlIO_get_base(fp) : 0)));

    for (;;) {
      screamer:
        /* if there is stuff left in the read-ahead buffer */
        if (cnt > 0) {
            /* if there is a separator */
            if (rslen) {
                /* find next rslast */
                STDCHAR *p;

                /* shortcut common case of blank line */
                cnt--;
                if ((*bp++ = *ptr++) == rslast)
                    goto thats_all_folks;

                p = (STDCHAR *)memchr(ptr, rslast, cnt);
                if (p) {
                    SSize_t got = p - ptr + 1;
                    Copy(ptr, bp, got, STDCHAR);
                    ptr += got;
                    bp  += got;
                    cnt -= got;
                    goto thats_all_folks;
                }
                Copy(ptr, bp, cnt, STDCHAR);
                ptr += cnt;
                bp  += cnt;
                cnt = 0;
            }
            else {
                /* no separator, slurp the full buffer */
                Copy(ptr, bp, cnt, char);	     /* this     |  eat */
                bp += cnt;			     /* screams  |  dust */
                ptr += cnt;			     /* louder   |  sed :-) */
                cnt = 0;
                assert (!shortbuffered);
                goto cannot_be_shortbuffered;
            }
        }

        if (shortbuffered) {		/* oh well, must extend */
            /* we didn't have enough room to fit the line into the target buffer
             * so we must extend the target buffer and keep going */
            cnt = shortbuffered;
            shortbuffered = 0;
            bpx = bp - (STDCHAR*)SvPVX_const(sv); /* box up before relocation */
            SvCUR_set(sv, bpx);
            /* extned the target sv's buffer so it can hold the full read-ahead buffer */
            SvGROW(sv, SvLEN(sv) + append + cnt + 2);
            bp = (STDCHAR*)SvPVX_const(sv) + bpx; /* unbox after relocation */
            continue;
        }

    cannot_be_shortbuffered:
        /* we need to refill the read-ahead buffer if possible */

        DEBUG_P(PerlIO_printf(Perl_debug_log,
                             "Screamer: going to getc, ptr=%" UVuf ", cnt=%" IVdf "\n",
                              PTR2UV(ptr),(IV)cnt));
        PerlIO_set_ptrcnt(fp, (STDCHAR*)ptr, cnt); /* deregisterize cnt and ptr */

        DEBUG_Pv(PerlIO_printf(Perl_debug_log,
           "Screamer: pre: FILE * thinks ptr=%" UVuf ", cnt=%" IVdf ", base=%" UVuf "\n",
            PTR2UV(PerlIO_get_ptr(fp)), (IV)PerlIO_get_cnt(fp),
            PTR2UV(PerlIO_has_base (fp) ? PerlIO_get_base(fp) : 0)));

        /*
            call PerlIO_getc() to let it prefill the lookahead buffer

            This used to call 'filbuf' in stdio form, but as that behaves like
            getc when cnt <= 0 we use PerlIO_getc here to avoid introducing
            another abstraction.

            Note we have to deal with the char in 'i' if we are not at EOF
        */
        bpx = bp - (STDCHAR*)SvPVX_const(sv);
        /* signals might be called here, possibly modifying sv */
        i   = PerlIO_getc(fp);		/* get more characters */
        bp = (STDCHAR*)SvPVX_const(sv) + bpx;

        DEBUG_Pv(PerlIO_printf(Perl_debug_log,
           "Screamer: post: FILE * thinks ptr=%" UVuf ", cnt=%" IVdf ", base=%" UVuf "\n",
            PTR2UV(PerlIO_get_ptr(fp)), (IV)PerlIO_get_cnt(fp),
            PTR2UV(PerlIO_has_base (fp) ? PerlIO_get_base(fp) : 0)));

        /* find out how much is left in the read-ahead buffer, and rextract its pointer */
        cnt = PerlIO_get_cnt(fp);
        ptr = (STDCHAR*)PerlIO_get_ptr(fp);	/* reregisterize cnt and ptr */
        DEBUG_P(PerlIO_printf(Perl_debug_log,
            "Screamer: after getc, ptr=%" UVuf ", cnt=%" IVdf "\n",
            PTR2UV(ptr),(IV)cnt));

        if (i == EOF)			/* all done for ever? */
            goto thats_really_all_folks;

        /* make sure we have enough space in the target sv */
        bpx = bp - (STDCHAR*)SvPVX_const(sv);	/* box up before relocation */
        SvCUR_set(sv, bpx);
        SvGROW(sv, bpx + cnt + 2);
        bp = (STDCHAR*)SvPVX_const(sv) + bpx;	/* unbox after relocation */

        /* copy of the char we got from getc() */
        *bp++ = (STDCHAR)i;		/* store character from PerlIO_getc */

        /* make sure we deal with the i being the last character of a separator */
        if (rslen && (STDCHAR)i == rslast)  /* all done for now? */
            goto thats_all_folks;
    }

  thats_all_folks:
    /* check if we have actually found the separator - only really applies
     * when rslen > 1 */
    if ((rslen > 1 && (STRLEN)(bp - (STDCHAR*)SvPVX_const(sv)) < rslen) ||
          memNE((char*)bp - rslen, rsptr, rslen))
        goto screamer;				/* go back to the fray */
  thats_really_all_folks:
    if (shortbuffered)
        cnt += shortbuffered;
    DEBUG_P(PerlIO_printf(Perl_debug_log,
         "Screamer: quitting, ptr=%" UVuf ", cnt=%" IVdf "\n",PTR2UV(ptr),(IV)cnt));
    PerlIO_set_ptrcnt(fp, (STDCHAR*)ptr, cnt);	/* put these back or we're in trouble */
    DEBUG_P(PerlIO_printf(Perl_debug_log,
        "Screamer: end: FILE * thinks ptr=%" UVuf ", cnt=%" IVdf ", base=%" UVuf
        "\n",
        PTR2UV(PerlIO_get_ptr(fp)), (IV)PerlIO_get_cnt(fp),
        PTR2UV(PerlIO_has_base (fp) ? PerlIO_get_base(fp) : 0)));
    *bp = '\0';
    SvCUR_set(sv, bp - (STDCHAR*)SvPVX_const(sv));	/* set length */
    DEBUG_P(PerlIO_printf(Perl_debug_log,
        "Screamer: done, len=%ld, string=|%.*s|\n",
        (long)SvCUR(sv),(int)SvCUR(sv),SvPVX_const(sv)));
    }
   else
    {
       /*The big, slow, and stupid way. */
        STDCHAR buf[8192];

      screamer2:
        if (rslen) {
            const STDCHAR * const bpe = buf + sizeof(buf);
            bp = buf;
            while ((i = PerlIO_getc(fp)) != EOF && (*bp++ = (STDCHAR)i) != rslast && bp < bpe)
                ; /* keep reading */
            cnt = bp - buf;
        }
        else {
            cnt = PerlIO_read(fp,(char*)buf, sizeof(buf));
            /* Accommodate broken VAXC compiler, which applies U8 cast to
             * both args of ?: operator, causing EOF to change into 255
             */
            if (cnt > 0)
                 i = (U8)buf[cnt - 1];
            else
                 i = EOF;
        }

        if (cnt < 0)
            cnt = 0;  /* we do need to re-set the sv even when cnt <= 0 */
        if (append)
            sv_catpvn_nomg(sv, (char *) buf, cnt);
        else
            sv_setpvn(sv, (char *) buf, cnt);   /* "nomg" is implied */

        if (i != EOF &&			/* joy */
            (!rslen ||
             SvCUR(sv) < rslen ||
             memNE(SvPVX_const(sv) + SvCUR(sv) - rslen, rsptr, rslen)))
        {
            append = -1;
            /*
             * If we're reading from a TTY and we get a short read,
             * indicating that the user hit his EOF character, we need
             * to notice it now, because if we try to read from the TTY
             * again, the EOF condition will disappear.
             *
             * The comparison of cnt to sizeof(buf) is an optimization
             * that prevents unnecessary calls to feof().
             *
             * - jik 9/25/96
             */
            if (!(cnt < (I32)sizeof(buf) && PerlIO_eof(fp)))
                goto screamer2;
        }

    }

    if (rspara) {		/* have to do this both before and after */
        while (i != EOF) {	/* to make sure file boundaries work right */
            i = PerlIO_getc(fp);
            if (i != '\n') {
                PerlIO_ungetc(fp,i);
                break;
            }
        }
    }

    return (SvCUR(sv) - append) ? SvPVX(sv) : NULL;
}

/*
=for apidoc sv_inc
=for apidoc_item sv_inc_nomg

These auto-increment the value in the SV, doing string to numeric conversion
if necessary.  They both handle operator overloading.

They differ only in that C<sv_inc> performs 'get' magic; C<sv_inc_nomg> skips
any magic.

=cut
*/

void
Perl_sv_inc(pTHX_ SV *const sv)
{
    if (!sv)
        return;
    SvGETMAGIC(sv);
    sv_inc_nomg(sv);
}

void
Perl_sv_inc_nomg(pTHX_ SV *const sv)
{
    char *d;
    int flags;

    if (!sv)
        return;
    if (SvTHINKFIRST(sv)) {
        if (SvREADONLY(sv)) {
                Perl_croak_no_modify();
        }
        if (SvROK(sv)) {
            IV i;
            if (SvAMAGIC(sv) && AMG_CALLunary(sv, inc_amg))
                return;
            i = PTR2IV(SvRV(sv));
            sv_unref(sv);
            sv_setiv(sv, i);
        }
        else sv_force_normal_flags(sv, 0);
    }
    flags = SvFLAGS(sv);
    if ((flags & (SVp_NOK|SVp_IOK)) == SVp_NOK) {
        /* It's (privately or publicly) a float, but not tested as an
           integer, so test it to see. */
        (void) SvIV(sv);
        flags = SvFLAGS(sv);
    }
    if ((flags & SVf_IOK) || ((flags & (SVp_IOK | SVp_NOK)) == SVp_IOK)) {
        /* It's publicly an integer, or privately an integer-not-float */
#ifdef PERL_PRESERVE_IVUV
      oops_its_int:
#endif
        if (SvIsUV(sv)) {
            if (SvUVX(sv) == UV_MAX)
                sv_setnv(sv, UV_MAX_P1);
            else {
                (void)SvIOK_only_UV(sv);
                SvUV_set(sv, SvUVX(sv) + 1);
            }
        } else {
            if (SvIVX(sv) == IV_MAX)
                sv_setuv(sv, (UV)IV_MAX + 1);
            else {
                (void)SvIOK_only(sv);
                SvIV_set(sv, SvIVX(sv) + 1);
            }
        }
        return;
    }
    if (flags & SVp_NOK) {
        const NV was = SvNVX(sv);
        if (NV_OVERFLOWS_INTEGERS_AT != 0.0 &&
            /* If NVX was NaN, the following comparisons return always false */
            UNLIKELY(was >= NV_OVERFLOWS_INTEGERS_AT ||
                     was < -NV_OVERFLOWS_INTEGERS_AT) &&
#if defined(NAN_COMPARE_BROKEN)
            LIKELY(!Perl_isinfnan(was))
#else
            LIKELY(!Perl_isinf(was))
#endif
            ) {
            /* diag_listed_as: Lost precision when %s %f by 1 */
            Perl_ck_warner(aTHX_ packWARN(WARN_IMPRECISION),
                           "Lost precision when incrementing %" NVff " by 1",
                           was);
        }
        (void)SvNOK_only(sv);
        SvNV_set(sv, was + 1.0);
        return;
    }

    /* treat AV/HV/CV/FM/IO and non-fake GVs as immutable */
    if (SvTYPE(sv) >= SVt_PVAV || (isGV_with_GP(sv) && !SvFAKE(sv)))
        Perl_croak_no_modify();

    if (!(flags & SVp_POK) || !*SvPVX_const(sv)) {
        if ((flags & SVTYPEMASK) < SVt_PVIV)
            sv_upgrade(sv, ((flags & SVTYPEMASK) > SVt_IV ? SVt_PVIV : SVt_IV));
        (void)SvIOK_only(sv);
        SvIV_set(sv, 1);
        return;
    }
    d = SvPVX(sv);
    while (isALPHA(*d)) d++;
    while (isDIGIT(*d)) d++;
    if (d < SvEND(sv)) {
        const int numtype = grok_number_flags(SvPVX_const(sv), SvCUR(sv), NULL, PERL_SCAN_TRAILING);
#ifdef PERL_PRESERVE_IVUV
        /* Got to punt this as an integer if needs be, but we don't issue
           warnings. Probably ought to make the sv_iv_please() that does
           the conversion if possible, and silently.  */
        if (numtype && !(numtype & IS_NUMBER_INFINITY)) {
            /* Need to try really hard to see if it's an integer.
               9.22337203685478e+18 is an integer.
               but "9.22337203685478e+18" + 0 is UV=9223372036854779904
               so $a="9.22337203685478e+18"; $a+0; $a++
               needs to be the same as $a="9.22337203685478e+18"; $a++
               or we go insane. */

            (void) sv_2iv(sv);
            if (SvIOK(sv))
                goto oops_its_int;

            /* sv_2iv *should* have made this an NV */
            if (flags & SVp_NOK) {
                (void)SvNOK_only(sv);
                SvNV_set(sv, SvNVX(sv) + 1.0);
                return;
            }
            /* I don't think we can get here. Maybe I should assert this
               And if we do get here I suspect that sv_setnv will croak. NWC
               Fall through. */
            DEBUG_c(PerlIO_printf(Perl_debug_log,"sv_inc punt failed to convert '%s' to IOK or NOKp, UV=0x%" UVxf " NV=%" NVgf "\n",
                                  SvPVX_const(sv), SvIVX(sv), SvNVX(sv)));
        }
#endif /* PERL_PRESERVE_IVUV */
        if (!numtype && ckWARN(WARN_NUMERIC))
            not_incrementable(sv);
        sv_setnv(sv,Atof(SvPVX_const(sv)) + 1.0);
        return;
    }
    d--;
    while (d >= SvPVX_const(sv)) {
        if (isDIGIT(*d)) {
            if (++*d <= '9')
                return;
            *(d--) = '0';
        }
        else {
#ifdef EBCDIC
            /* MKS: The original code here died if letters weren't consecutive.
             * at least it didn't have to worry about non-C locales.  The
             * new code assumes that ('z'-'a')==('Z'-'A'), letters are
             * arranged in order (although not consecutively) and that only
             * [A-Za-z] are accepted by isALPHA in the C locale.
             */
            if (isALPHA_FOLD_NE(*d, 'z')) {
                do { ++*d; } while (!isALPHA(*d));
                return;
            }
            *(d--) -= 'z' - 'a';
#else
            ++*d;
            if (isALPHA(*d))
                return;
            *(d--) -= 'z' - 'a' + 1;
#endif
        }
    }
    /* oh,oh, the number grew */
    SvGROW(sv, SvCUR(sv) + 2);
    SvCUR_set(sv, SvCUR(sv) + 1);
    for (d = SvPVX(sv) + SvCUR(sv); d > SvPVX_const(sv); d--)
        *d = d[-1];
    if (isDIGIT(d[1]))
        *d = '1';
    else
        *d = d[1];
}

/*
=for apidoc sv_dec
=for apidoc_item sv_dec_nomg

These auto-decrement the value in the SV, doing string to numeric conversion
if necessary.  They both handle operator overloading.

They differ only in that:

C<sv_dec> handles 'get' magic; C<sv_dec_nomg> skips 'get' magic.

=cut
*/

void
Perl_sv_dec(pTHX_ SV *const sv)
{
    if (!sv)
        return;
    SvGETMAGIC(sv);
    sv_dec_nomg(sv);
}

void
Perl_sv_dec_nomg(pTHX_ SV *const sv)
{
    int flags;

    if (!sv)
        return;
    if (SvTHINKFIRST(sv)) {
        if (SvREADONLY(sv)) {
                Perl_croak_no_modify();
        }
        if (SvROK(sv)) {
            IV i;
            if (SvAMAGIC(sv) && AMG_CALLunary(sv, dec_amg))
                return;
            i = PTR2IV(SvRV(sv));
            sv_unref(sv);
            sv_setiv(sv, i);
        }
        else sv_force_normal_flags(sv, 0);
    }
    /* Unlike sv_inc we don't have to worry about string-never-numbers
       and keeping them magic. But we mustn't warn on punting */
    flags = SvFLAGS(sv);
    if ((flags & SVf_IOK) || ((flags & (SVp_IOK | SVp_NOK)) == SVp_IOK)) {
        /* It's publicly an integer, or privately an integer-not-float */
#ifdef PERL_PRESERVE_IVUV
      oops_its_int:
#endif
        if (SvIsUV(sv)) {
            if (SvUVX(sv) == 0) {
                (void)SvIOK_only(sv);
                SvIV_set(sv, -1);
            }
            else {
                (void)SvIOK_only_UV(sv);
                SvUV_set(sv, SvUVX(sv) - 1);
            }
        } else {
            if (SvIVX(sv) == IV_MIN) {
                sv_setnv(sv, (NV)IV_MIN);
                goto oops_its_num;
            }
            else {
                (void)SvIOK_only(sv);
                SvIV_set(sv, SvIVX(sv) - 1);
            }
        }
        return;
    }
    if (flags & SVp_NOK) {
    oops_its_num:
        {
            const NV was = SvNVX(sv);
            if (NV_OVERFLOWS_INTEGERS_AT != 0.0 &&
                /* If NVX was NaN, these comparisons return always false */
                UNLIKELY(was <= -NV_OVERFLOWS_INTEGERS_AT ||
                         was > NV_OVERFLOWS_INTEGERS_AT) &&
#if defined(NAN_COMPARE_BROKEN)
                LIKELY(!Perl_isinfnan(was))
#else
                LIKELY(!Perl_isinf(was))
#endif
                ) {
                /* diag_listed_as: Lost precision when %s %f by 1 */
                Perl_ck_warner(aTHX_ packWARN(WARN_IMPRECISION),
                               "Lost precision when decrementing %" NVff " by 1",
                               was);
            }
            (void)SvNOK_only(sv);
            SvNV_set(sv, was - 1.0);
            return;
        }
    }

    /* treat AV/HV/CV/FM/IO and non-fake GVs as immutable */
    if (SvTYPE(sv) >= SVt_PVAV || (isGV_with_GP(sv) && !SvFAKE(sv)))
        Perl_croak_no_modify();

    if (!(flags & SVp_POK)) {
        if ((flags & SVTYPEMASK) < SVt_PVIV)
            sv_upgrade(sv, ((flags & SVTYPEMASK) > SVt_IV) ? SVt_PVIV : SVt_IV);
        SvIV_set(sv, -1);
        (void)SvIOK_only(sv);
        return;
    }
#ifdef PERL_PRESERVE_IVUV
    {
        const int numtype = grok_number(SvPVX_const(sv), SvCUR(sv), NULL);
        if (numtype && !(numtype & IS_NUMBER_INFINITY)) {
            /* Need to try really hard to see if it's an integer.
               9.22337203685478e+18 is an integer.
               but "9.22337203685478e+18" + 0 is UV=9223372036854779904
               so $a="9.22337203685478e+18"; $a+0; $a--
               needs to be the same as $a="9.22337203685478e+18"; $a--
               or we go insane. */

            (void) sv_2iv(sv);
            if (SvIOK(sv))
                goto oops_its_int;

            /* sv_2iv *should* have made this an NV */
            if (flags & SVp_NOK) {
                (void)SvNOK_only(sv);
                SvNV_set(sv, SvNVX(sv) - 1.0);
                return;
            }
            /* I don't think we can get here. Maybe I should assert this
               And if we do get here I suspect that sv_setnv will croak. NWC
               Fall through. */
            DEBUG_c(PerlIO_printf(Perl_debug_log,"sv_dec punt failed to convert '%s' to IOK or NOKp, UV=0x%" UVxf " NV=%" NVgf "\n",
                                  SvPVX_const(sv), SvIVX(sv), SvNVX(sv)));
        }
    }
#endif /* PERL_PRESERVE_IVUV */
    sv_setnv(sv,Atof(SvPVX_const(sv)) - 1.0);	/* punt */
}

/* this define is used to eliminate a chunk of duplicated but shared logic
 * it has the suffix __SV_C to signal that it isnt API, and isnt meant to be
 * used anywhere but here - yves
 */
#define PUSH_EXTEND_MORTAL__SV_C(AnSv) \
    STMT_START {      \
        SSize_t ix = ++PL_tmps_ix;		\
        if (UNLIKELY(ix >= PL_tmps_max))	\
            ix = tmps_grow_p(ix);			\
        PL_tmps_stack[ix] = (AnSv); \
    } STMT_END

/*
=for apidoc sv_mortalcopy

Creates a new SV which is a copy of the original SV (using C<sv_setsv>).
The new SV is marked as mortal.  It will be destroyed "soon", either by an
explicit call to C<FREETMPS>, or by an implicit call at places such as
statement boundaries.  See also C<L</sv_newmortal>> and C<L</sv_2mortal>>.

=for apidoc sv_mortalcopy_flags

Like C<sv_mortalcopy>, but the extra C<flags> are passed to the
C<sv_setsv_flags>.

=cut
*/

/* Make a string that will exist for the duration of the expression
 * evaluation.  Actually, it may have to last longer than that, but
 * hopefully we won't free it until it has been assigned to a
 * permanent location. */

SV *
Perl_sv_mortalcopy_flags(pTHX_ SV *const oldstr, U32 flags)
{
    SV *sv;

    if (flags & SV_GMAGIC)
        SvGETMAGIC(oldstr); /* before new_SV, in case it dies */
    new_SV(sv);
    sv_setsv_flags(sv,oldstr,flags & ~SV_GMAGIC);
    PUSH_EXTEND_MORTAL__SV_C(sv);
    SvTEMP_on(sv);
    return sv;
}

/*
=for apidoc sv_newmortal

Creates a new null SV which is mortal.  The reference count of the SV is
set to 1.  It will be destroyed "soon", either by an explicit call to
C<FREETMPS>, or by an implicit call at places such as statement boundaries.
See also C<L</sv_mortalcopy>> and C<L</sv_2mortal>>.

=cut
*/

SV *
Perl_sv_newmortal(pTHX)
{
    SV *sv;

    new_SV(sv);
    SvFLAGS(sv) = SVs_TEMP;
    PUSH_EXTEND_MORTAL__SV_C(sv);
    return sv;
}


/*
=for apidoc newSVpvn_flags

Creates a new SV and copies a string (which may contain C<NUL> (C<\0>)
characters) into it.  The reference count for the
SV is set to 1.  Note that if C<len> is zero, Perl will create a zero length
string.  You are responsible for ensuring that the source string is at least
C<len> bytes long.  If the C<s> argument is NULL the new SV will be undefined.
Currently the only flag bits accepted are C<SVf_UTF8> and C<SVs_TEMP>.
If C<SVs_TEMP> is set, then C<sv_2mortal()> is called on the result before
returning.  If C<SVf_UTF8> is set, C<s>
is considered to be in UTF-8 and the
C<SVf_UTF8> flag will be set on the new SV.
C<newSVpvn_utf8()> is a convenience wrapper for this function, defined as

    #define newSVpvn_utf8(s, len, u)			\
        newSVpvn_flags((s), (len), (u) ? SVf_UTF8 : 0)

=for apidoc Amnh||SVs_TEMP

=cut
*/

SV *
Perl_newSVpvn_flags(pTHX_ const char *const s, const STRLEN len, const U32 flags)
{
    SV *sv;

    /* All the flags we don't support must be zero.
       And we're new code so I'm going to assert this from the start.  */
    assert(!(flags & ~(SVf_UTF8|SVs_TEMP)));
    sv = newSV_type(SVt_PV);
    sv_setpvn_fresh(sv,s,len);

    /* This code used to do a sv_2mortal(), however we now unroll the call to
     * sv_2mortal() and do what it does ourselves here.  Since we have asserted
     * that flags can only have the SVf_UTF8 and/or SVs_TEMP flags set above we
     * can use it to enable the sv flags directly (bypassing SvTEMP_on), which
     * in turn means we don't need to mask out the SVf_UTF8 flag below, which
     * means that we eliminate quite a few steps than it looks - Yves
     * (explaining patch by gfx) */

    SvFLAGS(sv) |= flags;

    if(flags & SVs_TEMP){
        PUSH_EXTEND_MORTAL__SV_C(sv);
    }

    return sv;
}

/*
=for apidoc sv_2mortal

Marks an existing SV as mortal.  The SV will be destroyed "soon", either
by an explicit call to C<FREETMPS>, or by an implicit call at places such as
statement boundaries.  C<SvTEMP()> is turned on which means that the SV's
string buffer can be "stolen" if this SV is copied.  See also
C<L</sv_newmortal>> and C<L</sv_mortalcopy>>.

=cut
*/

SV *
Perl_sv_2mortal(pTHX_ SV *const sv)
{
    if (!sv)
        return sv;
    if (SvIMMORTAL(sv))
        return sv;
    PUSH_EXTEND_MORTAL__SV_C(sv);
    SvTEMP_on(sv);
    return sv;
}

/*
=for apidoc newSVpv

Creates a new SV and copies a string (which may contain C<NUL> (C<\0>)
characters) into it.  The reference count for the
SV is set to 1.  If C<len> is zero, Perl will compute the length using
C<strlen()>, (which means if you use this option, that C<s> can't have embedded
C<NUL> characters and has to have a terminating C<NUL> byte).

This function can cause reliability issues if you are likely to pass in
empty strings that are not null terminated, because it will run
strlen on the string and potentially run past valid memory.

Using L</newSVpvn> is a safer alternative for non C<NUL> terminated strings.
For string literals use L</newSVpvs> instead.  This function will work fine for
C<NUL> terminated strings, but if you want to avoid the if statement on whether
to call C<strlen> use C<newSVpvn> instead (calling C<strlen> yourself).

=cut
*/

SV *
Perl_newSVpv(pTHX_ const char *const s, const STRLEN len)
{
    SV *sv = newSV_type(SVt_PV);
    sv_setpvn_fresh(sv, s, len || s == NULL ? len : strlen(s));
    return sv;
}

/*
=for apidoc newSVpvn

Creates a new SV and copies a string into it, which may contain C<NUL> characters
(C<\0>) and other binary data.  The reference count for the SV is set to 1.
Note that if C<len> is zero, Perl will create a zero length (Perl) string.  You
are responsible for ensuring that the source buffer is at least
C<len> bytes long.  If the C<buffer> argument is NULL the new SV will be
undefined.

=cut
*/

SV *
Perl_newSVpvn(pTHX_ const char *const buffer, const STRLEN len)
{
    SV *sv = newSV_type(SVt_PV);
    sv_setpvn_fresh(sv,buffer,len);
    return sv;
}

/*
=for apidoc newSVhek_mortal

Creates a new mortal SV from the hash key structure.  It will generate
scalars that point to the shared string table where possible.  Returns
a new (undefined) SV if C<hek> is NULL.

This is more efficient than using sv_2mortal(newSVhek( ... ))

=cut
*/

SV *
Perl_newSVhek_mortal(pTHX_ const HEK *const hek)
{
    SV * const sv = newSVhek(hek);
    assert(sv);
    assert(!SvIMMORTAL(sv));

    PUSH_EXTEND_MORTAL__SV_C(sv);
    SvTEMP_on(sv);
    return sv;
}

/*
=for apidoc newSVhek

Creates a new SV from the hash key structure.  It will generate scalars that
point to the shared string table where possible.  Returns a new (undefined)
SV if C<hek> is NULL.

=cut
*/

SV *
Perl_newSVhek(pTHX_ const HEK *const hek)
{
    if (!hek) {
        SV *sv;

        new_SV(sv);
        return sv;
    }

    if (HEK_LEN(hek) == HEf_SVKEY) {
        return newSVsv(*(SV**)HEK_KEY(hek));
    } else {
        const int flags = HEK_FLAGS(hek);
        if (flags & HVhek_WASUTF8) {
            /* Trouble :-)
               Andreas would like keys he put in as utf8 to come back as utf8
            */
            STRLEN utf8_len = HEK_LEN(hek);
            SV * const sv = newSV_type(SVt_PV);
            char *as_utf8 = (char *)bytes_to_utf8 ((U8*)HEK_KEY(hek), &utf8_len);
            /* bytes_to_utf8() allocates a new string, which we can repurpose: */
            sv_usepvn_flags(sv, as_utf8, utf8_len, SV_HAS_TRAILING_NUL);
            SvUTF8_on (sv);
            return sv;
        } else if (flags & HVhek_NOTSHARED) {
            /* A hash that isn't using shared hash keys has to have
               the flag in every key so that we know not to try to call
               share_hek_hek on it.  */

            SV * const sv = newSVpvn (HEK_KEY(hek), HEK_LEN(hek));
            if (HEK_UTF8(hek))
                SvUTF8_on (sv);
            return sv;
        }
        /* This will be overwhelmingly the most common case.  */
        {
            /* Inline most of newSVpvn_share(), because share_hek_hek() is far
               more efficient than sharepvn().  */
            SV *sv = newSV_type(SVt_PV);

            SvPV_set(sv, (char *)HEK_KEY(share_hek_hek(hek)));
            SvCUR_set(sv, HEK_LEN(hek));
            SvLEN_set(sv, 0);
            SvIsCOW_on(sv);
            SvPOK_on(sv);
            if (HEK_UTF8(hek))
                SvUTF8_on(sv);
            return sv;
        }
    }
}

/*
=for apidoc newSVpvn_share

Creates a new SV with its C<SvPVX_const> pointing to a shared string in the string
table.  If the string does not already exist in the table, it is
created first.  Turns on the C<SvIsCOW> flag (or C<READONLY>
and C<FAKE> in 5.16 and earlier).  If the C<hash> parameter
is non-zero, that value is used; otherwise the hash is computed.
The string's hash can later be retrieved from the SV
with the C<L</SvSHARED_HASH>> macro.  The idea here is
that as the string table is used for shared hash keys these strings will have
C<SvPVX_const == HeKEY> and hash lookup will avoid string compare.

=cut
*/

SV *
Perl_newSVpvn_share(pTHX_ const char *src, I32 len, U32 hash)
{
    SV *sv;
    bool is_utf8 = FALSE;
    const char *const orig_src = src;

    if (len < 0) {
        STRLEN tmplen = -len;
        is_utf8 = TRUE;
        /* See the note in hv.c:hv_fetch() --jhi */
        src = (char*)bytes_from_utf8((const U8*)src, &tmplen, &is_utf8);
        len = tmplen;
    }
    if (!hash)
        PERL_HASH(hash, src, len);
    sv = newSV_type(SVt_PV);
    /* The logic for this is inlined in S_mro_get_linear_isa_dfs(), so if it
       changes here, update it there too.  */
    SvPV_set(sv, sharepvn(src, is_utf8?-len:len, hash));
    SvCUR_set(sv, len);
    SvLEN_set(sv, 0);
    SvIsCOW_on(sv);
    SvPOK_on(sv);
    if (is_utf8)
        SvUTF8_on(sv);
    if (src != orig_src)
        Safefree(src);
    return sv;
}

/*
=for apidoc newSVpv_share

Like C<newSVpvn_share>, but takes a C<NUL>-terminated string instead of a
string/length pair.

=cut
*/

SV *
Perl_newSVpv_share(pTHX_ const char *src, U32 hash)
{
    return newSVpvn_share(src, strlen(src), hash);
}

#if defined(MULTIPLICITY)

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

SV *
Perl_newSVpvf_nocontext(const char *const pat, ...)
{
    dTHX;
    SV *sv;
    va_list args;

    PERL_ARGS_ASSERT_NEWSVPVF_NOCONTEXT;

    va_start(args, pat);
    sv = vnewSVpvf(pat, &args);
    va_end(args);
    return sv;
}
#endif

/*
=for apidoc newSVpvf

Creates a new SV and initializes it with the string formatted like
C<sv_catpvf>.

=for apidoc newSVpvf_nocontext
Like C<L</newSVpvf>> but does not take a thread context (C<aTHX>) parameter,
so is used in situations where the caller doesn't already have the thread
context.

=for apidoc vnewSVpvf
Like C<L</newSVpvf>> but the arguments are an encapsulated argument list.

=cut
*/

SV *
Perl_newSVpvf(pTHX_ const char *const pat, ...)
{
    SV *sv;
    va_list args;

    PERL_ARGS_ASSERT_NEWSVPVF;

    va_start(args, pat);
    sv = vnewSVpvf(pat, &args);
    va_end(args);
    return sv;
}

/* backend for newSVpvf() and newSVpvf_nocontext() */

SV *
Perl_vnewSVpvf(pTHX_ const char *const pat, va_list *const args)
{
    SV *sv;

    PERL_ARGS_ASSERT_VNEWSVPVF;

    sv = newSV(1);
    SvPVCLEAR_FRESH(sv);
    sv_vcatpvfn_flags(sv, pat, strlen(pat), args, NULL, 0, NULL, 0);
    return sv;
}

/*
=for apidoc newSVnv

Creates a new SV and copies a floating point value into it.
The reference count for the SV is set to 1.

=cut
*/

SV *
Perl_newSVnv(pTHX_ const NV n)
{
    SV *sv = newSV_type(SVt_NV);
    (void)SvNOK_on(sv);

    SvNV_set(sv, n);
    SvTAINT(sv);

    return sv;
}

/*
=for apidoc newSViv

Creates a new SV and copies an integer into it.  The reference count for the
SV is set to 1.

=cut
*/

SV *
Perl_newSViv(pTHX_ const IV i)
{
    SV *sv = newSV_type(SVt_IV);
    (void)SvIOK_on(sv);

    SvIV_set(sv, i);
    SvTAINT(sv);

    return sv;
}

/*
=for apidoc newSVuv

Creates a new SV and copies an unsigned integer into it.
The reference count for the SV is set to 1.

=cut
*/

SV *
Perl_newSVuv(pTHX_ const UV u)
{
    SV *sv;

    /* Inlining ONLY the small relevant subset of sv_setuv here
     * for performance. Makes a significant difference. */

    /* Using ivs is more efficient than using uvs - see sv_setuv */
    if (u <= (UV)IV_MAX) {
        return newSViv((IV)u);
    }

    new_SV(sv);

    /* We're starting from SVt_FIRST, so provided that's
     * actual 0, we don't have to unset any SV type flags
     * to promote to SVt_IV. */
    STATIC_ASSERT_STMT(SVt_FIRST == 0);

    SET_SVANY_FOR_BODYLESS_IV(sv);
    SvFLAGS(sv) |= SVt_IV;
    (void)SvIOK_on(sv);
    (void)SvIsUV_on(sv);

    SvUV_set(sv, u);
    SvTAINT(sv);

    return sv;
}

/*
=for apidoc newSVbool

Creates a new SV boolean.

=cut
*/

SV *
Perl_newSVbool(pTHX_ bool bool_val)
{
    PERL_ARGS_ASSERT_NEWSVBOOL;
    SV *sv = newSVsv(bool_val ? &PL_sv_yes : &PL_sv_no);

    return sv;
}

/*
=for apidoc newSV_true

Creates a new SV that is a boolean true.

=cut
*/
SV *
Perl_newSV_true(pTHX)
{
    PERL_ARGS_ASSERT_NEWSV_TRUE;
    SV *sv = newSVsv(&PL_sv_yes);

    return sv;
}

/*
=for apidoc newSV_false

Creates a new SV that is a boolean false.

=cut
*/

SV *
Perl_newSV_false(pTHX)
{
    PERL_ARGS_ASSERT_NEWSV_FALSE;
    SV *sv = newSVsv(&PL_sv_no);

    return sv;
}

/* newRV_inc is the official function name to use now.
 * newRV_inc is in fact #defined to newRV in sv.h
 */

SV *
Perl_newRV(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_NEWRV;

    return newRV_noinc(SvREFCNT_inc_simple_NN(sv));
}

/*
=for apidoc newSVsv
=for apidoc_item newSVsv_flags
=for apidoc_item newSVsv_nomg

These create a new SV which is an exact duplicate of the original SV
(using C<sv_setsv>.)

They differ only in that C<newSVsv> performs 'get' magic; C<newSVsv_nomg> skips
any magic; and C<newSVsv_flags> allows you to explicitly set a C<flags>
parameter.

=cut
*/

SV *
Perl_newSVsv_flags(pTHX_ SV *const old, I32 flags)
{
    SV *sv;

    if (!old)
        return NULL;
    if (SvIS_FREED(old)) {
        Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL), "semi-panic: attempt to dup freed string");
        return NULL;
    }
    /* Do this here, otherwise we leak the new SV if this croaks. */
    if (flags & SV_GMAGIC)
        SvGETMAGIC(old);
    new_SV(sv);
    sv_setsv_flags(sv, old, flags & ~SV_GMAGIC);
    return sv;
}

/*
=for apidoc sv_reset

Underlying implementation for the C<reset> Perl function.
Note that the perl-level function is vaguely deprecated.

=cut
*/

void
Perl_sv_reset(pTHX_ const char *s, HV *const stash)
{
    PERL_ARGS_ASSERT_SV_RESET;

    sv_resetpvn(*s ? s : NULL, strlen(s), stash);
}

void
Perl_sv_resetpvn(pTHX_ const char *s, STRLEN len, HV * const stash)
{
    char todo[PERL_UCHAR_MAX+1];
    const char *send;

    if (!stash || SvTYPE(stash) != SVt_PVHV)
        return;

    if (!s) {		/* reset ?? searches */
        MAGIC * const mg = mg_find((const SV *)stash, PERL_MAGIC_symtab);
        if (mg && mg->mg_len) {
            const U32 count = mg->mg_len / sizeof(PMOP**);
            PMOP **pmp = (PMOP**) mg->mg_ptr;
            PMOP *const *const end = pmp + count;

            while (pmp < end) {
#ifdef USE_ITHREADS
                SvREADONLY_off(PL_regex_pad[(*pmp)->op_pmoffset]);
#else
                (*pmp)->op_pmflags &= ~PMf_USED;
#endif
                ++pmp;
            }
        }
        return;
    }

    /* reset variables */

    if (!HvTOTALKEYS(stash))
        return;

    Zero(todo, 256, char);
    send = s + len;
    while (s < send) {
        I32 max;
        I32 i = (unsigned char)*s;
        if (s[1] == '-') {
            s += 2;
        }
        max = (unsigned char)*s++;
        for ( ; i <= max; i++) {
            todo[i] = 1;
        }
        for (i = 0; i <= (I32) HvMAX(stash); i++) {
            HE *entry;
            for (entry = HvARRAY(stash)[i];
                 entry;
                 entry = HeNEXT(entry))
            {
                GV *gv;
                SV *sv;

                if (!todo[(U8)*HeKEY(entry)])
                    continue;
                gv = MUTABLE_GV(HeVAL(entry));
                if (!isGV(gv))
                    continue;
                sv = GvSV(gv);
                if (sv && !SvREADONLY(sv)) {
                    SV_CHECK_THINKFIRST_COW_DROP(sv);
                    if (!isGV(sv)) SvOK_off(sv);
                }
                if (GvAV(gv)) {
                    av_clear(GvAV(gv));
                }
                if (GvHV(gv) && !HvHasNAME(GvHV(gv))) {
                    hv_clear(GvHV(gv));
                }
            }
        }
    }
}

/*
=for apidoc sv_2io

Using various gambits, try to get an IO from an SV: the IO slot if its a
GV; or the recursive result if we're an RV; or the IO slot of the symbol
named after the PV if we're a string.

'Get' magic is ignored on the C<sv> passed in, but will be called on
C<SvRV(sv)> if C<sv> is an RV.

=cut
*/

IO*
Perl_sv_2io(pTHX_ SV *const sv)
{
    IO* io;
    GV* gv;

    PERL_ARGS_ASSERT_SV_2IO;

    switch (SvTYPE(sv)) {
    case SVt_PVIO:
        io = MUTABLE_IO(sv);
        break;
    case SVt_PVGV:
    case SVt_PVLV:
        if (isGV_with_GP(sv)) {
            gv = MUTABLE_GV(sv);
            io = GvIO(gv);
            if (!io)
                Perl_croak(aTHX_ "Bad filehandle: %" HEKf,
                                    HEKfARG(GvNAME_HEK(gv)));
            break;
        }
        /* FALLTHROUGH */
    default:
        if (!SvOK(sv))
            Perl_croak(aTHX_ PL_no_usym, "filehandle");
        if (SvROK(sv)) {
            SvGETMAGIC(SvRV(sv));
            return sv_2io(SvRV(sv));
        }
        gv = gv_fetchsv_nomg(sv, 0, SVt_PVIO);
        if (gv)
            io = GvIO(gv);
        else
            io = 0;
        if (!io) {
            SV *newsv = sv;
            if (SvGMAGICAL(sv)) {
                newsv = sv_newmortal();
                sv_setsv_nomg(newsv, sv);
            }
            Perl_croak(aTHX_ "Bad filehandle: %" SVf, SVfARG(newsv));
        }
        break;
    }
    return io;
}

/*
=for apidoc sv_2cv

Using various gambits, try to get a CV from an SV; in addition, try if
possible to set C<*st> and C<*gvp> to the stash and GV associated with it.
The flags in C<lref> are passed to C<gv_fetchsv>.

=cut
*/

CV *
Perl_sv_2cv(pTHX_ SV *sv, HV **const st, GV **const gvp, const I32 lref)
{
    GV *gv = NULL;
    CV *cv = NULL;

    PERL_ARGS_ASSERT_SV_2CV;

    if (!sv) {
        *st = NULL;
        *gvp = NULL;
        return NULL;
    }
    switch (SvTYPE(sv)) {
    case SVt_PVCV:
        *st = CvSTASH(sv);
        *gvp = NULL;
        return MUTABLE_CV(sv);
    case SVt_PVHV:
    case SVt_PVAV:
        *st = NULL;
        *gvp = NULL;
        return NULL;
    default:
        SvGETMAGIC(sv);
        if (SvROK(sv)) {
            if (SvAMAGIC(sv))
                sv = amagic_deref_call(sv, to_cv_amg);

            sv = SvRV(sv);
            if (SvTYPE(sv) == SVt_PVCV) {
                cv = MUTABLE_CV(sv);
                *gvp = NULL;
                *st = CvSTASH(cv);
                return cv;
            }
            else if(SvGETMAGIC(sv), isGV_with_GP(sv))
                gv = MUTABLE_GV(sv);
            else
                Perl_croak(aTHX_ "Not a subroutine reference");
        }
        else if (isGV_with_GP(sv)) {
            gv = MUTABLE_GV(sv);
        }
        else {
            gv = gv_fetchsv_nomg(sv, lref, SVt_PVCV);
        }
        *gvp = gv;
        if (!gv) {
            *st = NULL;
            return NULL;
        }
        /* Some flags to gv_fetchsv mean don't really create the GV  */
        if (!isGV_with_GP(gv)) {
            *st = NULL;
            return NULL;
        }
        *st = GvESTASH(gv);
        if (lref & ~GV_ADDMG && !GvCVu(gv)) {
            /* XXX this is probably not what they think they're getting.
             * It has the same effect as "sub name;", i.e. just a forward
             * declaration! */
            newSTUB(gv,0);
        }
        return GvCVu(gv);
    }
}

/*
=for apidoc sv_true

Returns true if the SV has a true value by Perl's rules.
Use the C<SvTRUE> macro instead, which may call C<sv_true()> or may
instead use an in-line version.

=cut
*/

I32
Perl_sv_true(pTHX_ SV *const sv)
{
    if (!sv)
        return 0;
    if (SvPOK(sv)) {
        const XPV* const tXpv = (XPV*)SvANY(sv);
        if (tXpv &&
                (tXpv->xpv_cur > 1 ||
                (tXpv->xpv_cur && *sv->sv_u.svu_pv != '0')))
            return 1;
        else
            return 0;
    }
    else {
        if (SvIOK(sv))
            return SvIVX(sv) != 0;
        else {
            if (SvNOK(sv))
                return SvNVX(sv) != 0.0;
            else
                return sv_2bool(sv);
        }
    }
}

/*
=for apidoc sv_pvn_force

Get a sensible string out of the SV somehow.
A private implementation of the C<SvPV_force> macro for compilers which
can't cope with complex macro expressions.  Always use the macro instead.

=for apidoc sv_pvn_force_flags

Get a sensible string out of the SV somehow.
If C<flags> has the C<SV_GMAGIC> bit set, will C<L</mg_get>> on C<sv> if
appropriate, else not.  C<sv_pvn_force> and C<sv_pvn_force_nomg> are
implemented in terms of this function.
You normally want to use the various wrapper macros instead: see
C<L</SvPV_force>> and C<L</SvPV_force_nomg>>.

=cut
*/

char *
Perl_sv_pvn_force_flags(pTHX_ SV *const sv, STRLEN *const lp, const U32 flags)
{
    PERL_ARGS_ASSERT_SV_PVN_FORCE_FLAGS;

    if (flags & SV_GMAGIC) SvGETMAGIC(sv);
    if (SvTHINKFIRST(sv) && (!SvROK(sv) || SvREADONLY(sv)))
        sv_force_normal_flags(sv, 0);

    if (SvPOK(sv)) {
        if (lp)
            *lp = SvCUR(sv);
    }
    else {
        char *s;
        STRLEN len;

        if (SvTYPE(sv) > SVt_PVLV
            || isGV_with_GP(sv))
            /* diag_listed_as: Can't coerce %s to %s in %s */
            Perl_croak(aTHX_ "Can't coerce %s to string in %s", sv_reftype(sv,0),
                OP_DESC(PL_op));
        s = sv_2pv_flags(sv, &len, flags &~ SV_GMAGIC);
        if (!s) {
          s = (char *)"";
        }
        if (lp)
            *lp = len;

        if (SvTYPE(sv) < SVt_PV ||
            s != SvPVX_const(sv)) {	/* Almost, but not quite, sv_setpvn() */
            if (SvROK(sv))
                sv_unref(sv);
            SvUPGRADE(sv, SVt_PV);		/* Never FALSE */
            SvGROW(sv, len + 1);
            Move(s,SvPVX(sv),len,char);
            SvCUR_set(sv, len);
            SvPVX(sv)[len] = '\0';
        }
        if (!SvPOK(sv)) {
            SvPOK_on(sv);		/* validate pointer */
            SvTAINT(sv);
            DEBUG_c(PerlIO_printf(Perl_debug_log, "0x%" UVxf " 2pv(%s)\n",
                                  PTR2UV(sv),SvPVX_const(sv)));
        }
    }
    (void)SvPOK_only_UTF8(sv);
    return SvPVX_mutable(sv);
}

/*
=for apidoc sv_pvbyten_force

The backend for the C<SvPVbytex_force> macro.  Always use the macro
instead.  If the SV cannot be downgraded from UTF-8, this croaks.

=cut
*/

char *
Perl_sv_pvbyten_force(pTHX_ SV *const sv, STRLEN *const lp)
{
    PERL_ARGS_ASSERT_SV_PVBYTEN_FORCE;

    sv_pvn_force(sv,lp);
    (void)sv_utf8_downgrade(sv,0);
    *lp = SvCUR(sv);
    return SvPVX(sv);
}

/*
=for apidoc sv_pvutf8n_force

The backend for the C<SvPVutf8x_force> macro.  Always use the macro
instead.

=cut
*/

char *
Perl_sv_pvutf8n_force(pTHX_ SV *const sv, STRLEN *const lp)
{
    PERL_ARGS_ASSERT_SV_PVUTF8N_FORCE;

    sv_pvn_force(sv,0);
    sv_utf8_upgrade_nomg(sv);
    *lp = SvCUR(sv);
    return SvPVX(sv);
}

/*
=for apidoc sv_reftype

Returns a string describing what the SV is a reference to.

If ob is true and the SV is blessed, the string is the class name,
otherwise it is the type of the SV, "SCALAR", "ARRAY" etc.

=cut
*/

const char *
Perl_sv_reftype(pTHX_ const SV *const sv, const int ob)
{
    PERL_ARGS_ASSERT_SV_REFTYPE;
    if (ob && SvOBJECT(sv)) {
        return SvPV_nolen_const(sv_ref(NULL, sv, ob));
    }
    else {
        /* WARNING - There is code, for instance in mg.c, that assumes that
         * the only reason that sv_reftype(sv,0) would return a string starting
         * with 'L' or 'S' is that it is a LVALUE or a SCALAR.
         * Yes this a dodgy way to do type checking, but it saves practically reimplementing
         * this routine inside other subs, and it saves time.
         * Do not change this assumption without searching for "dodgy type check" in
         * the code.
         * - Yves */
        switch (SvTYPE(sv)) {
        case SVt_NULL:
        case SVt_IV:
        case SVt_NV:
        case SVt_PV:
        case SVt_PVIV:
        case SVt_PVNV:
        case SVt_PVMG:
                                if (SvVOK(sv))
                                    return "VSTRING";
                                if (SvROK(sv))
                                    return "REF";
                                else
                                    return "SCALAR";

        case SVt_PVLV:		return (char *)  (SvROK(sv) ? "REF"
                                /* tied lvalues should appear to be
                                 * scalars for backwards compatibility */
                                : (isALPHA_FOLD_EQ(LvTYPE(sv), 't'))
                                    ? "SCALAR" : "LVALUE");
        case SVt_PVAV:		return "ARRAY";
        case SVt_PVHV:		return "HASH";
        case SVt_PVCV:		return "CODE";
        case SVt_PVGV:		return (char *) (isGV_with_GP(sv)
                                    ? "GLOB" : "SCALAR");
        case SVt_PVFM:		return "FORMAT";
        case SVt_PVIO:		return "IO";
        case SVt_INVLIST:	return "INVLIST";
        case SVt_REGEXP:	return "REGEXP";
        case SVt_PVOBJ:         return "OBJECT";
        default:		return "UNKNOWN";
        }
    }
}

/*
=for apidoc sv_ref

Returns a SV describing what the SV passed in is a reference to.

dst can be a SV to be set to the description or NULL, in which case a
mortal SV is returned.

If ob is true and the SV is blessed, the description is the class
name, otherwise it is the type of the SV, "SCALAR", "ARRAY" etc.

=cut
*/

SV *
Perl_sv_ref(pTHX_ SV *dst, const SV *const sv, const int ob)
{
    PERL_ARGS_ASSERT_SV_REF;

    if (!dst)
        dst = sv_newmortal();

    if (ob && SvOBJECT(sv)) {
        if (HvHasNAME(SvSTASH(sv)))
            sv_sethek(dst, HvNAME_HEK(SvSTASH(sv)));
        else
            sv_setpvs(dst, "__ANON__");
    }
    else {
        const char * reftype = sv_reftype(sv, 0);
        sv_setpv(dst, reftype);
    }
    return dst;
}

/*
=for apidoc sv_isobject

Returns a boolean indicating whether the SV is an RV pointing to a blessed
object.  If the SV is not an RV, or if the object is not blessed, then this
will return false.

=cut
*/

int
Perl_sv_isobject(pTHX_ SV *sv)
{
    if (!sv)
        return 0;
    SvGETMAGIC(sv);
    if (!SvROK(sv))
        return 0;
    sv = SvRV(sv);
    if (!SvOBJECT(sv))
        return 0;
    return 1;
}

/*
=for apidoc sv_isa

Returns a boolean indicating whether the SV is blessed into the specified
class.

This does not check for subtypes or method overloading. Use C<sv_isa_sv> to
verify an inheritance relationship in the same way as the C<isa> operator by
respecting any C<isa()> method overloading; or C<sv_derived_from_sv> to test
directly on the actual object type.

=cut
*/

int
Perl_sv_isa(pTHX_ SV *sv, const char *const name)
{
    const char *hvname;

    PERL_ARGS_ASSERT_SV_ISA;

    if (!sv)
        return 0;
    SvGETMAGIC(sv);
    if (!SvROK(sv))
        return 0;
    sv = SvRV(sv);
    if (!SvOBJECT(sv))
        return 0;
    hvname = HvNAME_get(SvSTASH(sv));
    if (!hvname)
        return 0;

    return strEQ(hvname, name);
}

/*
=for apidoc newSVrv

Creates a new SV for the existing RV, C<rv>, to point to.  If C<rv> is not an
RV then it will be upgraded to one.  If C<classname> is non-null then the new
SV will be blessed in the specified package.  The new SV is returned and its
reference count is 1.  The reference count 1 is owned by C<rv>. See also
newRV_inc() and newRV_noinc() for creating a new RV properly.

=cut
*/

SV*
Perl_newSVrv(pTHX_ SV *const rv, const char *const classname)
{
    SV *sv;

    PERL_ARGS_ASSERT_NEWSVRV;

    new_SV(sv);

    SV_CHECK_THINKFIRST_COW_DROP(rv);

    if (UNLIKELY( SvTYPE(rv) >= SVt_PVMG )) {
        const U32 refcnt = SvREFCNT(rv);
        SvREFCNT(rv) = 0;
        sv_clear(rv);
        SvFLAGS(rv) = 0;
        SvREFCNT(rv) = refcnt;

        sv_upgrade(rv, SVt_IV);
    } else if (SvROK(rv)) {
        SvREFCNT_dec(SvRV(rv));
    } else {
        prepare_SV_for_RV(rv);
    }

    SvOK_off(rv);
    SvRV_set(rv, sv);
    SvROK_on(rv);

    if (classname) {
        HV* const stash = gv_stashpv(classname, GV_ADD);
        (void)sv_bless(rv, stash);
    }
    return sv;
}

SV *
Perl_newSVavdefelem(pTHX_ AV *av, SSize_t ix, bool extendible)
{
    SV * const lv = newSV_type(SVt_PVLV);
    PERL_ARGS_ASSERT_NEWSVAVDEFELEM;
    LvTYPE(lv) = 'y';
    sv_magic(lv, NULL, PERL_MAGIC_defelem, NULL, 0);
    LvTARG(lv) = SvREFCNT_inc_simple_NN(av);
    LvSTARGOFF(lv) = ix;
    LvTARGLEN(lv) = extendible ? 1 : (STRLEN)UV_MAX;
    return lv;
}

/*
=for apidoc sv_setref_pv

Copies a pointer into a new SV, optionally blessing the SV.  The C<rv>
argument will be upgraded to an RV.  That RV will be modified to point to
the new SV.  If the C<pv> argument is C<NULL>, then C<PL_sv_undef> will be placed
into the SV.  The C<classname> argument indicates the package for the
blessing.  Set C<classname> to C<NULL> to avoid the blessing.  The new SV
will have a reference count of 1, and the RV will be returned.

Do not use with other Perl types such as HV, AV, SV, CV, because those
objects will become corrupted by the pointer copy process.

Note that C<sv_setref_pvn> copies the string while this copies the pointer.

=cut
*/

SV*
Perl_sv_setref_pv(pTHX_ SV *const rv, const char *const classname, void *const pv)
{
    PERL_ARGS_ASSERT_SV_SETREF_PV;

    if (!pv) {
        sv_set_undef(rv);
        SvSETMAGIC(rv);
    }
    else
        sv_setiv(newSVrv(rv,classname), PTR2IV(pv));
    return rv;
}

/*
=for apidoc sv_setref_iv

Copies an integer into a new SV, optionally blessing the SV.  The C<rv>
argument will be upgraded to an RV.  That RV will be modified to point to
the new SV.  The C<classname> argument indicates the package for the
blessing.  Set C<classname> to C<NULL> to avoid the blessing.  The new SV
will have a reference count of 1, and the RV will be returned.

=cut
*/

SV*
Perl_sv_setref_iv(pTHX_ SV *const rv, const char *const classname, const IV iv)
{
    PERL_ARGS_ASSERT_SV_SETREF_IV;

    sv_setiv(newSVrv(rv,classname), iv);
    return rv;
}

/*
=for apidoc sv_setref_uv

Copies an unsigned integer into a new SV, optionally blessing the SV.  The C<rv>
argument will be upgraded to an RV.  That RV will be modified to point to
the new SV.  The C<classname> argument indicates the package for the
blessing.  Set C<classname> to C<NULL> to avoid the blessing.  The new SV
will have a reference count of 1, and the RV will be returned.

=cut
*/

SV*
Perl_sv_setref_uv(pTHX_ SV *const rv, const char *const classname, const UV uv)
{
    PERL_ARGS_ASSERT_SV_SETREF_UV;

    sv_setuv(newSVrv(rv,classname), uv);
    return rv;
}

/*
=for apidoc sv_setref_nv

Copies a double into a new SV, optionally blessing the SV.  The C<rv>
argument will be upgraded to an RV.  That RV will be modified to point to
the new SV.  The C<classname> argument indicates the package for the
blessing.  Set C<classname> to C<NULL> to avoid the blessing.  The new SV
will have a reference count of 1, and the RV will be returned.

=cut
*/

SV*
Perl_sv_setref_nv(pTHX_ SV *const rv, const char *const classname, const NV nv)
{
    PERL_ARGS_ASSERT_SV_SETREF_NV;

    sv_setnv(newSVrv(rv,classname), nv);
    return rv;
}

/*
=for apidoc sv_setref_pvn

Copies a string into a new SV, optionally blessing the SV.  The length of the
string must be specified with C<n>.  The C<rv> argument will be upgraded to
an RV.  That RV will be modified to point to the new SV.  The C<classname>
argument indicates the package for the blessing.  Set C<classname> to
C<NULL> to avoid the blessing.  The new SV will have a reference count
of 1, and the RV will be returned.

Note that C<sv_setref_pv> copies the pointer while this copies the string.

=cut
*/

SV*
Perl_sv_setref_pvn(pTHX_ SV *const rv, const char *const classname,
                   const char *const pv, const STRLEN n)
{
    PERL_ARGS_ASSERT_SV_SETREF_PVN;

    sv_setpvn(newSVrv(rv,classname), pv, n);
    return rv;
}

/*
=for apidoc sv_bless

Blesses an SV into a specified package.  The SV must be an RV.  The package
must be designated by its stash (see C<L</gv_stashpv>>).  The reference count
of the SV is unaffected.

=cut
*/

SV*
Perl_sv_bless(pTHX_ SV *const sv, HV *const stash)
{
    SV *tmpRef;
    HV *oldstash = NULL;

    PERL_ARGS_ASSERT_SV_BLESS;

    SvGETMAGIC(sv);
    if (!SvROK(sv))
        Perl_croak(aTHX_ "Can't bless non-reference value");
    if (HvSTASH_IS_CLASS(stash))
        Perl_croak(aTHX_ "Attempt to bless into a class");

    tmpRef = SvRV(sv);
    if (SvFLAGS(tmpRef) & (SVs_OBJECT|SVf_READONLY|SVf_PROTECT)) {
        if (SvREADONLY(tmpRef))
            Perl_croak_no_modify();
        if (SvTYPE(tmpRef) == SVt_PVOBJ)
            Perl_croak(aTHX_ "Can't bless an object reference");
        if (SvOBJECT(tmpRef)) {
            oldstash = SvSTASH(tmpRef);
        }
    }
    SvOBJECT_on(tmpRef);
    SvUPGRADE(tmpRef, SVt_PVMG);
    SvSTASH_set(tmpRef, MUTABLE_HV(SvREFCNT_inc_simple(stash)));
    SvREFCNT_dec(oldstash);

    if(SvSMAGICAL(tmpRef))
        if(mg_find(tmpRef, PERL_MAGIC_ext) || mg_find(tmpRef, PERL_MAGIC_uvar))
            mg_set(tmpRef);



    return sv;
}

/* Downgrades a PVGV to a PVMG. If it's actually a PVLV, we leave the type
 * as it is after unglobbing it.
 */

PERL_STATIC_INLINE void
S_sv_unglob(pTHX_ SV *const sv, U32 flags)
{
    void *xpvmg;
    HV *stash;
    SV * const temp = flags & SV_COW_DROP_PV ? NULL : sv_newmortal();

    PERL_ARGS_ASSERT_SV_UNGLOB;

    assert(SvTYPE(sv) == SVt_PVGV || SvTYPE(sv) == SVt_PVLV);
    SvFAKE_off(sv);
    if (!(flags & SV_COW_DROP_PV))
        gv_efullname3(temp, MUTABLE_GV(sv), "*");

    SvREFCNT_inc_simple_void_NN(sv_2mortal(sv));
    if (GvGP(sv)) {
        if(GvCVu((const GV *)sv) && (stash = GvSTASH(MUTABLE_GV(sv)))
           && HvHasNAME(stash))
            mro_method_changed_in(stash);
        gp_free(MUTABLE_GV(sv));
    }
    if (GvSTASH(sv)) {
        sv_del_backref(MUTABLE_SV(GvSTASH(sv)), sv);
        GvSTASH(sv) = NULL;
    }
    GvMULTI_off(sv);
    if (GvNAME_HEK(sv)) {
        unshare_hek(GvNAME_HEK(sv));
    }
    isGV_with_GP_off(sv);

    if(SvTYPE(sv) == SVt_PVGV) {
        /* need to keep SvANY(sv) in the right arena */
        xpvmg = new_XPVMG();
        StructCopy(SvANY(sv), xpvmg, XPVMG);
        del_body_by_type(SvANY(sv), SVt_PVGV);
        SvANY(sv) = xpvmg;

        SvFLAGS(sv) &= ~SVTYPEMASK;
        SvFLAGS(sv) |= SVt_PVMG;
    }

    /* Intentionally not calling any local SET magic, as this isn't so much a
       set operation as merely an internal storage change.  */
    if (flags & SV_COW_DROP_PV) SvOK_off(sv);
    else sv_setsv_flags(sv, temp, 0);

    if ((const GV *)sv == PL_last_in_gv)
        PL_last_in_gv = NULL;
    else if ((const GV *)sv == PL_statgv)
        PL_statgv = NULL;
}

/*
=for apidoc sv_unref_flags

Unsets the RV status of the SV, and decrements the reference count of
whatever was being referenced by the RV.  This can almost be thought of
as a reversal of C<newSVrv>.  The C<cflags> argument can contain
C<SV_IMMEDIATE_UNREF> to force the reference count to be decremented
(otherwise the decrementing is conditional on the reference count being
different from one or the reference being a readonly SV).
See C<L</SvROK_off>>.

=for apidoc Amnh||SV_IMMEDIATE_UNREF

=cut
*/

void
Perl_sv_unref_flags(pTHX_ SV *const ref, const U32 flags)
{
    SV* const target = SvRV(ref);

    PERL_ARGS_ASSERT_SV_UNREF_FLAGS;

    if (SvWEAKREF(ref)) {
        sv_del_backref(target, ref);
        SvWEAKREF_off(ref);
        SvRV_set(ref, NULL);
        return;
    }
    SvRV_set(ref, NULL);
    SvROK_off(ref);
    /* You can't have a || SvREADONLY(target) here, as $a = $$a, where $a was
       assigned to as BEGIN {$a = \"Foo"} will fail.  */
    if (SvREFCNT(target) != 1 || (flags & SV_IMMEDIATE_UNREF))
        SvREFCNT_dec_NN(target);
    else /* XXX Hack, but hard to make $a=$a->[1] work otherwise */
        sv_2mortal(target);	/* Schedule for freeing later */
}

/*
=for apidoc sv_untaint

Untaint an SV.  Use C<SvTAINTED_off> instead.

=cut
*/

void
Perl_sv_untaint(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_UNTAINT;
    PERL_UNUSED_CONTEXT;

    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
        MAGIC * const mg = mg_find(sv, PERL_MAGIC_taint);
        if (mg)
            mg->mg_len &= ~1;
    }
}

/*
=for apidoc sv_tainted

Test an SV for taintedness.  Use C<SvTAINTED> instead.

=cut
*/

bool
Perl_sv_tainted(pTHX_ SV *const sv)
{
    PERL_ARGS_ASSERT_SV_TAINTED;
    PERL_UNUSED_CONTEXT;

    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
        const MAGIC * const mg = mg_find(sv, PERL_MAGIC_taint);
        if (mg && (mg->mg_len & 1) )
            return TRUE;
    }
    return FALSE;
}

#if defined(MULTIPLICITY)

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

void
Perl_sv_setpvf_nocontext(SV *const sv, const char *const pat, ...)
{
    dTHX;
    va_list args;

    PERL_ARGS_ASSERT_SV_SETPVF_NOCONTEXT;

    va_start(args, pat);
    sv_vsetpvf(sv, pat, &args);
    va_end(args);
}

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

void
Perl_sv_setpvf_mg_nocontext(SV *const sv, const char *const pat, ...)
{
    dTHX;
    va_list args;

    PERL_ARGS_ASSERT_SV_SETPVF_MG_NOCONTEXT;

    va_start(args, pat);
    sv_vsetpvf_mg(sv, pat, &args);
    va_end(args);
}
#endif

/*
=for apidoc      sv_setpvf
=for apidoc_item sv_setpvf_mg
=for apidoc_item sv_setpvf_mg_nocontext
=for apidoc_item sv_setpvf_nocontext

These work like C<L</sv_catpvf>> but copy the text into the SV instead of
appending it.

The differences between these are:

C<sv_setpvf_mg> and C<sv_setpvf_mg_nocontext> perform 'set' magic; C<sv_setpvf>
and C<sv_setpvf_nocontext> skip all magic.

C<sv_setpvf_nocontext> and C<sv_setpvf_mg_nocontext> do not take a thread
context (C<aTHX>) parameter, so are used in situations where the caller
doesn't already have the thread context.

=cut
*/

void
Perl_sv_setpvf(pTHX_ SV *const sv, const char *const pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_SV_SETPVF;

    va_start(args, pat);
    sv_vsetpvf(sv, pat, &args);
    va_end(args);
}

/*
=for apidoc sv_vsetpvf
=for apidoc_item sv_vsetpvf_mg

These work like C<L</sv_vcatpvf>> but copy the text into the SV instead of
appending it.

They differ only in that C<sv_vsetpvf_mg> performs 'set' magic;
C<sv_vsetpvf> skips all magic.

They are usually used via their frontends, C<L</sv_setpvf>> and
C<L</sv_setpvf_mg>>.

=cut
*/

void
Perl_sv_vsetpvf(pTHX_ SV *const sv, const char *const pat, va_list *const args)
{
    PERL_ARGS_ASSERT_SV_VSETPVF;

    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
}

void
Perl_sv_setpvf_mg(pTHX_ SV *const sv, const char *const pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_SV_SETPVF_MG;

    va_start(args, pat);
    sv_vsetpvf_mg(sv, pat, &args);
    va_end(args);
}

void
Perl_sv_vsetpvf_mg(pTHX_ SV *const sv, const char *const pat, va_list *const args)
{
    PERL_ARGS_ASSERT_SV_VSETPVF_MG;

    sv_vsetpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    SvSETMAGIC(sv);
}

#if defined(MULTIPLICITY)

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

void
Perl_sv_catpvf_nocontext(SV *const sv, const char *const pat, ...)
{
    dTHX;
    va_list args;

    PERL_ARGS_ASSERT_SV_CATPVF_NOCONTEXT;

    va_start(args, pat);
    sv_vcatpvfn_flags(sv, pat, strlen(pat), &args, NULL, 0, NULL, SV_GMAGIC|SV_SMAGIC);
    va_end(args);
}

/* pTHX_ magic can't cope with varargs, so this is a no-context
 * version of the main function, (which may itself be aliased to us).
 * Don't access this version directly.
 */

void
Perl_sv_catpvf_mg_nocontext(SV *const sv, const char *const pat, ...)
{
    dTHX;
    va_list args;

    PERL_ARGS_ASSERT_SV_CATPVF_MG_NOCONTEXT;

    va_start(args, pat);
    sv_vcatpvfn_flags(sv, pat, strlen(pat), &args, NULL, 0, NULL, SV_GMAGIC|SV_SMAGIC);
    SvSETMAGIC(sv);
    va_end(args);
}
#endif

/*
=for apidoc sv_catpvf
=for apidoc_item sv_catpvf_mg
=for apidoc_item sv_catpvf_mg_nocontext
=for apidoc_item sv_catpvf_nocontext

These process their arguments like C<sprintf>, and append the formatted
output to an SV.  As with C<sv_vcatpvfn>, argument reordering is not supporte
when called with a non-null C-style variable argument list.

If the appended data contains "wide" characters
(including, but not limited to, SVs with a UTF-8 PV formatted with C<%s>,
and characters >255 formatted with C<%c>), the original SV might get
upgraded to UTF-8.

If the original SV was UTF-8, the pattern should be
valid UTF-8; if the original SV was bytes, the pattern should be too.

All perform 'get' magic, but only C<sv_catpvf_mg> and C<sv_catpvf_mg_nocontext>
perform 'set' magic.

C<sv_catpvf_nocontext> and C<sv_catpvf_mg_nocontext> do not take a thread
context (C<aTHX>) parameter, so are used in situations where the caller
doesn't already have the thread context.

=cut
*/

void
Perl_sv_catpvf(pTHX_ SV *const sv, const char *const pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_SV_CATPVF;

    va_start(args, pat);
    sv_vcatpvfn_flags(sv, pat, strlen(pat), &args, NULL, 0, NULL, SV_GMAGIC|SV_SMAGIC);
    va_end(args);
}

/*
=for apidoc sv_vcatpvf
=for apidoc_item sv_vcatpvf_mg

These process their arguments like C<sv_vcatpvfn> called with a non-null
C-style variable argument list, and append the formatted output to C<sv>.

They differ only in that C<sv_vcatpvf_mg> performs 'set' magic;
C<sv_vcatpvf> skips 'set' magic.

Both perform 'get' magic.

They are usually accessed via their frontends C<L</sv_catpvf>> and
C<L</sv_catpvf_mg>>.

=cut
*/

void
Perl_sv_vcatpvf(pTHX_ SV *const sv, const char *const pat, va_list *const args)
{
    PERL_ARGS_ASSERT_SV_VCATPVF;

    sv_vcatpvfn_flags(sv, pat, strlen(pat), args, NULL, 0, NULL, SV_GMAGIC|SV_SMAGIC);
}

void
Perl_sv_catpvf_mg(pTHX_ SV *const sv, const char *const pat, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_SV_CATPVF_MG;

    va_start(args, pat);
    sv_vcatpvfn_flags(sv, pat, strlen(pat), &args, NULL, 0, NULL, SV_GMAGIC|SV_SMAGIC);
    SvSETMAGIC(sv);
    va_end(args);
}

void
Perl_sv_vcatpvf_mg(pTHX_ SV *const sv, const char *const pat, va_list *const args)
{
    PERL_ARGS_ASSERT_SV_VCATPVF_MG;

    sv_vcatpvfn(sv, pat, strlen(pat), args, NULL, 0, NULL);
    SvSETMAGIC(sv);
}

/*
=for apidoc sv_vsetpvfn

Works like C<sv_vcatpvfn> but copies the text into the SV instead of
appending it.

Usually used via one of its frontends L</C<sv_vsetpvf>> and
L</C<sv_vsetpvf_mg>>.

=cut
*/

void
Perl_sv_vsetpvfn(pTHX_ SV *const sv, const char *const pat, const STRLEN patlen,
                 va_list *const args, SV **const svargs, const Size_t sv_count, bool *const maybe_tainted)
{
    PERL_ARGS_ASSERT_SV_VSETPVFN;

    SvPVCLEAR(sv);
    sv_vcatpvfn_flags(sv, pat, patlen, args, svargs, sv_count, maybe_tainted, 0);
}


/* simplified inline Perl_sv_catpvn_nomg() when you know the SV's SvPOK */

PERL_STATIC_INLINE void
S_sv_catpvn_simple(pTHX_ SV *const sv, const char* const buf, const STRLEN len)
{
    STRLEN const need = len + SvCUR(sv) + 1;
    char *end;

    /* can't wrap as both len and SvCUR() are allocated in
     * memory and together can't consume all the address space
     */
    assert(need > len);

    assert(SvPOK(sv));
    SvGROW(sv, need);
    end = SvEND(sv);
    Copy(buf, end, len, char);
    end += len;
    *end = '\0';
    SvCUR_set(sv, need - 1);
}


/*
 * Warn of missing argument to sprintf. The value used in place of such
 * arguments should be &PL_sv_no; an undefined value would yield
 * inappropriate "use of uninit" warnings [perl #71000].
 */
STATIC void
S_warn_vcatpvfn_missing_argument(pTHX) {
    if (ckWARN(WARN_MISSING)) {
        Perl_warner(aTHX_ packWARN(WARN_MISSING), "Missing argument in %s",
                PL_op ? OP_DESC(PL_op) : "sv_vcatpvfn()");
    }
}


static void
S_croak_overflow()
{
    dTHX;
    Perl_croak(aTHX_ "Integer overflow in format string for %s",
                    (PL_op ? OP_DESC(PL_op) : "sv_vcatpvfn"));
}


/* Given an int i from the next arg (if args is true) or an sv from an arg
 * (if args is false), try to extract a STRLEN-ranged value from the arg,
 * with overflow checking.
 * Sets *neg to true if the value was negative (untouched otherwise.
 * Returns the absolute value.
 * As an extra margin of safety, it croaks if the returned value would
 * exceed the maximum value of a STRLEN / 4.
 */

static STRLEN
S_sprintf_arg_num_val(pTHX_ va_list *const args, int i, SV *sv, bool *neg)
{
    IV iv;

    if (args) {
        iv = i;
        goto do_iv;
    }

    if (!sv)
        return 0;

    SvGETMAGIC(sv);

    if (UNLIKELY(SvIsUV(sv))) {
        UV uv = SvUV_nomg(sv);
        if (uv > IV_MAX)
            S_croak_overflow();
        iv = uv;
    }
    else {
        iv = SvIV_nomg(sv);
      do_iv:
        if (iv < 0) {
            if (iv < -IV_MAX)
                S_croak_overflow();
            iv = -iv;
            *neg = TRUE;
        }
    }

    if (iv > (IV)(((STRLEN)~0) / 4))
        S_croak_overflow();

    return (STRLEN)iv;
}

/* Read in and return a number. Updates *pattern to point to the char
 * following the number. Expects the first char to 1..9.
 * Croaks if the number exceeds 1/4 of the maximum value of STRLEN.
 * This is a belt-and-braces safety measure to complement any
 * overflow/wrap checks done in the main body of sv_vcatpvfn_flags.
 * It means that e.g. on a 32-bit system the width/precision can't be more
 * than 1G, which seems reasonable.
 */

STATIC STRLEN
S_expect_number(pTHX_ const char **const pattern)
{
    STRLEN var;

    PERL_ARGS_ASSERT_EXPECT_NUMBER;

    assert(inRANGE(**pattern, '1', '9'));

    var = *(*pattern)++ - '0';
    while (isDIGIT(**pattern)) {
        /* if var * 10 + 9 would exceed 1/4 max strlen, croak */
        if (var > ((((STRLEN)~0) / 4 - 9) / 10))
            S_croak_overflow();
        var = var * 10 + (*(*pattern)++ - '0');
    }
    return var;
}

/* Implement a fast "%.0f": given a pointer to the end of a buffer (caller
 * ensures it's big enough), back fill it with the rounded integer part of
 * nv. Returns ptr to start of string, and sets *len to its length.
 * Returns NULL if not convertible.
 */

STATIC char *
S_F0convert(NV nv, char *const endbuf, STRLEN *const len)
{
    const int neg = nv < 0;
    UV uv;

    PERL_ARGS_ASSERT_F0CONVERT;

    assert(!Perl_isinfnan(nv));
    if (neg)
        nv = -nv;
    if (nv != 0.0 && nv < (NV) UV_MAX) {
        char *p = endbuf;
        uv = (UV)nv;
        if (uv != nv) {
            nv += 0.5;
            uv = (UV)nv;
            if (uv & 1 && uv == nv)
                uv--;			/* Round to even */
        }
        do {
            const unsigned dig = uv % 10;
            *--p = '0' + dig;
        } while (uv /= 10);
        if (neg)
            *--p = '-';
        *len = endbuf - p;
        return p;
    }
    return NULL;
}


/* XXX maybe_tainted is never assigned to, so the doc above is lying. */

void
Perl_sv_vcatpvfn(pTHX_ SV *const sv, const char *const pat, const STRLEN patlen,
                 va_list *const args, SV **const svargs, const Size_t sv_count, bool *const maybe_tainted)
{
    PERL_ARGS_ASSERT_SV_VCATPVFN;

    sv_vcatpvfn_flags(sv, pat, patlen, args, svargs, sv_count, maybe_tainted, SV_GMAGIC|SV_SMAGIC);
}


/* For the vcatpvfn code, we need a long double target in case
 * HAS_LONG_DOUBLE, even without USE_LONG_DOUBLE, so that we can printf
 * with long double formats, even without NV being long double.  But we
 * call the target 'fv' instead of 'nv', since most of the time it is not
 * (most compilers these days recognize "long double", even if only as a
 * synonym for "double").
*/
#if defined(HAS_LONG_DOUBLE) && LONG_DOUBLESIZE > DOUBLESIZE && \
        defined(PERL_PRIgldbl) && !defined(USE_QUADMATH)
#  define VCATPVFN_FV_GF PERL_PRIgldbl
#  if defined(__VMS) && defined(__ia64) && defined(__IEEE_FLOAT)
       /* Work around breakage in OTS$CVT_FLOAT_T_X */
#    define VCATPVFN_NV_TO_FV(nv,fv)                    \
            STMT_START {                                \
                double _dv = nv;                        \
                fv = Perl_isnan(_dv) ? LDBL_QNAN : _dv; \
            } STMT_END
#  else
#    define VCATPVFN_NV_TO_FV(nv,fv) (fv)=(nv)
#  endif
   typedef long double vcatpvfn_long_double_t;
#else
#  define VCATPVFN_FV_GF NVgf
#  define VCATPVFN_NV_TO_FV(nv,fv) (fv)=(nv)
   typedef NV vcatpvfn_long_double_t;
#endif

#ifdef LONGDOUBLE_DOUBLEDOUBLE
/* The first double can be as large as 2**1023, or '1' x '0' x 1023.
 * The second double can be as small as 2**-1074, or '0' x 1073 . '1'.
 * The sum of them can be '1' . '0' x 2096 . '1', with implied radix point
 * after the first 1023 zero bits.
 *
 * XXX The 2098 is quite large (262.25 bytes) and therefore some sort
 * of dynamically growing buffer might be better, start at just 16 bytes
 * (for example) and grow only when necessary.  Or maybe just by looking
 * at the exponents of the two doubles? */
#  define DOUBLEDOUBLE_MAXBITS 2098
#endif

/* vhex will contain the values (0..15) of the hex digits ("nybbles"
 * of 4 bits); 1 for the implicit 1, and the mantissa bits, four bits
 * per xdigit.  For the double-double case, this can be rather many.
 * The non-double-double-long-double overshoots since all bits of NV
 * are not mantissa bits, there are also exponent bits. */
#ifdef LONGDOUBLE_DOUBLEDOUBLE
#  define VHEX_SIZE (3+DOUBLEDOUBLE_MAXBITS/4)
#else
#  define VHEX_SIZE (1+(NVSIZE * 8)/4)
#endif

/* If we do not have a known long double format, (including not using
 * long doubles, or long doubles being equal to doubles) then we will
 * fall back to the ldexp/frexp route, with which we can retrieve at
 * most as many bits as our widest unsigned integer type is.  We try
 * to get a 64-bit unsigned integer even if we are not using a 64-bit UV.
 *
 * (If you want to test the case of UVSIZE == 4, NVSIZE == 8,
 *  set the MANTISSATYPE to int and the MANTISSASIZE to 4.)
 */
#if defined(HAS_QUAD) && defined(Uquad_t)
#  define MANTISSATYPE Uquad_t
#  define MANTISSASIZE 8
#else
#  define MANTISSATYPE UV
#  define MANTISSASIZE UVSIZE
#endif

#if defined(DOUBLE_LITTLE_ENDIAN) || defined(LONGDOUBLE_LITTLE_ENDIAN)
#  define HEXTRACT_LITTLE_ENDIAN
#elif defined(DOUBLE_BIG_ENDIAN) || defined(LONGDOUBLE_BIG_ENDIAN)
#  define HEXTRACT_BIG_ENDIAN
#else
#  define HEXTRACT_MIX_ENDIAN
#endif

/* S_hextract() is a helper for S_format_hexfp, for extracting
 * the hexadecimal values (for %a/%A).  The nv is the NV where the value
 * are being extracted from (either directly from the long double in-memory
 * presentation, or from the uquad computed via frexp+ldexp).  frexp also
 * is used to update the exponent.  The subnormal is set to true
 * for IEEE 754 subnormals/denormals (including the x86 80-bit format).
 * The vhex is the pointer to the beginning of the output buffer of VHEX_SIZE.
 *
 * The tricky part is that S_hextract() needs to be called twice:
 * the first time with vend as NULL, and the second time with vend as
 * the pointer returned by the first call.  What happens is that on
 * the first round the output size is computed, and the intended
 * extraction sanity checked.  On the second round the actual output
 * (the extraction of the hexadecimal values) takes place.
 * Sanity failures cause fatal failures during both rounds. */
STATIC U8*
S_hextract(pTHX_ const NV nv, int* exponent, bool *subnormal,
           U8* vhex, U8* vend)
{
    U8* v = vhex;
    int ix;
    int ixmin = 0, ixmax = 0;

    /* XXX Inf/NaN are not handled here, since it is
     * assumed they are to be output as "Inf" and "NaN". */

    /* These macros are just to reduce typos, they have multiple
     * repetitions below, but usually only one (or sometimes two)
     * of them is really being used. */
    /* HEXTRACT_OUTPUT() extracts the high nybble first. */
#define HEXTRACT_OUTPUT_HI(ix) (*v++ = nvp[ix] >> 4)
#define HEXTRACT_OUTPUT_LO(ix) (*v++ = nvp[ix] & 0xF)
#define HEXTRACT_OUTPUT(ix) \
    STMT_START { \
      HEXTRACT_OUTPUT_HI(ix); HEXTRACT_OUTPUT_LO(ix); \
   } STMT_END
#define HEXTRACT_COUNT(ix, c) \
    STMT_START { \
      v += c; if (ix < ixmin) ixmin = ix; else if (ix > ixmax) ixmax = ix; \
   } STMT_END
#define HEXTRACT_BYTE(ix) \
    STMT_START { \
      if (vend) HEXTRACT_OUTPUT(ix); else HEXTRACT_COUNT(ix, 2); \
   } STMT_END
#define HEXTRACT_LO_NYBBLE(ix) \
    STMT_START { \
      if (vend) HEXTRACT_OUTPUT_LO(ix); else HEXTRACT_COUNT(ix, 1); \
   } STMT_END
    /* HEXTRACT_TOP_NYBBLE is just convenience disguise,
     * to make it look less odd when the top bits of a NV
     * are extracted using HEXTRACT_LO_NYBBLE: the highest
     * order bits can be in the "low nybble" of a byte. */
#define HEXTRACT_TOP_NYBBLE(ix) HEXTRACT_LO_NYBBLE(ix)
#define HEXTRACT_BYTES_LE(a, b) \
    for (ix = a; ix >= b; ix--) { HEXTRACT_BYTE(ix); }
#define HEXTRACT_BYTES_BE(a, b) \
    for (ix = a; ix <= b; ix++) { HEXTRACT_BYTE(ix); }
#define HEXTRACT_GET_SUBNORMAL(nv) *subnormal = Perl_fp_class_denorm(nv)
#define HEXTRACT_IMPLICIT_BIT(nv) \
    STMT_START { \
        if (!*subnormal) { \
            if (vend) *v++ = ((nv) == 0.0) ? 0 : 1; else v++; \
        } \
   } STMT_END

/* Most formats do.  Those which don't should undef this.
 *
 * But also note that IEEE 754 subnormals do not have it, or,
 * expressed alternatively, their implicit bit is zero. */
#define HEXTRACT_HAS_IMPLICIT_BIT

/* Many formats do.  Those which don't should undef this. */
#define HEXTRACT_HAS_TOP_NYBBLE

    /* HEXTRACTSIZE is the maximum number of xdigits. */
#if defined(USE_LONG_DOUBLE) && defined(LONGDOUBLE_DOUBLEDOUBLE)
#  define HEXTRACTSIZE (2+DOUBLEDOUBLE_MAXBITS/4)
#else
#  define HEXTRACTSIZE 2 * NVSIZE
#endif

    const U8* vmaxend = vhex + HEXTRACTSIZE;

    assert(HEXTRACTSIZE <= VHEX_SIZE);

    PERL_UNUSED_VAR(ix); /* might happen */
    (void)Perl_frexp(PERL_ABS(nv), exponent);
    *subnormal = FALSE;
    if (vend && (vend <= vhex || vend > vmaxend)) {
        /* diag_listed_as: Hexadecimal float: internal error (%s) */
        Perl_croak(aTHX_ "Hexadecimal float: internal error (entry)");
    }
    {
        /* First check if using long doubles. */
#if defined(USE_LONG_DOUBLE) && (NVSIZE > DOUBLESIZE)
#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_IEEE_754_128_BIT_LITTLE_ENDIAN
        /* Used in e.g. VMS and HP-UX IA-64, e.g. -0.1L:
         * 9a 99 99 99 99 99 99 99 99 99 99 99 99 99 fb bf */
        /* The bytes 13..0 are the mantissa/fraction,
         * the 15,14 are the sign+exponent. */
        const U8* nvp = (const U8*)(&nv);
        HEXTRACT_GET_SUBNORMAL(nv);
        HEXTRACT_IMPLICIT_BIT(nv);
#    undef HEXTRACT_HAS_TOP_NYBBLE
        HEXTRACT_BYTES_LE(13, 0);
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_IEEE_754_128_BIT_BIG_ENDIAN
        /* Used in e.g. Solaris Sparc and HP-UX PA-RISC, e.g. -0.1L:
         * bf fb 99 99 99 99 99 99 99 99 99 99 99 99 99 9a */
        /* The bytes 2..15 are the mantissa/fraction,
         * the 0,1 are the sign+exponent. */
        const U8* nvp = (const U8*)(&nv);
        HEXTRACT_GET_SUBNORMAL(nv);
        HEXTRACT_IMPLICIT_BIT(nv);
#    undef HEXTRACT_HAS_TOP_NYBBLE
        HEXTRACT_BYTES_BE(2, 15);
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_LITTLE_ENDIAN
        /* x86 80-bit "extended precision", 64 bits of mantissa / fraction /
         * significand, 15 bits of exponent, 1 bit of sign.  No implicit bit.
         * NVSIZE can be either 12 (ILP32, Solaris x86) or 16 (LP64, Linux
         * and OS X), meaning that 2 or 6 bytes are empty padding. */
        /* The bytes 0..1 are the sign+exponent,
         * the bytes 2..9 are the mantissa/fraction. */
        const U8* nvp = (const U8*)(&nv);
#    undef HEXTRACT_HAS_IMPLICIT_BIT
#    undef HEXTRACT_HAS_TOP_NYBBLE
        HEXTRACT_GET_SUBNORMAL(nv);
        HEXTRACT_BYTES_LE(7, 0);
#  elif LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_BIG_ENDIAN
        /* Does this format ever happen? (Wikipedia says the Motorola
         * 6888x math coprocessors used format _like_ this but padded
         * to 96 bits with 16 unused bits between the exponent and the
         * mantissa.) */
        const U8* nvp = (const U8*)(&nv);
#    undef HEXTRACT_HAS_IMPLICIT_BIT
#    undef HEXTRACT_HAS_TOP_NYBBLE
        HEXTRACT_GET_SUBNORMAL(nv);
        HEXTRACT_BYTES_BE(0, 7);
#  else
#    define HEXTRACT_FALLBACK
        /* Double-double format: two doubles next to each other.
         * The first double is the high-order one, exactly like
         * it would be for a "lone" double.  The second double
         * is shifted down using the exponent so that that there
         * are no common bits.  The tricky part is that the value
         * of the double-double is the SUM of the two doubles and
         * the second one can be also NEGATIVE.
         *
         * Because of this tricky construction the bytewise extraction we
         * use for the other long double formats doesn't work, we must
         * extract the values bit by bit.
         *
         * The little-endian double-double is used .. somewhere?
         *
         * The big endian double-double is used in e.g. PPC/Power (AIX)
         * and MIPS (SGI).
         *
         * The mantissa bits are in two separate stretches, e.g. for -0.1L:
         * 9a 99 99 99 99 99 59 bc 9a 99 99 99 99 99 b9 3f (LE)
         * 3f b9 99 99 99 99 99 9a bc 59 99 99 99 99 99 9a (BE)
         */
#  endif
#else /* #if defined(USE_LONG_DOUBLE) && (NVSIZE > DOUBLESIZE) */
        /* Using normal doubles, not long doubles.
         *
         * We generate 4-bit xdigits (nybble/nibble) instead of 8-bit
         * bytes, since we might need to handle printf precision, and
         * also need to insert the radix. */
#  if NVSIZE == 8
#    ifdef HEXTRACT_LITTLE_ENDIAN
        /* 0 1 2 3 4 5 6 7 (MSB = 7, LSB = 0, 6+7 = exponent+sign) */
        const U8* nvp = (const U8*)(&nv);
        HEXTRACT_GET_SUBNORMAL(nv);
        HEXTRACT_IMPLICIT_BIT(nv);
        HEXTRACT_TOP_NYBBLE(6);
        HEXTRACT_BYTES_LE(5, 0);
#    elif defined(HEXTRACT_BIG_ENDIAN)
        /* 7 6 5 4 3 2 1 0 (MSB = 7, LSB = 0, 6+7 = exponent+sign) */
        const U8* nvp = (const U8*)(&nv);
        HEXTRACT_GET_SUBNORMAL(nv);
        HEXTRACT_IMPLICIT_BIT(nv);
        HEXTRACT_TOP_NYBBLE(1);
        HEXTRACT_BYTES_BE(2, 7);
#    elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_MIXED_ENDIAN_LE_BE
        /* 4 5 6 7 0 1 2 3 (MSB = 7, LSB = 0, 6:7 = nybble:exponent:sign) */
        const U8* nvp = (const U8*)(&nv);
        HEXTRACT_GET_SUBNORMAL(nv);
        HEXTRACT_IMPLICIT_BIT(nv);
        HEXTRACT_TOP_NYBBLE(2); /* 6 */
        HEXTRACT_BYTE(1); /* 5 */
        HEXTRACT_BYTE(0); /* 4 */
        HEXTRACT_BYTE(7); /* 3 */
        HEXTRACT_BYTE(6); /* 2 */
        HEXTRACT_BYTE(5); /* 1 */
        HEXTRACT_BYTE(4); /* 0 */
#    elif DOUBLEKIND == DOUBLE_IS_IEEE_754_64_BIT_MIXED_ENDIAN_BE_LE
        /* 3 2 1 0 7 6 5 4 (MSB = 7, LSB = 0, 7:6 = sign:exponent:nybble) */
        const U8* nvp = (const U8*)(&nv);
        HEXTRACT_GET_SUBNORMAL(nv);
        HEXTRACT_IMPLICIT_BIT(nv);
        HEXTRACT_TOP_NYBBLE(5); /* 6 */
        HEXTRACT_BYTE(6); /* 5 */
        HEXTRACT_BYTE(7); /* 4 */
        HEXTRACT_BYTE(0); /* 3 */
        HEXTRACT_BYTE(1); /* 2 */
        HEXTRACT_BYTE(2); /* 1 */
        HEXTRACT_BYTE(3); /* 0 */
#    else
#      define HEXTRACT_FALLBACK
#    endif
#  else
#    define HEXTRACT_FALLBACK
#  endif
#endif /* #if defined(USE_LONG_DOUBLE) && (NVSIZE > DOUBLESIZE) #else */

#ifdef HEXTRACT_FALLBACK
        HEXTRACT_GET_SUBNORMAL(nv);
#  undef HEXTRACT_HAS_TOP_NYBBLE /* Meaningless, but consistent. */
        /* The fallback is used for the double-double format, and
         * for unknown long double formats, and for unknown double
         * formats, or in general unknown NV formats. */
        if (nv == (NV)0.0) {
            if (vend)
                *v++ = 0;
            else
                v++;
            *exponent = 0;
        }
        else {
            NV d = nv < 0 ? -nv : nv;
            NV e = (NV)1.0;
            U8 ha = 0x0; /* hexvalue accumulator */
            U8 hd = 0x8; /* hexvalue digit */

            /* Shift d and e (and update exponent) so that e <= d < 2*e,
             * this is essentially manual frexp(). Multiplying by 0.5 and
             * doubling should be lossless in binary floating point. */

            *exponent = 1;

            while (e > d) {
                e *= (NV)0.5;
                (*exponent)--;
            }
            /* Now d >= e */

            while (d >= e + e) {
                e += e;
                (*exponent)++;
            }
            /* Now e <= d < 2*e */

            /* First extract the leading hexdigit (the implicit bit). */
            if (d >= e) {
                d -= e;
                if (vend)
                    *v++ = 1;
                else
                    v++;
            }
            else {
                if (vend)
                    *v++ = 0;
                else
                    v++;
            }
            e *= (NV)0.5;

            /* Then extract the remaining hexdigits. */
            while (d > (NV)0.0) {
                if (d >= e) {
                    ha |= hd;
                    d -= e;
                }
                if (hd == 1) {
                    /* Output or count in groups of four bits,
                     * that is, when the hexdigit is down to one. */
                    if (vend)
                        *v++ = ha;
                    else
                        v++;
                    /* Reset the hexvalue. */
                    ha = 0x0;
                    hd = 0x8;
                }
                else
                    hd >>= 1;
                e *= (NV)0.5;
            }

            /* Flush possible pending hexvalue. */
            if (ha) {
                if (vend)
                    *v++ = ha;
                else
                    v++;
            }
        }
#endif
    }
    /* Croak for various reasons: if the output pointer escaped the
     * output buffer, if the extraction index escaped the extraction
     * buffer, or if the ending output pointer didn't match the
     * previously computed value. */
    if (v <= vhex || v - vhex >= VHEX_SIZE ||
        /* For double-double the ixmin and ixmax stay at zero,
         * which is convenient since the HEXTRACTSIZE is tricky
         * for double-double. */
        ixmin < 0 || ixmax >= NVSIZE ||
        (vend && v != vend)) {
        /* diag_listed_as: Hexadecimal float: internal error (%s) */
        Perl_croak(aTHX_ "Hexadecimal float: internal error (overflow)");
    }
    return v;
}


/* S_format_hexfp(): helper function for Perl_sv_vcatpvfn_flags().
 *
 * Processes the %a/%A hexadecimal floating-point format, since the
 * built-in snprintf()s which are used for most of the f/p formats, don't
 * universally handle %a/%A.
 * Populates buf of length bufsize, and returns the length of the created
 * string.
 * The rest of the args have the same meaning as the local vars of the
 * same name within Perl_sv_vcatpvfn_flags().
 *
 * The caller's determination of IN_LC(LC_NUMERIC), passed as in_lc_numeric,
 * is used to ensure we do the right thing when we need to access the locale's
 * numeric radix.
 *
 * It requires the caller to make buf large enough.
 */

static STRLEN
S_format_hexfp(pTHX_ char * const buf, const STRLEN bufsize, const char c,
                    const NV nv, const vcatpvfn_long_double_t fv,
                    bool has_precis, STRLEN precis, STRLEN width,
                    bool alt, char plus, bool left, bool fill, bool in_lc_numeric)
{
    /* Hexadecimal floating point. */
    char* p = buf;
    U8 vhex[VHEX_SIZE];
    U8* v = vhex; /* working pointer to vhex */
    U8* vend; /* pointer to one beyond last digit of vhex */
    U8* vfnz = NULL; /* first non-zero */
    U8* vlnz = NULL; /* last non-zero */
    U8* v0 = NULL; /* first output */
    const bool lower = (c == 'a');
    /* At output the values of vhex (up to vend) will
     * be mapped through the xdig to get the actual
     * human-readable xdigits. */
    const char* xdig = PL_hexdigit;
    STRLEN zerotail = 0; /* how many extra zeros to append */
    int exponent = 0; /* exponent of the floating point input */
    bool hexradix = FALSE; /* should we output the radix */
    bool subnormal = FALSE; /* IEEE 754 subnormal/denormal */
    bool negative = FALSE;
    STRLEN elen;

    /* XXX: NaN, Inf -- though they are printed as "NaN" and "Inf".
     *
     * For example with denormals, (assuming the vanilla
     * 64-bit double): the exponent is zero. 1xp-1074 is
     * the smallest denormal and the smallest double, it
     * could be output also as 0x0.0000000000001p-1022 to
     * match its internal structure. */

    vend = S_hextract(aTHX_ nv, &exponent, &subnormal, vhex, NULL);
    S_hextract(aTHX_ nv, &exponent, &subnormal, vhex, vend);

#if NVSIZE > DOUBLESIZE
#  ifdef HEXTRACT_HAS_IMPLICIT_BIT
    /* In this case there is an implicit bit,
     * and therefore the exponent is shifted by one. */
    exponent--;
#  elif defined(NV_X86_80_BIT)
    if (subnormal) {
        /* The subnormals of the x86-80 have a base exponent of -16382,
         * (while the physical exponent bits are zero) but the frexp()
         * returned the scientific-style floating exponent.  We want
         * to map the last one as:
         * -16831..-16384 -> -16382 (the last normal is 0x1p-16382)
         * -16835..-16388 -> -16384
         * since we want to keep the first hexdigit
         * as one of the [8421]. */
        exponent = -4 * ( (exponent + 1) / -4) - 2;
    } else {
        exponent -= 4;
    }
    /* TBD: other non-implicit-bit platforms than the x86-80. */
#  endif
#endif

    negative = fv < 0 || Perl_signbit(nv);
    if (negative)
        *p++ = '-';
    else if (plus)
        *p++ = plus;
    *p++ = '0';
    if (lower) {
        *p++ = 'x';
    }
    else {
        *p++ = 'X';
        xdig += 16; /* Use uppercase hex. */
    }

    /* Find the first non-zero xdigit. */
    for (v = vhex; v < vend; v++) {
        if (*v) {
            vfnz = v;
            break;
        }
    }

    if (vfnz) {
        /* Find the last non-zero xdigit. */
        for (v = vend - 1; v >= vhex; v--) {
            if (*v) {
                vlnz = v;
                break;
            }
        }

#if NVSIZE == DOUBLESIZE
        if (fv != 0.0)
            exponent--;
#endif

        if (subnormal) {
#ifndef NV_X86_80_BIT
          if (vfnz[0] > 1) {
            /* IEEE 754 subnormals (but not the x86 80-bit):
             * we want "normalize" the subnormal,
             * so we need to right shift the hex nybbles
             * so that the output of the subnormal starts
             * from the first true bit.  (Another, equally
             * valid, policy would be to dump the subnormal
             * nybbles as-is, to display the "physical" layout.) */
            int i, n;
            U8 *vshr;
            /* Find the ceil(log2(v[0])) of
             * the top non-zero nybble. */
            for (i = vfnz[0], n = 0; i > 1; i >>= 1, n++) { }
            assert(n < 4);
            assert(vlnz);
            vlnz[1] = 0;
            for (vshr = vlnz; vshr >= vfnz; vshr--) {
              vshr[1] |= (vshr[0] & (0xF >> (4 - n))) << (4 - n);
              vshr[0] >>= n;
            }
            if (vlnz[1]) {
              vlnz++;
            }
          }
#endif
          v0 = vfnz;
        } else {
          v0 = vhex;
        }

        if (has_precis) {
            U8* ve = (subnormal ? vlnz + 1 : vend);
            SSize_t vn = ve - v0;
            assert(vn >= 1);
            if (precis < (Size_t)(vn - 1)) {
                bool overflow = FALSE;
                if (v0[precis + 1] < 0x8) {
                    /* Round down, nothing to do. */
                } else if (v0[precis + 1] > 0x8) {
                    /* Round up. */
                    v0[precis]++;
                    overflow = v0[precis] > 0xF;
                    v0[precis] &= 0xF;
                } else { /* v0[precis] == 0x8 */
                    /* Half-point: round towards the one
                     * with the even least-significant digit:
                     * 08 -> 0  88 -> 8
                     * 18 -> 2  98 -> a
                     * 28 -> 2  a8 -> a
                     * 38 -> 4  b8 -> c
                     * 48 -> 4  c8 -> c
                     * 58 -> 6  d8 -> e
                     * 68 -> 6  e8 -> e
                     * 78 -> 8  f8 -> 10 */
                    if ((v0[precis] & 0x1)) {
                        v0[precis]++;
                    }
                    overflow = v0[precis] > 0xF;
                    v0[precis] &= 0xF;
                }

                if (overflow) {
                    for (v = v0 + precis - 1; v >= v0; v--) {
                        (*v)++;
                        overflow = *v > 0xF;
                        (*v) &= 0xF;
                        if (!overflow) {
                            break;
                        }
                    }
                    if (v == v0 - 1 && overflow) {
                        /* If the overflow goes all the
                         * way to the front, we need to
                         * insert 0x1 in front, and adjust
                         * the exponent. */
                        Move(v0, v0 + 1, vn - 1, char);
                        *v0 = 0x1;
                        exponent += 4;
                    }
                }

                /* The new effective "last non zero". */
                vlnz = v0 + precis;
            }
            else {
                zerotail =
                  subnormal ? precis - vn + 1 :
                  precis - (vlnz - vhex);
            }
        }

        v = v0;
        *p++ = xdig[*v++];

        /* If there are non-zero xdigits, the radix
         * is output after the first one. */
        if (vfnz < vlnz) {
          hexradix = TRUE;
        }
    }
    else {
        *p++ = '0';
        exponent = 0;
        zerotail = has_precis ? precis : 0;
    }

    /* The radix is always output if precis, or if alt. */
    if ((has_precis && precis > 0) || alt) {
      hexradix = TRUE;
    }

    if (hexradix) {
#ifndef USE_LOCALE_NUMERIC
        PERL_UNUSED_ARG(in_lc_numeric);

        *p++ = '.';
#else
        if (in_lc_numeric) {
            STRLEN n;
            WITH_LC_NUMERIC_SET_TO_NEEDED_IN(TRUE, {
                const char* r = SvPV(PL_numeric_radix_sv, n);
                Copy(r, p, n, char);
            });
            p += n;
        }
        else {
            *p++ = '.';
        }
#endif
    }

    if (vlnz) {
        while (v <= vlnz)
            *p++ = xdig[*v++];
    }

    if (zerotail > 0) {
      while (zerotail--) {
        *p++ = '0';
      }
    }

    elen = p - buf;

    /* sanity checks */
    if (elen >= bufsize || width >= bufsize)
        /* diag_listed_as: Hexadecimal float: internal error (%s) */
        Perl_croak(aTHX_ "Hexadecimal float: internal error (overflow)");

    elen += my_snprintf(p, bufsize - elen,
                        "%c%+d", lower ? 'p' : 'P',
                        exponent);

    if (elen < width) {
        STRLEN gap = (STRLEN)(width - elen);
        if (left) {
            /* Pad the back with spaces. */
            memset(buf + elen, ' ', gap);
        }
        else if (fill) {
            /* Insert the zeros after the "0x" and the
             * the potential sign, but before the digits,
             * otherwise we end up with "0000xH.HHH...",
             * when we want "0x000H.HHH..."  */
            STRLEN nzero = gap;
            char* zerox = buf + 2;
            STRLEN nmove = elen - 2;
            if (negative || plus) {
                zerox++;
                nmove--;
            }
            Move(zerox, zerox + nzero, nmove, char);
            memset(zerox, fill ? '0' : ' ', nzero);
        }
        else {
            /* Move it to the right. */
            Move(buf, buf + gap,
                 elen, char);
            /* Pad the front with spaces. */
            memset(buf, ' ', gap);
        }
        elen = width;
    }
    return elen;
}

/*
=for apidoc sv_vcatpvfn
=for apidoc_item sv_vcatpvfn_flags

These process their arguments like C<L<vsprintf(3)>> and append the formatted output
to an SV.  They use an array of SVs if the C-style variable argument list is
missing (C<NULL>). Argument reordering (using format specifiers like C<%2$d> or
C<%*2$d>) is supported only when using an array of SVs; using a C-style
C<va_list> argument list with a format string that uses argument reordering
will yield an exception.

When running with taint checks enabled, they indicate via C<maybe_tainted> if
results are untrustworthy (often due to the use of locales).

They assume that C<pat> has the same utf8-ness as C<sv>.  It's the caller's
responsibility to ensure that this is so.

They differ in that C<sv_vcatpvfn_flags> has a C<flags> parameter in which you
can set or clear the C<SV_GMAGIC> and/or S<SV_SMAGIC> flags, to specify which
magic to handle or not handle; whereas plain C<sv_vcatpvfn> always specifies
both 'get' and 'set' magic.

They are usually used via one of the frontends L</C<sv_vcatpvf>> and
L</C<sv_vcatpvf_mg>>.

=cut
*/


void
Perl_sv_vcatpvfn_flags(pTHX_ SV *const sv, const char *const pat, const STRLEN patlen,
                       va_list *const args, SV **const svargs, const Size_t sv_count, bool *const maybe_tainted,
                       const U32 flags)
{
    const char *fmtstart; /* character following the current '%' */
    const char *q;        /* current position within format */
    const char *patend;
    STRLEN origlen;
    Size_t svix = 0;
    static const char nullstr[] = "(null)";
    bool has_utf8 = DO_UTF8(sv);    /* has the result utf8? */
    const bool pat_utf8 = has_utf8; /* the pattern is in utf8? */
    /* Times 4: a decimal digit takes more than 3 binary digits.
     * NV_DIG: mantissa takes that many decimal digits.
     * Plus 32: Playing safe. */
    char ebuf[IV_DIG * 4 + NV_DIG + 32];
    bool no_redundant_warning = FALSE; /* did we use any explicit format parameter index? */
#ifdef USE_LOCALE_NUMERIC
    bool have_in_lc_numeric = FALSE;
#endif
    /* we never change this unless USE_LOCALE_NUMERIC */
    bool in_lc_numeric = FALSE;
    SV *tmp_sv = NULL;

    PERL_ARGS_ASSERT_SV_VCATPVFN_FLAGS;
    PERL_UNUSED_ARG(maybe_tainted);

    if (flags & SV_GMAGIC)
        SvGETMAGIC(sv);

    /* no matter what, this is a string now */
    (void)SvPV_force_nomg(sv, origlen);

    /* the code that scans for flags etc following a % relies on
     * a '\0' being present to avoid falling off the end. Ideally that
     * should be fixed */
    assert(pat[patlen] == '\0');


    /* Special-case "", "%s", "%-p" (SVf - see below) and "%.0f".
     * In each case, if there isn't the correct number of args, instead
     * fall through to the main code to handle the issuing of any
     * warnings etc.
     */

    if (patlen == 0 && (args || sv_count == 0))
        return;

    if (patlen <= 4 && pat[0] == '%' && (args || sv_count == 1)) {

        /* "%s" */
        if (patlen == 2 && pat[1] == 's') {
            if (args) {
                const char * const s = va_arg(*args, char*);
                sv_catpv_nomg(sv, s ? s : nullstr);
            }
            else {
                /* we want get magic on the source but not the target.
                 * sv_catsv can't do that, though */
                SvGETMAGIC(*svargs);
                sv_catsv_nomg(sv, *svargs);
            }
            return;
        }

        /* "%-p" */
        if (args) {
            if (patlen == 3  && pat[1] == '-' && pat[2] == 'p') {
                SV *asv = MUTABLE_SV(va_arg(*args, void*));
                sv_catsv_nomg(sv, asv);
                return;
            }
        }
#if !defined(USE_LONG_DOUBLE) && !defined(USE_QUADMATH)
        /* special-case "%.0f" */
        else if (   patlen == 4
                 && pat[1] == '.' && pat[2] == '0' && pat[3] == 'f')
        {
            const NV nv = SvNV(*svargs);
            if (LIKELY(!Perl_isinfnan(nv))) {
                STRLEN l;
                char *p;

                if ((p = F0convert(nv, ebuf + sizeof ebuf, &l))) {
                    sv_catpvn_nomg(sv, p, l);
                    return;
                }
            }
        }
#endif /* !USE_LONG_DOUBLE */
    }


    patend = (char*)pat + patlen;
    for (fmtstart = pat; fmtstart < patend; fmtstart = q) {
        char intsize     = 0;         /* size qualifier in "%hi..." etc */
        bool alt         = FALSE;     /* has      "%#..."    */
        bool left        = FALSE;     /* has      "%-..."    */
        bool fill        = FALSE;     /* has      "%0..."    */
        char plus        = 0;         /* has      "%+..."    */
        STRLEN width     = 0;         /* value of "%NNN..."  */
        bool has_precis  = FALSE;     /* has      "%.NNN..." */
        STRLEN precis    = 0;         /* value of "%.NNN..." */
        int base         = 0;         /* base to print in, e.g. 8 for %o */
        UV uv            = 0;         /* the value to print of int-ish args */

        bool vectorize   = FALSE;     /* has      "%v..."    */
        bool vec_utf8    = FALSE;     /* SvUTF8(vec arg)     */
        const U8 *vecstr = NULL;      /* SvPVX(vec arg)      */
        STRLEN veclen    = 0;         /* SvCUR(vec arg)      */
        const char *dotstr = NULL;    /* separator string for %v */
        STRLEN dotstrlen;             /* length of separator string for %v */

        Size_t efix      = 0;         /* explicit format parameter index */
        const Size_t osvix  = svix;   /* original index in case of bad fmt */

        SV *argsv        = NULL;
        bool is_utf8     = FALSE;     /* is this item utf8?   */
        bool arg_missing = FALSE;     /* give "Missing argument" warning */
        char esignbuf[4];             /* holds sign prefix, e.g. "-0x" */
        STRLEN esignlen  = 0;         /* length of e.g. "-0x" */
        STRLEN zeros     = 0;         /* how many '0' to prepend */

        const char *eptr = NULL;      /* the address of the element string */
        STRLEN elen      = 0;         /* the length  of the element string */

        char c;                       /* the actual format ('d', s' etc) */

        bool escape_it   = FALSE;     /* if this is a string should we quote and escape it? */


        /* echo everything up to the next format specification */
        for (q = fmtstart; q < patend && *q != '%'; ++q)
            {};

        if (q > fmtstart) {
            if (has_utf8 && !pat_utf8) {
                /* upgrade and copy the bytes of fmtstart..q-1 to utf8 on
                 * the fly */
                const char *p;
                char *dst;
                STRLEN need = SvCUR(sv) + (q - fmtstart) + 1;

                for (p = fmtstart; p < q; p++)
                    if (!NATIVE_BYTE_IS_INVARIANT(*p))
                        need++;
                SvGROW(sv, need);

                dst = SvEND(sv);
                for (p = fmtstart; p < q; p++)
                    append_utf8_from_native_byte((U8)*p, (U8**)&dst);
                *dst = '\0';
                SvCUR_set(sv, need - 1);
            }
            else
                S_sv_catpvn_simple(aTHX_ sv, fmtstart, q - fmtstart);
        }
        if (q++ >= patend)
            break;

        fmtstart = q; /* fmtstart is char following the '%' */

/*
    We allow format specification elements in this order:
        \d+\$              explicit format parameter index
        [-+ 0#]+           flags
        v|\*(\d+\$)?v      vector with optional (optionally specified) arg
        0		   flag (as above): repeated to allow "v02"
        \d+|\*(\d+\$)?     width using optional (optionally specified) arg
        \.(\d*|\*(\d+\$)?) precision using optional (optionally specified) arg
        [hlqLV]            size
    [%bcdefginopsuxDFOUX] format (mandatory)
*/

        if (inRANGE(*q, '1', '9')) {
            width = expect_number(&q);
            if (*q == '$') {
                if (args)
                    Perl_croak_nocontext(
                        "Cannot yet reorder sv_vcatpvfn() arguments from va_list");
                ++q;
                efix = (Size_t)width;
                width = 0;
                no_redundant_warning = TRUE;
            } else {
                goto gotwidth;
            }
        }

        /* FLAGS */

        while (*q) {
            switch (*q) {
            case ' ':
            case '+':
                if (plus == '+' && *q == ' ') /* '+' over ' ' */
                    q++;
                else
                    plus = *q++;
                continue;

            case '-':
                left = TRUE;
                q++;
                continue;

            case '0':
                fill = TRUE;
                q++;
                continue;

            case '#':
                alt = TRUE;
                q++;
                continue;

            default:
                break;
            }
            break;
        }

      /* at this point we can expect one of:
       *
       *  123  an explicit width
       *  *    width taken from next arg
       *  *12$ width taken from 12th arg
       *       or no width
       *
       * But any width specification may be preceded by a v, in one of its
       * forms:
       *        v
       *        *v
       *        *12$v
       * So an asterisk may be either a width specifier or a vector
       * separator arg specifier, and we don't know which initially
       */

      tryasterisk:
        if (*q == '*') {
            STRLEN ix; /* explicit width/vector separator index */
            q++;
            if (inRANGE(*q, '1', '9')) {
                ix = expect_number(&q);
                if (*q++ == '$') {
                    if (args)
                        Perl_croak_nocontext(
                            "Cannot yet reorder sv_vcatpvfn() arguments from va_list");
                    no_redundant_warning = TRUE;
                } else
                    goto unknown;
            }
            else
                ix = 0;

            if (*q == 'v') {
                SV *vecsv;
                /* The asterisk was for  *v, *NNN$v: vectorizing, but not
                 * with the default "." */
                q++;
                if (vectorize)
                    goto unknown;
                if (args)
                    vecsv = va_arg(*args, SV*);
                else {
                    ix = ix ? ix - 1 : svix++;
                    vecsv = ix < sv_count ? svargs[ix]
                                       : (arg_missing = TRUE, &PL_sv_no);
                }
                dotstr = SvPV_const(vecsv, dotstrlen);
                /* Keep the DO_UTF8 test *after* the SvPV call, else things go
                   bad with tied or overloaded values that return UTF8.  */
                if (DO_UTF8(vecsv))
                    is_utf8 = TRUE;
                else if (has_utf8) {
                    vecsv = sv_mortalcopy(vecsv);
                    sv_utf8_upgrade(vecsv);
                    dotstr = SvPV_const(vecsv, dotstrlen);
                    is_utf8 = TRUE;
                }
                vectorize = TRUE;
                goto tryasterisk;
            }

            /* the asterisk specified a width */
            {
                int i = 0;
                SV *width_sv = NULL;
                if (args)
                    i = va_arg(*args, int);
                else {
                    ix = ix ? ix - 1 : svix++;
                    width_sv = (ix < sv_count) ? svargs[ix]
                                      : (arg_missing = TRUE, (SV*)NULL);
                }
                width = S_sprintf_arg_num_val(aTHX_ args, i, width_sv, &left);
            }
        }
        else if (*q == 'v') {
            q++;
            if (vectorize)
                goto unknown;
            vectorize = TRUE;
            dotstr = ".";
            dotstrlen = 1;
            goto tryasterisk;

        }
        else {
        /* explicit width? */
            if(*q == '0') {
                fill = TRUE;
                q++;
            }
            if (inRANGE(*q, '1', '9'))
                width = expect_number(&q);
        }

      gotwidth:

        /* PRECISION */

        if (*q == '.') {
            q++;
            if (*q == '*') {
                STRLEN ix; /* explicit precision index */
                q++;
                if (inRANGE(*q, '1', '9')) {
                    ix = expect_number(&q);
                    if (*q++ == '$') {
                        if (args)
                            Perl_croak_nocontext(
                                "Cannot yet reorder sv_vcatpvfn() arguments from va_list");
                        no_redundant_warning = TRUE;
                    } else
                        goto unknown;
                }
                else
                    ix = 0;

                {
                    int i = 0;
                    SV *width_sv = NULL;
                    bool neg = FALSE;

                    if (args)
                        i = va_arg(*args, int);
                    else {
                        ix = ix ? ix - 1 : svix++;
                        width_sv = (ix < sv_count) ? svargs[ix]
                                          : (arg_missing = TRUE, (SV*)NULL);
                    }
                    precis = S_sprintf_arg_num_val(aTHX_ args, i, width_sv, &neg);
                    has_precis = !neg;
                    /* ignore negative precision */
                    if (!has_precis)
                        precis = 0;
                }
            }
            else {
                /* although it doesn't seem documented, this code has long
                 * behaved so that:
                 *   no digits following the '.' is treated like '.0'
                 *   the number may be preceded by any number of zeroes,
                 *      e.g. "%.0001f", which is the same as "%.1f"
                 * so I've kept that behaviour. DAPM May 2017
                 */
                while (*q == '0')
                    q++;
                precis = inRANGE(*q, '1', '9') ? expect_number(&q) : 0;
                has_precis = TRUE;
            }
        }

        /* SIZE */

        switch (*q) {
#ifdef WIN32
        case 'I':			/* Ix, I32x, and I64x */
#  ifdef USE_64_BIT_INT
            if (q[1] == '6' && q[2] == '4') {
                q += 3;
                intsize = 'q';
                break;
            }
#  endif
            if (q[1] == '3' && q[2] == '2') {
                q += 3;
                break;
            }
#  ifdef USE_64_BIT_INT
            intsize = 'q';
#  endif
            q++;
            break;
#endif
#if (IVSIZE >= 8 || defined(HAS_LONG_DOUBLE)) || \
    (IVSIZE == 4 && !defined(HAS_LONG_DOUBLE))
        case 'L':			/* Ld */
            /* FALLTHROUGH */
#  if IVSIZE >= 8
        case 'q':			/* qd */
#  endif
            intsize = 'q';
            q++;
            break;
#endif
        case 'l':
            ++q;
#if (IVSIZE >= 8 || defined(HAS_LONG_DOUBLE)) || \
    (IVSIZE == 4 && !defined(HAS_LONG_DOUBLE))
            if (*q == 'l') {	/* lld, llf */
                intsize = 'q';
                ++q;
            }
            else
#endif
                intsize = 'l';
            break;
        case 'h':
            if (*++q == 'h') {	/* hhd, hhu */
                intsize = 'c';
                ++q;
            }
            else
                intsize = 'h';
            break;
#ifdef USE_QUADMATH
        case 'Q':
#endif
        case 'V':
        case 'z':
        case 't':
        case 'j':
            intsize = *q++;
            break;
        }

        /* CONVERSION */

        c = *q++; /* c now holds the conversion type */

        /* '%' doesn't have an arg, so skip arg processing */
        if (c == '%') {
            eptr = q - 1;
            elen = 1;
            if (vectorize)
                goto unknown;
            goto string;
        }

        if (vectorize && !memCHRs("BbDdiOouUXx", c))
            goto unknown;

        /* get next arg (individual branches do their own va_arg()
         * handling for the args case) */

        if (!args) {
            efix = efix ? efix - 1 : svix++;
            argsv = efix < sv_count ? svargs[efix]
                                 : (arg_missing = TRUE, &PL_sv_no);
        }


        switch (c) {

            /* STRINGS */

        case 's':
            if (args) {
                eptr = va_arg(*args, char*);
                if (eptr)
                    if (has_precis)
                        elen = my_strnlen(eptr, precis);
                    else
                        elen = strlen(eptr);
                else {
                    eptr = (char *)nullstr;
                    elen = sizeof nullstr - 1;
                }
            }
            else {
                eptr = SvPV_const(argsv, elen);
                if (DO_UTF8(argsv)) {
                    STRLEN old_precis = precis;
                    if (has_precis && precis < elen) {
                        STRLEN ulen = sv_or_pv_len_utf8(argsv, eptr, elen);
                        STRLEN p = precis > ulen ? ulen : precis;
                        precis = sv_or_pv_pos_u2b(argsv, eptr, p, 0);
                                                        /* sticks at end */
                    }
                    if (width) { /* fudge width (can't fudge elen) */
                        if (has_precis && precis < elen)
                            width += precis - old_precis;
                        else
                            width +=
                                elen - sv_or_pv_len_utf8(argsv,eptr,elen);
                    }
                    is_utf8 = TRUE;
                }
            }

        string:
            if (escape_it) {
                U32 flags = PERL_PV_PRETTY_QUOTEDPREFIX;
                if (is_utf8)
                    flags |= PERL_PV_ESCAPE_UNI;

                if (!tmp_sv) {
                    /* "blah"... where blah might be made up
                     * of characters like \x{1234} */
                    tmp_sv = newSV(1 + (PERL_QUOTEDPREFIX_LEN * 8) + 1 + 3);
                    sv_2mortal(tmp_sv);
                }
                pv_pretty(tmp_sv, eptr, elen, PERL_QUOTEDPREFIX_LEN,
                            NULL, NULL, flags);
                eptr = SvPV_const(tmp_sv, elen);
            }
            if (has_precis && precis < elen)
                elen = precis;
            break;

            /* INTEGERS */

        case 'p':

            /* BEGIN NOTE
             *
             * We want to extend the C level sprintf format API with
             * custom formats for specific types (eg SV*) and behavior.
             * However some C compilers are "sprintf aware" and will
             * throw compile time exceptions when an illegal sprintf is
             * encountered, so we can't just add new format letters.
             *
             * However it turns out the length argument to the %p format
             * is more or less useless (the size of a pointer does not
             * change over time) and is not really used in the C level
             * code. Accordingly we can map our special behavior to
             * specific "length" options to the %p format. We hide these
             * mappings behind defines anyway, so nobody needs to know
             * that HEKf is actually %2p. This keeps the C compiler
             * happy while allowing us to add new formats.
             *
             * Note the existing logic for which number is used for what
             * is torturous. All negative values are used for SVf, and
             * non-negative values have arbitrary meanings with no
             * structure to them. This may change in the future.
             *
             * NEVER use the raw %p values directly. Always use the define
             * as the underlying mapping may change in the future.
             *
             * END NOTE
             *
             * %p extensions:
             *
             * "%...p" is normally treated like "%...x", except that the
             * number to print is the SV's address (or a pointer address
             * for C-ish sprintf).
             *
             * However, the C-ish sprintf variant allows a few special
             * extensions. These are currently:
             *
             * %-p       (SVf)  Like %s, but gets the string from an SV*
             *                  arg rather than a char* arg. Use C<SVfARG()>
             *                  to set up the argument properly.
             *                  (This was previously %_).
             *
             * %-<num>p         Ditto but like %.<num>s (i.e. num is max
             *                  width), there is no escaped and quoted version
             *                  of this.
             *
             * %1p       (PVf_QUOTEDPREFIX). Like raw %s, but it is escaped
             *                  and quoted.
             *
             * %5p       (SVf_QUOTEDPREFIX) Like SVf, but length restricted,
             *                  escaped and quoted with pv_pretty. Intended
             *                  for error messages.
             *
             * %2p       (HEKf) Like %s, but using the key string in a HEK
             * %7p       (HEKf_QUOTEDPREFIX) ... but escaped and quoted.
             *
             * %3p       (HEKf256) Ditto but like %.256s
             * %8p       (HEKf256_QUOTEDPREFIX) ... but escaped and quoted
             *
             * %d%lu%4p  (UTF8f) A utf8 string. Consumes 3 args:
             *                       (cBOOL(utf8), len, string_buf).
             *                   It's handled by the "case 'd'" branch
             *                   rather than here.
             * %d%lu%9p  (UTF8f_QUOTEDPREFIX) .. but escaped and quoted.
             *
             * %6p       (HvNAMEf) Like %s, but using the HvNAME() and HvNAMELEN()
             * %10p      (HvNAMEf_QUOTEDPREFIX) ... but escaped and quoted
             *
             * %<num>p   where num is > 9: reserved for future
             *           extensions. Warns, but then is treated as a
             *           general %p (print hex address) format.
             *
             * NOTE: If you add a new magic %p value you will
             * need to update F<t/porting/diag.t> to be aware of it
             * on top of adding the various defines and etc. Do not
             * forget to add it to F<pod/perlguts.pod> as well.
             */

            if (   args
                && !intsize
                && !fill
                && !plus
                && !has_precis
                    /* not %*p or %*1$p - any width was explicit */
                && q[-2] != '*'
                && q[-2] != '$'
            ) {
                if (left || width == 5) {                /* %-p (SVf), %-NNNp, %5p */
                    if (left && width) {
                        precis = width;
                        has_precis = TRUE;
                    } else if (width == 5) {
                        escape_it = TRUE;
                    }
                    argsv = MUTABLE_SV(va_arg(*args, void*));
                    eptr = SvPV_const(argsv, elen);
                    if (DO_UTF8(argsv))
                        is_utf8 = TRUE;
                    width = 0;
                    goto string;
                }
                else if (width == 2 || width == 3 ||
                         width == 7 || width == 8)
                {        /* HEKf, HEKf256, HEKf_QUOTEDPREFIX, HEKf256_QUOTEDPREFIX */
                    HEK * const hek = va_arg(*args, HEK *);
                    eptr = HEK_KEY(hek);
                    elen = HEK_LEN(hek);
                    if (HEK_UTF8(hek))
                        is_utf8 = TRUE;
                    if (width == 3) {
                        precis = 256;
                        has_precis = TRUE;
                    }
                    if (width > 5)
                        escape_it = TRUE;
                    width = 0;
                    goto string;
                }
                else if (width == 1) {
                    eptr = va_arg(*args,char *);
                    elen = strlen(eptr);
                    escape_it = TRUE;
                    width = 0;
                    goto string;
                }
                else if (width == 6 || width == 10) {
                    HV *hv = va_arg(*args, HV *);
                    eptr = HvNAME(hv);
                    elen = HvNAMELEN(hv);
                    if (HvNAMEUTF8(hv))
                        is_utf8 = TRUE;
                    if (width == 10)
                        escape_it = TRUE;
                    width = 0;
                    goto string;
                }
                else if (width) {
                    /* note width=4 or width=9 is handled under %d */
                    Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL),
                         "internal %%<num>p might conflict with future printf extensions");
                }
            }

            /* treat as normal %...p */

            uv = PTR2UV(args ? va_arg(*args, void*) : argsv);
            base = 16;
            c = 'x';    /* in case the format string contains '#' */
            goto do_integer;

        case 'c':
            /* Ignore any size specifiers, since they're not documented as
             * being allowed for %c (ideally we should warn on e.g. '%hc').
             * Setting a default intsize, along with a positive
             * (which signals unsigned) base, causes, for C-ish use, the
             * va_arg to be interpreted as an unsigned int, when it's
             * actually signed, which will convert -ve values to high +ve
             * values. Note that unlike the libc %c, values > 255 will
             * convert to high unicode points rather than being truncated
             * to 8 bits. For perlish use, it will do SvUV(argsv), which
             * will again convert -ve args to high -ve values.
             */
            intsize = 0;
            base = 1; /* special value that indicates we're doing a 'c' */
            goto get_int_arg_val;

        case 'D':
#ifdef IV_IS_QUAD
            intsize = 'q';
#else
            intsize = 'l';
#endif
            base = -10;
            goto get_int_arg_val;

        case 'd':
            /* probably just a plain %d, but it might be the start of the
             * special UTF8f format, which usually looks something like
             * "%d%lu%4p" (the lu may vary by platform) or
             * "%d%lu%9p" for an escaped version.
             */
            assert((UTF8f)[0] == 'd');
            assert((UTF8f)[1] == '%');

             if (   args              /* UTF8f only valid for C-ish sprintf */
                 && q == fmtstart + 1 /* plain %d, not %....d */
                 && patend >= fmtstart + sizeof(UTF8f) - 1 /* long enough */
                 && *q == '%'
                 && strnEQ(q + 1, (UTF8f) + 2, sizeof(UTF8f) - 5)
                 && q[sizeof(UTF8f)-3] == 'p'
                 && (q[sizeof(UTF8f)-4] == '4' ||
                     q[sizeof(UTF8f)-4] == '9'))
            {
                /* The argument has already gone through cBOOL, so the cast
                   is safe. */
                if (q[sizeof(UTF8f)-4] == '9')
                    escape_it = TRUE;
                is_utf8 = (bool)va_arg(*args, int);
                elen = va_arg(*args, UV);
                /* if utf8 length is larger than 0x7ffff..., then it might
                 * have been a signed value that wrapped */
                if (elen  > ((~(STRLEN)0) >> 1)) {
                    assert(0); /* in DEBUGGING build we want to crash */
                    elen = 0; /* otherwise we want to treat this as an empty string */
                }
                eptr = va_arg(*args, char *);
                q += sizeof(UTF8f) - 2;
                goto string;
            }

            /* FALLTHROUGH */
        case 'i':
            base = -10;
            goto get_int_arg_val;

        case 'U':
#ifdef IV_IS_QUAD
            intsize = 'q';
#else
            intsize = 'l';
#endif
            /* FALLTHROUGH */
        case 'u':
            base = 10;
            goto get_int_arg_val;

        case 'B':
        case 'b':
            base = 2;
            goto get_int_arg_val;

        case 'O':
#ifdef IV_IS_QUAD
            intsize = 'q';
#else
            intsize = 'l';
#endif
            /* FALLTHROUGH */
        case 'o':
            base = 8;
            goto get_int_arg_val;

        case 'X':
        case 'x':
            base = 16;

          get_int_arg_val:

            if (vectorize) {
                STRLEN ulen;
                SV *vecsv;

                if (base < 0) {
                    base = -base;
                    if (plus)
                         esignbuf[esignlen++] = plus;
                }

                /* initialise the vector string to iterate over */

                vecsv = args ? va_arg(*args, SV*) : argsv;

                /* if this is a version object, we need to convert
                 * back into v-string notation and then let the
                 * vectorize happen normally
                 */
                if (sv_isobject(vecsv) && sv_derived_from(vecsv, "version")) {
                    if ( hv_existss(MUTABLE_HV(SvRV(vecsv)), "alpha") ) {
                        Perl_ck_warner_d(aTHX_ packWARN(WARN_PRINTF),
                        "vector argument not supported with alpha versions");
                        vecsv = &PL_sv_no;
                    }
                    else {
                        vecstr = (U8*)SvPV_const(vecsv,veclen);
                        vecsv = sv_newmortal();
                        scan_vstring((char *)vecstr, (char *)vecstr + veclen,
                                     vecsv);
                    }
                }
                vecstr = (U8*)SvPV_const(vecsv, veclen);
                vec_utf8 = DO_UTF8(vecsv);

              /* This is the re-entry point for when we're iterating
               * over the individual characters of a vector arg */
              vector:
                if (!veclen)
                    goto done_valid_conversion;
                if (vec_utf8)
                    uv = utf8n_to_uvchr(vecstr, veclen, &ulen,
                                        UTF8_ALLOW_ANYUV);
                else {
                    uv = *vecstr;
                    ulen = 1;
                }
                vecstr += ulen;
                veclen -= ulen;
            }
            else {
                /* test arg for inf/nan. This can trigger an unwanted
                 * 'str' overload, so manually force 'num' overload first
                 * if necessary */
                if (argsv) {
                    SvGETMAGIC(argsv);
                    if (UNLIKELY(SvAMAGIC(argsv)))
                        argsv = sv_2num(argsv);
                    if (UNLIKELY(isinfnansv(argsv)))
                        goto handle_infnan_argsv;
                }

                if (base < 0) {
                    /* signed int type */
                    IV iv;
                    base = -base;
                    if (args) {
                        switch (intsize) {
                        case 'c':  iv = (char)va_arg(*args, int);  break;
                        case 'h':  iv = (short)va_arg(*args, int); break;
                        case 'l':  iv = va_arg(*args, long);       break;
                        case 'V':  iv = va_arg(*args, IV);         break;
                        case 'z':  iv = va_arg(*args, SSize_t);    break;
#ifdef HAS_PTRDIFF_T
                        case 't':  iv = va_arg(*args, ptrdiff_t);  break;
#endif
                        default:   iv = va_arg(*args, int);        break;
                        case 'j':  iv = (IV) va_arg(*args, PERL_INTMAX_T); break;
                        case 'q':
#if IVSIZE >= 8
                                   iv = va_arg(*args, Quad_t);     break;
#else
                                   goto unknown;
#endif
                        }
                    }
                    else {
                        /* assign to tiv then cast to iv to work around
                         * 2003 GCC cast bug (gnu.org bugzilla #13488) */
                        IV tiv = SvIV_nomg(argsv);
                        switch (intsize) {
                        case 'c':  iv = (char)tiv;   break;
                        case 'h':  iv = (short)tiv;  break;
                        case 'l':  iv = (long)tiv;   break;
                        case 'V':
                        default:   iv = tiv;         break;
                        case 'q':
#if IVSIZE >= 8
                                   iv = (Quad_t)tiv; break;
#else
                                   goto unknown;
#endif
                        }
                    }

                    /* now convert iv to uv */
                    if (iv >= 0) {
                        uv = iv;
                        if (plus)
                            esignbuf[esignlen++] = plus;
                    }
                    else {
                        /* Using 0- here to silence bogus warning from MS VC */
                        uv = (UV) (0 - (UV) iv);
                        esignbuf[esignlen++] = '-';
                    }
                }
                else {
                    /* unsigned int type */
                    if (args) {
                        switch (intsize) {
                        case 'c': uv = (unsigned char)va_arg(*args, unsigned);
                                  break;
                        case 'h': uv = (unsigned short)va_arg(*args, unsigned);
                                  break;
                        case 'l': uv = va_arg(*args, unsigned long); break;
                        case 'V': uv = va_arg(*args, UV);            break;
                        case 'z': uv = va_arg(*args, Size_t);        break;
#ifdef HAS_PTRDIFF_T
                                  /* will sign extend, but there is no
                                   * uptrdiff_t, so oh well */
                        case 't': uv = va_arg(*args, ptrdiff_t);     break;
#endif
                        case 'j': uv = (UV) va_arg(*args, PERL_UINTMAX_T); break;
                        default:  uv = va_arg(*args, unsigned);      break;
                        case 'q':
#if IVSIZE >= 8
                                  uv = va_arg(*args, Uquad_t);       break;
#else
                                  goto unknown;
#endif
                        }
                    }
                    else {
                        /* assign to tiv then cast to iv to work around
                         * 2003 GCC cast bug (gnu.org bugzilla #13488) */
                        UV tuv = SvUV_nomg(argsv);
                        switch (intsize) {
                        case 'c': uv = (unsigned char)tuv;  break;
                        case 'h': uv = (unsigned short)tuv; break;
                        case 'l': uv = (unsigned long)tuv;  break;
                        case 'V':
                        default:  uv = tuv;                 break;
                        case 'q':
#if IVSIZE >= 8
                                  uv = (Uquad_t)tuv;        break;
#else
                                  goto unknown;
#endif
                        }
                    }
                }
            }

        do_integer:
            {
                char *ptr = ebuf + sizeof ebuf;
                unsigned dig;
                zeros = 0;

                switch (base) {
                case 16:
                    {
                    const char * const p =
                            (c == 'X') ? PL_hexdigit + 16 : PL_hexdigit;

                        do {
                            dig = uv & 15;
                            *--ptr = p[dig];
                        } while (uv >>= 4);
                        if (alt && *ptr != '0') {
                            esignbuf[esignlen++] = '0';
                            esignbuf[esignlen++] = c;  /* 'x' or 'X' */
                        }
                        break;
                    }
                case 8:
                    do {
                        dig = uv & 7;
                        *--ptr = '0' + dig;
                    } while (uv >>= 3);
                    if (alt && *ptr != '0')
                        *--ptr = '0';
                    break;
                case 2:
                    do {
                        dig = uv & 1;
                        *--ptr = '0' + dig;
                    } while (uv >>= 1);
                    if (alt && *ptr != '0') {
                        esignbuf[esignlen++] = '0';
                        esignbuf[esignlen++] = c; /* 'b' or 'B' */
                    }
                    break;

                case 1:
                    /* special-case: base 1 indicates a 'c' format:
                     * we use the common code for extracting a uv,
                     * but handle that value differently here than
                     * all the other int types */
                    if ((uv > 255 ||
                         (!UVCHR_IS_INVARIANT(uv) && SvUTF8(sv)))
                        && !IN_BYTES)
                    {
                        STATIC_ASSERT_STMT(sizeof(ebuf) >= UTF8_MAXBYTES + 1);
                        eptr = ebuf;
                        elen = uvchr_to_utf8((U8*)eptr, uv) - (U8*)ebuf;
                        is_utf8 = TRUE;
                    }
                    else {
                        eptr = ebuf;
                        ebuf[0] = (char)uv;
                        elen = 1;
                    }
                    goto string;

                default:		/* it had better be ten or less */
                    do {
                        dig = uv % base;
                        *--ptr = '0' + dig;
                    } while (uv /= base);
                    break;
                }
                elen = (ebuf + sizeof ebuf) - ptr;
                eptr = ptr;
                if (has_precis) {
                    if (precis > elen)
                        zeros = precis - elen;
                    else if (precis == 0 && elen == 1 && *eptr == '0'
                             && !(base == 8 && alt)) /* "%#.0o" prints "0" */
                        elen = 0;

                    /* a precision nullifies the 0 flag. */
                    fill = FALSE;
                }
            }
            break;

            /* FLOATING POINT */

        case 'F':
            c = 'f';		/* maybe %F isn't supported here */
            /* FALLTHROUGH */
        case 'e': case 'E':
        case 'f':
        case 'g': case 'G':
        case 'a': case 'A':

        {
            STRLEN float_need; /* what PL_efloatsize needs to become */
            bool hexfp;        /* hexadecimal floating point? */

            vcatpvfn_long_double_t fv;
            NV                     nv;

            /* This is evil, but floating point is even more evil */

            /* for SV-style calling, we can only get NV
               for C-style calling, we assume %f is double;
               for simplicity we allow any of %Lf, %llf, %qf for long double
            */
            switch (intsize) {
#if defined(USE_QUADMATH)
            case 'Q':
                break;
#endif
            case 'V':
#if defined(USE_LONG_DOUBLE) || defined(USE_QUADMATH)
                intsize = 'q';
#endif
                break;
/* [perl #20339] - we should accept and ignore %lf rather than die */
            case 'l':
                /* FALLTHROUGH */
            default:
#if defined(USE_LONG_DOUBLE) || defined(USE_QUADMATH)
                intsize = args ? 0 : 'q';
#endif
                break;
            case 'q':
#if defined(HAS_LONG_DOUBLE)
                break;
#else
                /* FALLTHROUGH */
#endif
            case 'c':
            case 'h':
            case 'z':
            case 't':
            case 'j':
                goto unknown;
            }

            /* Now we need (long double) if intsize == 'q', else (double). */
            if (args) {
                /* Note: do not pull NVs off the va_list with va_arg()
                 * (pull doubles instead) because if you have a build
                 * with long doubles, you would always be pulling long
                 * doubles, which would badly break anyone using only
                 * doubles (i.e. the majority of builds). In other
                 * words, you cannot mix doubles and long doubles.
                 * The only case where you can pull off long doubles
                 * is when the format specifier explicitly asks so with
                 * e.g. "%Lg". */
#ifdef USE_QUADMATH
                nv = intsize == 'Q' ? va_arg(*args, NV) :
                    intsize == 'q' ? va_arg(*args, long double) :
                    va_arg(*args, double);
                fv = nv;
#elif LONG_DOUBLESIZE > DOUBLESIZE
                if (intsize == 'q') {
                    fv = va_arg(*args, long double);
                    nv = fv;
                } else {
                    nv = va_arg(*args, double);
                    VCATPVFN_NV_TO_FV(nv, fv);
                }
#else
                nv = va_arg(*args, double);
                fv = nv;
#endif
            }
            else
            {
                SvGETMAGIC(argsv);
                /* we jump here if an int-ish format encountered an
                 * infinite/Nan argsv. After setting nv/fv, it falls
                 * into the isinfnan block which follows */
              handle_infnan_argsv:
                nv = SvNV_nomg(argsv);
                VCATPVFN_NV_TO_FV(nv, fv);
            }

            if (Perl_isinfnan(nv)) {
                if (c == 'c')
                    Perl_croak(aTHX_ "Cannot printf %" NVgf " with '%c'",
                               nv, (int)c);

                elen = S_infnan_2pv(nv, ebuf, sizeof(ebuf), plus);
                assert(elen);
                eptr = ebuf;
                zeros     = 0;
                esignlen  = 0;
                dotstrlen = 0;
                break;
            }

            /* special-case "%.0f" */
            if (   c == 'f'
                && !precis
                && has_precis
                && !(width || left || plus || alt)
                && !fill
                && intsize != 'q'
                && ((eptr = F0convert(nv, ebuf + sizeof ebuf, &elen)))
            )
                goto float_concat;

            /* Determine the buffer size needed for the various
             * floating-point formats.
             *
             * The basic possibilities are:
             *
             *               <---P--->
             *    %f 1111111.123456789
             *    %e       1.111111123e+06
             *    %a     0x1.0f4471f9bp+20
             *    %g        1111111.12
             *    %g        1.11111112e+15
             *
             * where P is the value of the precision in the format, or 6
             * if not specified. Note the two possible output formats of
             * %g; in both cases the number of significant digits is <=
             * precision.
             *
             * For most of the format types the maximum buffer size needed
             * is precision, plus: any leading 1 or 0x1, the radix
             * point, and an exponent.  The difficult one is %f: for a
             * large positive exponent it can have many leading digits,
             * which needs to be calculated specially. Also %a is slightly
             * different in that in the absence of a specified precision,
             * it uses as many digits as necessary to distinguish
             * different values.
             *
             * First, here are the constant bits. For ease of calculation
             * we over-estimate the needed buffer size, for example by
             * assuming all formats have an exponent and a leading 0x1.
             *
             * Also for production use, add a little extra overhead for
             * safety's sake. Under debugging don't, as it means we're
             * more likely to quickly spot issues during development.
             */

            float_need =     1  /* possible unary minus */
                          +  4  /* "0x1" plus very unlikely carry */
                          +  1  /* default radix point '.' */
                          +  2  /* "e-", "p+" etc */
                          +  6  /* exponent: up to 16383 (quad fp) */
#ifndef DEBUGGING
                          + 20  /* safety net */
#endif
                          +  1; /* \0 */


            /* determine the radix point len, e.g. length(".") in "1.2" */
#ifdef USE_LOCALE_NUMERIC
            /* note that we may either explicitly use PL_numeric_radix_sv
             * below, or implicitly, via an snprintf() variant.
             * Note also things like ps_AF.utf8 which has
             * "\N{ARABIC DECIMAL SEPARATOR} as a radix point */
            if (! have_in_lc_numeric) {
                in_lc_numeric = IN_LC(LC_NUMERIC);
                have_in_lc_numeric = TRUE;
            }

            if (in_lc_numeric) {
                WITH_LC_NUMERIC_SET_TO_NEEDED_IN(TRUE, {
                    /* this can't wrap unless PL_numeric_radix_sv is a string
                     * consuming virtually all the 32-bit or 64-bit address
                     * space
                     */
                    float_need += (SvCUR(PL_numeric_radix_sv) - 1);

                    /* floating-point formats only get utf8 if the radix point
                     * is utf8. All other characters in the string are < 128
                     * and so can be safely appended to both a non-utf8 and utf8
                     * string as-is.
                     * Note that this will convert the output to utf8 even if
                     * the radix point didn't get output.
                     */
                    if (SvUTF8(PL_numeric_radix_sv) && !has_utf8) {
                        sv_utf8_upgrade(sv);
                        has_utf8 = TRUE;
                    }
                });
            }
#endif

            hexfp = FALSE;

            if (isALPHA_FOLD_EQ(c, 'f')) {
                /* Determine how many digits before the radix point
                 * might be emitted.  frexp() (or frexpl) has some
                 * unspecified behaviour for nan/inf/-inf, so lucky we've
                 * already handled them above */
                STRLEN digits;
                int i = PERL_INT_MIN;
                (void)Perl_frexp((NV)fv, &i);
                if (i == PERL_INT_MIN)
                    Perl_die(aTHX_ "panic: frexp: %" VCATPVFN_FV_GF, fv);

                if (i > 0) {
                    digits = BIT_DIGITS(i);
                    /* this can't overflow. 'digits' will only be a few
                     * thousand even for the largest floating-point types.
                     * And up until now float_need is just some small
                     * constants plus radix len, which can't be in
                     * overflow territory unless the radix SV is consuming
                     * over 1/2 the address space */
                    assert(float_need < ((STRLEN)~0) - digits);
                    float_need += digits;
                }
            }
            else if (UNLIKELY(isALPHA_FOLD_EQ(c, 'a'))) {
                hexfp = TRUE;
                if (!has_precis) {
                    /* %a in the absence of precision may print as many
                     * digits as needed to represent the entire mantissa
                     * bit pattern.
                     * This estimate seriously overshoots in most cases,
                     * but better the undershooting.  Firstly, all bytes
                     * of the NV are not mantissa, some of them are
                     * exponent.  Secondly, for the reasonably common
                     * long doubles case, the "80-bit extended", two
                     * or six bytes of the NV are unused. Also, we'll
                     * still pick up an extra +6 from the default
                     * precision calculation below. */
                    STRLEN digits =
#ifdef LONGDOUBLE_DOUBLEDOUBLE
                        /* For the "double double", we need more.
                         * Since each double has their own exponent, the
                         * doubles may float (haha) rather far from each
                         * other, and the number of required bits is much
                         * larger, up to total of DOUBLEDOUBLE_MAXBITS bits.
                         * See the definition of DOUBLEDOUBLE_MAXBITS.
                         *
                         * Need 2 hexdigits for each byte. */
                        (DOUBLEDOUBLE_MAXBITS/8 + 1) * 2;
#else
                        NVSIZE * 2; /* 2 hexdigits for each byte */
#endif
                    /* see "this can't overflow" comment above */
                    assert(float_need < ((STRLEN)~0) - digits);
                    float_need += digits;
                }
            }
            /* special-case "%.<number>g" if it will fit in ebuf */
            else if (c == 'g'
                && precis   /* See earlier comment about buggy Gconvert
                               when digits, aka precis, is 0  */
                && has_precis
                /* check that "%.<number>g" formatting will fit in ebuf  */
                && sizeof(ebuf) - float_need > precis
                /* sizeof(ebuf) - float_need will have wrapped if float_need > sizeof(ebuf).     *
                 * Therefore we should check that float_need < sizeof(ebuf). Normally, we would  *
                 * have run this check first, but that triggers incorrect -Wformat-overflow      *
                 * compilation warnings with some versions of gcc if Gconvert invokes sprintf(). *
                 * ( See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=89161 )                   *
                 * So, instead, we check it next:                                                */
                && float_need < sizeof(ebuf)
                && !(width || left || plus || alt)
                && !fill
                && intsize != 'q'
            ) {
                WITH_LC_NUMERIC_SET_TO_NEEDED_IN(in_lc_numeric,
                    SNPRINTF_G(fv, ebuf, sizeof(ebuf), precis)
                );
                elen = strlen(ebuf);
                eptr = ebuf;
                goto float_concat;
            }


            {
                STRLEN pr = has_precis ? precis : 6; /* known default */
                /* this probably can't wrap, since precis is limited
                 * to 1/4 address space size, but better safe than sorry
                 */
                if (float_need >= ((STRLEN)~0) - pr)
                    croak_memory_wrap();
                float_need += pr;
            }

            if (float_need < width)
                float_need = width;

            if (float_need > INT_MAX) {
                /* snprintf() returns an int, and we use that return value,
                   so die horribly if the expected size is too large for int
                */
                Perl_croak(aTHX_ "Numeric format result too large");
            }

            if (PL_efloatsize <= float_need) {
                /* PL_efloatbuf should be at least 1 greater than
                 * float_need to allow a trailing \0 to be returned by
                 * snprintf().  If we need to grow, overgrow for the
                 * benefit of future generations */
                const STRLEN extra = 0x20;
                if (float_need >= ((STRLEN)~0) - extra)
                    croak_memory_wrap();
                float_need += extra;
                Safefree(PL_efloatbuf);
                PL_efloatsize = float_need;
                Newx(PL_efloatbuf, PL_efloatsize, char);
                PL_efloatbuf[0] = '\0';
            }

            if (UNLIKELY(hexfp)) {
                elen = S_format_hexfp(aTHX_ PL_efloatbuf, PL_efloatsize, c,
                                nv, fv, has_precis, precis, width,
                                alt, plus, left, fill, in_lc_numeric);
            }
            else {
                char *ptr = ebuf + sizeof ebuf;
                *--ptr = '\0';
                *--ptr = c;
#if defined(USE_QUADMATH)
                /* always use Q here.  my_snprint() throws an exception if we
                   fallthrough to the double/long double code, even when the
                   format is correct, presumably to avoid any accidentally
                   missing Q.
                */
                *--ptr = 'Q';
                /* FIXME: what to do if HAS_LONG_DOUBLE but not PERL_PRIfldbl? */
#elif defined(HAS_LONG_DOUBLE) && defined(PERL_PRIfldbl)
                /* Note that this is HAS_LONG_DOUBLE and PERL_PRIfldbl,
                 * not USE_LONG_DOUBLE and NVff.  In other words,
                 * this needs to work without USE_LONG_DOUBLE. */
                if (intsize == 'q') {
                    /* Copy the one or more characters in a long double
                     * format before the 'base' ([efgEFG]) character to
                     * the format string. */
                    static char const ldblf[] = PERL_PRIfldbl;
                    char const *p = ldblf + sizeof(ldblf) - 3;
                    while (p >= ldblf) { *--ptr = *p--; }
                }
#endif
                if (has_precis) {
                    base = precis;
                    do { *--ptr = '0' + (base % 10); } while (base /= 10);
                    *--ptr = '.';
                }
                if (width) {
                    base = width;
                    do { *--ptr = '0' + (base % 10); } while (base /= 10);
                }
                if (fill)
                    *--ptr = '0';
                if (left)
                    *--ptr = '-';
                if (plus)
                    *--ptr = plus;
                if (alt)
                    *--ptr = '#';
                *--ptr = '%';

                /* No taint.  Otherwise we are in the strange situation
                 * where printf() taints but print($float) doesn't.
                 * --jhi */

                /* hopefully the above makes ptr a very constrained format
                 * that is safe to use, even though it's not literal */
                GCC_DIAG_IGNORE_STMT(-Wformat-nonliteral);
#ifdef USE_QUADMATH
                {
                    if (!quadmath_format_valid(ptr))
                        Perl_croak_nocontext("panic: quadmath invalid format \"%s\"", ptr);
                    WITH_LC_NUMERIC_SET_TO_NEEDED_IN(in_lc_numeric,
                        elen = quadmath_snprintf(PL_efloatbuf, PL_efloatsize,
                                                 ptr, nv);
                    );
                    if ((IV)elen == -1) {
                        Perl_croak_nocontext("panic: quadmath_snprintf failed, format \"%s\"", ptr);
                    }
                }
#elif defined(HAS_LONG_DOUBLE)
                WITH_LC_NUMERIC_SET_TO_NEEDED_IN(in_lc_numeric,
                    elen = ((intsize == 'q')
                            ? my_snprintf(PL_efloatbuf, PL_efloatsize, ptr, fv)
                            : my_snprintf(PL_efloatbuf, PL_efloatsize, ptr, (double)fv))
                );
#else
                WITH_LC_NUMERIC_SET_TO_NEEDED_IN(in_lc_numeric,
                    elen = my_snprintf(PL_efloatbuf, PL_efloatsize, ptr, fv)
                );
#endif
                GCC_DIAG_RESTORE_STMT;
            }

            eptr = PL_efloatbuf;

          float_concat:

            /* Since floating-point formats do their own formatting and
             * padding, we skip the main block of code at the end of this
             * loop which handles appending eptr to sv, and do our own
             * stripped-down version */

            assert(!zeros);
            assert(!esignlen);
            assert(elen);
            assert(elen >= width);

            S_sv_catpvn_simple(aTHX_ sv, eptr, elen);

            goto done_valid_conversion;
        }

            /* SPECIAL */

        case 'n':
            {
                STRLEN len;
                /* XXX ideally we should warn if any flags etc have been
                 * set, e.g. "%-4.5n" */
                /* XXX if sv was originally non-utf8 with a char in the
                 * range 0x80-0xff, then if it got upgraded, we should
                 * calculate char len rather than byte len here */
                len = SvCUR(sv) - origlen;
                if (args) {
                    int i = (len > PERL_INT_MAX) ? PERL_INT_MAX : (int)len;

                    switch (intsize) {
                    case 'c':  *(va_arg(*args, char*))      = i; break;
                    case 'h':  *(va_arg(*args, short*))     = i; break;
                    default:   *(va_arg(*args, int*))       = i; break;
                    case 'l':  *(va_arg(*args, long*))      = i; break;
                    case 'V':  *(va_arg(*args, IV*))        = i; break;
                    case 'z':  *(va_arg(*args, SSize_t*))   = i; break;
#ifdef HAS_PTRDIFF_T
                    case 't':  *(va_arg(*args, ptrdiff_t*)) = i; break;
#endif
                    case 'j':  *(va_arg(*args, PERL_INTMAX_T*)) = i; break;
                    case 'q':
#if IVSIZE >= 8
                               *(va_arg(*args, Quad_t*))    = i; break;
#else
                               goto unknown;
#endif
                    }
                }
                else {
                    if (arg_missing)
                        Perl_croak_nocontext(
                            "Missing argument for %%n in %s",
                                PL_op ? OP_DESC(PL_op) : "sv_vcatpvfn()");
                    sv_setuv_mg(argsv, has_utf8
                        ? (UV)utf8_length((U8*)SvPVX(sv), (U8*)SvEND(sv))
                        : (UV)len);
                }
                goto done_valid_conversion;
            }

            /* UNKNOWN */

        default:
      unknown:
            if (!args
                && (PL_op->op_type == OP_PRTF || PL_op->op_type == OP_SPRINTF)
                && ckWARN(WARN_PRINTF))
            {
                SV * const msg = sv_newmortal();
                Perl_sv_setpvf(aTHX_ msg, "Invalid conversion in %sprintf: ",
                          (PL_op->op_type == OP_PRTF) ? "" : "s");
                if (fmtstart < patend) {
                    const char * const fmtend = q < patend ? q : patend;
                    const char * f;
                    sv_catpvs(msg, "\"%");
                    for (f = fmtstart; f < fmtend; f++) {
                        if (isPRINT(*f)) {
                            sv_catpvn_nomg(msg, f, 1);
                        } else {
                            Perl_sv_catpvf(aTHX_ msg, "\\%03o", (U8) *f);
                        }
                    }
                    sv_catpvs(msg, "\"");
                } else {
                    sv_catpvs(msg, "end of string");
                }
                Perl_warner(aTHX_ packWARN(WARN_PRINTF), "%" SVf, SVfARG(msg)); /* yes, this is reentrant */
            }

            /* mangled format: output the '%', then continue from the
             * character following that */
            sv_catpvn_nomg(sv, fmtstart-1, 1);
            q = fmtstart;
            svix = osvix;
            /* Any "redundant arg" warning from now onwards will probably
             * just be misleading, so don't bother. */
            no_redundant_warning = TRUE;
            continue;	/* not "break" */
        }

        if (is_utf8 != has_utf8) {
            if (is_utf8) {
                if (SvCUR(sv))
                    sv_utf8_upgrade(sv);
            }
            else {
                const STRLEN old_elen = elen;
                SV * const nsv = newSVpvn_flags(eptr, elen, SVs_TEMP);
                sv_utf8_upgrade(nsv);
                eptr = SvPVX_const(nsv);
                elen = SvCUR(nsv);

                if (width) { /* fudge width (can't fudge elen) */
                    width += elen - old_elen;
                }
                is_utf8 = TRUE;
            }
        }


        /* append esignbuf, filler, zeros, eptr and dotstr to sv */

        {
            STRLEN need, have, gap;
            STRLEN i;
            char *s;

            /* signed value that's wrapped? */
            assert(elen  <= ((~(STRLEN)0) >> 1));

            /* if zeros is non-zero, then it represents filler between
             * elen and precis. So adding elen and zeros together will
             * always be <= precis, and the addition can never wrap */
            assert(!zeros || (precis > elen && precis - elen == zeros));
            have = elen + zeros;

            if (have >= (((STRLEN)~0) - esignlen))
                croak_memory_wrap();
            have += esignlen;

            need = (have > width ? have : width);
            gap = need - have;

            if (need >= (((STRLEN)~0) - (SvCUR(sv) + 1)))
                croak_memory_wrap();
            need += (SvCUR(sv) + 1);

            SvGROW(sv, need);

            s = SvEND(sv);

            if (left) {
                for (i = 0; i < esignlen; i++)
                    *s++ = esignbuf[i];
                for (i = zeros; i; i--)
                    *s++ = '0';
                Copy(eptr, s, elen, char);
                s += elen;
                for (i = gap; i; i--)
                    *s++ = ' ';
            }
            else {
                if (fill) {
                    for (i = 0; i < esignlen; i++)
                        *s++ = esignbuf[i];
                    assert(!zeros);
                    zeros = gap;
                }
                else {
                    for (i = gap; i; i--)
                        *s++ = ' ';
                    for (i = 0; i < esignlen; i++)
                        *s++ = esignbuf[i];
                }

                for (i = zeros; i; i--)
                    *s++ = '0';
                Copy(eptr, s, elen, char);
                s += elen;
            }

            *s = '\0';
            SvCUR_set(sv, s - SvPVX_const(sv));

            if (is_utf8)
                has_utf8 = TRUE;
            if (has_utf8)
                SvUTF8_on(sv);
        }

        if (vectorize && veclen) {
            /* we append the vector separator separately since %v isn't
             * very common: don't slow down the general case by adding
             * dotstrlen to need etc */
            sv_catpvn_nomg(sv, dotstr, dotstrlen);
            esignlen = 0;
            goto vector; /* do next iteration */
        }

      done_valid_conversion:

        if (arg_missing)
            S_warn_vcatpvfn_missing_argument(aTHX);
    }

    /* Now that we've consumed all our printf format arguments (svix)
     * do we have things left on the stack that we didn't use?
     */
    if (!no_redundant_warning && sv_count >= svix + 1 && ckWARN(WARN_REDUNDANT)) {
        Perl_warner(aTHX_ packWARN(WARN_REDUNDANT), "Redundant argument in %s",
                PL_op ? OP_DESC(PL_op) : "sv_vcatpvfn()");
    }

    if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
        /* while we shouldn't set the cache, it may have been previously
           set in the caller, so clear it */
        MAGIC *mg = mg_find(sv, PERL_MAGIC_utf8);
        if (mg)
            magic_setutf8(sv,mg); /* clear UTF8 cache */
    }
    SvTAINT(sv);
}

/* =========================================================================

=for apidoc_section $embedding

=cut

All the macros and functions in this section are for the private use of
the main function, perl_clone().

The foo_dup() functions make an exact copy of an existing foo thingy.
During the course of a cloning, a hash table is used to map old addresses
to new addresses.  The table is created and manipulated with the
ptr_table_* functions.

 * =========================================================================*/


#if defined(USE_ITHREADS)

/* XXX Remove this so it doesn't have to go thru the macro and return for nothing */
#ifndef GpREFCNT_inc
#  define GpREFCNT_inc(gp)	((gp) ? (++(gp)->gp_refcnt, (gp)) : (GP*)NULL)
#endif


#define SAVEPV(p)	((p) ? savepv(p) : NULL)
#define SAVEPVN(p,n)	((p) ? savepvn(p,n) : NULL)

/* clone a parser */

yy_parser *
Perl_parser_dup(pTHX_ const yy_parser *const proto, CLONE_PARAMS *const param)
{
    yy_parser *parser;

    PERL_ARGS_ASSERT_PARSER_DUP;

    if (!proto)
        return NULL;

    /* look for it in the table first */
    parser = (yy_parser *)ptr_table_fetch(PL_ptr_table, proto);
    if (parser)
        return parser;

    /* create anew and remember what it is */
    Newxz(parser, 1, yy_parser);
    ptr_table_store(PL_ptr_table, proto, parser);

    /* XXX eventually, just Copy() most of the parser struct ? */

    parser->lex_brackets = proto->lex_brackets;
    parser->lex_casemods = proto->lex_casemods;
    parser->lex_brackstack = savepvn(proto->lex_brackstack,
                    (proto->lex_brackets < 120 ? 120 : proto->lex_brackets));
    parser->lex_casestack = savepvn(proto->lex_casestack,
                    (proto->lex_casemods < 12 ? 12 : proto->lex_casemods));
    parser->lex_defer	= proto->lex_defer;
    parser->lex_dojoin	= proto->lex_dojoin;
    parser->lex_formbrack = proto->lex_formbrack;
    parser->lex_inpat	= proto->lex_inpat;
    parser->lex_inwhat	= proto->lex_inwhat;
    parser->lex_op	= proto->lex_op;
    parser->lex_repl	= sv_dup_inc(proto->lex_repl, param);
    parser->lex_starts	= proto->lex_starts;
    parser->lex_stuff	= sv_dup_inc(proto->lex_stuff, param);
    parser->multi_close	= proto->multi_close;
    parser->multi_open	= proto->multi_open;
    parser->multi_start	= proto->multi_start;
    parser->multi_end	= proto->multi_end;
    parser->preambled	= proto->preambled;
    parser->lex_super_state = proto->lex_super_state;
    parser->lex_sub_inwhat  = proto->lex_sub_inwhat;
    parser->lex_sub_op	= proto->lex_sub_op;
    parser->lex_sub_repl= sv_dup_inc(proto->lex_sub_repl, param);
    parser->linestr	= sv_dup_inc(proto->linestr, param);
    parser->expect	= proto->expect;
    parser->copline	= proto->copline;
    parser->last_lop_op	= proto->last_lop_op;
    parser->lex_state	= proto->lex_state;
    parser->rsfp	= fp_dup(proto->rsfp, '<', param);
    /* rsfp_filters entries have fake IoDIRP() */
    parser->rsfp_filters= av_dup_inc(proto->rsfp_filters, param);
    parser->in_my	= proto->in_my;
    parser->in_my_stash	= hv_dup(proto->in_my_stash, param);
    parser->error_count	= proto->error_count;
    parser->sig_elems	= proto->sig_elems;
    parser->sig_optelems= proto->sig_optelems;
    parser->sig_slurpy  = proto->sig_slurpy;
    parser->recheck_utf8_validity = proto->recheck_utf8_validity;

    {
        char * const ols = SvPVX(proto->linestr);
        char * const ls  = SvPVX(parser->linestr);

        parser->bufptr	    = ls + (proto->bufptr >= ols ?
                                    proto->bufptr -  ols : 0);
        parser->oldbufptr   = ls + (proto->oldbufptr >= ols ?
                                    proto->oldbufptr -  ols : 0);
        parser->oldoldbufptr= ls + (proto->oldoldbufptr >= ols ?
                                    proto->oldoldbufptr -  ols : 0);
        parser->linestart   = ls + (proto->linestart >= ols ?
                                    proto->linestart -  ols : 0);
        parser->last_uni    = ls + (proto->last_uni >= ols ?
                                    proto->last_uni -  ols : 0);
        parser->last_lop    = ls + (proto->last_lop >= ols ?
                                    proto->last_lop -  ols : 0);

        parser->bufend	    = ls + SvCUR(parser->linestr);
    }

    Copy(proto->tokenbuf, parser->tokenbuf, 256, char);


    Copy(proto->nextval, parser->nextval, 5, YYSTYPE);
    Copy(proto->nexttype, parser->nexttype, 5,	I32);
    parser->nexttoke	= proto->nexttoke;

    /* XXX should clone saved_curcop here, but we aren't passed
     * proto_perl; so do it in perl_clone_using instead */

    return parser;
}

/*
=for apidoc_section $io
=for apidoc fp_dup

Duplicate a file handle, returning a pointer to the cloned object.

=cut
*/

PerlIO *
Perl_fp_dup(pTHX_ PerlIO *const fp, const char type, CLONE_PARAMS *const param)
{
    PerlIO *ret;

    PERL_ARGS_ASSERT_FP_DUP;
    PERL_UNUSED_ARG(type);

    if (!fp)
        return (PerlIO*)NULL;

    /* look for it in the table first */
    ret = (PerlIO*)ptr_table_fetch(PL_ptr_table, fp);
    if (ret)
        return ret;

    /* create anew and remember what it is */
#ifdef __amigaos4__
    ret = PerlIO_fdupopen(aTHX_ fp, param, PERLIO_DUP_CLONE|PERLIO_DUP_FD);
#else
    ret = PerlIO_fdupopen(aTHX_ fp, param, PERLIO_DUP_CLONE);
#endif
    ptr_table_store(PL_ptr_table, fp, ret);
    return ret;
}

/*
=for apidoc_section $io
=for apidoc dirp_dup

Duplicate a directory handle, returning a pointer to the cloned object.

=cut
*/

DIR *
Perl_dirp_dup(pTHX_ DIR *const dp, CLONE_PARAMS *const param)
{
    DIR *ret;

#if defined(HAS_FCHDIR) && defined(HAS_TELLDIR) && defined(HAS_SEEKDIR)
    DIR *pwd;
    const Direntry_t *dirent;
    char smallbuf[256]; /* XXX MAXPATHLEN, surely? */
    char *name = NULL;
    STRLEN len = 0;
    long pos;
#endif

    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_DIRP_DUP;

    if (!dp)
        return (DIR*)NULL;

    /* look for it in the table first */
    ret = (DIR*)ptr_table_fetch(PL_ptr_table, dp);
    if (ret)
        return ret;

#if defined(HAS_FCHDIR) && defined(HAS_TELLDIR) && defined(HAS_SEEKDIR)

    PERL_UNUSED_ARG(param);

    /* create anew */

    /* open the current directory (so we can switch back) */
    if (!(pwd = PerlDir_open("."))) return (DIR *)NULL;

    /* chdir to our dir handle and open the present working directory */
    if (fchdir(my_dirfd(dp)) < 0 || !(ret = PerlDir_open("."))) {
        PerlDir_close(pwd);
        return (DIR *)NULL;
    }
    /* Now we should have two dir handles pointing to the same dir. */

    /* Be nice to the calling code and chdir back to where we were. */
    /* XXX If this fails, then what? */
    PERL_UNUSED_RESULT(fchdir(my_dirfd(pwd)));

    /* We have no need of the pwd handle any more. */
    PerlDir_close(pwd);

#ifdef DIRNAMLEN
# define d_namlen(d) (d)->d_namlen
#else
# define d_namlen(d) strlen((d)->d_name)
#endif
    /* Iterate once through dp, to get the file name at the current posi-
       tion. Then step back. */
    pos = PerlDir_tell(dp);
    if ((dirent = PerlDir_read(dp))) {
        len = d_namlen(dirent);
        if (len > sizeof(dirent->d_name) && sizeof(dirent->d_name) > PTRSIZE) {
            /* If the len is somehow magically longer than the
             * maximum length of the directory entry, even though
             * we could fit it in a buffer, we could not copy it
             * from the dirent.  Bail out. */
            PerlDir_close(ret);
            return (DIR*)NULL;
        }
        if (len <= sizeof smallbuf) name = smallbuf;
        else Newx(name, len, char);
        Move(dirent->d_name, name, len, char);
    }
    PerlDir_seek(dp, pos);

    /* Iterate through the new dir handle, till we find a file with the
       right name. */
    if (!dirent) /* just before the end */
        for(;;) {
            pos = PerlDir_tell(ret);
            if (PerlDir_read(ret)) continue; /* not there yet */
            PerlDir_seek(ret, pos); /* step back */
            break;
        }
    else {
        const long pos0 = PerlDir_tell(ret);
        for(;;) {
            pos = PerlDir_tell(ret);
            if ((dirent = PerlDir_read(ret))) {
                if (len == (STRLEN)d_namlen(dirent)
                    && memEQ(name, dirent->d_name, len)) {
                    /* found it */
                    PerlDir_seek(ret, pos); /* step back */
                    break;
                }
                /* else we are not there yet; keep iterating */
            }
            else { /* This is not meant to happen. The best we can do is
                      reset the iterator to the beginning. */
                PerlDir_seek(ret, pos0);
                break;
            }
        }
    }
#undef d_namlen

    if (name && name != smallbuf)
        Safefree(name);
#endif

#ifdef WIN32
    ret = win32_dirp_dup(dp, param);
#endif

    /* pop it in the pointer table */
    if (ret)
        ptr_table_store(PL_ptr_table, dp, ret);

    return ret;
}

/*
=for apidoc_section $GV
=for apidoc gp_dup

Duplicate a typeglob, returning a pointer to the cloned object.

=cut
*/

GP *
Perl_gp_dup(pTHX_ GP *const gp, CLONE_PARAMS *const param)
{
    GP *ret;

    PERL_ARGS_ASSERT_GP_DUP;

    if (!gp)
        return (GP*)NULL;
    /* look for it in the table first */
    ret = (GP*)ptr_table_fetch(PL_ptr_table, gp);
    if (ret)
        return ret;

    /* create anew and remember what it is */
    Newxz(ret, 1, GP);
    ptr_table_store(PL_ptr_table, gp, ret);

    /* clone */
    /* ret->gp_refcnt must be 0 before any other dups are called. We're relying
       on Newxz() to do this for us.  */
    ret->gp_sv		= sv_dup_inc(gp->gp_sv, param);
    ret->gp_io		= io_dup_inc(gp->gp_io, param);
    ret->gp_form	= cv_dup_inc(gp->gp_form, param);
    ret->gp_av		= av_dup_inc(gp->gp_av, param);
    ret->gp_hv		= hv_dup_inc(gp->gp_hv, param);
    ret->gp_egv	= gv_dup(gp->gp_egv, param);/* GvEGV is not refcounted */
    ret->gp_cv		= cv_dup_inc(gp->gp_cv, param);
    ret->gp_cvgen	= gp->gp_cvgen;
    ret->gp_line	= gp->gp_line;
    ret->gp_file_hek	= hek_dup(gp->gp_file_hek, param);
    return ret;
}


/*
=for apidoc_section $magic
=for apidoc mg_dup

Duplicate a chain of magic, returning a pointer to the cloned object.

=cut
*/

MAGIC *
Perl_mg_dup(pTHX_ MAGIC *mg, CLONE_PARAMS *const param)
{
    MAGIC *mgret = NULL;
    MAGIC **mgprev_p = &mgret;

    PERL_ARGS_ASSERT_MG_DUP;

    for (; mg; mg = mg->mg_moremagic) {
        MAGIC *nmg;

        if ((param->flags & CLONEf_JOIN_IN)
                && mg->mg_type == PERL_MAGIC_backref)
            /* when joining, we let the individual SVs add themselves to
             * backref as needed. */
            continue;

        Newx(nmg, 1, MAGIC);
        *mgprev_p = nmg;
        mgprev_p = &(nmg->mg_moremagic);

        /* There was a comment "XXX copy dynamic vtable?" but as we don't have
           dynamic vtables, I'm not sure why Sarathy wrote it. The comment dates
           from the original commit adding Perl_mg_dup() - revision 4538.
           Similarly there is the annotation "XXX random ptr?" next to the
           assignment to nmg->mg_ptr.  */
        *nmg = *mg;

        /* FIXME for plugins
        if (nmg->mg_type == PERL_MAGIC_qr) {
            nmg->mg_obj	= MUTABLE_SV(CALLREGDUPE((REGEXP*)nmg->mg_obj, param));
        }
        else
        */
        nmg->mg_obj = (nmg->mg_flags & MGf_REFCOUNTED)
                          ? nmg->mg_type == PERL_MAGIC_backref
                                /* The backref AV has its reference
                                 * count deliberately bumped by 1 */
                                ? SvREFCNT_inc(av_dup_inc((const AV *)
                                                    nmg->mg_obj, param))
                                : sv_dup_inc(nmg->mg_obj, param)
                          : (nmg->mg_type == PERL_MAGIC_regdatum ||
                             nmg->mg_type == PERL_MAGIC_regdata)
                                  ? nmg->mg_obj
                                  : sv_dup(nmg->mg_obj, param);

        if (nmg->mg_ptr && nmg->mg_type != PERL_MAGIC_regex_global) {
            if (nmg->mg_len > 0) {
                nmg->mg_ptr	= SAVEPVN(nmg->mg_ptr, nmg->mg_len);
                if (nmg->mg_type == PERL_MAGIC_overload_table &&
                        AMT_AMAGIC((AMT*)nmg->mg_ptr))
                {
                    AMT * const namtp = (AMT*)nmg->mg_ptr;
                    sv_dup_inc_multiple((SV**)(namtp->table),
                                        (SV**)(namtp->table), NofAMmeth, param);
                }
            }
            else if (nmg->mg_len == HEf_SVKEY)
                nmg->mg_ptr = (char*)sv_dup_inc((const SV *)nmg->mg_ptr, param);
        }
        if ((nmg->mg_flags & MGf_DUP) && nmg->mg_virtual && nmg->mg_virtual->svt_dup) {
            nmg->mg_virtual->svt_dup(aTHX_ nmg, param);
        }
    }
    return mgret;
}

#endif /* USE_ITHREADS */

struct ptr_tbl_arena {
    struct ptr_tbl_arena *next;
    struct ptr_tbl_ent array[1023/3]; /* as ptr_tbl_ent has 3 pointers.  */
};

/*
=for apidoc_section $embedding
=for apidoc ptr_table_new

Create a new pointer-mapping table

=cut
*/

PTR_TBL_t *
Perl_ptr_table_new(pTHX)
{
    PTR_TBL_t *tbl;
    PERL_UNUSED_CONTEXT;

    Newx(tbl, 1, PTR_TBL_t);
    tbl->tbl_max	= 511;
    tbl->tbl_items	= 0;
    tbl->tbl_arena	= NULL;
    tbl->tbl_arena_next	= NULL;
    tbl->tbl_arena_end	= NULL;
    Newxz(tbl->tbl_ary, tbl->tbl_max + 1, PTR_TBL_ENT_t*);
    return tbl;
}

#define PTR_TABLE_HASH(ptr) \
  ((PTR2UV(ptr) >> 3) ^ (PTR2UV(ptr) >> (3 + 7)) ^ (PTR2UV(ptr) >> (3 + 17)))

/* map an existing pointer using a table */

STATIC PTR_TBL_ENT_t *
S_ptr_table_find(PTR_TBL_t *const tbl, const void *const sv)
{
    PTR_TBL_ENT_t *tblent;
    const UV hash = PTR_TABLE_HASH(sv);

    PERL_ARGS_ASSERT_PTR_TABLE_FIND;

    tblent = tbl->tbl_ary[hash & tbl->tbl_max];
    for (; tblent; tblent = tblent->next) {
        if (tblent->oldval == sv)
            return tblent;
    }
    return NULL;
}

/*
=for apidoc ptr_table_fetch

Look for C<sv> in the pointer-mapping table C<tbl>, returning its value, or
NULL if not found.

=cut
*/

void *
Perl_ptr_table_fetch(pTHX_ PTR_TBL_t *const tbl, const void *const sv)
{
    PTR_TBL_ENT_t const *const tblent = ptr_table_find(tbl, sv);

    PERL_ARGS_ASSERT_PTR_TABLE_FETCH;
    PERL_UNUSED_CONTEXT;

    return tblent ? tblent->newval : NULL;
}

/*
=for apidoc ptr_table_store

Add a new entry to a pointer-mapping table C<tbl>.
In hash terms, C<oldsv> is the key; Cnewsv> is the value.

The names "old" and "new" are specific to the core's typical use of ptr_tables
in thread cloning.

=cut
*/

void
Perl_ptr_table_store(pTHX_ PTR_TBL_t *const tbl, const void *const oldsv, void *const newsv)
{
    PTR_TBL_ENT_t *tblent = ptr_table_find(tbl, oldsv);

    PERL_ARGS_ASSERT_PTR_TABLE_STORE;
    PERL_UNUSED_CONTEXT;

    if (tblent) {
        tblent->newval = newsv;
    } else {
        const UV entry = PTR_TABLE_HASH(oldsv) & tbl->tbl_max;

        if (tbl->tbl_arena_next == tbl->tbl_arena_end) {
            struct ptr_tbl_arena *new_arena;

            Newx(new_arena, 1, struct ptr_tbl_arena);
            new_arena->next = tbl->tbl_arena;
            tbl->tbl_arena = new_arena;
            tbl->tbl_arena_next = new_arena->array;
            tbl->tbl_arena_end = C_ARRAY_END(new_arena->array);
        }

        tblent = tbl->tbl_arena_next++;

        tblent->oldval = oldsv;
        tblent->newval = newsv;
        tblent->next = tbl->tbl_ary[entry];
        tbl->tbl_ary[entry] = tblent;
        tbl->tbl_items++;
        if (tblent->next && tbl->tbl_items > tbl->tbl_max)
            ptr_table_split(tbl);
    }
}

/*
=for apidoc ptr_table_split

Double the hash bucket size of an existing ptr table

=cut
*/

void
Perl_ptr_table_split(pTHX_ PTR_TBL_t *const tbl)
{
    PTR_TBL_ENT_t **ary = tbl->tbl_ary;
    const UV oldsize = tbl->tbl_max + 1;
    UV newsize = oldsize * 2;
    UV i;

    PERL_ARGS_ASSERT_PTR_TABLE_SPLIT;
    PERL_UNUSED_CONTEXT;

    Renew(ary, newsize, PTR_TBL_ENT_t*);
    Zero(&ary[oldsize], newsize-oldsize, PTR_TBL_ENT_t*);
    tbl->tbl_max = --newsize;
    tbl->tbl_ary = ary;
    for (i=0; i < oldsize; i++, ary++) {
        PTR_TBL_ENT_t **entp = ary;
        PTR_TBL_ENT_t *ent = *ary;
        PTR_TBL_ENT_t **curentp;
        if (!ent)
            continue;
        curentp = ary + oldsize;
        do {
            if ((newsize & PTR_TABLE_HASH(ent->oldval)) != i) {
                *entp = ent->next;
                ent->next = *curentp;
                *curentp = ent;
            }
            else
                entp = &ent->next;
            ent = *entp;
        } while (ent);
    }
}

/*
=for apidoc ptr_table_free

Clear and free a ptr table

=cut
*/

void
Perl_ptr_table_free(pTHX_ PTR_TBL_t *const tbl)
{
    struct ptr_tbl_arena *arena;

    PERL_UNUSED_CONTEXT;

    if (!tbl) {
        return;
    }

    arena = tbl->tbl_arena;

    while (arena) {
        struct ptr_tbl_arena *next = arena->next;

        Safefree(arena);
        arena = next;
    }

    Safefree(tbl->tbl_ary);
    Safefree(tbl);
}

#if defined(USE_ITHREADS)

void
Perl_rvpv_dup(pTHX_ SV *const dsv, const SV *const ssv, CLONE_PARAMS *const param)
{
    PERL_ARGS_ASSERT_RVPV_DUP;

    assert(!isREGEXP(ssv));
    if (SvROK(ssv)) {
        if (SvWEAKREF(ssv)) {
            SvRV_set(dsv, sv_dup(SvRV_const(ssv), param));
            if (param->flags & CLONEf_JOIN_IN) {
                /* if joining, we add any back references individually rather
                 * than copying the whole backref array */
                Perl_sv_add_backref(aTHX_ SvRV(dsv), dsv);
            }
        }
        else
            SvRV_set(dsv, sv_dup_inc(SvRV_const(ssv), param));
    }
    else if (SvPVX_const(ssv)) {
        /* Has something there */
        if (SvLEN(ssv)) {
            /* Normal PV - clone whole allocated space */
            SvPV_set(dsv, SAVEPVN(SvPVX_const(ssv), SvLEN(ssv)-1));
            /* ssv may not be that normal, but actually copy on write.
               But we are a true, independent SV, so:  */
            SvIsCOW_off(dsv);
        }
        else {
            /* Special case - not normally malloced for some reason */
            if (isGV_with_GP(ssv)) {
                /* Don't need to do anything here.  */
            }
            else if ((SvIsCOW_shared_hash(ssv))) {
                /* A "shared" PV - clone it as "shared" PV */
                SvPV_set(dsv,
                         HEK_KEY(hek_dup(SvSHARED_HEK_FROM_PV(SvPVX_const(ssv)),
                                         param)));
            }
            else {
                /* Some other special case - random pointer */
                SvPV_set(dsv, (char *) SvPVX_const(ssv));
            }
        }
    }
    else {
        /* Copy the NULL */
        SvPV_set(dsv, NULL);
    }
}

/* duplicate a list of SVs. source and dest may point to the same memory.  */
static SV **
S_sv_dup_inc_multiple(pTHX_ SV *const *source, SV **dest,
                      SSize_t items, CLONE_PARAMS *const param)
{
    PERL_ARGS_ASSERT_SV_DUP_INC_MULTIPLE;

    while (items-- > 0) {
        *dest++ = sv_dup_inc(*source++, param);
    }

    return dest;
}

/* duplicate the HvAUX of an HV */
static void
S_sv_dup_hvaux(pTHX_ const SV *const ssv, SV *dsv, CLONE_PARAMS *const param)
{
    PERL_ARGS_ASSERT_SV_DUP_HVAUX;

    const struct xpvhv_aux * const saux = HvAUX(ssv);
    struct xpvhv_aux * const daux = HvAUX(dsv);
    /* This flag isn't copied.  */
    SvFLAGS(dsv) |= SVphv_HasAUX;

    if (saux->xhv_name_count) {
        HEK ** const sname = saux->xhv_name_u.xhvnameu_names;
        const I32 count = saux->xhv_name_count < 0
            ? -saux->xhv_name_count
            :  saux->xhv_name_count;
        HEK **shekp = sname + count;
        HEK **dhekp;
        Newx(daux->xhv_name_u.xhvnameu_names, count, HEK *);
        dhekp = daux->xhv_name_u.xhvnameu_names + count;
        while (shekp-- > sname) {
            dhekp--;
            *dhekp = hek_dup(*shekp, param);
        }
    }
    else {
        daux->xhv_name_u.xhvnameu_name = hek_dup(saux->xhv_name_u.xhvnameu_name, param);
    }
    daux->xhv_name_count = saux->xhv_name_count;

    daux->xhv_aux_flags = saux->xhv_aux_flags;
#ifdef PERL_HASH_RANDOMIZE_KEYS
    daux->xhv_rand = saux->xhv_rand;
    daux->xhv_last_rand = saux->xhv_last_rand;
#endif
    daux->xhv_riter = saux->xhv_riter;
    daux->xhv_eiter = saux->xhv_eiter ? he_dup(saux->xhv_eiter, FALSE, param) : 0;
    /* backref array needs refcnt=2; see sv_add_backref */
    daux->xhv_backreferences =
        (param->flags & CLONEf_JOIN_IN)
            /* when joining, we let the individual GVs and
             * CVs add themselves to backref as
             * needed. This avoids pulling in stuff
             * that isn't required, and simplifies the
             * case where stashes aren't cloned back
             * if they already exist in the parent
             * thread */
        ? NULL
        : saux->xhv_backreferences
            ? (SvTYPE(saux->xhv_backreferences) == SVt_PVAV)
                ? MUTABLE_AV(SvREFCNT_inc(
                      sv_dup_inc((const SV *)
                        saux->xhv_backreferences, param)))
                : MUTABLE_AV(sv_dup((const SV *)
                        saux->xhv_backreferences, param))
            : 0;

    daux->xhv_mro_meta = saux->xhv_mro_meta
        ? mro_meta_dup(saux->xhv_mro_meta, param)
        : 0;

    /* Record stashes for possible cloning in Perl_clone(). */
    if (HvNAME(ssv))
        av_push(param->stashes, dsv);

    if (HvSTASH_IS_CLASS(ssv)) {
        daux->xhv_class_superclass    = hv_dup_inc(saux->xhv_class_superclass,    param);
        daux->xhv_class_initfields_cv = cv_dup_inc(saux->xhv_class_initfields_cv, param);
        daux->xhv_class_adjust_blocks = av_dup_inc(saux->xhv_class_adjust_blocks, param);
        daux->xhv_class_fields        = padnamelist_dup_inc(saux->xhv_class_fields, param);
        daux->xhv_class_next_fieldix  = saux->xhv_class_next_fieldix;
        daux->xhv_class_param_map     = hv_dup_inc(saux->xhv_class_param_map,     param);

        /* TODO: This does mean that we can't compile more `field` expressions
         * in the cloned thread, but surely we're done with compiletime now..?
         */
        daux->xhv_class_suspended_initfields_compcv = NULL;
    }
}

/* duplicate an SV of any type (including AV, HV etc) */

static SV *
S_sv_dup_common(pTHX_ const SV *const ssv, CLONE_PARAMS *const param)
{
    SV *dsv;

    PERL_ARGS_ASSERT_SV_DUP_COMMON;

    if (SvIS_FREED(ssv)) {
#ifdef DEBUG_LEAKING_SCALARS_ABORT
        abort();
#endif
        return NULL;
    }
    /* look for it in the table first */
    dsv = MUTABLE_SV(ptr_table_fetch(PL_ptr_table, ssv));
    if (dsv)
        return dsv;

    if(param->flags & CLONEf_JOIN_IN) {
        /** We are joining here so we don't want do clone
            something that is bad **/
        if (SvTYPE(ssv) == SVt_PVHV) {
            const HEK * const hvname = HvNAME_HEK(ssv);
            if (hvname) {
                /** don't clone stashes if they already exist **/
                dsv = MUTABLE_SV(gv_stashpvn(HEK_KEY(hvname), HEK_LEN(hvname),
                                                HEK_UTF8(hvname) ? SVf_UTF8 : 0));
                ptr_table_store(PL_ptr_table, ssv, dsv);
                return dsv;
            }
        }
        else if (SvTYPE(ssv) == SVt_PVGV && !SvFAKE(ssv)) {
            HV *stash = GvSTASH(ssv);
            const HEK * hvname;
            if (stash && (hvname = HvNAME_HEK(stash))) {
                /** don't clone GVs if they already exist **/
                SV **svp;
                stash = gv_stashpvn(HEK_KEY(hvname), HEK_LEN(hvname),
                                    HEK_UTF8(hvname) ? SVf_UTF8 : 0);
                svp = hv_fetch(
                        stash, GvNAME(ssv),
                        GvNAMEUTF8(ssv)
                            ? -GvNAMELEN(ssv)
                            :  GvNAMELEN(ssv),
                        0
                      );
                if (svp && *svp && SvTYPE(*svp) == SVt_PVGV) {
                    ptr_table_store(PL_ptr_table, ssv, *svp);
                    return *svp;
                }
            }
        }
    }

    /* create anew and remember what it is */
    new_SV(dsv);

#ifdef DEBUG_LEAKING_SCALARS
    dsv->sv_debug_optype = ssv->sv_debug_optype;
    dsv->sv_debug_line = ssv->sv_debug_line;
    dsv->sv_debug_inpad = ssv->sv_debug_inpad;
    dsv->sv_debug_parent = (SV*)ssv;
    FREE_SV_DEBUG_FILE(dsv);
    dsv->sv_debug_file = savesharedpv(ssv->sv_debug_file);
#endif

    ptr_table_store(PL_ptr_table, ssv, dsv);

    /* clone */
    SvFLAGS(dsv)	= SvFLAGS(ssv);
    SvFLAGS(dsv)	&= ~SVf_OOK;		/* don't propagate OOK hack */
    SvREFCNT(dsv)	= 0;			/* must be before any other dups! */

#ifdef DEBUGGING
    if (SvANY(ssv) && PL_watch_pvx && SvPVX_const(ssv) == PL_watch_pvx)
        PerlIO_printf(Perl_debug_log, "watch at %p hit, found string \"%s\"\n",
                      (void*)PL_watch_pvx, SvPVX_const(ssv));
#endif

    /* don't clone objects whose class has asked us not to */
    if (SvOBJECT(ssv)
     && ! (SvFLAGS(SvSTASH(ssv)) & SVphv_CLONEABLE))
    {
        SvFLAGS(dsv) = 0;
        return dsv;
    }

    switch (SvTYPE(ssv)) {
    case SVt_NULL:
        SvANY(dsv)	= NULL;
        break;
    case SVt_IV:
        SET_SVANY_FOR_BODYLESS_IV(dsv);
        if(SvROK(ssv)) {
            Perl_rvpv_dup(aTHX_ dsv, ssv, param);
        } else {
            SvIV_set(dsv, SvIVX(ssv));
        }
        break;
    case SVt_NV:
#if NVSIZE <= IVSIZE
        SET_SVANY_FOR_BODYLESS_NV(dsv);
#else
        SvANY(dsv)	= new_XNV();
#endif
        SvNV_set(dsv, SvNVX(ssv));
        break;
    default:
        {
            /* These are all the types that need complex bodies allocating.  */
            void *new_body;
            const svtype sv_type = SvTYPE(ssv);
            const struct body_details *sv_type_details
                = bodies_by_type + sv_type;

            switch (sv_type) {
            default:
                Perl_croak(param->proto_perl, "Bizarre SvTYPE [%" IVdf "]", (IV)SvTYPE(ssv));
                NOT_REACHED; /* NOTREACHED */
                break;

            case SVt_PVHV:
                if (HvHasAUX(ssv)) {
                    sv_type_details = &fake_hv_with_aux;
#ifdef PURIFY
                    new_body = new_NOARENA(sv_type_details);
#else
                    new_body_from_arena(new_body, HVAUX_ARENA_ROOT_IX, fake_hv_with_aux);
#endif
                    goto have_body;
                }
                /* FALLTHROUGH */
            case SVt_PVOBJ:
            case SVt_PVGV:
            case SVt_PVIO:
            case SVt_PVFM:
            case SVt_PVAV:
            case SVt_PVCV:
            case SVt_PVLV:
            case SVt_REGEXP:
            case SVt_PVMG:
            case SVt_PVNV:
            case SVt_PVIV:
            case SVt_INVLIST:
            case SVt_PV:
                assert(sv_type_details->body_size);
#ifndef PURIFY
                if (sv_type_details->arena) {
                    new_body = S_new_body(aTHX_ sv_type);
                    new_body
                        = (void*)((char*)new_body - sv_type_details->offset);
                } else
#endif
                {
                    new_body = new_NOARENA(sv_type_details);
                }
            }
        have_body:
            assert(new_body);
            SvANY(dsv) = new_body;

#ifndef PURIFY
            Copy(((char*)SvANY(ssv)) + sv_type_details->offset,
                 ((char*)SvANY(dsv)) + sv_type_details->offset,
                 sv_type_details->copy, char);
#else
            Copy(((char*)SvANY(ssv)),
                 ((char*)SvANY(dsv)),
                 sv_type_details->body_size + sv_type_details->offset, char);
#endif

            if (sv_type != SVt_PVAV && sv_type != SVt_PVHV && sv_type != SVt_PVOBJ
                && !isGV_with_GP(dsv)
                && !isREGEXP(dsv)
                && !(sv_type == SVt_PVIO && !(IoFLAGS(dsv) & IOf_FAKE_DIRP)))
                Perl_rvpv_dup(aTHX_ dsv, ssv, param);

            /* The Copy above means that all the source (unduplicated) pointers
               are now in the destination.  We can check the flags and the
               pointers in either, but it's possible that there's less cache
               missing by always going for the destination.
               FIXME - instrument and check that assumption  */
            if (sv_type >= SVt_PVMG) {
                if (SvMAGIC(dsv))
                    SvMAGIC_set(dsv, mg_dup(SvMAGIC(dsv), param));
                if (SvOBJECT(dsv) && SvSTASH(dsv))
                    SvSTASH_set(dsv, hv_dup_inc(SvSTASH(dsv), param));
                else SvSTASH_set(dsv, 0); /* don't copy DESTROY cache */
            }

            /* The cast silences a GCC warning about unhandled types.  */
            switch ((int)sv_type) {
            case SVt_PV:
                break;
            case SVt_PVIV:
                break;
            case SVt_PVNV:
                break;
            case SVt_PVMG:
                break;
            case SVt_REGEXP:
              duprex:
                /* FIXME for plugins */
                re_dup_guts((REGEXP*) ssv, (REGEXP*) dsv, param);
                break;
            case SVt_PVLV:
                /* XXX LvTARGOFF sometimes holds PMOP* when DEBUGGING */
                if (LvTYPE(dsv) == 't') /* for tie: unrefcnted fake (SV**) */
                    LvTARG(dsv) = dsv;
                else if (LvTYPE(dsv) == 'T') /* for tie: fake HE */
                    LvTARG(dsv) = MUTABLE_SV(he_dup((HE*)LvTARG(dsv), FALSE, param));
                else
                    LvTARG(dsv) = sv_dup_inc(LvTARG(dsv), param);
                if (isREGEXP(ssv)) goto duprex;
                /* FALLTHROUGH */
            case SVt_PVGV:
                /* non-GP case already handled above */
                if(isGV_with_GP(ssv)) {
                    GvNAME_HEK(dsv) = hek_dup(GvNAME_HEK(dsv), param);
                    /* Don't call sv_add_backref here as it's going to be
                       created as part of the magic cloning of the symbol
                       table--unless this is during a join and the stash
                       is not actually being cloned.  */
                    /* Danger Will Robinson - GvGP(dsv) isn't initialised
                       at the point of this comment.  */
                    GvSTASH(dsv) = hv_dup(GvSTASH(dsv), param);
                    if (param->flags & CLONEf_JOIN_IN)
                        Perl_sv_add_backref(aTHX_ MUTABLE_SV(GvSTASH(dsv)), dsv);
                    GvGP_set(dsv, gp_dup(GvGP(ssv), param));
                    (void)GpREFCNT_inc(GvGP(dsv));
                }
                break;
            case SVt_PVIO:
                /* PL_parser->rsfp_filters entries have fake IoDIRP() */
                if(IoFLAGS(dsv) & IOf_FAKE_DIRP) {
                    /* I have no idea why fake dirp (rsfps)
                       should be treated differently but otherwise
                       we end up with leaks -- sky*/
                    IoTOP_GV(dsv)      = gv_dup_inc(IoTOP_GV(dsv), param);
                    IoFMT_GV(dsv)      = gv_dup_inc(IoFMT_GV(dsv), param);
                    IoBOTTOM_GV(dsv)   = gv_dup_inc(IoBOTTOM_GV(dsv), param);
                } else {
                    IoTOP_GV(dsv)      = gv_dup(IoTOP_GV(dsv), param);
                    IoFMT_GV(dsv)      = gv_dup(IoFMT_GV(dsv), param);
                    IoBOTTOM_GV(dsv)   = gv_dup(IoBOTTOM_GV(dsv), param);
                    if (IoDIRP(dsv)) {
                        IoDIRP(dsv)	= dirp_dup(IoDIRP(dsv), param);
                    } else {
                        NOOP;
                        /* IoDIRP(dsv) is already a copy of IoDIRP(ssv)  */
                    }
                    IoIFP(dsv)	= fp_dup(IoIFP(ssv), IoTYPE(dsv), param);
                }
                if (IoOFP(dsv) == IoIFP(ssv))
                    IoOFP(dsv) = IoIFP(dsv);
                else
                    IoOFP(dsv)	= fp_dup(IoOFP(dsv), IoTYPE(dsv), param);
                IoTOP_NAME(dsv)	= SAVEPV(IoTOP_NAME(dsv));
                IoFMT_NAME(dsv)	= SAVEPV(IoFMT_NAME(dsv));
                IoBOTTOM_NAME(dsv)	= SAVEPV(IoBOTTOM_NAME(dsv));
                break;
            case SVt_PVAV:
                /* avoid cloning an empty array */
                if (AvARRAY((const AV *)ssv) && AvFILLp((const AV *)ssv) >= 0) {
                    SV **dst_ary, **src_ary;
                    SSize_t items = AvFILLp((const AV *)ssv) + 1;

                    src_ary = AvARRAY((const AV *)ssv);
                    Newx(dst_ary, AvMAX((const AV *)ssv)+1, SV*);
                    ptr_table_store(PL_ptr_table, src_ary, dst_ary);
                    AvARRAY(MUTABLE_AV(dsv)) = dst_ary;
                    AvALLOC((const AV *)dsv) = dst_ary;
                    if (AvREAL((const AV *)ssv)) {
                        dst_ary = sv_dup_inc_multiple(src_ary, dst_ary, items,
                                                      param);
                    }
                    else {
                        while (items-- > 0)
                            *dst_ary++ = sv_dup(*src_ary++, param);
                    }
                    items = AvMAX((const AV *)ssv) - AvFILLp((const AV *)ssv);
                    while (items-- > 0) {
                        *dst_ary++ = NULL;
                    }
                }
                else {
                    AvARRAY(MUTABLE_AV(dsv))	= NULL;
                    AvALLOC((const AV *)dsv)	= (SV**)NULL;
                    AvMAX(  (const AV *)dsv)	= -1;
                    AvFILLp((const AV *)dsv)	= -1;
                }
                break;
            case SVt_PVHV:
                if (HvARRAY((const HV *)ssv)) {
                    STRLEN i = 0;
                    XPVHV * const dxhv = (XPVHV*)SvANY(dsv);
                    XPVHV * const sxhv = (XPVHV*)SvANY(ssv);
                    char *darray;
                    Newx(darray, PERL_HV_ARRAY_ALLOC_BYTES(dxhv->xhv_max+1),
                        char);
                    HvARRAY(dsv) = (HE**)darray;
                    while (i <= sxhv->xhv_max) {
                        const HE * const source = HvARRAY(ssv)[i];
                        HvARRAY(dsv)[i] = source
                            ? he_dup(source, FALSE, param) : 0;
                        ++i;
                    }
                    if (HvHasAUX(ssv))
                        sv_dup_hvaux(ssv, dsv, param);
                }
                else
                    HvARRAY(MUTABLE_HV(dsv)) = NULL;
                break;
            case SVt_PVCV:
                if (!(param->flags & CLONEf_COPY_STACKS)) {
                    CvDEPTH(dsv) = 0;
                }
                /* FALLTHROUGH */
            case SVt_PVFM:
                /* NOTE: not refcounted */
                SvANY(MUTABLE_CV(dsv))->xcv_stash =
                    hv_dup(CvSTASH(dsv), param);
                if ((param->flags & CLONEf_JOIN_IN) && CvSTASH(dsv))
                    Perl_sv_add_backref(aTHX_ MUTABLE_SV(CvSTASH(dsv)), dsv);
                if (!CvISXSUB(dsv)) {
                    OP_REFCNT_LOCK;
                    CvROOT(dsv) = OpREFCNT_inc(CvROOT(dsv));
                    OP_REFCNT_UNLOCK;
                    CvSLABBED_off(dsv);
                } else if (CvCONST(dsv)) {
                    CvXSUBANY(dsv).any_ptr =
                        sv_dup_inc((const SV *)CvXSUBANY(dsv).any_ptr, param);
                } else if (CvREFCOUNTED_ANYSV(dsv)) {
                    CvXSUBANY(dsv).any_sv =
                        sv_dup_inc((const SV *)CvXSUBANY(dsv).any_sv, param);
                }
                assert(!CvSLABBED(dsv));
                if (CvDYNFILE(dsv)) CvFILE(dsv) = SAVEPV(CvFILE(dsv));
                if (CvNAMED(dsv))
                    SvANY((CV *)dsv)->xcv_gv_u.xcv_hek =
                        hek_dup(CvNAME_HEK((CV *)ssv), param);
                /* don't dup if copying back - CvGV isn't refcounted, so the
                 * duped GV may never be freed. A bit of a hack! DAPM */
                else
                  SvANY(MUTABLE_CV(dsv))->xcv_gv_u.xcv_gv =
                    CvCVGV_RC(dsv)
                    ? gv_dup_inc(CvGV(ssv), param)
                    : (param->flags & CLONEf_JOIN_IN)
                        ? NULL
                        : gv_dup(CvGV(ssv), param);

                if (!CvISXSUB(ssv)) {
                    PADLIST * padlist = CvPADLIST(ssv);
                    if(padlist)
                        padlist = padlist_dup(padlist, param);
                    CvPADLIST_set(dsv, padlist);
                } else
/* unthreaded perl can't sv_dup so we don't support unthreaded's CvHSCXT */
                    PoisonPADLIST(dsv);

                CvOUTSIDE(dsv)	=
                    CvWEAKOUTSIDE(ssv)
                    ? cv_dup(    CvOUTSIDE(dsv), param)
                    : cv_dup_inc(CvOUTSIDE(dsv), param);
                break;
            case SVt_PVOBJ:
                {
                    Size_t fieldcount = ObjectMAXFIELD(ssv) + 1;

                    Newx(ObjectFIELDS(dsv), fieldcount, SV *);
                    ObjectMAXFIELD(dsv) = fieldcount - 1;

                    sv_dup_inc_multiple(ObjectFIELDS(ssv), ObjectFIELDS(dsv), fieldcount, param);
                }
                break;
            }
        }
    }

    return dsv;
 }

SV *
Perl_sv_dup_inc(pTHX_ const SV *const ssv, CLONE_PARAMS *const param)
{
    PERL_ARGS_ASSERT_SV_DUP_INC;
    return ssv ? SvREFCNT_inc(sv_dup_common(ssv, param)) : NULL;
}

SV *
Perl_sv_dup(pTHX_ const SV *const ssv, CLONE_PARAMS *const param)
{
    SV *dsv = ssv ? sv_dup_common(ssv, param) : NULL;
    PERL_ARGS_ASSERT_SV_DUP;

    /* Track every SV that (at least initially) had a reference count of 0.
       We need to do this by holding an actual reference to it in this array.
       If we attempt to cheat, turn AvREAL_off(), and store only pointers
       (akin to the stashes hash, and the perl stack), we come unstuck if
       a weak reference (or other SV legitimately SvREFCNT() == 0 for this
       thread) is manipulated in a CLONE method, because CLONE runs before the
       unreferenced array is walked to find SVs still with SvREFCNT() == 0
       (and fix things up by giving each a reference via the temps stack).
       Instead, during CLONE, if the 0-referenced SV has SvREFCNT_inc() and
       then SvREFCNT_dec(), it will be cleaned up (and added to the free list)
       before the walk of unreferenced happens and a reference to that is SV
       added to the temps stack. At which point we have the same SV considered
       to be in use, and free to be re-used. Not good.
    */
    if (dsv && !(param->flags & CLONEf_COPY_STACKS) && !SvREFCNT(dsv)) {
        assert(param->unreferenced);
        av_push(param->unreferenced, SvREFCNT_inc(dsv));
    }

    return dsv;
}

/* duplicate a context */

PERL_CONTEXT *
Perl_cx_dup(pTHX_ PERL_CONTEXT *cxs, I32 ix, I32 max, CLONE_PARAMS* param)
{
    PERL_CONTEXT *ncxs;

    PERL_ARGS_ASSERT_CX_DUP;

    if (!cxs)
        return (PERL_CONTEXT*)NULL;

    /* look for it in the table first */
    ncxs = (PERL_CONTEXT*)ptr_table_fetch(PL_ptr_table, cxs);
    if (ncxs)
        return ncxs;

    /* create anew and remember what it is */
    Newx(ncxs, max + 1, PERL_CONTEXT);
    ptr_table_store(PL_ptr_table, cxs, ncxs);
    Copy(cxs, ncxs, max + 1, PERL_CONTEXT);

    while (ix >= 0) {
        PERL_CONTEXT * const ncx = &ncxs[ix];
        if (CxTYPE(ncx) == CXt_SUBST) {
            Perl_croak(aTHX_ "Cloning substitution context is unimplemented");
        }
        else {
            ncx->blk_oldcop = (COP*)any_dup(ncx->blk_oldcop, param->proto_perl);
            switch (CxTYPE(ncx)) {
            case CXt_SUB:
                ncx->blk_sub.cv		= cv_dup_inc(ncx->blk_sub.cv, param);
                if(CxHASARGS(ncx)){
                    ncx->blk_sub.savearray = av_dup_inc(ncx->blk_sub.savearray,param);
                } else {
                    ncx->blk_sub.savearray = NULL;
                }
                ncx->blk_sub.prevcomppad = (PAD*)ptr_table_fetch(PL_ptr_table,
                                           ncx->blk_sub.prevcomppad);
                break;
            case CXt_EVAL:
                ncx->blk_eval.old_namesv = sv_dup_inc(ncx->blk_eval.old_namesv,
                                                      param);
                /* XXX should this sv_dup_inc? Or only if CxEVAL_TXT_REFCNTED ???? */
                ncx->blk_eval.cur_text	= sv_dup(ncx->blk_eval.cur_text, param);
                ncx->blk_eval.cv = cv_dup(ncx->blk_eval.cv, param);
                /* XXX what to do with cur_top_env ???? */
                break;
            case CXt_LOOP_LAZYSV:
                ncx->blk_loop.state_u.lazysv.end
                    = sv_dup_inc(ncx->blk_loop.state_u.lazysv.end, param);
                /* Fallthrough: duplicate lazysv.cur by using the ary.ary
                   duplication code instead.
                   We are taking advantage of (1) av_dup_inc and sv_dup_inc
                   actually being the same function, and (2) order
                   equivalence of the two unions.
                   We can assert the later [but only at run time :-(]  */
                assert ((void *) &ncx->blk_loop.state_u.ary.ary ==
                        (void *) &ncx->blk_loop.state_u.lazysv.cur);
                /* FALLTHROUGH */
            case CXt_LOOP_ARY:
                ncx->blk_loop.state_u.ary.ary
                    = av_dup_inc(ncx->blk_loop.state_u.ary.ary, param);
                /* FALLTHROUGH */
            case CXt_LOOP_LIST:
            case CXt_LOOP_LAZYIV:
                /* code common to all 'for' CXt_LOOP_* types */
                ncx->blk_loop.itersave =
                                    sv_dup_inc(ncx->blk_loop.itersave, param);
                if (CxPADLOOP(ncx)) {
                    PADOFFSET off = ncx->blk_loop.itervar_u.svp
                                    - &CX_CURPAD_SV(ncx->blk_loop, 0);
                    ncx->blk_loop.oldcomppad =
                                    (PAD*)ptr_table_fetch(PL_ptr_table,
                                                ncx->blk_loop.oldcomppad);
                    ncx->blk_loop.itervar_u.svp =
                                    &CX_CURPAD_SV(ncx->blk_loop, off);
                }
                else {
                    /* this copies the GV if CXp_FOR_GV, or the SV for an
                     * alias (for \$x (...)) - relies on gv_dup being the
                     * same as sv_dup */
                    ncx->blk_loop.itervar_u.gv
                        = gv_dup((const GV *)ncx->blk_loop.itervar_u.gv,
                                    param);
                }
                break;
            case CXt_LOOP_PLAIN:
                break;
            case CXt_FORMAT:
                ncx->blk_format.prevcomppad =
                        (PAD*)ptr_table_fetch(PL_ptr_table,
                                           ncx->blk_format.prevcomppad);
                ncx->blk_format.cv	= cv_dup_inc(ncx->blk_format.cv, param);
                ncx->blk_format.gv	= gv_dup(ncx->blk_format.gv, param);
                ncx->blk_format.dfoutgv	= gv_dup_inc(ncx->blk_format.dfoutgv,
                                                     param);
                break;
            case CXt_GIVEN:
                ncx->blk_givwhen.defsv_save =
                                sv_dup_inc(ncx->blk_givwhen.defsv_save, param);
                break;
            case CXt_BLOCK:
            case CXt_NULL:
            case CXt_WHEN:
            case CXt_DEFER:
                break;
            }
        }
        --ix;
    }
    return ncxs;
}

/*
=for apidoc si_dup

Duplicate a stack info structure, returning a pointer to the cloned object.

=cut
*/

PERL_SI *
Perl_si_dup(pTHX_ PERL_SI *si, CLONE_PARAMS* param)
{
    PERL_SI *nsi;

    PERL_ARGS_ASSERT_SI_DUP;

    if (!si)
        return (PERL_SI*)NULL;

    /* look for it in the table first */
    nsi = (PERL_SI*)ptr_table_fetch(PL_ptr_table, si);
    if (nsi)
        return nsi;

    /* create anew and remember what it is */
    Newx(nsi, 1, PERL_SI);
    ptr_table_store(PL_ptr_table, si, nsi);

    nsi->si_stack	= av_dup_inc(si->si_stack, param);
    nsi->si_cxix	= si->si_cxix;
    nsi->si_cxsubix	= si->si_cxsubix;
    nsi->si_cxmax	= si->si_cxmax;
    nsi->si_cxstack	= cx_dup(si->si_cxstack, si->si_cxix, si->si_cxmax, param);
    nsi->si_type	= si->si_type;
    nsi->si_prev	= si_dup(si->si_prev, param);
    nsi->si_next	= si_dup(si->si_next, param);
    nsi->si_markoff	= si->si_markoff;
#if defined DEBUGGING && !defined DEBUGGING_RE_ONLY
    nsi->si_stack_hwm   = 0;
#endif

    return nsi;
}

#define POPINT(ss,ix)	((ss)[--(ix)].any_i32)
#define TOPINT(ss,ix)	((ss)[ix].any_i32)
#define POPLONG(ss,ix)	((ss)[--(ix)].any_long)
#define TOPLONG(ss,ix)	((ss)[ix].any_long)
#define POPIV(ss,ix)	((ss)[--(ix)].any_iv)
#define TOPIV(ss,ix)	((ss)[ix].any_iv)
#define POPUV(ss,ix)	((ss)[--(ix)].any_uv)
#define TOPUV(ss,ix)	((ss)[ix].any_uv)
#define POPBOOL(ss,ix)	((ss)[--(ix)].any_bool)
#define TOPBOOL(ss,ix)	((ss)[ix].any_bool)
#define POPPTR(ss,ix)	((ss)[--(ix)].any_ptr)
#define TOPPTR(ss,ix)	((ss)[ix].any_ptr)
#define POPDPTR(ss,ix)	((ss)[--(ix)].any_dptr)
#define TOPDPTR(ss,ix)	((ss)[ix].any_dptr)
#define POPDXPTR(ss,ix)	((ss)[--(ix)].any_dxptr)
#define TOPDXPTR(ss,ix)	((ss)[ix].any_dxptr)

/* XXXXX todo */
#define pv_dup_inc(p)	SAVEPV(p)
#define pv_dup(p)	SAVEPV(p)
#define svp_dup_inc(p,pp)	any_dup(p,pp)

/* map any object to the new equivalent - either something in the
 * ptr table, or something in the interpreter structure
 */

void *
Perl_any_dup(pTHX_ void *v, const PerlInterpreter *proto_perl)
{
    void *ret;

    PERL_ARGS_ASSERT_ANY_DUP;

    if (!v)
        return (void*)NULL;

    /* look for it in the table first */
    ret = ptr_table_fetch(PL_ptr_table, v);
    if (ret)
        return ret;

    /* see if it is part of the interpreter structure */
    if (v >= (void*)proto_perl && v < (void*)(proto_perl+1))
        ret = (void*)(((char*)aTHX) + (((char*)v) - (char*)proto_perl));
    else {
        ret = v;
    }

    return ret;
}

/*
=for apidoc ss_dup

Duplicate the save stack, returning a pointer to the cloned object.

=cut
*/

ANY *
Perl_ss_dup(pTHX_ PerlInterpreter *proto_perl, CLONE_PARAMS* param)
{
    ANY * const ss	= proto_perl->Isavestack;
    const I32 max	= proto_perl->Isavestack_max + SS_MAXPUSH;
    I32 ix		= proto_perl->Isavestack_ix;
    ANY *nss;
    const SV *sv;
    const GV *gv;
    const AV *av;
    const HV *hv;
    char *pv; /* no const deliberately */
    void* ptr;
    int intval;
    long longval;
    GP *gp;
    IV iv;
    I32 i;
    char *c = NULL;
    void (*dptr) (void*);
    void (*dxptr) (pTHX_ void*);

    PERL_ARGS_ASSERT_SS_DUP;

    Newx(nss, max, ANY);

    while (ix > 0) {
        const UV uv = POPUV(ss,ix);
        const U8 type = (U8)uv & SAVE_MASK;

        TOPUV(nss,ix) = uv;
        switch (type) {
        case SAVEt_CLEARSV:
        case SAVEt_CLEARPADRANGE:
            break;
        case SAVEt_HELEM:		/* hash element */
        case SAVEt_SV:			/* scalar reference */
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = SvREFCNT_inc(sv_dup_inc(sv, param));
            /* FALLTHROUGH */
        case SAVEt_ITEM:			/* normal string */
        case SAVEt_GVSV:			/* scalar slot in GV */
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup_inc(sv, param);
            if (type == SAVEt_SV)
                break;
            /* FALLTHROUGH */
        case SAVEt_FREESV:
        case SAVEt_MORTALIZESV:
        case SAVEt_READONLY_OFF:
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup_inc(sv, param);
            break;
        case SAVEt_FREEPADNAME:
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = padname_dup((PADNAME *)ptr, param);
            PadnameREFCNT((PADNAME *)TOPPTR(nss,ix))++;
            break;
        case SAVEt_SHARED_PVREF:		/* char* in shared space */
            c = (char*)POPPTR(ss,ix);
            TOPPTR(nss,ix) = savesharedpv(c);
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            break;
        case SAVEt_GENERIC_SVREF:		/* generic sv */
        case SAVEt_SVREF:			/* scalar reference */
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup_inc(sv, param);
            if (type == SAVEt_SVREF)
                SvREFCNT_inc_simple_void((SV *)TOPPTR(nss,ix));
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = svp_dup_inc((SV**)ptr, proto_perl);/* XXXXX */
            /* this feels very strange, we have a **SV from one thread,
             * we copy the SV, but dont change the **SV. But in this thread
             * the target of the **SV could be something from the *other* thread.
             * So how can this possibly work correctly? */
            break;
        case SAVEt_RCPV:
            pv = (char *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = rcpv_copy(pv);
            ptr = POPPTR(ss,ix);
            (void)rcpv_copy(*((char **)ptr));
            TOPPTR(nss,ix) = ptr;
            /* XXXXX: see comment above. */
            break;
        case SAVEt_GVSLOT:		/* any slot in GV */
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup_inc(sv, param);
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = svp_dup_inc((SV**)ptr, proto_perl);/* XXXXX */
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup_inc(sv, param);
            break;
        case SAVEt_HV:				/* hash reference */
        case SAVEt_AV:				/* array reference */
            sv = (const SV *) POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup_inc(sv, param);
            /* FALLTHROUGH */
        case SAVEt_COMPPAD:
        case SAVEt_NSTAB:
            sv = (const SV *) POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup(sv, param);
            break;
        case SAVEt_INT:				/* int reference */
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            intval = (int)POPINT(ss,ix);
            TOPINT(nss,ix) = intval;
            break;
        case SAVEt_LONG:			/* long reference */
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            longval = (long)POPLONG(ss,ix);
            TOPLONG(nss,ix) = longval;
            break;
        case SAVEt_I32:				/* I32 reference */
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            i = POPINT(ss,ix);
            TOPINT(nss,ix) = i;
            break;
        case SAVEt_IV:				/* IV reference */
        case SAVEt_STRLEN:			/* STRLEN/size_t ref */
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            iv = POPIV(ss,ix);
            TOPIV(nss,ix) = iv;
            break;
        case SAVEt_TMPSFLOOR:
            iv = POPIV(ss,ix);
            TOPIV(nss,ix) = iv;
            break;
        case SAVEt_HPTR:			/* HV* reference */
        case SAVEt_APTR:			/* AV* reference */
        case SAVEt_SPTR:			/* SV* reference */
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup(sv, param);
            break;
        case SAVEt_VPTR:			/* random* reference */
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            /* FALLTHROUGH */
        case SAVEt_STRLEN_SMALL:
        case SAVEt_INT_SMALL:
        case SAVEt_I32_SMALL:
        case SAVEt_I16:				/* I16 reference */
        case SAVEt_I8:				/* I8 reference */
        case SAVEt_BOOL:
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            break;
        case SAVEt_GENERIC_PVREF:		/* generic char* */
        case SAVEt_PPTR:			/* char* reference */
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            c = (char*)POPPTR(ss,ix);
            TOPPTR(nss,ix) = pv_dup(c);
            break;
        case SAVEt_GP:				/* scalar reference */
            gp = (GP*)POPPTR(ss,ix);
            TOPPTR(nss,ix) = gp = gp_dup(gp, param);
            (void)GpREFCNT_inc(gp);
            gv = (const GV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = gv_dup_inc(gv, param);
            break;
        case SAVEt_FREEOP:
            ptr = POPPTR(ss,ix);
            if (ptr && (((OP*)ptr)->op_private & OPpREFCOUNTED)) {
                /* these are assumed to be refcounted properly */
                OP *o;
                switch (((OP*)ptr)->op_type) {
                case OP_LEAVESUB:
                case OP_LEAVESUBLV:
                case OP_LEAVEEVAL:
                case OP_LEAVE:
                case OP_SCOPE:
                case OP_LEAVEWRITE:
                    TOPPTR(nss,ix) = ptr;
                    o = (OP*)ptr;
                    OP_REFCNT_LOCK;
                    (void) OpREFCNT_inc(o);
                    OP_REFCNT_UNLOCK;
                    break;
                default:
                    TOPPTR(nss,ix) = NULL;
                    break;
                }
            }
            else
                TOPPTR(nss,ix) = NULL;
            break;
        case SAVEt_FREECOPHH:
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = cophh_copy((COPHH *)ptr);
            break;
        case SAVEt_ADELETE:
            av = (const AV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = av_dup_inc(av, param);
            i = POPINT(ss,ix);
            TOPINT(nss,ix) = i;
            break;
        case SAVEt_DELETE:
            hv = (const HV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = hv_dup_inc(hv, param);
            i = POPINT(ss,ix);
            TOPINT(nss,ix) = i;
            /* FALLTHROUGH */
        case SAVEt_FREEPV:
            c = (char*)POPPTR(ss,ix);
            TOPPTR(nss,ix) = pv_dup_inc(c);
            break;
        case SAVEt_FREERCPV:
            c = (char *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = rcpv_copy(c);
            break;
        case SAVEt_STACK_POS:		/* Position on Perl stack */
            i = POPINT(ss,ix);
            TOPINT(nss,ix) = i;
            break;
        case SAVEt_DESTRUCTOR:
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);	/* XXX quite arbitrary */
            dptr = POPDPTR(ss,ix);
            TOPDPTR(nss,ix) = DPTR2FPTR(void (*)(void*),
                                        any_dup(FPTR2DPTR(void *, dptr),
                                                proto_perl));
            break;
        case SAVEt_DESTRUCTOR_X:
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);	/* XXX quite arbitrary */
            dxptr = POPDXPTR(ss,ix);
            TOPDXPTR(nss,ix) = DPTR2FPTR(void (*)(pTHX_ void*),
                                         any_dup(FPTR2DPTR(void *, dxptr),
                                                 proto_perl));
            break;
        case SAVEt_REGCONTEXT:
        case SAVEt_ALLOC:
            ix -= uv >> SAVE_TIGHT_SHIFT;
            break;
        case SAVEt_AELEM:		/* array element */
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = SvREFCNT_inc(sv_dup_inc(sv, param));
            iv = POPIV(ss,ix);
            TOPIV(nss,ix) = iv;
            av = (const AV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = av_dup_inc(av, param);
            break;
        case SAVEt_OP:
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = ptr;
            break;
        case SAVEt_HINTS_HH:
            hv = (const HV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = hv_dup_inc(hv, param);
            /* FALLTHROUGH */
        case SAVEt_HINTS:
            ptr = POPPTR(ss,ix);
            ptr = cophh_copy((COPHH*)ptr);
            TOPPTR(nss,ix) = ptr;
            i = POPINT(ss,ix);
            TOPINT(nss,ix) = i;
            break;
        case SAVEt_PADSV_AND_MORTALIZE:
            longval = (long)POPLONG(ss,ix);
            TOPLONG(nss,ix) = longval;
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = any_dup(ptr, proto_perl);
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup_inc(sv, param);
            break;
        case SAVEt_SET_SVFLAGS:
            i = POPINT(ss,ix);
            TOPINT(nss,ix) = i;
            i = POPINT(ss,ix);
            TOPINT(nss,ix) = i;
            sv = (const SV *)POPPTR(ss,ix);
            TOPPTR(nss,ix) = sv_dup(sv, param);
            break;
        case SAVEt_CURCOP_WARNINGS:
            /* FALLTHROUGH */
        case SAVEt_COMPILE_WARNINGS:
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = DUP_WARNINGS((char*)ptr);
            break;
        case SAVEt_PARSER:
            ptr = POPPTR(ss,ix);
            TOPPTR(nss,ix) = parser_dup((const yy_parser*)ptr, param);
            break;
        default:
            Perl_croak(aTHX_
                       "panic: ss_dup inconsistency (%" IVdf ")", (IV) type);
        }
    }

    return nss;
}


/* if sv is a stash, call $class->CLONE_SKIP(), and set the SVphv_CLONEABLE
 * flag to the result. This is done for each stash before cloning starts,
 * so we know which stashes want their objects cloned */

static void
do_mark_cloneable_stash(pTHX_ SV *const sv)
{
    const HEK * const hvname = HvNAME_HEK((const HV *)sv);
    if (hvname) {
        GV* const cloner = gv_fetchmethod_autoload(MUTABLE_HV(sv), "CLONE_SKIP", 0);
        SvFLAGS(sv) |= SVphv_CLONEABLE; /* clone objects by default */
        if (cloner && GvCV(cloner)) {
            dSP;
            UV status;

            ENTER;
            SAVETMPS;
            PUSHMARK(SP);
            mXPUSHs(newSVhek(hvname));
            PUTBACK;
            call_sv(MUTABLE_SV(GvCV(cloner)), G_SCALAR);
            SPAGAIN;
            status = POPu;
            PUTBACK;
            FREETMPS;
            LEAVE;
            if (status)
                SvFLAGS(sv) &= ~SVphv_CLONEABLE;
        }
    }
}



/*
=for apidoc perl_clone

Create and return a new interpreter by cloning the current one.

C<perl_clone> takes these flags as parameters:

C<CLONEf_COPY_STACKS> - is used to, well, copy the stacks also,
without it we only clone the data and zero the stacks,
with it we copy the stacks and the new perl interpreter is
ready to run at the exact same point as the previous one.
The pseudo-fork code uses C<COPY_STACKS> while the
threads->create doesn't.

C<CLONEf_KEEP_PTR_TABLE> -
C<perl_clone> keeps a ptr_table with the pointer of the old
variable as a key and the new variable as a value,
this allows it to check if something has been cloned and not
clone it again, but rather just use the value and increase the
refcount.
If C<KEEP_PTR_TABLE> is not set then C<perl_clone> will kill the ptr_table
using the function S<C<ptr_table_free(PL_ptr_table); PL_ptr_table = NULL;>>.
A reason to keep it around is if you want to dup some of your own
variables which are outside the graph that perl scans.

C<CLONEf_CLONE_HOST> -
This is a win32 thing, it is ignored on unix, it tells perl's
win32host code (which is c++) to clone itself, this is needed on
win32 if you want to run two threads at the same time,
if you just want to do some stuff in a separate perl interpreter
and then throw it away and return to the original one,
you don't need to do anything.

=cut
*/

/* XXX the above needs expanding by someone who actually understands it ! */
EXTERN_C PerlInterpreter *
perl_clone_host(PerlInterpreter* proto_perl, UV flags);

PerlInterpreter *
perl_clone(PerlInterpreter *proto_perl, UV flags)
{
#ifdef PERL_IMPLICIT_SYS

    PERL_ARGS_ASSERT_PERL_CLONE;

   /* perlhost.h so we need to call into it
   to clone the host, CPerlHost should have a c interface, sky */

#ifndef __amigaos4__
   if (flags & CLONEf_CLONE_HOST) {
       return perl_clone_host(proto_perl,flags);
   }
#endif
   return perl_clone_using(proto_perl, flags,
                            proto_perl->IMem,
                            proto_perl->IMemShared,
                            proto_perl->IMemParse,
                            proto_perl->IEnv,
                            proto_perl->IStdIO,
                            proto_perl->ILIO,
                            proto_perl->IDir,
                            proto_perl->ISock,
                            proto_perl->IProc);
}

PerlInterpreter *
perl_clone_using(PerlInterpreter *proto_perl, UV flags,
                 struct IPerlMem* ipM, struct IPerlMem* ipMS,
                 struct IPerlMem* ipMP, struct IPerlEnv* ipE,
                 struct IPerlStdIO* ipStd, struct IPerlLIO* ipLIO,
                 struct IPerlDir* ipD, struct IPerlSock* ipS,
                 struct IPerlProc* ipP)
{
    /* XXX many of the string copies here can be optimized if they're
     * constants; they need to be allocated as common memory and just
     * their pointers copied. */

    IV i;
    CLONE_PARAMS clone_params;
    CLONE_PARAMS* const param = &clone_params;

    PerlInterpreter * const my_perl = (PerlInterpreter*)(*ipM->pMalloc)(ipM, sizeof(PerlInterpreter));

    PERL_ARGS_ASSERT_PERL_CLONE_USING;
#else		/* !PERL_IMPLICIT_SYS */
    IV i;
    CLONE_PARAMS clone_params;
    CLONE_PARAMS* param = &clone_params;
    PerlInterpreter * const my_perl = (PerlInterpreter*)PerlMem_malloc(sizeof(PerlInterpreter));

    PERL_ARGS_ASSERT_PERL_CLONE;
#endif		/* PERL_IMPLICIT_SYS */

    /* for each stash, determine whether its objects should be cloned */
    S_visit(proto_perl, do_mark_cloneable_stash, SVt_PVHV, SVTYPEMASK);
    my_perl->Iphase = PERL_PHASE_CONSTRUCT;
    PERL_SET_THX(my_perl);

#ifdef DEBUGGING
    PoisonNew(my_perl, 1, PerlInterpreter);
    PL_op = NULL;
    PL_curcop = NULL;
    PL_defstash = NULL; /* may be used by perl malloc() */
    PL_markstack = 0;
    PL_scopestack = 0;
    PL_scopestack_name = 0;
    PL_savestack = 0;
    PL_savestack_ix = 0;
    PL_savestack_max = -1;
    PL_sig_pending = 0;
    PL_parser = NULL;
    PL_eval_begin_nest_depth = proto_perl->Ieval_begin_nest_depth;
    Zero(&PL_debug_pad, 1, struct perl_debug_pad);
    Zero(&PL_padname_undef, 1, PADNAME);
    Zero(&PL_padname_const, 1, PADNAME);
#  ifdef DEBUG_LEAKING_SCALARS
    PL_sv_serial = (((UV)my_perl >> 2) & 0xfff) * 1000000;
#  endif
#  ifdef PERL_TRACE_OPS
    Zero(PL_op_exec_cnt, OP_max+2, UV);
#  endif
#else	/* !DEBUGGING */
    Zero(my_perl, 1, PerlInterpreter);
#endif	/* DEBUGGING */

#ifdef PERL_IMPLICIT_SYS
    /* host pointers */
    PL_Mem		= ipM;
    PL_MemShared	= ipMS;
    PL_MemParse		= ipMP;
    PL_Env		= ipE;
    PL_StdIO		= ipStd;
    PL_LIO		= ipLIO;
    PL_Dir		= ipD;
    PL_Sock		= ipS;
    PL_Proc		= ipP;
#endif		/* PERL_IMPLICIT_SYS */


    param->flags = flags;
    /* Nothing in the core code uses this, but we make it available to
       extensions (using mg_dup).  */
    param->proto_perl = proto_perl;
    /* Likely nothing will use this, but it is initialised to be consistent
       with Perl_clone_params_new().  */
    param->new_perl = my_perl;
    param->unreferenced = NULL;


    INIT_TRACK_MEMPOOL(my_perl->Imemory_debug_header, my_perl);

    PL_body_arenas = NULL;
    Zero(&PL_body_roots, 1, PL_body_roots);

    PL_sv_count		= 0;
    PL_sv_root		= NULL;
    PL_sv_arenaroot	= NULL;

    PL_debug		= proto_perl->Idebug;

    /* dbargs array probably holds garbage */
    PL_dbargs		= NULL;

    PL_compiling = proto_perl->Icompiling;

    /* pseudo environmental stuff */
    PL_origargc		= proto_perl->Iorigargc;
    PL_origargv		= proto_perl->Iorigargv;

#ifndef NO_TAINT_SUPPORT
    /* Set tainting stuff before PerlIO_debug can possibly get called */
    PL_tainting		= proto_perl->Itainting;
    PL_taint_warn	= proto_perl->Itaint_warn;
#else
    PL_tainting         = FALSE;
    PL_taint_warn	= FALSE;
#endif

    PL_minus_c		= proto_perl->Iminus_c;

    PL_localpatches	= proto_perl->Ilocalpatches;
    PL_splitstr		= SAVEPV(proto_perl->Isplitstr);
    PL_minus_n		= proto_perl->Iminus_n;
    PL_minus_p		= proto_perl->Iminus_p;
    PL_minus_l		= proto_perl->Iminus_l;
    PL_minus_a		= proto_perl->Iminus_a;
    PL_minus_E		= proto_perl->Iminus_E;
    PL_minus_F		= proto_perl->Iminus_F;
    PL_doswitches	= proto_perl->Idoswitches;
    PL_dowarn		= proto_perl->Idowarn;
#ifdef PERL_SAWAMPERSAND
    PL_sawampersand	= proto_perl->Isawampersand;
#endif
    PL_unsafe		= proto_perl->Iunsafe;
    PL_perldb		= proto_perl->Iperldb;
    PL_perl_destruct_level = proto_perl->Iperl_destruct_level;
    PL_exit_flags       = proto_perl->Iexit_flags;

    /* XXX time(&PL_basetime) when asked for? */
    PL_basetime		= proto_perl->Ibasetime;

    PL_maxsysfd		= proto_perl->Imaxsysfd;
    PL_statusvalue	= proto_perl->Istatusvalue;
#ifdef __VMS
    PL_statusvalue_vms	= proto_perl->Istatusvalue_vms;
#else
    PL_statusvalue_posix = proto_perl->Istatusvalue_posix;
#endif

    /* RE engine related */
    PL_regmatch_slab	= NULL;
    PL_reg_curpm	= NULL;

    PL_sub_generation	= proto_perl->Isub_generation;

    /* funky return mechanisms */
    PL_forkprocess	= proto_perl->Iforkprocess;

    /* internal state */
    PL_main_start	= proto_perl->Imain_start;
    PL_eval_root	= proto_perl->Ieval_root;
    PL_eval_start	= proto_perl->Ieval_start;

    PL_filemode		= proto_perl->Ifilemode;
    PL_lastfd		= proto_perl->Ilastfd;
    PL_oldname		= proto_perl->Ioldname;		/* XXX not quite right */
    PL_gensym		= proto_perl->Igensym;

    PL_laststatval	= proto_perl->Ilaststatval;
    PL_laststype	= proto_perl->Ilaststype;
    PL_mess_sv		= NULL;

    PL_profiledata	= NULL;

    PL_generation	= proto_perl->Igeneration;

    PL_in_clean_objs	= proto_perl->Iin_clean_objs;
    PL_in_clean_all	= proto_perl->Iin_clean_all;

    PL_delaymagic_uid	= proto_perl->Idelaymagic_uid;
    PL_delaymagic_euid	= proto_perl->Idelaymagic_euid;
    PL_delaymagic_gid	= proto_perl->Idelaymagic_gid;
    PL_delaymagic_egid	= proto_perl->Idelaymagic_egid;
    PL_nomemok		= proto_perl->Inomemok;
    PL_an		= proto_perl->Ian;
    PL_evalseq		= proto_perl->Ievalseq;
    PL_origalen		= proto_perl->Iorigalen;

    PL_sighandlerp	= proto_perl->Isighandlerp;
    PL_sighandler1p	= proto_perl->Isighandler1p;
    PL_sighandler3p	= proto_perl->Isighandler3p;

    PL_runops		= proto_perl->Irunops;

    PL_subline		= proto_perl->Isubline;

    PL_cv_has_eval	= proto_perl->Icv_has_eval;
    /* Unicode features (see perlrun/-C) */
    PL_unicode		= proto_perl->Iunicode;

    /* Pre-5.8 signals control */
    PL_signals		= proto_perl->Isignals;

    /* times() ticks per second */
    PL_clocktick	= proto_perl->Iclocktick;

    /* Recursion stopper for PerlIO_find_layer */
    PL_in_load_module	= proto_perl->Iin_load_module;

    /* Not really needed/useful since the reenrant_retint is "volatile",
     * but do it for consistency's sake. */
    PL_reentrant_retint	= proto_perl->Ireentrant_retint;

    /* Hooks to shared SVs and locks. */
    PL_sharehook	= proto_perl->Isharehook;
    PL_lockhook		= proto_perl->Ilockhook;
    PL_unlockhook	= proto_perl->Iunlockhook;
    PL_threadhook	= proto_perl->Ithreadhook;
    PL_destroyhook	= proto_perl->Idestroyhook;
    PL_signalhook	= proto_perl->Isignalhook;

    PL_globhook		= proto_perl->Iglobhook;

    PL_srand_called	= proto_perl->Isrand_called;
    Copy(&(proto_perl->Irandom_state), &PL_random_state, 1, PL_RANDOM_STATE_TYPE);
    PL_srand_override   = proto_perl->Isrand_override;
    PL_srand_override_next = proto_perl->Isrand_override_next;

    if (flags & CLONEf_COPY_STACKS) {
        /* next allocation will be PL_tmps_stack[PL_tmps_ix+1] */
        PL_tmps_ix		= proto_perl->Itmps_ix;
        PL_tmps_max		= proto_perl->Itmps_max;
        PL_tmps_floor		= proto_perl->Itmps_floor;

        /* next push_scope()/ENTER sets PL_scopestack[PL_scopestack_ix]
         * NOTE: unlike the others! */
        PL_scopestack_ix	= proto_perl->Iscopestack_ix;
        PL_scopestack_max	= proto_perl->Iscopestack_max;

        /* next SSPUSHFOO() sets PL_savestack[PL_savestack_ix]
         * NOTE: unlike the others! */
        PL_savestack_ix		= proto_perl->Isavestack_ix;
        PL_savestack_max	= proto_perl->Isavestack_max;
    }

    PL_start_env	= proto_perl->Istart_env;	/* XXXXXX */
    PL_top_env		= &PL_start_env;

    PL_op		= proto_perl->Iop;

    PL_Sv		= NULL;
    PL_Xpv		= (XPV*)NULL;
    my_perl->Ina	= proto_perl->Ina;

    PL_statcache	= proto_perl->Istatcache;

#ifndef NO_TAINT_SUPPORT
    PL_tainted		= proto_perl->Itainted;
#else
    PL_tainted          = FALSE;
#endif
    PL_curpm		= proto_perl->Icurpm;	/* XXX No PMOP ref count */

    PL_chopset		= proto_perl->Ichopset;	/* XXX never deallocated */

    PL_restartjmpenv	= proto_perl->Irestartjmpenv;
    PL_restartop	= proto_perl->Irestartop;
    PL_in_eval		= proto_perl->Iin_eval;
    PL_delaymagic	= proto_perl->Idelaymagic;
    PL_phase		= proto_perl->Iphase;
    PL_localizing	= proto_perl->Ilocalizing;

    PL_hv_fetch_ent_mh	= NULL;
    PL_modcount		= proto_perl->Imodcount;
    PL_lastgotoprobe	= NULL;
    PL_dumpindent	= proto_perl->Idumpindent;

    PL_efloatbuf	= NULL;		/* reinits on demand */
    PL_efloatsize	= 0;			/* reinits on demand */

    /* regex stuff */

    PL_colorset		= 0;		/* reinits PL_colors[] */
    /*PL_colors[6]	= {0,0,0,0,0,0};*/

    /* Pluggable optimizer */
    PL_peepp		= proto_perl->Ipeepp;
    PL_rpeepp		= proto_perl->Irpeepp;
    /* op_free() hook */
    PL_opfreehook	= proto_perl->Iopfreehook;

#  ifdef PERL_MEM_LOG
    Zero(PL_mem_log, sizeof(PL_mem_log), char);
#  endif

#ifdef USE_REENTRANT_API
    /* XXX: things like -Dm will segfault here in perlio, but doing
     *  PERL_SET_CONTEXT(proto_perl);
     * breaks too many other things
     */
    Perl_reentrant_init(aTHX);
#endif

    /* create SV map for pointer relocation */
    PL_ptr_table = ptr_table_new();

    /* initialize these special pointers as early as possible */
    init_constants();
    ptr_table_store(PL_ptr_table, &proto_perl->Isv_undef, &PL_sv_undef);
    ptr_table_store(PL_ptr_table, &proto_perl->Isv_no, &PL_sv_no);
    ptr_table_store(PL_ptr_table, &proto_perl->Isv_zero, &PL_sv_zero);
    ptr_table_store(PL_ptr_table, &proto_perl->Isv_yes, &PL_sv_yes);
    ptr_table_store(PL_ptr_table, &proto_perl->Ipadname_const,
                    &PL_padname_const);

    /* create (a non-shared!) shared string table */
    PL_strtab		= newHV();
    HvSHAREKEYS_off(PL_strtab);
    hv_ksplit(PL_strtab, HvTOTALKEYS(proto_perl->Istrtab));
    ptr_table_store(PL_ptr_table, proto_perl->Istrtab, PL_strtab);

    Zero(PL_sv_consts, SV_CONSTS_COUNT, SV*);

    PL_compiling.cop_file    = rcpv_copy(proto_perl->Icompiling.cop_file);

    ptr_table_store(PL_ptr_table, &proto_perl->Icompiling, &PL_compiling);
    PL_compiling.cop_warnings = DUP_WARNINGS(PL_compiling.cop_warnings);
    CopHINTHASH_set(&PL_compiling, cophh_copy(CopHINTHASH_get(&PL_compiling)));
    PL_curcop		= (COP*)any_dup(proto_perl->Icurcop, proto_perl);

    param->stashes      = newAV();  /* Setup array of objects to call clone on */
    /* This makes no difference to the implementation, as it always pushes
       and shifts pointers to other SVs without changing their reference
       count, with the array becoming empty before it is freed. However, it
       makes it conceptually clear what is going on, and will avoid some
       work inside av.c, filling slots between AvFILL() and AvMAX() with
       &PL_sv_undef, and SvREFCNT_dec()ing those.  */
    AvREAL_off(param->stashes);

    if (!(flags & CLONEf_COPY_STACKS)) {
        param->unreferenced = newAV();
    }

#ifdef PERLIO_LAYERS
    /* Clone PerlIO tables as soon as we can handle general xx_dup() */
    PerlIO_clone(aTHX_ proto_perl, param);
#endif

    PL_envgv		= gv_dup_inc(proto_perl->Ienvgv, param);
    PL_incgv		= gv_dup_inc(proto_perl->Iincgv, param);
    PL_hintgv		= gv_dup_inc(proto_perl->Ihintgv, param);
    PL_origfilename	= SAVEPV(proto_perl->Iorigfilename);
    PL_xsubfilename	= proto_perl->Ixsubfilename;
    PL_diehook		= sv_dup_inc(proto_perl->Idiehook, param);
    PL_warnhook		= sv_dup_inc(proto_perl->Iwarnhook, param);

    PL_hook__require__before = sv_dup_inc(proto_perl->Ihook__require__before, param);
    PL_hook__require__after  = sv_dup_inc(proto_perl->Ihook__require__after, param);

    /* switches */
    PL_patchlevel	= sv_dup_inc(proto_perl->Ipatchlevel, param);
    PL_inplace		= SAVEPV(proto_perl->Iinplace);
    PL_e_script		= sv_dup_inc(proto_perl->Ie_script, param);

    /* magical thingies */

    SvPVCLEAR(PERL_DEBUG_PAD(0));        /* For regex debugging. */
    SvPVCLEAR(PERL_DEBUG_PAD(1));        /* ext/re needs these */
    SvPVCLEAR(PERL_DEBUG_PAD(2));        /* even without DEBUGGING. */


    /* Clone the regex array */
    /* ORANGE FIXME for plugins, probably in the SV dup code.
       newSViv(PTR2IV(CALLREGDUPE(
       INT2PTR(REGEXP *, SvIVX(regex)), param))))
    */
    PL_regex_padav = av_dup_inc(proto_perl->Iregex_padav, param);
    PL_regex_pad = AvARRAY(PL_regex_padav);

    PL_stashpadmax	= proto_perl->Istashpadmax;
    PL_stashpadix	= proto_perl->Istashpadix ;
    Newx(PL_stashpad, PL_stashpadmax, HV *);
    {
        PADOFFSET o = 0;
        for (; o < PL_stashpadmax; ++o)
            PL_stashpad[o] = hv_dup(proto_perl->Istashpad[o], param);
    }

    /* shortcuts to various I/O objects */
    PL_ofsgv            = gv_dup_inc(proto_perl->Iofsgv, param);
    PL_stdingv		= gv_dup(proto_perl->Istdingv, param);
    PL_stderrgv		= gv_dup(proto_perl->Istderrgv, param);
    PL_defgv		= gv_dup(proto_perl->Idefgv, param);
    PL_argvgv		= gv_dup_inc(proto_perl->Iargvgv, param);
    PL_argvoutgv	= gv_dup(proto_perl->Iargvoutgv, param);
    PL_argvout_stack	= av_dup_inc(proto_perl->Iargvout_stack, param);

    /* shortcuts to regexp stuff */
    PL_replgv		= gv_dup_inc(proto_perl->Ireplgv, param);

    /* shortcuts to misc objects */
    PL_errgv		= gv_dup(proto_perl->Ierrgv, param);

    /* shortcuts to debugging objects */
    PL_DBgv		= gv_dup_inc(proto_perl->IDBgv, param);
    PL_DBline		= gv_dup_inc(proto_perl->IDBline, param);
    PL_DBsub		= gv_dup_inc(proto_perl->IDBsub, param);
    PL_DBsingle		= sv_dup(proto_perl->IDBsingle, param);
    PL_DBtrace		= sv_dup(proto_perl->IDBtrace, param);
    PL_DBsignal		= sv_dup(proto_perl->IDBsignal, param);
    Copy(proto_perl->IDBcontrol, PL_DBcontrol, DBVARMG_COUNT, IV);

    /* symbol tables */
    PL_defstash		= hv_dup_inc(proto_perl->Idefstash, param);
    PL_curstash		= hv_dup_inc(proto_perl->Icurstash, param);
    PL_debstash		= hv_dup(proto_perl->Idebstash, param);
    PL_globalstash	= hv_dup(proto_perl->Iglobalstash, param);
    PL_curstname	= sv_dup_inc(proto_perl->Icurstname, param);

    PL_beginav		= av_dup_inc(proto_perl->Ibeginav, param);
    PL_beginav_save	= av_dup_inc(proto_perl->Ibeginav_save, param);
    PL_checkav_save	= av_dup_inc(proto_perl->Icheckav_save, param);
    PL_unitcheckav      = av_dup_inc(proto_perl->Iunitcheckav, param);
    PL_unitcheckav_save = av_dup_inc(proto_perl->Iunitcheckav_save, param);
    PL_endav		= av_dup_inc(proto_perl->Iendav, param);
    PL_checkav		= av_dup_inc(proto_perl->Icheckav, param);
    PL_initav		= av_dup_inc(proto_perl->Iinitav, param);
    PL_savebegin	= proto_perl->Isavebegin;

    PL_isarev		= hv_dup_inc(proto_perl->Iisarev, param);

    /* subprocess state */
    PL_fdpid		= av_dup_inc(proto_perl->Ifdpid, param);

    if (proto_perl->Iop_mask)
        PL_op_mask	= SAVEPVN(proto_perl->Iop_mask, PL_maxo);
    else
        PL_op_mask 	= NULL;
    /* PL_asserting        = proto_perl->Iasserting; */

    /* current interpreter roots */
    PL_main_cv		= cv_dup_inc(proto_perl->Imain_cv, param);
    OP_REFCNT_LOCK;
    PL_main_root	= OpREFCNT_inc(proto_perl->Imain_root);
    OP_REFCNT_UNLOCK;

    /* runtime control stuff */
    PL_curcopdb		= (COP*)any_dup(proto_perl->Icurcopdb, proto_perl);

    PL_preambleav	= av_dup_inc(proto_perl->Ipreambleav, param);

    PL_ors_sv		= sv_dup_inc(proto_perl->Iors_sv, param);

    /* interpreter atexit processing */
    PL_exitlistlen	= proto_perl->Iexitlistlen;
    if (PL_exitlistlen) {
        Newx(PL_exitlist, PL_exitlistlen, PerlExitListEntry);
        Copy(proto_perl->Iexitlist, PL_exitlist, PL_exitlistlen, PerlExitListEntry);
    }
    else
        PL_exitlist	= (PerlExitListEntry*)NULL;

    PL_my_cxt_size = proto_perl->Imy_cxt_size;
    if (PL_my_cxt_size) {
        Newx(PL_my_cxt_list, PL_my_cxt_size, void *);
        Copy(proto_perl->Imy_cxt_list, PL_my_cxt_list, PL_my_cxt_size, void *);
    }
    else {
        PL_my_cxt_list	= (void**)NULL;
    }
    PL_modglobal	= hv_dup_inc(proto_perl->Imodglobal, param);
    PL_custom_op_names  = hv_dup_inc(proto_perl->Icustom_op_names,param);
    PL_custom_op_descs  = hv_dup_inc(proto_perl->Icustom_op_descs,param);
    PL_custom_ops	= hv_dup_inc(proto_perl->Icustom_ops, param);

    PL_compcv			= cv_dup(proto_perl->Icompcv, param);

    PAD_CLONE_VARS(proto_perl, param);

#ifdef HAVE_INTERP_INTERN
    sys_intern_dup(&proto_perl->Isys_intern, &PL_sys_intern);
#endif

    PL_DBcv		= cv_dup(proto_perl->IDBcv, param);

#ifdef PERL_USES_PL_PIDSTATUS
    PL_pidstatus	= newHV();			/* XXX flag for cloning? */
#endif
    PL_osname		= SAVEPV(proto_perl->Iosname);
    PL_parser		= parser_dup(proto_perl->Iparser, param);

    /* XXX this only works if the saved cop has already been cloned */
    if (proto_perl->Iparser) {
        PL_parser->saved_curcop = (COP*)any_dup(
                                    proto_perl->Iparser->saved_curcop,
                                    proto_perl);
    }

    PL_subname		= sv_dup_inc(proto_perl->Isubname, param);

#ifdef USE_PL_CURLOCALES
    for (i = 0; i < (int) C_ARRAY_LENGTH(PL_curlocales); i++) {
        PL_curlocales[i] = SAVEPV("C");
    }
#endif
#ifdef USE_PL_CUR_LC_ALL
    PL_cur_LC_ALL = SAVEPV("C");
#endif
#ifdef USE_LOCALE_CTYPE
    Copy(PL_fold, PL_fold_locale, 256, U8);

    /* Should we warn if uses locale? */
    PL_ctype_name	= SAVEPV("C");
    PL_warn_locale      = sv_dup_inc(proto_perl->Iwarn_locale, param);
    PL_in_utf8_CTYPE_locale   = false;
    PL_in_utf8_turkic_locale  = false;
#endif

    /* Did the locale setup indicate UTF-8? */
    PL_utf8locale	= false;

#ifdef USE_LOCALE_COLLATE
    PL_in_utf8_COLLATE_locale = false;
    PL_collation_name	= SAVEPV("C");
    PL_collation_ix	= proto_perl->Icollation_ix;
    PL_collation_standard = true;
    PL_collxfrm_base	= 0;
    PL_collxfrm_mult	= 0;
    PL_strxfrm_max_cp   = 0;
    PL_strxfrm_is_behaved = proto_perl->Istrxfrm_is_behaved;
    PL_strxfrm_NUL_replacement = '\0';
#endif /* USE_LOCALE_COLLATE */

#ifdef USE_LOCALE_THREADS
    assert(PL_locale_mutex_depth <= 0);
    PL_locale_mutex_depth = 0;
#endif

#ifdef USE_LOCALE_NUMERIC
    PL_numeric_name	= SAVEPV("C");
    PL_numeric_radix_sv	= newSVpvs(".");
    PL_underlying_radix_sv = newSVpvs(".");
    PL_numeric_standard	= true;
    PL_numeric_underlying = true;
    PL_numeric_underlying_is_standard = true;

#  if defined(USE_POSIX_2008_LOCALE)
    PL_underlying_numeric_obj = NULL;
#  endif
#endif /* !USE_LOCALE_NUMERIC */
#if defined(USE_POSIX_2008_LOCALE)
    PL_scratch_locale_obj = NULL;
    PL_cur_locale_obj = PL_C_locale_obj;
#endif

#ifdef HAS_MBRLEN
    PL_mbrlen_ps = proto_perl->Imbrlen_ps;
#endif
#ifdef HAS_MBRTOWC
    PL_mbrtowc_ps = proto_perl->Imbrtowc_ps;
#endif
#ifdef HAS_WCRTOMB
    PL_wcrtomb_ps = proto_perl->Iwcrtomb_ps;
#endif

    PL_langinfo_buf = NULL;
    PL_langinfo_bufsize = 0;

    PL_setlocale_buf = NULL;
    PL_setlocale_bufsize = 0;

    PL_stdize_locale_buf = NULL;
    PL_stdize_locale_bufsize = 0;

    /* Unicode inversion lists */

    PL_AboveLatin1            = sv_dup_inc(proto_perl->IAboveLatin1, param);
    PL_Assigned_invlist       = sv_dup_inc(proto_perl->IAssigned_invlist, param);
    PL_GCB_invlist            = sv_dup_inc(proto_perl->IGCB_invlist, param);
    PL_HasMultiCharFold       = sv_dup_inc(proto_perl->IHasMultiCharFold, param);
    PL_InMultiCharFold        = sv_dup_inc(proto_perl->IInMultiCharFold, param);
    PL_Latin1                 = sv_dup_inc(proto_perl->ILatin1, param);
    PL_LB_invlist             = sv_dup_inc(proto_perl->ILB_invlist, param);
    PL_SB_invlist             = sv_dup_inc(proto_perl->ISB_invlist, param);
    PL_SCX_invlist            = sv_dup_inc(proto_perl->ISCX_invlist, param);
    PL_UpperLatin1            = sv_dup_inc(proto_perl->IUpperLatin1, param);
    PL_in_some_fold           = sv_dup_inc(proto_perl->Iin_some_fold, param);
    PL_utf8_foldclosures      = sv_dup_inc(proto_perl->Iutf8_foldclosures, param);
    PL_utf8_idcont            = sv_dup_inc(proto_perl->Iutf8_idcont, param);
    PL_utf8_idstart           = sv_dup_inc(proto_perl->Iutf8_idstart, param);
    PL_utf8_perl_idcont       = sv_dup_inc(proto_perl->Iutf8_perl_idcont, param);
    PL_utf8_perl_idstart      = sv_dup_inc(proto_perl->Iutf8_perl_idstart, param);
    PL_utf8_xidcont           = sv_dup_inc(proto_perl->Iutf8_xidcont, param);
    PL_utf8_xidstart          = sv_dup_inc(proto_perl->Iutf8_xidstart, param);
    PL_WB_invlist             = sv_dup_inc(proto_perl->IWB_invlist, param);
    for (i = 0; i < POSIX_CC_COUNT; i++) {
        PL_XPosix_ptrs[i]     = sv_dup_inc(proto_perl->IXPosix_ptrs[i], param);
        if (i != CC_CASED_ && i != CC_VERTSPACE_) {
            PL_Posix_ptrs[i]  = sv_dup_inc(proto_perl->IPosix_ptrs[i], param);
        }
    }
    PL_Posix_ptrs[CC_CASED_]  = PL_Posix_ptrs[CC_ALPHA_];
    PL_Posix_ptrs[CC_VERTSPACE_] = NULL;

    PL_utf8_toupper           = sv_dup_inc(proto_perl->Iutf8_toupper, param);
    PL_utf8_totitle           = sv_dup_inc(proto_perl->Iutf8_totitle, param);
    PL_utf8_tolower           = sv_dup_inc(proto_perl->Iutf8_tolower, param);
    PL_utf8_tofold            = sv_dup_inc(proto_perl->Iutf8_tofold, param);
    PL_utf8_tosimplefold      = sv_dup_inc(proto_perl->Iutf8_tosimplefold, param);
    PL_utf8_charname_begin    = sv_dup_inc(proto_perl->Iutf8_charname_begin, param);
    PL_utf8_charname_continue = sv_dup_inc(proto_perl->Iutf8_charname_continue, param);
    PL_utf8_mark              = sv_dup_inc(proto_perl->Iutf8_mark, param);
    PL_InBitmap               = sv_dup_inc(proto_perl->IInBitmap, param);
    PL_CCC_non0_non230        = sv_dup_inc(proto_perl->ICCC_non0_non230, param);
    PL_Private_Use            = sv_dup_inc(proto_perl->IPrivate_Use, param);

#if 0
    PL_seen_deprecated_macro = hv_dup_inc(proto_perl->Iseen_deprecated_macro, param);
#endif

    if (proto_perl->Ipsig_pend) {
        Newxz(PL_psig_pend, SIG_SIZE, int);
    }
    else {
        PL_psig_pend	= (int*)NULL;
    }

    if (proto_perl->Ipsig_name) {
        Newx(PL_psig_name, 2 * SIG_SIZE, SV*);
        sv_dup_inc_multiple(proto_perl->Ipsig_name, PL_psig_name, 2 * SIG_SIZE,
                            param);
        PL_psig_ptr = PL_psig_name + SIG_SIZE;
    }
    else {
        PL_psig_ptr	= (SV**)NULL;
        PL_psig_name	= (SV**)NULL;
    }

    if (flags & CLONEf_COPY_STACKS) {
        Newx(PL_tmps_stack, PL_tmps_max, SV*);
        sv_dup_inc_multiple(proto_perl->Itmps_stack, PL_tmps_stack,
                            PL_tmps_ix+1, param);

        /* next PUSHMARK() sets *(PL_markstack_ptr+1) */
        i = proto_perl->Imarkstack_max - proto_perl->Imarkstack;
        Newx(PL_markstack, i, I32);
        PL_markstack_max	= PL_markstack + (proto_perl->Imarkstack_max
                                                  - proto_perl->Imarkstack);
        PL_markstack_ptr	= PL_markstack + (proto_perl->Imarkstack_ptr
                                                  - proto_perl->Imarkstack);
        Copy(proto_perl->Imarkstack, PL_markstack,
             PL_markstack_ptr - PL_markstack + 1, I32);

        /* next push_scope()/ENTER sets PL_scopestack[PL_scopestack_ix]
         * NOTE: unlike the others! */
        Newx(PL_scopestack, PL_scopestack_max, I32);
        Copy(proto_perl->Iscopestack, PL_scopestack, PL_scopestack_ix, I32);

#ifdef DEBUGGING
        Newx(PL_scopestack_name, PL_scopestack_max, const char *);
        Copy(proto_perl->Iscopestack_name, PL_scopestack_name, PL_scopestack_ix, const char *);
#endif
        /* reset stack AV to correct length before its duped via
         * PL_curstackinfo */
        AvFILLp(proto_perl->Icurstack) =
                            proto_perl->Istack_sp - proto_perl->Istack_base;

        /* NOTE: si_dup() looks at PL_markstack */
        PL_curstackinfo		= si_dup(proto_perl->Icurstackinfo, param);

        /* PL_curstack		= PL_curstackinfo->si_stack; */
        PL_curstack		= av_dup(proto_perl->Icurstack, param);
        PL_mainstack		= av_dup(proto_perl->Imainstack, param);

        /* next PUSHs() etc. set *(PL_stack_sp+1) */
        PL_stack_base		= AvARRAY(PL_curstack);
        PL_stack_sp		= PL_stack_base + (proto_perl->Istack_sp
                                                   - proto_perl->Istack_base);
        PL_stack_max		= PL_stack_base + AvMAX(PL_curstack);

        /*Newxz(PL_savestack, PL_savestack_max, ANY);*/
        PL_savestack		= ss_dup(proto_perl, param);
    }
    else {
        init_stacks();
        ENTER;			/* perl_destruct() wants to LEAVE; */
    }

    PL_statgv		= gv_dup(proto_perl->Istatgv, param);
    PL_statname		= sv_dup_inc(proto_perl->Istatname, param);

    PL_rs		= sv_dup_inc(proto_perl->Irs, param);
    PL_last_in_gv	= gv_dup(proto_perl->Ilast_in_gv, param);
    PL_defoutgv		= gv_dup_inc(proto_perl->Idefoutgv, param);
    PL_toptarget	= sv_dup_inc(proto_perl->Itoptarget, param);
    PL_bodytarget	= sv_dup_inc(proto_perl->Ibodytarget, param);
    PL_formtarget	= sv_dup(proto_perl->Iformtarget, param);

    PL_errors		= sv_dup_inc(proto_perl->Ierrors, param);

    PL_sortcop		= (OP*)any_dup(proto_perl->Isortcop, proto_perl);
    PL_firstgv		= gv_dup_inc(proto_perl->Ifirstgv, param);
    PL_secondgv		= gv_dup_inc(proto_perl->Isecondgv, param);

    PL_stashcache       = newHV();

    PL_watchaddr	= (char **) ptr_table_fetch(PL_ptr_table,
                                            proto_perl->Iwatchaddr);
    PL_watchok		= PL_watchaddr ? * PL_watchaddr : NULL;
    if (PL_debug && PL_watchaddr) {
        PerlIO_printf(Perl_debug_log,
          "WATCHING: %" UVxf " cloned as %" UVxf " with value %" UVxf "\n",
          PTR2UV(proto_perl->Iwatchaddr), PTR2UV(PL_watchaddr),
          PTR2UV(PL_watchok));
    }

    PL_registered_mros  = hv_dup_inc(proto_perl->Iregistered_mros, param);
    PL_blockhooks	= av_dup_inc(proto_perl->Iblockhooks, param);

    /* Call the ->CLONE method, if it exists, for each of the stashes
       identified by sv_dup() above.
    */
    while(av_count(param->stashes) != 0) {
        HV* const stash = MUTABLE_HV(av_shift(param->stashes));
        GV* const cloner = gv_fetchmethod_autoload(stash, "CLONE", 0);
        if (cloner && GvCV(cloner)) {
            dSP;
            ENTER;
            SAVETMPS;
            PUSHMARK(SP);
            mXPUSHs(newSVhek(HvNAME_HEK(stash)));
            PUTBACK;
            call_sv(MUTABLE_SV(GvCV(cloner)), G_DISCARD);
            FREETMPS;
            LEAVE;
        }
    }

    if (!(flags & CLONEf_KEEP_PTR_TABLE)) {
        ptr_table_free(PL_ptr_table);
        PL_ptr_table = NULL;
    }

    if (!(flags & CLONEf_COPY_STACKS)) {
        unreferenced_to_tmp_stack(param->unreferenced);
    }

    SvREFCNT_dec(param->stashes);

    /* orphaned? eg threads->new inside BEGIN or use */
    if (PL_compcv && ! SvREFCNT(PL_compcv)) {
        SvREFCNT_inc_simple_void(PL_compcv);
        SAVEFREESV(PL_compcv);
    }

    return my_perl;
}

static void
S_unreferenced_to_tmp_stack(pTHX_ AV *const unreferenced)
{
    PERL_ARGS_ASSERT_UNREFERENCED_TO_TMP_STACK;

    if (AvFILLp(unreferenced) > -1) {
        SV **svp = AvARRAY(unreferenced);
        SV **const last = svp + AvFILLp(unreferenced);
        SSize_t count = 0;

        do {
            if (SvREFCNT(*svp) == 1)
                ++count;
        } while (++svp <= last);

        EXTEND_MORTAL(count);
        svp = AvARRAY(unreferenced);

        do {
            if (SvREFCNT(*svp) == 1) {
                /* Our reference is the only one to this SV. This means that
                   in this thread, the scalar effectively has a 0 reference.
                   That doesn't work (cleanup never happens), so donate our
                   reference to it onto the save stack. */
                PL_tmps_stack[++PL_tmps_ix] = *svp;
            } else {
                /* As an optimisation, because we are already walking the
                   entire array, instead of above doing either
                   SvREFCNT_inc(*svp) or *svp = &PL_sv_undef, we can instead
                   release our reference to the scalar, so that at the end of
                   the array owns zero references to the scalars it happens to
                   point to. We are effectively converting the array from
                   AvREAL() on to AvREAL() off. This saves the av_clear()
                   (triggered by the SvREFCNT_dec(unreferenced) below) from
                   walking the array a second time.  */
                SvREFCNT_dec(*svp);
            }

        } while (++svp <= last);
        AvREAL_off(unreferenced);
    }
    SvREFCNT_dec_NN(unreferenced);
}

void
Perl_clone_params_del(CLONE_PARAMS *param)
{
    PerlInterpreter *const was = PERL_GET_THX;
    PerlInterpreter *const to = param->new_perl;
    dTHXa(to);

    PERL_ARGS_ASSERT_CLONE_PARAMS_DEL;

    if (was != to) {
        PERL_SET_THX(to);
    }

    SvREFCNT_dec(param->stashes);
    if (param->unreferenced)
        unreferenced_to_tmp_stack(param->unreferenced);

    Safefree(param);

    if (was != to) {
        PERL_SET_THX(was);
    }
}

CLONE_PARAMS *
Perl_clone_params_new(PerlInterpreter *const from, PerlInterpreter *const to)
{
    /* Need to play this game, as newAV() can call safesysmalloc(), and that
       does a dTHX; to get the context from thread local storage.
       FIXME - under PERL_CORE Newx(), Safefree() and friends should expand to
       a version that passes in my_perl.  */
    PerlInterpreter *const was = PERL_GET_THX;
    CLONE_PARAMS *param;

    PERL_ARGS_ASSERT_CLONE_PARAMS_NEW;

    if (was != to) {
        PERL_SET_THX(to);
    }

    /* Given that we've set the context, we can do this unshared.  */
    Newx(param, 1, CLONE_PARAMS);

    param->flags = 0;
    param->proto_perl = from;
    param->new_perl = to;
    param->stashes = (AV *)Perl_newSV_type(to, SVt_PVAV);
    AvREAL_off(param->stashes);
    param->unreferenced = (AV *)Perl_newSV_type(to, SVt_PVAV);

    if (was != to) {
        PERL_SET_THX(was);
    }
    return param;
}

#endif /* USE_ITHREADS */

void
Perl_init_constants(pTHX)
{

    SvREFCNT(&PL_sv_undef)	= SvREFCNT_IMMORTAL;
    SvFLAGS(&PL_sv_undef)	= SVf_READONLY|SVf_PROTECT|SVt_NULL;
    SvANY(&PL_sv_undef)		= NULL;

    SvANY(&PL_sv_no)		= new_XPVNV();
    SvREFCNT(&PL_sv_no)		= SvREFCNT_IMMORTAL;
    SvFLAGS(&PL_sv_no)		= SVt_PVNV|SVf_READONLY|SVf_PROTECT
                                  |SVp_IOK|SVf_IOK|SVp_NOK|SVf_NOK
                                  |SVp_POK|SVf_POK|SVf_IsCOW|SVppv_STATIC;

    SvANY(&PL_sv_yes)		= new_XPVNV();
    SvREFCNT(&PL_sv_yes)	= SvREFCNT_IMMORTAL;
    SvFLAGS(&PL_sv_yes)		= SVt_PVNV|SVf_READONLY|SVf_PROTECT
                                  |SVp_IOK|SVf_IOK|SVp_NOK|SVf_NOK
                                  |SVp_POK|SVf_POK|SVf_IsCOW|SVppv_STATIC;

    SvANY(&PL_sv_zero)		= new_XPVNV();
    SvREFCNT(&PL_sv_zero)	= SvREFCNT_IMMORTAL;
    SvFLAGS(&PL_sv_zero)	= SVt_PVNV|SVf_READONLY|SVf_PROTECT
                                  |SVp_IOK|SVf_IOK|SVp_NOK|SVf_NOK
                                  |SVp_POK|SVf_POK
                                  |SVs_PADTMP;

    SvPV_set(&PL_sv_no, (char*)PL_No);
    SvCUR_set(&PL_sv_no, 0);
    SvLEN_set(&PL_sv_no, 0);
    SvIV_set(&PL_sv_no, 0);
    SvNV_set(&PL_sv_no, 0);

    SvPV_set(&PL_sv_yes, (char*)PL_Yes);
    SvCUR_set(&PL_sv_yes, 1);
    SvLEN_set(&PL_sv_yes, 0);
    SvIV_set(&PL_sv_yes, 1);
    SvNV_set(&PL_sv_yes, 1);

    SvPV_set(&PL_sv_zero, (char*)PL_Zero);
    SvCUR_set(&PL_sv_zero, 1);
    SvLEN_set(&PL_sv_zero, 0);
    SvIV_set(&PL_sv_zero, 0);
    SvNV_set(&PL_sv_zero, 0);

    PadnamePV(&PL_padname_const) = (char *)PL_No;

    assert(SvIMMORTAL_INTERP(&PL_sv_yes));
    assert(SvIMMORTAL_INTERP(&PL_sv_undef));
    assert(SvIMMORTAL_INTERP(&PL_sv_no));
    assert(SvIMMORTAL_INTERP(&PL_sv_zero));

    assert(SvIMMORTAL(&PL_sv_yes));
    assert(SvIMMORTAL(&PL_sv_undef));
    assert(SvIMMORTAL(&PL_sv_no));
    assert(SvIMMORTAL(&PL_sv_zero));

    assert( SvIMMORTAL_TRUE(&PL_sv_yes));
    assert(!SvIMMORTAL_TRUE(&PL_sv_undef));
    assert(!SvIMMORTAL_TRUE(&PL_sv_no));
    assert(!SvIMMORTAL_TRUE(&PL_sv_zero));

    assert( SvTRUE_nomg_NN(&PL_sv_yes));
    assert(!SvTRUE_nomg_NN(&PL_sv_undef));
    assert(!SvTRUE_nomg_NN(&PL_sv_no));
    assert(!SvTRUE_nomg_NN(&PL_sv_zero));
}

/*
=for apidoc_section $unicode

=for apidoc sv_recode_to_utf8

C<encoding> is assumed to be an C<Encode> object, on entry the PV
of C<sv> is assumed to be octets in that encoding, and C<sv>
will be converted into Unicode (and UTF-8).

If C<sv> already is UTF-8 (or if it is not C<POK>), or if C<encoding>
is not a reference, nothing is done to C<sv>.  If C<encoding> is not
an C<Encode::XS> Encoding object, bad things will happen.
(See L<encoding> and L<Encode>.)

The PV of C<sv> is returned.

=cut */

char *
Perl_sv_recode_to_utf8(pTHX_ SV *sv, SV *encoding)
{
    PERL_ARGS_ASSERT_SV_RECODE_TO_UTF8;

    if (SvPOK(sv) && !SvUTF8(sv) && !IN_BYTES && SvROK(encoding)) {
        SV *uni;
        STRLEN len;
        const char *s;
        dSP;
        SV *nsv = sv;
        ENTER;
        PUSHSTACK;
        SAVETMPS;
        if (SvPADTMP(nsv)) {
            nsv = sv_newmortal();
            SvSetSV_nosteal(nsv, sv);
        }
        save_re_context();
        PUSHMARK(sp);
        EXTEND(SP, 3);
        PUSHs(encoding);
        PUSHs(nsv);
/*
  NI-S 2002/07/09
  Passing sv_yes is wrong - it needs to be or'ed set of constants
  for Encode::XS, while UTf-8 decode (currently) assumes a true value means
  remove converted chars from source.

  Both will default the value - let them.

        XPUSHs(&PL_sv_yes);
*/
        PUTBACK;
        call_method("decode", G_SCALAR);
        SPAGAIN;
        uni = POPs;
        PUTBACK;
        s = SvPV_const(uni, len);
        if (s != SvPVX_const(sv)) {
            SvGROW(sv, len + 1);
            Move(s, SvPVX(sv), len + 1, char);
            SvCUR_set(sv, len);
        }
        FREETMPS;
        POPSTACK;
        LEAVE;
        if (SvTYPE(sv) >= SVt_PVMG && SvMAGIC(sv)) {
            /* clear pos and any utf8 cache */
            MAGIC * mg = mg_find(sv, PERL_MAGIC_regex_global);
            if (mg)
                mg->mg_len = -1;
            if ((mg = mg_find(sv, PERL_MAGIC_utf8)))
                magic_setutf8(sv,mg); /* clear UTF8 cache */
        }
        SvUTF8_on(sv);
        return SvPVX(sv);
    }
    return SvPOKp(sv) ? SvPVX(sv) : NULL;
}

/*
=for apidoc sv_cat_decode

C<encoding> is assumed to be an C<Encode> object, the PV of C<ssv> is
assumed to be octets in that encoding and decoding the input starts
from the position which S<C<(PV + *offset)>> pointed to.  C<dsv> will be
concatenated with the decoded UTF-8 string from C<ssv>.  Decoding will terminate
when the string C<tstr> appears in decoding output or the input ends on
the PV of C<ssv>.  The value which C<offset> points will be modified
to the last input position on C<ssv>.

Returns TRUE if the terminator was found, else returns FALSE.

=cut */

bool
Perl_sv_cat_decode(pTHX_ SV *dsv, SV *encoding,
                   SV *ssv, int *offset, char *tstr, int tlen)
{
    bool ret = FALSE;

    PERL_ARGS_ASSERT_SV_CAT_DECODE;

    if (SvPOK(ssv) && SvPOK(dsv) && SvROK(encoding)) {
        SV *offsv;
        dSP;
        ENTER;
        SAVETMPS;
        save_re_context();
        PUSHMARK(sp);
        EXTEND(SP, 6);
        PUSHs(encoding);
        PUSHs(dsv);
        PUSHs(ssv);
        offsv = newSViv(*offset);
        mPUSHs(offsv);
        mPUSHp(tstr, tlen);
        PUTBACK;
        call_method("cat_decode", G_SCALAR);
        SPAGAIN;
        ret = SvTRUE(TOPs);
        *offset = SvIV(offsv);
        PUTBACK;
        FREETMPS;
        LEAVE;
    }
    else
        Perl_croak(aTHX_ "Invalid argument to sv_cat_decode");
    return ret;

}

/* ---------------------------------------------------------------------
 *
 * support functions for report_uninit()
 */

/* the maxiumum size of array or hash where we will scan looking
 * for the undefined element that triggered the warning */

#define FUV_MAX_SEARCH_SIZE 1000

/* Look for an entry in the hash whose value has the same SV as val;
 * If so, return a mortal copy of the key. */

STATIC SV*
S_find_hash_subscript(pTHX_ const HV *const hv, const SV *const val)
{
    HE **array;
    I32 i;

    PERL_ARGS_ASSERT_FIND_HASH_SUBSCRIPT;

    if (!hv || SvMAGICAL(hv) || !HvTOTALKEYS(hv) ||
                        (HvTOTALKEYS(hv) > FUV_MAX_SEARCH_SIZE))
        return NULL;

    if (val == &PL_sv_undef || val == &PL_sv_placeholder)
        return NULL;

    array = HvARRAY(hv);

    for (i=HvMAX(hv); i>=0; i--) {
        HE *entry;
        for (entry = array[i]; entry; entry = HeNEXT(entry)) {
            if (HeVAL(entry) == val)
                return newSVhek_mortal(HeKEY_hek(entry));
        }
    }
    return NULL;
}

/* Look for an entry in the array whose value has the same SV as val;
 * If so, return the index, otherwise return -1. */

STATIC SSize_t
S_find_array_subscript(pTHX_ const AV *const av, const SV *const val)
{
    PERL_ARGS_ASSERT_FIND_ARRAY_SUBSCRIPT;

    if (!av || SvMAGICAL(av) || !AvARRAY(av) ||
                        (AvFILLp(av) > FUV_MAX_SEARCH_SIZE))
        return -1;

    if (val != &PL_sv_undef) {
        SV ** const svp = AvARRAY(av);
        SSize_t i;

        for (i=AvFILLp(av); i>=0; i--)
            if (svp[i] == val)
                return i;
    }
    return -1;
}

/* varname(): return the name of a variable, optionally with a subscript.
 * If gv is non-zero, use the name of that global, along with gvtype (one
 * of "$", "@", "%"); otherwise use the name of the lexical at pad offset
 * targ.  Depending on the value of the subscript_type flag, return:
 */

#define FUV_SUBSCRIPT_NONE	1	/* "@foo"          */
#define FUV_SUBSCRIPT_ARRAY	2	/* "$foo[aindex]"  */
#define FUV_SUBSCRIPT_HASH	3	/* "$foo{keyname}" */
#define FUV_SUBSCRIPT_WITHIN	4	/* "within @foo"   */

SV*
Perl_varname(pTHX_ const GV *const gv, const char gvtype, PADOFFSET targ,
        const SV *const keyname, SSize_t aindex, int subscript_type)
{

    SV * const name = sv_newmortal();
    if (gv && isGV(gv)) {
        char buffer[2];
        buffer[0] = gvtype;
        buffer[1] = 0;

        /* as gv_fullname4(), but add literal '^' for $^FOO names  */

        gv_fullname4(name, gv, buffer, 0);

        if ((unsigned int)SvPVX(name)[1] <= 26) {
            buffer[0] = '^';
            buffer[1] = SvPVX(name)[1] + 'A' - 1;

            /* Swap the 1 unprintable control character for the 2 byte pretty
               version - ie substr($name, 1, 1) = $buffer; */
            sv_insert(name, 1, 1, buffer, 2);
        }
    }
    else {
        CV * const cv = gv ? ((CV *)gv) : find_runcv(NULL);
        PADNAME *sv;

        assert(!cv || SvTYPE(cv) == SVt_PVCV || SvTYPE(cv) == SVt_PVFM);

        if (!cv || !CvPADLIST(cv))
            return NULL;
        sv = padnamelist_fetch(PadlistNAMES(CvPADLIST(cv)), targ);
        sv_setpvn(name, PadnamePV(sv), PadnameLEN(sv));
        SvUTF8_on(name);
    }

    if (subscript_type == FUV_SUBSCRIPT_HASH) {
        SV * const sv = newSV_type(SVt_NULL);
        STRLEN len;
        const char * const pv = SvPV_nomg_const((SV*)keyname, len);

        *SvPVX(name) = '$';
        Perl_sv_catpvf(aTHX_ name, "{%s}",
            pv_pretty(sv, pv, len, 32, NULL, NULL,
                    PERL_PV_PRETTY_DUMP | PERL_PV_ESCAPE_UNI_DETECT ));
        SvREFCNT_dec_NN(sv);
    }
    else if (subscript_type == FUV_SUBSCRIPT_ARRAY) {
        *SvPVX(name) = '$';
        Perl_sv_catpvf(aTHX_ name, "[%" IVdf "]", (IV)aindex);
    }
    else if (subscript_type == FUV_SUBSCRIPT_WITHIN) {
        /* We know that name has no magic, so can use 0 instead of SV_GMAGIC */
        Perl_sv_insert_flags(aTHX_ name, 0, 0,  STR_WITH_LEN("within "), 0);
    }

    return name;
}


/*
=apidoc_section $warning
=for apidoc find_uninit_var

Find the name of the undefined variable (if any) that caused the operator
to issue a "Use of uninitialized value" warning.
If match is true, only return a name if its value matches C<uninit_sv>.
So roughly speaking, if a unary operator (such as C<OP_COS>) generates a
warning, then following the direct child of the op may yield an
C<OP_PADSV> or C<OP_GV> that gives the name of the undefined variable.  On the
other hand, with C<OP_ADD> there are two branches to follow, so we only print
the variable name if we get an exact match.
C<desc_p> points to a string pointer holding the description of the op.
This may be updated if needed.

The name is returned as a mortal SV.

Assumes that C<PL_op> is the OP that originally triggered the error, and that
C<PL_comppad>/C<PL_curpad> points to the currently executing pad.

=cut
*/

STATIC SV *
S_find_uninit_var(pTHX_ const OP *const obase, const SV *const uninit_sv,
                  bool match, const char **desc_p)
{
    SV *sv;
    const GV *gv;
    const OP *o, *o2, *kid;

    PERL_ARGS_ASSERT_FIND_UNINIT_VAR;

    if (!obase || (match && (!uninit_sv || uninit_sv == &PL_sv_undef ||
                            uninit_sv == &PL_sv_placeholder)))
        return NULL;

    switch (obase->op_type) {

    case OP_UNDEF:
        /* the optimizer rewrites '$x = undef' to 'undef $x' for lexical
         * variables, which can occur as the source of warnings:
         *   ($x = undef) =~ s/a/b/;
         * The OPpUNDEF_KEEP_PV flag indicates that this used to be an
         * assignment op.
         * Otherwise undef should not care if its args are undef - any warnings
         * will be from tied/magic vars */
        if (
            (obase->op_private & (OPpTARGET_MY | OPpUNDEF_KEEP_PV)) == (OPpTARGET_MY | OPpUNDEF_KEEP_PV)
            && (!match || PAD_SVl(obase->op_targ) == uninit_sv)
        ) {
            return varname(NULL, '$', obase->op_targ, NULL, 0, FUV_SUBSCRIPT_NONE);
        }
        break;

    case OP_RV2AV:
    case OP_RV2HV:
    case OP_PADAV:
    case OP_PADHV:
      {
        const bool pad  = (    obase->op_type == OP_PADAV
                            || obase->op_type == OP_PADHV
                            || obase->op_type == OP_PADRANGE
                          );

        const bool hash = (    obase->op_type == OP_PADHV
                            || obase->op_type == OP_RV2HV
                            || (obase->op_type == OP_PADRANGE
                                && SvTYPE(PAD_SVl(obase->op_targ)) == SVt_PVHV)
                          );
        SSize_t index = 0;
        SV *keysv = NULL;
        int subscript_type = FUV_SUBSCRIPT_WITHIN;

        if (pad) { /* @lex, %lex */
            sv = PAD_SVl(obase->op_targ);
            gv = NULL;
        }
        else {
            if (cUNOPx(obase)->op_first->op_type == OP_GV) {
            /* @global, %global */
                gv = cGVOPx_gv(cUNOPx(obase)->op_first);
                if (!gv)
                    break;
                sv = hash ? MUTABLE_SV(GvHV(gv)): MUTABLE_SV(GvAV(gv));
            }
            else if (obase == PL_op) /* @{expr}, %{expr} */
                return find_uninit_var(cUNOPx(obase)->op_first,
                                                uninit_sv, match, desc_p);
            else /* @{expr}, %{expr} as a sub-expression */
                return NULL;
        }

        /* attempt to find a match within the aggregate */
        if (hash) {
            keysv = find_hash_subscript((const HV*)sv, uninit_sv);
            if (keysv)
                subscript_type = FUV_SUBSCRIPT_HASH;
        }
        else {
            index = find_array_subscript((const AV *)sv, uninit_sv);
            if (index >= 0)
                subscript_type = FUV_SUBSCRIPT_ARRAY;
        }

        if (match && subscript_type == FUV_SUBSCRIPT_WITHIN)
            break;

        return varname(gv, (char)(hash ? '%' : '@'), obase->op_targ,
                                    keysv, index, subscript_type);
      }

    case OP_RV2SV:
        if (cUNOPx(obase)->op_first->op_type == OP_GV) {
            /* $global */
            gv = cGVOPx_gv(cUNOPx(obase)->op_first);
            if (!gv || !GvSTASH(gv))
                break;
            if (match && (GvSV(gv) != uninit_sv))
                break;
            return varname(gv, '$', 0, NULL, 0, FUV_SUBSCRIPT_NONE);
        }
        /* ${expr} */
        return find_uninit_var(cUNOPx(obase)->op_first, uninit_sv, 1, desc_p);

    case OP_PADSV:
        if (match && PAD_SVl(obase->op_targ) != uninit_sv)
            break;
        return varname(NULL, '$', obase->op_targ,
                                    NULL, 0, FUV_SUBSCRIPT_NONE);

    case OP_PADSV_STORE:
        if (match && PAD_SVl(obase->op_targ) != uninit_sv)
            goto do_op;
        return varname(NULL, '$', obase->op_targ,
                                    NULL, 0, FUV_SUBSCRIPT_NONE);

    case OP_GVSV:
        gv = cGVOPx_gv(obase);
        if (!gv || (match && GvSV(gv) != uninit_sv) || !GvSTASH(gv))
            break;
        return varname(gv, '$', 0, NULL, 0, FUV_SUBSCRIPT_NONE);

    case OP_AELEMFAST_LEX:
        if (match) {
            SV **svp;
            AV *av = MUTABLE_AV(PAD_SV(obase->op_targ));
            if (!av || SvRMAGICAL(av))
                break;
            svp = av_fetch(av, (I8)obase->op_private, FALSE);
            if (!svp || *svp != uninit_sv)
                break;
        }
        return varname(NULL, '$', obase->op_targ,
                       NULL, (I8)obase->op_private, FUV_SUBSCRIPT_ARRAY);

    case OP_AELEMFASTLEX_STORE:
        if (match) {
            SV **svp;
            AV *av = MUTABLE_AV(PAD_SV(obase->op_targ));
            if (!av || SvRMAGICAL(av))
                goto do_op;
            svp = av_fetch(av, (I8)obase->op_private, FALSE);
            if (!svp || *svp != uninit_sv)
                goto do_op;
        }
        return varname(NULL, '$', obase->op_targ,
                       NULL, (I8)obase->op_private, FUV_SUBSCRIPT_ARRAY);

    case OP_AELEMFAST:
        {
            gv = cGVOPx_gv(obase);
            if (!gv)
                break;
            if (match) {
                SV **svp;
                AV *const av = GvAV(gv);
                if (!av || SvRMAGICAL(av))
                    break;
                svp = av_fetch(av, (I8)obase->op_private, FALSE);
                if (!svp || *svp != uninit_sv)
                    break;
            }
            return varname(gv, '$', 0,
                    NULL, (I8)obase->op_private, FUV_SUBSCRIPT_ARRAY);
        }
        NOT_REACHED; /* NOTREACHED */

    case OP_EXISTS:
        o = cUNOPx(obase)->op_first;
        if (!o || o->op_type != OP_NULL ||
                ! (o->op_targ == OP_AELEM || o->op_targ == OP_HELEM))
            break;
        return find_uninit_var(cBINOPo->op_last, uninit_sv, match, desc_p);

    case OP_AELEM:
    case OP_HELEM:
    {
        bool negate = FALSE;

        if (PL_op == obase)
            /* $a[uninit_expr] or $h{uninit_expr} */
            return find_uninit_var(cBINOPx(obase)->op_last,
                                                uninit_sv, match, desc_p);

        gv = NULL;
        o = cBINOPx(obase)->op_first;
        kid = cBINOPx(obase)->op_last;

        /* get the av or hv, and optionally the gv */
        sv = NULL;
        if  (o->op_type == OP_PADAV || o->op_type == OP_PADHV) {
            sv = PAD_SV(o->op_targ);
        }
        else if ((o->op_type == OP_RV2AV || o->op_type == OP_RV2HV)
                && cUNOPo->op_first->op_type == OP_GV)
        {
            gv = cGVOPx_gv(cUNOPo->op_first);
            if (!gv)
                break;
            sv = o->op_type
                == OP_RV2HV ? MUTABLE_SV(GvHV(gv)) : MUTABLE_SV(GvAV(gv));
        }
        if (!sv)
            break;

        if (kid && kid->op_type == OP_NEGATE) {
            negate = TRUE;
            kid = cUNOPx(kid)->op_first;
        }

        if (kid && kid->op_type == OP_CONST && SvOK(cSVOPx_sv(kid))) {
            /* index is constant */
            SV* kidsv;
            if (negate) {
                kidsv = newSVpvs_flags("-", SVs_TEMP);
                sv_catsv(kidsv, cSVOPx_sv(kid));
            }
            else
                kidsv = cSVOPx_sv(kid);
            if (match) {
                if (SvMAGICAL(sv))
                    break;
                if (obase->op_type == OP_HELEM) {
                    HE* he = hv_fetch_ent(MUTABLE_HV(sv), kidsv, 0, 0);
                    if (!he || HeVAL(he) != uninit_sv)
                        break;
                }
                else {
                    SV * const  opsv = cSVOPx_sv(kid);
                    const IV  opsviv = SvIV(opsv);
                    SV * const * const svp = av_fetch(MUTABLE_AV(sv),
                        negate ? - opsviv : opsviv,
                        FALSE);
                    if (!svp || *svp != uninit_sv)
                        break;
                }
            }
            if (obase->op_type == OP_HELEM)
                return varname(gv, '%', o->op_targ,
                            kidsv, 0, FUV_SUBSCRIPT_HASH);
            else
                return varname(gv, '@', o->op_targ, NULL,
                    negate ? - SvIV(cSVOPx_sv(kid)) : SvIV(cSVOPx_sv(kid)),
                    FUV_SUBSCRIPT_ARRAY);
        }
        else {
            /* index is an expression;
             * attempt to find a match within the aggregate */
            if (obase->op_type == OP_HELEM) {
                SV * const keysv = find_hash_subscript((const HV*)sv, uninit_sv);
                if (keysv)
                    return varname(gv, '%', o->op_targ,
                                                keysv, 0, FUV_SUBSCRIPT_HASH);
            }
            else {
                const SSize_t index
                    = find_array_subscript((const AV *)sv, uninit_sv);
                if (index >= 0)
                    return varname(gv, '@', o->op_targ,
                                        NULL, index, FUV_SUBSCRIPT_ARRAY);
            }
            if (match)
                break;
            return varname(gv,
                (char)((o->op_type == OP_PADAV || o->op_type == OP_RV2AV)
                ? '@' : '%'),
                o->op_targ, NULL, 0, FUV_SUBSCRIPT_WITHIN);
        }
        NOT_REACHED; /* NOTREACHED */
    }

    case OP_MULTIDEREF: {
        /* If we were executing OP_MULTIDEREF when the undef warning
         * triggered, then it must be one of the index values within
         * that triggered it. If not, then the only possibility is that
         * the value retrieved by the last aggregate index might be the
         * culprit. For the former, we set PL_multideref_pc each time before
         * using an index, so work though the item list until we reach
         * that point. For the latter, just work through the entire item
         * list; the last aggregate retrieved will be the candidate.
         * There is a third rare possibility: something triggered
         * magic while fetching an array/hash element. Just display
         * nothing in this case.
         */

        /* the named aggregate, if any */
        PADOFFSET agg_targ = 0;
        GV       *agg_gv   = NULL;
        /* the last-seen index */
        UV        index_type;
        PADOFFSET index_targ;
        GV       *index_gv;
        IV        index_const_iv = 0; /* init for spurious compiler warn */
        SV       *index_const_sv;
        int       depth = 0;  /* how many array/hash lookups we've done */

        UNOP_AUX_item *items = cUNOP_AUXx(obase)->op_aux;
        UNOP_AUX_item *last = NULL;
        UV actions = items->uv;
        bool is_hv;

        if (PL_op == obase) {
            last = PL_multideref_pc;
            assert(last >= items && last <= items + items[-1].uv);
        }

        assert(actions);

        while (1) {
            is_hv = FALSE;
            switch (actions & MDEREF_ACTION_MASK) {

            case MDEREF_reload:
                actions = (++items)->uv;
                continue;

            case MDEREF_HV_padhv_helem:               /* $lex{...} */
                is_hv = TRUE;
                /* FALLTHROUGH */
            case MDEREF_AV_padav_aelem:               /* $lex[...] */
                agg_targ = (++items)->pad_offset;
                agg_gv = NULL;
                break;

            case MDEREF_HV_gvhv_helem:                /* $pkg{...} */
                is_hv = TRUE;
                /* FALLTHROUGH */
            case MDEREF_AV_gvav_aelem:                /* $pkg[...] */
                agg_targ = 0;
                agg_gv = (GV*)UNOP_AUX_item_sv(++items);
                assert(isGV_with_GP(agg_gv));
                break;

            case MDEREF_HV_gvsv_vivify_rv2hv_helem:   /* $pkg->{...} */
            case MDEREF_HV_padsv_vivify_rv2hv_helem:  /* $lex->{...} */
                ++items;
                /* FALLTHROUGH */
            case MDEREF_HV_pop_rv2hv_helem:           /* expr->{...} */
            case MDEREF_HV_vivify_rv2hv_helem:        /* vivify, ->{...} */
                agg_targ = 0;
                agg_gv   = NULL;
                is_hv    = TRUE;
                break;

            case MDEREF_AV_gvsv_vivify_rv2av_aelem:   /* $pkg->[...] */
            case MDEREF_AV_padsv_vivify_rv2av_aelem:  /* $lex->[...] */
                ++items;
                /* FALLTHROUGH */
            case MDEREF_AV_pop_rv2av_aelem:           /* expr->[...] */
            case MDEREF_AV_vivify_rv2av_aelem:        /* vivify, ->[...] */
                agg_targ = 0;
                agg_gv   = NULL;
            } /* switch */

            index_targ     = 0;
            index_gv       = NULL;
            index_const_sv = NULL;

            index_type = (actions & MDEREF_INDEX_MASK);
            switch (index_type) {
            case MDEREF_INDEX_none:
                break;
            case MDEREF_INDEX_const:
                if (is_hv)
                    index_const_sv = UNOP_AUX_item_sv(++items)
                else
                    index_const_iv = (++items)->iv;
                break;
            case MDEREF_INDEX_padsv:
                index_targ = (++items)->pad_offset;
                break;
            case MDEREF_INDEX_gvsv:
                index_gv = (GV*)UNOP_AUX_item_sv(++items);
                assert(isGV_with_GP(index_gv));
                break;
            }

            if (index_type != MDEREF_INDEX_none)
                depth++;

            if (   index_type == MDEREF_INDEX_none
                || (actions & MDEREF_FLAG_last)
                || (last && items >= last)
            )
                break;

            actions >>= MDEREF_SHIFT;
        } /* while */

        if (PL_op == obase) {
            /* most likely index was undef */

            *desc_p = (    (actions & MDEREF_FLAG_last)
                        && (obase->op_private
                                & (OPpMULTIDEREF_EXISTS|OPpMULTIDEREF_DELETE)))
                        ?
                            (obase->op_private & OPpMULTIDEREF_EXISTS)
                                ? "exists"
                                : "delete"
                        : is_hv ? "hash element" : "array element";
            assert(index_type != MDEREF_INDEX_none);
            if (index_gv) {
                if (GvSV(index_gv) == uninit_sv)
                    return varname(index_gv, '$', 0, NULL, 0,
                                                    FUV_SUBSCRIPT_NONE);
                else
                    return NULL;
            }
            if (index_targ) {
                if (PL_curpad[index_targ] == uninit_sv)
                    return varname(NULL, '$', index_targ,
                                    NULL, 0, FUV_SUBSCRIPT_NONE);
                else
                    return NULL;
            }
            /* If we got to this point it was undef on a const subscript,
             * so magic probably involved, e.g. $ISA[0]. Give up. */
            return NULL;
        }

        /* the SV returned by pp_multideref() was undef, if anything was */

        if (depth != 1)
            break;

        if (agg_targ)
            sv = PAD_SV(agg_targ);
        else if (agg_gv) {
            sv = is_hv ? MUTABLE_SV(GvHV(agg_gv)) : MUTABLE_SV(GvAV(agg_gv));
            if (!sv)
                break;
            }
        else
            break;

        if (index_type == MDEREF_INDEX_const) {
            if (match) {
                if (SvMAGICAL(sv))
                    break;
                if (is_hv) {
                    HE* he = hv_fetch_ent(MUTABLE_HV(sv), index_const_sv, 0, 0);
                    if (!he || HeVAL(he) != uninit_sv)
                        break;
                }
                else {
                    SV * const * const svp =
                            av_fetch(MUTABLE_AV(sv), index_const_iv, FALSE);
                    if (!svp || *svp != uninit_sv)
                        break;
                }
            }
            return is_hv
                ? varname(agg_gv, '%', agg_targ,
                                index_const_sv, 0,    FUV_SUBSCRIPT_HASH)
                : varname(agg_gv, '@', agg_targ,
                                NULL, index_const_iv, FUV_SUBSCRIPT_ARRAY);
        }
        else {
            /* index is an var */
            if (is_hv) {
                SV * const keysv = find_hash_subscript((const HV*)sv, uninit_sv);
                if (keysv)
                    return varname(agg_gv, '%', agg_targ,
                                                keysv, 0, FUV_SUBSCRIPT_HASH);
            }
            else {
                const SSize_t index
                    = find_array_subscript((const AV *)sv, uninit_sv);
                if (index >= 0)
                    return varname(agg_gv, '@', agg_targ,
                                        NULL, index, FUV_SUBSCRIPT_ARRAY);
            }
            /* look for an element not found */
            if (!SvMAGICAL(sv)) {
                SV *index_sv = NULL;
                if (index_targ) {
                    index_sv = PL_curpad[index_targ];
                }
                else if (index_gv) {
                    index_sv = GvSV(index_gv);
                }
                if (index_sv && !SvMAGICAL(index_sv) && !SvROK(index_sv)) {
                    if (is_hv) {
                        SV *report_index_sv = SvOK(index_sv) ? index_sv : &PL_sv_no;
                        HE *he = hv_fetch_ent(MUTABLE_HV(sv), report_index_sv, 0, 0);
                        if (!he) {
                            return varname(agg_gv, '%', agg_targ,
                                           report_index_sv, 0, FUV_SUBSCRIPT_HASH);
                        }
                    }
                    else {
                        SSize_t index = SvOK(index_sv) ? SvIV(index_sv) : 0;
                        SV * const * const svp =
                            av_fetch(MUTABLE_AV(sv), index, FALSE);
                        if (!svp) {
                            return varname(agg_gv, '@', agg_targ,
                                           NULL, index, FUV_SUBSCRIPT_ARRAY);
                        }
                    }
                }
            }
            if (match)
                break;
            return varname(agg_gv,
                is_hv ? '%' : '@',
                agg_targ, NULL, 0, FUV_SUBSCRIPT_WITHIN);
        }
        NOT_REACHED; /* NOTREACHED */
    }

    case OP_AASSIGN:
        /* only examine RHS */
        return find_uninit_var(cBINOPx(obase)->op_first, uninit_sv,
                                                                match, desc_p);

    case OP_OPEN:
        o = cUNOPx(obase)->op_first;
        if (   o->op_type == OP_PUSHMARK
           || (o->op_type == OP_NULL && o->op_targ == OP_PUSHMARK)
        )
            o = OpSIBLING(o);

        if (!OpHAS_SIBLING(o)) {
            /* one-arg version of open is highly magical */

            if (o->op_type == OP_GV) { /* open FOO; */
                gv = cGVOPx_gv(o);
                if (match && GvSV(gv) != uninit_sv)
                    break;
                return varname(gv, '$', 0,
                            NULL, 0, FUV_SUBSCRIPT_NONE);
            }
            /* other possibilities not handled are:
             * open $x; or open my $x;	should return '${*$x}'
             * open expr;		should return '$'.expr ideally
             */
             break;
        }
        match = 1;
        goto do_op;

    /* ops where $_ may be an implicit arg */
    case OP_TRANS:
    case OP_TRANSR:
    case OP_SUBST:
    case OP_MATCH:
        if ( !(obase->op_flags & OPf_STACKED)) {
            if (uninit_sv == DEFSV)
                return newSVpvs_flags("$_", SVs_TEMP);
            else if (obase->op_targ
                  && uninit_sv == PAD_SVl(obase->op_targ))
                return varname(NULL, '$', obase->op_targ, NULL, 0,
                               FUV_SUBSCRIPT_NONE);
        }
        goto do_op;

    case OP_PRTF:
    case OP_PRINT:
    case OP_SAY:
        match = 1; /* print etc can return undef on defined args */
        /* skip filehandle as it can't produce 'undef' warning  */
        o = cUNOPx(obase)->op_first;
        if ((obase->op_flags & OPf_STACKED)
            &&
               (   o->op_type == OP_PUSHMARK
               || (o->op_type == OP_NULL && o->op_targ == OP_PUSHMARK)))
            o = OpSIBLING(OpSIBLING(o));
        goto do_op2;


    case OP_ENTEREVAL: /* could be eval $undef or $x='$undef'; eval $x */
    case OP_CUSTOM: /* XS or custom code could trigger random warnings */

        /* the following ops are capable of returning PL_sv_undef even for
         * defined arg(s) */

    case OP_BACKTICK:
    case OP_PIPE_OP:
    case OP_FILENO:
    case OP_BINMODE:
    case OP_TIED:
    case OP_GETC:
    case OP_SYSREAD:
    case OP_SEND:
    case OP_IOCTL:
    case OP_SOCKET:
    case OP_SOCKPAIR:
    case OP_BIND:
    case OP_CONNECT:
    case OP_LISTEN:
    case OP_ACCEPT:
    case OP_SHUTDOWN:
    case OP_SSOCKOPT:
    case OP_GETPEERNAME:
    case OP_FTRREAD:
    case OP_FTRWRITE:
    case OP_FTREXEC:
    case OP_FTROWNED:
    case OP_FTEREAD:
    case OP_FTEWRITE:
    case OP_FTEEXEC:
    case OP_FTEOWNED:
    case OP_FTIS:
    case OP_FTZERO:
    case OP_FTSIZE:
    case OP_FTFILE:
    case OP_FTDIR:
    case OP_FTLINK:
    case OP_FTPIPE:
    case OP_FTSOCK:
    case OP_FTBLK:
    case OP_FTCHR:
    case OP_FTTTY:
    case OP_FTSUID:
    case OP_FTSGID:
    case OP_FTSVTX:
    case OP_FTTEXT:
    case OP_FTBINARY:
    case OP_FTMTIME:
    case OP_FTATIME:
    case OP_FTCTIME:
    case OP_READLINK:
    case OP_OPEN_DIR:
    case OP_READDIR:
    case OP_TELLDIR:
    case OP_SEEKDIR:
    case OP_REWINDDIR:
    case OP_CLOSEDIR:
    case OP_GMTIME:
    case OP_ALARM:
    case OP_SEMGET:
    case OP_GETLOGIN:
    case OP_SUBSTR:
    case OP_AEACH:
    case OP_EACH:
    case OP_SORT:
    case OP_CALLER:
    case OP_DOFILE:
    case OP_PROTOTYPE:
    case OP_NCMP:
    case OP_SMARTMATCH:
    case OP_UNPACK:
    case OP_SYSOPEN:
    case OP_SYSSEEK:
        match = 1;
        goto do_op;

    case OP_ENTERSUB:
    case OP_GOTO:
        /* XXX tmp hack: these two may call an XS sub, and currently
          XS subs don't have a SUB entry on the context stack, so CV and
          pad determination goes wrong, and BAD things happen. So, just
          don't try to determine the value under those circumstances.
          Need a better fix at dome point. DAPM 11/2007 */
        break;

    case OP_FLIP:
    case OP_FLOP:
    {
        GV * const gv = gv_fetchpvs(".", GV_NOTQUAL, SVt_PV);
        if (gv && GvSV(gv) == uninit_sv)
            return newSVpvs_flags("$.", SVs_TEMP);
        goto do_op;
    }

    case OP_POS:
        /* def-ness of rval pos() is independent of the def-ness of its arg */
        if ( !(obase->op_flags & OPf_MOD))
            break;
        /* FALLTHROUGH */

    case OP_SCHOMP:
    case OP_CHOMP:
        if (SvROK(PL_rs) && uninit_sv == SvRV(PL_rs))
            return newSVpvs_flags("${$/}", SVs_TEMP);
        /* FALLTHROUGH */

    default:
    do_op:
        if (!(obase->op_flags & OPf_KIDS))
            break;
        o = cUNOPx(obase)->op_first;

    do_op2:
        if (!o)
            break;

        /* This loop checks all the kid ops, skipping any that cannot pos-
         * sibly be responsible for the uninitialized value; i.e., defined
         * constants and ops that return nothing.  If there is only one op
         * left that is not skipped, then we *know* it is responsible for
         * the uninitialized value.  If there is more than one op left, we
         * have to look for an exact match in the while() loop below.
         * Note that we skip padrange, because the individual pad ops that
         * it replaced are still in the tree, so we work on them instead.
         */
        o2 = NULL;
        for (kid=o; kid; kid = OpSIBLING(kid)) {
            const OPCODE type = kid->op_type;
            if ( (type == OP_CONST && SvOK(cSVOPx_sv(kid)))
              || (type == OP_NULL  && ! (kid->op_flags & OPf_KIDS))
              || (type == OP_PUSHMARK)
              || (type == OP_PADRANGE)
            )
            continue;

            if (o2) { /* more than one found */
                o2 = NULL;
                break;
            }
            o2 = kid;
        }
        if (o2)
            return find_uninit_var(o2, uninit_sv, match, desc_p);

        /* scan all args */
        while (o) {
            sv = find_uninit_var(o, uninit_sv, 1, desc_p);
            if (sv)
                return sv;
            o = OpSIBLING(o);
        }
        break;
    }
    return NULL;
}


/*
=for apidoc_section $warning
=for apidoc report_uninit

Print appropriate "Use of uninitialized variable" warning.

=cut
*/

void
Perl_report_uninit(pTHX_ const SV *uninit_sv)
{
    const char *desc = NULL;
    SV* varname = NULL;

    if (PL_op) {
        desc = PL_op->op_type == OP_STRINGIFY && PL_op->op_folded
                ? "join or string"
                : PL_op->op_type == OP_MULTICONCAT
                    && (PL_op->op_private & OPpMULTICONCAT_FAKE)
                ? "sprintf"
                : OP_DESC(PL_op);
        if (uninit_sv && PL_curpad) {
            varname = find_uninit_var(PL_op, uninit_sv, 0, &desc);
            if (varname)
                sv_insert(varname, 0, 0, " ", 1);
        }
    }
    else if (PL_curstackinfo->si_type == PERLSI_SORT && cxstack_ix == 0)
        /* we've reached the end of a sort block or sub,
         * and the uninit value is probably what that code returned */
        desc = "sort";

    /* PL_warn_uninit_sv is constant */
    GCC_DIAG_IGNORE_STMT(-Wformat-nonliteral);
    if (desc)
        /* diag_listed_as: Use of uninitialized value%s */
        Perl_warner(aTHX_ packWARN(WARN_UNINITIALIZED), PL_warn_uninit_sv,
                SVfARG(varname ? varname : &PL_sv_no),
                " in ", desc);
    else
        Perl_warner(aTHX_ packWARN(WARN_UNINITIALIZED), PL_warn_uninit,
                "", "", "");
    GCC_DIAG_RESTORE_STMT;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
