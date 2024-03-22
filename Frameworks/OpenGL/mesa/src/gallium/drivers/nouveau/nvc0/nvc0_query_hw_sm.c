/*
 * Copyright 2011 Christoph Bumiller
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

#define NVC0_PUSH_EXPLICIT_SPACE_CHECKING

#include "nvc0/nvc0_context.h"
#include "nvc0/nvc0_query_hw_sm.h"

#include "nv_object.xml.h"
#include "nvc0/nve4_compute.xml.h"
#include "nvc0/nvc0_compute.xml.h"

/* NOTE: intentionally using the same names as NV */
#define _Q(t, n, d) { NVC0_HW_SM_QUERY_##t, n, d }
static const struct {
   unsigned type;
   const char *name;
   const char *desc;
} nvc0_hw_sm_queries[] = {
   _Q(ACTIVE_CTAS,
      "active_ctas",
      "Accumulated number of active blocks per cycle. For every cycle it "
      "increments by the number of active blocks in the cycle which can be in "
      "the range 0 to 32."),

   _Q(ACTIVE_CYCLES,
      "active_cycles",
      "Number of cycles a multiprocessor has at least one active warp"),

   _Q(ACTIVE_WARPS,
      "active_warps",
      "Accumulated number of active warps per cycle. For every cycle it "
      "increments by the number of active warps in the cycle which can be in "
      "the range 0 to 64"),

   _Q(ATOM_CAS_COUNT,
      "atom_cas_count",
      "Number of warps executing atomic compare and swap operations. Increments "
      "by one if at least one thread in a warp executes the instruction."),

   _Q(ATOM_COUNT,
      "atom_count",
      "Number of warps executing atomic reduction operations. Increments by one "
      "if at least one thread in a warp executes the instruction"),

   _Q(BRANCH,
      "branch",
      "Number of branch instructions executed per warp on a multiprocessor"),

   _Q(DIVERGENT_BRANCH,
      "divergent_branch",
      "Number of divergent branches within a warp. This counter will be "
      "incremented by one if at least one thread in a warp diverges (that is, "
      "follows a different execution path) via a conditional branch"),

   _Q(GLD_REQUEST,
      "gld_request",
      "Number of executed load instructions where the state space is not "
      "specified and hence generic addressing is used, increments per warp on a "
      "multiprocessor. It can include the load operations from global,local and "
      "shared state space"),

   _Q(GLD_MEM_DIV_REPLAY,
      "global_ld_mem_divergence_replays",
      "Number of instruction replays for global memory loads. Instruction is "
      "replayed if the instruction is accessing more than one cache line of "
      "128 bytes. For each extra cache line access the counter is incremented "
      "by 1"),

   _Q(GLOBAL_ATOM_CAS,
      "global_atom_cas",
      "Number of ATOM.CAS instructions executed per warp."),

   _Q(GLOBAL_LD,
      "global_load",
      "Number of executed load instructions where state space is specified as "
      "global, increments per warp on a multiprocessor."),

   _Q(GLOBAL_ST,
      "global_store",
      "Number of executed store instructions where state space is specified as "
      "global, increments per warp on a multiprocessor."),

   _Q(GST_TRANSACTIONS,
      "global_store_transaction",
      "Number of global store transactions. Increments by 1 per transaction. "
      "Transaction can be 32/64/96/128B"),

   _Q(GST_MEM_DIV_REPLAY,
      "global_st_mem_divergence_replays",
      "Number of instruction replays for global memory stores. Instruction is "
      "replayed if the instruction is accessing more than one cache line of "
      "128 bytes. For each extra cache line access the counter is incremented "
      "by 1"),

   _Q(GRED_COUNT,
      "gred_count",
      "Number of warps executing reduction operations on global memory. "
      "Increments by one if at least one thread in a warp executes the "
      "instruction"),

   _Q(GST_REQUEST,
      "gst_request",
      "Number of executed store instructions where the state space is not "
      "specified and hence generic addressing is used, increments per warp on a "
      "multiprocessor. It can include the store operations to global,local and "
      "shared state space"),

   _Q(INST_EXECUTED,
      "inst_executed",
      "Number of instructions executed, do not include replays"),

   _Q(INST_ISSUED,
      "inst_issued",
      "Number of instructions issued including replays"),

   _Q(INST_ISSUED0,
      "inst_issued0",
      "Number of cycles that did not issue any instruction, increments per "
      "warp."),

   _Q(INST_ISSUED1,
      "inst_issued1",
      "Number of single instruction issued per cycle"),

   _Q(INST_ISSUED2,
      "inst_issued2",
      "Number of dual instructions issued per cycle"),

   _Q(INST_ISSUED1_0,
      "inst_issued1_0",
      "Number of single instruction issued per cycle in pipeline 0"),

   _Q(INST_ISSUED1_1,
      "inst_issued1_1",
      "Number of single instruction issued per cycle in pipeline 1"),

   _Q(INST_ISSUED2_0,
      "inst_issued2_0",
      "Number of dual instructions issued per cycle in pipeline 0"),

   _Q(INST_ISSUED2_1,
      "inst_issued2_1",
      "Number of dual instructions issued per cycle in pipeline 1"),

   _Q(L1_GLD_HIT,
      "l1_global_load_hit",
      "Number of cache lines that hit in L1 cache for global memory load "
      "accesses. In case of perfect coalescing this increments by 1,2, and 4 for "
      "32, 64 and 128 bit accesses by a warp respectively"),

   _Q(L1_GLD_MISS,
      "l1_global_load_miss",
      "Number of cache lines that miss in L1 cache for global memory load "
      "accesses. In case of perfect coalescing this increments by 1,2, and 4 for "
      "32, 64 and 128 bit accesses by a warp respectively"),

   _Q(L1_GLD_TRANSACTIONS,
      "__l1_global_load_transactions",
      "Number of global load transactions from L1 cache. Increments by 1 per "
      "transaction. Transaction can be 32/64/96/128B"),

   _Q(L1_GST_TRANSACTIONS,
      "__l1_global_store_transactions",
      "Number of global store transactions from L1 cache. Increments by 1 per "
      "transaction. Transaction can be 32/64/96/128B"),

   _Q(L1_LOCAL_LD_HIT,
      "l1_local_load_hit",
      "Number of cache lines that hit in L1 cache for local memory load "
      "accesses. In case of perfect coalescing this increments by 1,2, and 4 for "
      "32, 64 and 128 bit accesses by a warp respectively"),

   _Q(L1_LOCAL_LD_MISS,
      "l1_local_load_miss",
      "Number of cache lines that miss in L1 cache for local memory load "
      "accesses. In case of perfect coalescing this increments by 1,2, and 4 for "
      "32, 64 and 128 bit accesses by a warp respectively"),

   _Q(L1_LOCAL_ST_HIT,
      "l1_local_store_hit",
      "Number of cache lines that hit in L1 cache for local memory store "
      "accesses. In case of perfect coalescing this increments by 1,2, and 4 for "
      "32, 64 and 128 bit accesses by a warp respectively"),

   _Q(L1_LOCAL_ST_MISS,
      "l1_local_store_miss",
      "Number of cache lines that miss in L1 cache for local memory store "
      "accesses. In case of perfect coalescing this increments by 1,2, and 4 for "
      "32,64 and 128 bit accesses by a warp respectively"),

   _Q(L1_SHARED_LD_TRANSACTIONS,
      "l1_shared_load_transactions",
      "Number of shared load transactions. Increments by 1 per transaction. "
      "Transaction can be 32/64/96/128B"),

   _Q(L1_SHARED_ST_TRANSACTIONS,
      "l1_shared_store_transactions",
      "Number of shared store transactions. Increments by 1 per transaction. "
      "Transaction can be 32/64/96/128B"),

   _Q(LOCAL_LD,
      "local_load",
      "Number of executed load instructions where state space is specified as "
      "local, increments per warp on a multiprocessor"),

   _Q(LOCAL_LD_TRANSACTIONS,
      "local_load_transactions",
      "Number of local load transactions from L1 cache. Increments by 1 per "
      "transaction. Transaction can be 32/64/96/128B"),

   _Q(LOCAL_ST,
      "local_store",
      "Number of executed store instructions where state space is specified as "
      "local, increments per warp on a multiprocessor"),

   _Q(LOCAL_ST_TRANSACTIONS,
      "local_store_transactions",
      "Number of local store transactions to L1 cache. Increments by 1 per "
      "transaction. Transaction can be 32/64/96/128B."),

   _Q(NOT_PRED_OFF_INST_EXECUTED,
      "not_predicated_off_thread_inst_executed",
      "Number of not predicated off instructions executed by all threads, does "
      "not include replays. For each instruction it increments by the number of "
      "threads that execute this instruction"),

   _Q(PROF_TRIGGER_0,
      "prof_trigger_00",
      "User profiled generic trigger that can be inserted in any place of the "
      "code to collect the related information. Increments per warp."),

   _Q(PROF_TRIGGER_1,
      "prof_trigger_01",
      "User profiled generic trigger that can be inserted in any place of the "
      "code to collect the related information. Increments per warp."),

   _Q(PROF_TRIGGER_2,
      "prof_trigger_02",
      "User profiled generic trigger that can be inserted in any place of the "
      "code to collect the related information. Increments per warp."),

   _Q(PROF_TRIGGER_3,
      "prof_trigger_03",
      "User profiled generic trigger that can be inserted in any place of the "
      "code to collect the related information. Increments per warp."),

   _Q(PROF_TRIGGER_4,
      "prof_trigger_04",
      "User profiled generic trigger that can be inserted in any place of the "
      "code to collect the related information. Increments per warp."),

   _Q(PROF_TRIGGER_5,
      "prof_trigger_05",
      "User profiled generic trigger that can be inserted in any place of the "
      "code to collect the related information. Increments per warp."),

   _Q(PROF_TRIGGER_6,
      "prof_trigger_06",
      "User profiled generic trigger that can be inserted in any place of the "
      "code to collect the related information. Increments per warp."),

   _Q(PROF_TRIGGER_7,
      "prof_trigger_07",
      "User profiled generic trigger that can be inserted in any place of the "
      "code to collect the related information. Increments per warp."),

   _Q(SHARED_ATOM,
      "shared_atom",
      "Number of ATOMS instructions executed per warp."),

   _Q(SHARED_ATOM_CAS,
      "shared_atom_cas",
      "Number of ATOMS.CAS instructions executed per warp."),

   _Q(SHARED_LD,
      "shared_load",
      "Number of executed load instructions where state space is specified as "
      "shared, increments per warp on a multiprocessor"),

   _Q(SHARED_LD_BANK_CONFLICT,
      "shared_load_bank_conflict",
      "Number of shared load bank conflict generated when the addresses for "
      "two or more shared memory load requests fall in the same memory bank."),

   _Q(SHARED_LD_REPLAY,
      "shared_load_replay",
      "Replays caused due to shared load bank conflict (when the addresses for "
      "two or more shared memory load requests fall in the same memory bank) or "
      "when there is no conflict but the total number of words accessed by all "
      "threads in the warp executing that instruction exceed the number of words "
      "that can be loaded in one cycle (256 bytes)"),

   _Q(SHARED_LD_TRANSACTIONS,
      "shared_ld_transactions",
      "Number of transactions for shared load accesses. Maximum transaction "
      "size in maxwell is 128 bytes, any warp accessing more that 128 bytes "
      "will cause multiple transactions for a shared load instruction. This "
      "also includes extra transactions caused by shared bank conflicts."),

   _Q(SHARED_ST,
      "shared_store",
      "Number of executed store instructions where state space is specified as "
      "shared, increments per warp on a multiprocessor"),

   _Q(SHARED_ST_BANK_CONFLICT,
      "shared_store_bank_conflict",
      "Number of shared store bank conflict generated when the addresses for "
      "two or more shared memory store requests fall in the same memory bank."),

   _Q(SHARED_ST_REPLAY,
      "shared_store_replay",
      "Replays caused due to shared store bank conflict (when the addresses for "
      "two or more shared memory store requests fall in the same memory bank) or "
      "when there is no conflict but the total number of words accessed by all "
      "threads in the warp executing that instruction exceed the number of words "
      "that can be stored in one cycle"),

   _Q(SHARED_ST_TRANSACTIONS,
      "shared_st_transactions",
      "Number of transactions for shared store accesses. Maximum transaction "
      "size in maxwell is 128 bytes, any warp accessing more that 128 bytes "
      "will cause multiple transactions for a shared store instruction. This "
      "also includes extra transactions caused by shared bank conflicts."),

   _Q(SM_CTA_LAUNCHED,
      "sm_cta_launched",
      "Number of thread blocks launched on a multiprocessor"),

   _Q(THREADS_LAUNCHED,
      "threads_launched",
      "Number of threads launched on a multiprocessor"),

   _Q(TH_INST_EXECUTED,
      "thread_inst_executed",
      "Number of instructions executed by all threads, does not include "
      "replays. For each instruction it increments by the number of threads in "
      "the warp that execute the instruction"),

   _Q(TH_INST_EXECUTED_0,
      "thread_inst_executed_0",
      "Number of instructions executed by all threads, does not include "
      "replays. For each instruction it increments by the number of threads in "
      "the warp that execute the instruction in pipeline 0"),

   _Q(TH_INST_EXECUTED_1,
      "thread_inst_executed_1",
      "Number of instructions executed by all threads, does not include "
      "replays. For each instruction it increments by the number of threads in "
      "the warp that execute the instruction in pipeline 1"),

   _Q(TH_INST_EXECUTED_2,
      "thread_inst_executed_2",
      "Number of instructions executed by all threads, does not include "
      "replays. For each instruction it increments by the number of threads in "
      "the warp that execute the instruction in pipeline 2"),

   _Q(TH_INST_EXECUTED_3,
      "thread_inst_executed_3",
      "Number of instructions executed by all threads, does not include "
      "replays. For each instruction it increments by the number of threads in "
      "the warp that execute the instruction in pipeline 3"),

   _Q(UNCACHED_GLD_TRANSACTIONS,
      "uncached_global_load_transaction",
      "Number of uncached global load transactions. Increments by 1 per "
      "transaction. Transaction can be 32/64/96/128B."),

   _Q(WARPS_LAUNCHED,
      "warps_launched",
      "Number of warps launched on a multiprocessor"),
};

#undef _Q

static inline const char *
nvc0_hw_sm_query_get_name(unsigned query_type)
{
   unsigned i;

   for (i = 0; i < ARRAY_SIZE(nvc0_hw_sm_queries); i++) {
      if (nvc0_hw_sm_queries[i].type == query_type)
         return nvc0_hw_sm_queries[i].name;
   }
   assert(0);
   return NULL;
}

/* === PERFORMANCE MONITORING COUNTERS for NVE4+ === */

/* Code to read out MP counters: They are accessible via mmio, too, but let's
 * just avoid mapping registers in userspace. We'd have to know which MPs are
 * enabled/present, too, and that information is not presently exposed.
 * We could add a kernel interface for it, but reading the counters like this
 * has the advantage of being async (if get_result isn't called immediately).
 */
static const uint64_t nve4_read_hw_sm_counters_code[] =
{
   /* sched 0x20 0x20 0x20 0x20 0x20 0x20 0x20
    * mov b32 $r8 $tidx
    * mov b32 $r12 $physid
    * mov b32 $r0 $pm0
    * mov b32 $r1 $pm1
    * mov b32 $r2 $pm2
    * mov b32 $r3 $pm3
    * mov b32 $r4 $pm4
    * sched 0x20 0x20 0x23 0x04 0x20 0x04 0x2b
    * mov b32 $r5 $pm5
    * mov b32 $r6 $pm6
    * mov b32 $r7 $pm7
    * set $p0 0x1 eq u32 $r8 0x0
    * mov b32 $r10 c7[0x6a0]
    * ext u32 $r8 $r12 0x414
    * mov b32 $r11 c7[0x6a4]
    * sched 0x04 0x2e 0x04 0x20 0x20 0x28 0x04
    * ext u32 $r9 $r12 0x208
    * (not $p0) exit
    * set $p1 0x1 eq u32 $r9 0x0
    * mul $r8 u32 $r8 u32 96
    * mul $r12 u32 $r9 u32 16
    * mul $r13 u32 $r9 u32 4
    * add b32 $r9 $r8 $r13
    * sched 0x28 0x04 0x2c 0x04 0x2c 0x04 0x2c
    * add b32 $r8 $r8 $r12
    * mov b32 $r12 $r10
    * add b32 $r10 $c $r10 $r8
    * mov b32 $r13 $r11
    * add b32 $r11 $r11 0x0 $c
    * add b32 $r12 $c $r12 $r9
    * st b128 wt g[$r10d] $r0q
    * sched 0x4 0x2c 0x20 0x04 0x2e 0x00 0x00
    * mov b32 $r0 c7[0x6a8]
    * add b32 $r13 $r13 0x0 $c
    * $p1 st b128 wt g[$r12d+0x40] $r4q
    * st b32 wt g[$r12d+0x50] $r0
    * exit */
   0x2202020202020207ULL,
   0x2c00000084021c04ULL,
   0x2c0000000c031c04ULL,
   0x2c00000010001c04ULL,
   0x2c00000014005c04ULL,
   0x2c00000018009c04ULL,
   0x2c0000001c00dc04ULL,
   0x2c00000020011c04ULL,
   0x22b0420042320207ULL,
   0x2c00000024015c04ULL,
   0x2c00000028019c04ULL,
   0x2c0000002c01dc04ULL,
   0x190e0000fc81dc03ULL,
   0x28005c1a80029de4ULL,
   0x7000c01050c21c03ULL,
   0x28005c1a9002dde4ULL,
   0x204282020042e047ULL,
   0x7000c00820c25c03ULL,
   0x80000000000021e7ULL,
   0x190e0000fc93dc03ULL,
   0x1000000180821c02ULL,
   0x1000000040931c02ULL,
   0x1000000010935c02ULL,
   0x4800000034825c03ULL,
   0x22c042c042c04287ULL,
   0x4800000030821c03ULL,
   0x2800000028031de4ULL,
   0x4801000020a29c03ULL,
   0x280000002c035de4ULL,
   0x0800000000b2dc42ULL,
   0x4801000024c31c03ULL,
   0x9400000000a01fc5ULL,
   0x200002e04202c047ULL,
   0x28005c1aa0001de4ULL,
   0x0800000000d35c42ULL,
   0x9400000100c107c5ULL,
   0x9400000140c01f85ULL,
   0x8000000000001de7ULL
};

static const uint64_t nvf0_read_hw_sm_counters_code[] =
{
   /* Same kernel as GK104 */
   0x0880808080808080ULL,
   0x86400000109c0022ULL,
   0x86400000019c0032ULL,
   0x86400000021c0002ULL,
   0x86400000029c0006ULL,
   0x86400000031c000aULL,
   0x86400000039c000eULL,
   0x86400000041c0012ULL,
   0x08ac1080108c8080ULL,
   0x86400000049c0016ULL,
   0x86400000051c001aULL,
   0x86400000059c001eULL,
   0xdb201c007f9c201eULL,
   0x64c03ce0d41c002aULL,
   0xc00000020a1c3021ULL,
   0x64c03ce0d49c002eULL,
   0x0810a0808010b810ULL,
   0xc0000001041c3025ULL,
   0x180000000020003cULL,
   0xdb201c007f9c243eULL,
   0xc1c00000301c2021ULL,
   0xc1c00000081c2431ULL,
   0xc1c00000021c2435ULL,
   0xe0800000069c2026ULL,
   0x08b010b010b010a0ULL,
   0xe0800000061c2022ULL,
   0xe4c03c00051c0032ULL,
   0xe0840000041c282aULL,
   0xe4c03c00059c0036ULL,
   0xe08040007f9c2c2eULL,
   0xe0840000049c3032ULL,
   0xfe800000001c2800ULL,
   0x080000b81080b010ULL,
   0x64c03ce0d51c0002ULL,
   0xe08040007f9c3436ULL,
   0xfe80000020043010ULL,
   0xfc800000281c3000ULL,
   0x18000000001c003cULL,
};

static const uint64_t gm107_read_hw_sm_counters_code[] =
{
   0x001d0400e4200701ULL, /* sched (st 0x1 wr 0x0) (st 0x1 wr 0x1) (st 0x1 wr 0x2)  */
   0xf0c8000002170008ULL, /* mov $r8 $tidx                                          */
   0xf0c800000037000cULL, /* mov $r12 $virtid                                       */
   0xf0c8000000470000ULL, /* mov $r0 $pm0                                           */
   0x001e8400f0200761ULL, /* sched (st 0x1 wr 0x3) (st 0x1 wr 0x4) (st 0x1 wr 0x5)  */
   0xf0c8000000570001ULL, /* mov $r1 $pm1                                           */
   0xf0c8000000670002ULL, /* mov $r2 $pm2                                           */
   0xf0c8000000770003ULL, /* mov $r3 $pm3                                           */
   0x001e8400f42007a1ULL, /* sched (st 0x1 wr 0x5) (st 0x1 wr 0x5) (st 0x1 wr 0x5)  */
   0xf0c8000000870004ULL, /* mov $r4 $pm4                                           */
   0xf0c8000000970005ULL, /* mov $r5 $pm5                                           */
   0xf0c8000000a70006ULL, /* mov $r6 $pm6                                           */
   0x001f8401fc2007a1ULL, /* sched (st 0x1 wr 0x5) (st 0x1 wt 0x1) (st 0x1)         */
   0xf0c8000000b70007ULL, /* mov $r7 $pm7                                           */
   0x5b6403800087ff07ULL, /* isetp eq u32 and $p0 0x1 0x0 $r8 0x1                   */
   0x4c98079c1a87000aULL, /* mov $r10 c7[0x6a0] 0xf                                 */
   0x001fa400fc2017e1ULL, /* sched (st 0x1 wt 0x2) (st 0x1) (st 0x9)                */
   0x3800000091470c08ULL, /* bfe u32 $r8 $r12 0x914                                 */
   0x4c98079c1a97000bULL, /* mov $r11 c7[0x6a4] 0xf                                 */
   0x3800000020870c09ULL, /* bfe u32 $r9 $r12 0x208                                 */
   0x001c1800fc2007edULL, /* sched (st 0xd) (st 0x1) (st 0x6 wr 0x0)                */
   0xe30000000008000fULL, /* not $p0 exit                                           */
   0x5b6403800097ff0fULL, /* isetp eq u32 and $p1 0x1 0x0 $r9 0x1                   */
   0x3838000006070808ULL, /* imul u32 u32 $r8 $r8 0x60                              */
   0x003f8400e0c00726ULL, /* sched (st 0x6 wr 0x1) (st 0x6 wr 0x0) (st 0x1 wt 0x1)  */
   0x383800000107090cULL, /* imul u32 u32 $r12 $r9 0x10                             */
   0x383800000047090dULL, /* imul u32 u32 $r13 $r9 0x4                              */
   0x5c10000000d70809ULL, /* iadd $r9 $r8 $r13                                      */
   0x001f8400fcc017e1ULL, /* sched (st 0x1 wt 0x2) (st 0x6) (st 0x1)                */
   0x5c10000000c70808ULL, /* iadd $r8 $r8 $r12                                      */
   0x5c98078000a7000cULL, /* mov $r12 $r10 0xf                                      */
   0x5c10800000870a0aULL, /* iadd cc $r10 $r10 $r8                                  */
   0x001f8400fc2007e6ULL, /* sched (st 0x6) (st 0x1) (st 0x1)                       */
   0x5c98078000b7000dULL, /* mov $r13 $r11 0xf                                      */
   0x5c1008000ff70b0bULL, /* iadd x $r11 $r11 0x0                                   */
   0x5c10800000970c0cULL, /* iadd cc $r12 $r12 $r9                                  */
   0x003f983c1c4007e1ULL, /* sched (st 0x1) (st 0x2 rd 0x0 wt 0x3c) (st 0x6 wt 0x1) */
   0x5c1008000ff70d0dULL, /* iadd x $r13 $r13 0x0                                   */
   0xbfd0000000070a00ULL, /* st e wt b128 g[$r10] $r0 0x1                           */
   0x4c98079c1aa70000ULL, /* mov $r0 c7[0x6a8] 0xf                                  */
   0x001fbc00fc2007e6ULL, /* sched (st 0x1) (st 0x1) (st 0xf)                       */
   0xbfd0000004010c04ULL, /* $p1 st e wt b128 g[$r12+0x40] $r4 0x1                  */
   0xbf90000005070c00ULL, /* st e wt b32 g[$r12+0x50] $r0 0x1                       */
   0xe30000000007000fULL, /* exit                                                   */
};

/* For simplicity, we will allocate as many group slots as we allocate counter
 * slots. This means that a single counter which wants to source from 2 groups
 * will have to be declared as using 2 counter slots. This shouldn't really be
 * a problem because such queries don't make much sense ... (unless someone is
 * really creative).
 */
struct nvc0_hw_sm_counter_cfg
{
   uint32_t func    : 16; /* mask or 4-bit logic op (depending on mode) */
   uint32_t mode    : 4;  /* LOGOP,B6,LOGOP_B6(_PULSE) */
   uint32_t sig_dom : 1;  /* if 0, MP_PM_A (per warp-sched), if 1, MP_PM_B */
   uint32_t sig_sel : 8;  /* signal group */
   uint32_t src_mask;     /* mask for signal selection (only for NVC0:NVE4) */
   uint32_t src_sel;      /* signal selection for up to 4 sources */
};

struct nvc0_hw_sm_query_cfg
{
   unsigned type;
   struct nvc0_hw_sm_counter_cfg ctr[8];
   uint8_t num_counters;
   uint8_t norm[2]; /* normalization num,denom */
};

#define _CA(f, m, g, s) { f, NVE4_COMPUTE_MP_PM_FUNC_MODE_##m, 0, g, 0, s }
#define _CB(f, m, g, s) { f, NVE4_COMPUTE_MP_PM_FUNC_MODE_##m, 1, g, 0, s }
#define _Q(n, c) [NVE4_HW_SM_QUERY_##n] = c

/* ==== Compute capability 3.0 (GK104:GK110) ==== */
static const struct nvc0_hw_sm_query_cfg
sm30_active_cycles =
{
   .type         = NVC0_HW_SM_QUERY_ACTIVE_CYCLES,
   .ctr[0]       = _CB(0x0001, B6, 0x02, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_active_warps =
{
   .type         = NVC0_HW_SM_QUERY_ACTIVE_WARPS,
   .ctr[0]       = _CB(0x003f, B6, 0x02, 0x31483104),
   .num_counters = 1,
   .norm         = { 2, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_atom_cas_count =
{
   .type         = NVC0_HW_SM_QUERY_ATOM_CAS_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x1c, 0x000000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_atom_count =
{
   .type         = NVC0_HW_SM_QUERY_ATOM_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x1c, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_branch =
{
   .type         = NVC0_HW_SM_QUERY_BRANCH,
   .ctr[0]       = _CA(0x0001, B6, 0x1c, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_divergent_branch =
{
   .type         = NVC0_HW_SM_QUERY_DIVERGENT_BRANCH,
   .ctr[0]       = _CA(0x0001, B6, 0x1c, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_gld_request =
{
   .type         = NVC0_HW_SM_QUERY_GLD_REQUEST,
   .ctr[0]       = _CA(0x0001, B6, 0x1b, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_gld_mem_div_replay =
{
   .type         = NVC0_HW_SM_QUERY_GLD_MEM_DIV_REPLAY,
   .ctr[0]       = _CB(0x0001, B6, 0x08, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_gst_transactions =
{
   .type         = NVC0_HW_SM_QUERY_GST_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x11, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_gst_mem_div_replay =
{
   .type         = NVC0_HW_SM_QUERY_GST_MEM_DIV_REPLAY,
   .ctr[0]       = _CB(0x0001, B6, 0x08, 0x00000014),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_gred_count =
{
   .type         = NVC0_HW_SM_QUERY_GRED_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x1c, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_gst_request =
{
   .type         = NVC0_HW_SM_QUERY_GST_REQUEST,
   .ctr[0]       = _CA(0x0001, B6, 0x1b, 0x00000014),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_INST_EXECUTED,
   .ctr[0]       = _CA(0x0003, B6, 0x04, 0x00000398),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_inst_issued1 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED1,
   .ctr[0]       = _CA(0x0001, B6, 0x05, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_inst_issued2 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED2,
   .ctr[0]       = _CA(0x0001, B6, 0x05, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_gld_hit =
{
   .type         = NVC0_HW_SM_QUERY_L1_GLD_HIT,
   .ctr[0]       = _CB(0x0001, B6, 0x10, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_gld_miss =
{
   .type         = NVC0_HW_SM_QUERY_L1_GLD_MISS,
   .ctr[0]       = _CB(0x0001, B6, 0x10, 0x00000014),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_gld_transactions =
{
   .type         = NVC0_HW_SM_QUERY_L1_GLD_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x0f, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_gst_transactions =
{
   .type         = NVC0_HW_SM_QUERY_L1_GST_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x0f, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_local_ld_hit =
{
   .type         = NVC0_HW_SM_QUERY_L1_LOCAL_LD_HIT,
   .ctr[0]       = _CB(0x0001, B6, 0x10, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_local_ld_miss =
{
   .type         = NVC0_HW_SM_QUERY_L1_LOCAL_LD_MISS,
   .ctr[0]       = _CB(0x0001, B6, 0x10, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_local_st_hit =
{
   .type         = NVC0_HW_SM_QUERY_L1_LOCAL_ST_HIT,
   .ctr[0]       = _CB(0x0001, B6, 0x10, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_local_st_miss =
{
   .type         = NVC0_HW_SM_QUERY_L1_LOCAL_ST_MISS,
   .ctr[0]       = _CB(0x0001, B6, 0x10, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_shared_ld_transactions =
{
   .type         = NVC0_HW_SM_QUERY_L1_SHARED_LD_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x0e, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_l1_shared_st_transactions =
{
   .type         = NVC0_HW_SM_QUERY_L1_SHARED_ST_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x0e, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_local_ld =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_LD,
   .ctr[0]       = _CA(0x0001, B6, 0x1b, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_local_ld_transactions =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_LD_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x0e, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_local_st =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_ST,
   .ctr[0]       = _CA(0x0001, B6, 0x1b, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_local_st_transactions =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_ST_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x0e, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_prof_trigger_0 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_0,
   .ctr[0]       = _CA(0x0001, B6, 0x01, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_prof_trigger_1 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_1,
   .ctr[0]       = _CA(0x0001, B6, 0x01, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_prof_trigger_2 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_2,
   .ctr[0]       = _CA(0x0001, B6, 0x01, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_prof_trigger_3 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_3,
   .ctr[0]       = _CA(0x0001, B6, 0x01, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_prof_trigger_4 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_4,
   .ctr[0]       = _CA(0x0001, B6, 0x01, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_prof_trigger_5 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_5,
   .ctr[0]       = _CA(0x0001, B6, 0x01, 0x00000014),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_prof_trigger_6 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_6,
   .ctr[0]       = _CA(0x0001, B6, 0x01, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_prof_trigger_7 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_7,
   .ctr[0]       = _CA(0x0001, B6, 0x01, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_shared_ld =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_LD,
   .ctr[0]       = _CA(0x0001, B6, 0x1b, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_shared_ld_replay =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_LD_REPLAY,
   .ctr[0]       = _CB(0x0001, B6, 0x08, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_shared_st =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ST,
   .ctr[0]       = _CA(0x0001, B6, 0x1b, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_shared_st_replay =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ST_REPLAY,
   .ctr[0]       = _CB(0x0001, B6, 0x08, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_sm_cta_launched =
{
   .type         = NVC0_HW_SM_QUERY_SM_CTA_LAUNCHED,
   .ctr[0]       = _CB(0x0001, B6, 0x02, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_threads_launched =
{
   .type         = NVC0_HW_SM_QUERY_THREADS_LAUNCHED,
   .ctr[0]       = _CA(0x003f, B6, 0x03, 0x398a4188),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_uncached_gld_transactions =
{
   .type         = NVC0_HW_SM_QUERY_UNCACHED_GLD_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x11, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm30_warps_launched =
{
   .type         = NVC0_HW_SM_QUERY_WARPS_LAUNCHED,
   .ctr[0]       = _CA(0x0001, B6, 0x03, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

/* NOTES:
 * active_warps: bit 0 alternates btw 0 and 1 for odd nr of warps
 * inst_executed etc.: we only count a single warp scheduler
 */
static const struct nvc0_hw_sm_query_cfg *sm30_hw_sm_queries[] =
{
   &sm30_active_cycles,
   &sm30_active_warps,
   &sm30_atom_cas_count,
   &sm30_atom_count,
   &sm30_branch,
   &sm30_divergent_branch,
   &sm30_gld_request,
   &sm30_gld_mem_div_replay,
   &sm30_gst_transactions,
   &sm30_gst_mem_div_replay,
   &sm30_gred_count,
   &sm30_gst_request,
   &sm30_inst_executed,
   &sm30_inst_issued1,
   &sm30_inst_issued2,
   &sm30_l1_gld_hit,
   &sm30_l1_gld_miss,
   &sm30_l1_gld_transactions,
   &sm30_l1_gst_transactions,
   &sm30_l1_local_ld_hit,
   &sm30_l1_local_ld_miss,
   &sm30_l1_local_st_hit,
   &sm30_l1_local_st_miss,
   &sm30_l1_shared_ld_transactions,
   &sm30_l1_shared_st_transactions,
   &sm30_local_ld,
   &sm30_local_ld_transactions,
   &sm30_local_st,
   &sm30_local_st_transactions,
   &sm30_prof_trigger_0,
   &sm30_prof_trigger_1,
   &sm30_prof_trigger_2,
   &sm30_prof_trigger_3,
   &sm30_prof_trigger_4,
   &sm30_prof_trigger_5,
   &sm30_prof_trigger_6,
   &sm30_prof_trigger_7,
   &sm30_shared_ld,
   &sm30_shared_ld_replay,
   &sm30_shared_st,
   &sm30_shared_st_replay,
   &sm30_sm_cta_launched,
   &sm30_threads_launched,
   &sm30_uncached_gld_transactions,
   &sm30_warps_launched,
};

/* ==== Compute capability 3.5 (GK110/GK208) ==== */
static const struct nvc0_hw_sm_query_cfg
sm35_atom_cas_count =
{
   .type         = NVC0_HW_SM_QUERY_ATOM_CAS_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x1a, 0x00000014),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm35_atom_count =
{
   .type         = NVC0_HW_SM_QUERY_ATOM_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x1a, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm35_gred_count =
{
   .type         = NVC0_HW_SM_QUERY_GRED_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x1a, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm35_not_pred_off_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_NOT_PRED_OFF_INST_EXECUTED,
   .ctr[0]       = _CA(0x003f, B6, 0x14, 0x29062080),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm35_shared_ld_replay =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_LD_REPLAY,
   .ctr[0]       = _CB(0xaaaa, LOGOP, 0x13, 0x00000018),
   .ctr[1]       = _CB(0x8888, LOGOP, 0x08, 0x00000151),
   .num_counters = 2,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm35_shared_st_replay =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ST_REPLAY,
   .ctr[0]       = _CB(0xaaaa, LOGOP, 0x13, 0x00000018),
   .ctr[1]       = _CB(0x8888, LOGOP, 0x08, 0x000001d1),
   .num_counters = 2,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm35_th_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_TH_INST_EXECUTED,
   .ctr[0]       = _CA(0x003f, B6, 0x11, 0x29062080),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg *sm35_hw_sm_queries[] =
{
   &sm30_active_cycles,
   &sm30_active_warps,
   &sm35_atom_cas_count,
   &sm35_atom_count,
   &sm30_gld_request,
   &sm30_gld_mem_div_replay,
   &sm30_gst_transactions,
   &sm30_gst_mem_div_replay,
   &sm35_gred_count,
   &sm30_gst_request,
   &sm30_inst_executed,
   &sm30_inst_issued1,
   &sm30_inst_issued2,
   &sm30_l1_gld_hit,
   &sm30_l1_gld_miss,
   &sm30_l1_gld_transactions,
   &sm30_l1_gst_transactions,
   &sm30_l1_local_ld_hit,
   &sm30_l1_local_ld_miss,
   &sm30_l1_local_st_hit,
   &sm30_l1_local_st_miss,
   &sm30_l1_shared_ld_transactions,
   &sm30_l1_shared_st_transactions,
   &sm30_local_ld,
   &sm30_local_ld_transactions,
   &sm30_local_st,
   &sm30_local_st_transactions,
   &sm35_not_pred_off_inst_executed,
   &sm30_prof_trigger_0,
   &sm30_prof_trigger_1,
   &sm30_prof_trigger_2,
   &sm30_prof_trigger_3,
   &sm30_prof_trigger_4,
   &sm30_prof_trigger_5,
   &sm30_prof_trigger_6,
   &sm30_prof_trigger_7,
   &sm30_shared_ld,
   &sm35_shared_ld_replay,
   &sm30_shared_st,
   &sm35_shared_st_replay,
   &sm30_sm_cta_launched,
   &sm35_th_inst_executed,
   &sm30_threads_launched,
   &sm30_uncached_gld_transactions,
   &sm30_warps_launched,
};

/* ==== Compute capability 5.0 (GM107/GM108) ==== */
static const struct nvc0_hw_sm_query_cfg
sm50_active_ctas =
{
   .type         = NVC0_HW_SM_QUERY_ACTIVE_CTAS,
   .ctr[0]       = _CB(0x003f, B6, 0x01, 0x29062080),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_active_cycles =
{
   .type         = NVC0_HW_SM_QUERY_ACTIVE_CYCLES,
   .ctr[0]       = _CB(0x0001, B6, 0x00, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_active_warps =
{
   .type         = NVC0_HW_SM_QUERY_ACTIVE_WARPS,
   .ctr[0]       = _CB(0x003f, B6, 0x00, 0x398a4188),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_atom_count =
{
   .type         = NVC0_HW_SM_QUERY_ATOM_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x14, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_branch =
{
   .type         = NVC0_HW_SM_QUERY_BRANCH,
   .ctr[0]       = _CA(0x0001, B6, 0x1a, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_divergent_branch =
{
   .type         = NVC0_HW_SM_QUERY_DIVERGENT_BRANCH,
   .ctr[0]       = _CA(0x0001, B6, 0x1a, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_global_atom_cas =
{
   .type         = NVC0_HW_SM_QUERY_GLOBAL_ATOM_CAS,
   .ctr[0]       = _CA(0x0001, B6, 0x14, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_global_ld =
{
   .type         = NVC0_HW_SM_QUERY_GLOBAL_LD,
   .ctr[0]       = _CA(0x0001, B6, 0x14, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_global_st =
{
   .type         = NVC0_HW_SM_QUERY_GLOBAL_ST,
   .ctr[0]       = _CA(0x0001, B6, 0x14, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_gred_count =
{
   .type         = NVC0_HW_SM_QUERY_GRED_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x14, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_INST_EXECUTED,
   .ctr[0]       = _CA(0x0003, B6, 0x02, 0x00000398),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_inst_issued0 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED0,
   .ctr[0]       = _CA(0x0001, B6, 0x02, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_inst_issued1 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED1,
   .ctr[0]       = _CA(0x0001, B6, 0x02, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_inst_issued2 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED2,
   .ctr[0]       = _CA(0x0001, B6, 0x02, 0x00000014),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_local_ld =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_LD,
   .ctr[0]       = _CA(0x0001, B6, 0x13, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_local_st =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_ST,
   .ctr[0]       = _CA(0x0001, B6, 0x13, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_not_pred_off_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_NOT_PRED_OFF_INST_EXECUTED,
   .ctr[0]       = _CA(0x003f, B6, 0x05, 0x29062080),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_prof_trigger_0 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_0,
   .ctr[0]       = _CA(0x0001, B6, 0x00, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_prof_trigger_1 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_1,
   .ctr[0]       = _CA(0x0001, B6, 0x00, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_prof_trigger_2 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_2,
   .ctr[0]       = _CA(0x0001, B6, 0x00, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_prof_trigger_3 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_3,
   .ctr[0]       = _CA(0x0001, B6, 0x00, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_prof_trigger_4 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_4,
   .ctr[0]       = _CA(0x0001, B6, 0x00, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_prof_trigger_5 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_5,
   .ctr[0]       = _CA(0x0001, B6, 0x00, 0x00000014),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_prof_trigger_6 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_6,
   .ctr[0]       = _CA(0x0001, B6, 0x00, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_prof_trigger_7 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_7,
   .ctr[0]       = _CA(0x0001, B6, 0x00, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_shared_atom =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ATOM,
   .ctr[0]       = _CA(0x0001, B6, 0x13, 0x00000014),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_shared_atom_cas =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ATOM_CAS,
   .ctr[0]       = _CA(0x0001, B6, 0x13, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_shared_ld =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_LD,
   .ctr[0]       = _CA(0x0001, B6, 0x13, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_shared_ld_bank_conflict =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_LD_BANK_CONFLICT,
   .ctr[0]       = _CB(0x0001, B6, 0x0e, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_shared_ld_transactions =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_LD_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x0e, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_shared_st =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ST,
   .ctr[0]       = _CA(0x0001, B6, 0x13, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_shared_st_bank_conflict =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ST_BANK_CONFLICT,
   .ctr[0]       = _CB(0x0001, B6, 0x0e, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_shared_st_transactions =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ST_TRANSACTIONS,
   .ctr[0]       = _CB(0x0001, B6, 0x0e, 0x0000000c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_sm_cta_launched =
{
   .type         = NVC0_HW_SM_QUERY_SM_CTA_LAUNCHED,
   .ctr[0]       = _CB(0x0001, B6, 0x01, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_th_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_TH_INST_EXECUTED,
   .ctr[0]       = _CA(0x003f, B6, 0x04, 0x29062080),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm50_warps_launched =
{
   .type         = NVC0_HW_SM_QUERY_WARPS_LAUNCHED,
   .ctr[0]       = _CA(0x0001, B6, 0x02, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg *sm50_hw_sm_queries[] =
{
   &sm50_active_ctas,
   &sm50_active_cycles,
   &sm50_active_warps,
   &sm50_atom_count,
   &sm50_branch,
   &sm50_divergent_branch,
   &sm50_global_atom_cas,
   &sm50_global_ld,
   &sm50_global_st,
   &sm50_gred_count,
   &sm50_inst_executed,
   &sm50_inst_issued0,
   &sm50_inst_issued1,
   &sm50_inst_issued2,
   &sm50_local_ld,
   &sm50_local_st,
   &sm50_not_pred_off_inst_executed,
   &sm50_prof_trigger_0,
   &sm50_prof_trigger_1,
   &sm50_prof_trigger_2,
   &sm50_prof_trigger_3,
   &sm50_prof_trigger_4,
   &sm50_prof_trigger_5,
   &sm50_prof_trigger_6,
   &sm50_prof_trigger_7,
   &sm50_shared_atom,
   &sm50_shared_atom_cas,
   &sm50_shared_ld,
   &sm50_shared_ld_bank_conflict,
   &sm50_shared_ld_transactions,
   &sm50_shared_st,
   &sm50_shared_st_bank_conflict,
   &sm50_shared_st_transactions,
   &sm50_sm_cta_launched,
   &sm50_th_inst_executed,
   &sm50_warps_launched,
};

/* ==== Compute capability 5.2 (GM200/GM204/GM206) ==== */
static const struct nvc0_hw_sm_query_cfg
sm52_atom_count =
{
   .type         = NVC0_HW_SM_QUERY_ATOM_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x0a, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_global_atom_cas =
{
   .type         = NVC0_HW_SM_QUERY_GLOBAL_ATOM_CAS,
   .ctr[0]       = _CA(0x0001, B6, 0x0a, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_global_ld =
{
   .type         = NVC0_HW_SM_QUERY_GLOBAL_LD,
   .ctr[0]       = _CA(0x0001, B6, 0x0b, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_global_st =
{
   .type         = NVC0_HW_SM_QUERY_GLOBAL_ST,
   .ctr[0]       = _CA(0x0001, B6, 0x0b, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_gred_count =
{
   .type         = NVC0_HW_SM_QUERY_GRED_COUNT,
   .ctr[0]       = _CA(0x0001, B6, 0x0f, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_INST_EXECUTED,
   .ctr[0]       = _CA(0x0003, B6, 0x03, 0x0000020c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_inst_issued0 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED0,
   .ctr[0]       = _CA(0x0001, B6, 0x03, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_inst_issued1 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED1,
   .ctr[0]       = _CA(0x0001, B6, 0x03, 0x00000004),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_inst_issued2 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED2,
   .ctr[0]       = _CA(0x0001, B6, 0x03, 0x00000008),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_local_ld =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_LD,
   .ctr[0]       = _CA(0x0001, B6, 0x06, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_local_st =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_ST,
   .ctr[0]       = _CA(0x0001, B6, 0x06, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_shared_atom =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ATOM,
   .ctr[0]       = _CA(0x0001, B6, 0x08, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_shared_atom_cas =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ATOM_CAS,
   .ctr[0]       = _CA(0x0001, B6, 0x08, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_shared_ld =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_LD,
   .ctr[0]       = _CA(0x0001, B6, 0x07, 0x00000018),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_shared_st =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ST,
   .ctr[0]       = _CA(0x0001, B6, 0x07, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm52_warps_launched =
{
   .type         = NVC0_HW_SM_QUERY_WARPS_LAUNCHED,
   .ctr[0]       = _CA(0x0001, B6, 0x02, 0x0000001c),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg *sm52_hw_sm_queries[] =
{
   &sm50_active_ctas,
   &sm50_active_cycles,
   &sm50_active_warps,
   &sm52_atom_count,
   &sm50_branch,
   &sm50_divergent_branch,
   &sm52_global_atom_cas,
   &sm52_global_ld,
   &sm52_global_st,
   &sm52_gred_count,
   &sm52_inst_executed,
   &sm52_inst_issued0,
   &sm52_inst_issued1,
   &sm52_inst_issued2,
   &sm52_local_ld,
   &sm52_local_st,
   &sm50_not_pred_off_inst_executed,
   &sm50_prof_trigger_0,
   &sm50_prof_trigger_1,
   &sm50_prof_trigger_2,
   &sm50_prof_trigger_3,
   &sm50_prof_trigger_4,
   &sm50_prof_trigger_5,
   &sm50_prof_trigger_6,
   &sm50_prof_trigger_7,
   &sm52_shared_atom,
   &sm52_shared_atom_cas,
   &sm52_shared_ld,
   &sm50_shared_ld_bank_conflict,
   &sm50_shared_ld_transactions,
   &sm52_shared_st,
   &sm50_shared_st_bank_conflict,
   &sm50_shared_st_transactions,
   &sm50_sm_cta_launched,
   &sm50_th_inst_executed,
   &sm52_warps_launched,
};

#undef _Q
#undef _CA
#undef _CB

/* === PERFORMANCE MONITORING COUNTERS for NVC0:NVE4 === */
/* NOTES:
 * - MP counters on GF100/GF110 (compute capability 2.0) are buggy
 *   because there is a context-switch problem that we need to fix.
 *   Results might be wrong sometimes, be careful!
 */
static const uint64_t nvc0_read_hw_sm_counters_code[] =
{
   /* mov b32 $r8 $tidx
    * mov b32 $r9 $physid
    * mov b32 $r0 $pm0
    * mov b32 $r1 $pm1
    * mov b32 $r2 $pm2
    * mov b32 $r3 $pm3
    * mov b32 $r4 $pm4
    * mov b32 $r5 $pm5
    * mov b32 $r6 $pm6
    * mov b32 $r7 $pm7
    * set $p0 0x1 eq u32 $r8 0x0
    * mov b32 $r10 c15[0x6a0]
    * mov b32 $r11 c15[0x6a4]
    * ext u32 $r8 $r9 0x414
    * (not $p0) exit
    * mul $r8 u32 $r8 u32 48
    * add b32 $r10 $c $r10 $r8
    * add b32 $r11 $r11 0x0 $c
    * mov b32 $r8 c15[0x6a8]
    * st b128 wt g[$r10d+0x00] $r0q
    * st b128 wt g[$r10d+0x10] $r4q
    * st b32 wt g[$r10d+0x20] $r8
    * exit */
   0x2c00000084021c04ULL,
   0x2c0000000c025c04ULL,
   0x2c00000010001c04ULL,
   0x2c00000014005c04ULL,
   0x2c00000018009c04ULL,
   0x2c0000001c00dc04ULL,
   0x2c00000020011c04ULL,
   0x2c00000024015c04ULL,
   0x2c00000028019c04ULL,
   0x2c0000002c01dc04ULL,
   0x190e0000fc81dc03ULL,
   0x28007c1a80029de4ULL,
   0x28007c1a9002dde4ULL,
   0x7000c01050921c03ULL,
   0x80000000000021e7ULL,
   0x10000000c0821c02ULL,
   0x4801000020a29c03ULL,
   0x0800000000b2dc42ULL,
   0x28007c1aa0021de4ULL,
   0x9400000000a01fc5ULL,
   0x9400000040a11fc5ULL,
   0x9400000080a21f85ULL,
   0x8000000000001de7ULL
};

#define _C(f, o, g, m, s) { f, NVC0_COMPUTE_MP_PM_OP_MODE_##o, 0, g, m, s }

/* ==== Compute capability 2.0 (GF100/GF110) ==== */
static const struct nvc0_hw_sm_query_cfg
sm20_active_cycles =
{
   .type         = NVC0_HW_SM_QUERY_ACTIVE_CYCLES,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x11, 0x000000ff, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_active_warps =
{
   .type         = NVC0_HW_SM_QUERY_ACTIVE_WARPS,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x24, 0x000000ff, 0x00000010),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x24, 0x000000ff, 0x00000020),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0x24, 0x000000ff, 0x00000030),
   .ctr[3]       = _C(0xaaaa, LOGOP, 0x24, 0x000000ff, 0x00000040),
   .ctr[4]       = _C(0xaaaa, LOGOP, 0x24, 0x000000ff, 0x00000050),
   .ctr[5]       = _C(0xaaaa, LOGOP, 0x24, 0x000000ff, 0x00000060),
   .num_counters = 6,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_atom_count =
{
   .type         = NVC0_HW_SM_QUERY_ATOM_COUNT,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x63, 0x000000ff, 0x00000030),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_branch =
{
   .type         = NVC0_HW_SM_QUERY_BRANCH,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x1a, 0x000000ff, 0x00000000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x1a, 0x000000ff, 0x00000010),
   .num_counters = 2,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_divergent_branch =
{
   .type         = NVC0_HW_SM_QUERY_DIVERGENT_BRANCH,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x19, 0x000000ff, 0x00000020),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x19, 0x000000ff, 0x00000030),
   .num_counters = 2,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_gld_request =
{
   .type         = NVC0_HW_SM_QUERY_GLD_REQUEST,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x64, 0x000000ff, 0x00000030),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_gred_count =
{
   .type         = NVC0_HW_SM_QUERY_GRED_COUNT,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x63, 0x000000ff, 0x00000040),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_gst_request =
{
   .type         = NVC0_HW_SM_QUERY_GST_REQUEST,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x64, 0x000000ff, 0x00000060),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_INST_EXECUTED,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x2d, 0x0000ffff, 0x00001000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x2d, 0x0000ffff, 0x00001010),
   .num_counters = 2,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_inst_issued =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x27, 0x0000ffff, 0x00007060),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x27, 0x0000ffff, 0x00007070),
   .num_counters = 2,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_local_ld =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_LD,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x64, 0x000000ff, 0x00000020),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_local_st =
{
   .type         = NVC0_HW_SM_QUERY_LOCAL_ST,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x64, 0x000000ff, 0x00000050),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_prof_trigger_0 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_0,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x01, 0x000000ff, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_prof_trigger_1 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_1,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x01, 0x000000ff, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_prof_trigger_2 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_2,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x01, 0x000000ff, 0x00000020),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_prof_trigger_3 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_3,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x01, 0x000000ff, 0x00000030),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_prof_trigger_4 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_4,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x01, 0x000000ff, 0x00000040),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_prof_trigger_5 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_5,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x01, 0x000000ff, 0x00000050),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_prof_trigger_6 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_6,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x01, 0x000000ff, 0x00000060),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_prof_trigger_7 =
{
   .type         = NVC0_HW_SM_QUERY_PROF_TRIGGER_7,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x01, 0x000000ff, 0x00000070),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_shared_ld =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_LD,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x64, 0x000000ff, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_shared_st =
{
   .type         = NVC0_HW_SM_QUERY_SHARED_ST,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x64, 0x000000ff, 0x00000040),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_threads_launched =
{
   .type         = NVC0_HW_SM_QUERY_THREADS_LAUNCHED,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x26, 0x000000ff, 0x00000010),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x26, 0x000000ff, 0x00000020),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0x26, 0x000000ff, 0x00000030),
   .ctr[3]       = _C(0xaaaa, LOGOP, 0x26, 0x000000ff, 0x00000040),
   .ctr[4]       = _C(0xaaaa, LOGOP, 0x26, 0x000000ff, 0x00000050),
   .ctr[5]       = _C(0xaaaa, LOGOP, 0x26, 0x000000ff, 0x00000060),
   .num_counters = 6,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_th_inst_executed_0 =
{
   .type         = NVC0_HW_SM_QUERY_TH_INST_EXECUTED_0,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x2f, 0x000000ff, 0x00000000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x2f, 0x000000ff, 0x00000010),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0x2f, 0x000000ff, 0x00000020),
   .ctr[3]       = _C(0xaaaa, LOGOP, 0x2f, 0x000000ff, 0x00000030),
   .ctr[4]       = _C(0xaaaa, LOGOP, 0x2f, 0x000000ff, 0x00000040),
   .ctr[5]       = _C(0xaaaa, LOGOP, 0x2f, 0x000000ff, 0x00000050),
   .num_counters = 6,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_th_inst_executed_1 =
{
   .type         = NVC0_HW_SM_QUERY_TH_INST_EXECUTED_1,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x30, 0x000000ff, 0x00000000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x30, 0x000000ff, 0x00000010),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0x30, 0x000000ff, 0x00000020),
   .ctr[3]       = _C(0xaaaa, LOGOP, 0x30, 0x000000ff, 0x00000030),
   .ctr[4]       = _C(0xaaaa, LOGOP, 0x30, 0x000000ff, 0x00000040),
   .ctr[5]       = _C(0xaaaa, LOGOP, 0x30, 0x000000ff, 0x00000050),
   .num_counters = 6,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm20_warps_launched =
{
   .type         = NVC0_HW_SM_QUERY_WARPS_LAUNCHED,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x26, 0x000000ff, 0x00000000),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg *sm20_hw_sm_queries[] =
{
   &sm20_active_cycles,
   &sm20_active_warps,
   &sm20_atom_count,
   &sm20_branch,
   &sm20_divergent_branch,
   &sm20_gld_request,
   &sm20_gred_count,
   &sm20_gst_request,
   &sm20_inst_executed,
   &sm20_inst_issued,
   &sm20_local_ld,
   &sm20_local_st,
   &sm20_prof_trigger_0,
   &sm20_prof_trigger_1,
   &sm20_prof_trigger_2,
   &sm20_prof_trigger_3,
   &sm20_prof_trigger_4,
   &sm20_prof_trigger_5,
   &sm20_prof_trigger_6,
   &sm20_prof_trigger_7,
   &sm20_shared_ld,
   &sm20_shared_st,
   &sm20_threads_launched,
   &sm20_th_inst_executed_0,
   &sm20_th_inst_executed_1,
   &sm20_warps_launched,
};

/* ==== Compute capability 2.1 (GF108+ except GF110) ==== */
static const struct nvc0_hw_sm_query_cfg
sm21_inst_executed =
{
   .type         = NVC0_HW_SM_QUERY_INST_EXECUTED,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x2d, 0x000000ff, 0x00000000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0x2d, 0x000000ff, 0x00000010),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0x2d, 0x000000ff, 0x00000020),
   .num_counters = 3,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm21_inst_issued1_0 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED1_0,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x7e, 0x000000ff, 0x00000010),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm21_inst_issued1_1 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED1_1,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x7e, 0x000000ff, 0x00000040),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm21_inst_issued2_0 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED2_0,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x7e, 0x000000ff, 0x00000020),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm21_inst_issued2_1 =
{
   .type         = NVC0_HW_SM_QUERY_INST_ISSUED2_1,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0x7e, 0x000000ff, 0x00000050),
   .num_counters = 1,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm21_th_inst_executed_0 =
{
   .type         = NVC0_HW_SM_QUERY_TH_INST_EXECUTED_0,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0xa3, 0x000000ff, 0x00000000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0xa3, 0x000000ff, 0x00000010),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0xa3, 0x000000ff, 0x00000020),
   .ctr[3]       = _C(0xaaaa, LOGOP, 0xa3, 0x000000ff, 0x00000030),
   .ctr[4]       = _C(0xaaaa, LOGOP, 0xa3, 0x000000ff, 0x00000040),
   .ctr[5]       = _C(0xaaaa, LOGOP, 0xa3, 0x000000ff, 0x00000050),
   .num_counters = 6,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm21_th_inst_executed_1 =
{
   .type         = NVC0_HW_SM_QUERY_TH_INST_EXECUTED_1,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0xa5, 0x000000ff, 0x00000000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0xa5, 0x000000ff, 0x00000010),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0xa5, 0x000000ff, 0x00000020),
   .ctr[3]       = _C(0xaaaa, LOGOP, 0xa5, 0x000000ff, 0x00000030),
   .ctr[4]       = _C(0xaaaa, LOGOP, 0xa5, 0x000000ff, 0x00000040),
   .ctr[5]       = _C(0xaaaa, LOGOP, 0xa5, 0x000000ff, 0x00000050),
   .num_counters = 6,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm21_th_inst_executed_2 =
{
   .type         = NVC0_HW_SM_QUERY_TH_INST_EXECUTED_2,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0xa4, 0x000000ff, 0x00000000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0xa4, 0x000000ff, 0x00000010),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0xa4, 0x000000ff, 0x00000020),
   .ctr[3]       = _C(0xaaaa, LOGOP, 0xa4, 0x000000ff, 0x00000030),
   .ctr[4]       = _C(0xaaaa, LOGOP, 0xa4, 0x000000ff, 0x00000040),
   .ctr[5]       = _C(0xaaaa, LOGOP, 0xa4, 0x000000ff, 0x00000050),
   .num_counters = 6,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg
sm21_th_inst_executed_3 =
{
   .type         = NVC0_HW_SM_QUERY_TH_INST_EXECUTED_3,
   .ctr[0]       = _C(0xaaaa, LOGOP, 0xa6, 0x000000ff, 0x00000000),
   .ctr[1]       = _C(0xaaaa, LOGOP, 0xa6, 0x000000ff, 0x00000010),
   .ctr[2]       = _C(0xaaaa, LOGOP, 0xa6, 0x000000ff, 0x00000020),
   .ctr[3]       = _C(0xaaaa, LOGOP, 0xa6, 0x000000ff, 0x00000030),
   .ctr[4]       = _C(0xaaaa, LOGOP, 0xa6, 0x000000ff, 0x00000040),
   .ctr[5]       = _C(0xaaaa, LOGOP, 0xa6, 0x000000ff, 0x00000050),
   .num_counters = 6,
   .norm         = { 1, 1 },
};

static const struct nvc0_hw_sm_query_cfg *sm21_hw_sm_queries[] =
{
   &sm20_active_cycles,
   &sm20_active_warps,
   &sm20_atom_count,
   &sm20_branch,
   &sm20_divergent_branch,
   &sm20_gld_request,
   &sm20_gred_count,
   &sm20_gst_request,
   &sm21_inst_executed,
   &sm21_inst_issued1_0,
   &sm21_inst_issued1_1,
   &sm21_inst_issued2_0,
   &sm21_inst_issued2_1,
   &sm20_local_ld,
   &sm20_local_st,
   &sm20_prof_trigger_0,
   &sm20_prof_trigger_1,
   &sm20_prof_trigger_2,
   &sm20_prof_trigger_3,
   &sm20_prof_trigger_4,
   &sm20_prof_trigger_5,
   &sm20_prof_trigger_6,
   &sm20_prof_trigger_7,
   &sm20_shared_ld,
   &sm20_shared_st,
   &sm20_threads_launched,
   &sm21_th_inst_executed_0,
   &sm21_th_inst_executed_1,
   &sm21_th_inst_executed_2,
   &sm21_th_inst_executed_3,
   &sm20_warps_launched,
};

#undef _C

static inline const struct nvc0_hw_sm_query_cfg **
nvc0_hw_sm_get_queries(struct nvc0_screen *screen)
{
   struct nouveau_device *dev = screen->base.device;

   switch (screen->base.class_3d) {
   case GM200_3D_CLASS:
      return sm52_hw_sm_queries;
   case GM107_3D_CLASS:
      return sm50_hw_sm_queries;
   case NVF0_3D_CLASS:
      return sm35_hw_sm_queries;
   case NVE4_3D_CLASS:
      return sm30_hw_sm_queries;
   case NVC0_3D_CLASS:
   case NVC1_3D_CLASS:
   case NVC8_3D_CLASS:
      if (dev->chipset == 0xc0 || dev->chipset == 0xc8)
         return sm20_hw_sm_queries;
      return sm21_hw_sm_queries;
   }
   assert(0);
   return NULL;
}

unsigned
nvc0_hw_sm_get_num_queries(struct nvc0_screen *screen)
{
   struct nouveau_device *dev = screen->base.device;

   switch (screen->base.class_3d) {
   case GM200_3D_CLASS:
      return ARRAY_SIZE(sm52_hw_sm_queries);
   case GM107_3D_CLASS:
      return ARRAY_SIZE(sm50_hw_sm_queries);
   case NVF0_3D_CLASS:
      return ARRAY_SIZE(sm35_hw_sm_queries);
   case NVE4_3D_CLASS:
      return ARRAY_SIZE(sm30_hw_sm_queries);
   case NVC0_3D_CLASS:
   case NVC1_3D_CLASS:
   case NVC8_3D_CLASS:
      if (dev->chipset == 0xc0 || dev->chipset == 0xc8)
         return ARRAY_SIZE(sm20_hw_sm_queries);
      return ARRAY_SIZE(sm21_hw_sm_queries);
   }
   return 0;
}

static const struct nvc0_hw_sm_query_cfg *
nvc0_hw_sm_query_get_cfg(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   const struct nvc0_hw_sm_query_cfg **queries;
   struct nvc0_screen *screen = nvc0->screen;
   struct nvc0_query *q = &hq->base;
   unsigned num_queries;
   unsigned i;

   num_queries = nvc0_hw_sm_get_num_queries(screen);
   queries = nvc0_hw_sm_get_queries(screen);

   for (i = 0; i < num_queries; i++) {
      if (NVC0_HW_SM_QUERY(queries[i]->type) == q->type)
         return queries[i];
   }
   assert(0);
   return NULL;
}

static void
nvc0_hw_sm_destroy_query(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   struct nvc0_query *q = &hq->base;
   nvc0_hw_query_allocate(nvc0, q, 0);
   nouveau_fence_ref(NULL, &hq->fence);
   FREE(hq);
}

static bool
nve4_hw_sm_begin_query(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   struct nvc0_screen *screen = nvc0->screen;
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_hw_sm_query *hsq = nvc0_hw_sm_query(hq);
   const struct nvc0_hw_sm_query_cfg *cfg;
   unsigned i, c;
   unsigned num_ab[2] = { 0, 0 };

   cfg = nvc0_hw_sm_query_get_cfg(nvc0, hq);

   /* check if we have enough free counter slots */
   for (i = 0; i < cfg->num_counters; ++i)
      num_ab[cfg->ctr[i].sig_dom]++;

   if (screen->pm.num_hw_sm_active[0] + num_ab[0] > 4 ||
       screen->pm.num_hw_sm_active[1] + num_ab[1] > 4) {
      NOUVEAU_ERR("Not enough free MP counter slots !\n");
      return false;
   }

   assert(cfg->num_counters <= 4);
   PUSH_SPACE(push, 4 * 8 * + 6);

   if (!screen->pm.mp_counters_enabled) {
      screen->pm.mp_counters_enabled = true;
      BEGIN_NVC0(push, SUBC_SW(0x06ac), 1);
      PUSH_DATA (push, 0x1fcb);
   }

   /* set sequence field to 0 (used to check if result is available) */
   for (i = 0; i < screen->mp_count; ++i)
      hq->data[i * 10 + 10] = 0;
   hq->sequence++;

   for (i = 0; i < cfg->num_counters; ++i) {
      const unsigned d = cfg->ctr[i].sig_dom;

      if (!screen->pm.num_hw_sm_active[d]) {
         uint32_t m = (1 << 22) | (1 << (7 + (8 * !d)));
         if (screen->pm.num_hw_sm_active[!d])
            m |= 1 << (7 + (8 * d));
         BEGIN_NVC0(push, SUBC_SW(0x0600), 1);
         PUSH_DATA (push, m);
      }
      screen->pm.num_hw_sm_active[d]++;

      for (c = d * 4; c < (d * 4 + 4); ++c) {
         if (!screen->pm.mp_counter[c]) {
            hsq->ctr[i] = c;
            screen->pm.mp_counter[c] = hsq;
            break;
         }
      }
      assert(c <= (d * 4 + 3)); /* must succeed, already checked for space */

      /* configure and reset the counter(s) */
     if (d == 0)
        BEGIN_NVC0(push, NVE4_CP(MP_PM_A_SIGSEL(c & 3)), 1);
     else
        BEGIN_NVC0(push, NVE4_CP(MP_PM_B_SIGSEL(c & 3)), 1);
     PUSH_DATA (push, cfg->ctr[i].sig_sel);
     BEGIN_NVC0(push, NVE4_CP(MP_PM_SRCSEL(c)), 1);
     PUSH_DATA (push, cfg->ctr[i].src_sel + 0x2108421 * (c & 3));
     BEGIN_NVC0(push, NVE4_CP(MP_PM_FUNC(c)), 1);
     PUSH_DATA (push, (cfg->ctr[i].func << 4) | cfg->ctr[i].mode);
     BEGIN_NVC0(push, NVE4_CP(MP_PM_SET(c)), 1);
     PUSH_DATA (push, 0);
   }

   if (screen->base.class_3d >= GM107_3D_CLASS) {
      /* Enable mask for counters, it's 8-bits value where 0:3 is for domain A
       * and 4:7 for domain B. For example, the mask for active_warps should be
       * 0x70 because it uses 3 counters in domain B. However, let's always
       * enable all counters because we don't want to track which ones is
       * enabled or not, and this allows to monitor multiple queries at the
       * same time. */
      BEGIN_NVC0(push, SUBC_CP(0x33e0), 1);
      PUSH_DATA (push, 0xff);
   }

   return true;
}

static bool
nvc0_hw_sm_begin_query(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   struct nvc0_screen *screen = nvc0->screen;
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_hw_sm_query *hsq = nvc0_hw_sm_query(hq);
   const struct nvc0_hw_sm_query_cfg *cfg;
   unsigned i, c;

   if (screen->base.class_3d >= NVE4_3D_CLASS)
      return nve4_hw_sm_begin_query(nvc0, hq);

   cfg = nvc0_hw_sm_query_get_cfg(nvc0, hq);

   /* check if we have enough free counter slots */
   if (screen->pm.num_hw_sm_active[0] + cfg->num_counters > 8) {
      NOUVEAU_ERR("Not enough free MP counter slots !\n");
      return false;
   }

   assert(cfg->num_counters <= 8);
   PUSH_SPACE(push, 8 * 8 + 2);

   /* set sequence field to 0 (used to check if result is available) */
   for (i = 0; i < screen->mp_count; ++i) {
      const unsigned b = (0x30 / 4) * i;
      hq->data[b + 8] = 0;
   }
   hq->sequence++;

   for (i = 0; i < cfg->num_counters; ++i) {
      uint32_t mask_sel = 0x00000000;

      if (!screen->pm.num_hw_sm_active[0]) {
         BEGIN_NVC0(push, SUBC_SW(0x0600), 1);
         PUSH_DATA (push, 0x80000000);
      }
      screen->pm.num_hw_sm_active[0]++;

      for (c = 0; c < 8; ++c) {
         if (!screen->pm.mp_counter[c]) {
            hsq->ctr[i] = c;
            screen->pm.mp_counter[c] = hsq;
            break;
         }
      }

      /* Oddly-enough, the signal id depends on the slot selected on Fermi but
       * not on Kepler. Fortunately, the signal ids are just offsetted by the
       * slot id! */
      mask_sel |= c;
      mask_sel |= (c << 8);
      mask_sel |= (c << 16);
      mask_sel |= (c << 24);
      mask_sel &= cfg->ctr[i].src_mask;

      /* configure and reset the counter(s) */
      BEGIN_NVC0(push, NVC0_CP(MP_PM_SIGSEL(c)), 1);
      PUSH_DATA (push, cfg->ctr[i].sig_sel);
      BEGIN_NVC0(push, NVC0_CP(MP_PM_SRCSEL(c)), 1);
      PUSH_DATA (push, cfg->ctr[i].src_sel | mask_sel);
      BEGIN_NVC0(push, NVC0_CP(MP_PM_OP(c)), 1);
      PUSH_DATA (push, (cfg->ctr[i].func << 4) | cfg->ctr[i].mode);
      BEGIN_NVC0(push, NVC0_CP(MP_PM_SET(c)), 1);
      PUSH_DATA (push, 0);
   }
   return true;
}

static inline struct nvc0_program *
nvc0_hw_sm_get_program(struct nvc0_screen *screen)
{
   struct nvc0_program *prog;

   prog = CALLOC_STRUCT(nvc0_program);
   if (!prog)
      return NULL;

   prog->type = PIPE_SHADER_COMPUTE;
   prog->translated = true;
   prog->parm_size = 12;

   if (screen->base.class_3d >= GM107_3D_CLASS) {
      prog->code = (uint32_t *)gm107_read_hw_sm_counters_code;
      prog->code_size = sizeof(gm107_read_hw_sm_counters_code);
      prog->num_gprs = 14;
   } else
   if (screen->base.class_3d == NVE4_3D_CLASS ||
       screen->base.class_3d == NVF0_3D_CLASS) {
      if (screen->base.class_3d == NVE4_3D_CLASS) {
         prog->code = (uint32_t *)nve4_read_hw_sm_counters_code;
         prog->code_size = sizeof(nve4_read_hw_sm_counters_code);
      } else {
         prog->code = (uint32_t *)nvf0_read_hw_sm_counters_code;
         prog->code_size = sizeof(nvf0_read_hw_sm_counters_code);
      }
      prog->num_gprs = 14;
   } else {
      prog->code = (uint32_t *)nvc0_read_hw_sm_counters_code;
      prog->code_size = sizeof(nvc0_read_hw_sm_counters_code);
      prog->num_gprs = 12;
   }
   return prog;
}

static inline void
nvc0_hw_sm_upload_input(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   struct nvc0_screen *screen = nvc0->screen;
   uint64_t address;
   const int s = 5;

   address = screen->uniform_bo->offset + NVC0_CB_AUX_INFO(s);

   PUSH_SPACE(push, 11);

   if (screen->base.class_3d >= NVE4_3D_CLASS) {
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_DST_ADDRESS_HIGH), 2);
      PUSH_DATAh(push, address + NVC0_CB_AUX_MP_INFO);
      PUSH_DATA (push, address + NVC0_CB_AUX_MP_INFO);
      BEGIN_NVC0(push, NVE4_CP(UPLOAD_LINE_LENGTH_IN), 2);
      PUSH_DATA (push, 3 * 4);
      PUSH_DATA (push, 0x1);
      BEGIN_1IC0(push, NVE4_CP(UPLOAD_EXEC), 1 + 3);
      PUSH_DATA (push, NVE4_COMPUTE_UPLOAD_EXEC_LINEAR | (0x20 << 1));
   } else {
      BEGIN_NVC0(push, NVC0_CP(CB_SIZE), 3);
      PUSH_DATA (push, NVC0_CB_AUX_SIZE);
      PUSH_DATAh(push, address);
      PUSH_DATA (push, address);
      BEGIN_1IC0(push, NVC0_CP(CB_POS), 1 + 3);
      PUSH_DATA (push, NVC0_CB_AUX_MP_INFO);
   }
   PUSH_DATA (push, (hq->bo->offset + hq->base_offset));
   PUSH_DATAh(push, (hq->bo->offset + hq->base_offset));
   PUSH_DATA (push, hq->sequence);
}

static void
nvc0_hw_sm_end_query(struct nvc0_context *nvc0, struct nvc0_hw_query *hq)
{
   struct nvc0_screen *screen = nvc0->screen;
   struct pipe_context *pipe = &nvc0->base.pipe;
   struct nouveau_pushbuf *push = nvc0->base.pushbuf;
   const bool is_nve4 = screen->base.class_3d >= NVE4_3D_CLASS;
   struct nvc0_hw_sm_query *hsq = nvc0_hw_sm_query(hq);
   struct nvc0_program *old = nvc0->compprog;
   struct pipe_grid_info info = {};
   uint32_t mask;
   uint32_t input[3];
   const uint block[3] = { 32, is_nve4 ? 4 : 1, 1 };
   const uint grid[3] = { screen->mp_count, screen->gpc_count, 1 };
   unsigned c, i;

   if (unlikely(!screen->pm.prog))
      screen->pm.prog = nvc0_hw_sm_get_program(screen);

   /* disable all counting */
   PUSH_SPACE(push, 8);
   for (c = 0; c < 8; ++c)
      if (screen->pm.mp_counter[c]) {
         if (is_nve4) {
            IMMED_NVC0(push, NVE4_CP(MP_PM_FUNC(c)), 0);
         } else {
            IMMED_NVC0(push, NVC0_CP(MP_PM_OP(c)), 0);
         }
      }
   /* release counters for this query */
   for (c = 0; c < 8; ++c) {
      if (screen->pm.mp_counter[c] == hsq) {
         uint8_t d = is_nve4 ? c / 4 : 0; /* only one domain for NVC0:NVE4 */
         screen->pm.num_hw_sm_active[d]--;
         screen->pm.mp_counter[c] = NULL;
      }
   }

   if (screen->base.class_3d >= GM107_3D_CLASS)
      IMMED_NVC0(push, SUBC_CP(0x33e0), 0);

   BCTX_REFN_bo(nvc0->bufctx_cp, CP_QUERY, NOUVEAU_BO_GART | NOUVEAU_BO_WR,
                hq->bo);

   PUSH_SPACE(push, 1);
   IMMED_NVC0(push, SUBC_CP(NV50_GRAPH_SERIALIZE), 0);

   /* upload input data for the compute shader which reads MP counters */
   nvc0_hw_sm_upload_input(nvc0, hq);

   pipe->bind_compute_state(pipe, screen->pm.prog);
   for (i = 0; i < 3; i++) {
      info.block[i] = block[i];
      info.grid[i] = grid[i];
   }
   info.pc = 0;
   info.input = input;
   pipe->launch_grid(pipe, &info);
   pipe->bind_compute_state(pipe, old);

   nouveau_bufctx_reset(nvc0->bufctx_cp, NVC0_BIND_CP_QUERY);

   /* re-activate other counters */
   PUSH_SPACE(push, 16);
   mask = 0;
   for (c = 0; c < 8; ++c) {
      const struct nvc0_hw_sm_query_cfg *cfg;
      unsigned i;

      hsq = screen->pm.mp_counter[c];
      if (!hsq)
         continue;

      cfg = nvc0_hw_sm_query_get_cfg(nvc0, &hsq->base);
      for (i = 0; i < cfg->num_counters; ++i) {
         if (mask & (1 << hsq->ctr[i]))
            break;
         mask |= 1 << hsq->ctr[i];
         if (is_nve4) {
            BEGIN_NVC0(push, NVE4_CP(MP_PM_FUNC(hsq->ctr[i])), 1);
         } else {
            BEGIN_NVC0(push, NVC0_CP(MP_PM_OP(hsq->ctr[i])), 1);
         }
         PUSH_DATA (push, (cfg->ctr[i].func << 4) | cfg->ctr[i].mode);
      }
   }
}

static inline bool
nvc0_hw_sm_query_read_data(uint32_t count[32][8],
                           struct nvc0_context *nvc0, bool wait,
                           struct nvc0_hw_query *hq,
                           const struct nvc0_hw_sm_query_cfg *cfg,
                           unsigned mp_count)
{
   struct nvc0_hw_sm_query *hsq = nvc0_hw_sm_query(hq);
   unsigned p, c;

   for (p = 0; p < mp_count; ++p) {
      const unsigned b = (0x30 / 4) * p;

      for (c = 0; c < cfg->num_counters; ++c) {
         if (hq->data[b + 8] != hq->sequence) {
            if (!wait)
               return false;
            if (BO_WAIT(&nvc0->screen->base, hq->bo, NOUVEAU_BO_RD, nvc0->base.client))
               return false;
         }
         count[p][c] = hq->data[b + hsq->ctr[c]] * (1 << c);
      }
   }
   return true;
}

static inline bool
nve4_hw_sm_query_read_data(uint32_t count[32][8],
                           struct nvc0_context *nvc0, bool wait,
                           struct nvc0_hw_query *hq,
                           const struct nvc0_hw_sm_query_cfg *cfg,
                           unsigned mp_count)
{
   struct nvc0_hw_sm_query *hsq = nvc0_hw_sm_query(hq);
   unsigned p, c, d;

   for (p = 0; p < mp_count; ++p) {
      const unsigned b = (0x60 / 4) * p;

      for (c = 0; c < cfg->num_counters; ++c) {
         count[p][c] = 0;
         for (d = 0; d < ((hsq->ctr[c] & ~3) ? 1 : 4); ++d) {
            if (hq->data[b + 20 + d] != hq->sequence) {
               if (!wait)
                  return false;
               if (BO_WAIT(&nvc0->screen->base, hq->bo, NOUVEAU_BO_RD, nvc0->base.client))
                  return false;
            }
            if (hsq->ctr[c] & ~0x3)
               count[p][c] = hq->data[b + 16 + (hsq->ctr[c] & 3)];
            else
               count[p][c] += hq->data[b + d * 4 + hsq->ctr[c]];
         }
      }
   }
   return true;
}

static bool
nvc0_hw_sm_get_query_result(struct nvc0_context *nvc0, struct nvc0_hw_query *hq,
                            bool wait, union pipe_query_result *result)
{
   uint32_t count[32][8];
   uint64_t value = 0;
   unsigned mp_count = MIN2(nvc0->screen->mp_count_compute, 32);
   unsigned p, c;
   const struct nvc0_hw_sm_query_cfg *cfg;
   bool ret;

   cfg = nvc0_hw_sm_query_get_cfg(nvc0, hq);

   if (nvc0->screen->base.class_3d >= NVE4_3D_CLASS)
      ret = nve4_hw_sm_query_read_data(count, nvc0, wait, hq, cfg, mp_count);
   else
      ret = nvc0_hw_sm_query_read_data(count, nvc0, wait, hq, cfg, mp_count);
   if (!ret)
      return false;

   for (c = 0; c < cfg->num_counters; ++c)
      for (p = 0; p < mp_count; ++p)
         value += count[p][c];
   value = (value * cfg->norm[0]) / cfg->norm[1];

   *(uint64_t *)result = value;
   return true;
}

static const struct nvc0_hw_query_funcs hw_sm_query_funcs = {
   .destroy_query = nvc0_hw_sm_destroy_query,
   .begin_query = nvc0_hw_sm_begin_query,
   .end_query = nvc0_hw_sm_end_query,
   .get_query_result = nvc0_hw_sm_get_query_result,
};

struct nvc0_hw_query *
nvc0_hw_sm_create_query(struct nvc0_context *nvc0, unsigned type)
{
   struct nvc0_screen *screen = nvc0->screen;
   struct nvc0_hw_sm_query *hsq;
   struct nvc0_hw_query *hq;
   unsigned space;

   if (nvc0->screen->base.drm->version < 0x01000101)
      return NULL;

   if (type < NVC0_HW_SM_QUERY(0) || type > NVC0_HW_SM_QUERY_LAST)
      return NULL;

   hsq = CALLOC_STRUCT(nvc0_hw_sm_query);
   if (!hsq)
      return NULL;

   hq = &hsq->base;
   hq->funcs = &hw_sm_query_funcs;
   hq->base.type = type;

   if (screen->base.class_3d >= NVE4_3D_CLASS) {
       /* for each MP:
        * [00] = WS0.C0
        * [04] = WS0.C1
        * [08] = WS0.C2
        * [0c] = WS0.C3
        * [10] = WS1.C0
        * [14] = WS1.C1
        * [18] = WS1.C2
        * [1c] = WS1.C3
        * [20] = WS2.C0
        * [24] = WS2.C1
        * [28] = WS2.C2
        * [2c] = WS2.C3
        * [30] = WS3.C0
        * [34] = WS3.C1
        * [38] = WS3.C2
        * [3c] = WS3.C3
        * [40] = MP.C4
        * [44] = MP.C5
        * [48] = MP.C6
        * [4c] = MP.C7
        * [50] = WS0.sequence
        * [54] = WS1.sequence
        * [58] = WS2.sequence
        * [5c] = WS3.sequence
        */
       space = (4 * 4 + 4 + 4) * nvc0->screen->mp_count * sizeof(uint32_t);
   } else {
      /*
       * Note that padding is used to align memory access to 128 bits.
       *
       * for each MP:
       * [00] = MP.C0
       * [04] = MP.C1
       * [08] = MP.C2
       * [0c] = MP.C3
       * [10] = MP.C4
       * [14] = MP.C5
       * [18] = MP.C6
       * [1c] = MP.C7
       * [20] = MP.sequence
       * [24] = padding
       * [28] = padding
       * [2c] = padding
       */
      space = (8 + 1 + 3) * nvc0->screen->mp_count * sizeof(uint32_t);
   }

   if (!nvc0_hw_query_allocate(nvc0, &hq->base, space)) {
      FREE(hq);
      return NULL;
   }

   return hq;
}

int
nvc0_hw_sm_get_driver_query_info(struct nvc0_screen *screen, unsigned id,
                                 struct pipe_driver_query_info *info)
{
   int count = 0;

   if (screen->base.drm->version >= 0x01000101) {
      if (screen->compute)
         count = nvc0_hw_sm_get_num_queries(screen);
   }

   if (!info)
      return count;

   if (id < count) {
      if (screen->compute) {
         if (screen->base.class_3d <= GM200_3D_CLASS) {
            const struct nvc0_hw_sm_query_cfg **queries =
               nvc0_hw_sm_get_queries(screen);

            info->name = nvc0_hw_sm_query_get_name(queries[id]->type);
            info->query_type = NVC0_HW_SM_QUERY(queries[id]->type);
            info->group_id = NVC0_HW_SM_QUERY_GROUP;
            return 1;
         }
      }
   }
   return 0;
}
