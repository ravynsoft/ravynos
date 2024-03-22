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
#ifndef IRIS_SCREEN_H
#define IRIS_SCREEN_H

#include "pipe/p_screen.h"
#include "frontend/drm_driver.h"
#include "util/disk_cache.h"
#include "util/slab.h"
#include "util/u_screen.h"
#include "intel/dev/intel_device_info.h"
#include "intel/isl/isl.h"
#include "iris_bufmgr.h"
#include "iris_binder.h"
#include "iris_measure.h"
#include "iris_resource.h"

struct intel_l3_config;
struct brw_vue_map;
struct iris_vs_prog_key;
struct iris_tcs_prog_key;
struct iris_tes_prog_key;
struct iris_gs_prog_key;
struct iris_fs_prog_key;
struct iris_cs_prog_key;
enum iris_program_cache_id;

struct u_trace;

#define READ_ONCE(x) (*(volatile __typeof__(x) *)&(x))
#define WRITE_ONCE(x, v) *(volatile __typeof__(x) *)&(x) = (v)

#define IRIS_MAX_TEXTURES 128
#define IRIS_MAX_SAMPLERS 32
#define IRIS_MAX_IMAGES 64
#define IRIS_MAX_SOL_BUFFERS 4
#define IRIS_MAP_BUFFER_ALIGNMENT 64

/**
 * Virtual table for generation-specific (genxml) function calls.
 */
struct iris_vtable {
   void (*destroy_state)(struct iris_context *ice);
   void (*init_render_context)(struct iris_batch *batch);
   void (*init_compute_context)(struct iris_batch *batch);
   void (*init_copy_context)(struct iris_batch *batch);
   void (*upload_render_state)(struct iris_context *ice,
                               struct iris_batch *batch,
                               const struct pipe_draw_info *draw,
                               unsigned drawid_offset,
                               const struct pipe_draw_indirect_info *indirect,
                               const struct pipe_draw_start_count_bias *sc);
   void (*upload_indirect_render_state)(struct iris_context *ice,
                                        const struct pipe_draw_info *draw,
                                        const struct pipe_draw_indirect_info *indirect,
                                        const struct pipe_draw_start_count_bias *sc);
   void (*update_binder_address)(struct iris_batch *batch,
                                 struct iris_binder *binder);
   void (*upload_compute_state)(struct iris_context *ice,
                                struct iris_batch *batch,
                                const struct pipe_grid_info *grid);
   void (*rebind_buffer)(struct iris_context *ice,
                         struct iris_resource *res);
   void (*load_register_reg32)(struct iris_batch *batch, uint32_t dst,
                               uint32_t src);
   void (*load_register_reg64)(struct iris_batch *batch, uint32_t dst,
                               uint32_t src);
   void (*load_register_imm32)(struct iris_batch *batch, uint32_t reg,
                               uint32_t val);
   void (*load_register_imm64)(struct iris_batch *batch, uint32_t reg,
                               uint64_t val);
   void (*load_register_mem32)(struct iris_batch *batch, uint32_t reg,
                               struct iris_bo *bo, uint32_t offset);
   void (*load_register_mem64)(struct iris_batch *batch, uint32_t reg,
                               struct iris_bo *bo, uint32_t offset);
   void (*store_register_mem32)(struct iris_batch *batch, uint32_t reg,
                                struct iris_bo *bo, uint32_t offset,
                                bool predicated);
   void (*store_register_mem64)(struct iris_batch *batch, uint32_t reg,
                                struct iris_bo *bo, uint32_t offset,
                                bool predicated);
   void (*store_data_imm32)(struct iris_batch *batch,
                            struct iris_bo *bo, uint32_t offset,
                            uint32_t value);
   void (*store_data_imm64)(struct iris_batch *batch,
                            struct iris_bo *bo, uint32_t offset,
                            uint64_t value);
   void (*copy_mem_mem)(struct iris_batch *batch,
                        struct iris_bo *dst_bo, uint32_t dst_offset,
                        struct iris_bo *src_bo, uint32_t src_offset,
                        unsigned bytes);
   void (*emit_raw_pipe_control)(struct iris_batch *batch,
                                 const char *reason, uint32_t flags,
                                 struct iris_bo *bo, uint32_t offset,
                                 uint64_t imm);

   void (*emit_mi_report_perf_count)(struct iris_batch *batch,
                                     struct iris_bo *bo,
                                     uint32_t offset_in_bytes,
                                     uint32_t report_id);

   void (*rewrite_compute_walker_pc)(struct iris_batch *batch,
                                     uint32_t *walker,
                                     struct iris_bo *bo,
                                     uint32_t offset);

   unsigned (*derived_program_state_size)(enum iris_program_cache_id id);
   void (*store_derived_program_state)(const struct intel_device_info *devinfo,
                                       enum iris_program_cache_id cache_id,
                                       struct iris_compiled_shader *shader);
   uint32_t *(*create_so_decl_list)(const struct pipe_stream_output_info *sol,
                                    const struct brw_vue_map *vue_map);
   void (*populate_vs_key)(const struct iris_context *ice,
                           const struct shader_info *info,
                           gl_shader_stage last_stage,
                           struct iris_vs_prog_key *key);
   void (*populate_tcs_key)(const struct iris_context *ice,
                            struct iris_tcs_prog_key *key);
   void (*populate_tes_key)(const struct iris_context *ice,
                            const struct shader_info *info,
                            gl_shader_stage last_stage,
                            struct iris_tes_prog_key *key);
   void (*populate_gs_key)(const struct iris_context *ice,
                           const struct shader_info *info,
                           gl_shader_stage last_stage,
                           struct iris_gs_prog_key *key);
   void (*populate_fs_key)(const struct iris_context *ice,
                           const struct shader_info *info,
                           struct iris_fs_prog_key *key);
   void (*populate_cs_key)(const struct iris_context *ice,
                           struct iris_cs_prog_key *key);
   void (*lost_genx_state)(struct iris_context *ice, struct iris_batch *batch);
   void (*disable_rhwo_optimization)(struct iris_batch *batch, bool disable);
};

struct iris_address {
   struct iris_bo *bo;
   uint64_t offset;
   enum iris_domain access;
};

struct iris_screen {
   struct pipe_screen base;

   uint32_t refcount;

   /** Global slab allocator for iris_transfer_map objects */
   struct slab_parent_pool transfer_pool;

   /** drm device file descriptor, shared with bufmgr, do not close. */
   int fd;

   /**
    * drm device file descriptor to used for window system integration, owned
    * by iris_screen, can be a different DRM instance than fd.
    */
   int winsys_fd;

   struct iris_vtable vtbl;

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
      bool sync_compile;
      bool limit_trig_input_range;
      float lower_depth_range_rate;
      bool intel_enable_wa_14018912822;
      bool enable_tbimr;
   } driconf;

   /** Does the kernel support various features (KERNEL_HAS_* bitfield)? */
   unsigned kernel_features;
#define KERNEL_HAS_WAIT_FOR_SUBMIT   (1U<<0)
#define KERNEL_HAS_PROTECTED_CONTEXT (1U<<1)

   /**
    * Last sequence number allocated by the cache tracking mechanism.
    *
    * These are used for synchronization and are expected to identify a single
    * section of a batch, so they should be monotonically increasing and
    * unique across a single pipe_screen.
    */
   uint64_t last_seqno;

   const struct intel_device_info *devinfo;
   struct isl_device isl_dev;
   struct iris_bufmgr *bufmgr;
   struct brw_compiler *compiler;
   struct intel_perf_config *perf_cfg;

   const struct intel_l3_config *l3_config_3d;
   const struct intel_l3_config *l3_config_cs;

   /**
    * A buffer containing a marker + description of the driver. This buffer is
    * added to all execbufs syscalls so that we can identify the driver that
    * generated a hang by looking at the content of the buffer in the error
    * state. It is also used for hardware workarounds that require scratch
    * writes or reads from some unimportant memory. To avoid overriding the
    * debug data, use the workaround_address field for workarounds.
    */
   struct iris_bo *workaround_bo;
   struct iris_address workaround_address;

   struct util_queue shader_compiler_queue;

   struct disk_cache *disk_cache;

   struct intel_measure_device measure;

   /** Every screen on a bufmgr has an unique ID assigned by the bufmgr. */
   int id;

   struct iris_bo *breakpoint_bo;
};

struct pipe_screen *
iris_screen_create(int fd, const struct pipe_screen_config *config);

void iris_screen_destroy(struct iris_screen *screen);

UNUSED static inline struct pipe_screen *
iris_pscreen_ref(struct pipe_screen *pscreen)
{
   struct iris_screen *screen = (struct iris_screen *) pscreen;

   p_atomic_inc(&screen->refcount);
   return pscreen;
}

UNUSED static inline void
iris_pscreen_unref(struct pipe_screen *pscreen)
{
   struct iris_screen *screen = (struct iris_screen *) pscreen;

   if (p_atomic_dec_zero(&screen->refcount))
      iris_screen_destroy(screen);
}

bool
iris_is_format_supported(struct pipe_screen *pscreen,
                         enum pipe_format format,
                         enum pipe_texture_target target,
                         unsigned sample_count,
                         unsigned storage_sample_count,
                         unsigned usage);

void iris_disk_cache_init(struct iris_screen *screen);

#endif
