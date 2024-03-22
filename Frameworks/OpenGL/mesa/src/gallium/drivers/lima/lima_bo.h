/*
 * Copyright (C) 2018-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef H_LIMA_BO
#define H_LIMA_BO

#include <stdbool.h>
#include <stdint.h>

#include "util/u_atomic.h"
#include "util/list.h"

struct lima_bo {
   struct lima_screen *screen;
   struct list_head time_list;
   struct list_head size_list;
   int refcnt;
   bool cacheable;
   time_t free_time;

   uint32_t size;
   uint32_t flags;
   uint32_t handle;
   uint64_t offset;
   uint32_t flink_name;

   void *map;
   uint32_t va;
};

bool lima_bo_table_init(struct lima_screen *screen);
void lima_bo_table_fini(struct lima_screen *screen);
bool lima_bo_cache_init(struct lima_screen *screen);
void lima_bo_cache_fini(struct lima_screen *screen);

struct lima_bo *lima_bo_create(struct lima_screen *screen, uint32_t size,
                               uint32_t flags);
void lima_bo_unreference(struct lima_bo *bo);

static inline void lima_bo_reference(struct lima_bo *bo)
{
   p_atomic_inc(&bo->refcnt);
}

void *lima_bo_map(struct lima_bo *bo);
void lima_bo_unmap(struct lima_bo *bo);

bool lima_bo_export(struct lima_bo *bo, struct winsys_handle *handle);
struct lima_bo *lima_bo_import(struct lima_screen *screen,
                               struct winsys_handle *handle);

bool lima_bo_wait(struct lima_bo *bo, uint32_t op, uint64_t timeout_ns);

#endif
