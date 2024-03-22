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

#include "nvc0/nvc0_context.h"
#include "nvc0/nvc0_query_hw_metric.h"
#include "nvc0/nvc0_query_hw_sm.h"

#define _Q(i,n,t,d) { NVC0_HW_METRIC_QUERY_##i, n, PIPE_DRIVER_QUERY_TYPE_##t, d }
static const struct nvc0_hw_metric_cfg {
   unsigned id;
   const char *name;
   enum pipe_driver_query_type type;
   const char *desc;
} nvc0_hw_metric_queries[] = {
   _Q(ACHIEVED_OCCUPANCY,
      "metric-achieved_occupancy",
      PERCENTAGE,
      "Ratio of the average active warps per active cycle to the maximum "
      "number of warps supported on a multiprocessor"),

   _Q(BRANCH_EFFICIENCY,
      "metric-branch_efficiency",
      PERCENTAGE,
      "Ratio of non-divergent branches to total branches"),

   _Q(INST_ISSUED,
      "metric-inst_issued",
      UINT64,
      "The number of instructions issued"),

   _Q(INST_PER_WRAP,
      "metric-inst_per_wrap",
      UINT64,
      "Average number of instructions executed by each warp"),

   _Q(INST_REPLAY_OVERHEAD,
      "metric-inst_replay_overhead",
      UINT64,
      "Average number of replays for each instruction executed"),

   _Q(ISSUED_IPC,
      "metric-issued_ipc",
      UINT64,
      "Instructions issued per cycle"),

   _Q(ISSUE_SLOTS,
      "metric-issue_slots",
      UINT64,
      "The number of issue slots used"),

   _Q(ISSUE_SLOT_UTILIZATION,
      "metric-issue_slot_utilization",
      PERCENTAGE,
      "Percentage of issue slots that issued at least one instruction, "
      "averaged across all cycles"),

   _Q(IPC,
      "metric-ipc",
      UINT64,
      "Instructions executed per cycle"),

   _Q(SHARED_REPLAY_OVERHEAD,
      "metric-shared_replay_overhead",
      UINT64,
      "Average number of replays due to shared memory conflicts for each "
      "instruction executed"),

   _Q(WARP_EXECUTION_EFFICIENCY,
      "metric-warp_execution_efficiency",
      PERCENTAGE,
      "Ratio of the average active threads per warp to the maximum number of "
      "threads per warp supported on a multiprocessor"),

   _Q(WARP_NONPRED_EXECUTION_EFFICIENCY,
      "metric-warp_nonpred_execution_efficiency",
      PERCENTAGE,
      "Ratio of the average active threads per warp executing non-predicated "
      "instructions to the maximum number of threads per warp supported on a "
      "multiprocessor"),
};

#undef _Q

static inline const struct nvc0_hw_metric_cfg *
nvc0_hw_metric_get_cfg(unsigned metric_id)
{
   unsigned i;

   for (i = 0; i < ARRAY_SIZE(nvc0_hw_metric_queries); i++) {
      if (nvc0_hw_metric_queries[i].id == metric_id)
         return &nvc0_hw_metric_queries[i];
   }
   assert(0);
   return NULL;
}

struct nvc0_hw_metric_query_cfg {
   unsigned type;
   uint32_t queries[8];
   uint32_t num_queries;
};

#define _SM(n) NVC0_HW_SM_QUERY(NVC0_HW_SM_QUERY_ ##n)

/* ==== Compute capability 2.0 (GF100/GF110) ==== */
static const struct nvc0_hw_metric_query_cfg
sm20_achieved_occupancy =
{
   .type        = NVC0_HW_METRIC_QUERY_ACHIEVED_OCCUPANCY,
   .queries[0]  = _SM(ACTIVE_WARPS),
   .queries[1]  = _SM(ACTIVE_CYCLES),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg
sm20_branch_efficiency =
{
   .type        = NVC0_HW_METRIC_QUERY_BRANCH_EFFICIENCY,
   .queries[0]  = _SM(BRANCH),
   .queries[1]  = _SM(DIVERGENT_BRANCH),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg
sm20_inst_per_wrap =
{
   .type        = NVC0_HW_METRIC_QUERY_INST_PER_WRAP,
   .queries[0]  = _SM(INST_EXECUTED),
   .queries[1]  = _SM(WARPS_LAUNCHED),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg
sm20_inst_replay_overhead =
{
   .type        = NVC0_HW_METRIC_QUERY_INST_REPLAY_OVERHEAD,
   .queries[0]  = _SM(INST_ISSUED),
   .queries[1]  = _SM(INST_EXECUTED),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg
sm20_issued_ipc =
{
   .type        = NVC0_HW_METRIC_QUERY_ISSUED_IPC,
   .queries[0]  = _SM(INST_ISSUED),
   .queries[1]  = _SM(ACTIVE_CYCLES),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg
sm20_issue_slot_utilization =
{
   .type        = NVC0_HW_METRIC_QUERY_ISSUE_SLOT_UTILIZATION,
   .queries[0]  = _SM(INST_ISSUED),
   .queries[1]  = _SM(ACTIVE_CYCLES),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg
sm20_ipc =
{
   .type        = NVC0_HW_METRIC_QUERY_IPC,
   .queries[0]  = _SM(INST_EXECUTED),
   .queries[1]  = _SM(ACTIVE_CYCLES),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg *sm20_hw_metric_queries[] =
{
   &sm20_achieved_occupancy,
   &sm20_branch_efficiency,
   &sm20_inst_per_wrap,
   &sm20_inst_replay_overhead,
   &sm20_ipc,
   &sm20_issued_ipc,
   &sm20_issue_slot_utilization,
};

/* ==== Compute capability 2.1 (GF108+ except GF110) ==== */
static const struct nvc0_hw_metric_query_cfg
sm21_inst_issued =
{
   .type        = NVC0_HW_METRIC_QUERY_INST_ISSUED,
   .queries[0]  = _SM(INST_ISSUED1_0),
   .queries[1]  = _SM(INST_ISSUED1_1),
   .queries[2]  = _SM(INST_ISSUED2_0),
   .queries[3]  = _SM(INST_ISSUED2_1),
   .num_queries = 4,
};

static const struct nvc0_hw_metric_query_cfg
sm21_inst_replay_overhead =
{
   .type        = NVC0_HW_METRIC_QUERY_INST_REPLAY_OVERHEAD,
   .queries[0]  = _SM(INST_ISSUED1_0),
   .queries[1]  = _SM(INST_ISSUED1_1),
   .queries[2]  = _SM(INST_ISSUED2_0),
   .queries[3]  = _SM(INST_ISSUED2_1),
   .queries[4]  = _SM(INST_EXECUTED),
   .num_queries = 5,
};

static const struct nvc0_hw_metric_query_cfg
sm21_issued_ipc =
{
   .type        = NVC0_HW_METRIC_QUERY_ISSUED_IPC,
   .queries[0]  = _SM(INST_ISSUED1_0),
   .queries[1]  = _SM(INST_ISSUED1_1),
   .queries[2]  = _SM(INST_ISSUED2_0),
   .queries[3]  = _SM(INST_ISSUED2_1),
   .queries[4]  = _SM(ACTIVE_CYCLES),
   .num_queries = 5,
};

static const struct nvc0_hw_metric_query_cfg
sm21_issue_slots =
{
   .type        = NVC0_HW_METRIC_QUERY_ISSUE_SLOTS,
   .queries[0]  = _SM(INST_ISSUED1_0),
   .queries[1]  = _SM(INST_ISSUED1_1),
   .queries[2]  = _SM(INST_ISSUED2_0),
   .queries[3]  = _SM(INST_ISSUED2_1),
   .num_queries = 4,
};

static const struct nvc0_hw_metric_query_cfg
sm21_issue_slot_utilization =
{
   .type        = NVC0_HW_METRIC_QUERY_ISSUE_SLOT_UTILIZATION,
   .queries[0]  = _SM(INST_ISSUED1_0),
   .queries[1]  = _SM(INST_ISSUED1_1),
   .queries[2]  = _SM(INST_ISSUED2_0),
   .queries[3]  = _SM(INST_ISSUED2_1),
   .queries[4]  = _SM(ACTIVE_CYCLES),
   .num_queries = 5,
};

static const struct nvc0_hw_metric_query_cfg *sm21_hw_metric_queries[] =
{
   &sm20_achieved_occupancy,
   &sm20_branch_efficiency,
   &sm21_inst_issued,
   &sm20_inst_per_wrap,
   &sm21_inst_replay_overhead,
   &sm20_ipc,
   &sm21_issued_ipc,
   &sm21_issue_slots,
   &sm21_issue_slot_utilization,
};

/* ==== Compute capability 3.0 (GK104/GK106/GK107) ==== */
static const struct nvc0_hw_metric_query_cfg
sm30_inst_issued =
{
   .type        = NVC0_HW_METRIC_QUERY_INST_ISSUED,
   .queries[0]  = _SM(INST_ISSUED1),
   .queries[1]  = _SM(INST_ISSUED2),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg
sm30_inst_replay_overhead =
{
   .type        = NVC0_HW_METRIC_QUERY_INST_REPLAY_OVERHEAD,
   .queries[0]  = _SM(INST_ISSUED1),
   .queries[1]  = _SM(INST_ISSUED2),
   .queries[2]  = _SM(INST_EXECUTED),
   .num_queries = 3,
};

static const struct nvc0_hw_metric_query_cfg
sm30_issued_ipc =
{
   .type        = NVC0_HW_METRIC_QUERY_ISSUED_IPC,
   .queries[0]  = _SM(INST_ISSUED1),
   .queries[1]  = _SM(INST_ISSUED2),
   .queries[2]  = _SM(ACTIVE_CYCLES),
   .num_queries = 3,
};

static const struct nvc0_hw_metric_query_cfg
sm30_issue_slots =
{
   .type        = NVC0_HW_METRIC_QUERY_ISSUE_SLOTS,
   .queries[0]  = _SM(INST_ISSUED1),
   .queries[1]  = _SM(INST_ISSUED2),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg
sm30_issue_slot_utilization =
{
   .type        = NVC0_HW_METRIC_QUERY_ISSUE_SLOT_UTILIZATION,
   .queries[0]  = _SM(INST_ISSUED1),
   .queries[1]  = _SM(INST_ISSUED2),
   .queries[2]  = _SM(ACTIVE_CYCLES),
   .num_queries = 3,
};

static const struct nvc0_hw_metric_query_cfg
sm30_shared_replay_overhead =
{
   .type        = NVC0_HW_METRIC_QUERY_SHARED_REPLAY_OVERHEAD,
   .queries[0]  = _SM(SHARED_LD_REPLAY),
   .queries[1]  = _SM(SHARED_ST_REPLAY),
   .queries[2]  = _SM(INST_EXECUTED),
   .num_queries = 3,
};

static const struct nvc0_hw_metric_query_cfg
sm30_warp_execution_efficiency =
{
   .type        = NVC0_HW_METRIC_QUERY_WARP_EXECUTION_EFFICIENCY,
   .queries[0]  = _SM(INST_EXECUTED),
   .queries[1]  = _SM(TH_INST_EXECUTED),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg *sm30_hw_metric_queries[] =
{
   &sm20_achieved_occupancy,
   &sm20_branch_efficiency,
   &sm30_inst_issued,
   &sm20_inst_per_wrap,
   &sm30_inst_replay_overhead,
   &sm20_ipc,
   &sm30_issued_ipc,
   &sm30_issue_slots,
   &sm30_issue_slot_utilization,
   &sm30_shared_replay_overhead,
   &sm30_warp_execution_efficiency,
};

/* ==== Compute capability 3.5 (GK110/GK208) ==== */
static const struct nvc0_hw_metric_query_cfg
sm35_warp_nonpred_execution_efficiency =
{
   .type        = NVC0_HW_METRIC_QUERY_WARP_NONPRED_EXECUTION_EFFICIENCY,
   .queries[0]  = _SM(INST_EXECUTED),
   .queries[1]  = _SM(NOT_PRED_OFF_INST_EXECUTED),
   .num_queries = 2,
};

static const struct nvc0_hw_metric_query_cfg *sm35_hw_metric_queries[] =
{
   &sm20_achieved_occupancy,
   &sm30_inst_issued,
   &sm20_inst_per_wrap,
   &sm30_inst_replay_overhead,
   &sm20_ipc,
   &sm30_issued_ipc,
   &sm30_issue_slots,
   &sm30_issue_slot_utilization,
   &sm30_shared_replay_overhead,
   &sm30_warp_execution_efficiency,
   &sm35_warp_nonpred_execution_efficiency,
};

/* ==== Compute capability 5.0 (GM107/GM108) ==== */
static const struct nvc0_hw_metric_query_cfg *sm50_hw_metric_queries[] =
{
   &sm20_achieved_occupancy,
   &sm20_branch_efficiency,
   &sm30_inst_issued,
   &sm20_inst_per_wrap,
   &sm30_inst_replay_overhead,
   &sm20_ipc,
   &sm30_issued_ipc,
   &sm30_issue_slots,
   &sm30_issue_slot_utilization,
   &sm30_warp_execution_efficiency,
   &sm35_warp_nonpred_execution_efficiency,
};

#undef _SM

static inline const struct nvc0_hw_metric_query_cfg **
nvc0_hw_metric_get_queries(struct nvc0_screen *screen)
{
   struct nouveau_device *dev = screen->base.device;

   switch (screen->base.class_3d) {
   case GM200_3D_CLASS:
   case GM107_3D_CLASS:
      return sm50_hw_metric_queries;
   case NVF0_3D_CLASS:
      return sm35_hw_metric_queries;
   case NVE4_3D_CLASS:
      return sm30_hw_metric_queries;
   case NVC0_3D_CLASS:
   case NVC1_3D_CLASS:
   case NVC8_3D_CLASS:
      if (dev->chipset == 0xc0 || dev->chipset == 0xc8)
         return sm20_hw_metric_queries;
      return sm21_hw_metric_queries;
   }
   assert(0);
   return NULL;
}

unsigned
nvc0_hw_metric_get_num_queries(struct nvc0_screen *screen)
{
   struct nouveau_device *dev = screen->base.device;

   switch (screen->base.class_3d) {
   case GM200_3D_CLASS:
   case GM107_3D_CLASS:
      return ARRAY_SIZE(sm50_hw_metric_queries);
   case NVF0_3D_CLASS:
      return ARRAY_SIZE(sm35_hw_metric_queries);
   case NVE4_3D_CLASS:
      return ARRAY_SIZE(sm30_hw_metric_queries);
   case NVC0_3D_CLASS:
   case NVC1_3D_CLASS:
   case NVC8_3D_CLASS:
      if (dev->chipset == 0xc0 || dev->chipset == 0xc8)
         return ARRAY_SIZE(sm20_hw_metric_queries);
      return ARRAY_SIZE(sm21_hw_metric_queries);
   }
   return 0;
}

static const struct nvc0_hw_metric_query_cfg *
nvc0_hw_metric_query_get_cfg(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   const struct nvc0_hw_metric_query_cfg **queries;
   struct nvc0_screen *screen = nvc0->screen;
   struct nvc0_query *q = &hq->base;
   unsigned num_queries;
   unsigned i;

   num_queries = nvc0_hw_metric_get_num_queries(screen);
   queries = nvc0_hw_metric_get_queries(screen);

   for (i = 0; i < num_queries; i++) {
      if (NVC0_HW_METRIC_QUERY(queries[i]->type) == q->type)
         return queries[i];
   }
   assert(0);
   return NULL;
}

static void
nvc0_hw_metric_destroy_query(struct nvc0_context *nvc0,
                             struct nvc0_hw_query *hq)
{
   struct nvc0_hw_metric_query *hmq = nvc0_hw_metric_query(hq);
   unsigned i;

   for (i = 0; i < hmq->num_queries; i++)
      if (hmq->queries[i]->funcs->destroy_query)
         hmq->queries[i]->funcs->destroy_query(nvc0, hmq->queries[i]);
   FREE(hmq);
}

static bool
nvc0_hw_metric_begin_query(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   struct nvc0_hw_metric_query *hmq = nvc0_hw_metric_query(hq);
   bool ret = false;
   unsigned i;

   for (i = 0; i < hmq->num_queries; i++) {
      ret = hmq->queries[i]->funcs->begin_query(nvc0, hmq->queries[i]);
      if (!ret)
         return ret;
   }
   return ret;
}

static void
nvc0_hw_metric_end_query(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   struct nvc0_hw_metric_query *hmq = nvc0_hw_metric_query(hq);
   unsigned i;

   for (i = 0; i < hmq->num_queries; i++)
      hmq->queries[i]->funcs->end_query(nvc0, hmq->queries[i]);
}

static uint64_t
sm20_hw_metric_calc_result(struct nvc0_hw_query *hq, uint64_t res64[8])
{
   switch (hq->base.type - NVC0_HW_METRIC_QUERY(0)) {
   case NVC0_HW_METRIC_QUERY_ACHIEVED_OCCUPANCY:
      /* ((active_warps / active_cycles) / max. number of warps on a MP) * 100 */
      if (res64[1])
         return ((res64[0] / (double)res64[1]) / 48) * 100;
      break;
   case NVC0_HW_METRIC_QUERY_BRANCH_EFFICIENCY:
      /* (branch / (branch + divergent_branch)) * 100 */
      if (res64[0] + res64[1])
         return (res64[0] / (double)(res64[0] + res64[1])) * 100;
      break;
   case NVC0_HW_METRIC_QUERY_INST_PER_WRAP:
      /* inst_executed / warps_launched */
      if (res64[1])
         return res64[0] / (double)res64[1];
      break;
   case NVC0_HW_METRIC_QUERY_INST_REPLAY_OVERHEAD:
      /* (inst_issued - inst_executed) / inst_executed */
      if (res64[1])
         return (res64[0] - res64[1]) / (double)res64[1];
      break;
   case NVC0_HW_METRIC_QUERY_ISSUED_IPC:
      /* inst_issued / active_cycles */
      if (res64[1])
         return res64[0] / (double)res64[1];
      break;
   case NVC0_HW_METRIC_QUERY_ISSUE_SLOT_UTILIZATION:
      /* ((inst_issued / 2) / active_cycles) * 100 */
      if (res64[1])
         return ((res64[0] / 2) / (double)res64[1]) * 100;
      break;
   case NVC0_HW_METRIC_QUERY_IPC:
      /* inst_executed / active_cycles */
      if (res64[1])
         return res64[0] / (double)res64[1];
      break;
   default:
      debug_printf("invalid metric type: %d\n",
                   hq->base.type - NVC0_HW_METRIC_QUERY(0));
      break;
   }
   return 0;
}

static uint64_t
sm21_hw_metric_calc_result(struct nvc0_hw_query *hq, uint64_t res64[8])
{
   switch (hq->base.type - NVC0_HW_METRIC_QUERY(0)) {
   case NVC0_HW_METRIC_QUERY_ACHIEVED_OCCUPANCY:
      return sm20_hw_metric_calc_result(hq, res64);
   case NVC0_HW_METRIC_QUERY_BRANCH_EFFICIENCY:
      return sm20_hw_metric_calc_result(hq, res64);
   case NVC0_HW_METRIC_QUERY_INST_ISSUED:
      /* issued1_0 + issued1_1 + (issued2_0 + issued2_1) * 2 */
      return res64[0] + res64[1] + (res64[2] + res64[3]) * 2;
      break;
   case NVC0_HW_METRIC_QUERY_INST_PER_WRAP:
      return sm20_hw_metric_calc_result(hq, res64);
   case NVC0_HW_METRIC_QUERY_INST_REPLAY_OVERHEAD:
      /* (metric-inst_issued - inst_executed) / inst_executed */
      if (res64[4])
         return (((res64[0] + res64[1] + (res64[2] + res64[3]) * 2) -
                   res64[4]) / (double)res64[4]);
      break;
   case NVC0_HW_METRIC_QUERY_ISSUED_IPC:
      /* metric-inst_issued / active_cycles */
      if (res64[4])
         return (res64[0] + res64[1] + (res64[2] + res64[3]) * 2) /
                (double)res64[4];
      break;
   case NVC0_HW_METRIC_QUERY_ISSUE_SLOTS:
      /* issued1_0 + issued1_1 + issued2_0 + issued2_1 */
      return res64[0] + res64[1] + res64[2] + res64[3];
      break;
   case NVC0_HW_METRIC_QUERY_ISSUE_SLOT_UTILIZATION:
      /* ((metric-issue_slots / 2) / active_cycles) * 100 */
      if (res64[4])
         return (((res64[0] + res64[1] + res64[2] + res64[3]) / 2) /
                 (double)res64[4]) * 100;
      break;
   case NVC0_HW_METRIC_QUERY_IPC:
      return sm20_hw_metric_calc_result(hq, res64);
   default:
      debug_printf("invalid metric type: %d\n",
                   hq->base.type - NVC0_HW_METRIC_QUERY(0));
      break;
   }
   return 0;
}

static uint64_t
sm30_hw_metric_calc_result(struct nvc0_hw_query *hq, uint64_t res64[8])
{
   switch (hq->base.type - NVC0_HW_METRIC_QUERY(0)) {
   case NVC0_HW_METRIC_QUERY_ACHIEVED_OCCUPANCY:
      /* ((active_warps / active_cycles) / max. number of warps on a MP) * 100 */
      if (res64[1])
         return ((res64[0] / (double)res64[1]) / 64) * 100;
      break;
   case NVC0_HW_METRIC_QUERY_BRANCH_EFFICIENCY:
      return sm20_hw_metric_calc_result(hq, res64);
   case NVC0_HW_METRIC_QUERY_INST_ISSUED:
      /* inst_issued1 + inst_issued2 * 2 */
      return res64[0] + res64[1] * 2;
   case NVC0_HW_METRIC_QUERY_INST_PER_WRAP:
      return sm20_hw_metric_calc_result(hq, res64);
   case NVC0_HW_METRIC_QUERY_INST_REPLAY_OVERHEAD:
      /* (metric-inst_issued - inst_executed) / inst_executed */
      if (res64[2])
         return (((res64[0] + res64[1] * 2) - res64[2]) / (double)res64[2]);
      break;
   case NVC0_HW_METRIC_QUERY_ISSUED_IPC:
      /* metric-inst_issued / active_cycles */
      if (res64[2])
         return (res64[0] + res64[1] * 2) / (double)res64[2];
      break;
   case NVC0_HW_METRIC_QUERY_ISSUE_SLOTS:
      /* inst_issued1 + inst_issued2 */
      return res64[0] + res64[1];
   case NVC0_HW_METRIC_QUERY_ISSUE_SLOT_UTILIZATION:
      /* ((metric-issue_slots / 2) / active_cycles) * 100 */
      if (res64[2])
         return (((res64[0] + res64[1]) / 2) / (double)res64[2]) * 100;
      break;
   case NVC0_HW_METRIC_QUERY_IPC:
      return sm20_hw_metric_calc_result(hq, res64);
   case NVC0_HW_METRIC_QUERY_SHARED_REPLAY_OVERHEAD:
      /* (shared_load_replay + shared_store_replay) / inst_executed */
      if (res64[2])
         return (res64[0] + res64[1]) / (double)res64[2];
      break;
   case NVC0_HW_METRIC_QUERY_WARP_EXECUTION_EFFICIENCY:
      /* thread_inst_executed / (inst_executed * max. number of threads per
       * wrap) * 100 */
      if (res64[0])
         return (res64[1] / ((double)res64[0] * 32)) * 100;
      break;
   default:
      debug_printf("invalid metric type: %d\n",
                   hq->base.type - NVC0_HW_METRIC_QUERY(0));
      break;
   }
   return 0;
}

static uint64_t
sm35_hw_metric_calc_result(struct nvc0_hw_query *hq, uint64_t res64[8])
{
   switch (hq->base.type - NVC0_HW_METRIC_QUERY(0)) {
   case NVC0_HW_METRIC_QUERY_WARP_NONPRED_EXECUTION_EFFICIENCY:
      /* not_predicated_off_thread_inst_executed / (inst_executed * max. number
       * of threads per wrap) * 100 */
      if (res64[0])
         return (res64[1] / ((double)res64[0] * 32)) * 100;
      break;
   default:
      return sm30_hw_metric_calc_result(hq, res64);
   }
   return 0;
}

static bool
nvc0_hw_metric_get_query_result(struct nvc0_context *nvc0,
                                struct nvc0_hw_query *hq, bool wait,
                                union pipe_query_result *result)
{
   struct nvc0_hw_metric_query *hmq = nvc0_hw_metric_query(hq);
   struct nvc0_screen *screen = nvc0->screen;
   struct nouveau_device *dev = screen->base.device;
   union pipe_query_result results[8] = {};
   uint64_t res64[8] = {};
   uint64_t value = 0;
   bool ret = false;
   unsigned i;

   for (i = 0; i < hmq->num_queries; i++) {
      ret = hmq->queries[i]->funcs->get_query_result(nvc0, hmq->queries[i],
                                                     wait, &results[i]);
      if (!ret)
         return ret;
      res64[i] = *(uint64_t *)&results[i];
   }

   switch (screen->base.class_3d) {
   case GM200_3D_CLASS:
   case GM107_3D_CLASS:
   case NVF0_3D_CLASS:
      value = sm35_hw_metric_calc_result(hq, res64);
      break;
   case NVE4_3D_CLASS:
      value = sm30_hw_metric_calc_result(hq, res64);
      break;
   default:
      if (dev->chipset == 0xc0 || dev->chipset == 0xc8)
         value = sm20_hw_metric_calc_result(hq, res64);
      else
         value = sm21_hw_metric_calc_result(hq, res64);
      break;
   }

   *(uint64_t *)result = value;
   return ret;
}

static const struct nvc0_hw_query_funcs hw_metric_query_funcs = {
   .destroy_query = nvc0_hw_metric_destroy_query,
   .begin_query = nvc0_hw_metric_begin_query,
   .end_query = nvc0_hw_metric_end_query,
   .get_query_result = nvc0_hw_metric_get_query_result,
};

struct nvc0_hw_query *
nvc0_hw_metric_create_query(struct nvc0_context *nvc0, unsigned type)
{
   const struct nvc0_hw_metric_query_cfg *cfg;
   struct nvc0_hw_metric_query *hmq;
   struct nvc0_hw_query *hq;
   unsigned i;

   if (type < NVC0_HW_METRIC_QUERY(0) || type > NVC0_HW_METRIC_QUERY_LAST)
      return NULL;

   hmq = CALLOC_STRUCT(nvc0_hw_metric_query);
   if (!hmq)
      return NULL;

   hq = &hmq->base;
   hq->funcs = &hw_metric_query_funcs;
   hq->base.type = type;

   cfg = nvc0_hw_metric_query_get_cfg(nvc0, hq);

   for (i = 0; i < cfg->num_queries; i++) {
      hmq->queries[i] = nvc0_hw_sm_create_query(nvc0, cfg->queries[i]);
      if (!hmq->queries[i]) {
         nvc0_hw_metric_destroy_query(nvc0, hq);
         return NULL;
      }
      hmq->num_queries++;
   }

   return hq;
}

int
nvc0_hw_metric_get_driver_query_info(struct nvc0_screen *screen, unsigned id,
                                     struct pipe_driver_query_info *info)
{
   int count = 0;

   if (screen->base.drm->version >= 0x01000101) {
      if (screen->compute)
         count = nvc0_hw_metric_get_num_queries(screen);
   }

   if (!info)
      return count;

   if (id < count) {
      if (screen->compute) {
         if (screen->base.class_3d <= GM200_3D_CLASS) {
            const struct nvc0_hw_metric_query_cfg **queries =
               nvc0_hw_metric_get_queries(screen);
            const struct nvc0_hw_metric_cfg *cfg =
               nvc0_hw_metric_get_cfg(queries[id]->type);

            info->name = cfg->name;
            info->query_type = NVC0_HW_METRIC_QUERY(queries[id]->type);
            info->type = cfg->type;
            info->group_id = NVC0_HW_METRIC_QUERY_GROUP;
            return 1;
         }
      }
   }
   return 0;
}
