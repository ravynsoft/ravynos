/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_CMD_BUFFER_H
#define NVK_CMD_BUFFER_H 1

#include "nvk_private.h"

#include "nv_push.h"
#include "nvk_cmd_pool.h"
#include "nvk_descriptor_set.h"

#include "util/u_dynarray.h"

#include "vk_command_buffer.h"

#include <stdio.h>

struct nvk_buffer;
struct nvk_cbuf;
struct nvk_cmd_bo;
struct nvk_cmd_pool;
struct nvk_image_view;
struct nvk_push_descriptor_set;

struct nvk_sample_location {
   uint8_t x_u4:4;
   uint8_t y_u4:4;
};

/** Root descriptor table.  This gets pushed to the GPU directly */
struct nvk_root_descriptor_table {
   uint64_t root_desc_addr;

   union {
      struct {
         uint32_t base_vertex;
         uint32_t base_instance;
         uint32_t draw_id;
         uint32_t view_index;
         struct nvk_sample_location sample_locations[8];
      } draw;
      struct {
         uint32_t base_group[3];
         uint32_t group_count[3];
      } cs;
   };

   /* Client push constants */
   uint8_t push[NVK_MAX_PUSH_SIZE];

   /* Descriptor set base addresses */
   uint64_t sets[NVK_MAX_SETS];

   /* Dynamic buffer bindings */
   struct nvk_buffer_address dynamic_buffers[NVK_MAX_DYNAMIC_BUFFERS];

   /* enfore alignment to 0x100 as needed pre pascal */
   uint8_t __padding[0x20];
};

/* helper macro for computing root descriptor byte offsets */
#define nvk_root_descriptor_offset(member)\
   offsetof(struct nvk_root_descriptor_table, member)

struct nvk_descriptor_state {
   struct nvk_root_descriptor_table root;
   uint32_t set_sizes[NVK_MAX_SETS];
   struct nvk_descriptor_set *sets[NVK_MAX_SETS];
   uint32_t sets_dirty;

   struct nvk_push_descriptor_set *push[NVK_MAX_SETS];
   uint32_t push_dirty;
};

struct nvk_attachment {
   VkFormat vk_format;
   struct nvk_image_view *iview;

   VkResolveModeFlagBits resolve_mode;
   struct nvk_image_view *resolve_iview;
};

struct nvk_rendering_state {
   VkRenderingFlagBits flags;

   VkRect2D area;
   uint32_t layer_count;
   uint32_t view_mask;

   uint32_t color_att_count;
   struct nvk_attachment color_att[NVK_MAX_RTS];
   struct nvk_attachment depth_att;
   struct nvk_attachment stencil_att;
};

struct nvk_graphics_state {
   struct nvk_rendering_state render;
   struct nvk_graphics_pipeline *pipeline;
   struct nvk_descriptor_state descriptors;

   /* Used for meta save/restore */
   struct nvk_addr_range vb0;

   /* Needed by vk_command_buffer::dynamic_graphics_state */
   struct vk_vertex_input_state _dynamic_vi;
   struct vk_sample_locations_state _dynamic_sl;
};

struct nvk_compute_state {
   struct nvk_compute_pipeline *pipeline;
   struct nvk_descriptor_state descriptors;
};

struct nvk_cmd_push {
   void *map;
   uint64_t addr;
   uint32_t range;
   bool no_prefetch;
};

struct nvk_cmd_buffer {
   struct vk_command_buffer vk;

   struct {
      struct nvk_graphics_state gfx;
      struct nvk_compute_state cs;
   } state;

   /** List of nvk_cmd_bo
    *
    * This list exists entirely for ownership tracking.  Everything in here
    * must also be in pushes or bo_refs if it is to be referenced by this
    * command buffer.
    */
   struct list_head bos;
   struct list_head gart_bos;

   struct nvk_cmd_bo *upload_bo;
   uint32_t upload_offset;

   struct nvk_cmd_bo *cond_render_gart_bo;
   uint32_t cond_render_gart_offset;

   struct nvk_cmd_bo *push_bo;
   uint32_t *push_bo_limit;
   struct nv_push push;

   /** Array of struct nvk_cmd_push
    *
    * This acts both as a BO reference as well as provides a range in the
    * buffer to use as a pushbuf.
    */
   struct util_dynarray pushes;

   uint64_t tls_space_needed;
};

VK_DEFINE_HANDLE_CASTS(nvk_cmd_buffer, vk.base, VkCommandBuffer,
                       VK_OBJECT_TYPE_COMMAND_BUFFER)

extern const struct vk_command_buffer_ops nvk_cmd_buffer_ops;

static inline struct nvk_device *
nvk_cmd_buffer_device(struct nvk_cmd_buffer *cmd)
{
   return (struct nvk_device *)cmd->vk.base.device;
}

static inline struct nvk_cmd_pool *
nvk_cmd_buffer_pool(struct nvk_cmd_buffer *cmd)
{
   return (struct nvk_cmd_pool *)cmd->vk.pool;
}

void nvk_cmd_buffer_new_push(struct nvk_cmd_buffer *cmd);

#define NVK_CMD_BUFFER_MAX_PUSH 512

static inline struct nv_push *
nvk_cmd_buffer_push(struct nvk_cmd_buffer *cmd, uint32_t dw_count)
{
   assert(dw_count <= NVK_CMD_BUFFER_MAX_PUSH);

   /* Compare to the actual limit on our push bo */
   if (unlikely(cmd->push.end + dw_count > cmd->push_bo_limit))
      nvk_cmd_buffer_new_push(cmd);

   cmd->push.limit = cmd->push.end + dw_count;
   
   return &cmd->push;
}

void
nvk_cmd_buffer_push_indirect(struct nvk_cmd_buffer *cmd,
                             uint64_t addr, uint32_t dw_count);

void nvk_cmd_buffer_begin_graphics(struct nvk_cmd_buffer *cmd,
                                   const VkCommandBufferBeginInfo *pBeginInfo);
void nvk_cmd_buffer_begin_compute(struct nvk_cmd_buffer *cmd,
                                  const VkCommandBufferBeginInfo *pBeginInfo);

void nvk_cmd_bind_graphics_pipeline(struct nvk_cmd_buffer *cmd,
                                    struct nvk_graphics_pipeline *pipeline);
void nvk_cmd_bind_compute_pipeline(struct nvk_cmd_buffer *cmd,
                                   struct nvk_compute_pipeline *pipeline);

void nvk_cmd_bind_vertex_buffer(struct nvk_cmd_buffer *cmd, uint32_t vb_idx,
                                struct nvk_addr_range addr_range);

static inline struct nvk_descriptor_state *
nvk_get_descriptors_state(struct nvk_cmd_buffer *cmd,
                          VkPipelineBindPoint bind_point)
{
   switch (bind_point) {
   case VK_PIPELINE_BIND_POINT_GRAPHICS:
      return &cmd->state.gfx.descriptors;
   case VK_PIPELINE_BIND_POINT_COMPUTE:
      return &cmd->state.cs.descriptors;
   default:
      unreachable("Unhandled bind point");
   }
};

VkResult nvk_cmd_buffer_upload_alloc(struct nvk_cmd_buffer *cmd,
                                     uint32_t size, uint32_t alignment,
                                     uint64_t *addr, void **ptr);

VkResult nvk_cmd_buffer_upload_data(struct nvk_cmd_buffer *cmd,
                                    const void *data, uint32_t size,
                                    uint32_t alignment, uint64_t *addr);

VkResult nvk_cmd_buffer_cond_render_alloc(struct nvk_cmd_buffer *cmd,
					  uint64_t *addr);

void nvk_cmd_flush_wait_dep(struct nvk_cmd_buffer *cmd,
                            const VkDependencyInfo *dep,
                            bool wait);

void nvk_cmd_invalidate_deps(struct nvk_cmd_buffer *cmd,
                             uint32_t dep_count,
                             const VkDependencyInfo *deps);

void
nvk_cmd_buffer_flush_push_descriptors(struct nvk_cmd_buffer *cmd,
                                      struct nvk_descriptor_state *desc);

bool
nvk_cmd_buffer_get_cbuf_descriptor(struct nvk_cmd_buffer *cmd,
                                   const struct nvk_descriptor_state *desc,
                                   const struct nvk_cbuf *cbuf,
                                   struct nvk_buffer_address *desc_out);
uint64_t
nvk_cmd_buffer_get_cbuf_descriptor_addr(struct nvk_cmd_buffer *cmd,
                                        const struct nvk_descriptor_state *desc,
                                        const struct nvk_cbuf *cbuf);

void nvk_meta_resolve_rendering(struct nvk_cmd_buffer *cmd,
                                const VkRenderingInfo *pRenderingInfo);

void nvk_cmd_buffer_dump(struct nvk_cmd_buffer *cmd, FILE *fp);

#endif
