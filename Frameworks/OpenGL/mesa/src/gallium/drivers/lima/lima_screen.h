/*
 * Copyright (c) 2017-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef H_LIMA_SCREEN
#define H_LIMA_SCREEN

#include <stdio.h>

#include "util/slab.h"
#include "util/list.h"
#include "util/disk_cache.h"
#include "util/u_thread.h"

#include "pipe/p_screen.h"

#define LIMA_DEBUG_GP             (1 << 0)
#define LIMA_DEBUG_PP             (1 << 1)
#define LIMA_DEBUG_DUMP           (1 << 2)
#define LIMA_DEBUG_SHADERDB       (1 << 3)
#define LIMA_DEBUG_NO_BO_CACHE    (1 << 4)
#define LIMA_DEBUG_BO_CACHE       (1 << 5)
#define LIMA_DEBUG_NO_TILING      (1 << 6)
#define LIMA_DEBUG_NO_GROW_HEAP   (1 << 7)
#define LIMA_DEBUG_SINGLE_JOB     (1 << 8)
#define LIMA_DEBUG_PRECOMPILE     (1 << 9)
#define LIMA_DEBUG_DISK_CACHE     (1 << 10)
#define LIMA_DEBUG_NO_BLIT        (1 << 11)

extern uint32_t lima_debug;
extern int lima_ctx_num_plb;
extern int lima_plb_max_blk;
extern int lima_ppir_force_spilling;
extern int lima_plb_pp_stream_cache_size;

struct ra_regs;

#define MIN_BO_CACHE_BUCKET (12) /* 2^12 = 4KB */
#define MAX_BO_CACHE_BUCKET (22) /* 2^22 = 4MB */

#define NR_BO_CACHE_BUCKETS (MAX_BO_CACHE_BUCKET - MIN_BO_CACHE_BUCKET + 1)

/* const0 1 0 0 -1.67773, mov.v0 $0 ^const0.xxxx, stop */
#define PP_CLEAR_PROGRAM \
   0x00020425, 0x0000000c, 0x01e007cf, 0xb0000000, \
   0x000005f5, 0x00000000, 0x00000000, 0x00000000, \

struct lima_screen {
   struct pipe_screen base;
   struct renderonly *ro;

   int fd;
   int gpu_type;
   int num_pp;
   uint32_t plb_max_blk;

   /* bo table */
   mtx_t bo_table_lock;
   mtx_t bo_cache_lock;
   struct hash_table *bo_handles;
   struct hash_table *bo_flink_names;
   struct list_head bo_cache_buckets[NR_BO_CACHE_BUCKETS];
   struct list_head bo_cache_time;

   struct slab_parent_pool transfer_pool;

   struct ra_regs *pp_ra;

   struct lima_bo *pp_buffer;
   #define pp_frame_rsw_offset       0x0000
   #define pp_clear_program_offset   0x0040
   #define pp_reload_program_offset  0x0080
   #define pp_shared_index_offset    0x00c0
   #define pp_clear_gl_pos_offset    0x0100
   #define pp_buffer_size            0x1000

   bool has_growable_heap_buffer;

   struct disk_cache *disk_cache;
};

static inline struct lima_screen *
lima_screen(struct pipe_screen *pscreen)
{
   return (struct lima_screen *)pscreen;
}

struct pipe_screen *
lima_screen_create(int fd, const struct pipe_screen_config *config,
                   struct renderonly *ro);

#endif
