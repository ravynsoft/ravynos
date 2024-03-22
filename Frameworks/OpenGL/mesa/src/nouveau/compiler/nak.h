/*
 * Copyright © 2022 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#ifndef NAK_H
#define NAK_H

#include "compiler/shader_enums.h"
#include "nir.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nak_compiler;
struct nir_shader_compiler_options;
struct nv_device_info;

struct nak_compiler *nak_compiler_create(const struct nv_device_info *dev);
void nak_compiler_destroy(struct nak_compiler *nak);

uint64_t nak_debug_flags(const struct nak_compiler *nak);

const struct nir_shader_compiler_options *
nak_nir_options(const struct nak_compiler *nak);

void nak_optimize_nir(nir_shader *nir, const struct nak_compiler *nak);
void nak_preprocess_nir(nir_shader *nir, const struct nak_compiler *nak);

struct nak_fs_key {
   bool zs_self_dep;

   /** True if sample shading is forced on via an API knob such as
    * VkPipelineMultisampleStateCreateInfo::minSampleShading
    */
   bool force_sample_shading;

   /**
    * The constant buffer index and offset at which the sample locations table lives.
    * Each sample location is two 4-bit unorm values packed into an 8-bit value
    * with the bottom 4 bits for x and the top 4 bits for y.
   */
   uint8_t sample_locations_cb;
   uint32_t sample_locations_offset;
};

void nak_postprocess_nir(nir_shader *nir, const struct nak_compiler *nak,
                         nir_variable_mode robust2_modes,
                         const struct nak_fs_key *fs_key);

enum ENUM_PACKED nak_ts_domain {
   NAK_TS_DOMAIN_ISOLINE = 0,
   NAK_TS_DOMAIN_TRIANGLE = 1,
   NAK_TS_DOMAIN_QUAD = 2,
};

enum ENUM_PACKED nak_ts_spacing {
   NAK_TS_SPACING_INTEGER = 0,
   NAK_TS_SPACING_FRACT_ODD = 1,
   NAK_TS_SPACING_FRACT_EVEN = 2,
};

enum ENUM_PACKED nak_ts_prims {
   NAK_TS_PRIMS_POINTS = 0,
   NAK_TS_PRIMS_LINES = 1,
   NAK_TS_PRIMS_TRIANGLES_CW = 2,
   NAK_TS_PRIMS_TRIANGLES_CCW = 3,
};

struct nak_xfb_info {
   uint32_t stride[4];
   uint8_t stream[4];
   uint8_t attr_count[4];
   uint8_t attr_index[4][128];
};

struct nak_shader_info {
   gl_shader_stage stage;

   /** Number of GPRs used */
   uint8_t num_gprs;

   /** Number of barriers used */
   uint8_t num_barriers;

   /** Size of shader local (scratch) memory */
   uint32_t slm_size;

   union {
      struct {
         /* Local workgroup size */
         uint16_t local_size[3];

         /* Shared memory size */
         uint16_t smem_size;
      } cs;

      struct {
         bool writes_depth;
         bool reads_sample_mask;
         bool post_depth_coverage;
         bool uses_sample_shading;
         bool early_fragment_tests;
      } fs;

      struct {
         enum nak_ts_domain domain;
         enum nak_ts_spacing spacing;
         enum nak_ts_prims prims;
      } ts;

      /* Used to initialize the union for other stages */
      uint32_t dummy;
   };

   struct {
      bool writes_layer;
      uint8_t clip_enable;
      uint8_t cull_enable;

      struct nak_xfb_info xfb;
   } vtg;

   /** Shader header for 3D stages */
   uint32_t hdr[32];
};

struct nak_shader_bin {
   struct nak_shader_info info;

   uint32_t code_size;
   const void *code;

   const char *asm_str;
};

void nak_shader_bin_destroy(struct nak_shader_bin *bin);

struct nak_shader_bin *
nak_compile_shader(nir_shader *nir, bool dump_asm,
                   const struct nak_compiler *nak,
                   nir_variable_mode robust2_modes,
                   const struct nak_fs_key *fs_key);

#ifdef __cplusplus
}
#endif

#endif /* NAK_H */
