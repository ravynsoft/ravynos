/*
 * Copyright 2023 Advanced Micro Devices, Inc.
 * All Rights Reserved.
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

#include "ac_shadowed_regs.h"
#include "radv_cs.h"
#include "radv_debug.h"
#include "radv_private.h"
#include "sid.h"

static void
radv_set_context_reg_array(struct radeon_cmdbuf *cs, unsigned reg, unsigned num, const uint32_t *values)
{
   radeon_set_context_reg_seq(cs, reg, num);
   radeon_emit_array(cs, values, num);
}

VkResult
radv_create_shadow_regs_preamble(const struct radv_device *device, struct radv_queue_state *queue_state)
{
   struct radeon_winsys *ws = device->ws;
   const struct radeon_info *info = &device->physical_device->rad_info;
   VkResult result;

   struct radeon_cmdbuf *cs = ws->cs_create(ws, AMD_IP_GFX, false);
   if (!cs)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   radeon_check_space(ws, cs, 256);

   /* allocate memory for queue_state->shadowed_regs where register states are saved */
   result = ws->buffer_create(ws, SI_SHADOWED_REG_BUFFER_SIZE, 4096, RADEON_DOMAIN_VRAM,
                              RADEON_FLAG_ZERO_VRAM | RADEON_FLAG_NO_INTERPROCESS_SHARING, RADV_BO_PRIORITY_SCRATCH, 0,
                              &queue_state->shadowed_regs);
   if (result != VK_SUCCESS)
      goto fail;

   /* fill the cs for shadow regs preamble ib that starts the register shadowing */
   ac_create_shadowing_ib_preamble(info, (pm4_cmd_add_fn)&radeon_emit, cs, queue_state->shadowed_regs->va,
                                   device->pbb_allowed);

   while (cs->cdw & 7) {
      if (info->gfx_ib_pad_with_type2)
         radeon_emit(cs, PKT2_NOP_PAD);
      else
         radeon_emit(cs, PKT3_NOP_PAD);
   }

   result = ws->buffer_create(
      ws, cs->cdw * 4, 4096, ws->cs_domain(ws),
      RADEON_FLAG_CPU_ACCESS | RADEON_FLAG_NO_INTERPROCESS_SHARING | RADEON_FLAG_READ_ONLY | RADEON_FLAG_GTT_WC,
      RADV_BO_PRIORITY_CS, 0, &queue_state->shadow_regs_ib);
   if (result != VK_SUCCESS)
      goto fail_ib_buffer;

   /* copy the cs to queue_state->shadow_regs_ib. This will be the first preamble ib
    * added in radv_update_preamble_cs.
    */
   void *map = ws->buffer_map(queue_state->shadow_regs_ib);
   if (!map) {
      result = VK_ERROR_MEMORY_MAP_FAILED;
      goto fail_map;
   }
   memcpy(map, cs->buf, cs->cdw * 4);
   queue_state->shadow_regs_ib_size_dw = cs->cdw;

   ws->buffer_unmap(queue_state->shadow_regs_ib);
   ws->cs_destroy(cs);
   return VK_SUCCESS;
fail_map:
   ws->buffer_destroy(ws, queue_state->shadow_regs_ib);
   queue_state->shadow_regs_ib = NULL;
fail_ib_buffer:
   ws->buffer_destroy(ws, queue_state->shadowed_regs);
   queue_state->shadowed_regs = NULL;
fail:
   ws->cs_destroy(cs);
   return result;
}

void
radv_destroy_shadow_regs_preamble(struct radv_queue_state *queue_state, struct radeon_winsys *ws)
{
   if (queue_state->shadow_regs_ib)
      ws->buffer_destroy(ws, queue_state->shadow_regs_ib);
   if (queue_state->shadowed_regs)
      ws->buffer_destroy(ws, queue_state->shadowed_regs);
}

void
radv_emit_shadow_regs_preamble(struct radeon_cmdbuf *cs, const struct radv_device *device,
                               struct radv_queue_state *queue_state)
{
   struct radeon_winsys *ws = device->ws;

   ws->cs_execute_ib(cs, queue_state->shadow_regs_ib, 0, queue_state->shadow_regs_ib_size_dw & 0xffff, false);

   radv_cs_add_buffer(device->ws, cs, queue_state->shadowed_regs);
   radv_cs_add_buffer(device->ws, cs, queue_state->shadow_regs_ib);
}

/* radv_init_shadowed_regs_buffer_state() will be called once from radv_queue_init(). This
 * initializes the shadowed_regs buffer to good state */
VkResult
radv_init_shadowed_regs_buffer_state(const struct radv_device *device, struct radv_queue *queue)
{
   const struct radeon_info *info = &device->physical_device->rad_info;
   struct radeon_winsys *ws = device->ws;
   struct radeon_cmdbuf *cs;
   VkResult result;

   cs = ws->cs_create(ws, AMD_IP_GFX, false);
   if (!cs)
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;

   radeon_check_space(ws, cs, 768);

   radv_emit_shadow_regs_preamble(cs, device, &queue->state);
   ac_emulate_clear_state(info, cs, radv_set_context_reg_array);

   result = ws->cs_finalize(cs);
   if (result == VK_SUCCESS) {
      if (!radv_queue_internal_submit(queue, cs))
         result = VK_ERROR_UNKNOWN;
   }

   ws->cs_destroy(cs);
   return result;
}
