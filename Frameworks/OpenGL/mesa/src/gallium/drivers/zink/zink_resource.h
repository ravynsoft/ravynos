/*
 * Copyright 2018 Collabora Ltd.
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ZINK_RESOURCE_H
#define ZINK_RESOURCE_H

#include "zink_types.h"

#define ZINK_MAP_TEMPORARY (PIPE_MAP_DRV_PRV << 0)
#define ZINK_BIND_DESCRIPTOR (1u << 27)
#define ZINK_BIND_MUTABLE (1u << 28)
#define ZINK_BIND_DMABUF (1u << 29)
#define ZINK_BIND_TRANSIENT (1u << 30) //transient fb attachment
#define ZINK_BIND_VIDEO (1u << 31)

#ifdef __cplusplus
extern "C" {
#endif

bool
zink_screen_resource_init(struct pipe_screen *pscreen);

void
zink_context_resource_init(struct pipe_context *pctx);
void
zink_screen_buffer_unmap(struct pipe_screen *pscreen, struct pipe_transfer *ptrans);
void
zink_get_depth_stencil_resources(struct pipe_resource *res,
                                 struct zink_resource **out_z,
                                 struct zink_resource **out_s);
VkMappedMemoryRange
zink_resource_init_mem_range(struct zink_screen *screen, struct zink_resource_object *obj, VkDeviceSize offset, VkDeviceSize size);
void
zink_resource_setup_transfer_layouts(struct zink_context *ctx, struct zink_resource *src, struct zink_resource *dst);

void
zink_destroy_resource_object(struct zink_screen *screen, struct zink_resource_object *resource_object);

void
debug_describe_zink_resource_object(char *buf, const struct zink_resource_object *ptr);

static inline void
zink_resource_object_reference(struct zink_screen *screen,
                             struct zink_resource_object **dst,
                             struct zink_resource_object *src)
{
   struct zink_resource_object *old_dst = dst ? *dst : NULL;

   if (pipe_reference_described(old_dst ? &old_dst->reference : NULL, &src->reference,
                                (debug_reference_descriptor)debug_describe_zink_resource_object))
      zink_destroy_resource_object(screen, old_dst);
   if (dst) *dst = src;
}

bool
zink_resource_object_init_storage(struct zink_context *ctx, struct zink_resource *res);
bool
zink_resource_object_init_mutable(struct zink_context *ctx, struct zink_resource *res);

VkDeviceAddress
zink_resource_get_address(struct zink_screen *screen, struct zink_resource *res);

static ALWAYS_INLINE bool
zink_resource_has_binds(const struct zink_resource *res)
{
   return res->all_binds > 0;
}

static ALWAYS_INLINE  bool
zink_is_swapchain(const struct zink_resource *res)
{
   return res->swapchain;
}

bool
zink_resource_copy_box_intersects(struct zink_resource *res, unsigned level, const struct pipe_box *box);
void
zink_resource_copy_box_add(struct zink_context *ctx, struct zink_resource *res, unsigned level, const struct pipe_box *box);
void
zink_resource_copies_reset(struct zink_resource *res);

#include "zink_batch.h"
#include "zink_bo.h"
#include "zink_kopper.h"

static inline bool
zink_resource_usage_is_unflushed(const struct zink_resource *res)
{
   return zink_bo_has_unflushed_usage(res->obj->bo);
}

static inline bool
zink_resource_usage_is_unflushed_write(const struct zink_resource *res)
{
   return zink_batch_usage_is_unflushed(res->obj->bo->writes.u);
}


static inline bool
zink_resource_usage_matches(const struct zink_resource *res, const struct zink_batch_state *bs)
{
   return zink_bo_usage_matches(res->obj->bo, bs);
}

static inline bool
zink_resource_has_usage(const struct zink_resource *res)
{
   return zink_bo_has_usage(res->obj->bo);
}

static inline bool
zink_resource_has_unflushed_usage(const struct zink_resource *res)
{
   return zink_bo_has_unflushed_usage(res->obj->bo);
}

static inline bool
zink_resource_usage_check_completion(struct zink_screen *screen, struct zink_resource *res, enum zink_resource_access access)
{
   return zink_bo_usage_check_completion(screen, res->obj->bo, access);
}

static inline bool
zink_resource_usage_check_completion_fast(struct zink_screen *screen, struct zink_resource *res, enum zink_resource_access access)
{
   return zink_bo_usage_check_completion_fast(screen, res->obj->bo, access);
}

static inline void
zink_resource_usage_try_wait(struct zink_context *ctx, struct zink_resource *res, enum zink_resource_access access)
{
   zink_bo_usage_try_wait(ctx, res->obj->bo, access);
}

static inline void
zink_resource_usage_wait(struct zink_context *ctx, struct zink_resource *res, enum zink_resource_access access)
{
   zink_bo_usage_wait(ctx, res->obj->bo, access);
}

static inline void
zink_resource_usage_set(struct zink_resource *res, struct zink_batch_state *bs, bool write)
{
   zink_bo_usage_set(res->obj->bo, bs, write);
   res->obj->unsync_access = false;
}

static inline bool
zink_resource_object_usage_unset(struct zink_resource_object *obj, struct zink_batch_state *bs)
{
   return zink_bo_usage_unset(obj->bo, bs);
}

static inline void
zink_batch_resource_usage_set(struct zink_batch *batch, struct zink_resource *res, bool write, bool is_buffer)
{
   if (!is_buffer) {
      if (res->obj->dt) {
         VkSemaphore acquire = zink_kopper_acquire_submit(zink_screen(batch->state->ctx->base.screen), res);
         if (acquire)
            util_dynarray_append(&batch->state->acquires, VkSemaphore, acquire);
      }
      if (write) {
         if (!res->valid && res->fb_bind_count)
            batch->state->ctx->rp_loadop_changed = true;
         res->valid = true;
      }
   }
   zink_resource_usage_set(res, batch->state, write);

   batch->has_work = true;
}

void
zink_debug_mem_print_stats(struct zink_screen *screen);

#ifdef __cplusplus
}
#endif

#endif
