/*
 * Copyright © 2021 Valve Corporation
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

#include <inttypes.h>

#include "ac_perfcounter.h"
#include "amdgfxregs.h"
#include "radv_cs.h"
#include "radv_private.h"
#include "sid.h"

void
radv_perfcounter_emit_shaders(struct radv_device *device, struct radeon_cmdbuf *cs, unsigned shaders)
{
   if (device->physical_device->rad_info.gfx_level >= GFX11) {
      radeon_set_uconfig_reg(cs, R_036760_SQG_PERFCOUNTER_CTRL, shaders & 0x7f);
   } else {
      radeon_set_uconfig_reg_seq(cs, R_036780_SQ_PERFCOUNTER_CTRL, 2);
      radeon_emit(cs, shaders & 0x7f);
      radeon_emit(cs, 0xffffffff);
   }
}

static void
radv_emit_windowed_counters(struct radv_device *device, struct radeon_cmdbuf *cs, int family, bool enable)
{
   if (family == RADV_QUEUE_GENERAL) {
      radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
      radeon_emit(cs, EVENT_TYPE(enable ? V_028A90_PERFCOUNTER_START : V_028A90_PERFCOUNTER_STOP) | EVENT_INDEX(0));
   }

   radeon_set_sh_reg(cs, R_00B82C_COMPUTE_PERFCOUNT_ENABLE, S_00B82C_PERFCOUNT_ENABLE(enable));
}

void
radv_perfcounter_emit_spm_reset(struct radeon_cmdbuf *cs)
{
   radeon_set_uconfig_reg(cs, R_036020_CP_PERFMON_CNTL,
                          S_036020_PERFMON_STATE(V_036020_CP_PERFMON_STATE_DISABLE_AND_RESET) |
                             S_036020_SPM_PERFMON_STATE(V_036020_STRM_PERFMON_STATE_DISABLE_AND_RESET));
}

void
radv_perfcounter_emit_spm_start(struct radv_device *device, struct radeon_cmdbuf *cs, int family)
{
   /* Start SPM counters. */
   radeon_set_uconfig_reg(cs, R_036020_CP_PERFMON_CNTL,
                          S_036020_PERFMON_STATE(V_036020_CP_PERFMON_STATE_DISABLE_AND_RESET) |
                             S_036020_SPM_PERFMON_STATE(V_036020_STRM_PERFMON_STATE_START_COUNTING));

   radv_emit_windowed_counters(device, cs, family, true);
}

void
radv_perfcounter_emit_spm_stop(struct radv_device *device, struct radeon_cmdbuf *cs, int family)
{
   radv_emit_windowed_counters(device, cs, family, false);

   /* Stop SPM counters. */
   radeon_set_uconfig_reg(cs, R_036020_CP_PERFMON_CNTL,
                          S_036020_PERFMON_STATE(V_036020_CP_PERFMON_STATE_DISABLE_AND_RESET) |
                             S_036020_SPM_PERFMON_STATE(device->physical_device->rad_info.never_stop_sq_perf_counters
                                                           ? V_036020_STRM_PERFMON_STATE_START_COUNTING
                                                           : V_036020_STRM_PERFMON_STATE_STOP_COUNTING));
}

enum radv_perfcounter_op {
   RADV_PC_OP_SUM,
   RADV_PC_OP_MAX,
   RADV_PC_OP_RATIO_DIVSCALE,
   RADV_PC_OP_REVERSE_RATIO, /* (reg1 - reg0) / reg1 */
   RADV_PC_OP_SUM_WEIGHTED_4,
};

#define S_REG_SEL(x)   ((x)&0xFFFF)
#define G_REG_SEL(x)   ((x)&0xFFFF)
#define S_REG_BLOCK(x) ((x) << 16)
#define G_REG_BLOCK(x) (((x) >> 16) & 0x7FFF)

#define S_REG_OFFSET(x)    ((x)&0xFFFF)
#define G_REG_OFFSET(x)    ((x)&0xFFFF)
#define S_REG_INSTANCES(x) ((x) << 16)
#define G_REG_INSTANCES(x) (((x) >> 16) & 0x7FFF)
#define S_REG_CONSTANT(x)  ((x) << 31)
#define G_REG_CONSTANT(x)  ((x) >> 31)

struct radv_perfcounter_impl {
   enum radv_perfcounter_op op;
   uint32_t regs[8];
};

/* Only append to this list, never insert into the middle or remove (but can rename).
 *
 * The invariant we're trying to get here is counters that have the same meaning, so
 * these can be shared between counters that have different implementations on different
 * GPUs, but should be unique within a GPU.
 */
enum radv_perfcounter_uuid {
   RADV_PC_UUID_GPU_CYCLES,
   RADV_PC_UUID_SHADER_WAVES,
   RADV_PC_UUID_SHADER_INSTRUCTIONS,
   RADV_PC_UUID_SHADER_INSTRUCTIONS_VALU,
   RADV_PC_UUID_SHADER_INSTRUCTIONS_SALU,
   RADV_PC_UUID_SHADER_INSTRUCTIONS_VMEM_LOAD,
   RADV_PC_UUID_SHADER_INSTRUCTIONS_SMEM_LOAD,
   RADV_PC_UUID_SHADER_INSTRUCTIONS_VMEM_STORE,
   RADV_PC_UUID_SHADER_INSTRUCTIONS_LDS,
   RADV_PC_UUID_SHADER_INSTRUCTIONS_GDS,
   RADV_PC_UUID_SHADER_VALU_BUSY,
   RADV_PC_UUID_SHADER_SALU_BUSY,
   RADV_PC_UUID_VRAM_READ_SIZE,
   RADV_PC_UUID_VRAM_WRITE_SIZE,
   RADV_PC_UUID_L0_CACHE_HIT_RATIO,
   RADV_PC_UUID_L1_CACHE_HIT_RATIO,
   RADV_PC_UUID_L2_CACHE_HIT_RATIO,
};

struct radv_perfcounter_desc {
   struct radv_perfcounter_impl impl;

   VkPerformanceCounterUnitKHR unit;

   char name[VK_MAX_DESCRIPTION_SIZE];
   char category[VK_MAX_DESCRIPTION_SIZE];
   char description[VK_MAX_DESCRIPTION_SIZE];
   enum radv_perfcounter_uuid uuid;
};

#define PC_DESC(arg_op, arg_unit, arg_name, arg_category, arg_description, arg_uuid, ...)                              \
   (struct radv_perfcounter_desc)                                                                                      \
   {                                                                                                                   \
      .impl = {.op = arg_op, .regs = {__VA_ARGS__}}, .unit = VK_PERFORMANCE_COUNTER_UNIT_##arg_unit##_KHR,             \
      .name = arg_name, .category = arg_category, .description = arg_description, .uuid = RADV_PC_UUID_##arg_uuid      \
   }

#define ADD_PC(op, unit, name, category, description, uuid, ...)                                                       \
   do {                                                                                                                \
      if (descs) {                                                                                                     \
         descs[*count] = PC_DESC((op), unit, name, category, description, uuid, __VA_ARGS__);                          \
      }                                                                                                                \
      ++*count;                                                                                                        \
   } while (0)
#define CTR(block, ctr) (S_REG_BLOCK(block) | S_REG_SEL(ctr))
#define CONSTANT(v)     (S_REG_CONSTANT(1) | (uint32_t)(v))

enum { GRBM_PERF_SEL_GUI_ACTIVE = CTR(GRBM, 2) };

enum { CPF_PERF_SEL_CPF_STAT_BUSY_GFX10 = CTR(CPF, 0x18) };

enum {
   GL1C_PERF_SEL_REQ = CTR(GL1C, 0xe),
   GL1C_PERF_SEL_REQ_MISS = CTR(GL1C, 0x12),
};

enum {
   GL2C_PERF_SEL_REQ = CTR(GL2C, 0x3),

   GL2C_PERF_SEL_MISS_GFX101 = CTR(GL2C, 0x23),
   GL2C_PERF_SEL_MC_WRREQ_GFX101 = CTR(GL2C, 0x4b),
   GL2C_PERF_SEL_EA_WRREQ_64B_GFX101 = CTR(GL2C, 0x4c),
   GL2C_PERF_SEL_EA_RDREQ_32B_GFX101 = CTR(GL2C, 0x59),
   GL2C_PERF_SEL_EA_RDREQ_64B_GFX101 = CTR(GL2C, 0x5a),
   GL2C_PERF_SEL_EA_RDREQ_96B_GFX101 = CTR(GL2C, 0x5b),
   GL2C_PERF_SEL_EA_RDREQ_128B_GFX101 = CTR(GL2C, 0x5c),

   GL2C_PERF_SEL_MISS_GFX103 = CTR(GL2C, 0x2b),
   GL2C_PERF_SEL_MC_WRREQ_GFX103 = CTR(GL2C, 0x53),
   GL2C_PERF_SEL_EA_WRREQ_64B_GFX103 = CTR(GL2C, 0x55),
   GL2C_PERF_SEL_EA_RDREQ_32B_GFX103 = CTR(GL2C, 0x63),
   GL2C_PERF_SEL_EA_RDREQ_64B_GFX103 = CTR(GL2C, 0x64),
   GL2C_PERF_SEL_EA_RDREQ_96B_GFX103 = CTR(GL2C, 0x65),
   GL2C_PERF_SEL_EA_RDREQ_128B_GFX103 = CTR(GL2C, 0x66),
};

enum {
   SQ_PERF_SEL_WAVES = CTR(SQ, 0x4),
   SQ_PERF_SEL_INSTS_ALL_GFX10 = CTR(SQ, 0x31),
   SQ_PERF_SEL_INSTS_GDS_GFX10 = CTR(SQ, 0x37),
   SQ_PERF_SEL_INSTS_LDS_GFX10 = CTR(SQ, 0x3b),
   SQ_PERF_SEL_INSTS_SALU_GFX10 = CTR(SQ, 0x3c),
   SQ_PERF_SEL_INSTS_SMEM_GFX10 = CTR(SQ, 0x3d),
   SQ_PERF_SEL_INSTS_VALU_GFX10 = CTR(SQ, 0x40),
   SQ_PERF_SEL_INSTS_TEX_LOAD_GFX10 = CTR(SQ, 0x45),
   SQ_PERF_SEL_INSTS_TEX_STORE_GFX10 = CTR(SQ, 0x46),
   SQ_PERF_SEL_INST_CYCLES_VALU_GFX10 = CTR(SQ, 0x75),
};

enum {
   TCP_PERF_SEL_REQ_GFX10 = CTR(TCP, 0x9),
   TCP_PERF_SEL_REQ_MISS_GFX10 = CTR(TCP, 0x12),
};

#define CTR_NUM_SIMD CONSTANT(pdev->rad_info.num_simd_per_compute_unit * pdev->rad_info.num_cu)
#define CTR_NUM_CUS  CONSTANT(pdev->rad_info.num_cu)

static void
radv_query_perfcounter_descs(struct radv_physical_device *pdev, uint32_t *count, struct radv_perfcounter_desc *descs)
{
   *count = 0;

   ADD_PC(RADV_PC_OP_MAX, CYCLES, "GPU active cycles", "GRBM", "cycles the GPU is active processing a command buffer.",
          GPU_CYCLES, GRBM_PERF_SEL_GUI_ACTIVE);

   ADD_PC(RADV_PC_OP_SUM, GENERIC, "Waves", "Shaders", "Number of waves executed", SHADER_WAVES, SQ_PERF_SEL_WAVES);
   ADD_PC(RADV_PC_OP_SUM, GENERIC, "Instructions", "Shaders", "Number of Instructions executed", SHADER_INSTRUCTIONS,
          SQ_PERF_SEL_INSTS_ALL_GFX10);
   ADD_PC(RADV_PC_OP_SUM, GENERIC, "VALU Instructions", "Shaders", "Number of VALU Instructions executed",
          SHADER_INSTRUCTIONS_VALU, SQ_PERF_SEL_INSTS_VALU_GFX10);
   ADD_PC(RADV_PC_OP_SUM, GENERIC, "SALU Instructions", "Shaders", "Number of SALU Instructions executed",
          SHADER_INSTRUCTIONS_SALU, SQ_PERF_SEL_INSTS_SALU_GFX10);
   ADD_PC(RADV_PC_OP_SUM, GENERIC, "VMEM Load Instructions", "Shaders", "Number of VMEM load instructions executed",
          SHADER_INSTRUCTIONS_VMEM_LOAD, SQ_PERF_SEL_INSTS_TEX_LOAD_GFX10);
   ADD_PC(RADV_PC_OP_SUM, GENERIC, "SMEM Load Instructions", "Shaders", "Number of SMEM load instructions executed",
          SHADER_INSTRUCTIONS_SMEM_LOAD, SQ_PERF_SEL_INSTS_SMEM_GFX10);
   ADD_PC(RADV_PC_OP_SUM, GENERIC, "VMEM Store Instructions", "Shaders", "Number of VMEM store instructions executed",
          SHADER_INSTRUCTIONS_VMEM_STORE, SQ_PERF_SEL_INSTS_TEX_STORE_GFX10);
   ADD_PC(RADV_PC_OP_SUM, GENERIC, "LDS Instructions", "Shaders", "Number of LDS Instructions executed",
          SHADER_INSTRUCTIONS_LDS, SQ_PERF_SEL_INSTS_LDS_GFX10);
   ADD_PC(RADV_PC_OP_SUM, GENERIC, "GDS Instructions", "Shaders", "Number of GDS Instructions executed",
          SHADER_INSTRUCTIONS_GDS, SQ_PERF_SEL_INSTS_GDS_GFX10);

   ADD_PC(RADV_PC_OP_RATIO_DIVSCALE, PERCENTAGE, "VALU Busy", "Shader Utilization",
          "Percentage of time the VALU units are busy", SHADER_VALU_BUSY, SQ_PERF_SEL_INST_CYCLES_VALU_GFX10,
          CPF_PERF_SEL_CPF_STAT_BUSY_GFX10, CTR_NUM_SIMD);
   ADD_PC(RADV_PC_OP_RATIO_DIVSCALE, PERCENTAGE, "SALU Busy", "Shader Utilization",
          "Percentage of time the SALU units are busy", SHADER_SALU_BUSY, SQ_PERF_SEL_INSTS_SALU_GFX10,
          CPF_PERF_SEL_CPF_STAT_BUSY_GFX10, CTR_NUM_CUS);

   if (pdev->rad_info.gfx_level >= GFX10_3) {
      ADD_PC(RADV_PC_OP_SUM_WEIGHTED_4, BYTES, "VRAM read size", "Memory", "Number of bytes read from VRAM",
             VRAM_READ_SIZE, GL2C_PERF_SEL_EA_RDREQ_32B_GFX103, CONSTANT(32), GL2C_PERF_SEL_EA_RDREQ_64B_GFX103,
             CONSTANT(64), GL2C_PERF_SEL_EA_RDREQ_96B_GFX103, CONSTANT(96), GL2C_PERF_SEL_EA_RDREQ_128B_GFX103,
             CONSTANT(128));
      ADD_PC(RADV_PC_OP_SUM_WEIGHTED_4, BYTES, "VRAM write size", "Memory", "Number of bytes written to VRAM",
             VRAM_WRITE_SIZE, GL2C_PERF_SEL_MC_WRREQ_GFX103, CONSTANT(32), GL2C_PERF_SEL_EA_WRREQ_64B_GFX103,
             CONSTANT(64), CONSTANT(0), CONSTANT(0), CONSTANT(0), CONSTANT(0));
   } else {
      ADD_PC(RADV_PC_OP_SUM_WEIGHTED_4, BYTES, "VRAM read size", "Memory", "Number of bytes read from VRAM",
             VRAM_READ_SIZE, GL2C_PERF_SEL_EA_RDREQ_32B_GFX101, CONSTANT(32), GL2C_PERF_SEL_EA_RDREQ_64B_GFX101,
             CONSTANT(64), GL2C_PERF_SEL_EA_RDREQ_96B_GFX101, CONSTANT(96), GL2C_PERF_SEL_EA_RDREQ_128B_GFX101,
             CONSTANT(128));
      ADD_PC(RADV_PC_OP_SUM_WEIGHTED_4, BYTES, "VRAM write size", "Memory", "Number of bytes written to VRAM",
             VRAM_WRITE_SIZE, GL2C_PERF_SEL_MC_WRREQ_GFX101, CONSTANT(32), GL2C_PERF_SEL_EA_WRREQ_64B_GFX101,
             CONSTANT(32), CONSTANT(0), CONSTANT(0), CONSTANT(0), CONSTANT(0));
   }

   ADD_PC(RADV_PC_OP_REVERSE_RATIO, BYTES, "L0 cache hit ratio", "Memory", "Hit ratio of L0 cache", L0_CACHE_HIT_RATIO,
          TCP_PERF_SEL_REQ_MISS_GFX10, TCP_PERF_SEL_REQ_GFX10);
   ADD_PC(RADV_PC_OP_REVERSE_RATIO, BYTES, "L1 cache hit ratio", "Memory", "Hit ratio of L1 cache", L1_CACHE_HIT_RATIO,
          GL1C_PERF_SEL_REQ_MISS, GL1C_PERF_SEL_REQ);
   if (pdev->rad_info.gfx_level >= GFX10_3) {
      ADD_PC(RADV_PC_OP_REVERSE_RATIO, BYTES, "L2 cache hit ratio", "Memory", "Hit ratio of L2 cache",
             L2_CACHE_HIT_RATIO, GL2C_PERF_SEL_MISS_GFX103, GL2C_PERF_SEL_REQ);
   } else {
      ADD_PC(RADV_PC_OP_REVERSE_RATIO, BYTES, "L2 cache hit ratio", "Memory", "Hit ratio of L2 cache",
             L2_CACHE_HIT_RATIO, GL2C_PERF_SEL_MISS_GFX101, GL2C_PERF_SEL_REQ);
   }
}

static bool
radv_init_perfcounter_descs(struct radv_physical_device *pdev)
{
   if (pdev->perfcounters)
      return true;

   uint32_t count;
   radv_query_perfcounter_descs(pdev, &count, NULL);

   struct radv_perfcounter_desc *descs = malloc(sizeof(*descs) * count);
   if (!descs)
      return false;

   radv_query_perfcounter_descs(pdev, &count, descs);
   pdev->num_perfcounters = count;
   pdev->perfcounters = descs;

   return true;
}

static int
cmp_uint32_t(const void *a, const void *b)
{
   uint32_t l = *(const uint32_t *)a;
   uint32_t r = *(const uint32_t *)b;

   return (l < r) ? -1 : (l > r) ? 1 : 0;
}

static VkResult
radv_get_counter_registers(const struct radv_physical_device *pdevice, uint32_t num_indices, const uint32_t *indices,
                           unsigned *out_num_regs, uint32_t **out_regs)
{
   ASSERTED uint32_t num_counters = pdevice->num_perfcounters;
   const struct radv_perfcounter_desc *descs = pdevice->perfcounters;

   unsigned full_reg_cnt = num_indices * ARRAY_SIZE(descs->impl.regs);
   uint32_t *regs = malloc(full_reg_cnt * sizeof(uint32_t));
   if (!regs)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   unsigned reg_cnt = 0;
   for (unsigned i = 0; i < num_indices; ++i) {
      uint32_t index = indices[i];
      assert(index < num_counters);
      for (unsigned j = 0; j < ARRAY_SIZE(descs[index].impl.regs) && descs[index].impl.regs[j]; ++j) {
         if (!G_REG_CONSTANT(descs[index].impl.regs[j]))
            regs[reg_cnt++] = descs[index].impl.regs[j];
      }
   }

   qsort(regs, reg_cnt, sizeof(uint32_t), cmp_uint32_t);

   unsigned deduped_reg_cnt = 0;
   for (unsigned i = 1; i < reg_cnt; ++i) {
      if (regs[i] != regs[deduped_reg_cnt])
         regs[++deduped_reg_cnt] = regs[i];
   }
   ++deduped_reg_cnt;

   *out_num_regs = deduped_reg_cnt;
   *out_regs = regs;
   return VK_SUCCESS;
}

static unsigned
radv_pc_get_num_instances(const struct radv_physical_device *pdevice, struct ac_pc_block *ac_block)
{
   return ac_block->num_instances * ((ac_block->b->b->flags & AC_PC_BLOCK_SE) ? pdevice->rad_info.max_se : 1);
}

static unsigned
radv_get_num_counter_passes(const struct radv_physical_device *pdevice, unsigned num_regs, const uint32_t *regs)
{
   enum ac_pc_gpu_block prev_block = NUM_GPU_BLOCK;
   unsigned block_reg_count = 0;
   struct ac_pc_block *ac_block = NULL;
   unsigned passes_needed = 1;

   for (unsigned i = 0; i < num_regs; ++i) {
      enum ac_pc_gpu_block block = G_REG_BLOCK(regs[i]);

      if (block != prev_block) {
         block_reg_count = 0;
         prev_block = block;
         ac_block = ac_pc_get_block(&pdevice->ac_perfcounters, block);
      }

      ++block_reg_count;

      passes_needed = MAX2(passes_needed, DIV_ROUND_UP(block_reg_count, ac_block->b->b->num_counters));
   }

   return passes_needed;
}

void
radv_pc_deinit_query_pool(struct radv_pc_query_pool *pool)
{
   free(pool->counters);
   free(pool->pc_regs);
}

VkResult
radv_pc_init_query_pool(struct radv_physical_device *pdevice, const VkQueryPoolCreateInfo *pCreateInfo,
                        struct radv_pc_query_pool *pool)
{
   const VkQueryPoolPerformanceCreateInfoKHR *perf_info =
      vk_find_struct_const(pCreateInfo->pNext, QUERY_POOL_PERFORMANCE_CREATE_INFO_KHR);
   VkResult result;

   if (!radv_init_perfcounter_descs(pdevice))
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   result = radv_get_counter_registers(pdevice, perf_info->counterIndexCount, perf_info->pCounterIndices,
                                       &pool->num_pc_regs, &pool->pc_regs);
   if (result != VK_SUCCESS)
      return result;

   pool->num_passes = radv_get_num_counter_passes(pdevice, pool->num_pc_regs, pool->pc_regs);

   uint32_t *pc_reg_offsets = malloc(pool->num_pc_regs * sizeof(uint32_t));
   if (!pc_reg_offsets)
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   unsigned offset = 0;
   for (unsigned i = 0; i < pool->num_pc_regs; ++i) {
      enum ac_pc_gpu_block block = pool->pc_regs[i] >> 16;
      struct ac_pc_block *ac_block = ac_pc_get_block(&pdevice->ac_perfcounters, block);
      unsigned num_instances = radv_pc_get_num_instances(pdevice, ac_block);

      pc_reg_offsets[i] = S_REG_OFFSET(offset) | S_REG_INSTANCES(num_instances);
      offset += sizeof(uint64_t) * 2 * num_instances;
   }

   /* allow an uint32_t per pass to signal completion. */
   pool->b.stride = offset + 8 * pool->num_passes;

   pool->num_counters = perf_info->counterIndexCount;
   pool->counters = malloc(pool->num_counters * sizeof(struct radv_perfcounter_impl));
   if (!pool->counters) {
      free(pc_reg_offsets);
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   for (unsigned i = 0; i < pool->num_counters; ++i) {
      pool->counters[i] = pdevice->perfcounters[perf_info->pCounterIndices[i]].impl;

      for (unsigned j = 0; j < ARRAY_SIZE(pool->counters[i].regs); ++j) {
         uint32_t reg = pool->counters[i].regs[j];
         if (!reg || G_REG_CONSTANT(reg))
            continue;

         unsigned k;
         for (k = 0; k < pool->num_pc_regs; ++k)
            if (pool->pc_regs[k] == reg)
               break;
         pool->counters[i].regs[j] = pc_reg_offsets[k];
      }
   }

   free(pc_reg_offsets);
   return VK_SUCCESS;
}

static void
radv_emit_instance(struct radv_cmd_buffer *cmd_buffer, int se, int instance)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   unsigned value = S_030800_SH_BROADCAST_WRITES(1);

   if (se >= 0) {
      value |= S_030800_SE_INDEX(se);
   } else {
      value |= S_030800_SE_BROADCAST_WRITES(1);
   }

   if (instance >= 0) {
      value |= S_030800_INSTANCE_INDEX(instance);
   } else {
      value |= S_030800_INSTANCE_BROADCAST_WRITES(1);
   }

   radeon_set_uconfig_reg(cs, R_030800_GRBM_GFX_INDEX, value);
}

static void
radv_emit_select(struct radv_cmd_buffer *cmd_buffer, struct ac_pc_block *block, unsigned count, unsigned *selectors)
{
   const enum amd_gfx_level gfx_level = cmd_buffer->device->physical_device->rad_info.gfx_level;
   const enum radv_queue_family qf = cmd_buffer->qf;
   struct ac_pc_block_base *regs = block->b->b;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   unsigned idx;

   assert(count <= regs->num_counters);

   /* Fake counters. */
   if (!regs->select0)
      return;

   for (idx = 0; idx < count; ++idx) {
      radeon_set_perfctr_reg(gfx_level, qf, cs, regs->select0[idx], G_REG_SEL(selectors[idx]) | regs->select_or);
   }

   for (idx = 0; idx < regs->num_spm_counters; idx++) {
      radeon_set_uconfig_reg_seq(cs, regs->select1[idx], 1);
      radeon_emit(cs, 0);
   }
}

static void
radv_pc_emit_block_instance_read(struct radv_cmd_buffer *cmd_buffer, struct ac_pc_block *block, unsigned count,
                                 uint64_t va)
{
   struct ac_pc_block_base *regs = block->b->b;
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   unsigned reg = regs->counter0_lo;
   unsigned reg_delta = 8;

   assert(regs->select0);
   for (unsigned idx = 0; idx < count; ++idx) {
      if (regs->counters)
         reg = regs->counters[idx];

      radeon_emit(cs, PKT3(PKT3_COPY_DATA, 4, 0));
      radeon_emit(cs, COPY_DATA_SRC_SEL(COPY_DATA_PERF) | COPY_DATA_DST_SEL(COPY_DATA_TC_L2) | COPY_DATA_WR_CONFIRM |
                         COPY_DATA_COUNT_SEL); /* 64 bits */
      radeon_emit(cs, reg >> 2);
      radeon_emit(cs, 0); /* unused */
      radeon_emit(cs, va);
      radeon_emit(cs, va >> 32);

      va += sizeof(uint64_t) * 2 * radv_pc_get_num_instances(cmd_buffer->device->physical_device, block);
      reg += reg_delta;
   }
}

static void
radv_pc_sample_block(struct radv_cmd_buffer *cmd_buffer, struct ac_pc_block *block, unsigned count, uint64_t va)
{
   unsigned se_end = 1;
   if (block->b->b->flags & AC_PC_BLOCK_SE)
      se_end = cmd_buffer->device->physical_device->rad_info.max_se;

   for (unsigned se = 0; se < se_end; ++se) {
      for (unsigned instance = 0; instance < block->num_instances; ++instance) {
         radv_emit_instance(cmd_buffer, se, instance);
         radv_pc_emit_block_instance_read(cmd_buffer, block, count, va);
         va += sizeof(uint64_t) * 2;
      }
   }
}

static void
radv_pc_wait_idle(struct radv_cmd_buffer *cmd_buffer)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;

   radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
   radeon_emit(cs, EVENT_TYPE(V_028A90_CS_PARTIAL_FLUSH | EVENT_INDEX(4)));

   radeon_emit(cs, PKT3(PKT3_ACQUIRE_MEM, 6, 0));
   radeon_emit(cs, 0);          /* CP_COHER_CNTL */
   radeon_emit(cs, 0xffffffff); /* CP_COHER_SIZE */
   radeon_emit(cs, 0xffffff);   /* CP_COHER_SIZE_HI */
   radeon_emit(cs, 0);          /* CP_COHER_BASE */
   radeon_emit(cs, 0);          /* CP_COHER_BASE_HI */
   radeon_emit(cs, 0x0000000A); /* POLL_INTERVAL */
   radeon_emit(cs, 0);          /* GCR_CNTL */

   radeon_emit(cs, PKT3(PKT3_PFP_SYNC_ME, 0, 0));
   radeon_emit(cs, 0);
}

static void
radv_pc_stop_and_sample(struct radv_cmd_buffer *cmd_buffer, struct radv_pc_query_pool *pool, uint64_t va, bool end)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;

   radeon_emit(cs, PKT3(PKT3_EVENT_WRITE, 0, 0));
   radeon_emit(cs, EVENT_TYPE(V_028A90_PERFCOUNTER_SAMPLE) | EVENT_INDEX(0));

   radv_pc_wait_idle(cmd_buffer);

   radv_emit_instance(cmd_buffer, -1, -1);
   radv_emit_windowed_counters(cmd_buffer->device, cs, cmd_buffer->qf, false);

   radeon_set_uconfig_reg(
      cs, R_036020_CP_PERFMON_CNTL,
      S_036020_PERFMON_STATE(V_036020_CP_PERFMON_STATE_STOP_COUNTING) | S_036020_PERFMON_SAMPLE_ENABLE(1));

   for (unsigned pass = 0; pass < pool->num_passes; ++pass) {
      uint64_t pred_va = radv_buffer_get_va(cmd_buffer->device->perf_counter_bo) + PERF_CTR_BO_PASS_OFFSET + 8 * pass;
      uint64_t reg_va = va + (end ? 8 : 0);

      radeon_emit(cs, PKT3(PKT3_COND_EXEC, 3, 0));
      radeon_emit(cs, pred_va);
      radeon_emit(cs, pred_va >> 32);
      radeon_emit(cs, 0); /* Cache policy */

      uint32_t *skip_dwords = cs->buf + cs->cdw;
      radeon_emit(cs, 0);

      for (unsigned i = 0; i < pool->num_pc_regs;) {
         enum ac_pc_gpu_block block = G_REG_BLOCK(pool->pc_regs[i]);
         struct ac_pc_block *ac_block = ac_pc_get_block(&pdevice->ac_perfcounters, block);
         unsigned offset = ac_block->num_instances * pass;
         unsigned num_instances = radv_pc_get_num_instances(pdevice, ac_block);

         unsigned cnt = 1;
         while (cnt < pool->num_pc_regs - i && block == G_REG_BLOCK(pool->pc_regs[i + cnt]))
            ++cnt;

         if (offset < cnt) {
            unsigned pass_reg_cnt = MIN2(cnt - offset, ac_block->b->b->num_counters);
            radv_pc_sample_block(cmd_buffer, ac_block, pass_reg_cnt,
                                 reg_va + offset * num_instances * sizeof(uint64_t));
         }

         i += cnt;
         reg_va += num_instances * sizeof(uint64_t) * 2 * cnt;
      }

      if (end) {
         uint64_t signal_va = va + pool->b.stride - 8 - 8 * pass;
         radeon_emit(cs, PKT3(PKT3_WRITE_DATA, 3, 0));
         radeon_emit(cs, S_370_DST_SEL(V_370_MEM) | S_370_WR_CONFIRM(1) | S_370_ENGINE_SEL(V_370_ME));
         radeon_emit(cs, signal_va);
         radeon_emit(cs, signal_va >> 32);
         radeon_emit(cs, 1); /* value */
      }

      *skip_dwords = cs->buf + cs->cdw - skip_dwords - 1;
   }

   radv_emit_instance(cmd_buffer, -1, -1);
}

void
radv_pc_begin_query(struct radv_cmd_buffer *cmd_buffer, struct radv_pc_query_pool *pool, uint64_t va)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   struct radv_physical_device *pdevice = cmd_buffer->device->physical_device;
   ASSERTED unsigned cdw_max;

   cmd_buffer->state.uses_perf_counters = true;

   cdw_max = radeon_check_space(cmd_buffer->device->ws, cs,
                                256 +                      /* Random one time stuff */
                                   10 * pool->num_passes + /* COND_EXECs */
                                   pool->b.stride / 8 * (5 + 8));

   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, pool->b.bo);
   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, cmd_buffer->device->perf_counter_bo);

   uint64_t perf_ctr_va = radv_buffer_get_va(cmd_buffer->device->perf_counter_bo) + PERF_CTR_BO_FENCE_OFFSET;
   radeon_emit(cs, PKT3(PKT3_WRITE_DATA, 3, 0));
   radeon_emit(cs, S_370_DST_SEL(V_370_MEM) | S_370_WR_CONFIRM(1) | S_370_ENGINE_SEL(V_370_ME));
   radeon_emit(cs, perf_ctr_va);
   radeon_emit(cs, perf_ctr_va >> 32);
   radeon_emit(cs, 0); /* value */

   radv_pc_wait_idle(cmd_buffer);

   radeon_set_uconfig_reg(cs, R_036020_CP_PERFMON_CNTL,
                          S_036020_PERFMON_STATE(V_036020_CP_PERFMON_STATE_DISABLE_AND_RESET));

   radv_emit_inhibit_clockgating(cmd_buffer->device, cs, true);
   radv_emit_spi_config_cntl(cmd_buffer->device, cs, true);
   radv_perfcounter_emit_shaders(cmd_buffer->device, cs, 0x7f);

   for (unsigned pass = 0; pass < pool->num_passes; ++pass) {
      uint64_t pred_va = radv_buffer_get_va(cmd_buffer->device->perf_counter_bo) + PERF_CTR_BO_PASS_OFFSET + 8 * pass;

      radeon_emit(cs, PKT3(PKT3_COND_EXEC, 3, 0));
      radeon_emit(cs, pred_va);
      radeon_emit(cs, pred_va >> 32);
      radeon_emit(cs, 0); /* Cache policy */

      uint32_t *skip_dwords = cs->buf + cs->cdw;
      radeon_emit(cs, 0);

      for (unsigned i = 0; i < pool->num_pc_regs;) {
         enum ac_pc_gpu_block block = G_REG_BLOCK(pool->pc_regs[i]);
         struct ac_pc_block *ac_block = ac_pc_get_block(&pdevice->ac_perfcounters, block);
         unsigned offset = ac_block->num_instances * pass;

         unsigned cnt = 1;
         while (cnt < pool->num_pc_regs - i && block == G_REG_BLOCK(pool->pc_regs[i + cnt]))
            ++cnt;

         if (offset < cnt) {
            unsigned pass_reg_cnt = MIN2(cnt - offset, ac_block->b->b->num_counters);
            radv_emit_select(cmd_buffer, ac_block, pass_reg_cnt, pool->pc_regs + i + offset);
         }

         i += cnt;
      }

      *skip_dwords = cs->buf + cs->cdw - skip_dwords - 1;
   }

   radv_emit_instance(cmd_buffer, -1, -1);

   /* The following sequence actually starts the perfcounters. */

   radv_pc_stop_and_sample(cmd_buffer, pool, va, false);

   radeon_set_uconfig_reg(cs, R_036020_CP_PERFMON_CNTL,
                          S_036020_PERFMON_STATE(V_036020_CP_PERFMON_STATE_START_COUNTING));

   radv_emit_windowed_counters(cmd_buffer->device, cs, cmd_buffer->qf, true);

   assert(cmd_buffer->cs->cdw <= cdw_max);
}

void
radv_pc_end_query(struct radv_cmd_buffer *cmd_buffer, struct radv_pc_query_pool *pool, uint64_t va)
{
   struct radeon_cmdbuf *cs = cmd_buffer->cs;
   ASSERTED unsigned cdw_max;

   cdw_max = radeon_check_space(cmd_buffer->device->ws, cs,
                                256 + /* Reserved for things that don't scale with passes/counters */
                                   5 * pool->num_passes + /* COND_EXECs */
                                   pool->b.stride / 8 * 8);

   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, pool->b.bo);
   radv_cs_add_buffer(cmd_buffer->device->ws, cmd_buffer->cs, cmd_buffer->device->perf_counter_bo);

   uint64_t perf_ctr_va = radv_buffer_get_va(cmd_buffer->device->perf_counter_bo) + PERF_CTR_BO_FENCE_OFFSET;
   radv_cs_emit_write_event_eop(cs, cmd_buffer->device->physical_device->rad_info.gfx_level, cmd_buffer->qf,
                                V_028A90_BOTTOM_OF_PIPE_TS, 0, EOP_DST_SEL_MEM, EOP_DATA_SEL_VALUE_32BIT, perf_ctr_va,
                                1, cmd_buffer->gfx9_fence_va);
   radv_cp_wait_mem(cs, cmd_buffer->qf, WAIT_REG_MEM_EQUAL, perf_ctr_va, 1, 0xffffffff);

   radv_pc_wait_idle(cmd_buffer);
   radv_pc_stop_and_sample(cmd_buffer, pool, va, true);

   radeon_set_uconfig_reg(cs, R_036020_CP_PERFMON_CNTL,
                          S_036020_PERFMON_STATE(V_036020_CP_PERFMON_STATE_DISABLE_AND_RESET));
   radv_emit_spi_config_cntl(cmd_buffer->device, cs, false);
   radv_emit_inhibit_clockgating(cmd_buffer->device, cs, false);

   assert(cmd_buffer->cs->cdw <= cdw_max);
}

static uint64_t
radv_pc_sum_reg(uint32_t reg, const uint64_t *data)
{
   unsigned instances = G_REG_INSTANCES(reg);
   unsigned offset = G_REG_OFFSET(reg) / 8;
   uint64_t result = 0;

   if (G_REG_CONSTANT(reg))
      return reg & 0x7fffffffu;

   for (unsigned i = 0; i < instances; ++i) {
      result += data[offset + 2 * i + 1] - data[offset + 2 * i];
   }

   return result;
}

static uint64_t
radv_pc_max_reg(uint32_t reg, const uint64_t *data)
{
   unsigned instances = G_REG_INSTANCES(reg);
   unsigned offset = G_REG_OFFSET(reg) / 8;
   uint64_t result = 0;

   if (G_REG_CONSTANT(reg))
      return reg & 0x7fffffffu;

   for (unsigned i = 0; i < instances; ++i) {
      result = MAX2(result, data[offset + 2 * i + 1]);
   }

   return result;
}

static union VkPerformanceCounterResultKHR
radv_pc_get_result(const struct radv_perfcounter_impl *impl, const uint64_t *data)
{
   union VkPerformanceCounterResultKHR result;

   switch (impl->op) {
   case RADV_PC_OP_MAX:
      result.float64 = radv_pc_max_reg(impl->regs[0], data);
      break;
   case RADV_PC_OP_SUM:
      result.float64 = radv_pc_sum_reg(impl->regs[0], data);
      break;
   case RADV_PC_OP_RATIO_DIVSCALE:
      result.float64 = radv_pc_sum_reg(impl->regs[0], data) / (double)radv_pc_sum_reg(impl->regs[1], data) /
                       radv_pc_sum_reg(impl->regs[2], data) * 100.0;
      break;
   case RADV_PC_OP_REVERSE_RATIO: {
      double tmp = radv_pc_sum_reg(impl->regs[1], data);
      result.float64 = (tmp - radv_pc_sum_reg(impl->regs[0], data)) / tmp * 100.0;
      break;
   }
   case RADV_PC_OP_SUM_WEIGHTED_4:
      result.float64 = 0.0;
      for (unsigned i = 0; i < 4; ++i)
         result.float64 += radv_pc_sum_reg(impl->regs[2 * i], data) * radv_pc_sum_reg(impl->regs[2 * i + 1], data);
      break;
   default:
      unreachable("unhandled performance counter operation");
   }
   return result;
}

void
radv_pc_get_results(const struct radv_pc_query_pool *pc_pool, const uint64_t *data, void *out)
{
   union VkPerformanceCounterResultKHR *pc_result = out;

   for (unsigned i = 0; i < pc_pool->num_counters; ++i) {
      pc_result[i] = radv_pc_get_result(pc_pool->counters + i, data);
   }
}

VKAPI_ATTR VkResult VKAPI_CALL
radv_EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(
   VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, uint32_t *pCounterCount,
   VkPerformanceCounterKHR *pCounters, VkPerformanceCounterDescriptionKHR *pCounterDescriptions)
{
   RADV_FROM_HANDLE(radv_physical_device, pdevice, physicalDevice);

   if (vk_queue_to_radv(pdevice, queueFamilyIndex) != RADV_QUEUE_GENERAL) {
      *pCounterCount = 0;
      return VK_SUCCESS;
   }

   if (!radv_init_perfcounter_descs(pdevice))
      return VK_ERROR_OUT_OF_HOST_MEMORY;

   uint32_t counter_cnt = pdevice->num_perfcounters;
   const struct radv_perfcounter_desc *descs = pdevice->perfcounters;

   if (!pCounters && !pCounterDescriptions) {
      *pCounterCount = counter_cnt;
      return VK_SUCCESS;
   }

   VkResult result = counter_cnt > *pCounterCount ? VK_INCOMPLETE : VK_SUCCESS;
   counter_cnt = MIN2(counter_cnt, *pCounterCount);
   *pCounterCount = counter_cnt;

   for (uint32_t i = 0; i < counter_cnt; ++i) {
      if (pCounters) {
         pCounters[i].sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_KHR;
         pCounters[i].unit = descs[i].unit;
         pCounters[i].scope = VK_PERFORMANCE_COUNTER_SCOPE_COMMAND_KHR;
         pCounters[i].storage = VK_PERFORMANCE_COUNTER_STORAGE_FLOAT64_KHR;

         memset(&pCounters[i].uuid, 0, sizeof(pCounters[i].uuid));
         strcpy((char *)&pCounters[i].uuid, "RADV");

         const uint32_t uuid = descs[i].uuid;
         memcpy(&pCounters[i].uuid[12], &uuid, sizeof(uuid));
      }

      if (pCounterDescriptions) {
         pCounterDescriptions[i].sType = VK_STRUCTURE_TYPE_PERFORMANCE_COUNTER_DESCRIPTION_KHR;
         pCounterDescriptions[i].flags = VK_PERFORMANCE_COUNTER_DESCRIPTION_CONCURRENTLY_IMPACTED_BIT_KHR;
         strcpy(pCounterDescriptions[i].name, descs[i].name);
         strcpy(pCounterDescriptions[i].category, descs[i].category);
         strcpy(pCounterDescriptions[i].description, descs[i].description);
      }
   }
   return result;
}

VKAPI_ATTR void VKAPI_CALL
radv_GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(
   VkPhysicalDevice physicalDevice, const VkQueryPoolPerformanceCreateInfoKHR *pPerformanceQueryCreateInfo,
   uint32_t *pNumPasses)
{
   RADV_FROM_HANDLE(radv_physical_device, pdevice, physicalDevice);

   if (pPerformanceQueryCreateInfo->counterIndexCount == 0) {
      *pNumPasses = 0;
      return;
   }

   if (!radv_init_perfcounter_descs(pdevice)) {
      /* Can't return an error, so log */
      fprintf(stderr, "radv: Failed to init perf counters\n");
      *pNumPasses = 1;
      return;
   }

   assert(vk_queue_to_radv(pdevice, pPerformanceQueryCreateInfo->queueFamilyIndex) == RADV_QUEUE_GENERAL);

   unsigned num_regs = 0;
   uint32_t *regs = NULL;
   VkResult result = radv_get_counter_registers(pdevice, pPerformanceQueryCreateInfo->counterIndexCount,
                                                pPerformanceQueryCreateInfo->pCounterIndices, &num_regs, &regs);
   if (result != VK_SUCCESS) {
      /* Can't return an error, so log */
      fprintf(stderr, "radv: Failed to allocate memory for perf counters\n");
   }

   *pNumPasses = radv_get_num_counter_passes(pdevice, num_regs, regs);
   free(regs);
}
