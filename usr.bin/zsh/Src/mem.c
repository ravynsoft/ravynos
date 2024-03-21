/*
 * mem.c - memory management
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 1992-1997 Paul Falstad
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Paul Falstad or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Paul Falstad and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Paul Falstad and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Paul Falstad and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 *
 */

#include "zsh.mdh"
#include "mem.pro"

/*
	There are two ways to allocate memory in zsh.  The first way is
	to call zalloc/zshcalloc, which call malloc/calloc directly.  It
	is legal to call realloc() or free() on memory allocated this way.
	The second way is to call zhalloc/hcalloc, which allocates memory
	from one of the memory pools on the heap stack.  Such memory pools 
	will automatically created when the heap allocation routines are
	called.  To be sure that they are freed at appropriate times
	one should call pushheap() before one starts using heaps and
	popheap() after that (when the memory allocated on the heaps since
	the last pushheap() isn't needed anymore).
	pushheap() saves the states of all currently allocated heaps and
	popheap() resets them to the last state saved and destroys the
	information about that state.  If you called pushheap() and
	allocated some memory on the heaps and then come to a place where
	you don't need the allocated memory anymore but you still want
	to allocate memory on the heap, you should call freeheap().  This
	works like popheap(), only that it doesn't free the information
	about the heap states (i.e. the heaps are like after the call to
	pushheap() and you have to call popheap some time later).

	Memory allocated in this way does not have to be freed explicitly;
	it will all be freed when the pool is destroyed.  In fact,
	attempting to free this memory may result in a core dump.

	If possible, the heaps are allocated using mmap() so that the
	(*real*) heap isn't filled up with empty zsh heaps. If mmap()
	is not available and zsh's own allocator is used, we use a simple trick
	to avoid that: we allocate a large block of memory before allocating
	a heap pool, this memory is freed again immediately after the pool
	is allocated. If there are only small blocks on the free list this
	guarantees that the memory for the pool is at the end of the memory
	which means that we can give it back to the system when the pool is
	freed.

	hrealloc(char *p, size_t old, size_t new) is an optimisation
	with a similar interface to realloc().  Typically the new size
	will be larger than the old one, since there is no gain in
	shrinking the allocation (indeed, that will confused hrealloc()
	since it will forget that the unused space once belonged to this
	pointer).  However, new == 0 is a special case; then if we
	had to allocate a special heap for this memory it is freed at
	that point.
*/

#if defined(HAVE_SYS_MMAN_H) && defined(HAVE_MMAP) && defined(HAVE_MUNMAP)

#include <sys/mman.h>

/*
 * This definition is designed to enable use of memory mapping on MacOS.
 * However, performance tests indicate that MacOS mapped regions are
 * somewhat slower to allocate than memory from malloc(), so whether
 * using this improves performance depends on details of zhalloc().
 */
#if defined(MAP_ANON) && !defined(MAP_ANONYMOUS)
#define MAP_ANONYMOUS MAP_ANON
#endif

#if defined(MAP_ANONYMOUS) && defined(MAP_PRIVATE)

#define USE_MMAP 1
#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE)

#endif
#endif

#ifdef ZSH_MEM_WARNING
# ifndef DEBUG
#  define DEBUG 1
# endif
#endif

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)

static int h_m[1025], h_push, h_pop, h_free;

#endif

/* Make sure we align to the longest fundamental type. */
union mem_align {
    zlong l;
    double d;
};

#define H_ISIZE  sizeof(union mem_align)
#define HEAPSIZE (16384 - H_ISIZE)
/* Memory available for user data in default arena size */
#define HEAP_ARENA_SIZE (HEAPSIZE - sizeof(struct heap))
#define HEAPFREE (16384 - H_ISIZE)

/* Memory available for user data in heap h */
#define ARENA_SIZEOF(h) ((h)->size - sizeof(struct heap))

/* list of zsh heaps */

static Heap heaps;

/* a heap with free space, not always correct (it will be the last heap
 * if that was newly allocated but it may also be another one) */

static Heap fheap;

/**/
#ifdef ZSH_HEAP_DEBUG
/*
 * The heap ID we'll allocate next.
 *
 * We'll avoid using 0 as that means zero-initialised memory
 * containing a heap ID is (correctly) marked as invalid.
 */
static Heapid next_heap_id = (Heapid)1;

/*
 * The ID of the heap from which we last allocated heap memory.
 * In theory, since we carefully avoid allocating heap memory during
 * interrupts, after any call to zhalloc() or wrappers this should
 * be the ID of the heap containing the memory just returned.
 */
/**/
mod_export Heapid last_heap_id;

/*
 * Stack of heaps saved by new_heaps().
 * Assumes old_heaps() will come along and restore it later
 * (outputs an error if old_heaps() is called out of sequence).
 */
static LinkList heaps_saved;

/*
 * Debugging verbosity.  This must be set from a debugger.
 * An 'or' of bits from the enum heap_debug_verbosity.
 */
static volatile int heap_debug_verbosity;

/*
 * Generate a heap identifier that's unique up to unsigned integer wrap.
 *
 * For the purposes of debugging we won't bother trying to make a
 * heap_id globally unique, which would require checking all existing
 * heaps every time we create an ID and still wouldn't do what we
 * ideally want, which is to make sure the IDs of valid heaps are
 * different from the IDs of no-longer-valid heaps.  Given that,
 * we'll just assume that if we haven't tracked the problem when the
 * ID wraps we're out of luck.  We could change the type to a long long
 * if we wanted more room
 */

static Heapid
new_heap_id(void)
{
    return next_heap_id++;
}

/**/
#endif

/* Use new heaps from now on. This returns the old heap-list. */

/**/
mod_export Heap
new_heaps(void)
{
    Heap h;

    queue_signals();
    h = heaps;

    fheap = heaps = NULL;
    unqueue_signals();

#ifdef ZSH_HEAP_DEBUG
    if (heap_debug_verbosity & HDV_NEW) {
	fprintf(stderr, "HEAP DEBUG: heap " HEAPID_FMT
		" saved, new heaps created.\n", h->heap_id);
    }
    if (!heaps_saved)
	heaps_saved = znewlinklist();
    zpushnode(heaps_saved, h);
#endif
    return h;
}

/* Re-install the old heaps again, freeing the new ones. */

/**/
mod_export void
old_heaps(Heap old)
{
    Heap h, n;

    queue_signals();
    for (h = heaps; h; h = n) {
	n = h->next;
	DPUTS(h->sp, "BUG: old_heaps() with pushed heaps");
#ifdef ZSH_HEAP_DEBUG
	if (heap_debug_verbosity & HDV_FREE) {
	    fprintf(stderr, "HEAP DEBUG: heap " HEAPID_FMT
		    "freed in old_heaps().\n", h->heap_id);
	}
#endif
#ifdef USE_MMAP
	munmap((void *) h, h->size);
#else
	zfree(h, HEAPSIZE);
#endif
#ifdef ZSH_VALGRIND
	VALGRIND_DESTROY_MEMPOOL((char *)h);
#endif
    }
    heaps = old;
#ifdef ZSH_HEAP_DEBUG
    if (heap_debug_verbosity & HDV_OLD) {
	fprintf(stderr, "HEAP DEBUG: heap " HEAPID_FMT
		"restored.\n", heaps->heap_id);
    }
    {
	Heap myold = heaps_saved ? getlinknode(heaps_saved) : NULL;
	if (old != myold)
	{
	    fprintf(stderr, "HEAP DEBUG: invalid old heap " HEAPID_FMT
		    ", expecting " HEAPID_FMT ".\n", old->heap_id,
		    myold->heap_id);
	}
    }
#endif
    fheap = NULL;
    unqueue_signals();
}

/* Temporarily switch to other heaps (or back again). */

/**/
mod_export Heap
switch_heaps(Heap new)
{
    Heap h;

    queue_signals();
    h = heaps;

#ifdef ZSH_HEAP_DEBUG
    if (heap_debug_verbosity & HDV_SWITCH) {
	fprintf(stderr, "HEAP DEBUG: heap temporarily switched from "
		HEAPID_FMT " to " HEAPID_FMT ".\n", h->heap_id, new->heap_id);
    }
#endif
    heaps = new;
    fheap = NULL;
    unqueue_signals();

    return h;
}

/* save states of zsh heaps */

/**/
mod_export void
pushheap(void)
{
    Heap h;
    Heapstack hs;

    queue_signals();

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)
    h_push++;
#endif

    for (h = heaps; h; h = h->next) {
	DPUTS(!h->used && h->next, "BUG: empty heap");
	hs = (Heapstack) zalloc(sizeof(*hs));
	hs->next = h->sp;
	h->sp = hs;
	hs->used = h->used;
#ifdef ZSH_HEAP_DEBUG
	hs->heap_id = h->heap_id;
	h->heap_id = new_heap_id();
	if (heap_debug_verbosity & HDV_PUSH) {
	    fprintf(stderr, "HEAP DEBUG: heap " HEAPID_FMT " pushed, new id is "
		    HEAPID_FMT ".\n",
		    hs->heap_id, h->heap_id);
	}
#endif
    }
    unqueue_signals();
}

/* reset heaps to previous state */

/**/
mod_export void
freeheap(void)
{
    Heap h, hn, hl = NULL;

    queue_signals();

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)
    h_free++;
#endif

    /*
     * When pushheap() is called, it sweeps over the entire heaps list of
     * arenas and marks every one of them with the amount of free space in
     * that arena at that moment.  zhalloc() is then allowed to grab bits
     * out of any of those arenas that have free space.
     *
     * Whenever fheap is NULL here, the loop below sweeps back over the
     * entire heap list again, resetting the free space in every arena to
     * the amount stashed by pushheap() and finding the arena with the most
     * free space to optimize zhalloc()'s next search.  When there's a lot
     * of stuff already on the heap, this is an enormous amount of work,
     * and performance goes to hell.
     *
     * Therefore, we defer freeing the most recently allocated arena until
     * we reach popheap().
     *
     * However, if the arena to which fheap points is unused, we want to
     * reclaim space in earlier arenas, so we have no choice but to do the
     * sweep for a new fheap.
     */
    if (fheap && !fheap->sp)
       fheap = NULL;   /* We used to do this unconditionally */
    /*
     * In other cases, either fheap is already correct, or it has never
     * been set and this loop will do it, or it'll be reset from scratch
     * on the next popheap().  So all that's needed here is to pick up
     * the scan wherever the last pass [or the last popheap()] left off.
     */
    for (h = (fheap ? fheap : heaps); h; h = hn) {
	hn = h->next;
	if (h->sp) {
#ifdef ZSH_MEM_DEBUG
#ifdef ZSH_VALGRIND
	    VALGRIND_MAKE_MEM_UNDEFINED((char *)arena(h) + h->sp->used,
					h->used - h->sp->used);
#endif
	    memset(arena(h) + h->sp->used, 0xff, h->used - h->sp->used);
#endif
	    h->used = h->sp->used;
	    if (!fheap) {
		if (h->used < ARENA_SIZEOF(h))
		    fheap = h;
	    } else if (ARENA_SIZEOF(h) - h->used >
		       ARENA_SIZEOF(fheap) - fheap->used)
		fheap = h;
	    hl = h;
#ifdef ZSH_HEAP_DEBUG
	    /*
	     * As the free makes the heap invalid, give it a new
	     * identifier.  We're not popping it, so don't use
	     * the one in the heap stack.
	     */
	    {
		Heapid new_id = new_heap_id();
		if (heap_debug_verbosity & HDV_FREE) {
		    fprintf(stderr, "HEAP DEBUG: heap " HEAPID_FMT
			    " freed, new id is " HEAPID_FMT ".\n",
			    h->heap_id, new_id);
		}
		h->heap_id = new_id;
	    }
#endif
#ifdef ZSH_VALGRIND
	    VALGRIND_MEMPOOL_TRIM((char *)h, (char *)arena(h), h->used);
#endif
	} else {
	    if (fheap == h)
		fheap = NULL;
	    if (h->next) {
		/* We want to cut this out of the arena list if we can */
		if (h == heaps)
		    hl = heaps = h->next;
		else if (hl && hl->next == h)
		    hl->next = h->next;
		else {
		    DPUTS(hl, "hl->next != h when freeing");
		    hl = h;
		    continue;
		}
		h->next = NULL;
	    } else {
		/* Leave an empty arena at the end until popped */
		h->used = 0;
		fheap = hl = h;
		break;
	    }
#ifdef USE_MMAP
	    munmap((void *) h, h->size);
#else
	    zfree(h, HEAPSIZE);
#endif
#ifdef ZSH_VALGRIND
	    VALGRIND_DESTROY_MEMPOOL((char *)h);
#endif
	}
    }
    if (hl)
	hl->next = NULL;
    else
	heaps = fheap = NULL;

    unqueue_signals();
}

/* reset heap to previous state and destroy state information */

/**/
mod_export void
popheap(void)
{
    Heap h, hn, hl = NULL;
    Heapstack hs;

    queue_signals();

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)
    h_pop++;
#endif

    fheap = NULL;
    for (h = heaps; h; h = hn) {
	hn = h->next;
	if ((hs = h->sp)) {
	    h->sp = hs->next;
#ifdef ZSH_MEM_DEBUG
#ifdef ZSH_VALGRIND
	    VALGRIND_MAKE_MEM_UNDEFINED((char *)arena(h) + hs->used,
					h->used - hs->used);
#endif
	    memset(arena(h) + hs->used, 0xff, h->used - hs->used);
#endif
	    h->used = hs->used;
#ifdef ZSH_HEAP_DEBUG
	    if (heap_debug_verbosity & HDV_POP) {
		fprintf(stderr, "HEAP DEBUG: heap " HEAPID_FMT
			" popped, old heap was " HEAPID_FMT ".\n",
			h->heap_id, hs->heap_id);
	    }
	    h->heap_id = hs->heap_id;
#endif
#ifdef ZSH_VALGRIND
	    VALGRIND_MEMPOOL_TRIM((char *)h, (char *)arena(h), h->used);
#endif
	    if (!fheap) {
		if (h->used < ARENA_SIZEOF(h))
		    fheap = h;
	    } else if (ARENA_SIZEOF(h) - h->used >
		       ARENA_SIZEOF(fheap) - fheap->used)
		fheap = h;
	    zfree(hs, sizeof(*hs));

	    hl = h;
	} else {
	    if (h->next) {
		/* We want to cut this out of the arena list if we can */
		if (h == heaps)
		    hl = heaps = h->next;
		else if (hl && hl->next == h)
		    hl->next = h->next;
		else {
		    DPUTS(hl, "hl->next != h when popping");
		    hl = h;
		    continue;
		}
		h->next = NULL;
	    } else if (hl == h)	/* This is the last arena of all */
		hl = NULL;
#ifdef USE_MMAP
	    munmap((void *) h, h->size);
#else
	    zfree(h, HEAPSIZE);
#endif
#ifdef ZSH_VALGRIND
	    VALGRIND_DESTROY_MEMPOOL((char *)h);
#endif
	}
    }
    if (hl)
	hl->next = NULL;
    else
	heaps = NULL;

    unqueue_signals();
}

#ifdef USE_MMAP
/*
 * Utility function to allocate a heap area of at least *n bytes.
 * *n will be rounded up to the next page boundary.
 */
static Heap
mmap_heap_alloc(size_t *n)
{
    Heap h;
    static size_t pgsz = 0;

    if (!pgsz) {

#ifdef _SC_PAGESIZE
	pgsz = sysconf(_SC_PAGESIZE);     /* SVR4 */
#else
# ifdef _SC_PAGE_SIZE
	pgsz = sysconf(_SC_PAGE_SIZE);    /* HPUX */
# else
	pgsz = getpagesize();
# endif
#endif

	pgsz--;
    }
    *n = (*n + pgsz) & ~pgsz;
    h = (Heap) mmap(NULL, *n, PROT_READ | PROT_WRITE,
		    MMAP_FLAGS, -1, 0);
    if (h == ((Heap) -1)) {
	zerr("fatal error: out of heap memory");
	exit(1);
    }

    return h;
}
#endif

/* check whether a pointer is within a memory pool */

/**/
mod_export void *
zheapptr(void *p)
{
    Heap h;
    queue_signals();
    for (h = heaps; h; h = h->next)
	if ((char *)p >= arena(h) &&
	    (char *)p + H_ISIZE < arena(h) + ARENA_SIZEOF(h))
	    break;
    unqueue_signals();
    return (h ? p : 0);
}

/* allocate memory from the current memory pool */

/**/
mod_export void *
zhalloc(size_t size)
{
    Heap h, hp = NULL;
    size_t n;
#ifdef ZSH_VALGRIND
    size_t req_size = size;

    if (size == 0)
	return NULL;
#endif

    size = (size + H_ISIZE - 1) & ~(H_ISIZE - 1);

    queue_signals();

#if defined(ZSH_MEM) && defined(ZSH_MEM_DEBUG)
    h_m[size < (1024 * H_ISIZE) ? (size / H_ISIZE) : 1024]++;
#endif

    /* find a heap with enough free space */

    /*
     * This previously assigned:
     *   h = ((fheap && ARENA_SIZEOF(fheap) >= (size + fheap->used))
     *	      ? fheap : heaps);
     * but we think that nothing upstream of fheap has more free space,
     * so why start over at heaps just because fheap has too little?
     */
    for (h = (fheap ? fheap : heaps); h; h = h->next) {
	hp = h;
	if (ARENA_SIZEOF(h) >= (n = size + h->used)) {
	    void *ret;

	    h->used = n;
	    ret = arena(h) + n - size;
	    unqueue_signals();
#ifdef ZSH_HEAP_DEBUG
	    last_heap_id = h->heap_id;
	    if (heap_debug_verbosity & HDV_ALLOC) {
		fprintf(stderr, "HEAP DEBUG: allocated memory from heap "
			HEAPID_FMT ".\n", h->heap_id);
	    }
#endif
#ifdef ZSH_VALGRIND
	    VALGRIND_MEMPOOL_ALLOC((char *)h, (char *)ret, req_size);
#endif
	    return ret;
	}
    }
    {
        /* not found, allocate new heap */
#if defined(ZSH_MEM) && !defined(USE_MMAP)
	static int called = 0;
	void *foo = called ? (void *)malloc(HEAPFREE) : NULL;
            /* tricky, see above */
#endif

	n = HEAP_ARENA_SIZE > size ? HEAPSIZE : size + sizeof(*h);

#ifdef USE_MMAP
	h = mmap_heap_alloc(&n);
#else
	h = (Heap) zalloc(n);
#endif

#if defined(ZSH_MEM) && !defined(USE_MMAP)
	if (called)
	    zfree(foo, HEAPFREE);
	called = 1;
#endif

	h->size = n;
	h->used = size;
	h->next = NULL;
	h->sp = NULL;
#ifdef ZSH_HEAP_DEBUG
	h->heap_id = new_heap_id();
	if (heap_debug_verbosity & HDV_CREATE) {
	    fprintf(stderr, "HEAP DEBUG: create new heap " HEAPID_FMT ".\n",
		    h->heap_id);
	}
#endif
#ifdef ZSH_VALGRIND
	VALGRIND_CREATE_MEMPOOL((char *)h, 0, 0);
	VALGRIND_MAKE_MEM_NOACCESS((char *)arena(h),
				   n - ((char *)arena(h)-(char *)h));
	VALGRIND_MEMPOOL_ALLOC((char *)h, (char *)arena(h), req_size);
#endif

	DPUTS(hp && hp->next, "failed to find end of chain in zhalloc");
	if (hp)
	    hp->next = h;
	else
	    heaps = h;
	fheap = h;

	unqueue_signals();
#ifdef ZSH_HEAP_DEBUG
	last_heap_id = h->heap_id;
	if (heap_debug_verbosity & HDV_ALLOC) {
	    fprintf(stderr, "HEAP DEBUG: allocated memory from heap "
		    HEAPID_FMT ".\n", h->heap_id);
	}
#endif
	return arena(h);
    }
}

/**/
mod_export void *
hrealloc(char *p, size_t old, size_t new)
{
    Heap h, ph;

#ifdef ZSH_VALGRIND
    size_t new_req = new;
#endif

    old = (old + H_ISIZE - 1) & ~(H_ISIZE - 1);
    new = (new + H_ISIZE - 1) & ~(H_ISIZE - 1);

    if (old == new)
	return p;
    if (!old && !p)
#ifdef ZSH_VALGRIND
	return zhalloc(new_req);
#else
	return zhalloc(new);
#endif

    /* find the heap with p */

    queue_signals();
    for (h = heaps, ph = NULL; h; ph = h, h = h->next)
	if (p >= arena(h) && p < arena(h) + ARENA_SIZEOF(h))
	    break;

    DPUTS(!h, "BUG: hrealloc() called for non-heap memory.");
    DPUTS(h->sp && arena(h) + h->sp->used > p,
	  "BUG: hrealloc() wants to realloc pushed memory");

    /*
     * If the end of the old chunk is before the used pointer,
     * more memory has been zhalloc'ed afterwards.
     * We can't tell if that's still in use, obviously, since
     * that's the whole point of heap memory.
     * We have no choice other than to grab some more memory
     * somewhere else and copy in the old stuff.
     */
    if (p + old < arena(h) + h->used) {
	if (new > old) {
#ifdef ZSH_VALGRIND
	    char *ptr = (char *) zhalloc(new_req);
#else
	    char *ptr = (char *) zhalloc(new);
#endif
	    memcpy(ptr, p, old);
#ifdef ZSH_MEM_DEBUG
	    memset(p, 0xff, old);
#endif
#ifdef ZSH_VALGRIND
	    VALGRIND_MEMPOOL_FREE((char *)h, (char *)p);
	    /*
	     * zhalloc() marked h,ptr,new as an allocation so we don't
	     * need to do that here.
	     */
#endif
	    unqueue_signals();
	    return ptr;
	} else {
#ifdef ZSH_VALGRIND
	    VALGRIND_MEMPOOL_FREE((char *)h, (char *)p);
	    if (p) {
		VALGRIND_MEMPOOL_ALLOC((char *)h, (char *)p,
				       new_req);
		VALGRIND_MAKE_MEM_DEFINED((char *)h, (char *)p);
	    }
#endif
	    unqueue_signals();
	    return new ? p : NULL;
	}
    }

    DPUTS(p + old != arena(h) + h->used, "BUG: hrealloc more than allocated");

    /*
     * We now know there's nothing afterwards in the heap, now see if
     * there's nothing before.  Then we can reallocate the whole thing.
     * Otherwise, we need to keep the stuff at the start of the heap,
     * then allocate a new one too; this is handled below.  (This will
     * guarantee we occupy a full heap next time round, provided we
     * don't use the heap for anything else.)
     */
    if (p == arena(h)) {
#ifdef ZSH_HEAP_DEBUG
	Heapid heap_id = h->heap_id;
#endif
	/*
	 * Zero new seems to be a special case saying we've finished
	 * with the specially reallocated memory, see scanner() in glob.c.
	 */
	if (!new) {
	    if (ph)
		ph->next = h->next;
	    else
		heaps = h->next;
	    fheap = NULL;
#ifdef USE_MMAP
	    munmap((void *) h, h->size);
#else
	    zfree(h, HEAPSIZE);
#endif
#ifdef ZSH_VALGRIND
	    VALGRIND_DESTROY_MEMPOOL((char *)h);
#endif
	    unqueue_signals();
	    return NULL;
	}
	if (new > ARENA_SIZEOF(h)) {
	    Heap hnew;
	    /*
	     * Not enough memory in this heap.  Allocate a new
	     * one of sufficient size.
	     *
	     * To avoid this happening too often, allocate
	     * chunks in multiples of HEAPSIZE.
	     * (Historical note:  there didn't used to be any
	     * point in this since we didn't consistently record
	     * the allocated size of the heap, but now we do.)
	     */
	    size_t n = (new + sizeof(*h) + HEAPSIZE);
	    n -= n % HEAPSIZE;
	    fheap = NULL;

#ifdef USE_MMAP
	    {
		/*
		 * I don't know any easy portable way of requesting
		 * a mmap'd segment be extended, so simply allocate
		 * a new one and copy.
		 */
		hnew = mmap_heap_alloc(&n);
		/* Copy the entire heap, header (with next pointer) included */
		memcpy(hnew, h, h->size);
		munmap((void *)h, h->size);
	    }
#else
	    hnew = (Heap) realloc(h, n);
#endif
#ifdef ZSH_VALGRIND
	    VALGRIND_MEMPOOL_FREE((char *)h, p);
	    VALGRIND_DESTROY_MEMPOOL((char *)h);
	    VALGRIND_CREATE_MEMPOOL((char *)hnew, 0, 0);
	    VALGRIND_MEMPOOL_ALLOC((char *)hnew, (char *)arena(hnew),
				   new_req);
	    VALGRIND_MAKE_MEM_DEFINED((char *)hnew, (char *)arena(hnew));
#endif
	    h = hnew;

	    h->size = n;
	    if (ph)
		ph->next = h;
	    else
		heaps = h;
	}
#ifdef ZSH_VALGRIND
	else {
	    VALGRIND_MEMPOOL_FREE((char *)h, (char *)p);
	    VALGRIND_MEMPOOL_ALLOC((char *)h, (char *)p, new_req);
	    VALGRIND_MAKE_MEM_DEFINED((char *)h, (char *)p);
	}
#endif
	h->used = new;
#ifdef ZSH_HEAP_DEBUG
	h->heap_id = heap_id;
#endif
	unqueue_signals();
	return arena(h);
    }
#ifndef USE_MMAP
    DPUTS(h->used > ARENA_SIZEOF(h), "BUG: hrealloc at invalid address");
#endif
    if (h->used + (new - old) <= ARENA_SIZEOF(h)) {
	h->used += new - old;
	unqueue_signals();
#ifdef ZSH_VALGRIND
	VALGRIND_MEMPOOL_FREE((char *)h, (char *)p);
	VALGRIND_MEMPOOL_ALLOC((char *)h, (char *)p, new_req);
	VALGRIND_MAKE_MEM_DEFINED((char *)h, (char *)p);
#endif
	return p;
    } else {
	char *t = zhalloc(new);
	memcpy(t, p, old > new ? new : old);
	h->used -= old;
#ifdef ZSH_MEM_DEBUG
	memset(p, 0xff, old);
#endif
#ifdef ZSH_VALGRIND
	VALGRIND_MEMPOOL_FREE((char *)h, (char *)p);
	/* t already marked as allocated by zhalloc() */
#endif
	unqueue_signals();
	return t;
    }
}

/**/
#ifdef ZSH_HEAP_DEBUG
/*
 * Check if heap_id is the identifier of a currently valid heap,
 * including any heap buried on the stack, or of permanent memory.
 * Return 0 if so, else 1.
 *
 * This gets confused by use of switch_heaps().  That's because so do I.
 */

/**/
mod_export int
memory_validate(Heapid heap_id)
{
    Heap h;
    Heapstack hs;
    LinkNode node;

    if (heap_id == HEAPID_PERMANENT)
	return 0;

    queue_signals();
    for (h = heaps; h; h = h->next) {
	if (h->heap_id == heap_id) {
	    unqueue_signals();
	    return 0;
	}
	for (hs = heaps->sp; hs; hs = hs->next) {
	    if (hs->heap_id == heap_id) {
		unqueue_signals();
		return 0;
	    }
	}
    }

    if (heaps_saved) {
	for (node = firstnode(heaps_saved); node; incnode(node)) {
	    for (h = (Heap)getdata(node); h; h = h->next) {
		if (h->heap_id == heap_id) {
		    unqueue_signals();
		    return 0;
		}
		for (hs = heaps->sp; hs; hs = hs->next) {
		    if (hs->heap_id == heap_id) {
			unqueue_signals();
			return 0;
		    }
		}
	    }
	}
    }

    unqueue_signals();
    return 1;
}
/**/
#endif

/* allocate memory from the current memory pool and clear it */

/**/
mod_export void *
hcalloc(size_t size)
{
    void *ptr;

    ptr = zhalloc(size);
    memset(ptr, 0, size);
    return ptr;
}

/* allocate permanent memory */

/**/
mod_export void *
zalloc(size_t size)
{
    void *ptr;

    if (!size)
	size = 1;
    queue_signals();
    if (!(ptr = (void *) malloc(size))) {
	zerr("fatal error: out of memory");
	exit(1);
    }
    unqueue_signals();

    return ptr;
}

/**/
mod_export void *
zshcalloc(size_t size)
{
    void *ptr = zalloc(size);
    if (!size)
	size = 1;
    memset(ptr, 0, size);
    return ptr;
}

/* This front-end to realloc is used to make sure we have a realloc *
 * that conforms to POSIX realloc.  Older realloc's can fail if     *
 * passed a NULL pointer, but POSIX realloc should handle this.  A  *
 * better solution would be for configure to check if realloc is    *
 * POSIX compliant, but I'm not sure how to do that.                */

/**/
mod_export void *
zrealloc(void *ptr, size_t size)
{
    queue_signals();
    if (ptr) {
	if (size) {
	    /* Do normal realloc */
	    if (!(ptr = (void *) realloc(ptr, size))) {
		zerr("fatal error: out of memory");
		exit(1);
	    }
	    unqueue_signals();
	    return ptr;
	}
	else
	    /* If ptr is not NULL, but size is zero, *
	     * then object pointed to is freed.      */
	    free(ptr);

	ptr = NULL;
    } else {
	/* If ptr is NULL, then behave like malloc */
        if (!(ptr = (void *) malloc(size))) {
            zerr("fatal error: out of memory");
            exit(1);
        }
    }
    unqueue_signals();

    return ptr;
}

/**/
#ifdef ZSH_MEM

/*
   Below is a simple segment oriented memory allocator for systems on
   which it is better than the system's one. Memory is given in blocks
   aligned to an integer multiple of sizeof(union mem_align), which will
   probably be 64-bit as it is the longer of zlong or double. Each block is
   preceded by a header which contains the length of the data part (in
   bytes). In allocated blocks only this field of the structure m_hdr is
   senseful. In free blocks the second field (next) is a pointer to the next
   free segment on the free list.

   On top of this simple allocator there is a second allocator for small
   chunks of data. It should be both faster and less space-consuming than
   using the normal segment mechanism for such blocks.
   For the first M_NSMALL-1 possible sizes memory is allocated in arrays
   that can hold M_SNUM blocks. Each array is stored in one segment of the
   main allocator. In these segments the third field of the header structure
   (free) contains a pointer to the first free block in the array. The
   last field (used) gives the number of already used blocks in the array.

   If the macro name ZSH_MEM_DEBUG is defined, some information about the memory
   usage is stored. This information can than be viewed by calling the
   builtin `mem' (which is only available if ZSH_MEM_DEBUG is set).

   If ZSH_MEM_WARNING is defined, error messages are printed in case of errors.

   If ZSH_SECURE_FREE is defined, free() checks if the given address is really
   one that was returned by malloc(), it ignores it if it wasn't (printing
   an error message if ZSH_MEM_WARNING is also defined).
*/
#if !defined(__hpux) && !defined(DGUX) && !defined(__osf__)
# if defined(_BSD)
#  ifndef HAVE_BRK_PROTO
   extern int brk _((caddr_t));
#  endif
#  ifndef HAVE_SBRK_PROTO
   extern caddr_t sbrk _((int));
#  endif
# else
#  ifndef HAVE_BRK_PROTO
   extern int brk _((void *));
#  endif
#  ifndef HAVE_SBRK_PROTO
   extern void *sbrk _((int));
#  endif
# endif
#endif

/* structure for building free list in blocks holding small blocks */

struct m_shdr {
    struct m_shdr *next;	/* next one on free list */
#ifdef PAD_64_BIT
    /* dummy to make this 64-bit aligned */
    struct m_shdr *dummy;
#endif
};

struct m_hdr {
    zlong len;			/* length of memory block */
#if defined(PAD_64_BIT) && !defined(ZSH_64_BIT_TYPE)
    /* either 1 or 2 zlong's, whichever makes up 64 bits. */
    zlong dummy1;
#endif
    struct m_hdr *next;		/* if free: next on free list
				   if block of small blocks: next one with
				                 small blocks of same size*/
    struct m_shdr *free;	/* if block of small blocks: free list */
    zlong used;			/* if block of small blocks: number of used
				                                     blocks */
#if defined(PAD_64_BIT) && !defined(ZSH_64_BIT_TYPE)
    zlong dummy2;
#endif
};


/* alignment for memory blocks */

#define M_ALIGN (sizeof(union mem_align))

/* length of memory header, length of first field of memory header and
   minimal size of a block left free (if we allocate memory and take a
   block from the free list that is larger than needed, it must have at
   least M_MIN extra bytes to be split; if it has, the rest is put on
   the free list) */

#define M_HSIZE (sizeof(struct m_hdr))
#if defined(PAD_64_BIT) && !defined(ZSH_64_BIT_TYPE)
# define M_ISIZE (2*sizeof(zlong))
#else
# define M_ISIZE (sizeof(zlong))
#endif
#define M_MIN   (2 * M_ISIZE)

/* M_FREE  is the number of bytes that have to be free before memory is
 *         given back to the system
 * M_KEEP  is the number of bytes that will be kept when memory is given
 *         back; note that this has to be less than M_FREE
 * M_ALLOC is the number of extra bytes to request from the system */

#define M_FREE  32768
#define M_KEEP  16384
#define M_ALLOC M_KEEP

/* a pointer to the last free block, a pointer to the free list (the blocks
   on this list are kept in order - lowest address first) */

static struct m_hdr *m_lfree, *m_free;

/* system's pagesize */

static long m_pgsz = 0;

/* the highest and the lowest valid memory addresses, kept for fast validity
   checks in free() and to find out if and when we can give memory back to
   the system */

static char *m_high, *m_low;

/* Management of blocks for small blocks:
   Such blocks are kept in lists (one list for each of the sizes that are
   allocated in such blocks).  The lists are stored in the m_small array.
   M_SIDX() calculates the index into this array for a given size.  M_SNUM
   is the size (in small blocks) of such blocks.  M_SLEN() calculates the
   size of the small blocks held in a memory block, given a pointer to the
   header of it.  M_SBLEN() gives the size of a memory block that can hold
   an array of small blocks, given the size of these small blocks.  M_BSLEN()
   calculates the size of the small blocks held in a memory block, given the
   length of that block (including the header of the memory block.  M_NSMALL
   is the number of possible block sizes that small blocks should be used
   for. */


#define M_SIDX(S)  ((S) / M_ISIZE)
#define M_SNUM     128
#define M_SLEN(M)  ((M)->len / M_SNUM)
#if defined(PAD_64_BIT) && !defined(ZSH_64_BIT_TYPE)
/* Include the dummy in the alignment */
#define M_SBLEN(S) ((S) * M_SNUM + sizeof(struct m_shdr *) +  \
		    2*sizeof(zlong) + sizeof(struct m_hdr *))
#define M_BSLEN(S) (((S) - sizeof(struct m_shdr *) -  \
		     2*sizeof(zlong) - sizeof(struct m_hdr *)) / M_SNUM)
#else
#define M_SBLEN(S) ((S) * M_SNUM + sizeof(struct m_shdr *) +  \
		    sizeof(zlong) + sizeof(struct m_hdr *))
#define M_BSLEN(S) (((S) - sizeof(struct m_shdr *) -  \
		     sizeof(zlong) - sizeof(struct m_hdr *)) / M_SNUM)
#endif
#define M_NSMALL    8

static struct m_hdr *m_small[M_NSMALL];

#ifdef ZSH_MEM_DEBUG

static int m_s = 0, m_b = 0;
static int m_m[1025], m_f[1025];

static struct m_hdr *m_l;

#endif /* ZSH_MEM_DEBUG */

void *
malloc(size_t size)
{
    struct m_hdr *m, *mp, *mt;
    long n, s, os = 0;
#ifndef USE_MMAP
    struct heap *h, *hp, *hf = NULL, *hfp = NULL;
#endif

    /* some systems want malloc to return the highest valid address plus one
       if it is called with an argument of zero.
    
       TODO: really?  Suppose we allocate more memory, so
       that this is now in bounds, then a more rational application
       that thinks it can free() anything it malloc'ed, even
       of zero length, calls free for it?  Aren't we in big
       trouble?  Wouldn't it be safer just to allocate some
       memory anyway?

       If the above comment is really correct, then at least
       we need to check in free() if we're freeing memory
       at m_high.
    */

    if (!size)
#if 1
	size = 1;
#else
	return (void *) m_high;
#endif

    queue_signals();  /* just queue signals rather than handling them */

    /* first call, get page size */

    if (!m_pgsz) {

#ifdef _SC_PAGESIZE
	m_pgsz = sysconf(_SC_PAGESIZE);     /* SVR4 */
#else
# ifdef _SC_PAGE_SIZE
	m_pgsz = sysconf(_SC_PAGE_SIZE);    /* HPUX */
# else
	m_pgsz = getpagesize();
# endif
#endif

	m_free = m_lfree = NULL;
    }
    size = (size + M_ALIGN - 1) & ~(M_ALIGN - 1);

    /* Do we need a small block? */

    if ((s = M_SIDX(size)) && s < M_NSMALL) {
	/* yep, find a memory block with free small blocks of the
	   appropriate size (if we find it in this list, this means that
	   it has room for at least one more small block) */
	for (mp = NULL, m = m_small[s]; m && !m->free; mp = m, m = m->next);

	if (m) {
	    /* we found one */
	    struct m_shdr *sh = m->free;

	    m->free = sh->next;
	    m->used++;

	    /* if all small blocks in this block are allocated, the block is 
	       put at the end of the list blocks with small blocks of this
	       size (i.e., we try to keep blocks with free blocks at the
	       beginning of the list, to make the search faster) */

	    if (m->used == M_SNUM && m->next) {
		for (mt = m; mt->next; mt = mt->next);

		mt->next = m;
		if (mp)
		    mp->next = m->next;
		else
		    m_small[s] = m->next;
		m->next = NULL;
	    }
#ifdef ZSH_MEM_DEBUG
	    m_m[size / M_ISIZE]++;
#endif

	    unqueue_signals();
	    return (void *) sh;
	}
	/* we still want a small block but there were no block with a free
	   small block of the requested size; so we use the real allocation
	   routine to allocate a block for small blocks of this size */
	os = size;
	size = M_SBLEN(size);
    } else
	s = 0;

    /* search the free list for an block of at least the requested size */
    for (mp = NULL, m = m_free; m && m->len < size; mp = m, m = m->next);

#ifndef USE_MMAP

    /* if there is an empty zsh heap at a lower address we steal it and take
       the memory from it, putting the rest on the free list (remember
       that the blocks on the free list are ordered) */

    for (hp = NULL, h = heaps; h; hp = h, h = h->next)
	if (!h->used &&
	    (!hf || h < hf) &&
	    (!m || ((char *)m) > ((char *)h)))
	    hf = h, hfp = hp;

    if (hf) {
	/* we found such a heap */
	Heapstack hso, hsn;

	/* delete structures on the list holding the heap states */
	for (hso = hf->sp; hso; hso = hsn) {
	    hsn = hso->next;
	    zfree(hso, sizeof(*hso));
	}
	/* take it from the list of heaps */
	if (hfp)
	    hfp->next = hf->next;
	else
	    heaps = hf->next;
	/* now we simply free it and than search the free list again */
	zfree(hf, HEAPSIZE);

	for (mp = NULL, m = m_free; m && m->len < size; mp = m, m = m->next);
    }
#endif
    if (!m) {
	long nal;
	/* no matching free block was found, we have to request new
	   memory from the system */
	n = (size + M_HSIZE + M_ALLOC + m_pgsz - 1) & ~(m_pgsz - 1);

	if (((char *)(m = (struct m_hdr *)sbrk(n))) == ((char *)-1)) {
	    DPUTS1(1, "MEM: allocation error at sbrk, size %L.", n);
	    unqueue_signals();
	    return NULL;
	}
	if ((nal = ((long)(char *)m) & (M_ALIGN-1))) {
	    if ((char *)sbrk(M_ALIGN - nal) == (char *)-1) {
		DPUTS(1, "MEM: allocation error at sbrk.");
		unqueue_signals();
		return NULL;
	    }
	    m = (struct m_hdr *) ((char *)m + (M_ALIGN - nal));
	}
	/* set m_low, for the check in free() */
	if (!m_low)
	    m_low = (char *)m;

#ifdef ZSH_MEM_DEBUG
	m_s += n;

	if (!m_l)
	    m_l = m;
#endif

	/* save new highest address */
	m_high = ((char *)m) + n;

	/* initialize header */
	m->len = n - M_ISIZE;
	m->next = NULL;

	/* put it on the free list and set m_lfree pointing to it */
	if ((mp = m_lfree))
	    m_lfree->next = m;
	m_lfree = m;
    }
    if ((n = m->len - size) > M_MIN) {
	/* the block we want to use has more than M_MIN bytes plus the
	   number of bytes that were requested; we split it in two and
	   leave the rest on the free list */
	struct m_hdr *mtt = (struct m_hdr *)(((char *)m) + M_ISIZE + size);

	mtt->len = n - M_ISIZE;
	mtt->next = m->next;

	m->len = size;

	/* put the rest on the list */
	if (m_lfree == m)
	    m_lfree = mtt;

	if (mp)
	    mp->next = mtt;
	else
	    m_free = mtt;
    } else if (mp) {
	/* the block we found wasn't the first one on the free list */
	if (m == m_lfree)
	    m_lfree = mp;
	mp->next = m->next;
    } else {
	/* it was the first one */
	m_free = m->next;
	if (m == m_lfree)
	    m_lfree = m_free;
    }

    if (s) {
	/* we are allocating a block that should hold small blocks */
	struct m_shdr *sh, *shn;

	/* build the free list in this block and set `used' filed */
	m->free = sh = (struct m_shdr *)(((char *)m) +
					 sizeof(struct m_hdr) + os);

	for (n = M_SNUM - 2; n--; sh = shn)
	    shn = sh->next = sh + s;
	sh->next = NULL;

	m->used = 1;

	/* put the block on the list of blocks holding small blocks if
	   this size */
	m->next = m_small[s];
	m_small[s] = m;

#ifdef ZSH_MEM_DEBUG
	m_m[os / M_ISIZE]++;
#endif

	unqueue_signals();
	return (void *) (((char *)m) + sizeof(struct m_hdr));
    }
#ifdef ZSH_MEM_DEBUG
    m_m[m->len < (1024 * M_ISIZE) ? (m->len / M_ISIZE) : 1024]++;
#endif

    unqueue_signals();
    return (void *) & m->next;
}

/* this is an internal free(); the second argument may, but need not hold
   the size of the block the first argument is pointing to; if it is the
   right size of this block, freeing it will be faster, though; the value
   0 for this parameter means: `don't know' */

/**/
mod_export void
zfree(void *p, int sz)
{
    struct m_hdr *m = (struct m_hdr *)(((char *)p) - M_ISIZE), *mp, *mt = NULL;
    int i;
# ifdef DEBUG
    int osz = sz;
# endif

#ifdef ZSH_SECURE_FREE
    sz = 0;
#else
    sz = (sz + M_ALIGN - 1) & ~(M_ALIGN - 1);
#endif

    if (!p)
	return;

    /* first a simple check if the given address is valid */
    if (((char *)p) < m_low || ((char *)p) > m_high ||
	((long)p) & (M_ALIGN - 1)) {
	DPUTS(1, "BUG: attempt to free storage at invalid address");
	return;
    }

    queue_signals();

  fr_rec:

    if ((i = sz / M_ISIZE) < M_NSMALL || !sz)
	/* if the given sizes says that it is a small block, find the
	   memory block holding it; we search all blocks with blocks
	   of at least the given size; if the size parameter is zero,
	   this means, that all blocks are searched */
	for (; i < M_NSMALL; i++) {
	    for (mp = NULL, mt = m_small[i];
		 mt && (((char *)mt) > ((char *)p) ||
			(((char *)mt) + mt->len) < ((char *)p));
		 mp = mt, mt = mt->next);

	    if (mt) {
		/* we found the block holding the small block */
		struct m_shdr *sh = (struct m_shdr *)p;

#ifdef ZSH_SECURE_FREE
		struct m_shdr *sh2;

		/* check if the given address is equal to the address of
		   the first small block plus an integer multiple of the
		   block size */
		if ((((char *)p) - (((char *)mt) + sizeof(struct m_hdr))) %
		    M_BSLEN(mt->len)) {

		    DPUTS(1, "BUG: attempt to free storage at invalid address");
		    unqueue_signals();
		    return;
		}
		/* check, if the address is on the (block-intern) free list */
		for (sh2 = mt->free; sh2; sh2 = sh2->next)
		    if (((char *)p) == ((char *)sh2)) {

			DPUTS(1, "BUG: attempt to free already free storage");
			unqueue_signals();
			return;
		    }
#endif
		DPUTS(M_BSLEN(mt->len) < osz,
		      "BUG: attempt to free more than allocated.");

#ifdef ZSH_MEM_DEBUG
		m_f[M_BSLEN(mt->len) / M_ISIZE]++;
		memset(sh, 0xff, M_BSLEN(mt->len));
#endif

		/* put the block onto the free list */
		sh->next = mt->free;
		mt->free = sh;

		if (--mt->used) {
		    /* if there are still used blocks in this block, we
		       put it at the beginning of the list with blocks
		       holding small blocks of the same size (since we
		       know that there is at least one free block in it,
		       this will make allocation of small blocks faster;
		       it also guarantees that long living memory blocks
		       are preferred over younger ones */
		    if (mp) {
			mp->next = mt->next;
			mt->next = m_small[i];
			m_small[i] = mt;
		    }
		    unqueue_signals();
		    return;
		}
		/* if there are no more used small blocks in this
		   block, we free the whole block */
		if (mp)
		    mp->next = mt->next;
		else
		    m_small[i] = mt->next;

		m = mt;
		p = (void *) & m->next;

		break;
	    } else if (sz) {
		/* if we didn't find a block and a size was given, try it
		   again as if no size were given */
		sz = 0;
		goto fr_rec;
	    }
	}
#ifdef ZSH_MEM_DEBUG
    if (!mt)
	m_f[m->len < (1024 * M_ISIZE) ? (m->len / M_ISIZE) : 1024]++;
#endif

#ifdef ZSH_SECURE_FREE
    /* search all memory blocks, if one of them is at the given address */
    for (mt = (struct m_hdr *)m_low;
	 ((char *)mt) < m_high;
	 mt = (struct m_hdr *)(((char *)mt) + M_ISIZE + mt->len))
	if (((char *)p) == ((char *)&mt->next))
	    break;

    /* no block was found at the given address */
    if (((char *)mt) >= m_high) {
	DPUTS(1, "BUG: attempt to free storage at invalid address");
	unqueue_signals();
	return;
    }
#endif

    /* see if the block is on the free list */
    for (mp = NULL, mt = m_free; mt && mt < m; mp = mt, mt = mt->next);

    if (m == mt) {
	/* it is, ouch! */
	DPUTS(1, "BUG: attempt to free already free storage");
	unqueue_signals();
	return;
    }
    DPUTS(m->len < osz, "BUG: attempt to free more than allocated");
#ifdef ZSH_MEM_DEBUG
    memset(p, 0xff, m->len);
#endif
    if (mt && ((char *)mt) == (((char *)m) + M_ISIZE + m->len)) {
	/* the block after the one we are freeing is free, we put them
	   together */
	m->len += mt->len + M_ISIZE;
	m->next = mt->next;

	if (mt == m_lfree)
	    m_lfree = m;
    } else
	m->next = mt;

    if (mp && ((char *)m) == (((char *)mp) + M_ISIZE + mp->len)) {
	/* the block before the one we are freeing is free, we put them
	   together */
	mp->len += m->len + M_ISIZE;
	mp->next = m->next;

	if (m == m_lfree)
	    m_lfree = mp;
    } else if (mp)
	/* otherwise, we just put it on the free list */
	mp->next = m;
    else {
	m_free = m;
	if (!m_lfree)
	    m_lfree = m_free;
    }

    /* if the block we have just freed was at the end of the process heap
       and now there is more than one page size of memory, we can give
       it back to the system (and we do it ;-) */
    if ((((char *)m_lfree) + M_ISIZE + m_lfree->len) == m_high &&
	m_lfree->len >= m_pgsz + M_MIN + M_FREE) {
	long n = (m_lfree->len - M_MIN - M_KEEP) & ~(m_pgsz - 1);

	m_lfree->len -= n;
#ifdef HAVE_BRK
	if (brk(m_high -= n) == -1) {
#else
	m_high -= n;
	if (sbrk(-n) == (void *)-1) {
#endif /* HAVE_BRK */
	    DPUTS(1, "MEM: allocation error at brk.");
	}

#ifdef ZSH_MEM_DEBUG
	m_b += n;
#endif
    }
    unqueue_signals();
}

void
free(void *p)
{
    zfree(p, 0);		/* 0 means: size is unknown */
}

/* this one is for strings (and only strings, real strings, real C strings,
   those that have a zero byte at the end) */

/**/
mod_export void
zsfree(char *p)
{
    if (p)
	zfree(p, strlen(p) + 1);
}

void *
realloc(void *p, size_t size)
{
    struct m_hdr *m = (struct m_hdr *)(((char *)p) - M_ISIZE), *mt;
    char *r;
    int i, l = 0;

    /* some system..., see above */
    if (!p && size) {
	queue_signals();
	r = malloc(size);
	unqueue_signals();
	return (void *) r;
    }

    /* and some systems even do this... */
    if (!p || !size)
	return p;

    queue_signals();  /* just queue signals caught rather than handling them */

    /* check if we are reallocating a small block, if we do, we have
       to compute the size of the block from the sort of block it is in */
    for (i = 0; i < M_NSMALL; i++) {
	for (mt = m_small[i];
	     mt && (((char *)mt) > ((char *)p) ||
		    (((char *)mt) + mt->len) < ((char *)p));
	     mt = mt->next);

	if (mt) {
	    l = M_BSLEN(mt->len);
	    break;
	}
    }
    if (!l)
	/* otherwise the size of the block is in the memory just before
	   the given address */
	l = m->len;

    /* now allocate the new block, copy the old contents, and free the
       old block */
    r = malloc(size);
    memcpy(r, (char *)p, (size > l) ? l : size);
    free(p);

    unqueue_signals();
    return (void *) r;
}

void *
calloc(size_t n, size_t size)
{
    long l;
    char *r;

    if (!(l = n * size))
	return (void *) m_high;

    /*
     * use realloc() (with a NULL `p` argument it behaves exactly the same
     * as malloc() does) to prevent an infinite loop caused by sibling-call
     * optimizations (the malloc() call would otherwise be replaced by an
     * unconditional branch back to line 1719 ad infinitum).
     */
    r = realloc(NULL, l);

    memset(r, 0, l);

    return (void *) r;
}

#ifdef ZSH_MEM_DEBUG

/**/
int
bin_mem(char *name, char **argv, Options ops, int func)
{
    int i, ii, fi, ui, j;
    struct m_hdr *m, *mf, *ms;
    char *b, *c, buf[40];
    long u = 0, f = 0, to, cu;

    queue_signals();
    if (OPT_ISSET(ops,'v')) {
	printf("The lower and the upper addresses of the heap. Diff gives\n");
	printf("the difference between them, i.e. the size of the heap.\n\n");
    }
    printf("low mem %ld\t high mem %ld\t diff %ld\n",
	   (long)m_l, (long)m_high, (long)(m_high - ((char *)m_l)));

    if (OPT_ISSET(ops,'v')) {
	printf("\nThe number of bytes that were allocated using sbrk() and\n");
	printf("the number of bytes that were given back to the system\n");
	printf("via brk().\n");
    }
    printf("\nsbrk %d\tbrk %d\n", m_s, m_b);

    if (OPT_ISSET(ops,'v')) {
	printf("\nInformation about the sizes that were allocated or freed.\n");
	printf("For each size that were used the number of mallocs and\n");
	printf("frees is shown. Diff gives the difference between these\n");
	printf("values, i.e. the number of blocks of that size that is\n");
	printf("currently allocated. Total is the product of size and diff,\n");
	printf("i.e. the number of bytes that are allocated for blocks of\n");
	printf("this size. The last field gives the accumulated number of\n");
	printf("bytes for all sizes.\n");
    }
    printf("\nsize\tmalloc\tfree\tdiff\ttotal\tcum\n");
    for (i = 0, cu = 0; i < 1024; i++)
	if (m_m[i] || m_f[i]) {
	    to = (long) i * M_ISIZE * (m_m[i] - m_f[i]);
	    printf("%ld\t%d\t%d\t%d\t%ld\t%ld\n",
		   (long)i * M_ISIZE, m_m[i], m_f[i], m_m[i] - m_f[i],
		   to, (cu += to));
	}

    if (m_m[i] || m_f[i])
	printf("big\t%d\t%d\t%d\n", m_m[i], m_f[i], m_m[i] - m_f[i]);

    if (OPT_ISSET(ops,'v')) {
	printf("\nThe list of memory blocks. For each block the following\n");
	printf("information is shown:\n\n");
	printf("num\tthe number of this block\n");
	printf("tnum\tlike num but counted separately for used and free\n");
	printf("\tblocks\n");
	printf("addr\tthe address of this block\n");
	printf("len\tthe length of the block\n");
	printf("state\tthe state of this block, this can be:\n");
	printf("\t  used\tthis block is used for one big block\n");
	printf("\t  free\tthis block is free\n");
	printf("\t  small\tthis block is used for an array of small blocks\n");
	printf("cum\tthe accumulated sizes of the blocks, counted\n");
	printf("\tseparately for used and free blocks\n");
	printf("\nFor blocks holding small blocks the number of free\n");
	printf("blocks, the number of used blocks and the size of the\n");
	printf("blocks is shown. For otherwise used blocks the first few\n");
	printf("bytes are shown as an ASCII dump.\n");
    }
    printf("\nblock list:\nnum\ttnum\taddr\t\tlen\tstate\tcum\n");
    for (m = m_l, mf = m_free, ii = fi = ui = 1; ((char *)m) < m_high;
	 m = (struct m_hdr *)(((char *)m) + M_ISIZE + m->len), ii++) {
	for (j = 0, ms = NULL; j < M_NSMALL && !ms; j++)
	    for (ms = m_small[j]; ms; ms = ms->next)
		if (ms == m)
		    break;

	if (m == mf)
	    buf[0] = '\0';
	else if (m == ms)
	    sprintf(buf, "%ld %ld %ld", (long)(M_SNUM - ms->used),
		    (long)ms->used,
		    (long)(m->len - sizeof(struct m_hdr)) / M_SNUM + 1);

	else {
	    for (i = 0, b = buf, c = (char *)&m->next; i < 20 && i < m->len;
		 i++, c++)
		*b++ = (*c >= ' ' && *c < 127) ? *c : '.';
	    *b = '\0';
	}

	printf("%d\t%d\t%ld\t%ld\t%s\t%ld\t%s\n", ii,
	       (m == mf) ? fi++ : ui++,
	       (long)m, (long)m->len,
	       (m == mf) ? "free" : ((m == ms) ? "small" : "used"),
	       (m == mf) ? (f += m->len) : (u += m->len),
	       buf);

	if (m == mf)
	    mf = mf->next;
    }

    if (OPT_ISSET(ops,'v')) {
	printf("\nHere is some information about the small blocks used.\n");
	printf("For each size the arrays with the number of free and the\n");
	printf("number of used blocks are shown.\n");
    }
    printf("\nsmall blocks:\nsize\tblocks (free/used)\n");

    for (i = 0; i < M_NSMALL; i++)
	if (m_small[i]) {
	    printf("%ld\t", (long)i * M_ISIZE);

	    for (ii = 0, m = m_small[i]; m; m = m->next) {
		printf("(%ld/%ld) ", (long)(M_SNUM - m->used),
		       (long)m->used);
		if (!((++ii) & 7))
		    printf("\n\t");
	    }
	    putchar('\n');
	}
    if (OPT_ISSET(ops,'v')) {
	printf("\n\nBelow is some information about the allocation\n");
	printf("behaviour of the zsh heaps. First the number of times\n");
	printf("pushheap(), popheap(), and freeheap() were called.\n");
    }
    printf("\nzsh heaps:\n\n");

    printf("push %d\tpop %d\tfree %d\n\n", h_push, h_pop, h_free);

    if (OPT_ISSET(ops,'v')) {
	printf("\nThe next list shows for several sizes the number of times\n");
	printf("memory of this size were taken from heaps.\n\n");
    }
    printf("size\tmalloc\ttotal\n");
    for (i = 0; i < 1024; i++)
	if (h_m[i])
	    printf("%ld\t%d\t%ld\n", (long)i * H_ISIZE, h_m[i],
		   (long)i * H_ISIZE * h_m[i]);
    if (h_m[1024])
	printf("big\t%d\n", h_m[1024]);

    unqueue_signals();
    return 0;
}

#endif

/**/
#else				/* not ZSH_MEM */

/**/
mod_export void
zfree(void *p, UNUSED(int sz))
{
    free(p);
}

/**/
mod_export void
zsfree(char *p)
{
    free(p);
}

/**/
#endif
