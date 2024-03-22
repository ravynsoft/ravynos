/*
 * Copyright © 2014-2017 Broadcom
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file v3d_simulator.c
 *
 * Implements V3D simulation on top of a non-V3D GEM fd.
 *
 * This file's goal is to emulate the V3D ioctls' behavior in the kernel on
 * top of the simpenrose software simulator.  Generally, V3D driver BOs have a
 * GEM-side copy of their contents and a simulator-side memory area that the
 * GEM contents get copied into during simulation.  Once simulation is done,
 * the simulator's data is copied back out to the GEM BOs, so that rendering
 * appears on the screen as if actual hardware rendering had been done.
 *
 * One of the limitations of this code is that we shouldn't really need a
 * GEM-side BO for non-window-system BOs.  However, do we need unique BO
 * handles for each of our GEM bos so that this file can look up its state
 * from the handle passed in at submit ioctl time (also, a couple of places
 * outside of this file still call ioctls directly on the fd).
 *
 * Another limitation is that BO import doesn't work unless the underlying
 * window system's BO size matches what V3D is going to use, which of course
 * doesn't work out in practice.  This means that for now, only DRI3 (V3D
 * makes the winsys BOs) is supported, not DRI2 (window system makes the winys
 * BOs).
 */

#ifdef USE_V3D_SIMULATOR

#include <stdio.h>
#include <sys/mman.h>
#include "c11/threads.h"
#include "util/hash_table.h"
#include "util/ralloc.h"
#include "util/set.h"
#include "util/simple_mtx.h"
#include "util/u_dynarray.h"
#include "util/u_memory.h"
#include "util/u_mm.h"
#include "util/u_math.h"

#include <xf86drm.h>
#include "drm-uapi/amdgpu_drm.h"
#include "drm-uapi/i915_drm.h"
#include "drm-uapi/v3d_drm.h"

#include "v3d_simulator.h"
#include "v3d_simulator_wrapper.h"

#include "broadcom/common/v3d_csd.h"

/** Global (across GEM fds) state for the simulator */
static struct v3d_simulator_state {
        simple_mtx_t mutex;
        mtx_t submit_lock;

        struct v3d_hw *v3d;
        int ver;

        /* Base virtual address of the heap. */
        void *mem;
        /* Base hardware address of the heap. */
        uint32_t mem_base;
        /* Size of the heap. */
        uint32_t mem_size;

        struct mem_block *heap;
        struct mem_block *overflow;

        /** Mapping from GEM fd to struct v3d_simulator_file * */
        struct hash_table *fd_map;

        /** Last performance monitor ID. */
        uint32_t last_perfid;

        /** Total performance counters */
        uint32_t perfcnt_total;

        struct util_dynarray bin_oom;
        int refcount;
} sim_state = {
        .mutex = SIMPLE_MTX_INITIALIZER,
};

enum gem_type {
        GEM_I915,
        GEM_AMDGPU,
        GEM_DUMB
};

/** Per-GEM-fd state for the simulator. */
struct v3d_simulator_file {
        int fd;

        /** Mapping from GEM handle to struct v3d_simulator_bo * */
        struct hash_table *bo_map;

        /** Dynamic array with performance monitors */
        struct v3d_simulator_perfmon **perfmons;
        uint32_t perfmons_size;
        uint32_t active_perfid;

        struct mem_block *gmp;
        void *gmp_vaddr;

        /** For specific gpus, use their create ioctl. Otherwise use dumb bo. */
        enum gem_type gem_type;
};

/** Wrapper for drm_v3d_bo tracking the simulator-specific state. */
struct v3d_simulator_bo {
        struct v3d_simulator_file *file;

        /** Area for this BO within sim_state->mem */
        struct mem_block *block;
        uint32_t size;
        uint64_t mmap_offset;
        void *sim_vaddr;
        void *gem_vaddr;

        int handle;
};

struct v3d_simulator_perfmon {
        uint32_t ncounters;
        uint8_t counters[DRM_V3D_MAX_PERF_COUNTERS];
        uint64_t values[DRM_V3D_MAX_PERF_COUNTERS];
};

static void *
int_to_key(int key)
{
        return (void *)(uintptr_t)key;
}

#define PERFMONS_ALLOC_SIZE 100

static uint32_t
perfmons_next_id(struct v3d_simulator_file *sim_file) {
        sim_state.last_perfid++;
        if (sim_state.last_perfid > sim_file->perfmons_size) {
                sim_file->perfmons_size += PERFMONS_ALLOC_SIZE;
                sim_file->perfmons = reralloc(sim_file,
                                              sim_file->perfmons,
                                              struct v3d_simulator_perfmon *,
                                              sim_file->perfmons_size);
        }

        return sim_state.last_perfid;
}

static struct v3d_simulator_file *
v3d_get_simulator_file_for_fd(int fd)
{
        struct hash_entry *entry = _mesa_hash_table_search(sim_state.fd_map,
                                                           int_to_key(fd + 1));
        return entry ? entry->data : NULL;
}

/* A marker placed just after each BO, then checked after rendering to make
 * sure it's still there.
 */
#define BO_SENTINEL		0xfedcba98

/* 128kb */
#define GMP_ALIGN2		17

/**
 * Sets the range of GPU virtual address space to have the given GMP
 * permissions (bit 0 = read, bit 1 = write, write-only forbidden).
 */
static void
set_gmp_flags(struct v3d_simulator_file *file,
              uint32_t offset, uint32_t size, uint32_t flag)
{
        assert((offset & ((1 << GMP_ALIGN2) - 1)) == 0);
        int gmp_offset = offset >> GMP_ALIGN2;
        int gmp_count = align(size, 1 << GMP_ALIGN2) >> GMP_ALIGN2;
        uint32_t *gmp = file->gmp_vaddr;

        assert(flag <= 0x3);

        for (int i = gmp_offset; i < gmp_offset + gmp_count; i++) {
                int32_t bitshift = (i % 16) * 2;
                gmp[i / 16] &= ~(0x3 << bitshift);
                gmp[i / 16] |= flag << bitshift;
        }
}

/**
 * Allocates space in simulator memory and returns a tracking struct for it
 * that also contains the drm_gem_cma_object struct.
 */
static struct v3d_simulator_bo *
v3d_create_simulator_bo(int fd, unsigned size)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        struct v3d_simulator_bo *sim_bo = rzalloc(file,
                                                  struct v3d_simulator_bo);
        size = align(size, 4096);

        sim_bo->file = file;

        simple_mtx_lock(&sim_state.mutex);
        sim_bo->block = u_mmAllocMem(sim_state.heap, size + 4, GMP_ALIGN2, 0);
        simple_mtx_unlock(&sim_state.mutex);
        assert(sim_bo->block);

        set_gmp_flags(file, sim_bo->block->ofs, size, 0x3);

        sim_bo->size = size;

        /* Allocate space for the buffer in simulator memory. */
        sim_bo->sim_vaddr = sim_state.mem + sim_bo->block->ofs - sim_state.mem_base;
        memset(sim_bo->sim_vaddr, 0xd0, size);

        *(uint32_t *)(sim_bo->sim_vaddr + sim_bo->size) = BO_SENTINEL;

        return sim_bo;
}

static struct v3d_simulator_bo *
v3d_create_simulator_bo_for_gem(int fd, int handle, unsigned size)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        struct v3d_simulator_bo *sim_bo =
                v3d_create_simulator_bo(fd, size);

        sim_bo->handle = handle;

        /* Map the GEM buffer for copy in/out to the simulator.  i915 blocks
         * dumb mmap on render nodes, so use their ioctl directly if we're on
         * one.
         */
        int ret;
        switch (file->gem_type) {
        case GEM_I915:
        {
                struct drm_i915_gem_mmap_gtt map = {
                        .handle = handle,
                };

                /* We could potentially use non-gtt (cached) for LLC systems,
                 * but the copy-in/out won't be the limiting factor on
                 * simulation anyway.
                 */
                ret = drmIoctl(fd, DRM_IOCTL_I915_GEM_MMAP_GTT, &map);
                sim_bo->mmap_offset = map.offset;
                break;
        }
        case GEM_AMDGPU:
        {
                union drm_amdgpu_gem_mmap map = { 0 };
                map.in.handle = handle;

                ret = drmIoctl(fd, DRM_IOCTL_AMDGPU_GEM_MMAP, &map);
                sim_bo->mmap_offset = map.out.addr_ptr;
                break;
        }
        default:
        {
                struct drm_mode_map_dumb map = {
                        .handle = handle,
                };
                ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map);
                sim_bo->mmap_offset = map.offset;
        }
        }
        if (ret) {
                fprintf(stderr, "Failed to get MMAP offset: %d\n", ret);
                abort();
        }

        sim_bo->gem_vaddr = mmap(NULL, sim_bo->size,
                                 PROT_READ | PROT_WRITE, MAP_SHARED,
                                 fd, sim_bo->mmap_offset);
        if (sim_bo->gem_vaddr == MAP_FAILED) {
                fprintf(stderr, "mmap of bo %d (offset 0x%016llx, size %d) failed\n",
                        handle, (long long)sim_bo->mmap_offset, sim_bo->size);
                abort();
        }

        /* A handle of 0 is used for v3d_gem.c internal allocations that
         * don't need to go in the lookup table.
         */
        if (handle != 0) {
                simple_mtx_lock(&sim_state.mutex);
                _mesa_hash_table_insert(file->bo_map, int_to_key(handle),
                                        sim_bo);
                simple_mtx_unlock(&sim_state.mutex);
        }

        return sim_bo;
}

static int bin_fd;

uint32_t
v3d_simulator_get_spill(uint32_t spill_size)
{
        struct v3d_simulator_bo *sim_bo =
                v3d_create_simulator_bo(bin_fd, spill_size);

        util_dynarray_append(&sim_state.bin_oom, struct v3d_simulator_bo *,
                             sim_bo);

        return sim_bo->block->ofs;
}

static void
v3d_free_simulator_bo(struct v3d_simulator_bo *sim_bo)
{
        struct v3d_simulator_file *sim_file = sim_bo->file;

        set_gmp_flags(sim_file, sim_bo->block->ofs, sim_bo->size, 0x0);

        if (sim_bo->gem_vaddr)
                munmap(sim_bo->gem_vaddr, sim_bo->size);

        simple_mtx_lock(&sim_state.mutex);
        u_mmFreeMem(sim_bo->block);
        if (sim_bo->handle) {
                _mesa_hash_table_remove_key(sim_file->bo_map,
                                            int_to_key(sim_bo->handle));
        }
        simple_mtx_unlock(&sim_state.mutex);
        ralloc_free(sim_bo);
}

static struct v3d_simulator_bo *
v3d_get_simulator_bo(struct v3d_simulator_file *file, int gem_handle)
{
        if (gem_handle == 0)
                return NULL;

        simple_mtx_lock(&sim_state.mutex);
        struct hash_entry *entry =
                _mesa_hash_table_search(file->bo_map, int_to_key(gem_handle));
        simple_mtx_unlock(&sim_state.mutex);

        return entry ? entry->data : NULL;
}

static void
v3d_simulator_copy_in_handle(struct v3d_simulator_file *file, int handle)
{
        struct v3d_simulator_bo *sim_bo = v3d_get_simulator_bo(file, handle);

        if (!sim_bo)
                return;

        memcpy(sim_bo->sim_vaddr, sim_bo->gem_vaddr, sim_bo->size);
}

static void
v3d_simulator_copy_out_handle(struct v3d_simulator_file *file, int handle)
{
        struct v3d_simulator_bo *sim_bo = v3d_get_simulator_bo(file, handle);

        if (!sim_bo)
                return;

        memcpy(sim_bo->gem_vaddr, sim_bo->sim_vaddr, sim_bo->size);

        if (*(uint32_t *)(sim_bo->sim_vaddr +
                          sim_bo->size) != BO_SENTINEL) {
                fprintf(stderr, "Buffer overflow in handle %d\n",
                        handle);
        }
}

static int
v3d_simulator_pin_bos(struct v3d_simulator_file *file,
                      struct drm_v3d_submit_cl *submit)
{
        uint32_t *bo_handles = (uint32_t *)(uintptr_t)submit->bo_handles;

        for (int i = 0; i < submit->bo_handle_count; i++)
                v3d_simulator_copy_in_handle(file, bo_handles[i]);

        return 0;
}

static int
v3d_simulator_unpin_bos(struct v3d_simulator_file *file,
                        struct drm_v3d_submit_cl *submit)
{
        uint32_t *bo_handles = (uint32_t *)(uintptr_t)submit->bo_handles;

        for (int i = 0; i < submit->bo_handle_count; i++)
                v3d_simulator_copy_out_handle(file, bo_handles[i]);

        return 0;
}

static struct v3d_simulator_perfmon *
v3d_get_simulator_perfmon(int fd, uint32_t perfid)
{
        if (!perfid || perfid > sim_state.last_perfid)
                return NULL;

        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);

        simple_mtx_lock(&sim_state.mutex);
        assert(perfid <= file->perfmons_size);
        struct v3d_simulator_perfmon *perfmon = file->perfmons[perfid - 1];
        simple_mtx_unlock(&sim_state.mutex);

        return perfmon;
}

static void
v3d_simulator_perfmon_switch(int fd, uint32_t perfid)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        struct v3d_simulator_perfmon *perfmon;

        if (perfid == file->active_perfid)
                return;

        perfmon = v3d_get_simulator_perfmon(fd, file->active_perfid);
        if (perfmon)
                v3d_X_simulator(perfmon_stop)(sim_state.v3d,
                                              perfmon->ncounters,
                                              perfmon->values);

        perfmon = v3d_get_simulator_perfmon(fd, perfid);
        if (perfmon)
                v3d_X_simulator(perfmon_start)(sim_state.v3d,
                                               perfmon->ncounters,
                                               perfmon->counters);

        file->active_perfid = perfid;
}

static int
v3d_simulator_signal_syncobjs(int fd, struct drm_v3d_multi_sync *ms)
{
        struct drm_v3d_sem *out_syncs = (void *)(uintptr_t)ms->out_syncs;
        int n_syncobjs = ms->out_sync_count;
        uint32_t syncobjs[n_syncobjs];

        for (int i = 0; i < n_syncobjs; i++)
                syncobjs[i] = out_syncs[i].handle;
        return drmSyncobjSignal(fd, (uint32_t *) &syncobjs, n_syncobjs);
}

static int
v3d_simulator_process_post_deps(int fd, struct drm_v3d_extension *ext)
{
        int ret = 0;
        while (ext && ext->id != DRM_V3D_EXT_ID_MULTI_SYNC)
                ext = (void *)(uintptr_t) ext->next;

        if (ext) {
                struct drm_v3d_multi_sync *ms = (struct drm_v3d_multi_sync *) ext;
                ret = v3d_simulator_signal_syncobjs(fd, ms);
        }
        return ret;
}

static int
v3d_simulator_submit_cl_ioctl(int fd, struct drm_v3d_submit_cl *submit)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        int ret;

        ret = v3d_simulator_pin_bos(file, submit);
        if (ret)
                return ret;

        mtx_lock(&sim_state.submit_lock);
        bin_fd = fd;

        v3d_simulator_perfmon_switch(fd, submit->perfmon_id);
        v3d_X_simulator(submit_cl_ioctl)(sim_state.v3d, submit, file->gmp->ofs);

        util_dynarray_foreach(&sim_state.bin_oom, struct v3d_simulator_bo *,
                              sim_bo) {
                v3d_free_simulator_bo(*sim_bo);
        }
        util_dynarray_clear(&sim_state.bin_oom);

        mtx_unlock(&sim_state.submit_lock);

        ret = v3d_simulator_unpin_bos(file, submit);
        if (ret)
                return ret;

        if (submit->flags & DRM_V3D_SUBMIT_EXTENSION) {
                struct drm_v3d_extension *ext = (void *)(uintptr_t)submit->extensions;
                ret = v3d_simulator_process_post_deps(fd, ext);
        }

        return ret;
}

/**
 * Do fixups after a BO has been opened from a handle.
 *
 * This could be done at DRM_IOCTL_GEM_OPEN/DRM_IOCTL_GEM_PRIME_FD_TO_HANDLE
 * time, but we're still using drmPrimeFDToHandle() so we have this helper to
 * be called afterward instead.
 */
void v3d_simulator_open_from_handle(int fd, int handle, uint32_t size)
{
        v3d_create_simulator_bo_for_gem(fd, handle, size);
}

/**
 * Simulated ioctl(fd, DRM_V3D_CREATE_BO) implementation.
 *
 * Making a V3D BO is just a matter of making a corresponding BO on the host.
 */
static int
v3d_simulator_create_bo_ioctl(int fd, struct drm_v3d_create_bo *args)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);

        /* i915 bans dumb create on render nodes, so we have to use their
         * native ioctl in case we're on a render node.
         */
        int ret;
        switch (file->gem_type) {
        case GEM_I915:
        {
                struct drm_i915_gem_create create = {
                        .size = args->size,
                };

                ret = drmIoctl(fd, DRM_IOCTL_I915_GEM_CREATE, &create);

                args->handle = create.handle;
                break;
        }
        case GEM_AMDGPU:
        {
                union drm_amdgpu_gem_create create = { 0 };
                create.in.bo_size = args->size;

                ret = drmIoctl(fd, DRM_IOCTL_AMDGPU_GEM_CREATE, &create);

                args->handle = create.out.handle;
                break;
        }
        default:
        {
                struct drm_mode_create_dumb create = {
                        .width = 128,
                        .bpp = 8,
                        .height = (args->size + 127) / 128,
                };

                ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
                assert(ret != 0 || create.size >= args->size);

                args->handle = create.handle;
        }
        }
        if (ret == 0) {
                struct v3d_simulator_bo *sim_bo =
                        v3d_create_simulator_bo_for_gem(fd, args->handle,
                                                        args->size);

                args->offset = sim_bo->block->ofs;
        }

        return ret;
}

/**
 * Simulated ioctl(fd, DRM_V3D_MMAP_BO) implementation.
 *
 * We've already grabbed the mmap offset when we created the sim bo, so just
 * return it.
 */
static int
v3d_simulator_mmap_bo_ioctl(int fd, struct drm_v3d_mmap_bo *args)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        struct v3d_simulator_bo *sim_bo = v3d_get_simulator_bo(file,
                                                               args->handle);

        args->offset = sim_bo->mmap_offset;

        return 0;
}

static int
v3d_simulator_get_bo_offset_ioctl(int fd, struct drm_v3d_get_bo_offset *args)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        struct v3d_simulator_bo *sim_bo = v3d_get_simulator_bo(file,
                                                               args->handle);

        args->offset = sim_bo->block->ofs;

        return 0;
}

static int
v3d_simulator_gem_close_ioctl(int fd, struct drm_gem_close *args)
{
        /* Free the simulator's internal tracking. */
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        struct v3d_simulator_bo *sim_bo = v3d_get_simulator_bo(file,
                                                               args->handle);

        v3d_free_simulator_bo(sim_bo);

        /* Pass the call on down. */
        return drmIoctl(fd, DRM_IOCTL_GEM_CLOSE, args);
}

static int
v3d_simulator_submit_tfu_ioctl(int fd, struct drm_v3d_submit_tfu *args)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        int ret;

        v3d_simulator_copy_in_handle(file, args->bo_handles[0]);
        v3d_simulator_copy_in_handle(file, args->bo_handles[1]);
        v3d_simulator_copy_in_handle(file, args->bo_handles[2]);
        v3d_simulator_copy_in_handle(file, args->bo_handles[3]);

        ret = v3d_X_simulator(submit_tfu_ioctl)(sim_state.v3d, args);

        v3d_simulator_copy_out_handle(file, args->bo_handles[0]);

        if (ret)
                return ret;

        if (args->flags & DRM_V3D_SUBMIT_EXTENSION) {
                struct drm_v3d_extension *ext = (void *)(uintptr_t)args->extensions;
                ret = v3d_simulator_process_post_deps(fd, ext);
        }

        return ret;
}

static int
v3d_simulator_submit_csd_ioctl(int fd, struct drm_v3d_submit_csd *args)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        uint32_t *bo_handles = (uint32_t *)(uintptr_t)args->bo_handles;
        int ret;

        for (int i = 0; i < args->bo_handle_count; i++)
                v3d_simulator_copy_in_handle(file, bo_handles[i]);

        v3d_simulator_perfmon_switch(fd, args->perfmon_id);

        ret = v3d_X_simulator(submit_csd_ioctl)(sim_state.v3d, args,
                                                file->gmp->ofs);

        for (int i = 0; i < args->bo_handle_count; i++)
                v3d_simulator_copy_out_handle(file, bo_handles[i]);

        if (ret < 0)
                return ret;

        if (args->flags & DRM_V3D_SUBMIT_EXTENSION) {
                struct drm_v3d_extension *ext = (void *)(uintptr_t)args->extensions;
                ret = v3d_simulator_process_post_deps(fd, ext);
        }

        return ret;
}

static void
v3d_rewrite_csd_job_wg_counts_from_indirect(int fd,
					    struct drm_v3d_extension *ext,
					    struct drm_v3d_submit_cpu *args)
{
	struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
	struct drm_v3d_indirect_csd *indirect_csd = (struct drm_v3d_indirect_csd *) ext;
	uint32_t *bo_handles = (uint32_t *)(uintptr_t)args->bo_handles;

	assert(args->bo_handle_count == 1);
	struct v3d_simulator_bo *bo = v3d_get_simulator_bo(file, bo_handles[0]);
	struct v3d_simulator_bo *indirect = v3d_get_simulator_bo(file, indirect_csd->indirect);
	struct drm_v3d_submit_csd *submit = &indirect_csd->submit;

	uint32_t *wg_counts = (uint32_t *) (bo->gem_vaddr + indirect_csd->offset);

	if (wg_counts[0] == 0 || wg_counts[1] == 0 || wg_counts[2] == 0)
		return;

	submit->cfg[0] = wg_counts[0] << V3D_CSD_CFG012_WG_COUNT_SHIFT;
	submit->cfg[1] = wg_counts[1] << V3D_CSD_CFG012_WG_COUNT_SHIFT;
	submit->cfg[2] = wg_counts[2] << V3D_CSD_CFG012_WG_COUNT_SHIFT;
	submit->cfg[4] = DIV_ROUND_UP(indirect_csd->wg_size, 16) *
			(wg_counts[0] * wg_counts[1] * wg_counts[2]) - 1;

	for (int i = 0; i < 3; i++) {
		/* 0xffffffff indicates that the uniform rewrite is not needed */
		if (indirect_csd->wg_uniform_offsets[i] != 0xffffffff) {
			uint32_t uniform_idx = indirect_csd->wg_uniform_offsets[i];
			((uint32_t *) indirect->gem_vaddr)[uniform_idx] = wg_counts[i];
		}
	}

	v3d_simulator_submit_csd_ioctl(fd, submit);
}

static void
v3d_timestamp_query(int fd,
		    struct drm_v3d_extension *ext,
		    struct drm_v3d_submit_cpu *args)
{
	struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
	struct drm_v3d_timestamp_query *timestamp_query = (struct drm_v3d_timestamp_query *) ext;
	uint32_t *bo_handles = (uint32_t *)(uintptr_t)args->bo_handles;
	struct v3d_simulator_bo *bo = v3d_get_simulator_bo(file, bo_handles[0]);
	uint32_t *offsets = (void *)(uintptr_t) timestamp_query->offsets;
	uint32_t *syncs = (void *)(uintptr_t) timestamp_query->syncs;
	uint8_t *value_addr;

	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);

	for (uint32_t i = 0; i < timestamp_query->count; i++) {
		value_addr = ((uint8_t *) bo->sim_vaddr) + offsets[i];
		*((uint64_t*)value_addr) = (i == 0) ? t.tv_sec * 1000000000ull + t.tv_nsec : 0ull;
	}

	drmSyncobjSignal(fd, syncs, timestamp_query->count);
}

static void
v3d_reset_timestamp_queries(int fd,
			    struct drm_v3d_extension *ext,
			    struct drm_v3d_submit_cpu *args)
{
	struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
	struct drm_v3d_reset_timestamp_query *reset = (struct drm_v3d_reset_timestamp_query *) ext;
	uint32_t *bo_handles = (uint32_t *)(uintptr_t)args->bo_handles;
	struct v3d_simulator_bo *bo = v3d_get_simulator_bo(file, bo_handles[0]);
	uint32_t *syncs = (void *)(uintptr_t) reset->syncs;
	uint8_t *base_addr;

	base_addr = ((uint8_t *) bo->sim_vaddr) + reset->offset;
	memset(base_addr, 0, 8 * reset->count);

	drmSyncobjReset(fd, syncs, reset->count);
}

static void
write_to_buffer(void *dst, uint32_t idx, bool do_64bit, uint64_t value)
{
        if (do_64bit) {
                uint64_t *dst64 = (uint64_t *) dst;
                dst64[idx] = value;
        } else {
                uint32_t *dst32 = (uint32_t *) dst;
                dst32[idx] = (uint32_t) value;
        }
}

static void
v3d_copy_query_results(int fd,
		       struct drm_v3d_extension *ext,
		       struct drm_v3d_submit_cpu *args)
{
	struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
	struct drm_v3d_copy_timestamp_query *copy = (struct drm_v3d_copy_timestamp_query *) ext;
	uint32_t *bo_handles = (uint32_t *)(uintptr_t)args->bo_handles;
	struct v3d_simulator_bo *bo = v3d_get_simulator_bo(file, bo_handles[0]);
	struct v3d_simulator_bo *timestamp = v3d_get_simulator_bo(file, bo_handles[1]);
	uint32_t *offsets = (void *)(uintptr_t) copy->offsets;
	uint32_t *syncs = (void *)(uintptr_t) copy->syncs;
	bool available, write_result;
	uint8_t *query_addr;
	uint8_t *data;

	data = ((uint8_t *) bo->sim_vaddr) + copy->offset;

	for (uint32_t i = 0; i < copy->count; i++) {
		available = (drmSyncobjWait(fd, &syncs[i], 1, 0, 0, NULL) == 0);

		write_result = available || copy->do_partial;
		if (write_result) {
			query_addr = ((uint8_t *) timestamp->sim_vaddr) + offsets[i];
			write_to_buffer(data, 0, copy->do_64bit, *((uint64_t *) query_addr));
		}

		if (copy->availability_bit)
			write_to_buffer(data, 1, copy->do_64bit, available ? 1u : 0u);

		data += copy->stride;
	}
}

static void
v3d_reset_performance_queries(int fd,
			      struct drm_v3d_extension *ext,
			      struct drm_v3d_submit_cpu *args)
{
	struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
	struct drm_v3d_reset_performance_query *reset = (struct drm_v3d_reset_performance_query *) ext;
	uint64_t *kperfmon_ids = (void *)(uintptr_t) reset->kperfmon_ids;
	uint32_t *syncs = (void *)(uintptr_t) reset->syncs;
	struct v3d_simulator_perfmon *perfmon;

	for (uint32_t i = 0; i < reset->count; i++) {
		uint32_t *ids = (void *)(uintptr_t) kperfmon_ids[i];

		for (uint32_t j = 0; j < reset->nperfmons; j++) {
			mtx_lock(&sim_state.submit_lock);

			/* Stop the perfmon if it is still active */
			if (ids[j] == file->active_perfid)
				v3d_simulator_perfmon_switch(fd, 0);

			mtx_unlock(&sim_state.submit_lock);

			perfmon = v3d_get_simulator_perfmon(fd, ids[j]);

			if (!perfmon)
				return;

			memset(perfmon->values, 0, perfmon->ncounters * sizeof(uint64_t));
		}
	}

	drmSyncobjReset(fd, syncs, reset->count);
}

static void
v3d_write_performance_query_result(int fd,
				   struct drm_v3d_copy_performance_query *copy,
				   uint32_t *kperfmon_ids,
				   void *data)
{
	struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
	struct v3d_simulator_perfmon *perfmon;
	uint64_t counter_values[V3D_PERFCNT_NUM];

	for (uint32_t i = 0; i < copy->nperfmons; i++) {
		mtx_lock(&sim_state.submit_lock);

		/* Stop the perfmon if it is still active */
		if (kperfmon_ids[i] == file->active_perfid)
			v3d_simulator_perfmon_switch(fd, 0);

		mtx_unlock(&sim_state.submit_lock);

		perfmon = v3d_get_simulator_perfmon(fd, kperfmon_ids[i]);

		if (!perfmon)
			return;

		memcpy(&counter_values[i * DRM_V3D_MAX_PERF_COUNTERS], perfmon->values,
		       perfmon->ncounters * sizeof(uint64_t));
	}

	for (uint32_t i = 0; i < copy->ncounters; i++)
		write_to_buffer(data, i, copy->do_64bit, counter_values[i]);
}

static void
v3d_copy_performance_query(int fd,
			   struct drm_v3d_extension *ext,
			   struct drm_v3d_submit_cpu *args)
{
	struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
	struct drm_v3d_copy_performance_query *copy = (struct drm_v3d_copy_performance_query *) ext;
	uint32_t *bo_handles = (uint32_t *)(uintptr_t)args->bo_handles;
	struct v3d_simulator_bo *bo = v3d_get_simulator_bo(file, bo_handles[0]);
	uint64_t *kperfmon_ids = (void *)(uintptr_t) copy->kperfmon_ids;
	uint32_t *syncs = (void *)(uintptr_t) copy->syncs;
	bool available, write_result;
	uint8_t *data;

	data = ((uint8_t *) bo->sim_vaddr) + copy->offset;

	for (uint32_t i = 0; i < copy->count; i++) {
		/* Although we don't have in_syncs implemented in the simulator,
		 * we don't need to wait for the availability of the syncobjs,
		 * as they are signaled by CL and CSD jobs, which are serialized
		 * by the simulator.
		 */
		available = (drmSyncobjWait(fd, &syncs[i], 1, 0, 0, NULL) == 0);

		write_result = available || copy->do_partial;
		if (write_result) {
			v3d_write_performance_query_result(fd, copy,
							   (void *)(uintptr_t) kperfmon_ids[i],
							   data);
		}

		if (copy->availability_bit) {
			write_to_buffer(data, copy->ncounters, copy->do_64bit,
					available ? 1u : 0u);
		}

		data += copy->stride;
	}
}

static int
v3d_simulator_submit_cpu_ioctl(int fd, struct drm_v3d_submit_cpu *args)
{
	struct drm_v3d_extension *ext = (void *)(uintptr_t)args->extensions;
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        uint32_t *bo_handles = (uint32_t *)(uintptr_t)args->bo_handles;
        int ret = 0;

        for (int i = 0; i < args->bo_handle_count; i++)
                v3d_simulator_copy_in_handle(file, bo_handles[i]);

	while (ext) {
		switch (ext->id) {
		case DRM_V3D_EXT_ID_MULTI_SYNC:
			/* As the simulator serializes the jobs, we don't need
			 * to handle the in_syncs here. The out_syncs are handled
			 * by the end of the ioctl in v3d_simulator_process_post_deps().
			 */
			break;
		case DRM_V3D_EXT_ID_CPU_INDIRECT_CSD:
			v3d_rewrite_csd_job_wg_counts_from_indirect(fd, ext, args);
			break;
		case DRM_V3D_EXT_ID_CPU_TIMESTAMP_QUERY:
			v3d_timestamp_query(fd, ext, args);
			break;
		case DRM_V3D_EXT_ID_CPU_RESET_TIMESTAMP_QUERY:
			v3d_reset_timestamp_queries(fd, ext, args);
			break;
		case DRM_V3D_EXT_ID_CPU_COPY_TIMESTAMP_QUERY:
			v3d_copy_query_results(fd, ext, args);
			break;
		case DRM_V3D_EXT_ID_CPU_RESET_PERFORMANCE_QUERY:
			v3d_reset_performance_queries(fd, ext, args);
			break;
		case DRM_V3D_EXT_ID_CPU_COPY_PERFORMANCE_QUERY:
			v3d_copy_performance_query(fd, ext, args);
			break;
		default:
			fprintf(stderr, "Unknown CPU job 0x%08x\n", (int)ext->id);
			break;
		}

                ext = (void *)(uintptr_t) ext->next;
	}

        for (int i = 0; i < args->bo_handle_count; i++)
                v3d_simulator_copy_out_handle(file, bo_handles[i]);

        if (ret < 0)
                return ret;

        if (args->flags & DRM_V3D_SUBMIT_EXTENSION) {
                ext = (void *)(uintptr_t)args->extensions;
                ret = v3d_simulator_process_post_deps(fd, ext);
        }

        return ret;
}

static int
v3d_simulator_perfmon_create_ioctl(int fd, struct drm_v3d_perfmon_create *args)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);

        if (args->ncounters == 0 ||
            args->ncounters > DRM_V3D_MAX_PERF_COUNTERS)
                return -EINVAL;

        struct v3d_simulator_perfmon *perfmon = rzalloc(file,
                                                        struct v3d_simulator_perfmon);

        perfmon->ncounters = args->ncounters;
        for (int i = 0; i < args->ncounters; i++) {
                if (args->counters[i] >= sim_state.perfcnt_total) {
                        ralloc_free(perfmon);
                        return -EINVAL;
                } else {
                        perfmon->counters[i] = args->counters[i];
                }
        }

        simple_mtx_lock(&sim_state.mutex);
        args->id = perfmons_next_id(file);
        file->perfmons[args->id - 1] = perfmon;
        simple_mtx_unlock(&sim_state.mutex);

        return 0;
}

static int
v3d_simulator_perfmon_destroy_ioctl(int fd, struct drm_v3d_perfmon_destroy *args)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);
        struct v3d_simulator_perfmon *perfmon =
                v3d_get_simulator_perfmon(fd, args->id);

        if (!perfmon)
                return -EINVAL;

        simple_mtx_lock(&sim_state.mutex);
        file->perfmons[args->id - 1] = NULL;
        simple_mtx_unlock(&sim_state.mutex);

        ralloc_free(perfmon);

        return 0;
}

static int
v3d_simulator_perfmon_get_values_ioctl(int fd, struct drm_v3d_perfmon_get_values *args)
{
        struct v3d_simulator_file *file = v3d_get_simulator_file_for_fd(fd);

        mtx_lock(&sim_state.submit_lock);

        /* Stop the perfmon if it is still active */
        if (args->id == file->active_perfid)
                v3d_simulator_perfmon_switch(fd, 0);

        mtx_unlock(&sim_state.submit_lock);

        struct v3d_simulator_perfmon *perfmon =
                v3d_get_simulator_perfmon(fd, args->id);

        if (!perfmon)
                return -EINVAL;

        memcpy((void *)args->values_ptr, perfmon->values, perfmon->ncounters * sizeof(uint64_t));

        return 0;
}

int
v3d_simulator_ioctl(int fd, unsigned long request, void *args)
{
        switch (request) {
        case DRM_IOCTL_V3D_SUBMIT_CL:
                return v3d_simulator_submit_cl_ioctl(fd, args);
        case DRM_IOCTL_V3D_CREATE_BO:
                return v3d_simulator_create_bo_ioctl(fd, args);
        case DRM_IOCTL_V3D_MMAP_BO:
                return v3d_simulator_mmap_bo_ioctl(fd, args);
        case DRM_IOCTL_V3D_GET_BO_OFFSET:
                return v3d_simulator_get_bo_offset_ioctl(fd, args);

        case DRM_IOCTL_V3D_WAIT_BO:
                /* We do all of the v3d rendering synchronously, so we just
                 * return immediately on the wait ioctls.  This ignores any
                 * native rendering to the host BO, so it does mean we race on
                 * front buffer rendering.
                 */
                return 0;

        case DRM_IOCTL_V3D_GET_PARAM:
                return v3d_X_simulator(get_param_ioctl)(sim_state.v3d, args);

        case DRM_IOCTL_GEM_CLOSE:
                return v3d_simulator_gem_close_ioctl(fd, args);

        case DRM_IOCTL_V3D_SUBMIT_TFU:
                return v3d_simulator_submit_tfu_ioctl(fd, args);

        case DRM_IOCTL_V3D_SUBMIT_CSD:
                return v3d_simulator_submit_csd_ioctl(fd, args);

	case DRM_IOCTL_V3D_SUBMIT_CPU:
		return v3d_simulator_submit_cpu_ioctl(fd, args);

        case DRM_IOCTL_V3D_PERFMON_CREATE:
                return v3d_simulator_perfmon_create_ioctl(fd, args);

        case DRM_IOCTL_V3D_PERFMON_DESTROY:
                return v3d_simulator_perfmon_destroy_ioctl(fd, args);

        case DRM_IOCTL_V3D_PERFMON_GET_VALUES:
                return v3d_simulator_perfmon_get_values_ioctl(fd, args);

        case DRM_IOCTL_GEM_OPEN:
        case DRM_IOCTL_GEM_FLINK:
                return drmIoctl(fd, request, args);
        default:
                fprintf(stderr, "Unknown ioctl 0x%08x\n", (int)request);
                abort();
        }
}

uint32_t
v3d_simulator_get_mem_size(void)
{
   return sim_state.mem_size;
}

uint32_t
v3d_simulator_get_mem_free(void)
{
   uint32_t total_free = 0;
   struct mem_block *p;
   for (p = sim_state.heap->next_free; p != sim_state.heap; p = p->next_free)
      total_free += p->size;
   return total_free;
}

static void
v3d_simulator_init_global()
{
        simple_mtx_lock(&sim_state.mutex);
        if (sim_state.refcount++) {
                simple_mtx_unlock(&sim_state.mutex);
                return;
        }

        sim_state.v3d = v3d_hw_auto_new(NULL);
        v3d_hw_alloc_mem(sim_state.v3d, 1024 * 1024 * 1024);
        sim_state.mem_base =
                v3d_hw_get_mem(sim_state.v3d, &sim_state.mem_size,
                               &sim_state.mem);

        /* Allocate from anywhere from 4096 up.  We don't allocate at 0,
         * because for OQs and some other addresses in the HW, 0 means
         * disabled.
         */
        sim_state.heap = u_mmInit(4096, sim_state.mem_size - 4096);

        /* Make a block of 0xd0 at address 0 to make sure we don't screw up
         * and land there.
         */
        struct mem_block *b = u_mmAllocMem(sim_state.heap, 4096, GMP_ALIGN2, 0);
        memset(sim_state.mem + b->ofs - sim_state.mem_base, 0xd0, 4096);

        sim_state.ver = v3d_hw_get_version(sim_state.v3d);

        simple_mtx_unlock(&sim_state.mutex);

        sim_state.fd_map =
                _mesa_hash_table_create(NULL,
                                        _mesa_hash_pointer,
                                        _mesa_key_pointer_equal);

        util_dynarray_init(&sim_state.bin_oom, NULL);

        v3d_X_simulator(init_regs)(sim_state.v3d);
        v3d_X_simulator(get_perfcnt_total)(&sim_state.perfcnt_total);
}

struct v3d_simulator_file *
v3d_simulator_init(int fd)
{
        v3d_simulator_init_global();

        struct v3d_simulator_file *sim_file = rzalloc(NULL, struct v3d_simulator_file);

        drmVersionPtr version = drmGetVersion(fd);
        if (version && strncmp(version->name, "i915", version->name_len) == 0)
                sim_file->gem_type = GEM_I915;
        else if (version && strncmp(version->name, "amdgpu", version->name_len) == 0)
                sim_file->gem_type = GEM_AMDGPU;
        else
                sim_file->gem_type = GEM_DUMB;
        drmFreeVersion(version);

        sim_file->bo_map =
                _mesa_hash_table_create(sim_file,
                                        _mesa_hash_pointer,
                                        _mesa_key_pointer_equal);

        simple_mtx_lock(&sim_state.mutex);
        _mesa_hash_table_insert(sim_state.fd_map, int_to_key(fd + 1),
                                sim_file);
        simple_mtx_unlock(&sim_state.mutex);

        sim_file->gmp = u_mmAllocMem(sim_state.heap, 8096, GMP_ALIGN2, 0);
        sim_file->gmp_vaddr = (sim_state.mem + sim_file->gmp->ofs -
                               sim_state.mem_base);
        memset(sim_file->gmp_vaddr, 0, 8096);

        return sim_file;
}

void
v3d_simulator_destroy(struct v3d_simulator_file *sim_file)
{
        simple_mtx_lock(&sim_state.mutex);
        if (!--sim_state.refcount) {
                _mesa_hash_table_destroy(sim_state.fd_map, NULL);
                util_dynarray_fini(&sim_state.bin_oom);
                u_mmDestroy(sim_state.heap);
                /* No memsetting the struct, because it contains the mutex. */
                sim_state.mem = NULL;
        }
        simple_mtx_unlock(&sim_state.mutex);
        ralloc_free(sim_file);
}

#endif /* USE_V3D_SIMULATOR */
