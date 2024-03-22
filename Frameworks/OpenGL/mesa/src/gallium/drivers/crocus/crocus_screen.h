/*
 * Copyright © 2017 Intel Corporation
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
#ifndef CROCUS_SCREEN_H
#define CROCUS_SCREEN_H

#include "pipe/p_screen.h"
#include "pipe/p_state.h"
#include "frontend/drm_driver.h"
#include "util/disk_cache.h"
#include "util/slab.h"
#include "util/u_screen.h"
#include "intel/dev/intel_device_info.h"
#include "intel/isl/isl.h"
#include "crocus_bufmgr.h"
#include "compiler/shader_enums.h"

struct crocus_resource;
struct crocus_context;
struct crocus_sampler_state;
struct brw_vue_map;
struct brw_tcs_prog_key;
struct brw_tes_prog_key;
struct brw_cs_prog_key;
struct brw_wm_prog_key;
struct brw_vs_prog_key;
struct brw_gs_prog_key;
struct shader_info;

#define READ_ONCE(x) (*(volatile __typeof__(x) *)&(x))
#define WRITE_ONCE(x, v) *(volatile __typeof__(x) *)&(x) = (v)

#define CROCUS_MAX_TEXTURE_SAMPLERS 32
#define CROCUS_MAX_SOL_BUFFERS 4
#define CROCUS_MAP_BUFFER_ALIGNMENT 64


/**
 * Virtual table for generation-specific (genxml) function calls.
 */
struct crocus_vtable {
   void (*destroy_state)(struct crocus_context *ice);
   void (*init_render_context)(struct crocus_batch *batch);
   void (*init_compute_context)(struct crocus_batch *batch);
   void (*upload_render_state)(struct crocus_context *ice,
                               struct crocus_batch *batch,
                               const struct pipe_draw_info *draw,
                               unsigned drawid_offset,
                               const struct pipe_draw_indirect_info *indirect,
                               const struct pipe_draw_start_count_bias *sc);
   void (*update_surface_base_address)(struct crocus_batch *batch);

   void (*upload_compute_state)(struct crocus_context *ice,
                                struct crocus_batch *batch,
                                const struct pipe_grid_info *grid);
   void (*rebind_buffer)(struct crocus_context *ice,
                         struct crocus_resource *res);
   void (*resolve_conditional_render)(struct crocus_context *ice);
   void (*emit_compute_predicate)(struct crocus_batch *batch);
   void (*load_register_reg32)(struct crocus_batch *batch, uint32_t dst,
                               uint32_t src);
   void (*load_register_reg64)(struct crocus_batch *batch, uint32_t dst,
                               uint32_t src);
   void (*load_register_imm32)(struct crocus_batch *batch, uint32_t reg,
                               uint32_t val);
   void (*load_register_imm64)(struct crocus_batch *batch, uint32_t reg,
                               uint64_t val);
   void (*load_register_mem32)(struct crocus_batch *batch, uint32_t reg,
                               struct crocus_bo *bo, uint32_t offset);
   void (*load_register_mem64)(struct crocus_batch *batch, uint32_t reg,
                               struct crocus_bo *bo, uint32_t offset);
   void (*store_register_mem32)(struct crocus_batch *batch, uint32_t reg,
                                struct crocus_bo *bo, uint32_t offset,
                                bool predicated);
   void (*store_register_mem64)(struct crocus_batch *batch, uint32_t reg,
                                struct crocus_bo *bo, uint32_t offset,
                                bool predicated);
   void (*store_data_imm32)(struct crocus_batch *batch,
                            struct crocus_bo *bo, uint32_t offset,
                            uint32_t value);
   void (*store_data_imm64)(struct crocus_batch *batch,
                            struct crocus_bo *bo, uint32_t offset,
                            uint64_t value);
   void (*copy_mem_mem)(struct crocus_batch *batch,
                        struct crocus_bo *dst_bo, uint32_t dst_offset,
                        struct crocus_bo *src_bo, uint32_t src_offset,
                        unsigned bytes);
   void (*emit_raw_pipe_control)(struct crocus_batch *batch,
                                 const char *reason, uint32_t flags,
                                 struct crocus_bo *bo, uint32_t offset,
                                 uint64_t imm);

   void (*emit_mi_report_perf_count)(struct crocus_batch *batch,
                                     struct crocus_bo *bo,
                                     uint32_t offset_in_bytes,
                                     uint32_t report_id);

   uint32_t *(*create_so_decl_list)(const struct pipe_stream_output_info *sol,
                                    const struct brw_vue_map *vue_map);
   void (*populate_vs_key)(const struct crocus_context *ice,
                           const struct shader_info *info,
                           gl_shader_stage last_stage,
                           struct brw_vs_prog_key *key);
   void (*populate_tcs_key)(const struct crocus_context *ice,
                            struct brw_tcs_prog_key *key);
   void (*populate_tes_key)(const struct crocus_context *ice,
                            const struct shader_info *info,
                            gl_shader_stage last_stage,
                            struct brw_tes_prog_key *key);
   void (*populate_gs_key)(const struct crocus_context *ice,
                           const struct shader_info *info,
                           gl_shader_stage last_stage,
                           struct brw_gs_prog_key *key);
   void (*populate_fs_key)(const struct crocus_context *ice,
                           const struct shader_info *info,
                           struct brw_wm_prog_key *key);
   void (*populate_cs_key)(const struct crocus_context *ice,
                           struct brw_cs_prog_key *key);
   void (*fill_clamp_mask)(const struct crocus_sampler_state *state,
                           int s,
                           uint32_t *clamp_mask);
   void (*lost_genx_state)(struct crocus_context *ice, struct crocus_batch *batch);

   void (*finish_batch)(struct crocus_batch *batch); /* haswell only */

   void (*upload_urb_fence)(struct crocus_batch *batch); /* gen4/5 only */

   bool (*blit_blt)(struct crocus_batch *batch,
                    const struct pipe_blit_info *info);
   bool (*copy_region_blt)(struct crocus_batch *batch,
                           struct crocus_resource *dst,
                           unsigned dst_level,
                           unsigned dstx, unsigned dsty, unsigned dstz,
                           struct crocus_resource *src,
                           unsigned src_level,
                           const struct pipe_box *src_box);
   bool (*calculate_urb_fence)(struct crocus_batch *batch, unsigned csize,
                               unsigned vsize, unsigned sfsize);
   void (*batch_reset_dirty)(struct crocus_batch *batch);
   unsigned (*translate_prim_type)(enum mesa_prim prim, uint8_t verts_per_patch);

   void (*update_so_strides)(struct crocus_context *ice,
                             uint16_t *strides);

   uint32_t (*get_so_offset)(struct pipe_stream_output_target *tgt);
};

struct crocus_screen {
   struct pipe_screen base;

   uint32_t refcount;

   /** Global slab allocator for crocus_transfer_map objects */
   struct slab_parent_pool transfer_pool;

   /** drm device file descriptor, shared with bufmgr, do not close. */
   int fd;

   /**
    * drm device file descriptor to used for window system integration, owned
    * by iris_screen, can be a different DRM instance than fd.
    */
   int winsys_fd;

   /** PCI ID for our GPU device */
   int pci_id;

   struct crocus_vtable vtbl;

   /** Global program_string_id counter (see get_program_string_id()) */
   unsigned program_id;

   /** Precompile shaders at link time?  (Can be disabled for debugging.) */
   bool precompile;

   /** driconf options and application workarounds */
   struct {
      /** Dual color blend by location instead of index (for broken apps) */
      bool dual_color_blend_by_location;
      bool disable_throttling;
      bool always_flush_cache;
      bool limit_trig_input_range;
      float lower_depth_range_rate;
   } driconf;

   uint64_t aperture_bytes;
   uint64_t aperture_threshold;

   struct intel_device_info devinfo;
   struct isl_device isl_dev;
   struct crocus_bufmgr *bufmgr;
   struct brw_compiler *compiler;
   struct intel_perf_config *perf_cfg;

   const struct intel_l3_config *l3_config_3d;
   const struct intel_l3_config *l3_config_cs;

   struct disk_cache *disk_cache;
};

struct pipe_screen *
crocus_screen_create(int fd, const struct pipe_screen_config *config);

void crocus_screen_destroy(struct crocus_screen *screen);

UNUSED static inline struct pipe_screen *
crocus_pscreen_ref(struct pipe_screen *pscreen)
{
   struct crocus_screen *screen = (struct crocus_screen *) pscreen;

   p_atomic_inc(&screen->refcount);
   return pscreen;
}

UNUSED static inline void
crocus_pscreen_unref(struct pipe_screen *pscreen)
{
   struct crocus_screen *screen = (struct crocus_screen *) pscreen;

   if (p_atomic_dec_zero(&screen->refcount))
      crocus_screen_destroy(screen);
}

bool
crocus_is_format_supported(struct pipe_screen *pscreen,
                           enum pipe_format format,
                           enum pipe_texture_target target,
                           unsigned sample_count,
                           unsigned storage_sample_count,
                           unsigned usage);

void crocus_disk_cache_init(struct crocus_screen *screen);

#endif
