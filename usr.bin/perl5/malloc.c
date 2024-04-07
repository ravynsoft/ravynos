/*    malloc.c
 *
 */

/*
 * 'The Chamber of Records,' said Gimli.  'I guess that is where we now stand.'
 *
 *     [p.321 of _The Lord of the Rings_, II/v: "The Bridge of Khazad-Dûm"]
 */

/* This file contains Perl's own implementation of the malloc library.
 * It is used if Configure decides that, on your platform, Perl's
 * version is better than the OS's, or if you give Configure the
 * -Dusemymalloc command-line option.
 */

/*
  Here are some notes on configuring Perl's malloc.

  There are two macros which serve as bulk disablers of advanced
  features of this malloc: NO_FANCY_MALLOC, PLAIN_MALLOC (undef by
  default).  Look in the list of default values below to understand
  their exact effect.  Defining NO_FANCY_MALLOC returns malloc.c to the
  state of the malloc in Perl 5.004.  Additionally defining PLAIN_MALLOC
  returns it to the state as of Perl 5.000.

  Note that some of the settings below may be ignored in the code based
  on values of other macros.  The PERL_CORE symbol is only defined when
  perl itself is being compiled (so malloc can make some assumptions
  about perl's facilities being available to it).

  Each config option has a short description, followed by its name,
  default value, and a comment about the default (if applicable).  Some
  options take a precise value, while the others are just boolean.
  The boolean ones are listed first.

    # Read configuration settings from malloc_cfg.h
    HAVE_MALLOC_CFG_H		undef

    # Enable code for an emergency memory pool in $^M.  See perlvar.pod
    # for a description of $^M.
    PERL_EMERGENCY_SBRK		!PLAIN_MALLOC

    # Enable code for printing memory statistics.
    DEBUGGING_MSTATS		!PLAIN_MALLOC

    # Move allocation info for small buckets into separate areas.
    # Memory optimization (especially for small allocations, of the
    # less than 64 bytes).  Since perl usually makes a large number
    # of small allocations, this is usually a win.
    PACK_MALLOC			(!PLAIN_MALLOC && !RCHECK)

    # Add one page to big powers of two when calculating bucket size.
    # This is targeted at big allocations, as are common in image
    # processing.
    TWO_POT_OPTIMIZE		!PLAIN_MALLOC
 
    # Use intermediate bucket sizes between powers-of-two.  This is
    # generally a memory optimization, and a (small) speed pessimization.
    BUCKETS_ROOT2		!NO_FANCY_MALLOC

    # Do not check small deallocations for bad free().  Memory
    # and speed optimization, error reporting pessimization.
    IGNORE_SMALL_BAD_FREE	(!NO_FANCY_MALLOC && !RCHECK)

    # Use table lookup to decide in which bucket a given allocation will go.
    SMALL_BUCKET_VIA_TABLE	!NO_FANCY_MALLOC

    # Use a perl-defined sbrk() instead of the (presumably broken or
    # missing) system-supplied sbrk().
    USE_PERL_SBRK		undef

    # Use system malloc() (or calloc() etc.) to emulate sbrk(). Normally
    # only used with broken sbrk()s.
    PERL_SBRK_VIA_MALLOC	undef

    # Which allocator to use if PERL_SBRK_VIA_MALLOC
    SYSTEM_ALLOC(a) 		malloc(a)

    # Minimal alignment (in bytes, should be a power of 2) of SYSTEM_ALLOC
    SYSTEM_ALLOC_ALIGNMENT	MEM_ALIGNBYTES

    # Disable memory overwrite checking with DEBUGGING.  Memory and speed
    # optimization, error reporting pessimization.
    NO_RCHECK			undef

    # Enable memory overwrite checking with DEBUGGING.  Memory and speed
    # pessimization, error reporting optimization
    RCHECK			(DEBUGGING && !NO_RCHECK)

    # Do not overwrite uninit areas with DEBUGGING.  Speed
    # optimization, error reporting pessimization
    NO_MFILL			undef

    # Overwrite uninit areas with DEBUGGING.  Speed
    # pessimization, error reporting optimization
    MALLOC_FILL			(DEBUGGING && !NO_RCHECK && !NO_MFILL)

    # Do not check overwritten uninit areas with DEBUGGING.  Speed
    # optimization, error reporting pessimization
    NO_FILL_CHECK		undef

    # Check overwritten uninit areas with DEBUGGING.  Speed
    # pessimization, error reporting optimization
    MALLOC_FILL_CHECK		(DEBUGGING && !NO_RCHECK && !NO_FILL_CHECK)

    # Failed allocations bigger than this size croak (if
    # PERL_EMERGENCY_SBRK is enabled) without touching $^M.  See
    # perlvar.pod for a description of $^M.
    BIG_SIZE			 (1<<16)	# 64K

    # Starting from this power of two, add an extra page to the
    # size of the bucket. This enables optimized allocations of sizes
    # close to powers of 2.  Note that the value is indexed at 0.
    FIRST_BIG_POW2 		15		# 32K, 16K is used too often

    # Estimate of minimal memory footprint.  malloc uses this value to
    # request the most reasonable largest blocks of memory from the system.
    FIRST_SBRK 			(48*1024)

    # Round up sbrk()s to multiples of this.
    MIN_SBRK 			2048

    # Round up sbrk()s to multiples of this percent of footprint.
    MIN_SBRK_FRAC 		3

    # Round up sbrk()s to multiples of this multiple of 1/1000 of footprint.
    MIN_SBRK_FRAC1000 		(10 * MIN_SBRK_FRAC)

    # Add this much memory to big powers of two to get the bucket size.
    PERL_PAGESIZE 		4096

    # This many sbrk() discontinuities should be tolerated even
    # from the start without deciding that sbrk() is usually
    # discontinuous.
    SBRK_ALLOW_FAILURES		3

    # This many continuous sbrk()s compensate for one discontinuous one.
    SBRK_FAILURE_PRICE		50

    # Some configurations may ask for 12-byte-or-so allocations which
    # require 8-byte alignment (?!).  In such situation one needs to
    # define this to disable 12-byte bucket (will increase memory footprint)
    STRICT_ALIGNMENT		undef

    # Do not allow configuration of runtime options at runtime
    NO_MALLOC_DYNAMIC_CFG	undef

    # Do not allow configuration of runtime options via $ENV{PERL_MALLOC_OPT}
    NO_PERL_MALLOC_ENV		undef

        [The variable consists of ;-separated parts of the form CODE=VALUE
         with 1-character codes F, M, f, A, P, G, d, a, c for runtime
         configuration of FIRST_SBRK, MIN_SBRK, MIN_SBRK_FRAC1000,
         SBRK_ALLOW_FAILURES, SBRK_FAILURE_PRICE, sbrk_goodness,
         filldead, fillalive, fillcheck.  The last 3 are for DEBUGGING
         build, and allow switching the tests for free()ed memory read,
         uninit memory reads, and free()ed memory write.]

  This implementation assumes that calling PerlIO_printf() does not
  result in any memory allocation calls (used during a panic).

 */


#ifdef HAVE_MALLOC_CFG_H
#  include "malloc_cfg.h"
#endif

#ifndef NO_FANCY_MALLOC
#  ifndef SMALL_BUCKET_VIA_TABLE
#    define SMALL_BUCKET_VIA_TABLE
#  endif 
#  ifndef BUCKETS_ROOT2
#    define BUCKETS_ROOT2
#  endif 
#  ifndef IGNORE_SMALL_BAD_FREE
#    define IGNORE_SMALL_BAD_FREE
#  endif 
#endif 

#ifndef PLAIN_MALLOC			/* Bulk enable features */
#  ifndef PACK_MALLOC
#      define PACK_MALLOC
#  endif 
#  ifndef TWO_POT_OPTIMIZE
#    define TWO_POT_OPTIMIZE
#  endif 
#  ifndef PERL_EMERGENCY_SBRK
#    define PERL_EMERGENCY_SBRK
#  endif 
#  ifndef DEBUGGING_MSTATS
#    define DEBUGGING_MSTATS
#  endif 
#endif

#define MIN_BUC_POW2 (sizeof(void*) > 4 ? 3 : 2) /* Allow for 4-byte arena. */
#define MIN_BUCKET (MIN_BUC_POW2 * BUCKETS_PER_POW2)

#define LOG_OF_MIN_ARENA 11

#if defined(DEBUGGING) && !defined(NO_RCHECK)
#  define RCHECK
#endif
#if defined(DEBUGGING) && !defined(NO_RCHECK) && !defined(NO_MFILL) && !defined(MALLOC_FILL)
#  define MALLOC_FILL
#endif
#if defined(DEBUGGING) && !defined(NO_RCHECK) && !defined(NO_FILL_CHECK) && !defined(MALLOC_FILL_CHECK)
#  define MALLOC_FILL_CHECK
#endif
#if defined(RCHECK) && defined(IGNORE_SMALL_BAD_FREE)
#  undef IGNORE_SMALL_BAD_FREE
#endif 
/*
 * malloc.c (Caltech) 2/21/82
 * Chris Kingsley, kingsley@cit-20.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks that
 * don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are 2^n-4 (or 2^n-12) bytes long.
 * If PACK_MALLOC is defined, small blocks are 2^n bytes long.
 * This is designed for use in a program that uses vast quantities of memory,
 * but bombs when it runs out.
 * 
 * Modifications Copyright Ilya Zakharevich 1996-99.
 * 
 * Still very quick, but much more thrifty.  (Std config is 10% slower
 * than it was, and takes 67% of old heap size for typical usage.)
 *
 * Allocations of small blocks are now table-driven to many different
 * buckets.  Sizes of really big buckets are increased to accommodate
 * common size=power-of-2 blocks.  Running-out-of-memory is made into
 * an exception.  Deeply configurable and thread-safe.
 * 
 */

#include "EXTERN.h"
#define PERL_IN_MALLOC_C
#include "perl.h"
#if defined(MULTIPLICITY)
#    define croak	Perl_croak_nocontext
#    define croak2	Perl_croak_nocontext
#    define warn	Perl_warn_nocontext
#    define warn2	Perl_warn_nocontext
#else
#    define croak2	croak
#    define warn2	warn
#endif
#ifdef USE_ITHREADS
#     define PERL_MAYBE_ALIVE	PL_thr_key
#else
#     define PERL_MAYBE_ALIVE	1
#endif

#ifndef MYMALLOC
#  error "MYMALLOC is not defined"
#endif

#ifndef MUTEX_LOCK
#  define MUTEX_LOCK(l)
#endif 

#ifndef MUTEX_UNLOCK
#  define MUTEX_UNLOCK(l)
#endif 

#ifndef MALLOC_LOCK
#  define MALLOC_LOCK		MUTEX_LOCK(&PL_malloc_mutex)
#endif 

#ifndef MALLOC_UNLOCK
#  define MALLOC_UNLOCK		MUTEX_UNLOCK(&PL_malloc_mutex)
#endif 

#  ifndef fatalcroak				/* make depend */
#    define fatalcroak(mess)	(write(2, (mess), strlen(mess)), exit(2))
#  endif 

#ifdef DEBUGGING
#  undef DEBUG_m
#  define DEBUG_m(a) 							\
    STMT_START {							\
        if (PERL_MAYBE_ALIVE && PERL_GET_THX) {						\
            dTHX;							\
            if (DEBUG_m_TEST) {						\
                PL_debug &= ~DEBUG_m_FLAG;				\
                a;							\
                PL_debug |= DEBUG_m_FLAG;				\
            }								\
        }								\
    } STMT_END
#endif

#ifdef MULTIPLICITY
#  define PERL_IS_ALIVE		aTHX
#else
#  define PERL_IS_ALIVE		TRUE
#endif
    

/*
 * Layout of memory:
 * ~~~~~~~~~~~~~~~~
 * The memory is broken into "blocks" which occupy multiples of 2K (and
 * generally speaking, have size "close" to a power of 2).  The addresses
 * of such *unused* blocks are kept in nextf[i] with big enough i.  (nextf
 * is an array of linked lists.)  (Addresses of used blocks are not known.)
 * 
 * Moreover, since the algorithm may try to "bite" smaller blocks out
 * of unused bigger ones, there are also regions of "irregular" size,
 * managed separately, by a linked list chunk_chain.
 * 
 * The third type of storage is the sbrk()ed-but-not-yet-used space, its
 * end and size are kept in last_sbrk_top and sbrked_remains.
 * 
 * Growing blocks "in place":
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * The address of the block with the greatest address is kept in last_op
 * (if not known, last_op is 0).  If it is known that the memory above
 * last_op is not continuous, or contains a chunk from chunk_chain,
 * last_op is set to 0.
 * 
 * The chunk with address last_op may be grown by expanding into
 * sbrk()ed-but-not-yet-used space, or trying to sbrk() more continuous
 * memory.
 * 
 * Management of last_op:
 * ~~~~~~~~~~~~~~~~~~~~~
 * 
 * free() never changes the boundaries of blocks, so is not relevant.
 * 
 * The only way realloc() may change the boundaries of blocks is if it
 * grows a block "in place".  However, in the case of success such a
 * chunk is automatically last_op, and it remains last_op.  In the case
 * of failure getpages_adjacent() clears last_op.
 * 
 * malloc() may change blocks by calling morecore() only.
 * 
 * morecore() may create new blocks by:
 *   a) biting pieces from chunk_chain (cannot create one above last_op);
 *   b) biting a piece from an unused block (if block was last_op, this
 *      may create a chunk from chain above last_op, thus last_op is
 *      invalidated in such a case).
 *   c) biting of sbrk()ed-but-not-yet-used space.  This creates 
 *      a block which is last_op.
 *   d) Allocating new pages by calling getpages();
 * 
 * getpages() creates a new block.  It marks last_op at the bottom of
 * the chunk of memory it returns.
 * 
 * Active pages footprint:
 * ~~~~~~~~~~~~~~~~~~~~~~
 * Note that we do not need to traverse the lists in nextf[i], just take
 * the first element of this list.  However, we *need* to traverse the
 * list in chunk_chain, but most the time it should be a very short one,
 * so we do not step on a lot of pages we are not going to use.
 * 
 * Flaws:
 * ~~~~~
 * get_from_bigger_buckets(): forget to increment price => Quite
 * aggressive.
 */

/* I don't much care whether these are defined in sys/types.h--LAW */

#define u_char unsigned char
#define u_int unsigned int
/* 
 * I removed the definition of u_bigint which appeared to be u_bigint = UV
 * u_bigint was only used in TWOK_MASKED and TWOK_SHIFT 
 * where I have used PTR2UV.  RMB
 */
#define u_short unsigned short

#if defined(RCHECK) && defined(PACK_MALLOC)
#  undef PACK_MALLOC
#endif 

/*
 * The description below is applicable if PACK_MALLOC is not defined.
 *
 * The overhead on a block is at least 4 bytes.  When free, this space
 * contains a pointer to the next free block, and the bottom two bits must
 * be zero.  When in use, the first byte is set to MAGIC, and the second
 * byte is the size index.  The remaining bytes are for alignment.
 * If range checking is enabled and the size of the block fits
 * in two bytes, then the top two bytes hold the size of the requested block
 * plus the range checking words, and the header word MINUS ONE.
 */
union	overhead {
        union	overhead *ov_next;	/* when free */
#if MEM_ALIGNBYTES > 4
        double	strut;			/* alignment problems */
#  if MEM_ALIGNBYTES > 8
        char	sstrut[MEM_ALIGNBYTES]; /* for the sizing */
#  endif
#endif
        struct {
/*
 * Keep the ovu_index and ovu_magic in this order, having a char
 * field first gives alignment indigestion in some systems, such as
 * MachTen.
 */
                u_char	ovu_index;	/* bucket # */
                u_char	ovu_magic;	/* magic number */
#ifdef RCHECK
            /* Subtract one to fit into u_short for an extra bucket */
                u_short	ovu_size;	/* block size (requested + overhead - 1) */
                u_int	ovu_rmagic;	/* range magic number */
#endif
        } ovu;
#define ov_magic        ovu.ovu_magic
#define ov_index        ovu.ovu_index
#define ov_size         ovu.ovu_size
#define ov_rmagic       ovu.ovu_rmagic
};

#define MAGIC           0xff            /* magic # on accounting info */
#define RMAGIC		0x55555555	/* magic # on range info */
#define RMAGIC_C	0x55		/* magic # on range info */

#ifdef RCHECK
#  define RMAGIC_SZ     sizeof (u_int) /* Overhead at end of bucket */
#  ifdef TWO_POT_OPTIMIZE
#    define MAX_SHORT_BUCKET (12 * BUCKETS_PER_POW2) /* size-1 fits in short */
#  else
#    define MAX_SHORT_BUCKET (13 * BUCKETS_PER_POW2)
#  endif 
#else
#  define RMAGIC_SZ     0
#endif

#if !defined(PACK_MALLOC) && defined(BUCKETS_ROOT2)
#  undef BUCKETS_ROOT2
#endif 

#ifdef BUCKETS_ROOT2
#  define BUCKET_TABLE_SHIFT 2
#  define BUCKET_POW2_SHIFT 1
#  define BUCKETS_PER_POW2 2
#else
#  define BUCKET_TABLE_SHIFT MIN_BUC_POW2
#  define BUCKET_POW2_SHIFT 0
#  define BUCKETS_PER_POW2 1
#endif 

#if !defined(MEM_ALIGNBYTES) || ((MEM_ALIGNBYTES > 4) && !defined(STRICT_ALIGNMENT))
/* Figure out the alignment of void*. */
struct aligner {
  char c;
  void *p;
};
#  define ALIGN_SMALL ((IV)((caddr_t)&(((struct aligner*)0)->p)))
#else
#  define ALIGN_SMALL MEM_ALIGNBYTES
#endif

#define IF_ALIGN_8(yes,no)	((ALIGN_SMALL>4) ? (yes) : (no))

#ifdef BUCKETS_ROOT2
#  define MAX_BUCKET_BY_TABLE 13
static const u_short buck_size[MAX_BUCKET_BY_TABLE + 1] = 
  { 
      0, 0, 0, 0, 4, 4, 8, 12, 16, 24, 32, 48, 64, 80,
  };
#  define BUCKET_SIZE_NO_SURPLUS(i) ((i) % 2 ? buck_size[i] : (1 << ((i) >> BUCKET_POW2_SHIFT)))
#  define BUCKET_SIZE_REAL(i) ((i) <= MAX_BUCKET_BY_TABLE		\
                               ? ((size_t)buck_size[i])			\
                               : ((((size_t)1) << ((i) >> BUCKET_POW2_SHIFT)) \
                                  - MEM_OVERHEAD(i)			\
                                  + POW2_OPTIMIZE_SURPLUS(i)))
#else
#  define BUCKET_SIZE_NO_SURPLUS(i) (((size_t)1) << ((i) >> BUCKET_POW2_SHIFT))
#  define BUCKET_SIZE(i) (BUCKET_SIZE_NO_SURPLUS(i) + POW2_OPTIMIZE_SURPLUS(i))
#  define BUCKET_SIZE_REAL(i) (BUCKET_SIZE(i) - MEM_OVERHEAD(i))
#endif 


#ifdef PACK_MALLOC
/* In this case there are several possible layout of arenas depending
 * on the size.  Arenas are of sizes multiple to 2K, 2K-aligned, and
 * have a size close to a power of 2.
 *
 * Arenas of the size >= 4K keep one chunk only.  Arenas of size 2K
 * may keep one chunk or multiple chunks.  Here are the possible
 * layouts of arenas:
 *
 *	# One chunk only, chunksize 2^k + SOMETHING - ALIGN, k >= 11
 *
 * INDEX MAGIC1 UNUSED CHUNK1
 *
 *	# Multichunk with sanity checking and chunksize 2^k-ALIGN, k>7
 *
 * INDEX MAGIC1 MAGIC2 MAGIC3 UNUSED CHUNK1 CHUNK2 CHUNK3 ...
 *
 *	# Multichunk with sanity checking and size 2^k-ALIGN, k=7
 *
 * INDEX MAGIC1 MAGIC2 MAGIC3 UNUSED CHUNK1 UNUSED CHUNK2 CHUNK3 ...
 *
 *	# Multichunk with sanity checking and size up to 80
 *
 * INDEX UNUSED MAGIC1 UNUSED MAGIC2 UNUSED ... CHUNK1 CHUNK2 CHUNK3 ...
 *
 *	# No sanity check (usually up to 48=byte-long buckets)
 * INDEX UNUSED CHUNK1 CHUNK2 ...
 *
 * Above INDEX and MAGIC are one-byte-long.  Sizes of UNUSED are
 * appropriate to keep algorithms simple and memory aligned.  INDEX
 * encodes the size of the chunk, while MAGICn encodes state (used,
 * free or non-managed-by-us-so-it-indicates-a-bug) of CHUNKn.  MAGIC
 * is used for sanity checking purposes only.  SOMETHING is 0 or 4K
 * (to make size of big CHUNK accommodate allocations for powers of two
 * better).
 *
 * [There is no need to alignment between chunks, since C rules ensure
 *  that structs which need 2^k alignment have sizeof which is
 *  divisible by 2^k.  Thus as far as the last chunk is aligned at the
 *  end of the arena, and 2K-alignment does not contradict things,
 *  everything is going to be OK for sizes of chunks 2^n and 2^n +
 *  2^k.  Say, 80-bit buckets will be 16-bit aligned, and as far as we
 *  put allocations for requests in 65..80 range, all is fine.
 *
 *  Note, however, that standard malloc() puts more strict
 *  requirements than the above C rules.  Moreover, our algorithms of
 *  realloc() may break this idyll, but we suppose that realloc() does
 *  need not change alignment.]
 *
 * Is very important to make calculation of the offset of MAGICm as
 * quick as possible, since it is done on each malloc()/free().  In
 * fact it is so quick that it has quite little effect on the speed of
 * doing malloc()/free().  [By default] We forego such calculations
 * for small chunks, but only to save extra 3% of memory, not because
 * of speed considerations.
 *
 * Here is the algorithm [which is the same for all the allocations
 * schemes above], see OV_MAGIC(block,bucket).  Let OFFSETm be the
 * offset of the CHUNKm from the start of ARENA.  Then offset of
 * MAGICm is (OFFSET1 >> SHIFT) + ADDOFFSET.  Here SHIFT and ADDOFFSET
 * are numbers which depend on the size of the chunks only.
 *
 * Let as check some sanity conditions.  Numbers OFFSETm>>SHIFT are
 * different for all the chunks in the arena if 2^SHIFT is not greater
 * than size of the chunks in the arena.  MAGIC1 will not overwrite
 * INDEX provided ADDOFFSET is >0 if OFFSET1 < 2^SHIFT.  MAGIClast
 * will not overwrite CHUNK1 if OFFSET1 > (OFFSETlast >> SHIFT) +
 * ADDOFFSET.
 * 
 * Make SHIFT the maximal possible (there is no point in making it
 * smaller).  Since OFFSETlast is 2K - CHUNKSIZE, above restrictions
 * give restrictions on OFFSET1 and on ADDOFFSET.
 * 
 * In particular, for chunks of size 2^k with k>=6 we can put
 * ADDOFFSET to be from 0 to 2^k - 2^(11-k), and have
 * OFFSET1==chunksize.  For chunks of size 80 OFFSET1 of 2K%80=48 is
 * large enough to have ADDOFFSET between 1 and 16 (similarly for 96,
 * when ADDOFFSET should be 1).  In particular, keeping MAGICs for
 * these sizes gives no additional size penalty.
 * 
 * However, for chunks of size 2^k with k<=5 this gives OFFSET1 >=
 * ADDOFSET + 2^(11-k).  Keeping ADDOFFSET 0 allows for 2^(11-k)-2^(11-2k)
 * chunks per arena.  This is smaller than 2^(11-k) - 1 which are
 * needed if no MAGIC is kept.  [In fact, having a negative ADDOFFSET
 * would allow for slightly more buckets per arena for k=2,3.]
 * 
 * Similarly, for chunks of size 3/2*2^k with k<=5 MAGICs would span
 * the area up to 2^(11-k)+ADDOFFSET.  For k=4 this give optimal
 * ADDOFFSET as -7..0.  For k=3 ADDOFFSET can go up to 4 (with tiny
 * savings for negative ADDOFFSET).  For k=5 ADDOFFSET can go -1..16
 * (with no savings for negative values).
 *
 * In particular, keeping ADDOFFSET 0 for sizes of chunks up to 2^6
 * leads to tiny pessimizations in case of sizes 4, 8, 12, 24, and
 * leads to no contradictions except for size=80 (or 96.)
 *
 * However, it also makes sense to keep no magic for sizes 48 or less.
 * This is what we do.  In this case one needs ADDOFFSET>=1 also for
 * chunksizes 12, 24, and 48, unless one gets one less chunk per
 * arena.
 *  
 * The algo of OV_MAGIC(block,bucket) keeps ADDOFFSET 0 until
 * chunksize of 64, then makes it 1. 
 *
 * This allows for an additional optimization: the above scheme leads
 * to giant overheads for sizes 128 or more (one whole chunk needs to
 * be sacrificed to keep INDEX).  Instead we use chunks not of size
 * 2^k, but of size 2^k-ALIGN.  If we pack these chunks at the end of
 * the arena, then the beginnings are still in different 2^k-long
 * sections of the arena if k>=7 for ALIGN==4, and k>=8 if ALIGN=8.
 * Thus for k>7 the above algo of calculating the offset of the magic
 * will still give different answers for different chunks.  And to
 * avoid the overrun of MAGIC1 into INDEX, one needs ADDOFFSET of >=1.
 * In the case k=7 we just move the first chunk an extra ALIGN
 * backward inside the ARENA (this is done once per arena lifetime,
 * thus is not a big overhead).  */
#  define MAX_PACKED_POW2 6
#  define MAX_PACKED (MAX_PACKED_POW2 * BUCKETS_PER_POW2 + BUCKET_POW2_SHIFT)
#  define MAX_POW2_ALGO ((1<<(MAX_PACKED_POW2 + 1)) - M_OVERHEAD)
#  define TWOK_MASK nBIT_MASK(LOG_OF_MIN_ARENA)
#  define TWOK_MASKED(x) (PTR2UV(x) & ~TWOK_MASK)
#  define TWOK_SHIFT(x) (PTR2UV(x) & TWOK_MASK)
#  define OV_INDEXp(block) (INT2PTR(u_char*,TWOK_MASKED(block)))
#  define OV_INDEX(block) (*OV_INDEXp(block))
#  define OV_MAGIC(block,bucket) (*(OV_INDEXp(block) +			\
                                    (TWOK_SHIFT(block)>>		\
                                     (bucket>>BUCKET_POW2_SHIFT)) +	\
                                    (bucket >= MIN_NEEDS_SHIFT ? 1 : 0)))
    /* A bucket can have a shift smaller than it size, we need to
       shift its magic number so it will not overwrite index: */
#  ifdef BUCKETS_ROOT2
#    define MIN_NEEDS_SHIFT (7*BUCKETS_PER_POW2 - 1) /* Shift 80 greater than chunk 64. */
#  else
#    define MIN_NEEDS_SHIFT (7*BUCKETS_PER_POW2) /* Shift 128 greater than chunk 32. */
#  endif 
#  define CHUNK_SHIFT 0

/* Number of active buckets of given ordinal. */
#ifdef IGNORE_SMALL_BAD_FREE
#define FIRST_BUCKET_WITH_CHECK (6 * BUCKETS_PER_POW2) /* 64 */
#  define N_BLKS(bucket) ( (bucket) < FIRST_BUCKET_WITH_CHECK 		\
                         ? nBIT_MASK(LOG_OF_MIN_ARENA)/BUCKET_SIZE_NO_SURPLUS(bucket) \
                         : n_blks[bucket] )
#else
#  define N_BLKS(bucket) n_blks[bucket]
#endif 

static const u_short n_blks[LOG_OF_MIN_ARENA * BUCKETS_PER_POW2] =
  {
#  if BUCKETS_PER_POW2==1
      0, 0,
      (MIN_BUC_POW2==2 ? 384 : 0),
      224, 120, 62, 31, 16, 8, 4, 2
#  else
      0, 0, 0, 0,
      (MIN_BUC_POW2==2 ? 384 : 0), (MIN_BUC_POW2==2 ? 384 : 0),	/* 4, 4 */
      224, 149, 120, 80, 62, 41, 31, 25, 16, 16, 8, 8, 4, 4, 2, 2
#  endif
  };

/* Shift of the first bucket with the given ordinal inside 2K chunk. */
#ifdef IGNORE_SMALL_BAD_FREE
#  define BLK_SHIFT(bucket) ( (bucket) < FIRST_BUCKET_WITH_CHECK 	\
                              ? ((1<<LOG_OF_MIN_ARENA)			\
                                 - BUCKET_SIZE_NO_SURPLUS(bucket) * N_BLKS(bucket)) \
                              : blk_shift[bucket])
#else
#  define BLK_SHIFT(bucket) blk_shift[bucket]
#endif 

static const u_short blk_shift[LOG_OF_MIN_ARENA * BUCKETS_PER_POW2] =
  { 
#  if BUCKETS_PER_POW2==1
      0, 0,
      (MIN_BUC_POW2==2 ? 512 : 0),
      256, 128, 64, 64,			/* 8 to 64 */
      16*sizeof(union overhead), 
      8*sizeof(union overhead), 
      4*sizeof(union overhead), 
      2*sizeof(union overhead), 
#  else
      0, 0, 0, 0,
      (MIN_BUC_POW2==2 ? 512 : 0), (MIN_BUC_POW2==2 ? 512 : 0),
      256, 260, 128, 128, 64, 80, 64, 48, /* 8 to 96 */
      16*sizeof(union overhead), 16*sizeof(union overhead), 
      8*sizeof(union overhead), 8*sizeof(union overhead), 
      4*sizeof(union overhead), 4*sizeof(union overhead), 
      2*sizeof(union overhead), 2*sizeof(union overhead), 
#  endif 
  };

#  define NEEDED_ALIGNMENT 0x800	/* 2k boundaries */
#  define WANTED_ALIGNMENT 0x800	/* 2k boundaries */

#else  /* !PACK_MALLOC */

#  define OV_MAGIC(block,bucket) (block)->ov_magic
#  define OV_INDEX(block) (block)->ov_index
#  define CHUNK_SHIFT 1
#  define MAX_PACKED -1
#  define NEEDED_ALIGNMENT MEM_ALIGNBYTES
#  define WANTED_ALIGNMENT 0x400	/* 1k boundaries */

#endif /* !PACK_MALLOC */

#define M_OVERHEAD (sizeof(union overhead) + RMAGIC_SZ) /* overhead at start+end */

#ifdef PACK_MALLOC
#  define MEM_OVERHEAD(bucket) \
  (bucket <= MAX_PACKED ? ((size_t)0) : M_OVERHEAD)
#  ifdef SMALL_BUCKET_VIA_TABLE
#    define START_SHIFTS_BUCKET ((MAX_PACKED_POW2 + 1) * BUCKETS_PER_POW2)
#    define START_SHIFT MAX_PACKED_POW2
#    ifdef BUCKETS_ROOT2		/* Chunks of size 3*2^n. */
#      define SIZE_TABLE_MAX 80
#    else
#      define SIZE_TABLE_MAX 64
#    endif 
static const char bucket_of[] =
  {
#    ifdef BUCKETS_ROOT2		/* Chunks of size 3*2^n. */
      /* 0 to 15 in 4-byte increments. */
      (sizeof(void*) > 4 ? 6 : 5),	/* 4/8, 5-th bucket for better reports */
      6,				/* 8 */
      IF_ALIGN_8(8,7), 8,		/* 16/12, 16 */
      9, 9, 10, 10,			/* 24, 32 */
      11, 11, 11, 11,			/* 48 */
      12, 12, 12, 12,			/* 64 */
      13, 13, 13, 13,			/* 80 */
      13, 13, 13, 13			/* 80 */
#    else /* !BUCKETS_ROOT2 */
      /* 0 to 15 in 4-byte increments. */
      (sizeof(void*) > 4 ? 3 : 2),
      3, 
      4, 4, 
      5, 5, 5, 5,
      6, 6, 6, 6,
      6, 6, 6, 6
#    endif /* !BUCKETS_ROOT2 */
  };
#  else  /* !SMALL_BUCKET_VIA_TABLE */
#    define START_SHIFTS_BUCKET MIN_BUCKET
#    define START_SHIFT (MIN_BUC_POW2 - 1)
#  endif /* !SMALL_BUCKET_VIA_TABLE */
#else  /* !PACK_MALLOC */
#  define MEM_OVERHEAD(bucket) M_OVERHEAD
#  ifdef SMALL_BUCKET_VIA_TABLE
#    undef SMALL_BUCKET_VIA_TABLE
#  endif 
#  define START_SHIFTS_BUCKET MIN_BUCKET
#  define START_SHIFT (MIN_BUC_POW2 - 1)
#endif /* !PACK_MALLOC */

/*
 * Big allocations are often of the size 2^n bytes. To make them a
 * little bit better, make blocks of size 2^n+pagesize for big n.
 */

#ifdef TWO_POT_OPTIMIZE

#  ifndef PERL_PAGESIZE
#    define PERL_PAGESIZE 4096
#  endif 
#  ifndef FIRST_BIG_POW2
#    define FIRST_BIG_POW2 15	/* 32K, 16K is used too often. */
#  endif
#  define FIRST_BIG_BLOCK (1<<FIRST_BIG_POW2)
/* If this value or more, check against bigger blocks. */
#  define FIRST_BIG_BOUND (FIRST_BIG_BLOCK - M_OVERHEAD)
/* If less than this value, goes into 2^n-overhead-block. */
#  define LAST_SMALL_BOUND ((FIRST_BIG_BLOCK>>1) - M_OVERHEAD)

#  define POW2_OPTIMIZE_ADJUST(nbytes)				\
   ((nbytes >= FIRST_BIG_BOUND) ? nbytes -= PERL_PAGESIZE : 0)
#  define POW2_OPTIMIZE_SURPLUS(bucket)				\
   ((size_t)((bucket >= FIRST_BIG_POW2 * BUCKETS_PER_POW2) ? PERL_PAGESIZE : 0))

#else  /* !TWO_POT_OPTIMIZE */
#  define POW2_OPTIMIZE_ADJUST(nbytes)
#  define POW2_OPTIMIZE_SURPLUS(bucket) ((size_t)0)
#endif /* !TWO_POT_OPTIMIZE */

#define BARK_64K_LIMIT(what,nbytes,size)

#ifndef MIN_SBRK
#  define MIN_SBRK 2048
#endif 

#ifndef FIRST_SBRK
#  define FIRST_SBRK (48*1024)
#endif 

/* Minimal sbrk in percents of what is already alloced. */
#ifndef MIN_SBRK_FRAC
#  define MIN_SBRK_FRAC 3
#endif 

#ifndef SBRK_ALLOW_FAILURES
#  define SBRK_ALLOW_FAILURES 3
#endif 

#ifndef SBRK_FAILURE_PRICE
#  define SBRK_FAILURE_PRICE 50
#endif 

static void	morecore	(int bucket);
#  if defined(DEBUGGING)
static void	botch		(const char *diag, const char *s, const char *file, int line);
#  endif
static void	add_to_chain	(void *p, MEM_SIZE size, MEM_SIZE chip);
static void*	get_from_chain	(MEM_SIZE size);
static void*	get_from_bigger_buckets(int bucket, MEM_SIZE size);
static union overhead *getpages	(MEM_SIZE needed, int *nblksp, int bucket);
static int	getpages_adjacent(MEM_SIZE require);

#ifdef I_MACH_CTHREADS
#  undef  MUTEX_LOCK
#  define MUTEX_LOCK(m)   STMT_START { if (*m) mutex_lock(*m);   } STMT_END
#  undef  MUTEX_UNLOCK
#  define MUTEX_UNLOCK(m) STMT_START { if (*m) mutex_unlock(*m); } STMT_END
#endif

#ifndef PTRSIZE
#  define PTRSIZE	sizeof(void*)
#endif

#ifndef BITS_IN_PTR
#  define BITS_IN_PTR (8*PTRSIZE)
#endif

/*
 * nextf[i] is the pointer to the next free block of size 2^i.  The
 * smallest allocatable block is 8 bytes.  The overhead information
 * precedes the data area returned to the user.
 */
#define NBUCKETS (BITS_IN_PTR*BUCKETS_PER_POW2 + 1)
static	union overhead *nextf[NBUCKETS];

#if defined(PURIFY) && !defined(USE_PERL_SBRK)
#  define USE_PERL_SBRK
#endif

#ifdef USE_PERL_SBRK
# define sbrk(a) Perl_sbrk(a)
Malloc_t Perl_sbrk (int size);
#elif !defined(HAS_SBRK_PROTO) /* <unistd.h> usually takes care of this */
extern	Malloc_t sbrk(int);
#endif

#ifndef MIN_SBRK_FRAC1000	/* Backward compatibility */
#  define MIN_SBRK_FRAC1000	(MIN_SBRK_FRAC * 10)
#endif

#include "malloc_ctl.h"

#ifndef NO_MALLOC_DYNAMIC_CFG
#  define PERL_MALLOC_OPT_CHARS "FMfAPGdac"

#  ifndef FILL_DEAD_DEFAULT
#    define FILL_DEAD_DEFAULT	1
#  endif
#  ifndef FILL_ALIVE_DEFAULT
#    define FILL_ALIVE_DEFAULT	1
#  endif
#  ifndef FILL_CHECK_DEFAULT
#    define FILL_CHECK_DEFAULT	1
#  endif

static IV MallocCfg[MallocCfg_last] = {
  FIRST_SBRK,
  MIN_SBRK,
  MIN_SBRK_FRAC,
  SBRK_ALLOW_FAILURES,
  SBRK_FAILURE_PRICE,
  SBRK_ALLOW_FAILURES * SBRK_FAILURE_PRICE,	/* sbrk_goodness */
  FILL_DEAD_DEFAULT,	/* FILL_DEAD */
  FILL_ALIVE_DEFAULT,	/* FILL_ALIVE */
  FILL_CHECK_DEFAULT,	/* FILL_CHECK */
  0,			/* MallocCfg_skip_cfg_env */
  0,			/* MallocCfg_cfg_env_read */
  0,			/* MallocCfg_emergency_buffer_size */
  0,			/* MallocCfg_emergency_buffer_prepared_size */
  0			/* MallocCfg_emergency_buffer_last_req */
};
IV *MallocCfg_ptr = MallocCfg;

static char* MallocCfgP[MallocCfg_last] = {
  0,			/* MallocCfgP_emergency_buffer */
  0,			/* MallocCfgP_emergency_buffer_prepared */
};
char **MallocCfgP_ptr = MallocCfgP;

#  undef MIN_SBRK
#  undef FIRST_SBRK
#  undef MIN_SBRK_FRAC1000
#  undef SBRK_ALLOW_FAILURES
#  undef SBRK_FAILURE_PRICE

#  define MIN_SBRK		MallocCfg[MallocCfg_MIN_SBRK]
#  define FIRST_SBRK		MallocCfg[MallocCfg_FIRST_SBRK]
#  define MIN_SBRK_FRAC1000	MallocCfg[MallocCfg_MIN_SBRK_FRAC1000]
#  define SBRK_ALLOW_FAILURES	MallocCfg[MallocCfg_SBRK_ALLOW_FAILURES]
#  define SBRK_FAILURE_PRICE	MallocCfg[MallocCfg_SBRK_FAILURE_PRICE]

#  define sbrk_goodness		MallocCfg[MallocCfg_sbrk_goodness]

#  define emergency_buffer_size	MallocCfg[MallocCfg_emergency_buffer_size]
#  define emergency_buffer_last_req	MallocCfg[MallocCfg_emergency_buffer_last_req]

#  define FILL_DEAD		MallocCfg[MallocCfg_filldead]
#  define FILL_ALIVE		MallocCfg[MallocCfg_fillalive]
#  define FILL_CHECK_CFG	MallocCfg[MallocCfg_fillcheck]
#  define FILL_CHECK		(FILL_DEAD && FILL_CHECK_CFG)

#  define emergency_buffer	MallocCfgP[MallocCfgP_emergency_buffer]
#  define emergency_buffer_prepared	MallocCfgP[MallocCfgP_emergency_buffer_prepared]

#else	/* defined(NO_MALLOC_DYNAMIC_CFG) */

#  define FILL_DEAD	1
#  define FILL_ALIVE	1
#  define FILL_CHECK	1
static int sbrk_goodness = SBRK_ALLOW_FAILURES * SBRK_FAILURE_PRICE;

#  define NO_PERL_MALLOC_ENV

#endif

#ifdef DEBUGGING_MSTATS
/*
 * nmalloc[i] is the difference between the number of mallocs and frees
 * for a given block size.
 */
static	u_int nmalloc[NBUCKETS];
static  u_int sbrk_slack;
static  u_int start_slack;
#else	/* !( defined DEBUGGING_MSTATS ) */
#  define sbrk_slack	0
#endif

static	u_int goodsbrk;

#ifdef PERL_EMERGENCY_SBRK

#  ifndef BIG_SIZE
#    define BIG_SIZE (1<<16)		/* 64K */
#  endif

#  ifdef NO_MALLOC_DYNAMIC_CFG
static MEM_SIZE emergency_buffer_size;
        /* 0 if the last request for more memory succeeded.
           Otherwise the size of the failing request. */
static MEM_SIZE emergency_buffer_last_req;
static char *emergency_buffer;
static char *emergency_buffer_prepared;
#  endif

#  ifndef emergency_sbrk_croak
#    define emergency_sbrk_croak	croak2
#  endif

static char *
perl_get_emergency_buffer(IV *size)
{
    dTHX;
    /* First offense, give a possibility to recover by dieing. */
    /* No malloc involved here: */
    SV *sv;
    char *pv;
    GV **gvp = (GV**)hv_fetchs(PL_defstash, "^M", FALSE);

    if (!gvp) gvp = (GV**)hv_fetchs(PL_defstash, "\015", FALSE);
    if (!gvp || !(sv = GvSV(*gvp)) || !SvPOK(sv) 
        || (SvLEN(sv) < (1<<LOG_OF_MIN_ARENA) - M_OVERHEAD))
        return NULL;		/* Now die die die... */
    /* Got it, now detach SvPV: */
    pv = SvPV_nolen(sv);
    /* Check alignment: */
    if ((PTR2UV(pv) - sizeof(union overhead)) & (NEEDED_ALIGNMENT - 1)) {
        PerlIO_puts(PerlIO_stderr(),"Bad alignment of $^M!\n");
        return NULL;		/* die die die */
    }

    SvPOK_off(sv);
    SvPV_set(sv, NULL);
    SvCUR_set(sv, 0);
    SvLEN_set(sv, 0);
    *size = malloced_size(pv) + M_OVERHEAD;
    return pv - sizeof(union overhead);
}
#  define PERL_GET_EMERGENCY_BUFFER(p)	perl_get_emergency_buffer(p)

#  ifndef NO_MALLOC_DYNAMIC_CFG
static char *
get_emergency_buffer(IV *size)
{
    char *pv = emergency_buffer_prepared;

    *size = MallocCfg[MallocCfg_emergency_buffer_prepared_size];
    emergency_buffer_prepared = 0;
    MallocCfg[MallocCfg_emergency_buffer_prepared_size] = 0;
    return pv;
}

#    define GET_EMERGENCY_BUFFER(p)	get_emergency_buffer(p)
#  else		/* NO_MALLOC_DYNAMIC_CFG */
#    define GET_EMERGENCY_BUFFER(p)	NULL
#  endif

static Malloc_t
emergency_sbrk(MEM_SIZE size)
{
    MEM_SIZE rsize = (((size - 1)>>LOG_OF_MIN_ARENA) + 1)<<LOG_OF_MIN_ARENA;

    if (size >= BIG_SIZE
        && (!emergency_buffer_last_req ||
            (size < (MEM_SIZE)emergency_buffer_last_req))) {
        /* Give the possibility to recover, but avoid an infinite cycle. */
        MALLOC_UNLOCK;
        emergency_buffer_last_req = size;
        emergency_sbrk_croak("Out of memory during \"large\" request for %" UVuf
                             " bytes, total sbrk() is %" UVuf " bytes",
                             (UV)size, (UV)(goodsbrk + sbrk_slack));
    }

    if ((MEM_SIZE)emergency_buffer_size >= rsize) {
        char *old = emergency_buffer;
        
        emergency_buffer_size -= rsize;
        emergency_buffer += rsize;
        return old;
    } else {		
        /* First offense, give a possibility to recover by dieing. */
        /* No malloc involved here: */
        IV Size;
        char *pv = GET_EMERGENCY_BUFFER(&Size);
        int have = 0;

        if (emergency_buffer_size) {
            add_to_chain(emergency_buffer, emergency_buffer_size, 0);
            emergency_buffer_size = 0;
            emergency_buffer = NULL;
            have = 1;
        }

        if (!pv)
            pv = PERL_GET_EMERGENCY_BUFFER(&Size);
        if (!pv) {
            if (have)
                goto do_croak;
            return (char *)-1;		/* Now die die die... */
        }

        /* Check alignment: */
        if (PTR2UV(pv) & (NEEDED_ALIGNMENT - 1)) {
            dTHX;

            PerlIO_puts(PerlIO_stderr(),"Bad alignment of $^M!\n");
            return (char *)-1;		/* die die die */
        }

        emergency_buffer = pv;
        emergency_buffer_size = Size;
    }
  do_croak:
    MALLOC_UNLOCK;
    emergency_sbrk_croak("Out of memory during request for %" UVuf
                         " bytes, total sbrk() is %" UVuf " bytes",
                         (UV)size, (UV)(goodsbrk + sbrk_slack));
    NOT_REACHED; /* NOTREACHED */
    return NULL;
}

#else /*  !defined(PERL_EMERGENCY_SBRK) */
#  define emergency_sbrk(size)	-1
#endif	/* defined PERL_EMERGENCY_SBRK */

/* Don't use PerlIO buffered writes as they allocate memory. */
#define MYMALLOC_WRITE2STDERR(s) PERL_UNUSED_RESULT(PerlLIO_write(PerlIO_fileno(PerlIO_stderr()),s,strlen(s)))

#ifdef DEBUGGING
#undef ASSERT
#define ASSERT(p,diag)   if (!(p)) botch(diag,STRINGIFY(p),__FILE__,__LINE__);

static void
botch(const char *diag, const char *s, const char *file, int line)
{
    dTHX;
    if (!(PERL_MAYBE_ALIVE && PERL_GET_THX))
        goto do_write;
    else {
        if (PerlIO_printf(PerlIO_stderr(),
                          "assertion botched (%s?): %s %s:%d\n",
                          diag, s, file, line) != 0) {
         do_write:		/* Can be initializing interpreter */
            MYMALLOC_WRITE2STDERR("assertion botched (");
            MYMALLOC_WRITE2STDERR(diag);
            MYMALLOC_WRITE2STDERR("?): ");
            MYMALLOC_WRITE2STDERR(s);
            MYMALLOC_WRITE2STDERR(" (");
            MYMALLOC_WRITE2STDERR(file);
            MYMALLOC_WRITE2STDERR(":");
            {
              char linebuf[10];
              char *s = linebuf + sizeof(linebuf) - 1;
              int n = line;
              *s = 0;
              do {
                *--s = '0' + (n % 10);
              } while (n /= 10);
              MYMALLOC_WRITE2STDERR(s);
            }
            MYMALLOC_WRITE2STDERR(")\n");
        }
        PerlProc_abort();
    }
}
#else
#define ASSERT(p, diag)
#endif

#ifdef MALLOC_FILL
/* Fill should be long enough to cover long */
static void
fill_pat_4bytes(unsigned char *s, size_t nbytes, const unsigned char *fill)
{
    unsigned char *e = s + nbytes;
    long *lp;
    const long lfill = *(long*)fill;

    if (PTR2UV(s) & (sizeof(long)-1)) {		/* Align the pattern */
        int shift = sizeof(long) - (PTR2UV(s) & (sizeof(long)-1));
        unsigned const char *f = fill + sizeof(long) - shift;
        unsigned char *e1 = s + shift;

        while (s < e1)
            *s++ = *f++;
    }
    lp = (long*)s;
    while ((unsigned char*)(lp + 1) <= e)
        *lp++ = lfill;
    s = (unsigned char*)lp;
    while (s < e)
        *s++ = *fill++;
}
/* Just malloc()ed */
static const unsigned char fill_feedadad[] =
 {0xFE, 0xED, 0xAD, 0xAD, 0xFE, 0xED, 0xAD, 0xAD,
  0xFE, 0xED, 0xAD, 0xAD, 0xFE, 0xED, 0xAD, 0xAD};
/* Just free()ed */
static const unsigned char fill_deadbeef[] =
 {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
  0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF};
#  define FILL_DEADBEEF(s, n)	\
        (void)(FILL_DEAD?  (fill_pat_4bytes((s), (n), fill_deadbeef), 0) : 0)
#  define FILL_FEEDADAD(s, n)	\
        (void)(FILL_ALIVE? (fill_pat_4bytes((s), (n), fill_feedadad), 0) : 0)
#else
#  define FILL_DEADBEEF(s, n)	((void)0)
#  define FILL_FEEDADAD(s, n)	((void)0)
#  undef MALLOC_FILL_CHECK
#endif

#ifdef MALLOC_FILL_CHECK
static int
cmp_pat_4bytes(unsigned char *s, size_t nbytes, const unsigned char *fill)
{
    unsigned char *e = s + nbytes;
    long *lp;
    const long lfill = *(long*)fill;

    if (PTR2UV(s) & (sizeof(long)-1)) {		/* Align the pattern */
        int shift = sizeof(long) - (PTR2UV(s) & (sizeof(long)-1));
        unsigned const char *f = fill + sizeof(long) - shift;
        unsigned char *e1 = s + shift;

        while (s < e1)
            if (*s++ != *f++)
                return 1;
    }
    lp = (long*)s;
    while ((unsigned char*)(lp + 1) <= e)
        if (*lp++ != lfill)
            return 1;
    s = (unsigned char*)lp;
    while (s < e)
        if (*s++ != *fill++)
            return 1;
    return 0;
}
#  define FILLCHECK_DEADBEEF(s, n)					\
        ASSERT(!FILL_CHECK || !cmp_pat_4bytes(s, n, fill_deadbeef),	\
               "free()ed/realloc()ed-away memory was overwritten")
#else
#  define FILLCHECK_DEADBEEF(s, n)	((void)0)
#endif

STATIC int
S_adjust_size_and_find_bucket(size_t *nbytes_p)
{
        MEM_SIZE shiftr;
        int bucket;
        size_t nbytes;

        PERL_ARGS_ASSERT_ADJUST_SIZE_AND_FIND_BUCKET;

        nbytes = *nbytes_p;

        /*
         * Convert amount of memory requested into
         * closest block size stored in hash buckets
         * which satisfies request.  Account for
         * space used per block for accounting.
         */
#ifdef PACK_MALLOC
#  ifdef SMALL_BUCKET_VIA_TABLE
        if (nbytes == 0)
            bucket = MIN_BUCKET;
        else if (nbytes <= SIZE_TABLE_MAX) {
            bucket = bucket_of[(nbytes - 1) >> BUCKET_TABLE_SHIFT];
        } else
#  else
        if (nbytes == 0)
            nbytes = 1;
        if (nbytes <= MAX_POW2_ALGO) goto do_shifts;
        else
#  endif
#endif 
        {
            POW2_OPTIMIZE_ADJUST(nbytes);
            nbytes += M_OVERHEAD;
            nbytes = (nbytes + 3) &~ 3; 
#if defined(PACK_MALLOC) && !defined(SMALL_BUCKET_VIA_TABLE)
          do_shifts:
#endif
            shiftr = (nbytes - 1) >> START_SHIFT;
            bucket = START_SHIFTS_BUCKET;
            /* apart from this loop, this is O(1) */
            while (shiftr >>= 1)
                bucket += BUCKETS_PER_POW2;
        }
        *nbytes_p = nbytes;
        return bucket;
}

/*
=for apidoc malloc

Implements L<perlapi/C<Newx>> which you should use instead.

=cut
*/

Malloc_t
Perl_malloc(size_t nbytes)
{
        union overhead *p;
        int bucket;
#if defined(DEBUGGING) || defined(RCHECK)
        MEM_SIZE size = nbytes;
#endif

        /* A structure that has more than PTRDIFF_MAX bytes is unfortunately
         * legal in C, but in such, if two elements are far enough apart, we
         * can't legally find out how far apart they are.  Limit the size of a
         * malloc so that pointer subtraction in the same structure is always
         * well defined */
        if (nbytes > PTRDIFF_MAX) {
            dTHX;
            MYMALLOC_WRITE2STDERR("Memory requests are limited to PTRDIFF_MAX"
                                  " bytes to prevent possible undefined"
                                  " behavior");
            return NULL;
        }

        BARK_64K_LIMIT("Allocation",nbytes,nbytes);
#ifdef DEBUGGING
        if ((long)nbytes < 0)
            croak("%s", "panic: malloc");
#endif

        bucket = adjust_size_and_find_bucket(&nbytes);
        MALLOC_LOCK;
        /*
         * If nothing in hash bucket right now,
         * request more memory from the system.
         */
        if (nextf[bucket] == NULL)    
                morecore(bucket);
        if ((p = nextf[bucket]) == NULL) {
                MALLOC_UNLOCK;
                {
                    dTHX;
                    if (!PL_nomemok) {
#if defined(PLAIN_MALLOC) && defined(NO_FANCY_MALLOC)
                        MYMALLOC_WRITE2STDERR("Out of memory!\n");
#else
                        char buff[80];
                        char *eb = buff + sizeof(buff) - 1;
                        char *s = eb;
                        size_t n = nbytes;

                        MYMALLOC_WRITE2STDERR("Out of memory during request for ");
#if defined(DEBUGGING) || defined(RCHECK)
                        n = size;
#endif
                        *s = 0;			
                        do {
                            *--s = '0' + (n % 10);
                        } while (n /= 10);
                        MYMALLOC_WRITE2STDERR(s);
                        MYMALLOC_WRITE2STDERR(" bytes, total sbrk() is ");
                        s = eb;
                        n = goodsbrk + sbrk_slack;
                        do {
                            *--s = '0' + (n % 10);
                        } while (n /= 10);
                        MYMALLOC_WRITE2STDERR(s);
                        MYMALLOC_WRITE2STDERR(" bytes!\n");
#endif /* defined(PLAIN_MALLOC) && defined(NO_FANCY_MALLOC) */
                        my_exit(1);
                    }
                }
                return (NULL);
        }

        /* remove from linked list */
#ifdef DEBUGGING
        if ( (PTR2UV(p) & (MEM_ALIGNBYTES - 1))
                                                /* Can't get this low */
             || (p && PTR2UV(p) < (1<<LOG_OF_MIN_ARENA)) ) {
            dTHX;
            PerlIO_printf(PerlIO_stderr(),
                          "Unaligned pointer in the free chain 0x%" UVxf "\n",
                          PTR2UV(p));
        }
        if ( (PTR2UV(p->ov_next) & (MEM_ALIGNBYTES - 1))
             || (p->ov_next && PTR2UV(p->ov_next) < (1<<LOG_OF_MIN_ARENA)) ) {
            dTHX;
            PerlIO_printf(PerlIO_stderr(),
                          "Unaligned \"next\" pointer in the free "
                          "chain 0x%" UVxf " at 0x%" UVxf "\n",
                          PTR2UV(p->ov_next), PTR2UV(p));
        }
#endif
        nextf[bucket] = p->ov_next;

        MALLOC_UNLOCK;

        DEBUG_m(PerlIO_printf(Perl_debug_log,
                              "%p: (%05lu) malloc %ld bytes\n",
                              (Malloc_t)(p + CHUNK_SHIFT),
                              (unsigned long)(PL_an++),
                              (long)size));

        FILLCHECK_DEADBEEF((unsigned char*)(p + CHUNK_SHIFT),
                           BUCKET_SIZE_REAL(bucket) + RMAGIC_SZ);

#ifdef IGNORE_SMALL_BAD_FREE
        if (bucket >= FIRST_BUCKET_WITH_CHECK)
#endif 
            OV_MAGIC(p, bucket) = MAGIC;
#ifndef PACK_MALLOC
        OV_INDEX(p) = bucket;
#endif
#ifdef RCHECK
        /*
         * Record allocated size of block and
         * bound space with magic numbers.
         */
        p->ov_rmagic = RMAGIC;
        if (bucket <= MAX_SHORT_BUCKET) {
            int i;
            
            nbytes = size + M_OVERHEAD; 
            p->ov_size = nbytes - 1;
            if ((i = nbytes & (RMAGIC_SZ-1))) {
                i = RMAGIC_SZ - i;
                while (i--) /* nbytes - RMAGIC_SZ is end of alloced area */
                    ((caddr_t)p + nbytes - RMAGIC_SZ)[i] = RMAGIC_C;
            }
            /* Same at RMAGIC_SZ-aligned RMAGIC */
            nbytes = (nbytes + RMAGIC_SZ - 1) & ~(RMAGIC_SZ - 1);
            ((u_int *)((caddr_t)p + nbytes))[-1] = RMAGIC;
        }
        FILL_FEEDADAD((unsigned char *)(p + CHUNK_SHIFT), size);
#endif
        return ((Malloc_t)(p + CHUNK_SHIFT));
}

static char *last_sbrk_top;
static char *last_op;			/* This arena can be easily extended. */
static MEM_SIZE sbrked_remains;

#ifdef DEBUGGING_MSTATS
static int sbrks;
#endif 

struct chunk_chain_s {
    struct chunk_chain_s *next;
    MEM_SIZE size;
};
static struct chunk_chain_s *chunk_chain;
static int n_chunks;
static char max_bucket;

/* Cutoff a piece of one of the chunks in the chain.  Prefer smaller chunk. */
static void *
get_from_chain(MEM_SIZE size)
{
    struct chunk_chain_s *elt = chunk_chain, **oldp = &chunk_chain;
    struct chunk_chain_s **oldgoodp = NULL;
    long min_remain = LONG_MAX;

    while (elt) {
        if (elt->size >= size) {
            long remains = elt->size - size;
            if (remains >= 0 && remains < min_remain) {
                oldgoodp = oldp;
                min_remain = remains;
            }
            if (remains == 0) {
                break;
            }
        }
        oldp = &( elt->next );
        elt = elt->next;
    }
    if (!oldgoodp) return NULL;
    if (min_remain) {
        void *ret = *oldgoodp;
        struct chunk_chain_s *next = (*oldgoodp)->next;
        
        *oldgoodp = (struct chunk_chain_s *)((char*)ret + size);
        (*oldgoodp)->size = min_remain;
        (*oldgoodp)->next = next;
        return ret;
    } else {
        void *ret = *oldgoodp;
        *oldgoodp = (*oldgoodp)->next;
        n_chunks--;
        return ret;
    }
}

static void
add_to_chain(void *p, MEM_SIZE size, MEM_SIZE chip)
{
    struct chunk_chain_s *next = chunk_chain;
    char *cp = (char*)p;
    
    cp += chip;
    chunk_chain = (struct chunk_chain_s *)cp;
    chunk_chain->size = size - chip;
    chunk_chain->next = next;
    n_chunks++;
}

static void *
get_from_bigger_buckets(int bucket, MEM_SIZE size)
{
    int price = 1;
    static int bucketprice[NBUCKETS];
    while (bucket <= max_bucket) {
        /* We postpone stealing from bigger buckets until we want it
           often enough. */
        if (nextf[bucket] && bucketprice[bucket]++ >= price) {
            /* Steal it! */
            void *ret = (void*)(nextf[bucket] - 1 + CHUNK_SHIFT);
            bucketprice[bucket] = 0;
            if (((char*)nextf[bucket]) - M_OVERHEAD == last_op) {
                last_op = NULL;		/* Disable optimization */
            }
            nextf[bucket] = nextf[bucket]->ov_next;
#ifdef DEBUGGING_MSTATS
            nmalloc[bucket]--;
            start_slack -= M_OVERHEAD;
#endif 
            add_to_chain(ret, (BUCKET_SIZE_NO_SURPLUS(bucket) +
                               POW2_OPTIMIZE_SURPLUS(bucket)), 
                         size);
            return ret;
        }
        bucket++;
    }
    return NULL;
}

static union overhead *
getpages(MEM_SIZE needed, int *nblksp, int bucket)
{
    /* Need to do (possibly expensive) system call. Try to
       optimize it for rare calling. */
    MEM_SIZE require = needed - sbrked_remains;
    char *cp;
    union overhead *ovp;
    MEM_SIZE slack = 0;

    if (sbrk_goodness > 0) {
        if (!last_sbrk_top && require < (MEM_SIZE)FIRST_SBRK) 
            require = FIRST_SBRK;
        else if (require < (MEM_SIZE)MIN_SBRK) require = MIN_SBRK;

        if (require < (Size_t)(goodsbrk * MIN_SBRK_FRAC1000 / 1000))
            require = goodsbrk * MIN_SBRK_FRAC1000 / 1000;
        require = ((require - 1 + MIN_SBRK) / MIN_SBRK) * MIN_SBRK;
    } else {
        require = needed;
        last_sbrk_top = 0;
        sbrked_remains = 0;
    }

    DEBUG_m(PerlIO_printf(Perl_debug_log, 
                          "sbrk(%ld) for %ld-byte-long arena\n",
                          (long)require, (long) needed));
    cp = (char *)sbrk(require);
#ifdef DEBUGGING_MSTATS
    sbrks++;
#endif 
    if (cp == last_sbrk_top) {
        /* Common case, anything is fine. */
        sbrk_goodness++;
        ovp = (union overhead *) (cp - sbrked_remains);
        last_op = cp - sbrked_remains;
        sbrked_remains = require - (needed - sbrked_remains);
    } else if (cp == (char *)-1) { /* no more room! */
        ovp = (union overhead *)emergency_sbrk(needed);
        if (ovp == (union overhead *)-1)
            return 0;
        if (((char*)ovp) > last_op) {	/* Cannot happen with current emergency_sbrk() */
            last_op = 0;
        }
        return ovp;
    } else {			/* Non-continuous or first sbrk(). */
        long add = sbrked_remains;
        char *newcp;

        if (sbrked_remains) {	/* Put rest into chain, we
                                   cannot use it right now. */
            add_to_chain((void*)(last_sbrk_top - sbrked_remains),
                         sbrked_remains, 0);
        }

        /* Second, check alignment. */
        slack = 0;

        /* WANTED_ALIGNMENT may be more than NEEDED_ALIGNMENT, but this may
           improve performance of memory access. */
        if (PTR2UV(cp) & (WANTED_ALIGNMENT - 1)) { /* Not aligned. */
            slack = WANTED_ALIGNMENT - (PTR2UV(cp) & (WANTED_ALIGNMENT - 1));
            add += slack;
        }
                
        if (add) {
            DEBUG_m(PerlIO_printf(Perl_debug_log, 
                                  "sbrk(%ld) to fix non-continuous/off-page sbrk:\n\t%ld for alignment,\t%ld were assumed to come from the tail of the previous sbrk\n",
                                  (long)add, (long) slack,
                                  (long) sbrked_remains));
            newcp = (char *)sbrk(add);
#if defined(DEBUGGING_MSTATS)
            sbrks++;
            sbrk_slack += add;
#endif
            if (newcp != cp + require) {
                /* Too bad: even rounding sbrk() is not continuous.*/
                DEBUG_m(PerlIO_printf(Perl_debug_log, 
                                      "failed to fix bad sbrk()\n"));
#ifdef PACK_MALLOC
                if (slack) {
                    MALLOC_UNLOCK;
                    fatalcroak("panic: Off-page sbrk\n");
                }
#endif
                if (sbrked_remains) {
                    /* Try again. */
#if defined(DEBUGGING_MSTATS)
                    sbrk_slack += require;
#endif
                    require = needed;
                    DEBUG_m(PerlIO_printf(Perl_debug_log, 
                                          "straight sbrk(%ld)\n",
                                          (long)require));
                    cp = (char *)sbrk(require);
#ifdef DEBUGGING_MSTATS
                    sbrks++;
#endif 
                    if (cp == (char *)-1)
                        return 0;
                }
                sbrk_goodness = -1;	/* Disable optimization!
                                   Continue with not-aligned... */
            } else {
                cp += slack;
                require += sbrked_remains;
            }
        }

        if (last_sbrk_top) {
            sbrk_goodness -= SBRK_FAILURE_PRICE;
        }

        ovp = (union overhead *) cp;
        /*
         * Round up to minimum allocation size boundary
         * and deduct from block count to reflect.
         */

#  if NEEDED_ALIGNMENT > MEM_ALIGNBYTES
        if (PTR2UV(ovp) & (NEEDED_ALIGNMENT - 1))
            fatalcroak("Misalignment of sbrk()\n");
        else
#  endif
        if (PTR2UV(ovp) & (MEM_ALIGNBYTES - 1)) {
            DEBUG_m(PerlIO_printf(Perl_debug_log, 
                                  "fixing sbrk(): %d bytes off machine alignment\n",
                                  (int)(PTR2UV(ovp) & (MEM_ALIGNBYTES - 1))));
            ovp = INT2PTR(union overhead *,(PTR2UV(ovp) + MEM_ALIGNBYTES) &
                                     (MEM_ALIGNBYTES - 1));
            (*nblksp)--;
# if defined(DEBUGGING_MSTATS)
            /* This is only approx. if TWO_POT_OPTIMIZE: */
            sbrk_slack += (1 << (bucket >> BUCKET_POW2_SHIFT));
# endif
        }
        ;				/* Finish "else" */
        sbrked_remains = require - needed;
        last_op = cp;
    }
#if !defined(PLAIN_MALLOC) && !defined(NO_FANCY_MALLOC)
    emergency_buffer_last_req = 0;
#endif
    last_sbrk_top = cp + require;
#ifdef DEBUGGING_MSTATS
    goodsbrk += require;
#endif	
    return ovp;
}

static int
getpages_adjacent(MEM_SIZE require)
{	    
    if (require <= sbrked_remains) {
        sbrked_remains -= require;
    } else {
        char *cp;

        require -= sbrked_remains;
        /* We do not try to optimize sbrks here, we go for place. */
        cp = (char*) sbrk(require);
#ifdef DEBUGGING_MSTATS
        sbrks++;
        goodsbrk += require;
#endif 
        if (cp == last_sbrk_top) {
            sbrked_remains = 0;
            last_sbrk_top = cp + require;
        } else {
            if (cp == (char*)-1) {	/* Out of memory */
#ifdef DEBUGGING_MSTATS
                goodsbrk -= require;
#endif
                return 0;
            }
            /* Report the failure: */
            if (sbrked_remains)
                add_to_chain((void*)(last_sbrk_top - sbrked_remains),
                             sbrked_remains, 0);
            add_to_chain((void*)cp, require, 0);
            sbrk_goodness -= SBRK_FAILURE_PRICE;
            sbrked_remains = 0;
            last_sbrk_top = 0;
            last_op = 0;
            return 0;
        }
    }
            
    return 1;
}

/*
 * Allocate more memory to the indicated bucket.
 */
static void
morecore(int bucket)
{
        union overhead *ovp;
        int rnu;       /* 2^rnu bytes will be requested */
        int nblks;		/* become nblks blocks of the desired size */
        MEM_SIZE siz, needed;
        static int were_called = 0;

        if (nextf[bucket])
                return;
#ifndef NO_PERL_MALLOC_ENV
        if (!were_called) {
            /* It's our first time.  Initialize ourselves */
            were_called = 1;	/* Avoid a loop */
            if (!MallocCfg[MallocCfg_skip_cfg_env]) {
                char *s = getenv("PERL_MALLOC_OPT"), *t = s;
                const char *off;
                const char *opts = PERL_MALLOC_OPT_CHARS;
                int changed = 0;

                while ( t && t[0] && t[1] == '='
                        && ((off = strchr(opts, *t))) ) {
                    IV val = 0;

                    t += 2;
                    while (isDIGIT(*t))
                        val = 10*val + *t++ - '0';
                    if (!*t || *t == ';') {
                        if (MallocCfg[off - opts] != val)
                            changed = 1;
                        MallocCfg[off - opts] = val;
                        if (*t)
                            t++;
                    }
                }
                if (t && *t) {
                    dTHX;
                    MYMALLOC_WRITE2STDERR("Unrecognized part of PERL_MALLOC_OPT: \"");
                    MYMALLOC_WRITE2STDERR(t);
                    MYMALLOC_WRITE2STDERR("\"\n");
                }
                if (changed)
                    MallocCfg[MallocCfg_cfg_env_read] = 1;
            }
        }
#endif
        if (bucket == sizeof(MEM_SIZE)*8*BUCKETS_PER_POW2) {
            MALLOC_UNLOCK;
            croak("%s", "Out of memory during ridiculously large request");
        }
        if (bucket > max_bucket)
            max_bucket = bucket;

        rnu = ( (bucket <= (LOG_OF_MIN_ARENA << BUCKET_POW2_SHIFT)) 
                ? LOG_OF_MIN_ARENA 
                : (bucket >> BUCKET_POW2_SHIFT) );
        /* This may be overwritten later: */
        nblks = 1 << (rnu - (bucket >> BUCKET_POW2_SHIFT)); /* how many blocks to get */
        needed = ((MEM_SIZE)1 << rnu) + POW2_OPTIMIZE_SURPLUS(bucket);
        if (nextf[rnu << BUCKET_POW2_SHIFT]) { /* 2048b bucket. */
            ovp = nextf[rnu << BUCKET_POW2_SHIFT] - 1 + CHUNK_SHIFT;
            nextf[rnu << BUCKET_POW2_SHIFT]
                = nextf[rnu << BUCKET_POW2_SHIFT]->ov_next;
#ifdef DEBUGGING_MSTATS
            nmalloc[rnu << BUCKET_POW2_SHIFT]--;
            start_slack -= M_OVERHEAD;
#endif 
            DEBUG_m(PerlIO_printf(Perl_debug_log, 
                                  "stealing %ld bytes from %ld arena\n",
                                  (long) needed, (long) rnu << BUCKET_POW2_SHIFT));
        } else if (chunk_chain 
                   && (ovp = (union overhead*) get_from_chain(needed))) {
            DEBUG_m(PerlIO_printf(Perl_debug_log, 
                                  "stealing %ld bytes from chain\n",
                                  (long) needed));
        } else if ( (ovp = (union overhead*)
                     get_from_bigger_buckets((rnu << BUCKET_POW2_SHIFT) + 1,
                                             needed)) ) {
            DEBUG_m(PerlIO_printf(Perl_debug_log, 
                                  "stealing %ld bytes from bigger buckets\n",
                                  (long) needed));
        } else if (needed <= sbrked_remains) {
            ovp = (union overhead *)(last_sbrk_top - sbrked_remains);
            sbrked_remains -= needed;
            last_op = (char*)ovp;
        } else 
            ovp = getpages(needed, &nblks, bucket);

        if (!ovp)
            return;
        FILL_DEADBEEF((unsigned char*)ovp, needed);

        /*
         * Add new memory allocated to that on
         * free list for this hash bucket.
         */
        siz = BUCKET_SIZE_NO_SURPLUS(bucket); /* No surplus if nblks > 1 */
#ifdef PACK_MALLOC
        *(u_char*)ovp = bucket;	/* Fill index. */
        if (bucket <= MAX_PACKED) {
            ovp = (union overhead *) ((char*)ovp + BLK_SHIFT(bucket));
            nblks = N_BLKS(bucket);
#  ifdef DEBUGGING_MSTATS
            start_slack += BLK_SHIFT(bucket);
#  endif
        } else if (bucket < LOG_OF_MIN_ARENA * BUCKETS_PER_POW2) {
            ovp = (union overhead *) ((char*)ovp + BLK_SHIFT(bucket));
            siz -= sizeof(union overhead);
        } else ovp++;		/* One chunk per block. */
#endif /* PACK_MALLOC */
        nextf[bucket] = ovp;
#ifdef DEBUGGING_MSTATS
        nmalloc[bucket] += nblks;
        if (bucket > MAX_PACKED) {
            start_slack += M_OVERHEAD * nblks;
        }
#endif 

        while (--nblks > 0) {
                ovp->ov_next = (union overhead *)((caddr_t)ovp + siz);
                ovp = (union overhead *)((caddr_t)ovp + siz);
        }
        /* Not all sbrks return zeroed memory.*/
        ovp->ov_next = (union overhead *)NULL;
#ifdef PACK_MALLOC
        if (bucket == 7*BUCKETS_PER_POW2) { /* Special case, explanation is above. */
            union overhead *n_op = nextf[7*BUCKETS_PER_POW2]->ov_next;
            nextf[7*BUCKETS_PER_POW2] = 
                (union overhead *)((caddr_t)nextf[7*BUCKETS_PER_POW2] 
                                   - sizeof(union overhead));
            nextf[7*BUCKETS_PER_POW2]->ov_next = n_op;
        }
#endif /* !PACK_MALLOC */
}

/*
=for apidoc mfree

Implements L<perlapi/C<Safefree>> which you should use instead.

=cut
*/

Free_t
Perl_mfree(Malloc_t where)
{
        MEM_SIZE size;
        union overhead *ovp;
        char *cp = (char*)where;
#ifdef PACK_MALLOC
        u_char bucket;
#endif 

        DEBUG_m(PerlIO_printf(Perl_debug_log, 
                              "0x%" UVxf ": (%05lu) free\n",
                              PTR2UV(cp), (unsigned long)(PL_an++)));

        if (cp == NULL)
                return;
#ifdef DEBUGGING
        if (PTR2UV(cp) & (MEM_ALIGNBYTES - 1))
            croak("%s", "wrong alignment in free()");
#endif
        ovp = (union overhead *)((caddr_t)cp 
                                - sizeof (union overhead) * CHUNK_SHIFT);
#ifdef PACK_MALLOC
        bucket = OV_INDEX(ovp);
#endif 
#ifdef IGNORE_SMALL_BAD_FREE
        if ((bucket >= FIRST_BUCKET_WITH_CHECK) 
            && (OV_MAGIC(ovp, bucket) != MAGIC))
#else
        if (OV_MAGIC(ovp, bucket) != MAGIC)
#endif 
            {
                static int bad_free_warn = -1;
                if (bad_free_warn == -1) {
                    dTHX;
                    char *pbf = PerlEnv_getenv("PERL_BADFREE");
                    bad_free_warn = (pbf) ? strNE("0", pbf) : 1;
                }
                if (!bad_free_warn)
                    return;
#ifdef RCHECK
                {
                    dTHX;
                    if (!PERL_IS_ALIVE || !PL_curcop)
                        Perl_ck_warner_d(aTHX_ packWARN(WARN_MALLOC), "%s free() ignored (RMAGIC, PERL_CORE)",
                                         ovp->ov_rmagic == RMAGIC - 1 ?
                                         "Duplicate" : "Bad");
                }
#else
                {
                    dTHX;
                    if (!PERL_IS_ALIVE || !PL_curcop)
                        Perl_ck_warner_d(aTHX_ packWARN(WARN_MALLOC), "%s", "Bad free() ignored (PERL_CORE)");
                }
#endif
                return;				/* sanity */
            }
#ifdef RCHECK
        ASSERT(ovp->ov_rmagic == RMAGIC, "chunk's head overwrite");
        if (OV_INDEX(ovp) <= MAX_SHORT_BUCKET) {
            int i;
            MEM_SIZE nbytes = ovp->ov_size + 1;

            if ((i = nbytes & (RMAGIC_SZ-1))) {
                i = RMAGIC_SZ - i;
                while (i--) {	/* nbytes - RMAGIC_SZ is end of alloced area */
                    ASSERT(((caddr_t)ovp + nbytes - RMAGIC_SZ)[i] == RMAGIC_C,
                           "chunk's tail overwrite");
                }
            }
            /* Same at RMAGIC_SZ-aligned RMAGIC */
            nbytes = (nbytes + (RMAGIC_SZ-1)) & ~(RMAGIC_SZ-1);
            ASSERT(((u_int *)((caddr_t)ovp + nbytes))[-1] == RMAGIC,
                   "chunk's tail overwrite");	    
            FILLCHECK_DEADBEEF((unsigned char*)((caddr_t)ovp + nbytes),
                               BUCKET_SIZE(OV_INDEX(ovp)) - nbytes);
        }
        FILL_DEADBEEF((unsigned char*)(ovp+CHUNK_SHIFT),
                      BUCKET_SIZE_REAL(OV_INDEX(ovp)) + RMAGIC_SZ);
        ovp->ov_rmagic = RMAGIC - 1;
#endif
        ASSERT(OV_INDEX(ovp) < NBUCKETS, "chunk's head overwrite");
        size = OV_INDEX(ovp);

        MALLOC_LOCK;
        ovp->ov_next = nextf[size];
        nextf[size] = ovp;
        MALLOC_UNLOCK;
}

/*
=for apidoc realloc

Implements L<perlapi/C<Renew>> which you should use instead.

=cut
*/

/* There is no need to do any locking in realloc (with an exception of
   trying to grow in place if we are at the end of the chain).
   If somebody calls us from a different thread with the same address,
   we are sole anyway.  */

Malloc_t
Perl_realloc(void *mp, size_t nbytes)
{
        MEM_SIZE onb;
        union overhead *ovp;
        char *res;
        int prev_bucket;
        int bucket;
        int incr;		/* 1 if does not fit, -1 if "easily" fits in a
                                   smaller bucket, otherwise 0.  */
        char *cp = (char*)mp;

#ifdef DEBUGGING
        MEM_SIZE size = nbytes;

        if ((long)nbytes < 0)
            croak("%s", "panic: realloc");
#endif

        BARK_64K_LIMIT("Reallocation",nbytes,size);
        if (!cp)
                return Perl_malloc(nbytes);

        ovp = (union overhead *)((caddr_t)cp 
                                - sizeof (union overhead) * CHUNK_SHIFT);
        bucket = OV_INDEX(ovp);

#ifdef IGNORE_SMALL_BAD_FREE
        if ((bucket >= FIRST_BUCKET_WITH_CHECK) 
            && (OV_MAGIC(ovp, bucket) != MAGIC))
#else
        if (OV_MAGIC(ovp, bucket) != MAGIC)
#endif 
            {
                static int bad_free_warn = -1;
                if (bad_free_warn == -1) {
                    dTHX;
                    char *pbf = PerlEnv_getenv("PERL_BADFREE");
                    bad_free_warn = (pbf) ? strNE("0", pbf) : 1;
                }
                if (!bad_free_warn)
                    return NULL;
#ifdef RCHECK
                {
                    dTHX;
                    if (!PERL_IS_ALIVE || !PL_curcop)
                        Perl_ck_warner_d(aTHX_ packWARN(WARN_MALLOC), "%srealloc() %signored",
                                         (ovp->ov_rmagic == RMAGIC - 1 ? "" : "Bad "),
                                         ovp->ov_rmagic == RMAGIC - 1
                                         ? "of freed memory " : "");
                }
#else
                {
                    dTHX;
                    if (!PERL_IS_ALIVE || !PL_curcop)
                        Perl_ck_warner_d(aTHX_ packWARN(WARN_MALLOC), "%s",
                                         "Bad realloc() ignored");
                }
#endif
                return NULL;			/* sanity */
            }

        onb = BUCKET_SIZE_REAL(bucket);
        /* 
         *  avoid the copy if same size block.
         *  We are not aggressive with boundary cases. Note that it might
         *  (for a small number of cases) give false negative if
         *  both new size and old one are in the bucket for
         *  FIRST_BIG_POW2, but the new one is near the lower end.
         *
         *  We do not try to go to 1.5 times smaller bucket so far.
         */
        if (nbytes > onb) incr = 1;
        else {
#ifdef DO_NOT_TRY_HARDER_WHEN_SHRINKING
            if ( /* This is a little bit pessimal if PACK_MALLOC: */
                nbytes > ( (onb >> 1) - M_OVERHEAD )
#  ifdef TWO_POT_OPTIMIZE
                || (bucket == FIRST_BIG_POW2 && nbytes >= LAST_SMALL_BOUND )
#  endif	
                )
#else  /* !DO_NOT_TRY_HARDER_WHEN_SHRINKING */
                prev_bucket = ( (bucket > MAX_PACKED + 1) 
                                ? bucket - BUCKETS_PER_POW2
                                : bucket - 1);
             if (nbytes > BUCKET_SIZE_REAL(prev_bucket))
#endif /* !DO_NOT_TRY_HARDER_WHEN_SHRINKING */
                 incr = 0;
             else incr = -1;
        }
#ifdef STRESS_REALLOC
        goto hard_way;
#endif
        if (incr == 0) {
          inplace_label:
#ifdef RCHECK
                /*
                 * Record new allocated size of block and
                 * bound space with magic numbers.
                 */
                if (OV_INDEX(ovp) <= MAX_SHORT_BUCKET) {
                       int i, nb = ovp->ov_size + 1;

                       if ((i = nb & (RMAGIC_SZ-1))) {
                           i = RMAGIC_SZ - i;
                           while (i--) { /* nb - RMAGIC_SZ is end of alloced area */
                               ASSERT(((caddr_t)ovp + nb - RMAGIC_SZ)[i] == RMAGIC_C, "chunk's tail overwrite");
                           }
                       }
                       /* Same at RMAGIC_SZ-aligned RMAGIC */
                       nb = (nb + (RMAGIC_SZ-1)) & ~(RMAGIC_SZ-1);
                       ASSERT(((u_int *)((caddr_t)ovp + nb))[-1] == RMAGIC,
                              "chunk's tail overwrite");
                       FILLCHECK_DEADBEEF((unsigned char*)((caddr_t)ovp + nb),
                                          BUCKET_SIZE(OV_INDEX(ovp)) - nb);
                       if (nbytes > ovp->ov_size + 1 - M_OVERHEAD)
                           FILL_FEEDADAD((unsigned char*)cp + ovp->ov_size + 1 - M_OVERHEAD,
                                     nbytes - (ovp->ov_size + 1 - M_OVERHEAD));
                       else
                           FILL_DEADBEEF((unsigned char*)cp + nbytes,
                                         nb - M_OVERHEAD + RMAGIC_SZ - nbytes);
                        /*
                         * Convert amount of memory requested into
                         * closest block size stored in hash buckets
                         * which satisfies request.  Account for
                         * space used per block for accounting.
                         */
                        nbytes += M_OVERHEAD;
                        ovp->ov_size = nbytes - 1;
                        if ((i = nbytes & (RMAGIC_SZ-1))) {
                            i = RMAGIC_SZ - i;
                            while (i--)	/* nbytes - RMAGIC_SZ is end of alloced area */
                                ((caddr_t)ovp + nbytes - RMAGIC_SZ)[i]
                                    = RMAGIC_C;
                        }
                        /* Same at RMAGIC_SZ-aligned RMAGIC */
                        nbytes = (nbytes + (RMAGIC_SZ-1)) & ~(RMAGIC_SZ - 1);
                        ((u_int *)((caddr_t)ovp + nbytes))[-1] = RMAGIC;
                }
#endif
                res = cp;
                DEBUG_m(PerlIO_printf(Perl_debug_log, 
                              "0x%" UVxf ": (%05lu) realloc %ld bytes inplace\n",
                              PTR2UV(res),(unsigned long)(PL_an++),
                              (long)size));
        } else if (incr == 1 && (cp - M_OVERHEAD == last_op) 
                   && (onb > (1 << LOG_OF_MIN_ARENA))) {
            MEM_SIZE require, newarena = nbytes, pow;
            int shiftr;

            POW2_OPTIMIZE_ADJUST(newarena);
            newarena = newarena + M_OVERHEAD;
            /* newarena = (newarena + 3) &~ 3; */
            shiftr = (newarena - 1) >> LOG_OF_MIN_ARENA;
            pow = LOG_OF_MIN_ARENA + 1;
            /* apart from this loop, this is O(1) */
            while (shiftr >>= 1)
                pow++;
            newarena = (1 << pow) + POW2_OPTIMIZE_SURPLUS(pow * BUCKETS_PER_POW2);
            require = newarena - onb - M_OVERHEAD;
            
            MALLOC_LOCK;
            if (cp - M_OVERHEAD == last_op /* We *still* are the last chunk */
                && getpages_adjacent(require)) {
#ifdef DEBUGGING_MSTATS
                nmalloc[bucket]--;
                nmalloc[pow * BUCKETS_PER_POW2]++;
#endif 	    
                if (pow * BUCKETS_PER_POW2 > (MEM_SIZE)max_bucket)
                    max_bucket = pow * BUCKETS_PER_POW2;
                *(cp - M_OVERHEAD) = pow * BUCKETS_PER_POW2; /* Fill index. */
                MALLOC_UNLOCK;
                goto inplace_label;
            } else {
                MALLOC_UNLOCK;		
                goto hard_way;
            }
        } else {
          hard_way:
            DEBUG_m(PerlIO_printf(Perl_debug_log, 
                              "0x%" UVxf ": (%05lu) realloc %ld bytes the hard way\n",
                              PTR2UV(cp),(unsigned long)(PL_an++),
                              (long)size));
            if ((res = (char*)Perl_malloc(nbytes)) == NULL)
                return (NULL);
            if (cp != res)			/* common optimization */
                Copy(cp, res, (MEM_SIZE)(nbytes<onb?nbytes:onb), char);
            Perl_mfree(cp);
        }
        return ((Malloc_t)res);
}

/*
=for apidoc calloc

Implements L<perlapi/C<Newxz>> which you should use instead.

=cut
*/

Malloc_t
Perl_calloc(size_t elements, size_t size)
{
    long sz = elements * size;
    Malloc_t p = Perl_malloc(sz);

    if (p) {
        memset((void*)p, 0, sz);
    }
    return p;
}

char *
Perl_strdup(const char *s)
{
    MEM_SIZE l = strlen(s);
    char *s1 = (char *)Perl_malloc(l+1);

    return (char *)CopyD(s, s1, (MEM_SIZE)(l+1), char);
}

int
Perl_putenv(char *a)
{
    /* Sometimes system's putenv conflicts with my_setenv() - this is system
       malloc vs Perl's free(). */
  dTHX;
  char *var;
  char *val = a;
  MEM_SIZE l;
  char buf[80];

  while (*val && *val != '=')
      val++;
  if (!*val)
      return -1;
  l = val - a;
  if (l < sizeof(buf))
      var = buf;
  else
      var = (char *)Perl_malloc(l + 1);
  Copy(a, var, l, char);
  var[l + 1] = 0;
  my_setenv(var, val+1);
  if (var != buf)
      Perl_mfree(var);
  return 0;
}

MEM_SIZE
Perl_malloced_size(void *p)
{
    union overhead * const ovp = (union overhead *)
        ((caddr_t)p - sizeof (union overhead) * CHUNK_SHIFT);
    const int bucket = OV_INDEX(ovp);

    PERL_ARGS_ASSERT_MALLOCED_SIZE;

#ifdef RCHECK
    /* The caller wants to have a complete control over the chunk,
       disable the memory checking inside the chunk.  */
    if (bucket <= MAX_SHORT_BUCKET) {
        const MEM_SIZE size = BUCKET_SIZE_REAL(bucket);
        ovp->ov_size = size + M_OVERHEAD - 1;
        *((u_int *)((caddr_t)ovp + size + M_OVERHEAD - RMAGIC_SZ)) = RMAGIC;
    }
#endif
    return BUCKET_SIZE_REAL(bucket);
}


MEM_SIZE
Perl_malloc_good_size(size_t wanted)
{
    return BUCKET_SIZE_REAL(adjust_size_and_find_bucket(&wanted));
}

#  ifdef BUCKETS_ROOT2
#    define MIN_EVEN_REPORT 6
#  else
#    define MIN_EVEN_REPORT MIN_BUCKET
#  endif 

int
Perl_get_mstats(pTHX_ perl_mstats_t *buf, int buflen, int level)
{
#ifdef DEBUGGING_MSTATS
        int i, j;
        union overhead *p;
        struct chunk_chain_s* nextchain;

        PERL_ARGS_ASSERT_GET_MSTATS;

        buf->topbucket = buf->topbucket_ev = buf->topbucket_odd 
            = buf->totfree = buf->total = buf->total_chain = 0;

        buf->minbucket = MIN_BUCKET;
        MALLOC_LOCK;
        for (i = MIN_BUCKET ; i < NBUCKETS; i++) {
                for (j = 0, p = nextf[i]; p; p = p->ov_next, j++)
                        ;
                if (i < buflen) {
                    buf->nfree[i] = j;
                    buf->ntotal[i] = nmalloc[i];
                }		
                buf->totfree += j * BUCKET_SIZE_REAL(i);
                buf->total += nmalloc[i] * BUCKET_SIZE_REAL(i);
                if (nmalloc[i]) {
                    i % 2 ? (buf->topbucket_odd = i) : (buf->topbucket_ev = i);
                    buf->topbucket = i;
                }
        }
        nextchain = chunk_chain;
        while (nextchain) {
            buf->total_chain += nextchain->size;
            nextchain = nextchain->next;
        }
        buf->total_sbrk = goodsbrk + sbrk_slack;
        buf->sbrks = sbrks;
        buf->sbrk_good = sbrk_goodness;
        buf->sbrk_slack = sbrk_slack;
        buf->start_slack = start_slack;
        buf->sbrked_remains = sbrked_remains;
        MALLOC_UNLOCK;
        buf->nbuckets = NBUCKETS;
        if (level) {
            for (i = MIN_BUCKET ; i < NBUCKETS; i++) {
                if (i >= buflen)
                    break;
                buf->bucket_mem_size[i] = BUCKET_SIZE_NO_SURPLUS(i);
                buf->bucket_available_size[i] = BUCKET_SIZE_REAL(i);
            }
        }
#else /* defined DEBUGGING_MSTATS */
        PerlIO_printf(Perl_error_log, "perl not compiled with DEBUGGING_MSTATS\n");
#endif	/* defined DEBUGGING_MSTATS */
        return 0;		/* XXX unused */
}

/*
=for apidoc dump_mstats

When enabled by compiling with C<-DDEBUGGING_MSTATS>, print out statistics
about malloc as two lines of numbers, one showing the length of the free list
for each size category, the second showing the number of S<mallocs - frees> for
each size category.

C<s>, if not NULL, is used as a phrase to include in the output, such as
S<"after compilation">.

=cut
*/

void
Perl_dump_mstats(pTHX_ const char *s)
{
#ifdef DEBUGGING_MSTATS
        int i;
        perl_mstats_t buffer;
        UV nf[NBUCKETS];
        UV nt[NBUCKETS];

        PERL_ARGS_ASSERT_DUMP_MSTATS;

        buffer.nfree  = nf;
        buffer.ntotal = nt;
        get_mstats(&buffer, NBUCKETS, 0);

        if (s)
            PerlIO_printf(Perl_error_log,
                          "Memory allocation statistics %s (buckets %" IVdf
                          "(%" IVdf ")..%" IVdf "(%" IVdf ")\n",
                          s, 
                          (IV)BUCKET_SIZE_REAL(MIN_BUCKET), 
                          (IV)BUCKET_SIZE_NO_SURPLUS(MIN_BUCKET),
                          (IV)BUCKET_SIZE_REAL(buffer.topbucket), 
                          (IV)BUCKET_SIZE_NO_SURPLUS(buffer.topbucket));
        PerlIO_printf(Perl_error_log, "%8" IVdf " free:", buffer.totfree);
        for (i = MIN_EVEN_REPORT; i <= buffer.topbucket; i += BUCKETS_PER_POW2) {
                PerlIO_printf(Perl_error_log, 
                              ((i < 8*BUCKETS_PER_POW2 || i == 10*BUCKETS_PER_POW2)
                               ? " %5" UVuf
                               : ((i < 12*BUCKETS_PER_POW2) ? " %3" UVuf
                                                            : " %" UVuf)),
                              buffer.nfree[i]);
        }
#ifdef BUCKETS_ROOT2
        PerlIO_printf(Perl_error_log, "\n\t   ");
        for (i = MIN_BUCKET + 1; i <= buffer.topbucket_odd; i += BUCKETS_PER_POW2) {
                PerlIO_printf(Perl_error_log, 
                              ((i < 8*BUCKETS_PER_POW2 || i == 10*BUCKETS_PER_POW2)
                               ? " %5"UVuf 
                               : ((i < 12*BUCKETS_PER_POW2) ? " %3"UVuf : " %"UVuf)),
                              buffer.nfree[i]);
        }
#endif 
        PerlIO_printf(Perl_error_log, "\n%8" IVdf " used:",
                                      buffer.total - buffer.totfree);
        for (i = MIN_EVEN_REPORT; i <= buffer.topbucket; i += BUCKETS_PER_POW2) {
                PerlIO_printf(Perl_error_log, 
                              ((i < 8*BUCKETS_PER_POW2 || i == 10*BUCKETS_PER_POW2)
                               ? " %5" IVdf
                               : ((i < 12*BUCKETS_PER_POW2) ? " %3" IVdf : " %" IVdf)),
                              buffer.ntotal[i] - buffer.nfree[i]);
        }
#ifdef BUCKETS_ROOT2
        PerlIO_printf(Perl_error_log, "\n\t   ");
        for (i = MIN_BUCKET + 1; i <= buffer.topbucket_odd; i += BUCKETS_PER_POW2) {
                PerlIO_printf(Perl_error_log, 
                              ((i < 8*BUCKETS_PER_POW2 || i == 10*BUCKETS_PER_POW2)
                               ? " %5"IVdf 
                               : ((i < 12*BUCKETS_PER_POW2) ? " %3"IVdf : " %"IVdf)),
                              buffer.ntotal[i] - buffer.nfree[i]);
        }
#endif 
        PerlIO_printf(Perl_error_log, "\nTotal sbrk(): %" IVdf "/%" IVdf ":%"
                      IVdf ". Odd ends: pad+heads+chain+tail: %" IVdf "+%"
                      IVdf "+%" IVdf "+%" IVdf ".\n",
                      buffer.total_sbrk, buffer.sbrks, buffer.sbrk_good,
                      buffer.sbrk_slack, buffer.start_slack,
                      buffer.total_chain, buffer.sbrked_remains);
#else /* DEBUGGING_MSTATS */
        PerlIO_printf(Perl_error_log, "%s: perl not compiled with DEBUGGING_MSTATS\n",s);
#endif /* DEBUGGING_MSTATS */
}

#ifdef USE_PERL_SBRK

#   if defined(PURIFY)
#      define PERL_SBRK_VIA_MALLOC
#   endif

#   ifdef PERL_SBRK_VIA_MALLOC

/* it may seem schizophrenic to use perl's malloc and let it call system */
/* malloc, the reason for that is only the 3.2 version of the OS that had */
/* frequent core dumps within nxzonefreenolock. This sbrk routine put an */
/* end to the cores */

#      ifndef SYSTEM_ALLOC
#         define SYSTEM_ALLOC(a) malloc(a)
#      endif
#      ifndef SYSTEM_ALLOC_ALIGNMENT
#         define SYSTEM_ALLOC_ALIGNMENT MEM_ALIGNBYTES
#      endif

#   endif  /* PERL_SBRK_VIA_MALLOC */

static IV Perl_sbrk_oldchunk;
static long Perl_sbrk_oldsize;

#   define PERLSBRK_32_K (1<<15)
#   define PERLSBRK_64_K (1<<16)

Malloc_t
Perl_sbrk(int size)
{
    IV got;
    int small, reqsize;

    if (!size) return 0;
    reqsize = size; /* just for the DEBUG_m statement */
#ifdef PACK_MALLOC
    size = (size + 0x7ff) & ~0x7ff;
#endif
    if (size <= Perl_sbrk_oldsize) {
        got = Perl_sbrk_oldchunk;
        Perl_sbrk_oldchunk += size;
        Perl_sbrk_oldsize -= size;
    } else {
      if (size >= PERLSBRK_32_K) {
        small = 0;
      } else {
        size = PERLSBRK_64_K;
        small = 1;
      }
#  if NEEDED_ALIGNMENT > SYSTEM_ALLOC_ALIGNMENT
      size += NEEDED_ALIGNMENT - SYSTEM_ALLOC_ALIGNMENT;
#  endif
      got = (IV)SYSTEM_ALLOC(size);
#  if NEEDED_ALIGNMENT > SYSTEM_ALLOC_ALIGNMENT
      got = (got + NEEDED_ALIGNMENT - 1) & ~(NEEDED_ALIGNMENT - 1);
#  endif
      if (small) {
        /* Chunk is small, register the rest for future allocs. */
        Perl_sbrk_oldchunk = got + reqsize;
        Perl_sbrk_oldsize = size - reqsize;
      }
    }

    DEBUG_m(PerlIO_printf(Perl_debug_log,
            "sbrk malloc size %ld (reqsize %ld), left size %ld, give addr 0x%"
            UVxf "\n",
            size, reqsize, Perl_sbrk_oldsize, PTR2UV(got)));

    return (void *)got;
}

#endif /* ! defined USE_PERL_SBRK */

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
