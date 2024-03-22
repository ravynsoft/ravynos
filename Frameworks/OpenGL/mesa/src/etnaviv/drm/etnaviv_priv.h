/*
 * Copyright (C) 2014-2015 Etnaviv Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Christian Gmeiner <christian.gmeiner@gmail.com>
 */

#ifndef ETNAVIV_PRIV_H_
#define ETNAVIV_PRIV_H_

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <assert.h>

#include <xf86drm.h>

#include "util/list.h"
#include "util/log.h"
#include "util/macros.h"
#include "util/simple_mtx.h"
#include "util/timespec.h"
#include "util/u_atomic.h"
#include "util/u_debug.h"
#include "util/u_math.h"
#include "util/vma.h"

#include "etnaviv_drmif.h"
#include "drm-uapi/etnaviv_drm.h"

extern simple_mtx_t etna_device_lock;

struct etna_bo_bucket {
	uint32_t size;
	struct list_head list;
};

struct etna_bo_cache {
	struct etna_bo_bucket cache_bucket[14 * 4];
	unsigned num_buckets;
	time_t time;
};

struct etna_device {
	int fd;
	uint32_t drm_version;
	int refcnt;

	/* tables to keep track of bo's, to avoid "evil-twin" etna_bo objects:
	 *
	 *   handle_table: maps handle to etna_bo
	 *   name_table: maps flink name to etna_bo
	 *
	 * We end up needing two tables, because DRM_IOCTL_GEM_OPEN always
	 * returns a new handle.  So we need to figure out if the bo is already
	 * open in the process first, before calling gem-open.
	 */
	void *handle_table, *name_table;

	struct etna_bo_cache bo_cache;
	struct list_head zombie_list;

	int use_softpin;
	struct util_vma_heap address_space;

	int closefd;        /* call close(fd) upon destruction */
};

void etna_bo_free(struct etna_bo *bo);
void etna_bo_kill_zombies(struct etna_device *dev);

void etna_bo_cache_init(struct etna_bo_cache *cache);
void etna_bo_cache_cleanup(struct etna_bo_cache *cache, time_t time);
struct etna_bo *etna_bo_cache_alloc(struct etna_bo_cache *cache,
		uint32_t *size, uint32_t flags);
int etna_bo_cache_free(struct etna_bo_cache *cache, struct etna_bo *bo);

/* for where @etna_drm_table_lock is already held: */
void etna_device_del_locked(struct etna_device *dev);

/* a GEM buffer object allocated from the DRM device */
struct etna_bo {
	struct etna_device      *dev;
	void            *map;           /* userspace mmap'ing (if there is one) */
	uint32_t        size;
	uint32_t        handle;
	uint32_t        flags;
	uint32_t        name;           /* flink global handle (DRI2 name) */
	uint32_t        va;             /* GPU virtual address */
	int		refcnt;

	int reuse;
	struct list_head list;   /* bucket-list entry */
	time_t free_time;        /* time when added to bucket-list */
};

struct etna_gpu {
	struct etna_device *dev;
	uint32_t core;
	uint32_t model;
	uint32_t revision;
};

struct etna_pipe {
	enum etna_pipe_id id;
	struct etna_gpu *gpu;
};

struct etna_cmd_stream_priv {
	struct etna_cmd_stream base;
	struct etna_pipe *pipe;

	uint32_t last_timestamp;
	uint32_t offset_end_of_context_init;

	/* submit ioctl related tables: */
	struct {
		/* bo's table: */
		struct drm_etnaviv_gem_submit_bo *bos;
		uint32_t nr_bos, max_bos;

		/* reloc's table: */
		struct drm_etnaviv_gem_submit_reloc *relocs;
		uint32_t nr_relocs, max_relocs;

		/* perf's table: */
		struct drm_etnaviv_gem_submit_pmr *pmrs;
		uint32_t nr_pmrs, max_pmrs;
	} submit;

	/* should have matching entries in submit.bos: */
	struct etna_bo **bos;
	uint32_t nr_bos, max_bos;

	/* notify callback if buffer reset happened */
	void (*force_flush)(struct etna_cmd_stream *stream, void *priv);
	void *force_flush_priv;

	void *bo_table;
};

struct etna_perfmon {
	struct list_head domains;
	struct etna_pipe *pipe;
};

struct etna_perfmon_domain
{
	struct list_head head;
	struct list_head signals;
	uint8_t id;
	char name[64];
};

struct etna_perfmon_signal
{
	struct list_head head;
	struct etna_perfmon_domain *domain;
	uint8_t signal;
	char name[64];
};

#define ETNA_DRM_MSGS 0x40
extern int etna_mesa_debug;

#define INFO_MSG(fmt, ...) \
		do { mesa_logi("%s:%d: " fmt, \
				__func__, __LINE__, ##__VA_ARGS__); } while (0)
#define DEBUG_MSG(fmt, ...) \
		do if (etna_mesa_debug & ETNA_DRM_MSGS) { \
		     mesa_logd("%s:%d: " fmt, \
				__func__, __LINE__, ##__VA_ARGS__); } while (0)
#define WARN_MSG(fmt, ...) \
		do { mesa_logw("%s:%d: " fmt, \
				__func__, __LINE__, ##__VA_ARGS__); } while (0)
#define ERROR_MSG(fmt, ...) \
		do { mesa_loge("%s:%d: " fmt, \
				__func__, __LINE__, ##__VA_ARGS__); } while (0)

#define DEBUG_BO(msg, bo)						\
   DEBUG_MSG("%s %p, va: 0x%.8x, size: 0x%.8x, flags: 0x%.8x, "		\
	     "refcnt: %d, handle: 0x%.8x, name: 0x%.8x",		\
	     msg, bo, bo->va, bo->size, bo->flags, bo->refcnt, bo->handle, bo->name);

#define VOID2U64(x) ((uint64_t)(unsigned long)(x))

static inline void get_abs_timeout(struct drm_etnaviv_timespec *tv, uint64_t ns)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	tv->tv_sec = t.tv_sec + ns / NSEC_PER_SEC;
	tv->tv_nsec = t.tv_nsec + ns % NSEC_PER_SEC;
	if (tv->tv_nsec >= NSEC_PER_SEC) {
		tv->tv_nsec -= NSEC_PER_SEC;
		tv->tv_sec++;
	}
}

#if HAVE_VALGRIND
#  include <memcheck.h>

/*
 * For tracking the backing memory (if valgrind enabled, we force a mmap
 * for the purposes of tracking)
 */
static inline void VG_BO_ALLOC(struct etna_bo *bo)
{
	if (bo && RUNNING_ON_VALGRIND) {
		VALGRIND_MALLOCLIKE_BLOCK(etna_bo_map(bo), bo->size, 0, 1);
	}
}

static inline void VG_BO_FREE(struct etna_bo *bo)
{
	VALGRIND_FREELIKE_BLOCK(bo->map, 0);
}

/*
 * For tracking bo structs that are in the buffer-cache, so that valgrind
 * doesn't attribute ownership to the first one to allocate the recycled
 * bo.
 *
 * Note that the list_head in etna_bo is used to track the buffers in cache
 * so disable error reporting on the range while they are in cache so
 * valgrind doesn't squawk about list traversal.
 *
 */
static inline void VG_BO_RELEASE(struct etna_bo *bo)
{
	if (RUNNING_ON_VALGRIND) {
		VALGRIND_DISABLE_ADDR_ERROR_REPORTING_IN_RANGE(bo, sizeof(*bo));
		VALGRIND_MAKE_MEM_NOACCESS(bo, sizeof(*bo));
		VALGRIND_FREELIKE_BLOCK(bo->map, 0);
	}
}
static inline void VG_BO_OBTAIN(struct etna_bo *bo)
{
	if (RUNNING_ON_VALGRIND) {
		VALGRIND_MAKE_MEM_DEFINED(bo, sizeof(*bo));
		VALGRIND_ENABLE_ADDR_ERROR_REPORTING_IN_RANGE(bo, sizeof(*bo));
		VALGRIND_MALLOCLIKE_BLOCK(bo->map, bo->size, 0, 1);
	}
}
#else
static inline void VG_BO_ALLOC(struct etna_bo *bo)   {}
static inline void VG_BO_FREE(struct etna_bo *bo)    {}
static inline void VG_BO_RELEASE(struct etna_bo *bo) {}
static inline void VG_BO_OBTAIN(struct etna_bo *bo)  {}
#endif

#endif /* ETNAVIV_PRIV_H_ */
