/*
 * Copyright © 2023 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PVR_SPM_H
#define PVR_SPM_H

/**
 * \file pvr_spm.h
 *
 * \brief Smart Parameter Management.
 *
 * With large amounts of geometry the device can run out of Parameter Buffer
 * (PB) as no more free pages are left in the freelist to allow the PB to grow.
 * In such cases the render is split into multiple partial renders (PRs) to fit
 * within the memory constraints. Each PR produces intermediary results until
 * they have all completed, producing the final scene equivalent to what would
 * have been produced by the original render.
 *
 * SPM comprises all the necessary work required of the driver to manage the PB.
 */

#include <stdint.h>
#include <vulkan/vulkan_core.h>

#include "hwdef/rogue_hw_defs.h"
#include "pvr_limits.h"
#include "util/simple_mtx.h"

struct pvr_bo;
struct pvr_device;
struct pvr_framebuffer;
struct pvr_render_pass;
struct pvr_renderpass_hwsetup_render;
struct pvr_spm_scratch_buffer;

struct pvr_spm_scratch_buffer_store {
   simple_mtx_t mtx;
   struct pvr_spm_scratch_buffer *head_ref;
};

struct pvr_spm_eot_state {
   uint64_t pbe_reg_words[PVR_MAX_COLOR_ATTACHMENTS]
                         [ROGUE_NUM_PBESTATE_REG_WORDS];

   struct pvr_suballoc_bo *usc_eot_program;

   /* TODO: Make this struct pvr_pds_upload? It would pull in pvr_private.h
    * though which causes a cycle since that includes pvr_spm.h .
    */
   /* This is only the data section upload. The code was uploaded at device
    * creation.
    */
   uint64_t pixel_event_program_data_offset;
   struct pvr_suballoc_bo *pixel_event_program_data_upload;
};

struct pvr_spm_bgobj_state {
   struct pvr_bo *consts_buffer;

   /* TODO: Make this struct pvr_pds_upload? It would pull in pvr_private.h
    * though which causes a cycle since that includes pvr_spm.h .
    */
   struct pvr_suballoc_bo *pds_texture_data_upload;

   uint64_t pds_reg_values[ROGUE_NUM_CR_PDS_BGRND_WORDS];
};

void pvr_spm_init_scratch_buffer_store(struct pvr_device *device);
void pvr_spm_finish_scratch_buffer_store(struct pvr_device *device);

/* A scratch buffer is required in various situations:
 *
 *  - An MSAA workload which needs saving to a larger buffer than the output for
 *    PRs.
 *  - To store transient results during a PR with read only attachments (i.e.
 *    VK_ATTACHMENT_STORE_OP_NONE, not currently supported) or lazily allocated
 *    attachments with no backing.
 */
uint64_t
pvr_spm_scratch_buffer_calc_required_size(const struct pvr_render_pass *pass,
                                          uint32_t framebuffer_width,
                                          uint32_t framebuffer_height);
VkResult pvr_spm_scratch_buffer_get_buffer(
   struct pvr_device *device,
   uint64_t size,
   struct pvr_spm_scratch_buffer **const buffer_out);
void pvr_spm_scratch_buffer_release(struct pvr_device *device,
                                    struct pvr_spm_scratch_buffer *buffer);

/* The SPM load programs are needed for the SPM background object load op. */
VkResult pvr_device_init_spm_load_state(struct pvr_device *device);
void pvr_device_finish_spm_load_state(struct pvr_device *device);

VkResult
pvr_spm_init_eot_state(struct pvr_device *device,
                       struct pvr_spm_eot_state *spm_eot_state,
                       const struct pvr_framebuffer *framebuffer,
                       const struct pvr_renderpass_hwsetup_render *hw_render,
                       uint32_t *emit_count_out);
void pvr_spm_finish_eot_state(struct pvr_device *device,
                              struct pvr_spm_eot_state *spm_eot_state);

VkResult
pvr_spm_init_bgobj_state(struct pvr_device *device,
                         struct pvr_spm_bgobj_state *spm_bgobj_state,
                         const struct pvr_framebuffer *framebuffer,
                         const struct pvr_renderpass_hwsetup_render *hw_render,
                         uint32_t emit_count);
void pvr_spm_finish_bgobj_state(struct pvr_device *device,
                                struct pvr_spm_bgobj_state *spm_bgobj_state);

#endif /* PVR_SPM_H */
