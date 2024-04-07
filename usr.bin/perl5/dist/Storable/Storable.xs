/* -*-  c-basic-offset: 4 -*-
 *
 *  Fast store and retrieve mechanism.
 *
 *  Copyright (c) 1995-2000, Raphael Manfredi
 *  Copyright (c) 2016, 2017 cPanel Inc
 *  Copyright (c) 2017 Reini Urban
 *
 *  You may redistribute only under the same terms as Perl 5, as specified
 *  in the README file that comes with the distribution.
 *
 */

#define PERL_NO_GET_CONTEXT     /* we want efficiency */
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#define NEED_sv_2pv_flags
#define NEED_load_module
#define NEED_vload_module
#define NEED_newCONSTSUB
#define NEED_newSVpvn_flags
#define NEED_newRV_noinc
#include "ppport.h"             /* handle old perls */

#ifdef DEBUGGING
#define DEBUGME /* Debug mode, turns assertions on as well */
#define DASSERT /* Assertion mode */
#endif

/*
 * Earlier versions of perl might be used, we can't assume they have the latest!
 */

/* perl <= 5.8.2 needs this */
#ifndef SvIsCOW
# define SvIsCOW(sv) 0
#endif

#ifndef HvRITER_set
#  define HvRITER_set(hv,r)	(HvRITER(hv) = r)
#endif
#ifndef HvEITER_set
#  define HvEITER_set(hv,r)	(HvEITER(hv) = r)
#endif

#ifndef HvRITER_get
#  define HvRITER_get           HvRITER
#endif
#ifndef HvEITER_get
#  define HvEITER_get           HvEITER
#endif

#ifndef HvPLACEHOLDERS_get
#  define HvPLACEHOLDERS_get    HvPLACEHOLDERS
#endif

#ifndef HvTOTALKEYS
#  define HvTOTALKEYS(hv)	HvKEYS(hv)
#endif
/* 5.6 */
#ifndef HvUSEDKEYS
#  define HvUSEDKEYS(hv)	HvKEYS(hv)
#endif

#ifdef SVf_IsCOW
#  define SvTRULYREADONLY(sv)	SvREADONLY(sv)
#else
#  define SvTRULYREADONLY(sv)	(SvREADONLY(sv) && !SvIsCOW(sv))
#endif

#ifndef strEQc
#  define strEQc(s,c) memEQ(s, ("" c ""), sizeof(c))
#endif

#if defined(HAS_FLOCK) || defined(FCNTL_CAN_LOCK) && defined(HAS_LOCKF)
#define CAN_FLOCK &PL_sv_yes
#else
#define CAN_FLOCK &PL_sv_no
#endif

#ifdef DEBUGME

#ifndef DASSERT
#define DASSERT
#endif

/*
 * TRACEME() will only output things when the $Storable::DEBUGME is true,
 * using the value traceme cached in the context.
 *
 *
 * TRACEMED() directly looks at the variable, for use before traceme has been
 * updated.
 */

#define TRACEME(x)                                            \
    STMT_START {					      \
        if (cxt->traceme)				      \
            { PerlIO_stdoutf x; PerlIO_stdoutf("\n"); }       \
    } STMT_END

#define TRACEMED(x)                                           \
    STMT_START {                                              \
        if (SvTRUE(get_sv("Storable::DEBUGME", GV_ADD)))      \
            { PerlIO_stdoutf x; PerlIO_stdoutf("\n"); }       \
    } STMT_END

#define INIT_TRACEME							\
    STMT_START {							\
	cxt->traceme = SvTRUE(get_sv("Storable::DEBUGME", GV_ADD));	\
    } STMT_END

#else
#define TRACEME(x)
#define TRACEMED(x)
#define INIT_TRACEME
#endif	/* DEBUGME */

#ifdef DASSERT
#define ASSERT(x,y)                                              \
    STMT_START {                                                 \
        if (!(x)) {                                              \
            PerlIO_stdoutf("ASSERT FAILED (\"%s\", line %d): ",  \
                           __FILE__, (int)__LINE__);             \
            PerlIO_stdoutf y; PerlIO_stdoutf("\n");              \
        }                                                        \
    } STMT_END
#else
#define ASSERT(x,y)
#endif

/*
 * Type markers.
 */

#define C(x) ((char) (x))	/* For markers with dynamic retrieval handling */

#define SX_OBJECT	C(0)	/* Already stored object */
#define SX_LSCALAR	C(1)	/* Scalar (large binary) follows (length, data) */
#define SX_ARRAY	C(2)	/* Array forthcoming (size, item list) */
#define SX_HASH		C(3)	/* Hash forthcoming (size, key/value pair list) */
#define SX_REF		C(4)	/* Reference to object forthcoming */
#define SX_UNDEF	C(5)	/* Undefined scalar */
#define SX_INTEGER	C(6)	/* Integer forthcoming */
#define SX_DOUBLE	C(7)	/* Double forthcoming */
#define SX_BYTE		C(8)	/* (signed) byte forthcoming */
#define SX_NETINT	C(9)	/* Integer in network order forthcoming */
#define SX_SCALAR	C(10)	/* Scalar (binary, small) follows (length, data) */
#define SX_TIED_ARRAY	C(11)	/* Tied array forthcoming */
#define SX_TIED_HASH	C(12)	/* Tied hash forthcoming */
#define SX_TIED_SCALAR	C(13)	/* Tied scalar forthcoming */
#define SX_SV_UNDEF	C(14)	/* Perl's immortal PL_sv_undef */
#define SX_SV_YES	C(15)	/* Perl's immortal PL_sv_yes */
#define SX_SV_NO	C(16)	/* Perl's immortal PL_sv_no */
#define SX_BLESS	C(17)	/* Object is blessed */
#define SX_IX_BLESS	C(18)	/* Object is blessed, classname given by index */
#define SX_HOOK		C(19)	/* Stored via hook, user-defined */
#define SX_OVERLOAD	C(20)	/* Overloaded reference */
#define SX_TIED_KEY	C(21)	/* Tied magic key forthcoming */
#define SX_TIED_IDX	C(22)	/* Tied magic index forthcoming */
#define SX_UTF8STR	C(23)	/* UTF-8 string forthcoming (small) */
#define SX_LUTF8STR	C(24)	/* UTF-8 string forthcoming (large) */
#define SX_FLAG_HASH	C(25)	/* Hash with flags forthcoming (size, flags, key/flags/value triplet list) */
#define SX_CODE         C(26)   /* Code references as perl source code */
#define SX_WEAKREF	C(27)	/* Weak reference to object forthcoming */
#define SX_WEAKOVERLOAD	C(28)	/* Overloaded weak reference */
#define SX_VSTRING	C(29)	/* vstring forthcoming (small) */
#define SX_LVSTRING	C(30)	/* vstring forthcoming (large) */
#define SX_SVUNDEF_ELEM	C(31)	/* array element set to &PL_sv_undef */
#define SX_REGEXP	C(32)	/* Regexp */
#define SX_LOBJECT	C(33)	/* Large object: string, array or hash (size >2G) */
#define SX_BOOLEAN_TRUE	C(34)	/* Boolean true */
#define SX_BOOLEAN_FALSE	C(35)	/* Boolean false */
#define SX_LAST		C(36)	/* invalid. marker only */

/*
 * Those are only used to retrieve "old" pre-0.6 binary images.
 */
#define SX_ITEM		'i'	/* An array item introducer */
#define SX_IT_UNDEF	'I'	/* Undefined array item */
#define SX_KEY		'k'	/* A hash key introducer */
#define SX_VALUE	'v'	/* A hash value introducer */
#define SX_VL_UNDEF	'V'	/* Undefined hash value */

/*
 * Those are only used to retrieve "old" pre-0.7 binary images
 */

#define SX_CLASS	'b'	/* Object is blessed, class name length <255 */
#define SX_LG_CLASS	'B'	/* Object is blessed, class name length >255 */
#define SX_STORED	'X'	/* End of object */

/*
 * Limits between short/long length representation.
 */

#define LG_SCALAR	255	/* Large scalar length limit */
#define LG_BLESS	127	/* Large classname bless limit */

/*
 * Operation types
 */

#define ST_STORE	0x1	/* Store operation */
#define ST_RETRIEVE	0x2	/* Retrieval operation */
#define ST_CLONE	0x4	/* Deep cloning operation */

/*
 * The following structure is used for hash table key retrieval. Since, when
 * retrieving objects, we'll be facing blessed hash references, it's best
 * to pre-allocate that buffer once and resize it as the need arises, never
 * freeing it (keys will be saved away someplace else anyway, so even large
 * keys are not enough a motivation to reclaim that space).
 *
 * This structure is also used for memory store/retrieve operations which
 * happen in a fixed place before being malloc'ed elsewhere if persistence
 * is required. Hence the aptr pointer.
 */
struct extendable {
    char *arena;	/* Will hold hash key strings, resized as needed */
    STRLEN asiz;	/* Size of aforementioned buffer */
    char *aptr;		/* Arena pointer, for in-place read/write ops */
    char *aend;		/* First invalid address */
};

/*
 * At store time:
 * A hash table records the objects which have already been stored.
 * Those are referred to as SX_OBJECT in the file, and their "tag" (i.e.
 * an arbitrary sequence number) is used to identify them.
 *
 * At retrieve time:
 * An array table records the objects which have already been retrieved,
 * as seen by the tag determined by counting the objects themselves. The
 * reference to that retrieved object is kept in the table, and is returned
 * when an SX_OBJECT is found bearing that same tag.
 *
 * The same processing is used to record "classname" for blessed objects:
 * indexing by a hash at store time, and via an array at retrieve time.
 */

typedef unsigned long stag_t;	/* Used by pre-0.6 binary format */

/*
 * Make the tag type 64-bit on 64-bit platforms.
 *
 * If the tag number is low enough it's stored as a 32-bit value, but
 * with very large arrays and hashes it's possible to go over 2**32
 * scalars.
 */

typedef STRLEN ntag_t;

/* used for where_is_undef - marks an unset value */
#define UNSET_NTAG_T (~(ntag_t)0)

/*
 * The following "thread-safe" related defines were contributed by
 * Murray Nesbitt <murray@activestate.com> and integrated by RAM, who
 * only renamed things a little bit to ensure consistency with surrounding
 * code.	-- RAM, 14/09/1999
 *
 * The original patch suffered from the fact that the stcxt_t structure
 * was global.  Murray tried to minimize the impact on the code as much as
 * possible.
 *
 * Starting with 0.7, Storable can be re-entrant, via the STORABLE_xxx hooks
 * on objects.  Therefore, the notion of context needs to be generalized,
 * threading or not.
 */

#define MY_VERSION "Storable(" XS_VERSION ")"


/*
 * Conditional UTF8 support.
 *
 */
#define STORE_UTF8STR(pv, len)	STORE_PV_LEN(pv, len, SX_UTF8STR, SX_LUTF8STR)
#define HAS_UTF8_SCALARS
#ifdef HeKUTF8
#define HAS_UTF8_HASHES
#define HAS_UTF8_ALL
#else
/* 5.6 perl has utf8 scalars but not hashes */
#endif
#ifndef HAS_UTF8_ALL
#define UTF8_CROAK() CROAK(("Cannot retrieve UTF8 data in non-UTF8 perl"))
#endif
#ifndef SvWEAKREF
#define WEAKREF_CROAK() CROAK(("Cannot retrieve weak references in this perl"))
#endif
#ifndef SvVOK
#define VSTRING_CROAK() CROAK(("Cannot retrieve vstring in this perl"))
#endif

#ifdef HvPLACEHOLDERS
#define HAS_RESTRICTED_HASHES
#else
#define HVhek_PLACEHOLD	0x200
#define RESTRICTED_HASH_CROAK() CROAK(("Cannot retrieve restricted hash"))
#endif

#ifdef HvHASKFLAGS
#define HAS_HASH_KEY_FLAGS
#endif

#ifdef ptr_table_new
#define USE_PTR_TABLE
#endif

/* do we need/want to clear padding on NVs? */
#if defined(LONG_DOUBLEKIND) && defined(USE_LONG_DOUBLE)
#  if LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_LITTLE_ENDIAN || \
      LONG_DOUBLEKIND == LONG_DOUBLE_IS_X86_80_BIT_BIG_ENDIAN
#    define NV_PADDING (NVSIZE - 10)
#  else
#    define NV_PADDING 0
#  endif
#else
/* This is kind of a guess - it means we'll get an unneeded clear on 128-bit NV
   but an upgraded perl will fix that
*/
#  if NVSIZE > 8
#    define NV_CLEAR
#  endif
#  define NV_PADDING 0
#endif

typedef union {
    NV nv;
    U8 bytes[sizeof(NV)];
} NV_bytes;

/* Needed for 32bit with lengths > 2G - 4G, and 64bit */
#if PTRSIZE > 4
#define HAS_U64
#endif

/*
 * Fields s_tainted and s_dirty are prefixed with s_ because Perl's include
 * files remap tainted and dirty when threading is enabled.  That's bad for
 * perl to remap such common words.	-- RAM, 29/09/00
 */

struct stcxt;
typedef struct stcxt {
    int entry;		/* flags recursion */
    int optype;		/* type of traversal operation */
    /* which objects have been seen, store time.
       tags are numbers, which are cast to (SV *) and stored directly */
#ifdef USE_PTR_TABLE
    /* use pseen if we have ptr_tables. We have to store tag+1, because
       tag numbers start at 0, and we can't store (SV *) 0 in a ptr_table
       without it being confused for a fetch lookup failure.  */
    struct ptr_tbl *pseen;
    /* Still need hseen for the 0.6 file format code. */
#endif
    HV *hseen;
    AV *hook_seen;		/* which SVs were returned by STORABLE_freeze() */
    AV *aseen;			/* which objects have been seen, retrieve time */
    ntag_t where_is_undef;		/* index in aseen of PL_sv_undef */
    HV *hclass;			/* which classnames have been seen, store time */
    AV *aclass;			/* which classnames have been seen, retrieve time */
    HV *hook;			/* cache for hook methods per class name */
    IV tagnum;			/* incremented at store time for each seen object */
    IV classnum;		/* incremented at store time for each seen classname */
    int netorder;		/* true if network order used */
    int s_tainted;		/* true if input source is tainted, at retrieve time */
    int forgive_me;		/* whether to be forgiving... */
    int deparse;		/* whether to deparse code refs */
    SV *eval;			/* whether to eval source code */
    int canonical;		/* whether to store hashes sorted by key */
#ifndef HAS_RESTRICTED_HASHES
    int derestrict;		/* whether to downgrade restricted hashes */
#endif
#ifndef HAS_UTF8_ALL
    int use_bytes;		/* whether to bytes-ify utf8 */
#endif
    int accept_future_minor;	/* croak immediately on future minor versions?  */
    int s_dirty;		/* context is dirty due to CROAK() -- can be cleaned */
    int membuf_ro;		/* true means membuf is read-only and msaved is rw */
    struct extendable keybuf;	/* for hash key retrieval */
    struct extendable membuf;	/* for memory store/retrieve operations */
    struct extendable msaved;	/* where potentially valid mbuf is saved */
    PerlIO *fio;		/* where I/O are performed, NULL for memory */
    int ver_major;		/* major of version for retrieved object */
    int ver_minor;		/* minor of version for retrieved object */
    SV *(**retrieve_vtbl)(pTHX_ struct stcxt *, const char *);	/* retrieve dispatch table */
    SV *prev;			/* contexts chained backwards in real recursion */
    SV *my_sv;			/* the blessed scalar who's SvPVX() I am */

    /* recur_sv:

       A hashref of hashrefs or arrayref of arrayrefs is actually a
       chain of four SVs, eg for an array ref containing an array ref:

         RV -> AV (element) -> RV -> AV

       To make this depth appear natural from a perl level we only
       want to count this as two levels, so store_ref() stores it's RV
       into recur_sv and store_array()/store_hash() will only count
       that level if the AV/HV *isn't* recur_sv.

       We can't just have store_hash()/store_array() not count that
       level, since it's possible for XS code to store an AV or HV
       directly as an element (though perl code trying to access such
       an object will generally croak.)
     */
    SV *recur_sv;               /* check only one recursive SV */
    int in_retrieve_overloaded; /* performance hack for retrieving overloaded objects */
    int flags;			/* controls whether to bless or tie objects */
    IV recur_depth;        	/* avoid stack overflows RT #97526 */
    IV max_recur_depth;        /* limit for recur_depth */
    IV max_recur_depth_hash;   /* limit for recur_depth for hashes */
#ifdef DEBUGME
    int traceme;                /* TRACEME() produces output */
#endif
} stcxt_t;

#define RECURSION_TOO_DEEP() \
    (cxt->max_recur_depth != -1 && ++cxt->recur_depth > cxt->max_recur_depth)

/* There's cases where we need to check whether the hash recursion
   limit has been reached without bumping the recursion levels, so the
   hash check doesn't bump the depth.
*/
#define RECURSION_TOO_DEEP_HASH() \
    (cxt->max_recur_depth_hash != -1 && cxt->recur_depth > cxt->max_recur_depth_hash)
#define MAX_DEPTH_ERROR "Max. recursion depth with nested structures exceeded"

static int storable_free(pTHX_ SV *sv, MAGIC* mg);

static MGVTBL vtbl_storable = {
    NULL, /* get */
    NULL, /* set */
    NULL, /* len */
    NULL, /* clear */
    storable_free,
#ifdef MGf_COPY
    NULL, /* copy */
#endif
#ifdef MGf_DUP
    NULL, /* dup */
#endif
#ifdef MGf_LOCAL
    NULL /* local */
#endif
};

/* From Digest::MD5.  */
#ifndef sv_magicext
# define sv_magicext(sv, obj, type, vtbl, name, namlen)         \
    THX_sv_magicext(aTHX_ sv, obj, type, vtbl, name, namlen)
static MAGIC *THX_sv_magicext(pTHX_
	SV *sv, SV *obj, int type,
	MGVTBL const *vtbl, char const *name, I32 namlen)
{
    MAGIC *mg;
    if (obj || namlen)
        /* exceeded intended usage of this reserve implementation */
        return NULL;
    Newxz(mg, 1, MAGIC);
    mg->mg_virtual = (MGVTBL*)vtbl;
    mg->mg_type = type;
    mg->mg_ptr = (char *)name;
    mg->mg_len = -1;
    (void) SvUPGRADE(sv, SVt_PVMG);
    mg->mg_moremagic = SvMAGIC(sv);
    SvMAGIC_set(sv, mg);
    SvMAGICAL_off(sv);
    mg_magical(sv);
    return mg;
}
#endif

#define NEW_STORABLE_CXT_OBJ(cxt)				\
    STMT_START {						\
        SV *self = newSV(sizeof(stcxt_t) - 1);                  \
        SV *my_sv = newRV_noinc(self);                          \
        sv_magicext(self, NULL, PERL_MAGIC_ext, &vtbl_storable, NULL, 0); \
        cxt = (stcxt_t *)SvPVX(self);                           \
        Zero(cxt, 1, stcxt_t);                                  \
        cxt->my_sv = my_sv;                                     \
    } STMT_END

#if defined(MULTIPLICITY) || defined(PERL_OBJECT) || defined(PERL_CAPI)

#define dSTCXT_SV						\
    SV *perinterp_sv = *hv_fetch(PL_modglobal,                  \
				 MY_VERSION, sizeof(MY_VERSION)-1, TRUE)

#define dSTCXT_PTR(T,name)					\
    T name = ((perinterp_sv                                     \
               && SvIOK(perinterp_sv) && SvIVX(perinterp_sv)    \
               ? (T)SvPVX(SvRV(INT2PTR(SV*,SvIVX(perinterp_sv)))) : (T) 0))
#define dSTCXT					\
    dSTCXT_SV;                                  \
    dSTCXT_PTR(stcxt_t *, cxt)

#define INIT_STCXT					\
    dSTCXT;                                             \
    NEW_STORABLE_CXT_OBJ(cxt);                          \
    assert(perinterp_sv);				\
    sv_setiv(perinterp_sv, PTR2IV(cxt->my_sv))

#define SET_STCXT(x)					\
    STMT_START {					\
        dSTCXT_SV;                                      \
        sv_setiv(perinterp_sv, PTR2IV(x->my_sv));       \
    } STMT_END

#else /* !MULTIPLICITY && !PERL_OBJECT && !PERL_CAPI */

static stcxt_t *Context_ptr = NULL;
#define dSTCXT			stcxt_t *cxt = Context_ptr
#define SET_STCXT(x)		Context_ptr = x
#define INIT_STCXT				\
    dSTCXT;                                     \
    NEW_STORABLE_CXT_OBJ(cxt);                  \
    SET_STCXT(cxt)


#endif /* MULTIPLICITY || PERL_OBJECT || PERL_CAPI */

/*
 * KNOWN BUG:
 *   Croaking implies a memory leak, since we don't use setjmp/longjmp
 *   to catch the exit and free memory used during store or retrieve
 *   operations.  This is not too difficult to fix, but I need to understand
 *   how Perl does it, and croaking is exceptional anyway, so I lack the
 *   motivation to do it.
 *
 * The current workaround is to mark the context as dirty when croaking,
 * so that data structures can be freed whenever we renter Storable code
 * (but only *then*: it's a workaround, not a fix).
 *
 * This is also imperfect, because we don't really know how far they trapped
 * the croak(), and when we were recursing, we won't be able to clean anything
 * but the topmost context stacked.
 */

#define CROAK(x)	STMT_START { cxt->s_dirty = 1; croak x; } STMT_END

/*
 * End of "thread-safe" related definitions.
 */

/*
 * LOW_32BITS
 *
 * Keep only the low 32 bits of a pointer (used for tags, which are not
 * really pointers).
 */

#if PTRSIZE <= 4
#define LOW_32BITS(x)	((I32) (x))
#else
#define LOW_32BITS(x)	((I32) ((STRLEN) (x) & 0xffffffffUL))
#endif

/*
 * PTR2TAG(x)
 *
 * Convert a pointer into an ntag_t.
 */

#define PTR2TAG(x) ((ntag_t)(x))

#define TAG2PTR(x, type) ((y)(x))

/*
 * oI, oS, oC
 *
 * Hack for Crays, where sizeof(I32) == 8, and which are big-endians.
 * Used in the WLEN and RLEN macros.
 */

#if INTSIZE > 4
#define oI(x)	((I32 *) ((char *) (x) + 4))
#define oS(x)	((x) - 4)
#define oL(x)	(x)
#define oC(x)	(x = 0)
#define CRAY_HACK
#else
#define oI(x)	(x)
#define oS(x)	(x)
#define oL(x)	(x)
#define oC(x)
#endif

/*
 * key buffer handling
 */
#define kbuf	(cxt->keybuf).arena
#define ksiz	(cxt->keybuf).asiz
#define KBUFINIT()							\
    STMT_START {							\
        if (!kbuf) {                                                    \
            TRACEME(("** allocating kbuf of 128 bytes"));               \
            New(10003, kbuf, 128, char);                                \
            ksiz = 128;                                                 \
        }                                                               \
    } STMT_END
#define KBUFCHK(x)							\
    STMT_START {							\
        if (x >= ksiz) {                                                \
            if (x >= I32_MAX)                                           \
                CROAK(("Too large size > I32_MAX"));                    \
            TRACEME(("** extending kbuf to %d bytes (had %d)",          \
                     (int)(x+1), (int)ksiz));                           \
            Renew(kbuf, x+1, char);                                     \
            ksiz = x+1;                                                 \
        }                                                               \
    } STMT_END

/*
 * memory buffer handling
 */
#define mbase	(cxt->membuf).arena
#define msiz	(cxt->membuf).asiz
#define mptr	(cxt->membuf).aptr
#define mend	(cxt->membuf).aend

#define MGROW	(1 << 13)
#define MMASK	(MGROW - 1)

#define round_mgrow(x)	\
    ((STRLEN) (((STRLEN) (x) + MMASK) & ~MMASK))
#define trunc_int(x)	\
    ((STRLEN) ((STRLEN) (x) & ~(sizeof(int)-1)))
#define int_aligned(x)	\
    ((STRLEN)(x) == trunc_int(x))

#define MBUF_INIT(x)							\
    STMT_START {							\
        if (!mbase) {                                                   \
            TRACEME(("** allocating mbase of %d bytes", MGROW));        \
            New(10003, mbase, (int)MGROW, char);                        \
            msiz = (STRLEN)MGROW;                                       \
        }                                                               \
        mptr = mbase;                                                   \
        if (x)                                                          \
            mend = mbase + x;                                           \
        else                                                            \
            mend = mbase + msiz;                                        \
    } STMT_END

#define MBUF_TRUNC(x)	mptr = mbase + x
#define MBUF_SIZE()	(mptr - mbase)

/*
 * MBUF_SAVE_AND_LOAD
 * MBUF_RESTORE
 *
 * Those macros are used in do_retrieve() to save the current memory
 * buffer into cxt->msaved, before MBUF_LOAD() can be used to retrieve
 * data from a string.
 */
#define MBUF_SAVE_AND_LOAD(in)						\
    STMT_START {							\
        ASSERT(!cxt->membuf_ro, ("mbase not already saved"));           \
        cxt->membuf_ro = 1;                                             \
        TRACEME(("saving mbuf"));                                       \
        StructCopy(&cxt->membuf, &cxt->msaved, struct extendable);      \
        MBUF_LOAD(in);                                                  \
    } STMT_END

#define MBUF_RESTORE()							\
    STMT_START {							\
        ASSERT(cxt->membuf_ro, ("mbase is read-only"));                 \
        cxt->membuf_ro = 0;                                             \
        TRACEME(("restoring mbuf"));                                    \
        StructCopy(&cxt->msaved, &cxt->membuf, struct extendable);      \
    } STMT_END

/*
 * Use SvPOKp(), because SvPOK() fails on tainted scalars.
 * See store_scalar() for other usage of this workaround.
 */
#define MBUF_LOAD(v)						\
    STMT_START {						\
        ASSERT(cxt->membuf_ro, ("mbase is read-only"));         \
        if (!SvPOKp(v))                                         \
            CROAK(("Not a scalar string"));                     \
        mptr = mbase = SvPV(v, msiz);                           \
        mend = mbase + msiz;                                    \
    } STMT_END

#define MBUF_XTEND(x)						\
    STMT_START {						\
        STRLEN nsz = (STRLEN) round_mgrow((x)+msiz);            \
        STRLEN offset = mptr - mbase;                           \
        ASSERT(!cxt->membuf_ro, ("mbase is not read-only"));    \
        TRACEME(("** extending mbase from %lu to %lu bytes (wants %lu new)", \
                 (unsigned long)msiz, (unsigned long)nsz, (unsigned long)(x)));  \
        Renew(mbase, nsz, char);                                \
        msiz = nsz;                                             \
        mptr = mbase + offset;                                  \
        mend = mbase + nsz;                                     \
    } STMT_END

#define MBUF_CHK(x)				\
    STMT_START {				\
        if ((mptr + (x)) > mend)                \
            MBUF_XTEND(x);                      \
    } STMT_END

#define MBUF_GETC(x)				\
    STMT_START {				\
        if (mptr < mend)                        \
            x = (int) (unsigned char) *mptr++;  \
        else                                    \
            return (SV *) 0;                    \
    } STMT_END

#ifdef CRAY_HACK
#define MBUF_GETINT(x)				\
    STMT_START {				\
        oC(x);                                  \
        if ((mptr + 4) <= mend) {               \
            memcpy(oI(&x), mptr, 4);            \
            mptr += 4;                          \
        } else                                  \
            return (SV *) 0;                    \
    } STMT_END
#else
#define MBUF_GETINT(x)				\
    STMT_START {				\
        if ((mptr + sizeof(int)) <= mend) {     \
            if (int_aligned(mptr))              \
                x = *(int *) mptr;              \
            else                                \
                memcpy(&x, mptr, sizeof(int));  \
            mptr += sizeof(int);                \
        } else                                  \
            return (SV *) 0;                    \
    } STMT_END
#endif

#define MBUF_READ(x,s)				\
    STMT_START {				\
        if ((mptr + (s)) <= mend) {             \
            memcpy(x, mptr, s);                 \
            mptr += s;                          \
        } else                                  \
            return (SV *) 0;                    \
    } STMT_END

#define MBUF_SAFEREAD(x,s,z)			\
    STMT_START {				\
        if ((mptr + (s)) <= mend) {             \
            memcpy(x, mptr, s);                 \
            mptr += s;                          \
        } else {                                \
            sv_free(z);                         \
            return (SV *) 0;                    \
        }                                       \
    } STMT_END

#define MBUF_SAFEPVREAD(x,s,z)			\
    STMT_START {				\
        if ((mptr + (s)) <= mend) {             \
            memcpy(x, mptr, s);                 \
            mptr += s;                          \
        } else {                                \
            Safefree(z);                        \
            return (SV *) 0;                    \
        }                                       \
    } STMT_END

#define MBUF_PUTC(c)				\
    STMT_START {				\
        if (mptr < mend)                        \
            *mptr++ = (char) c;                 \
        else {                                  \
            MBUF_XTEND(1);                      \
            *mptr++ = (char) c;                 \
        }                                       \
    } STMT_END

#ifdef CRAY_HACK
#define MBUF_PUTINT(i)				\
    STMT_START {				\
        MBUF_CHK(4);                            \
        memcpy(mptr, oI(&i), 4);                \
        mptr += 4;                              \
    } STMT_END
#else
#define MBUF_PUTINT(i) 				\
    STMT_START {				\
        MBUF_CHK(sizeof(int));                  \
        if (int_aligned(mptr))                  \
            *(int *) mptr = i;                  \
        else                                    \
            memcpy(mptr, &i, sizeof(int));      \
        mptr += sizeof(int);                    \
    } STMT_END
#endif

#define MBUF_PUTLONG(l)				\
    STMT_START {				\
        MBUF_CHK(8);                            \
        memcpy(mptr, &l, 8);                    \
        mptr += 8;                              \
    } STMT_END
#define MBUF_WRITE(x,s)				\
    STMT_START {				\
        MBUF_CHK(s);                            \
        memcpy(mptr, x, s);                     \
        mptr += s;                              \
    } STMT_END

/*
 * Possible return values for sv_type().
 */

#define svis_REF		0
#define svis_SCALAR		1
#define svis_ARRAY		2
#define svis_HASH		3
#define svis_TIED		4
#define svis_TIED_ITEM		5
#define svis_CODE		6
#define svis_REGEXP		7
#define svis_OTHER		8

/*
 * Flags for SX_HOOK.
 */

#define SHF_TYPE_MASK		0x03
#define SHF_LARGE_CLASSLEN	0x04
#define SHF_LARGE_STRLEN	0x08
#define SHF_LARGE_LISTLEN	0x10
#define SHF_IDX_CLASSNAME	0x20
#define SHF_NEED_RECURSE	0x40
#define SHF_HAS_LIST		0x80

/*
 * Types for SX_HOOK (last 2 bits in flags).
 */

#define SHT_SCALAR		0
#define SHT_ARRAY		1
#define SHT_HASH		2
#define SHT_EXTRA		3	/* Read extra byte for type */

/*
 * The following are held in the "extra byte"...
 */

#define SHT_TSCALAR		4	/* 4 + 0 -- tied scalar */
#define SHT_TARRAY		5	/* 4 + 1 -- tied array */
#define SHT_THASH		6	/* 4 + 2 -- tied hash */

/*
 * per hash flags for flagged hashes
 */

#define SHV_RESTRICTED		0x01

/*
 * per key flags for flagged hashes
 */

#define SHV_K_UTF8		0x01
#define SHV_K_WASUTF8		0x02
#define SHV_K_LOCKED		0x04
#define SHV_K_ISSV		0x08
#define SHV_K_PLACEHOLDER	0x10

/*
 * flags to allow blessing and/or tieing data the data we load
 */
#define FLAG_BLESS_OK 2
#define FLAG_TIE_OK   4

/*
 * Flags for SX_REGEXP.
 */

#define SHR_U32_RE_LEN		0x01

/*
 * Before 0.6, the magic string was "perl-store" (binary version number 0).
 *
 * Since 0.6 introduced many binary incompatibilities, the magic string has
 * been changed to "pst0" to allow an old image to be properly retrieved by
 * a newer Storable, but ensure a newer image cannot be retrieved with an
 * older version.
 *
 * At 0.7, objects are given the ability to serialize themselves, and the
 * set of markers is extended, backward compatibility is not jeopardized,
 * so the binary version number could have remained unchanged.  To correctly
 * spot errors if a file making use of 0.7-specific extensions is given to
 * 0.6 for retrieval, the binary version was moved to "2".  And I'm introducing
 * a "minor" version, to better track this kind of evolution from now on.
 * 
 */
static const char old_magicstr[] = "perl-store"; /* Magic number before 0.6 */
static const char magicstr[] = "pst0";		 /* Used as a magic number */

#define MAGICSTR_BYTES  'p','s','t','0'
#define OLDMAGICSTR_BYTES  'p','e','r','l','-','s','t','o','r','e'

/* 5.6.x introduced the ability to have IVs as long long.
   However, Configure still defined BYTEORDER based on the size of a long.
   Storable uses the BYTEORDER value as part of the header, but doesn't
   explicitly store sizeof(IV) anywhere in the header.  Hence on 5.6.x built
   with IV as long long on a platform that uses Configure (ie most things
   except VMS and Windows) headers are identical for the different IV sizes,
   despite the files containing some fields based on sizeof(IV)
   Erk. Broken-ness.
   5.8 is consistent - the following redefinition kludge is only needed on
   5.6.x, but the interwork is needed on 5.8 while data survives in files
   with the 5.6 header.

*/

#if defined (IVSIZE) && (IVSIZE == 8) && (LONGSIZE == 4)
#ifndef NO_56_INTERWORK_KLUDGE
#define USE_56_INTERWORK_KLUDGE
#endif
#if BYTEORDER == 0x1234
#undef BYTEORDER
#define BYTEORDER 0x12345678
#else
#if BYTEORDER == 0x4321
#undef BYTEORDER
#define BYTEORDER 0x87654321
#endif
#endif
#endif

#if BYTEORDER == 0x1234
#define BYTEORDER_BYTES  '1','2','3','4'
#else
#if BYTEORDER == 0x12345678
#define BYTEORDER_BYTES  '1','2','3','4','5','6','7','8'
#ifdef USE_56_INTERWORK_KLUDGE
#define BYTEORDER_BYTES_56  '1','2','3','4'
#endif
#else
#if BYTEORDER == 0x87654321
#define BYTEORDER_BYTES  '8','7','6','5','4','3','2','1'
#ifdef USE_56_INTERWORK_KLUDGE
#define BYTEORDER_BYTES_56  '4','3','2','1'
#endif
#else
#if BYTEORDER == 0x4321
#define BYTEORDER_BYTES  '4','3','2','1'
#else
#error Unknown byteorder. Please append your byteorder to Storable.xs
#endif
#endif
#endif
#endif

#ifndef INT32_MAX
# define INT32_MAX 2147483647
#endif
#if IVSIZE > 4 && !defined(INT64_MAX)
# define INT64_MAX 9223372036854775807LL
#endif

static const char byteorderstr[] = {BYTEORDER_BYTES, 0};
#ifdef USE_56_INTERWORK_KLUDGE
static const char byteorderstr_56[] = {BYTEORDER_BYTES_56, 0};
#endif

#define STORABLE_BIN_MAJOR	2		/* Binary major "version" */
#define STORABLE_BIN_MINOR	12		/* Binary minor "version" */

#if !defined (SvVOK)
/*
 * Perl 5.6.0-5.8.0 can do weak references, but not vstring magic.
*/
#define STORABLE_BIN_WRITE_MINOR	8
#elif PERL_VERSION_GE(5,19,0)
/* Perl 5.19 takes away the special meaning of PL_sv_undef in arrays. */
/* With 3.x we added LOBJECT */
#define STORABLE_BIN_WRITE_MINOR	11
#else
#define STORABLE_BIN_WRITE_MINOR	9
#endif

#if PERL_VERSION_LT(5,8,1)
#define PL_sv_placeholder PL_sv_undef
#endif

/*
 * Useful store shortcuts...
 */

/*
 * Note that if you put more than one mark for storing a particular
 * type of thing, *and* in the retrieve_foo() function you mark both
 * the thingy's you get off with SEEN(), you *must* increase the
 * tagnum with cxt->tagnum++ along with this macro!
 *     - samv 20Jan04
 */
#define PUTMARK(x) 					\
    STMT_START {					\
        if (!cxt->fio)                                  \
            MBUF_PUTC(x);                               \
        else if (PerlIO_putc(cxt->fio, x) == EOF)       \
            return -1;                                  \
    } STMT_END

#define WRITE_I32(x)						\
    STMT_START {						\
        ASSERT(sizeof(x) == sizeof(I32), ("writing an I32"));   \
        if (!cxt->fio)                                          \
            MBUF_PUTINT(x);                                     \
        else if (PerlIO_write(cxt->fio, oI(&x),                 \
                              oS(sizeof(x))) != oS(sizeof(x)))  \
            return -1;                                          \
    } STMT_END

#define WRITE_U64(x)							\
    STMT_START {							\
        ASSERT(sizeof(x) == sizeof(UV), ("writing an UV"));		\
        if (!cxt->fio)                                                  \
            MBUF_PUTLONG(x);                                            \
        else if (PerlIO_write(cxt->fio, oL(&x),                         \
                              oS(sizeof(x))) != oS(sizeof(x)))          \
            return -1;                                                  \
    } STMT_END

#ifdef HAS_HTONL
#define WLEN(x)                                                         \
    STMT_START {							\
        ASSERT(sizeof(x) == sizeof(int), ("WLEN writing an int"));      \
        if (cxt->netorder) {                                            \
            int y = (int) htonl(x);                                     \
            if (!cxt->fio)                                              \
                MBUF_PUTINT(y);                                         \
            else if (PerlIO_write(cxt->fio,oI(&y),oS(sizeof(y))) != oS(sizeof(y))) \
                return -1;                                              \
        } else {                                                        \
            if (!cxt->fio)                                              \
                MBUF_PUTINT(x);                                         \
            else if (PerlIO_write(cxt->fio,oI(&x),                      \
                                  oS(sizeof(x))) != oS(sizeof(x)))      \
                return -1;                                              \
        }                                                               \
    } STMT_END

#  ifdef HAS_U64

#define W64LEN(x)							\
    STMT_START {							\
        ASSERT(sizeof(x) == 8, ("W64LEN writing a U64"));               \
        if (cxt->netorder) {                                            \
            U32 buf[2];      						\
            buf[1] = htonl(x & 0xffffffffUL);                           \
            buf[0] = htonl(x >> 32);                                    \
            if (!cxt->fio)                                              \
                MBUF_PUTLONG(buf);                                      \
            else if (PerlIO_write(cxt->fio, buf,                        \
                                  sizeof(buf)) != sizeof(buf))          \
                return -1;                                              \
        } else {                                                        \
            if (!cxt->fio)                                              \
                MBUF_PUTLONG(x);                                        \
            else if (PerlIO_write(cxt->fio,oI(&x),                      \
                                  oS(sizeof(x))) != oS(sizeof(x)))      \
                return -1;                                              \
        }                                                               \
    } STMT_END

#  else

#define W64LEN(x) CROAK(("No 64bit UVs"))

#  endif

#else
#define WLEN(x)	WRITE_I32(x)
#ifdef HAS_U64
#define W64LEN(x) WRITE_U64(x)
#else
#define W64LEN(x) CROAK(("no 64bit UVs"))
#endif
#endif

#define WRITE(x,y) 							\
    STMT_START {							\
        if (!cxt->fio)                                                  \
            MBUF_WRITE(x,y);                                            \
        else if (PerlIO_write(cxt->fio, x, y) != (SSize_t)y)            \
            return -1;                                                  \
    } STMT_END

#define STORE_PV_LEN(pv, len, small, large)			\
    STMT_START {						\
        if (len <= LG_SCALAR) {                                 \
            int ilen = (int) len;                               \
            unsigned char clen = (unsigned char) len;           \
            PUTMARK(small);                                     \
            PUTMARK(clen);                                      \
            if (len)                                            \
                WRITE(pv, ilen);                                \
        } else if (sizeof(len) > 4 && len > INT32_MAX) {        \
            PUTMARK(SX_LOBJECT);                                \
            PUTMARK(large);                                     \
            W64LEN(len);                                        \
            WRITE(pv, len);                                     \
        } else {                                                \
            int ilen = (int) len;                               \
            PUTMARK(large);                                     \
            WLEN(ilen);                                         \
            WRITE(pv, ilen);                                    \
        }                                                       \
    } STMT_END

#define STORE_SCALAR(pv, len)	STORE_PV_LEN(pv, len, SX_SCALAR, SX_LSCALAR)

/*
 * Store &PL_sv_undef in arrays without recursing through store().  We
 * actually use this to represent nonexistent elements, for historical
 * reasons.
 */
#define STORE_SV_UNDEF() 					\
    STMT_START {                                                \
	cxt->tagnum++;						\
	PUTMARK(SX_SV_UNDEF);					\
    } STMT_END

/*
 * Useful retrieve shortcuts...
 */

#define GETCHAR() \
    (cxt->fio ? PerlIO_getc(cxt->fio)                   \
              : (mptr >= mend ? EOF : (int) *mptr++))

#define GETMARK(x)							\
    STMT_START {							\
        if (!cxt->fio)                                                  \
            MBUF_GETC(x);                                               \
        else if ((int) (x = PerlIO_getc(cxt->fio)) == EOF)              \
            return (SV *) 0;                                            \
    } STMT_END

#define READ_I32(x)							\
    STMT_START {							\
        ASSERT(sizeof(x) == sizeof(I32), ("reading an I32"));           \
        oC(x);                                                          \
        if (!cxt->fio)                                                  \
            MBUF_GETINT(x);                                             \
        else if (PerlIO_read(cxt->fio, oI(&x),                          \
                                 oS(sizeof(x))) != oS(sizeof(x)))       \
            return (SV *) 0;                                            \
    } STMT_END

#ifdef HAS_NTOHL
#define RLEN(x)                                                         \
    STMT_START {							\
        oC(x);                                                          \
        if (!cxt->fio)                                                  \
            MBUF_GETINT(x);                                             \
        else if (PerlIO_read(cxt->fio, oI(&x),                          \
                                 oS(sizeof(x))) != oS(sizeof(x)))       \
            return (SV *) 0;                                            \
        if (cxt->netorder)                                              \
            x = (int) ntohl(x);                                         \
    } STMT_END
#else
#define RLEN(x) READ_I32(x)
#endif

#define READ(x,y) 							\
    STMT_START {							\
	if (!cxt->fio)							\
            MBUF_READ(x, y);                                            \
	else if (PerlIO_read(cxt->fio, x, y) != (SSize_t)y)             \
            return (SV *) 0;                                            \
    } STMT_END

#define SAFEREAD(x,y,z)                                                 \
    STMT_START {							\
        if (!cxt->fio)                                                  \
            MBUF_SAFEREAD(x,y,z);                                       \
        else if (PerlIO_read(cxt->fio, x, y) != (SSize_t)y) {           \
            sv_free(z);                                                 \
            return (SV *) 0;                                            \
        }                                                               \
    } STMT_END

#define SAFEPVREAD(x,y,z)					\
    STMT_START {						\
        if (!cxt->fio)                                          \
            MBUF_SAFEPVREAD(x,y,z);                             \
        else if (PerlIO_read(cxt->fio, x, y) != y) {            \
            Safefree(z);                                        \
            return (SV *) 0;                                    \
        }                                                       \
    } STMT_END

#ifdef HAS_U64

#  if defined(HAS_NTOHL)
#    define Sntohl(x) ntohl(x)
#  elif BYTEORDER == 0x87654321 || BYTEORDER == 0x4321
#    define Sntohl(x) (x)
#  else
static U32 Sntohl(U32 x) {
    return (((U8) x) << 24) + ((x & 0xFF00) << 8)
	+ ((x & 0xFF0000) >> 8) + ((x & 0xFF000000) >> 24);
}
#  endif

#  define READ_U64(x)                                                       \
    STMT_START {                                                          \
	ASSERT(sizeof(x) == 8, ("R64LEN reading a U64"));                 \
	if (cxt->netorder) {                                              \
	    U32 buf[2];                                                   \
	    READ((void *)buf, sizeof(buf));                               \
	    (x) = ((UV)Sntohl(buf[0]) << 32) + Sntohl(buf[1]);		\
	}                                                                 \
	else {                                                            \
	    READ(&(x), sizeof(x));                                        \
	}                                                                 \
    } STMT_END

#endif

/*
 * SEEN() is used at retrieve time, to remember where object 'y', bearing a
 * given tag 'tagnum', has been retrieved. Next time we see an SX_OBJECT marker,
 * we'll therefore know where it has been retrieved and will be able to
 * share the same reference, as in the original stored memory image.
 *
 * We also need to bless objects ASAP for hooks (which may compute "ref $x"
 * on the objects given to STORABLE_thaw and expect that to be defined), and
 * also for overloaded objects (for which we might not find the stash if the
 * object is not blessed yet--this might occur for overloaded objects that
 * refer to themselves indirectly: if we blessed upon return from a sub
 * retrieve(), the SX_OBJECT marker we'd found could not have overloading
 * restored on it because the underlying object would not be blessed yet!).
 *
 * To achieve that, the class name of the last retrieved object is passed down
 * recursively, and the first SEEN() call for which the class name is not NULL
 * will bless the object.
 *
 * i should be true iff sv is immortal (ie PL_sv_yes, PL_sv_no or PL_sv_undef)
 *
 * SEEN0() is a short-cut where stash is always NULL.
 *
 * The _NN variants dont check for y being null
 */
#define SEEN0_NN(y,i)							\
    STMT_START {							\
        if (av_store(cxt->aseen, cxt->tagnum++, i ? (SV*)(y)            \
                     : SvREFCNT_inc(y)) == 0)                           \
            return (SV *) 0;                                            \
        TRACEME(("aseen(#%d) = 0x%" UVxf " (refcnt=%d)",                \
                 (int)cxt->tagnum-1,                                    \
                 PTR2UV(y), (int)SvREFCNT(y)-1));                       \
    } STMT_END

#define SEEN0(y,i)							\
    STMT_START {							\
        if (!y)                                                         \
            return (SV *) 0;                                            \
        SEEN0_NN(y,i);                                                  \
    } STMT_END

#define SEEN_NN(y,stash,i)						\
    STMT_START {							\
        SEEN0_NN(y,i);                                                  \
        if (stash)							\
            BLESS((SV *)(y), (HV *)(stash));                            \
    } STMT_END

#define SEEN(y,stash,i)							\
    STMT_START {                                                	\
        if (!y)                                                         \
            return (SV *) 0;                                            \
        SEEN_NN(y,stash, i);                                            \
    } STMT_END

/*
 * Bless 's' in 'p', via a temporary reference, required by sv_bless().
 * "A" magic is added before the sv_bless for overloaded classes, this avoids
 * an expensive call to S_reset_amagic in sv_bless.
 */
#define BLESS(s,stash)							\
    STMT_START {							\
        SV *ref;                                                        \
        if (cxt->flags & FLAG_BLESS_OK) {                               \
            TRACEME(("blessing 0x%" UVxf " in %s", PTR2UV(s),           \
                     HvNAME_get(stash)));                               \
            ref = newRV_noinc(s);                                       \
            if (cxt->in_retrieve_overloaded && Gv_AMG(stash)) {         \
                cxt->in_retrieve_overloaded = 0;                        \
                SvAMAGIC_on(ref);                                       \
            }                                                           \
            (void) sv_bless(ref, stash);                                \
            SvRV_set(ref, NULL);                                        \
            SvREFCNT_dec(ref);                                          \
        }                                                               \
        else {                                                          \
            TRACEME(("not blessing 0x%" UVxf " in %s", PTR2UV(s),       \
                     (HvNAME_get(stash))));                             \
        }                                                               \
    } STMT_END
/*
 * sort (used in store_hash) - conditionally use qsort when
 * sortsv is not available ( <= 5.6.1 ).
 */

#if PERL_VERSION_LT(5,7,0)

#if defined(USE_ITHREADS)

#define STORE_HASH_SORT						\
    ENTER; {                                                    \
        PerlInterpreter *orig_perl = PERL_GET_CONTEXT;          \
        SAVESPTR(orig_perl);                                    \
        PERL_SET_CONTEXT(aTHX);                                 \
        qsort((char *) AvARRAY(av), len, sizeof(SV *), sortcmp);\
    } LEAVE;

#else /* ! USE_ITHREADS */

#define STORE_HASH_SORT					\
    qsort((char *) AvARRAY(av), len, sizeof(SV *), sortcmp);

#endif  /* USE_ITHREADS */

#else /* PERL >= 5.7.0 */

#define STORE_HASH_SORT \
    sortsv(AvARRAY(av), len, Perl_sv_cmp);

#endif /* PERL_VERSION_LT(5,7,0) */

static int store(pTHX_ stcxt_t *cxt, SV *sv);
static SV *retrieve(pTHX_ stcxt_t *cxt, const char *cname);

#define UNSEE()			\
    STMT_START {			\
        av_pop(cxt->aseen);             \
        cxt->tagnum--;                  \
    } STMT_END

/*
 * Dynamic dispatching table for SV store.
 */

static int store_ref(pTHX_ stcxt_t *cxt, SV *sv);
static int store_scalar(pTHX_ stcxt_t *cxt, SV *sv);
static int store_array(pTHX_ stcxt_t *cxt, AV *av);
static int store_hash(pTHX_ stcxt_t *cxt, HV *hv);
static int store_tied(pTHX_ stcxt_t *cxt, SV *sv);
static int store_tied_item(pTHX_ stcxt_t *cxt, SV *sv);
static int store_code(pTHX_ stcxt_t *cxt, CV *cv);
static int store_regexp(pTHX_ stcxt_t *cxt, SV *sv);
static int store_other(pTHX_ stcxt_t *cxt, SV *sv);
static int store_blessed(pTHX_ stcxt_t *cxt, SV *sv, int type, HV *pkg);

typedef int (*sv_store_t)(pTHX_ stcxt_t *cxt, SV *sv);

static const sv_store_t sv_store[] = {
    (sv_store_t)store_ref,	/* svis_REF */
    (sv_store_t)store_scalar,	/* svis_SCALAR */
    (sv_store_t)store_array,	/* svis_ARRAY */
    (sv_store_t)store_hash,	/* svis_HASH */
    (sv_store_t)store_tied,	/* svis_TIED */
    (sv_store_t)store_tied_item,/* svis_TIED_ITEM */
    (sv_store_t)store_code,	/* svis_CODE */
    (sv_store_t)store_regexp,	/* svis_REGEXP */
    (sv_store_t)store_other,	/* svis_OTHER */
};

#define SV_STORE(x)	(*sv_store[x])

/*
 * Dynamic dispatching tables for SV retrieval.
 */

static SV *retrieve_lscalar(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_lutf8str(pTHX_ stcxt_t *cxt, const char *cname);
static SV *old_retrieve_array(pTHX_ stcxt_t *cxt, const char *cname);
static SV *old_retrieve_hash(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_ref(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_undef(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_integer(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_double(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_byte(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_netint(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_scalar(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_utf8str(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_tied_array(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_tied_hash(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_tied_scalar(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_other(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_lobject(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_regexp(pTHX_ stcxt_t *cxt, const char *cname);

/* helpers for U64 lobjects */

static SV *get_lstring(pTHX_ stcxt_t *cxt, UV len, int isutf8, const char *cname);
#ifdef HAS_U64
static SV *get_larray(pTHX_ stcxt_t *cxt, UV len, const char *cname);
static SV *get_lhash(pTHX_ stcxt_t *cxt, UV len, int hash_flags, const char *cname);
static int store_lhash(pTHX_ stcxt_t *cxt, HV *hv, unsigned char hash_flags);
#endif
static int store_hentry(pTHX_ stcxt_t *cxt, HV* hv, UV i, HEK *hek, SV *val,
                        unsigned char hash_flags);

typedef SV* (*sv_retrieve_t)(pTHX_ stcxt_t *cxt, const char *name);

static const sv_retrieve_t sv_old_retrieve[] = {
    0,					/* SX_OBJECT -- entry unused dynamically */
    (sv_retrieve_t)retrieve_lscalar,	/* SX_LSCALAR */
    (sv_retrieve_t)old_retrieve_array,	/* SX_ARRAY -- for pre-0.6 binaries */
    (sv_retrieve_t)old_retrieve_hash,	/* SX_HASH -- for pre-0.6 binaries */
    (sv_retrieve_t)retrieve_ref,	/* SX_REF */
    (sv_retrieve_t)retrieve_undef,	/* SX_UNDEF */
    (sv_retrieve_t)retrieve_integer,	/* SX_INTEGER */
    (sv_retrieve_t)retrieve_double,	/* SX_DOUBLE */
    (sv_retrieve_t)retrieve_byte,	/* SX_BYTE */
    (sv_retrieve_t)retrieve_netint,	/* SX_NETINT */
    (sv_retrieve_t)retrieve_scalar,	/* SX_SCALAR */
    (sv_retrieve_t)retrieve_tied_array,	/* SX_TIED_ARRAY */
    (sv_retrieve_t)retrieve_tied_hash,	/* SX_TIED_HASH */
    (sv_retrieve_t)retrieve_tied_scalar,/* SX_TIED_SCALAR */
    (sv_retrieve_t)retrieve_other,	/* SX_SV_UNDEF not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_SV_YES not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_SV_NO not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_BLESS not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_IX_BLESS not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_HOOK not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_OVERLOADED not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_TIED_KEY not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_TIED_IDX not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_UTF8STR not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_LUTF8STR not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_FLAG_HASH not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_CODE not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_WEAKREF not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_WEAKOVERLOAD not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_VSTRING not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_LVSTRING not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_SVUNDEF_ELEM not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_REGEXP */
    (sv_retrieve_t)retrieve_other,  	/* SX_LOBJECT not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_BOOLEAN_TRUE not supported */
    (sv_retrieve_t)retrieve_other,	/* SX_BOOLEAN_FALSE not supported */
    (sv_retrieve_t)retrieve_other,  	/* SX_LAST */
};

static SV *retrieve_hook_common(pTHX_ stcxt_t *cxt, const char *cname, int large);

static SV *retrieve_array(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_hash(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_sv_undef(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_sv_yes(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_sv_no(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_blessed(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_idx_blessed(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_hook(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_overloaded(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_tied_key(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_tied_idx(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_flag_hash(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_code(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_weakref(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_weakoverloaded(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_vstring(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_lvstring(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_svundef_elem(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_boolean_true(pTHX_ stcxt_t *cxt, const char *cname);
static SV *retrieve_boolean_false(pTHX_ stcxt_t *cxt, const char *cname);

static const sv_retrieve_t sv_retrieve[] = {
    0,					/* SX_OBJECT -- entry unused dynamically */
    (sv_retrieve_t)retrieve_lscalar,	/* SX_LSCALAR */
    (sv_retrieve_t)retrieve_array,	/* SX_ARRAY */
    (sv_retrieve_t)retrieve_hash,	/* SX_HASH */
    (sv_retrieve_t)retrieve_ref,	/* SX_REF */
    (sv_retrieve_t)retrieve_undef,	/* SX_UNDEF */
    (sv_retrieve_t)retrieve_integer,	/* SX_INTEGER */
    (sv_retrieve_t)retrieve_double,	/* SX_DOUBLE */
    (sv_retrieve_t)retrieve_byte,	/* SX_BYTE */
    (sv_retrieve_t)retrieve_netint,	/* SX_NETINT */
    (sv_retrieve_t)retrieve_scalar,	/* SX_SCALAR */
    (sv_retrieve_t)retrieve_tied_array,	/* SX_TIED_ARRAY */
    (sv_retrieve_t)retrieve_tied_hash,	/* SX_TIED_HASH */
    (sv_retrieve_t)retrieve_tied_scalar,/* SX_TIED_SCALAR */
    (sv_retrieve_t)retrieve_sv_undef,	/* SX_SV_UNDEF */
    (sv_retrieve_t)retrieve_sv_yes,	/* SX_SV_YES */
    (sv_retrieve_t)retrieve_sv_no,	/* SX_SV_NO */
    (sv_retrieve_t)retrieve_blessed,	/* SX_BLESS */
    (sv_retrieve_t)retrieve_idx_blessed,/* SX_IX_BLESS */
    (sv_retrieve_t)retrieve_hook,	/* SX_HOOK */
    (sv_retrieve_t)retrieve_overloaded,	/* SX_OVERLOAD */
    (sv_retrieve_t)retrieve_tied_key,	/* SX_TIED_KEY */
    (sv_retrieve_t)retrieve_tied_idx,	/* SX_TIED_IDX */
    (sv_retrieve_t)retrieve_utf8str,	/* SX_UTF8STR  */
    (sv_retrieve_t)retrieve_lutf8str,	/* SX_LUTF8STR */
    (sv_retrieve_t)retrieve_flag_hash,	/* SX_HASH */
    (sv_retrieve_t)retrieve_code,	/* SX_CODE */
    (sv_retrieve_t)retrieve_weakref,	/* SX_WEAKREF */
    (sv_retrieve_t)retrieve_weakoverloaded,/* SX_WEAKOVERLOAD */
    (sv_retrieve_t)retrieve_vstring,	/* SX_VSTRING */
    (sv_retrieve_t)retrieve_lvstring,	/* SX_LVSTRING */
    (sv_retrieve_t)retrieve_svundef_elem,/* SX_SVUNDEF_ELEM */
    (sv_retrieve_t)retrieve_regexp,	/* SX_REGEXP */
    (sv_retrieve_t)retrieve_lobject,	/* SX_LOBJECT */
    (sv_retrieve_t)retrieve_boolean_true,	/* SX_BOOLEAN_TRUE */
    (sv_retrieve_t)retrieve_boolean_false,	/* SX_BOOLEAN_FALSE */
    (sv_retrieve_t)retrieve_other,  	/* SX_LAST */
};

#define RETRIEVE(c,x) ((x) >= SX_LAST ? retrieve_other : *(c)->retrieve_vtbl[x])

static SV *mbuf2sv(pTHX);

/***
 *** Context management.
 ***/

/*
 * init_perinterp
 *
 * Called once per "thread" (interpreter) to initialize some global context.
 */
static void init_perinterp(pTHX)
{
    INIT_STCXT;
    INIT_TRACEME;
    cxt->netorder = 0;		/* true if network order used */
    cxt->forgive_me = -1;	/* whether to be forgiving... */
    cxt->accept_future_minor = -1; /* would otherwise occur too late */
}

/*
 * reset_context
 *
 * Called at the end of every context cleaning, to perform common reset
 * operations.
 */
static void reset_context(stcxt_t *cxt)
{
    cxt->entry = 0;
    cxt->s_dirty = 0;
    cxt->recur_sv = NULL;
    cxt->recur_depth = 0;
    cxt->optype &= ~(ST_STORE|ST_RETRIEVE);	/* Leave ST_CLONE alone */
}

/*
 * init_store_context
 *
 * Initialize a new store context for real recursion.
 */
static void init_store_context(pTHX_
	stcxt_t *cxt,
        PerlIO *f,
        int optype,
        int network_order)
{
    INIT_TRACEME;

    TRACEME(("init_store_context"));

    cxt->netorder = network_order;
    cxt->forgive_me = -1;		/* Fetched from perl if needed */
    cxt->deparse = -1;			/* Idem */
    cxt->eval = NULL;			/* Idem */
    cxt->canonical = -1;		/* Idem */
    cxt->tagnum = -1;			/* Reset tag numbers */
    cxt->classnum = -1;			/* Reset class numbers */
    cxt->fio = f;			/* Where I/O are performed */
    cxt->optype = optype;		/* A store, or a deep clone */
    cxt->entry = 1;			/* No recursion yet */

    /*
     * The 'hseen' table is used to keep track of each SV stored and their
     * associated tag numbers is special. It is "abused" because the
     * values stored are not real SV, just integers cast to (SV *),
     * which explains the freeing below.
     *
     * It is also one possible bottleneck to achieve good storing speed,
     * so the "shared keys" optimization is turned off (unlikely to be
     * of any use here), and the hash table is "pre-extended". Together,
     * those optimizations increase the throughput by 12%.
     */

#ifdef USE_PTR_TABLE
    cxt->pseen = ptr_table_new();
    cxt->hseen = 0;
#else
    cxt->hseen = newHV();	/* Table where seen objects are stored */
    HvSHAREKEYS_off(cxt->hseen);
#endif
    /*
     * The following does not work well with perl5.004_04, and causes
     * a core dump later on, in a completely unrelated spot, which
     * makes me think there is a memory corruption going on.
     *
     * Calling hv_ksplit(hseen, HBUCKETS) instead of manually hacking
     * it below does not make any difference. It seems to work fine
     * with perl5.004_68 but given the probable nature of the bug,
     * that does not prove anything.
     *
     * It's a shame because increasing the amount of buckets raises
     * store() throughput by 5%, but until I figure this out, I can't
     * allow for this to go into production.
     *
     * It is reported fixed in 5.005, hence the #if.
     */
#define HBUCKETS	4096		/* Buckets for %hseen */
#ifndef USE_PTR_TABLE
    HvMAX(cxt->hseen) = HBUCKETS - 1;	/* keys %hseen = $HBUCKETS; */
#endif

    /*
     * The 'hclass' hash uses the same settings as 'hseen' above, but it is
     * used to assign sequential tags (numbers) to class names for blessed
     * objects.
     *
     * We turn the shared key optimization on.
     */

    cxt->hclass = newHV();		/* Where seen classnames are stored */

    HvMAX(cxt->hclass) = HBUCKETS - 1;	/* keys %hclass = $HBUCKETS; */

    /*
     * The 'hook' hash table is used to keep track of the references on
     * the STORABLE_freeze hook routines, when found in some class name.
     *
     * It is assumed that the inheritance tree will not be changed during
     * storing, and that no new method will be dynamically created by the
     * hooks.
     */

    cxt->hook = newHV();		/* Table where hooks are cached */

    /*
     * The 'hook_seen' array keeps track of all the SVs returned by
     * STORABLE_freeze hooks for us to serialize, so that they are not
     * reclaimed until the end of the serialization process.  Each SV is
     * only stored once, the first time it is seen.
     */

    cxt->hook_seen = newAV(); /* Lists SVs returned by STORABLE_freeze */

    cxt->max_recur_depth = SvIV(get_sv("Storable::recursion_limit", GV_ADD));
    cxt->max_recur_depth_hash = SvIV(get_sv("Storable::recursion_limit_hash", GV_ADD));
}

/*
 * clean_store_context
 *
 * Clean store context by
 */
static void clean_store_context(pTHX_ stcxt_t *cxt)
{
    HE *he;

    TRACEMED(("clean_store_context"));

    ASSERT(cxt->optype & ST_STORE, ("was performing a store()"));

    /*
     * Insert real values into hashes where we stored faked pointers.
     */

#ifndef USE_PTR_TABLE
    if (cxt->hseen) {
        hv_iterinit(cxt->hseen);
        while ((he = hv_iternext(cxt->hseen)))	/* Extra () for -Wall */
            HeVAL(he) = &PL_sv_undef;
    }
#endif

    if (cxt->hclass) {
        hv_iterinit(cxt->hclass);
        while ((he = hv_iternext(cxt->hclass))) /* Extra () for -Wall */
            HeVAL(he) = &PL_sv_undef;
    }

    /*
     * And now dispose of them...
     *
     * The surrounding if() protection has been added because there might be
     * some cases where this routine is called more than once, during
     * exceptional events.  This was reported by Marc Lehmann when Storable
     * is executed from mod_perl, and the fix was suggested by him.
     * 		-- RAM, 20/12/2000
     */

#ifdef USE_PTR_TABLE
    if (cxt->pseen) {
        struct ptr_tbl *pseen = cxt->pseen;
        cxt->pseen = 0;
        ptr_table_free(pseen);
    }
    assert(!cxt->hseen);
#else
    if (cxt->hseen) {
        HV *hseen = cxt->hseen;
        cxt->hseen = 0;
        hv_undef(hseen);
        sv_free((SV *) hseen);
    }
#endif

    if (cxt->hclass) {
        HV *hclass = cxt->hclass;
        cxt->hclass = 0;
        hv_undef(hclass);
        sv_free((SV *) hclass);
    }

    if (cxt->hook) {
        HV *hook = cxt->hook;
        cxt->hook = 0;
        hv_undef(hook);
        sv_free((SV *) hook);
    }

    if (cxt->hook_seen) {
        AV *hook_seen = cxt->hook_seen;
        cxt->hook_seen = 0;
        av_undef(hook_seen);
        sv_free((SV *) hook_seen);
    }

    cxt->forgive_me = -1;	/* Fetched from perl if needed */
    cxt->deparse = -1;		/* Idem */
    if (cxt->eval) {
        SvREFCNT_dec(cxt->eval);
    }
    cxt->eval = NULL;		/* Idem */
    cxt->canonical = -1;	/* Idem */

    reset_context(cxt);
}

/*
 * init_retrieve_context
 *
 * Initialize a new retrieve context for real recursion.
 */
static void init_retrieve_context(pTHX_
	stcxt_t *cxt, int optype, int is_tainted)
{
    INIT_TRACEME;

    TRACEME(("init_retrieve_context"));

    /*
     * The hook hash table is used to keep track of the references on
     * the STORABLE_thaw hook routines, when found in some class name.
     *
     * It is assumed that the inheritance tree will not be changed during
     * storing, and that no new method will be dynamically created by the
     * hooks.
     */

    cxt->hook  = newHV();			/* Caches STORABLE_thaw */

#ifdef USE_PTR_TABLE
    cxt->pseen = 0;
#endif

    /*
     * If retrieving an old binary version, the cxt->retrieve_vtbl variable
     * was set to sv_old_retrieve. We'll need a hash table to keep track of
     * the correspondence between the tags and the tag number used by the
     * new retrieve routines.
     */

    cxt->hseen = (((void*)cxt->retrieve_vtbl == (void*)sv_old_retrieve)
                  ? newHV() : 0);

    cxt->aseen = newAV();	/* Where retrieved objects are kept */
    cxt->where_is_undef = UNSET_NTAG_T;	/* Special case for PL_sv_undef */
    cxt->aclass = newAV();	/* Where seen classnames are kept */
    cxt->tagnum = 0;		/* Have to count objects... */
    cxt->classnum = 0;		/* ...and class names as well */
    cxt->optype = optype;
    cxt->s_tainted = is_tainted;
    cxt->entry = 1;		/* No recursion yet */
#ifndef HAS_RESTRICTED_HASHES
    cxt->derestrict = -1;	/* Fetched from perl if needed */
#endif
#ifndef HAS_UTF8_ALL
    cxt->use_bytes = -1;	/* Fetched from perl if needed */
#endif
    cxt->accept_future_minor = -1;/* Fetched from perl if needed */
    cxt->in_retrieve_overloaded = 0;

    cxt->max_recur_depth = SvIV(get_sv("Storable::recursion_limit", GV_ADD));
    cxt->max_recur_depth_hash = SvIV(get_sv("Storable::recursion_limit_hash", GV_ADD));
}

/*
 * clean_retrieve_context
 *
 * Clean retrieve context by
 */
static void clean_retrieve_context(pTHX_ stcxt_t *cxt)
{
    TRACEMED(("clean_retrieve_context"));

    ASSERT(cxt->optype & ST_RETRIEVE, ("was performing a retrieve()"));

    if (cxt->aseen) {
        AV *aseen = cxt->aseen;
        cxt->aseen = 0;
        av_undef(aseen);
        sv_free((SV *) aseen);
    }
    cxt->where_is_undef = UNSET_NTAG_T;

    if (cxt->aclass) {
        AV *aclass = cxt->aclass;
        cxt->aclass = 0;
        av_undef(aclass);
        sv_free((SV *) aclass);
    }

    if (cxt->hook) {
        HV *hook = cxt->hook;
        cxt->hook = 0;
        hv_undef(hook);
        sv_free((SV *) hook);
    }

    if (cxt->hseen) {
        HV *hseen = cxt->hseen;
        cxt->hseen = 0;
        hv_undef(hseen);
        sv_free((SV *) hseen);	/* optional HV, for backward compat. */
    }

#ifndef HAS_RESTRICTED_HASHES
    cxt->derestrict = -1;		/* Fetched from perl if needed */
#endif
#ifndef HAS_UTF8_ALL
    cxt->use_bytes = -1;		/* Fetched from perl if needed */
#endif
    cxt->accept_future_minor = -1;	/* Fetched from perl if needed */

    cxt->in_retrieve_overloaded = 0;
    reset_context(cxt);
}

/*
 * clean_context
 *
 * A workaround for the CROAK bug: cleanup the last context.
 */
static void clean_context(pTHX_ stcxt_t *cxt)
{
    TRACEMED(("clean_context"));

    ASSERT(cxt->s_dirty, ("dirty context"));

    if (cxt->membuf_ro)
        MBUF_RESTORE();

    ASSERT(!cxt->membuf_ro, ("mbase is not read-only"));

    if (cxt->optype & ST_RETRIEVE)
        clean_retrieve_context(aTHX_ cxt);
    else if (cxt->optype & ST_STORE)
        clean_store_context(aTHX_ cxt);
    else
        reset_context(cxt);

    ASSERT(!cxt->s_dirty, ("context is clean"));
    ASSERT(cxt->entry == 0, ("context is reset"));
}

/*
 * allocate_context
 *
 * Allocate a new context and push it on top of the parent one.
 * This new context is made globally visible via SET_STCXT().
 */
static stcxt_t *allocate_context(pTHX_ stcxt_t *parent_cxt)
{
    stcxt_t *cxt;

    ASSERT(!parent_cxt->s_dirty, ("parent context clean"));

    NEW_STORABLE_CXT_OBJ(cxt);
    TRACEMED(("allocate_context"));

    cxt->prev = parent_cxt->my_sv;
    SET_STCXT(cxt);

    ASSERT(!cxt->s_dirty, ("clean context"));

    return cxt;
}

/*
 * free_context
 *
 * Free current context, which cannot be the "root" one.
 * Make the context underneath globally visible via SET_STCXT().
 */
static void free_context(pTHX_ stcxt_t *cxt)
{
    stcxt_t *prev = (stcxt_t *)(cxt->prev ? SvPVX(SvRV(cxt->prev)) : 0);

    TRACEMED(("free_context"));

    ASSERT(!cxt->s_dirty, ("clean context"));
    ASSERT(prev, ("not freeing root context"));
    assert(prev);

    SvREFCNT_dec(cxt->my_sv);
    SET_STCXT(prev);

    ASSERT(cxt, ("context not void"));
}

/***
 *** Predicates.
 ***/

/* these two functions are currently only used within asserts */
#ifdef DASSERT
/*
 * is_storing
 *
 * Tells whether we're in the middle of a store operation.
 */
static int is_storing(pTHX)
{
    dSTCXT;

    return cxt->entry && (cxt->optype & ST_STORE);
}

/*
 * is_retrieving
 *
 * Tells whether we're in the middle of a retrieve operation.
 */
static int is_retrieving(pTHX)
{
    dSTCXT;

    return cxt->entry && (cxt->optype & ST_RETRIEVE);
}
#endif

/*
 * last_op_in_netorder
 *
 * Returns whether last operation was made using network order.
 *
 * This is typically out-of-band information that might prove useful
 * to people wishing to convert native to network order data when used.
 */
static int last_op_in_netorder(pTHX)
{
    dSTCXT;

    assert(cxt);
    return cxt->netorder;
}

/***
 *** Hook lookup and calling routines.
 ***/

/*
 * pkg_fetchmeth
 *
 * A wrapper on gv_fetchmethod_autoload() which caches results.
 *
 * Returns the routine reference as an SV*, or null if neither the package
 * nor its ancestors know about the method.
 */
static SV *pkg_fetchmeth(pTHX_
	HV *cache,
	HV *pkg,
	const char *method)
{
    GV *gv;
    SV *sv;
    const char *hvname = HvNAME_get(pkg);
#ifdef DEBUGME
    dSTCXT;
#endif

    /*
     * The following code is the same as the one performed by UNIVERSAL::can
     * in the Perl core.
     */

    gv = gv_fetchmethod_autoload(pkg, method, FALSE);
    if (gv && isGV(gv)) {
        sv = newRV_inc((SV*) GvCV(gv));
        TRACEME(("%s->%s: 0x%" UVxf, hvname, method, PTR2UV(sv)));
    } else {
        sv = newSVsv(&PL_sv_undef);
        TRACEME(("%s->%s: not found", hvname, method));
    }

    /*
     * Cache the result, ignoring failure: if we can't store the value,
     * it just won't be cached.
     */

    (void) hv_store(cache, hvname, strlen(hvname), sv, 0);

    return SvOK(sv) ? sv : (SV *) 0;
}

/*
 * pkg_hide
 *
 * Force cached value to be undef: hook ignored even if present.
 */
static void pkg_hide(pTHX_
	HV *cache,
	HV *pkg,
	const char *method)
{
    const char *hvname = HvNAME_get(pkg);
    PERL_UNUSED_ARG(method);
    (void) hv_store(cache,
                    hvname, strlen(hvname), newSVsv(&PL_sv_undef), 0);
}

/*
 * pkg_uncache
 *
 * Discard cached value: a whole fetch loop will be retried at next lookup.
 */
static void pkg_uncache(pTHX_
	HV *cache,
	HV *pkg,
	const char *method)
{
    const char *hvname = HvNAME_get(pkg);
    PERL_UNUSED_ARG(method);
    (void) hv_delete(cache, hvname, strlen(hvname), G_DISCARD);
}

/*
 * pkg_can
 *
 * Our own "UNIVERSAL::can", which caches results.
 *
 * Returns the routine reference as an SV*, or null if the object does not
 * know about the method.
 */
static SV *pkg_can(pTHX_
	HV *cache,
	HV *pkg,
	const char *method)
{
    SV **svh;
    SV *sv;
    const char *hvname = HvNAME_get(pkg);
#ifdef DEBUGME
    dSTCXT;
#endif

    TRACEME(("pkg_can for %s->%s", hvname, method));

    /*
     * Look into the cache to see whether we already have determined
     * where the routine was, if any.
     *
     * NOTA BENE: we don't use 'method' at all in our lookup, since we know
     * that only one hook (i.e. always the same) is cached in a given cache.
     */

    svh = hv_fetch(cache, hvname, strlen(hvname), FALSE);
    if (svh) {
        sv = *svh;
        if (!SvOK(sv)) {
            TRACEME(("cached %s->%s: not found", hvname, method));
            return (SV *) 0;
        } else {
            TRACEME(("cached %s->%s: 0x%" UVxf,
                     hvname, method, PTR2UV(sv)));
            return sv;
        }
    }

    TRACEME(("not cached yet"));
    return pkg_fetchmeth(aTHX_ cache, pkg, method);	/* Fetch and cache */
}

/*
 * scalar_call
 *
 * Call routine as obj->hook(av) in scalar context.
 * Propagates the single returned value if not called in void context.
 */
static SV *scalar_call(pTHX_
	SV *obj,
	SV *hook,
	int cloning,
	AV *av,
	I32 flags)
{
    dSP;
    int count;
    SV *sv = 0;
#ifdef DEBUGME
    dSTCXT;
#endif

    TRACEME(("scalar_call (cloning=%d)", cloning));

    ENTER;
    SAVETMPS;

    PUSHMARK(sp);
    XPUSHs(obj);
    XPUSHs(sv_2mortal(newSViv(cloning)));		/* Cloning flag */
    if (av) {
        SV **ary = AvARRAY(av);
        SSize_t cnt = AvFILLp(av) + 1;
        SSize_t i;
        XPUSHs(ary[0]);					/* Frozen string */
        for (i = 1; i < cnt; i++) {
            TRACEME(("pushing arg #%d (0x%" UVxf ")...",
                     (int)i, PTR2UV(ary[i])));
            XPUSHs(sv_2mortal(newRV_inc(ary[i])));
        }
    }
    PUTBACK;

    TRACEME(("calling..."));
    count = call_sv(hook, flags);	/* Go back to Perl code */
    TRACEME(("count = %d", count));

    SPAGAIN;

    if (count) {
        sv = POPs;
        SvREFCNT_inc(sv); /* We're returning it, must stay alive! */
    }

    PUTBACK;
    FREETMPS;
    LEAVE;

    return sv;
}

/*
 * array_call
 *
 * Call routine obj->hook(cloning) in list context.
 * Returns the list of returned values in an array.
 */
static AV *array_call(pTHX_
	SV *obj,
	SV *hook,
	int cloning)
{
    dSP;
    int count;
    AV *av;
    int i;
#ifdef DEBUGME
    dSTCXT;
#endif

    TRACEME(("array_call (cloning=%d)", cloning));

    ENTER;
    SAVETMPS;

    PUSHMARK(sp);
    XPUSHs(obj);				/* Target object */
    XPUSHs(sv_2mortal(newSViv(cloning)));	/* Cloning flag */
    PUTBACK;

    count = call_sv(hook, G_LIST);	/* Go back to Perl code */

    SPAGAIN;

    av = newAV();
    for (i = count - 1; i >= 0; i--) {
        SV *sv = POPs;
        av_store(av, i, SvREFCNT_inc(sv));
    }

    PUTBACK;
    FREETMPS;
    LEAVE;

    return av;
}

#if PERL_VERSION_LT(5,15,0)
static void
cleanup_recursive_av(pTHX_ AV* av) {
    SSize_t i = AvFILLp(av);
    SV** arr = AvARRAY(av);
    if (SvMAGICAL(av)) return;
    while (i >= 0) {
        if (arr[i]) {
#if PERL_VERSION_LT(5,14,0)
            arr[i] = NULL;
#else
            SvREFCNT_dec(arr[i]);
#endif
        }
        i--;
    }
}

#ifndef SvREFCNT_IMMORTAL
#ifdef DEBUGGING
   /* exercise the immortal resurrection code in sv_free2() */
#  define SvREFCNT_IMMORTAL 1000
#else
#  define SvREFCNT_IMMORTAL ((~(U32)0)/2)
#endif
#endif

static void
cleanup_recursive_hv(pTHX_ HV* hv) {
    SSize_t i = HvTOTALKEYS(hv);
    HE** arr = HvARRAY(hv);
    if (SvMAGICAL(hv)) return;
    while (i >= 0) {
        if (arr[i]) {
            SvREFCNT(HeVAL(arr[i])) = SvREFCNT_IMMORTAL;
            arr[i] = NULL; /* let it leak. too dangerous to clean it up here */
        }
        i--;
    }
#if PERL_VERSION_LT(5,8,0)
    ((XPVHV*)SvANY(hv))->xhv_array = NULL;
#else
    HvARRAY(hv) = NULL;
#endif
    HvTOTALKEYS(hv) = 0;
}
static void
cleanup_recursive_rv(pTHX_ SV* sv) {
    if (sv && SvROK(sv))
        SvREFCNT_dec(SvRV(sv));
}
static void
cleanup_recursive_data(pTHX_ SV* sv) {
    if (SvTYPE(sv) == SVt_PVAV) {
        cleanup_recursive_av(aTHX_ (AV*)sv);
    }
    else if (SvTYPE(sv) == SVt_PVHV) {
        cleanup_recursive_hv(aTHX_ (HV*)sv);
    }
    else {
        cleanup_recursive_rv(aTHX_ sv);
    }
}
#endif

/*
 * known_class
 *
 * Lookup the class name in the 'hclass' table and either assign it a new ID
 * or return the existing one, by filling in 'classnum'.
 *
 * Return true if the class was known, false if the ID was just generated.
 */
static int known_class(pTHX_
	stcxt_t *cxt,
	char *name,		/* Class name */
	int len,		/* Name length */
	I32 *classnum)
{
    SV **svh;
    HV *hclass = cxt->hclass;

    TRACEME(("known_class (%s)", name));

    /*
     * Recall that we don't store pointers in this hash table, but tags.
     * Therefore, we need LOW_32BITS() to extract the relevant parts.
     */

    svh = hv_fetch(hclass, name, len, FALSE);
    if (svh) {
        *classnum = LOW_32BITS(*svh);
        return TRUE;
    }

    /*
     * Unknown classname, we need to record it.
     */

    cxt->classnum++;
    if (!hv_store(hclass, name, len, INT2PTR(SV*, cxt->classnum), 0))
        CROAK(("Unable to record new classname"));

    *classnum = cxt->classnum;
    return FALSE;
}

/***
 *** Specific store routines.
 ***/

/*
 * store_ref
 *
 * Store a reference.
 * Layout is SX_REF <object> or SX_OVERLOAD <object>.
 */
static int store_ref(pTHX_ stcxt_t *cxt, SV *sv)
{
    int retval;
    int is_weak = 0;
    TRACEME(("store_ref (0x%" UVxf ")", PTR2UV(sv)));

    /*
     * Follow reference, and check if target is overloaded.
     */

#ifdef SvWEAKREF
    if (SvWEAKREF(sv))
        is_weak = 1;
    TRACEME(("ref (0x%" UVxf ") is%s weak", PTR2UV(sv),
             is_weak ? "" : "n't"));
#endif
    sv = SvRV(sv);

    if (SvOBJECT(sv)) {
        HV *stash = (HV *) SvSTASH(sv);
        if (stash && Gv_AMG(stash)) {
            TRACEME(("ref (0x%" UVxf ") is overloaded", PTR2UV(sv)));
            PUTMARK(is_weak ? SX_WEAKOVERLOAD : SX_OVERLOAD);
        } else
            PUTMARK(is_weak ? SX_WEAKREF : SX_REF);
    } else
        PUTMARK(is_weak ? SX_WEAKREF : SX_REF);

    cxt->recur_sv = sv;

    TRACEME((">ref recur_depth %" IVdf ", recur_sv (0x%" UVxf ") max %" IVdf, cxt->recur_depth,
             PTR2UV(cxt->recur_sv), cxt->max_recur_depth));
    if (RECURSION_TOO_DEEP()) {
#if PERL_VERSION_LT(5,15,0)
        cleanup_recursive_data(aTHX_ (SV*)sv);
#endif
        CROAK((MAX_DEPTH_ERROR));
    }

    retval = store(aTHX_ cxt, sv);
    if (cxt->max_recur_depth != -1 && cxt->recur_depth > 0) {
        TRACEME(("<ref recur_depth --%" IVdf, cxt->recur_depth));
        --cxt->recur_depth;
    }
    return retval;
}

/*
 * store_scalar
 *
 * Store a scalar.
 *
 * Layout is SX_LSCALAR <length> <data>, SX_SCALAR <length> <data> or SX_UNDEF.
 * SX_LUTF8STR and SX_UTF8STR are used for UTF-8 strings.
 * The <data> section is omitted if <length> is 0.
 *
 * For vstrings, the vstring portion is stored first with
 * SX_LVSTRING <length> <data> or SX_VSTRING <length> <data>, followed by
 * SX_(L)SCALAR or SX_(L)UTF8STR with the actual PV.
 *
 * If integer or double, the layout is SX_INTEGER <data> or SX_DOUBLE <data>.
 * Small integers (within [-127, +127]) are stored as SX_BYTE <byte>.
 *
 * For huge strings use SX_LOBJECT SX_type SX_U64 <type> <data>
 */
static int store_scalar(pTHX_ stcxt_t *cxt, SV *sv)
{
    IV iv;
    char *pv;
    STRLEN len;
    U32 flags = SvFLAGS(sv);	/* "cc -O" may put it in register */

    TRACEME(("store_scalar (0x%" UVxf ")", PTR2UV(sv)));

    /*
     * For efficiency, break the SV encapsulation by peaking at the flags
     * directly without using the Perl macros to avoid dereferencing
     * sv->sv_flags each time we wish to check the flags.
     */

    if (!(flags & SVf_OK)) {			/* !SvOK(sv) */
        if (sv == &PL_sv_undef) {
            TRACEME(("immortal undef"));
            PUTMARK(SX_SV_UNDEF);
        } else {
            TRACEME(("undef at 0x%" UVxf, PTR2UV(sv)));
            PUTMARK(SX_UNDEF);
        }
        return 0;
    }

    /*
     * Always store the string representation of a scalar if it exists.
     * Gisle Aas provided me with this test case, better than a long speach:
     *
     *  perl -MDevel::Peek -le '$a="abc"; $a+0; Dump($a)'
     *  SV = PVNV(0x80c8520)
     *       REFCNT = 1
     *       FLAGS = (NOK,POK,pNOK,pPOK)
     *       IV = 0
     *       NV = 0
     *       PV = 0x80c83d0 "abc"\0
     *       CUR = 3
     *       LEN = 4
     *
     * Write SX_SCALAR, length, followed by the actual data.
     *
     * Otherwise, write an SX_BYTE, SX_INTEGER or an SX_DOUBLE as
     * appropriate, followed by the actual (binary) data. A double
     * is written as a string if network order, for portability.
     *
     * NOTE: instead of using SvNOK(sv), we test for SvNOKp(sv).
     * The reason is that when the scalar value is tainted, the SvNOK(sv)
     * value is false.
     *
     * The test for a read-only scalar with both POK and NOK set is meant
     * to quickly detect &PL_sv_yes and &PL_sv_no without having to pay the
     * address comparison for each scalar we store.
     */

#define SV_MAYBE_IMMORTAL (SVf_READONLY|SVf_POK|SVf_NOK)

    if ((flags & SV_MAYBE_IMMORTAL) == SV_MAYBE_IMMORTAL) {
        if (sv == &PL_sv_yes) {
            TRACEME(("immortal yes"));
            PUTMARK(SX_SV_YES);
        } else if (sv == &PL_sv_no) {
            TRACEME(("immortal no"));
            PUTMARK(SX_SV_NO);
        } else {
            pv = SvPV(sv, len);		/* We know it's SvPOK */
            goto string;			/* Share code below */
        }
#ifdef SvIsBOOL
    } else if (SvIsBOOL(sv)) {
        TRACEME(("mortal boolean"));
        if (SvTRUE_nomg_NN(sv)) {
            PUTMARK(SX_BOOLEAN_TRUE);
        }
        else {
            PUTMARK(SX_BOOLEAN_FALSE);
        }
#endif
    } else if (flags & SVf_POK) {
        /* public string - go direct to string read.  */
        goto string_readlen;
    } else if (
#if PERL_VERSION_LT(5,7,0)
               /* For 5.6 and earlier NV flag trumps IV flag, so only use integer
                  direct if NV flag is off.  */
               (flags & (SVf_NOK | SVf_IOK)) == SVf_IOK
#else
               /* 5.7 rules are that if IV public flag is set, IV value is as
                  good, if not better, than NV value.  */
               flags & SVf_IOK
#endif
               ) {
        iv = SvIV(sv);
        /*
         * Will come here from below with iv set if double is an integer.
         */
    integer:

        /* Sorry. This isn't in 5.005_56 (IIRC) or earlier.  */
#ifdef SVf_IVisUV
        /* Need to do this out here, else 0xFFFFFFFF becomes iv of -1
         * (for example) and that ends up in the optimised small integer
         * case. 
         */
        if ((flags & SVf_IVisUV) && SvUV(sv) > IV_MAX) {
            TRACEME(("large unsigned integer as string, value = %" UVuf,
                     SvUV(sv)));
            goto string_readlen;
        }
#endif
        /*
         * Optimize small integers into a single byte, otherwise store as
         * a real integer (converted into network order if they asked).
         */

        if (iv >= -128 && iv <= 127) {
            unsigned char siv = (unsigned char) (iv + 128); /* [0,255] */
            PUTMARK(SX_BYTE);
            PUTMARK(siv);
            TRACEME(("small integer stored as %d", (int)siv));
        } else if (cxt->netorder) {
#ifndef HAS_HTONL
            TRACEME(("no htonl, fall back to string for integer"));
            goto string_readlen;
#else
            I32 niv;


#if IVSIZE > 4
            if (
#ifdef SVf_IVisUV
                /* Sorry. This isn't in 5.005_56 (IIRC) or earlier.  */
                ((flags & SVf_IVisUV) && SvUV(sv) > (UV)0x7FFFFFFF) ||
#endif
                (iv > (IV)0x7FFFFFFF) || (iv < -(IV)0x80000000)) {
                /* Bigger than 32 bits.  */
                TRACEME(("large network order integer as string, value = %" IVdf, iv));
                goto string_readlen;
            }
#endif

            niv = (I32) htonl((I32) iv);
            TRACEME(("using network order"));
            PUTMARK(SX_NETINT);
            WRITE_I32(niv);
#endif
        } else {
            PUTMARK(SX_INTEGER);
            WRITE(&iv, sizeof(iv));
        }

        TRACEME(("ok (integer 0x%" UVxf ", value = %" IVdf ")", PTR2UV(sv), iv));
    } else if (flags & SVf_NOK) {
        NV_bytes nv;
#ifdef NV_CLEAR
        /* if we can't tell if there's padding, clear the whole NV and hope the
           compiler leaves the padding alone
        */
        Zero(&nv, 1, NV_bytes);
#endif
#if PERL_VERSION_LT(5,7,0)
        nv.nv = SvNV(sv);
        /*
         * Watch for number being an integer in disguise.
         */
        if (nv.nv == (NV) (iv = I_V(nv.nv))) {
            TRACEME(("double %" NVff " is actually integer %" IVdf, nv, iv));
            goto integer;		/* Share code above */
        }
#else

        SvIV_please(sv);
        if (SvIOK_notUV(sv)) {
            iv = SvIV(sv);
            goto integer;		/* Share code above */
        }
        nv.nv = SvNV(sv);
#endif

        if (cxt->netorder) {
            TRACEME(("double %" NVff " stored as string", nv.nv));
            goto string_readlen;		/* Share code below */
        }
#if NV_PADDING
        Zero(nv.bytes + NVSIZE - NV_PADDING, NV_PADDING, char);
#endif

        PUTMARK(SX_DOUBLE);
        WRITE(&nv, sizeof(nv));

        TRACEME(("ok (double 0x%" UVxf ", value = %" NVff ")", PTR2UV(sv), nv.nv));

    } else if (flags & (SVp_POK | SVp_NOK | SVp_IOK)) {
#ifdef SvVOK
        MAGIC *mg;
#endif
        UV wlen; /* For 64-bit machines */

    string_readlen:
        pv = SvPV(sv, len);

        /*
         * Will come here from above  if it was readonly, POK and NOK but
         * neither &PL_sv_yes nor &PL_sv_no.
         */
    string:

#ifdef SvVOK
        if (SvMAGICAL(sv) && (mg = mg_find(sv, 'V'))) {
            /* The macro passes this by address, not value, and a lot of
               called code assumes that it's 32 bits without checking.  */
            const SSize_t len = mg->mg_len;
            /* we no longer accept vstrings over I32_SIZE-1, so don't emit
               them, also, older Storables handle them badly.
            */
            if (len >= I32_MAX) {
                CROAK(("vstring too large to freeze"));
            }
            STORE_PV_LEN((const char *)mg->mg_ptr,
                         len, SX_VSTRING, SX_LVSTRING);
        }
#endif

        wlen = (Size_t)len;
        if (SvUTF8 (sv))
            STORE_UTF8STR(pv, wlen);
        else
            STORE_SCALAR(pv, wlen);
        TRACEME(("ok (scalar 0x%" UVxf " '%s', length = %" UVuf ")",
                 PTR2UV(sv), len >= 2048 ? "<string too long>" : SvPVX(sv),
                 (UV)len));
    } else {
        CROAK(("Can't determine type of %s(0x%" UVxf ")",
               sv_reftype(sv, FALSE),
               PTR2UV(sv)));
    }
    return 0;		/* Ok, no recursion on scalars */
}

/*
 * store_array
 *
 * Store an array.
 *
 * Layout is SX_ARRAY <size> followed by each item, in increasing index order.
 * Each item is stored as <object>.
 */
static int store_array(pTHX_ stcxt_t *cxt, AV *av)
{
    SV **sav;
    UV len = av_len(av) + 1;
    UV i;
    int ret;
    SV *const recur_sv = cxt->recur_sv;

    TRACEME(("store_array (0x%" UVxf ")", PTR2UV(av)));

#ifdef HAS_U64
    if (len > 0x7fffffffu) {
        /*
         * Large array by emitting SX_LOBJECT 1 U64 data
         */
        PUTMARK(SX_LOBJECT);
        PUTMARK(SX_ARRAY);
        W64LEN(len);
        TRACEME(("lobject size = %lu", (unsigned long)len));
    } else
#endif
    {
        /*
         * Normal array by emitting SX_ARRAY, followed by the array length.
         */
        I32 l = (I32)len;
        PUTMARK(SX_ARRAY);
        WLEN(l);
        TRACEME(("size = %d", (int)l));
    }

    TRACEME((">array recur_depth %" IVdf ", recur_sv (0x%" UVxf ") max %" IVdf, cxt->recur_depth,
             PTR2UV(cxt->recur_sv), cxt->max_recur_depth));
    if (recur_sv != (SV*)av) {
        if (RECURSION_TOO_DEEP()) {
            /* with <= 5.14 it recurses in the cleanup also, needing 2x stack size */
#if PERL_VERSION_LT(5,15,0)
            cleanup_recursive_data(aTHX_ (SV*)av);
#endif
            CROAK((MAX_DEPTH_ERROR));
        }
    }

    /*
     * Now store each item recursively.
     */

    for (i = 0; i < len; i++) {
        sav = av_fetch(av, i, 0);
        if (!sav) {
            TRACEME(("(#%d) nonexistent item", (int)i));
            STORE_SV_UNDEF();
            continue;
        }
#if PERL_VERSION_GE(5,19,0)
        /* In 5.19.3 and up, &PL_sv_undef can actually be stored in
         * an array; it no longer represents nonexistent elements.
         * Historically, we have used SX_SV_UNDEF in arrays for
         * nonexistent elements, so we use SX_SVUNDEF_ELEM for
         * &PL_sv_undef itself. */
        if (*sav == &PL_sv_undef) {
            TRACEME(("(#%d) undef item", (int)i));
            cxt->tagnum++;
            PUTMARK(SX_SVUNDEF_ELEM);
            continue;
        }
#endif
        TRACEME(("(#%d) item", (int)i));
        if ((ret = store(aTHX_ cxt, *sav)))	/* Extra () for -Wall */
            return ret;
    }

    if (recur_sv != (SV*)av) {
        assert(cxt->max_recur_depth == -1 || cxt->recur_depth > 0);
        if (cxt->max_recur_depth != -1 && cxt->recur_depth > 0) {
            TRACEME(("<array recur_depth --%" IVdf, cxt->recur_depth));
            --cxt->recur_depth;
        }
    }
    TRACEME(("ok (array)"));

    return 0;
}


#if PERL_VERSION_LT(5,7,0)

/*
 * sortcmp
 *
 * Sort two SVs
 * Borrowed from perl source file pp_ctl.c, where it is used by pp_sort.
 */
static int
sortcmp(const void *a, const void *b)
{
#if defined(USE_ITHREADS)
    dTHX;
#endif /* USE_ITHREADS */
    return sv_cmp(*(SV * const *) a, *(SV * const *) b);
}

#endif /* PERL_VERSION_LT(5,7,0) */

/*
 * store_hash
 *
 * Store a hash table.
 *
 * For a "normal" hash (not restricted, no utf8 keys):
 *
 * Layout is SX_HASH <size> followed by each key/value pair, in random order.
 * Values are stored as <object>.
 * Keys are stored as <length> <data>, the <data> section being omitted
 * if length is 0.
 *
 * For a "fancy" hash (restricted or utf8 keys):
 *
 * Layout is SX_FLAG_HASH <size> <hash flags> followed by each key/value pair,
 * in random order.
 * Values are stored as <object>.
 * Keys are stored as <flags> <length> <data>, the <data> section being omitted
 * if length is 0.
 * Currently the only hash flag is "restricted"
 * Key flags are as for hv.h
 */
static int store_hash(pTHX_ stcxt_t *cxt, HV *hv)
{
    dVAR;
    UV len = (UV)HvTOTALKEYS(hv);
    Size_t i;
    int ret = 0;
    I32 riter;
    HE *eiter;
    int flagged_hash = ((SvREADONLY(hv)
#ifdef HAS_HASH_KEY_FLAGS
                         || HvHASKFLAGS(hv)
#endif
                         ) ? 1 : 0);
    unsigned char hash_flags = (SvREADONLY(hv) ? SHV_RESTRICTED : 0);
    SV * const recur_sv = cxt->recur_sv;

    /* 
     * Signal hash by emitting SX_HASH, followed by the table length.
     * Max number of keys per perl version:
     *    IV            - 5.12
     *    STRLEN  5.14  - 5.24   (size_t: U32/U64)
     *    SSize_t 5.22c - 5.24c  (I32/I64)
     *    U32     5.25c -
     */

    if (len > 0x7fffffffu) { /* keys > I32_MAX */
        /* 
         * Large hash: SX_LOBJECT type hashflags? U64 data
         *
         * Stupid limitation:
         * Note that perl5 can store more than 2G keys, but only iterate
         * over 2G max. (cperl can)
         * We need to manually iterate over it then, unsorted.
         * But until perl itself cannot do that, skip that.
         */
        TRACEME(("lobject size = %lu", (unsigned long)len));
#ifdef HAS_U64
        PUTMARK(SX_LOBJECT);
        if (flagged_hash) {
            PUTMARK(SX_FLAG_HASH);
            PUTMARK(hash_flags);
        } else {
            PUTMARK(SX_HASH);
        }
        W64LEN(len);
        return store_lhash(aTHX_ cxt, hv, hash_flags);
#else
        /* <5.12 you could store larger hashes, but cannot iterate over them.
           So we reject them, it's a bug. */
        CROAK(("Cannot store large objects on a 32bit system"));
#endif
    } else {
        I32 l = (I32)len;
        if (flagged_hash) {
            TRACEME(("store_hash (0x%" UVxf ") (flags %x)", PTR2UV(hv),
                     (unsigned int)hash_flags));
            PUTMARK(SX_FLAG_HASH);
            PUTMARK(hash_flags);
        } else {
            TRACEME(("store_hash (0x%" UVxf ")", PTR2UV(hv)));
            PUTMARK(SX_HASH);
        }
        WLEN(l);
        TRACEME(("size = %d, used = %d", (int)l, (int)HvUSEDKEYS(hv)));
    }

    TRACEME((">hash recur_depth %" IVdf ", recur_sv (0x%" UVxf ") max %" IVdf, cxt->recur_depth,
             PTR2UV(cxt->recur_sv), cxt->max_recur_depth_hash));
    if (recur_sv != (SV*)hv && cxt->max_recur_depth_hash != -1) {
        ++cxt->recur_depth;
    }
    if (RECURSION_TOO_DEEP_HASH()) {
#if PERL_VERSION_LT(5,15,0)
        cleanup_recursive_data(aTHX_ (SV*)hv);
#endif
        CROAK((MAX_DEPTH_ERROR));
    }

    /*
     * Save possible iteration state via each() on that table.
     *
     * Note that perl as of 5.24 *can* store more than 2G keys, but *not*
     * iterate over it.
     * Lengths of hash keys are also limited to I32, which is good.
     */

    riter = HvRITER_get(hv);
    eiter = HvEITER_get(hv);
    hv_iterinit(hv);

    /*
     * Now store each item recursively.
     *
     * If canonical is defined to some true value then store each
     * key/value pair in sorted order otherwise the order is random.
     * Canonical order is irrelevant when a deep clone operation is performed.
     *
     * Fetch the value from perl only once per store() operation, and only
     * when needed.
     */

    if (
        !(cxt->optype & ST_CLONE)
        && (cxt->canonical == 1
            || (cxt->canonical < 0
                && (cxt->canonical =
                    (SvTRUE(get_sv("Storable::canonical", GV_ADD))
                     ? 1 : 0))))
	) {
        /*
         * Storing in order, sorted by key.
         * Run through the hash, building up an array of keys in a
         * mortal array, sort the array and then run through the
         * array.
         */
        AV *av = newAV();
        av_extend (av, len);

        TRACEME(("using canonical order"));

        for (i = 0; i < len; i++) {
#ifdef HAS_RESTRICTED_HASHES
            HE *he = hv_iternext_flags(hv, HV_ITERNEXT_WANTPLACEHOLDERS);
#else
            HE *he = hv_iternext(hv);
#endif
            av_store(av, i, hv_iterkeysv(he));
        }

        STORE_HASH_SORT;

        for (i = 0; i < len; i++) {
#ifdef HAS_RESTRICTED_HASHES
            int placeholders = (int)HvPLACEHOLDERS_get(hv);
#endif
            unsigned char flags = 0;
            char *keyval;
            STRLEN keylen_tmp;
            I32 keylen;
            SV *key = av_shift(av);
            /* This will fail if key is a placeholder.
               Track how many placeholders we have, and error if we
               "see" too many.  */
            HE *he  = hv_fetch_ent(hv, key, 0, 0);
            SV *val;

            if (he) {
                if (!(val =  HeVAL(he))) {
                    /* Internal error, not I/O error */
                    return 1;
                }
            } else {
#ifdef HAS_RESTRICTED_HASHES
                /* Should be a placeholder.  */
                if (placeholders-- < 0) {
                    /* This should not happen - number of
                       retrieves should be identical to
                       number of placeholders.  */
                    return 1;
                }
                /* Value is never needed, and PL_sv_undef is
                   more space efficient to store.  */
                val = &PL_sv_undef;
                ASSERT (flags == 0,
                        ("Flags not 0 but %d", (int)flags));
                flags = SHV_K_PLACEHOLDER;
#else
                return 1;
#endif
            }

            /*
             * Store value first.
             */

            TRACEME(("(#%d) value 0x%" UVxf, (int)i, PTR2UV(val)));

            if ((ret = store(aTHX_ cxt, val)))	/* Extra () for -Wall, grr... */
                goto out;

            /*
             * Write key string.
             * Keys are written after values to make sure retrieval
             * can be optimal in terms of memory usage, where keys are
             * read into a fixed unique buffer called kbuf.
             * See retrieve_hash() for details.
             */

            /* Implementation of restricted hashes isn't nicely
               abstracted:  */
            if ((hash_flags & SHV_RESTRICTED)
                && SvTRULYREADONLY(val)) {
                flags |= SHV_K_LOCKED;
            }

            keyval = SvPV(key, keylen_tmp);
            keylen = keylen_tmp;
            if (SvUTF8(key)) {
                const char *keysave = keyval;
                bool is_utf8 = TRUE;

                /* Just casting the &klen to (STRLEN) won't work
                   well if STRLEN and I32 are of different widths.
                   --jhi */
                keyval = (char*)bytes_from_utf8((U8*)keyval,
                                                &keylen_tmp,
                                                &is_utf8);

                /* If we were able to downgrade here, then than
                   means that we have  a key which only had chars
                   0-255, but was utf8 encoded.  */

                if (keyval != keysave) {
                    keylen = keylen_tmp;
                    flags |= SHV_K_WASUTF8;
                } else {
                    /* keylen_tmp can't have changed, so no need
                       to assign back to keylen.  */
                    flags |= SHV_K_UTF8;
                }
            }

            if (flagged_hash) {
                PUTMARK(flags);
                TRACEME(("(#%d) key '%s' flags %x %u", (int)i, keyval, flags, *keyval));
            } else {
                /* This is a workaround for a bug in 5.8.0
                   that causes the HEK_WASUTF8 flag to be
                   set on an HEK without the hash being
                   marked as having key flags. We just
                   cross our fingers and drop the flag.
                   AMS 20030901 */
                assert (flags == 0 || flags == SHV_K_WASUTF8);
                TRACEME(("(#%d) key '%s'", (int)i, keyval));
            }
            WLEN(keylen);
            if (keylen)
                WRITE(keyval, keylen);
            if (flags & SHV_K_WASUTF8)
                Safefree (keyval);
        }

        /* 
         * Free up the temporary array
         */

        av_undef(av);
        sv_free((SV *) av);

    } else {

        /*
         * Storing in "random" order (in the order the keys are stored
         * within the hash).  This is the default and will be faster!
         */

        for (i = 0; i < len; i++) {
#ifdef HV_ITERNEXT_WANTPLACEHOLDERS
            HE *he = hv_iternext_flags(hv, HV_ITERNEXT_WANTPLACEHOLDERS);
#else
            HE *he = hv_iternext(hv);
#endif
            SV *val = (he ? hv_iterval(hv, he) : 0);

            if (val == 0)
                return 1;		/* Internal error, not I/O error */

            if ((ret = store_hentry(aTHX_ cxt, hv, i, HeKEY_hek(he), val, hash_flags)))
                goto out;
        }
    }

    TRACEME(("ok (hash 0x%" UVxf ")", PTR2UV(hv)));

 out:
    assert(cxt->max_recur_depth_hash != -1 && cxt->recur_depth > 0);
    TRACEME(("<hash recur_depth --%" IVdf , cxt->recur_depth));
    if (cxt->max_recur_depth_hash != -1 && recur_sv != (SV*)hv && cxt->recur_depth > 0) {
        --cxt->recur_depth;
    }
    HvRITER_set(hv, riter);		/* Restore hash iterator state */
    HvEITER_set(hv, eiter);

    return ret;
}

static int store_hentry(pTHX_
	stcxt_t *cxt, HV* hv, UV i, HEK *hek, SV *val, unsigned char hash_flags)
{
    int ret = 0;
    int flagged_hash = ((SvREADONLY(hv)
#ifdef HAS_HASH_KEY_FLAGS
                         || HvHASKFLAGS(hv)
#endif
                         ) ? 1 : 0);
    /* Implementation of restricted hashes isn't nicely
       abstracted:  */
    unsigned char flags = (((hash_flags & SHV_RESTRICTED)
                            && SvTRULYREADONLY(val))
                           ? SHV_K_LOCKED : 0);
#ifndef DEBUGME
    PERL_UNUSED_ARG(i);
#endif
    if (val == &PL_sv_placeholder) {
        flags |= SHV_K_PLACEHOLDER;
        val = &PL_sv_undef;
    }

    /*
     * Store value first.
     */

    TRACEME(("(#%d) value 0x%" UVxf, (int)i, PTR2UV(val)));

    {
        I32  len = HEK_LEN(hek);
        SV *key_sv = NULL;
        char *key = 0;

        if ((ret = store(aTHX_ cxt, val)))
            return ret;
        if (len == HEf_SVKEY) {
            /* This is somewhat sick, but the internal APIs are
             * such that XS code could put one of these in
             * a regular hash.
             * Maybe we should be capable of storing one if
             * found.
             */
            key_sv = (SV *)HEK_KEY(hek);
            flags |= SHV_K_ISSV;
        } else {
            /* Regular string key. */
#ifdef HAS_HASH_KEY_FLAGS
            if (HEK_UTF8(hek))
                flags |= SHV_K_UTF8;
            if (HEK_WASUTF8(hek))
                flags |= SHV_K_WASUTF8;
#endif
            key = HEK_KEY(hek);
        }
        /*
         * Write key string.
         * Keys are written after values to make sure retrieval
         * can be optimal in terms of memory usage, where keys are
         * read into a fixed unique buffer called kbuf.
         * See retrieve_hash() for details.
         */

        if (flagged_hash) {
            PUTMARK(flags);
            TRACEME(("(#%d) key '%s' flags %x", (int)i, key, flags));
        } else {
            /* This is a workaround for a bug in 5.8.0
               that causes the HEK_WASUTF8 flag to be
               set on an HEK without the hash being
               marked as having key flags. We just
               cross our fingers and drop the flag.
               AMS 20030901 */
            assert (flags == 0 || flags == SHV_K_WASUTF8);
            TRACEME(("(#%d) key '%s'", (int)i, key));
        }
        if (flags & SHV_K_ISSV) {
            if ((ret = store(aTHX_ cxt, key_sv)))
                return ret;
        } else {
            WLEN(len);
            if (len)
                WRITE(key, len);
        }
    }
    return ret;
}


#ifdef HAS_U64
/*
 * store_lhash
 *
 * Store a overlong hash table, with >2G keys, which we cannot iterate
 * over with perl5. xhv_eiter is only I32 there. (only cperl can)
 * and we also do not want to sort it.
 * So we walk the buckets and chains manually.
 *
 * type, len and flags are already written.
 */

static int store_lhash(pTHX_ stcxt_t *cxt, HV *hv, unsigned char hash_flags)
{
    dVAR;
    int ret = 0;
    Size_t i;
    UV ix = 0;
    HE** array;
#ifdef DEBUGME
    UV len = (UV)HvTOTALKEYS(hv);
#endif
    SV * const recur_sv = cxt->recur_sv;
    if (hash_flags) {
        TRACEME(("store_lhash (0x%" UVxf ") (flags %x)", PTR2UV(hv),
                 (int) hash_flags));
    } else {
        TRACEME(("store_lhash (0x%" UVxf ")", PTR2UV(hv)));
    }
    TRACEME(("size = %" UVuf ", used = %" UVuf, len, (UV)HvUSEDKEYS(hv)));

    TRACEME(("recur_depth %" IVdf ", recur_sv (0x%" UVxf ")", cxt->recur_depth,
             PTR2UV(cxt->recur_sv)));
    if (recur_sv != (SV*)hv && cxt->max_recur_depth_hash != -1) {
        ++cxt->recur_depth;
    }
    if (RECURSION_TOO_DEEP_HASH()) {
#if PERL_VERSION_LT(5,15,0)
        cleanup_recursive_data(aTHX_ (SV*)hv);
#endif
        CROAK((MAX_DEPTH_ERROR));
    }

    array = HvARRAY(hv);
    for (i = 0; i <= (Size_t)HvMAX(hv); i++) {
        HE* entry = array[i];

        while (entry) {
            SV* val = hv_iterval(hv, entry);
            if ((ret = store_hentry(aTHX_ cxt, hv, ix++, HeKEY_hek(entry), val, hash_flags)))
                return ret;
            entry = HeNEXT(entry);
        }
    }
    if (recur_sv != (SV*)hv && cxt->max_recur_depth_hash != -1 && cxt->recur_depth > 0) {
        TRACEME(("recur_depth --%" IVdf, cxt->recur_depth));
        --cxt->recur_depth;
    }
    assert(ix == len);
    return ret;
}
#endif

/*
 * store_code
 *
 * Store a code reference.
 *
 * Layout is SX_CODE <length> followed by a scalar containing the perl
 * source code of the code reference.
 */
static int store_code(pTHX_ stcxt_t *cxt, CV *cv)
{
    dSP;
    STRLEN len;
    STRLEN count, reallen;
    SV *text, *bdeparse;

    TRACEME(("store_code (0x%" UVxf ")", PTR2UV(cv)));

    if (
        cxt->deparse == 0 ||
        (cxt->deparse < 0 &&
         !(cxt->deparse =
           SvTRUE(get_sv("Storable::Deparse", GV_ADD)) ? 1 : 0))
	) {
        return store_other(aTHX_ cxt, (SV*)cv);
    }

    /*
     * Require B::Deparse. At least B::Deparse 0.61 is needed for
     * blessed code references.
     */
    /* Ownership of both SVs is passed to load_module, which frees them. */
    load_module(PERL_LOADMOD_NOIMPORT, newSVpvs("B::Deparse"), newSVnv(0.61));
    SPAGAIN;

    ENTER;
    SAVETMPS;

    /*
     * create the B::Deparse object
     */

    PUSHMARK(sp);
    XPUSHs(newSVpvs_flags("B::Deparse", SVs_TEMP));
    PUTBACK;
    count = call_method("new", G_SCALAR);
    SPAGAIN;
    if (count != 1)
        CROAK(("Unexpected return value from B::Deparse::new\n"));
    bdeparse = POPs;

    /*
     * call the coderef2text method
     */

    PUSHMARK(sp);
    XPUSHs(bdeparse); /* XXX is this already mortal? */
    XPUSHs(sv_2mortal(newRV_inc((SV*)cv)));
    PUTBACK;
    count = call_method("coderef2text", G_SCALAR);
    SPAGAIN;
    if (count != 1)
        CROAK(("Unexpected return value from B::Deparse::coderef2text\n"));

    text = POPs;
    PUTBACK;
    len = SvCUR(text);
    reallen = strlen(SvPV_nolen(text));

    /*
     * Empty code references or XS functions are deparsed as
     * "(prototype) ;" or ";".
     */

    if (len == 0 || *(SvPV_nolen(text)+reallen-1) == ';') {
        CROAK(("The result of B::Deparse::coderef2text was empty - maybe you're trying to serialize an XS function?\n"));
    }

    /* 
     * Signal code by emitting SX_CODE.
     */

    PUTMARK(SX_CODE);
    cxt->tagnum++;   /* necessary, as SX_CODE is a SEEN() candidate */
    TRACEME(("size = %d", (int)len));
    TRACEME(("code = %s", SvPV_nolen(text)));

    /*
     * Now store the source code.
     */

    if(SvUTF8 (text))
        STORE_UTF8STR(SvPV_nolen(text), len);
    else
        STORE_SCALAR(SvPV_nolen(text), len);

    FREETMPS;
    LEAVE;

    TRACEME(("ok (code)"));

    return 0;
}

#if PERL_VERSION_LT(5,8,0)
#   define PERL_MAGIC_qr                  'r' /* precompiled qr// regex */
#   define BFD_Svs_SMG_OR_RMG SVs_RMG
#elif PERL_VERSION_GE(5,8,1)
#   define BFD_Svs_SMG_OR_RMG SVs_SMG
#   define MY_PLACEHOLDER PL_sv_placeholder
#else
#   define BFD_Svs_SMG_OR_RMG SVs_RMG
#   define MY_PLACEHOLDER PL_sv_undef
#endif

static int get_regexp(pTHX_ stcxt_t *cxt, SV* sv, SV **re, SV **flags) {
    dSP;
    SV* rv;
#if PERL_VERSION_GE(5,12,0)
    CV *cv = get_cv("re::regexp_pattern", 0);
#else
    CV *cv = get_cv("Storable::_regexp_pattern", 0);
#endif
    I32 count;

    assert(cv);

    ENTER;
    SAVETMPS;
    rv = sv_2mortal((SV*)newRV_inc(sv));
    PUSHMARK(sp);
    XPUSHs(rv);
    PUTBACK;
    /* optimize to call the XS directly later */
    count = call_sv((SV*)cv, G_LIST);
    SPAGAIN;
    if (count < 2)
      CROAK(("re::regexp_pattern returned only %d results", (int)count));
    *flags = POPs;
    SvREFCNT_inc(*flags);
    *re = POPs;
    SvREFCNT_inc(*re);

    PUTBACK;
    FREETMPS;
    LEAVE;

    return 1;
}

static int store_regexp(pTHX_ stcxt_t *cxt, SV *sv) {
    SV *re = NULL;
    SV *flags = NULL;
    const char *re_pv;
    const char *flags_pv;
    STRLEN re_len;
    STRLEN flags_len;
    U8 op_flags = 0;

    if (!get_regexp(aTHX_ cxt, sv, &re, &flags))
      return -1;

    re_pv = SvPV(re, re_len);
    flags_pv = SvPV(flags, flags_len);

    if (re_len > 0xFF) {
      op_flags |= SHR_U32_RE_LEN;
    }
    
    PUTMARK(SX_REGEXP);
    PUTMARK(op_flags);
    if (op_flags & SHR_U32_RE_LEN) {
      U32 re_len32 = re_len;
      WLEN(re_len32);
    }
    else
      PUTMARK(re_len);
    WRITE(re_pv, re_len);
    PUTMARK(flags_len);
    WRITE(flags_pv, flags_len);

    return 0;
}

/*
 * store_tied
 *
 * When storing a tied object (be it a tied scalar, array or hash), we lay out
 * a special mark, followed by the underlying tied object. For instance, when
 * dealing with a tied hash, we store SX_TIED_HASH <hash object>, where
 * <hash object> stands for the serialization of the tied hash.
 */
static int store_tied(pTHX_ stcxt_t *cxt, SV *sv)
{
    MAGIC *mg;
    SV *obj = NULL;
    int ret = 0;
    int svt = SvTYPE(sv);
    char mtype = 'P';

    TRACEME(("store_tied (0x%" UVxf ")", PTR2UV(sv)));

    /*
     * We have a small run-time penalty here because we chose to factorise
     * all tieds objects into the same routine, and not have a store_tied_hash,
     * a store_tied_array, etc...
     *
     * Don't use a switch() statement, as most compilers don't optimize that
     * well for 2/3 values. An if() else if() cascade is just fine. We put
     * tied hashes first, as they are the most likely beasts.
     */

    if (svt == SVt_PVHV) {
        TRACEME(("tied hash"));
        PUTMARK(SX_TIED_HASH);		/* Introduces tied hash */
    } else if (svt == SVt_PVAV) {
        TRACEME(("tied array"));
        PUTMARK(SX_TIED_ARRAY);		/* Introduces tied array */
    } else {
        TRACEME(("tied scalar"));
        PUTMARK(SX_TIED_SCALAR);	/* Introduces tied scalar */
        mtype = 'q';
    }

    if (!(mg = mg_find(sv, mtype)))
        CROAK(("No magic '%c' found while storing tied %s", mtype,
               (svt == SVt_PVHV) ? "hash" :
               (svt == SVt_PVAV) ? "array" : "scalar"));

    /*
     * The mg->mg_obj found by mg_find() above actually points to the
     * underlying tied Perl object implementation. For instance, if the
     * original SV was that of a tied array, then mg->mg_obj is an AV.
     *
     * Note that we store the Perl object as-is. We don't call its FETCH
     * method along the way. At retrieval time, we won't call its STORE
     * method either, but the tieing magic will be re-installed. In itself,
     * that ensures that the tieing semantics are preserved since further
     * accesses on the retrieved object will indeed call the magic methods...
     */

    /* [#17040] mg_obj is NULL for scalar self-ties. AMS 20030416 */
    obj = mg->mg_obj ? mg->mg_obj : newSV(0);
    if ((ret = store(aTHX_ cxt, obj)))
        return ret;

    TRACEME(("ok (tied)"));

    return 0;
}

/*
 * store_tied_item
 *
 * Stores a reference to an item within a tied structure:
 *
 *  . \$h{key}, stores both the (tied %h) object and 'key'.
 *  . \$a[idx], stores both the (tied @a) object and 'idx'.
 *
 * Layout is therefore either:
 *     SX_TIED_KEY <object> <key>
 *     SX_TIED_IDX <object> <index>
 */
static int store_tied_item(pTHX_ stcxt_t *cxt, SV *sv)
{
    MAGIC *mg;
    int ret;

    TRACEME(("store_tied_item (0x%" UVxf ")", PTR2UV(sv)));

    if (!(mg = mg_find(sv, 'p')))
        CROAK(("No magic 'p' found while storing reference to tied item"));

    /*
     * We discriminate between \$h{key} and \$a[idx] via mg_ptr.
     */

    if (mg->mg_ptr) {
        TRACEME(("store_tied_item: storing a ref to a tied hash item"));
        PUTMARK(SX_TIED_KEY);
        TRACEME(("store_tied_item: storing OBJ 0x%" UVxf, PTR2UV(mg->mg_obj)));

        if ((ret = store(aTHX_ cxt, mg->mg_obj)))	/* Extra () for -Wall, grr... */
            return ret;

        TRACEME(("store_tied_item: storing PTR 0x%" UVxf, PTR2UV(mg->mg_ptr)));

        if ((ret = store(aTHX_ cxt, (SV *) mg->mg_ptr))) /* Idem, for -Wall */
            return ret;
    } else {
        I32 idx = mg->mg_len;

        TRACEME(("store_tied_item: storing a ref to a tied array item "));
        PUTMARK(SX_TIED_IDX);
        TRACEME(("store_tied_item: storing OBJ 0x%" UVxf, PTR2UV(mg->mg_obj)));

        if ((ret = store(aTHX_ cxt, mg->mg_obj)))	/* Idem, for -Wall */
            return ret;

        TRACEME(("store_tied_item: storing IDX %d", (int)idx));

        WLEN(idx);
    }

    TRACEME(("ok (tied item)"));

    return 0;
}

/*
 * store_hook		-- dispatched manually, not via sv_store[]
 *
 * The blessed SV is serialized by a hook.
 *
 * Simple Layout is:
 *
 *     SX_HOOK <flags> <len> <classname> <len2> <str> [<len3> <object-IDs>]
 *
 * where <flags> indicates how long <len>, <len2> and <len3> are, whether
 * the trailing part [] is present, the type of object (scalar, array or hash).
 * There is also a bit which says how the classname is stored between:
 *
 *     <len> <classname>
 *     <index>
 *
 * and when the <index> form is used (classname already seen), the "large
 * classname" bit in <flags> indicates how large the <index> is.
 * 
 * The serialized string returned by the hook is of length <len2> and comes
 * next.  It is an opaque string for us.
 *
 * Those <len3> object IDs which are listed last represent the extra references
 * not directly serialized by the hook, but which are linked to the object.
 *
 * When recursion is mandated to resolve object-IDs not yet seen, we have
 * instead, with <header> being flags with bits set to indicate the object type
 * and that recursion was indeed needed:
 *
 *     SX_HOOK <header> <object> <header> <object> <flags>
 *
 * that same header being repeated between serialized objects obtained through
 * recursion, until we reach flags indicating no recursion, at which point
 * we know we've resynchronized with a single layout, after <flags>.
 *
 * When storing a blessed ref to a tied variable, the following format is
 * used:
 *
 *     SX_HOOK <flags> <extra> ... [<len3> <object-IDs>] <magic object>
 *
 * The first <flags> indication carries an object of type SHT_EXTRA, and the
 * real object type is held in the <extra> flag.  At the very end of the
 * serialization stream, the underlying magic object is serialized, just like
 * any other tied variable.
 */
static int store_hook(
                      pTHX_
                      stcxt_t *cxt,
                      SV *sv,
                      int type,
                      HV *pkg,
                      SV *hook)
{
    I32 len;
    char *classname;
    STRLEN len2;
    SV *ref;
    AV *av;
    SV **ary;
    IV count;			/* really len3 + 1 */
    unsigned char flags;
    char *pv;
    int i;
    int recursed = 0;		/* counts recursion */
    int obj_type;		/* object type, on 2 bits */
    I32 classnum;
    int ret;
    int clone = cxt->optype & ST_CLONE;
    char mtype = '\0';		/* for blessed ref to tied structures */
    unsigned char eflags = '\0'; /* used when object type is SHT_EXTRA */
#ifdef HAS_U64
    int need_large_oids = 0;
#endif

    classname = HvNAME_get(pkg);
    len = strlen(classname);

    TRACEME(("store_hook, classname \"%s\", tagged #%d", classname, (int)cxt->tagnum));

    /*
     * Determine object type on 2 bits.
     */

    switch (type) {
    case svis_REF:
    case svis_SCALAR:
    case svis_REGEXP:
        obj_type = SHT_SCALAR;
        break;
    case svis_ARRAY:
        obj_type = SHT_ARRAY;
        break;
    case svis_HASH:
        obj_type = SHT_HASH;
        break;
    case svis_TIED:
        /*
         * Produced by a blessed ref to a tied data structure, $o in the
         * following Perl code.
         *
         * 	my %h;
         *  tie %h, 'FOO';
         *	my $o = bless \%h, 'BAR';
         *
         * Signal the tie-ing magic by setting the object type as SHT_EXTRA
         * (since we have only 2 bits in <flags> to store the type), and an
         * <extra> byte flag will be emitted after the FIRST <flags> in the
         * stream, carrying what we put in 'eflags'.
         */
        obj_type = SHT_EXTRA;
        switch (SvTYPE(sv)) {
        case SVt_PVHV:
            eflags = (unsigned char) SHT_THASH;
            mtype = 'P';
            break;
        case SVt_PVAV:
            eflags = (unsigned char) SHT_TARRAY;
            mtype = 'P';
            break;
        default:
            eflags = (unsigned char) SHT_TSCALAR;
            mtype = 'q';
            break;
        }
        break;
    default:
        {
            /* pkg_can() always returns a ref to a CV on success */
            CV *cv = (CV*)SvRV(hook);
            const GV * const gv = CvGV(cv);
            const char *gvname = GvNAME(gv);
            const HV * const stash = GvSTASH(gv);
            const char *hvname = stash ? HvNAME(stash) : NULL;

            CROAK(("Unexpected object type (%s) of class '%s' in store_hook() calling %s::%s",
                   sv_reftype(sv, FALSE), classname, hvname, gvname));
        }
    }
    flags = SHF_NEED_RECURSE | obj_type;

    /*
     * To call the hook, we need to fake a call like:
     *
     *    $object->STORABLE_freeze($cloning);
     *
     * but we don't have the $object here.  For instance, if $object is
     * a blessed array, what we have in 'sv' is the array, and we can't
     * call a method on those.
     *
     * Therefore, we need to create a temporary reference to the object and
     * make the call on that reference.
     */

    TRACEME(("about to call STORABLE_freeze on class %s", classname));

    ref = newRV_inc(sv);		/* Temporary reference */
    av = array_call(aTHX_ ref, hook, clone); /* @a = $object->STORABLE_freeze($c) */
    SvREFCNT_dec(ref);			/* Reclaim temporary reference */

    count = AvFILLp(av) + 1;
    TRACEME(("store_hook, array holds %" IVdf " items", count));

    /*
     * If they return an empty list, it means they wish to ignore the
     * hook for this class (and not just this instance -- that's for them
     * to handle if they so wish).
     *
     * Simply disable the cached entry for the hook (it won't be recomputed
     * since it's present in the cache) and recurse to store_blessed().
     */

    if (!count) {
        /* free empty list returned by the hook */
        av_undef(av);
        sv_free((SV *) av);

        /*
         * They must not change their mind in the middle of a serialization.
         */

        if (hv_fetch(cxt->hclass, classname, len, FALSE))
            CROAK(("Too late to ignore hooks for %s class \"%s\"",
                   (cxt->optype & ST_CLONE) ? "cloning" : "storing",
                   classname));

        pkg_hide(aTHX_ cxt->hook, pkg, "STORABLE_freeze");

        ASSERT(!pkg_can(aTHX_ cxt->hook, pkg, "STORABLE_freeze"),
               ("hook invisible"));
        TRACEME(("ignoring STORABLE_freeze in class \"%s\"", classname));

        return store_blessed(aTHX_ cxt, sv, type, pkg);
    }

    /*
     * Get frozen string.
     */

    ary = AvARRAY(av);
    pv = SvPV(ary[0], len2);
    /* We can't use pkg_can here because it only caches one method per
     * package */
    { 
        GV* gv = gv_fetchmethod_autoload(pkg, "STORABLE_attach", FALSE);
        if (gv && isGV(gv)) {
            if (count > 1)
                CROAK(("Freeze cannot return references if %s class is using STORABLE_attach", classname));
            goto check_done;
        }
    }

#ifdef HAS_U64
    if (count > I32_MAX) {
	CROAK(("Too many references returned by STORABLE_freeze()"));
    }
#endif

    /*
     * If they returned more than one item, we need to serialize some
     * extra references if not already done.
     *
     * Loop over the array, starting at position #1, and for each item,
     * ensure it is a reference, serialize it if not already done, and
     * replace the entry with the tag ID of the corresponding serialized
     * object.
     *
     * We CHEAT by not calling av_fetch() and read directly within the
     * array, for speed.
     */

    for (i = 1; i < count; i++) {
#ifdef USE_PTR_TABLE
        char *fake_tag;
#else
        SV **svh;
#endif
        SV *rsv = ary[i];
        SV *xsv;
        SV *tag;
        AV *av_hook = cxt->hook_seen;

        if (!SvROK(rsv))
            CROAK(("Item #%d returned by STORABLE_freeze "
                   "for %s is not a reference", (int)i, classname));
        xsv = SvRV(rsv);	/* Follow ref to know what to look for */

        /*
         * Look in hseen and see if we have a tag already.
         * Serialize entry if not done already, and get its tag.
         */

#ifdef USE_PTR_TABLE
        /* Fakery needed because ptr_table_fetch returns zero for a
           failure, whereas the existing code assumes that it can
           safely store a tag zero. So for ptr_tables we store tag+1
        */
        if ((fake_tag = (char *)ptr_table_fetch(cxt->pseen, xsv)))
            goto sv_seen;	/* Avoid moving code too far to the right */
#else
        if ((svh = hv_fetch(cxt->hseen, (char *) &xsv, sizeof(xsv), FALSE)))
            goto sv_seen;	/* Avoid moving code too far to the right */
#endif

        TRACEME(("listed object %d at 0x%" UVxf " is unknown", i-1,
                 PTR2UV(xsv)));

        /*
         * We need to recurse to store that object and get it to be known
         * so that we can resolve the list of object-IDs at retrieve time.
         *
         * The first time we do this, we need to emit the proper header
         * indicating that we recursed, and what the type of object is (the
         * object we're storing via a user-hook).  Indeed, during retrieval,
         * we'll have to create the object before recursing to retrieve the
         * others, in case those would point back at that object.
         */

        /* [SX_HOOK] <flags> [<extra>] <object>*/
        if (!recursed++) {
#ifdef HAS_U64
            if (len2 > INT32_MAX)
                PUTMARK(SX_LOBJECT);
#endif
	    PUTMARK(SX_HOOK);
            PUTMARK(flags);
            if (obj_type == SHT_EXTRA)
                PUTMARK(eflags);
        } else
            PUTMARK(flags);

        if ((ret = store(aTHX_ cxt, xsv)))	/* Given by hook for us to store */
            return ret;

#ifdef USE_PTR_TABLE
        fake_tag = (char *)ptr_table_fetch(cxt->pseen, xsv);
        if (!fake_tag)
            CROAK(("Could not serialize item #%d from hook in %s",
                   (int)i, classname));
#else
        svh = hv_fetch(cxt->hseen, (char *) &xsv, sizeof(xsv), FALSE);
        if (!svh)
            CROAK(("Could not serialize item #%d from hook in %s",
                   (int)i, classname));
#endif
        /*
         * It was the first time we serialized 'xsv'.
         *
         * Keep this SV alive until the end of the serialization: if we
         * disposed of it right now by decrementing its refcount, and it was
         * a temporary value, some next temporary value allocated during
         * another STORABLE_freeze might take its place, and we'd wrongly
         * assume that new SV was already serialized, based on its presence
         * in cxt->hseen.
         *
         * Therefore, push it away in cxt->hook_seen.
         */

        av_store(av_hook, AvFILLp(av_hook)+1, SvREFCNT_inc(xsv));

    sv_seen:
        /*
         * Dispose of the REF they returned.  If we saved the 'xsv' away
         * in the array of returned SVs, that will not cause the underlying
         * referenced SV to be reclaimed.
         */

        ASSERT(SvREFCNT(xsv) > 1, ("SV will survive disposal of its REF"));
        SvREFCNT_dec(rsv);		/* Dispose of reference */

        /*
         * Replace entry with its tag (not a real SV, so no refcnt increment)
         */

#ifdef USE_PTR_TABLE
        tag = (SV *)--fake_tag;
#else
        tag = *svh;
#endif
        ary[i] = tag;
        TRACEME(("listed object %d at 0x%" UVxf " is tag #%" UVuf,
                 i-1, PTR2UV(xsv), PTR2UV(tag)));
#ifdef HAS_U64
       if ((U32)PTR2TAG(tag) != PTR2TAG(tag))
           need_large_oids = 1;
#endif
    }

    /*
     * Allocate a class ID if not already done.
     *
     * This needs to be done after the recursion above, since at retrieval
     * time, we'll see the inner objects first.  Many thanks to
     * Salvador Ortiz Garcia <sog@msg.com.mx> who spot that bug and
     * proposed the right fix.  -- RAM, 15/09/2000
     */

 check_done:
    if (!known_class(aTHX_ cxt, classname, len, &classnum)) {
        TRACEME(("first time we see class %s, ID = %d", classname, (int)classnum));
        classnum = -1;			/* Mark: we must store classname */
    } else {
        TRACEME(("already seen class %s, ID = %d", classname, (int)classnum));
    }

    /*
     * Compute leading flags.
     */

    flags = obj_type;
    if (((classnum == -1) ? len : classnum) > LG_SCALAR)
        flags |= SHF_LARGE_CLASSLEN;
    if (classnum != -1)
        flags |= SHF_IDX_CLASSNAME;
    if (len2 > LG_SCALAR)
        flags |= SHF_LARGE_STRLEN;
    if (count > 1)
        flags |= SHF_HAS_LIST;
    if (count > (LG_SCALAR + 1))
        flags |= SHF_LARGE_LISTLEN;
#ifdef HAS_U64
    if (need_large_oids)
        flags |= SHF_LARGE_LISTLEN;
#endif

    /*
     * We're ready to emit either serialized form:
     *
     *   SX_HOOK <flags> <len> <classname> <len2> <str> [<len3> <object-IDs>]
     *   SX_HOOK <flags> <index>           <len2> <str> [<len3> <object-IDs>]
     *
     * If we recursed, the SX_HOOK has already been emitted.
     */

    TRACEME(("SX_HOOK (recursed=%d) flags=0x%x "
             "class=%" IVdf " len=%" IVdf " len2=%" IVdf " len3=%" IVdf,
             recursed, flags, (IV)classnum, (IV)len, (IV)len2, count-1));

    /* SX_HOOK <flags> [<extra>] */
    if (!recursed) {
#ifdef HAS_U64
        if (len2 > INT32_MAX)
	    PUTMARK(SX_LOBJECT);
#endif
	PUTMARK(SX_HOOK);
        PUTMARK(flags);
        if (obj_type == SHT_EXTRA)
            PUTMARK(eflags);
    } else
        PUTMARK(flags);

    /* <len> <classname> or <index> */
    if (flags & SHF_IDX_CLASSNAME) {
        if (flags & SHF_LARGE_CLASSLEN)
            WLEN(classnum);
        else {
            unsigned char cnum = (unsigned char) classnum;
            PUTMARK(cnum);
        }
    } else {
        if (flags & SHF_LARGE_CLASSLEN)
            WLEN(len);
        else {
            unsigned char clen = (unsigned char) len;
            PUTMARK(clen);
        }
        WRITE(classname, len);		/* Final \0 is omitted */
    }

    /* <len2> <frozen-str> */
#ifdef HAS_U64
    if (len2 > INT32_MAX) {
        W64LEN(len2);
    }
    else
#endif
    if (flags & SHF_LARGE_STRLEN) {
        U32 wlen2 = len2;		/* STRLEN might be 8 bytes */
        WLEN(wlen2);			/* Must write an I32 for 64-bit machines */
    } else {
        unsigned char clen = (unsigned char) len2;
        PUTMARK(clen);
    }
    if (len2)
        WRITE(pv, (SSize_t)len2);	/* Final \0 is omitted */

    /* [<len3> <object-IDs>] */
    if (flags & SHF_HAS_LIST) {
        int len3 = count - 1;
        if (flags & SHF_LARGE_LISTLEN) {
#ifdef HAS_U64
  	    int tlen3 = need_large_oids ? -len3 : len3;
	    WLEN(tlen3);
#else
            WLEN(len3);
#endif
	}
        else {
            unsigned char clen = (unsigned char) len3;
            PUTMARK(clen);
        }

        /*
         * NOTA BENE, for 64-bit machines: the ary[i] below does not yield a
         * real pointer, rather a tag number, well under the 32-bit limit.
         * Which is wrong... if we have more than 2**32 SVs we can get ids over
         * the 32-bit limit.
         */

        for (i = 1; i < count; i++) {
#ifdef HAS_U64
            if (need_large_oids) {
                ntag_t tag = PTR2TAG(ary[i]);
                W64LEN(tag);
                TRACEME(("object %d, tag #%" UVuf, i-1, (UV)tag));
            }
            else
#endif
            {
                I32 tagval = htonl(LOW_32BITS(ary[i]));
                WRITE_I32(tagval);
                TRACEME(("object %d, tag #%d", i-1, ntohl(tagval)));
            }
        }
    }

    /*
     * Free the array.  We need extra care for indices after 0, since they
     * don't hold real SVs but integers cast.
     */

    if (count > 1)
        AvFILLp(av) = 0;	/* Cheat, nothing after 0 interests us */
    av_undef(av);
    sv_free((SV *) av);

    /*
     * If object was tied, need to insert serialization of the magic object.
     */

    if (obj_type == SHT_EXTRA) {
        MAGIC *mg;

        if (!(mg = mg_find(sv, mtype))) {
            int svt = SvTYPE(sv);
            CROAK(("No magic '%c' found while storing ref to tied %s with hook",
                   mtype, (svt == SVt_PVHV) ? "hash" :
                   (svt == SVt_PVAV) ? "array" : "scalar"));
        }

        TRACEME(("handling the magic object 0x%" UVxf " part of 0x%" UVxf,
                 PTR2UV(mg->mg_obj), PTR2UV(sv)));

        /*
         * [<magic object>]
         */
        if ((ret = store(aTHX_ cxt, mg->mg_obj)))
            return ret;
    }

    return 0;
}

/*
 * store_blessed	-- dispatched manually, not via sv_store[]
 *
 * Check whether there is a STORABLE_xxx hook defined in the class or in one
 * of its ancestors.  If there is, then redispatch to store_hook();
 *
 * Otherwise, the blessed SV is stored using the following layout:
 *
 *    SX_BLESS <flag> <len> <classname> <object>
 *
 * where <flag> indicates whether <len> is stored on 0 or 4 bytes, depending
 * on the high-order bit in flag: if 1, then length follows on 4 bytes.
 * Otherwise, the low order bits give the length, thereby giving a compact
 * representation for class names less than 127 chars long.
 *
 * Each <classname> seen is remembered and indexed, so that the next time
 * an object in the blessed in the same <classname> is stored, the following
 * will be emitted:
 *
 *    SX_IX_BLESS <flag> <index> <object>
 *
 * where <index> is the classname index, stored on 0 or 4 bytes depending
 * on the high-order bit in flag (same encoding as above for <len>).
 */
static int store_blessed(
                         pTHX_
                         stcxt_t *cxt,
                         SV *sv,
                         int type,
                         HV *pkg)
{
    SV *hook;
    char *classname;
    I32 len;
    I32 classnum;

    TRACEME(("store_blessed, type %d, class \"%s\"", type, HvNAME_get(pkg)));

    /*
     * Look for a hook for this blessed SV and redirect to store_hook()
     * if needed.
     */

    hook = pkg_can(aTHX_ cxt->hook, pkg, "STORABLE_freeze");
    if (hook)
        return store_hook(aTHX_ cxt, sv, type, pkg, hook);

    /*
     * This is a blessed SV without any serialization hook.
     */

    classname = HvNAME_get(pkg);
    len = strlen(classname);

    TRACEME(("blessed 0x%" UVxf " in %s, no hook: tagged #%d",
             PTR2UV(sv), classname, (int)cxt->tagnum));

    /*
     * Determine whether it is the first time we see that class name (in which
     * case it will be stored in the SX_BLESS form), or whether we already
     * saw that class name before (in which case the SX_IX_BLESS form will be
     * used).
     */

    if (known_class(aTHX_ cxt, classname, len, &classnum)) {
        TRACEME(("already seen class %s, ID = %d", classname, (int)classnum));
        PUTMARK(SX_IX_BLESS);
        if (classnum <= LG_BLESS) {
            unsigned char cnum = (unsigned char) classnum;
            PUTMARK(cnum);
        } else {
            unsigned char flag = (unsigned char) 0x80;
            PUTMARK(flag);
            WLEN(classnum);
        }
    } else {
        TRACEME(("first time we see class %s, ID = %d", classname,
                 (int)classnum));
        PUTMARK(SX_BLESS);
        if (len <= LG_BLESS) {
            unsigned char clen = (unsigned char) len;
            PUTMARK(clen);
        } else {
            unsigned char flag = (unsigned char) 0x80;
            PUTMARK(flag);
            WLEN(len);	/* Don't BER-encode, this should be rare */
        }
        WRITE(classname, len);	/* Final \0 is omitted */
    }

    /*
     * Now emit the <object> part.
     */

    return SV_STORE(type)(aTHX_ cxt, sv);
}

/*
 * store_other
 *
 * We don't know how to store the item we reached, so return an error condition.
 * (it's probably a GLOB, some CODE reference, etc...)
 *
 * If they defined the 'forgive_me' variable at the Perl level to some
 * true value, then don't croak, just warn, and store a placeholder string
 * instead.
 */
static int store_other(pTHX_ stcxt_t *cxt, SV *sv)
{
    STRLEN len;
    char buf[80];

    TRACEME(("store_other"));

    /*
     * Fetch the value from perl only once per store() operation.
     */

    if (
        cxt->forgive_me == 0 ||
        (cxt->forgive_me < 0 &&
         !(cxt->forgive_me = SvTRUE
           (get_sv("Storable::forgive_me", GV_ADD)) ? 1 : 0))
	)
        CROAK(("Can't store %s items", sv_reftype(sv, FALSE)));

    warn("Can't store item %s(0x%" UVxf ")",
         sv_reftype(sv, FALSE), PTR2UV(sv));

    /*
     * Store placeholder string as a scalar instead...
     */

    (void) sprintf(buf, "You lost %s(0x%" UVxf ")%c", sv_reftype(sv, FALSE),
                   PTR2UV(sv), (char) 0);

    len = strlen(buf);
    if (len < 80)
        STORE_SCALAR(buf, len);
    TRACEME(("ok (dummy \"%s\", length = %" IVdf ")", buf, (IV) len));

    return 0;
}

/***
 *** Store driving routines
 ***/

/*
 * sv_type
 *
 * WARNING: partially duplicates Perl's sv_reftype for speed.
 *
 * Returns the type of the SV, identified by an integer. That integer
 * may then be used to index the dynamic routine dispatch table.
 */
static int sv_type(pTHX_ SV *sv)
{
    switch (SvTYPE(sv)) {
    case SVt_NULL:
#if PERL_VERSION_LT(5,11,0)
    case SVt_IV:
#endif
    case SVt_NV:
        /*
         * No need to check for ROK, that can't be set here since there
         * is no field capable of hodling the xrv_rv reference.
         */
        return svis_SCALAR;
    case SVt_PV:
#if PERL_VERSION_LT(5,11,0)
    case SVt_RV:
#else
    case SVt_IV:
#endif
    case SVt_PVIV:
    case SVt_PVNV:
        /*
         * Starting from SVt_PV, it is possible to have the ROK flag
         * set, the pointer to the other SV being either stored in
         * the xrv_rv (in the case of a pure SVt_RV), or as the
         * xpv_pv field of an SVt_PV and its heirs.
         *
         * However, those SV cannot be magical or they would be an
         * SVt_PVMG at least.
         */
        return SvROK(sv) ? svis_REF : svis_SCALAR;
    case SVt_PVMG:
#if PERL_VERSION_LT(5,11,0)
        if ((SvFLAGS(sv) & (SVs_OBJECT|SVf_OK|SVs_GMG|SVs_SMG|SVs_RMG))
	          == (SVs_OBJECT|BFD_Svs_SMG_OR_RMG)
	    && mg_find(sv, PERL_MAGIC_qr)) {
	      return svis_REGEXP;
	}
#endif
    case SVt_PVLV:		/* Workaround for perl5.004_04 "LVALUE" bug */
        if ((SvFLAGS(sv) & (SVs_GMG|SVs_SMG|SVs_RMG)) ==
            (SVs_GMG|SVs_SMG|SVs_RMG) &&
            (mg_find(sv, 'p')))
            return svis_TIED_ITEM;
        /* FALL THROUGH */
#if PERL_VERSION_LT(5,9,0)
    case SVt_PVBM:
#endif
        if ((SvFLAGS(sv) & (SVs_GMG|SVs_SMG|SVs_RMG)) ==
            (SVs_GMG|SVs_SMG|SVs_RMG) &&
            (mg_find(sv, 'q')))
            return svis_TIED;
        return SvROK(sv) ? svis_REF : svis_SCALAR;
    case SVt_PVAV:
        if (SvRMAGICAL(sv) && (mg_find(sv, 'P')))
            return svis_TIED;
        return svis_ARRAY;
    case SVt_PVHV:
        if (SvRMAGICAL(sv) && (mg_find(sv, 'P')))
            return svis_TIED;
        return svis_HASH;
    case SVt_PVCV:
        return svis_CODE;
#if PERL_VERSION_GE(5,9,0)
	/* case SVt_INVLIST: */
#endif
#if PERL_VERSION_GE(5,11,0)
    case SVt_REGEXP:
        return svis_REGEXP;
#endif
    default:
        break;
    }

    return svis_OTHER;
}

/*
 * store
 *
 * Recursively store objects pointed to by the sv to the specified file.
 *
 * Layout is <content> or SX_OBJECT <tagnum> if we reach an already stored
 * object (one for which storage has started -- it may not be over if we have
 * a self-referenced structure). This data set forms a stored <object>.
 */
static int store(pTHX_ stcxt_t *cxt, SV *sv)
{
    SV **svh;
    int ret;
    int type;
#ifdef USE_PTR_TABLE
    struct ptr_tbl *pseen = cxt->pseen;
#else
    HV *hseen = cxt->hseen;
#endif

    TRACEME(("store (0x%" UVxf ")", PTR2UV(sv)));

    /*
     * If object has already been stored, do not duplicate data.
     * Simply emit the SX_OBJECT marker followed by its tag data.
     * The tag is always written in network order.
     *
     * NOTA BENE, for 64-bit machines: the "*svh" below does not yield a
     * real pointer, rather a tag number (watch the insertion code below).
     * That means it probably safe to assume it is well under the 32-bit
     * limit, and makes the truncation safe.
     *		-- RAM, 14/09/1999
     */

#ifdef USE_PTR_TABLE
    svh = (SV **)ptr_table_fetch(pseen, sv);
#else
    svh = hv_fetch(hseen, (char *) &sv, sizeof(sv), FALSE);
#endif
    if (svh) {
	ntag_t tagval;
	if (sv == &PL_sv_undef) {
            /* We have seen PL_sv_undef before, but fake it as
               if we have not.

               Not the simplest solution to making restricted
               hashes work on 5.8.0, but it does mean that
               repeated references to the one true undef will
               take up less space in the output file.
            */
            /* Need to jump past the next hv_store, because on the
               second store of undef the old hash value will be
               SvREFCNT_dec()ed, and as Storable cheats horribly
               by storing non-SVs in the hash a SEGV will ensure.
               Need to increase the tag number so that the
               receiver has no idea what games we're up to.  This
               special casing doesn't affect hooks that store
               undef, as the hook routine does its own lookup into
               hseen.  Also this means that any references back
               to PL_sv_undef (from the pathological case of hooks
               storing references to it) will find the seen hash
               entry for the first time, as if we didn't have this
               hackery here. (That hseen lookup works even on 5.8.0
               because it's a key of &PL_sv_undef and a value
               which is a tag number, not a value which is
               PL_sv_undef.)  */
            cxt->tagnum++;
            type = svis_SCALAR;
            goto undef_special_case;
        }

#ifdef USE_PTR_TABLE
	tagval = PTR2TAG(((char *)svh)-1);
#else
	tagval = PTR2TAG(*svh);
#endif
#ifdef HAS_U64

       /* older versions of Storable streat the tag as a signed value
          used in an array lookup, corrupting the data structure.
          Ensure only a newer Storable will be able to parse this tag id
          if it's over the 2G mark.
        */
	if (tagval > I32_MAX) {

	    TRACEME(("object 0x%" UVxf " seen as #%" UVuf, PTR2UV(sv),
		     (UV)tagval));

	    PUTMARK(SX_LOBJECT);
	    PUTMARK(SX_OBJECT);
	    W64LEN(tagval);
	    return 0;
	}
	else
#endif
	{
	    I32 ltagval;

	    ltagval = htonl((I32)tagval);

	    TRACEME(("object 0x%" UVxf " seen as #%d", PTR2UV(sv),
		     ntohl(ltagval)));

	    PUTMARK(SX_OBJECT);
	    WRITE_I32(ltagval);
	    return 0;
	}
    }

    /*
     * Allocate a new tag and associate it with the address of the sv being
     * stored, before recursing...
     *
     * In order to avoid creating new SvIVs to hold the tagnum we just
     * cast the tagnum to an SV pointer and store that in the hash.  This
     * means that we must clean up the hash manually afterwards, but gives
     * us a 15% throughput increase.
     *
     */

    cxt->tagnum++;
#ifdef USE_PTR_TABLE
    ptr_table_store(pseen, sv, INT2PTR(SV*, 1 + cxt->tagnum));
#else
    if (!hv_store(hseen,
                  (char *) &sv, sizeof(sv), INT2PTR(SV*, cxt->tagnum), 0))
        return -1;
#endif

    /*
     * Store 'sv' and everything beneath it, using appropriate routine.
     * Abort immediately if we get a non-zero status back.
     */

    type = sv_type(aTHX_ sv);

 undef_special_case:
    TRACEME(("storing 0x%" UVxf " tag #%d, type %d...",
             PTR2UV(sv), (int)cxt->tagnum, (int)type));

    if (SvOBJECT(sv)) {
        HV *pkg = SvSTASH(sv);
        ret = store_blessed(aTHX_ cxt, sv, type, pkg);
    } else
        ret = SV_STORE(type)(aTHX_ cxt, sv);

    TRACEME(("%s (stored 0x%" UVxf ", refcnt=%d, %s)",
             ret ? "FAILED" : "ok", PTR2UV(sv),
             (int)SvREFCNT(sv), sv_reftype(sv, FALSE)));

    return ret;
}

/*
 * magic_write
 *
 * Write magic number and system information into the file.
 * Layout is <magic> <network> [<len> <byteorder> <sizeof int> <sizeof long>
 * <sizeof ptr>] where <len> is the length of the byteorder hexa string.
 * All size and lengths are written as single characters here.
 *
 * Note that no byte ordering info is emitted when <network> is true, since
 * integers will be emitted in network order in that case.
 */
static int magic_write(pTHX_ stcxt_t *cxt)
{
    /*
     * Starting with 0.6, the "use_network_order" byte flag is also used to
     * indicate the version number of the binary image, encoded in the upper
     * bits. The bit 0 is always used to indicate network order.
     */
    /*
     * Starting with 0.7, a full byte is dedicated to the minor version of
     * the binary format, which is incremented only when new markers are
     * introduced, for instance, but when backward compatibility is preserved.
     */

    /* Make these at compile time.  The WRITE() macro is sufficiently complex
       that it saves about 200 bytes doing it this way and only using it
       once.  */
    static const unsigned char network_file_header[] = {
        MAGICSTR_BYTES,
        (STORABLE_BIN_MAJOR << 1) | 1,
        STORABLE_BIN_WRITE_MINOR
    };
    static const unsigned char file_header[] = {
        MAGICSTR_BYTES,
        (STORABLE_BIN_MAJOR << 1) | 0,
        STORABLE_BIN_WRITE_MINOR,
        /* sizeof the array includes the 0 byte at the end:  */
        (char) sizeof (byteorderstr) - 1,
        BYTEORDER_BYTES,
        (unsigned char) sizeof(int),
        (unsigned char) sizeof(long),
        (unsigned char) sizeof(char *),
        (unsigned char) sizeof(NV)
    };
#ifdef USE_56_INTERWORK_KLUDGE
    static const unsigned char file_header_56[] = {
        MAGICSTR_BYTES,
        (STORABLE_BIN_MAJOR << 1) | 0,
        STORABLE_BIN_WRITE_MINOR,
        /* sizeof the array includes the 0 byte at the end:  */
        (char) sizeof (byteorderstr_56) - 1,
        BYTEORDER_BYTES_56,
        (unsigned char) sizeof(int),
        (unsigned char) sizeof(long),
        (unsigned char) sizeof(char *),
        (unsigned char) sizeof(NV)
    };
#endif
    const unsigned char *header;
    SSize_t length;

    TRACEME(("magic_write on fd=%d", cxt->fio ? PerlIO_fileno(cxt->fio) : -1));

    if (cxt->netorder) {
        header = network_file_header;
        length = sizeof (network_file_header);
    } else {
#ifdef USE_56_INTERWORK_KLUDGE
        if (SvTRUE(get_sv("Storable::interwork_56_64bit", GV_ADD))) {
            header = file_header_56;
            length = sizeof (file_header_56);
        } else
#endif
            {
                header = file_header;
                length = sizeof (file_header);
            }
    }

    if (!cxt->fio) {
        /* sizeof the array includes the 0 byte at the end.  */
        header += sizeof (magicstr) - 1;
        length -= sizeof (magicstr) - 1;
    }

    WRITE( (unsigned char*) header, length);

    if (!cxt->netorder) {
        TRACEME(("ok (magic_write byteorder = 0x%lx [%d], I%d L%d P%d D%d)",
                 (unsigned long) BYTEORDER, (int) sizeof (byteorderstr) - 1,
                 (int) sizeof(int), (int) sizeof(long),
                 (int) sizeof(char *), (int) sizeof(NV)));
    }
    return 0;
}

/*
 * do_store
 *
 * Common code for store operations.
 *
 * When memory store is requested (f = NULL) and a non null SV* is given in
 * 'res', it is filled with a new SV created out of the memory buffer.
 *
 * It is required to provide a non-null 'res' when the operation type is not
 * dclone() and store() is performed to memory.
 */
static int do_store(pTHX_
	PerlIO *f,
        SV *sv,
        int optype,
        int network_order,
        SV **res)
{
    dSTCXT;
    int status;

    ASSERT(!(f == 0 && !(optype & ST_CLONE)) || res,
           ("must supply result SV pointer for real recursion to memory"));

    TRACEMED(("do_store (optype=%d, netorder=%d)",
             optype, network_order));

    optype |= ST_STORE;

    /*
     * Workaround for CROAK leak: if they enter with a "dirty" context,
     * free up memory for them now.
     */

    assert(cxt);
    if (cxt->s_dirty)
        clean_context(aTHX_ cxt);

    /*
     * Now that STORABLE_xxx hooks exist, it is possible that they try to
     * re-enter store() via the hooks.  We need to stack contexts.
     */

    if (cxt->entry)
        cxt = allocate_context(aTHX_ cxt);

    INIT_TRACEME;

    cxt->entry++;

    ASSERT(cxt->entry == 1, ("starting new recursion"));
    ASSERT(!cxt->s_dirty, ("clean context"));

    /*
     * Ensure sv is actually a reference. From perl, we called something
     * like:
     *       pstore(aTHX_ FILE, \@array);
     * so we must get the scalar value behind that reference.
     */

    if (!SvROK(sv))
        CROAK(("Not a reference"));
    sv = SvRV(sv);		/* So follow it to know what to store */

    /* 
     * If we're going to store to memory, reset the buffer.
     */

    if (!f)
        MBUF_INIT(0);

    /*
     * Prepare context and emit headers.
     */

    init_store_context(aTHX_ cxt, f, optype, network_order);

    if (-1 == magic_write(aTHX_ cxt))	/* Emit magic and ILP info */
        return 0;			/* Error */

    /*
     * Recursively store object...
     */

    ASSERT(is_storing(aTHX), ("within store operation"));

    status = store(aTHX_ cxt, sv);	/* Just do it! */

    /*
     * If they asked for a memory store and they provided an SV pointer,
     * make an SV string out of the buffer and fill their pointer.
     *
     * When asking for ST_REAL, it's MANDATORY for the caller to provide
     * an SV, since context cleanup might free the buffer if we did recurse.
     * (unless caller is dclone(), which is aware of that).
     */

    if (!cxt->fio && res)
        *res = mbuf2sv(aTHX);

    TRACEME(("do_store returns %d", status));

    /*
     * Final cleanup.
     *
     * The "root" context is never freed, since it is meant to be always
     * handy for the common case where no recursion occurs at all (i.e.
     * we enter store() outside of any Storable code and leave it, period).
     * We know it's the "root" context because there's nothing stacked
     * underneath it.
     *
     * OPTIMIZATION:
     *
     * When deep cloning, we don't free the context: doing so would force
     * us to copy the data in the memory buffer.  Sicne we know we're
     * about to enter do_retrieve...
     */

    clean_store_context(aTHX_ cxt);
    if (cxt->prev && !(cxt->optype & ST_CLONE))
        free_context(aTHX_ cxt);

    return status == 0;
}

/***
 *** Memory stores.
 ***/

/*
 * mbuf2sv
 *
 * Build a new SV out of the content of the internal memory buffer.
 */
static SV *mbuf2sv(pTHX)
{
    dSTCXT;

    assert(cxt);
    return newSVpv(mbase, MBUF_SIZE());
}

/***
 *** Specific retrieve callbacks.
 ***/

/*
 * retrieve_other
 *
 * Return an error via croak, since it is not possible that we get here
 * under normal conditions, when facing a file produced via pstore().
 */
static SV *retrieve_other(pTHX_ stcxt_t *cxt, const char *cname)
{
    PERL_UNUSED_ARG(cname);
    if (
        cxt->ver_major != STORABLE_BIN_MAJOR &&
        cxt->ver_minor != STORABLE_BIN_MINOR
	) {
        CROAK(("Corrupted storable %s (binary v%d.%d), current is v%d.%d",
               cxt->fio ? "file" : "string",
               cxt->ver_major, cxt->ver_minor,
               STORABLE_BIN_MAJOR, STORABLE_BIN_MINOR));
    } else {
        CROAK(("Corrupted storable %s (binary v%d.%d)",
               cxt->fio ? "file" : "string",
               cxt->ver_major, cxt->ver_minor));
    }

    return (SV *) 0;		/* Just in case */
}

/*
 * retrieve_idx_blessed
 *
 * Layout is SX_IX_BLESS <index> <object> with SX_IX_BLESS already read.
 * <index> can be coded on either 1 or 5 bytes.
 */
static SV *retrieve_idx_blessed(pTHX_ stcxt_t *cxt, const char *cname)
{
    I32 idx;
    const char *classname;
    SV **sva;
    SV *sv;

    PERL_UNUSED_ARG(cname);
    TRACEME(("retrieve_idx_blessed (#%d)", (int)cxt->tagnum));
    ASSERT(!cname, ("no bless-into class given here, got %s", cname));

    GETMARK(idx);			/* Index coded on a single char? */
    if (idx & 0x80)
        RLEN(idx);

    /*
     * Fetch classname in 'aclass'
     */

    sva = av_fetch(cxt->aclass, idx, FALSE);
    if (!sva)
        CROAK(("Class name #%" IVdf " should have been seen already",
               (IV) idx));

    classname = SvPVX(*sva);	/* We know it's a PV, by construction */

    TRACEME(("class ID %d => %s", (int)idx, classname));

    /*
     * Retrieve object and bless it.
     */

    sv = retrieve(aTHX_ cxt, classname); /* First SV which is SEEN
                                            will be blessed */

    return sv;
}

/*
 * retrieve_blessed
 *
 * Layout is SX_BLESS <len> <classname> <object> with SX_BLESS already read.
 * <len> can be coded on either 1 or 5 bytes.
 */
static SV *retrieve_blessed(pTHX_ stcxt_t *cxt, const char *cname)
{
    U32 len;
    SV *sv;
    char buf[LG_BLESS + 1];		/* Avoid malloc() if possible */
    char *classname = buf;
    char *malloced_classname = NULL;

    PERL_UNUSED_ARG(cname);
    TRACEME(("retrieve_blessed (#%d)", (int)cxt->tagnum));
    ASSERT(!cname, ("no bless-into class given here, got %s", cname));

    /*
     * Decode class name length and read that name.
     *
     * Short classnames have two advantages: their length is stored on one
     * single byte, and the string can be read on the stack.
     */

    GETMARK(len);			/* Length coded on a single char? */
    if (len & 0x80) {
        RLEN(len);
        TRACEME(("** allocating %ld bytes for class name", (long)len+1));
        if (len > I32_MAX)
            CROAK(("Corrupted classname length %lu", (long)len));
        PL_nomemok = TRUE; /* handle error by ourselves */
        New(10003, classname, len+1, char);
        PL_nomemok = FALSE;
        if (!classname)
            CROAK(("Out of memory with len %ld", (long)len));
        PL_nomemok = FALSE;
        malloced_classname = classname;
    }
    SAFEPVREAD(classname, (I32)len, malloced_classname);
    classname[len] = '\0';		/* Mark string end */

    /*
     * It's a new classname, otherwise it would have been an SX_IX_BLESS.
     */

    TRACEME(("new class name \"%s\" will bear ID = %d", classname,
             (int)cxt->classnum));

    if (!av_store(cxt->aclass, cxt->classnum++, newSVpvn(classname, len))) {
        Safefree(malloced_classname);
        return (SV *) 0;
    }

    /*
     * Retrieve object and bless it.
     */

    sv = retrieve(aTHX_ cxt, classname); /* First SV which is SEEN will be blessed */
    if (malloced_classname)
        Safefree(malloced_classname);

    return sv;
}

/*
 * retrieve_hook
 *
 * Layout: SX_HOOK <flags> <len> <classname> <len2> <str> [<len3> <object-IDs>]
 * with leading mark already read, as usual.
 *
 * When recursion was involved during serialization of the object, there
 * is an unknown amount of serialized objects after the SX_HOOK mark.  Until
 * we reach a <flags> marker with the recursion bit cleared.
 *
 * If the first <flags> byte contains a type of SHT_EXTRA, then the real type
 * is held in the <extra> byte, and if the object is tied, the serialized
 * magic object comes at the very end:
 *
 *     SX_HOOK <flags> <extra> ... [<len3> <object-IDs>] <magic object>
 *
 * This means the STORABLE_thaw hook will NOT get a tied variable during its
 * processing (since we won't have seen the magic object by the time the hook
 * is called).  See comments below for why it was done that way.
 */
static SV *retrieve_hook_common(pTHX_ stcxt_t *cxt, const char *cname, int large)
{
    U32 len;
    char buf[LG_BLESS + 1];		/* Avoid malloc() if possible */
    char *classname = buf;
    unsigned int flags;
    STRLEN len2;
    SV *frozen;
    I32 len3 = 0;
    AV *av = 0;
    SV *hook;
    SV *sv;
    SV *rv;
    GV *attach;
    HV *stash;
    int obj_type;
    int clone = cxt->optype & ST_CLONE;
    char mtype = '\0';
    unsigned int extra_type = 0;
#ifdef HAS_U64
    int has_large_oids = 0;
#endif

    PERL_UNUSED_ARG(cname);
    TRACEME(("retrieve_hook (#%d)", (int)cxt->tagnum));
    ASSERT(!cname, ("no bless-into class given here, got %s", cname));

#ifndef HAS_U64
    assert(!large);
    PERL_UNUSED_ARG(large);
#endif

    /*
     * Read flags, which tell us about the type, and whether we need
     * to recurse.
     */

    GETMARK(flags);

    /*
     * Create the (empty) object, and mark it as seen.
     *
     * This must be done now, because tags are incremented, and during
     * serialization, the object tag was affected before recursion could
     * take place.
     */

    obj_type = flags & SHF_TYPE_MASK;
    switch (obj_type) {
    case SHT_SCALAR:
        sv = newSV(0);
        break;
    case SHT_ARRAY:
        sv = (SV *) newAV();
        break;
    case SHT_HASH:
        sv = (SV *) newHV();
        break;
    case SHT_EXTRA:
        /*
         * Read <extra> flag to know the type of the object.
         * Record associated magic type for later.
         */
        GETMARK(extra_type);
        switch (extra_type) {
        case SHT_TSCALAR:
            sv = newSV(0);
            mtype = 'q';
            break;
        case SHT_TARRAY:
            sv = (SV *) newAV();
            mtype = 'P';
            break;
        case SHT_THASH:
            sv = (SV *) newHV();
            mtype = 'P';
            break;
        default:
            return retrieve_other(aTHX_ cxt, 0);/* Let it croak */
        }
        break;
    default:
        return retrieve_other(aTHX_ cxt, 0);	/* Let it croak */
    }
    SEEN0_NN(sv, 0);				/* Don't bless yet */

    /*
     * Whilst flags tell us to recurse, do so.
     *
     * We don't need to remember the addresses returned by retrieval, because
     * all the references will be obtained through indirection via the object
     * tags in the object-ID list.
     *
     * We need to decrement the reference count for these objects
     * because, if the user doesn't save a reference to them in the hook,
     * they must be freed when this context is cleaned.
     */

    while (flags & SHF_NEED_RECURSE) {
        TRACEME(("retrieve_hook recursing..."));
        rv = retrieve(aTHX_ cxt, 0);
        if (!rv)
            return (SV *) 0;
        SvREFCNT_dec(rv);
        TRACEME(("retrieve_hook back with rv=0x%" UVxf,
                 PTR2UV(rv)));
        GETMARK(flags);
    }

    if (flags & SHF_IDX_CLASSNAME) {
        SV **sva;
        I32 idx;

        /*
         * Fetch index from 'aclass'
         */

        if (flags & SHF_LARGE_CLASSLEN)
            RLEN(idx);
        else
            GETMARK(idx);

        sva = av_fetch(cxt->aclass, idx, FALSE);
        if (!sva)
            CROAK(("Class name #%" IVdf " should have been seen already",
                   (IV) idx));

        classname = SvPVX(*sva);	/* We know it's a PV, by construction */
        TRACEME(("class ID %d => %s", (int)idx, classname));

    } else {
        /*
         * Decode class name length and read that name.
         *
         * NOTA BENE: even if the length is stored on one byte, we don't read
         * on the stack.  Just like retrieve_blessed(), we limit the name to
         * LG_BLESS bytes.  This is an arbitrary decision.
         */
        char *malloced_classname = NULL;

        if (flags & SHF_LARGE_CLASSLEN)
            RLEN(len);
        else
            GETMARK(len);

        TRACEME(("** allocating %ld bytes for class name", (long)len+1));
        if (len > I32_MAX) /* security */
            CROAK(("Corrupted classname length %lu", (long)len));
        else if (len > LG_BLESS) { /* security: signed len */
            PL_nomemok = TRUE;     /* handle error by ourselves */
            New(10003, classname, len+1, char);
            PL_nomemok = FALSE;
            if (!classname)
                CROAK(("Out of memory with len %u", (unsigned)len+1));
            malloced_classname = classname;
        }

        SAFEPVREAD(classname, (I32)len, malloced_classname);
        classname[len] = '\0';		/* Mark string end */

        /*
         * Record new classname.
         */

        if (!av_store(cxt->aclass, cxt->classnum++,
                      newSVpvn(classname, len))) {
            Safefree(malloced_classname);
            return (SV *) 0;
        }
    }

    TRACEME(("class name: %s", classname));

    /*
     * Decode user-frozen string length and read it in an SV.
     *
     * For efficiency reasons, we read data directly into the SV buffer.
     * To understand that code, read retrieve_scalar()
     */

#ifdef HAS_U64
    if (large) {
        READ_U64(len2);
    }
    else
#endif
    if (flags & SHF_LARGE_STRLEN) {
        U32 len32;
        RLEN(len32);
        len2 = len32;
    }
    else
        GETMARK(len2);

    frozen = NEWSV(10002, len2 ? len2 : 1);
    if (len2) {
        SAFEREAD(SvPVX(frozen), len2, frozen);
    }
    SvCUR_set(frozen, len2);
    *SvEND(frozen) = '\0';
    (void) SvPOK_only(frozen);		/* Validates string pointer */
    if (cxt->s_tainted)			/* Is input source tainted? */
        SvTAINT(frozen);

    TRACEME(("frozen string: %d bytes", (int)len2));

    /*
     * Decode object-ID list length, if present.
     */

    if (flags & SHF_HAS_LIST) {
        if (flags & SHF_LARGE_LISTLEN) {
            RLEN(len3);
	    if (len3 < 0) {
#ifdef HAS_U64
	        ++has_large_oids;
		len3 = -len3;
#else
		CROAK(("Large object ids in hook data not supported on 32-bit platforms"));
#endif
	        
	    }
	}
	else
            GETMARK(len3);
        if (len3) {
            av = newAV();
            av_extend(av, len3 + 1);	/* Leave room for [0] */
            AvFILLp(av) = len3;		/* About to be filled anyway */
        }
    }

    TRACEME(("has %d object IDs to link", (int)len3));

    /*
     * Read object-ID list into array.
     * Because we pre-extended it, we can cheat and fill it manually.
     *
     * We read object tags and we can convert them into SV* on the fly
     * because we know all the references listed in there (as tags)
     * have been already serialized, hence we have a valid correspondence
     * between each of those tags and the recreated SV.
     */

    if (av) {
        SV **ary = AvARRAY(av);
        int i;
        for (i = 1; i <= len3; i++) {	/* We leave [0] alone */
            ntag_t tag;
            SV **svh;
            SV *xsv;

#ifdef HAS_U64
	    if (has_large_oids) {
		READ_U64(tag);
	    }
	    else {
		U32 tmp;
		READ_I32(tmp);
		tag = ntohl(tmp);
	    }
#else
	    READ_I32(tag);
	    tag = ntohl(tag);
#endif

            svh = av_fetch(cxt->aseen, tag, FALSE);
            if (!svh) {
                if (tag == cxt->where_is_undef) {
                    /* av_fetch uses PL_sv_undef internally, hence this
                       somewhat gruesome hack. */
                    xsv = &PL_sv_undef;
                    svh = &xsv;
                } else {
                    CROAK(("Object #%" IVdf
                           " should have been retrieved already",
                           (IV) tag));
                }
            }
            xsv = *svh;
            ary[i] = SvREFCNT_inc(xsv);
        }
    }

    /*
     * Look up the STORABLE_attach hook
     * If blessing is disabled, just return what we've got.
     */
    if (!(cxt->flags & FLAG_BLESS_OK)) {
        TRACEME(("skipping bless because flags is %d", cxt->flags));
        return sv;
    }

    /*
     * Bless the object and look up the STORABLE_thaw hook.
     */
    stash = gv_stashpv(classname, GV_ADD);

    /* Handle attach case; again can't use pkg_can because it only
     * caches one method */
    attach = gv_fetchmethod_autoload(stash, "STORABLE_attach", FALSE);
    if (attach && isGV(attach)) {
        SV* attached;
        SV* attach_hook = newRV_inc((SV*) GvCV(attach));

        if (av)
            CROAK(("STORABLE_attach called with unexpected references"));
        av = newAV();
        av_extend(av, 1);
        AvFILLp(av) = 0;
        AvARRAY(av)[0] = SvREFCNT_inc(frozen);
        rv = newSVpv(classname, 0);
        attached = scalar_call(aTHX_ rv, attach_hook, clone, av, G_SCALAR);
        /* Free memory after a call */
        SvREFCNT_dec(rv);
        SvREFCNT_dec(frozen);
        av_undef(av);
        sv_free((SV *) av);
        SvREFCNT_dec(attach_hook);
        if (attached &&
            SvROK(attached) && 
            sv_derived_from(attached, classname)
            ) {
            UNSEE();
            /* refcnt of unneeded sv is 2 at this point
               (one from newHV, second from SEEN call) */
            SvREFCNT_dec(sv);
            SvREFCNT_dec(sv);
            /* we need to free RV but preserve value that RV point to */
            sv = SvRV(attached);
            SEEN0_NN(sv, 0);
            SvRV_set(attached, NULL);
            SvREFCNT_dec(attached);
            if (!(flags & SHF_IDX_CLASSNAME) && classname != buf)
                Safefree(classname);
            return sv;
        }
        CROAK(("STORABLE_attach did not return a %s object", classname));
    }

    /*
     * Bless the object and look up the STORABLE_thaw hook.
     */

    BLESS(sv, stash);

    hook = pkg_can(aTHX_ cxt->hook, stash, "STORABLE_thaw");
    if (!hook) {
        /*
         * Hook not found.  Maybe they did not require the module where this
         * hook is defined yet?
         *
         * If the load below succeeds, we'll be able to find the hook.
         * Still, it only works reliably when each class is defined in a
         * file of its own.
         */

        TRACEME(("No STORABLE_thaw defined for objects of class %s", classname));
        TRACEME(("Going to load module '%s'", classname));
        load_module(PERL_LOADMOD_NOIMPORT, newSVpv(classname, 0), Nullsv);

        /*
         * We cache results of pkg_can, so we need to uncache before attempting
         * the lookup again.
         */

        pkg_uncache(aTHX_ cxt->hook, SvSTASH(sv), "STORABLE_thaw");
        hook = pkg_can(aTHX_ cxt->hook, SvSTASH(sv), "STORABLE_thaw");

        if (!hook)
            CROAK(("No STORABLE_thaw defined for objects of class %s "
                   "(even after a \"require %s;\")", classname, classname));
    }

    /*
     * If we don't have an 'av' yet, prepare one.
     * Then insert the frozen string as item [0].
     */

    if (!av) {
        av = newAV();
        av_extend(av, 1);
        AvFILLp(av) = 0;
    }
    AvARRAY(av)[0] = SvREFCNT_inc(frozen);

    /*
     * Call the hook as:
     *
     *   $object->STORABLE_thaw($cloning, $frozen, @refs);
     *
     * where $object is our blessed (empty) object, $cloning is a boolean
     * telling whether we're running a deep clone, $frozen is the frozen
     * string the user gave us in his serializing hook, and @refs, which may
     * be empty, is the list of extra references he returned along for us
     * to serialize.
     *
     * In effect, the hook is an alternate creation routine for the class,
     * the object itself being already created by the runtime.
     */

    TRACEME(("calling STORABLE_thaw on %s at 0x%" UVxf " (%" IVdf " args)",
             classname, PTR2UV(sv), (IV) AvFILLp(av) + 1));

    rv = newRV_inc(sv);
    (void) scalar_call(aTHX_ rv, hook, clone, av, G_SCALAR|G_DISCARD);
    SvREFCNT_dec(rv);

    /*
     * Final cleanup.
     */

    SvREFCNT_dec(frozen);
    av_undef(av);
    sv_free((SV *) av);
    if (!(flags & SHF_IDX_CLASSNAME) && classname != buf)
        Safefree(classname);

    /*
     * If we had an <extra> type, then the object was not as simple, and
     * we need to restore extra magic now.
     */

    if (!extra_type)
        return sv;

    TRACEME(("retrieving magic object for 0x%" UVxf "...", PTR2UV(sv)));

    rv = retrieve(aTHX_ cxt, 0);	/* Retrieve <magic object> */

    TRACEME(("restoring the magic object 0x%" UVxf " part of 0x%" UVxf,
             PTR2UV(rv), PTR2UV(sv)));

    switch (extra_type) {
    case SHT_TSCALAR:
        sv_upgrade(sv, SVt_PVMG);
        break;
    case SHT_TARRAY:
        sv_upgrade(sv, SVt_PVAV);
        AvREAL_off((AV *)sv);
        break;
    case SHT_THASH:
        sv_upgrade(sv, SVt_PVHV);
        break;
    default:
        CROAK(("Forgot to deal with extra type %d", extra_type));
        break;
    }

    /*
     * Adding the magic only now, well after the STORABLE_thaw hook was called
     * means the hook cannot know it deals with an object whose variable is
     * tied.  But this is happening when retrieving $o in the following case:
     *
     *	my %h;
     *  tie %h, 'FOO';
     *	my $o = bless \%h, 'BAR';
     *
     * The 'BAR' class is NOT the one where %h is tied into.  Therefore, as
     * far as the 'BAR' class is concerned, the fact that %h is not a REAL
     * hash but a tied one should not matter at all, and remain transparent.
     * This means the magic must be restored by Storable AFTER the hook is
     * called.
     *
     * That looks very reasonable to me, but then I've come up with this
     * after a bug report from David Nesting, who was trying to store such
     * an object and caused Storable to fail.  And unfortunately, it was
     * also the easiest way to retrofit support for blessed ref to tied objects
     * into the existing design.  -- RAM, 17/02/2001
     */

    sv_magic(sv, rv, mtype, (char *)NULL, 0);
    SvREFCNT_dec(rv);			/* Undo refcnt inc from sv_magic() */

    return sv;
}

static SV *retrieve_hook(pTHX_ stcxt_t *cxt, const char *cname) {
    return retrieve_hook_common(aTHX_ cxt, cname, FALSE);
}

/*
 * retrieve_ref
 *
 * Retrieve reference to some other scalar.
 * Layout is SX_REF <object>, with SX_REF already read.
 */
static SV *retrieve_ref(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *rv;
    SV *sv;
    HV *stash;

    TRACEME(("retrieve_ref (#%d)", (int)cxt->tagnum));

    /*
     * We need to create the SV that holds the reference to the yet-to-retrieve
     * object now, so that we may record the address in the seen table.
     * Otherwise, if the object to retrieve references us, we won't be able
     * to resolve the SX_OBJECT we'll see at that point! Hence we cannot
     * do the retrieve first and use rv = newRV(sv) since it will be too late
     * for SEEN() recording.
     */

    rv = NEWSV(10002, 0);
    if (cname)
        stash = gv_stashpv(cname, GV_ADD);
    else
        stash = 0;
    SEEN_NN(rv, stash, 0);	/* Will return if rv is null */
    sv = retrieve(aTHX_ cxt, 0);/* Retrieve <object> */
    if (!sv)
        return (SV *) 0;	/* Failed */

    /*
     * WARNING: breaks RV encapsulation.
     *
     * Now for the tricky part. We have to upgrade our existing SV, so that
     * it is now an RV on sv... Again, we cheat by duplicating the code
     * held in newSVrv(), since we already got our SV from retrieve().
     *
     * We don't say:
     *
     *		SvRV(rv) = SvREFCNT_inc(sv);
     *
     * here because the reference count we got from retrieve() above is
     * already correct: if the object was retrieved from the file, then
     * its reference count is one. Otherwise, if it was retrieved via
     * an SX_OBJECT indication, a ref count increment was done.
     */

    if (cname) {
        /* No need to do anything, as rv will already be PVMG.  */
        assert (SvTYPE(rv) == SVt_RV || SvTYPE(rv) >= SVt_PV);
    } else {
        sv_upgrade(rv, SVt_RV);
    }

    SvRV_set(rv, sv);		/* $rv = \$sv */
    SvROK_on(rv);
    /*if (cxt->entry && ++cxt->ref_cnt > MAX_REF_CNT) {
        CROAK(("Max. recursion depth with nested refs exceeded"));
    }*/

    TRACEME(("ok (retrieve_ref at 0x%" UVxf ")", PTR2UV(rv)));

    return rv;
}

/*
 * retrieve_weakref
 *
 * Retrieve weak reference to some other scalar.
 * Layout is SX_WEAKREF <object>, with SX_WEAKREF already read.
 */
static SV *retrieve_weakref(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;

    TRACEME(("retrieve_weakref (#%d)", (int)cxt->tagnum));

    sv = retrieve_ref(aTHX_ cxt, cname);
    if (sv) {
#ifdef SvWEAKREF
        sv_rvweaken(sv);
#else
        WEAKREF_CROAK();
#endif
    }
    return sv;
}

/*
 * retrieve_overloaded
 *
 * Retrieve reference to some other scalar with overloading.
 * Layout is SX_OVERLOAD <object>, with SX_OVERLOAD already read.
 */
static SV *retrieve_overloaded(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *rv;
    SV *sv;
    HV *stash;

    TRACEME(("retrieve_overloaded (#%d)", (int)cxt->tagnum));

    /*
     * Same code as retrieve_ref(), duplicated to avoid extra call.
     */

    rv = NEWSV(10002, 0);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(rv, stash, 0);	     /* Will return if rv is null */
    cxt->in_retrieve_overloaded = 1; /* so sv_bless doesn't call S_reset_amagic */
    sv = retrieve(aTHX_ cxt, 0);     /* Retrieve <object> */
    cxt->in_retrieve_overloaded = 0;
    if (!sv)
        return (SV *) 0;	/* Failed */

    /*
     * WARNING: breaks RV encapsulation.
     */

    SvUPGRADE(rv, SVt_RV);
    SvRV_set(rv, sv);		/* $rv = \$sv */
    SvROK_on(rv);

    /*
     * Restore overloading magic.
     */

    stash = SvTYPE(sv) ? (HV *) SvSTASH (sv) : 0;
    if (!stash) {
        CROAK(("Cannot restore overloading on %s(0x%" UVxf
               ") (package <unknown>)",
               sv_reftype(sv, FALSE),
               PTR2UV(sv)));
    }
    if (!Gv_AMG(stash)) {
        const char *package = HvNAME_get(stash);
        TRACEME(("No overloading defined for package %s", package));
        TRACEME(("Going to load module '%s'", package));
        load_module(PERL_LOADMOD_NOIMPORT, newSVpv(package, 0), Nullsv);
        if (!Gv_AMG(stash)) {
            CROAK(("Cannot restore overloading on %s(0x%" UVxf
                   ") (package %s) (even after a \"require %s;\")",
                   sv_reftype(sv, FALSE),
                   PTR2UV(sv),
                   package, package));
        }
    }

    SvAMAGIC_on(rv);

    TRACEME(("ok (retrieve_overloaded at 0x%" UVxf ")", PTR2UV(rv)));

    return rv;
}

/*
 * retrieve_weakoverloaded
 *
 * Retrieve weak overloaded reference to some other scalar.
 * Layout is SX_WEAKOVERLOADED <object>, with SX_WEAKOVERLOADED already read.
 */
static SV *retrieve_weakoverloaded(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;

    TRACEME(("retrieve_weakoverloaded (#%d)", (int)cxt->tagnum));

    sv = retrieve_overloaded(aTHX_ cxt, cname);
    if (sv) {
#ifdef SvWEAKREF
        sv_rvweaken(sv);
#else
        WEAKREF_CROAK();
#endif
    }
    return sv;
}

/*
 * retrieve_tied_array
 *
 * Retrieve tied array
 * Layout is SX_TIED_ARRAY <object>, with SX_TIED_ARRAY already read.
 */
static SV *retrieve_tied_array(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *tv;
    SV *sv;
    HV *stash;

    TRACEME(("retrieve_tied_array (#%d)", (int)cxt->tagnum));

    if (!(cxt->flags & FLAG_TIE_OK)) {
        CROAK(("Tying is disabled."));
    }

    tv = NEWSV(10002, 0);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(tv, stash, 0);		/* Will return if tv is null */
    sv = retrieve(aTHX_ cxt, 0);	/* Retrieve <object> */
    if (!sv)
        return (SV *) 0;		/* Failed */

    sv_upgrade(tv, SVt_PVAV);
    sv_magic(tv, sv, 'P', (char *)NULL, 0);
    SvREFCNT_dec(sv);			/* Undo refcnt inc from sv_magic() */

    TRACEME(("ok (retrieve_tied_array at 0x%" UVxf ")", PTR2UV(tv)));

    return tv;
}

/*
 * retrieve_tied_hash
 *
 * Retrieve tied hash
 * Layout is SX_TIED_HASH <object>, with SX_TIED_HASH already read.
 */
static SV *retrieve_tied_hash(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *tv;
    SV *sv;
    HV *stash;

    TRACEME(("retrieve_tied_hash (#%d)", (int)cxt->tagnum));

    if (!(cxt->flags & FLAG_TIE_OK)) {
        CROAK(("Tying is disabled."));
    }

    tv = NEWSV(10002, 0);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(tv, stash, 0);		/* Will return if tv is null */
    sv = retrieve(aTHX_ cxt, 0);	/* Retrieve <object> */
    if (!sv)
        return (SV *) 0;		/* Failed */

    sv_upgrade(tv, SVt_PVHV);
    sv_magic(tv, sv, 'P', (char *)NULL, 0);
    SvREFCNT_dec(sv);			/* Undo refcnt inc from sv_magic() */

    TRACEME(("ok (retrieve_tied_hash at 0x%" UVxf ")", PTR2UV(tv)));

    return tv;
}

/*
 * retrieve_tied_scalar
 *
 * Retrieve tied scalar
 * Layout is SX_TIED_SCALAR <object>, with SX_TIED_SCALAR already read.
 */
static SV *retrieve_tied_scalar(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *tv;
    SV *sv, *obj = NULL;
    HV *stash;

    TRACEME(("retrieve_tied_scalar (#%d)", (int)cxt->tagnum));

    if (!(cxt->flags & FLAG_TIE_OK)) {
        CROAK(("Tying is disabled."));
    }

    tv = NEWSV(10002, 0);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(tv, stash, 0);		/* Will return if rv is null */
    sv = retrieve(aTHX_ cxt, 0);	/* Retrieve <object> */
    if (!sv) {
        return (SV *) 0;		/* Failed */
    }
    else if (SvTYPE(sv) != SVt_NULL) {
        obj = sv;
    }

    sv_upgrade(tv, SVt_PVMG);
    sv_magic(tv, obj, 'q', (char *)NULL, 0);

    if (obj) {
        /* Undo refcnt inc from sv_magic() */
        SvREFCNT_dec(obj);
    }

    TRACEME(("ok (retrieve_tied_scalar at 0x%" UVxf ")", PTR2UV(tv)));

    return tv;
}

/*
 * retrieve_tied_key
 *
 * Retrieve reference to value in a tied hash.
 * Layout is SX_TIED_KEY <object> <key>, with SX_TIED_KEY already read.
 */
static SV *retrieve_tied_key(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *tv;
    SV *sv;
    SV *key;
    HV *stash;

    TRACEME(("retrieve_tied_key (#%d)", (int)cxt->tagnum));

    if (!(cxt->flags & FLAG_TIE_OK)) {
        CROAK(("Tying is disabled."));
    }

    tv = NEWSV(10002, 0);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(tv, stash, 0);		/* Will return if tv is null */
    sv = retrieve(aTHX_ cxt, 0);	/* Retrieve <object> */
    if (!sv)
        return (SV *) 0;		/* Failed */

    key = retrieve(aTHX_ cxt, 0);	/* Retrieve <key> */
    if (!key)
        return (SV *) 0;		/* Failed */

    sv_upgrade(tv, SVt_PVMG);
    sv_magic(tv, sv, 'p', (char *)key, HEf_SVKEY);
    SvREFCNT_dec(key);			/* Undo refcnt inc from sv_magic() */
    SvREFCNT_dec(sv);			/* Undo refcnt inc from sv_magic() */

    return tv;
}

/*
 * retrieve_tied_idx
 *
 * Retrieve reference to value in a tied array.
 * Layout is SX_TIED_IDX <object> <idx>, with SX_TIED_IDX already read.
 */
static SV *retrieve_tied_idx(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *tv;
    SV *sv;
    HV *stash;
    I32 idx;

    TRACEME(("retrieve_tied_idx (#%d)", (int)cxt->tagnum));

    if (!(cxt->flags & FLAG_TIE_OK)) {
        CROAK(("Tying is disabled."));
    }

    tv = NEWSV(10002, 0);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(tv, stash, 0);		/* Will return if tv is null */
    sv = retrieve(aTHX_ cxt, 0);	/* Retrieve <object> */
    if (!sv)
        return (SV *) 0;		/* Failed */

    RLEN(idx);				/* Retrieve <idx> */

    sv_upgrade(tv, SVt_PVMG);
    sv_magic(tv, sv, 'p', (char *)NULL, idx);
    SvREFCNT_dec(sv);			/* Undo refcnt inc from sv_magic() */

    return tv;
}

/*
 * get_lstring
 *
 * Helper to read a string
 */
static SV *get_lstring(pTHX_ stcxt_t *cxt, UV len, int isutf8, const char *cname)
{
    SV *sv;
    HV *stash;

    TRACEME(("get_lstring (#%d), len = %" UVuf, (int)cxt->tagnum, len));

    /*
     * Allocate an empty scalar of the suitable length.
     */

    sv = NEWSV(10002, len);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);	/* Associate this new scalar with tag "tagnum" */

    if (len ==  0) {
        SvPVCLEAR(sv);
        return sv;
    }

    /*
     * WARNING: duplicates parts of sv_setpv and breaks SV data encapsulation.
     *
     * Now, for efficiency reasons, read data directly inside the SV buffer,
     * and perform the SV final settings directly by duplicating the final
     * work done by sv_setpv. Since we're going to allocate lots of scalars
     * this way, it's worth the hassle and risk.
     */

    SAFEREAD(SvPVX(sv), len, sv);
    SvCUR_set(sv, len);			/* Record C string length */
    *SvEND(sv) = '\0';			/* Ensure it's null terminated anyway */
    (void) SvPOK_only(sv);		/* Validate string pointer */
    if (cxt->s_tainted)			/* Is input source tainted? */
        SvTAINT(sv);			/* External data cannot be trusted */

    /* Check for CVE-215-1592 */
    if (cname && len == 13 && strEQc(cname, "CGITempFile")
        && strEQc(SvPVX(sv), "mt-config.cgi")) {
#if defined(USE_CPERL) && defined(WARN_SECURITY)
        Perl_warn_security(aTHX_
            "Movable-Type CVE-2015-1592 Storable metasploit attack");
#else
        Perl_warn(aTHX_
            "SECURITY: Movable-Type CVE-2015-1592 Storable metasploit attack");
#endif
    }

    if (isutf8) {
        TRACEME(("large utf8 string len %" UVuf " '%s'", len,
                 len >= 2048 ? "<string too long>" : SvPVX(sv)));
#ifdef HAS_UTF8_SCALARS
        SvUTF8_on(sv);
#else
        if (cxt->use_bytes < 0)
            cxt->use_bytes
                = (SvTRUE(get_sv("Storable::drop_utf8", GV_ADD))
                   ? 1 : 0);
        if (cxt->use_bytes == 0)
            UTF8_CROAK();
#endif
    } else {
        TRACEME(("large string len %" UVuf " '%s'", len,
                 len >= 2048 ? "<string too long>" : SvPVX(sv)));
    }
    TRACEME(("ok (get_lstring at 0x%" UVxf ")", PTR2UV(sv)));

    return sv;
}

/*
 * retrieve_lscalar
 *
 * Retrieve defined long (string) scalar.
 *
 * Layout is SX_LSCALAR <length> <data>, with SX_LSCALAR already read.
 * The scalar is "long" in that <length> is larger than LG_SCALAR so it
 * was not stored on a single byte, but in 4 bytes. For strings longer than
 * 4 byte (>2GB) see retrieve_lobject.
 */
static SV *retrieve_lscalar(pTHX_ stcxt_t *cxt, const char *cname)
{
    U32 len;
    RLEN(len);
    return get_lstring(aTHX_ cxt, len, 0, cname);
}

/*
 * retrieve_scalar
 *
 * Retrieve defined short (string) scalar.
 *
 * Layout is SX_SCALAR <length> <data>, with SX_SCALAR already read.
 * The scalar is "short" so <length> is single byte. If it is 0, there
 * is no <data> section.
 */
static SV *retrieve_scalar(pTHX_ stcxt_t *cxt, const char *cname)
{
    int len;
    /*SV *sv;
      HV *stash;*/

    GETMARK(len);
    TRACEME(("retrieve_scalar (#%d), len = %d", (int)cxt->tagnum, len));
    return get_lstring(aTHX_ cxt, (UV)len, 0, cname);
}

/*
 * retrieve_utf8str
 *
 * Like retrieve_scalar(), but tag result as utf8.
 * If we're retrieving UTF8 data in a non-UTF8 perl, croaks.
 */
static SV *retrieve_utf8str(pTHX_ stcxt_t *cxt, const char *cname)
{
    int len;
    /*SV *sv;*/

    TRACEME(("retrieve_utf8str"));
    GETMARK(len);
    return get_lstring(aTHX_ cxt, (UV)len, 1, cname);
}

/*
 * retrieve_lutf8str
 *
 * Like retrieve_lscalar(), but tag result as utf8.
 * If we're retrieving UTF8 data in a non-UTF8 perl, croaks.
 */
static SV *retrieve_lutf8str(pTHX_ stcxt_t *cxt, const char *cname)
{
    U32 len;

    TRACEME(("retrieve_lutf8str"));

    RLEN(len);
    return get_lstring(aTHX_ cxt, (UV)len, 1, cname);
}

/*
 * retrieve_vstring
 *
 * Retrieve a vstring, and then retrieve the stringy scalar following it,
 * attaching the vstring to the scalar via magic.
 * If we're retrieving a vstring in a perl without vstring magic, croaks.
 *
 * The vstring layout mirrors an SX_SCALAR string:
 * SX_VSTRING <length> <data> with SX_VSTRING already read.
 */
static SV *retrieve_vstring(pTHX_ stcxt_t *cxt, const char *cname)
{
#ifdef SvVOK
    char s[256];
    int len;
    SV *sv;

    GETMARK(len);
    TRACEME(("retrieve_vstring (#%d), len = %d", (int)cxt->tagnum, len));

    READ(s, len);
    sv = retrieve(aTHX_ cxt, cname);
    if (!sv)
        return (SV *) 0;		/* Failed */
    sv_magic(sv,NULL,PERL_MAGIC_vstring,s,len);
    /* 5.10.0 and earlier seem to need this */
    SvRMAGICAL_on(sv);

    TRACEME(("ok (retrieve_vstring at 0x%" UVxf ")", PTR2UV(sv)));
    return sv;
#else
    VSTRING_CROAK();
    return Nullsv;
#endif
}

/*
 * retrieve_lvstring
 *
 * Like retrieve_vstring, but for longer vstrings.
 */
static SV *retrieve_lvstring(pTHX_ stcxt_t *cxt, const char *cname)
{
#ifdef SvVOK
    char *s;
    U32 len;
    SV *sv;

    RLEN(len);
    TRACEME(("retrieve_lvstring (#%d), len = %" UVuf,
             (int)cxt->tagnum, (UV)len));

    /* Since we'll no longer produce such large vstrings, reject them
       here too.
    */
    if (len >= I32_MAX) {
        CROAK(("vstring too large to fetch"));
    }

    New(10003, s, len+1, char);
    SAFEPVREAD(s, (I32)len, s);

    sv = retrieve(aTHX_ cxt, cname);
    if (!sv) {
        Safefree(s);
        return (SV *) 0;		/* Failed */
    }
    sv_magic(sv,NULL,PERL_MAGIC_vstring,s,len);
    /* 5.10.0 and earlier seem to need this */
    SvRMAGICAL_on(sv);

    Safefree(s);

    TRACEME(("ok (retrieve_lvstring at 0x%" UVxf ")", PTR2UV(sv)));
    return sv;
#else
    VSTRING_CROAK();
    return Nullsv;
#endif
}

/*
 * retrieve_integer
 *
 * Retrieve defined integer.
 * Layout is SX_INTEGER <data>, whith SX_INTEGER already read.
 */
static SV *retrieve_integer(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;
    HV *stash;
    IV iv;

    TRACEME(("retrieve_integer (#%d)", (int)cxt->tagnum));

    READ(&iv, sizeof(iv));
    sv = newSViv(iv);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);	/* Associate this new scalar with tag "tagnum" */

    TRACEME(("integer %" IVdf, iv));
    TRACEME(("ok (retrieve_integer at 0x%" UVxf ")", PTR2UV(sv)));

    return sv;
}

/*
 * retrieve_boolean_true
 *
 * Retrieve boolean true copy.
 */
static SV *retrieve_boolean_true(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;
    HV *stash;

    TRACEME(("retrieve_boolean_true (#%d)", (int)cxt->tagnum));

    sv = newSVsv(&PL_sv_yes);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);  /* Associate this new scalar with tag "tagnum" */

    TRACEME(("boolean true"));
    TRACEME(("ok (retrieve_boolean_true at 0x%" UVxf ")", PTR2UV(sv)));

    return sv;
}

/*
 * retrieve_boolean_false
 *
 * Retrieve boolean false copy.
 */
static SV *retrieve_boolean_false(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;
    HV *stash;

    TRACEME(("retrieve_boolean_false (#%d)", (int)cxt->tagnum));

    sv = newSVsv(&PL_sv_no);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);  /* Associate this new scalar with tag "tagnum" */

    TRACEME(("boolean false"));
    TRACEME(("ok (retrieve_boolean_false at 0x%" UVxf ")", PTR2UV(sv)));

    return sv;
}

/*
 * retrieve_lobject
 *
 * Retrieve overlong scalar, array or hash.
 * Layout is SX_LOBJECT type U64_len ...
 */
static SV *retrieve_lobject(pTHX_ stcxt_t *cxt, const char *cname)
{
    int type;
#ifdef HAS_U64
    UV  len;
    SV *sv;
    int hash_flags = 0;
#endif

    TRACEME(("retrieve_lobject (#%d)", (int)cxt->tagnum));

    GETMARK(type);
    TRACEME(("object type %d", type));
#ifdef HAS_U64

    if (type == SX_FLAG_HASH) {
	/* we write the flags immediately after the op.  I could have
	   changed the writer, but this may allow someone to recover
	   data they're already frozen, though such a very large hash
	   seems unlikely.
	*/
	GETMARK(hash_flags);
    }
    else if (type == SX_HOOK) {
        return retrieve_hook_common(aTHX_ cxt, cname, TRUE);
    }

    READ_U64(len);
    TRACEME(("wlen %" UVuf, len));
    switch (type) {
    case SX_OBJECT:
        {
            /* not a large object, just a large index */
            SV **svh = av_fetch(cxt->aseen, len, FALSE);
            if (!svh)
                CROAK(("Object #%" UVuf " should have been retrieved already",
                      len));
            sv = *svh;
            TRACEME(("had retrieved #%" UVuf " at 0x%" UVxf, len, PTR2UV(sv)));
            SvREFCNT_inc(sv);
        }
        break;
    case SX_LSCALAR:
        sv = get_lstring(aTHX_ cxt, len, 0, cname);
        break;
    case SX_LUTF8STR:
        sv = get_lstring(aTHX_ cxt, len, 1, cname);
        break;
    case SX_ARRAY:
        sv = get_larray(aTHX_ cxt, len, cname);
        break;
    /* <5.12 you could store larger hashes, but cannot iterate over them.
       So we reject them, it's a bug. */
    case SX_FLAG_HASH:
        sv = get_lhash(aTHX_ cxt, len, hash_flags, cname);
        break;
    case SX_HASH:
        sv = get_lhash(aTHX_ cxt, len, 0, cname);
        break;
    default:
        CROAK(("Unexpected type %d in retrieve_lobject\n", type));
    }

    TRACEME(("ok (retrieve_lobject at 0x%" UVxf ")", PTR2UV(sv)));
    return sv;
#else
    PERL_UNUSED_ARG(cname);

    /* previously this (brokenly) checked the length value and only failed if 
       the length was over 4G.
       Since this op should only occur with objects over 4GB (or 2GB) we can just
       reject it.
    */
    CROAK(("Invalid large object op for this 32bit system"));
#endif
}

/*
 * retrieve_netint
 *
 * Retrieve defined integer in network order.
 * Layout is SX_NETINT <data>, whith SX_NETINT already read.
 */
static SV *retrieve_netint(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;
    HV *stash;
    I32 iv;

    TRACEME(("retrieve_netint (#%d)", (int)cxt->tagnum));

    READ_I32(iv);
#ifdef HAS_NTOHL
    sv = newSViv((int) ntohl(iv));
    TRACEME(("network integer %d", (int) ntohl(iv)));
#else
    sv = newSViv(iv);
    TRACEME(("network integer (as-is) %d", iv));
#endif
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);	/* Associate this new scalar with tag "tagnum" */

    TRACEME(("ok (retrieve_netint at 0x%" UVxf ")", PTR2UV(sv)));

    return sv;
}

/*
 * retrieve_double
 *
 * Retrieve defined double.
 * Layout is SX_DOUBLE <data>, whith SX_DOUBLE already read.
 */
static SV *retrieve_double(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;
    HV *stash;
    NV nv;

    TRACEME(("retrieve_double (#%d)", (int)cxt->tagnum));

    READ(&nv, sizeof(nv));
    sv = newSVnv(nv);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);	/* Associate this new scalar with tag "tagnum" */

    TRACEME(("double %" NVff, nv));
    TRACEME(("ok (retrieve_double at 0x%" UVxf ")", PTR2UV(sv)));

    return sv;
}

/*
 * retrieve_byte
 *
 * Retrieve defined byte (small integer within the [-128, +127] range).
 * Layout is SX_BYTE <data>, whith SX_BYTE already read.
 */
static SV *retrieve_byte(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;
    HV *stash;
    int siv;
#ifdef _MSC_VER
    /* MSVC 2017 doesn't handle the AIX workaround well */
    int tmp;
#else
    signed char tmp;	/* Workaround for AIX cc bug --H.Merijn Brand */
#endif

    TRACEME(("retrieve_byte (#%d)", (int)cxt->tagnum));

    GETMARK(siv);
    TRACEME(("small integer read as %d", (unsigned char) siv));
    tmp = (unsigned char) siv - 128;
    sv = newSViv(tmp);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);	/* Associate this new scalar with tag "tagnum" */

    TRACEME(("byte %d", tmp));
    TRACEME(("ok (retrieve_byte at 0x%" UVxf ")", PTR2UV(sv)));

    return sv;
}

/*
 * retrieve_undef
 *
 * Return the undefined value.
 */
static SV *retrieve_undef(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv;
    HV *stash;

    TRACEME(("retrieve_undef"));

    sv = newSV(0);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);

    return sv;
}

/*
 * retrieve_sv_undef
 *
 * Return the immortal undefined value.
 */
static SV *retrieve_sv_undef(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv = &PL_sv_undef;
    HV *stash;

    TRACEME(("retrieve_sv_undef"));

    /* Special case PL_sv_undef, as av_fetch uses it internally to mark
       deleted elements, and will return NULL (fetch failed) whenever it
       is fetched.  */
    if (cxt->where_is_undef == UNSET_NTAG_T) {
        cxt->where_is_undef = cxt->tagnum;
    }
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 1);
    return sv;
}

/*
 * retrieve_sv_yes
 *
 * Return the immortal yes value.
 */
static SV *retrieve_sv_yes(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv = &PL_sv_yes;
    HV *stash;

    TRACEME(("retrieve_sv_yes"));

    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 1);
    return sv;
}

/*
 * retrieve_sv_no
 *
 * Return the immortal no value.
 */
static SV *retrieve_sv_no(pTHX_ stcxt_t *cxt, const char *cname)
{
    SV *sv = &PL_sv_no;
    HV *stash;

    TRACEME(("retrieve_sv_no"));

    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 1);
    return sv;
}

/*
 * retrieve_svundef_elem
 *
 * Return &PL_sv_placeholder, representing &PL_sv_undef in an array.  This
 * is a bit of a hack, but we already use SX_SV_UNDEF to mean a nonexistent
 * element, for historical reasons.
 */
static SV *retrieve_svundef_elem(pTHX_ stcxt_t *cxt, const char *cname)
{
    TRACEME(("retrieve_svundef_elem"));

    /* SEEN reads the contents of its SV argument, which we are not
       supposed to do with &PL_sv_placeholder. */
    SEEN_NN(&PL_sv_undef, cname, 1);

    return &PL_sv_placeholder;
}

/*
 * retrieve_array
 *
 * Retrieve a whole array.
 * Layout is SX_ARRAY <size> followed by each item, in increasing index order.
 * Each item is stored as <object>.
 *
 * When we come here, SX_ARRAY has been read already.
 */
static SV *retrieve_array(pTHX_ stcxt_t *cxt, const char *cname)
{
    I32 len, i;
    AV *av;
    SV *sv;
    HV *stash;
    bool seen_null = FALSE;

    TRACEME(("retrieve_array (#%d)", (int)cxt->tagnum));

    /*
     * Read length, and allocate array, then pre-extend it.
     */

    RLEN(len);
    TRACEME(("size = %d", (int)len));
    av = newAV();
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(av, stash, 0); /* Will return if array not allocated nicely */
    if (len)
        av_extend(av, len);
    else
        return (SV *) av;	/* No data follow if array is empty */

    /*
     * Now get each item in turn...
     */

    for (i = 0; i < len; i++) {
        TRACEME(("(#%d) item", (int)i));
        sv = retrieve(aTHX_ cxt, 0);	/* Retrieve item */
        if (!sv)
            return (SV *) 0;
        if (sv == &PL_sv_undef) {
            seen_null = TRUE;
            continue;
        }
        if (sv == &PL_sv_placeholder)
            sv = &PL_sv_undef;
        if (av_store(av, i, sv) == 0)
            return (SV *) 0;
    }
    if (seen_null) av_fill(av, len-1);

    TRACEME(("ok (retrieve_array at 0x%" UVxf ")", PTR2UV(av)));

    return (SV *) av;
}

#ifdef HAS_U64

/* internal method with len already read */

static SV *get_larray(pTHX_ stcxt_t *cxt, UV len, const char *cname)
{
    UV i;
    AV *av;
    SV *sv;
    HV *stash;
    bool seen_null = FALSE;

    TRACEME(("get_larray (#%d) %lu", (int)cxt->tagnum, (unsigned long)len));

    /*
     * allocate array, then pre-extend it.
     */

    av = newAV();
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(av, stash, 0); /* Will return if array not allocated nicely */
    assert(len);
    av_extend(av, len);

    /*
     * Now get each item in turn...
     */

    for (i = 0; i < len; i++) {
        TRACEME(("(#%d) item", (int)i));
        sv = retrieve(aTHX_ cxt, 0);		/* Retrieve item */
        if (!sv)
            return (SV *) 0;
        if (sv == &PL_sv_undef) {
            seen_null = TRUE;
            continue;
        }
        if (sv == &PL_sv_placeholder)
            sv = &PL_sv_undef;
        if (av_store(av, i, sv) == 0)
            return (SV *) 0;
    }
    if (seen_null) av_fill(av, len-1);

    TRACEME(("ok (get_larray at 0x%" UVxf ")", PTR2UV(av)));

    return (SV *) av;
}

/*
 * get_lhash
 *
 * Retrieve a overlong hash table.
 * <len> is already read. What follows is each key/value pair, in random order.
 * Keys are stored as <length> <data>, the <data> section being omitted
 * if length is 0.
 * Values are stored as <object>.
 *
 */
static SV *get_lhash(pTHX_ stcxt_t *cxt, UV len, int hash_flags, const char *cname)
{
    UV size;
    UV i;
    HV *hv;
    SV *sv;
    HV *stash;

    TRACEME(("get_lhash (#%d)", (int)cxt->tagnum));

#ifdef HAS_RESTRICTED_HASHES
    PERL_UNUSED_ARG(hash_flags);
#else        
    if (hash_flags & SHV_RESTRICTED) {
        if (cxt->derestrict < 0)
            cxt->derestrict = (SvTRUE
                (get_sv("Storable::downgrade_restricted", GV_ADD))
                               ? 1 : 0);
        if (cxt->derestrict == 0)
            RESTRICTED_HASH_CROAK();
    }
#endif

    TRACEME(("size = %lu", (unsigned long)len));
    hv = newHV();
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(hv, stash, 0);	/* Will return if table not allocated properly */
    if (len == 0)
        return (SV *) hv;	/* No data follow if table empty */
    TRACEME(("split %lu", (unsigned long)len+1));
    hv_ksplit(hv, len+1);	/* pre-extend hash to save multiple splits */

    /*
     * Now get each key/value pair in turn...
     */

    for (i = 0; i < len; i++) {
        /*
         * Get value first.
         */

        TRACEME(("(#%d) value", (int)i));
        sv = retrieve(aTHX_ cxt, 0);
        if (!sv)
            return (SV *) 0;

        /*
         * Get key.
         * Since we're reading into kbuf, we must ensure we're not
         * recursing between the read and the hv_store() where it's used.
         * Hence the key comes after the value.
         */

        RLEN(size);		/* Get key size */
        KBUFCHK((STRLEN)size);	/* Grow hash key read pool if needed */
        if (size)
            READ(kbuf, size);
        kbuf[size] = '\0';	/* Mark string end, just in case */
        TRACEME(("(#%d) key '%s'", (int)i, kbuf));

        /*
         * Enter key/value pair into hash table.
         */

        if (hv_store(hv, kbuf, (U32) size, sv, 0) == 0)
            return (SV *) 0;
    }

    TRACEME(("ok (get_lhash at 0x%" UVxf ")", PTR2UV(hv)));
    return (SV *) hv;
}
#endif

/*
 * retrieve_hash
 *
 * Retrieve a whole hash table.
 * Layout is SX_HASH <size> followed by each key/value pair, in random order.
 * Keys are stored as <length> <data>, the <data> section being omitted
 * if length is 0.
 * Values are stored as <object>.
 *
 * When we come here, SX_HASH has been read already.
 */
static SV *retrieve_hash(pTHX_ stcxt_t *cxt, const char *cname)
{
    I32 len;
    I32 size;
    I32 i;
    HV *hv;
    SV *sv;
    HV *stash;

    TRACEME(("retrieve_hash (#%d)", (int)cxt->tagnum));

    /*
     * Read length, allocate table.
     */

    RLEN(len);
    TRACEME(("size = %d", (int)len));
    hv = newHV();
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(hv, stash, 0);	/* Will return if table not allocated properly */
    if (len == 0)
        return (SV *) hv;	/* No data follow if table empty */
    TRACEME(("split %d", (int)len+1));
    hv_ksplit(hv, len+1);	/* pre-extend hash to save multiple splits */

    /*
     * Now get each key/value pair in turn...
     */

    for (i = 0; i < len; i++) {
        /*
         * Get value first.
         */

        TRACEME(("(#%d) value", (int)i));
        sv = retrieve(aTHX_ cxt, 0);
        if (!sv)
            return (SV *) 0;

        /*
         * Get key.
         * Since we're reading into kbuf, we must ensure we're not
         * recursing between the read and the hv_store() where it's used.
         * Hence the key comes after the value.
         */

        RLEN(size);		/* Get key size */
        KBUFCHK((STRLEN)size);	/* Grow hash key read pool if needed */
        if (size)
            READ(kbuf, size);
        kbuf[size] = '\0';	/* Mark string end, just in case */
        TRACEME(("(#%d) key '%s'", (int)i, kbuf));

        /*
         * Enter key/value pair into hash table.
         */

        if (hv_store(hv, kbuf, (U32) size, sv, 0) == 0)
            return (SV *) 0;
    }

    TRACEME(("ok (retrieve_hash at 0x%" UVxf ")", PTR2UV(hv)));

    return (SV *) hv;
}

/*
 * retrieve_hash
 *
 * Retrieve a whole hash table.
 * Layout is SX_HASH <size> followed by each key/value pair, in random order.
 * Keys are stored as <length> <data>, the <data> section being omitted
 * if length is 0.
 * Values are stored as <object>.
 *
 * When we come here, SX_HASH has been read already.
 */
static SV *retrieve_flag_hash(pTHX_ stcxt_t *cxt, const char *cname)
{
    dVAR;
    I32 len;
    I32 size;
    I32 i;
    HV *hv;
    SV *sv;
    HV *stash;
    int hash_flags;

    GETMARK(hash_flags);
    TRACEME(("retrieve_flag_hash (#%d)", (int)cxt->tagnum));
    /*
     * Read length, allocate table.
     */

#ifndef HAS_RESTRICTED_HASHES
    if (hash_flags & SHV_RESTRICTED) {
        if (cxt->derestrict < 0)
            cxt->derestrict = (SvTRUE
                (get_sv("Storable::downgrade_restricted", GV_ADD))
                               ? 1 : 0);
        if (cxt->derestrict == 0)
            RESTRICTED_HASH_CROAK();
    }
#endif

    RLEN(len);
    TRACEME(("size = %d, flags = %d", (int)len, hash_flags));
    hv = newHV();
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(hv, stash, 0);	/* Will return if table not allocated properly */
    if (len == 0)
        return (SV *) hv;	/* No data follow if table empty */
    TRACEME(("split %d", (int)len+1));
    hv_ksplit(hv, len+1);	/* pre-extend hash to save multiple splits */

    /*
     * Now get each key/value pair in turn...
     */

    for (i = 0; i < len; i++) {
        int flags;
        int store_flags = 0;
        /*
         * Get value first.
         */

        TRACEME(("(#%d) value", (int)i));
        sv = retrieve(aTHX_ cxt, 0);
        if (!sv)
            return (SV *) 0;

        GETMARK(flags);
#ifdef HAS_RESTRICTED_HASHES
        if ((hash_flags & SHV_RESTRICTED) && (flags & SHV_K_LOCKED))
            SvREADONLY_on(sv);
#endif

        if (flags & SHV_K_ISSV) {
            /* XXX you can't set a placeholder with an SV key.
               Then again, you can't get an SV key.
               Without messing around beyond what the API is supposed to do.
            */
            SV *keysv;
            TRACEME(("(#%d) keysv, flags=%d", (int)i, flags));
            keysv = retrieve(aTHX_ cxt, 0);
            if (!keysv)
                return (SV *) 0;

            if (!hv_store_ent(hv, keysv, sv, 0))
                return (SV *) 0;
        } else {
            /*
             * Get key.
             * Since we're reading into kbuf, we must ensure we're not
             * recursing between the read and the hv_store() where it's used.
             * Hence the key comes after the value.
             */

            if (flags & SHV_K_PLACEHOLDER) {
                SvREFCNT_dec (sv);
                sv = &PL_sv_placeholder;
                store_flags |= HVhek_PLACEHOLD;
            }
            if (flags & SHV_K_UTF8) {
#ifdef HAS_UTF8_HASHES
                store_flags |= HVhek_UTF8;
#else
                if (cxt->use_bytes < 0)
                    cxt->use_bytes
                        = (SvTRUE(get_sv("Storable::drop_utf8", GV_ADD))
                           ? 1 : 0);
                if (cxt->use_bytes == 0)
                    UTF8_CROAK();
#endif
            }
#ifdef HAS_UTF8_HASHES
            if (flags & SHV_K_WASUTF8)
                store_flags |= HVhek_WASUTF8;
#endif

            RLEN(size);		/* Get key size */
            KBUFCHK((STRLEN)size);/* Grow hash key read pool if needed */
            if (size)
                READ(kbuf, size);
            kbuf[size] = '\0';	/* Mark string end, just in case */
            TRACEME(("(#%d) key '%s' flags %X store_flags %X", (int)i, kbuf,
                     flags, store_flags));

            /*
             * Enter key/value pair into hash table.
             */

#ifdef HAS_RESTRICTED_HASHES
            if (hv_store_flags(hv, kbuf, size, sv, 0, store_flags) == 0)
                return (SV *) 0;
#else
            if (!(store_flags & HVhek_PLACEHOLD))
                if (hv_store(hv, kbuf, size, sv, 0) == 0)
                    return (SV *) 0;
#endif
        }
    }
#ifdef HAS_RESTRICTED_HASHES
    if (hash_flags & SHV_RESTRICTED)
        SvREADONLY_on(hv);
#endif

    TRACEME(("ok (retrieve_hash at 0x%" UVxf ")", PTR2UV(hv)));

    return (SV *) hv;
}

/*
 * retrieve_code
 *
 * Return a code reference.
 */
static SV *retrieve_code(pTHX_ stcxt_t *cxt, const char *cname)
{
    dSP;
    I32 type, count;
    IV tagnum;
    SV *cv;
    SV *sv, *text, *sub, *errsv;
    HV *stash;

    TRACEME(("retrieve_code (#%d)", (int)cxt->tagnum));

    /*
     *  Insert dummy SV in the aseen array so that we don't screw
     *  up the tag numbers.  We would just make the internal
     *  scalar an untagged item in the stream, but
     *  retrieve_scalar() calls SEEN().  So we just increase the
     *  tag number.
     */
    tagnum = cxt->tagnum;
    sv = newSViv(0);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);

    /*
     * Retrieve the source of the code reference
     * as a small or large scalar
     */

    GETMARK(type);
    switch (type) {
    case SX_SCALAR:
        text = retrieve_scalar(aTHX_ cxt, cname);
        break;
    case SX_LSCALAR:
        text = retrieve_lscalar(aTHX_ cxt, cname);
        break;
    case SX_UTF8STR:
        text = retrieve_utf8str(aTHX_ cxt, cname);
        break;
    case SX_LUTF8STR:
        text = retrieve_lutf8str(aTHX_ cxt, cname);
        break;
    default:
        CROAK(("Unexpected type %d in retrieve_code\n", (int)type));
    }

    if (!text) {
        CROAK(("Unable to retrieve code\n"));
    }

    /*
     * prepend "sub " to the source
     */

    sub = newSVpvs("sub ");
    if (SvUTF8(text))
        SvUTF8_on(sub);
    sv_catpv(sub, SvPV_nolen(text)); /* XXX no sv_catsv! */
    SvREFCNT_dec(text);

    /*
     * evaluate the source to a code reference and use the CV value
     */

    if (cxt->eval == NULL) {
        cxt->eval = get_sv("Storable::Eval", GV_ADD);
        SvREFCNT_inc(cxt->eval);
    }
    if (!SvTRUE(cxt->eval)) {
        if (cxt->forgive_me == 0 ||
            (cxt->forgive_me < 0 &&
             !(cxt->forgive_me = SvTRUE
               (get_sv("Storable::forgive_me", GV_ADD)) ? 1 : 0))
            ) {
            CROAK(("Can't eval, please set $Storable::Eval to a true value"));
        } else {
            sv = newSVsv(sub);
            /* fix up the dummy entry... */
            av_store(cxt->aseen, tagnum, SvREFCNT_inc(sv));
            return sv;
        }
    }

    ENTER;
    SAVETMPS;

    errsv = get_sv("@", GV_ADD);
    SvPVCLEAR(errsv);	/* clear $@ */
    if (SvROK(cxt->eval) && SvTYPE(SvRV(cxt->eval)) == SVt_PVCV) {
        PUSHMARK(sp);
        XPUSHs(sv_2mortal(newSVsv(sub)));
        PUTBACK;
        count = call_sv(cxt->eval, G_SCALAR);
        if (count != 1)
            CROAK(("Unexpected return value from $Storable::Eval callback\n"));
    } else {
        eval_sv(sub, G_SCALAR);
    }
    SPAGAIN;
    cv = POPs;
    PUTBACK;

    if (SvTRUE(errsv)) {
        CROAK(("code %s caused an error: %s",
               SvPV_nolen(sub), SvPV_nolen(errsv)));
    }

    if (cv && SvROK(cv) && SvTYPE(SvRV(cv)) == SVt_PVCV) {
        sv = SvRV(cv);
    } else {
        CROAK(("code %s did not evaluate to a subroutine reference\n",
               SvPV_nolen(sub)));
    }

    SvREFCNT_inc(sv); /* XXX seems to be necessary */
    SvREFCNT_dec(sub);

    FREETMPS;
    LEAVE;
    /* fix up the dummy entry... */
    av_store(cxt->aseen, tagnum, SvREFCNT_inc(sv));

    return sv;
}

static SV *retrieve_regexp(pTHX_ stcxt_t *cxt, const char *cname) {
#if PERL_VERSION_GE(5,8,0)
    int op_flags;
    U32 re_len;
    STRLEN flags_len;
    SV *re;
    SV *flags;
    SV *re_ref;
    SV *sv;
    dSP;
    I32 count;
    HV *stash;

    ENTER;
    SAVETMPS;

    GETMARK(op_flags);
    if (op_flags & SHR_U32_RE_LEN) {
        RLEN(re_len);
    }
    else
        GETMARK(re_len);

    re = sv_2mortal(NEWSV(10002, re_len ? re_len : 1));
    READ(SvPVX(re), re_len);
    SvCUR_set(re, re_len);
    *SvEND(re) = '\0';
    SvPOK_only(re);

    GETMARK(flags_len);
    flags = sv_2mortal(NEWSV(10002, flags_len ? flags_len : 1));
    READ(SvPVX(flags), flags_len);
    SvCUR_set(flags, flags_len);
    *SvEND(flags) = '\0';
    SvPOK_only(flags);

    PUSHMARK(SP);

    XPUSHs(re);
    XPUSHs(flags);

    PUTBACK;

    count = call_pv("Storable::_make_re", G_SCALAR);

    SPAGAIN;

    if (count != 1)
        CROAK(("Bad count %d calling _make_re", (int)count));

    re_ref = POPs;

    PUTBACK;

    if (!SvROK(re_ref))
      CROAK(("_make_re didn't return a reference"));

    sv = SvRV(re_ref);
    SvREFCNT_inc(sv);
    stash = cname ? gv_stashpv(cname, GV_ADD) : 0;
    SEEN_NN(sv, stash, 0);
    
    FREETMPS;
    LEAVE;

    return sv;
#else
    CROAK(("retrieve_regexp does not work with 5.6 or earlier"));
#endif
}

/*
 * old_retrieve_array
 *
 * Retrieve a whole array in pre-0.6 binary format.
 *
 * Layout is SX_ARRAY <size> followed by each item, in increasing index order.
 * Each item is stored as SX_ITEM <object> or SX_IT_UNDEF for "holes".
 *
 * When we come here, SX_ARRAY has been read already.
 */
static SV *old_retrieve_array(pTHX_ stcxt_t *cxt, const char *cname)
{
    I32 len;
    I32 i;
    AV *av;
    SV *sv;
    int c;

    PERL_UNUSED_ARG(cname);
    TRACEME(("old_retrieve_array (#%d)", (int)cxt->tagnum));

    /*
     * Read length, and allocate array, then pre-extend it.
     */

    RLEN(len);
    TRACEME(("size = %d", (int)len));
    av = newAV();
    SEEN0_NN(av, 0);	/* Will return if array not allocated nicely */
    if (len)
        av_extend(av, len);
    else
        return (SV *) av;	/* No data follow if array is empty */

    /*
     * Now get each item in turn...
     */

    for (i = 0; i < len; i++) {
        GETMARK(c);
        if (c == SX_IT_UNDEF) {
            TRACEME(("(#%d) undef item", (int)i));
            continue;		/* av_extend() already filled us with undef */
        }
        if (c != SX_ITEM)
            (void) retrieve_other(aTHX_ cxt, 0);/* Will croak out */
        TRACEME(("(#%d) item", (int)i));
        sv = retrieve(aTHX_ cxt, 0);		/* Retrieve item */
        if (!sv)
            return (SV *) 0;
        if (av_store(av, i, sv) == 0)
            return (SV *) 0;
    }

    TRACEME(("ok (old_retrieve_array at 0x%" UVxf ")", PTR2UV(av)));

    return (SV *) av;
}

/*
 * old_retrieve_hash
 *
 * Retrieve a whole hash table in pre-0.6 binary format.
 *
 * Layout is SX_HASH <size> followed by each key/value pair, in random order.
 * Keys are stored as SX_KEY <length> <data>, the <data> section being omitted
 * if length is 0.
 * Values are stored as SX_VALUE <object> or SX_VL_UNDEF for "holes".
 *
 * When we come here, SX_HASH has been read already.
 */
static SV *old_retrieve_hash(pTHX_ stcxt_t *cxt, const char *cname)
{
    I32 len;
    I32 size;
    I32 i;
    HV *hv;
    SV *sv = (SV *) 0;
    int c;
    SV *sv_h_undef = (SV *) 0;	/* hv_store() bug */

    PERL_UNUSED_ARG(cname);
    TRACEME(("old_retrieve_hash (#%d)", (int)cxt->tagnum));

    /*
     * Read length, allocate table.
     */

    RLEN(len);
    TRACEME(("size = %d", (int)len));
    hv = newHV();
    SEEN0_NN(hv, 0);		/* Will return if table not allocated properly */
    if (len == 0)
        return (SV *) hv;	/* No data follow if table empty */
    TRACEME(("split %d", (int)len+1));
    hv_ksplit(hv, len+1);	/* pre-extend hash to save multiple splits */

    /*
     * Now get each key/value pair in turn...
     */

    for (i = 0; i < len; i++) {
        /*
         * Get value first.
         */

        GETMARK(c);
        if (c == SX_VL_UNDEF) {
            TRACEME(("(#%d) undef value", (int)i));
            /*
             * Due to a bug in hv_store(), it's not possible to pass
             * &PL_sv_undef to hv_store() as a value, otherwise the
             * associated key will not be creatable any more. -- RAM, 14/01/97
             */
            if (!sv_h_undef)
                sv_h_undef = newSVsv(&PL_sv_undef);
            sv = SvREFCNT_inc(sv_h_undef);
        } else if (c == SX_VALUE) {
            TRACEME(("(#%d) value", (int)i));
            sv = retrieve(aTHX_ cxt, 0);
            if (!sv)
                return (SV *) 0;
        } else
            (void) retrieve_other(aTHX_ cxt, 0); /* Will croak out */

        /*
         * Get key.
         * Since we're reading into kbuf, we must ensure we're not
         * recursing between the read and the hv_store() where it's used.
         * Hence the key comes after the value.
         */

        GETMARK(c);
        if (c != SX_KEY)
            (void) retrieve_other(aTHX_ cxt, 0); /* Will croak out */
        RLEN(size);				/* Get key size */
        KBUFCHK((STRLEN)size);			/* Grow hash key read pool if needed */
        if (size)
            READ(kbuf, size);
        kbuf[size] = '\0';			/* Mark string end, just in case */
        TRACEME(("(#%d) key '%s'", (int)i, kbuf));

        /*
         * Enter key/value pair into hash table.
         */

        if (hv_store(hv, kbuf, (U32) size, sv, 0) == 0)
            return (SV *) 0;
    }

    TRACEME(("ok (retrieve_hash at 0x%" UVxf ")", PTR2UV(hv)));

    return (SV *) hv;
}

/***
 *** Retrieval engine.
 ***/

/*
 * magic_check
 *
 * Make sure the stored data we're trying to retrieve has been produced
 * on an ILP compatible system with the same byteorder. It croaks out in
 * case an error is detected. [ILP = integer-long-pointer sizes]
 * Returns null if error is detected, &PL_sv_undef otherwise.
 *
 * Note that there's no byte ordering info emitted when network order was
 * used at store time.
 */
static SV *magic_check(pTHX_ stcxt_t *cxt)
{
    /* The worst case for a malicious header would be old magic (which is
       longer), major, minor, byteorder length byte of 255, 255 bytes of
       garbage, sizeof int, long, pointer, NV.
       So the worse of that we can read is 255 bytes of garbage plus 4.
       Err, I am assuming 8 bit bytes here. Please file a bug report if you're
       compiling perl on a system with chars that are larger than 8 bits.
       (Even Crays aren't *that* perverse).
    */
    unsigned char buf[4 + 255];
    unsigned char *current;
    int c;
    int length;
    int use_network_order;
    int use_NV_size;
    int old_magic = 0;
    int version_major;
    int version_minor = 0;

    TRACEME(("magic_check"));

    /*
     * The "magic number" is only for files, not when freezing in memory.
     */

    if (cxt->fio) {
        /* This includes the '\0' at the end.  I want to read the extra byte,
           which is usually going to be the major version number.  */
        STRLEN len = sizeof(magicstr);
        STRLEN old_len;

        READ(buf, (SSize_t)(len));	/* Not null-terminated */

        /* Point at the byte after the byte we read.  */
        current = buf + --len;	/* Do the -- outside of macros.  */

        if (memNE(buf, magicstr, len)) {
            /*
             * Try to read more bytes to check for the old magic number, which
             * was longer.
             */

            TRACEME(("trying for old magic number"));

            old_len = sizeof(old_magicstr) - 1;
            READ(current + 1, (SSize_t)(old_len - len));

            if (memNE(buf, old_magicstr, old_len))
                CROAK(("File is not a perl storable"));
            old_magic++;
            current = buf + old_len;
        }
        use_network_order = *current;
    } else {
        GETMARK(use_network_order);
    }

    /*
     * Starting with 0.6, the "use_network_order" byte flag is also used to
     * indicate the version number of the binary, and therefore governs the
     * setting of sv_retrieve_vtbl. See magic_write().
     */
    if (old_magic && use_network_order > 1) {
        /*  0.1 dump - use_network_order is really byte order length */
        version_major = -1;
    }
    else {
        version_major = use_network_order >> 1;
    }
    cxt->retrieve_vtbl = (SV*(**)(pTHX_ stcxt_t *cxt, const char *cname)) (version_major > 0 ? sv_retrieve : sv_old_retrieve);

    TRACEME(("magic_check: netorder = 0x%x", use_network_order));


    /*
     * Starting with 0.7 (binary major 2), a full byte is dedicated to the
     * minor version of the protocol.  See magic_write().
     */

    if (version_major > 1)
        GETMARK(version_minor);

    cxt->ver_major = version_major;
    cxt->ver_minor = version_minor;

    TRACEME(("binary image version is %d.%d", version_major, version_minor));

    /*
     * Inter-operability sanity check: we can't retrieve something stored
     * using a format more recent than ours, because we have no way to
     * know what has changed, and letting retrieval go would mean a probable
     * failure reporting a "corrupted" storable file.
     */

    if (
        version_major > STORABLE_BIN_MAJOR ||
        (version_major == STORABLE_BIN_MAJOR &&
         version_minor > STORABLE_BIN_MINOR)
        ) {
        int croak_now = 1;
        TRACEME(("but I am version is %d.%d", STORABLE_BIN_MAJOR,
                 STORABLE_BIN_MINOR));

        if (version_major == STORABLE_BIN_MAJOR) {
            TRACEME(("cxt->accept_future_minor is %d",
                     cxt->accept_future_minor));
            if (cxt->accept_future_minor < 0)
                cxt->accept_future_minor
                    = (SvTRUE(get_sv("Storable::accept_future_minor",
                                          GV_ADD))
                       ? 1 : 0);
            if (cxt->accept_future_minor == 1)
                croak_now = 0;  /* Don't croak yet.  */
        }
        if (croak_now) {
            CROAK(("Storable binary image v%d.%d more recent than I am (v%d.%d)",
                   version_major, version_minor,
                   STORABLE_BIN_MAJOR, STORABLE_BIN_MINOR));
        }
    }

    /*
     * If they stored using network order, there's no byte ordering
     * information to check.
     */

    if ((cxt->netorder = (use_network_order & 0x1)))	/* Extra () for -Wall */
        return &PL_sv_undef;			/* No byte ordering info */

    /* In C truth is 1, falsehood is 0. Very convenient.  */
    use_NV_size = version_major >= 2 && version_minor >= 2;

    if (version_major >= 0) {
        GETMARK(c);
    }
    else {
        c = use_network_order;
    }
    length = c + 3 + use_NV_size;
    READ(buf, length);	/* Not null-terminated */

    TRACEME(("byte order '%.*s' %d", c, buf, c));

#ifdef USE_56_INTERWORK_KLUDGE
    /* No point in caching this in the context as we only need it once per
       retrieve, and we need to recheck it each read.  */
    if (SvTRUE(get_sv("Storable::interwork_56_64bit", GV_ADD))) {
        if ((c != (sizeof (byteorderstr_56) - 1))
            || memNE(buf, byteorderstr_56, c))
            CROAK(("Byte order is not compatible"));
    } else
#endif
    {
        if ((c != (sizeof (byteorderstr) - 1))
          || memNE(buf, byteorderstr, c))
            CROAK(("Byte order is not compatible"));
    }

    current = buf + c;

    /* sizeof(int) */
    if ((int) *current++ != sizeof(int))
        CROAK(("Integer size is not compatible"));

    /* sizeof(long) */
    if ((int) *current++ != sizeof(long))
        CROAK(("Long integer size is not compatible"));

    /* sizeof(char *) */
    if ((int) *current != sizeof(char *))
        CROAK(("Pointer size is not compatible"));

    if (use_NV_size) {
        /* sizeof(NV) */
        if ((int) *++current != sizeof(NV))
            CROAK(("Double size is not compatible"));
    }

    return &PL_sv_undef;	/* OK */
}

/*
 * retrieve
 *
 * Recursively retrieve objects from the specified file and return their
 * root SV (which may be an AV or an HV for what we care).
 * Returns null if there is a problem.
 */
static SV *retrieve(pTHX_ stcxt_t *cxt, const char *cname)
{
    int type;
    SV **svh;
    SV *sv;

    TRACEME(("retrieve"));

    /*
     * Grab address tag which identifies the object if we are retrieving
     * an older format. Since the new binary format counts objects and no
     * longer explicitly tags them, we must keep track of the correspondence
     * ourselves.
     *
     * The following section will disappear one day when the old format is
     * no longer supported, hence the final "goto" in the "if" block.
     */

    if (cxt->hseen) {			/* Retrieving old binary */
        stag_t tag;
        if (cxt->netorder) {
            I32 nettag;
            READ(&nettag, sizeof(I32));	/* Ordered sequence of I32 */
            tag = (stag_t) nettag;
        } else
            READ(&tag, sizeof(stag_t));	/* Original address of the SV */

        GETMARK(type);
        if (type == SX_OBJECT) {
            I32 tagn;
            svh = hv_fetch(cxt->hseen, (char *) &tag, sizeof(tag), FALSE);
            if (!svh)
                CROAK(("Old tag 0x%" UVxf " should have been mapped already",
                       (UV) tag));
            tagn = SvIV(*svh);	/* Mapped tag number computed earlier below */

            /*
             * The following code is common with the SX_OBJECT case below.
             */

            svh = av_fetch(cxt->aseen, tagn, FALSE);
            if (!svh)
                CROAK(("Object #%" IVdf " should have been retrieved already",
                       (IV) tagn));
            sv = *svh;
            TRACEME(("has retrieved #%d at 0x%" UVxf, (int)tagn, PTR2UV(sv)));
            SvREFCNT_inc(sv);	/* One more reference to this same sv */
            return sv;		/* The SV pointer where object was retrieved */
        }

        /*
         * Map new object, but don't increase tagnum. This will be done
         * by each of the retrieve_* functions when they call SEEN().
         *
         * The mapping associates the "tag" initially present with a unique
         * tag number. See test for SX_OBJECT above to see how this is perused.
         */

        if (!hv_store(cxt->hseen, (char *) &tag, sizeof(tag),
                      newSViv(cxt->tagnum), 0))
            return (SV *) 0;

        goto first_time;
    }

    /*
     * Regular post-0.6 binary format.
     */

    GETMARK(type);

    TRACEME(("retrieve type = %d", type));

    /*
     * Are we dealing with an object we should have already retrieved?
     */

    if (type == SX_OBJECT) {
        I32 tag;
        READ_I32(tag);
        tag = ntohl(tag);
#ifndef HAS_U64
        /* A 32-bit system can't have over 2**31 objects anyway */
        if (tag < 0)
            CROAK(("Object #%" IVdf " out of range", (IV)tag));
#endif
        /* Older versions of Storable on with 64-bit support on 64-bit
           systems can produce values above the 2G boundary (or wrapped above
           the 4G boundary, which we can't do much about), treat those as
           unsigned.
           This same commit stores tag ids over the 2G boundary as long tags
           since older Storables will mis-handle them as short tags.
         */
        svh = av_fetch(cxt->aseen, (U32)tag, FALSE);
        if (!svh)
            CROAK(("Object #%" IVdf " should have been retrieved already",
                   (IV) tag));
        sv = *svh;
        TRACEME(("had retrieved #%d at 0x%" UVxf, (int)tag, PTR2UV(sv)));
        SvREFCNT_inc(sv);	/* One more reference to this same sv */
        return sv;		/* The SV pointer where object was retrieved */
    } else if (type >= SX_LAST && cxt->ver_minor > STORABLE_BIN_MINOR) {
        if (cxt->accept_future_minor < 0)
            cxt->accept_future_minor
                = (SvTRUE(get_sv("Storable::accept_future_minor",
                                      GV_ADD))
                   ? 1 : 0);
        if (cxt->accept_future_minor == 1) {
            CROAK(("Storable binary image v%d.%d contains data of type %d. "
                   "This Storable is v%d.%d and can only handle data types up to %d",
                   cxt->ver_major, cxt->ver_minor, type,
                   STORABLE_BIN_MAJOR, STORABLE_BIN_MINOR, SX_LAST - 1));
        }
    }

 first_time:	/* Will disappear when support for old format is dropped */

    /*
     * Okay, first time through for this one.
     */

    sv = RETRIEVE(cxt, type)(aTHX_ cxt, cname);
    if (!sv)
        return (SV *) 0;		/* Failed */

    /*
     * Old binary formats (pre-0.7).
     *
     * Final notifications, ended by SX_STORED may now follow.
     * Currently, the only pertinent notification to apply on the
     * freshly retrieved object is either:
     *    SX_CLASS <char-len> <classname> for short classnames.
     *    SX_LG_CLASS <int-len> <classname> for larger one (rare!).
     * Class name is then read into the key buffer pool used by
     * hash table key retrieval.
     */

    if (cxt->ver_major < 2) {
        while ((type = GETCHAR()) != SX_STORED) {
            I32 len;
            HV* stash;
            switch (type) {
            case SX_CLASS:
                GETMARK(len);		/* Length coded on a single char */
                break;
            case SX_LG_CLASS:		/* Length coded on a regular integer */
                RLEN(len);
                break;
            case EOF:
            default:
                return (SV *) 0;	/* Failed */
            }
            KBUFCHK((STRLEN)len);	/* Grow buffer as necessary */
            if (len)
                READ(kbuf, len);
            kbuf[len] = '\0';		/* Mark string end */
            stash = gv_stashpvn(kbuf, len, GV_ADD);
            BLESS(sv, stash);
        }
    }

    TRACEME(("ok (retrieved 0x%" UVxf ", refcnt=%d, %s)", PTR2UV(sv),
             (int)SvREFCNT(sv) - 1, sv_reftype(sv, FALSE)));

    return sv;	/* Ok */
}

/*
 * do_retrieve
 *
 * Retrieve data held in file and return the root object.
 * Common routine for pretrieve and mretrieve.
 */
static SV *do_retrieve(
                       pTHX_
                       PerlIO *f,
                       SV *in,
                       int optype,
                       int flags)
{
    dSTCXT;
    SV *sv;
    int is_tainted;		/* Is input source tainted? */
    int pre_06_fmt = 0;		/* True with pre Storable 0.6 formats */

    TRACEMED(("do_retrieve (optype = 0x%x, flags=0x%x)",
	     (unsigned)optype, (unsigned)flags));

    optype |= ST_RETRIEVE;
    cxt->flags = flags;

    /*
     * Sanity assertions for retrieve dispatch tables.
     */

    ASSERT(sizeof(sv_old_retrieve) == sizeof(sv_retrieve),
           ("old and new retrieve dispatch table have same size"));
    ASSERT(sv_old_retrieve[(int)SX_LAST] == retrieve_other,
           ("SX_LAST entry correctly initialized in old dispatch table"));
    ASSERT(sv_retrieve[(int)SX_LAST] == retrieve_other,
           ("SX_LAST entry correctly initialized in new dispatch table"));

    /*
     * Workaround for CROAK leak: if they enter with a "dirty" context,
     * free up memory for them now.
     */

    assert(cxt);
    if (cxt->s_dirty)
        clean_context(aTHX_ cxt);

    /*
     * Now that STORABLE_xxx hooks exist, it is possible that they try to
     * re-enter retrieve() via the hooks.
     */

    if (cxt->entry) {
        cxt = allocate_context(aTHX_ cxt);
        cxt->flags = flags;
    }
    INIT_TRACEME;

    cxt->entry++;

    ASSERT(cxt->entry == 1, ("starting new recursion"));
    ASSERT(!cxt->s_dirty, ("clean context"));

    /*
     * Prepare context.
     *
     * Data is loaded into the memory buffer when f is NULL, unless 'in' is
     * also NULL, in which case we're expecting the data to already lie
     * in the buffer (dclone case).
     */

    KBUFINIT();			 /* Allocate hash key reading pool once */

    if (!f && in) {
#ifdef SvUTF8_on
        if (SvUTF8(in)) {
            STRLEN length;
            const char *orig = SvPV(in, length);
            char *asbytes;
            /* This is quite deliberate. I want the UTF8 routines
               to encounter the '\0' which perl adds at the end
               of all scalars, so that any new string also has
               this.
            */
            STRLEN klen_tmp = length + 1;
            bool is_utf8 = TRUE;

            /* Just casting the &klen to (STRLEN) won't work
               well if STRLEN and I32 are of different widths.
               --jhi */
            asbytes = (char*)bytes_from_utf8((U8*)orig,
                                             &klen_tmp,
                                             &is_utf8);
            if (is_utf8) {
                CROAK(("Frozen string corrupt - contains characters outside 0-255"));
            }
            if (asbytes != orig) {
                /* String has been converted.
                   There is no need to keep any reference to
                   the old string.  */
                in = sv_newmortal();
                /* We donate the SV the malloc()ed string
                   bytes_from_utf8 returned us.  */
                SvUPGRADE(in, SVt_PV);
                SvPOK_on(in);
                SvPV_set(in, asbytes);
                SvLEN_set(in, klen_tmp);
                SvCUR_set(in, klen_tmp - 1);
            }
        }
#endif
        MBUF_SAVE_AND_LOAD(in);
    }

    /*
     * Magic number verifications.
     *
     * This needs to be done before calling init_retrieve_context()
     * since the format indication in the file are necessary to conduct
     * some of the initializations.
     */

    cxt->fio = f;			/* Where I/O are performed */

    if (!magic_check(aTHX_ cxt))
        CROAK(("Magic number checking on storable %s failed",
               cxt->fio ? "file" : "string"));

    TRACEME(("data stored in %s format",
             cxt->netorder ? "net order" : "native"));

    /*
     * Check whether input source is tainted, so that we don't wrongly
     * taint perfectly good values...
     *
     * We assume file input is always tainted.  If both 'f' and 'in' are
     * NULL, then we come from dclone, and tainted is already filled in
     * the context.  That's a kludge, but the whole dclone() thing is
     * already quite a kludge anyway! -- RAM, 15/09/2000.
     */

    is_tainted = f ? 1 : (in ? SvTAINTED(in) : cxt->s_tainted);
    TRACEME(("input source is %s", is_tainted ? "tainted" : "trusted"));
    init_retrieve_context(aTHX_ cxt, optype, is_tainted);

    ASSERT(is_retrieving(aTHX), ("within retrieve operation"));

    sv = retrieve(aTHX_ cxt, 0); /* Recursively retrieve object, get root SV */

    /*
     * Final cleanup.
     */

    if (!f && in)
        MBUF_RESTORE();

    pre_06_fmt = cxt->hseen != NULL;	/* Before we clean context */

    /*
     * The "root" context is never freed.
     */

    clean_retrieve_context(aTHX_ cxt);
    if (cxt->prev)			/* This context was stacked */
        free_context(aTHX_ cxt);	/* It was not the "root" context */

    /*
     * Prepare returned value.
     */

    if (!sv) {
        TRACEMED(("retrieve ERROR"));
        return &PL_sv_undef;		/* Something went wrong, return undef */
    }

    TRACEMED(("retrieve got %s(0x%" UVxf ")",
             sv_reftype(sv, FALSE), PTR2UV(sv)));

    /*
     * Backward compatibility with Storable-0.5@9 (which we know we
     * are retrieving if hseen is non-null): don't create an extra RV
     * for objects since we special-cased it at store time.
     *
     * Build a reference to the SV returned by pretrieve even if it is
     * already one and not a scalar, for consistency reasons.
     */

    if (pre_06_fmt) {			/* Was not handling overloading by then */
        SV *rv;
        TRACEMED(("fixing for old formats -- pre 0.6"));
        if (sv_type(aTHX_ sv) == svis_REF && (rv = SvRV(sv)) && SvOBJECT(rv)) {
            TRACEME(("ended do_retrieve() with an object -- pre 0.6"));
            return sv;
        }
    }

    /*
     * If reference is overloaded, restore behaviour.
     *
     * NB: minor glitch here: normally, overloaded refs are stored specially
     * so that we can croak when behaviour cannot be re-installed, and also
     * avoid testing for overloading magic at each reference retrieval.
     *
     * Unfortunately, the root reference is implicitly stored, so we must
     * check for possible overloading now.  Furthermore, if we don't restore
     * overloading, we cannot croak as if the original ref was, because we
     * have no way to determine whether it was an overloaded ref or not in
     * the first place.
     *
     * It's a pity that overloading magic is attached to the rv, and not to
     * the underlying sv as blessing is.
     */

    if (SvOBJECT(sv)) {
        HV *stash = (HV *) SvSTASH(sv);
        SV *rv = newRV_noinc(sv);
        if (stash && Gv_AMG(stash)) {
            SvAMAGIC_on(rv);
            TRACEMED(("restored overloading on root reference"));
        }
        TRACEMED(("ended do_retrieve() with an object"));
        return rv;
    }

    TRACEMED(("regular do_retrieve() end"));

    return newRV_noinc(sv);
}

/*
 * pretrieve
 *
 * Retrieve data held in file and return the root object, undef on error.
 */
static SV *pretrieve(pTHX_ PerlIO *f, IV flag)
{
    TRACEMED(("pretrieve"));
    return do_retrieve(aTHX_ f, Nullsv, 0, (int)flag);
}

/*
 * mretrieve
 *
 * Retrieve data held in scalar and return the root object, undef on error.
 */
static SV *mretrieve(pTHX_ SV *sv, IV flag)
{
    TRACEMED(("mretrieve"));
    return do_retrieve(aTHX_ (PerlIO*) 0, sv, 0, (int)flag);
}

/***
 *** Deep cloning
 ***/

/*
 * dclone
 *
 * Deep clone: returns a fresh copy of the original referenced SV tree.
 *
 * This is achieved by storing the object in memory and restoring from
 * there. Not that efficient, but it should be faster than doing it from
 * pure perl anyway.
 */
static SV *dclone(pTHX_ SV *sv)
{
    dSTCXT;
    STRLEN size;
    stcxt_t *real_context;
    SV *out;

    TRACEMED(("dclone"));

    /*
     * Workaround for CROAK leak: if they enter with a "dirty" context,
     * free up memory for them now.
     */

    assert(cxt);
    if (cxt->s_dirty)
        clean_context(aTHX_ cxt);

    /*
     * Tied elements seem to need special handling.
     */

    if ((SvTYPE(sv) == SVt_PVLV
#if PERL_VERSION_LT(5,8,0)
         || SvTYPE(sv) == SVt_PVMG
#endif
         ) && (SvFLAGS(sv) & (SVs_GMG|SVs_SMG|SVs_RMG)) ==
        (SVs_GMG|SVs_SMG|SVs_RMG) &&
        mg_find(sv, 'p')) {
        mg_get(sv);
    }

    /*
     * do_store() optimizes for dclone by not freeing its context, should
     * we need to allocate one because we're deep cloning from a hook.
     */

    if (!do_store(aTHX_ (PerlIO*) 0, sv, ST_CLONE, FALSE, (SV**) 0))
        return &PL_sv_undef;				/* Error during store */

    /*
     * Because of the above optimization, we have to refresh the context,
     * since a new one could have been allocated and stacked by do_store().
     */

    { dSTCXT; real_context = cxt; }		/* Sub-block needed for macro */
    cxt = real_context;					/* And we need this temporary... */

    /*
     * Now, 'cxt' may refer to a new context.
     */

    assert(cxt);
    ASSERT(!cxt->s_dirty, ("clean context"));
    ASSERT(!cxt->entry, ("entry will not cause new context allocation"));

    size = MBUF_SIZE();
    TRACEME(("dclone stored %ld bytes", (long)size));
    MBUF_INIT(size);

    /*
     * Since we're passing do_retrieve() both a NULL file and sv, we need
     * to pre-compute the taintedness of the input by setting cxt->tainted
     * to whatever state our own input string was.	-- RAM, 15/09/2000
     *
     * do_retrieve() will free non-root context.
     */

    cxt->s_tainted = SvTAINTED(sv);
    out = do_retrieve(aTHX_ (PerlIO*) 0, Nullsv, ST_CLONE, FLAG_BLESS_OK | FLAG_TIE_OK);

    TRACEMED(("dclone returns 0x%" UVxf, PTR2UV(out)));

    return out;
}

/***
 *** Glue with perl.
 ***/

/*
 * The Perl IO GV object distinguishes between input and output for sockets
 * but not for plain files. To allow Storable to transparently work on
 * plain files and sockets transparently, we have to ask xsubpp to fetch the
 * right object for us. Hence the OutputStream and InputStream declarations.
 *
 * Before perl 5.004_05, those entries in the standard typemap are not
 * defined in perl include files, so we do that here.
 */

#ifndef OutputStream
#define OutputStream	PerlIO *
#define InputStream	PerlIO *
#endif	/* !OutputStream */

static int
storable_free(pTHX_ SV *sv, MAGIC* mg) {
    stcxt_t *cxt = (stcxt_t *)SvPVX(sv);

    PERL_UNUSED_ARG(mg);
#ifdef USE_PTR_TABLE
    if (cxt->pseen)
        ptr_table_free(cxt->pseen);
#endif
    if (kbuf)
        Safefree(kbuf);
    if (!cxt->membuf_ro && mbase)
        Safefree(mbase);
    if (cxt->membuf_ro && (cxt->msaved).arena)
        Safefree((cxt->msaved).arena);
    return 0;
}

MODULE = Storable	PACKAGE = Storable

PROTOTYPES: ENABLE

BOOT:
{
    HV *stash = gv_stashpvn("Storable", 8, GV_ADD);
    newCONSTSUB(stash, "BIN_MAJOR", newSViv(STORABLE_BIN_MAJOR));
    newCONSTSUB(stash, "BIN_MINOR", newSViv(STORABLE_BIN_MINOR));
    newCONSTSUB(stash, "BIN_WRITE_MINOR", newSViv(STORABLE_BIN_WRITE_MINOR));

    newCONSTSUB(stash, "CAN_FLOCK", CAN_FLOCK);

    init_perinterp(aTHX);
    gv_fetchpv("Storable::drop_utf8",   GV_ADDMULTI, SVt_PV);
#ifdef DEBUGME
    /* Only disable the used only once warning if we are in debugging mode.  */
    gv_fetchpv("Storable::DEBUGME",   GV_ADDMULTI, SVt_PV);
#endif
#ifdef USE_56_INTERWORK_KLUDGE
    gv_fetchpv("Storable::interwork_56_64bit",   GV_ADDMULTI, SVt_PV);
#endif
    }

void
init_perinterp()
CODE:
    init_perinterp(aTHX);

# pstore
#
# Store the transitive data closure of given object to disk.
# Returns undef on error, a true value otherwise.

# net_pstore
#
# Same as pstore(), but network order is used for integers and doubles are
# emitted as strings.

SV *
pstore(f,obj)
    OutputStream f
    SV*		obj
ALIAS:
    net_pstore = 1
PPCODE:
    RETVAL = do_store(aTHX_ f, obj, 0, ix, (SV **)0) ? &PL_sv_yes : &PL_sv_undef;
    /* do_store() can reallocate the stack, so need a sequence point to ensure
       that ST(0) knows about it. Hence using two statements.  */
    ST(0) = RETVAL;
    XSRETURN(1);

# mstore
#
# Store the transitive data closure of given object to memory.
# Returns undef on error, a scalar value containing the data otherwise.

# net_mstore
#
# Same as mstore(), but network order is used for integers and doubles are
# emitted as strings.

SV *
mstore(obj)
    SV*	obj
ALIAS:
    net_mstore = 1
CODE:
    RETVAL = &PL_sv_undef;
    if (!do_store(aTHX_ (PerlIO*) 0, obj, 0, ix, &RETVAL))
        RETVAL = &PL_sv_undef;
OUTPUT:
    RETVAL

SV *
pretrieve(f, flag = 6)
    InputStream	f
    IV		flag
CODE:
    RETVAL = pretrieve(aTHX_ f, flag);
OUTPUT:
    RETVAL

SV *
mretrieve(sv, flag = 6)
    SV* sv
    IV	flag
CODE:
    RETVAL = mretrieve(aTHX_ sv, flag);
OUTPUT:
    RETVAL

SV *
dclone(sv)
    SV*	sv
CODE:
    RETVAL = dclone(aTHX_ sv);
OUTPUT:
    RETVAL

void
last_op_in_netorder()
ALIAS:
    is_storing = ST_STORE
    is_retrieving = ST_RETRIEVE
PREINIT:
    bool result;
CODE:
    if (ix) {
        dSTCXT;
        assert(cxt);
        result = cxt->entry && (cxt->optype & ix) ? TRUE : FALSE;
    } else {
        result = cBOOL(last_op_in_netorder(aTHX));
    }
    ST(0) = boolSV(result);


IV
stack_depth()
CODE:
    RETVAL = SvIV(get_sv("Storable::recursion_limit", GV_ADD));
OUTPUT:
    RETVAL

IV
stack_depth_hash()
CODE:
    RETVAL = SvIV(get_sv("Storable::recursion_limit_hash", GV_ADD));
OUTPUT:
    RETVAL
