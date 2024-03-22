/*
 * Copyright © 2020 Google, Inc.
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
 */

#include "ir3/ir3_compiler.h"

#include "util/u_math.h"

#include "adreno_pm4.xml.h"
#include "adreno_common.xml.h"
#include "a6xx.xml.h"

#include "common/freedreno_dev_info.h"

#include "ir3_asm.h"
#include "main.h"

#define FD_BO_NO_HARDPIN 1
#include "common/fd6_pack.h"

struct a6xx_backend {
   struct backend base;

   struct ir3_compiler *compiler;
   struct fd_device *dev;

   const struct fd_dev_info *info;

   unsigned seqno;
   struct fd_bo *control_mem;

   struct fd_bo *query_mem;
   const struct perfcntr *perfcntrs;
   unsigned num_perfcntrs;
};
define_cast(backend, a6xx_backend);

/*
 * Data structures shared with GPU:
 */

/* This struct defines the layout of the fd6_context::control buffer: */
struct fd6_control {
   uint32_t seqno; /* seqno for async CP_EVENT_WRITE, etc */
   uint32_t _pad0;
   volatile uint32_t vsc_overflow;
   uint32_t _pad1;
   /* flag set from cmdstream when VSC overflow detected: */
   uint32_t vsc_scratch;
   uint32_t _pad2;
   uint32_t _pad3;
   uint32_t _pad4;

   /* scratch space for VPC_SO[i].FLUSH_BASE_LO/HI, start on 32 byte boundary. */
   struct {
      uint32_t offset;
      uint32_t pad[7];
   } flush_base[4];
};

#define control_ptr(a6xx_backend, member)                                      \
   (a6xx_backend)->control_mem, offsetof(struct fd6_control, member), 0, 0

struct PACKED fd6_query_sample {
   uint64_t start;
   uint64_t result;
   uint64_t stop;
};

/* offset of a single field of an array of fd6_query_sample: */
#define query_sample_idx(a6xx_backend, idx, field)                             \
   (a6xx_backend)->query_mem,                                                  \
      (idx * sizeof(struct fd6_query_sample)) +                                \
         offsetof(struct fd6_query_sample, field),                             \
      0, 0

/*
 * Backend implementation:
 */

static struct kernel *
a6xx_assemble(struct backend *b, FILE *in)
{
   struct a6xx_backend *a6xx_backend = to_a6xx_backend(b);
   struct ir3_kernel *ir3_kernel = ir3_asm_assemble(a6xx_backend->compiler, in);
   ir3_kernel->backend = b;
   return &ir3_kernel->base;
}

static void
a6xx_disassemble(struct kernel *kernel, FILE *out)
{
   ir3_asm_disassemble(to_ir3_kernel(kernel), out);
}

template<chip CHIP>
static void
cs_program_emit(struct fd_ringbuffer *ring, struct kernel *kernel)
{
   struct ir3_kernel *ir3_kernel = to_ir3_kernel(kernel);
   struct a6xx_backend *a6xx_backend = to_a6xx_backend(ir3_kernel->backend);
   struct ir3_shader_variant *v = ir3_kernel->v;
   const unsigned *local_size = kernel->local_size;
   const struct ir3_info *i = &v->info;
   enum a6xx_threadsize thrsz = i->double_threadsize ? THREAD128 : THREAD64;

   OUT_REG(ring, A6XX_SP_MODE_CONTROL(.constant_demotion_enable = true,
                                      .isammode = ISAMMODE_GL,
                                      .shared_consts_enable = false));

   OUT_PKT4(ring, REG_A6XX_SP_PERFCTR_ENABLE, 1);
   OUT_RING(ring, A6XX_SP_PERFCTR_ENABLE_CS);

   OUT_PKT4(ring, REG_A6XX_SP_FLOAT_CNTL, 1);
   OUT_RING(ring, 0);

   for (size_t i = 0; i < ARRAY_SIZE(a6xx_backend->info->a6xx.magic_raw); i++) {
      auto magic_reg = a6xx_backend->info->a6xx.magic_raw[i];
      if (!magic_reg.reg)
         break;

      OUT_PKT4(ring, magic_reg.reg, 1);
      OUT_RING(ring, magic_reg.value);
   }

   OUT_REG(ring, HLSQ_INVALIDATE_CMD(CHIP,
      .vs_state = true,
      .hs_state = true,
      .ds_state = true,
      .gs_state = true,
      .fs_state = true,
      .cs_state = true,
      .gfx_ibo = true,
   ));

   unsigned constlen = align(v->constlen, 4);
   OUT_REG(ring, HLSQ_CS_CNTL(CHIP, .constlen = constlen, .enabled = true, ));

   OUT_PKT4(ring, REG_A6XX_SP_CS_CONFIG, 2);
   OUT_RING(ring, A6XX_SP_CS_CONFIG_ENABLED |
                     A6XX_SP_CS_CONFIG_NIBO(kernel->num_bufs) |
                     A6XX_SP_CS_CONFIG_NTEX(v->num_samp) |
                     A6XX_SP_CS_CONFIG_NSAMP(v->num_samp)); /* SP_VS_CONFIG */
   OUT_RING(ring, v->instrlen);                             /* SP_VS_INSTRLEN */

   OUT_PKT4(ring, REG_A6XX_SP_CS_CTRL_REG0, 1);
   OUT_RING(ring,
            A6XX_SP_CS_CTRL_REG0_THREADSIZE(thrsz) |
               A6XX_SP_CS_CTRL_REG0_FULLREGFOOTPRINT(i->max_reg + 1) |
               A6XX_SP_CS_CTRL_REG0_HALFREGFOOTPRINT(i->max_half_reg + 1) |
               COND(v->mergedregs, A6XX_SP_CS_CTRL_REG0_MERGEDREGS) |
               COND(ir3_kernel->info.early_preamble, A6XX_SP_CS_CTRL_REG0_EARLYPREAMBLE) |
               A6XX_SP_CS_CTRL_REG0_BRANCHSTACK(ir3_shader_branchstack_hw(v)));
   if (CHIP == A7XX) {
      OUT_REG(ring, HLSQ_FS_CNTL_0(CHIP, .threadsize = THREAD64));

      OUT_REG(ring, HLSQ_CONTROL_2_REG(CHIP, .dword = 0xfcfcfcfc),
              HLSQ_CONTROL_3_REG(CHIP, .dword = 0xfcfcfcfc),
              HLSQ_CONTROL_4_REG(CHIP, .dword = 0xfcfcfcfc),
              HLSQ_CONTROL_5_REG(CHIP, .dword = 0x0000fc00), );
   }

   OUT_PKT4(ring, REG_A6XX_SP_CS_UNKNOWN_A9B1, 1);
   OUT_RING(ring, A6XX_SP_CS_UNKNOWN_A9B1_SHARED_SIZE(1) |
                  A6XX_SP_CS_UNKNOWN_A9B1_UNK6);

   if (CHIP == A6XX && a6xx_backend->info->a6xx.has_lpac) {
      OUT_PKT4(ring, REG_A6XX_HLSQ_CS_UNKNOWN_B9D0, 1);
      OUT_RING(ring, A6XX_HLSQ_CS_UNKNOWN_B9D0_SHARED_SIZE(1) |
                        A6XX_HLSQ_CS_UNKNOWN_B9D0_UNK6);
   }

   uint32_t local_invocation_id, work_group_id;
   local_invocation_id =
      ir3_find_sysval_regid(v, SYSTEM_VALUE_LOCAL_INVOCATION_ID);
   work_group_id = ir3_find_sysval_regid(v, SYSTEM_VALUE_WORKGROUP_ID);

   if (CHIP == A6XX) {
      OUT_PKT4(ring, REG_A6XX_HLSQ_CS_CNTL_0, 2);
      OUT_RING(ring, A6XX_HLSQ_CS_CNTL_0_WGIDCONSTID(work_group_id) |
                        A6XX_HLSQ_CS_CNTL_0_WGSIZECONSTID(regid(63, 0)) |
                        A6XX_HLSQ_CS_CNTL_0_WGOFFSETCONSTID(regid(63, 0)) |
                        A6XX_HLSQ_CS_CNTL_0_LOCALIDREGID(local_invocation_id));
      OUT_RING(ring, A6XX_HLSQ_CS_CNTL_1_LINEARLOCALIDREGID(regid(63, 0)) |
                        A6XX_HLSQ_CS_CNTL_1_THREADSIZE(thrsz));
   } else {
      enum a7xx_cs_yalign yalign = (local_size[1] % 8 == 0)   ? CS_YALIGN_8
                                   : (local_size[1] % 4 == 0) ? CS_YALIGN_4
                                   : (local_size[1] % 2 == 0) ? CS_YALIGN_2
                                                              : CS_YALIGN_1;

      OUT_REG(ring, A7XX_HLSQ_CS_CNTL_1(.linearlocalidregid = regid(63, 0),
                                        .threadsize = thrsz,
                                        .unk11 = true,
                                        .unk22 = true,
                                        .yalign = yalign, ));
   }

   if (CHIP == A7XX || a6xx_backend->info->a6xx.has_lpac) {
      OUT_PKT4(ring, REG_A6XX_SP_CS_CNTL_0, 1);
      OUT_RING(ring, A6XX_SP_CS_CNTL_0_WGIDCONSTID(work_group_id) |
                        A6XX_SP_CS_CNTL_0_WGSIZECONSTID(regid(63, 0)) |
                        A6XX_SP_CS_CNTL_0_WGOFFSETCONSTID(regid(63, 0)) |
                        A6XX_SP_CS_CNTL_0_LOCALIDREGID(local_invocation_id));
      OUT_REG(ring,
         SP_CS_CNTL_1(CHIP, .linearlocalidregid = regid(63, 0),
                            .threadsize = thrsz, ));
   }

   OUT_PKT4(ring, REG_A6XX_SP_CS_OBJ_START, 2);
   OUT_RELOC(ring, v->bo, 0, 0, 0); /* SP_CS_OBJ_START_LO/HI */

   OUT_PKT4(ring, REG_A6XX_SP_CS_INSTRLEN, 1);
   OUT_RING(ring, v->instrlen);

   OUT_PKT4(ring, REG_A6XX_SP_CS_OBJ_START, 2);
   OUT_RELOC(ring, v->bo, 0, 0, 0);

   uint32_t shader_preload_size =
      MIN2(v->instrlen, a6xx_backend->info->a6xx.instr_cache_size);
   OUT_PKT7(ring, CP_LOAD_STATE6_FRAG, 3);
   OUT_RING(ring, CP_LOAD_STATE6_0_DST_OFF(0) |
                     CP_LOAD_STATE6_0_STATE_TYPE(ST6_SHADER) |
                     CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                     CP_LOAD_STATE6_0_STATE_BLOCK(SB6_CS_SHADER) |
                     CP_LOAD_STATE6_0_NUM_UNIT(shader_preload_size));
   OUT_RELOC(ring, v->bo, 0, 0, 0);

   if (v->pvtmem_size > 0) {
      uint32_t per_fiber_size = v->pvtmem_size;
      uint32_t per_sp_size =
         ALIGN(per_fiber_size * a6xx_backend->info->fibers_per_sp, 1 << 12);
      uint32_t total_size = per_sp_size * a6xx_backend->info->num_sp_cores;

      struct fd_bo *pvtmem = fd_bo_new(a6xx_backend->dev, total_size, 0, "pvtmem");
      OUT_PKT4(ring, REG_A6XX_SP_CS_PVT_MEM_PARAM, 4);
      OUT_RING(ring, A6XX_SP_CS_PVT_MEM_PARAM_MEMSIZEPERITEM(per_fiber_size));
      OUT_RELOC(ring, pvtmem, 0, 0, 0);
      OUT_RING(ring, A6XX_SP_CS_PVT_MEM_SIZE_TOTALPVTMEMSIZE(per_sp_size) |
                     COND(v->pvtmem_per_wave,
                          A6XX_SP_CS_PVT_MEM_SIZE_PERWAVEMEMLAYOUT));

      OUT_PKT4(ring, REG_A6XX_SP_CS_PVT_MEM_HW_STACK_OFFSET, 1);
      OUT_RING(ring, A6XX_SP_CS_PVT_MEM_HW_STACK_OFFSET_OFFSET(per_sp_size));
   }
}

template<chip CHIP>
static void
emit_const(struct fd_ringbuffer *ring, uint32_t regid, uint32_t sizedwords,
           const uint32_t *dwords)
{
   uint32_t align_sz;

   assert((regid % 4) == 0);

   align_sz = align(sizedwords, 4);

   OUT_PKT7(ring, CP_LOAD_STATE6_FRAG, 3 + align_sz);
   OUT_RING(ring, CP_LOAD_STATE6_0_DST_OFF(regid / 4) |
                     CP_LOAD_STATE6_0_STATE_TYPE(ST6_CONSTANTS) |
                     CP_LOAD_STATE6_0_STATE_SRC(SS6_DIRECT) |
                     CP_LOAD_STATE6_0_STATE_BLOCK(SB6_CS_SHADER) |
                     CP_LOAD_STATE6_0_NUM_UNIT(DIV_ROUND_UP(sizedwords, 4)));
   OUT_RING(ring, CP_LOAD_STATE6_1_EXT_SRC_ADDR(0));
   OUT_RING(ring, CP_LOAD_STATE6_2_EXT_SRC_ADDR_HI(0));

   for (uint32_t i = 0; i < sizedwords; i++) {
      OUT_RING(ring, dwords[i]);
   }

   /* Zero-pad to multiple of 4 dwords */
   for (uint32_t i = sizedwords; i < align_sz; i++) {
      OUT_RING(ring, 0);
   }
}

template<chip CHIP>
static void
cs_const_emit(struct fd_ringbuffer *ring, struct kernel *kernel,
              uint32_t grid[3])
{
   struct ir3_kernel *ir3_kernel = to_ir3_kernel(kernel);
   struct ir3_shader_variant *v = ir3_kernel->v;

   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t base = const_state->offsets.immediate;
   int size = DIV_ROUND_UP(const_state->immediates_count, 4);

   if (ir3_kernel->info.numwg != INVALID_REG) {
      assert((ir3_kernel->info.numwg & 0x3) == 0);
      int idx = ir3_kernel->info.numwg >> 2;
      const_state->immediates[idx * 4 + 0] = grid[0];
      const_state->immediates[idx * 4 + 1] = grid[1];
      const_state->immediates[idx * 4 + 2] = grid[2];
   }

   for (int i = 0; i < MAX_BUFS; i++) {
      if (kernel->buf_addr_regs[i] != INVALID_REG) {
         assert((kernel->buf_addr_regs[i] & 0x3) == 0);
         int idx = kernel->buf_addr_regs[i] >> 2;

         uint64_t iova = fd_bo_get_iova(kernel->bufs[i]);

         const_state->immediates[idx * 4 + 1] = iova >> 32;
         const_state->immediates[idx * 4 + 0] = (iova << 32) >> 32;
      }
   }

   /* truncate size to avoid writing constants that shader
    * does not use:
    */
   size = MIN2(size + base, v->constlen) - base;

   /* convert out of vec4: */
   base *= 4;
   size *= 4;

   if (size > 0) {
      emit_const<CHIP>(ring, base, size, const_state->immediates);
   }
}

template<chip CHIP>
static void
cs_ibo_emit(struct fd_ringbuffer *ring, struct fd_submit *submit,
            struct kernel *kernel)
{
   struct fd_ringbuffer *state = fd_submit_new_ringbuffer(
      submit, kernel->num_bufs * 16 * 4, FD_RINGBUFFER_STREAMING);

   for (unsigned i = 0; i < kernel->num_bufs; i++) {
      /* size is encoded with low 15b in WIDTH and high bits in HEIGHT,
       * in units of elements:
       */
      unsigned sz = kernel->buf_sizes[i];
      unsigned width = sz & MASK(15);
      unsigned height = sz >> 15;

      OUT_RING(state, A6XX_TEX_CONST_0_FMT(FMT6_32_UINT) | A6XX_TEX_CONST_0_TILE_MODE(TILE6_LINEAR));
      OUT_RING(state, A6XX_TEX_CONST_1_WIDTH(width) | A6XX_TEX_CONST_1_HEIGHT(height));
      OUT_RING(state, A6XX_TEX_CONST_2_PITCH(0) |
                      A6XX_TEX_CONST_2_STRUCTSIZETEXELS(1) |
                      A6XX_TEX_CONST_2_TYPE(A6XX_TEX_BUFFER));
      OUT_RING(state, A6XX_TEX_CONST_3_ARRAY_PITCH(0));
      OUT_RELOC(state, kernel->bufs[i], 0, 0, 0);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
      OUT_RING(state, 0x00000000);
   }

   OUT_PKT7(ring, CP_LOAD_STATE6_FRAG, 3);
   OUT_RING(ring, CP_LOAD_STATE6_0_DST_OFF(0) |
                     CP_LOAD_STATE6_0_STATE_TYPE(ST6_IBO) |
                     CP_LOAD_STATE6_0_STATE_SRC(SS6_INDIRECT) |
                     CP_LOAD_STATE6_0_STATE_BLOCK(SB6_CS_SHADER) |
                     CP_LOAD_STATE6_0_NUM_UNIT(kernel->num_bufs));
   OUT_RB(ring, state);

   OUT_PKT4(ring, REG_A6XX_SP_CS_IBO, 2);
   OUT_RB(ring, state);

   OUT_PKT4(ring, REG_A6XX_SP_CS_IBO_COUNT, 1);
   OUT_RING(ring, kernel->num_bufs);

   fd_ringbuffer_del(state);
}

template<chip CHIP>
static inline unsigned
event_write(struct fd_ringbuffer *ring, struct kernel *kernel,
            enum vgt_event_type evt, bool timestamp)
{
   unsigned seqno = 0;

   if (CHIP == A6XX) {
      OUT_PKT7(ring, CP_EVENT_WRITE, timestamp ? 4 : 1);
      OUT_RING(ring, CP_EVENT_WRITE_0_EVENT(evt));
   } else {
      OUT_PKT7(ring, CP_EVENT_WRITE7, timestamp ? 4 : 1);
      OUT_RING(ring,
         CP_EVENT_WRITE7_0_EVENT(evt) |
            COND(timestamp, CP_EVENT_WRITE7_0_WRITE_ENABLED |
                               CP_EVENT_WRITE7_0_WRITE_SRC(EV_WRITE_USER_32B)));
   }

   if (timestamp) {
      struct ir3_kernel *ir3_kernel = to_ir3_kernel(kernel);
      struct a6xx_backend *a6xx_backend = to_a6xx_backend(ir3_kernel->backend);
      seqno = ++a6xx_backend->seqno;
      OUT_RELOC(ring, control_ptr(a6xx_backend, seqno)); /* ADDR_LO/HI */
      OUT_RING(ring, seqno);
   }

   return seqno;
}

template<chip CHIP>
static inline void
cache_flush(struct fd_ringbuffer *ring, struct kernel *kernel)
{
   struct ir3_kernel *ir3_kernel = to_ir3_kernel(kernel);
   struct a6xx_backend *a6xx_backend = to_a6xx_backend(ir3_kernel->backend);
   unsigned seqno;

   seqno = event_write<CHIP>(ring, kernel, RB_DONE_TS, true);

   OUT_PKT7(ring, CP_WAIT_REG_MEM, 6);
   OUT_RING(ring, CP_WAIT_REG_MEM_0_FUNCTION(WRITE_EQ) |
                     CP_WAIT_REG_MEM_0_POLL(POLL_MEMORY));
   OUT_RELOC(ring, control_ptr(a6xx_backend, seqno));
   OUT_RING(ring, CP_WAIT_REG_MEM_3_REF(seqno));
   OUT_RING(ring, CP_WAIT_REG_MEM_4_MASK(~0));
   OUT_RING(ring, CP_WAIT_REG_MEM_5_DELAY_LOOP_CYCLES(16));

   if (CHIP == A6XX) {
      seqno = event_write<CHIP>(ring, kernel, CACHE_FLUSH_TS, true);

      OUT_PKT7(ring, CP_WAIT_MEM_GTE, 4);
      OUT_RING(ring, CP_WAIT_MEM_GTE_0_RESERVED(0));
      OUT_RELOC(ring, control_ptr(a6xx_backend, seqno));
      OUT_RING(ring, CP_WAIT_MEM_GTE_3_REF(seqno));
   } else {
      event_write<CHIP>(ring, kernel, CACHE_FLUSH7, false);
   }
}

template<chip CHIP>
static void
a6xx_emit_grid(struct kernel *kernel, uint32_t grid[3],
               struct fd_submit *submit)
{
   struct ir3_kernel *ir3_kernel = to_ir3_kernel(kernel);
   struct a6xx_backend *a6xx_backend = to_a6xx_backend(ir3_kernel->backend);
   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      submit, 0,
      (enum fd_ringbuffer_flags)(FD_RINGBUFFER_PRIMARY | FD_RINGBUFFER_GROWABLE));

   cs_program_emit<CHIP>(ring, kernel);
   cs_const_emit<CHIP>(ring, kernel, grid);
   cs_ibo_emit<CHIP>(ring, submit, kernel);

   OUT_PKT7(ring, CP_SET_MARKER, 1);
   OUT_RING(ring, A6XX_CP_SET_MARKER_0_MODE(RM6_COMPUTE));

   const unsigned *local_size = kernel->local_size;
   const unsigned *num_groups = grid;

   unsigned work_dim = 0;
   for (int i = 0; i < 3; i++) {
      if (!grid[i])
         break;
      work_dim++;
   }

   OUT_REG(ring, HLSQ_CS_NDRANGE_0(CHIP,
                    .kerneldim = work_dim,
                    .localsizex = local_size[0] - 1,
                    .localsizey = local_size[1] - 1,
                    .localsizez = local_size[2] - 1,
                 ));
   if (CHIP == A7XX) {
      OUT_REG(ring, A7XX_HLSQ_CS_LOCAL_SIZE(.localsizex = local_size[0] - 1,
                                            .localsizey = local_size[1] - 1,
                                            .localsizez = local_size[2] - 1, ));
   }

   OUT_REG(ring, HLSQ_CS_NDRANGE_1(CHIP,
                    .globalsize_x = local_size[0] * num_groups[0],
                 ));
   OUT_REG(ring, HLSQ_CS_NDRANGE_2(CHIP, 0));
   OUT_REG(ring, HLSQ_CS_NDRANGE_3(CHIP,
                    .globalsize_y = local_size[1] * num_groups[1],
                 ));
   OUT_REG(ring, HLSQ_CS_NDRANGE_4(CHIP, 0));
   OUT_REG(ring, HLSQ_CS_NDRANGE_5(CHIP,
                    .globalsize_z = local_size[2] * num_groups[2],
                 ));
   OUT_REG(ring, HLSQ_CS_NDRANGE_6(CHIP, 0));

   OUT_REG(ring, HLSQ_CS_KERNEL_GROUP_X(CHIP, 1));
   OUT_REG(ring, HLSQ_CS_KERNEL_GROUP_Y(CHIP, 1));
   OUT_REG(ring, HLSQ_CS_KERNEL_GROUP_Z(CHIP, 1));

   if (a6xx_backend->num_perfcntrs > 0) {
      a6xx_backend->query_mem = fd_bo_new(
         a6xx_backend->dev,
         a6xx_backend->num_perfcntrs * sizeof(struct fd6_query_sample), 0, "query");

      /* configure the performance counters to count the requested
       * countables:
       */
      for (unsigned i = 0; i < a6xx_backend->num_perfcntrs; i++) {
         const struct perfcntr *counter = &a6xx_backend->perfcntrs[i];

         OUT_PKT4(ring, counter->select_reg, 1);
         OUT_RING(ring, counter->selector);
      }

      OUT_PKT7(ring, CP_WAIT_FOR_IDLE, 0);

      /* and snapshot the start values: */
      for (unsigned i = 0; i < a6xx_backend->num_perfcntrs; i++) {
         const struct perfcntr *counter = &a6xx_backend->perfcntrs[i];

         OUT_PKT7(ring, CP_REG_TO_MEM, 3);
         OUT_RING(ring, CP_REG_TO_MEM_0_64B |
                           CP_REG_TO_MEM_0_REG(counter->counter_reg_lo));
         OUT_RELOC(ring, query_sample_idx(a6xx_backend, i, start));
      }
   }

   OUT_PKT7(ring, CP_EXEC_CS, 4);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, CP_EXEC_CS_1_NGROUPS_X(grid[0]));
   OUT_RING(ring, CP_EXEC_CS_2_NGROUPS_Y(grid[1]));
   OUT_RING(ring, CP_EXEC_CS_3_NGROUPS_Z(grid[2]));

   OUT_PKT7(ring, CP_WAIT_FOR_IDLE, 0);

   if (a6xx_backend->num_perfcntrs > 0) {
      /* snapshot the end values: */
      for (unsigned i = 0; i < a6xx_backend->num_perfcntrs; i++) {
         const struct perfcntr *counter = &a6xx_backend->perfcntrs[i];

         OUT_PKT7(ring, CP_REG_TO_MEM, 3);
         OUT_RING(ring, CP_REG_TO_MEM_0_64B |
                           CP_REG_TO_MEM_0_REG(counter->counter_reg_lo));
         OUT_RELOC(ring, query_sample_idx(a6xx_backend, i, stop));
      }

      /* and compute the result: */
      for (unsigned i = 0; i < a6xx_backend->num_perfcntrs; i++) {
         /* result += stop - start: */
         OUT_PKT7(ring, CP_MEM_TO_MEM, 9);
         OUT_RING(ring, CP_MEM_TO_MEM_0_DOUBLE | CP_MEM_TO_MEM_0_NEG_C);
         OUT_RELOC(ring, query_sample_idx(a6xx_backend, i, result)); /* dst */
         OUT_RELOC(ring, query_sample_idx(a6xx_backend, i, result)); /* srcA */
         OUT_RELOC(ring, query_sample_idx(a6xx_backend, i, stop));   /* srcB */
         OUT_RELOC(ring, query_sample_idx(a6xx_backend, i, start));  /* srcC */
      }
   }

   cache_flush<CHIP>(ring, kernel);
}

static void
a6xx_set_perfcntrs(struct backend *b, const struct perfcntr *perfcntrs,
                   unsigned num_perfcntrs)
{
   struct a6xx_backend *a6xx_backend = to_a6xx_backend(b);

   a6xx_backend->perfcntrs = perfcntrs;
   a6xx_backend->num_perfcntrs = num_perfcntrs;
}

static void
a6xx_read_perfcntrs(struct backend *b, uint64_t *results)
{
   struct a6xx_backend *a6xx_backend = to_a6xx_backend(b);

   fd_bo_cpu_prep(a6xx_backend->query_mem, NULL, FD_BO_PREP_READ);
   struct fd6_query_sample *samples =
      (struct fd6_query_sample *)fd_bo_map(a6xx_backend->query_mem);

   for (unsigned i = 0; i < a6xx_backend->num_perfcntrs; i++) {
      results[i] = samples[i].result;
   }
}

template<chip CHIP>
struct backend *
a6xx_init(struct fd_device *dev, const struct fd_dev_id *dev_id)
{
   struct a6xx_backend *a6xx_backend =
      (struct a6xx_backend *)calloc(1, sizeof(*a6xx_backend));

   a6xx_backend->base = (struct backend){
      .assemble = a6xx_assemble,
      .disassemble = a6xx_disassemble,
      .emit_grid = a6xx_emit_grid<CHIP>,
      .set_perfcntrs = a6xx_set_perfcntrs,
      .read_perfcntrs = a6xx_read_perfcntrs,
   };

   struct ir3_compiler_options compiler_options = {};
   a6xx_backend->compiler =
      ir3_compiler_create(dev, dev_id, fd_dev_info_raw(dev_id), &compiler_options);
   a6xx_backend->dev = dev;

   a6xx_backend->info = fd_dev_info_raw(dev_id);

   a6xx_backend->control_mem =
      fd_bo_new(dev, 0x1000, 0, "control");

   return &a6xx_backend->base;
}

template
struct backend *a6xx_init<A6XX>(struct fd_device *dev, const struct fd_dev_id *dev_id);

template
struct backend *a6xx_init<A7XX>(struct fd_device *dev, const struct fd_dev_id *dev_id);
