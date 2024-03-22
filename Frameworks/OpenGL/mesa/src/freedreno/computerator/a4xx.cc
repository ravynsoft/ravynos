/*
 * Copyright © 2021 Ilia Mirkin
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
#include "util/u_queue.h"
#include "util/half_float.h"

#include "adreno_pm4.xml.h"
#include "adreno_common.xml.h"
#include "a4xx.xml.h"

#include "ir3_asm.h"
#include "main.h"

struct a4xx_backend {
   struct backend base;

   struct ir3_compiler *compiler;
   struct fd_device *dev;
};
define_cast(backend, a4xx_backend);

/*
 * Backend implementation:
 */

static struct kernel *
a4xx_assemble(struct backend *b, FILE *in)
{
   struct a4xx_backend *a4xx_backend = to_a4xx_backend(b);
   struct ir3_kernel *ir3_kernel = ir3_asm_assemble(a4xx_backend->compiler, in);
   ir3_kernel->backend = b;
   return &ir3_kernel->base;
}

static void
a4xx_disassemble(struct kernel *kernel, FILE *out)
{
   ir3_asm_disassemble(to_ir3_kernel(kernel), out);
}

static void
cs_program_emit(struct fd_ringbuffer *ring, struct kernel *kernel)
{
   struct ir3_kernel *ir3_kernel = to_ir3_kernel(kernel);
   struct ir3_shader_variant *v = ir3_kernel->v;
   const struct ir3_info *i = &v->info;
   enum a3xx_threadsize thrsz = i->double_threadsize ? FOUR_QUADS : TWO_QUADS;

   OUT_PKT0(ring, REG_A4XX_UCHE_INVALIDATE0, 2);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, 0x00000012);

   OUT_WFI(ring);

   OUT_PKT0(ring, REG_A4XX_SP_MODE_CONTROL, 1);
   OUT_RING(ring, 0x0000001e);

   OUT_PKT0(ring, REG_A4XX_TPL1_TP_MODE_CONTROL, 1);
   OUT_RING(ring, 0x00000038);

   OUT_PKT0(ring, REG_A4XX_TPL1_TP_FS_TEX_COUNT, 1);
   OUT_RING(ring, 0x00000000);

   OUT_WFI(ring);

   OUT_PKT0(ring, REG_A4XX_HLSQ_MODE_CONTROL, 1);
   OUT_RING(ring, 0x00000003);

   OUT_PKT0(ring, REG_A4XX_HLSQ_CONTROL_0_REG, 1);
   OUT_RING(ring, 0x080005f0);

   OUT_PKT0(ring, REG_A4XX_HLSQ_UPDATE_CONTROL, 1);
   OUT_RING(ring, 0x00000038);

   OUT_PKT0(ring, REG_A4XX_SP_SP_CTRL_REG, 1);
   OUT_RING(ring, 0x00860010);
   // OUT_RING(ring, 0x00920000);

   OUT_PKT0(ring, REG_A4XX_SP_INSTR_CACHE_CTRL, 1);
   OUT_RING(ring, 0x000004ff);
   // OUT_RING(ring, 0x00000260);

   OUT_PKT0(ring, REG_A4XX_SP_FS_CTRL_REG1, 1);
   OUT_RING(ring, 0x80000000);

   OUT_PKT0(ring, REG_A4XX_SP_CS_CTRL_REG0, 1);
   OUT_RING(ring,
            A4XX_SP_CS_CTRL_REG0_THREADSIZE(thrsz) |
            A4XX_SP_CS_CTRL_REG0_SUPERTHREADMODE |
            A4XX_SP_CS_CTRL_REG0_HALFREGFOOTPRINT(i->max_half_reg + 1) |
            A4XX_SP_CS_CTRL_REG0_FULLREGFOOTPRINT(i->max_reg + 1));

   OUT_PKT0(ring, REG_A4XX_HLSQ_CS_CONTROL_REG, 1);
   OUT_RING(ring, A4XX_HLSQ_CS_CONTROL_REG_CONSTOBJECTOFFSET(0) |
                     A4XX_HLSQ_CS_CONTROL_REG_SHADEROBJOFFSET(0) |
                     A4XX_HLSQ_CS_CONTROL_REG_ENABLED |
                     A4XX_HLSQ_CS_CONTROL_REG_INSTRLENGTH(1) |
                     COND(v->has_ssbo, A4XX_HLSQ_CS_CONTROL_REG_SSBO_ENABLE) |
                     A4XX_HLSQ_CS_CONTROL_REG_CONSTLENGTH(v->constlen / 4));

   OUT_PKT0(ring, REG_A4XX_SP_CS_OBJ_START, 1);
   OUT_RELOC(ring, v->bo, 0, 0, 0); /* SP_CS_OBJ_START */

   OUT_PKT0(ring, REG_A4XX_SP_CS_LENGTH_REG, 1);
   OUT_RING(ring, v->instrlen);

   uint32_t local_invocation_id, work_group_id, num_wg_id;
   local_invocation_id =
      ir3_find_sysval_regid(v, SYSTEM_VALUE_LOCAL_INVOCATION_ID);
   work_group_id = ir3_kernel->info.wgid;
   num_wg_id = ir3_kernel->info.numwg;

   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_CONTROL_0, 2);
   OUT_RING(ring, A4XX_HLSQ_CL_CONTROL_0_WGIDCONSTID(work_group_id) |
                     A4XX_HLSQ_CL_CONTROL_0_KERNELDIMCONSTID(regid(63, 0)) |
                     A4XX_HLSQ_CL_CONTROL_0_LOCALIDREGID(local_invocation_id));
   OUT_RING(ring, A4XX_HLSQ_CL_CONTROL_1_UNK0CONSTID(regid(63, 0)) |
                     A4XX_HLSQ_CL_CONTROL_1_WORKGROUPSIZECONSTID(regid(63, 0)));

   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_KERNEL_CONST, 1);
   OUT_RING(ring, A4XX_HLSQ_CL_KERNEL_CONST_UNK0CONSTID(regid(63, 0)) |
                     A4XX_HLSQ_CL_KERNEL_CONST_NUMWGCONSTID(num_wg_id));

   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_WG_OFFSET, 1);
   OUT_RING(ring, A4XX_HLSQ_CL_WG_OFFSET_UNK0CONSTID(regid(63, 0)));

   OUT_PKT3(ring, CP_LOAD_STATE4, 2);
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
                     CP_LOAD_STATE4_0_STATE_SRC(SS4_INDIRECT) |
                     CP_LOAD_STATE4_0_STATE_BLOCK(SB4_CS_SHADER) |
                     CP_LOAD_STATE4_0_NUM_UNIT(v->instrlen));
   OUT_RELOC(ring, v->bo, 0, CP_LOAD_STATE4_1_STATE_TYPE(ST4_SHADER), 0);
}

static void
emit_const(struct fd_ringbuffer *ring, struct kernel *kernel, uint32_t constid, uint32_t sizedwords,
           const uint32_t *dwords)
{
   uint32_t align_sz;

   assert((constid % 4) == 0);

   /* Overwrite appropriate entries with buffer addresses */
   struct fd_bo **replacements =
      (struct fd_bo **)calloc(sizedwords, sizeof(struct fd_bo *));
   for (int i = 0; i < MAX_BUFS; i++) {
      if (kernel->buf_addr_regs[i] != INVALID_REG) {
         int idx = kernel->buf_addr_regs[i];
         assert(idx < sizedwords);

         replacements[idx] = kernel->bufs[i];
      }
   }

   align_sz = align(sizedwords, 4);

   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + align_sz);
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(constid / 4) |
                     CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
                     CP_LOAD_STATE4_0_STATE_BLOCK(SB4_CS_SHADER) |
                     CP_LOAD_STATE4_0_NUM_UNIT(DIV_ROUND_UP(sizedwords, 4)));
   OUT_RING(ring, CP_LOAD_STATE4_1_EXT_SRC_ADDR(0) |
                     CP_LOAD_STATE4_1_STATE_TYPE(ST4_CONSTANTS));
   for (unsigned i = 0; i < sizedwords; i++) {
      if (replacements[i])
         OUT_RELOC(ring, replacements[i], 0, 0, 0);
      else
         OUT_RING(ring, dwords[i]);
   }

   /* Zero-pad to multiple of 4 dwords */
   for (uint32_t i = sizedwords; i < align_sz; i++) {
      OUT_RING(ring, 0);
   }

   free(replacements);
}

static void
cs_const_emit(struct fd_ringbuffer *ring, struct kernel *kernel,
              uint32_t grid[3])
{
   struct ir3_kernel *ir3_kernel = to_ir3_kernel(kernel);
   struct ir3_shader_variant *v = ir3_kernel->v;

   const struct ir3_const_state *const_state = ir3_const_state(v);
   uint32_t base = const_state->offsets.immediate;
   int size = DIV_ROUND_UP(const_state->immediates_count, 4);

   /* truncate size to avoid writing constants that shader
    * does not use:
    */
   size = MIN2(size + base, v->constlen) - base;

   /* convert out of vec4: */
   base *= 4;
   size *= 4;

   if (size > 0) {
      emit_const(ring, kernel, base, size, const_state->immediates);
   }
}

static void
cs_ibo_emit(struct fd_ringbuffer *ring, struct fd_submit *submit,
            struct kernel *kernel)
{
   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + (4 * kernel->num_bufs));
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
         CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
         CP_LOAD_STATE4_0_STATE_BLOCK(SB4_CS_SSBO) |
         CP_LOAD_STATE4_0_NUM_UNIT(kernel->num_bufs));
   OUT_RING(ring, CP_LOAD_STATE4_1_STATE_TYPE(ST4_SHADER) |
         CP_LOAD_STATE4_1_EXT_SRC_ADDR(0));
   for (unsigned i = 0; i < kernel->num_bufs; i++) {
      OUT_RELOC(ring, kernel->bufs[i], 0, 0, 0);
#if 1
      OUT_RING(ring, 0);
      OUT_RING(ring, 0);
      OUT_RING(ring, 0);
#else
      OUT_RING(ring, kernel->buf_sizes[i]);
      OUT_RING(ring, kernel->buf_sizes[i]);
      OUT_RING(ring, 0x00000004);
#endif
   }

   OUT_PKT3(ring, CP_LOAD_STATE4, 2 + (2 * kernel->num_bufs));
   OUT_RING(ring, CP_LOAD_STATE4_0_DST_OFF(0) |
         CP_LOAD_STATE4_0_STATE_SRC(SS4_DIRECT) |
         CP_LOAD_STATE4_0_STATE_BLOCK(SB4_CS_SSBO) |
         CP_LOAD_STATE4_0_NUM_UNIT(kernel->num_bufs));
   OUT_RING(ring, CP_LOAD_STATE4_1_STATE_TYPE(ST4_CONSTANTS) |
         CP_LOAD_STATE4_1_EXT_SRC_ADDR(0));
   for (unsigned i = 0; i < kernel->num_bufs; i++) {
      unsigned sz = kernel->buf_sizes[i];

      /* width is in dwords, overflows into height: */
      sz /= 4;

#if 1
      OUT_RING(ring, A4XX_SSBO_1_0_WIDTH(sz));
      OUT_RING(ring, A4XX_SSBO_1_1_HEIGHT(sz >> 16));
#else
      OUT_RING(ring, A4XX_SSBO_1_0_WIDTH(sz) |
            A4XX_SSBO_1_0_FMT(RB4_R32_UINT) |
            A4XX_SSBO_1_0_CPP(4));
      OUT_RING(ring, A4XX_SSBO_1_1_HEIGHT(DIV_ROUND_UP(sz, 1 << 16)) |
            A4XX_SSBO_1_1_DEPTH(1));
#endif
   }
}

static void
a4xx_emit_grid(struct kernel *kernel, uint32_t grid[3],
               struct fd_submit *submit)
{
   struct fd_ringbuffer *ring = fd_submit_new_ringbuffer(
      submit, 0,
      (enum fd_ringbuffer_flags)(FD_RINGBUFFER_PRIMARY | FD_RINGBUFFER_GROWABLE));

   cs_program_emit(ring, kernel);
   cs_const_emit(ring, kernel, grid);
   cs_ibo_emit(ring, submit, kernel);

   const unsigned *local_size = kernel->local_size;
   const unsigned *num_groups = grid;

   unsigned work_dim = 0;
   for (int i = 0; i < 3; i++) {
      if (!grid[i])
         break;
      work_dim++;
   }

   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_NDRANGE_0, 7);
   OUT_RING(ring, A4XX_HLSQ_CL_NDRANGE_0_KERNELDIM(work_dim) |
                     A4XX_HLSQ_CL_NDRANGE_0_LOCALSIZEX(local_size[0] - 1) |
                     A4XX_HLSQ_CL_NDRANGE_0_LOCALSIZEY(local_size[1] - 1) |
                     A4XX_HLSQ_CL_NDRANGE_0_LOCALSIZEZ(local_size[2] - 1));
   OUT_RING(ring,
            A4XX_HLSQ_CL_NDRANGE_1_SIZE_X(local_size[0] * num_groups[0]));
   OUT_RING(ring, 0); /* HLSQ_CL_NDRANGE_2_GLOBALOFF_X */
   OUT_RING(ring,
            A4XX_HLSQ_CL_NDRANGE_3_SIZE_Y(local_size[1] * num_groups[1]));
   OUT_RING(ring, 0); /* HLSQ_CL_NDRANGE_4_GLOBALOFF_Y */
   OUT_RING(ring,
            A4XX_HLSQ_CL_NDRANGE_5_SIZE_Z(local_size[2] * num_groups[2]));
   OUT_RING(ring, 0); /* HLSQ_CL_NDRANGE_6_GLOBALOFF_Z */

#if 1
   OUT_PKT3(ring, CP_EXEC_CS, 4);
   OUT_RING(ring, 0x00000000);
   OUT_RING(ring, CP_EXEC_CS_1_NGROUPS_X(grid[0]));
   OUT_RING(ring, CP_EXEC_CS_2_NGROUPS_Y(grid[1]));
   OUT_RING(ring, CP_EXEC_CS_3_NGROUPS_Z(grid[2]));
#else
   OUT_PKT0(ring, REG_A4XX_HLSQ_CL_KERNEL_GROUP_X, 3);
   OUT_RING(ring, grid[0]); /* HLSQ_CL_KERNEL_GROUP_X */
   OUT_RING(ring, grid[1]); /* HLSQ_CL_KERNEL_GROUP_Y */
   OUT_RING(ring, grid[2]); /* HLSQ_CL_KERNEL_GROUP_Z */

   OUT_PKT3(ring, CP_RUN_OPENCL, 1);
   OUT_RING(ring, 0);
#endif

   OUT_WFI(ring);

   /* TODO: cache_flush */
}

struct backend *
a4xx_init(struct fd_device *dev, const struct fd_dev_id *dev_id)
{
   struct a4xx_backend *a4xx_backend =
      (struct a4xx_backend *)calloc(1, sizeof(*a4xx_backend));

   a4xx_backend->base = (struct backend){
      .assemble = a4xx_assemble,
      .disassemble = a4xx_disassemble,
      .emit_grid = a4xx_emit_grid,
   };

   struct ir3_compiler_options compiler_options = {};
   a4xx_backend->compiler =
      ir3_compiler_create(dev, dev_id, fd_dev_info_raw(dev_id), &compiler_options);
   a4xx_backend->dev = dev;

   return &a4xx_backend->base;
}
