/*
 * Copyright 2020 Axel Davy <davyaxel0@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

/*
 * Memory util function to allocate RAM backing for textures.
 * DEFAULT textures are stored on GPU
 * MANAGED textures have a RAM backing and upload the content to a GPU texture for use
 * SYSTEMMEM textures are stored in RAM and are meant to be uploaded to DEFAULT textures.
 * Basically SYSTEMMEM + DEFAULT enables to do manually what MANAGED does automatically.
 *
 * Once the GPU texture is created, the RAM backing of MANAGED textures can be used in
 * two occasions:
 * . Recreating the GPU texture (for example lod change, or GPU memory eviction)
 * . Reading the texture content (some games do that to fill higher res versions of the texture)
 *
 * When a lot of textures are used, the amount of addressing space (virtual memory) taken by MANAGED
 * and SYSTEMMEM textures can be significant and cause virtual memory exhaustion for 32 bits programs.
 *
 * One way to reduce the virtual memory taken is to ignore lod and delete the RAM backing of
 * MANAGED textures once it is uploaded. If the texture is read, or evicted from GPU memory, the RAM
 * backing would be recreated (Note that mapping the GPU memory is not acceptable as RAM memory is supposed
 * to have smaller (fixed) stride constraints).
 *
 * Instead the approach taken here is to keep the RAM backing alive, but free its addressing space.
 * In other words virtual memory usage is reduced, but the RAM usage of the app is the same.
 * To do so, we use the memfd feature of the linux kernel. It enables to allocate a file
 * stored in RAM and visible only to the app. We can map/unmap portions of the file as we need.
 * When a portion is mapped, it takes virtual memory space. When it is not, it doesn't.
 * The file is stored in RAM, and thus the access speed is the same as normal RAM. Using such
 * file to allocate data enables to use more than 4GB RAM on 32 bits.
 *
 * This approach adds some overhead: when accessing mapped content the first time, pages are allocated
 * by the system. This has a lot of overhead (several times the time to memset the area).
 * Releasing these pages (when unmapping) has overhead too, though significantly less.
 *
 * This overhead however is much less significant than the overhead of downloading the GPU content.
 * In addition, we reduce significantly the overhead spent in Gallium nine for new allocations by
 * using the fact new contents of the file are zero-allocated. By not calling memset in Gallium nine,
 * the overhead of page allocation happens client side, thus outside the d3d mutex. This should give
 * a performance boost for multithreaded applications. As malloc also has this overhead (at least for
 * large enough allocations which use mmap internally), allocating ends up faster than with the standard
 * allocation path.
 * By far the overhead induced by page allocation/deallocation is the biggest overhead involved in this
 * code. It is reduced significantly with huge pages, but it is too complex to configure for the user
 * to use it (and it has some memory management downsides too). The memset trick enables to move most of
 * the overhead outside Nine anyway.
 *
 * To prevent useless unmappings quickly followed by mapping again, we do not unmap right away allocations
 * that are not locked for access anymore. Indeed it is likely the allocation will be accessed several times
 * in a row, for example first to fill it, then to upload it.
 * We keep everything mapped until we reach a threshold of memory allocated. Then we use hints to prioritize
 * which regions to unmap first. Thus virtual memory usage is only reduced when the threshold is reached.
 *
 * Multiple memfd files are used, each of 100MB. Thus memory usage (but not virtual memory usage) increases
 * by amounts of 100MB. When not on x86 32 bits, we do use the standard malloc.
 *
 * Finally, for ease of use, we do not implement packing of allocation inside page-aligned regions.
 * One allocation is given one page-aligned region inside a memfd file.
 * Allocations smaller than a page (4KB on x86) go through malloc.
 * As texture sizes are usually multiples of powers of two, allocations above the page size are typically
 * multiples of the page size, thus space is not wasted in practice.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/memfd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ulimit.h>
#include <unistd.h>

#include "util/list.h"
#include "util/u_memory.h"
#include "util/slab.h"

#include "nine_debug.h"
#include "nine_memory_helper.h"
#include "nine_state.h"


#define DIVUP(a,b) (((a)+(b)-1)/(b))

/* Required alignment for allocations */
#define NINE_ALLOCATION_ALIGNMENT 32

#define DBG_CHANNEL (DBG_BASETEXTURE|DBG_SURFACE|DBG_VOLUME|DBG_TEXTURE|DBG_CUBETEXTURE)

/* Use memfd only for 32 bits. Check for memfd_create support */
#if DETECT_ARCH_X86 && defined(HAVE_MEMFD_CREATE)
#define NINE_ENABLE_MEMFD
#endif

#ifdef NINE_ENABLE_MEMFD

struct nine_memfd_file_region {
    unsigned offset;
    unsigned size;
    void *map; /* pointer to the mapped content of the file. Can be NULL */
    int num_locks; /* Total number of locks blocking the munmap */
    int num_weak_unlocks; /* Number of users which weakly block the munmap */
    bool zero_filled;
    struct list_head list;
};

struct nine_memfd_file {
    int fd;
    int filesize; /* Size of the file */
    struct list_head free_regions; /* This list is sorted by the offset, and consecutive regions are merged */
    struct list_head unmapped_allocated_regions; /* This list and the following ones are not sorted */
    struct list_head locked_mapped_allocated_regions;
    struct list_head weak_unlocked_mapped_allocated_regions;
    struct list_head unlocked_mapped_allocated_regions;
};

/* The allocation is stored inside a memfd */
#define NINE_MEMFD_ALLOC 1
/* The allocation is part of another allocation, which is stored inside a memfd */
#define NINE_MEMFD_SUBALLOC 2
/* The allocation was allocated with malloc and will have to be freed */
#define NINE_MALLOC_ALLOC 3
/* The pointer doesn't need memory management */
#define NINE_EXTERNAL_ALLOC 4

struct nine_memfd_allocation {
    struct nine_memfd_file *file; /* File in which the data is allocated */
    struct nine_memfd_file_region *region; /* Corresponding file memory region. Max 1 allocation per region */
};

/* 'Suballocations' are used to represent subregions of an allocation.
 * For example a given layer of a texture. These are not allocations,
 * but can be accessed separately. To correctly handle accessing them,
 * we encapsulate them into this structure. */
struct nine_memfd_suballocation {
    struct nine_memfd_allocation *parent; /* Parent allocation */
    int relative_offset; /* Offset relative to the parent */
};

/* A standard allocation with malloc */
struct nine_malloc_allocation {
    void *buf;
    unsigned allocation_size;
};

/* A pointer with no need of memory management.
 * For example a pointer passed by the application,
 * or a 'suballocation' inside a malloc-ed allocation. */
struct nine_external_allocation {
    void *buf;
};

/* Encapsulates all allocations */
struct nine_allocation {
    unsigned allocation_type; /* Type of allocation */
    union {
        struct nine_memfd_allocation memfd;
        struct nine_memfd_suballocation submemfd;
        struct nine_malloc_allocation malloc;
        struct nine_external_allocation external;
    } memory;
    struct list_head list_free; /* for pending frees */
    /* The fields below are only used for memfd/submemfd allocations */
    struct list_head list_release; /* for pending releases */
    /* Handling of the CSMT thread:
     * API calls are singled thread (global mutex protection).
     * However we multithreading internally (CSMT worker thread).
     * To handle this thread, we map/lock the allocation in the
     * main thread and increase pending_counter. When the worker thread
     * is done with the scheduled function, the pending_counter is decreased.
     * If pending_counter is 0, locks_on_counter can be subtracted from
     * active_locks (in the main thread). */
    unsigned locks_on_counter;
    unsigned *pending_counter;
    /* Hint from the last unlock indicating the data might be locked again soon */
    bool weak_unlock; 
};

struct nine_allocator {
    struct NineDevice9 *device;
    int page_size; /* Page size */
    int num_fd_max; /* Max number of memfd files */
    int min_file_size; /* Minimum memfd file size */
    /* Tracking of all allocations */
    long long total_allocations; /* Amount of memory allocated */
    long long total_locked_memory; /* TODO */ /* Amount of memory blocked by a lock */
    long long total_virtual_memory; /* Current virtual memory used by our allocations */
    long long total_virtual_memory_limit; /* Target maximum virtual memory used. Above that, tries to unmap memfd files whenever possible. */

    int num_fd; /* Number of memfd files */ /* TODO release unused memfd files */
    struct slab_mempool allocation_pool;
    struct slab_mempool region_pool;
    struct nine_memfd_file *memfd_pool; /* Table (of size num_fd) of memfd files */
    struct list_head pending_releases; /* List of allocations with unlocks depending on pending_counter */ /* TODO: Elements seem removed only on flush. Destruction ? */

    pthread_mutex_t mutex_pending_frees;
    struct list_head pending_frees;
};

#ifdef DEBUG

static void
debug_dump_memfd_state(struct nine_memfd_file *memfd_file, bool details)
{
    struct nine_memfd_file_region *region;

    DBG("fd: %d, filesize: %d\n", memfd_file->fd, memfd_file->filesize);
    if (!details)
        return;
    LIST_FOR_EACH_ENTRY(region, &memfd_file->free_regions, list) {
        DBG("FREE block: offset %d, size %d, map=%p, locks=%d, weak=%d, z=%d\n",
            region->offset, region->size, region->map,
        region->num_locks, region->num_weak_unlocks, (int)region->zero_filled);
    }
    LIST_FOR_EACH_ENTRY(region, &memfd_file->unmapped_allocated_regions, list) {
        DBG("UNMAPPED ALLOCATED block: offset %d, size %d, map=%p, locks=%d, weak=%d, z=%d\n",
            region->offset, region->size, region->map,
        region->num_locks, region->num_weak_unlocks, (int)region->zero_filled);
    }
    LIST_FOR_EACH_ENTRY(region, &memfd_file->locked_mapped_allocated_regions, list) {
        DBG("LOCKED MAPPED ALLOCATED block: offset %d, size %d, map=%p, locks=%d, weak=%d, z=%d\n",
            region->offset, region->size, region->map,
        region->num_locks, region->num_weak_unlocks, (int)region->zero_filled);
    }
    LIST_FOR_EACH_ENTRY(region, &memfd_file->unlocked_mapped_allocated_regions, list) {
        DBG("UNLOCKED MAPPED ALLOCATED block: offset %d, size %d, map=%p, locks=%d, weak=%d, z=%d\n",
            region->offset, region->size, region->map,
        region->num_locks, region->num_weak_unlocks, (int)region->zero_filled);
    }
    LIST_FOR_EACH_ENTRY(region, &memfd_file->weak_unlocked_mapped_allocated_regions, list) {
        DBG("WEAK UNLOCKED MAPPED ALLOCATED block: offset %d, size %d, map=%p, locks=%d, weak=%d, z=%d\n",
            region->offset, region->size, region->map,
        region->num_locks, region->num_weak_unlocks, (int)region->zero_filled);
    }
}

static void
debug_dump_allocation_state(struct nine_allocation *allocation)
{
    switch(allocation->allocation_type) {
        case NINE_MEMFD_ALLOC:
            DBG("Allocation is stored in this memfd file:\n");
            debug_dump_memfd_state(allocation->memory.memfd.file, true);
            DBG("Allocation is offset: %d, size: %d\n",
                allocation->memory.memfd.region->offset, allocation->memory.memfd.region->size);
            break;
        case NINE_MEMFD_SUBALLOC:
            DBG("Allocation is suballocation at relative offset %d of this allocation:\n",
                allocation->memory.submemfd.relative_offset);
            DBG("Parent allocation is stored in this memfd file:\n");
            debug_dump_memfd_state(allocation->memory.submemfd.parent->file, false);
            DBG("Parent allocation is offset: %d, size: %d\n",
                allocation->memory.submemfd.parent->region->offset,
                allocation->memory.submemfd.parent->region->size);
            break;
        case NINE_MALLOC_ALLOC:
            DBG("Allocation is a standard malloc\n");
            break;
        case NINE_EXTERNAL_ALLOC:
            DBG("Allocation is a suballocation of a standard malloc or an external allocation\n");
            break;
        default:
            assert(false);
    }
}

#else

static void
debug_dump_memfd_state(struct nine_memfd_file *memfd_file, bool details)
{
    (void)memfd_file;
    (void)details;
}

static void
debug_dump_allocation_state(struct nine_allocation *allocation)
{
   (void)allocation;
}

#endif

static void
debug_dump_allocator_state(struct nine_allocator *allocator)
{
    DBG("SURFACE ALLOCATOR STATUS:\n");
    DBG("Total allocated: %lld\n", allocator->total_allocations);
    DBG("Total virtual memory locked: %lld\n", allocator->total_locked_memory);
    DBG("Virtual memory used: %lld / %lld\n", allocator->total_virtual_memory, allocator->total_virtual_memory_limit);
    DBG("Num memfd files: %d / %d\n", allocator->num_fd, allocator->num_fd_max);
}


/* Retrieve file used for the storage of the content of this allocation.
 * NULL if not using memfd */
static struct nine_memfd_file *
nine_get_memfd_file_backing(struct nine_allocation *allocation)
{
    if (allocation->allocation_type > NINE_MEMFD_SUBALLOC)
        return NULL;
    if (allocation->allocation_type == NINE_MEMFD_ALLOC)
        return allocation->memory.memfd.file;
    return allocation->memory.submemfd.parent->file;
}

/* Retrieve region used for the storage of the content of this allocation.
 * NULL if not using memfd */
static struct nine_memfd_file_region *
nine_get_memfd_region_backing(struct nine_allocation *allocation)
{
    if (allocation->allocation_type > NINE_MEMFD_SUBALLOC)
        return NULL;
    if (allocation->allocation_type == NINE_MEMFD_ALLOC)
        return allocation->memory.memfd.region;
    return allocation->memory.submemfd.parent->region;
}

static void move_region(struct list_head *tail, struct nine_memfd_file_region *region)
{
    /* Remove from previous list (if any) */
    list_delinit(&region->list);
    /* Insert in new list (last) */
    list_addtail(&region->list, tail);
}

#if 0
static void move_region_ordered(struct list_head *tail, struct nine_memfd_file_region *region)
{
    struct nine_memfd_file_region *cur_region;
    struct list_head *insertion_point = tail;

    /* Remove from previous list (if any) */
    list_delinit(&region->list);

    LIST_FOR_EACH_ENTRY(cur_region, tail, list) {
        if (cur_region->offset > region->offset)
            break;
        insertion_point = &cur_region->list;
    }
    /* Insert just before cur_region */
    list_add(&region->list, insertion_point);
}
#endif

static void move_region_ordered_merge(struct nine_allocator *allocator, struct list_head *tail, struct nine_memfd_file_region *region)
{
    struct nine_memfd_file_region *p, *cur_region = NULL, *prev_region = NULL;

    /* Remove from previous list (if any) */
    list_delinit(&region->list);

    LIST_FOR_EACH_ENTRY(p, tail, list) {
        cur_region = p;
        if (cur_region->offset > region->offset)
            break;
        prev_region = cur_region;
    }

    /* Insert after prev_region and before cur_region. Try to merge */
    if (prev_region && ((prev_region->offset + prev_region->size) == region->offset)) {
        if (cur_region && (cur_region->offset == (region->offset + region->size))) {
            /* Merge all three regions */
            prev_region->size += region->size + cur_region->size;
            prev_region->zero_filled = prev_region->zero_filled && region->zero_filled && cur_region->zero_filled;
            list_del(&cur_region->list);
            slab_free_st(&allocator->region_pool, region);
            slab_free_st(&allocator->region_pool, cur_region);
        } else {
            prev_region->size += region->size;
            prev_region->zero_filled = prev_region->zero_filled && region->zero_filled;
            slab_free_st(&allocator->region_pool, region);
        }
    } else if (cur_region && (cur_region->offset == (region->offset + region->size))) {
        cur_region->offset = region->offset;
        cur_region->size += region->size;
        cur_region->zero_filled = region->zero_filled && cur_region->zero_filled;
        slab_free_st(&allocator->region_pool, region);
    } else {
        list_add(&region->list, prev_region ? &prev_region->list : tail);
    }
}

static struct nine_memfd_file_region *allocate_region(struct nine_allocator *allocator, unsigned offset, unsigned size) {
    struct nine_memfd_file_region *region = slab_alloc_st(&allocator->allocation_pool);
    if (!region)
        return NULL;
    region->offset = offset;
    region->size = size;
    region->num_locks = 0;
    region->num_weak_unlocks = 0;
    region->map = NULL;
    region->zero_filled = false;
    list_inithead(&region->list);
    return region;
}

/* Go through memfd allocated files, and try to use unused memory for the requested allocation.
 * Returns whether it suceeded */
static bool
insert_new_allocation(struct nine_allocator *allocator, struct nine_allocation *new_allocation, unsigned allocation_size)
{
    int memfd_index;
    struct nine_memfd_file *memfd_file, *best_memfd_file;
    struct nine_memfd_file_region *region, *best_region, *new_region;


    /* Find the smallest - but bigger than the requested size - unused memory
     * region inside the memfd files. */
    int min_blocksize = INT_MAX;

    for (memfd_index = 0; memfd_index < allocator->num_fd; memfd_index++) {
        memfd_file = (void*)allocator->memfd_pool + memfd_index*sizeof(struct nine_memfd_file);

        LIST_FOR_EACH_ENTRY(region, &memfd_file->free_regions, list) {
            if (region->size <= min_blocksize && region->size >= allocation_size) {
                min_blocksize = region->size;
                best_region = region;
                best_memfd_file = memfd_file;
            }
        }
        if (min_blocksize == allocation_size)
            break;
    }

    /* The allocation doesn't fit in any memfd file */
    if (min_blocksize == INT_MAX)
        return false;

    /* Target region found */
    /* Move from free to unmapped allocated */
    best_region->size = DIVUP(allocation_size, allocator->page_size) * allocator->page_size;
    assert(min_blocksize >= best_region->size);
    move_region(&best_memfd_file->unmapped_allocated_regions, best_region);
    new_allocation->memory.memfd.region = best_region;
    new_allocation->memory.memfd.file = best_memfd_file;

    /* If the original region is bigger than needed, add new region with remaining space */
    min_blocksize -= best_region->size;
    if (min_blocksize > 0) {
        new_region = allocate_region(allocator, best_region->offset + best_region->size, min_blocksize);
        new_region->zero_filled = best_region->zero_filled;
        move_region_ordered_merge(allocator, &best_memfd_file->free_regions, new_region);
    }
    allocator->total_allocations += best_region->size;
    return true;
}

/* Go through allocations with unlocks waiting on pending_counter being 0.
 * If 0 is indeed reached, update the allocation status */
static void
nine_flush_pending_releases(struct nine_allocator *allocator)
{
    struct nine_allocation *allocation, *ptr;
    LIST_FOR_EACH_ENTRY_SAFE(allocation, ptr, &allocator->pending_releases, list_release) {
        assert(allocation->locks_on_counter > 0);
        /* If pending_releases reached 0, remove from the list and update the status */
        if (*allocation->pending_counter == 0) {
            struct nine_memfd_file *memfd_file = nine_get_memfd_file_backing(allocation);
            struct nine_memfd_file_region *region = nine_get_memfd_region_backing(allocation);
            region->num_locks -= allocation->locks_on_counter;
            allocation->locks_on_counter = 0;
            list_delinit(&allocation->list_release);
            if (region->num_locks == 0) {
                /* Move to the correct list */
                if (region->num_weak_unlocks)
                    move_region(&memfd_file->weak_unlocked_mapped_allocated_regions, region);
                else
                    move_region(&memfd_file->unlocked_mapped_allocated_regions, region);
                allocator->total_locked_memory -= region->size;
            }
        }
    }
}

static void
nine_free_internal(struct nine_allocator *allocator, struct nine_allocation *allocation);

static void
nine_flush_pending_frees(struct nine_allocator *allocator)
{
    struct nine_allocation *allocation, *ptr;

    pthread_mutex_lock(&allocator->mutex_pending_frees);
    /* The order of release matters as suballocations are supposed to be released first */
    LIST_FOR_EACH_ENTRY_SAFE(allocation, ptr, &allocator->pending_frees, list_free) {
        /* Set the allocation in an unlocked state, and then free it */
        if (allocation->allocation_type == NINE_MEMFD_ALLOC ||
        allocation->allocation_type == NINE_MEMFD_SUBALLOC) {
            struct nine_memfd_file *memfd_file = nine_get_memfd_file_backing(allocation);
            struct nine_memfd_file_region *region = nine_get_memfd_region_backing(allocation);
            if (region->num_locks != 0) {
                region->num_locks = 0;
                allocator->total_locked_memory -= region->size;
                /* Useless, but to keep consistency */
                move_region(&memfd_file->unlocked_mapped_allocated_regions, region);
            }
            region->num_weak_unlocks = 0;
            allocation->weak_unlock = false;
            allocation->locks_on_counter = 0;
            list_delinit(&allocation->list_release);
        }
        list_delinit(&allocation->list_free);
        nine_free_internal(allocator, allocation);
    }
    pthread_mutex_unlock(&allocator->mutex_pending_frees);
}

/* Try to unmap the memfd_index-th file if not already unmapped.
 * If even_if_weak is False, will not unmap if there are weak unlocks */
static void
nine_memfd_unmap_region(struct nine_allocator *allocator,
                            struct nine_memfd_file *memfd_file,
                            struct nine_memfd_file_region *region)
{
    DBG("Unmapping memfd mapped region at %d: size: %d, map=%p, locks=%d, weak=%d\n",
        region->offset,  region->size, region->map,
        region->num_locks, region->num_weak_unlocks);
    assert(region->map != NULL);

    if (munmap(region->map, region->size) != 0)
        fprintf(stderr, "Error on unmapping, errno=%d\n", (int)errno);

    region->map = NULL;
    /* Move from one of the mapped region list to the unmapped one */
    move_region(&memfd_file->unmapped_allocated_regions, region);
    allocator->total_virtual_memory -= region->size;
}

/* Unallocate a region of a memfd file */
static void
remove_allocation(struct nine_allocator *allocator, struct nine_memfd_file *memfd_file, struct nine_memfd_file_region *region)
{
    assert(region->num_locks == 0);
    region->num_weak_unlocks = 0;
    /* Move from mapped region to unmapped region */
    if (region->map) {
        if (likely(!region->zero_filled)) {
            /* As the region is mapped, it is likely the pages are allocated.
             * Do the memset now for when we allocate again. It is much faster now,
             * as the pages are allocated. */
            DBG("memset on data=%p, size %d\n", region->map, region->size);
            memset(region->map, 0, region->size);
            region->zero_filled = true;
        }
        nine_memfd_unmap_region(allocator, memfd_file, region);
    }
    /* Move from unmapped region to free region */
    allocator->total_allocations -= region->size;
    move_region_ordered_merge(allocator, &memfd_file->free_regions, region);
}

/* Try to unmap the regions of the memfd_index-th file if not already unmapped.
 * If even_if_weak is False, will not unmap if there are weak unlocks */
static void
nine_memfd_try_unmap_file(struct nine_allocator *allocator,
                          int memfd_index,
                          bool weak)
{
    struct nine_memfd_file *memfd_file = (void*)allocator->memfd_pool + memfd_index*sizeof(struct nine_memfd_file);
    struct nine_memfd_file_region *region, *ptr;
    DBG("memfd file at %d: fd: %d, filesize: %d\n",
        memfd_index, memfd_file->fd, memfd_file->filesize);
    debug_dump_memfd_state(memfd_file, true);
    LIST_FOR_EACH_ENTRY_SAFE(region, ptr,
                             weak ?
                                &memfd_file->weak_unlocked_mapped_allocated_regions :
                                &memfd_file->unlocked_mapped_allocated_regions,
                             list) {
        nine_memfd_unmap_region(allocator, memfd_file, region);
    }
}

/* Unmap files until we are below the virtual memory target limit.
 * If unmap_everything_possible is set, ignore the limit and unmap
 * all that can be unmapped. */
static void
nine_memfd_files_unmap(struct nine_allocator *allocator,
                       bool unmap_everything_possible)
{
    long long memory_limit = unmap_everything_possible ?
        0 : allocator->total_virtual_memory_limit;
    int i;

    /* We are below the limit. Do nothing */
    if (memory_limit >= allocator->total_virtual_memory)
        return;

    /* Update allocations with pending releases */
    nine_flush_pending_releases(allocator);

    DBG("Trying to unmap files with no weak unlock (%lld / %lld)\n",
        allocator->total_virtual_memory, memory_limit);

    /* Try to release everything with no weak releases.
     * Those have data not needed for a long time (and
     * possibly ever). */
    for (i = 0; i < allocator->num_fd; i++) {
        nine_memfd_try_unmap_file(allocator, i, false);
        if (memory_limit >= allocator->total_virtual_memory) {
            return;}
    }

    DBG("Trying to unmap files even with weak unlocks (%lld / %lld)\n",
        allocator->total_virtual_memory, memory_limit);

    /* This wasn't enough. Also release files with weak releases */
    for (i = 0; i < allocator->num_fd; i++) {
        nine_memfd_try_unmap_file(allocator, i, true);
        /* Stop if the target is reached */
        if (memory_limit >= allocator->total_virtual_memory) {
            return;}
    }

    if (!unmap_everything_possible)
        return;

    /* If there are some pending uploads, execute them,
     * and retry. */
    if (list_is_empty(&allocator->pending_releases)) {
        return;}
    nine_csmt_process(allocator->device);
    nine_flush_pending_releases(allocator);

    DBG("Retrying after flushing (%lld / %lld)\n",
        allocator->total_virtual_memory, memory_limit);

    for (i = 0; i < allocator->num_fd; i++) {
        nine_memfd_try_unmap_file(allocator, i, false);
        nine_memfd_try_unmap_file(allocator, i, true);
    }
    /* We have done all we could */
}

/* Map a given memfd file */
static bool
nine_memfd_region_map(struct nine_allocator *allocator, struct nine_memfd_file *memfd_file, struct nine_memfd_file_region *region)
{
    if (region->map != NULL)
        return true;

    debug_dump_memfd_state(memfd_file, true);
    nine_memfd_files_unmap(allocator, false);

    void *buf = mmap(NULL, region->size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd_file->fd, region->offset);

    if (buf == MAP_FAILED && errno == ENOMEM) {
        DBG("Failed to mmap a memfd file - trying to unmap other files\n");
        nine_memfd_files_unmap(allocator, true);
        buf = mmap(NULL, region->size, PROT_READ | PROT_WRITE, MAP_SHARED, memfd_file->fd, region->offset);
    }
    if (buf == MAP_FAILED) {
        DBG("Failed to mmap a memfd file, errno=%d\n", (int)errno);
        return false;
    }
    region->map = buf;
    /* no need to move to an unlocked mapped regions list, the caller will handle the list */
    allocator->total_virtual_memory += region->size;
    assert((uintptr_t)buf % NINE_ALLOCATION_ALIGNMENT == 0); /* mmap should be page_size aligned, so it should be fine */

    return true;
}

/* Allocate with memfd some memory. Returns True if successful. */
static bool
nine_memfd_allocator(struct nine_allocator *allocator,
                     struct nine_allocation *new_allocation,
                     unsigned allocation_size)
{
    struct nine_memfd_file *memfd_file;
    struct nine_memfd_file_region *region;

    allocation_size = DIVUP(allocation_size, allocator->page_size) * allocator->page_size;
    new_allocation->allocation_type = NINE_MEMFD_ALLOC;
    new_allocation->locks_on_counter = 0;
    new_allocation->pending_counter = NULL;
    new_allocation->weak_unlock = false;
    list_inithead(&new_allocation->list_free);
    list_inithead(&new_allocation->list_release);

    /* Try to find free space in a file already allocated */
    if (insert_new_allocation(allocator, new_allocation, allocation_size))
        return true;

    /* No - allocate new memfd file */

    if (allocator->num_fd == allocator->num_fd_max)
        return false; /* Too many memfd files */

    allocator->num_fd++;
    memfd_file = (void*)allocator->memfd_pool + (allocator->num_fd-1)*sizeof(struct nine_memfd_file);
    /* If the allocation size is above the memfd file default size, use a bigger size */
    memfd_file->filesize = MAX2(allocation_size, allocator->min_file_size);

    memfd_file->fd = memfd_create("gallium_nine_ram", 0);
    if (memfd_file->fd == -1) {
        DBG("Failed to created a memfd file, errno=%d\n", (int)errno);
        allocator->num_fd--;
        return false;
    }

    if (ftruncate(memfd_file->fd, memfd_file->filesize) != 0) {
        DBG("Failed to resize a memfd file, errno=%d\n", (int)errno);
        close(memfd_file->fd);
        allocator->num_fd--;
        return false;
    }

    list_inithead(&memfd_file->free_regions);
    list_inithead(&memfd_file->unmapped_allocated_regions);
    list_inithead(&memfd_file->locked_mapped_allocated_regions);
    list_inithead(&memfd_file->unlocked_mapped_allocated_regions);
    list_inithead(&memfd_file->weak_unlocked_mapped_allocated_regions);

    /* Initialize the memfd file with empty region and the allocation */
    region = allocate_region(allocator, 0, allocation_size);
    region->zero_filled = true; /* ftruncate does zero-fill the new data */
    list_add(&region->list, &memfd_file->unmapped_allocated_regions);
    new_allocation->memory.memfd.file = memfd_file;
    new_allocation->memory.memfd.region = region;
    allocator->total_allocations += allocation_size;

    if (allocation_size == memfd_file->filesize)
        return true;

    /* Add empty region */
    region = allocate_region(allocator, allocation_size, memfd_file->filesize - allocation_size);
    region->zero_filled = true; /* ftruncate does zero-fill the new data */
    list_add(&region->list, &memfd_file->free_regions);

    return true;
}

/* Allocate memory */
struct nine_allocation *
nine_allocate(struct nine_allocator *allocator, unsigned size)
{

    struct nine_allocation *new_allocation = slab_alloc_st(&allocator->allocation_pool);
    debug_dump_allocator_state(allocator);
    if (!new_allocation)
        return NULL;

    nine_flush_pending_frees(allocator);

    /* Restrict to >= page_size to prevent having too much fragmentation, as the size of
     * allocations is rounded to the next page_size multiple. */
    if (size >= allocator->page_size && allocator->total_virtual_memory_limit >= 0 &&
        nine_memfd_allocator(allocator, new_allocation, size)) {
        struct nine_memfd_file_region *region = new_allocation->memory.memfd.region;
        if (!region->zero_filled) {
            void *data = nine_get_pointer(allocator, new_allocation);
            if (!data) {
                ERR("INTERNAL MMAP FOR NEW ALLOCATION FAILED\n");
                nine_free(allocator, new_allocation);
                return NULL;
            }
            DBG("memset on data=%p, size %d\n", data, region->size);
            memset(data, 0, region->size);
            region->zero_filled = true;
            /* Even though the user usually fills afterward, we don't weakrelease.
             * The reason is suballocations don't affect the weakrelease state of their
             * parents. Thus if only suballocations are accessed, the release would stay
             * weak forever. */
            nine_pointer_strongrelease(allocator, new_allocation);
        }
        DBG("ALLOCATION SUCCESSFUL\n");
        debug_dump_allocation_state(new_allocation);
        return new_allocation;
    }

    void *data = align_calloc(size, NINE_ALLOCATION_ALIGNMENT);
    if (!data) {
        DBG("ALLOCATION FAILED\n");
        return NULL;
    }

    new_allocation->allocation_type = NINE_MALLOC_ALLOC;
    new_allocation->memory.malloc.buf = data;
    new_allocation->memory.malloc.allocation_size = size;
    list_inithead(&new_allocation->list_free);
    allocator->total_allocations += size;
    allocator->total_locked_memory += size;
    allocator->total_virtual_memory += size;
    DBG("ALLOCATION SUCCESSFUL\n");
    debug_dump_allocation_state(new_allocation);
    return new_allocation;
}

/* Release memory */
static void
nine_free_internal(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    DBG("RELEASING ALLOCATION\n");
    debug_dump_allocation_state(allocation);
    if (allocation->allocation_type == NINE_MALLOC_ALLOC) {
        allocator->total_allocations -= allocation->memory.malloc.allocation_size;
        allocator->total_locked_memory -= allocation->memory.malloc.allocation_size;
        allocator->total_virtual_memory -= allocation->memory.malloc.allocation_size;
        align_free(allocation->memory.malloc.buf);
    } else if (allocation->allocation_type == NINE_MEMFD_ALLOC ||
        allocation->allocation_type == NINE_MEMFD_SUBALLOC) {
        struct nine_memfd_file *memfd_file = nine_get_memfd_file_backing(allocation);
        struct nine_memfd_file_region *region = nine_get_memfd_region_backing(allocation);
        if (allocation->weak_unlock)
            region->num_weak_unlocks--;
        if (allocation->allocation_type == NINE_MEMFD_ALLOC)
            remove_allocation(allocator, memfd_file, region);
    }

    slab_free_st(&allocator->allocation_pool, allocation);
    debug_dump_allocator_state(allocator);
}


void
nine_free(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    nine_flush_pending_frees(allocator);
    nine_flush_pending_releases(allocator);
    nine_free_internal(allocator, allocation);
}

/* Called from the worker thread. Similar to nine_free except we are not in the main thread, thus
 * we are disallowed to change the allocator structures except the fields reserved
 * for the worker. In addition, the allocation is allowed to not being unlocked (the release
 * will unlock it) */
void nine_free_worker(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    /* Add the allocation to the list of pending allocations to free */
    pthread_mutex_lock(&allocator->mutex_pending_frees);
    /* The order of free matters as suballocations are supposed to be released first */
    list_addtail(&allocation->list_free, &allocator->pending_frees);
    pthread_mutex_unlock(&allocator->mutex_pending_frees);
}

/* Lock an allocation, and retrieve the pointer */
void *
nine_get_pointer(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    struct nine_memfd_file *memfd_file;
    struct nine_memfd_file_region *region;

    nine_flush_pending_releases(allocator);
    DBG("allocation_type: %d\n", allocation->allocation_type);

    if (allocation->allocation_type == NINE_MALLOC_ALLOC)
        return allocation->memory.malloc.buf;
    if (allocation->allocation_type == NINE_EXTERNAL_ALLOC)
        return allocation->memory.external.buf;

    memfd_file = nine_get_memfd_file_backing(allocation);
    region = nine_get_memfd_region_backing(allocation);
    if (!nine_memfd_region_map(allocator, memfd_file, region)) {
        DBG("Couldn't map memfd region for get_pointer\n");
        return NULL;
    }

    move_region(&memfd_file->locked_mapped_allocated_regions, region); /* Note: redundant if region->num_locks */
    region->num_locks++;

    if (region->num_locks == 1)
        allocator->total_locked_memory += region->size;
    if (allocation->weak_unlock)
        region->num_weak_unlocks--;
    allocation->weak_unlock = false;
    region->zero_filled = false;


    if (allocation->allocation_type == NINE_MEMFD_ALLOC)
        return region->map;
    if (allocation->allocation_type == NINE_MEMFD_SUBALLOC)
        return region->map + allocation->memory.submemfd.relative_offset;

    assert(false);
    return NULL;
}

/* Unlock an allocation, but with hint that we might lock again soon */
void
nine_pointer_weakrelease(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    struct nine_memfd_file_region *region;
    if (allocation->allocation_type > NINE_MEMFD_SUBALLOC)
        return;

    region = nine_get_memfd_region_backing(allocation);
    if (!allocation->weak_unlock)
        region->num_weak_unlocks++;
    allocation->weak_unlock = true;
    region->num_locks--;
    if (region->num_locks == 0) {
        struct nine_memfd_file *memfd_file = nine_get_memfd_file_backing(allocation);
        allocator->total_locked_memory -= region->size;
        move_region(&memfd_file->weak_unlocked_mapped_allocated_regions, region);
    }
}

/* Unlock an allocation */
void
nine_pointer_strongrelease(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    struct nine_memfd_file_region *region;
    if (allocation->allocation_type > NINE_MEMFD_SUBALLOC)
        return;

    region = nine_get_memfd_region_backing(allocation);
    region->num_locks--;
    if (region->num_locks == 0) {
        struct nine_memfd_file *memfd_file = nine_get_memfd_file_backing(allocation);
        allocator->total_locked_memory -= region->size;
        if (region->num_weak_unlocks)
            move_region(&memfd_file->weak_unlocked_mapped_allocated_regions, region);
        else
            move_region(&memfd_file->unlocked_mapped_allocated_regions, region);
    }
}

/* Delay a release to when a given counter becomes zero */
void
nine_pointer_delayedstrongrelease(struct nine_allocator *allocator, struct nine_allocation *allocation, unsigned *counter)
{
    if (allocation->allocation_type > NINE_MEMFD_SUBALLOC)
        return;

    assert(allocation->pending_counter == NULL || allocation->pending_counter == counter);
    allocation->pending_counter = counter;
    allocation->locks_on_counter++;

    if (list_is_empty(&allocation->list_release))
        list_add(&allocation->list_release, &allocator->pending_releases);
}

/* Create a suballocation of an allocation */
struct nine_allocation *
nine_suballocate(struct nine_allocator* allocator, struct nine_allocation *allocation, int offset)
{
    struct nine_allocation *new_allocation = slab_alloc_st(&allocator->allocation_pool);
    if (!new_allocation)
        return NULL;

    DBG("Suballocate allocation at offset: %d\n", offset);
    assert(allocation->allocation_type != NINE_MEMFD_SUBALLOC);
    list_inithead(&new_allocation->list_free);

    if (allocation->allocation_type != NINE_MEMFD_ALLOC) {
        new_allocation->allocation_type = NINE_EXTERNAL_ALLOC;
        if (allocation->allocation_type == NINE_MALLOC_ALLOC)
            new_allocation->memory.external.buf = allocation->memory.malloc.buf + offset;
        else
            new_allocation->memory.external.buf = allocation->memory.external.buf + offset;
        return new_allocation;
    }
    new_allocation->allocation_type = NINE_MEMFD_SUBALLOC;
    new_allocation->memory.submemfd.parent = &allocation->memory.memfd;
    new_allocation->memory.submemfd.relative_offset = offset;
    new_allocation->locks_on_counter = 0;
    new_allocation->pending_counter = NULL;
    new_allocation->weak_unlock = false;
    list_inithead(&new_allocation->list_release);
    debug_dump_allocation_state(new_allocation);
    return new_allocation;
}

/* Wrap an external pointer as an allocation */
struct nine_allocation *
nine_wrap_external_pointer(struct nine_allocator* allocator, void* data)
{
    struct nine_allocation *new_allocation = slab_alloc_st(&allocator->allocation_pool);
    if (!new_allocation)
        return NULL;
    DBG("Wrapping external pointer: %p\n", data);
    new_allocation->allocation_type = NINE_EXTERNAL_ALLOC;
    new_allocation->memory.external.buf = data;
    list_inithead(&new_allocation->list_free);
    return new_allocation;
}

struct nine_allocator *
nine_allocator_create(struct NineDevice9 *device, int memfd_virtualsizelimit)
{
    struct nine_allocator* allocator = MALLOC(sizeof(struct nine_allocator));

    if (!allocator)
        return NULL;

    allocator->device = device;
    allocator->page_size = sysconf(_SC_PAGESIZE);
    assert(allocator->page_size == 4 << 10);
    allocator->num_fd_max = (memfd_virtualsizelimit >= 0) ? MIN2(128, sysconf(_SC_OPEN_MAX)) : 0;
    allocator->min_file_size = DIVUP(100 * (1 << 20), allocator->page_size) * allocator->page_size; /* 100MB files */
    allocator->total_allocations = 0;
    allocator->total_locked_memory = 0;
    allocator->total_virtual_memory = 0;
    allocator->total_virtual_memory_limit = memfd_virtualsizelimit * (1 << 20);
    allocator->num_fd = 0;

    DBG("Allocator created (ps: %d; fm: %d)\n", allocator->page_size, allocator->num_fd_max);

    slab_create(&allocator->allocation_pool, sizeof(struct nine_allocation), 4096);
    slab_create(&allocator->region_pool, sizeof(struct nine_memfd_file_region), 4096);
    allocator->memfd_pool = CALLOC(allocator->num_fd_max, sizeof(struct nine_memfd_file));
    list_inithead(&allocator->pending_releases);
    list_inithead(&allocator->pending_frees);
    pthread_mutex_init(&allocator->mutex_pending_frees, NULL);
    return allocator;
}

void
nine_allocator_destroy(struct nine_allocator* allocator)
{
    int i;
    DBG("DESTROYING ALLOCATOR\n");
    debug_dump_allocator_state(allocator);
    nine_flush_pending_releases(allocator);
    nine_flush_pending_frees(allocator);
    nine_memfd_files_unmap(allocator, true);
    pthread_mutex_destroy(&allocator->mutex_pending_frees);

    assert(list_is_empty(&allocator->pending_frees));
    assert(list_is_empty(&allocator->pending_releases));
    for (i = 0; i < allocator->num_fd; i++) {
        debug_dump_memfd_state(&allocator->memfd_pool[i], true);
        assert(list_is_empty(&allocator->memfd_pool[i].locked_mapped_allocated_regions));
        assert(list_is_empty(&allocator->memfd_pool[i].weak_unlocked_mapped_allocated_regions));
        assert(list_is_empty(&allocator->memfd_pool[i].unlocked_mapped_allocated_regions));
        assert(list_is_singular(&allocator->memfd_pool[i].free_regions));
        slab_free_st(&allocator->region_pool,
                     list_first_entry(&allocator->memfd_pool[i].free_regions,
                                      struct nine_memfd_file_region, list));
        close(allocator->memfd_pool[i].fd);
    }
    slab_destroy(&allocator->allocation_pool);
    slab_destroy(&allocator->region_pool);
    FREE(allocator->memfd_pool);
    FREE(allocator);
}

#else

struct nine_allocation {
    unsigned is_external;
    void *external;
};

struct nine_allocator {
    struct slab_mempool external_allocation_pool;
    pthread_mutex_t mutex_slab;
};

struct nine_allocation *
nine_allocate(struct nine_allocator *allocator, unsigned size)
{
    struct nine_allocation *allocation;
    (void)allocator;
    assert(sizeof(struct nine_allocation) <= NINE_ALLOCATION_ALIGNMENT);
    allocation = align_calloc(size + NINE_ALLOCATION_ALIGNMENT, NINE_ALLOCATION_ALIGNMENT);
    allocation->is_external = false;
    return allocation;
}


void nine_free(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    if (allocation->is_external) {
        pthread_mutex_lock(&allocator->mutex_slab);
        slab_free_st(&allocator->external_allocation_pool, allocation);
        pthread_mutex_unlock(&allocator->mutex_slab);
    } else
        align_free(allocation);
}

void nine_free_worker(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    nine_free(allocator, allocation);
}

void *nine_get_pointer(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    (void)allocator;
    if (allocation->is_external)
        return allocation->external;
    return (uint8_t *)allocation + NINE_ALLOCATION_ALIGNMENT;
}

void nine_pointer_weakrelease(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    (void)allocator;
    (void)allocation;
}

void nine_pointer_strongrelease(struct nine_allocator *allocator, struct nine_allocation *allocation)
{
    (void)allocator;
    (void)allocation;
}

void nine_pointer_delayedstrongrelease(struct nine_allocator *allocator,
                                       struct nine_allocation *allocation,
                                       unsigned *counter)
{
    (void)allocator;
    (void)allocation;
    (void)counter;
}

struct nine_allocation *
nine_suballocate(struct nine_allocator* allocator, struct nine_allocation *allocation, int offset)
{
    struct nine_allocation *new_allocation;
    pthread_mutex_lock(&allocator->mutex_slab);
    new_allocation = slab_alloc_st(&allocator->external_allocation_pool);
    pthread_mutex_unlock(&allocator->mutex_slab);
    new_allocation->is_external = true;
    new_allocation->external = (uint8_t *)allocation + NINE_ALLOCATION_ALIGNMENT + offset;
    return new_allocation;
}

struct nine_allocation *
nine_wrap_external_pointer(struct nine_allocator* allocator, void* data)
{
    struct nine_allocation *new_allocation;
    pthread_mutex_lock(&allocator->mutex_slab);
    new_allocation = slab_alloc_st(&allocator->external_allocation_pool);
    pthread_mutex_unlock(&allocator->mutex_slab);
    new_allocation->is_external = true;
    new_allocation->external = data;
    return new_allocation;
}

struct nine_allocator *
nine_allocator_create(struct NineDevice9 *device, int memfd_virtualsizelimit)
{
    struct nine_allocator* allocator = MALLOC(sizeof(struct nine_allocator));
    (void)device;
    (void)memfd_virtualsizelimit;

    if (!allocator)
        return NULL;

    slab_create(&allocator->external_allocation_pool, sizeof(struct nine_allocation), 4096);
    pthread_mutex_init(&allocator->mutex_slab, NULL);

    return allocator;
}

void
nine_allocator_destroy(struct nine_allocator *allocator)
{
    slab_destroy(&allocator->external_allocation_pool);
    pthread_mutex_destroy(&allocator->mutex_slab);
}

#endif /* NINE_ENABLE_MEMFD */
