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

#ifndef V3D_SCREEN_H
#define V3D_SCREEN_H

#include "pipe/p_screen.h"
#include "renderonly/renderonly.h"
#include "util/u_thread.h"
#include "frontend/drm_driver.h"
#include "util/disk_cache.h"
#include "util/list.h"
#include "util/slab.h"
#include "broadcom/common/v3d_debug.h"
#include "broadcom/common/v3d_device_info.h"

struct v3d_bo;

/* These are tunable parameters in the HW design, but all the V3D
 * implementations agree.
 */
#define V3D_UIFCFG_BANKS 8
#define V3D_UIFCFG_PAGE_SIZE 4096
#define V3D_UIFCFG_XOR_VALUE (1 << 4)
#define V3D_PAGE_CACHE_SIZE (V3D_UIFCFG_PAGE_SIZE * V3D_UIFCFG_BANKS)
#define V3D_UBLOCK_SIZE 64
#define V3D_UIFBLOCK_SIZE (4 * V3D_UBLOCK_SIZE)
#define V3D_UIFBLOCK_ROW_SIZE (4 * V3D_UIFBLOCK_SIZE)

struct v3d_simulator_file;

struct v3d_screen {
        struct pipe_screen base;
        struct renderonly *ro;
        int fd;

        struct v3d_device_info devinfo;

        const char *name;

        struct slab_parent_pool transfer_pool;

        struct v3d_bo_cache {
                /** List of struct v3d_bo freed, by age. */
                struct list_head time_list;
                /** List of struct v3d_bo freed, per size, by age. */
                struct list_head *size_list;
                uint32_t size_list_size;

                mtx_t lock;
        } bo_cache;

        const struct v3d_compiler *compiler;

        struct hash_table *bo_handles;
        mtx_t bo_handles_mutex;

        uint32_t bo_size;
        uint32_t bo_count;
        uint32_t prim_types;

        bool has_csd;
        bool has_cache_flush;
        bool has_perfmon;
        bool nonmsaa_texture_size_limit;

        struct v3d_simulator_file *sim_file;

#ifdef ENABLE_SHADER_CACHE
        struct disk_cache *disk_cache;
#endif
};

static inline struct v3d_screen *
v3d_screen(struct pipe_screen *screen)
{
        return (struct v3d_screen *)screen;
}

struct pipe_screen *v3d_screen_create(int fd,
                                      const struct pipe_screen_config *config,
                                      struct renderonly *ro);

void
v3d_fence_screen_init(struct v3d_screen *screen);

#ifdef ENABLE_SHADER_CACHE
void
v3d_disk_cache_init(struct v3d_screen *screen);
#endif

#endif /* V3D_SCREEN_H */
