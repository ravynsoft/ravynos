/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * @file iris_binder.c
 *
 * Shader programs refer to most resources via integer handles.  These are
 * indexes (BTIs) into a "Binding Table", which is simply a list of pointers
 * to SURFACE_STATE entries.  Each shader stage has its own binding table,
 * set by the 3DSTATE_BINDING_TABLE_POINTERS_* commands.  We stream out
 * binding tables dynamically, storing them in special BOs we call "binders."
 *
 * Unfortunately, the hardware designers made 3DSTATE_BINDING_TABLE_POINTERS
 * only accept a 16-bit pointer.  This means that all binding tables have to
 * live within the 64kB range starting at Surface State Base Address.  (The
 * actual SURFACE_STATE entries can live anywhere in the 4GB zone, as the
 * binding table entries are full 32-bit pointers.)
 *
 * To handle this, we split a 4GB region of VMA into two memory zones.
 * IRIS_MEMZONE_BINDER is a 1GB region at the bottom able to hold a few
 * binder BOs.  IRIS_MEMZONE_SURFACE contains the rest of the 4GB, and is
 * always at a higher address than the binders.  This allows us to program
 * Surface State Base Address to the binder BO's address, and offset the
 * values in the binding table to account for the base not starting at the
 * beginning of the 4GB region.
 *
 * This does mean that we have to emit STATE_BASE_ADDRESS and stall when
 * we run out of space in the binder, which hopefully won't happen too often.
 */

#include <stdlib.h>
#include "util/u_math.h"
#include "iris_binder.h"
#include "iris_bufmgr.h"
#include "iris_context.h"

static bool
binder_has_space(struct iris_binder *binder, unsigned size)
{
   return binder->insert_point + size <= binder->size;
}

static void
binder_realloc(struct iris_context *ice)
{
   struct iris_screen *screen = (void *) ice->ctx.screen;
   struct iris_bufmgr *bufmgr = screen->bufmgr;
   struct iris_binder *binder = &ice->state.binder;

   if (binder->bo)
      iris_bo_unreference(binder->bo);

   binder->bo = iris_bo_alloc(bufmgr, "binder", binder->size, binder->alignment,
                              IRIS_MEMZONE_BINDER, BO_ALLOC_PLAIN);
   binder->map = iris_bo_map(NULL, binder->bo, MAP_WRITE);

   /* Avoid using offset 0 - tools consider it NULL. */
   binder->insert_point = binder->alignment;

   /* Allocating a new binder requires changing Surface State Base Address,
    * which also invalidates all our previous binding tables - each entry
    * in those tables is an offset from the old base.
    *
    * We do this here so that iris_binder_reserve_3d correctly gets a new
    * larger total_size when making the updated reservation.
    *
    * Gfx11+ simply updates the binding table pool address instead, which
    * means the old binding table's contents are still valid.  Nevertheless,
    * it still lives in the old BO, so we'd at least need to copy it to the
    * new one.  Instead, we just flag it dirty and re-emit it anyway.
    */
   ice->state.dirty |= IRIS_DIRTY_RENDER_BUFFER;
   ice->state.stage_dirty |= IRIS_ALL_STAGE_DIRTY_BINDINGS;
}

static uint32_t
binder_insert(struct iris_binder *binder, unsigned size)
{
   uint32_t offset = binder->insert_point;

   binder->insert_point =
      align(binder->insert_point + size, binder->alignment);

   return offset;
}

/**
 * Reserve a block of space in the binder, given the raw size in bytes.
 */
uint32_t
iris_binder_reserve(struct iris_context *ice,
                    unsigned size)
{
   struct iris_binder *binder = &ice->state.binder;

   if (!binder_has_space(binder, size))
      binder_realloc(ice);

   assert(size > 0);
   return binder_insert(binder, size);
}

/**
 * Reserve and record binder space for 3D pipeline shader stages.
 *
 * Note that you must actually populate the new binding tables after
 * calling this command - the new area is uninitialized.
 */
void
iris_binder_reserve_3d(struct iris_context *ice)
{
   struct iris_compiled_shader **shaders = ice->shaders.prog;
   struct iris_binder *binder = &ice->state.binder;
   unsigned sizes[MESA_SHADER_STAGES] = {};
   unsigned total_size;

   /* If nothing is dirty, skip all this. */
   if (!(ice->state.dirty & IRIS_DIRTY_RENDER_BUFFER) &&
       !(ice->state.stage_dirty & IRIS_ALL_STAGE_DIRTY_BINDINGS_FOR_RENDER))
      return;

   /* Get the binding table sizes for each stage */
   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (!shaders[stage])
         continue;

      /* Round up the size so our next table has an aligned starting offset */
      sizes[stage] = align(shaders[stage]->bt.size_bytes, binder->alignment);
   }

   /* Make space for the new binding tables...this may take two tries. */
   while (true) {
      total_size = 0;
      for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
         if (ice->state.stage_dirty & (IRIS_STAGE_DIRTY_BINDINGS_VS << stage))
            total_size += sizes[stage];
      }

      assert(total_size < binder->size);

      if (total_size == 0)
         return;

      if (binder_has_space(binder, total_size))
         break;

      /* It didn't fit.  Allocate a new buffer and try again.  Note that
       * this will flag all bindings dirty, which may increase total_size
       * on the next iteration.
       */
      binder_realloc(ice);
   }

   /* Assign space and record the new binding table offsets. */
   uint32_t offset = binder_insert(binder, total_size);

   for (int stage = 0; stage <= MESA_SHADER_FRAGMENT; stage++) {
      if (ice->state.stage_dirty & (IRIS_STAGE_DIRTY_BINDINGS_VS << stage)) {
         binder->bt_offset[stage] = sizes[stage] > 0 ? offset : 0;
         iris_record_state_size(ice->state.sizes,
                                binder->bo->address + offset, sizes[stage]);
         offset += sizes[stage];
      }
   }
}

void
iris_binder_reserve_compute(struct iris_context *ice)
{
   if (!(ice->state.stage_dirty & IRIS_STAGE_DIRTY_BINDINGS_CS))
      return;

   struct iris_binder *binder = &ice->state.binder;
   struct iris_compiled_shader *shader =
      ice->shaders.prog[MESA_SHADER_COMPUTE];

   unsigned size = shader->bt.size_bytes;

   if (size == 0)
      return;

   binder->bt_offset[MESA_SHADER_COMPUTE] = iris_binder_reserve(ice, size);
}

void
iris_init_binder(struct iris_context *ice)
{
   struct iris_screen *screen = (void *) ice->ctx.screen;
   const struct intel_device_info *devinfo = screen->devinfo;

   memset(&ice->state.binder, 0, sizeof(struct iris_binder));

   /* We use different binding table pointer formats on various generations.
    *
    * - The 20:5 format gives us an alignment of 32B and max size of 1024kB.
    * - The 18:8 format gives us an alignment of 256B and max size of 512kB.
    * - The 15:5 format gives us an alignment of 32B and max size of 64kB.
    *
    * XeHP and later use the 20:5 format.  Icelake and Tigerlake use 18:8
    * in iris, but can use 15:5 if desired,  Older platforms require 15:5.
    */
   if (devinfo->verx10 >= 125) {
      ice->state.binder.alignment = 32;
      ice->state.binder.size = 1024 * 1024;
   } else if (devinfo->ver >= 11) {
      ice->state.binder.alignment = 256;
      ice->state.binder.size = 512 * 1024;
   } else {
      ice->state.binder.alignment = 32;
      ice->state.binder.size = 64 * 1024;
   }

   binder_realloc(ice);
}

void
iris_destroy_binder(struct iris_binder *binder)
{
   iris_bo_unreference(binder->bo);
}
