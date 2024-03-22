/*
 * Copyright 2015 Samuel Pitoiset
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#define NV50_PUSH_EXPLICIT_SPACE_CHECKING

#include "nv50/nv50_context.h"
#include "nv50/nv50_query_hw_sm.h"

#include "nv_object.xml.h"
#include "nv50/nv50_compute.xml.h"

/* === PERFORMANCE MONITORING COUNTERS for NV84+ === */

/* NOTE: intentionally using the same names as NV */
static const char *nv50_hw_sm_query_names[] =
{
   "branch",
   "divergent_branch",
   "instructions",
   "prof_trigger_00",
   "prof_trigger_01",
   "prof_trigger_02",
   "prof_trigger_03",
   "prof_trigger_04",
   "prof_trigger_05",
   "prof_trigger_06",
   "prof_trigger_07",
   "sm_cta_launched",
   "warp_serialize",
};

static const uint64_t nv50_read_hw_sm_counters_code[] =
{
   /* and b32 $r0 $r0 0x0000ffff
    * add b32 $c0 $r0 $r0 $r0
    * (lg $c0) ret
    * mov $r0 $pm0
    * mov $r1 $pm1
    * mov $r2 $pm2
    * mov $r3 $pm3
    * mov $r4 $physid
    * ld $r5 b32 s[0x14]
    * ld $r6 b32 s[0x18]
    * and b32 $r4 $r4 0x000f0000
    * shr u32 $r4 $r4 0x10
    * mul $r4 u24 $r4 0x14
    * add b32 $r5 $r5 $r4
    * st b32 g15[$r5] $r0
    * add b32 $r5 $r5 0x04
    * st b32 g15[$r5] $r1
    * add b32 $r5 $r5 0x04
    * st b32 g15[$r5] $r2
    * add b32 $r5 $r5 0x04
    * st b32 g15[$r5] $r3
    * add b32 $r5 $r5 0x04
    * exit st b32 g15[$r5] $r6 */
   0x00000fffd03f0001ULL,
   0x040007c020000001ULL,
   0x0000028030000003ULL,
   0x6001078000000001ULL,
   0x6001478000000005ULL,
   0x6001878000000009ULL,
   0x6001c7800000000dULL,
   0x6000078000000011ULL,
   0x4400c78010000a15ULL,
   0x4400c78010000c19ULL,
   0x0000f003d0000811ULL,
   0xe410078030100811ULL,
   0x0000000340540811ULL,
   0x0401078020000a15ULL,
   0xa0c00780d00f0a01ULL,
   0x0000000320048a15ULL,
   0xa0c00780d00f0a05ULL,
   0x0000000320048a15ULL,
   0xa0c00780d00f0a09ULL,
   0x0000000320048a15ULL,
   0xa0c00780d00f0a0dULL,
   0x0000000320048a15ULL,
   0xa0c00781d00f0a19ULL,
};

struct nv50_hw_sm_counter_cfg
{
   uint32_t mode : 4;    /* LOGOP, LOGOP_PULSE */
   uint32_t unit : 8;    /* UNK[0-5] */
   uint32_t sig  : 8;    /* signal selection */
};

struct nv50_hw_sm_query_cfg
{
   struct nv50_hw_sm_counter_cfg ctr[4];
   uint8_t num_counters;
};

#define _Q(n, m, u, s) [NV50_HW_SM_QUERY_##n] = { { { NV50_COMPUTE_MP_PM_CONTROL_MODE_##m, NV50_COMPUTE_MP_PM_CONTROL_UNIT_##u, s, }, {}, {}, {} }, 1 }

/* ==== Compute capability 1.1 (G84+) ==== */
static const struct nv50_hw_sm_query_cfg sm11_hw_sm_queries[] =
{
   _Q(BRANCH,           LOGOP, UNK4, 0x02),
   _Q(DIVERGENT_BRANCH, LOGOP, UNK4, 0x09),
   _Q(INSTRUCTIONS,     LOGOP, UNK4, 0x04),
   _Q(PROF_TRIGGER_0,   LOGOP, UNK1, 0x26),
   _Q(PROF_TRIGGER_1,   LOGOP, UNK1, 0x27),
   _Q(PROF_TRIGGER_2,   LOGOP, UNK1, 0x28),
   _Q(PROF_TRIGGER_3,   LOGOP, UNK1, 0x29),
   _Q(PROF_TRIGGER_4,   LOGOP, UNK1, 0x2a),
   _Q(PROF_TRIGGER_5,   LOGOP, UNK1, 0x2b),
   _Q(PROF_TRIGGER_6,   LOGOP, UNK1, 0x2c),
   _Q(PROF_TRIGGER_7,   LOGOP, UNK1, 0x2d),
   _Q(SM_CTA_LAUNCHED,  LOGOP, UNK1, 0x33),
   _Q(WARP_SERIALIZE,   LOGOP, UNK0, 0x0b),
};

static inline uint16_t nv50_hw_sm_get_func(uint8_t slot)
{
   switch (slot) {
   case 0: return 0xaaaa;
   case 1: return 0xcccc;
   case 2: return 0xf0f0;
   case 3: return 0xff00;
   }
   return 0;
}

static const struct nv50_hw_sm_query_cfg *
nv50_hw_sm_query_get_cfg(struct nv50_context *nv50, struct nv50_hw_query *hq)
{
   struct nv50_query *q = &hq->base;
   return &sm11_hw_sm_queries[q->type - NV50_HW_SM_QUERY(0)];
}

static void
nv50_hw_sm_destroy_query(struct nv50_context *nv50, struct nv50_hw_query *hq)
{
   struct nv50_query *q = &hq->base;
   nv50_hw_query_allocate(nv50, q, 0);
   nouveau_fence_ref(NULL, &hq->fence);
   FREE(hq);
}

static bool
nv50_hw_sm_begin_query(struct nv50_context *nv50, struct nv50_hw_query *hq)
{
   struct nv50_screen *screen = nv50->screen;
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_hw_sm_query *hsq = nv50_hw_sm_query(hq);
   const struct nv50_hw_sm_query_cfg *cfg;
   uint16_t func;
   int i, c;

   cfg = nv50_hw_sm_query_get_cfg(nv50, hq);

   /* check if we have enough free counter slots */
   if (screen->pm.num_hw_sm_active + cfg->num_counters > 4) {
      NOUVEAU_ERR("Not enough free MP counter slots !\n");
      return false;
   }

   assert(cfg->num_counters <= 4);
   PUSH_SPACE(push, 4 * 4);

   /* set sequence field to 0 (used to check if result is available) */
   for (i = 0; i < screen->MPsInTP; ++i) {
      const unsigned b = (0x14 / 4) * i;
      hq->data[b + 16] = 0;
   }
   hq->sequence++;

   for (i = 0; i < cfg->num_counters; i++) {
      screen->pm.num_hw_sm_active++;

      /* find free counter slots */
      for (c = 0; c < 4; ++c) {
         if (!screen->pm.mp_counter[c]) {
            hsq->ctr[i] = c;
            screen->pm.mp_counter[c] = hsq;
            break;
         }
      }

      /* select func to aggregate counters */
      func = nv50_hw_sm_get_func(c);

      /* configure and reset the counter(s) */
      BEGIN_NV04(push, NV50_CP(MP_PM_CONTROL(c)), 1);
      PUSH_DATA (push, (cfg->ctr[i].sig << 24) | (func << 8)
                        | cfg->ctr[i].unit | cfg->ctr[i].mode);
      BEGIN_NV04(push, NV50_CP(MP_PM_SET(c)), 1);
      PUSH_DATA (push, 0);
   }
   return true;
}

static void
nv50_hw_sm_end_query(struct nv50_context *nv50, struct nv50_hw_query *hq)
{
   struct nv50_screen *screen = nv50->screen;
   struct pipe_context *pipe = &nv50->base.pipe;
   struct nouveau_pushbuf *push = nv50->base.pushbuf;
   struct nv50_hw_sm_query *hsq = nv50_hw_sm_query(hq);
   struct nv50_program *old = nv50->compprog;
   struct pipe_grid_info info = {};
   uint32_t mask;
   uint32_t input[3];
   const uint block[3] = { 32, 1, 1 };
   const uint grid[3] = { screen->MPsInTP, screen->TPs, 1 };
   int c, i;

   if (unlikely(!screen->pm.prog)) {
      struct nv50_program *prog = CALLOC_STRUCT(nv50_program);
      prog->type = PIPE_SHADER_COMPUTE;
      prog->translated = true;
      prog->max_gpr = 7;
      prog->parm_size = 8;
      prog->code = (uint32_t *)nv50_read_hw_sm_counters_code;
      prog->code_size = sizeof(nv50_read_hw_sm_counters_code);
      screen->pm.prog = prog;
   }

   /* disable all counting */
   PUSH_SPACE(push, 8);
   for (c = 0; c < 4; c++) {
      if (screen->pm.mp_counter[c]) {
         BEGIN_NV04(push, NV50_CP(MP_PM_CONTROL(c)), 1);
         PUSH_DATA (push, 0);
      }
   }

   /* release counters for this query */
   for (c = 0; c < 4; c++) {
      if (screen->pm.mp_counter[c] == hsq) {
         screen->pm.num_hw_sm_active--;
         screen->pm.mp_counter[c] = NULL;
      }
   }

   BCTX_REFN_bo(nv50->bufctx_cp, CP_QUERY, NOUVEAU_BO_GART | NOUVEAU_BO_WR,
                hq->bo);

   PUSH_SPACE(push, 2);
   BEGIN_NV04(push, SUBC_CP(NV50_GRAPH_SERIALIZE), 1);
   PUSH_DATA (push, 0);

   pipe->bind_compute_state(pipe, screen->pm.prog);
   input[0] = hq->bo->offset + hq->base_offset;
   input[1] = hq->sequence;

   for (i = 0; i < 3; i++) {
      info.block[i] = block[i];
      info.grid[i] = grid[i];
   }
   info.pc = 0;
   info.input = input;
   pipe->launch_grid(pipe, &info);
   pipe->bind_compute_state(pipe, old);

   nouveau_bufctx_reset(nv50->bufctx_cp, NV50_BIND_CP_QUERY);

   /* re-active other counters */
   PUSH_SPACE(push, 8);
   mask = 0;
   for (c = 0; c < 4; c++) {
      const struct nv50_hw_sm_query_cfg *cfg;
      unsigned i;

      hsq = screen->pm.mp_counter[c];
      if (!hsq)
         continue;

      cfg = nv50_hw_sm_query_get_cfg(nv50, &hsq->base);
      for (i = 0; i < cfg->num_counters; i++) {
         uint16_t func;

         if (mask & (1 << hsq->ctr[i]))
            break;

         mask |= 1 << hsq->ctr[i];
         func  = nv50_hw_sm_get_func(hsq->ctr[i]);

         BEGIN_NV04(push, NV50_CP(MP_PM_CONTROL(hsq->ctr[i])), 1);
         PUSH_DATA (push, (cfg->ctr[i].sig << 24) | (func << 8)
                    | cfg->ctr[i].unit | cfg->ctr[i].mode);
      }
   }
}

static inline bool
nv50_hw_sm_query_read_data(uint32_t count[32][4],
                           struct nv50_context *nv50, bool wait,
                           struct nv50_hw_query *hq,
                           const struct nv50_hw_sm_query_cfg *cfg,
                           unsigned mp_count)
{
   struct nv50_hw_sm_query *hsq = nv50_hw_sm_query(hq);
   unsigned p, c;

   for (p = 0; p < mp_count; ++p) {
      const unsigned b = (0x14 / 4) * p;

      for (c = 0; c < cfg->num_counters; ++c) {
         if (hq->data[b + 4] != hq->sequence) {
            if (!wait)
               return false;
            if (BO_WAIT(&nv50->screen->base, hq->bo, NOUVEAU_BO_RD, nv50->base.client))
               return false;
         }
         count[p][c] = hq->data[b + hsq->ctr[c]];
      }
   }
   return true;
}

static bool
nv50_hw_sm_get_query_result(struct nv50_context *nv50, struct nv50_hw_query *hq,
                            bool wait, union pipe_query_result *result)
{
   uint32_t count[32][4];
   uint64_t value = 0;
   unsigned mp_count = MIN2(nv50->screen->MPsInTP, 32);
   unsigned p, c;
   const struct nv50_hw_sm_query_cfg *cfg;
   bool ret;

   cfg = nv50_hw_sm_query_get_cfg(nv50, hq);

   ret = nv50_hw_sm_query_read_data(count, nv50, wait, hq, cfg, mp_count);
   if (!ret)
      return false;

   for (c = 0; c < cfg->num_counters; ++c)
      for (p = 0; p < mp_count; ++p)
         value += count[p][c];

   /* We only count a single TP, and simply multiply by the total number of
    * TPs to compute result over all TPs. This is inaccurate, but enough! */
   value *= nv50->screen->TPs;

   *(uint64_t *)result = value;
   return true;
}

static const struct nv50_hw_query_funcs hw_sm_query_funcs = {
   .destroy_query = nv50_hw_sm_destroy_query,
   .begin_query = nv50_hw_sm_begin_query,
   .end_query = nv50_hw_sm_end_query,
   .get_query_result = nv50_hw_sm_get_query_result,
};

struct nv50_hw_query *
nv50_hw_sm_create_query(struct nv50_context *nv50, unsigned type)
{
   struct nv50_hw_sm_query *hsq;
   struct nv50_hw_query *hq;
   unsigned space;

   if (type < NV50_HW_SM_QUERY(0) || type > NV50_HW_SM_QUERY_LAST)
      return NULL;

   hsq = CALLOC_STRUCT(nv50_hw_sm_query);
   if (!hsq)
      return NULL;

   hq = &hsq->base;
   hq->funcs = &hw_sm_query_funcs;
   hq->base.type = type;

   /*
    * for each MP:
    * [00] = MP.C0
    * [04] = MP.C1
    * [08] = MP.C2
    * [0c] = MP.C3
    * [10] = MP.sequence
    */
   space = (4 + 1) * nv50->screen->MPsInTP * sizeof(uint32_t);

   if (!nv50_hw_query_allocate(nv50, &hq->base, space)) {
      FREE(hq);
      return NULL;
   }

   return hq;
}

int
nv50_hw_sm_get_driver_query_info(struct nv50_screen *screen, unsigned id,
                                 struct pipe_driver_query_info *info)
{
   int count = 0;

   if (screen->compute)
      if (screen->base.class_3d >= NV84_3D_CLASS)
         count += NV50_HW_SM_QUERY_COUNT;

   if (!info)
      return count;

   if (id < count) {
      if (screen->compute) {
         if (screen->base.class_3d >= NV84_3D_CLASS) {
            info->name = nv50_hw_sm_query_names[id];
            info->query_type = NV50_HW_SM_QUERY(id);
            info->group_id = NV50_HW_SM_QUERY_GROUP;
            return 1;
         }
      }
   }
   return 0;
}
