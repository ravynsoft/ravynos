/*
 * Copyright © 2020 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef BRW_NIR_RT_BUILDER_H
#define BRW_NIR_RT_BUILDER_H

/* This file provides helpers to access memory based data structures that the
 * RT hardware reads/writes and their locations.
 *
 * See also "Memory Based Data Structures for Ray Tracing" (BSpec 47547) and
 * "Ray Tracing Address Computation for Memory Resident Structures" (BSpec
 * 47550).
 */

#include "brw_rt.h"
#include "nir_builder.h"

#define is_access_for_builder(b) \
   ((b)->shader->info.stage == MESA_SHADER_FRAGMENT ? \
    ACCESS_INCLUDE_HELPERS : 0)

static inline nir_def *
brw_nir_rt_load(nir_builder *b, nir_def *addr, unsigned align,
                unsigned components, unsigned bit_size)
{
   return nir_build_load_global(b, components, bit_size, addr,
                                .align_mul = align,
                                .access = is_access_for_builder(b));
}

static inline void
brw_nir_rt_store(nir_builder *b, nir_def *addr, unsigned align,
                 nir_def *value, unsigned write_mask)
{
   nir_build_store_global(b, value, addr,
                          .align_mul = align,
                          .write_mask = (write_mask) &
                                        BITFIELD_MASK(value->num_components),
                          .access = is_access_for_builder(b));
}

static inline nir_def *
brw_nir_rt_load_const(nir_builder *b, unsigned components,
                      nir_def *addr, nir_def *pred)
{
   return nir_load_global_const_block_intel(b, components, addr, pred);
}

static inline nir_def *
brw_load_btd_dss_id(nir_builder *b)
{
   return nir_load_topology_id_intel(b, .base = BRW_TOPOLOGY_ID_DSS);
}

static inline nir_def *
brw_nir_rt_load_num_simd_lanes_per_dss(nir_builder *b,
                                       const struct intel_device_info *devinfo)
{
   return nir_imm_int(b, devinfo->num_thread_per_eu *
                         devinfo->max_eus_per_subslice *
                         16 /* The RT computation is based off SIMD16 */);
}

static inline nir_def *
brw_load_eu_thread_simd(nir_builder *b)
{
   return nir_load_topology_id_intel(b, .base = BRW_TOPOLOGY_ID_EU_THREAD_SIMD);
}

static inline nir_def *
brw_nir_rt_async_stack_id(nir_builder *b)
{
   return nir_iadd(b, nir_umul_32x16(b, nir_load_ray_num_dss_rt_stacks_intel(b),
                                        brw_load_btd_dss_id(b)),
                      nir_load_btd_stack_id_intel(b));
}

static inline nir_def *
brw_nir_rt_sync_stack_id(nir_builder *b)
{
   return brw_load_eu_thread_simd(b);
}

/* We have our own load/store scratch helpers because they emit a global
 * memory read or write based on the scratch_base_ptr system value rather
 * than a load/store_scratch intrinsic.
 */
static inline nir_def *
brw_nir_rt_load_scratch(nir_builder *b, uint32_t offset, unsigned align,
                        unsigned num_components, unsigned bit_size)
{
   nir_def *addr =
      nir_iadd_imm(b, nir_load_scratch_base_ptr(b, 1, 64, 1), offset);
   return brw_nir_rt_load(b, addr, MIN2(align, BRW_BTD_STACK_ALIGN),
                             num_components, bit_size);
}

static inline void
brw_nir_rt_store_scratch(nir_builder *b, uint32_t offset, unsigned align,
                         nir_def *value, nir_component_mask_t write_mask)
{
   nir_def *addr =
      nir_iadd_imm(b, nir_load_scratch_base_ptr(b, 1, 64, 1), offset);
   brw_nir_rt_store(b, addr, MIN2(align, BRW_BTD_STACK_ALIGN),
                    value, write_mask);
}

static inline void
brw_nir_btd_spawn(nir_builder *b, nir_def *record_addr)
{
   nir_btd_spawn_intel(b, nir_load_btd_global_arg_addr_intel(b), record_addr);
}

static inline void
brw_nir_btd_retire(nir_builder *b)
{
   nir_btd_retire_intel(b);
}

/** This is a pseudo-op which does a bindless return
 *
 * It loads the return address from the stack and calls btd_spawn to spawn the
 * resume shader.
 */
static inline void
brw_nir_btd_return(struct nir_builder *b)
{
   nir_def *resume_addr =
      brw_nir_rt_load_scratch(b, BRW_BTD_STACK_RESUME_BSR_ADDR_OFFSET,
                              8 /* align */, 1, 64);
   brw_nir_btd_spawn(b, resume_addr);
}

static inline void
assert_def_size(nir_def *def, unsigned num_components, unsigned bit_size)
{
   assert(def->num_components == num_components);
   assert(def->bit_size == bit_size);
}

static inline nir_def *
brw_nir_num_rt_stacks(nir_builder *b,
                      const struct intel_device_info *devinfo)
{
   return nir_imul_imm(b, nir_load_ray_num_dss_rt_stacks_intel(b),
                          intel_device_info_dual_subslice_id_bound(devinfo));
}

static inline nir_def *
brw_nir_rt_sw_hotzone_addr(nir_builder *b,
                           const struct intel_device_info *devinfo)
{
   nir_def *offset32 =
      nir_imul_imm(b, brw_nir_rt_async_stack_id(b),
                      BRW_RT_SIZEOF_HOTZONE);

   offset32 = nir_iadd(b, offset32, nir_ineg(b,
      nir_imul_imm(b, brw_nir_num_rt_stacks(b, devinfo),
                      BRW_RT_SIZEOF_HOTZONE)));

   return nir_iadd(b, nir_load_ray_base_mem_addr_intel(b),
                      nir_i2i64(b, offset32));
}

static inline nir_def *
brw_nir_rt_sync_stack_addr(nir_builder *b,
                           nir_def *base_mem_addr,
                           const struct intel_device_info *devinfo)
{
   /* For Ray queries (Synchronous Ray Tracing), the formula is similar but
    * goes down from rtMemBasePtr :
    *
    *    syncBase  = RTDispatchGlobals.rtMemBasePtr
    *              - (DSSID * NUM_SIMD_LANES_PER_DSS + SyncStackID + 1)
    *              * syncStackSize
    *
    * We assume that we can calculate a 32-bit offset first and then add it
    * to the 64-bit base address at the end.
    */
   nir_def *offset32 =
      nir_imul(b,
               nir_iadd(b,
                        nir_imul(b, brw_load_btd_dss_id(b),
                                    brw_nir_rt_load_num_simd_lanes_per_dss(b, devinfo)),
                        nir_iadd_imm(b, brw_nir_rt_sync_stack_id(b), 1)),
               nir_imm_int(b, BRW_RT_SIZEOF_RAY_QUERY));
   return nir_isub(b, base_mem_addr, nir_u2u64(b, offset32));
}

static inline nir_def *
brw_nir_rt_stack_addr(nir_builder *b)
{
   /* From the BSpec "Address Computation for Memory Based Data Structures:
    * Ray and TraversalStack (Async Ray Tracing)":
    *
    *    stackBase = RTDispatchGlobals.rtMemBasePtr
    *              + (DSSID * RTDispatchGlobals.numDSSRTStacks + stackID)
    *              * RTDispatchGlobals.stackSizePerRay // 64B aligned
    *
    * We assume that we can calculate a 32-bit offset first and then add it
    * to the 64-bit base address at the end.
    */
   nir_def *offset32 =
      nir_imul(b, brw_nir_rt_async_stack_id(b),
                  nir_load_ray_hw_stack_size_intel(b));
   return nir_iadd(b, nir_load_ray_base_mem_addr_intel(b),
                      nir_u2u64(b, offset32));
}

static inline nir_def *
brw_nir_rt_mem_hit_addr_from_addr(nir_builder *b,
                        nir_def *stack_addr,
                        bool committed)
{
   return nir_iadd_imm(b, stack_addr, committed ? 0 : BRW_RT_SIZEOF_HIT_INFO);
}

static inline nir_def *
brw_nir_rt_mem_hit_addr(nir_builder *b, bool committed)
{
   return nir_iadd_imm(b, brw_nir_rt_stack_addr(b),
                          committed ? 0 : BRW_RT_SIZEOF_HIT_INFO);
}

static inline nir_def *
brw_nir_rt_hit_attrib_data_addr(nir_builder *b)
{
   return nir_iadd_imm(b, brw_nir_rt_stack_addr(b),
                          BRW_RT_OFFSETOF_HIT_ATTRIB_DATA);
}

static inline nir_def *
brw_nir_rt_mem_ray_addr(nir_builder *b,
                        nir_def *stack_addr,
                        enum brw_rt_bvh_level bvh_level)
{
   /* From the BSpec "Address Computation for Memory Based Data Structures:
    * Ray and TraversalStack (Async Ray Tracing)":
    *
    *    rayBase = stackBase + sizeof(HitInfo) * 2 // 64B aligned
    *    rayPtr  = rayBase + bvhLevel * sizeof(Ray); // 64B aligned
    *
    * In Vulkan, we always have exactly two levels of BVH: World and Object.
    */
   uint32_t offset = BRW_RT_SIZEOF_HIT_INFO * 2 +
                     bvh_level * BRW_RT_SIZEOF_RAY;
   return nir_iadd_imm(b, stack_addr, offset);
}

static inline nir_def *
brw_nir_rt_sw_stack_addr(nir_builder *b,
                         const struct intel_device_info *devinfo)
{
   nir_def *addr = nir_load_ray_base_mem_addr_intel(b);

   nir_def *offset32 = nir_imul(b, brw_nir_num_rt_stacks(b, devinfo),
                                       nir_load_ray_hw_stack_size_intel(b));
   addr = nir_iadd(b, addr, nir_u2u64(b, offset32));

   nir_def *offset_in_stack =
      nir_imul(b, nir_u2u64(b, brw_nir_rt_async_stack_id(b)),
                  nir_u2u64(b, nir_load_ray_sw_stack_size_intel(b)));

   return nir_iadd(b, addr, offset_in_stack);
}

static inline nir_def *
nir_unpack_64_4x16_split_z(nir_builder *b, nir_def *val)
{
   return nir_unpack_32_2x16_split_x(b, nir_unpack_64_2x32_split_y(b, val));
}

struct brw_nir_rt_globals_defs {
   nir_def *base_mem_addr;
   nir_def *call_stack_handler_addr;
   nir_def *hw_stack_size;
   nir_def *num_dss_rt_stacks;
   nir_def *hit_sbt_addr;
   nir_def *hit_sbt_stride;
   nir_def *miss_sbt_addr;
   nir_def *miss_sbt_stride;
   nir_def *sw_stack_size;
   nir_def *launch_size;
   nir_def *call_sbt_addr;
   nir_def *call_sbt_stride;
   nir_def *resume_sbt_addr;
};

static inline void
brw_nir_rt_load_globals_addr(nir_builder *b,
                             struct brw_nir_rt_globals_defs *defs,
                             nir_def *addr)
{
   nir_def *data;
   data = brw_nir_rt_load_const(b, 16, addr, nir_imm_true(b));
   defs->base_mem_addr = nir_pack_64_2x32(b, nir_trim_vector(b, data, 2));

   defs->call_stack_handler_addr =
      nir_pack_64_2x32(b, nir_channels(b, data, 0x3 << 2));

   defs->hw_stack_size = nir_channel(b, data, 4);
   defs->num_dss_rt_stacks = nir_iand_imm(b, nir_channel(b, data, 5), 0xffff);
   defs->hit_sbt_addr =
      nir_pack_64_2x32_split(b, nir_channel(b, data, 8),
                                nir_extract_i16(b, nir_channel(b, data, 9),
                                                   nir_imm_int(b, 0)));
   defs->hit_sbt_stride =
      nir_unpack_32_2x16_split_y(b, nir_channel(b, data, 9));
   defs->miss_sbt_addr =
      nir_pack_64_2x32_split(b, nir_channel(b, data, 10),
                                nir_extract_i16(b, nir_channel(b, data, 11),
                                                   nir_imm_int(b, 0)));
   defs->miss_sbt_stride =
      nir_unpack_32_2x16_split_y(b, nir_channel(b, data, 11));
   defs->sw_stack_size = nir_channel(b, data, 12);
   defs->launch_size = nir_channels(b, data, 0x7u << 13);

   data = brw_nir_rt_load_const(b, 8, nir_iadd_imm(b, addr, 64), nir_imm_true(b));
   defs->call_sbt_addr =
      nir_pack_64_2x32_split(b, nir_channel(b, data, 0),
                                nir_extract_i16(b, nir_channel(b, data, 1),
                                                   nir_imm_int(b, 0)));
   defs->call_sbt_stride =
      nir_unpack_32_2x16_split_y(b, nir_channel(b, data, 1));

   defs->resume_sbt_addr =
      nir_pack_64_2x32(b, nir_channels(b, data, 0x3 << 2));
}

static inline void
brw_nir_rt_load_globals(nir_builder *b,
                        struct brw_nir_rt_globals_defs *defs)
{
   brw_nir_rt_load_globals_addr(b, defs, nir_load_btd_global_arg_addr_intel(b));
}

static inline nir_def *
brw_nir_rt_unpack_leaf_ptr(nir_builder *b, nir_def *vec2)
{
   /* Hit record leaf pointers are 42-bit and assumed to be in 64B chunks.
    * This leaves 22 bits at the top for other stuff.
    */
   nir_def *ptr64 = nir_imul_imm(b, nir_pack_64_2x32(b, vec2), 64);

   /* The top 16 bits (remember, we shifted by 6 already) contain garbage
    * that we need to get rid of.
    */
   nir_def *ptr_lo = nir_unpack_64_2x32_split_x(b, ptr64);
   nir_def *ptr_hi = nir_unpack_64_2x32_split_y(b, ptr64);
   ptr_hi = nir_extract_i16(b, ptr_hi, nir_imm_int(b, 0));
   return nir_pack_64_2x32_split(b, ptr_lo, ptr_hi);
}

/**
 * MemHit memory layout (BSpec 47547) :
 *
 *      name            bits    description
 *    - t               32      hit distance of current hit (or initial traversal distance)
 *    - u               32      barycentric hit coordinates
 *    - v               32      barycentric hit coordinates
 *    - primIndexDelta  16      prim index delta for compressed meshlets and quads
 *    - valid            1      set if there is a hit
 *    - leafType         3      type of node primLeafPtr is pointing to
 *    - primLeafIndex    4      index of the hit primitive inside the leaf
 *    - bvhLevel         3      the instancing level at which the hit occured
 *    - frontFace        1      whether we hit the front-facing side of a triangle (also used to pass opaque flag when calling intersection shaders)
 *    - pad0             4      unused bits
 *    - primLeafPtr     42      pointer to BVH leaf node (multiple of 64 bytes)
 *    - hitGroupRecPtr0 22      LSB of hit group record of the hit triangle (multiple of 16 bytes)
 *    - instLeafPtr     42      pointer to BVH instance leaf node (in multiple of 64 bytes)
 *    - hitGroupRecPtr1 22      MSB of hit group record of the hit triangle (multiple of 32 bytes)
 */
struct brw_nir_rt_mem_hit_defs {
   nir_def *t;
   nir_def *tri_bary; /**< Only valid for triangle geometry */
   nir_def *aabb_hit_kind; /**< Only valid for AABB geometry */
   nir_def *valid;
   nir_def *leaf_type;
   nir_def *prim_index_delta;
   nir_def *prim_leaf_index;
   nir_def *bvh_level;
   nir_def *front_face;
   nir_def *done; /**< Only for ray queries */
   nir_def *prim_leaf_ptr;
   nir_def *inst_leaf_ptr;
};

static inline void
brw_nir_rt_load_mem_hit_from_addr(nir_builder *b,
                                  struct brw_nir_rt_mem_hit_defs *defs,
                                  nir_def *stack_addr,
                                  bool committed)
{
   nir_def *hit_addr =
      brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr, committed);

   nir_def *data = brw_nir_rt_load(b, hit_addr, 16, 4, 32);
   defs->t = nir_channel(b, data, 0);
   defs->aabb_hit_kind = nir_channel(b, data, 1);
   defs->tri_bary = nir_channels(b, data, 0x6);
   nir_def *bitfield = nir_channel(b, data, 3);
   defs->prim_index_delta =
      nir_ubitfield_extract(b, bitfield, nir_imm_int(b, 0), nir_imm_int(b, 16));
   defs->valid = nir_i2b(b, nir_iand_imm(b, bitfield, 1u << 16));
   defs->leaf_type =
      nir_ubitfield_extract(b, bitfield, nir_imm_int(b, 17), nir_imm_int(b, 3));
   defs->prim_leaf_index =
      nir_ubitfield_extract(b, bitfield, nir_imm_int(b, 20), nir_imm_int(b, 4));
   defs->bvh_level =
      nir_ubitfield_extract(b, bitfield, nir_imm_int(b, 24), nir_imm_int(b, 3));
   defs->front_face = nir_i2b(b, nir_iand_imm(b, bitfield, 1 << 27));
   defs->done = nir_i2b(b, nir_iand_imm(b, bitfield, 1 << 28));

   data = brw_nir_rt_load(b, nir_iadd_imm(b, hit_addr, 16), 16, 4, 32);
   defs->prim_leaf_ptr =
      brw_nir_rt_unpack_leaf_ptr(b, nir_channels(b, data, 0x3 << 0));
   defs->inst_leaf_ptr =
      brw_nir_rt_unpack_leaf_ptr(b, nir_channels(b, data, 0x3 << 2));
}

static inline void
brw_nir_rt_load_mem_hit(nir_builder *b,
                        struct brw_nir_rt_mem_hit_defs *defs,
                        bool committed)
{
   brw_nir_rt_load_mem_hit_from_addr(b, defs, brw_nir_rt_stack_addr(b),
                                     committed);
}

static inline void
brw_nir_memcpy_global(nir_builder *b,
                      nir_def *dst_addr, uint32_t dst_align,
                      nir_def *src_addr, uint32_t src_align,
                      uint32_t size)
{
   /* We're going to copy in 16B chunks */
   assert(size % 16 == 0);
   dst_align = MIN2(dst_align, 16);
   src_align = MIN2(src_align, 16);

   for (unsigned offset = 0; offset < size; offset += 16) {
      nir_def *data =
         brw_nir_rt_load(b, nir_iadd_imm(b, src_addr, offset), 16,
                         4, 32);
      brw_nir_rt_store(b, nir_iadd_imm(b, dst_addr, offset), 16,
                       data, 0xf /* write_mask */);
   }
}

static inline void
brw_nir_memclear_global(nir_builder *b,
                        nir_def *dst_addr, uint32_t dst_align,
                        uint32_t size)
{
   /* We're going to copy in 16B chunks */
   assert(size % 16 == 0);
   dst_align = MIN2(dst_align, 16);

   nir_def *zero = nir_imm_ivec4(b, 0, 0, 0, 0);
   for (unsigned offset = 0; offset < size; offset += 16) {
      brw_nir_rt_store(b, nir_iadd_imm(b, dst_addr, offset), dst_align,
                       zero, 0xf /* write_mask */);
   }
}

static inline nir_def *
brw_nir_rt_query_done(nir_builder *b, nir_def *stack_addr)
{
   struct brw_nir_rt_mem_hit_defs hit_in = {};
   brw_nir_rt_load_mem_hit_from_addr(b, &hit_in, stack_addr,
                                     false /* committed */);

   return hit_in.done;
}

static inline void
brw_nir_rt_set_dword_bit_at(nir_builder *b,
                            nir_def *addr,
                            uint32_t addr_offset,
                            uint32_t bit)
{
   nir_def *dword_addr = nir_iadd_imm(b, addr, addr_offset);
   nir_def *dword = brw_nir_rt_load(b, dword_addr, 4, 1, 32);
   brw_nir_rt_store(b, dword_addr, 4, nir_ior_imm(b, dword, 1u << bit), 0x1);
}

static inline void
brw_nir_rt_query_mark_done(nir_builder *b, nir_def *stack_addr)
{
   brw_nir_rt_set_dword_bit_at(b,
                               brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr,
                                                                 false /* committed */),
                               4 * 3 /* dword offset */, 28 /* bit */);
}

/* This helper clears the 3rd dword of the MemHit structure where the valid
 * bit is located.
 */
static inline void
brw_nir_rt_query_mark_init(nir_builder *b, nir_def *stack_addr)
{
   nir_def *dword_addr;

   for (uint32_t i = 0; i < 2; i++) {
      dword_addr =
         nir_iadd_imm(b,
                      brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr,
                                                        i == 0 /* committed */),
                      4 * 3 /* dword offset */);
      brw_nir_rt_store(b, dword_addr, 4, nir_imm_int(b, 0), 0x1);
   }
}

/* This helper is pretty much a memcpy of uncommitted into committed hit
 * structure, just adding the valid bit.
 */
static inline void
brw_nir_rt_commit_hit_addr(nir_builder *b, nir_def *stack_addr)
{
   nir_def *dst_addr =
      brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr, true /* committed */);
   nir_def *src_addr =
      brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr, false /* committed */);

   for (unsigned offset = 0; offset < BRW_RT_SIZEOF_HIT_INFO; offset += 16) {
      nir_def *data =
         brw_nir_rt_load(b, nir_iadd_imm(b, src_addr, offset), 16, 4, 32);

      if (offset == 0) {
         data = nir_vec4(b,
                         nir_channel(b, data, 0),
                         nir_channel(b, data, 1),
                         nir_channel(b, data, 2),
                         nir_ior_imm(b,
                                     nir_channel(b, data, 3),
                                     0x1 << 16 /* valid */));

         /* Also write the potential hit as we change it. */
         brw_nir_rt_store(b, nir_iadd_imm(b, src_addr, offset), 16,
                          data, 0xf /* write_mask */);
      }

      brw_nir_rt_store(b, nir_iadd_imm(b, dst_addr, offset), 16,
                       data, 0xf /* write_mask */);
   }
}

static inline void
brw_nir_rt_commit_hit(nir_builder *b)
{
   nir_def *stack_addr = brw_nir_rt_stack_addr(b);
   brw_nir_rt_commit_hit_addr(b, stack_addr);
}

static inline void
brw_nir_rt_generate_hit_addr(nir_builder *b, nir_def *stack_addr, nir_def *t_val)
{
   nir_def *committed_addr =
      brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr, true /* committed */);
   nir_def *potential_addr =
      brw_nir_rt_mem_hit_addr_from_addr(b, stack_addr, false /* committed */);

   /* Set:
    *
    *   potential.t     = t_val;
    *   potential.valid = true;
    */
   nir_def *potential_hit_dwords_0_3 =
      brw_nir_rt_load(b, potential_addr, 16, 4, 32);
   potential_hit_dwords_0_3 =
      nir_vec4(b,
               t_val,
               nir_channel(b, potential_hit_dwords_0_3, 1),
               nir_channel(b, potential_hit_dwords_0_3, 2),
               nir_ior_imm(b, nir_channel(b, potential_hit_dwords_0_3, 3),
                           (0x1 << 16) /* valid */));
   brw_nir_rt_store(b, potential_addr, 16, potential_hit_dwords_0_3, 0xf /* write_mask */);

   /* Set:
    *
    *   committed.t               = t_val;
    *   committed.u               = 0.0f;
    *   committed.v               = 0.0f;
    *   committed.valid           = true;
    *   committed.leaf_type       = potential.leaf_type;
    *   committed.bvh_level       = BRW_RT_BVH_LEVEL_OBJECT;
    *   committed.front_face      = false;
    *   committed.prim_leaf_index = 0;
    *   committed.done            = false;
    */
   nir_def *committed_hit_dwords_0_3 =
      brw_nir_rt_load(b, committed_addr, 16, 4, 32);
   committed_hit_dwords_0_3 =
      nir_vec4(b,
               t_val,
               nir_imm_float(b, 0.0f),
               nir_imm_float(b, 0.0f),
               nir_ior_imm(b,
                           nir_ior_imm(b, nir_channel(b, potential_hit_dwords_0_3, 3), 0x000e0000),
                           (0x1 << 16)                     /* valid */ |
                           (BRW_RT_BVH_LEVEL_OBJECT << 24) /* leaf_type */));
   brw_nir_rt_store(b, committed_addr, 16, committed_hit_dwords_0_3, 0xf /* write_mask */);

   /* Set:
    *
    *   committed.prim_leaf_ptr   = potential.prim_leaf_ptr;
    *   committed.inst_leaf_ptr   = potential.inst_leaf_ptr;
    */
   brw_nir_memcpy_global(b,
                         nir_iadd_imm(b, committed_addr, 16), 16,
                         nir_iadd_imm(b, potential_addr, 16), 16,
                         16);
}

struct brw_nir_rt_mem_ray_defs {
   nir_def *orig;
   nir_def *dir;
   nir_def *t_near;
   nir_def *t_far;
   nir_def *root_node_ptr;
   nir_def *ray_flags;
   nir_def *hit_group_sr_base_ptr;
   nir_def *hit_group_sr_stride;
   nir_def *miss_sr_ptr;
   nir_def *shader_index_multiplier;
   nir_def *inst_leaf_ptr;
   nir_def *ray_mask;
};

static inline void
brw_nir_rt_store_mem_ray_query_at_addr(nir_builder *b,
                                       nir_def *ray_addr,
                                       const struct brw_nir_rt_mem_ray_defs *defs)
{
   assert_def_size(defs->orig, 3, 32);
   assert_def_size(defs->dir, 3, 32);
   brw_nir_rt_store(b, nir_iadd_imm(b, ray_addr, 0), 16,
      nir_vec4(b, nir_channel(b, defs->orig, 0),
                  nir_channel(b, defs->orig, 1),
                  nir_channel(b, defs->orig, 2),
                  nir_channel(b, defs->dir, 0)),
      ~0 /* write mask */);

   assert_def_size(defs->t_near, 1, 32);
   assert_def_size(defs->t_far, 1, 32);
   brw_nir_rt_store(b, nir_iadd_imm(b, ray_addr, 16), 16,
      nir_vec4(b, nir_channel(b, defs->dir, 1),
                  nir_channel(b, defs->dir, 2),
                  defs->t_near,
                  defs->t_far),
      ~0 /* write mask */);

   assert_def_size(defs->root_node_ptr, 1, 64);
   assert_def_size(defs->ray_flags, 1, 16);
   brw_nir_rt_store(b, nir_iadd_imm(b, ray_addr, 32), 16,
      nir_vec2(b, nir_unpack_64_2x32_split_x(b, defs->root_node_ptr),
                  nir_pack_32_2x16_split(b,
                     nir_unpack_64_4x16_split_z(b, defs->root_node_ptr),
                     defs->ray_flags)),
      0x3 /* write mask */);

   /* leaf_ptr is optional */
   nir_def *inst_leaf_ptr;
   if (defs->inst_leaf_ptr) {
      inst_leaf_ptr = defs->inst_leaf_ptr;
   } else {
      inst_leaf_ptr = nir_imm_int64(b, 0);
   }

   assert_def_size(inst_leaf_ptr, 1, 64);
   assert_def_size(defs->ray_mask, 1, 32);
   brw_nir_rt_store(b, nir_iadd_imm(b, ray_addr, 56), 8,
      nir_vec2(b, nir_unpack_64_2x32_split_x(b, inst_leaf_ptr),
                  nir_pack_32_2x16_split(b,
                     nir_unpack_64_4x16_split_z(b, inst_leaf_ptr),
                     nir_unpack_32_2x16_split_x(b, defs->ray_mask))),
      ~0 /* write mask */);
}

static inline void
brw_nir_rt_store_mem_ray(nir_builder *b,
                         const struct brw_nir_rt_mem_ray_defs *defs,
                         enum brw_rt_bvh_level bvh_level)
{
   nir_def *ray_addr =
      brw_nir_rt_mem_ray_addr(b, brw_nir_rt_stack_addr(b), bvh_level);

   assert_def_size(defs->orig, 3, 32);
   assert_def_size(defs->dir, 3, 32);
   brw_nir_rt_store(b, nir_iadd_imm(b, ray_addr, 0), 16,
      nir_vec4(b, nir_channel(b, defs->orig, 0),
                  nir_channel(b, defs->orig, 1),
                  nir_channel(b, defs->orig, 2),
                  nir_channel(b, defs->dir, 0)),
      ~0 /* write mask */);

   assert_def_size(defs->t_near, 1, 32);
   assert_def_size(defs->t_far, 1, 32);
   brw_nir_rt_store(b, nir_iadd_imm(b, ray_addr, 16), 16,
      nir_vec4(b, nir_channel(b, defs->dir, 1),
                  nir_channel(b, defs->dir, 2),
                  defs->t_near,
                  defs->t_far),
      ~0 /* write mask */);

   assert_def_size(defs->root_node_ptr, 1, 64);
   assert_def_size(defs->ray_flags, 1, 16);
   assert_def_size(defs->hit_group_sr_base_ptr, 1, 64);
   assert_def_size(defs->hit_group_sr_stride, 1, 16);
   brw_nir_rt_store(b, nir_iadd_imm(b, ray_addr, 32), 16,
      nir_vec4(b, nir_unpack_64_2x32_split_x(b, defs->root_node_ptr),
                  nir_pack_32_2x16_split(b,
                     nir_unpack_64_4x16_split_z(b, defs->root_node_ptr),
                     defs->ray_flags),
                  nir_unpack_64_2x32_split_x(b, defs->hit_group_sr_base_ptr),
                  nir_pack_32_2x16_split(b,
                     nir_unpack_64_4x16_split_z(b, defs->hit_group_sr_base_ptr),
                     defs->hit_group_sr_stride)),
      ~0 /* write mask */);

   /* leaf_ptr is optional */
   nir_def *inst_leaf_ptr;
   if (defs->inst_leaf_ptr) {
      inst_leaf_ptr = defs->inst_leaf_ptr;
   } else {
      inst_leaf_ptr = nir_imm_int64(b, 0);
   }

   assert_def_size(defs->miss_sr_ptr, 1, 64);
   assert_def_size(defs->shader_index_multiplier, 1, 32);
   assert_def_size(inst_leaf_ptr, 1, 64);
   assert_def_size(defs->ray_mask, 1, 32);
   brw_nir_rt_store(b, nir_iadd_imm(b, ray_addr, 48), 16,
      nir_vec4(b, nir_unpack_64_2x32_split_x(b, defs->miss_sr_ptr),
                  nir_pack_32_2x16_split(b,
                     nir_unpack_64_4x16_split_z(b, defs->miss_sr_ptr),
                     nir_unpack_32_2x16_split_x(b,
                        nir_ishl(b, defs->shader_index_multiplier,
                                    nir_imm_int(b, 8)))),
                  nir_unpack_64_2x32_split_x(b, inst_leaf_ptr),
                  nir_pack_32_2x16_split(b,
                     nir_unpack_64_4x16_split_z(b, inst_leaf_ptr),
                     nir_unpack_32_2x16_split_x(b, defs->ray_mask))),
      ~0 /* write mask */);
}

static inline void
brw_nir_rt_load_mem_ray_from_addr(nir_builder *b,
                                  struct brw_nir_rt_mem_ray_defs *defs,
                                  nir_def *ray_base_addr,
                                  enum brw_rt_bvh_level bvh_level)
{
   nir_def *ray_addr = brw_nir_rt_mem_ray_addr(b,
                                                   ray_base_addr,
                                                   bvh_level);

   nir_def *data[4] = {
      brw_nir_rt_load(b, nir_iadd_imm(b, ray_addr,  0), 16, 4, 32),
      brw_nir_rt_load(b, nir_iadd_imm(b, ray_addr, 16), 16, 4, 32),
      brw_nir_rt_load(b, nir_iadd_imm(b, ray_addr, 32), 16, 4, 32),
      brw_nir_rt_load(b, nir_iadd_imm(b, ray_addr, 48), 16, 4, 32),
   };

   defs->orig = nir_trim_vector(b, data[0], 3);
   defs->dir = nir_vec3(b, nir_channel(b, data[0], 3),
                           nir_channel(b, data[1], 0),
                           nir_channel(b, data[1], 1));
   defs->t_near = nir_channel(b, data[1], 2);
   defs->t_far = nir_channel(b, data[1], 3);
   defs->root_node_ptr =
      nir_pack_64_2x32_split(b, nir_channel(b, data[2], 0),
                                nir_extract_i16(b, nir_channel(b, data[2], 1),
                                                   nir_imm_int(b, 0)));
   defs->ray_flags =
      nir_unpack_32_2x16_split_y(b, nir_channel(b, data[2], 1));
   defs->hit_group_sr_base_ptr =
      nir_pack_64_2x32_split(b, nir_channel(b, data[2], 2),
                                nir_extract_i16(b, nir_channel(b, data[2], 3),
                                                   nir_imm_int(b, 0)));
   defs->hit_group_sr_stride =
      nir_unpack_32_2x16_split_y(b, nir_channel(b, data[2], 3));
   defs->miss_sr_ptr =
      nir_pack_64_2x32_split(b, nir_channel(b, data[3], 0),
                                nir_extract_i16(b, nir_channel(b, data[3], 1),
                                                   nir_imm_int(b, 0)));
   defs->shader_index_multiplier =
      nir_ushr(b, nir_unpack_32_2x16_split_y(b, nir_channel(b, data[3], 1)),
                  nir_imm_int(b, 8));
   defs->inst_leaf_ptr =
      nir_pack_64_2x32_split(b, nir_channel(b, data[3], 2),
                                nir_extract_i16(b, nir_channel(b, data[3], 3),
                                                   nir_imm_int(b, 0)));
   defs->ray_mask =
      nir_unpack_32_2x16_split_y(b, nir_channel(b, data[3], 3));
}

static inline void
brw_nir_rt_load_mem_ray(nir_builder *b,
                        struct brw_nir_rt_mem_ray_defs *defs,
                        enum brw_rt_bvh_level bvh_level)
{
   brw_nir_rt_load_mem_ray_from_addr(b, defs, brw_nir_rt_stack_addr(b),
                                     bvh_level);
}

struct brw_nir_rt_bvh_instance_leaf_defs {
   nir_def *shader_index;
   nir_def *contribution_to_hit_group_index;
   nir_def *world_to_object[4];
   nir_def *instance_id;
   nir_def *instance_index;
   nir_def *object_to_world[4];
};

static inline void
brw_nir_rt_load_bvh_instance_leaf(nir_builder *b,
                                  struct brw_nir_rt_bvh_instance_leaf_defs *defs,
                                  nir_def *leaf_addr)
{
   nir_def *leaf_desc = brw_nir_rt_load(b, leaf_addr, 4, 2, 32);

   defs->shader_index =
      nir_iand_imm(b, nir_channel(b, leaf_desc, 0), (1 << 24) - 1);
   defs->contribution_to_hit_group_index =
      nir_iand_imm(b, nir_channel(b, leaf_desc, 1), (1 << 24) - 1);

   defs->world_to_object[0] =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 16), 4, 3, 32);
   defs->world_to_object[1] =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 28), 4, 3, 32);
   defs->world_to_object[2] =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 40), 4, 3, 32);
   /* The last column of the matrices is swapped between the two probably
    * because it makes it easier/faster for hardware somehow.
    */
   defs->object_to_world[3] =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 52), 4, 3, 32);

   nir_def *data =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 64), 4, 4, 32);
   defs->instance_id = nir_channel(b, data, 2);
   defs->instance_index = nir_channel(b, data, 3);

   defs->object_to_world[0] =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 80), 4, 3, 32);
   defs->object_to_world[1] =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 92), 4, 3, 32);
   defs->object_to_world[2] =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 104), 4, 3, 32);
   defs->world_to_object[3] =
      brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 116), 4, 3, 32);
}

struct brw_nir_rt_bvh_primitive_leaf_defs {
   nir_def *shader_index;
   nir_def *geom_mask;
   nir_def *geom_index;
   nir_def *type;
   nir_def *geom_flags;
};

static inline void
brw_nir_rt_load_bvh_primitive_leaf(nir_builder *b,
                                   struct brw_nir_rt_bvh_primitive_leaf_defs *defs,
                                   nir_def *leaf_addr)
{
   nir_def *desc = brw_nir_rt_load(b, leaf_addr, 4, 2, 32);

   defs->shader_index =
      nir_ubitfield_extract(b, nir_channel(b, desc, 0),
                            nir_imm_int(b, 23), nir_imm_int(b, 0));
   defs->geom_mask =
      nir_ubitfield_extract(b, nir_channel(b, desc, 0),
                            nir_imm_int(b, 31), nir_imm_int(b, 24));

   defs->geom_index =
      nir_ubitfield_extract(b, nir_channel(b, desc, 1),
                            nir_imm_int(b, 28), nir_imm_int(b, 0));
   defs->type =
      nir_ubitfield_extract(b, nir_channel(b, desc, 1),
                            nir_imm_int(b, 29), nir_imm_int(b, 29));
   defs->geom_flags =
      nir_ubitfield_extract(b, nir_channel(b, desc, 1),
                            nir_imm_int(b, 31), nir_imm_int(b, 30));
}

struct brw_nir_rt_bvh_primitive_leaf_positions_defs {
   nir_def *positions[3];
};

static inline void
brw_nir_rt_load_bvh_primitive_leaf_positions(nir_builder *b,
                                             struct brw_nir_rt_bvh_primitive_leaf_positions_defs *defs,
                                             nir_def *leaf_addr)
{
   for (unsigned i = 0; i < ARRAY_SIZE(defs->positions); i++) {
      defs->positions[i] =
         brw_nir_rt_load(b, nir_iadd_imm(b, leaf_addr, 16 + i * 4 * 3), 4, 3, 32);
   }
}

static inline nir_def *
brw_nir_rt_load_primitive_id_from_hit(nir_builder *b,
                                      nir_def *is_procedural,
                                      const struct brw_nir_rt_mem_hit_defs *defs)
{
   if (!is_procedural) {
      is_procedural =
         nir_ieq_imm(b, defs->leaf_type,
                        BRW_RT_BVH_NODE_TYPE_PROCEDURAL);
   }

   nir_def *prim_id_proc, *prim_id_quad;
   nir_push_if(b, is_procedural);
   {
      /* For procedural leafs, the index is in dw[3]. */
      nir_def *offset =
         nir_iadd_imm(b, nir_ishl_imm(b, defs->prim_leaf_index, 2), 12);
      prim_id_proc = nir_load_global(b, nir_iadd(b, defs->prim_leaf_ptr,
                                                 nir_u2u64(b, offset)),
                                     4, /* align */ 1, 32);
   }
   nir_push_else(b, NULL);
   {
      /* For quad leafs, the index is dw[2] and there is a 16bit additional
       * offset in dw[3].
       */
      prim_id_quad = nir_load_global(b, nir_iadd_imm(b, defs->prim_leaf_ptr, 8),
                                     4, /* align */ 1, 32);
      prim_id_quad = nir_iadd(b,
                              prim_id_quad,
                              defs->prim_index_delta);
   }
   nir_pop_if(b, NULL);

   return nir_if_phi(b, prim_id_proc, prim_id_quad);
}

static inline nir_def *
brw_nir_rt_acceleration_structure_to_root_node(nir_builder *b,
                                               nir_def *as_addr)
{
   /* The HW memory structure in which we specify what acceleration structure
    * to traverse, takes the address to the root node in the acceleration
    * structure, not the acceleration structure itself. To find that, we have
    * to read the root node offset from the acceleration structure which is
    * the first QWord.
    *
    * But if the acceleration structure pointer is NULL, then we should return
    * NULL as root node pointer.
    *
    * TODO: we could optimize this by assuming that for a given version of the
    * BVH, we can find the root node at a given offset.
    */
   nir_def *root_node_ptr, *null_node_ptr;
   nir_push_if(b, nir_ieq_imm(b, as_addr, 0));
   {
      null_node_ptr = nir_imm_int64(b, 0);
   }
   nir_push_else(b, NULL);
   {
      root_node_ptr =
         nir_iadd(b, as_addr, brw_nir_rt_load(b, as_addr, 256, 1, 64));
   }
   nir_pop_if(b, NULL);

   return nir_if_phi(b, null_node_ptr, root_node_ptr);
}

#endif /* BRW_NIR_RT_BUILDER_H */
