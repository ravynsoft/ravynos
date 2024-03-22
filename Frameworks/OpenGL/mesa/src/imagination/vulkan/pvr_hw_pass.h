/*
 * Copyright © 2022 Imagination Technologies Ltd.
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

#ifndef PVR_HW_PASS_H
#define PVR_HW_PASS_H

#include <stdbool.h>
#include <stdint.h>
#include <vulkan/vulkan.h>

struct pvr_device;
struct pvr_render_pass;

/* Specifies the location of render target writes. */
enum usc_mrt_resource_type {
   USC_MRT_RESOURCE_TYPE_INVALID = 0, /* explicitly treat 0 as invalid. */
   USC_MRT_RESOURCE_TYPE_OUTPUT_REG,
   USC_MRT_RESOURCE_TYPE_MEMORY,
};

enum pvr_resolve_type {
   PVR_RESOLVE_TYPE_INVALID = 0, /* explicitly treat 0 as invalid. */
   PVR_RESOLVE_TYPE_PBE,
   PVR_RESOLVE_TYPE_TRANSFER,
};

enum pvr_renderpass_hwsetup_input_access {
   /* The attachment must be loaded using a texture sample. */
   PVR_RENDERPASS_HWSETUP_INPUT_ACCESS_OFFCHIP,
   /* The attachment can be loaded from an output register or tile buffer. */
   PVR_RENDERPASS_HWSETUP_INPUT_ACCESS_ONCHIP,
   /* As _ONCHIP but the attachment is the result of a Z replicate in the same
    * subpass.
    */
   PVR_RENDERPASS_HWSETUP_INPUT_ACCESS_ONCHIP_ZREPLICATE,
};

#define PVR_USC_RENDER_TARGET_MAXIMUM_SIZE_IN_DWORDS (4)

struct usc_mrt_desc {
   /* Size (in bytes) of the intermediate storage required for each pixel in the
    * render target.
    */
   uint32_t intermediate_size;

   /* Mask of the bits from each dword which are read by the PBE. */
   uint32_t valid_mask[PVR_USC_RENDER_TARGET_MAXIMUM_SIZE_IN_DWORDS];

   /* Higher number = higher priority. Used to decide which render targets get
    * allocated dedicated output registers.
    */
   uint32_t priority;
};

struct usc_mrt_resource {
   /* Input description of render target. */
   struct usc_mrt_desc mrt_desc;

   /* Resource type allocated for render target. */
   enum usc_mrt_resource_type type;

   /* Intermediate pixel size (in bytes). */
   uint32_t intermediate_size;

   union {
      /* If type == USC_MRT_RESOURCE_TYPE_OUTPUT_REG. */
      struct {
         /* The output register to use. */
         uint32_t output_reg;

         /* The offset in bytes into the output register. */
         uint32_t offset;
      } reg;

      /* If type == USC_MRT_RESOURCE_TYPE_MEMORY. */
      struct {
         /* The index of the tile buffer to use. */
         uint32_t tile_buffer;

         /* The offset in dwords within the tile buffer. */
         uint32_t offset_dw;
      } mem;
   };
};

struct usc_mrt_setup {
   /* Number of render targets present. */
   uint32_t num_render_targets;

   /* Number of output registers used per-pixel (1, 2 or 4). */
   uint32_t num_output_regs;

   /* Number of tile buffers used. */
   uint32_t num_tile_buffers;

   /* Size of a tile buffer in bytes. */
   uint32_t tile_buffer_size;

   /* Array of MRT resources allocated for each render target. The number of
    * elements is determined by usc_mrt_setup::num_render_targets.
    */
   struct usc_mrt_resource *mrt_resources;

   /* Don't set up source pos in emit. */
   bool disable_source_pos_override;

   /* Hash unique to this particular setup. */
   uint32_t hash;
};

struct pvr_renderpass_hwsetup_eot_surface {
   /* MRT index to store from. Also used to index into
    * usc_mrt_setup::mrt_resources.
    */
   uint32_t mrt_idx;

   /* Index of pvr_render_pass_info::attachments to store into. */
   uint32_t attachment_idx;

   /* True if the surface should be resolved. */
   bool need_resolve;

   /* How the surface should be resolved at the end of a render. Only valid if
    * pvr_renderpass_hwsetup_eot_surface::need_resolve is set to true.
    */
   enum pvr_resolve_type resolve_type;

   /* Index of pvr_render_pass_info::attachments to resolve from. Only valid if
    * pvr_renderpass_hwsetup_eot_surface::need_resolve is set to true.
    */
   uint32_t src_attachment_idx;
};

struct pvr_renderpass_hwsetup_subpass {
   /* Mapping from fragment stage pixel outputs to hardware storage for all
    * fragment programs in the subpass.
    */
   struct usc_mrt_setup setup;

   /* If >=0 then copy the depth into this pixel output for all fragment
    * programs in the subpass.
    */
   int32_t z_replicate;

   /* The operation to perform on the depth at the start of the subpass. Loads
    * are deferred to subpasses when depth has been replicated.
    */
   VkAttachmentLoadOp depth_initop;

   /* If true then clear the stencil at the start of the subpass. */
   bool stencil_clear;

   /* Subpass index from the input pvr_render_subpass structure. */
   uint32_t index;

   /* For each color attachment to the subpass the operation to perform at
    * the start of the subpass.
    */
   VkAttachmentLoadOp *color_initops;

   struct pvr_load_op *load_op;

   struct {
      enum pvr_renderpass_hwsetup_input_access type;
      uint32_t on_chip_rt;
   } * input_access;

   uint8_t output_register_mask;
};

struct pvr_renderpass_colorinit {
   /* Source attachment for the operation. */
   uint32_t index;

   /* Type of operation either clear or load. */
   VkAttachmentLoadOp op;
};

struct pvr_renderpass_hwsetup_render {
   /* Number of pixel output registers to allocate for this render. */
   uint32_t output_regs_count;

   /* Number of tile buffers to allocate for this render. */
   uint32_t tile_buffers_count;

   /* Number of subpasses in this render. */
   uint32_t subpass_count;

   /* Description of each subpass. */
   struct pvr_renderpass_hwsetup_subpass *subpasses;

   /* The sample count of every color attachment (or depth attachment if
    * z-only) in this render.
    */
   uint32_t sample_count;

   /* Index of the attachment to use for depth/stencil load/store in this
    * render.
    */
   uint32_t ds_attach_idx;

   /* Operation on the on-chip depth at the start of the render.
    * Either load from 'ds_attach_idx', clear using 'ds_attach_idx' or leave
    * uninitialized.
    */
   VkAttachmentLoadOp depth_init;

   /* Operation on the on-chip stencil at the start of the render. */
   VkAttachmentLoadOp stencil_init;

   /* Count of operations on on-chip color storage at the start of the render.
    */
   uint32_t color_init_count;

   /* For each operation: the destination in the on-chip color storage. */
   struct usc_mrt_setup init_setup;

   /* How to initialize render targets at the start of the render. */
   struct pvr_renderpass_colorinit *color_init;

   /* true to store depth to 'ds_attach_idx' at the end of the render. */
   bool depth_store;
   /* true to store stencil to 'ds_attach_idx' at the end of the render. */
   bool stencil_store;

   /* Describes the location of the source data for each stored surface. */
   struct usc_mrt_setup eot_setup;

   struct pvr_renderpass_hwsetup_eot_surface *eot_surfaces;
   uint32_t eot_surface_count;

   uint32_t pbe_emits;

   /* true if this HW render has lasting effects on its attachments. */
   bool has_side_effects;

   struct pvr_load_op *load_op;
};

struct pvr_renderpass_hw_map {
   uint32_t render;
   uint32_t subpass;
};

struct pvr_renderpass_hwsetup {
   /* Number of renders. */
   uint32_t render_count;

   /* Description of each render. */
   struct pvr_renderpass_hwsetup_render *renders;

   /* Maps indices from pvr_render_pass::subpasses to the
    * pvr_renderpass_hwsetup_render/pvr_renderpass_hwsetup_subpass relative to
    * that render where the subpass is scheduled.
    */
   struct pvr_renderpass_hw_map *subpass_map;

   bool *surface_allocate;
};

VkResult pvr_create_renderpass_hwsetup(
   struct pvr_device *device,
   const VkAllocationCallbacks *alloc,
   struct pvr_render_pass *pass,
   bool disable_merge,
   struct pvr_renderpass_hwsetup **const hw_setup_out);

void pvr_destroy_renderpass_hwsetup(const VkAllocationCallbacks *alloc,
                                    struct pvr_renderpass_hwsetup *hw_setup);

uint32_t pvr_get_tile_buffer_size(const struct pvr_device *device);

#endif /* PVR_HW_PASS_H */
