/**************************************************************************
 * 
 * Copyright 2007-2008 VMware, Inc.
 * All Rights Reserved.
 * Copyright 2009-2010 VMware, Inc.  All rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/**
 * TGSI interpreter/executor.
 *
 * Flow control information:
 *
 * Since we operate on 'quads' (4 pixels or 4 vertices in parallel)
 * flow control statements (IF/ELSE/ENDIF, LOOP/ENDLOOP) require special
 * care since a condition may be true for some quad components but false
 * for other components.
 *
 * We basically execute all statements (even if they're in the part of
 * an IF/ELSE clause that's "not taken") and use a special mask to
 * control writing to destination registers.  This is the ExecMask.
 * See store_dest().
 *
 * The ExecMask is computed from three other masks (CondMask, LoopMask and
 * ContMask) which are controlled by the flow control instructions (namely:
 * (IF/ELSE/ENDIF, LOOP/ENDLOOP and CONT).
 *
 *
 * Authors:
 *   Michal Krol
 *   Brian Paul
 */

#include "util/compiler.h"
#include "pipe/p_state.h"
#include "pipe/p_shader_tokens.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_util.h"
#include "tgsi_exec.h"
#include "util/compiler.h"
#include "util/half_float.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/rounding.h"


#define DEBUG_EXECUTION 0


#define TILE_TOP_LEFT     0
#define TILE_TOP_RIGHT    1
#define TILE_BOTTOM_LEFT  2
#define TILE_BOTTOM_RIGHT 3

static_assert(alignof(union tgsi_exec_channel) == 16, "");
static_assert(alignof(struct tgsi_exec_vector) == 16, "");
static_assert(alignof(struct tgsi_exec_machine) == 16, "");

union tgsi_double_channel {
   alignas(16)
   double d[TGSI_QUAD_SIZE];
   unsigned u[TGSI_QUAD_SIZE][2];
   uint64_t u64[TGSI_QUAD_SIZE];
   int64_t i64[TGSI_QUAD_SIZE];
};

struct tgsi_double_vector {
   alignas(16)
   union tgsi_double_channel xy;
   union tgsi_double_channel zw;
};

static_assert(alignof(union tgsi_double_channel) == 16, "");
static_assert(alignof(struct tgsi_double_vector) == 16, "");

static void
micro_abs(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = fabsf(src->f[0]);
   dst->f[1] = fabsf(src->f[1]);
   dst->f[2] = fabsf(src->f[2]);
   dst->f[3] = fabsf(src->f[3]);
}

static void
micro_arl(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->i[0] = (int)floorf(src->f[0]);
   dst->i[1] = (int)floorf(src->f[1]);
   dst->i[2] = (int)floorf(src->f[2]);
   dst->i[3] = (int)floorf(src->f[3]);
}

static void
micro_arr(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->i[0] = (int)floorf(src->f[0] + 0.5f);
   dst->i[1] = (int)floorf(src->f[1] + 0.5f);
   dst->i[2] = (int)floorf(src->f[2] + 0.5f);
   dst->i[3] = (int)floorf(src->f[3] + 0.5f);
}

static void
micro_ceil(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->f[0] = ceilf(src->f[0]);
   dst->f[1] = ceilf(src->f[1]);
   dst->f[2] = ceilf(src->f[2]);
   dst->f[3] = ceilf(src->f[3]);
}

static void
micro_cmp(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1,
          const union tgsi_exec_channel *src2)
{
   dst->f[0] = src0->f[0] < 0.0f ? src1->f[0] : src2->f[0];
   dst->f[1] = src0->f[1] < 0.0f ? src1->f[1] : src2->f[1];
   dst->f[2] = src0->f[2] < 0.0f ? src1->f[2] : src2->f[2];
   dst->f[3] = src0->f[3] < 0.0f ? src1->f[3] : src2->f[3];
}

static void
micro_cos(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = cosf(src->f[0]);
   dst->f[1] = cosf(src->f[1]);
   dst->f[2] = cosf(src->f[2]);
   dst->f[3] = cosf(src->f[3]);
}

static void
micro_d2f(union tgsi_exec_channel *dst,
          const union tgsi_double_channel *src)
{
   dst->f[0] = (float)src->d[0];
   dst->f[1] = (float)src->d[1];
   dst->f[2] = (float)src->d[2];
   dst->f[3] = (float)src->d[3];
}

static void
micro_d2i(union tgsi_exec_channel *dst,
          const union tgsi_double_channel *src)
{
   dst->i[0] = (int)src->d[0];
   dst->i[1] = (int)src->d[1];
   dst->i[2] = (int)src->d[2];
   dst->i[3] = (int)src->d[3];
}

static void
micro_d2u(union tgsi_exec_channel *dst,
          const union tgsi_double_channel *src)
{
   dst->u[0] = (unsigned)src->d[0];
   dst->u[1] = (unsigned)src->d[1];
   dst->u[2] = (unsigned)src->d[2];
   dst->u[3] = (unsigned)src->d[3];
}
static void
micro_dabs(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = src->d[0] >= 0.0 ? src->d[0] : -src->d[0];
   dst->d[1] = src->d[1] >= 0.0 ? src->d[1] : -src->d[1];
   dst->d[2] = src->d[2] >= 0.0 ? src->d[2] : -src->d[2];
   dst->d[3] = src->d[3] >= 0.0 ? src->d[3] : -src->d[3];
}

static void
micro_dadd(union tgsi_double_channel *dst,
          const union tgsi_double_channel *src)
{
   dst->d[0] = src[0].d[0] + src[1].d[0];
   dst->d[1] = src[0].d[1] + src[1].d[1];
   dst->d[2] = src[0].d[2] + src[1].d[2];
   dst->d[3] = src[0].d[3] + src[1].d[3];
}

static void
micro_ddiv(union tgsi_double_channel *dst,
          const union tgsi_double_channel *src)
{
   dst->d[0] = src[0].d[0] / src[1].d[0];
   dst->d[1] = src[0].d[1] / src[1].d[1];
   dst->d[2] = src[0].d[2] / src[1].d[2];
   dst->d[3] = src[0].d[3] / src[1].d[3];
}

static void
micro_ddx(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] =
   dst->f[1] =
   dst->f[2] =
   dst->f[3] = src->f[TILE_BOTTOM_RIGHT] - src->f[TILE_BOTTOM_LEFT];
}

static void
micro_ddx_fine(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] =
   dst->f[1] = src->f[TILE_TOP_RIGHT] - src->f[TILE_TOP_LEFT];
   dst->f[2] =
   dst->f[3] = src->f[TILE_BOTTOM_RIGHT] - src->f[TILE_BOTTOM_LEFT];
}


static void
micro_ddy(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] =
   dst->f[1] =
   dst->f[2] =
   dst->f[3] = src->f[TILE_BOTTOM_LEFT] - src->f[TILE_TOP_LEFT];
}

static void
micro_ddy_fine(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] =
   dst->f[2] = src->f[TILE_BOTTOM_LEFT] - src->f[TILE_TOP_LEFT];
   dst->f[1] =
   dst->f[3] = src->f[TILE_BOTTOM_RIGHT] - src->f[TILE_TOP_RIGHT];
}

static void
micro_dmul(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = src[0].d[0] * src[1].d[0];
   dst->d[1] = src[0].d[1] * src[1].d[1];
   dst->d[2] = src[0].d[2] * src[1].d[2];
   dst->d[3] = src[0].d[3] * src[1].d[3];
}

static void
micro_dmax(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = fmax(src[0].d[0], src[1].d[0]);
   dst->d[1] = fmax(src[0].d[1], src[1].d[1]);
   dst->d[2] = fmax(src[0].d[2], src[1].d[2]);
   dst->d[3] = fmax(src[0].d[3], src[1].d[3]);
}

static void
micro_dmin(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = fmin(src[0].d[0], src[1].d[0]);
   dst->d[1] = fmin(src[0].d[1], src[1].d[1]);
   dst->d[2] = fmin(src[0].d[2], src[1].d[2]);
   dst->d[3] = fmin(src[0].d[3], src[1].d[3]);
}

static void
micro_dneg(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = -src->d[0];
   dst->d[1] = -src->d[1];
   dst->d[2] = -src->d[2];
   dst->d[3] = -src->d[3];
}

static void
micro_dslt(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].d[0] < src[1].d[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].d[1] < src[1].d[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].d[2] < src[1].d[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].d[3] < src[1].d[3] ? ~0U : 0U;
}

static void
micro_dsne(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].d[0] != src[1].d[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].d[1] != src[1].d[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].d[2] != src[1].d[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].d[3] != src[1].d[3] ? ~0U : 0U;
}

static void
micro_dsge(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].d[0] >= src[1].d[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].d[1] >= src[1].d[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].d[2] >= src[1].d[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].d[3] >= src[1].d[3] ? ~0U : 0U;
}

static void
micro_dseq(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].d[0] == src[1].d[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].d[1] == src[1].d[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].d[2] == src[1].d[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].d[3] == src[1].d[3] ? ~0U : 0U;
}

static void
micro_drcp(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = 1.0 / src->d[0];
   dst->d[1] = 1.0 / src->d[1];
   dst->d[2] = 1.0 / src->d[2];
   dst->d[3] = 1.0 / src->d[3];
}

static void
micro_dsqrt(union tgsi_double_channel *dst,
            const union tgsi_double_channel *src)
{
   dst->d[0] = sqrt(src->d[0]);
   dst->d[1] = sqrt(src->d[1]);
   dst->d[2] = sqrt(src->d[2]);
   dst->d[3] = sqrt(src->d[3]);
}

static void
micro_drsq(union tgsi_double_channel *dst,
          const union tgsi_double_channel *src)
{
   dst->d[0] = 1.0 / sqrt(src->d[0]);
   dst->d[1] = 1.0 / sqrt(src->d[1]);
   dst->d[2] = 1.0 / sqrt(src->d[2]);
   dst->d[3] = 1.0 / sqrt(src->d[3]);
}

static void
micro_dmad(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = src[0].d[0] * src[1].d[0] + src[2].d[0];
   dst->d[1] = src[0].d[1] * src[1].d[1] + src[2].d[1];
   dst->d[2] = src[0].d[2] * src[1].d[2] + src[2].d[2];
   dst->d[3] = src[0].d[3] * src[1].d[3] + src[2].d[3];
}

static void
micro_dfrac(union tgsi_double_channel *dst,
            const union tgsi_double_channel *src)
{
   dst->d[0] = src->d[0] - floor(src->d[0]);
   dst->d[1] = src->d[1] - floor(src->d[1]);
   dst->d[2] = src->d[2] - floor(src->d[2]);
   dst->d[3] = src->d[3] - floor(src->d[3]);
}

static void
micro_dflr(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = floor(src->d[0]);
   dst->d[1] = floor(src->d[1]);
   dst->d[2] = floor(src->d[2]);
   dst->d[3] = floor(src->d[3]);
}

static void
micro_dldexp(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src0,
             union tgsi_exec_channel *src1)
{
   dst->d[0] = ldexp(src0->d[0], src1->i[0]);
   dst->d[1] = ldexp(src0->d[1], src1->i[1]);
   dst->d[2] = ldexp(src0->d[2], src1->i[2]);
   dst->d[3] = ldexp(src0->d[3], src1->i[3]);
}

static void
micro_exp2(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
#if DEBUG
   /* Inf is okay for this instruction, so clamp it to silence assertions. */
   unsigned i;
   union tgsi_exec_channel clamped;

   for (i = 0; i < 4; i++) {
      if (src->f[i] > 127.99999f) {
         clamped.f[i] = 127.99999f;
      } else if (src->f[i] < -126.99999f) {
         clamped.f[i] = -126.99999f;
      } else {
         clamped.f[i] = src->f[i];
      }
   }
   src = &clamped;
#endif /* DEBUG */

   dst->f[0] = powf(2.0f, src->f[0]);
   dst->f[1] = powf(2.0f, src->f[1]);
   dst->f[2] = powf(2.0f, src->f[2]);
   dst->f[3] = powf(2.0f, src->f[3]);
}

static void
micro_f2d(union tgsi_double_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->d[0] = (double)src->f[0];
   dst->d[1] = (double)src->f[1];
   dst->d[2] = (double)src->f[2];
   dst->d[3] = (double)src->f[3];
}

static void
micro_flr(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = floorf(src->f[0]);
   dst->f[1] = floorf(src->f[1]);
   dst->f[2] = floorf(src->f[2]);
   dst->f[3] = floorf(src->f[3]);
}

static void
micro_frc(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = src->f[0] - floorf(src->f[0]);
   dst->f[1] = src->f[1] - floorf(src->f[1]);
   dst->f[2] = src->f[2] - floorf(src->f[2]);
   dst->f[3] = src->f[3] - floorf(src->f[3]);
}

static void
micro_i2d(union tgsi_double_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->d[0] = (double)src->i[0];
   dst->d[1] = (double)src->i[1];
   dst->d[2] = (double)src->i[2];
   dst->d[3] = (double)src->i[3];
}

static void
micro_iabs(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->i[0] = src->i[0] >= 0 ? src->i[0] : -src->i[0];
   dst->i[1] = src->i[1] >= 0 ? src->i[1] : -src->i[1];
   dst->i[2] = src->i[2] >= 0 ? src->i[2] : -src->i[2];
   dst->i[3] = src->i[3] >= 0 ? src->i[3] : -src->i[3];
}

static void
micro_ineg(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->i[0] = -src->i[0];
   dst->i[1] = -src->i[1];
   dst->i[2] = -src->i[2];
   dst->i[3] = -src->i[3];
}

static void
micro_lg2(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = logf(src->f[0]) * 1.442695f;
   dst->f[1] = logf(src->f[1]) * 1.442695f;
   dst->f[2] = logf(src->f[2]) * 1.442695f;
   dst->f[3] = logf(src->f[3]) * 1.442695f;
}

static void
micro_lrp(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1,
          const union tgsi_exec_channel *src2)
{
   dst->f[0] = src0->f[0] * (src1->f[0] - src2->f[0]) + src2->f[0];
   dst->f[1] = src0->f[1] * (src1->f[1] - src2->f[1]) + src2->f[1];
   dst->f[2] = src0->f[2] * (src1->f[2] - src2->f[2]) + src2->f[2];
   dst->f[3] = src0->f[3] * (src1->f[3] - src2->f[3]) + src2->f[3];
}

static void
micro_mad(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1,
          const union tgsi_exec_channel *src2)
{
   dst->f[0] = src0->f[0] * src1->f[0] + src2->f[0];
   dst->f[1] = src0->f[1] * src1->f[1] + src2->f[1];
   dst->f[2] = src0->f[2] * src1->f[2] + src2->f[2];
   dst->f[3] = src0->f[3] * src1->f[3] + src2->f[3];
}

static void
micro_mov(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->u[0] = src->u[0];
   dst->u[1] = src->u[1];
   dst->u[2] = src->u[2];
   dst->u[3] = src->u[3];
}

static void
micro_rcp(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
#if 0 /* for debugging */
   assert(src->f[0] != 0.0f);
   assert(src->f[1] != 0.0f);
   assert(src->f[2] != 0.0f);
   assert(src->f[3] != 0.0f);
#endif
   dst->f[0] = 1.0f / src->f[0];
   dst->f[1] = 1.0f / src->f[1];
   dst->f[2] = 1.0f / src->f[2];
   dst->f[3] = 1.0f / src->f[3];
}

static void
micro_rnd(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = _mesa_roundevenf(src->f[0]);
   dst->f[1] = _mesa_roundevenf(src->f[1]);
   dst->f[2] = _mesa_roundevenf(src->f[2]);
   dst->f[3] = _mesa_roundevenf(src->f[3]);
}

static void
micro_rsq(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
#if 0 /* for debugging */
   assert(src->f[0] != 0.0f);
   assert(src->f[1] != 0.0f);
   assert(src->f[2] != 0.0f);
   assert(src->f[3] != 0.0f);
#endif
   dst->f[0] = 1.0f / sqrtf(src->f[0]);
   dst->f[1] = 1.0f / sqrtf(src->f[1]);
   dst->f[2] = 1.0f / sqrtf(src->f[2]);
   dst->f[3] = 1.0f / sqrtf(src->f[3]);
}

static void
micro_sqrt(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->f[0] = sqrtf(src->f[0]);
   dst->f[1] = sqrtf(src->f[1]);
   dst->f[2] = sqrtf(src->f[2]);
   dst->f[3] = sqrtf(src->f[3]);
}

static void
micro_seq(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] == src1->f[0] ? 1.0f : 0.0f;
   dst->f[1] = src0->f[1] == src1->f[1] ? 1.0f : 0.0f;
   dst->f[2] = src0->f[2] == src1->f[2] ? 1.0f : 0.0f;
   dst->f[3] = src0->f[3] == src1->f[3] ? 1.0f : 0.0f;
}

static void
micro_sge(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] >= src1->f[0] ? 1.0f : 0.0f;
   dst->f[1] = src0->f[1] >= src1->f[1] ? 1.0f : 0.0f;
   dst->f[2] = src0->f[2] >= src1->f[2] ? 1.0f : 0.0f;
   dst->f[3] = src0->f[3] >= src1->f[3] ? 1.0f : 0.0f;
}

static void
micro_sgn(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = src->f[0] < 0.0f ? -1.0f : src->f[0] > 0.0f ? 1.0f : 0.0f;
   dst->f[1] = src->f[1] < 0.0f ? -1.0f : src->f[1] > 0.0f ? 1.0f : 0.0f;
   dst->f[2] = src->f[2] < 0.0f ? -1.0f : src->f[2] > 0.0f ? 1.0f : 0.0f;
   dst->f[3] = src->f[3] < 0.0f ? -1.0f : src->f[3] > 0.0f ? 1.0f : 0.0f;
}

static void
micro_isgn(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->i[0] = src->i[0] < 0 ? -1 : src->i[0] > 0 ? 1 : 0;
   dst->i[1] = src->i[1] < 0 ? -1 : src->i[1] > 0 ? 1 : 0;
   dst->i[2] = src->i[2] < 0 ? -1 : src->i[2] > 0 ? 1 : 0;
   dst->i[3] = src->i[3] < 0 ? -1 : src->i[3] > 0 ? 1 : 0;
}

static void
micro_sgt(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] > src1->f[0] ? 1.0f : 0.0f;
   dst->f[1] = src0->f[1] > src1->f[1] ? 1.0f : 0.0f;
   dst->f[2] = src0->f[2] > src1->f[2] ? 1.0f : 0.0f;
   dst->f[3] = src0->f[3] > src1->f[3] ? 1.0f : 0.0f;
}

static void
micro_sin(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = sinf(src->f[0]);
   dst->f[1] = sinf(src->f[1]);
   dst->f[2] = sinf(src->f[2]);
   dst->f[3] = sinf(src->f[3]);
}

static void
micro_sle(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] <= src1->f[0] ? 1.0f : 0.0f;
   dst->f[1] = src0->f[1] <= src1->f[1] ? 1.0f : 0.0f;
   dst->f[2] = src0->f[2] <= src1->f[2] ? 1.0f : 0.0f;
   dst->f[3] = src0->f[3] <= src1->f[3] ? 1.0f : 0.0f;
}

static void
micro_slt(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] < src1->f[0] ? 1.0f : 0.0f;
   dst->f[1] = src0->f[1] < src1->f[1] ? 1.0f : 0.0f;
   dst->f[2] = src0->f[2] < src1->f[2] ? 1.0f : 0.0f;
   dst->f[3] = src0->f[3] < src1->f[3] ? 1.0f : 0.0f;
}

static void
micro_sne(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] != src1->f[0] ? 1.0f : 0.0f;
   dst->f[1] = src0->f[1] != src1->f[1] ? 1.0f : 0.0f;
   dst->f[2] = src0->f[2] != src1->f[2] ? 1.0f : 0.0f;
   dst->f[3] = src0->f[3] != src1->f[3] ? 1.0f : 0.0f;
}

static void
micro_trunc(union tgsi_exec_channel *dst,
            const union tgsi_exec_channel *src)
{
   dst->f[0] = truncf(src->f[0]);
   dst->f[1] = truncf(src->f[1]);
   dst->f[2] = truncf(src->f[2]);
   dst->f[3] = truncf(src->f[3]);
}

static void
micro_u2d(union tgsi_double_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->d[0] = (double)src->u[0];
   dst->d[1] = (double)src->u[1];
   dst->d[2] = (double)src->u[2];
   dst->d[3] = (double)src->u[3];
}

static void
micro_i64abs(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->i64[0] = src->i64[0] >= 0.0 ? src->i64[0] : -src->i64[0];
   dst->i64[1] = src->i64[1] >= 0.0 ? src->i64[1] : -src->i64[1];
   dst->i64[2] = src->i64[2] >= 0.0 ? src->i64[2] : -src->i64[2];
   dst->i64[3] = src->i64[3] >= 0.0 ? src->i64[3] : -src->i64[3];
}

static void
micro_i64sgn(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->i64[0] = src->i64[0] < 0 ? -1 : src->i64[0] > 0 ? 1 : 0;
   dst->i64[1] = src->i64[1] < 0 ? -1 : src->i64[1] > 0 ? 1 : 0;
   dst->i64[2] = src->i64[2] < 0 ? -1 : src->i64[2] > 0 ? 1 : 0;
   dst->i64[3] = src->i64[3] < 0 ? -1 : src->i64[3] > 0 ? 1 : 0;
}

static void
micro_i64neg(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->i64[0] = -src->i64[0];
   dst->i64[1] = -src->i64[1];
   dst->i64[2] = -src->i64[2];
   dst->i64[3] = -src->i64[3];
}

static void
micro_u64seq(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].u64[0] == src[1].u64[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].u64[1] == src[1].u64[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].u64[2] == src[1].u64[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].u64[3] == src[1].u64[3] ? ~0U : 0U;
}

static void
micro_u64sne(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].u64[0] != src[1].u64[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].u64[1] != src[1].u64[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].u64[2] != src[1].u64[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].u64[3] != src[1].u64[3] ? ~0U : 0U;
}

static void
micro_i64slt(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].i64[0] < src[1].i64[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].i64[1] < src[1].i64[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].i64[2] < src[1].i64[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].i64[3] < src[1].i64[3] ? ~0U : 0U;
}

static void
micro_u64slt(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].u64[0] < src[1].u64[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].u64[1] < src[1].u64[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].u64[2] < src[1].u64[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].u64[3] < src[1].u64[3] ? ~0U : 0U;
}

static void
micro_i64sge(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].i64[0] >= src[1].i64[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].i64[1] >= src[1].i64[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].i64[2] >= src[1].i64[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].i64[3] >= src[1].i64[3] ? ~0U : 0U;
}

static void
micro_u64sge(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u[0][0] = src[0].u64[0] >= src[1].u64[0] ? ~0U : 0U;
   dst->u[1][0] = src[0].u64[1] >= src[1].u64[1] ? ~0U : 0U;
   dst->u[2][0] = src[0].u64[2] >= src[1].u64[2] ? ~0U : 0U;
   dst->u[3][0] = src[0].u64[3] >= src[1].u64[3] ? ~0U : 0U;
}

static void
micro_u64max(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u64[0] = src[0].u64[0] > src[1].u64[0] ? src[0].u64[0] : src[1].u64[0];
   dst->u64[1] = src[0].u64[1] > src[1].u64[1] ? src[0].u64[1] : src[1].u64[1];
   dst->u64[2] = src[0].u64[2] > src[1].u64[2] ? src[0].u64[2] : src[1].u64[2];
   dst->u64[3] = src[0].u64[3] > src[1].u64[3] ? src[0].u64[3] : src[1].u64[3];
}

static void
micro_i64max(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->i64[0] = src[0].i64[0] > src[1].i64[0] ? src[0].i64[0] : src[1].i64[0];
   dst->i64[1] = src[0].i64[1] > src[1].i64[1] ? src[0].i64[1] : src[1].i64[1];
   dst->i64[2] = src[0].i64[2] > src[1].i64[2] ? src[0].i64[2] : src[1].i64[2];
   dst->i64[3] = src[0].i64[3] > src[1].i64[3] ? src[0].i64[3] : src[1].i64[3];
}

static void
micro_u64min(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u64[0] = src[0].u64[0] < src[1].u64[0] ? src[0].u64[0] : src[1].u64[0];
   dst->u64[1] = src[0].u64[1] < src[1].u64[1] ? src[0].u64[1] : src[1].u64[1];
   dst->u64[2] = src[0].u64[2] < src[1].u64[2] ? src[0].u64[2] : src[1].u64[2];
   dst->u64[3] = src[0].u64[3] < src[1].u64[3] ? src[0].u64[3] : src[1].u64[3];
}

static void
micro_i64min(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->i64[0] = src[0].i64[0] < src[1].i64[0] ? src[0].i64[0] : src[1].i64[0];
   dst->i64[1] = src[0].i64[1] < src[1].i64[1] ? src[0].i64[1] : src[1].i64[1];
   dst->i64[2] = src[0].i64[2] < src[1].i64[2] ? src[0].i64[2] : src[1].i64[2];
   dst->i64[3] = src[0].i64[3] < src[1].i64[3] ? src[0].i64[3] : src[1].i64[3];
}

static void
micro_u64add(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u64[0] = src[0].u64[0] + src[1].u64[0];
   dst->u64[1] = src[0].u64[1] + src[1].u64[1];
   dst->u64[2] = src[0].u64[2] + src[1].u64[2];
   dst->u64[3] = src[0].u64[3] + src[1].u64[3];
}

static void
micro_u64mul(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u64[0] = src[0].u64[0] * src[1].u64[0];
   dst->u64[1] = src[0].u64[1] * src[1].u64[1];
   dst->u64[2] = src[0].u64[2] * src[1].u64[2];
   dst->u64[3] = src[0].u64[3] * src[1].u64[3];
}

static void
micro_u64div(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u64[0] = src[1].u64[0] ? src[0].u64[0] / src[1].u64[0] : ~0ull;
   dst->u64[1] = src[1].u64[1] ? src[0].u64[1] / src[1].u64[1] : ~0ull;
   dst->u64[2] = src[1].u64[2] ? src[0].u64[2] / src[1].u64[2] : ~0ull;
   dst->u64[3] = src[1].u64[3] ? src[0].u64[3] / src[1].u64[3] : ~0ull;
}

static void
micro_i64div(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->i64[0] = src[1].i64[0] ? src[0].i64[0] / src[1].i64[0] : 0;
   dst->i64[1] = src[1].i64[1] ? src[0].i64[1] / src[1].i64[1] : 0;
   dst->i64[2] = src[1].i64[2] ? src[0].i64[2] / src[1].i64[2] : 0;
   dst->i64[3] = src[1].i64[3] ? src[0].i64[3] / src[1].i64[3] : 0;
}

static void
micro_u64mod(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->u64[0] = src[1].u64[0] ? src[0].u64[0] % src[1].u64[0] : ~0ull;
   dst->u64[1] = src[1].u64[1] ? src[0].u64[1] % src[1].u64[1] : ~0ull;
   dst->u64[2] = src[1].u64[2] ? src[0].u64[2] % src[1].u64[2] : ~0ull;
   dst->u64[3] = src[1].u64[3] ? src[0].u64[3] % src[1].u64[3] : ~0ull;
}

static void
micro_i64mod(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src)
{
   dst->i64[0] = src[1].i64[0] ? src[0].i64[0] % src[1].i64[0] : ~0ll;
   dst->i64[1] = src[1].i64[1] ? src[0].i64[1] % src[1].i64[1] : ~0ll;
   dst->i64[2] = src[1].i64[2] ? src[0].i64[2] % src[1].i64[2] : ~0ll;
   dst->i64[3] = src[1].i64[3] ? src[0].i64[3] % src[1].i64[3] : ~0ll;
}

static void
micro_u64shl(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src0,
             union tgsi_exec_channel *src1)
{
   unsigned masked_count;
   masked_count = src1->u[0] & 0x3f;
   dst->u64[0] = src0->u64[0] << masked_count;
   masked_count = src1->u[1] & 0x3f;
   dst->u64[1] = src0->u64[1] << masked_count;
   masked_count = src1->u[2] & 0x3f;
   dst->u64[2] = src0->u64[2] << masked_count;
   masked_count = src1->u[3] & 0x3f;
   dst->u64[3] = src0->u64[3] << masked_count;
}

static void
micro_i64shr(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src0,
             union tgsi_exec_channel *src1)
{
   unsigned masked_count;
   masked_count = src1->u[0] & 0x3f;
   dst->i64[0] = src0->i64[0] >> masked_count;
   masked_count = src1->u[1] & 0x3f;
   dst->i64[1] = src0->i64[1] >> masked_count;
   masked_count = src1->u[2] & 0x3f;
   dst->i64[2] = src0->i64[2] >> masked_count;
   masked_count = src1->u[3] & 0x3f;
   dst->i64[3] = src0->i64[3] >> masked_count;
}

static void
micro_u64shr(union tgsi_double_channel *dst,
             const union tgsi_double_channel *src0,
             union tgsi_exec_channel *src1)
{
   unsigned masked_count;
   masked_count = src1->u[0] & 0x3f;
   dst->u64[0] = src0->u64[0] >> masked_count;
   masked_count = src1->u[1] & 0x3f;
   dst->u64[1] = src0->u64[1] >> masked_count;
   masked_count = src1->u[2] & 0x3f;
   dst->u64[2] = src0->u64[2] >> masked_count;
   masked_count = src1->u[3] & 0x3f;
   dst->u64[3] = src0->u64[3] >> masked_count;
}

enum tgsi_exec_datatype {
   TGSI_EXEC_DATA_FLOAT,
   TGSI_EXEC_DATA_INT,
   TGSI_EXEC_DATA_UINT,
   TGSI_EXEC_DATA_DOUBLE,
   TGSI_EXEC_DATA_INT64,
   TGSI_EXEC_DATA_UINT64,
};

/** The execution mask depends on the conditional mask and the loop mask */
#define UPDATE_EXEC_MASK(MACH) \
      MACH->ExecMask = MACH->CondMask & MACH->LoopMask & MACH->ContMask & MACH->Switch.mask & MACH->FuncMask


static const union tgsi_exec_channel ZeroVec =
   { { 0.0, 0.0, 0.0, 0.0 } };

static const union tgsi_exec_channel OneVec = {
   {1.0f, 1.0f, 1.0f, 1.0f}
};

static const union tgsi_exec_channel P128Vec = {
   {128.0f, 128.0f, 128.0f, 128.0f}
};

static const union tgsi_exec_channel M128Vec = {
   {-128.0f, -128.0f, -128.0f, -128.0f}
};

#ifdef DEBUG
static void
print_chan(const char *msg, const union tgsi_exec_channel *chan)
{
   debug_printf("%s = {%f, %f, %f, %f}\n",
                msg, chan->f[0], chan->f[1], chan->f[2], chan->f[3]);
}
#endif


#ifdef DEBUG
static void
print_temp(const struct tgsi_exec_machine *mach, unsigned index)
{
   const struct tgsi_exec_vector *tmp = &mach->Temps[index];
   int i;
   debug_printf("Temp[%u] =\n", index);
   for (i = 0; i < 4; i++) {
      debug_printf("  %c: { %f, %f, %f, %f }\n",
                   "XYZW"[i],
                   tmp->xyzw[i].f[0],
                   tmp->xyzw[i].f[1],
                   tmp->xyzw[i].f[2],
                   tmp->xyzw[i].f[3]);
   }
}
#endif


void
tgsi_exec_set_constant_buffers(struct tgsi_exec_machine *mach,
                               unsigned num_bufs,
                               const struct tgsi_exec_consts_info *bufs)
{
   unsigned i;

   for (i = 0; i < num_bufs; i++) {
      mach->Consts[i] = bufs[i].ptr;
      mach->ConstsSize[i] = bufs[i].size;
   }
}

/**
 * Initialize machine state by expanding tokens to full instructions,
 * allocating temporary storage, setting up constants, etc.
 * After this, we can call tgsi_exec_machine_run() many times.
 */
void 
tgsi_exec_machine_bind_shader(
   struct tgsi_exec_machine *mach,
   const struct tgsi_token *tokens,
   struct tgsi_sampler *sampler,
   struct tgsi_image *image,
   struct tgsi_buffer *buffer)
{
   unsigned k;
   struct tgsi_parse_context parse;
   struct tgsi_full_instruction *instructions;
   struct tgsi_full_declaration *declarations;
   unsigned maxInstructions = 10, numInstructions = 0;
   unsigned maxDeclarations = 10, numDeclarations = 0;

#if 0
   tgsi_dump(tokens, 0);
#endif

   mach->Tokens = tokens;
   mach->Sampler = sampler;
   mach->Image = image;
   mach->Buffer = buffer;

   if (!tokens) {
      /* unbind and free all */
      FREE(mach->Declarations);
      mach->Declarations = NULL;
      mach->NumDeclarations = 0;

      FREE(mach->Instructions);
      mach->Instructions = NULL;
      mach->NumInstructions = 0;

      return;
   }

   k = tgsi_parse_init (&parse, mach->Tokens);
   if (k != TGSI_PARSE_OK) {
      debug_printf( "Problem parsing!\n" );
      return;
   }

   mach->ImmLimit = 0;
   mach->NumOutputs = 0;

   for (k = 0; k < TGSI_SEMANTIC_COUNT; k++)
      mach->SysSemanticToIndex[k] = -1;

   if (mach->ShaderType == PIPE_SHADER_GEOMETRY &&
       !mach->UsedGeometryShader) {
      struct tgsi_exec_vector *inputs;
      struct tgsi_exec_vector *outputs;

      inputs = align_malloc(sizeof(struct tgsi_exec_vector) *
                            TGSI_MAX_PRIM_VERTICES * PIPE_MAX_SHADER_INPUTS,
                            16);

      if (!inputs)
         return;

      outputs = align_malloc(sizeof(struct tgsi_exec_vector) *
                             TGSI_MAX_TOTAL_VERTICES, 16);

      if (!outputs) {
         align_free(inputs);
         return;
      }

      align_free(mach->Inputs);
      align_free(mach->Outputs);

      mach->Inputs = inputs;
      mach->Outputs = outputs;
      mach->UsedGeometryShader = true;
   }

   declarations = (struct tgsi_full_declaration *)
      MALLOC( maxDeclarations * sizeof(struct tgsi_full_declaration) );

   if (!declarations) {
      return;
   }

   instructions = (struct tgsi_full_instruction *)
      MALLOC( maxInstructions * sizeof(struct tgsi_full_instruction) );

   if (!instructions) {
      FREE( declarations );
      return;
   }

   while( !tgsi_parse_end_of_tokens( &parse ) ) {
      unsigned i;

      tgsi_parse_token( &parse );
      switch( parse.FullToken.Token.Type ) {
      case TGSI_TOKEN_TYPE_DECLARATION:
         /* save expanded declaration */
         if (numDeclarations == maxDeclarations) {
            declarations = REALLOC(declarations,
                                   maxDeclarations
                                   * sizeof(struct tgsi_full_declaration),
                                   (maxDeclarations + 10)
                                   * sizeof(struct tgsi_full_declaration));
            maxDeclarations += 10;
         }
         if (parse.FullToken.FullDeclaration.Declaration.File == TGSI_FILE_OUTPUT)
            mach->NumOutputs = MAX2(mach->NumOutputs, parse.FullToken.FullDeclaration.Range.Last + 1);
         else if (parse.FullToken.FullDeclaration.Declaration.File == TGSI_FILE_SYSTEM_VALUE) {
            const struct tgsi_full_declaration *decl = &parse.FullToken.FullDeclaration;
            mach->SysSemanticToIndex[decl->Semantic.Name] = decl->Range.First;
         }

         memcpy(declarations + numDeclarations,
                &parse.FullToken.FullDeclaration,
                sizeof(declarations[0]));
         numDeclarations++;
         break;

      case TGSI_TOKEN_TYPE_IMMEDIATE:
         {
            unsigned size = parse.FullToken.FullImmediate.Immediate.NrTokens - 1;
            assert( size <= 4 );
            if (mach->ImmLimit >= mach->ImmsReserved) {
               unsigned newReserved = mach->ImmsReserved ? 2 * mach->ImmsReserved : 128;
               float4 *imms = REALLOC(mach->Imms, mach->ImmsReserved, newReserved * sizeof(float4));
               if (imms) {
                  mach->ImmsReserved = newReserved;
                  mach->Imms = imms;
               } else {
                  debug_printf("Unable to (re)allocate space for immidiate constants\n");
                  break;
               }
            }

            for( i = 0; i < size; i++ ) {
               mach->Imms[mach->ImmLimit][i] = 
		  parse.FullToken.FullImmediate.u[i].Float;
            }
            mach->ImmLimit += 1;
         }
         break;

      case TGSI_TOKEN_TYPE_INSTRUCTION:

         /* save expanded instruction */
         if (numInstructions == maxInstructions) {
            instructions = REALLOC(instructions,
                                   maxInstructions
                                   * sizeof(struct tgsi_full_instruction),
                                   (maxInstructions + 10)
                                   * sizeof(struct tgsi_full_instruction));
            maxInstructions += 10;
         }

         memcpy(instructions + numInstructions,
                &parse.FullToken.FullInstruction,
                sizeof(instructions[0]));

         numInstructions++;
         break;

      case TGSI_TOKEN_TYPE_PROPERTY:
         if (mach->ShaderType == PIPE_SHADER_GEOMETRY) {
            if (parse.FullToken.FullProperty.Property.PropertyName == TGSI_PROPERTY_GS_MAX_OUTPUT_VERTICES) {
               mach->MaxOutputVertices = parse.FullToken.FullProperty.u[0].Data;
            }
         }
         break;

      default:
         assert( 0 );
      }
   }
   tgsi_parse_free (&parse);

   FREE(mach->Declarations);
   mach->Declarations = declarations;
   mach->NumDeclarations = numDeclarations;

   FREE(mach->Instructions);
   mach->Instructions = instructions;
   mach->NumInstructions = numInstructions;
}


struct tgsi_exec_machine *
tgsi_exec_machine_create(enum pipe_shader_type shader_type)
{
   struct tgsi_exec_machine *mach;

   mach = align_malloc( sizeof *mach, 16 );
   if (!mach)
      goto fail;

   memset(mach, 0, sizeof(*mach));

   mach->ShaderType = shader_type;

   if (shader_type != PIPE_SHADER_COMPUTE) {
      mach->Inputs = align_malloc(sizeof(struct tgsi_exec_vector) * PIPE_MAX_SHADER_INPUTS, 16);
      mach->Outputs = align_malloc(sizeof(struct tgsi_exec_vector) * PIPE_MAX_SHADER_OUTPUTS, 16);
      if (!mach->Inputs || !mach->Outputs)
         goto fail;
   }

   if (shader_type == PIPE_SHADER_FRAGMENT) {
      mach->InputSampleOffsetApply = align_malloc(sizeof(apply_sample_offset_func) * PIPE_MAX_SHADER_INPUTS, 16);
      if (!mach->InputSampleOffsetApply)
         goto fail;
   }

#ifdef DEBUG
   /* silence warnings */
   (void) print_chan;
   (void) print_temp;
#endif

   return mach;

fail:
   if (mach) {
      align_free(mach->InputSampleOffsetApply);
      align_free(mach->Inputs);
      align_free(mach->Outputs);
      align_free(mach);
   }
   return NULL;
}


void
tgsi_exec_machine_destroy(struct tgsi_exec_machine *mach)
{
   if (mach) {
      FREE(mach->Instructions);
      FREE(mach->Declarations);
      FREE(mach->Imms);

      align_free(mach->InputSampleOffsetApply);
      align_free(mach->Inputs);
      align_free(mach->Outputs);

      align_free(mach);
   }
}

static void
micro_add(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] + src1->f[0];
   dst->f[1] = src0->f[1] + src1->f[1];
   dst->f[2] = src0->f[2] + src1->f[2];
   dst->f[3] = src0->f[3] + src1->f[3];
}

static void
micro_div(
   union tgsi_exec_channel *dst,
   const union tgsi_exec_channel *src0,
   const union tgsi_exec_channel *src1 )
{
   dst->f[0] = src0->f[0] / src1->f[0];
   dst->f[1] = src0->f[1] / src1->f[1];
   dst->f[2] = src0->f[2] / src1->f[2];
   dst->f[3] = src0->f[3] / src1->f[3];
}

static void
micro_lt(
   union tgsi_exec_channel *dst,
   const union tgsi_exec_channel *src0,
   const union tgsi_exec_channel *src1,
   const union tgsi_exec_channel *src2,
   const union tgsi_exec_channel *src3 )
{
   dst->f[0] = src0->f[0] < src1->f[0] ? src2->f[0] : src3->f[0];
   dst->f[1] = src0->f[1] < src1->f[1] ? src2->f[1] : src3->f[1];
   dst->f[2] = src0->f[2] < src1->f[2] ? src2->f[2] : src3->f[2];
   dst->f[3] = src0->f[3] < src1->f[3] ? src2->f[3] : src3->f[3];
}

static void
micro_max(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = fmaxf(src0->f[0], src1->f[0]);
   dst->f[1] = fmaxf(src0->f[1], src1->f[1]);
   dst->f[2] = fmaxf(src0->f[2], src1->f[2]);
   dst->f[3] = fmaxf(src0->f[3], src1->f[3]);
}

static void
micro_min(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = fminf(src0->f[0], src1->f[0]);
   dst->f[1] = fminf(src0->f[1], src1->f[1]);
   dst->f[2] = fminf(src0->f[2], src1->f[2]);
   dst->f[3] = fminf(src0->f[3], src1->f[3]);
}

static void
micro_mul(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] * src1->f[0];
   dst->f[1] = src0->f[1] * src1->f[1];
   dst->f[2] = src0->f[2] * src1->f[2];
   dst->f[3] = src0->f[3] * src1->f[3];
}

static void
micro_neg(
   union tgsi_exec_channel *dst,
   const union tgsi_exec_channel *src )
{
   dst->f[0] = -src->f[0];
   dst->f[1] = -src->f[1];
   dst->f[2] = -src->f[2];
   dst->f[3] = -src->f[3];
}

static void
micro_pow(
   union tgsi_exec_channel *dst,
   const union tgsi_exec_channel *src0,
   const union tgsi_exec_channel *src1 )
{
   dst->f[0] = powf( src0->f[0], src1->f[0] );
   dst->f[1] = powf( src0->f[1], src1->f[1] );
   dst->f[2] = powf( src0->f[2], src1->f[2] );
   dst->f[3] = powf( src0->f[3], src1->f[3] );
}

static void
micro_ldexp(union tgsi_exec_channel *dst,
            const union tgsi_exec_channel *src0,
            const union tgsi_exec_channel *src1)
{
   dst->f[0] = ldexpf(src0->f[0], src1->i[0]);
   dst->f[1] = ldexpf(src0->f[1], src1->i[1]);
   dst->f[2] = ldexpf(src0->f[2], src1->i[2]);
   dst->f[3] = ldexpf(src0->f[3], src1->i[3]);
}

static void
micro_sub(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->f[0] = src0->f[0] - src1->f[0];
   dst->f[1] = src0->f[1] - src1->f[1];
   dst->f[2] = src0->f[2] - src1->f[2];
   dst->f[3] = src0->f[3] - src1->f[3];
}

static void
fetch_src_file_channel(const struct tgsi_exec_machine *mach,
                       const unsigned file,
                       const unsigned swizzle,
                       const union tgsi_exec_channel *index,
                       const union tgsi_exec_channel *index2D,
                       union tgsi_exec_channel *chan)
{
   unsigned i;

   assert(swizzle < 4);

   switch (file) {
   case TGSI_FILE_CONSTANT:
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         /* NOTE: copying the const value as a unsigned instead of float */
         const unsigned constbuf = index2D->i[i];
         const unsigned pos = index->i[i] * 4 + swizzle;
         /* const buffer bounds check */
         if (pos >= mach->ConstsSize[constbuf] / 4) {
            if (0) {
               /* Debug: print warning */
               static int count = 0;
               if (count++ < 100)
                  debug_printf("TGSI Exec: const buffer index %d"
                                 " out of bounds\n", pos);
            }
            chan->u[i] = 0;
         } else {
            const unsigned *buf = (const unsigned *)mach->Consts[constbuf];
            chan->u[i] = buf[pos];
         }
      }
      break;

   case TGSI_FILE_INPUT:
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         /*
         if (PIPE_SHADER_GEOMETRY == mach->ShaderType) {
            debug_printf("Fetching Input[%d] (2d=%d, 1d=%d)\n",
                         index2D->i[i] * TGSI_EXEC_MAX_INPUT_ATTRIBS + index->i[i],
                         index2D->i[i], index->i[i]);
                         }*/
         int pos = index2D->i[i] * TGSI_EXEC_MAX_INPUT_ATTRIBS + index->i[i];
         assert(pos >= 0);
         assert(pos < TGSI_MAX_PRIM_VERTICES * PIPE_MAX_ATTRIBS);
         chan->u[i] = mach->Inputs[pos].xyzw[swizzle].u[i];
      }
      break;

   case TGSI_FILE_SYSTEM_VALUE:
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         chan->u[i] = mach->SystemValue[index->i[i]].xyzw[swizzle].u[i];
      }
      break;

   case TGSI_FILE_TEMPORARY:
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         assert(index->i[i] < TGSI_EXEC_NUM_TEMPS);
         assert(index2D->i[i] == 0);

         chan->u[i] = mach->Temps[index->i[i]].xyzw[swizzle].u[i];
      }
      break;

   case TGSI_FILE_IMMEDIATE:
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         assert(index->i[i] >= 0 && index->i[i] < (int)mach->ImmLimit);
         assert(index2D->i[i] == 0);

         chan->f[i] = mach->Imms[index->i[i]][swizzle];
      }
      break;

   case TGSI_FILE_ADDRESS:
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         assert(index->i[i] >= 0 && index->i[i] < ARRAY_SIZE(mach->Addrs));
         assert(index2D->i[i] == 0);

         chan->u[i] = mach->Addrs[index->i[i]].xyzw[swizzle].u[i];
      }
      break;

   case TGSI_FILE_OUTPUT:
      /* vertex/fragment output vars can be read too */
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         assert(index->i[i] >= 0);
         assert(index2D->i[i] == 0);

         chan->u[i] = mach->Outputs[index->i[i]].xyzw[swizzle].u[i];
      }
      break;

   default:
      assert(0);
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         chan->u[i] = 0;
      }
   }
}

static void
get_index_registers(const struct tgsi_exec_machine *mach,
                    const struct tgsi_full_src_register *reg,
                    union tgsi_exec_channel *index,
                    union tgsi_exec_channel *index2D)
{
   /* We start with a direct index into a register file.
    *
    *    file[1],
    *    where:
    *       file = Register.File
    *       [1] = Register.Index
    */
   index->i[0] =
   index->i[1] =
   index->i[2] =
   index->i[3] = reg->Register.Index;

   /* There is an extra source register that indirectly subscripts
    * a register file. The direct index now becomes an offset
    * that is being added to the indirect register.
    *
    *    file[ind[2].x+1],
    *    where:
    *       ind = Indirect.File
    *       [2] = Indirect.Index
    *       .x = Indirect.SwizzleX
    */
   if (reg->Register.Indirect) {
      const unsigned execmask = mach->ExecMask;

      assert(reg->Indirect.File == TGSI_FILE_ADDRESS);
      const union tgsi_exec_channel *addr = &mach->Addrs[reg->Indirect.Index].xyzw[reg->Indirect.Swizzle];
      for (int i = 0; i < TGSI_QUAD_SIZE; i++)
         index->i[i] += addr->u[i];

      /* for disabled execution channels, zero-out the index to
       * avoid using a potential garbage value.
       */
      for (int i = 0; i < TGSI_QUAD_SIZE; i++) {
         if ((execmask & (1 << i)) == 0)
            index->i[i] = 0;
      }
   }

   /* There is an extra source register that is a second
    * subscript to a register file. Effectively it means that
    * the register file is actually a 2D array of registers.
    *
    *    file[3][1],
    *    where:
    *       [3] = Dimension.Index
    */
   if (reg->Register.Dimension) {
      index2D->i[0] =
      index2D->i[1] =
      index2D->i[2] =
      index2D->i[3] = reg->Dimension.Index;

      /* Again, the second subscript index can be addressed indirectly
       * identically to the first one.
       * Nothing stops us from indirectly addressing the indirect register,
       * but there is no need for that, so we won't exercise it.
       *
       *    file[ind[4].y+3][1],
       *    where:
       *       ind = DimIndirect.File
       *       [4] = DimIndirect.Index
       *       .y = DimIndirect.SwizzleX
       */
      if (reg->Dimension.Indirect) {
         const unsigned execmask = mach->ExecMask;

         assert(reg->DimIndirect.File == TGSI_FILE_ADDRESS);
         const union tgsi_exec_channel *addr = &mach->Addrs[reg->DimIndirect.Index].xyzw[reg->DimIndirect.Swizzle];
         for (int i = 0; i < TGSI_QUAD_SIZE; i++)
            index2D->i[i] += addr->u[i];

         /* for disabled execution channels, zero-out the index to
          * avoid using a potential garbage value.
          */
         for (int i = 0; i < TGSI_QUAD_SIZE; i++) {
            if ((execmask & (1 << i)) == 0) {
               index2D->i[i] = 0;
            }
         }
      }

      /* If by any chance there was a need for a 3D array of register
       * files, we would have to check whether Dimension is followed
       * by a dimension register and continue the saga.
       */
   } else {
      index2D->i[0] =
      index2D->i[1] =
      index2D->i[2] =
      index2D->i[3] = 0;
   }
}


static void
fetch_source_d(const struct tgsi_exec_machine *mach,
               union tgsi_exec_channel *chan,
               const struct tgsi_full_src_register *reg,
	       const unsigned chan_index)
{
   union tgsi_exec_channel index;
   union tgsi_exec_channel index2D;
   unsigned swizzle;

   get_index_registers(mach, reg, &index, &index2D);


   swizzle = tgsi_util_get_full_src_register_swizzle( reg, chan_index );
   fetch_src_file_channel(mach,
                          reg->Register.File,
                          swizzle,
                          &index,
                          &index2D,
                          chan);
}

static void
fetch_source(const struct tgsi_exec_machine *mach,
             union tgsi_exec_channel *chan,
             const struct tgsi_full_src_register *reg,
             const unsigned chan_index,
             enum tgsi_exec_datatype src_datatype)
{
   fetch_source_d(mach, chan, reg, chan_index);

   if (reg->Register.Absolute) {
      assert(src_datatype == TGSI_EXEC_DATA_FLOAT);
      micro_abs(chan, chan);
   }

   if (reg->Register.Negate) {
      if (src_datatype == TGSI_EXEC_DATA_FLOAT) {
         micro_neg(chan, chan);
      } else {
         micro_ineg(chan, chan);
      }
   }
}

static union tgsi_exec_channel *
store_dest_dstret(struct tgsi_exec_machine *mach,
                 const union tgsi_exec_channel *chan,
                 const struct tgsi_full_dst_register *reg,
                 unsigned chan_index)
{
   static union tgsi_exec_channel null;
   union tgsi_exec_channel *dst;
   int offset = 0;  /* indirection offset */
   int index;


   /* There is an extra source register that indirectly subscripts
    * a register file. The direct index now becomes an offset
    * that is being added to the indirect register.
    *
    *    file[ind[2].x+1],
    *    where:
    *       ind = Indirect.File
    *       [2] = Indirect.Index
    *       .x = Indirect.SwizzleX
    */
   if (reg->Register.Indirect) {
      union tgsi_exec_channel index;
      union tgsi_exec_channel indir_index;
      unsigned swizzle;

      /* which address register (always zero for now) */
      index.i[0] =
      index.i[1] =
      index.i[2] =
      index.i[3] = reg->Indirect.Index;

      /* get current value of address register[swizzle] */
      swizzle = reg->Indirect.Swizzle;

      /* fetch values from the address/indirection register */
      fetch_src_file_channel(mach,
                             reg->Indirect.File,
                             swizzle,
                             &index,
                             &ZeroVec,
                             &indir_index);

      /* save indirection offset */
      offset = indir_index.i[0];
   }

   switch (reg->Register.File) {
   case TGSI_FILE_NULL:
      dst = &null;
      break;

   case TGSI_FILE_OUTPUT:
      index = mach->OutputVertexOffset + reg->Register.Index;
      dst = &mach->Outputs[offset + index].xyzw[chan_index];
#if 0
      debug_printf("NumOutputs = %d, TEMP_O_C/I = %d, redindex = %d\n",
                   mach->NumOutputs, mach->Temps[TEMP_OUTPUT_I].xyzw[TEMP_OUTPUT_C].u[0],
                   reg->Register.Index);
      if (PIPE_SHADER_GEOMETRY == mach->ShaderType) {
         debug_printf("STORING OUT[%d] mask(%d), = (", offset + index, execmask);
         for (i = 0; i < TGSI_QUAD_SIZE; i++)
            if (execmask & (1 << i))
               debug_printf("%f, ", chan->f[i]);
         debug_printf(")\n");
      }
#endif
      break;

   case TGSI_FILE_TEMPORARY:
      index = reg->Register.Index;
      assert( index < TGSI_EXEC_NUM_TEMPS );
      dst = &mach->Temps[offset + index].xyzw[chan_index];
      break;

   case TGSI_FILE_ADDRESS:
      index = reg->Register.Index;
      assert(index >= 0 && index < ARRAY_SIZE(mach->Addrs));
      dst = &mach->Addrs[index].xyzw[chan_index];
      break;

   default:
      unreachable("Bad destination file");
   }

   return dst;
}

static void
store_dest_double(struct tgsi_exec_machine *mach,
                 const union tgsi_exec_channel *chan,
                 const struct tgsi_full_dst_register *reg,
                 unsigned chan_index)
{
   union tgsi_exec_channel *dst;
   const unsigned execmask = mach->ExecMask;
   int i;

   dst = store_dest_dstret(mach, chan, reg, chan_index);
   if (!dst)
      return;

   /* doubles path */
   for (i = 0; i < TGSI_QUAD_SIZE; i++)
      if (execmask & (1 << i))
         dst->i[i] = chan->i[i];
}

static void
store_dest(struct tgsi_exec_machine *mach,
           const union tgsi_exec_channel *chan,
           const struct tgsi_full_dst_register *reg,
           const struct tgsi_full_instruction *inst,
           unsigned chan_index)
{
   union tgsi_exec_channel *dst;
   const unsigned execmask = mach->ExecMask;
   int i;

   dst = store_dest_dstret(mach, chan, reg, chan_index);
   if (!dst)
      return;

   if (!inst->Instruction.Saturate) {
      for (i = 0; i < TGSI_QUAD_SIZE; i++)
         if (execmask & (1 << i))
            dst->i[i] = chan->i[i];
   }
   else {
      for (i = 0; i < TGSI_QUAD_SIZE; i++)
         if (execmask & (1 << i))
            dst->f[i] = fminf(fmaxf(chan->f[i], 0.0f), 1.0f);
   }
}

#define FETCH(VAL,INDEX,CHAN)\
    fetch_source(mach, VAL, &inst->Src[INDEX], CHAN, TGSI_EXEC_DATA_FLOAT)

#define IFETCH(VAL,INDEX,CHAN)\
    fetch_source(mach, VAL, &inst->Src[INDEX], CHAN, TGSI_EXEC_DATA_INT)


/**
 * Execute ARB-style KIL which is predicated by a src register.
 * Kill fragment if any of the four values is less than zero.
 */
static void
exec_kill_if(struct tgsi_exec_machine *mach,
             const struct tgsi_full_instruction *inst)
{
   unsigned uniquemask;
   unsigned chan_index;
   unsigned kilmask = 0; /* bit 0 = pixel 0, bit 1 = pixel 1, etc */
   union tgsi_exec_channel r[1];

   /* This mask stores component bits that were already tested. */
   uniquemask = 0;

   for (chan_index = 0; chan_index < 4; chan_index++)
   {
      unsigned swizzle;
      unsigned i;

      /* unswizzle channel */
      swizzle = tgsi_util_get_full_src_register_swizzle (
                        &inst->Src[0],
                        chan_index);

      /* check if the component has not been already tested */
      if (uniquemask & (1 << swizzle))
         continue;
      uniquemask |= 1 << swizzle;

      FETCH(&r[0], 0, chan_index);
      for (i = 0; i < 4; i++)
         if (r[0].f[i] < 0.0f)
            kilmask |= 1 << i;
   }

   /* restrict to fragments currently executing */
   kilmask &= mach->ExecMask;

   mach->KillMask |= kilmask;
}

/**
 * Unconditional fragment kill/discard.
 */
static void
exec_kill(struct tgsi_exec_machine *mach)
{
   /* kill fragment for all fragments currently executing.
    * bit 0 = pixel 0, bit 1 = pixel 1, etc.
    */
   mach->KillMask |= mach->ExecMask;
}

static void
emit_vertex(struct tgsi_exec_machine *mach,
            const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[1];
   unsigned stream_id;
   unsigned prim_count;
   /* FIXME: check for exec mask correctly
   unsigned i;
   for (i = 0; i < TGSI_QUAD_SIZE; ++i) {
         if ((mach->ExecMask & (1 << i)))
   */
   IFETCH(&r[0], 0, TGSI_CHAN_X);
   stream_id = r[0].u[0];
   prim_count = mach->OutputPrimCount[stream_id];
   if (mach->ExecMask) {
      if (mach->Primitives[stream_id][prim_count] >= mach->MaxOutputVertices)
         return;

      if (mach->Primitives[stream_id][prim_count] == 0)
         mach->PrimitiveOffsets[stream_id][prim_count] = mach->OutputVertexOffset;
      mach->OutputVertexOffset += mach->NumOutputs;
      mach->Primitives[stream_id][prim_count]++;
   }
}

static void
emit_primitive(struct tgsi_exec_machine *mach,
               const struct tgsi_full_instruction *inst)
{
   unsigned *prim_count;
   union tgsi_exec_channel r[1];
   unsigned stream_id = 0;
   /* FIXME: check for exec mask correctly
   unsigned i;
   for (i = 0; i < TGSI_QUAD_SIZE; ++i) {
         if ((mach->ExecMask & (1 << i)))
   */
   if (inst) {
      IFETCH(&r[0], 0, TGSI_CHAN_X);
      stream_id = r[0].u[0];
   }
   prim_count = &mach->OutputPrimCount[stream_id];
   if (mach->ExecMask) {
      ++(*prim_count);
      assert((*prim_count * mach->NumOutputs) < TGSI_MAX_TOTAL_VERTICES);
      mach->Primitives[stream_id][*prim_count] = 0;
   }
}

static void
conditional_emit_primitive(struct tgsi_exec_machine *mach)
{
   if (PIPE_SHADER_GEOMETRY == mach->ShaderType) {
      int emitted_verts = mach->Primitives[0][mach->OutputPrimCount[0]];
      if (emitted_verts) {
         emit_primitive(mach, NULL);
      }
   }
}


/*
 * Fetch four texture samples using STR texture coordinates.
 */
static void
fetch_texel( struct tgsi_sampler *sampler,
             const unsigned sview_idx,
             const unsigned sampler_idx,
             const union tgsi_exec_channel *s,
             const union tgsi_exec_channel *t,
             const union tgsi_exec_channel *p,
             const union tgsi_exec_channel *c0,
             const union tgsi_exec_channel *c1,
             float derivs[3][2][TGSI_QUAD_SIZE],
             const int8_t offset[3],
             enum tgsi_sampler_control control,
             union tgsi_exec_channel *r,
             union tgsi_exec_channel *g,
             union tgsi_exec_channel *b,
             union tgsi_exec_channel *a )
{
   unsigned j;
   float rgba[TGSI_NUM_CHANNELS][TGSI_QUAD_SIZE];

   /* FIXME: handle explicit derivs, offsets */
   sampler->get_samples(sampler, sview_idx, sampler_idx,
                        s->f, t->f, p->f, c0->f, c1->f, derivs, offset, control, rgba);

   for (j = 0; j < 4; j++) {
      r->f[j] = rgba[0][j];
      g->f[j] = rgba[1][j];
      b->f[j] = rgba[2][j];
      a->f[j] = rgba[3][j];
   }
}


enum tex_modifier {
   TEX_MODIFIER_NONE         = 0,
   TEX_MODIFIER_PROJECTED    = 1,
   TEX_MODIFIER_LOD_BIAS     = 2,
   TEX_MODIFIER_EXPLICIT_LOD = 3,
   TEX_MODIFIER_LEVEL_ZERO   = 4,
   TEX_MODIFIER_GATHER       = 5,
};

/*
 * Fetch all 3 (for s,t,r coords) texel offsets, put them into int array.
 */
static void
fetch_texel_offsets(struct tgsi_exec_machine *mach,
                    const struct tgsi_full_instruction *inst,
                    int8_t offsets[3])
{
   if (inst->Texture.NumOffsets == 1) {
      union tgsi_exec_channel index;
      union tgsi_exec_channel offset[3];
      index.i[0] = index.i[1] = index.i[2] = index.i[3] = inst->TexOffsets[0].Index;
      fetch_src_file_channel(mach, inst->TexOffsets[0].File,
                             inst->TexOffsets[0].SwizzleX, &index, &ZeroVec, &offset[0]);
      fetch_src_file_channel(mach, inst->TexOffsets[0].File,
                             inst->TexOffsets[0].SwizzleY, &index, &ZeroVec, &offset[1]);
      fetch_src_file_channel(mach, inst->TexOffsets[0].File,
                             inst->TexOffsets[0].SwizzleZ, &index, &ZeroVec, &offset[2]);
     offsets[0] = offset[0].i[0];
     offsets[1] = offset[1].i[0];
     offsets[2] = offset[2].i[0];
   } else {
     assert(inst->Texture.NumOffsets == 0);
     offsets[0] = offsets[1] = offsets[2] = 0;
   }
}


/*
 * Fetch dx and dy values for one channel (s, t or r).
 * Put dx values into one float array, dy values into another.
 */
static void
fetch_assign_deriv_channel(struct tgsi_exec_machine *mach,
                           const struct tgsi_full_instruction *inst,
                           unsigned regdsrcx,
                           unsigned chan,
                           float derivs[2][TGSI_QUAD_SIZE])
{
   union tgsi_exec_channel d;
   FETCH(&d, regdsrcx, chan);
   derivs[0][0] = d.f[0];
   derivs[0][1] = d.f[1];
   derivs[0][2] = d.f[2];
   derivs[0][3] = d.f[3];
   FETCH(&d, regdsrcx + 1, chan);
   derivs[1][0] = d.f[0];
   derivs[1][1] = d.f[1];
   derivs[1][2] = d.f[2];
   derivs[1][3] = d.f[3];
}

static unsigned
fetch_sampler_unit(struct tgsi_exec_machine *mach,
                   const struct tgsi_full_instruction *inst,
                   unsigned sampler)
{
   unsigned unit = 0;
   int i;
   if (inst->Src[sampler].Register.Indirect) {
      const struct tgsi_full_src_register *reg = &inst->Src[sampler];
      union tgsi_exec_channel indir_index, index2;
      const unsigned execmask = mach->ExecMask;
      index2.i[0] =
      index2.i[1] =
      index2.i[2] =
      index2.i[3] = reg->Indirect.Index;

      fetch_src_file_channel(mach,
                             reg->Indirect.File,
                             reg->Indirect.Swizzle,
                             &index2,
                             &ZeroVec,
                             &indir_index);
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         if (execmask & (1 << i)) {
            unit = inst->Src[sampler].Register.Index + indir_index.i[i];
            break;
         }
      }

   } else {
      unit = inst->Src[sampler].Register.Index;
   }
   return unit;
}

/*
 * execute a texture instruction.
 *
 * modifier is used to control the channel routing for the
 * instruction variants like proj, lod, and texture with lod bias.
 * sampler indicates which src register the sampler is contained in.
 */
static void
exec_tex(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst,
         enum tex_modifier modifier, unsigned sampler)
{
   const union tgsi_exec_channel *args[5], *proj = NULL;
   union tgsi_exec_channel r[5];
   enum tgsi_sampler_control control = TGSI_SAMPLER_LOD_NONE;
   unsigned chan;
   unsigned unit;
   int8_t offsets[3];
   int dim, shadow_ref, i;

   unit = fetch_sampler_unit(mach, inst, sampler);
   /* always fetch all 3 offsets, overkill but keeps code simple */
   fetch_texel_offsets(mach, inst, offsets);

   assert(modifier != TEX_MODIFIER_LEVEL_ZERO);
   assert(inst->Texture.Texture != TGSI_TEXTURE_BUFFER);

   dim = tgsi_util_get_texture_coord_dim(inst->Texture.Texture);
   shadow_ref = tgsi_util_get_shadow_ref_src_index(inst->Texture.Texture);

   assert(dim <= 4);
   if (shadow_ref >= 0)
      assert(shadow_ref >= dim && shadow_ref < (int)ARRAY_SIZE(args));

   /* fetch modifier to the last argument */
   if (modifier != TEX_MODIFIER_NONE) {
      const int last = ARRAY_SIZE(args) - 1;

      /* fetch modifier from src0.w or src1.x */
      if (sampler == 1) {
         assert(dim <= TGSI_CHAN_W && shadow_ref != TGSI_CHAN_W);
         FETCH(&r[last], 0, TGSI_CHAN_W);
      }
      else {
         FETCH(&r[last], 1, TGSI_CHAN_X);
      }

      if (modifier != TEX_MODIFIER_PROJECTED) {
         args[last] = &r[last];
      }
      else {
         proj = &r[last];
         args[last] = &ZeroVec;
      }

      /* point unused arguments to zero vector */
      for (i = dim; i < last; i++)
         args[i] = &ZeroVec;

      if (modifier == TEX_MODIFIER_EXPLICIT_LOD)
         control = TGSI_SAMPLER_LOD_EXPLICIT;
      else if (modifier == TEX_MODIFIER_LOD_BIAS)
         control = TGSI_SAMPLER_LOD_BIAS;
      else if (modifier == TEX_MODIFIER_GATHER)
         control = TGSI_SAMPLER_GATHER;
   }
   else {
      for (i = dim; i < (int)ARRAY_SIZE(args); i++)
         args[i] = &ZeroVec;
   }

   /* fetch coordinates */
   for (i = 0; i < dim; i++) {
      FETCH(&r[i], 0, TGSI_CHAN_X + i);

      if (proj)
         micro_div(&r[i], &r[i], proj);

      args[i] = &r[i];
   }

   /* fetch reference value */
   if (shadow_ref >= 0) {
      FETCH(&r[shadow_ref], shadow_ref / 4, TGSI_CHAN_X + (shadow_ref % 4));

      if (proj)
         micro_div(&r[shadow_ref], &r[shadow_ref], proj);

      args[shadow_ref] = &r[shadow_ref];
   }

   fetch_texel(mach->Sampler, unit, unit,
         args[0], args[1], args[2], args[3], args[4],
         NULL, offsets, control,
         &r[0], &r[1], &r[2], &r[3]);     /* R, G, B, A */

#if 0
   debug_printf("fetch r: %g %g %g %g\n",
         r[0].f[0], r[0].f[1], r[0].f[2], r[0].f[3]);
   debug_printf("fetch g: %g %g %g %g\n",
         r[1].f[0], r[1].f[1], r[1].f[2], r[1].f[3]);
   debug_printf("fetch b: %g %g %g %g\n",
         r[2].f[0], r[2].f[1], r[2].f[2], r[2].f[3]);
   debug_printf("fetch a: %g %g %g %g\n",
         r[3].f[0], r[3].f[1], r[3].f[2], r[3].f[3]);
#endif

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &r[chan], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_lodq(struct tgsi_exec_machine *mach,
          const struct tgsi_full_instruction *inst)
{
   unsigned resource_unit, sampler_unit;
   unsigned dim;
   unsigned i;
   union tgsi_exec_channel coords[4];
   const union tgsi_exec_channel *args[ARRAY_SIZE(coords)];
   union tgsi_exec_channel r[2];

   resource_unit = fetch_sampler_unit(mach, inst, 1);
   if (inst->Instruction.Opcode == TGSI_OPCODE_LOD) {
      unsigned target = mach->SamplerViews[resource_unit].Resource;
      dim = tgsi_util_get_texture_coord_dim(target);
      sampler_unit = fetch_sampler_unit(mach, inst, 2);
   } else {
      dim = tgsi_util_get_texture_coord_dim(inst->Texture.Texture);
      sampler_unit = resource_unit;
   }
   assert(dim <= ARRAY_SIZE(coords));
   /* fetch coordinates */
   for (i = 0; i < dim; i++) {
      FETCH(&coords[i], 0, TGSI_CHAN_X + i);
      args[i] = &coords[i];
   }
   for (i = dim; i < ARRAY_SIZE(coords); i++) {
      args[i] = &ZeroVec;
   }
   mach->Sampler->query_lod(mach->Sampler, resource_unit, sampler_unit,
                            args[0]->f,
                            args[1]->f,
                            args[2]->f,
                            args[3]->f,
                            TGSI_SAMPLER_LOD_NONE,
                            r[0].f,
                            r[1].f);

   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      store_dest(mach, &r[0], &inst->Dst[0], inst, TGSI_CHAN_X);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      store_dest(mach, &r[1], &inst->Dst[0], inst, TGSI_CHAN_Y);
   }
   if (inst->Instruction.Opcode == TGSI_OPCODE_LOD) {
      unsigned char swizzles[4];
      unsigned chan;
      swizzles[0] = inst->Src[1].Register.SwizzleX;
      swizzles[1] = inst->Src[1].Register.SwizzleY;
      swizzles[2] = inst->Src[1].Register.SwizzleZ;
      swizzles[3] = inst->Src[1].Register.SwizzleW;

      for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
         if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
            if (swizzles[chan] >= 2) {
               store_dest(mach, &ZeroVec,
                          &inst->Dst[0], inst, chan);
            } else {
               store_dest(mach, &r[swizzles[chan]],
                          &inst->Dst[0], inst, chan);
            }
         }
      }
   } else {
      if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
         store_dest(mach, &r[0], &inst->Dst[0], inst, TGSI_CHAN_X);
      }
      if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
         store_dest(mach, &r[1], &inst->Dst[0], inst, TGSI_CHAN_Y);
      }
   }
}

static void
exec_txd(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[4];
   float derivs[3][2][TGSI_QUAD_SIZE];
   unsigned chan;
   unsigned unit;
   int8_t offsets[3];

   unit = fetch_sampler_unit(mach, inst, 3);
   /* always fetch all 3 offsets, overkill but keeps code simple */
   fetch_texel_offsets(mach, inst, offsets);

   switch (inst->Texture.Texture) {
   case TGSI_TEXTURE_1D:
      FETCH(&r[0], 0, TGSI_CHAN_X);

      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_X, derivs[0]);

      fetch_texel(mach->Sampler, unit, unit,
                  &r[0], &ZeroVec, &ZeroVec, &ZeroVec, &ZeroVec,   /* S, T, P, C, LOD */
                  derivs, offsets, TGSI_SAMPLER_DERIVS_EXPLICIT,
                  &r[0], &r[1], &r[2], &r[3]);           /* R, G, B, A */
      break;

   case TGSI_TEXTURE_SHADOW1D:
   case TGSI_TEXTURE_1D_ARRAY:
   case TGSI_TEXTURE_SHADOW1D_ARRAY:
      /* SHADOW1D/1D_ARRAY would not need Y/Z respectively, but don't bother */
      FETCH(&r[0], 0, TGSI_CHAN_X);
      FETCH(&r[1], 0, TGSI_CHAN_Y);
      FETCH(&r[2], 0, TGSI_CHAN_Z);

      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_X, derivs[0]);

      fetch_texel(mach->Sampler, unit, unit,
                  &r[0], &r[1], &r[2], &ZeroVec, &ZeroVec,   /* S, T, P, C, LOD */
                  derivs, offsets, TGSI_SAMPLER_DERIVS_EXPLICIT,
                  &r[0], &r[1], &r[2], &r[3]);           /* R, G, B, A */
      break;

   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
      FETCH(&r[0], 0, TGSI_CHAN_X);
      FETCH(&r[1], 0, TGSI_CHAN_Y);

      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_X, derivs[0]);
      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_Y, derivs[1]);

      fetch_texel(mach->Sampler, unit, unit,
                  &r[0], &r[1], &ZeroVec, &ZeroVec, &ZeroVec,   /* S, T, P, C, LOD */
                  derivs, offsets, TGSI_SAMPLER_DERIVS_EXPLICIT,
                  &r[0], &r[1], &r[2], &r[3]);           /* R, G, B, A */
      break;


   case TGSI_TEXTURE_SHADOW2D:
   case TGSI_TEXTURE_SHADOWRECT:
   case TGSI_TEXTURE_2D_ARRAY:
   case TGSI_TEXTURE_SHADOW2D_ARRAY:
      /* only SHADOW2D_ARRAY actually needs W */
      FETCH(&r[0], 0, TGSI_CHAN_X);
      FETCH(&r[1], 0, TGSI_CHAN_Y);
      FETCH(&r[2], 0, TGSI_CHAN_Z);
      FETCH(&r[3], 0, TGSI_CHAN_W);

      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_X, derivs[0]);
      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_Y, derivs[1]);

      fetch_texel(mach->Sampler, unit, unit,
                  &r[0], &r[1], &r[2], &r[3], &ZeroVec,   /* inputs */
                  derivs, offsets, TGSI_SAMPLER_DERIVS_EXPLICIT,
                  &r[0], &r[1], &r[2], &r[3]);     /* outputs */
      break;

   case TGSI_TEXTURE_3D:
   case TGSI_TEXTURE_CUBE:
   case TGSI_TEXTURE_CUBE_ARRAY:
   case TGSI_TEXTURE_SHADOWCUBE:
      /* only TEXTURE_CUBE_ARRAY and TEXTURE_SHADOWCUBE actually need W */
      FETCH(&r[0], 0, TGSI_CHAN_X);
      FETCH(&r[1], 0, TGSI_CHAN_Y);
      FETCH(&r[2], 0, TGSI_CHAN_Z);
      FETCH(&r[3], 0, TGSI_CHAN_W);

      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_X, derivs[0]);
      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_Y, derivs[1]);
      fetch_assign_deriv_channel(mach, inst, 1, TGSI_CHAN_Z, derivs[2]);

      fetch_texel(mach->Sampler, unit, unit,
                  &r[0], &r[1], &r[2], &r[3], &ZeroVec,   /* inputs */
                  derivs, offsets, TGSI_SAMPLER_DERIVS_EXPLICIT,
                  &r[0], &r[1], &r[2], &r[3]);     /* outputs */
      break;

   default:
      assert(0);
   }

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &r[chan], &inst->Dst[0], inst, chan);
      }
   }
}


static void
exec_txf(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[4];
   unsigned chan;
   unsigned unit;
   float rgba[TGSI_NUM_CHANNELS][TGSI_QUAD_SIZE];
   int j;
   int8_t offsets[3];
   unsigned target;

   unit = fetch_sampler_unit(mach, inst, 1);
   /* always fetch all 3 offsets, overkill but keeps code simple */
   fetch_texel_offsets(mach, inst, offsets);

   IFETCH(&r[3], 0, TGSI_CHAN_W);

   if (inst->Instruction.Opcode == TGSI_OPCODE_SAMPLE_I ||
       inst->Instruction.Opcode == TGSI_OPCODE_SAMPLE_I_MS) {
      target = mach->SamplerViews[unit].Resource;
   }
   else {
      target = inst->Texture.Texture;
   }
   switch(target) {
   case TGSI_TEXTURE_3D:
   case TGSI_TEXTURE_2D_ARRAY:
   case TGSI_TEXTURE_SHADOW2D_ARRAY:
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      IFETCH(&r[2], 0, TGSI_CHAN_Z);
      FALLTHROUGH;
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
   case TGSI_TEXTURE_SHADOW1D_ARRAY:
   case TGSI_TEXTURE_SHADOW2D:
   case TGSI_TEXTURE_SHADOWRECT:
   case TGSI_TEXTURE_1D_ARRAY:
   case TGSI_TEXTURE_2D_MSAA:
      IFETCH(&r[1], 0, TGSI_CHAN_Y);
      FALLTHROUGH;
   case TGSI_TEXTURE_BUFFER:
   case TGSI_TEXTURE_1D:
   case TGSI_TEXTURE_SHADOW1D:
      IFETCH(&r[0], 0, TGSI_CHAN_X);
      break;
   default:
      assert(0);
      break;
   }      

   mach->Sampler->get_texel(mach->Sampler, unit, r[0].i, r[1].i, r[2].i, r[3].i,
                            offsets, rgba);

   for (j = 0; j < TGSI_QUAD_SIZE; j++) {
      r[0].f[j] = rgba[0][j];
      r[1].f[j] = rgba[1][j];
      r[2].f[j] = rgba[2][j];
      r[3].f[j] = rgba[3][j];
   }

   if (inst->Instruction.Opcode == TGSI_OPCODE_SAMPLE_I ||
       inst->Instruction.Opcode == TGSI_OPCODE_SAMPLE_I_MS) {
      unsigned char swizzles[4];
      swizzles[0] = inst->Src[1].Register.SwizzleX;
      swizzles[1] = inst->Src[1].Register.SwizzleY;
      swizzles[2] = inst->Src[1].Register.SwizzleZ;
      swizzles[3] = inst->Src[1].Register.SwizzleW;

      for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
         if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
            store_dest(mach, &r[swizzles[chan]],
                       &inst->Dst[0], inst, chan);
         }
      }
   }
   else {
      for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
         if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
            store_dest(mach, &r[chan], &inst->Dst[0], inst, chan);
         }
      }
   }
}

static void
exec_txq(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   int result[4];
   union tgsi_exec_channel r[4], src;
   unsigned chan;
   unsigned unit;
   int i,j;

   unit = fetch_sampler_unit(mach, inst, 1);

   fetch_source(mach, &src, &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_INT);

   /* XXX: This interface can't return per-pixel values */
   mach->Sampler->get_dims(mach->Sampler, unit, src.i[0], result);

   for (i = 0; i < TGSI_QUAD_SIZE; i++) {
      for (j = 0; j < 4; j++) {
         r[j].i[i] = result[j];
      }
   }

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &r[chan], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_sample(struct tgsi_exec_machine *mach,
            const struct tgsi_full_instruction *inst,
            enum tex_modifier modifier, bool compare)
{
   const unsigned resource_unit = inst->Src[1].Register.Index;
   const unsigned sampler_unit = inst->Src[2].Register.Index;
   union tgsi_exec_channel r[5], c1;
   const union tgsi_exec_channel *lod = &ZeroVec;
   enum tgsi_sampler_control control = TGSI_SAMPLER_LOD_NONE;
   unsigned chan;
   unsigned char swizzles[4];
   int8_t offsets[3];

   /* always fetch all 3 offsets, overkill but keeps code simple */
   fetch_texel_offsets(mach, inst, offsets);

   assert(modifier != TEX_MODIFIER_PROJECTED);

   if (modifier != TEX_MODIFIER_NONE) {
      if (modifier == TEX_MODIFIER_LOD_BIAS) {
         FETCH(&c1, 3, TGSI_CHAN_X);
         lod = &c1;
         control = TGSI_SAMPLER_LOD_BIAS;
      }
      else if (modifier == TEX_MODIFIER_EXPLICIT_LOD) {
         FETCH(&c1, 3, TGSI_CHAN_X);
         lod = &c1;
         control = TGSI_SAMPLER_LOD_EXPLICIT;
      }
      else if (modifier == TEX_MODIFIER_GATHER) {
         control = TGSI_SAMPLER_GATHER;
      }
      else {
         assert(modifier == TEX_MODIFIER_LEVEL_ZERO);
         control = TGSI_SAMPLER_LOD_ZERO;
      }
   }

   FETCH(&r[0], 0, TGSI_CHAN_X);

   switch (mach->SamplerViews[resource_unit].Resource) {
   case TGSI_TEXTURE_1D:
      if (compare) {
         FETCH(&r[2], 3, TGSI_CHAN_X);
         fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                     &r[0], &ZeroVec, &r[2], &ZeroVec, lod, /* S, T, P, C, LOD */
                     NULL, offsets, control,
                     &r[0], &r[1], &r[2], &r[3]);     /* R, G, B, A */
      }
      else {
         fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                     &r[0], &ZeroVec, &ZeroVec, &ZeroVec, lod, /* S, T, P, C, LOD */
                     NULL, offsets, control,
                     &r[0], &r[1], &r[2], &r[3]);     /* R, G, B, A */
      }
      break;

   case TGSI_TEXTURE_1D_ARRAY:
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
      FETCH(&r[1], 0, TGSI_CHAN_Y);
      if (compare) {
         FETCH(&r[2], 3, TGSI_CHAN_X);
         fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                     &r[0], &r[1], &r[2], &ZeroVec, lod,    /* S, T, P, C, LOD */
                     NULL, offsets, control,
                     &r[0], &r[1], &r[2], &r[3]);  /* outputs */
      }
      else {
         fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                     &r[0], &r[1], &ZeroVec, &ZeroVec, lod,    /* S, T, P, C, LOD */
                     NULL, offsets, control,
                     &r[0], &r[1], &r[2], &r[3]);  /* outputs */
      }
      break;

   case TGSI_TEXTURE_2D_ARRAY:
   case TGSI_TEXTURE_3D:
   case TGSI_TEXTURE_CUBE:
      FETCH(&r[1], 0, TGSI_CHAN_Y);
      FETCH(&r[2], 0, TGSI_CHAN_Z);
      if(compare) {
         FETCH(&r[3], 3, TGSI_CHAN_X);
         fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                     &r[0], &r[1], &r[2], &r[3], lod,
                     NULL, offsets, control,
                     &r[0], &r[1], &r[2], &r[3]);
      }
      else {
         fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                     &r[0], &r[1], &r[2], &ZeroVec, lod,
                     NULL, offsets, control,
                     &r[0], &r[1], &r[2], &r[3]);
      }
      break;

   case TGSI_TEXTURE_CUBE_ARRAY:
      FETCH(&r[1], 0, TGSI_CHAN_Y);
      FETCH(&r[2], 0, TGSI_CHAN_Z);
      FETCH(&r[3], 0, TGSI_CHAN_W);
      if(compare) {
         FETCH(&r[4], 3, TGSI_CHAN_X);
         fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                     &r[0], &r[1], &r[2], &r[3], &r[4],
                     NULL, offsets, control,
                     &r[0], &r[1], &r[2], &r[3]);
      }
      else {
         fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                     &r[0], &r[1], &r[2], &r[3], lod,
                     NULL, offsets, control,
                     &r[0], &r[1], &r[2], &r[3]);
      }
      break;


   default:
      assert(0);
   }

   swizzles[0] = inst->Src[1].Register.SwizzleX;
   swizzles[1] = inst->Src[1].Register.SwizzleY;
   swizzles[2] = inst->Src[1].Register.SwizzleZ;
   swizzles[3] = inst->Src[1].Register.SwizzleW;

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &r[swizzles[chan]],
                    &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_sample_d(struct tgsi_exec_machine *mach,
              const struct tgsi_full_instruction *inst)
{
   const unsigned resource_unit = inst->Src[1].Register.Index;
   const unsigned sampler_unit = inst->Src[2].Register.Index;
   union tgsi_exec_channel r[4];
   float derivs[3][2][TGSI_QUAD_SIZE];
   unsigned chan;
   unsigned char swizzles[4];
   int8_t offsets[3];

   /* always fetch all 3 offsets, overkill but keeps code simple */
   fetch_texel_offsets(mach, inst, offsets);

   FETCH(&r[0], 0, TGSI_CHAN_X);

   switch (mach->SamplerViews[resource_unit].Resource) {
   case TGSI_TEXTURE_1D:
   case TGSI_TEXTURE_1D_ARRAY:
      /* only 1D array actually needs Y */
      FETCH(&r[1], 0, TGSI_CHAN_Y);

      fetch_assign_deriv_channel(mach, inst, 3, TGSI_CHAN_X, derivs[0]);

      fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                  &r[0], &r[1], &ZeroVec, &ZeroVec, &ZeroVec,   /* S, T, P, C, LOD */
                  derivs, offsets, TGSI_SAMPLER_DERIVS_EXPLICIT,
                  &r[0], &r[1], &r[2], &r[3]);           /* R, G, B, A */
      break;

   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
   case TGSI_TEXTURE_2D_ARRAY:
      /* only 2D array actually needs Z */
      FETCH(&r[1], 0, TGSI_CHAN_Y);
      FETCH(&r[2], 0, TGSI_CHAN_Z);

      fetch_assign_deriv_channel(mach, inst, 3, TGSI_CHAN_X, derivs[0]);
      fetch_assign_deriv_channel(mach, inst, 3, TGSI_CHAN_Y, derivs[1]);

      fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                  &r[0], &r[1], &r[2], &ZeroVec, &ZeroVec,   /* inputs */
                  derivs, offsets, TGSI_SAMPLER_DERIVS_EXPLICIT,
                  &r[0], &r[1], &r[2], &r[3]);     /* outputs */
      break;

   case TGSI_TEXTURE_3D:
   case TGSI_TEXTURE_CUBE:
   case TGSI_TEXTURE_CUBE_ARRAY:
      /* only cube array actually needs W */
      FETCH(&r[1], 0, TGSI_CHAN_Y);
      FETCH(&r[2], 0, TGSI_CHAN_Z);
      FETCH(&r[3], 0, TGSI_CHAN_W);

      fetch_assign_deriv_channel(mach, inst, 3, TGSI_CHAN_X, derivs[0]);
      fetch_assign_deriv_channel(mach, inst, 3, TGSI_CHAN_Y, derivs[1]);
      fetch_assign_deriv_channel(mach, inst, 3, TGSI_CHAN_Z, derivs[2]);

      fetch_texel(mach->Sampler, resource_unit, sampler_unit,
                  &r[0], &r[1], &r[2], &r[3], &ZeroVec,
                  derivs, offsets, TGSI_SAMPLER_DERIVS_EXPLICIT,
                  &r[0], &r[1], &r[2], &r[3]);
      break;

   default:
      assert(0);
   }

   swizzles[0] = inst->Src[1].Register.SwizzleX;
   swizzles[1] = inst->Src[1].Register.SwizzleY;
   swizzles[2] = inst->Src[1].Register.SwizzleZ;
   swizzles[3] = inst->Src[1].Register.SwizzleW;

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &r[swizzles[chan]],
                    &inst->Dst[0], inst, chan);
      }
   }
}


/**
 * Evaluate a constant-valued coefficient at the position of the
 * current quad.
 */
static void
eval_constant_coef(
   struct tgsi_exec_machine *mach,
   unsigned attrib,
   unsigned chan )
{
   unsigned i;

   for( i = 0; i < TGSI_QUAD_SIZE; i++ ) {
      mach->Inputs[attrib].xyzw[chan].f[i] = mach->InterpCoefs[attrib].a0[chan];
   }
}

static void
interp_constant_offset(
      UNUSED const struct tgsi_exec_machine *mach,
      UNUSED unsigned attrib,
      UNUSED unsigned chan,
      UNUSED float ofs_x,
      UNUSED float ofs_y,
      UNUSED union tgsi_exec_channel *out_chan)
{
}

/**
 * Evaluate a linear-valued coefficient at the position of the
 * current quad.
 */
static void
interp_linear_offset(
      const struct tgsi_exec_machine *mach,
      unsigned attrib,
      unsigned chan,
      float ofs_x,
      float ofs_y,
      union tgsi_exec_channel *out_chan)
{
   const float dadx = mach->InterpCoefs[attrib].dadx[chan];
   const float dady = mach->InterpCoefs[attrib].dady[chan];
   const float delta = ofs_x * dadx + ofs_y * dady;
   out_chan->f[0] += delta;
   out_chan->f[1] += delta;
   out_chan->f[2] += delta;
   out_chan->f[3] += delta;
}

static void
eval_linear_coef(struct tgsi_exec_machine *mach,
                 unsigned attrib,
                 unsigned chan)
{
   const float x = mach->QuadPos.xyzw[0].f[0];
   const float y = mach->QuadPos.xyzw[1].f[0];
   const float dadx = mach->InterpCoefs[attrib].dadx[chan];
   const float dady = mach->InterpCoefs[attrib].dady[chan];
   const float a0 = mach->InterpCoefs[attrib].a0[chan] + dadx * x + dady * y;

   mach->Inputs[attrib].xyzw[chan].f[0] = a0;
   mach->Inputs[attrib].xyzw[chan].f[1] = a0 + dadx;
   mach->Inputs[attrib].xyzw[chan].f[2] = a0 + dady;
   mach->Inputs[attrib].xyzw[chan].f[3] = a0 + dadx + dady;
}

/**
 * Evaluate a perspective-valued coefficient at the position of the
 * current quad.
 */

static void
interp_perspective_offset(
   const struct tgsi_exec_machine *mach,
   unsigned attrib,
   unsigned chan,
   float ofs_x,
   float ofs_y,
   union tgsi_exec_channel *out_chan)
{
   const float dadx = mach->InterpCoefs[attrib].dadx[chan];
   const float dady = mach->InterpCoefs[attrib].dady[chan];
   const float *w = mach->QuadPos.xyzw[3].f;
   const float delta = ofs_x * dadx + ofs_y * dady;
   out_chan->f[0] += delta / w[0];
   out_chan->f[1] += delta / w[1];
   out_chan->f[2] += delta / w[2];
   out_chan->f[3] += delta / w[3];
}

static void
eval_perspective_coef(
   struct tgsi_exec_machine *mach,
   unsigned attrib,
   unsigned chan )
{
   const float x = mach->QuadPos.xyzw[0].f[0];
   const float y = mach->QuadPos.xyzw[1].f[0];
   const float dadx = mach->InterpCoefs[attrib].dadx[chan];
   const float dady = mach->InterpCoefs[attrib].dady[chan];
   const float a0 = mach->InterpCoefs[attrib].a0[chan] + dadx * x + dady * y;
   const float *w = mach->QuadPos.xyzw[3].f;
   /* divide by W here */
   mach->Inputs[attrib].xyzw[chan].f[0] = a0 / w[0];
   mach->Inputs[attrib].xyzw[chan].f[1] = (a0 + dadx) / w[1];
   mach->Inputs[attrib].xyzw[chan].f[2] = (a0 + dady) / w[2];
   mach->Inputs[attrib].xyzw[chan].f[3] = (a0 + dadx + dady) / w[3];
}


typedef void (* eval_coef_func)(
   struct tgsi_exec_machine *mach,
   unsigned attrib,
   unsigned chan );

static void
exec_declaration(struct tgsi_exec_machine *mach,
                 const struct tgsi_full_declaration *decl)
{
   if (decl->Declaration.File == TGSI_FILE_SAMPLER_VIEW) {
      mach->SamplerViews[decl->Range.First] = decl->SamplerView;
      return;
   }

   if (mach->ShaderType == PIPE_SHADER_FRAGMENT) {
      if (decl->Declaration.File == TGSI_FILE_INPUT) {
         unsigned first, last, mask;

         first = decl->Range.First;
         last = decl->Range.Last;
         mask = decl->Declaration.UsageMask;

         /* XXX we could remove this special-case code since
          * mach->InterpCoefs[first].a0 should already have the
          * front/back-face value.  But we should first update the
          * ureg code to emit the right UsageMask value (WRITEMASK_X).
          * Then, we could remove the tgsi_exec_machine::Face field.
          */
         /* XXX make FACE a system value */
         if (decl->Semantic.Name == TGSI_SEMANTIC_FACE) {
            unsigned i;

            assert(decl->Semantic.Index == 0);
            assert(first == last);

            for (i = 0; i < TGSI_QUAD_SIZE; i++) {
               mach->Inputs[first].xyzw[0].f[i] = mach->Face;
            }
         } else {
            eval_coef_func eval;
            apply_sample_offset_func interp;
            unsigned i, j;

            switch (decl->Interp.Interpolate) {
            case TGSI_INTERPOLATE_CONSTANT:
               eval = eval_constant_coef;
               interp = interp_constant_offset;
               break;

            case TGSI_INTERPOLATE_LINEAR:
               eval = eval_linear_coef;
               interp = interp_linear_offset;
               break;

            case TGSI_INTERPOLATE_PERSPECTIVE:
               eval = eval_perspective_coef;
               interp = interp_perspective_offset;
               break;

            case TGSI_INTERPOLATE_COLOR:
               eval = mach->flatshade_color ? eval_constant_coef : eval_perspective_coef;
               interp = mach->flatshade_color ? interp_constant_offset : interp_perspective_offset;
               break;

            default:
               assert(0);
               return;
            }

            for (i = first; i <= last; i++)
               mach->InputSampleOffsetApply[i] = interp;

            for (j = 0; j < TGSI_NUM_CHANNELS; j++) {
               if (mask & (1 << j)) {
                  for (i = first; i <= last; i++) {
                     eval(mach, i, j);
                  }
               }
            }
         }

         if (DEBUG_EXECUTION) {
            unsigned i, j;
            for (i = first; i <= last; ++i) {
               debug_printf("IN[%2u] = ", i);
               for (j = 0; j < TGSI_NUM_CHANNELS; j++) {
                  if (j > 0) {
                     debug_printf("         ");
                  }
                  debug_printf("(%6f %u, %6f %u, %6f %u, %6f %u)\n",
                               mach->Inputs[i].xyzw[0].f[j], mach->Inputs[i].xyzw[0].u[j],
                               mach->Inputs[i].xyzw[1].f[j], mach->Inputs[i].xyzw[1].u[j],
                               mach->Inputs[i].xyzw[2].f[j], mach->Inputs[i].xyzw[2].u[j],
                               mach->Inputs[i].xyzw[3].f[j], mach->Inputs[i].xyzw[3].u[j]);
               }
            }
         }
      }
   }

}

typedef void (* micro_unary_op)(union tgsi_exec_channel *dst,
                                const union tgsi_exec_channel *src);

static void
exec_scalar_unary(struct tgsi_exec_machine *mach,
                  const struct tgsi_full_instruction *inst,
                  micro_unary_op op,
                  enum tgsi_exec_datatype src_datatype)
{
   unsigned int chan;
   union tgsi_exec_channel src;
   union tgsi_exec_channel dst;

   fetch_source(mach, &src, &inst->Src[0], TGSI_CHAN_X, src_datatype);
   op(&dst, &src);
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst, &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_vector_unary(struct tgsi_exec_machine *mach,
                  const struct tgsi_full_instruction *inst,
                  micro_unary_op op,
                  enum tgsi_exec_datatype src_datatype)
{
   unsigned int chan;
   struct tgsi_exec_vector dst;

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         union tgsi_exec_channel src;

         fetch_source(mach, &src, &inst->Src[0], chan, src_datatype);
         op(&dst.xyzw[chan], &src);
      }
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst.xyzw[chan], &inst->Dst[0], inst, chan);
      }
   }
}

typedef void (* micro_binary_op)(union tgsi_exec_channel *dst,
                                 const union tgsi_exec_channel *src0,
                                 const union tgsi_exec_channel *src1);

static void
exec_scalar_binary(struct tgsi_exec_machine *mach,
                   const struct tgsi_full_instruction *inst,
                   micro_binary_op op,
                   enum tgsi_exec_datatype src_datatype)
{
   unsigned int chan;
   union tgsi_exec_channel src[2];
   union tgsi_exec_channel dst;

   fetch_source(mach, &src[0], &inst->Src[0], TGSI_CHAN_X, src_datatype);
   fetch_source(mach, &src[1], &inst->Src[1], TGSI_CHAN_X, src_datatype);
   op(&dst, &src[0], &src[1]);
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst, &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_vector_binary(struct tgsi_exec_machine *mach,
                   const struct tgsi_full_instruction *inst,
                   micro_binary_op op,
                   enum tgsi_exec_datatype src_datatype)
{
   unsigned int chan;
   struct tgsi_exec_vector dst;

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         union tgsi_exec_channel src[2];

         fetch_source(mach, &src[0], &inst->Src[0], chan, src_datatype);
         fetch_source(mach, &src[1], &inst->Src[1], chan, src_datatype);
         op(&dst.xyzw[chan], &src[0], &src[1]);
      }
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst.xyzw[chan], &inst->Dst[0], inst, chan);
      }
   }
}

typedef void (* micro_trinary_op)(union tgsi_exec_channel *dst,
                                  const union tgsi_exec_channel *src0,
                                  const union tgsi_exec_channel *src1,
                                  const union tgsi_exec_channel *src2);

static void
exec_vector_trinary(struct tgsi_exec_machine *mach,
                    const struct tgsi_full_instruction *inst,
                    micro_trinary_op op,
                    enum tgsi_exec_datatype src_datatype)
{
   unsigned int chan;
   struct tgsi_exec_vector dst;

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         union tgsi_exec_channel src[3];

         fetch_source(mach, &src[0], &inst->Src[0], chan, src_datatype);
         fetch_source(mach, &src[1], &inst->Src[1], chan, src_datatype);
         fetch_source(mach, &src[2], &inst->Src[2], chan, src_datatype);
         op(&dst.xyzw[chan], &src[0], &src[1], &src[2]);
      }
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst.xyzw[chan], &inst->Dst[0], inst, chan);
      }
   }
}

typedef void (* micro_quaternary_op)(union tgsi_exec_channel *dst,
                                     const union tgsi_exec_channel *src0,
                                     const union tgsi_exec_channel *src1,
                                     const union tgsi_exec_channel *src2,
                                     const union tgsi_exec_channel *src3);

static void
exec_vector_quaternary(struct tgsi_exec_machine *mach,
                       const struct tgsi_full_instruction *inst,
                       micro_quaternary_op op,
                       enum tgsi_exec_datatype src_datatype)
{
   unsigned int chan;
   struct tgsi_exec_vector dst;

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         union tgsi_exec_channel src[4];

         fetch_source(mach, &src[0], &inst->Src[0], chan, src_datatype);
         fetch_source(mach, &src[1], &inst->Src[1], chan, src_datatype);
         fetch_source(mach, &src[2], &inst->Src[2], chan, src_datatype);
         fetch_source(mach, &src[3], &inst->Src[3], chan, src_datatype);
         op(&dst.xyzw[chan], &src[0], &src[1], &src[2], &src[3]);
      }
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst.xyzw[chan], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_dp3(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   unsigned int chan;
   union tgsi_exec_channel arg[3];

   fetch_source(mach, &arg[0], &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   fetch_source(mach, &arg[1], &inst->Src[1], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   micro_mul(&arg[2], &arg[0], &arg[1]);

   for (chan = TGSI_CHAN_Y; chan <= TGSI_CHAN_Z; chan++) {
      fetch_source(mach, &arg[0], &inst->Src[0], chan, TGSI_EXEC_DATA_FLOAT);
      fetch_source(mach, &arg[1], &inst->Src[1], chan, TGSI_EXEC_DATA_FLOAT);
      micro_mad(&arg[2], &arg[0], &arg[1], &arg[2]);
   }

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &arg[2], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_dp4(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   unsigned int chan;
   union tgsi_exec_channel arg[3];

   fetch_source(mach, &arg[0], &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   fetch_source(mach, &arg[1], &inst->Src[1], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   micro_mul(&arg[2], &arg[0], &arg[1]);

   for (chan = TGSI_CHAN_Y; chan <= TGSI_CHAN_W; chan++) {
      fetch_source(mach, &arg[0], &inst->Src[0], chan, TGSI_EXEC_DATA_FLOAT);
      fetch_source(mach, &arg[1], &inst->Src[1], chan, TGSI_EXEC_DATA_FLOAT);
      micro_mad(&arg[2], &arg[0], &arg[1], &arg[2]);
   }

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &arg[2], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_dp2(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   unsigned int chan;
   union tgsi_exec_channel arg[3];

   fetch_source(mach, &arg[0], &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   fetch_source(mach, &arg[1], &inst->Src[1], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   micro_mul(&arg[2], &arg[0], &arg[1]);

   fetch_source(mach, &arg[0], &inst->Src[0], TGSI_CHAN_Y, TGSI_EXEC_DATA_FLOAT);
   fetch_source(mach, &arg[1], &inst->Src[1], TGSI_CHAN_Y, TGSI_EXEC_DATA_FLOAT);
   micro_mad(&arg[2], &arg[0], &arg[1], &arg[2]);

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &arg[2], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_pk2h(struct tgsi_exec_machine *mach,
          const struct tgsi_full_instruction *inst)
{
   unsigned chan;
   union tgsi_exec_channel arg[2], dst;

   fetch_source(mach, &arg[0], &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   fetch_source(mach, &arg[1], &inst->Src[0], TGSI_CHAN_Y, TGSI_EXEC_DATA_FLOAT);
   for (chan = 0; chan < TGSI_QUAD_SIZE; chan++) {
      dst.u[chan] = _mesa_float_to_half(arg[0].f[chan]) |
         (_mesa_float_to_half(arg[1].f[chan]) << 16);
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst, &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_up2h(struct tgsi_exec_machine *mach,
          const struct tgsi_full_instruction *inst)
{
   unsigned chan;
   union tgsi_exec_channel arg, dst[2];

   fetch_source(mach, &arg, &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_UINT);
   for (chan = 0; chan < TGSI_QUAD_SIZE; chan++) {
      dst[0].f[chan] = _mesa_half_to_float(arg.u[chan] & 0xffff);
      dst[1].f[chan] = _mesa_half_to_float(arg.u[chan] >> 16);
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst[chan & 1], &inst->Dst[0], inst, chan);
      }
   }
}

static void
micro_ucmp(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1,
           const union tgsi_exec_channel *src2)
{
   dst->f[0] = src0->u[0] ? src1->f[0] : src2->f[0];
   dst->f[1] = src0->u[1] ? src1->f[1] : src2->f[1];
   dst->f[2] = src0->u[2] ? src1->f[2] : src2->f[2];
   dst->f[3] = src0->u[3] ? src1->f[3] : src2->f[3];
}

static void
exec_ucmp(struct tgsi_exec_machine *mach,
          const struct tgsi_full_instruction *inst)
{
   unsigned int chan;
   struct tgsi_exec_vector dst;

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         union tgsi_exec_channel src[3];

         fetch_source(mach, &src[0], &inst->Src[0], chan,
                      TGSI_EXEC_DATA_UINT);
         fetch_source(mach, &src[1], &inst->Src[1], chan,
                      TGSI_EXEC_DATA_FLOAT);
         fetch_source(mach, &src[2], &inst->Src[2], chan,
                      TGSI_EXEC_DATA_FLOAT);
         micro_ucmp(&dst.xyzw[chan], &src[0], &src[1], &src[2]);
      }
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &dst.xyzw[chan], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_dst(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[2];
   union tgsi_exec_channel d[4];

   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      fetch_source(mach, &r[0], &inst->Src[0], TGSI_CHAN_Y, TGSI_EXEC_DATA_FLOAT);
      fetch_source(mach, &r[1], &inst->Src[1], TGSI_CHAN_Y, TGSI_EXEC_DATA_FLOAT);
      micro_mul(&d[TGSI_CHAN_Y], &r[0], &r[1]);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
      fetch_source(mach, &d[TGSI_CHAN_Z], &inst->Src[0], TGSI_CHAN_Z, TGSI_EXEC_DATA_FLOAT);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      fetch_source(mach, &d[TGSI_CHAN_W], &inst->Src[1], TGSI_CHAN_W, TGSI_EXEC_DATA_FLOAT);
   }

   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      store_dest(mach, &OneVec, &inst->Dst[0], inst, TGSI_CHAN_X);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      store_dest(mach, &d[TGSI_CHAN_Y], &inst->Dst[0], inst, TGSI_CHAN_Y);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
      store_dest(mach, &d[TGSI_CHAN_Z], &inst->Dst[0], inst, TGSI_CHAN_Z);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      store_dest(mach, &d[TGSI_CHAN_W], &inst->Dst[0], inst, TGSI_CHAN_W);
   }
}

static void
exec_log(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[3];

   fetch_source(mach, &r[0], &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   micro_abs(&r[2], &r[0]);  /* r2 = abs(r0) */
   micro_lg2(&r[1], &r[2]);  /* r1 = lg2(r2) */
   micro_flr(&r[0], &r[1]);  /* r0 = floor(r1) */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      store_dest(mach, &r[0], &inst->Dst[0], inst, TGSI_CHAN_X);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      micro_exp2(&r[0], &r[0]);       /* r0 = 2 ^ r0 */
      micro_div(&r[0], &r[2], &r[0]); /* r0 = r2 / r0 */
      store_dest(mach, &r[0], &inst->Dst[0], inst, TGSI_CHAN_Y);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
      store_dest(mach, &r[1], &inst->Dst[0], inst, TGSI_CHAN_Z);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      store_dest(mach, &OneVec, &inst->Dst[0], inst, TGSI_CHAN_W);
   }
}

static void
exec_exp(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[3];

   fetch_source(mach, &r[0], &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   micro_flr(&r[1], &r[0]);  /* r1 = floor(r0) */
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      micro_exp2(&r[2], &r[1]);       /* r2 = 2 ^ r1 */
      store_dest(mach, &r[2], &inst->Dst[0], inst, TGSI_CHAN_X);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
      micro_sub(&r[2], &r[0], &r[1]); /* r2 = r0 - r1 */
      store_dest(mach, &r[2], &inst->Dst[0], inst, TGSI_CHAN_Y);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
      micro_exp2(&r[2], &r[0]);       /* r2 = 2 ^ r0 */
      store_dest(mach, &r[2], &inst->Dst[0], inst, TGSI_CHAN_Z);
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      store_dest(mach, &OneVec, &inst->Dst[0], inst, TGSI_CHAN_W);
   }
}

static void
exec_lit(struct tgsi_exec_machine *mach,
         const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[3];
   union tgsi_exec_channel d[3];

   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_YZ) {
      fetch_source(mach, &r[0], &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
      if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Z) {
         fetch_source(mach, &r[1], &inst->Src[0], TGSI_CHAN_Y, TGSI_EXEC_DATA_FLOAT);
         micro_max(&r[1], &r[1], &ZeroVec);

         fetch_source(mach, &r[2], &inst->Src[0], TGSI_CHAN_W, TGSI_EXEC_DATA_FLOAT);
         micro_min(&r[2], &r[2], &P128Vec);
         micro_max(&r[2], &r[2], &M128Vec);
         micro_pow(&r[1], &r[1], &r[2]);
         micro_lt(&d[TGSI_CHAN_Z], &ZeroVec, &r[0], &r[1], &ZeroVec);
         store_dest(mach, &d[TGSI_CHAN_Z], &inst->Dst[0], inst, TGSI_CHAN_Z);
      }
      if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_Y) {
         micro_max(&d[TGSI_CHAN_Y], &r[0], &ZeroVec);
         store_dest(mach, &d[TGSI_CHAN_Y], &inst->Dst[0], inst, TGSI_CHAN_Y);
      }
   }
   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      store_dest(mach, &OneVec, &inst->Dst[0], inst, TGSI_CHAN_X);
   }

   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_W) {
      store_dest(mach, &OneVec, &inst->Dst[0], inst, TGSI_CHAN_W);
   }
}

static void
exec_break(struct tgsi_exec_machine *mach)
{
   if (mach->BreakType == TGSI_EXEC_BREAK_INSIDE_LOOP) {
      /* turn off loop channels for each enabled exec channel */
      mach->LoopMask &= ~mach->ExecMask;
      /* Todo: if mach->LoopMask == 0, jump to end of loop */
      UPDATE_EXEC_MASK(mach);
   } else {
      assert(mach->BreakType == TGSI_EXEC_BREAK_INSIDE_SWITCH);

      mach->Switch.mask = 0x0;

      UPDATE_EXEC_MASK(mach);
   }
}

static void
exec_switch(struct tgsi_exec_machine *mach,
            const struct tgsi_full_instruction *inst)
{
   assert(mach->SwitchStackTop < TGSI_EXEC_MAX_SWITCH_NESTING);
   assert(mach->BreakStackTop < TGSI_EXEC_MAX_BREAK_STACK);

   mach->SwitchStack[mach->SwitchStackTop++] = mach->Switch;
   fetch_source(mach, &mach->Switch.selector, &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_UINT);
   mach->Switch.mask = 0x0;
   mach->Switch.defaultMask = 0x0;

   mach->BreakStack[mach->BreakStackTop++] = mach->BreakType;
   mach->BreakType = TGSI_EXEC_BREAK_INSIDE_SWITCH;

   UPDATE_EXEC_MASK(mach);
}

static void
exec_case(struct tgsi_exec_machine *mach,
          const struct tgsi_full_instruction *inst)
{
   unsigned prevMask = mach->SwitchStack[mach->SwitchStackTop - 1].mask;
   union tgsi_exec_channel src;
   unsigned mask = 0;

   fetch_source(mach, &src, &inst->Src[0], TGSI_CHAN_X, TGSI_EXEC_DATA_UINT);

   if (mach->Switch.selector.u[0] == src.u[0]) {
      mask |= 0x1;
   }
   if (mach->Switch.selector.u[1] == src.u[1]) {
      mask |= 0x2;
   }
   if (mach->Switch.selector.u[2] == src.u[2]) {
      mask |= 0x4;
   }
   if (mach->Switch.selector.u[3] == src.u[3]) {
      mask |= 0x8;
   }

   mach->Switch.defaultMask |= mask;

   mach->Switch.mask |= mask & prevMask;

   UPDATE_EXEC_MASK(mach);
}

/* FIXME: this will only work if default is last */
static void
exec_default(struct tgsi_exec_machine *mach)
{
   unsigned prevMask = mach->SwitchStack[mach->SwitchStackTop - 1].mask;

   mach->Switch.mask |= ~mach->Switch.defaultMask & prevMask;

   UPDATE_EXEC_MASK(mach);
}

static void
exec_endswitch(struct tgsi_exec_machine *mach)
{
   mach->Switch = mach->SwitchStack[--mach->SwitchStackTop];
   mach->BreakType = mach->BreakStack[--mach->BreakStackTop];

   UPDATE_EXEC_MASK(mach);
}

typedef void (* micro_dop)(union tgsi_double_channel *dst,
                           const union tgsi_double_channel *src);

typedef void (* micro_dop_sop)(union tgsi_double_channel *dst,
                               const union tgsi_double_channel *src0,
                               union tgsi_exec_channel *src1);

typedef void (* micro_dop_s)(union tgsi_double_channel *dst,
                             const union tgsi_exec_channel *src);

typedef void (* micro_sop_d)(union tgsi_exec_channel *dst,
                             const union tgsi_double_channel *src);

static void
fetch_double_channel(struct tgsi_exec_machine *mach,
                     union tgsi_double_channel *chan,
                     const struct tgsi_full_src_register *reg,
                     unsigned chan_0,
                     unsigned chan_1)
{
   union tgsi_exec_channel src[2];
   unsigned i;

   fetch_source_d(mach, &src[0], reg, chan_0);
   fetch_source_d(mach, &src[1], reg, chan_1);

   for (i = 0; i < TGSI_QUAD_SIZE; i++) {
      chan->u[i][0] = src[0].u[i];
      chan->u[i][1] = src[1].u[i];
   }
   assert(!reg->Register.Absolute);
   assert(!reg->Register.Negate);
}

static void
store_double_channel(struct tgsi_exec_machine *mach,
                     const union tgsi_double_channel *chan,
                     const struct tgsi_full_dst_register *reg,
                     const struct tgsi_full_instruction *inst,
                     unsigned chan_0,
                     unsigned chan_1)
{
   union tgsi_exec_channel dst[2];
   unsigned i;
   union tgsi_double_channel temp;
   const unsigned execmask = mach->ExecMask;

   if (!inst->Instruction.Saturate) {
      for (i = 0; i < TGSI_QUAD_SIZE; i++)
         if (execmask & (1 << i)) {
            dst[0].u[i] = chan->u[i][0];
            dst[1].u[i] = chan->u[i][1];
         }
   }
   else {
      for (i = 0; i < TGSI_QUAD_SIZE; i++)
         if (execmask & (1 << i)) {
            if (chan->d[i] < 0.0 || isnan(chan->d[i]))
               temp.d[i] = 0.0;
            else if (chan->d[i] > 1.0)
               temp.d[i] = 1.0;
            else
               temp.d[i] = chan->d[i];

            dst[0].u[i] = temp.u[i][0];
            dst[1].u[i] = temp.u[i][1];
         }
   }

   store_dest_double(mach, &dst[0], reg, chan_0);
   if (chan_1 != (unsigned)-1)
      store_dest_double(mach, &dst[1], reg, chan_1);
}

static void
exec_double_unary(struct tgsi_exec_machine *mach,
                  const struct tgsi_full_instruction *inst,
                  micro_dop op)
{
   union tgsi_double_channel src;
   union tgsi_double_channel dst;

   if ((inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_XY) == TGSI_WRITEMASK_XY) {
      fetch_double_channel(mach, &src, &inst->Src[0], TGSI_CHAN_X, TGSI_CHAN_Y);
      op(&dst, &src);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_X, TGSI_CHAN_Y);
   }
   if ((inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_ZW) == TGSI_WRITEMASK_ZW) {
      fetch_double_channel(mach, &src, &inst->Src[0], TGSI_CHAN_Z, TGSI_CHAN_W);
      op(&dst, &src);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_Z, TGSI_CHAN_W);
   }
}

static void
exec_double_binary(struct tgsi_exec_machine *mach,
                   const struct tgsi_full_instruction *inst,
                   micro_dop op,
                   enum tgsi_exec_datatype dst_datatype)
{
   union tgsi_double_channel src[2];
   union tgsi_double_channel dst;
   int first_dest_chan, second_dest_chan;
   int wmask;

   wmask = inst->Dst[0].Register.WriteMask;
   /* these are & because of the way DSLT etc store their destinations */
   if (wmask & TGSI_WRITEMASK_XY) {
      first_dest_chan = TGSI_CHAN_X;
      second_dest_chan = TGSI_CHAN_Y;
      if (dst_datatype == TGSI_EXEC_DATA_UINT) {
         first_dest_chan = (wmask & TGSI_WRITEMASK_X) ? TGSI_CHAN_X : TGSI_CHAN_Y;
         second_dest_chan = -1;
      }

      fetch_double_channel(mach, &src[0], &inst->Src[0], TGSI_CHAN_X, TGSI_CHAN_Y);
      fetch_double_channel(mach, &src[1], &inst->Src[1], TGSI_CHAN_X, TGSI_CHAN_Y);
      op(&dst, src);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, first_dest_chan, second_dest_chan);
   }

   if (wmask & TGSI_WRITEMASK_ZW) {
      first_dest_chan = TGSI_CHAN_Z;
      second_dest_chan = TGSI_CHAN_W;
      if (dst_datatype == TGSI_EXEC_DATA_UINT) {
         first_dest_chan = (wmask & TGSI_WRITEMASK_Z) ? TGSI_CHAN_Z : TGSI_CHAN_W;
         second_dest_chan = -1;
      }

      fetch_double_channel(mach, &src[0], &inst->Src[0], TGSI_CHAN_Z, TGSI_CHAN_W);
      fetch_double_channel(mach, &src[1], &inst->Src[1], TGSI_CHAN_Z, TGSI_CHAN_W);
      op(&dst, src);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, first_dest_chan, second_dest_chan);
   }
}

static void
exec_double_trinary(struct tgsi_exec_machine *mach,
                    const struct tgsi_full_instruction *inst,
                    micro_dop op)
{
   union tgsi_double_channel src[3];
   union tgsi_double_channel dst;

   if ((inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_XY) == TGSI_WRITEMASK_XY) {
      fetch_double_channel(mach, &src[0], &inst->Src[0], TGSI_CHAN_X, TGSI_CHAN_Y);
      fetch_double_channel(mach, &src[1], &inst->Src[1], TGSI_CHAN_X, TGSI_CHAN_Y);
      fetch_double_channel(mach, &src[2], &inst->Src[2], TGSI_CHAN_X, TGSI_CHAN_Y);
      op(&dst, src);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_X, TGSI_CHAN_Y);
   }
   if ((inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_ZW) == TGSI_WRITEMASK_ZW) {
      fetch_double_channel(mach, &src[0], &inst->Src[0], TGSI_CHAN_Z, TGSI_CHAN_W);
      fetch_double_channel(mach, &src[1], &inst->Src[1], TGSI_CHAN_Z, TGSI_CHAN_W);
      fetch_double_channel(mach, &src[2], &inst->Src[2], TGSI_CHAN_Z, TGSI_CHAN_W);
      op(&dst, src);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_Z, TGSI_CHAN_W);
   }
}

static void
exec_dldexp(struct tgsi_exec_machine *mach,
            const struct tgsi_full_instruction *inst)
{
   union tgsi_double_channel src0;
   union tgsi_exec_channel src1;
   union tgsi_double_channel dst;
   int wmask;

   wmask = inst->Dst[0].Register.WriteMask;
   if (wmask & TGSI_WRITEMASK_XY) {
      fetch_double_channel(mach, &src0, &inst->Src[0], TGSI_CHAN_X, TGSI_CHAN_Y);
      fetch_source(mach, &src1, &inst->Src[1], TGSI_CHAN_X, TGSI_EXEC_DATA_INT);
      micro_dldexp(&dst, &src0, &src1);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_X, TGSI_CHAN_Y);
   }

   if (wmask & TGSI_WRITEMASK_ZW) {
      fetch_double_channel(mach, &src0, &inst->Src[0], TGSI_CHAN_Z, TGSI_CHAN_W);
      fetch_source(mach, &src1, &inst->Src[1], TGSI_CHAN_Z, TGSI_EXEC_DATA_INT);
      micro_dldexp(&dst, &src0, &src1);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_Z, TGSI_CHAN_W);
   }
}

static void
exec_arg0_64_arg1_32(struct tgsi_exec_machine *mach,
            const struct tgsi_full_instruction *inst,
            micro_dop_sop op)
{
   union tgsi_double_channel src0;
   union tgsi_exec_channel src1;
   union tgsi_double_channel dst;
   int wmask;

   wmask = inst->Dst[0].Register.WriteMask;
   if (wmask & TGSI_WRITEMASK_XY) {
      fetch_double_channel(mach, &src0, &inst->Src[0], TGSI_CHAN_X, TGSI_CHAN_Y);
      fetch_source(mach, &src1, &inst->Src[1], TGSI_CHAN_X, TGSI_EXEC_DATA_INT);
      op(&dst, &src0, &src1);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_X, TGSI_CHAN_Y);
   }

   if (wmask & TGSI_WRITEMASK_ZW) {
      fetch_double_channel(mach, &src0, &inst->Src[0], TGSI_CHAN_Z, TGSI_CHAN_W);
      fetch_source(mach, &src1, &inst->Src[1], TGSI_CHAN_Z, TGSI_EXEC_DATA_INT);
      op(&dst, &src0, &src1);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_Z, TGSI_CHAN_W);
   }
}

static int
get_image_coord_dim(unsigned tgsi_tex)
{
   int dim;
   switch (tgsi_tex) {
   case TGSI_TEXTURE_BUFFER:
   case TGSI_TEXTURE_1D:
      dim = 1;
      break;
   case TGSI_TEXTURE_2D:
   case TGSI_TEXTURE_RECT:
   case TGSI_TEXTURE_1D_ARRAY:
   case TGSI_TEXTURE_2D_MSAA:
      dim = 2;
      break;
   case TGSI_TEXTURE_3D:
   case TGSI_TEXTURE_CUBE:
   case TGSI_TEXTURE_2D_ARRAY:
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
   case TGSI_TEXTURE_CUBE_ARRAY:
      dim = 3;
      break;
   default:
      assert(!"unknown texture target");
      dim = 0;
      break;
   }

   return dim;
}

static int
get_image_coord_sample(unsigned tgsi_tex)
{
   int sample = 0;
   switch (tgsi_tex) {
   case TGSI_TEXTURE_2D_MSAA:
      sample = 3;
      break;
   case TGSI_TEXTURE_2D_ARRAY_MSAA:
      sample = 4;
      break;
   default:
      break;
   }
   return sample;
}

static void
exec_load_img(struct tgsi_exec_machine *mach,
              const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[4], sample_r;
   unsigned unit;
   int sample;
   int i, j;
   int dim;
   unsigned chan;
   float rgba[TGSI_NUM_CHANNELS][TGSI_QUAD_SIZE];
   struct tgsi_image_params params;

   unit = fetch_sampler_unit(mach, inst, 0);
   dim = get_image_coord_dim(inst->Memory.Texture);
   sample = get_image_coord_sample(inst->Memory.Texture);
   assert(dim <= 3);

   params.execmask = mach->ExecMask & mach->NonHelperMask & ~mach->KillMask;
   params.unit = unit;
   params.tgsi_tex_instr = inst->Memory.Texture;
   params.format = inst->Memory.Format;

   for (i = 0; i < dim; i++) {
      IFETCH(&r[i], 1, TGSI_CHAN_X + i);
   }

   if (sample)
      IFETCH(&sample_r, 1, TGSI_CHAN_X + sample);

   mach->Image->load(mach->Image, &params,
                     r[0].i, r[1].i, r[2].i, sample_r.i,
                     rgba);
   for (j = 0; j < TGSI_QUAD_SIZE; j++) {
      r[0].f[j] = rgba[0][j];
      r[1].f[j] = rgba[1][j];
      r[2].f[j] = rgba[2][j];
      r[3].f[j] = rgba[3][j];
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &r[chan], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_load_membuf(struct tgsi_exec_machine *mach,
                 const struct tgsi_full_instruction *inst)
{
   uint32_t unit = fetch_sampler_unit(mach, inst, 0);

   uint32_t size;
   const char *ptr;
   switch (inst->Src[0].Register.File) {
   case TGSI_FILE_MEMORY:
      ptr = mach->LocalMem;
      size = mach->LocalMemSize;
      break;

   case TGSI_FILE_BUFFER:
      ptr = mach->Buffer->lookup(mach->Buffer, unit, &size);
      break;

   case TGSI_FILE_CONSTANT:
      if (unit < ARRAY_SIZE(mach->Consts)) {
         ptr = mach->Consts[unit];
         size = mach->ConstsSize[unit];
      } else {
         ptr = NULL;
         size = 0;
      }
      break;

   default:
      unreachable("unsupported TGSI_OPCODE_LOAD file");
   }

   union tgsi_exec_channel offset;
   IFETCH(&offset, 1, TGSI_CHAN_X);

   assert(inst->Dst[0].Register.WriteMask);
   uint32_t load_size = util_last_bit(inst->Dst[0].Register.WriteMask) * 4;

   union tgsi_exec_channel rgba[TGSI_NUM_CHANNELS];
   memset(&rgba, 0, sizeof(rgba));
   for (int j = 0; j < TGSI_QUAD_SIZE; j++) {
      if (size >= load_size && offset.u[j] <= (size - load_size)) {
         for (int chan = 0; chan < load_size / 4; chan++)
            rgba[chan].u[j] = *(uint32_t *)(ptr + offset.u[j] + chan * 4);
      }
   }

   for (int chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &rgba[chan], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_load(struct tgsi_exec_machine *mach,
          const struct tgsi_full_instruction *inst)
{
   if (inst->Src[0].Register.File == TGSI_FILE_IMAGE)
      exec_load_img(mach, inst);
   else
      exec_load_membuf(mach, inst);
}

static unsigned
fetch_store_img_unit(struct tgsi_exec_machine *mach,
                     const struct tgsi_full_dst_register *dst)
{
   unsigned unit = 0;
   int i;
   if (dst->Register.Indirect) {
      union tgsi_exec_channel indir_index, index2;
      const unsigned execmask = mach->ExecMask;
      index2.i[0] =
      index2.i[1] =
      index2.i[2] =
      index2.i[3] = dst->Indirect.Index;

      fetch_src_file_channel(mach,
                             dst->Indirect.File,
                             dst->Indirect.Swizzle,
                             &index2,
                             &ZeroVec,
                             &indir_index);
      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         if (execmask & (1 << i)) {
            unit = dst->Register.Index + indir_index.i[i];
            break;
         }
      }
   } else {
      unit = dst->Register.Index;
   }
   return unit;
}

static void
exec_store_img(struct tgsi_exec_machine *mach,
               const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[3], sample_r;
   union tgsi_exec_channel value[4];
   float rgba[TGSI_NUM_CHANNELS][TGSI_QUAD_SIZE];
   struct tgsi_image_params params;
   int dim;
   int sample;
   int i, j;
   unsigned unit;
   unit = fetch_store_img_unit(mach, &inst->Dst[0]);
   dim = get_image_coord_dim(inst->Memory.Texture);
   sample = get_image_coord_sample(inst->Memory.Texture);
   assert(dim <= 3);

   params.execmask = mach->ExecMask & mach->NonHelperMask & ~mach->KillMask;
   params.unit = unit;
   params.tgsi_tex_instr = inst->Memory.Texture;
   params.format = inst->Memory.Format;

   for (i = 0; i < dim; i++) {
      IFETCH(&r[i], 0, TGSI_CHAN_X + i);
   }

   for (i = 0; i < 4; i++) {
      FETCH(&value[i], 1, TGSI_CHAN_X + i);
   }
   if (sample)
      IFETCH(&sample_r, 0, TGSI_CHAN_X + sample);

   for (j = 0; j < TGSI_QUAD_SIZE; j++) {
      rgba[0][j] = value[0].f[j];
      rgba[1][j] = value[1].f[j];
      rgba[2][j] = value[2].f[j];
      rgba[3][j] = value[3].f[j];
   }

   mach->Image->store(mach->Image, &params,
                      r[0].i, r[1].i, r[2].i, sample_r.i,
                      rgba);
}


static void
exec_store_membuf(struct tgsi_exec_machine *mach,
               const struct tgsi_full_instruction *inst)
{
   uint32_t unit = fetch_store_img_unit(mach, &inst->Dst[0]);
   uint32_t size;

   int execmask = mach->ExecMask & mach->NonHelperMask & ~mach->KillMask;

   const char *ptr;
   switch (inst->Dst[0].Register.File) {
   case TGSI_FILE_MEMORY:
      ptr = mach->LocalMem;
      size = mach->LocalMemSize;
      break;

   case TGSI_FILE_BUFFER:
      ptr = mach->Buffer->lookup(mach->Buffer, unit, &size);
      break;

   default:
      unreachable("unsupported TGSI_OPCODE_STORE file");
   }

   union tgsi_exec_channel offset;
   IFETCH(&offset, 0, TGSI_CHAN_X);

   union tgsi_exec_channel value[4];
   for (int i = 0; i < 4; i++)
      FETCH(&value[i], 1, TGSI_CHAN_X + i);

   for (int j = 0; j < TGSI_QUAD_SIZE; j++) {
      if (!(execmask & (1 << j)))
         continue;
      if (size < offset.u[j])
         continue;

      uint32_t *invocation_ptr = (uint32_t *)(ptr + offset.u[j]);
      uint32_t size_avail = size - offset.u[j];

      for (int chan = 0; chan < MIN2(4, size_avail / 4); chan++) {
         if (inst->Dst[0].Register.WriteMask & (1 << chan))
            memcpy(&invocation_ptr[chan], &value[chan].u[j], 4);
      }
   }
}

static void
exec_store(struct tgsi_exec_machine *mach,
           const struct tgsi_full_instruction *inst)
{
   if (inst->Dst[0].Register.File == TGSI_FILE_IMAGE)
      exec_store_img(mach, inst);
   else
      exec_store_membuf(mach, inst);
}

static void
exec_atomop_img(struct tgsi_exec_machine *mach,
                const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel r[4], sample_r;
   union tgsi_exec_channel value[4], value2[4];
   float rgba[TGSI_NUM_CHANNELS][TGSI_QUAD_SIZE];
   float rgba2[TGSI_NUM_CHANNELS][TGSI_QUAD_SIZE];
   struct tgsi_image_params params;
   int dim;
   int sample;
   int i, j;
   unsigned unit, chan;
   unit = fetch_sampler_unit(mach, inst, 0);
   dim = get_image_coord_dim(inst->Memory.Texture);
   sample = get_image_coord_sample(inst->Memory.Texture);
   assert(dim <= 3);

   params.execmask = mach->ExecMask & mach->NonHelperMask & ~mach->KillMask;
   params.unit = unit;
   params.tgsi_tex_instr = inst->Memory.Texture;
   params.format = inst->Memory.Format;

   for (i = 0; i < dim; i++) {
      IFETCH(&r[i], 1, TGSI_CHAN_X + i);
   }

   for (i = 0; i < 4; i++) {
      FETCH(&value[i], 2, TGSI_CHAN_X + i);
      if (inst->Instruction.Opcode == TGSI_OPCODE_ATOMCAS)
         FETCH(&value2[i], 3, TGSI_CHAN_X + i);
   }
   if (sample)
      IFETCH(&sample_r, 1, TGSI_CHAN_X + sample);

   for (j = 0; j < TGSI_QUAD_SIZE; j++) {
      rgba[0][j] = value[0].f[j];
      rgba[1][j] = value[1].f[j];
      rgba[2][j] = value[2].f[j];
      rgba[3][j] = value[3].f[j];
   }
   if (inst->Instruction.Opcode == TGSI_OPCODE_ATOMCAS) {
      for (j = 0; j < TGSI_QUAD_SIZE; j++) {
         rgba2[0][j] = value2[0].f[j];
         rgba2[1][j] = value2[1].f[j];
         rgba2[2][j] = value2[2].f[j];
         rgba2[3][j] = value2[3].f[j];
      }
   }

   mach->Image->op(mach->Image, &params, inst->Instruction.Opcode,
                   r[0].i, r[1].i, r[2].i, sample_r.i,
                   rgba, rgba2);

   for (j = 0; j < TGSI_QUAD_SIZE; j++) {
      r[0].f[j] = rgba[0][j];
      r[1].f[j] = rgba[1][j];
      r[2].f[j] = rgba[2][j];
      r[3].f[j] = rgba[3][j];
   }
   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &r[chan], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_atomop_membuf(struct tgsi_exec_machine *mach,
                   const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel offset, r0, r1;
   unsigned chan, i;
   int execmask = mach->ExecMask & mach->NonHelperMask & ~mach->KillMask;
   IFETCH(&offset, 1, TGSI_CHAN_X);

   if (!(inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X))
      return;

   void *ptr[TGSI_QUAD_SIZE];
   if (inst->Src[0].Register.File == TGSI_FILE_BUFFER) {
      uint32_t unit = fetch_sampler_unit(mach, inst, 0);
      uint32_t size;
      char *buffer = mach->Buffer->lookup(mach->Buffer, unit, &size);
      for (int i = 0; i < TGSI_QUAD_SIZE; i++) {
         if (likely(size >= 4 && offset.u[i] <= size - 4))
            ptr[i] = buffer + offset.u[i];
         else
            ptr[i] = NULL;
      }
   } else {
      assert(inst->Src[0].Register.File == TGSI_FILE_MEMORY);

      for (i = 0; i < TGSI_QUAD_SIZE; i++) {
         if (likely(mach->LocalMemSize >= 4 && offset.u[i] <= mach->LocalMemSize - 4))
            ptr[i] = (char *)mach->LocalMem + offset.u[i];
         else
            ptr[i] = NULL;
      }
   }

   FETCH(&r0, 2, TGSI_CHAN_X);
   if (inst->Instruction.Opcode == TGSI_OPCODE_ATOMCAS)
      FETCH(&r1, 3, TGSI_CHAN_X);

   /* The load/op/store sequence has to happen inside the loop since ptr
    * may have the same ptr in some of the invocations.
    */
   for (int i = 0; i < TGSI_QUAD_SIZE; i++) {
      if (!(execmask & (1 << i)))
         continue;

      uint32_t val = 0;
      if (ptr[i]) {
         memcpy(&val, ptr[i], sizeof(val));

         uint32_t result;
         switch (inst->Instruction.Opcode) {
         case TGSI_OPCODE_ATOMUADD:
            result = val + r0.u[i];
            break;
         case TGSI_OPCODE_ATOMXOR:
            result = val ^ r0.u[i];
            break;
         case TGSI_OPCODE_ATOMOR:
            result = val | r0.u[i];
            break;
         case TGSI_OPCODE_ATOMAND:
            result = val & r0.u[i];
            break;
         case TGSI_OPCODE_ATOMUMIN:
            result = MIN2(val, r0.u[i]);
            break;
         case TGSI_OPCODE_ATOMUMAX:
            result = MAX2(val, r0.u[i]);
            break;
         case TGSI_OPCODE_ATOMIMIN:
            result = MIN2((int32_t)val, r0.i[i]);
            break;
         case TGSI_OPCODE_ATOMIMAX:
            result = MAX2((int32_t)val, r0.i[i]);
            break;
         case TGSI_OPCODE_ATOMXCHG:
            result = r0.u[i];
            break;
         case TGSI_OPCODE_ATOMCAS:
            if (val == r0.u[i])
               result = r1.u[i];
            else
               result = val;
            break;
         case TGSI_OPCODE_ATOMFADD:
               result = fui(uif(val) + r0.f[i]);
            break;
         default:
            unreachable("bad atomic op");
         }
         memcpy(ptr[i], &result, sizeof(result));
      }

      r0.u[i] = val;
   }

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++)
      store_dest(mach, &r0, &inst->Dst[0], inst, chan);
}

static void
exec_atomop(struct tgsi_exec_machine *mach,
            const struct tgsi_full_instruction *inst)
{
   if (inst->Src[0].Register.File == TGSI_FILE_IMAGE)
      exec_atomop_img(mach, inst);
   else
      exec_atomop_membuf(mach, inst);
}

static void
exec_resq_img(struct tgsi_exec_machine *mach,
              const struct tgsi_full_instruction *inst)
{
   int result[4];
   union tgsi_exec_channel r[4];
   unsigned unit;
   int i, chan, j;
   struct tgsi_image_params params;

   unit = fetch_sampler_unit(mach, inst, 0);

   params.execmask = mach->ExecMask & mach->NonHelperMask & ~mach->KillMask;
   params.unit = unit;
   params.tgsi_tex_instr = inst->Memory.Texture;
   params.format = inst->Memory.Format;

   mach->Image->get_dims(mach->Image, &params, result);

   for (i = 0; i < TGSI_QUAD_SIZE; i++) {
      for (j = 0; j < 4; j++) {
         r[j].i[i] = result[j];
      }
   }

   for (chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (inst->Dst[0].Register.WriteMask & (1 << chan)) {
         store_dest(mach, &r[chan], &inst->Dst[0], inst, chan);
      }
   }
}

static void
exec_resq_buf(struct tgsi_exec_machine *mach,
              const struct tgsi_full_instruction *inst)
{
   uint32_t unit = fetch_sampler_unit(mach, inst, 0);
   uint32_t size;
   (void)mach->Buffer->lookup(mach->Buffer, unit, &size);

   union tgsi_exec_channel r;
   for (int i = 0; i < TGSI_QUAD_SIZE; i++)
      r.i[i] = size;

   if (inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_X) {
      for (int chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
         store_dest(mach, &r, &inst->Dst[0], inst, TGSI_CHAN_X);
      }
   }
}

static void
exec_resq(struct tgsi_exec_machine *mach,
          const struct tgsi_full_instruction *inst)
{
   if (inst->Src[0].Register.File == TGSI_FILE_IMAGE)
      exec_resq_img(mach, inst);
   else
      exec_resq_buf(mach, inst);
}

static void
micro_f2u64(union tgsi_double_channel *dst,
            const union tgsi_exec_channel *src)
{
   dst->u64[0] = (uint64_t)src->f[0];
   dst->u64[1] = (uint64_t)src->f[1];
   dst->u64[2] = (uint64_t)src->f[2];
   dst->u64[3] = (uint64_t)src->f[3];
}

static void
micro_f2i64(union tgsi_double_channel *dst,
            const union tgsi_exec_channel *src)
{
   dst->i64[0] = (int64_t)src->f[0];
   dst->i64[1] = (int64_t)src->f[1];
   dst->i64[2] = (int64_t)src->f[2];
   dst->i64[3] = (int64_t)src->f[3];
}

static void
micro_u2i64(union tgsi_double_channel *dst,
            const union tgsi_exec_channel *src)
{
   dst->u64[0] = (uint64_t)src->u[0];
   dst->u64[1] = (uint64_t)src->u[1];
   dst->u64[2] = (uint64_t)src->u[2];
   dst->u64[3] = (uint64_t)src->u[3];
}

static void
micro_i2i64(union tgsi_double_channel *dst,
            const union tgsi_exec_channel *src)
{
   dst->i64[0] = (int64_t)src->i[0];
   dst->i64[1] = (int64_t)src->i[1];
   dst->i64[2] = (int64_t)src->i[2];
   dst->i64[3] = (int64_t)src->i[3];
}

static void
micro_d2u64(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->u64[0] = (uint64_t)src->d[0];
   dst->u64[1] = (uint64_t)src->d[1];
   dst->u64[2] = (uint64_t)src->d[2];
   dst->u64[3] = (uint64_t)src->d[3];
}

static void
micro_d2i64(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->i64[0] = (int64_t)src->d[0];
   dst->i64[1] = (int64_t)src->d[1];
   dst->i64[2] = (int64_t)src->d[2];
   dst->i64[3] = (int64_t)src->d[3];
}

static void
micro_u642d(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = (double)src->u64[0];
   dst->d[1] = (double)src->u64[1];
   dst->d[2] = (double)src->u64[2];
   dst->d[3] = (double)src->u64[3];
}

static void
micro_i642d(union tgsi_double_channel *dst,
           const union tgsi_double_channel *src)
{
   dst->d[0] = (double)src->i64[0];
   dst->d[1] = (double)src->i64[1];
   dst->d[2] = (double)src->i64[2];
   dst->d[3] = (double)src->i64[3];
}

static void
micro_u642f(union tgsi_exec_channel *dst,
            const union tgsi_double_channel *src)
{
   dst->f[0] = (float)src->u64[0];
   dst->f[1] = (float)src->u64[1];
   dst->f[2] = (float)src->u64[2];
   dst->f[3] = (float)src->u64[3];
}

static void
micro_i642f(union tgsi_exec_channel *dst,
            const union tgsi_double_channel *src)
{
   dst->f[0] = (float)src->i64[0];
   dst->f[1] = (float)src->i64[1];
   dst->f[2] = (float)src->i64[2];
   dst->f[3] = (float)src->i64[3];
}

static void
exec_t_2_64(struct tgsi_exec_machine *mach,
          const struct tgsi_full_instruction *inst,
          micro_dop_s op,
          enum tgsi_exec_datatype src_datatype)
{
   union tgsi_exec_channel src;
   union tgsi_double_channel dst;

   if ((inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_XY) == TGSI_WRITEMASK_XY) {
      fetch_source(mach, &src, &inst->Src[0], TGSI_CHAN_X, src_datatype);
      op(&dst, &src);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_X, TGSI_CHAN_Y);
   }
   if ((inst->Dst[0].Register.WriteMask & TGSI_WRITEMASK_ZW) == TGSI_WRITEMASK_ZW) {
      fetch_source(mach, &src, &inst->Src[0], TGSI_CHAN_Y, src_datatype);
      op(&dst, &src);
      store_double_channel(mach, &dst, &inst->Dst[0], inst, TGSI_CHAN_Z, TGSI_CHAN_W);
   }
}

static void
exec_64_2_t(struct tgsi_exec_machine *mach,
            const struct tgsi_full_instruction *inst,
            micro_sop_d op)
{
   union tgsi_double_channel src;
   union tgsi_exec_channel dst;
   int wm = inst->Dst[0].Register.WriteMask;
   int i;
   int bit;
   for (i = 0; i < 2; i++) {
      bit = ffs(wm);
      if (bit) {
         wm &= ~(1 << (bit - 1));
         if (i == 0)
            fetch_double_channel(mach, &src, &inst->Src[0], TGSI_CHAN_X, TGSI_CHAN_Y);
         else
            fetch_double_channel(mach, &src, &inst->Src[0], TGSI_CHAN_Z, TGSI_CHAN_W);
         op(&dst, &src);
         store_dest(mach, &dst, &inst->Dst[0], inst, bit - 1);
      }
   }
}

static void
micro_i2f(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = (float)src->i[0];
   dst->f[1] = (float)src->i[1];
   dst->f[2] = (float)src->i[2];
   dst->f[3] = (float)src->i[3];
}

static void
micro_not(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->u[0] = ~src->u[0];
   dst->u[1] = ~src->u[1];
   dst->u[2] = ~src->u[2];
   dst->u[3] = ~src->u[3];
}

static void
micro_shl(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   unsigned masked_count;
   masked_count = src1->u[0] & 0x1f;
   dst->u[0] = src0->u[0] << masked_count;
   masked_count = src1->u[1] & 0x1f;
   dst->u[1] = src0->u[1] << masked_count;
   masked_count = src1->u[2] & 0x1f;
   dst->u[2] = src0->u[2] << masked_count;
   masked_count = src1->u[3] & 0x1f;
   dst->u[3] = src0->u[3] << masked_count;
}

static void
micro_and(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] & src1->u[0];
   dst->u[1] = src0->u[1] & src1->u[1];
   dst->u[2] = src0->u[2] & src1->u[2];
   dst->u[3] = src0->u[3] & src1->u[3];
}

static void
micro_or(union tgsi_exec_channel *dst,
         const union tgsi_exec_channel *src0,
         const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] | src1->u[0];
   dst->u[1] = src0->u[1] | src1->u[1];
   dst->u[2] = src0->u[2] | src1->u[2];
   dst->u[3] = src0->u[3] | src1->u[3];
}

static void
micro_xor(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] ^ src1->u[0];
   dst->u[1] = src0->u[1] ^ src1->u[1];
   dst->u[2] = src0->u[2] ^ src1->u[2];
   dst->u[3] = src0->u[3] ^ src1->u[3];
}

static void
micro_mod(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1)
{
   dst->i[0] = src1->i[0] ? src0->i[0] % src1->i[0] : ~0;
   dst->i[1] = src1->i[1] ? src0->i[1] % src1->i[1] : ~0;
   dst->i[2] = src1->i[2] ? src0->i[2] % src1->i[2] : ~0;
   dst->i[3] = src1->i[3] ? src0->i[3] % src1->i[3] : ~0;
}

static void
micro_f2i(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->i[0] = (int)src->f[0];
   dst->i[1] = (int)src->f[1];
   dst->i[2] = (int)src->f[2];
   dst->i[3] = (int)src->f[3];
}

static void
micro_fseq(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->f[0] == src1->f[0] ? ~0 : 0;
   dst->u[1] = src0->f[1] == src1->f[1] ? ~0 : 0;
   dst->u[2] = src0->f[2] == src1->f[2] ? ~0 : 0;
   dst->u[3] = src0->f[3] == src1->f[3] ? ~0 : 0;
}

static void
micro_fsge(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->f[0] >= src1->f[0] ? ~0 : 0;
   dst->u[1] = src0->f[1] >= src1->f[1] ? ~0 : 0;
   dst->u[2] = src0->f[2] >= src1->f[2] ? ~0 : 0;
   dst->u[3] = src0->f[3] >= src1->f[3] ? ~0 : 0;
}

static void
micro_fslt(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->f[0] < src1->f[0] ? ~0 : 0;
   dst->u[1] = src0->f[1] < src1->f[1] ? ~0 : 0;
   dst->u[2] = src0->f[2] < src1->f[2] ? ~0 : 0;
   dst->u[3] = src0->f[3] < src1->f[3] ? ~0 : 0;
}

static void
micro_fsne(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->f[0] != src1->f[0] ? ~0 : 0;
   dst->u[1] = src0->f[1] != src1->f[1] ? ~0 : 0;
   dst->u[2] = src0->f[2] != src1->f[2] ? ~0 : 0;
   dst->u[3] = src0->f[3] != src1->f[3] ? ~0 : 0;
}

static void
micro_idiv(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->i[0] = src1->i[0] ? src0->i[0] / src1->i[0] : 0;
   dst->i[1] = src1->i[1] ? src0->i[1] / src1->i[1] : 0;
   dst->i[2] = src1->i[2] ? src0->i[2] / src1->i[2] : 0;
   dst->i[3] = src1->i[3] ? src0->i[3] / src1->i[3] : 0;
}

static void
micro_imax(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->i[0] = src0->i[0] > src1->i[0] ? src0->i[0] : src1->i[0];
   dst->i[1] = src0->i[1] > src1->i[1] ? src0->i[1] : src1->i[1];
   dst->i[2] = src0->i[2] > src1->i[2] ? src0->i[2] : src1->i[2];
   dst->i[3] = src0->i[3] > src1->i[3] ? src0->i[3] : src1->i[3];
}

static void
micro_imin(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->i[0] = src0->i[0] < src1->i[0] ? src0->i[0] : src1->i[0];
   dst->i[1] = src0->i[1] < src1->i[1] ? src0->i[1] : src1->i[1];
   dst->i[2] = src0->i[2] < src1->i[2] ? src0->i[2] : src1->i[2];
   dst->i[3] = src0->i[3] < src1->i[3] ? src0->i[3] : src1->i[3];
}

static void
micro_isge(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->i[0] = src0->i[0] >= src1->i[0] ? -1 : 0;
   dst->i[1] = src0->i[1] >= src1->i[1] ? -1 : 0;
   dst->i[2] = src0->i[2] >= src1->i[2] ? -1 : 0;
   dst->i[3] = src0->i[3] >= src1->i[3] ? -1 : 0;
}

static void
micro_ishr(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   unsigned masked_count;
   masked_count = src1->i[0] & 0x1f;
   dst->i[0] = src0->i[0] >> masked_count;
   masked_count = src1->i[1] & 0x1f;
   dst->i[1] = src0->i[1] >> masked_count;
   masked_count = src1->i[2] & 0x1f;
   dst->i[2] = src0->i[2] >> masked_count;
   masked_count = src1->i[3] & 0x1f;
   dst->i[3] = src0->i[3] >> masked_count;
}

static void
micro_islt(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->i[0] = src0->i[0] < src1->i[0] ? -1 : 0;
   dst->i[1] = src0->i[1] < src1->i[1] ? -1 : 0;
   dst->i[2] = src0->i[2] < src1->i[2] ? -1 : 0;
   dst->i[3] = src0->i[3] < src1->i[3] ? -1 : 0;
}

static void
micro_f2u(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->u[0] = (uint32_t)src->f[0];
   dst->u[1] = (uint32_t)src->f[1];
   dst->u[2] = (uint32_t)src->f[2];
   dst->u[3] = (uint32_t)src->f[3];
}

static void
micro_u2f(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->f[0] = (float)src->u[0];
   dst->f[1] = (float)src->u[1];
   dst->f[2] = (float)src->u[2];
   dst->f[3] = (float)src->u[3];
}

static void
micro_uadd(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] + src1->u[0];
   dst->u[1] = src0->u[1] + src1->u[1];
   dst->u[2] = src0->u[2] + src1->u[2];
   dst->u[3] = src0->u[3] + src1->u[3];
}

static void
micro_udiv(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src1->u[0] ? src0->u[0] / src1->u[0] : ~0u;
   dst->u[1] = src1->u[1] ? src0->u[1] / src1->u[1] : ~0u;
   dst->u[2] = src1->u[2] ? src0->u[2] / src1->u[2] : ~0u;
   dst->u[3] = src1->u[3] ? src0->u[3] / src1->u[3] : ~0u;
}

static void
micro_umad(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1,
           const union tgsi_exec_channel *src2)
{
   dst->u[0] = src0->u[0] * src1->u[0] + src2->u[0];
   dst->u[1] = src0->u[1] * src1->u[1] + src2->u[1];
   dst->u[2] = src0->u[2] * src1->u[2] + src2->u[2];
   dst->u[3] = src0->u[3] * src1->u[3] + src2->u[3];
}

static void
micro_umax(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] > src1->u[0] ? src0->u[0] : src1->u[0];
   dst->u[1] = src0->u[1] > src1->u[1] ? src0->u[1] : src1->u[1];
   dst->u[2] = src0->u[2] > src1->u[2] ? src0->u[2] : src1->u[2];
   dst->u[3] = src0->u[3] > src1->u[3] ? src0->u[3] : src1->u[3];
}

static void
micro_umin(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] < src1->u[0] ? src0->u[0] : src1->u[0];
   dst->u[1] = src0->u[1] < src1->u[1] ? src0->u[1] : src1->u[1];
   dst->u[2] = src0->u[2] < src1->u[2] ? src0->u[2] : src1->u[2];
   dst->u[3] = src0->u[3] < src1->u[3] ? src0->u[3] : src1->u[3];
}

static void
micro_umod(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src1->u[0] ? src0->u[0] % src1->u[0] : ~0u;
   dst->u[1] = src1->u[1] ? src0->u[1] % src1->u[1] : ~0u;
   dst->u[2] = src1->u[2] ? src0->u[2] % src1->u[2] : ~0u;
   dst->u[3] = src1->u[3] ? src0->u[3] % src1->u[3] : ~0u;
}

static void
micro_umul(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] * src1->u[0];
   dst->u[1] = src0->u[1] * src1->u[1];
   dst->u[2] = src0->u[2] * src1->u[2];
   dst->u[3] = src0->u[3] * src1->u[3];
}

static void
micro_imul_hi(union tgsi_exec_channel *dst,
              const union tgsi_exec_channel *src0,
              const union tgsi_exec_channel *src1)
{
#define I64M(x, y) ((((int64_t)x) * ((int64_t)y)) >> 32)
   dst->i[0] = I64M(src0->i[0], src1->i[0]);
   dst->i[1] = I64M(src0->i[1], src1->i[1]);
   dst->i[2] = I64M(src0->i[2], src1->i[2]);
   dst->i[3] = I64M(src0->i[3], src1->i[3]);
#undef I64M
}

static void
micro_umul_hi(union tgsi_exec_channel *dst,
              const union tgsi_exec_channel *src0,
              const union tgsi_exec_channel *src1)
{
#define U64M(x, y) ((((uint64_t)x) * ((uint64_t)y)) >> 32)
   dst->u[0] = U64M(src0->u[0], src1->u[0]);
   dst->u[1] = U64M(src0->u[1], src1->u[1]);
   dst->u[2] = U64M(src0->u[2], src1->u[2]);
   dst->u[3] = U64M(src0->u[3], src1->u[3]);
#undef U64M
}

static void
micro_useq(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] == src1->u[0] ? ~0 : 0;
   dst->u[1] = src0->u[1] == src1->u[1] ? ~0 : 0;
   dst->u[2] = src0->u[2] == src1->u[2] ? ~0 : 0;
   dst->u[3] = src0->u[3] == src1->u[3] ? ~0 : 0;
}

static void
micro_usge(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] >= src1->u[0] ? ~0 : 0;
   dst->u[1] = src0->u[1] >= src1->u[1] ? ~0 : 0;
   dst->u[2] = src0->u[2] >= src1->u[2] ? ~0 : 0;
   dst->u[3] = src0->u[3] >= src1->u[3] ? ~0 : 0;
}

static void
micro_ushr(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   unsigned masked_count;
   masked_count = src1->u[0] & 0x1f;
   dst->u[0] = src0->u[0] >> masked_count;
   masked_count = src1->u[1] & 0x1f;
   dst->u[1] = src0->u[1] >> masked_count;
   masked_count = src1->u[2] & 0x1f;
   dst->u[2] = src0->u[2] >> masked_count;
   masked_count = src1->u[3] & 0x1f;
   dst->u[3] = src0->u[3] >> masked_count;
}

static void
micro_uslt(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] < src1->u[0] ? ~0 : 0;
   dst->u[1] = src0->u[1] < src1->u[1] ? ~0 : 0;
   dst->u[2] = src0->u[2] < src1->u[2] ? ~0 : 0;
   dst->u[3] = src0->u[3] < src1->u[3] ? ~0 : 0;
}

static void
micro_usne(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1)
{
   dst->u[0] = src0->u[0] != src1->u[0] ? ~0 : 0;
   dst->u[1] = src0->u[1] != src1->u[1] ? ~0 : 0;
   dst->u[2] = src0->u[2] != src1->u[2] ? ~0 : 0;
   dst->u[3] = src0->u[3] != src1->u[3] ? ~0 : 0;
}

static void
micro_uarl(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->i[0] = src->u[0];
   dst->i[1] = src->u[1];
   dst->i[2] = src->u[2];
   dst->i[3] = src->u[3];
}

/**
 * Signed bitfield extract (i.e. sign-extend the extracted bits)
 */
static void
micro_ibfe(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1,
           const union tgsi_exec_channel *src2)
{
   int i;
   for (i = 0; i < 4; i++) {
      int width = src2->i[i];
      int offset = src1->i[i] & 0x1f;
      if (width == 32 && offset == 0) {
         dst->i[i] = src0->i[i];
         continue;
      }
      width &= 0x1f;
      if (width == 0)
         dst->i[i] = 0;
      else if (width + offset < 32)
         dst->i[i] = (src0->i[i] << (32 - width - offset)) >> (32 - width);
      else
         dst->i[i] = src0->i[i] >> offset;
   }
}

/**
 * Unsigned bitfield extract
 */
static void
micro_ubfe(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src0,
           const union tgsi_exec_channel *src1,
           const union tgsi_exec_channel *src2)
{
   int i;
   for (i = 0; i < 4; i++) {
      int width = src2->u[i];
      int offset = src1->u[i] & 0x1f;
      if (width == 32 && offset == 0) {
         dst->u[i] = src0->u[i];
         continue;
      }
      width &= 0x1f;
      if (width == 0)
         dst->u[i] = 0;
      else if (width + offset < 32)
         dst->u[i] = (src0->u[i] << (32 - width - offset)) >> (32 - width);
      else
         dst->u[i] = src0->u[i] >> offset;
   }
}

/**
 * Bitfield insert: copy low bits from src1 into a region of src0.
 */
static void
micro_bfi(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src0,
          const union tgsi_exec_channel *src1,
          const union tgsi_exec_channel *src2,
          const union tgsi_exec_channel *src3)
{
   int i;
   for (i = 0; i < 4; i++) {
      int width = src3->u[i];
      int offset = src2->u[i] & 0x1f;
      if (width == 32) {
         dst->u[i] = src1->u[i];
      } else {
         int bitmask = ((1 << width) - 1) << offset;
         dst->u[i] = ((src1->u[i] << offset) & bitmask) | (src0->u[i] & ~bitmask);
      }
   }
}

static void
micro_brev(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->u[0] = util_bitreverse(src->u[0]);
   dst->u[1] = util_bitreverse(src->u[1]);
   dst->u[2] = util_bitreverse(src->u[2]);
   dst->u[3] = util_bitreverse(src->u[3]);
}

static void
micro_popc(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->u[0] = util_bitcount(src->u[0]);
   dst->u[1] = util_bitcount(src->u[1]);
   dst->u[2] = util_bitcount(src->u[2]);
   dst->u[3] = util_bitcount(src->u[3]);
}

static void
micro_lsb(union tgsi_exec_channel *dst,
          const union tgsi_exec_channel *src)
{
   dst->i[0] = ffs(src->u[0]) - 1;
   dst->i[1] = ffs(src->u[1]) - 1;
   dst->i[2] = ffs(src->u[2]) - 1;
   dst->i[3] = ffs(src->u[3]) - 1;
}

static void
micro_imsb(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->i[0] = util_last_bit_signed(src->i[0]) - 1;
   dst->i[1] = util_last_bit_signed(src->i[1]) - 1;
   dst->i[2] = util_last_bit_signed(src->i[2]) - 1;
   dst->i[3] = util_last_bit_signed(src->i[3]) - 1;
}

static void
micro_umsb(union tgsi_exec_channel *dst,
           const union tgsi_exec_channel *src)
{
   dst->i[0] = util_last_bit(src->u[0]) - 1;
   dst->i[1] = util_last_bit(src->u[1]) - 1;
   dst->i[2] = util_last_bit(src->u[2]) - 1;
   dst->i[3] = util_last_bit(src->u[3]) - 1;
}


static void
exec_interp_at_sample(struct tgsi_exec_machine *mach,
                      const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel index;
   union tgsi_exec_channel index2D;
   union tgsi_exec_channel result[TGSI_NUM_CHANNELS];
   const struct tgsi_full_src_register *reg = &inst->Src[0];

   assert(reg->Register.File == TGSI_FILE_INPUT);
   assert(inst->Src[1].Register.File == TGSI_FILE_IMMEDIATE);

   get_index_registers(mach, reg, &index, &index2D);
   float sample = mach->Imms[inst->Src[1].Register.Index][inst->Src[1].Register.SwizzleX];

   /* Short cut: sample 0 is like a normal fetch */
   for (unsigned chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (!(inst->Dst[0].Register.WriteMask & (1 << chan)))
         continue;

      fetch_src_file_channel(mach, TGSI_FILE_INPUT, chan, &index, &index2D,
                             &result[chan]);
      if (sample != 0.0f) {

      /* TODO: define the samples > 0, but so far we only do fake MSAA */
         float x = 0;
         float y = 0;

         unsigned pos = index2D.i[chan] * TGSI_EXEC_MAX_INPUT_ATTRIBS + index.i[chan];
         assert(pos >= 0);
         assert(pos < TGSI_MAX_PRIM_VERTICES * PIPE_MAX_ATTRIBS);
         mach->InputSampleOffsetApply[pos](mach, pos, chan, x, y, &result[chan]);
      }
      store_dest(mach, &result[chan], &inst->Dst[0], inst, chan);
   }
}


static void
exec_interp_at_offset(struct tgsi_exec_machine *mach,
                      const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel index;
   union tgsi_exec_channel index2D;
   union tgsi_exec_channel ofsx;
   union tgsi_exec_channel ofsy;
   const struct tgsi_full_src_register *reg = &inst->Src[0];

   assert(reg->Register.File == TGSI_FILE_INPUT);

   get_index_registers(mach, reg, &index, &index2D);
   unsigned pos = index2D.i[0] * TGSI_EXEC_MAX_INPUT_ATTRIBS + index.i[0];

   fetch_source(mach, &ofsx, &inst->Src[1], TGSI_CHAN_X, TGSI_EXEC_DATA_FLOAT);
   fetch_source(mach, &ofsy, &inst->Src[1], TGSI_CHAN_Y, TGSI_EXEC_DATA_FLOAT);

   for (int chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (!(inst->Dst[0].Register.WriteMask & (1 << chan)))
         continue;
      union tgsi_exec_channel result;
      fetch_src_file_channel(mach, TGSI_FILE_INPUT, chan, &index, &index2D, &result);
      mach->InputSampleOffsetApply[pos](mach, pos, chan, ofsx.f[chan], ofsy.f[chan], &result);
      store_dest(mach, &result, &inst->Dst[0], inst, chan);
   }
}


static void
exec_interp_at_centroid(struct tgsi_exec_machine *mach,
                        const struct tgsi_full_instruction *inst)
{
   union tgsi_exec_channel index;
   union tgsi_exec_channel index2D;
   union tgsi_exec_channel result[TGSI_NUM_CHANNELS];
   const struct tgsi_full_src_register *reg = &inst->Src[0];

   assert(reg->Register.File == TGSI_FILE_INPUT);
   get_index_registers(mach, reg, &index, &index2D);

   for (unsigned chan = 0; chan < TGSI_NUM_CHANNELS; chan++) {
      if (!(inst->Dst[0].Register.WriteMask & (1 << chan)))
         continue;

      /* Here we should add the change to use a sample that lies within the
       * primitive (Section 15.2):
       *
       * "When interpolating variables declared using centroid in ,
       * the variable is sampled at a location within the pixel covered
       * by the primitive generating the fragment.
       * ...
       * The built-in functions interpolateAtCentroid ... will sample
       * variables as though they were declared with the centroid ...
       * qualifier[s]."
       *
       * Since we only support 1 sample currently, this is just a pass-through.
       */
      fetch_src_file_channel(mach, TGSI_FILE_INPUT, chan, &index, &index2D,
                             &result[chan]);
      store_dest(mach, &result[chan], &inst->Dst[0], inst, chan);
   }

}


/**
 * Execute a TGSI instruction.
 * Returns TRUE if a barrier instruction is hit,
 * otherwise FALSE.
 */
static bool
exec_instruction(
   struct tgsi_exec_machine *mach,
   const struct tgsi_full_instruction *inst,
   int *pc )
{
   union tgsi_exec_channel r[10];

   (*pc)++;

   switch (inst->Instruction.Opcode) {
   case TGSI_OPCODE_ARL:
      exec_vector_unary(mach, inst, micro_arl, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_MOV:
      exec_vector_unary(mach, inst, micro_mov, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_LIT:
      exec_lit(mach, inst);
      break;

   case TGSI_OPCODE_RCP:
      exec_scalar_unary(mach, inst, micro_rcp, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_RSQ:
      exec_scalar_unary(mach, inst, micro_rsq, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_EXP:
      exec_exp(mach, inst);
      break;

   case TGSI_OPCODE_LOG:
      exec_log(mach, inst);
      break;

   case TGSI_OPCODE_MUL:
      exec_vector_binary(mach, inst, micro_mul, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_ADD:
      exec_vector_binary(mach, inst, micro_add, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_DP3:
      exec_dp3(mach, inst);
      break;

   case TGSI_OPCODE_DP4:
      exec_dp4(mach, inst);
      break;

   case TGSI_OPCODE_DST:
      exec_dst(mach, inst);
      break;

   case TGSI_OPCODE_MIN:
      exec_vector_binary(mach, inst, micro_min, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_MAX:
      exec_vector_binary(mach, inst, micro_max, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_SLT:
      exec_vector_binary(mach, inst, micro_slt, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_SGE:
      exec_vector_binary(mach, inst, micro_sge, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_MAD:
      exec_vector_trinary(mach, inst, micro_mad, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_LRP:
      exec_vector_trinary(mach, inst, micro_lrp, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_SQRT:
      exec_scalar_unary(mach, inst, micro_sqrt, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_FRC:
      exec_vector_unary(mach, inst, micro_frc, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_FLR:
      exec_vector_unary(mach, inst, micro_flr, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_ROUND:
      exec_vector_unary(mach, inst, micro_rnd, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_EX2:
      exec_scalar_unary(mach, inst, micro_exp2, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_LG2:
      exec_scalar_unary(mach, inst, micro_lg2, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_POW:
      exec_scalar_binary(mach, inst, micro_pow, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_LDEXP:
      exec_vector_binary(mach, inst, micro_ldexp, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_COS:
      exec_scalar_unary(mach, inst, micro_cos, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_DDX_FINE:
      exec_vector_unary(mach, inst, micro_ddx_fine, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_DDX:
      exec_vector_unary(mach, inst, micro_ddx, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_DDY_FINE:
      exec_vector_unary(mach, inst, micro_ddy_fine, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_DDY:
      exec_vector_unary(mach, inst, micro_ddy, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_KILL:
      exec_kill (mach);
      break;

   case TGSI_OPCODE_KILL_IF:
      exec_kill_if (mach, inst);
      break;

   case TGSI_OPCODE_PK2H:
      exec_pk2h(mach, inst);
      break;

   case TGSI_OPCODE_PK2US:
      assert (0);
      break;

   case TGSI_OPCODE_PK4B:
      assert (0);
      break;

   case TGSI_OPCODE_PK4UB:
      assert (0);
      break;

   case TGSI_OPCODE_SEQ:
      exec_vector_binary(mach, inst, micro_seq, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_SGT:
      exec_vector_binary(mach, inst, micro_sgt, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_SIN:
      exec_scalar_unary(mach, inst, micro_sin, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_SLE:
      exec_vector_binary(mach, inst, micro_sle, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_SNE:
      exec_vector_binary(mach, inst, micro_sne, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_TEX:
      /* simple texture lookup */
      /* src[0] = texcoord */
      /* src[1] = sampler unit */
      exec_tex(mach, inst, TEX_MODIFIER_NONE, 1);
      break;

   case TGSI_OPCODE_TXB:
      /* Texture lookup with lod bias */
      /* src[0] = texcoord (src[0].w = LOD bias) */
      /* src[1] = sampler unit */
      exec_tex(mach, inst, TEX_MODIFIER_LOD_BIAS, 1);
      break;

   case TGSI_OPCODE_TXD:
      /* Texture lookup with explict partial derivatives */
      /* src[0] = texcoord */
      /* src[1] = d[strq]/dx */
      /* src[2] = d[strq]/dy */
      /* src[3] = sampler unit */
      exec_txd(mach, inst);
      break;

   case TGSI_OPCODE_TXL:
      /* Texture lookup with explit LOD */
      /* src[0] = texcoord (src[0].w = LOD) */
      /* src[1] = sampler unit */
      exec_tex(mach, inst, TEX_MODIFIER_EXPLICIT_LOD, 1);
      break;

   case TGSI_OPCODE_TXP:
      /* Texture lookup with projection */
      /* src[0] = texcoord (src[0].w = projection) */
      /* src[1] = sampler unit */
      exec_tex(mach, inst, TEX_MODIFIER_PROJECTED, 1);
      break;

   case TGSI_OPCODE_TG4:
      /* src[0] = texcoord */
      /* src[1] = component */
      /* src[2] = sampler unit */
      exec_tex(mach, inst, TEX_MODIFIER_GATHER, 2);
      break;

   case TGSI_OPCODE_LODQ:
      /* src[0] = texcoord */
      /* src[1] = sampler unit */
      exec_lodq(mach, inst);
      break;

   case TGSI_OPCODE_UP2H:
      exec_up2h(mach, inst);
      break;

   case TGSI_OPCODE_UP2US:
      assert (0);
      break;

   case TGSI_OPCODE_UP4B:
      assert (0);
      break;

   case TGSI_OPCODE_UP4UB:
      assert (0);
      break;

   case TGSI_OPCODE_ARR:
      exec_vector_unary(mach, inst, micro_arr, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_CAL:
      /* skip the call if no execution channels are enabled */
      if (mach->ExecMask) {
         /* do the call */

         /* First, record the depths of the execution stacks.
          * This is important for deeply nested/looped return statements.
          * We have to unwind the stacks by the correct amount.  For a
          * real code generator, we could determine the number of entries
          * to pop off each stack with simple static analysis and avoid
          * implementing this data structure at run time.
          */
         mach->CallStack[mach->CallStackTop].CondStackTop = mach->CondStackTop;
         mach->CallStack[mach->CallStackTop].LoopStackTop = mach->LoopStackTop;
         mach->CallStack[mach->CallStackTop].ContStackTop = mach->ContStackTop;
         mach->CallStack[mach->CallStackTop].SwitchStackTop = mach->SwitchStackTop;
         mach->CallStack[mach->CallStackTop].BreakStackTop = mach->BreakStackTop;
         /* note that PC was already incremented above */
         mach->CallStack[mach->CallStackTop].ReturnAddr = *pc;

         mach->CallStackTop++;

         /* Second, push the Cond, Loop, Cont, Func stacks */
         assert(mach->CondStackTop < TGSI_EXEC_MAX_COND_NESTING);
         assert(mach->LoopStackTop < TGSI_EXEC_MAX_LOOP_NESTING);
         assert(mach->ContStackTop < TGSI_EXEC_MAX_LOOP_NESTING);
         assert(mach->SwitchStackTop < TGSI_EXEC_MAX_SWITCH_NESTING);
         assert(mach->BreakStackTop < TGSI_EXEC_MAX_BREAK_STACK);
         assert(mach->FuncStackTop < TGSI_EXEC_MAX_CALL_NESTING);

         mach->CondStack[mach->CondStackTop++] = mach->CondMask;
         mach->LoopStack[mach->LoopStackTop++] = mach->LoopMask;
         mach->ContStack[mach->ContStackTop++] = mach->ContMask;
         mach->SwitchStack[mach->SwitchStackTop++] = mach->Switch;
         mach->BreakStack[mach->BreakStackTop++] = mach->BreakType;
         mach->FuncStack[mach->FuncStackTop++] = mach->FuncMask;

         /* Finally, jump to the subroutine.  The label is a pointer
          * (an instruction number) to the BGNSUB instruction.
          */
         *pc = inst->Label.Label;
         assert(mach->Instructions[*pc].Instruction.Opcode
                == TGSI_OPCODE_BGNSUB);
      }
      break;

   case TGSI_OPCODE_RET:
      mach->FuncMask &= ~mach->ExecMask;
      UPDATE_EXEC_MASK(mach);

      if (mach->FuncMask == 0x0) {
         /* really return now (otherwise, keep executing */

         if (mach->CallStackTop == 0) {
            /* returning from main() */
            mach->CondStackTop = 0;
            mach->LoopStackTop = 0;
            mach->ContStackTop = 0;
            mach->LoopLabelStackTop = 0;
            mach->SwitchStackTop = 0;
            mach->BreakStackTop = 0;
            *pc = -1;
            return false;
         }

         assert(mach->CallStackTop > 0);
         mach->CallStackTop--;

         mach->CondStackTop = mach->CallStack[mach->CallStackTop].CondStackTop;
         mach->CondMask = mach->CondStack[mach->CondStackTop];

         mach->LoopStackTop = mach->CallStack[mach->CallStackTop].LoopStackTop;
         mach->LoopMask = mach->LoopStack[mach->LoopStackTop];

         mach->ContStackTop = mach->CallStack[mach->CallStackTop].ContStackTop;
         mach->ContMask = mach->ContStack[mach->ContStackTop];

         mach->SwitchStackTop = mach->CallStack[mach->CallStackTop].SwitchStackTop;
         mach->Switch = mach->SwitchStack[mach->SwitchStackTop];

         mach->BreakStackTop = mach->CallStack[mach->CallStackTop].BreakStackTop;
         mach->BreakType = mach->BreakStack[mach->BreakStackTop];

         assert(mach->FuncStackTop > 0);
         mach->FuncMask = mach->FuncStack[--mach->FuncStackTop];

         *pc = mach->CallStack[mach->CallStackTop].ReturnAddr;

         UPDATE_EXEC_MASK(mach);
      }
      break;

   case TGSI_OPCODE_SSG:
      exec_vector_unary(mach, inst, micro_sgn, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_CMP:
      exec_vector_trinary(mach, inst, micro_cmp, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_DIV:
      exec_vector_binary(mach, inst, micro_div, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_DP2:
      exec_dp2(mach, inst);
      break;

   case TGSI_OPCODE_IF:
      /* push CondMask */
      assert(mach->CondStackTop < TGSI_EXEC_MAX_COND_NESTING);
      mach->CondStack[mach->CondStackTop++] = mach->CondMask;
      FETCH( &r[0], 0, TGSI_CHAN_X );
      for (int i = 0; i < TGSI_QUAD_SIZE; i++) {
         if (!r[0].f[i])
            mach->CondMask &= ~(1 << i);
      }
      UPDATE_EXEC_MASK(mach);
      /* If no channels are taking the then branch, jump to ELSE. */
      if (!mach->CondMask)
         *pc = inst->Label.Label;
      break;

   case TGSI_OPCODE_UIF:
      /* push CondMask */
      assert(mach->CondStackTop < TGSI_EXEC_MAX_COND_NESTING);
      mach->CondStack[mach->CondStackTop++] = mach->CondMask;
      IFETCH( &r[0], 0, TGSI_CHAN_X );
      for (int i = 0; i < TGSI_QUAD_SIZE; i++) {
         if (!r[0].u[i])
            mach->CondMask &= ~(1 << i);
      }
      UPDATE_EXEC_MASK(mach);
      /* If no channels are taking the then branch, jump to ELSE. */
      if (!mach->CondMask)
         *pc = inst->Label.Label;
      break;

   case TGSI_OPCODE_ELSE:
      /* invert CondMask wrt previous mask */
      {
         unsigned prevMask;
         assert(mach->CondStackTop > 0);
         prevMask = mach->CondStack[mach->CondStackTop - 1];
         mach->CondMask = ~mach->CondMask & prevMask;
         UPDATE_EXEC_MASK(mach);

         /* If no channels are taking ELSE, jump to ENDIF */
         if (!mach->CondMask)
            *pc = inst->Label.Label;
      }
      break;

   case TGSI_OPCODE_ENDIF:
      /* pop CondMask */
      assert(mach->CondStackTop > 0);
      mach->CondMask = mach->CondStack[--mach->CondStackTop];
      UPDATE_EXEC_MASK(mach);
      break;

   case TGSI_OPCODE_END:
      /* make sure we end primitives which haven't
       * been explicitly emitted */
      conditional_emit_primitive(mach);
      /* halt execution */
      *pc = -1;
      break;

   case TGSI_OPCODE_CEIL:
      exec_vector_unary(mach, inst, micro_ceil, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_I2F:
      exec_vector_unary(mach, inst, micro_i2f, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_NOT:
      exec_vector_unary(mach, inst, micro_not, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_TRUNC:
      exec_vector_unary(mach, inst, micro_trunc, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_SHL:
      exec_vector_binary(mach, inst, micro_shl, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_AND:
      exec_vector_binary(mach, inst, micro_and, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_OR:
      exec_vector_binary(mach, inst, micro_or, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_MOD:
      exec_vector_binary(mach, inst, micro_mod, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_XOR:
      exec_vector_binary(mach, inst, micro_xor, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_TXF:
      exec_txf(mach, inst);
      break;

   case TGSI_OPCODE_TXQ:
      exec_txq(mach, inst);
      break;

   case TGSI_OPCODE_EMIT:
      emit_vertex(mach, inst);
      break;

   case TGSI_OPCODE_ENDPRIM:
      emit_primitive(mach, inst);
      break;

   case TGSI_OPCODE_BGNLOOP:
      /* push LoopMask and ContMasks */
      assert(mach->LoopStackTop < TGSI_EXEC_MAX_LOOP_NESTING);
      assert(mach->ContStackTop < TGSI_EXEC_MAX_LOOP_NESTING);
      assert(mach->LoopLabelStackTop < TGSI_EXEC_MAX_LOOP_NESTING);
      assert(mach->BreakStackTop < TGSI_EXEC_MAX_BREAK_STACK);

      mach->LoopStack[mach->LoopStackTop++] = mach->LoopMask;
      mach->ContStack[mach->ContStackTop++] = mach->ContMask;
      mach->LoopLabelStack[mach->LoopLabelStackTop++] = *pc - 1;
      mach->BreakStack[mach->BreakStackTop++] = mach->BreakType;
      mach->BreakType = TGSI_EXEC_BREAK_INSIDE_LOOP;
      break;

   case TGSI_OPCODE_ENDLOOP:
      /* Restore ContMask, but don't pop */
      assert(mach->ContStackTop > 0);
      mach->ContMask = mach->ContStack[mach->ContStackTop - 1];
      UPDATE_EXEC_MASK(mach);
      if (mach->ExecMask) {
         /* repeat loop: jump to instruction just past BGNLOOP */
         assert(mach->LoopLabelStackTop > 0);
         *pc = mach->LoopLabelStack[mach->LoopLabelStackTop - 1] + 1;
      }
      else {
         /* exit loop: pop LoopMask */
         assert(mach->LoopStackTop > 0);
         mach->LoopMask = mach->LoopStack[--mach->LoopStackTop];
         /* pop ContMask */
         assert(mach->ContStackTop > 0);
         mach->ContMask = mach->ContStack[--mach->ContStackTop];
         assert(mach->LoopLabelStackTop > 0);
         --mach->LoopLabelStackTop;

         mach->BreakType = mach->BreakStack[--mach->BreakStackTop];
      }
      UPDATE_EXEC_MASK(mach);
      break;

   case TGSI_OPCODE_BRK:
      exec_break(mach);
      break;

   case TGSI_OPCODE_CONT:
      /* turn off cont channels for each enabled exec channel */
      mach->ContMask &= ~mach->ExecMask;
      /* Todo: if mach->LoopMask == 0, jump to end of loop */
      UPDATE_EXEC_MASK(mach);
      break;

   case TGSI_OPCODE_BGNSUB:
      /* no-op */
      break;

   case TGSI_OPCODE_ENDSUB:
      /*
       * XXX: This really should be a no-op. We should never reach this opcode.
       */

      assert(mach->CallStackTop > 0);
      mach->CallStackTop--;

      mach->CondStackTop = mach->CallStack[mach->CallStackTop].CondStackTop;
      mach->CondMask = mach->CondStack[mach->CondStackTop];

      mach->LoopStackTop = mach->CallStack[mach->CallStackTop].LoopStackTop;
      mach->LoopMask = mach->LoopStack[mach->LoopStackTop];

      mach->ContStackTop = mach->CallStack[mach->CallStackTop].ContStackTop;
      mach->ContMask = mach->ContStack[mach->ContStackTop];

      mach->SwitchStackTop = mach->CallStack[mach->CallStackTop].SwitchStackTop;
      mach->Switch = mach->SwitchStack[mach->SwitchStackTop];

      mach->BreakStackTop = mach->CallStack[mach->CallStackTop].BreakStackTop;
      mach->BreakType = mach->BreakStack[mach->BreakStackTop];

      assert(mach->FuncStackTop > 0);
      mach->FuncMask = mach->FuncStack[--mach->FuncStackTop];

      *pc = mach->CallStack[mach->CallStackTop].ReturnAddr;

      UPDATE_EXEC_MASK(mach);
      break;

   case TGSI_OPCODE_NOP:
      break;

   case TGSI_OPCODE_F2I:
      exec_vector_unary(mach, inst, micro_f2i, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_FSEQ:
      exec_vector_binary(mach, inst, micro_fseq, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_FSGE:
      exec_vector_binary(mach, inst, micro_fsge, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_FSLT:
      exec_vector_binary(mach, inst, micro_fslt, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_FSNE:
      exec_vector_binary(mach, inst, micro_fsne, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_IDIV:
      exec_vector_binary(mach, inst, micro_idiv, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_IMAX:
      exec_vector_binary(mach, inst, micro_imax, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_IMIN:
      exec_vector_binary(mach, inst, micro_imin, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_INEG:
      exec_vector_unary(mach, inst, micro_ineg, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_ISGE:
      exec_vector_binary(mach, inst, micro_isge, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_ISHR:
      exec_vector_binary(mach, inst, micro_ishr, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_ISLT:
      exec_vector_binary(mach, inst, micro_islt, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_F2U:
      exec_vector_unary(mach, inst, micro_f2u, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_U2F:
      exec_vector_unary(mach, inst, micro_u2f, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_UADD:
      exec_vector_binary(mach, inst, micro_uadd, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_UDIV:
      exec_vector_binary(mach, inst, micro_udiv, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_UMAD:
      exec_vector_trinary(mach, inst, micro_umad, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_UMAX:
      exec_vector_binary(mach, inst, micro_umax, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_UMIN:
      exec_vector_binary(mach, inst, micro_umin, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_UMOD:
      exec_vector_binary(mach, inst, micro_umod, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_UMUL:
      exec_vector_binary(mach, inst, micro_umul, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_IMUL_HI:
      exec_vector_binary(mach, inst, micro_imul_hi, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_UMUL_HI:
      exec_vector_binary(mach, inst, micro_umul_hi, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_USEQ:
      exec_vector_binary(mach, inst, micro_useq, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_USGE:
      exec_vector_binary(mach, inst, micro_usge, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_USHR:
      exec_vector_binary(mach, inst, micro_ushr, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_USLT:
      exec_vector_binary(mach, inst, micro_uslt, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_USNE:
      exec_vector_binary(mach, inst, micro_usne, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_SWITCH:
      exec_switch(mach, inst);
      break;

   case TGSI_OPCODE_CASE:
      exec_case(mach, inst);
      break;

   case TGSI_OPCODE_DEFAULT:
      exec_default(mach);
      break;

   case TGSI_OPCODE_ENDSWITCH:
      exec_endswitch(mach);
      break;

   case TGSI_OPCODE_SAMPLE_I:
      exec_txf(mach, inst);
      break;

   case TGSI_OPCODE_SAMPLE_I_MS:
      exec_txf(mach, inst);
      break;

   case TGSI_OPCODE_SAMPLE:
      exec_sample(mach, inst, TEX_MODIFIER_NONE, false);
      break;

   case TGSI_OPCODE_SAMPLE_B:
      exec_sample(mach, inst, TEX_MODIFIER_LOD_BIAS, false);
      break;

   case TGSI_OPCODE_SAMPLE_C:
      exec_sample(mach, inst, TEX_MODIFIER_NONE, true);
      break;

   case TGSI_OPCODE_SAMPLE_C_LZ:
      exec_sample(mach, inst, TEX_MODIFIER_LEVEL_ZERO, true);
      break;

   case TGSI_OPCODE_SAMPLE_D:
      exec_sample_d(mach, inst);
      break;

   case TGSI_OPCODE_SAMPLE_L:
      exec_sample(mach, inst, TEX_MODIFIER_EXPLICIT_LOD, false);
      break;

   case TGSI_OPCODE_GATHER4:
      exec_sample(mach, inst, TEX_MODIFIER_GATHER, false);
      break;

   case TGSI_OPCODE_SVIEWINFO:
      exec_txq(mach, inst);
      break;

   case TGSI_OPCODE_SAMPLE_POS:
      assert(0);
      break;

   case TGSI_OPCODE_SAMPLE_INFO:
      assert(0);
      break;

   case TGSI_OPCODE_LOD:
      exec_lodq(mach, inst);
      break;

   case TGSI_OPCODE_UARL:
      exec_vector_unary(mach, inst, micro_uarl, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_UCMP:
      exec_ucmp(mach, inst);
      break;

   case TGSI_OPCODE_IABS:
      exec_vector_unary(mach, inst, micro_iabs, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_ISSG:
      exec_vector_unary(mach, inst, micro_isgn, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_TEX2:
      /* simple texture lookup */
      /* src[0] = texcoord */
      /* src[1] = compare */
      /* src[2] = sampler unit */
      exec_tex(mach, inst, TEX_MODIFIER_NONE, 2);
      break;
   case TGSI_OPCODE_TXB2:
      /* simple texture lookup */
      /* src[0] = texcoord */
      /* src[1] = bias */
      /* src[2] = sampler unit */
      exec_tex(mach, inst, TEX_MODIFIER_LOD_BIAS, 2);
      break;
   case TGSI_OPCODE_TXL2:
      /* simple texture lookup */
      /* src[0] = texcoord */
      /* src[1] = lod */
      /* src[2] = sampler unit */
      exec_tex(mach, inst, TEX_MODIFIER_EXPLICIT_LOD, 2);
      break;

   case TGSI_OPCODE_IBFE:
      exec_vector_trinary(mach, inst, micro_ibfe, TGSI_EXEC_DATA_INT);
      break;
   case TGSI_OPCODE_UBFE:
      exec_vector_trinary(mach, inst, micro_ubfe, TGSI_EXEC_DATA_UINT);
      break;
   case TGSI_OPCODE_BFI:
      exec_vector_quaternary(mach, inst, micro_bfi, TGSI_EXEC_DATA_UINT);
      break;
   case TGSI_OPCODE_BREV:
      exec_vector_unary(mach, inst, micro_brev, TGSI_EXEC_DATA_UINT);
      break;
   case TGSI_OPCODE_POPC:
      exec_vector_unary(mach, inst, micro_popc, TGSI_EXEC_DATA_UINT);
      break;
   case TGSI_OPCODE_LSB:
      exec_vector_unary(mach, inst, micro_lsb, TGSI_EXEC_DATA_UINT);
      break;
   case TGSI_OPCODE_IMSB:
      exec_vector_unary(mach, inst, micro_imsb, TGSI_EXEC_DATA_INT);
      break;
   case TGSI_OPCODE_UMSB:
      exec_vector_unary(mach, inst, micro_umsb, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_F2D:
      exec_t_2_64(mach, inst, micro_f2d, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_D2F:
      exec_64_2_t(mach, inst, micro_d2f);
      break;

   case TGSI_OPCODE_DABS:
      exec_double_unary(mach, inst, micro_dabs);
      break;

   case TGSI_OPCODE_DNEG:
      exec_double_unary(mach, inst, micro_dneg);
      break;

   case TGSI_OPCODE_DADD:
      exec_double_binary(mach, inst, micro_dadd, TGSI_EXEC_DATA_DOUBLE);
      break;

   case TGSI_OPCODE_DDIV:
      exec_double_binary(mach, inst, micro_ddiv, TGSI_EXEC_DATA_DOUBLE);
      break;

   case TGSI_OPCODE_DMUL:
      exec_double_binary(mach, inst, micro_dmul, TGSI_EXEC_DATA_DOUBLE);
      break;

   case TGSI_OPCODE_DMAX:
      exec_double_binary(mach, inst, micro_dmax, TGSI_EXEC_DATA_DOUBLE);
      break;

   case TGSI_OPCODE_DMIN:
      exec_double_binary(mach, inst, micro_dmin, TGSI_EXEC_DATA_DOUBLE);
      break;

   case TGSI_OPCODE_DSLT:
      exec_double_binary(mach, inst, micro_dslt, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_DSGE:
      exec_double_binary(mach, inst, micro_dsge, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_DSEQ:
      exec_double_binary(mach, inst, micro_dseq, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_DSNE:
      exec_double_binary(mach, inst, micro_dsne, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_DRCP:
      exec_double_unary(mach, inst, micro_drcp);
      break;

   case TGSI_OPCODE_DSQRT:
      exec_double_unary(mach, inst, micro_dsqrt);
      break;

   case TGSI_OPCODE_DRSQ:
      exec_double_unary(mach, inst, micro_drsq);
      break;

   case TGSI_OPCODE_DMAD:
      exec_double_trinary(mach, inst, micro_dmad);
      break;

   case TGSI_OPCODE_DFRAC:
      exec_double_unary(mach, inst, micro_dfrac);
      break;

   case TGSI_OPCODE_DFLR:
      exec_double_unary(mach, inst, micro_dflr);
      break;

   case TGSI_OPCODE_DLDEXP:
      exec_dldexp(mach, inst);
      break;

   case TGSI_OPCODE_I2D:
      exec_t_2_64(mach, inst, micro_i2d, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_D2I:
      exec_64_2_t(mach, inst, micro_d2i);
      break;

   case TGSI_OPCODE_U2D:
      exec_t_2_64(mach, inst, micro_u2d, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_D2U:
      exec_64_2_t(mach, inst, micro_d2u);
      break;

   case TGSI_OPCODE_LOAD:
      exec_load(mach, inst);
      break;

   case TGSI_OPCODE_STORE:
      exec_store(mach, inst);
      break;

   case TGSI_OPCODE_ATOMUADD:
   case TGSI_OPCODE_ATOMXCHG:
   case TGSI_OPCODE_ATOMCAS:
   case TGSI_OPCODE_ATOMAND:
   case TGSI_OPCODE_ATOMOR:
   case TGSI_OPCODE_ATOMXOR:
   case TGSI_OPCODE_ATOMUMIN:
   case TGSI_OPCODE_ATOMUMAX:
   case TGSI_OPCODE_ATOMIMIN:
   case TGSI_OPCODE_ATOMIMAX:
   case TGSI_OPCODE_ATOMFADD:
      exec_atomop(mach, inst);
      break;

   case TGSI_OPCODE_RESQ:
      exec_resq(mach, inst);
      break;
   case TGSI_OPCODE_BARRIER:
   case TGSI_OPCODE_MEMBAR:
      return true;
      break;

   case TGSI_OPCODE_I64ABS:
      exec_double_unary(mach, inst, micro_i64abs);
      break;

   case TGSI_OPCODE_I64SSG:
      exec_double_unary(mach, inst, micro_i64sgn);
      break;

   case TGSI_OPCODE_I64NEG:
      exec_double_unary(mach, inst, micro_i64neg);
      break;

   case TGSI_OPCODE_U64SEQ:
      exec_double_binary(mach, inst, micro_u64seq, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_U64SNE:
      exec_double_binary(mach, inst, micro_u64sne, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_I64SLT:
      exec_double_binary(mach, inst, micro_i64slt, TGSI_EXEC_DATA_UINT);
      break;
   case TGSI_OPCODE_U64SLT:
      exec_double_binary(mach, inst, micro_u64slt, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_I64SGE:
      exec_double_binary(mach, inst, micro_i64sge, TGSI_EXEC_DATA_UINT);
      break;
   case TGSI_OPCODE_U64SGE:
      exec_double_binary(mach, inst, micro_u64sge, TGSI_EXEC_DATA_UINT);
      break;

   case TGSI_OPCODE_I64MIN:
      exec_double_binary(mach, inst, micro_i64min, TGSI_EXEC_DATA_INT64);
      break;
   case TGSI_OPCODE_U64MIN:
      exec_double_binary(mach, inst, micro_u64min, TGSI_EXEC_DATA_UINT64);
      break;
   case TGSI_OPCODE_I64MAX:
      exec_double_binary(mach, inst, micro_i64max, TGSI_EXEC_DATA_INT64);
      break;
   case TGSI_OPCODE_U64MAX:
      exec_double_binary(mach, inst, micro_u64max, TGSI_EXEC_DATA_UINT64);
      break;
   case TGSI_OPCODE_U64ADD:
      exec_double_binary(mach, inst, micro_u64add, TGSI_EXEC_DATA_UINT64);
      break;
   case TGSI_OPCODE_U64MUL:
      exec_double_binary(mach, inst, micro_u64mul, TGSI_EXEC_DATA_UINT64);
      break;
   case TGSI_OPCODE_U64SHL:
      exec_arg0_64_arg1_32(mach, inst, micro_u64shl);
      break;
   case TGSI_OPCODE_I64SHR:
      exec_arg0_64_arg1_32(mach, inst, micro_i64shr);
      break;
   case TGSI_OPCODE_U64SHR:
      exec_arg0_64_arg1_32(mach, inst, micro_u64shr);
      break;
   case TGSI_OPCODE_U64DIV:
      exec_double_binary(mach, inst, micro_u64div, TGSI_EXEC_DATA_UINT64);
      break;
   case TGSI_OPCODE_I64DIV:
      exec_double_binary(mach, inst, micro_i64div, TGSI_EXEC_DATA_INT64);
      break;
   case TGSI_OPCODE_U64MOD:
      exec_double_binary(mach, inst, micro_u64mod, TGSI_EXEC_DATA_UINT64);
      break;
   case TGSI_OPCODE_I64MOD:
      exec_double_binary(mach, inst, micro_i64mod, TGSI_EXEC_DATA_INT64);
      break;

   case TGSI_OPCODE_F2U64:
      exec_t_2_64(mach, inst, micro_f2u64, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_F2I64:
      exec_t_2_64(mach, inst, micro_f2i64, TGSI_EXEC_DATA_FLOAT);
      break;

   case TGSI_OPCODE_U2I64:
      exec_t_2_64(mach, inst, micro_u2i64, TGSI_EXEC_DATA_INT);
      break;
   case TGSI_OPCODE_I2I64:
      exec_t_2_64(mach, inst, micro_i2i64, TGSI_EXEC_DATA_INT);
      break;

   case TGSI_OPCODE_D2U64:
      exec_double_unary(mach, inst, micro_d2u64);
      break;

   case TGSI_OPCODE_D2I64:
      exec_double_unary(mach, inst, micro_d2i64);
      break;

   case TGSI_OPCODE_U642F:
      exec_64_2_t(mach, inst, micro_u642f);
      break;
   case TGSI_OPCODE_I642F:
      exec_64_2_t(mach, inst, micro_i642f);
      break;

   case TGSI_OPCODE_U642D:
      exec_double_unary(mach, inst, micro_u642d);
      break;
   case TGSI_OPCODE_I642D:
      exec_double_unary(mach, inst, micro_i642d);
      break;
   case TGSI_OPCODE_INTERP_SAMPLE:
      exec_interp_at_sample(mach, inst);
      break;
   case TGSI_OPCODE_INTERP_OFFSET:
      exec_interp_at_offset(mach, inst);
      break;
   case TGSI_OPCODE_INTERP_CENTROID:
      exec_interp_at_centroid(mach, inst);
      break;
   default:
      assert( 0 );
   }
   return false;
}

static void
tgsi_exec_machine_setup_masks(struct tgsi_exec_machine *mach)
{
   unsigned default_mask = 0xf;

   mach->KillMask = 0;
   mach->OutputVertexOffset = 0;

   if (mach->ShaderType == PIPE_SHADER_GEOMETRY) {
      for (unsigned i = 0; i < TGSI_MAX_VERTEX_STREAMS; i++) {
         mach->OutputPrimCount[i] = 0;
         mach->Primitives[i][0] = 0;
      }
      /* GS runs on a single primitive for now */
      default_mask = 0x1;
   }

   if (mach->NonHelperMask == 0)
      mach->NonHelperMask = default_mask;
   mach->CondMask = default_mask;
   mach->LoopMask = default_mask;
   mach->ContMask = default_mask;
   mach->FuncMask = default_mask;
   mach->ExecMask = default_mask;

   mach->Switch.mask = default_mask;

   assert(mach->CondStackTop == 0);
   assert(mach->LoopStackTop == 0);
   assert(mach->ContStackTop == 0);
   assert(mach->SwitchStackTop == 0);
   assert(mach->BreakStackTop == 0);
   assert(mach->CallStackTop == 0);
}

/**
 * Run TGSI interpreter.
 * \return bitmask of "alive" quad components
 */
uint
tgsi_exec_machine_run( struct tgsi_exec_machine *mach, int start_pc )
{
   unsigned i;

   mach->pc = start_pc;

   if (!start_pc) {
      tgsi_exec_machine_setup_masks(mach);

      /* execute declarations (interpolants) */
      for (i = 0; i < mach->NumDeclarations; i++) {
         exec_declaration( mach, mach->Declarations+i );
      }
   }

   {
#if DEBUG_EXECUTION
      struct tgsi_exec_vector temps[TGSI_EXEC_NUM_TEMPS];
      struct tgsi_exec_vector outputs[PIPE_MAX_ATTRIBS];
      unsigned inst = 1;

      if (!start_pc) {
         memset(mach->Temps, 0, sizeof(temps));
         if (mach->Outputs)
            memset(mach->Outputs, 0, sizeof(outputs));
         memset(temps, 0, sizeof(temps));
         memset(outputs, 0, sizeof(outputs));
      }
#endif

      /* execute instructions, until pc is set to -1 */
      while (mach->pc != -1) {
         bool barrier_hit;
#if DEBUG_EXECUTION
         unsigned i;

         tgsi_dump_instruction(&mach->Instructions[mach->pc], inst++);
#endif

         assert(mach->pc < (int) mach->NumInstructions);
         barrier_hit = exec_instruction(mach, mach->Instructions + mach->pc, &mach->pc);

         /* for compute shaders if we hit a barrier return now for later rescheduling */
         if (barrier_hit && mach->ShaderType == PIPE_SHADER_COMPUTE)
            return 0;

#if DEBUG_EXECUTION
         for (i = 0; i < TGSI_EXEC_NUM_TEMPS; i++) {
            if (memcmp(&temps[i], &mach->Temps[i], sizeof(temps[i]))) {
               unsigned j;

               memcpy(&temps[i], &mach->Temps[i], sizeof(temps[i]));
               debug_printf("TEMP[%2u] = ", i);
               for (j = 0; j < 4; j++) {
                  if (j > 0) {
                     debug_printf("           ");
                  }
                  debug_printf("(%6f %u, %6f %u, %6f %u, %6f %u)\n",
                               temps[i].xyzw[0].f[j], temps[i].xyzw[0].u[j],
                               temps[i].xyzw[1].f[j], temps[i].xyzw[1].u[j],
                               temps[i].xyzw[2].f[j], temps[i].xyzw[2].u[j],
                               temps[i].xyzw[3].f[j], temps[i].xyzw[3].u[j]);
               }
            }
         }
         if (mach->Outputs) {
            for (i = 0; i < PIPE_MAX_ATTRIBS; i++) {
               if (memcmp(&outputs[i], &mach->Outputs[i], sizeof(outputs[i]))) {
                  unsigned j;

                  memcpy(&outputs[i], &mach->Outputs[i], sizeof(outputs[i]));
                  debug_printf("OUT[%2u] =  ", i);
                  for (j = 0; j < 4; j++) {
                     if (j > 0) {
                        debug_printf("           ");
                     }
                     debug_printf("(%6f %u, %6f %u, %6f %u, %6f %u)\n",
                                  outputs[i].xyzw[0].f[j], outputs[i].xyzw[0].u[j],
                                  outputs[i].xyzw[1].f[j], outputs[i].xyzw[1].u[j],
                                  outputs[i].xyzw[2].f[j], outputs[i].xyzw[2].u[j],
                                  outputs[i].xyzw[3].f[j], outputs[i].xyzw[3].u[j]);
                  }
               }
            }
         }
#endif
      }
   }

#if 0
   /* we scale from floats in [0,1] to Zbuffer ints in sp_quad_depth_test.c */
   if (mach->ShaderType == PIPE_SHADER_FRAGMENT) {
      /*
       * Scale back depth component.
       */
      for (i = 0; i < 4; i++)
         mach->Outputs[0].xyzw[2].f[i] *= ctx->DrawBuffer->_DepthMaxF;
   }
#endif

   /* Strictly speaking, these assertions aren't really needed but they
    * can potentially catch some bugs in the control flow code.
    */
   assert(mach->CondStackTop == 0);
   assert(mach->LoopStackTop == 0);
   assert(mach->ContStackTop == 0);
   assert(mach->SwitchStackTop == 0);
   assert(mach->BreakStackTop == 0);
   assert(mach->CallStackTop == 0);

   return ~mach->KillMask;
}
