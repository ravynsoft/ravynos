/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef IR3_COMPILER_H_
#define IR3_COMPILER_H_

#include "compiler/nir/nir.h"
#include "util/disk_cache.h"
#include "util/log.h"
#include "util/perf/cpu_trace.h"

#include "freedreno_dev_info.h"

#include "ir3.h"

BEGINC;

struct ir3_ra_reg_set;
struct ir3_shader;

struct ir3_compiler_options {
   /* If true, UBO/SSBO accesses are assumed to be bounds-checked as defined by
    * VK_EXT_robustness2 and optimizations may have to be more conservative.
    */
   bool robust_buffer_access2;

   /* If true, promote UBOs (except for constant data) to constants using ldc.k
    * in the preamble. The driver should ignore everything in ubo_state except
    * for the constant data UBO, which is excluded because the command pushing
    * constants for it can be pre-baked when compiling the shader.
    */
   bool push_ubo_with_preamble;

   /* If true, disable the shader cache. The driver is then responsible for
    * caching.
    */
   bool disable_cache;

   /* If >= 0, this specifies the bindless descriptor set + descriptor to use
    * for txf_ms_fb
    */
   int bindless_fb_read_descriptor;
   int bindless_fb_read_slot;

   /* True if 16-bit descriptors are used for both 16-bit and 32-bit access. */
   bool storage_16bit;

  /* If base_vertex should be lowered in nir */
  bool lower_base_vertex;

  bool shared_push_consts;
};

struct ir3_compiler {
   struct fd_device *dev;
   const struct fd_dev_id *dev_id;
   uint8_t gen;
   uint32_t shader_count;

   struct disk_cache *disk_cache;

   struct nir_shader_compiler_options nir_options;

   /*
    * Configuration options for things handled differently by turnip vs
    * gallium
    */
   struct ir3_compiler_options options;

   /*
    * Configuration options for things that are handled differently on
    * different generations:
    */

   bool is_64bit;

   /* a4xx (and later) drops SP_FS_FLAT_SHAD_MODE_REG_* for flat-interpolate
    * so we need to use ldlv.u32 to load the varying directly:
    */
   bool flat_bypass;

   /* on a3xx, we need to add one to # of array levels:
    */
   bool levels_add_one;

   /* on a3xx, we need to scale up integer coords for isaml based
    * on LoD:
    */
   bool unminify_coords;

   /* on a3xx do txf_ms w/ isaml and scaled coords: */
   bool txf_ms_with_isaml;

   /* on a4xx, for array textures we need to add 0.5 to the array
    * index coordinate:
    */
   bool array_index_add_half;

   /* on a6xx, rewrite samgp to sequence of samgq0-3 in vertex shaders:
    */
   bool samgq_workaround;

   /* on a650, vertex shader <-> tess control io uses LDL/STL */
   bool tess_use_shared;

   /* The maximum number of constants, in vec4's, across the entire graphics
    * pipeline.
    */
   uint16_t max_const_pipeline;

   /* The maximum number of constants, in vec4's, for VS+HS+DS+GS. */
   uint16_t max_const_geom;

   /* The maximum number of constants, in vec4's, for FS. */
   uint16_t max_const_frag;

   /* A "safe" max constlen that can be applied to each shader in the
    * pipeline which we guarantee will never exceed any combined limits.
    */
   uint16_t max_const_safe;

   /* The maximum number of constants, in vec4's, for compute shaders. */
   uint16_t max_const_compute;

   /* Number of instructions that the shader's base address and length
    * (instrlen divides instruction count by this) must be aligned to.
    */
   uint32_t instr_align;

   /* on a3xx, the unit of indirect const load is higher than later gens (in
    * vec4 units):
    */
   uint32_t const_upload_unit;

   /* The base number of threads per wave. Some stages may be able to double
    * this.
    */
   uint32_t threadsize_base;

   /* On at least a6xx, waves are always launched in pairs. In calculations
    * about occupancy, we pretend that each wave pair is actually one wave,
    * which simplifies many of the calculations, but means we have to
    * multiply threadsize_base by this number.
    */
   uint32_t wave_granularity;

   /* The maximum number of simultaneous waves per core. */
   uint32_t max_waves;

   /* This is theoretical maximum number of vec4 registers that one wave of
    * the base threadsize could use. To get the actual size of the register
    * file in bytes one would need to compute:
    *
    * reg_size_vec4 * threadsize_base * wave_granularity * 16 (bytes per vec4)
    *
    * However this number is more often what we actually need. For example, a
    * max_reg more than half of this will result in a doubled threadsize
    * being impossible (because double-sized waves take up twice as many
    * registers). Also, the formula for the occupancy given a particular
    * register footprint is simpler.
    *
    * It is in vec4 units because the register file is allocated
    * with vec4 granularity, so it's in the same units as max_reg.
    */
   uint32_t reg_size_vec4;

   /* The size of local memory in bytes */
   uint32_t local_mem_size;

   /* The number of total branch stack entries, divided by wave_granularity. */
   uint32_t branchstack_size;

   /* The byte increment of MEMSIZEPERITEM, the private memory per-fiber allocation. */
   uint32_t pvtmem_per_fiber_align;

   /* Whether clip+cull distances are supported */
   bool has_clip_cull;

   /* Whether private memory is supported */
   bool has_pvtmem;

   /* Whether SSBOs have descriptors for sampling with ISAM */
   bool has_isam_ssbo;

   /* True if 16-bit descriptors are used for both 16-bit and 32-bit access. */
   bool storage_16bit;

   /* True if getfiberid, getlast.w8, brcst.active, and quad_shuffle
    * instructions are supported which are necessary to support
    * subgroup quad and arithmetic operations.
    */
   bool has_getfiberid;

   /* MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB */
   uint32_t max_variable_workgroup_size;

   bool has_dp2acc;
   bool has_dp4acc;

   /* Type to use for 1b nir bools: */
   type_t bool_type;

   /* Whether compute invocation params are passed in via shared regfile or
    * constbuf. a5xx+ has the shared regfile.
    */
   bool has_shared_regfile;

   /* True if preamble instructions (shps, shpe, etc.) are supported */
   bool has_preamble;

   /* Where the shared consts start in constants file, in vec4's. */
   uint16_t shared_consts_base_offset;

   /* The size of shared consts for CS and FS(in vec4's).
    * Also the size that is actually used on geometry stages (on a6xx).
    */
   uint64_t shared_consts_size;

   /* Found on a6xx for geometry stages, that is different from
    * actually used shared consts.
    *
    * TODO: Keep an eye on this for next gens.
    */
   uint64_t geom_shared_consts_size_quirk;

   bool has_fs_tex_prefetch;

   bool stsc_duplication_quirk;
};

void ir3_compiler_destroy(struct ir3_compiler *compiler);
struct ir3_compiler *ir3_compiler_create(struct fd_device *dev,
                                         const struct fd_dev_id *dev_id,
                                         const struct fd_dev_info *dev_info,
                                         const struct ir3_compiler_options *options);

void ir3_disk_cache_init(struct ir3_compiler *compiler);
void ir3_disk_cache_init_shader_key(struct ir3_compiler *compiler,
                                    struct ir3_shader *shader);
struct ir3_shader_variant *ir3_retrieve_variant(struct blob_reader *blob,
                                                struct ir3_compiler *compiler,
                                                void *mem_ctx);
void ir3_store_variant(struct blob *blob, const struct ir3_shader_variant *v);
bool ir3_disk_cache_retrieve(struct ir3_shader *shader,
                             struct ir3_shader_variant *v);
void ir3_disk_cache_store(struct ir3_shader *shader,
                          struct ir3_shader_variant *v);

const nir_shader_compiler_options *
ir3_get_compiler_options(struct ir3_compiler *compiler);

int ir3_compile_shader_nir(struct ir3_compiler *compiler,
                           struct ir3_shader *shader,
                           struct ir3_shader_variant *so);

/* gpu pointer size in units of 32bit registers/slots */
static inline unsigned
ir3_pointer_size(struct ir3_compiler *compiler)
{
   return compiler->is_64bit ? 2 : 1;
}

enum ir3_shader_debug {
   IR3_DBG_SHADER_VS = BITFIELD_BIT(0),
   IR3_DBG_SHADER_TCS = BITFIELD_BIT(1),
   IR3_DBG_SHADER_TES = BITFIELD_BIT(2),
   IR3_DBG_SHADER_GS = BITFIELD_BIT(3),
   IR3_DBG_SHADER_FS = BITFIELD_BIT(4),
   IR3_DBG_SHADER_CS = BITFIELD_BIT(5),
   IR3_DBG_DISASM = BITFIELD_BIT(6),
   IR3_DBG_OPTMSGS = BITFIELD_BIT(7),
   IR3_DBG_FORCES2EN = BITFIELD_BIT(8),
   IR3_DBG_NOUBOOPT = BITFIELD_BIT(9),
   IR3_DBG_NOFP16 = BITFIELD_BIT(10),
   IR3_DBG_NOCACHE = BITFIELD_BIT(11),
   IR3_DBG_SPILLALL = BITFIELD_BIT(12),
   IR3_DBG_NOPREAMBLE = BITFIELD_BIT(13),
   IR3_DBG_SHADER_INTERNAL = BITFIELD_BIT(14),

   /* DEBUG-only options: */
   IR3_DBG_SCHEDMSGS = BITFIELD_BIT(20),
   IR3_DBG_RAMSGS = BITFIELD_BIT(21),

   /* Only used for the disk-caching logic: */
   IR3_DBG_ROBUST_UBO_ACCESS = BITFIELD_BIT(30),
};

extern enum ir3_shader_debug ir3_shader_debug;
extern const char *ir3_shader_override_path;

static inline bool
shader_debug_enabled(gl_shader_stage type, bool internal)
{
   if (internal)
      return !!(ir3_shader_debug & IR3_DBG_SHADER_INTERNAL);

   if (ir3_shader_debug & IR3_DBG_DISASM)
      return true;

   switch (type) {
   case MESA_SHADER_VERTEX:
      return !!(ir3_shader_debug & IR3_DBG_SHADER_VS);
   case MESA_SHADER_TESS_CTRL:
      return !!(ir3_shader_debug & IR3_DBG_SHADER_TCS);
   case MESA_SHADER_TESS_EVAL:
      return !!(ir3_shader_debug & IR3_DBG_SHADER_TES);
   case MESA_SHADER_GEOMETRY:
      return !!(ir3_shader_debug & IR3_DBG_SHADER_GS);
   case MESA_SHADER_FRAGMENT:
      return !!(ir3_shader_debug & IR3_DBG_SHADER_FS);
   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_KERNEL:
      return !!(ir3_shader_debug & IR3_DBG_SHADER_CS);
   default:
      assert(0);
      return false;
   }
}

static inline void
ir3_debug_print(struct ir3 *ir, const char *when)
{
   if (ir3_shader_debug & IR3_DBG_OPTMSGS) {
      mesa_logi("%s:", when);
      ir3_print(ir);
   }
}

ENDC;

#endif /* IR3_COMPILER_H_ */
